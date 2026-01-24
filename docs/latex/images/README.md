# Images Directory

This folder contains all images/screenshots for the LeafSense Results Chapter.

**Last Updated:** January 2026

---

## Captured Images Inventory

### GUI Screenshots (10 images)
All GUI screenshots captured remotely from Raspberry Pi framebuffer (fb1, 480x320 RGB565).

| Filename | Description | Captured |
|----------|-------------|----------|
| `gui_login.png` | Login screen with username/password fields | ✅ |
| `gui_main_dashboard.png` | Main dashboard with health status and sensor readings | ✅ |
| `gui_analytics_sensors.png` | Analytics window - Sensor Readings tab | ✅ |
| `gui_analytics_trends.png` | Analytics window - Trends/Charts tab | ✅ |
| `gui_analytics_gallery.png` | Analytics window - Image Gallery tab | ✅ |
| `gui_logs.png` | Logs window with tabbed categories | ✅ |
| `gui_settings.png` | Settings window with configuration options | ✅ |
| `gui_info.png` | Info/About window | ✅ |
| `gui_logout_popup.png` | Logout confirmation dialog | ✅ |
| `gui_dark_mode.png` | Dashboard in dark mode theme | ✅ |

### LED Driver Screenshots (3 images)
Terminal screenshots showing kernel module operations.

| Filename | Description | Captured |
|----------|-------------|----------|
| `led_module_load.png` | Module loading with insmod, lsmod, device creation | ✅ |
| `led_control_test.png` | LED ON/OFF control and dmesg output | ✅ |
| `led_module_unload.png` | Module cleanup with rmmod | ✅ |

### Database Screenshots (4 images)
SQLite database schema and query results.

| Filename | Description | Captured |
|----------|-------------|----------|
| `db_schema_tables_1.png` | Database tables listing (.tables) | ✅ |
| `db_schema_tables_2.png` | Table schemas - Part 1 | ✅ |
| `db_schema_tables_3.png` | Table schemas - Part 2 | ✅ |
| `db_query_results.png` | Sample query output (sensor readings, alerts) | ✅ |

### Machine Learning Screenshots (2 images + 1 photo)
ML model verification and inference results.

| Filename | Description | Captured |
|----------|-------------|----------|
| `ml_model_verification.png` | Model file verification, classes.txt | ✅ |
| `ml_inference_result.png` | Application log showing ML prediction | ✅ |
| `ml_captured_plant.jpg` | Sample plant image captured by camera | ✅ |

### Hardware Driver Evidence (2 images)
Buildroot system and driver verification.

| Filename | Description | Captured |
|----------|-------------|----------|
| `buildroot_evidence_1.png` | System info, os-release, uname | ✅ |
| `buildroot_evidence_2.png` | Application deployment verification | ✅ |

### LED Alert System (2 images)
Integration of LED driver with application alerts.

| Filename | Description | Captured |
|----------|-------------|----------|
| `led_alert_system_1.png` | LED alert integration log output | ✅ |
| `led_alert_system_2.png` | Sensor readings and alert correlation | ✅ |

### Driver Evidence (2 images)
Hardware driver verification screenshots.

| Filename | Description | Captured |
|----------|-------------|----------|
| `driver_evidence_1.png` | Display/touchscreen driver status | ✅ |
| `driver_evidence_2.png` | Camera driver status | ✅ |

---

## Image Capture Methodology

GUI screenshots were captured using remote framebuffer extraction:

```bash
# On host machine
ssh root@10.42.0.196 "cat /dev/fb1 > /tmp/screenshot.raw"
scp root@10.42.0.196:/tmp/screenshot.raw /tmp/

# Convert RGB565 to PNG using Python
python3 convert_rgb565.py /tmp/screenshot.raw output.png
```

See `../../deploy/screenshot_pi.sh` for automated capture script.

---

## Image Format Specifications

- **GUI Screenshots**: PNG, 480x320 pixels (native framebuffer resolution)
- **Terminal Screenshots**: PNG, variable resolution
- **Plant Photos**: JPEG, camera native resolution (2592x1944)

---

## Naming Convention

Pattern: `{category}_{description}.{ext}`

Examples:
- `gui_login.png` - GUI category, login screen
- `led_module_load.png` - LED category, module loading
- `db_schema_tables_1.png` - Database category, schema part 1
- `ml_captured_plant.jpg` - ML category, captured plant image
