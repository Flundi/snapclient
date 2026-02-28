// esp32_amp_handler.c
// Beispiel-HTTP-Handler für /api/amp
// - erwartet POST JSON: { "on":0|1, "persist":0|1 }
// - setzt GPIO21 entsprechend
// - falls persist=1 schreibt er den String in NVS unter Namespace "storage", Key "set_GPIO"

#include <string.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

// Mongoose / webserver header (anpassen falls anderer Webserver genutzt wird)
#include "mongoose.h"

static int parse_json_flag(const char *body, size_t len, const char *key) {
  // sehr einfacher Parser: sucht '"key"' und dann ':' und eine 0/1
  const char *p = body;
  const char *end = body + len;
  size_t klen = strlen(key);
  while (p < end) {
    const char *found = strstr(p, key);
    if (!found) break;
    // look for ':' after key
    const char *colon = strchr(found + klen, ':');
    if (colon && colon < end) {
      // skip spaces
      const char *n = colon + 1;
      while(n < end && (*n == ' ' || *n == '\n' || *n == '\r' || *n == '\t')) n++;
      if(n < end) {
        if(*n == '1') return 1;
        if(*n == '0') return 0;
      }
    }
    p = found + klen;
  }
  return 0; // default
}

static void write_nvs_set_gpio(const char *value) {
  esp_err_t err;
  nvs_handle_t handle;
  err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) return;
  // set as string
  err = nvs_set_str(handle, "set_GPIO", value);
  if (err == ESP_OK) nvs_commit(handle);
  nvs_close(handle);
}

void amp_handler(struct mg_connection *nc, struct http_message *hm) {
  const char *body = hm->body.p;
  size_t blen = hm->body.len;

  int on = parse_json_flag(body, blen, "\"on\"");
  int persist = parse_json_flag(body, blen, "\"persist\"");

  // set gpio21
  gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_21, on ? 1 : 0);

  if (persist) {
    // write NVS key expected by snapclient
    // value example: "21=amp,22=green:0,39=jack:0" — here we write a minimal string
    // If you want to write more keys, adapt value format accordingly.
    const char *val = "21=amp,22=green:0,39=jack:0";
    write_nvs_set_gpio(val);
  }

  // respond JSON
  char resp[128];
  int r = snprintf(resp, sizeof(resp), "{\"ok\":1,\"on\":%d,\"persist\":%d}", on, persist);
  mg_send_head(nc, 200, r, "Content-Type: application/json");
  mg_printf(nc, "%s", resp);
}

// Integration: bei Webserver-Init diesen Handler registrieren, z.B. mit mongoose:
// mg_register_http_endpoint(ev_mgr, "/api/amp", amp_handler, NULL);

// Hinweise:
// - Passe Namespace/Key/Value an die snapclient-Erwartung an.
// - Überprüfe, ob dein Webserver Mongoose nutzt; signature des Handlers ggf. anpassen.
// - Stelle sicher, dass nvs_flash_init() beim Boot bereits aufgerufen wurde.
