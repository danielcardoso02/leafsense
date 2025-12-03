# LeafSense - Arquitetura do Sistema

## Diagrama de Arquitetura

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              RASPBERRY PI 4B                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        APPLICATION LAYER                            │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────────┐  │   │
│  │  │     GUI      │  │   ML Engine  │  │    Data Bridge           │  │   │
│  │  │   (Qt5)      │  │ (ONNX Runtime)│  │  (Signal/Slots)         │  │   │
│  │  │              │  │              │  │                          │  │   │
│  │  │ - Dashboard  │  │ - Inference  │  │ - Sensor Updates        │  │   │
│  │  │ - Charts     │  │ - 4 Classes  │  │ - Alert Notifications   │  │   │
│  │  │ - Settings   │  │ - 99.39% Acc │  │ - Log Management        │  │   │
│  │  │ - Alerts     │  │              │  │                          │  │   │
│  │  └──────┬───────┘  └──────┬───────┘  └────────────┬─────────────┘  │   │
│  └─────────┼─────────────────┼───────────────────────┼─────────────────┘   │
│            │                 │                       │                      │
│  ┌─────────┼─────────────────┼───────────────────────┼─────────────────┐   │
│  │         │           MIDDLEWARE LAYER              │                 │   │
│  │  ┌──────▼───────┐  ┌──────▼───────┐  ┌───────────▼──────────────┐  │   │
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
│            │                                                               │
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
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                           HARDWARE INTERFACES                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │   I2C    │  │  1-Wire  │  │   GPIO   │  │   CSI    │  │   USB    │     │
│  │  (pH,EC) │  │ (DS18B20)│  │  (Relays)│  │ (Camera) │  │ (Camera) │     │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Camadas do Sistema

### 1. Application Layer

#### GUI (Qt5)
- **MainWindow**: Janela principal com tabs
- **Dashboard**: Painel com visão geral do sistema
- **SensorsDisplay**: Visualização de dados de sensores
- **HealthDisplay**: Estado de saúde das plantas (ML)
- **AlertsDisplay**: Lista de alertas e notificações
- **AnalyticsWindow**: Gráficos históricos (Qt5Charts)
- **SettingsWindow**: Configurações do sistema
- **LogsWindow**: Visualização de logs

#### ML Engine
- **Modelo**: ResNet18 modificado para 4 classes
- **Runtime**: ONNX Runtime 1.16.3
- **Classes**: Healthy, Bacterial_Spot, Early_Blight, Late_Blight
- **Input**: Imagens 224x224 RGB
- **Accuracy**: 99.39%

#### Data Bridge
- Padrão Observer usando Qt Signals/Slots
- Sincronização entre sensores e UI
- Notificação de eventos em tempo real

### 2. Middleware Layer

#### Master Controller
- Loop principal de controlo
- Lógica de decisão para atuadores
- Scheduling de leituras de sensores

#### Message Queue Handler
- Comunicação inter-processo
- Fila de comandos para atuadores
- Buffer de eventos

#### Database Manager
- Abstração sobre SQLite
- CRUD operations
- Views pré-definidas para queries comuns

### 3. Driver Layer

#### Sensor Drivers
```cpp
// Interface comum para todos os sensores
class SensorInterface {
public:
    virtual double read() = 0;
    virtual bool calibrate() = 0;
    virtual std::string getName() = 0;
};
```

#### Actuator Drivers
```cpp
// Interface comum para atuadores
class ActuatorInterface {
public:
    virtual void activate(int duration_ms) = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() = 0;
};
```

#### Kernel Module (LED)
- Controlo direto de GPIO via registos de memória
- Device file: `/dev/led0`
- Operações: open, close, read, write

## Fluxo de Dados

### Leitura de Sensores
```
DS18B20 → 1-Wire Driver → Master Controller → Database → Data Bridge → GUI
```

### Controlo de pH
```
pH Sensor → Master → Decision Logic → Pump Driver → GPIO → Pump
                ↓
            Database (log)
                ↓
            Data Bridge
                ↓
            GUI (notification)
```

### Análise ML
```
Camera → OpenCV → Preprocessing → ONNX Runtime → Prediction → Database → GUI
```

## Comunicação Inter-Componentes

### Qt Signals/Slots
```cpp
// Exemplo de conexão
connect(dataBridge, &LeafSenseDataBridge::sensorDataUpdated,
        sensorsDisplay, &SensorsDisplay::updateDisplay);

connect(dataBridge, &LeafSenseDataBridge::alertTriggered,
        alertsDisplay, &AlertsDisplay::addAlert);
```

### Message Queue
```cpp
// Formato de mensagem
struct Message {
    MessageType type;     // SENSOR, COMMAND, LOG, ALERT
    std::string payload;  // Dados serializados
    timestamp_t time;     // Timestamp
};
```

## Configuração de Build

### Desktop (Desenvolvimento)
```cmake
set(CMAKE_CXX_STANDARD 17)
find_package(Qt5 COMPONENTS Core Gui Widgets Sql Charts REQUIRED)
find_package(OpenCV REQUIRED)
```

### ARM64 (Produção)
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-g++)
```

## Dependências Externas

| Biblioteca | Versão | Uso |
|------------|--------|-----|
| Qt5 | 5.15.14 | GUI Framework |
| ONNX Runtime | 1.16.3 | Inferência ML |
| OpenCV | 4.11.0 | Processamento de imagem |
| SQLite | 3.48.0 | Base de dados |
| BusyBox | 1.37.0 | Utilities Linux |
