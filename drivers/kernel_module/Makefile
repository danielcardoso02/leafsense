# =============================================================================
# LeafSense LED Kernel Module Makefile
# =============================================================================
#
# Builds the LED control kernel module for Raspberry Pi.
# Cross-compiles using Buildroot toolchain for ARM64.
#
# Usage:
#   make          - Build the kernel module
#   make clean    - Clean build artifacts
#   make modules_install - Install module
#
# =============================================================================

obj-m := led.o
led-objs := ledmodule.o utils.o

# Kernel source directory (Buildroot)
KDIR := /home/daniel/buildroot/buildroot-2025.08/output/build/linux-custom

# Cross-compilation settings
ARCH ?= arm64
CROSS_COMPILE ?= /home/daniel/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-

export ARCH CROSS_COMPILE

KBUILD_TARGETS := clean help modules modules_install

.PHONY: all $(KBUILD_TARGETS)

all: modules

$(KBUILD_TARGETS):
	$(MAKE) -C $(KDIR) M=$(shell pwd) $@
