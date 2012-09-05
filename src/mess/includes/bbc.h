/*****************************************************************************
 *
 * includes/bbc.h
 *
 * BBC Model B
 *
 * Driver by Gordon Jefferyes <mess_bbc@romvault.com>
 *
 ****************************************************************************/

#ifndef BBC_H_
#define BBC_H_

#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/i8271.h"
#include "machine/wd17xx.h"
#include "machine/upd7002.h"
#include "video/mc6845.h"
#include "sound/sn76496.h"

class bbc_state : public driver_device
{
public:
	bbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_sn(*this, "sn76489"),
		  m_ACCCON_IRR(CLEAR_LINE),
		  m_via_system_irq(CLEAR_LINE),
		  m_via_user_irq(CLEAR_LINE),
		  m_acia_irq(CLEAR_LINE)
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<sn76489_new_device> m_sn;

	void check_interrupts();

	int m_RAMSize;			// BBC Memory Size
	int m_DFSType;			// this stores the DIP switch setting for the DFS type being used
	int m_SWRAMtype;		// this stores the DIP switch setting for the SWRAM type being used
	int m_Master;			// if 0 then we are emulating a BBC B style machine
							// if 1 then we are emulating a BBC Master style machine

	int m_ACCCON_IRR;		//  IRQ inputs

	int m_rombank;			// This is the latch that holds the sideways ROM bank to read

	int m_userport;			// This stores the sideways RAM latch type.
							// Acorn and others use the bbc_rombank latch to select the write bank to be used.(type 0)
							// Solidisc use the BBC's userport to select the write bank to be used (type 1)

	int m_pagedRAM;			// BBC B+ memory handling
	int m_vdusel;			// BBC B+ memory handling

							/*
                            ACCCON

                            b7 IRR  1=Causes an IRQ to the processor
                            b6 TST  1=Selects &FC00-&FEFF read from OS-ROM
                            b5 IFJ  1=Internal 1 MHz bus
                                    0=External 1MHz bus
                            b4 ITU  1=Internal Tube
                                    0=External Tube
                            b3 Y    1=Read/Write HAZEL &C000-&DFFF RAM
                                    0=Read/Write ROM &C000-&DFFF OS-ROM
                            b2 X    1=Read/Write LYNNE
                                    0=Read/WRITE main memory &3000-&8000
                            b1 E    1=Causes shadow if VDU code
                                    0=Main all the time
                            b0 D    1=Display LYNNE as screen
                                    0=Display main RAM screen

                            ACCCON is a read/write register
                            */

	int m_ACCCON;
	int m_ACCCON_TST;
	int m_ACCCON_IFJ;
	int m_ACCCON_ITU;
	int m_ACCCON_Y;
	int m_ACCCON_X;
	int m_ACCCON_E;
	int m_ACCCON_D;


							/*
                            The addressable latch
                            This 8 bit addressable latch is operated from port B lines 0-3.
                            PB0-PB2 are set to the required address of the output bit to be set.
                            PB3 is set to the value which should be programmed at that bit.
                            The function of the 8 output bits from this latch are:-

                            B0 - Write Enable to the sound generator IC
                            B1 - READ select on the speech processor
                            B2 - WRITE select on the speech processor
                            B3 - Keyboard write enable
                            B4,B5 - these two outputs define the number to be added to the
                            start of screen address in hardware to control hardware scrolling:-
                            Mode    Size    Start of screen  Number to add  B5      B4
                            0,1,2   20K &3000        12K        1       1
                            3       16K &4000        16K        0   0
                            4,5     10K &5800 (or &1800) 22K        1   0
                            6       8K  &6000 (or &2000) 24K        0   1
                            B6 - Operates the CAPS lock LED  (Pin 17 keyboard connector)
                            B7 - Operates the SHIFT lock LED (Pin 16 keyboard connector)
                            */

	int m_b0_sound;
	int m_b1_speech_read;
	int m_b2_speech_write;
	int m_b3_keyboard;
	int m_b4_video0;
	int m_b5_video1;
	int m_b6_caps_lock_led;
	int m_b7_shift_lock_led;

	int m_MC146818_WR;		// FE30 bit 1 replaces  b1_speech_read
	int m_MC146818_DS;		// FE30 bit 2 replaces  b2_speech_write
	int m_MC146818_AS;		// 6522 port b bit 7
	int m_MC146818_CE;		// 6522 port b bit 6

	int m_via_system_porta;
	int m_via_system_irq;
	int m_via_user_irq;
	int m_acia_irq;

	int m_column;			// this is a counter in the keyboard circuit


							/***************************************
                              BBC 2C199 Serial Interface Cassette
                            ****************************************/

	double m_last_dev_val;
	int m_wav_len;
	int m_len0;
	int m_len1;
	int m_len2;
	int m_len3;
	int m_mc6850_clock;
	emu_timer *m_tape_timer;


							/**************************************
                               i8271 disc control
                            ***************************************/

	int m_previous_i8271_int_state;	// 8271 interupt status



							/**************************************
                               WD1770 disc control
                            ***************************************/
	int m_drive_control;
	int m_wd177x_irq_state;
	int m_wd177x_drq_state;
	int m_previous_wd177x_int_state;
	int m_1770_IntEnabled;

							/**************************************
                               Opus Challenger Disc control
                            ***************************************/
	int m_opusbank;



							/**************************************
                               Video Code
                            ***************************************/

