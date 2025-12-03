# LeafSense - Documentação

## Índice

1. [Visão Geral do Projeto](01-OVERVIEW.md)
2. [Arquitetura do Sistema](02-ARCHITECTURE.md)
3. [Machine Learning](03-MACHINE-LEARNING.md)
4. [Deployment - Raspberry Pi](04-RASPBERRY-PI-DEPLOYMENT.md)
5. [Kernel Module - LED Driver](05-KERNEL-MODULE.md)
6. [Base de Dados](06-DATABASE.md)
7. [Interface Gráfica (Qt5)](07-GUI.md)
8. [Guia de Troubleshooting](08-TROUBLESHOOTING.md)
9. [Changelog e Histórico](09-CHANGELOG.md)
10. [Secção de Implementação (Relatório)](10-IMPLEMENTATION-REPORT.md)

---

## Quick Start

### Compilar para Desktop (Desenvolvimento)
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
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

### Acesso SSH à Raspberry Pi
```bash
ssh root@10.42.0.196
# Password: leafsense
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
