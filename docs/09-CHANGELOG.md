# LeafSense - Changelog e Histórico de Desenvolvimento

## Versão Atual: 1.0.0 (3 de Dezembro de 2025)

---

## [1.0.0] - 2025-12-03

### 🎉 Primeiro Deploy na Raspberry Pi

Esta versão marca o primeiro deployment funcional do LeafSense numa Raspberry Pi 4B.

### Adicionado

#### Infraestrutura
- **Buildroot 2025.08** configurado para Raspberry Pi 4 (64-bit)
- **Toolchain de cross-compilation** (aarch64-linux-gcc 14.3.0)
- Ficheiro `deploy/toolchain-rpi4.cmake` para CMake
- Script `deploy/configure-buildroot.sh` para configuração automática
- Script `deploy/setup-onnxruntime-arm64.sh` para ONNX Runtime

#### Machine Learning
- Modelo treinado com **99.39% accuracy**
- 4 classes: Healthy, Bacterial_Spot, Early_Blight, Late_Blight
- Integração com ONNX Runtime 1.16.3 (ARM64)
- Ficheiro `ml/leafsense_model.onnx` (5.9MB)

#### Kernel Module
- Driver de LED para GPIO (`drivers/kernel_module/led.ko`)
- Device file `/dev/led0` para controlo userspace
- Acesso direto a registos GPIO BCM2711

#### Base de Dados
- Schema SQLite completo (`database/schema.sql`)
- 8 tabelas + 4 views
- Índices para performance

#### Interface Gráfica
- Suporte para Qt5 5.15.14
- Qt5Charts para gráficos
- Temas Light/Dark mode

### Corrigido

- **`ioremap_nocache` → `ioremap`**: Compatibilidade com kernel 6.12+
- **Qt5Charts em falta**: Adicionado ao Buildroot e copiado para Pi
- **Modelo ONNX não encontrado**: Corrigido caminho relativo
- **Tabelas da BD não existem**: Adicionado schema.sql ao deployment

### Configuração na Raspberry Pi

| Item | Valor |
|------|-------|
| IP | 10.42.0.196 |
| SSH User | root |
| SSH Password | leafsense |
| Hostname | leafsense-pi |
| Kernel | 6.12.41-v8 |

### Ficheiros no Pi

```
/opt/leafsense/
├── LeafSense              # Binário (380KB)
├── leafsense_model.onnx   # Modelo ML (5.9MB)
├── leafsense.db           # Base de dados
└── schema.sql             # Schema SQL

/usr/lib/
├── libonnxruntime.so*     # ONNX Runtime (16MB)
└── libQt5Charts.so*       # Qt5Charts (1.9MB)

/lib/modules/6.12.41-v8/
└── led.ko                 # Kernel module (13KB)

/etc/init.d/
└── S98leafsense           # Script de auto-start
```

### Métricas de Performance

| Métrica | Valor |
|---------|-------|
| RAM utilizada | ~60MB |
| Tempo de inferência ML | ~150ms |
| Leituras de sensor | 1/segundo |
| Uptime testado | 20+ minutos |

---

## Roadmap Futuro

### v1.1.0 (Planeado)
- [ ] Integração de sensores reais (DS18B20, pH, EC)
- [ ] Sincronização NTP para data/hora
- [ ] Interface web para acesso remoto

### v1.2.0 (Planeado)
- [ ] Captura de câmara para ML em tempo real
- [ ] Notificações push via Telegram/Email
- [ ] Dashboard mobile

### v2.0.0 (Futuro)
- [ ] Múltiplas plantas/zonas
- [ ] Controlo de iluminação
- [ ] Integração cloud

---

## Histórico de Sessões de Desenvolvimento

### Sessão 1: Preparação (Novembro 2025)
- Configuração inicial do projeto
- Desenvolvimento da GUI Qt5
- Implementação do schema da base de dados

### Sessão 2: Machine Learning (Novembro 2025)
- Treino do modelo ResNet18
- Exportação para ONNX
- Integração com ONNX Runtime
- Testes de accuracy: 99.39%

### Sessão 3: Deployment (3 Dezembro 2025)
- Configuração do Buildroot para RPi4
- Cross-compilation do LeafSense
- Compilação do kernel module
- Flashar SD Card
- Troubleshooting de bibliotecas em falta
- Primeiro boot com sucesso!

---

## Notas de Desenvolvimento

### Lições Aprendidas

1. **Buildroot é poderoso mas complexo** - A curva de aprendizagem é íngreme, mas oferece controlo total sobre o sistema.

2. **Cross-compilation requer cuidado** - Todas as bibliotecas devem ser compiladas para a mesma arquitetura.

3. **ONNX Runtime ARM64** - Usar versões pré-compiladas poupa muito tempo vs compilar do source.

4. **Kernel modules modernos** - APIs mudam frequentemente. `ioremap_nocache` foi removido no kernel 5.6.

5. **Qt platform plugins** - Buildroot não inclui todos os plugins por defeito. `linuxfb` e `offscreen` são suficientes para embebidos.

### Dicas para Futuros Developers

1. Sempre verificar a arquitetura dos binários com `file`
2. Usar `ldd` para verificar dependências
3. Logs são essenciais: `dmesg` e `/var/log/`
4. Testar em modo offscreen antes de ligar display
5. Fazer backup do SD card após configuração funcional

---

## Contribuidores

**Grupo 11**

- **Daniel Gonçalo Silva Cardoso (PG53753)** - Desenvolvimento e Deployment
- **Marco Xavier Leite Costa (PG60210)** - Desenvolvimento e Documentação

**Orientador:** Professor Adriano José Conceição Tavares

## Licença

Projeto académico - Embedded Systems and Computers (Master's in Industrial Electronics and Computers Engineering)
