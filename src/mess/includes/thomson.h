/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef _THOMSON_H_
#define _THOMSON_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/mc6846.h"
#include "machine/6850acia.h"
#include "machine/6551acia.h"
#include "sound/dac.h"
#include "audio/mea8000.h"
#include "machine/ctronics.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "machine/mc6843.h"
#include "machine/mc6846.h"
#include "machine/mc6854.h"
#include "formats/thom_cas.h"
#include "formats/thom_dsk.h"
#include "machine/thomflop.h"


/*----------- defined in machine/thomson.c -----------*/

/*************************** common ********************************/

/* 6821 PIAs */
#define THOM_PIA_SYS    "pia_0"  /* system PIA */
#define THOM_PIA_GAME   "pia_1"  /* music & game PIA (joypad + sound) */
#define THOM_PIA_IO     "pia_2"  /* CC 90-232 I/O extension (parallel & RS-232) */
#define THOM_PIA_MODEM  "pia_3"  /* MD 90-120 MODEM extension */

/* sound ports */
#define THOM_SOUND_BUZ    0 /* 1-bit buzzer */
#define THOM_SOUND_GAME   1 /* 6-bit game port DAC */
#define THOM_SOUND_SPEECH 2 /* speech synthesis */

/* bank-switching */
#define THOM_CART_BANK  "bank2" /* cartridge ROM */
#define THOM_RAM_BANK   "bank3" /* data RAM */
#define THOM_FLOP_BANK  "bank4" /* external floppy controller ROM */
#define THOM_BASE_BANK  "bank5" /* system RAM */

extern const pia6821_interface to7_pia6821_sys;
extern const pia6821_interface to7_pia6821_io;
extern const pia6821_interface to7_pia6821_modem;
extern const pia6821_interface to7_pia6821_game;
extern const pia6821_interface to770_pia6821_sys;
extern const pia6821_interface mo5_pia6821_sys;
extern const pia6821_interface to9_pia6821_sys;
extern const pia6821_interface to8_pia6821_sys;
extern const pia6821_interface to9p_pia6821_sys;
extern const pia6821_interface mo6_pia6821_game;
extern const pia6821_interface mo6_pia6821_sys;
extern const pia6821_interface mo5nr_pia6821_sys;
extern const pia6821_interface mo5nr_pia6821_game;

/***************************** TO7 / T9000 *************************/

/* cartridge bank-switching */
extern DEVICE_IMAGE_LOAD( to7_cartridge );
extern WRITE8_HANDLER ( to7_cartridge_w );
extern READ8_HANDLER  ( to7_cartridge_r );

/* dispatch MODEM or speech synthesis extension */
extern READ8_HANDLER ( to7_modem_mea8000_r );
extern WRITE8_HANDLER ( to7_modem_mea8000_w );

/* MIDI extension (actually an 6850 ACIA) */
extern READ8_HANDLER  ( to7_midi_r );
extern WRITE8_HANDLER ( to7_midi_w );

extern MACHINE_START ( to7 );
extern MACHINE_RESET ( to7 );

/* centronics */
extern const centronics_interface to7_centronics_config;
extern const centronics_interface mo6_centronics_config;

/* timer */
extern const mc6846_interface to7_timer;

/* speech synthesis */
extern const mea8000_interface to7_speech;

/* modem */
extern const acia6850_interface to7_modem;


/***************************** TO7/70 ******************************/

/* gate-array */
extern READ8_HANDLER  ( to770_gatearray_r );
extern WRITE8_HANDLER ( to770_gatearray_w );

extern MACHINE_START ( to770 );
extern MACHINE_RESET ( to770 );

extern const mc6846_interface to770_timer;

/***************************** MO5 ******************************/

/* gate-array */
extern READ8_HANDLER  ( mo5_gatearray_r );
extern WRITE8_HANDLER ( mo5_gatearray_w );

/* cartridge / extended RAM bank-switching */
extern DEVICE_IMAGE_LOAD( mo5_cartridge );
extern WRITE8_HANDLER ( mo5_ext_w );
extern WRITE8_HANDLER ( mo5_cartridge_w );
extern READ8_HANDLER  ( mo5_cartridge_r );

extern MACHINE_START ( mo5 );
extern MACHINE_RESET ( mo5 );


/***************************** TO9 ******************************/

/* IEEE extension */
extern WRITE8_HANDLER ( to9_ieee_w );
extern READ8_HANDLER  ( to9_ieee_r );

