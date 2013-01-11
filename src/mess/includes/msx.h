/*****************************************************************************
 *
 * includes/msx.h
 *
 ****************************************************************************/

#ifndef __MSX_H__
#define __MSX_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/rp5c01.h"
#include "machine/wd17xx.h"
#include "machine/ctronics.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "sound/k051649.h"
#include "sound/2413intf.h"
#include "video/v9938.h"
#include "video/tms9928a.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "formats/basicdsk.h"
#include "formats/fmsx_cas.h"
#include "formats/msx_dsk.h"
//#include "osdepend.h"
#include "hashfile.h"
#include "includes/msx_slot.h"

#define MSX_MAX_CARTS   (2)

#define TC8521_TAG  "rtc"

class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_v9938(*this, "v9938"),
	m_cass(*this, CASSETTE_TAG),
	m_ym(*this, "ym2413"),
	m_dac(*this, "dac"),
	m_rtc(*this, TC8521_TAG)
	{ }

	DECLARE_WRITE8_MEMBER(msx_page0_w);
	DECLARE_WRITE8_MEMBER(msx_page0_1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_1_w);
	DECLARE_WRITE8_MEMBER(msx_page2_2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_3_w);
	DECLARE_WRITE8_MEMBER(msx_page3_w);
	DECLARE_WRITE8_MEMBER(msx_page3_1_w);
	DECLARE_WRITE8_MEMBER(msx_sec_slot_w);
	DECLARE_READ8_MEMBER(msx_sec_slot_r);
	DECLARE_WRITE8_MEMBER(msx_ram_mapper_w);
	DECLARE_READ8_MEMBER(msx_ram_mapper_r);
	DECLARE_READ8_MEMBER(msx_kanji_r);
	DECLARE_WRITE8_MEMBER(msx_kanji_w);
	DECLARE_WRITE8_MEMBER(msx_90in1_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_c_w);
	DECLARE_READ8_MEMBER(msx_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(msx_fmpac_w);
	DECLARE_READ8_MEMBER(msx_rtc_reg_r);
	DECLARE_WRITE8_MEMBER(msx_rtc_reg_w);
	DECLARE_WRITE8_MEMBER(msx_rtc_latch_w);
	DECLARE_WRITE_LINE_MEMBER(msx_wd179x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(msx_wd179x_drq_w);


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
	UINT8 m_superloderunner_bank;
	UINT8 m_korean90in1_bank;
	UINT8 *m_top_page;
	int m_port_c_old;
	int keylatch;
	void msx_memory_map_all ();
	void msx_memory_map_page (UINT8 page);
	void msx_ch_reset_core ();
	void msx_memory_reset ();

	optional_device<v9938_device> m_v9938;
	required_device<cassette_image_device> m_cass;
	required_device<ym2413_device> m_ym;
	required_device<dac_device> m_dac;
	optional_device<rp5c01_device> m_rtc;
	DECLARE_READ8_MEMBER(msx_psg_port_a_r);
	DECLARE_READ8_MEMBER(msx_psg_port_b_r);
	DECLARE_WRITE8_MEMBER(msx_psg_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_psg_port_b_w);
	DECLARE_DRIVER_INIT(msx);
	DECLARE_MACHINE_START(msx);
	DECLARE_MACHINE_RESET(msx);
	DECLARE_MACHINE_START(msx2);
	DECLARE_MACHINE_RESET(msx2);
	INTERRUPT_GEN_MEMBER(msx_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(msx2_interrupt);
	DECLARE_WRITE8_MEMBER(msx_ay8910_w);
	DECLARE_WRITE8_MEMBER(msx_printer_strobe_w);
	DECLARE_WRITE8_MEMBER(msx_printer_data_w);
	DECLARE_READ8_MEMBER(msx_printer_status_r);
};


/*----------- defined in machine/msx.c -----------*/

extern const i8255_interface msx_ppi8255_interface;
extern const wd17xx_interface msx_wd17xx_interface;
/* start/stop functions */

DEVICE_IMAGE_LOAD( msx_cart );
DEVICE_IMAGE_UNLOAD( msx_cart );

void msx_vdp_interrupt(device_t *, v99x8_device &device, int i);

/* I/O functions */

#endif /* __MSX_H__ */
