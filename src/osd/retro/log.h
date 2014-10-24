
#ifdef RETRO_AND
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "mame151-retro", __VA_ARGS__))
#undef write_log
#define write_log LOGI
#else
#define  write_log printf
#define LOGI printf
#endif