/* ROM bank-switching */
extern WRITE8_HANDLER ( to9_cartridge_w );
extern READ8_HANDLER  ( to9_cartridge_r );

/* system gate-array */
extern READ8_HANDLER  ( to9_gatearray_r );
extern WRITE8_HANDLER ( to9_gatearray_w );

/* video gate-array */
extern READ8_HANDLER  ( to9_vreg_r );
extern WRITE8_HANDLER ( to9_vreg_w );

/* keyboard */
extern READ8_HANDLER  ( to9_kbd_r );
extern WRITE8_HANDLER ( to9_kbd_w );

extern MACHINE_START ( to9 );
extern MACHINE_RESET ( to9 );

extern const mc6846_interface to9_timer;


/***************************** TO8 ******************************/

/* bank-switching */
#define TO8_SYS_LO      "bank5" /* system RAM low 2 Kb */
#define TO8_SYS_HI      "bank6" /* system RAM hi 2 Kb */
#define TO8_DATA_LO     "bank7" /* data RAM low 2 Kb */
#define TO8_DATA_HI     "bank8" /* data RAM hi 2 Kb */
#define TO8_BIOS_BANK   "bank9" /* BIOS ROM */

extern UINT8 to8_data_vpage;
extern UINT8 to8_cart_vpage;

extern WRITE8_HANDLER ( to8_cartridge_w );
extern READ8_HANDLER  ( to8_cartridge_r );

/* system gate-array */
extern READ8_HANDLER  ( to8_gatearray_r );
extern WRITE8_HANDLER ( to8_gatearray_w );

/* video gate-array */
extern READ8_HANDLER  ( to8_vreg_r );
extern WRITE8_HANDLER ( to8_vreg_w );

/* floppy */
extern READ8_HANDLER  ( to8_floppy_r );
extern WRITE8_HANDLER ( to8_floppy_w );

extern MACHINE_START ( to8 );
extern MACHINE_RESET ( to8 );

extern const mc6846_interface to8_timer;


/***************************** TO9+ ******************************/

extern MACHINE_START ( to9p );
extern MACHINE_RESET ( to9p );

extern const mc6846_interface to9p_timer;


/***************************** MO6 ******************************/

extern READ8_HANDLER  ( mo6_cartridge_r );
extern WRITE8_HANDLER ( mo6_cartridge_w );
extern WRITE8_HANDLER ( mo6_ext_w );

/* system gate-array */
extern READ8_HANDLER  ( mo6_gatearray_r );
extern WRITE8_HANDLER ( mo6_gatearray_w );

/* video gate-array */
extern READ8_HANDLER  ( mo6_vreg_r );
extern WRITE8_HANDLER ( mo6_vreg_w );

extern MACHINE_START ( mo6 );
extern MACHINE_RESET ( mo6 );


/***************************** MO5 NR ******************************/

/* network */
extern READ8_HANDLER  ( mo5nr_net_r );
extern WRITE8_HANDLER ( mo5nr_net_w );

/* printer */
extern READ8_HANDLER  ( mo5nr_prn_r );
extern WRITE8_HANDLER ( mo5nr_prn_w );

extern MACHINE_START ( mo5nr );
extern MACHINE_RESET ( mo5nr );


/*----------- defined in video/thomson.c -----------*/

/*
   TO7 video:
   one line (64 us) =
      56 left border pixels ( 7 us)
   + 320 active pixels (40 us)
   +  56 right border pixels ( 7 us)
   +     horizontal retrace (10 us)

   one image (20 ms) =
      47 top border lines (~3 ms)
   + 200 active lines (12.8 ms)
   +  47 bottom border lines (~3 ms)
   +     vertical retrace (~1 ms)

   TO9 and up introduced a half (160 pixels) and double (640 pixels)
   horizontal mode, but still in 40 us (no change in refresh rate).
*/


/***************************** dimensions **************************/

/* original screen dimension (may be different from emulated screen!) */
#define THOM_ACTIVE_WIDTH  320
#define THOM_BORDER_WIDTH   56
#define THOM_ACTIVE_HEIGHT 200
#define THOM_BORDER_HEIGHT  47
#define THOM_TOTAL_WIDTH   432
#define THOM_TOTAL_HEIGHT  294

/* Emulated screen dimension may be doubled to allow hi-res 640x200 mode.
   Emulated screen can have smaller borders.
 */

