# LeafSense - Implementation Report

**Group 11**

**Authors:**
- Daniel Gonçalo Silva Cardoso, PG53753
- Marco Xavier Leite Costa, PG60210

**Specialization:** Embedded Systems and Computers  
**Program:** Master's in Industrial Electronics and Computers Engineering  
**Advisor:** Professor Adriano José Conceição Tavares  
**Date:** December 2025

---

## 4. Implementation

### 4.1 Environment de Development

O desenvolvimento do projeto LeafSense foi realizado utilizando as seguintes ferramentas e tecnologias:

| Componente | Ferramenta/Version |
|------------|-------------------|
| System Operativo (Host) | Ubuntu 22.04 LTS |
| IDE | Visual Studio Code |
| Linguagem Principal | C++17 |
| System de Build | CMake 3.22+ |
| Control de Versions | Git / GitHub |
| Framework GUI | Qt 5.15.14 |
| ML Framework (Treino) | PyTorch 2.0 |
| ML Runtime (Inference) | ONNX Runtime 1.16.3 |
| Visão Computacional | OpenCV 4.11.0 |
| Base de Data | SQLite 3.48.0 |

Para o desenvolvimento embebido, foi utilizado o **Buildroot 2025.08** como sistema de build para criar uma distribuição Linux customizada para a Raspberry Pi 4.

### 4.2 Cross-Compilation para Raspberry Pi

#### 4.2.1 Configuration do Buildroot

O Buildroot foi configurado a partir da configuração base `raspberrypi4_64_defconfig`, com as seguintes customizações:

```bash
# Configuration base
make raspberrypi4_64_defconfig

# Customizações via menuconfig
make menuconfig
```

**Opções habilitadas:**

| Categoria | Packages |
|-----------|---------|
| Toolchain | glibc, C++ support, GCC 14.3.0 |
| Qt5 | qt5base, qt5charts, qt5svg, qt5sql |
| Graphics | OpenCV 4.11.0 |
| Database | SQLite 3 |
| Networking | Dropbear SSH, dhcp client |
| Hardware | i2c-tools, 1-Wire support |
| Filesystem | ext4, 512MB root partition |

#### 4.2.2 Toolchain de Cross-Compilation

Foi criado um ficheiro CMake toolchain (`deploy/toolchain-rpi4.cmake`) para permitir a compilação cruzada:

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN_PREFIX "~/buildroot/buildroot-2025.08/output/host")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-g++")
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")
```

A compilação é realizada com:

```bash
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

O binário resultante tem aproximadamente 380KB e é um executável ELF 64-bit para ARM aarch64.

### 4.3 Camada de Drivers

#### 4.3.1 Module de Kernel (LED Driver)

Foi desenvolvido um módulo de kernel Linux para controlo de um LED indicador através de GPIO. O módulo implementa um character device driver que expõe o dispositivo `/dev/led0`.

**Características principais:**

- Acesso direto aos registos GPIO do BCM2711 via `ioremap`
- Endereço base GPIO: `0xFE200000`
- GPIO utilizado: Pin 20
- Operações suportadas: `open`, `close`, `read`, `write`

**Estrutura do código:**

```c
// Registos GPIO
#define GPIO_BASE   0xFE200000
#define GPFSEL2     0x08    // Function Select (GPIO 20-29)
#define GPSET0      0x1C    // Set Output High
#define GPCLR0      0x28    // Clear Output Low

// Mapear registos para memória virtual
gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);

// Configurar GPIO como output
SetGPIOOutput(LED_PIN);

// Controlar LED
SetGPIOOutputValue(LED_PIN, value);  // 1=ON, 0=OFF
```

**Nota técnica:** A função `ioremap_nocache` foi substituída por `ioremap` devido a alterações na API do kernel Linux 5.6+.

A compilação do módulo requer os headers do kernel:

```bash
export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
export CROSS_COMPILE=~/buildroot/.../bin/aarch64-linux-
export ARCH=arm64
make
```

#### 4.3.2 Drivers de Sensors

Os drivers de sensores foram implementados seguindo uma interface comum:

```cpp
class SensorInterface {
public:
    virtual double read() = 0;
    virtual bool calibrate() = 0;
    virtual std::string getName() = 0;
};
```

**Sensors implementados:**

| Sensor | Interface | Description |
|--------|-----------|-----------|
| DS18B20 | 1-Wire (GPIO4) | Temperatura da solução |
| pH Sensor | I2C | Medição de pH (0-14) |
| EC Sensor | I2C | Condutividade elétrica |

Para testes e desenvolvimento, foi implementado um modo mock que simula leituras realistas dos sensores.

#### 4.3.3 Drivers de Actuatores

