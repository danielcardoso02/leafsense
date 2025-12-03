# LeafSense - Secção de Implementação (Relatório)

**Grupo 11**

**Autores:**
- Daniel Gonçalo Silva Cardoso, PG53753
- Marco Xavier Leite Costa, PG60210

**Especialização:** Embedded Systems and Computers  
**Curso:** Master's in Industrial Electronics and Computers Engineering  
**Orientador:** Professor Adriano José Conceição Tavares  
**Data:** Dezembro 2025

---

## 4. Implementação

### 4.1 Ambiente de Desenvolvimento

O desenvolvimento do projeto LeafSense foi realizado utilizando as seguintes ferramentas e tecnologias:

| Componente | Ferramenta/Versão |
|------------|-------------------|
| Sistema Operativo (Host) | Ubuntu 22.04 LTS |
| IDE | Visual Studio Code |
| Linguagem Principal | C++17 |
| Sistema de Build | CMake 3.22+ |
| Controlo de Versões | Git / GitHub |
| Framework GUI | Qt 5.15.14 |
| ML Framework (Treino) | PyTorch 2.0 |
| ML Runtime (Inferência) | ONNX Runtime 1.16.3 |
| Visão Computacional | OpenCV 4.11.0 |
| Base de Dados | SQLite 3.48.0 |

Para o desenvolvimento embebido, foi utilizado o **Buildroot 2025.08** como sistema de build para criar uma distribuição Linux customizada para a Raspberry Pi 4.

### 4.2 Cross-Compilation para Raspberry Pi

#### 4.2.1 Configuração do Buildroot

O Buildroot foi configurado a partir da configuração base `raspberrypi4_64_defconfig`, com as seguintes customizações:

```bash
# Configuração base
make raspberrypi4_64_defconfig

# Customizações via menuconfig
make menuconfig
```

**Opções habilitadas:**

| Categoria | Pacotes |
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

#### 4.3.1 Módulo de Kernel (LED Driver)

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

#### 4.3.2 Drivers de Sensores

Os drivers de sensores foram implementados seguindo uma interface comum:

```cpp
class SensorInterface {
public:
    virtual double read() = 0;
    virtual bool calibrate() = 0;
    virtual std::string getName() = 0;
};
```

**Sensores implementados:**

| Sensor | Interface | Descrição |
|--------|-----------|-----------|
| DS18B20 | 1-Wire (GPIO4) | Temperatura da solução |
| pH Sensor | I2C | Medição de pH (0-14) |
| EC Sensor | I2C | Condutividade elétrica |

Para testes e desenvolvimento, foi implementado um modo mock que simula leituras realistas dos sensores.

#### 4.3.3 Drivers de Atuadores

Os atuadores (bombas doseadoras) são controlados através de relés conectados a GPIOs:

```cpp
class ActuatorInterface {
public:
    virtual void activate(int duration_ms) = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() = 0;
};
```

**Atuadores implementados:**

| Atuador | GPIO | Função |
|---------|------|--------|
| Bomba pH Up | 12 | Aumentar pH |
| Bomba pH Down | 13 | Diminuir pH |
| Bomba Nutrientes | 16 | Adicionar nutrientes |

### 4.4 Camada de Middleware

#### 4.4.1 Base de Dados (SQLite)

A persistência de dados é gerida através de SQLite, com o seguinte schema:

**Tabelas principais:**

| Tabela | Descrição |
|--------|-----------|
| `sensor_readings` | Leituras de temperatura, pH, EC |
| `logs` | Eventos e ações do sistema |
| `alerts` | Alertas com severidade |
| `ml_predictions` | Resultados de inferência ML |
| `ml_detections` | Deteções de doenças |
| `ml_recommendations` | Recomendações de ação |
| `plant` | Registo de plantas |
| `health_assessments` | Avaliações de saúde |

**Views para queries otimizadas:**

- `vw_latest_sensor_reading` - Última leitura
- `vw_daily_sensor_summary` - Resumo diário
- `vw_unread_alerts` - Alertas pendentes
- `vw_pending_recommendations` - Recomendações ativas

#### 4.4.2 Controlador Principal (Master)

O controlador principal implementa a lógica de negócio:

1. **Loop de leitura** - Aquisição periódica de dados dos sensores
2. **Lógica de decisão** - Ativação de bombas baseada em thresholds
3. **Gestão de eventos** - Geração de alertas e logs
4. **Comunicação** - Ponte com a interface gráfica via signals/slots

### 4.5 Camada de Aplicação

#### 4.5.1 Interface Gráfica (Qt5)

A interface foi desenvolvida em Qt5 com os seguintes componentes:

| Componente | Classe | Função |
|------------|--------|--------|
| Janela Principal | `MainWindow` | Container com tabs |
| Sensores | `SensorsDisplay` | Visualização tempo real |
| Saúde | `HealthDisplay` | Resultados ML |
| Alertas | `AlertsDisplay` | Lista de notificações |
| Analytics | `AnalyticsWindow` | Gráficos históricos |
| Configurações | `SettingsWindow` | Parâmetros do sistema |

**Características visuais:**