/* maximum number of video pages:
   1 for TO7 generation (including MO5)
   4 for TO8 generation (including TO9, MO6)
 */
#define THOM_NB_PAGES 4

/* page 0 is banked */
#define THOM_VRAM_BANK "bank1"

extern UINT8* thom_vram;

/*********************** video signals *****************************/

struct thom_vsignal {
  unsigned count;  /* pixel counter */
  unsigned init;   /* 1 -> active vertical windos, 0 -> border/VBLANK */
  unsigned inil;   /* 1 -> active horizontal window, 0 -> border/HBLANK */
  unsigned lt3;    /* bit 3 of us counter */
  unsigned line;   /* line counter */
};

/* current video position */
extern struct thom_vsignal thom_get_vsignal ( running_machine &machine );


/************************* lightpen ********************************/

/* specific TO7 / T9000 lightpen code (no video gate-array) */
extern unsigned to7_lightpen_gpl ( running_machine &machine, int decx, int decy );

/* video position corresponding to lightpen (with some offset) */
extern struct thom_vsignal thom_get_lightpen_vsignal ( running_machine &machine, int xdec, int ydec,
						       int xdec2 );

/* specify a lightpencall-back function, called nb times per frame */
extern void thom_set_lightpen_callback ( running_machine &machine, int nb, void (*cb) ( running_machine &machine, int step ) );


/***************************** commons *****************************/

extern VIDEO_START  ( thom );
extern SCREEN_UPDATE_IND16 ( thom );
extern PALETTE_INIT ( thom );
extern SCREEN_VBLANK    ( thom );

/* pass video init signal */
extern void thom_set_init_callback ( running_machine &machine, void (*cb) ( running_machine &machine, int init ) );

/* TO7 TO7/70 MO5 video bank switch */
extern void thom_set_mode_point ( running_machine &machine, int point );

/* set the palette index for the border color */
extern void thom_set_border_color ( running_machine &machine, unsigned color );

/* set one of 16 palette indices to one of 4096 colors */
extern void thom_set_palette ( running_machine &machine, unsigned index, UINT16 color );


/* video modes */
#define THOM_VMODE_TO770       0
#define THOM_VMODE_MO5         1
#define THOM_VMODE_BITMAP4     2
#define THOM_VMODE_BITMAP4_ALT 3
#define THOM_VMODE_80          4
#define THOM_VMODE_BITMAP16    5
#define THOM_VMODE_PAGE1       6
#define THOM_VMODE_PAGE2       7
#define THOM_VMODE_OVERLAY     8
#define THOM_VMODE_OVERLAY3    9
#define THOM_VMODE_TO9        10
#define THOM_VMODE_80_TO9     11
#define THOM_VMODE_NB         12

/* change the current video-mode */
extern void thom_set_video_mode ( running_machine &machine, unsigned mode );

/* select which video page shown among the 4 available */
extern void thom_set_video_page ( running_machine &machine, unsigned page );

/* to tell there is some floppy activity, stays up for a few frames */
extern void thom_floppy_active ( running_machine &machine, int write );


/***************************** TO7 / T9000 *************************/

extern WRITE8_HANDLER ( to7_vram_w );


/***************************** TO7/70 ******************************/

extern WRITE8_HANDLER ( to770_vram_w );


/***************************** TO8 ******************************/

/* write to video memory through system space (always page 1) */
WRITE8_HANDLER ( to8_sys_lo_w );
WRITE8_HANDLER ( to8_sys_hi_w );

/* write to video memory through data space */
WRITE8_HANDLER ( to8_data_lo_w );
WRITE8_HANDLER ( to8_data_hi_w );

/* write to video memory page through cartridge addresses space */
WRITE8_HANDLER ( to8_vcart_w );



class to7_io_line_device :  public device_t,
							public device_serial_interface
{
public:
    // construction/destruction
    to7_io_line_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void input_callback(UINT8 state);

	/* read data register */
	DECLARE_READ8_MEMBER(porta_in);

	/* write data register */
	DECLARE_WRITE8_MEMBER(porta_out);
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
};

extern const device_type TO7_IO_LINE;

#define MCFG_TO7_IO_LINE_ADD(_tag)	\
	MCFG_DEVICE_ADD((_tag), TO7_IO_LINE, 0)

#endif /* _THOMSON_H_ */