Os atuadores (bombas doseadoras) são controlados através de relés conectados a GPIOs:

```cpp
class ActuatorInterface {
public:
    virtual void activate(int duration_ms) = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() = 0;
};
```

**Actuatores implementados:**

| Actuator | GPIO | Function |
|---------|------|--------|
| Bomba pH Up | 12 | Aumentar pH |
| Bomba pH Down | 13 | Diminuir pH |
| Bomba Nutrientes | 16 | Adicionar nutrientes |

### 4.4 Camada de Middleware

#### 4.4.1 Base de Data (SQLite)

A persistência de dados é gerida através de SQLite, com o seguinte schema:

**Tables principais:**

| Table | Description |
|--------|-----------|
| `sensor_readings` | Readings de temperatura, pH, EC |
| `logs` | Eventos e ações do sistema |
| `alerts` | Alerts com severidade |
| `ml_predictions` | Resultados de inferência ML |
| `ml_detections` | Deteções de doenças |
| `ml_recommendations` | Recomendações de ação |
| `plant` | Registo de plantas |
| `health_assessments` | Avaliações de saúde |

**Views for optimized queries:**

- `vw_latest_sensor_reading` - Latest reading
- `vw_daily_sensor_summary` - Daily summary
- `vw_unread_alerts` - Pending alerts
- `vw_pending_recommendations` - Active recommendations

#### 4.4.2 Controlador Principal (Master)

O controlador principal implementa a lógica de negócio:

1. **Loop de leitura** - Aquisição periódica de dados dos sensores
2. **Lógica de decisão** - Ativação de bombas baseada em thresholds
3. **Gestão de eventos** - Geração de alertas e logs
4. **Communication** - Ponte com a interface gráfica via signals/slots

### 4.5 Camada de Application

#### 4.5.1 Interface Gráfica (Qt5)

A interface foi desenvolvida em Qt5 com os seguintes componentes:

| Componente | Class | Function |
|------------|--------|--------|
| Janela Principal | `MainWindow` | Container com tabs |
| Sensors | `SensorsDisplay` | Visualização tempo real |
| Saúde | `HealthDisplay` | Resultados ML |
| Alerts | `AlertsDisplay` | Lista de notificações |
| Analytics | `AnalyticsWindow` | Charts históricos |
| Settings | `SettingsWindow` | Parameters do sistema |

**Características visuais:**

