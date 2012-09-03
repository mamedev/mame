/*****************************************************************************
 *
 * includes/wswan.h
 *
 ****************************************************************************/

#ifndef WSWAN_H_
#define WSWAN_H_

#define WSWAN_TYPE_MONO 0
#define WSWAN_TYPE_COLOR 1

#define WSWAN_X_PIXELS	(28*8)
#define WSWAN_Y_PIXELS	(18*8)

/* Interrupt flags */
#define WSWAN_IFLAG_STX    0x1
#define WSWAN_IFLAG_KEY    0x2
#define WSWAN_IFLAG_RTC    0x4
#define WSWAN_IFLAG_SRX    0x8
#define WSWAN_IFLAG_LCMP   0x10
#define WSWAN_IFLAG_VBLTMR 0x20
#define WSWAN_IFLAG_VBL    0x40
#define WSWAN_IFLAG_HBLTMR 0x80
/* Interrupts */
#define WSWAN_INT_STX    0
#define WSWAN_INT_KEY    1
#define WSWAN_INT_RTC    2
#define WSWAN_INT_SRX    3
#define WSWAN_INT_LCMP   4
#define WSWAN_INT_VBLTMR 5
#define WSWAN_INT_VBL    6
#define WSWAN_INT_HBLTMR 7

#define INTERNAL_EEPROM_SIZE	1024

#include "emu.h"
#include "cpu/v30mz/v30mz.h"
#include "imagedev/cartslot.h"
#include "machine/nvram.h"


typedef struct
{
	UINT8	mode;		/* eeprom mode */
	UINT16	address;	/* Read/write address */
	UINT8	command;	/* Commands: 00, 01, 02, 03, 04, 08, 0C */
	UINT8	start;		/* start bit */
	UINT8	write_enabled;	/* write enabled yes/no */
	int	size;		/* size of eeprom/sram area */
	UINT8	*data;		/* pointer to start of sram/eeprom data */
	UINT8	*page;		/* pointer to current sram/eeprom page */
} EEPROM;

typedef struct
{
	UINT8	present;	/* Is an RTC present */
	UINT8	setting;	/* Timer setting byte */
	UINT8	year;		/* Year */
	UINT8	month;		/* Month */
	UINT8	day;		/* Day */
	UINT8	day_of_week;	/* Day of the week */
	UINT8	hour;		/* Hour, high bit = 0 => AM, high bit = 1 => PM */
	UINT8	minute;		/* Minute */
	UINT8	second;		/* Second */
	UINT8	index;		/* index for reading/writing of current of alarm time */
} RTC;

typedef struct
{
	UINT32	source;		/* Source address */
	UINT16	size;		/* Size */
	UINT8	enable;		/* Enabled */
} SoundDMA;

typedef struct
{
	UINT8 layer_bg_enable;			/* Background layer on/off */
	UINT8 layer_fg_enable;			/* Foreground layer on/off */
	UINT8 sprites_enable;			/* Sprites on/off */
	UINT8 window_sprites_enable;		/* Sprite window on/off */
	UINT8 window_fg_mode;			/* 0:inside/outside, 1:??, 2:inside, 3:outside */
	UINT8 current_line;			/* Current scanline : 0-158 (159?) */
	UINT8 line_compare;			/* Line to trigger line interrupt on */
	UINT32 sprite_table_address;		/* Address of the sprite table */
	UINT8 sprite_table_buffer[512];
	UINT8 sprite_first;			/* First sprite to draw */
	UINT8 sprite_count;			/* Number of sprites to draw */
	UINT16 layer_bg_address;		/* Address of the background screen map */
	UINT16 layer_fg_address;		/* Address of the foreground screen map */
	UINT8 window_fg_left;			/* Left coordinate of foreground window */
	UINT8 window_fg_top;			/* Top coordinate of foreground window */
	UINT8 window_fg_right;			/* Right coordinate of foreground window */
	UINT8 window_fg_bottom;			/* Bottom coordinate of foreground window */
	UINT8 window_sprites_left;		/* Left coordinate of sprites window */
	UINT8 window_sprites_top;		/* Top coordinate of sprites window */
	UINT8 window_sprites_right;		/* Right coordinate of sprites window */
	UINT8 window_sprites_bottom;		/* Bottom coordinate of sprites window */
	UINT8 layer_bg_scroll_x;		/* Background layer X scroll */
	UINT8 layer_bg_scroll_y;		/* Background layer Y scroll */
	UINT8 layer_fg_scroll_x;		/* Foreground layer X scroll */
	UINT8 layer_fg_scroll_y;		/* Foreground layer Y scroll */
	UINT8 lcd_enable;			/* LCD on/off */
	UINT8 icons;				/* FIXME: What do we do with these? Maybe artwork? */
	UINT8 color_mode;			/* monochrome/color mode */
	UINT8 colors_16;			/* 4/16 colors mode */
	UINT8 tile_packed;			/* layered/packed tile mode switch */
	UINT8 timer_hblank_enable;		/* Horizontal blank interrupt on/off */
	UINT8 timer_hblank_mode;		/* Horizontal blank timer mode */
	UINT16 timer_hblank_reload;		/* Horizontal blank timer reload value */
	UINT16 timer_hblank_count;		/* Horizontal blank timer counter value */
	UINT8 timer_vblank_enable;		/* Vertical blank interrupt on/off */
	UINT8 timer_vblank_mode;		/* Vertical blank timer mode */
	UINT16 timer_vblank_reload;		/* Vertical blank timer reload value */
	UINT16 timer_vblank_count;		/* Vertical blank timer counter value */
	UINT8 *vram;				/* pointer to start of ram/vram (set by MACHINE_RESET) */
	UINT8 *palette_vram;			/* pointer to start of palette area in ram/vram (set by MACHINE_RESET), WSC only */
	int main_palette[8];
	emu_timer *timer;
} VDP;

class wswan_state : public driver_device
{
public:
	wswan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(wswan_port_r);
	DECLARE_WRITE8_MEMBER(wswan_port_w);
	DECLARE_READ8_MEMBER(wswan_sram_r);
	DECLARE_WRITE8_MEMBER(wswan_sram_w);
	VDP m_vdp;
	UINT8 m_ws_portram[256];
	UINT8 *m_ROMMap[256];
	UINT32 m_ROMBanks;
	UINT8 m_internal_eeprom[INTERNAL_EEPROM_SIZE];
	UINT8 m_system_type;
	EEPROM m_eeprom;
	RTC m_rtc;
	SoundDMA m_sound_dma;
	UINT8 *m_ws_ram;
	UINT8 *m_ws_bios_bank;
	UINT8 m_bios_disabled;
	int m_pal[16][16];
	bitmap_ind16 m_bitmap;
	UINT8 m_rotate;
	void wswan_clear_irq_line(int irq);
};


/*----------- defined in machine/wswan.c -----------*/

MACHINE_START( wswan );
MACHINE_START( wscolor );
MACHINE_RESET( wswan );
DEVICE_START(wswan_cart);
DEVICE_IMAGE_LOAD(wswan_cart);


/*----------- defined in video/wswan.c -----------*/

void wswan_refresh_scanline( running_machine &machine );


/*----------- defined in audio/wswan.c -----------*/

class wswan_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~wswan_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type WSWAN;


WRITE8_DEVICE_HANDLER( wswan_sound_port_w );

#endif /* WSWAN_H_ */
