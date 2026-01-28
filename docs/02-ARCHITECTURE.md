<h1 align="center">LeafSense - System Architecture</h1>

---

<h2 align="center">Architecture Diagram</h2>

```
┌───────────────────────────────────────────────────────────────────────────┐
│                              RASPBERRY PI 4B                              │
├───────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │                        APPLICATION LAYER                           │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────────┐  │   │
│  │  │     GUI      │  │   ML Engine  │  │    Data Bridge           │  │   │
│  │  │   (Qt5)      │  │(ONNX Runtime)│  │  (Signal/Slots)          │  │   │
│  │  │              │  │              │  │                          │  │   │
│  │  │ - Dashboard  │  │ - Inference  │  │ - Sensor Updates         │  │   │
│  │  │ - Charts     │  │ - 4 Classes  │  │ - Alert Notifications    │  │   │
│  │  │ - Settings   │  │ - 99.39% Acc │  │ - Log Management         │  │   │
│  │  │ - Alerts     │  │              │  │                          │  │   │
│  │  └──────┬───────┘  └──────┬───────┘  └────────────┬─────────────┘  │   │
│  └─────────┼─────────────────┼───────────────────────┼────────────────┘   │
│            │                 │                       │                    │
│  ┌─────────┼─────────────────┼───────────────────────┼────────────────┐   │
│  │         │           MIDDLEWARE LAYER              │                │   │
│  │  ┌──────▼───────┐  ┌──────▼───────┐  ┌────────────▼─────────────┐  │   │
│  │  │   Master     │  │  MQueue      │  │     Database Manager     │  │   │
│  │  │  Controller  │  │  Handler     │  │        (SQLite)          │  │   │
│  │  │              │  │              │  │                          │  │   │
│  │  │ - Scheduling │  │ - IPC        │  │ - sensor_readings        │  │   │
│  │  │ - Control    │  │ - Commands   │  │ - logs                   │  │   │
│  │  │   Logic      │  │ - Events     │  │ - alerts                 │  │   │
│  │  └──────┬───────┘  └──────────────┘  │ - ml_predictions         │  │   │
│  │         │                            │ - health_assessments     │  │   │
│  │         │                            └──────────────────────────┘  │   │
│  └─────────┼──────────────────────────────────────────────────────────┘   │
│            │                                                              │
│  ┌─────────┼──────────────────────────────────────────────────────────┐   │
│  │         │              DRIVER LAYER                                │   │
│  │  ┌──────▼───────┐  ┌──────────────┐  ┌──────────────────────────┐  │   │
│  │  │   Sensors    │  │  Actuators   │  │     Kernel Module        │  │   │
│  │  │              │  │              │  │       (led.ko)           │  │   │
│  │  │ - DS18B20    │  │ - Pump pH+   │  │                          │  │   │
│  │  │ - pH Sensor  │  │ - Pump pH-   │  │ - /dev/led0              │  │   │
│  │  │ - EC Sensor  │  │ - Pump Nutr. │  │ - GPIO Control           │  │   │
│  │  │ - Camera     │  │              │  │ - ioremap                │  │   │
│  │  └──────────────┘  └──────────────┘  └──────────────────────────┘  │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                           │
├───────────────────────────────────────────────────────────────────────────┤
│                           HARDWARE INTERFACES                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │   I2C    │  │  1-Wire  │  │   GPIO   │  │   CSI    │  │   USB    │     │
│  │  (pH,EC) │  │ (DS18B20)│  │  (Relays)│  │ (Camera) │  │ (Camera) │     │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │
└───────────────────────────────────────────────────────────────────────────┘
```

## System Layers

### 1. Application Layer

#### GUI (Qt5)
- **MainWindow**: Main window with tabs
- **Dashboard**: Panel with system overview
- **SensorsDisplay**: Sensor data visualization
- **HealthDisplay**: Plant health status (ML)
- **AlertsDisplay**: List of alerts and notifications
- **AnalyticsWindow**: Historical charts (Qt5Charts)
- **SettingsWindow**: System settings
- **LogsWindow**: Log visualization

#### ML Engine
- **Model**: MobileNetV3-Small for 4 classes
- **Runtime**: ONNX Runtime 1.16.3
- **Classes**: Healthy, Nutrient Deficiency, Pest Damage, Disease
- **Input**: 224x224 RGB images
- **Accuracy**: 99.39%

#### Data Bridge
- Observer pattern using Qt Signals/Slots
- Synchronization between sensors and UI
- Real-time event notification

### 2. Middleware Layer

#### Master Controller
- Main control loop
- Decision logic for actuators
- Scheduling of sensor readings

#### Message Queue Handler
- Inter-process communication
- Command queue for actuators
- Event buffer

#### Database Manager
- SQLite abstraction
- CRUD operations
- Predefined views for common queries

### 3. Driver Layer

#### Sensor Drivers
```cpp
// Common interface for all sensors
class SensorInterface {
public:
    virtual double read() = 0;
    virtual bool calibrate() = 0;
    virtual std::string getName() = 0;
};
```

#### Actuator Drivers
```cpp
// Common interface for actuators
class ActuatorInterface {
public:
    virtual void activate(int duration_ms) = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() = 0;
};
```

#### Kernel Module (LED)
- Direct GPIO control via memory registers
- Device file: `/dev/led0`
- Operations: open, close, read, write

## Data Flow

### Sensor Reading
```
DS18B20 → 1-Wire Driver → Master Controller → Database → Data Bridge → GUI
```

### pH Control
```
pH Sensor → Master → Decision Logic → Pump Driver → GPIO → Pump
                ↓
            Database (log)
                ↓
            Data Bridge
                ↓
            GUI (notification)
```

### ML Analysis
```
Camera → OpenCV → Preprocessing → ONNX Runtime → Prediction → Database → GUI
```

## Inter-Component Communication

### Qt Signals/Slots
```cpp
// Connection example
connect(dataBridge, &LeafSenseDataBridge::sensorDataUpdated,
        sensorsDisplay, &SensorsDisplay::updateDisplay);

connect(dataBridge, &LeafSenseDataBridge::alertTriggered,
        alertsDisplay, &AlertsDisplay::addAlert);
```

### Message Queue
```cpp
// Message format
struct Message {
    MessageType type;     // SENSOR, COMMAND, LOG, ALERT
    std::string payload;  // Serialized data
    timestamp_t time;     // Timestamp
};
```

## Build Configuration

### Desktop (Development)
```cmake
set(CMAKE_CXX_STANDARD 17)
find_package(Qt5 COMPONENTS Core Gui Widgets Sql Charts REQUIRED)
find_package(OpenCV REQUIRED)
```

### ARM64 (Production)
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-g++)
```

<h2 align="center">External Dependencies</h2>

<div align="center">

| Library | Version | Use |
|------------|--------|-----|
| Qt5 | 5.15.14 | GUI Framework |
| ONNX Runtime | 1.16.3 | ML Inference |
| OpenCV | 4.11.0 | Image Processing |
| SQLite | 3.48.0 | Database |
| BusyBox | 1.37.0 | Linux Utilities |

</div>
