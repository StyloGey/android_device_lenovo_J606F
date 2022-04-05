#
# Copyright (C) 2022 Paranoid Android
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/lenovo/J606F

# Properties
$(call inherit-product, $(DEVICE_PATH)/properties.mk)

# Soong
PRODUCT_SOONG_NAMESPACES += \
    $(DEVICE_PATH)

$(call inherit-product-if-exists, vendor/lenovo/J606F-vendor.mk)
