#
# Copyright (C) 2023 Paranoid Android
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := audio_amplifier.j606f

LOCAL_C_INCLUDES += \
    $(call include-path-for, audio-route) \
    $(call include-path-for, audio-utils) \
    external/tinycompress/include \
    vendor/qcom/opensource/audio-hal/primary-hal/hal \
    vendor/qcom/opensource/audio-hal/primary-hal/hal/audio_extn \
    vendor/qcom/opensource/audio-hal/primary-hal/hal/msm8974

LOCAL_HEADER_LIBRARIES += \
    audio_kernel_headers \
    libhardware_headers

LOCAL_SHARED_LIBRARIES += \
    libaudioroute \
    libcutils \
    audio.primary.$(TARGET_BOARD_PLATFORM) \
    liblog \
    libtinyalsa

LOCAL_SRC_FILES := audio_amplifier.c

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

include $(BUILD_SHARED_LIBRARY)
