#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "set_default_nvs";

// Default values to write if key is absent
static const char *DEFAULT_SET_GPIO = "21=amp,22=green:0,39=jack:0";
static const char *DEFAULT_DAC_CONFIG = "model=ES8388,bck=27,ws=25,do=26,sda=33,scl=32,i2c=16";
static const char *DEFAULT_METADATA_CONFIG = "format=%artist% - %title%,artwork:0";

// Writes `key` with `value` into namespace "storage" only if key is not present.
static esp_err_t write_if_missing(const char *key, const char *value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    // check if key exists
    size_t required = 0;
    err = nvs_get_str(handle, key, NULL, &required);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "NVS key '%s' already exists (len=%u), skipping", key, (unsigned)required);
        nvs_close(handle);
        return ESP_OK;
    }

    if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "nvs_get_str for '%s' returned %s (will attempt to set)", key, esp_err_to_name(err));
    }

    err = nvs_set_str(handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str('%s') failed: %s", key, esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    ESP_LOGI(TAG, "Wrote default NVS '%s' -> '%s'", key, value);
    nvs_close(handle);
    return ESP_OK;
}

// Public helper: call this early in startup to ensure defaults are present.
void set_default_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs erasing
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return;
    }

    write_if_missing("set_GPIO", DEFAULT_SET_GPIO);
    write_if_missing("dac_config", DEFAULT_DAC_CONFIG);
    write_if_missing("metadata_config", DEFAULT_METADATA_CONFIG);
}

/*
Usage:
- Add this file into your firmware source (`src/`), build and flash.
- Call `set_default_nvs()` early in `app_main()` or initialization path:

    void app_main(void)
    {
        set_default_nvs();
        // ...rest of init
    }

This will only write the keys if they are not already present.
*/