- Temas Light/Dark mode
- Cores consistentes (verde #4CAF50 como cor primária)
- Charts com Qt5Charts
- Responsive design

**Plataformas Qt suportadas:**

| Plataforma | Usage |
|------------|-----|
| `xcb` | Desktop Linux (X11) |
| `linuxfb` | Raspberry Pi com HDMI |
| `offscreen` | Modo headless/testes |
| `vnc` | Acesso remoto |

#### 4.5.2 System de Machine Learning

##### Template

Foi trained um modelo baseado em MobileNetV3-Small para classification de doenças em plantas de tomate:

| Parameter | Value |
|-----------|-------|
| Architecture | MobileNetV3-Small (modificada) |
| Input | 224×224×3 (RGB) |
| Output | 4 classes |
| Tamanho | 5.9 MB (ONNX) |

**Classs de classification:**

| Class | Description |
|--------|-----------|
| Healthy | Healthy plant |
| Bacterial_Spot | Bacterial spot |
| Early_Blight | Early Blight |
| Late_Blight | Late Blight |

##### Treino

| Parameter | Value |
|-----------|-------|
| Dataset | PlantVillage (10.000 imagens) |
| Split | 80% treino, 10% validação, 10% teste |
| Epochs | 20 |
| Learning Rate | 0.001 → 0.0001 (scheduler) |
| Optimizer | Adam |
| Loss | CrossEntropyLoss |
| GPU | NVIDIA RTX 3070 |

**Augmentação de dados:**

- RandomResizedCrop
- RandomHorizontalFlip
- RandomRotation (±15°)
- ColorJitter

##### Resultados

| Metric | Value |
|---------|-------|
| **Accuracy** | 99.39% |
| Precision | 99.41% |
| Recall | 99.39% |
| F1-Score | 99.39% |

##### Inference

A inferência no Raspberry Pi é realizada com ONNX Runtime:

```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // 4 cores
session_options.SetGraphOptimizationLevel(ORT_ENABLE_ALL);

Ort::Session session(env, "leafsense_model.onnx", session_options);
```

**Performance no Raspberry Pi 4:**

| Metric | Value |
|---------|-------|
| Tempo de inferência | ~150 ms |
| Usage de RAM | ~50 MB |
| Usage de CPU | ~80% (1 core) |

### 4.6 Deploymentment

#### 4.6.1 Preparação da Imagem

O processo de deployment envolve:

1. **Compilation do Buildroot** (~1-2 horas)
   ```bash
   cd ~/buildroot/buildroot-2025.08
   make -j$(nproc)
   ```

2. **Cross-compilation do LeafSense**
   ```bash
   cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
   make -j$(nproc)
   ```

3. **Compilation do módulo de kernel**
   ```bash
   cd drivers/kernel_module
   make arm64
   ```

#### 4.6.2 Installation na Raspberry Pi

1. **Flash do cartão SD**
   ```bash
   sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M
   ```

2. **Expansão da partição**
   ```bash
   sudo parted /dev/sdX resizepart 2 100%
   sudo resize2fs /dev/sdX2
   ```

3. **Cópia de ficheiros via SSH**
   ```bash
   scp build-arm/LeafSense root@10.42.0.196:/opt/leafsense/
   scp ml/leafsense_model.onnx root@10.42.0.196:/opt/leafsense/
   scp external/onnxruntime-arm64/lib/*.so* root@10.42.0.196:/usr/lib/
   ```

4. **Configuration de auto-start**
   ```bash
   # /etc/init.d/S98leafsense
   export QT_QPA_PLATFORM=offscreen
   cd /opt/leafsense && ./LeafSense &
   ```

#### 4.6.3 Estrutura de Ficheiros Final

```
Raspberry Pi 4B
├── /opt/leafsense/
│   ├── LeafSense              # Application (380KB)
│   ├── leafsense_model.onnx   # Template ML (5.9MB)
│   ├── leafsense.db           # Database
│   └── schema.sql             # Schema SQL
├── /usr/lib/
│   ├── libonnxruntime.so*     # ONNX Runtime (16MB)
│   └── libQt5Charts.so*       # Qt5Charts (1.9MB)
├── /lib/modules/6.12.41-v8/
│   └── led.ko                 # Module kernel (13KB)
├── /etc/init.d/
│   └── S98leafsense           # Script de arranque
└── /var/log/
    └── leafsense.log          # Logs da aplicação
```

### 4.7 Validation

A validação do sistema foi realizada através de:

| Test | Resultado |
|-------|-----------|
| Boot da Raspberry Pi | ✅ Sucesso |
| Conectividade SSH | ✅ root@10.42.0.196 |
| Carregamento do módulo LED | ✅ `/dev/led0` criado |
| Control de LED | ✅ ON/OFF funcional |
| Initialization da base de dados | ✅ 8 tabelas criadas |
| Carregamento do modelo ONNX | ✅ Template carregado |
| Application LeafSense | ✅ A executar |
| Readings de sensores (mock) | ✅ Data a serem gravados |
| Auto-start no boot | ✅ S98leafsense |

**Metrics de sistema:**

| Metric | Value |
|---------|-------|
| RAM utilizada | ~60 MB / 1.8 GB (3%) |
| Load average | 0.23 |
| Uptime testado | 20+ minutos |

### 4.8 Dificuldades Encontradas e Soluções

| Problema | Causa | Solução |
|----------|-------|---------|
| `ioremap_nocache` não existe | API removida no kernel 5.6+ | Substituir por `ioremap` |
| Qt5Charts não encontrado | Não incluído no Buildroot por defeito | Adicionar `BR2_PACKAGE_QT5CHARTS=y` e recompilar |
| Template ONNX não carrega | Caminho relativo incorreto | Copiar modelo para `/opt/leafsense/` |
| Tables da BD não existem | BD não inicializada | Executar `sqlite3 leafsense.db < schema.sql` |
| Pi não encontrado na rede | DHCP não atribuiu IP | Usar USB-Ethernet e IP fixo |
| Plataforma Qt "eglfs" não disponível | Plugin não compilado | Usar `QT_QPA_PLATFORM=offscreen` |

### 4.9 Trabalho Futuro

As seguintes funcionalidades estão planeadas para versões futuras:

1. **Integration de sensores reais** - Substituir mocks por drivers I2C/1-Wire funcionais
2. **Synchronization NTP** - Corrigir data/hora do sistema
3. **Interface de câmara** - Captura em tempo real para ML
4. **Servidor web** - Acesso remoto via browser
5. **Notifications push** - Alerts via Telegram/Email
6. **Dashboard mobile** - Application Android/iOS

---

## Referências

1. Qt Documentation - https://doc.qt.io/qt-5/
2. ONNX Runtime - https://onnxruntime.ai/
3. Buildroot Manual - https://buildroot.org/downloads/manual/manual.html
4. BCM2711 ARM Peripherals - Raspberry Pi Documentation
5. PlantVillage Dataset - https://plantvillage.psu.edu/
