# PR: Web-UI Amp toggle + set_default_nvs helper

This branch `feature/web-ui-amp-toggle` contains two additions to help enable the onboard amp (GPIO21) on ESP32-A1S / ES8388 boards:

- `html/amp-toggle.js` — small client-side UI to toggle amp and optionally persist the mapping via a HTTP endpoint.
- `src/esp32_amp_handler.c` — example HTTP handler that toggles GPIO21 and writes `set_GPIO` to NVS when requested (example integration).
- `src/set_default_nvs.c` — helper that writes default NVS keys on first boot if missing:
  - `set_GPIO` = `21=amp,22=green:0,39=jack:0`
  - `dac_config` = `model=ES8388,bck=27,ws=25,do=26,sda=33,scl=32,i2c=16`
  - `metadata_config` = `format=%artist% - %title%,artwork:0`

How to build & test locally
1. Checkout this branch:

```powershell
cd C:\repos\snapclient_work_20260228152702
git checkout feature/web-ui-amp-toggle
```

2. Ensure `set_default_nvs()` is called early in `app_main()` (or include `esp32_amp_handler.c` integration) so NVS keys get written on first boot. Example:

```c
// in app_main.c
void set_default_nvs(void);

void app_main(void)
{
    set_default_nvs();
    // existing init
}
```

3. Build & flash (PlatformIO example):

```powershell
pio run -e <env> -t upload
```

4. Monitor serial log (115200) — you should see logs like:

```
I (###) set_default_nvs: Wrote default NVS 'set_GPIO' -> '21=amp,...'
```

5. Start playback from Music Assistant to the device — the onboard amp should enable when playback starts (and remain enabled according to the saved mapping).

Rollback / cleanup
- After a successful first-boot and verification, remove the `set_default_nvs()` call or keep it — it's idempotent and will skip writing existing keys.

Notes
- This helper uses ESP-IDF NVS APIs; adjust includes if your build system differs.
- The `html/amp-toggle.js` is a minimal example — to fully integrate, add an HTTP endpoint that accepts JSON {on:0|1, persist:0|1} and calls the handler shown in `src/esp32_amp_handler.c`.
