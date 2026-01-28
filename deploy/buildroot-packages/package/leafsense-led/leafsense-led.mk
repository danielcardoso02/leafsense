################################################################################
#
# leafsense-led
#
################################################################################

LEAFSENSE_LED_VERSION = 1.0
LEAFSENSE_LED_SITE = $(BR2_EXTERNAL_LEAFSENSE_PATH)/package/leafsense-led/src
LEAFSENSE_LED_SITE_METHOD = local
LEAFSENSE_LED_LICENSE = GPL-2.0
LEAFSENSE_LED_LICENSE_FILES = COPYING

# This is a kernel module package
LEAFSENSE_LED_DEPENDENCIES = linux

define LEAFSENSE_LED_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define LEAFSENSE_LED_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) \
		INSTALL_MOD_PATH=$(TARGET_DIR) modules_install
	# Create udev rule for permissions
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_LEAFSENSE_PATH)/package/leafsense-led/99-leddev.rules \
		$(TARGET_DIR)/etc/udev/rules.d/99-leddev.rules
endef

$(eval $(generic-package))
