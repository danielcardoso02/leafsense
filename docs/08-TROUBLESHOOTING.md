# LeafSense - Guia de Troubleshooting

## Índice

1. [Problemas de Compilação](#problemas-de-compilação)
2. [Problemas de Cross-Compilation](#problemas-de-cross-compilation)
3. [Problemas de Runtime](#problemas-de-runtime)
4. [Problemas de Rede](#problemas-de-rede)
5. [Problemas de Hardware](#problemas-de-hardware)
6. [Problemas de Base de Dados](#problemas-de-base-de-dados)
7. [Problemas de ML](#problemas-de-ml)

---

## Problemas de Compilação

### Qt5 não encontrado
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**Solução:**
```bash
# Ubuntu/Debian
sudo apt install qt5-default qtcharts5-dev libqt5svg5-dev libqt5sql5-sqlite

# Ou especificar caminho
cmake -DQt5_DIR=/path/to/qt5/lib/cmake/Qt5 ..
```

### OpenCV não encontrado
```
CMake Error: Could not find OpenCV
```

**Solução:**
```bash
# Ubuntu/Debian
sudo apt install libopencv-dev

# Ou compilar do source
git clone https://github.com/opencv/opencv.git
cd opencv && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc) && sudo make install
```

### Erro de compilação C++17
```
error: 'filesystem' is not a namespace-name
```

**Solução:**
```cmake
# CMakeLists.txt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# E adicionar ao link
target_link_libraries(LeafSense PRIVATE stdc++fs)
```

---

## Problemas de Cross-Compilation

### Toolchain não encontrada
```
CMake Error: CMAKE_C_COMPILER not found
```

**Solução:**
```bash
# Verificar se Buildroot foi compilado
ls ~/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-gcc

# Se não existir, compilar Buildroot
cd ~/buildroot/buildroot-2025.08
make raspberrypi4_64_defconfig
make -j$(nproc)
```

### Sysroot inválido
```
cannot find -lQt5Core
```

**Solução:**
```cmake
# Verificar CMAKE_SYSROOT no toolchain file
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")

# Verificar se bibliotecas existem
ls ${CMAKE_SYSROOT}/usr/lib/libQt5*.so
```

### Erro de ONNX Runtime ARM64
```
undefined reference to `OrtGetApiBase'
```

**Solução:**
```bash
# Verificar versão ARM64
file external/onnxruntime-arm64/lib/libonnxruntime.so
# Deve mostrar: ELF 64-bit LSB shared object, ARM aarch64

# Verificar linkagem
grep ONNXRUNTIME CMakeLists.txt
```

### ioremap_nocache não existe
```
error: implicit declaration of function 'ioremap_nocache'
```

**Solução (kernel 5.6+):**
```c
// Antes
gpio_base = ioremap_nocache(GPIO_BASE, GPIO_SIZE);

// Depois
gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
```

---

## Problemas de Runtime

### Biblioteca não encontrada
```
error while loading shared libraries: libQt5Charts.so.5: cannot open
```

**Solução:**
```bash
# Verificar bibliotecas disponíveis
ssh root@10.42.0.196 "ls /usr/lib/libQt5*"

# Copiar biblioteca em falta
scp ~/buildroot/buildroot-2025.08/output/target/usr/lib/libQt5Charts.so.5.15.14 \
    root@10.42.0.196:/usr/lib/

# Criar symlinks
ssh root@10.42.0.196 "cd /usr/lib && \
    ln -sf libQt5Charts.so.5.15.14 libQt5Charts.so.5 && \
    ln -sf libQt5Charts.so.5.15.14 libQt5Charts.so"
```

### Qt platform plugin não encontrado
```
qt.qpa.plugin: Could not find the Qt platform plugin "eglfs"
```

**Solução:**
```bash
# Usar linuxfb (com display)
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0

# Ou offscreen (sem display)
export QT_QPA_PLATFORM=offscreen

# Ver plugins disponíveis
ls /usr/lib/qt/plugins/platforms/
```

### Segmentation fault no startup
```
Segmentation fault
```

**Solução:**
```bash
# Executar com GDB (se disponível)
gdb ./LeafSense
(gdb) run
(gdb) bt

# Verificar dependências
ldd ./LeafSense

# Verificar arquitetura
file ./LeafSense
```

### Permissões insuficientes
```
Permission denied: /dev/led0
```

**Solução:**
```bash
# Executar como root
sudo ./LeafSense

# Ou ajustar permissões
sudo chmod 666 /dev/led0

# Ou criar regra udev
echo 'KERNEL=="led0", MODE="0666"' | sudo tee /etc/udev/rules.d/99-led.rules
```

---

## Problemas de Rede

### Pi não encontrado na rede
```
ssh: connect to host 10.42.0.196 port 22: No route to host
```

**Solução:**
```bash
# Verificar interface de rede
ip link show

# Procurar Pi na rede
arp-scan --localnet

# Verificar cabo USB-Ethernet
dmesg | grep -i eth

# Usar IP estático no Pi
# Adicionar a /etc/network/interfaces:
# auto eth0
# iface eth0 inet static
# address 10.42.0.196
# netmask 255.255.255.0
```

### SSH recusado
```
Connection refused
```

**Solução:**
```bash
# Verificar se dropbear está a correr (no Pi)
ps aux | grep dropbear

# Iniciar dropbear
/etc/init.d/S50dropbear start

# Verificar configuração
cat /etc/dropbear/dropbear.conf
```

### WiFi não conecta
```
wlan0: link is not ready
```

**Solução:**
```bash
# Verificar driver
lsmod | grep brcm

# Configurar wpa_supplicant
cat > /etc/wpa_supplicant.conf << EOF
network={
    ssid="YOUR_SSID"
    psk="YOUR_PASSWORD"
}
EOF

# Conectar
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
udhcpc -i wlan0
```

---

## Problemas de Hardware

### I2C não funciona
```
Error: Could not open file `/dev/i2c-1'
```

**Solução:**
```bash
# Verificar se módulo está carregado
lsmod | grep i2c

# Carregar módulo
modprobe i2c-bcm2835
modprobe i2c-dev

# Verificar device tree
cat /boot/config.txt | grep i2c
# Deve ter: dtparam=i2c_arm=on
```

### 1-Wire não deteta sensores
```
ls: /sys/bus/w1/devices/28-*: No such file or directory
```

**Solução:**
```bash
# Verificar módulo
lsmod | grep w1

# Carregar módulos
modprobe w1-gpio
modprobe w1-therm

# Verificar device tree
cat /boot/config.txt | grep w1
# Deve ter: dtoverlay=w1-gpio,gpiopin=4

# Verificar conexão física
# DS18B20: VCC (3.3V), GND, DATA (GPIO4)
# Resistor 4.7kΩ entre VCC e DATA
```

### LED não acende
```
echo: write error: Invalid argument
```

**Solução:**
```bash
# Verificar se módulo está carregado
lsmod | grep led

# Ver logs do kernel
dmesg | grep led

# Testar GPIO diretamente
echo 20 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio20/direction
echo 1 > /sys/class/gpio/gpio20/value

# Verificar conexão física
# GPIO20 → Resistor 330Ω → LED → GND
```

---

## Problemas de Base de Dados

### Tabelas não existem
```
[DB Exec Error] no such table: sensor_readings
```

**Solução:**
```bash
# Inicializar base de dados
cd /opt/leafsense
rm -f leafsense.db
sqlite3 leafsense.db < schema.sql

# Verificar tabelas
sqlite3 leafsense.db '.tables'
```

### Base de dados bloqueada
```
database is locked
```

**Solução:**
```bash
# Terminar processos que usam a BD
fuser /opt/leafsense/leafsense.db

# Ou usar timeout maior no código
db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=30000");
```

### Base de dados corrompida
```
database disk image is malformed
```

**Solução:**
```bash
# Tentar recuperar
sqlite3 leafsense.db ".dump" > backup.sql
sqlite3 leafsense_new.db < backup.sql
mv leafsense_new.db leafsense.db

# Ou recriar do zero
rm leafsense.db
sqlite3 leafsense.db < schema.sql
```

---

## Problemas de ML

### Modelo não carrega
```
[ML] Warning: Model file not found: ./leafsense_model.onnx
```

**Solução:**
```bash
# Verificar caminho
ls -la /opt/leafsense/leafsense_model.onnx

# Copiar para diretório de trabalho
cp /opt/leafsense/models/leafsense_model.onnx /opt/leafsense/

# Verificar tamanho (deve ser ~6MB)
ls -lh /opt/leafsense/leafsense_model.onnx
```

### ONNX Runtime crash
```
Segmentation fault (in onnxruntime)
```

**Solução:**
```bash
# Verificar versão do ONNX Runtime
strings /usr/lib/libonnxruntime.so | grep "1.16"

# Verificar compatibilidade ARM64
file /usr/lib/libonnxruntime.so
# Deve mostrar: ARM aarch64

# Verificar modelo ONNX
python3 -c "import onnx; m=onnx.load('leafsense_model.onnx'); print(m.opset_import)"
```

### Inferência muito lenta
```
Inference time: > 1000ms
```

**Solução:**
```cpp
// Otimizar session options
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // Usar 4 cores
session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_ENABLE_ALL
);

// Reduzir resolução de input
cv::resize(image, resized, cv::Size(224, 224));

// Usar batch size = 1
std::vector<int64_t> input_shape = {1, 3, 224, 224};
```

---

## Logs Úteis

### Ver logs do sistema
```bash
dmesg                           # Kernel messages
cat /var/log/messages           # System messages
tail -f /var/log/leafsense.log  # App logs
```

### Ver logs com timestamp
```bash
dmesg -T | tail -20
```

### Filtrar logs
```bash
grep -i error /var/log/leafsense.log
grep -i "ML\|model\|onnx" /var/log/leafsense.log
dmesg | grep -i "led\|gpio"
```

---

## Contacto

Para problemas não resolvidos por este guia, verificar:
1. Issues no repositório GitHub
2. Logs detalhados do sistema
3. Documentação oficial do Qt5, ONNX Runtime, Buildroot
