# LeafSense - Deployment na Raspberry Pi

## Requisitos

### Hardware
- Raspberry Pi 4 Model B (2GB+ RAM)
- Cartão microSD (16GB+, recomendado 32GB)
- Adaptador USB-Ethernet ou WiFi
- Fonte de alimentação 5V 3A USB-C

### Software (Host de Desenvolvimento)
- Ubuntu 22.04+ ou similar
- Buildroot 2025.08
- CMake 3.22+
- Git

## Passo 1: Configurar Buildroot

### 1.1 Download do Buildroot
```bash
cd ~/buildroot
wget https://buildroot.org/downloads/buildroot-2025.08.tar.xz
tar xf buildroot-2025.08.tar.xz
cd buildroot-2025.08
```

### 1.2 Configuração Base
```bash
make raspberrypi4_64_defconfig
make menuconfig
```

### 1.3 Opções Necessárias

#### Target Options
- Target Architecture: AArch64 (little endian)
- Target Architecture Variant: cortex-A72

#### Toolchain
- C library: glibc
- Enable C++ support: YES

#### System configuration
- Root password: `leafsense`
- System hostname: `leafsense-pi`

#### Target packages → Graphic libraries and applications → Qt5
- [x] qt5base
- [x] gui module
- [x] widgets module
- [x] sqlite2 support
- [x] qt5charts
- [x] qt5svg

#### Target packages → Libraries → Graphics
- [x] opencv4
- [x] opencv4 python support

#### Target packages → Networking applications
- [x] dropbear (SSH)
- [x] dhcp client

#### Target packages → Hardware handling
- [x] i2c-tools
- [x] Enable 1-Wire support

#### Filesystem images
- [x] ext2/3/4 root filesystem
- Size: 512M

### 1.4 Compilar Buildroot
```bash
make -j$(nproc)
# Demora ~1-2 horas na primeira compilação
```

A imagem será gerada em:
```
output/images/sdcard.img
```

## Passo 2: ONNX Runtime ARM64

### 2.1 Download
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir -p external/onnxruntime-arm64
cd external/onnxruntime-arm64

wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-aarch64-1.16.3.tgz
tar xzf onnxruntime-linux-aarch64-1.16.3.tgz
mv onnxruntime-linux-aarch64-1.16.3/* .
rm -rf onnxruntime-linux-aarch64-1.16.3*
```

### 2.2 Estrutura Esperada
```
external/onnxruntime-arm64/
├── include/
│   └── onnxruntime/
│       └── core/
│           └── session/
│               └── onnxruntime_cxx_api.h
└── lib/
    ├── libonnxruntime.so -> libonnxruntime.so.1.16.3
    ├── libonnxruntime.so.1 -> libonnxruntime.so.1.16.3
    └── libonnxruntime.so.1.16.3
```

## Passo 3: Cross-Compilation

### 3.1 Toolchain File
O ficheiro `deploy/toolchain-rpi4.cmake`:
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Buildroot toolchain
set(TOOLCHAIN_PREFIX "/home/daniel/buildroot/buildroot-2025.08/output/host")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-g++")
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")

# ONNX Runtime
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/external/onnxruntime-arm64")
```

### 3.2 Compilar LeafSense
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir -p build-arm && cd build-arm

cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

### 3.3 Verificar Binário
```bash
file LeafSense
# LeafSense: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV)...

ls -lh LeafSense
# -rwxrwxr-x 1 daniel daniel 380K ... LeafSense
```

## Passo 4: Kernel Module

### 4.1 Compilar o Módulo LED
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/drivers/kernel_module

export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
export CROSS_COMPILE=~/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-
export ARCH=arm64

make
```

### 4.2 Verificar Módulo
```bash
file led.ko
# led.ko: ELF 64-bit LSB relocatable, ARM aarch64...

ls -lh led.ko
# -rw-rw-r-- 1 daniel daniel 13K ... led.ko
```

## Passo 5: Flashar Cartão SD

### 5.1 Identificar Dispositivo
```bash
lsblk
# Identificar o cartão SD (ex: /dev/sdb)
```

### 5.2 Flashar Imagem
```bash
sudo dd if=~/buildroot/buildroot-2025.08/output/images/sdcard.img \
       of=/dev/sdX \
       bs=4M \
       status=progress

sync
```

### 5.3 Expandir Partição
```bash
# Após flashar, expandir ROOTFS para usar todo o cartão
sudo parted /dev/sdX
(parted) resizepart 2 100%
(parted) quit

sudo e2fsck -f /dev/sdX2
sudo resize2fs /dev/sdX2
```

## Passo 6: Copiar Ficheiros para o Pi

### 6.1 Acesso SSH
```bash
# Ligar Pi ao computador via USB-Ethernet ou router
ssh root@10.42.0.196
# Password: leafsense
```

### 6.2 Criar Estrutura
```bash
ssh root@10.42.0.196 "mkdir -p /opt/leafsense/{models,data,images}"
```

### 6.3 Copiar Ficheiros
```bash
# Aplicação
scp build-arm/LeafSense root@10.42.0.196:/opt/leafsense/

# Modelo ML
scp ml/leafsense_model.onnx root@10.42.0.196:/opt/leafsense/

# Schema da base de dados
scp database/schema.sql root@10.42.0.196:/opt/leafsense/

# ONNX Runtime
scp external/onnxruntime-arm64/lib/libonnxruntime.so.1.16.3 \
    root@10.42.0.196:/usr/lib/

# Kernel module
scp drivers/kernel_module/led.ko \
    root@10.42.0.196:/lib/modules/$(uname -r)/
```

### 6.4 Configurar no Pi
```bash
ssh root@10.42.0.196

# Criar symlinks ONNX Runtime
cd /usr/lib
ln -sf libonnxruntime.so.1.16.3 libonnxruntime.so.1
ln -sf libonnxruntime.so.1.16.3 libonnxruntime.so

# Inicializar base de dados
cd /opt/leafsense
sqlite3 leafsense.db < schema.sql

# Verificar tabelas
sqlite3 leafsense.db '.tables'
```

## Passo 7: Script de Init

### 7.1 Criar Script
```bash
cat > /etc/init.d/S98leafsense << 'EOF'
#!/bin/sh

case "$1" in
    start)
        echo "Starting LeafSense..."
        # Load LED kernel module
        if [ -f /lib/modules/$(uname -r)/led.ko ]; then
            insmod /lib/modules/$(uname -r)/led.ko 2>/dev/null || true
        fi
        # Set Qt platform
        export QT_QPA_PLATFORM=offscreen
        export LD_LIBRARY_PATH=/usr/lib
        # Start LeafSense
        cd /opt/leafsense
        ./LeafSense > /var/log/leafsense.log 2>&1 &
        echo "LeafSense started with PID $!"
        ;;
    stop)
        echo "Stopping LeafSense..."
        killall LeafSense 2>/dev/null || true
        ;;
    restart)
        $0 stop
        sleep 1
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
exit 0
EOF

chmod +x /etc/init.d/S98leafsense
```

### 7.2 Testar
```bash
/etc/init.d/S98leafsense start
ps aux | grep LeafSense
tail -f /var/log/leafsense.log
```

## Ficheiros no Pi

| Caminho | Descrição |
|---------|-----------|
| `/opt/leafsense/LeafSense` | Binário da aplicação |
| `/opt/leafsense/leafsense_model.onnx` | Modelo ML |
| `/opt/leafsense/leafsense.db` | Base de dados |
| `/usr/lib/libonnxruntime.so*` | ONNX Runtime |
| `/lib/modules/6.12.41-v8/led.ko` | Kernel module |
| `/etc/init.d/S98leafsense` | Script de init |
| `/var/log/leafsense.log` | Log da aplicação |

## Troubleshooting

### Problema: Pi não encontrado na rede
```bash
# Verificar interfaces de rede
ip link show
ip addr show

# Procurar por USB-Ethernet
arp-scan --interface=enx00e04c3601a6 --localnet
```

### Problema: Biblioteca não encontrada
```bash
# Verificar dependências
ldd /opt/leafsense/LeafSense

# Verificar LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib
```

### Problema: Qt platform plugin não encontrado
```bash
# Usar plugin offscreen (sem display)
export QT_QPA_PLATFORM=offscreen

# OU com display HDMI
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
```

### Problema: Modelo ML não carrega
```bash
# Verificar ficheiro
ls -la /opt/leafsense/leafsense_model.onnx

# Copiar para diretório de trabalho
cp /opt/leafsense/models/leafsense_model.onnx /opt/leafsense/
```
