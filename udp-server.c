/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
//
#include "net/ip/uip-udp-packet.h"
//
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define DUGUM_SAYISI 3
#define SATIR_SAYISI DUGUM_SAYISI-1
#define SUTUN_SAYISI DUGUM_SAYISI

#define UDP_EXAMPLE_ID  190

#define MAX_PAYLOAD_LEN   30      //BUFFER ICIN EKLENDI

static struct uip_udp_conn *server_conn;
int bilinmeyenX, bilinmeyenY;
double uzaklik[DUGUM_SAYISI], koordinatlar[DUGUM_SAYISI][2], a[SATIR_SAYISI][SUTUN_SAYISI];
PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void 
gaussEliminasyonUygula(double ga[SATIR_SAYISI][SUTUN_SAYISI]){
  int j, k, l, m, sifir_bulundu = 0;
  double katsayi,x[SUTUN_SAYISI];

 
  for(j = 0; j < SATIR_SAYISI; j++){                  //ilk kosegenden basla

    if(ga[j][j] != 1){                                //eger kosegeni 1 degilse

      if(ga[j][j] == 0){                              //eger kosegeni 0 ise
        for(l = j; l < SATIR_SAYISI; l++){            //ayni sutunda 0 olmayan degeri bul
          if(ga[l][j] != 0){                          //0 olmayan degeri bulursan bunu isaretle
            sifir_bulundu = 1;
            for(k = 0; k < SUTUN_SAYISI; k++){        //0 olmayan satirla 0 olan satirin yerini degistir
              x[k] = ga[j][k];
              ga[j][k] = ga[l][k];
              ga[l][k] = x[k];
            }
          } 
          if(sifir_bulundu == 1)  l = SATIR_SAYISI-1; //sifir bulunduysa donguden cikmasini sagla
          else if(sifir_bulundu == 0)  printf("%d. degisken icin tum sutun degerleri sifir, deger belirlenemedi.",l);
        }
      }
      
      else{                                           //kosegeni 1 yapmak icin butun elemanları kosegenin degerine bol
        katsayi = ga[j][j];
        for(k = j; k < SUTUN_SAYISI; k++){
          if(ga[j][k] != 0){                          //sutun degeri 0 ise bolme
            ga[j][k] = ga[j][k] / katsayi;
          }
        }
      }

    }

// ALT UCGENI SIFIRLA
    if(j != SATIR_SAYISI - 1){                          
      for(k = j + 1; k < SATIR_SAYISI; k++){            
        if(ga[k][j] != 0){
        katsayi = ga[k][j];
          for(l = j; l < SUTUN_SAYISI; l++){            //kosegen satirinin tum degerlerini katsayiyla carpip sifirlanacak
            ga[k][l] -= ga[j][l] * katsayi;             //satir degerlerinden cikart
          }
        }
      }
    }

//UST UCGEN ERITILIYOR
    else{
  
      for(k = j; k >= 0; k--){
        for(m = k - 1; m >= 0; m--){
          if(ga[m][k] != 0){
            katsayi = ga[m][k];
            for(l = k; l < SUTUN_SAYISI; l++){
              ga[m][l] -= ga[k][l] * katsayi;
            }
          }
        }
      }
        
    }

  bilinmeyenX = ga[0][SUTUN_SAYISI-1];
  bilinmeyenY = ga[1][SUTUN_SAYISI-1];
  
  }
  
}


/*---------------------------------------------------------------------------*/

static void
tcpip_handler(void)
{
  char *appdata = '\0';
  appdata = (char *)uip_appdata;
  appdata[uip_datalen()] = 0;
  int i = 0;
  if(uip_newdata()){
    if(strstr(appdata,"setx")){
      koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] = 0;
      if(appdata[5 + i] != '-'){
        while(appdata[5 + i] != '\0'){
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] *= 10.0;
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] += appdata[5 + i] - '0';
          ++i;
        }
      }
      else{
        while(appdata[6 + i] != '\0'){
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] *= 10.0;
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] += appdata[6 + i] - '0';
          ++i; 
        }
        koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0] = 0 - koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][0];
      }
    }
    else if(strstr(appdata,"sety")){
      koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] = 0;
      if(appdata[5 + i] != '-'){
        while(appdata[5 + i] != '\0'){
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] *= 10.0;
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] += appdata[5 + i] - '0';
          ++i;
        }
      }
      else{
        while(appdata[6 + i] != '\0'){
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] *= 10.0;
          koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] += appdata[6 + i] - '0';
          ++i;
        }
        koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1] = 0 - koordinatlar[UIP_IP_BUF->srcipaddr.u8[15] - 2][1];
      }
      i = 0;
    }

    else if(strstr(appdata,"uzaklik")){
      uzaklik[UIP_IP_BUF->srcipaddr.u8[15] - 2] = 0 - packetbuf_attr(PACKETBUF_ATTR_RSSI);
    }
    else if(strstr(appdata,"hesapla")){
      for(i = 0; i < SATIR_SAYISI; i++){
        a[i][0] = (int)((-2)*koordinatlar[i][0] + 2*koordinatlar[i+1][0]);
        a[i][1] = (int)((-2)*koordinatlar[i][1] + 2*koordinatlar[i+1][1]);
        a[i][2] = (int)(uzaklik[i] * uzaklik[i]) - (uzaklik[i+1] * uzaklik[i+1]) - (koordinatlar[i][0] * koordinatlar[i][0]) + (koordinatlar[i+1][0] * koordinatlar[i+1][0]) - (koordinatlar[i][1] * koordinatlar[i][1]) + (koordinatlar[i+1][1] * koordinatlar[i+1][1]);
      }

      for(i = 0; i < DUGUM_SAYISI;i++){
        printf("%d id'li dugumun uzakligi: %d \n", i+2,(int)uzaklik[i]);
      }

      gaussEliminasyonUygula(a);
      printf("Istenen nokta: (%d,%d)\n", bilinmeyenX, bilinmeyenY);

    }
    else{
      PRINTF("Veri: '%s' Gonderen Istemci: ", appdata);
      PRINTF("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
      PRINTF("\n");
    } 
  }

#if SERVER_REPLY
    PRINTF("Cevap gonderiliyor\n");
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
    uip_create_unspecified(&server_conn->ripaddr);
#endif
}
/*---------------------------------------------------------------------------*/

static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Sunucu IPv6 adresleri: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	      uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;
 // int konumX, konumY;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

//  SENSORS_ACTIVATE(button_sensor);

  PRINTF("UDP sunucu baslatildi, komsular:%d yollar:%d\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoWPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the
 * 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::. At present Wireshark copies Context/128 and
 * then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit
 * compressed address of fd00::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct
 * uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("Yeni bir RPL grafi olusturuldu\n");
  } else {
    PRINTF("RPL grafi olusturulurken hata!\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("Uygun UDP baglantisi bulunamadi, islem sonlandiriliyor!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));


  PRINTF("Uzaktan erisim adresiyle yeni bir sunucu balantisi olusturuldu ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" yerel/uzaktan erisim portlari %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
