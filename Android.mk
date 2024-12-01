#
# Copyright (C) 2022 Paranoid Android
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

ifneq ($(filter J606F,$(TARGET_DEVICE)),)
include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_MODULE := RemovePackages
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := \
    GoogleDialer \
    Messages \
    GmsTelecommOverlay \
    GmsTelephonyOverlay \
    CarrierConfigResCommon_Sys \
    TelecommResCommon_Sys \
    TelephonyResCommon_Sys
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_SRC_FILES := /dev/null
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := fastcv_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := device/qcom/common/vendor/media-legacy/include
include $(BUILD_HEADER_LIBRARY)

endif
