/*****************************************************************************
 *
 * includes/apple2gs.h
 *
 * Apple IIgs
 *
 ****************************************************************************/

#ifndef APPLE2GS_H_
#define APPLE2GS_H_

#include "includes/apple2.h"
#include "sound/es5503.h"
#include "machine/nvram.h"

// IIgs clocks as marked on the schematics
#define APPLE2GS_28M  (XTAL_28_63636MHz) // IIGS master clock
#define APPLE2GS_14M  (APPLE2GS_28M/2)
#define APPLE2GS_7M   (APPLE2GS_28M/4)

// screen dimensions
#define BORDER_LEFT	(32)
#define BORDER_RIGHT	(32)
#define BORDER_TOP	(16)	// (plus bottom)


enum apple2gs_clock_mode
{
	CLOCKMODE_IDLE,
	CLOCKMODE_TIME,
	CLOCKMODE_INTERNALREGS,
	CLOCKMODE_BRAM1,
	CLOCKMODE_BRAM2
};


enum adbstate_t
{
	ADBSTATE_IDLE,
	ADBSTATE_INCOMMAND,
	ADBSTATE_INRESPONSE
};

#define IRQ_KBD_SRQ			0x01
#define IRQ_ADB_DATA		0x02
#define IRQ_ADB_MOUSE		0x04
#define IRQ_VGC_SCANLINE	0x08
#define IRQ_VGC_SECOND		0x10
#define IRQ_INTEN_QSECOND	0x20
#define IRQ_INTEN_VBL		0x40
#define IRQ_DOC			    0x80
#define IRQ_SLOT            0x100

void apple2gs_add_irq(running_machine &machine, UINT16 irq_mask);
void apple2gs_remove_irq(running_machine &machine, UINT16 irq_mask);

class apple2gs_state : public apple2_state
{
public:
	apple2gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: apple2_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
        m_es5503(*this, "es5503"),
        m_fdc(*this, "fdc")
        { }

	required_device<cpu_device> m_maincpu;
	required_device<es5503_device> m_es5503;
    required_device<device_t> m_fdc;

	UINT8 *m_slowmem;
	UINT8 m_newvideo;
	UINT16 m_bordercolor;
	UINT8 m_vgcint;
	UINT8 m_langsel;
	UINT8 m_sltromsel;
	UINT8 m_cyareg;
	UINT8 m_inten;
	UINT8 m_intflag;
	UINT8 m_shadow;
	UINT16 m_pending_irqs;
	UINT8 m_mouse_x;
	UINT8 m_mouse_y;
	INT8 m_mouse_dx;
	INT8 m_mouse_dy;
	device_t *m_cur_slot6_image;
	emu_timer *m_scanline_timer;
	emu_timer *m_clock_timer;
	emu_timer *m_qsecond_timer;
	UINT8 m_clock_data;
	UINT8 m_clock_control;
	UINT8 m_clock_read;
	UINT8 m_clock_reg1;
	apple2gs_clock_mode m_clock_mode;
	UINT32 m_clock_curtime;
	seconds_t m_clock_curtime_interval;
	UINT8 m_clock_bram[256];
	adbstate_t m_adb_state;
	UINT8 m_adb_command;
	UINT8 m_adb_mode;
	UINT8 m_adb_kmstatus;
	UINT8 m_adb_latent_result;
	INT32 m_adb_command_length;
	INT32 m_adb_command_pos;
	UINT8 m_adb_command_bytes[8];
	UINT8 m_adb_response_bytes[8];
	UINT8 m_adb_response_length;
	INT32 m_adb_response_pos;
	UINT8 m_adb_memory[0x100];
	int m_adb_address_keyboard;
	int m_adb_address_mouse;
	UINT8 m_sndglu_ctrl;
	int m_sndglu_addr;
	int m_sndglu_dummy_read;
	bitmap_ind16 *m_legacy_gfx;
    bool m_is_rom3;
    UINT8 m_echo_bank;

	DECLARE_DIRECT_UPDATE_MEMBER(apple2gs_opbase);

    READ8_MEMBER( apple2gs_c0xx_r );
    WRITE8_MEMBER( apple2gs_c0xx_w );
    WRITE8_MEMBER( apple2gs_main0400_w );
    WRITE8_MEMBER( apple2gs_aux0400_w );
    WRITE8_MEMBER( apple2gs_main2000_w );
    WRITE8_MEMBER( apple2gs_aux2000_w );
    WRITE8_MEMBER( apple2gs_main4000_w );
    WRITE8_MEMBER( apple2gs_aux4000_w );

    UINT8 adb_read_datareg();
    UINT8 adb_read_kmstatus();

    void apple2gs_refresh_delegates();

    write8_delegate write_delegates_2gs0400[2];
    write8_delegate write_delegates_2gs2000[2];
    write8_delegate write_delegates_2gs4000[2];
	DECLARE_MACHINE_START(apple2gs);
	DECLARE_MACHINE_RESET(apple2gs);
	DECLARE_VIDEO_START(apple2gs);
	DECLARE_PALETTE_INIT(apple2gs);
	DECLARE_MACHINE_START(apple2gsr1);
	DECLARE_MACHINE_START(apple2gscommon);
};


/*----------- defined in machine/apple2gs.c -----------*/





void apple2gs_doc_irq(device_t *device, int state);


/*----------- defined in video/apple2gs.c -----------*/


SCREEN_UPDATE_IND16( apple2gs );


#endif /* APPLE2GS_H_ */