- Temas Light/Dark mode
- Cores consistentes (verde #4CAF50 como cor primária)
- Gráficos com Qt5Charts
- Responsive design

**Plataformas Qt suportadas:**

| Plataforma | Uso |
|------------|-----|
| `xcb` | Desktop Linux (X11) |
| `linuxfb` | Raspberry Pi com HDMI |
| `offscreen` | Modo headless/testes |
| `vnc` | Acesso remoto |

#### 4.5.2 Sistema de Machine Learning

##### Modelo

Foi treinado um modelo baseado em ResNet18 para classificação de doenças em plantas de tomate:

| Parâmetro | Valor |
|-----------|-------|
| Arquitetura | ResNet18 (modificada) |
| Input | 224×224×3 (RGB) |
| Output | 4 classes |
| Tamanho | 5.9 MB (ONNX) |

**Classes de classificação:**

| Classe | Descrição |
|--------|-----------|
| Healthy | Planta saudável |
| Bacterial_Spot | Mancha bacteriana |
| Early_Blight | Requeima precoce |
| Late_Blight | Requeima tardia |

##### Treino

| Parâmetro | Valor |
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

| Métrica | Valor |
|---------|-------|
| **Accuracy** | 99.39% |
| Precision | 99.41% |
| Recall | 99.39% |
| F1-Score | 99.39% |

##### Inferência

A inferência no Raspberry Pi é realizada com ONNX Runtime:

```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // 4 cores
session_options.SetGraphOptimizationLevel(ORT_ENABLE_ALL);

Ort::Session session(env, "leafsense_model.onnx", session_options);
```

**Performance no Raspberry Pi 4:**

| Métrica | Valor |
|---------|-------|
| Tempo de inferência | ~150 ms |
| Uso de RAM | ~50 MB |
| Uso de CPU | ~80% (1 core) |

### 4.6 Deployment

#### 4.6.1 Preparação da Imagem

O processo de deployment envolve:

1. **Compilação do Buildroot** (~1-2 horas)
   ```bash
   cd ~/buildroot/buildroot-2025.08
   make -j$(nproc)
   ```

2. **Cross-compilation do LeafSense**
   ```bash
   cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
   make -j$(nproc)
   ```

3. **Compilação do módulo de kernel**
   ```bash
   cd drivers/kernel_module
   make arm64
   ```

#### 4.6.2 Instalação na Raspberry Pi

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

4. **Configuração de auto-start**
   ```bash
   # /etc/init.d/S98leafsense
   export QT_QPA_PLATFORM=offscreen
   cd /opt/leafsense && ./LeafSense &
   ```

#### 4.6.3 Estrutura de Ficheiros Final

```
Raspberry Pi 4B
├── /opt/leafsense/
│   ├── LeafSense              # Aplicação (380KB)
│   ├── leafsense_model.onnx   # Modelo ML (5.9MB)
│   ├── leafsense.db           # Base de dados
│   └── schema.sql             # Schema SQL
├── /usr/lib/
│   ├── libonnxruntime.so*     # ONNX Runtime (16MB)
│   └── libQt5Charts.so*       # Qt5Charts (1.9MB)
├── /lib/modules/6.12.41-v8/
│   └── led.ko                 # Módulo kernel (13KB)
├── /etc/init.d/
│   └── S98leafsense           # Script de arranque
└── /var/log/
    └── leafsense.log          # Logs da aplicação
```

### 4.7 Validação

A validação do sistema foi realizada através de:

| Teste | Resultado |
|-------|-----------|
| Boot da Raspberry Pi | ✅ Sucesso |
| Conectividade SSH | ✅ root@10.42.0.196 |
| Carregamento do módulo LED | ✅ `/dev/led0` criado |
| Controlo de LED | ✅ ON/OFF funcional |
| Inicialização da base de dados | ✅ 8 tabelas criadas |
| Carregamento do modelo ONNX | ✅ Modelo carregado |
| Aplicação LeafSense | ✅ A executar |
| Leituras de sensores (mock) | ✅ Dados a serem gravados |
| Auto-start no boot | ✅ S98leafsense |

**Métricas de sistema:**

| Métrica | Valor |
|---------|-------|
| RAM utilizada | ~60 MB / 1.8 GB (3%) |
| Load average | 0.23 |
| Uptime testado | 20+ minutos |

### 4.8 Dificuldades Encontradas e Soluções

| Problema | Causa | Solução |
|----------|-------|---------|
| `ioremap_nocache` não existe | API removida no kernel 5.6+ | Substituir por `ioremap` |
| Qt5Charts não encontrado | Não incluído no Buildroot por defeito | Adicionar `BR2_PACKAGE_QT5CHARTS=y` e recompilar |
| Modelo ONNX não carrega | Caminho relativo incorreto | Copiar modelo para `/opt/leafsense/` |
| Tabelas da BD não existem | BD não inicializada | Executar `sqlite3 leafsense.db < schema.sql` |
| Pi não encontrado na rede | DHCP não atribuiu IP | Usar USB-Ethernet e IP fixo |
| Plataforma Qt "eglfs" não disponível | Plugin não compilado | Usar `QT_QPA_PLATFORM=offscreen` |

### 4.9 Trabalho Futuro

As seguintes funcionalidades estão planeadas para versões futuras:

1. **Integração de sensores reais** - Substituir mocks por drivers I2C/1-Wire funcionais
2. **Sincronização NTP** - Corrigir data/hora do sistema
3. **Interface de câmara** - Captura em tempo real para ML
4. **Servidor web** - Acesso remoto via browser
5. **Notificações push** - Alertas via Telegram/Email
6. **Dashboard mobile** - Aplicação Android/iOS

---

## Referências

1. Qt Documentation - https://doc.qt.io/qt-5/
2. ONNX Runtime - https://onnxruntime.ai/
3. Buildroot Manual - https://buildroot.org/downloads/manual/manual.html
4. BCM2711 ARM Peripherals - Raspberry Pi Documentation
5. PlantVillage Dataset - https://plantvillage.psu.edu/
