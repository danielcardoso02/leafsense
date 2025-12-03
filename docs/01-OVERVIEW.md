# LeafSense - Visão Geral do Projeto

## Descrição

O **LeafSense** é um sistema inteligente de monitorização e controlo para cultivo hidropónico, desenvolvido para Raspberry Pi 4B. O sistema combina sensores de ambiente, atuadores automáticos e Machine Learning para deteção de doenças em plantas.

## Objetivos

1. **Monitorização em tempo real** de parâmetros ambientais (temperatura, pH, EC)
2. **Controlo automático** de bombas doseadoras para manutenção de pH e nutrientes
3. **Deteção de doenças** em plantas usando visão computacional e ML
4. **Interface gráfica** para visualização e controlo manual
5. **Logging e analytics** para histórico e análise de tendências

## Hardware Utilizado

### Raspberry Pi 4 Model B
- **RAM:** 2GB
- **CPU:** Cortex-A72 (ARM64/aarch64) @ 1.5GHz
- **Conectividade:** WiFi, Ethernet, USB 3.0
- **GPIO:** 40 pinos para sensores e atuadores

### Sensores
| Sensor | Interface | Função |
|--------|-----------|--------|
| DS18B20 | 1-Wire (GPIO4) | Temperatura da água |
| pH Sensor | I2C | Medição de pH |
| EC Sensor | I2C | Condutividade elétrica |
| Câmara | CSI/USB | Captura de imagens para ML |

### Atuadores
| Atuador | Interface | Função |
|---------|-----------|--------|
| Bomba pH Up | GPIO (Relé) | Aumentar pH |
| Bomba pH Down | GPIO (Relé) | Diminuir pH |
| Bomba Nutrientes | GPIO (Relé) | Adicionar nutrientes |
| LED Indicador | GPIO (Kernel Module) | Estado do sistema |

## Stack Tecnológico

### Software
- **Linguagem:** C++17
- **GUI Framework:** Qt 5.15.14 (Widgets, Charts, Sql)
- **ML Runtime:** ONNX Runtime 1.16.3
- **Visão Computacional:** OpenCV 4.11.0
- **Base de Dados:** SQLite 3.48.0
- **Build System:** CMake 3.22+

### Sistema Operativo
- **Buildroot 2025.08** - Sistema Linux embebido customizado
- **Kernel:** Linux 6.12.41-v8 (64-bit ARM)
- **Init System:** BusyBox init

## Estrutura do Projeto

```
leafsense-project/
├── CMakeLists.txt              # Configuração de build principal
├── database/
│   └── schema.sql              # Schema da base de dados
├── deploy/
│   ├── toolchain-rpi4.cmake    # Toolchain para cross-compilation
│   ├── configure-buildroot.sh  # Script de configuração Buildroot
│   └── setup-onnxruntime-arm64.sh
├── docs/                       # Documentação
├── drivers/
│   ├── kernel_module/          # Módulo de kernel para LED
│   ├── actuators/              # Drivers de atuadores
│   └── sensors/                # Drivers de sensores
├── external/
│   └── onnxruntime-arm64/      # ONNX Runtime pré-compilado
├── include/
│   ├── application/
│   │   ├── gui/                # Headers da interface gráfica
│   │   └── ml/                 # Headers do sistema ML
│   ├── drivers/                # Headers dos drivers
│   └── middleware/             # Headers do middleware
├── ml/
│   ├── leafsense_model.onnx    # Modelo treinado (5.9MB)
│   └── training/               # Scripts de treino Python
├── resources/
│   ├── resources.qrc           # Recursos Qt
│   └── images/                 # Imagens da GUI
└── src/
    ├── main.cpp                # Ponto de entrada
    ├── application/
    │   ├── gui/                # Implementação da GUI
    │   └── ml/                 # Implementação do ML
    ├── drivers/                # Implementação dos drivers
    └── middleware/             # Implementação do middleware
```

## Funcionalidades Implementadas

### ✅ Concluídas
- [x] Interface gráfica Qt5 com dashboard, gráficos e configurações
- [x] Sistema de base de dados SQLite com schema completo
- [x] Integração ONNX Runtime para inferência ML
- [x] Modelo de ML treinado (4 classes, 99.39% accuracy)
- [x] Cross-compilation para ARM64
- [x] Módulo de kernel para controlo de LED
- [x] Sistema de logging e alertas
- [x] Auto-start no boot da Raspberry Pi

### 🔄 Em Progresso
- [ ] Integração de sensores reais (atualmente em modo mock)
- [ ] Calibração de sensores pH e EC
- [ ] Interface de câmara para ML em tempo real

### 📋 Planeado
- [ ] Servidor web para acesso remoto
- [ ] Notificações push
- [ ] Integração com serviços cloud

## Autores

**Grupo 11**

- Daniel Gonçalo Silva Cardoso, PG53753
- Marco Xavier Leite Costa, PG60210

**Orientador:** Professor Adriano José Conceição Tavares

## Licença

Este projeto foi desenvolvido no âmbito da unidade curricular de Embedded Systems and Computers - Master's in Industrial Electronics and Computers Engineering.
