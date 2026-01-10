# LeafSense - Documentation

## Index

1. [Overview do Projeto](01-OVERVIEW.md)
2. [Architecture do System](02-ARCHITECTURE.md)
3. [Machine Learning](03-MACHINE-LEARNING.md)
4. [Buildroot Image Configuration](04-BUILDROOT-IMAGE.md)
5. [Deployment - Raspberry Pi](05-RASPBERRY-PI-DEPLOYMENT.md)
6. [Device Driver (LED Module)](06-DEVICE-DRIVER.md)
7. [Base de Data](07-DATABASE.md)
8. [Interface Gráfica (Qt5)](08-GUI.md)
9. [Guide de Troubleshooting](09-TROUBLESHOOTING.md)
10. [Changelog e History](10-CHANGELOG.md)
11. [Secção de Implementation (Report)](11-IMPLEMENTATION-REPORT.md)
12. [**Testing Guide**](11-TESTING-GUIDE.md)
13. [Kernel Module - Low Level](13-KERNEL-MODULE.md)

---

## Quick Start

### Compilar para Desktop (Development)
```bash
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir build && cd build
cmake ..
make -j$(nproc)
./LeafSense
```

### Compilar para Raspberry Pi (Cross-Compilation)
```bash
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

### Acesso SSH à Raspberry Pi
```bash
ssh root@10.42.0.196
# Password: leafsense
```

### Start Application (on Pi)
```bash
/opt/leafsense/start_leafsense.sh
```

---

## Autores

**Grupo 11 - University of Minho, School of Engineering**

| Nome | Número |
|------|--------|
| Daniel Gonçalo Silva Cardoso | PG53753 |
| Marco Xavier Leite Costa | PG60210 |

**Orientador:** Professor Adriano José Conceição Tavares

**Curso:** Master's in Industrial Electronics and Computers Engineering  
**UC:** Embedded Systems and Computers

---

**Última atualização:** 3 de Dezembro de 2025
