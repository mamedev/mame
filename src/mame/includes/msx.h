/*****************************************************************************
 *
 * includes/msx.h
 *
 ****************************************************************************/

#ifndef __MSX_H__
#define __MSX_H__

#include "machine/wd17xx.h"
#include "imagedev/flopimg.h"

#define MSX_MAX_CARTS	(2)

#define TC8521_TAG	"rtc"

class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* PSG */
	int m_psg_b;
	int m_opll_active;
	/* mouse */
	UINT16 m_mouse[2];
	int m_mouse_stat[2];
	/* rtc */
	int m_rtc_latch;
	/* disk */
	UINT8 m_dsk_stat;
	/* kanji */
	UINT8 *m_kanji_mem;
	int m_kanji_latch;
	/* memory */
	const msx_slot_layout *layout;
	slot_state *m_cart_state[MSX_MAX_CARTS];
	slot_state *m_state[4];
	const msx_slot *m_slot[4];
	UINT8 *m_ram_pages[4];
	UINT8 *m_empty, m_ram_mapper[4];
	UINT8 m_ramio_set_bits;
	slot_state *m_all_state[4][4][4];
	int m_slot_expanded[4];
	UINT8 m_primary_slot;
	UINT8 m_secondary_slot[4];
	UINT8 m_superloadrunner_bank;
	UINT8 m_korean90in1_bank;
	UINT8 *m_top_page;
	int m_port_c_old;
	int keylatch;
};


/*----------- defined in machine/msx.c -----------*/

extern const i8255_interface msx_ppi8255_interface;
extern const wd17xx_interface msx_wd17xx_interface;
/* start/stop functions */
extern DRIVER_INIT( msx );
extern MACHINE_START( msx );
extern MACHINE_START( msx2 );
extern MACHINE_RESET( msx );
extern MACHINE_RESET( msx2 );
extern INTERRUPT_GEN( msx_interrupt );
extern INTERRUPT_GEN( msx2_interrupt );

DEVICE_IMAGE_LOAD( msx_cart );
DEVICE_IMAGE_UNLOAD( msx_cart );

void msx_vdp_interrupt(running_machine &machine, int i);

/* I/O functions */
READ8_DEVICE_HANDLER( msx_printer_status_r );
WRITE8_DEVICE_HANDLER( msx_printer_strobe_w );
WRITE8_DEVICE_HANDLER( msx_printer_data_w );

WRITE8_HANDLER ( msx_psg_port_a_w );
READ8_HANDLER ( msx_psg_port_a_r );
WRITE8_HANDLER ( msx_psg_port_b_w );
READ8_HANDLER ( msx_psg_port_b_r );
WRITE8_HANDLER ( msx_fmpac_w );
READ8_HANDLER ( msx_rtc_reg_r );
WRITE8_HANDLER ( msx_rtc_reg_w );
WRITE8_HANDLER ( msx_rtc_latch_w );
WRITE8_HANDLER ( msx_90in1_w );

/* new memory emulation */
WRITE8_HANDLER (msx_page0_w);
WRITE8_HANDLER (msx_page0_1_w);
WRITE8_HANDLER (msx_page1_w);
WRITE8_HANDLER (msx_page1_1_w);
WRITE8_HANDLER (msx_page1_2_w);
WRITE8_HANDLER (msx_page2_w);
WRITE8_HANDLER (msx_page2_1_w);
WRITE8_HANDLER (msx_page2_2_w);
WRITE8_HANDLER (msx_page2_3_w);
WRITE8_HANDLER (msx_page3_w);
WRITE8_HANDLER (msx_page3_1_w);
WRITE8_HANDLER (msx_sec_slot_w);
 READ8_HANDLER (msx_sec_slot_r);
WRITE8_HANDLER (msx_ram_mapper_w);
 READ8_HANDLER (msx_ram_mapper_r);
 READ8_HANDLER (msx_kanji_r);
WRITE8_HANDLER (msx_kanji_w);

#endif /* __MSX_H__ */
