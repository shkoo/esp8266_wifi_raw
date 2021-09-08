#include "wifi_raw.h"

#include <lwip/netif.h>
#include <lwip/pbuf.h>

#include "mem.h"
#include "osapi.h"

#define STATION_IF 0x00
#define STATION_MODE 0x01

#define SOFTAP_IF 0x01
#define SOFTAP_MODE 0x02

static wifi_raw_recv_cb_fn rx_func = NULL;

/* Do not add the ICACHE_FLASH_ATTR attribute here!
   The program randomly hangs and restarts after a
   watchdog reset if the attribute is put.

   My guess is that this is some kind of interrupt
   that should execute as fast as possible, and
   ICACHE_FLASH_ATTR stores it in a slower-to-access location... */
IRAM_ATTR void __wrap_ppEnqueueRxq(void *a) {
  // int i;
  // for (i = 0; i < 30; i++){
  // 	ets_uart_printf("%p ", ((void **)a)[i]);
  // 	if((uint32)((void**)a)[i]>0x30000000){
  // 		ets_uart_printf("Pointer greater than 0x30000000:\n");
  // 		int j;
  // 		for (j = 0; j < 100; j++){
  // 			ets_uart_printf("%02x ", ((uint8 **)a)[i][j]);
  // 		}
  // 		ets_uart_printf("\n\n");
  // 	}
  // }
  //	if (rx_func == NULL) {
  //		ets_uart_printf("Rx func is null\n");
  //	}

  // 4 is the only spot that contained the packets
  // Discovered by trial and error printing the data
  if (rx_func) rx_func((struct RxPacket *)(((void **)a)[4]));

  __real_ppEnqueueRxq(a);
}

void wifi_raw_set_recv_cb(wifi_raw_recv_cb_fn rx_fn) { rx_func = rx_fn; }

static void* transmit_buf = 0;

static void send_callback(uint8_t status) {
  free(transmit_buf);
  transmit_buf = 0;
}

int wifi_send_raw_packet(void *data, uint16_t len) {
  static uint8_t initted = 0;
  if (!initted) {
    wifi_register_send_pkt_freedom_cb(send_callback);
    initted = 1;
  }

  if (transmit_buf) {
    return -1;
  }
  transmit_buf = data;

  return wifi_send_pkt_freedom(transmit_buf, len, true);
}
