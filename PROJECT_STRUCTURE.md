# LeafSense Project Structure

**Version:** 1.4.0  
**Last Updated:** January 10, 2026

## Directory Overview

```
leafsense-project/
├── CMakeLists.txt              # Root build configuration
├── README.md                   # Project overview and quick start
├── .gitignore                  # Git exclusions
│
├── src/                        # Source code (C++)
│   ├── main.cpp               # Application entry point
│   ├── application/           # GUI and ML implementation
│   ├── drivers/               # Sensor and actuator drivers
│   └── middleware/            # Core logic (Master, database, queue)
│
├── include/                    # Header files
│   ├── application/           # GUI and ML headers
│   ├── drivers/               # Driver headers
│   └── middleware/            # Middleware headers
│
├── docs/                       # Complete documentation (17 files)
│   ├── 00-TERMINOLOGY.md      # Technical terms & concepts (NEW)
│   ├── 01-OVERVIEW.md         # System overview
│   ├── 02-ARCHITECTURE.md     # Architecture design
│   ├── 03-MACHINE-LEARNING.md # ML implementation
│   ├── 04-RASPBERRY-PI-DEPLOYMENT.md
│   ├── 05-DEVICE-DRIVER.md    # Device driver guide (was 13)
│   ├── 06-DATABASE.md
│   ├── 07-GUI.md
│   ├── 08-TROUBLESHOOTING.md
│   ├── 09-CHANGELOG.md
│   ├── 10-IMPLEMENTATION-REPORT.md
│   ├── 11-SENSOR-ACTUATOR-INTEGRATION.md
│   ├── 11-TESTING-GUIDE.md
│   ├── 12-DEMO-GUIDE.md
│   ├── 13-KERNEL-MODULE.md    # Low-level kernel (was 05)
│   ├── FINAL-STATUS.md        # System status report
│   ├── PROJECT-STATUS.md      # Development status
│   └── README.md              # Documentation index
│
├── resources/                  # Qt resources
│   ├── resources.qrc          # Resource file list
│   └── images/                # GUI icons and images
│
├── ml/                         # Machine Learning
│   ├── leafsense_model.onnx   # Trained model (5.9MB)
│   ├── leafsense_model_classes.txt  # Class names
│   ├── train_model.py         # Training script
│   └── dataset_4class/        # Training dataset
│
├── database/                   # Database schemas
│   └── schema.sql             # SQLite schema definition
│
├── drivers/                    # Kernel modules
│   └── kernel_module/         # LED control driver
│       ├── ledmodule.c        # Main driver code
│       ├── utils.c/h          # Helper functions
│       └── Makefile           # Module build
│
├── deploy/                     # Deployment tools
│   ├── README.md              # Deployment guide
│   ├── configure-buildroot.sh # System configuration
│   ├── setup-onnxruntime-arm64.sh
│   ├── setup-waveshare35c.sh  # Display setup
│   ├── toolchain-rpi4.cmake   # CMake toolchain
│   ├── waveshare35c-config.txt
│   └── waveshare35c.dtbo
│
├── external/                   # Third-party libraries
│   └── onnxruntime-arm64/     # Pre-compiled ONNX Runtime
│       ├── include/           # Headers
│       └── lib/               # Libraries
│
├── tests/                      # Test programs (NEW in v1.4.0)
│   ├── README.md              # Test documentation
│   ├── test_ml.cpp            # ML inference testing
│   └── test_ml_dataset.cpp    # Dataset validation
│
├── scripts/                    # Utility scripts (NEW in v1.4.0)
│   ├── README.md              # Script documentation
│   └── cross-compile-arm64.sh # Cross-compilation helper
│
└── build-arm64/                # Build artifacts (gitignored)
    └── LeafSense              # Compiled binary
```

## Build Artifacts (Excluded from Git)

The following directories are generated during build and excluded via `.gitignore`:

- `build/` - x86 build directory
- `build-arm64/` - ARM64 cross-compilation build
- `*_autogen/` - Qt MOC generated files
- `CMakeFiles/` - CMake temporary files

## Source Code Organization

### Application Layer
- **GUI** (`src/application/gui/`) - Qt5 interface implementation
  - `dashboard_window.cpp` - Main dashboard
  - `analytics_window.cpp` - Charts and gallery
  - `settings_window.cpp` - System configuration
  - Theme management system

- **ML** (`src/application/ml/`) - Machine learning inference
  - `MLEngine.cpp` - ONNX Runtime integration
  - Disease detection (4 classes)

### Driver Layer
- **Sensors** (`src/drivers/sensors/`) - Data acquisition
  - `Temp.cpp` - DS18B20 temperature sensor
  - `PHSensor.cpp` - pH electrode
  - `TDS.cpp` - EC/TDS sensor
  - `Cam.cpp` - Camera capture with CV enhancement

- **Actuators** (`src/drivers/actuators/`) - Control outputs
  - `PeristalticPump.cpp` - Dosing pumps
  - `PWMHeater.cpp` - Temperature control
  - `LED.cpp` - Alert indicator (kernel module interface)

### Middleware Layer
- **Master** (`src/middleware/Master.cpp`) - Main control loop
  - Sensor reading every 2 seconds
  - Automatic control decisions
  - Camera capture every 30 minutes
  - ML analysis integration

- **Database** (`src/middleware/dDatabase.cpp`) - Data persistence
  - Message queue daemon
  - SQLite integration
  - Enhanced SUCCESS/FAILED logging

## Documentation Structure

All documentation is consolidated in the `docs/` directory:

1. **Technical Guides** (01-08) - Implementation details
2. **Status Reports** (09-10, PROJECT-STATUS, FINAL-STATUS) - Project status
3. **Integration Guides** (11-13) - Hardware integration
4. **User Guides** (12-DEMO) - Operation instructions

## Key Files

- **CMakeLists.txt** (root) - Top-level build config
- **src/CMakeLists.txt** - Source build config
- **README.md** - Project quick start
- **docs/FINAL-STATUS.md** - Complete system status
- **.gitignore** - Build artifact exclusions

## Adding New Components

### New Source File
1. Add `.cpp` to `src/<layer>/<component>/`
2. Add `.h` to `include/<layer>/<component>/`
3. Update `src/CMakeLists.txt`

### New Documentation
1. Add `.md` to `docs/`
2. Update `docs/01-OVERVIEW.md` documentation list
3. Update `docs/README.md` index

### New Test
1. Add `.cpp` to `tests/`
2. Update `tests/README.md`
3. Add build instructions

## Build Directories

### Native Build (x86)
```bash
mkdir build && cd build
cmake ../src
make
```

### Cross-Compile (ARM64)
```bash
mkdir build-arm64 && cd build-arm64
cmake ../src -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake
make -j4
```

## Design Principles

1. **Separation of Concerns**
   - Application, Drivers, Middleware clearly separated
   - Headers in `include/`, source in `src/`

2. **Documentation First**
   - All docs in `docs/` - no scattered files
   - Numbered guides for easy navigation

3. **Clean Build Artifacts**
   - All builds in `build*/` directories
   - Excluded from version control

4. **Test Isolation**
   - Tests separate from production code
   - Easy to run without affecting main binary

5. **Deployment Ready**
   - All deployment tools in `deploy/`
   - Scripts for automation in `scripts/`

## Version History

- **v1.0.0** - Initial structure
- **v1.3.0** - Camera system completion
- **v1.4.0** - CV enhancements, reorganization, consolidated docs

---

For detailed information on any component, see the comprehensive guides in `docs/`.
