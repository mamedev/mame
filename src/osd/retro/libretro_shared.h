#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

#include <libco.h>

#ifndef MAX_BUTTONS
#define MAX_BUTTONS 16
#endif

extern int NEWGAME_FROM_OSD;

extern char g_rom_dir[1024];
extern const char *retro_save_directory;
extern const char *retro_system_directory;
extern const char *retro_content_directory;
extern int retro_pause;

extern bool experimental_cmdline;
extern bool hide_gameinfo;
extern bool mouse_enable;
extern bool cheats_enable;
extern bool alternate_renderer;
extern bool boot_to_osd_enable;
extern bool boot_to_bios_enable;
extern bool softlist_enable;
extern bool softlist_auto;
extern bool write_config_enable;
extern bool read_config_enable;
extern bool hide_nagscreen;
extern bool hide_warnings;
extern bool throttle_enable;
extern bool auto_save_enable;
extern bool game_specific_saves_enable;

extern int mouseLX;
extern int mouseLY;
extern int mouseBUT[4];

extern UINT16 retrokbd_state[RETROK_LAST];

extern char mediaType[10];

extern bool nobuffer_enable;

extern int mame_reset;

extern int fb_width;
extern int fb_height;
extern int fb_pitch;
extern float retro_aspect;
extern float retro_fps;

#if defined(WANT_MAME)
static const char core[] = "mame";
#elif defined(WANT_MESS)
static const char core[] = "mess";
#elif defined(WANT_UME)
static const char core[] = "ume";
#else
/* fallback */
static const char core[] = "mame";
#endif

/* libretro callbacks */
extern retro_log_printf_t log_cb;
extern retro_input_state_t input_state_cb;

void retro_switch_to_main_thread(void);

void *retro_get_fb_ptr(void);

void process_keyboard_state(void);

void process_joypad_state(void);

void process_mouse_state(void);

#endif
