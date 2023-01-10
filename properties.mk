#
# Copyright (C) 2022 Paranoid Android
#
# SPDX-License-Identifier: Apache-2.0
#

# Audio
PRODUCT_VENDOR_PROPERTIES += \
    ro.audio.monitorRotation=true \
    ro.hardware.audio_amplifier=j606f

PRODUCT_ODM_PROPERTIES += \
    vendor.audio.feature.hdmi_passthrough.enable=false

# Bluetooth
PRODUCT_VENDOR_PROPERTIES += \
    persist.vendor.qcom.bluetooth.soc=cherokee \
    persist.vendor.qcom.bluetooth.enable.splita2dp=true \
    persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac

# Camera
PRODUCT_VENDOR_PROPERTIES += \
    camera.disable_zsl_mode=1

# Crypto
PRODUCT_VENDOR_PROPERTIES += \
    ro.crypto.dm_default_key.options_format.version=2 \
    ro.crypto.volume.filenames_mode=aes-256-cts \
    ro.crypto.volume.metadata.method=dm-default-key \
    ro.crypto.volume.options=::v2 \
    ro.hardware.keystore_desede=true

# SF
PRODUCT_VENDOR_PROPERTIES += \
    debug.sf.enable_hwc_vds=1 \

# MM
PRODUCT_VENDOR_PROPERTIES += \
    media.aac_51_output_enabled=true \
    media.stagefright.enable-aac=true \
    media.stagefright.enable-fma2dp=true \
    media.stagefright.enable-http=true \
    media.stagefright.enable-player=true \
    media.stagefright.enable-qcp=true \
    media.stagefright.enable-scan=true \
    mmp.enable.3g2=true \
    persist.mm.enable.prefetch=true

# GLES
PRODUCT_VENDOR_PROPERTIES += \
    persist.sys.force_sw_gles=1 \
    ro.kernel.qemu.gles

# HWUI
PRODUCT_VENDOR_PROPERTIES += \
    ro.hwui.drop_shadow_cache_size=6 \
    ro.hwui.gradient_cache_size=1 \
    ro.hwui.layer_cache_size=48 \
    ro.hwui.path_cache_size=32 \
    ro.hwui.r_buffer_cache_size=8 \
    ro.hwui.text_large_cache_height=1024 \
    ro.hwui.text_large_cache_width=2048 \
    ro.hwui.text_small_cache_height=1024 \
    ro.hwui.text_small_cache_width=1024 \
    ro.hwui.texture_cache_flushrate=0.4 \
    ro.hwui.texture_cache_size=72

# Time
PRODUCT_VENDOR_PROPERTIES += \
    persist.timed.enable=true

# Sensors
PRODUCT_VENDOR_PROPERTIES += \
    persist.vendor.sensors.debug.ssc_qmi_debug=true \
    persist.vendor.sensors.hal_trigger_ssr=false \
    persist.vendor.sensors.enable.rt_task=false \
    persist.vendor.sensors.support_direct_channel=false \
    persist.vendor.sensors.enable.bypass_worker=true