	int m_memorySize;


// this is the real location of the start of the BBC's ram in the emulation
// it can be changed if shadow ram is being used to point at the upper 32K of RAM

// this is the screen memory location of the next pixels to be drawn

// this is a more global variable to store the bitmap variable passed in in the bbc_vh_screenrefresh function

// this is the X and Y screen location in emulation pixels of the next pixels to be drawn


	unsigned char *m_BBC_Video_RAM;
	UINT16 *m_BBC_display;
	UINT16 *m_BBC_display_left;
	UINT16 *m_BBC_display_right;
	bitmap_ind16 *m_BBC_bitmap;
	int m_y_screen_pos;
	unsigned char m_pixel_bits[256];
	int m_BBC_HSync;
	int m_BBC_VSync;
	device_t *m_saa505x;



	int m_Teletext_Latch;
	int m_VideoULA_CR;
	int m_VideoULA_CR_counter;
	int m_videoULA_Reg;
	int m_videoULA_master_cursor_size;
	int m_videoULA_width_of_cursor;
	int m_videoULA_6845_clock_rate;
	int m_videoULA_characters_per_line;
	int m_videoULA_teletext_normal_select;
	int m_videoULA_flash_colour_select;



	int m_pixels_per_byte;
	int m_emulation_pixels_per_real_pixel;
	int m_emulation_pixels_per_byte;

	int m_emulation_cursor_size;
	int m_cursor_state;

	int m_videoULA_pallet0[16];
	int m_videoULA_pallet1[16];
	int *m_videoULA_pallet_lookup;

	void (*m_draw_function)(running_machine &machine);
	DECLARE_WRITE8_MEMBER(bbc_page_selecta_w);
	DECLARE_WRITE8_MEMBER(bbc_memorya1_w);
	DECLARE_WRITE8_MEMBER(bbc_page_selectb_w);
	DECLARE_WRITE8_MEMBER(bbc_memoryb3_w);
	DECLARE_WRITE8_MEMBER(bbc_memoryb4_w);
	DECLARE_WRITE8_MEMBER(bbc_page_selectbp_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp1_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp2_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp4_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp4_128_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp6_128_w);
	DECLARE_READ8_MEMBER(bbcm_ACCCON_read);
	DECLARE_WRITE8_MEMBER(bbcm_ACCCON_write);
	DECLARE_WRITE8_MEMBER(page_selectbm_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm1_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm2_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm4_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm5_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm7_w);
	DECLARE_READ8_MEMBER(bbcm_r);
	DECLARE_WRITE8_MEMBER(bbcm_w);
	DECLARE_WRITE8_MEMBER(bbc_SerialULA_w);
	DECLARE_READ8_MEMBER(bbc_i8271_read);
	DECLARE_WRITE8_MEMBER(bbc_i8271_write);
	DECLARE_WRITE8_MEMBER(bbc_wd177x_status_w);
	DECLARE_READ8_MEMBER(bbc_wd1770_read);
	DECLARE_WRITE8_MEMBER(bbc_wd1770_write);
	DECLARE_WRITE8_MEMBER(bbc_opus_status_w);
	DECLARE_READ8_MEMBER(bbc_opus_read);
	DECLARE_WRITE8_MEMBER(bbc_opus_write);
	DECLARE_READ8_MEMBER(bbcm_wd1770_read);
	DECLARE_WRITE8_MEMBER(bbcm_wd1770_write);
	DECLARE_READ8_MEMBER(bbcm_wd1770l_read);
	DECLARE_WRITE8_MEMBER(bbcm_wd1770l_write);
	DECLARE_READ8_MEMBER(bbc_disc_r);
	DECLARE_WRITE8_MEMBER(bbc_disc_w);
	DECLARE_WRITE8_MEMBER(bbc_videoULA_w);
	DECLARE_WRITE8_MEMBER(bbc_6845_w);
	DECLARE_READ8_MEMBER(bbc_6845_r);
	DECLARE_READ8_MEMBER(bbc_fe_r);
	DECLARE_DIRECT_UPDATE_MEMBER(bbcbp_direct_handler);
	DECLARE_DIRECT_UPDATE_MEMBER(bbcm_direct_handler);
	DECLARE_DRIVER_INIT(bbc);
	DECLARE_DRIVER_INIT(bbcm);
};


/*----------- defined in machine/bbc.c -----------*/


extern const mc6845_interface bbc_mc6845_intf;


extern const via6522_interface bbcb_system_via;
extern const via6522_interface bbcb_user_via;
extern const wd17xx_interface bbc_wd17xx_interface;

MACHINE_START( bbca );
MACHINE_START( bbcb );
MACHINE_START( bbcbp );
MACHINE_START( bbcm );

MACHINE_RESET( bbca );
MACHINE_RESET( bbcb );
MACHINE_RESET( bbcbp );
MACHINE_RESET( bbcm );

INTERRUPT_GEN( bbcb_keyscan );
INTERRUPT_GEN( bbcm_keyscan );








/* disc support */

DEVICE_IMAGE_LOAD ( bbcb_cart );







/* tape support */


extern const i8271_interface bbc_i8271_interface;
extern const uPD7002_interface bbc_uPD7002;

/*----------- defined in video/bbc.c -----------*/

VIDEO_START( bbca );
VIDEO_START( bbcb );
VIDEO_START( bbcbp );
VIDEO_START( bbcm );
SCREEN_UPDATE_IND16( bbc );

void bbc_draw_RGB_in(device_t *device, int offset, int data);
void bbc_set_video_memory_lookups(running_machine &machine, int ramsize);
void bbc_setscreenstart(running_machine &machine, int b4, int b5);
void bbcbp_setvideoshadow(running_machine &machine, int vdusel);





#endif /* BBC_H_ */
