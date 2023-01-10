/*
 * Copyright (C) 2023, Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_amplifier.j606f"
// #define LOG_NDEBUG 0

#include <linux/ioctl.h>
#include <sys/cdefs.h>

#include <audio_hw.h>
#include <errno.h>
#include <hardware/audio_amplifier.h>
#include <hardware/hardware.h>
#include <log/log.h>
#include <malloc.h>
#include <platform.h>
#include <platform_api.h>

#define AW_SHOW_RE_TO_DSP_RE(re) (((re) << 12) / (1000))
#define AW_IOCTL_MAGIC 'a'
#define AW_IOCTL_SET_CALI_RE _IOWR(AW_IOCTL_MAGIC, 6, int32_t)
#define AW_CALI_FILE "/mnt/vendor/persist/factory/audio/aw_cali.bin"
#define AW_CALI_FRAG_SIZE 10
#define AW_DEV_NUM 4

typedef struct aw882xx_dev {
    amplifier_device_t ampdev;
    struct audio_device *adev;
    struct pcm *aw882xx_pcm;
} aw882xx_t;

static aw882xx_t *aw882xx_dev = NULL;

static int write_cali_to_dsp(char *path, int data) {
    int ret = 0;

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        ret = -ENOENT;
        ALOGE("failed to open %s", path);
        goto exit;
    }

    ret = ioctl(fd, AW_IOCTL_SET_CALI_RE, data);
    if (ret < 0) {
        ALOGE("failed to set cali data");
        goto exit;
    }

exit:
    close(fd);
    return ret;
}

static bool is_supported_feedback_on_device(snd_device_t snd_device) {
    switch (snd_device) {
        case SND_DEVICE_OUT_SPEAKER:
        case SND_DEVICE_OUT_SPEAKER_REVERSE:
        case SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES:
        case SND_DEVICE_OUT_SPEAKER_AND_LINE:
        case SND_DEVICE_OUT_VOICE_SPEAKER:
        case SND_DEVICE_OUT_SPEAKER_AND_HDMI:
        case SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET:
        case SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET:
            ALOGV("%d supports feedback", snd_device);
            return true;
        default:
            ALOGV("%d does not support feedback", snd_device);
            return false;
    }
}

static int aw882xx_cali(void *adev) {
    FILE *cali_file;
    char result[AW_DEV_NUM][AW_CALI_FRAG_SIZE];
    int cali_data[AW_DEV_NUM];
    char dev_path[16] = {'\0'};
    int i;
    int ret = 0;

    if (!adev) {
        ALOGE("adev can't be NULL");
        return -EINVAL;
    }
    aw882xx_dev->adev = adev;

    cali_file = fopen(AW_CALI_FILE, "rb");
    if (!cali_file) {
        ret = -ENOENT;
        ALOGE("failed to open %s", AW_CALI_FILE);
        goto exit;
    }

    ret = fread(&result, sizeof(result), 1, cali_file);
    if (ret != 1) {
        ALOGE("failed to read %s", AW_CALI_FILE);
        ret = -EINVAL;
        goto exit;
    }

    for (i = 0; i < AW_DEV_NUM; i++) {
        char cal_str[AW_CALI_FRAG_SIZE + 1] = {'\0'};
        strncpy(cal_str, result[i], AW_CALI_FRAG_SIZE);
        cali_data[i] = atoi(cal_str);

        if (cali_data[i] < 5000 || cali_data[i] > 7500) {
            ALOGE("cali data invalid");
            goto exit;
        }
    }

    for (i = 0; i < AW_DEV_NUM; i++) {
        snprintf(dev_path, sizeof(dev_path), "/dev/aw_dev_%d", i);
        write_cali_to_dsp(dev_path, AW_SHOW_RE_TO_DSP_RE(cali_data[i]));
    }

exit:
    fclose(cali_file);
    return ret;
}

static int aw882xx_start_feedback(struct audio_device *adev, snd_device_t snd_device) {
    struct audio_usecase *usecase;
    int pcm_id;
    int ret;
    struct pcm_config pcm_config_aw882xx = {
            .channels = 4,
            .rate = 48000,
            .period_size = 256,
            .period_count = 4,
            .format = PCM_FORMAT_S16_LE,
            .stop_threshold = INT_MAX,
    };

    ALOGV("starting feedback");

    if (!is_supported_feedback_on_device(snd_device) || aw882xx_dev->aw882xx_pcm) {
        return 0;
    }

    usecase = (struct audio_usecase *)calloc(1, sizeof(struct audio_usecase));
    if (!usecase) {
        ALOGE("Failed to allocate memory");
        return -ENOMEM;
    }

    usecase->id = USECASE_VOICE_CALL;
    usecase->type = PCM_CAPTURE;
    list_init(&usecase->device_list);

    usecase->in_snd_device = SND_DEVICE_IN_CAPTURE_VI_FEEDBACK;
    list_add_tail(&adev->usecase_list, &usecase->list);

    enable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);
    enable_audio_route(adev, usecase);

    pcm_id = platform_get_pcm_device_id(usecase->id, PCM_CAPTURE);
    if (pcm_id < 0) {
        ALOGE("Invalid pcm device");
        return -EACCES;
    }

    aw882xx_dev->aw882xx_pcm = pcm_open(adev->snd_card, pcm_id, PCM_IN, &pcm_config_aw882xx);
    if (aw882xx_dev->aw882xx_pcm && !pcm_is_ready(aw882xx_dev->aw882xx_pcm)) {
        ALOGE("pcm not ready: %s", pcm_get_error(aw882xx_dev->aw882xx_pcm));
        ret = -EIO;
        goto exit;
    }

    ret = pcm_start(aw882xx_dev->aw882xx_pcm);
    if (ret) {
        ALOGE("failed to start pcm: %d", ret);
        goto exit;
    }

exit:
    if (aw882xx_dev->aw882xx_pcm) {
        pcm_close(aw882xx_dev->aw882xx_pcm);
    }
    aw882xx_dev->aw882xx_pcm = NULL;

    disable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);

    list_remove(&usecase->list);
    usecase->out_snd_device = SND_DEVICE_NONE;
    usecase->in_snd_device = SND_DEVICE_IN_CAPTURE_VI_FEEDBACK;
    usecase->id = USECASE_VOICE_CALL;
    usecase->type = PCM_CAPTURE;

    disable_audio_route(adev, usecase);
    free(usecase);

    return ret;
}

static int aw882xx_stop_feedback(struct audio_device *adev, snd_device_t snd_device) {
    struct audio_usecase *usecase;

    ALOGV("stopping feedback");

    if (!is_supported_feedback_on_device(snd_device)) {
        return -ENOTSUP;
    }

    usecase = get_usecase_from_list(adev, USECASE_VOICE_CALL);
    if (aw882xx_dev->aw882xx_pcm) {
        pcm_close(aw882xx_dev->aw882xx_pcm);
    }
    aw882xx_dev->aw882xx_pcm = NULL;

    disable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);
    if (usecase) {
        list_remove(&usecase->list);
        disable_audio_route(adev, usecase);

        free(usecase);
    }

    return 0;
}

static int aw882xx_set_parameters(__unused struct amplifier_device *device,
                                  struct str_parms *parms) {
    int ret;
    int val;
    char value[20];

    ret = str_parms_get_int(parms, "rotation", &val);
    if (ret >= 0) {
        switch (val) {
            case 0:
            case 90:
            case 180:
            case 270:
                sprintf(value, "speaker_spin_%d", val);
                audio_route_apply_and_update_path(aw882xx_dev->adev->audio_route, value);
                break;
            default:
                ALOGE("Unexpected rotation of %d", val);
                return -EINVAL;
        }
    }

    return 0;
}

static int aw882xx_set_feedback(__unused struct amplifier_device *device, void *adev,
                                uint32_t devices, bool enable) {
    int ret = 0;

    if (!adev) {
        ALOGE("adev can't be NULL");
        return -EINVAL;
    }
    aw882xx_dev->adev = adev;

    if (enable) {
        ret = aw882xx_start_feedback(aw882xx_dev->adev, devices);
    } else {
        ret = aw882xx_stop_feedback(aw882xx_dev->adev, devices);
    }

    return ret;
}

static int aw882xx_module_open(__unused const struct hw_module_t *module, const char *id,
                               struct hw_device_t **device) {
    if (strcmp(id, AMPLIFIER_HARDWARE_INTERFACE) != 0) {
        return -EINVAL;
    }

    aw882xx_dev = calloc(1, sizeof(amplifier_device_t));
    if (!aw882xx_dev) {
        return -ENOMEM;
    }

    aw882xx_dev->ampdev.set_parameters = aw882xx_set_parameters;
    aw882xx_dev->ampdev.set_feedback = aw882xx_set_feedback;
    aw882xx_dev->ampdev.calibrate = aw882xx_cali;

    *device = (hw_device_t *)aw882xx_dev;
    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
        .open = aw882xx_module_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
        .common =
                {
                        .tag = HARDWARE_MODULE_TAG,
                        .module_api_version = AMPLIFIER_MODULE_API_VERSION_0_1,
                        .hal_api_version = HARDWARE_HAL_API_VERSION,
                        .id = AMPLIFIER_HARDWARE_MODULE_ID,
                        .name = "J606F Amplifier HAL",
                        .author = "Paranoid Android",
                        .methods = &hal_module_methods,
                },
};
