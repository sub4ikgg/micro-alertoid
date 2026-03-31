# Alertoid

ESP32-based alert device that monitors HTTP endpoint availability and signals the result via LEDs. Configure Wi-Fi credentials and the monitored URL wirelessly over BLE using a companion app.

## How it works

1. On first boot, hold the **BOOT button** to enter BLE configuration mode.
2. Send Wi-Fi credentials and the URL to monitor via BLE.
3. The device connects to Wi-Fi and starts polling the URL at the configured interval.
4. LEDs indicate the current state at a glance.

## LED States

| State | Yellow | Green | Red |
|---|---|---|---|
| Waiting for Wi-Fi | blinks | blinks | off |
| Resource OK | off | triple-blinks, stays on | off |
| Resource unavailable | off | off | rapid blinks, stays on |
| BLE advertising | blinks | off | blinks (sync) |
| BLE connected | alternates with red | off | alternates with yellow |

## Hardware

| Component | GPIO |
|---|---|
| Yellow LED | 27 |
| Green LED | 25 |
| Red LED | 32 |
| BOOT button | 0 (built-in) |

## BLE Configuration

Device advertises as `Alertoid-<MAC>`. Connect with any BLE client that supports GATT writes.

**Service UUID:** `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`

| Characteristic | UUID suffix | Direction | Payload |
|---|---|---|---|
| Wi-Fi config | `...03` | Write | `{"ssid":"...","passphrase":"..."}` |
| URL config | `...04` | Write | `{"url":"...","code":200,"check_interval":30}` |
| URL config | `...07` | Read | `{"url":"...","code":200,"check_interval":30}` |
| Firmware info | `...05` | Read | `{"firmware":"1.0.0","mac":"...","serial":"..."}` |
| Reboot | `...06` | Write | `"reboot"` |

`check_interval` is in seconds (minimum effective value: 1 s, default: 5 s).

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Flash
pio run --target upload

# Monitor serial output (115200 baud)
pio device monitor

# Flash and monitor
pio run --target upload && pio device monitor
```

## Configuration Persistence

Settings survive reboots via ESP32 NVS (Non-Volatile Storage):

| Namespace | Keys | Default |
|---|---|---|
| `wifi_prefs` | `wifi_ssid`, `wifi_passphrase` | — |
| `res_prefs` | `res_url`, `res_code`, `res_interval` | `https://httpbin.org/status/200`, `200`, `50` ticks |
| `ble` | `reboot` (bool) | `false` |

## Project Structure

```
src/
├── main.cpp          # Main loop and operating mode state machine
├── debug.h           # LOG() macro
├── ble/              # BLE server and characteristics
├── led/              # LED GPIO control and blink patterns
├── resource/         # HTTPS availability check
└── wifi/             # Wi-Fi connection management
docs/
└── led-blink-spec.md # Detailed LED timing specification
```

## License

See [LICENSE](LICENSE).
