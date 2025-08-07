#include "utility.hpp"
#include <android/log.h>

#define LOG_TAG "xandroid"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void utility_mgr::log(const char* message) {
    LOGD("%s", message);
}
