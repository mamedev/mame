// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/*****************************************************************************
 *
 * includes/osborne1.h
 *
 ****************************************************************************/

#ifndef OSBORNE1_H_
#define OSBORNE1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "bus/ieee488/ieee488.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

class osborne1_state : public driver_device
{
public:
	enum
	{
		TIMER_VIDEO,
		TIMER_ACIA_RXC_TXC
	};

	osborne1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_pia0(*this, "pia_0"),
		m_pia1(*this, "pia_1"),
		m_acia(*this, "acia"),
		m_fdc(*this, "mb8877"),
		m_ram(*this, RAM_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_floppy0(*this, "mb8877:0:525ssdd"),
		m_floppy1(*this, "mb8877:1:525ssdd"),
		m_keyb_row0(*this, "ROW0"),
		m_keyb_row1(*this, "ROW1"),
		m_keyb_row2(*this, "ROW2"),
		m_keyb_row3(*this, "ROW3"),
		m_keyb_row4(*this, "ROW4"),
		m_keyb_row5(*this, "ROW5"),
		m_keyb_row6(*this, "ROW6"),
		m_keyb_row7(*this, "ROW7"),
		m_btn_reset(*this, "RESET"),
		m_cnf(*this, "CNF"),
		m_region_maincpu(*this, "maincpu"),
		m_bank_0xxx(*this, "bank_0xxx"),
		m_bank_1xxx(*this, "bank_1xxx"),
		m_bank_fxxx(*this, "bank_fxxx"),
		m_video_timer(NULL),
		m_acia_rxc_txc_timer(NULL)
	{ }


	DECLARE_WRITE8_MEMBER(bank_0xxx_w);
	DECLARE_WRITE8_MEMBER(bank_1xxx_w);
	DECLARE_READ8_MEMBER(bank_2xxx_3xxx_r);
	DECLARE_WRITE8_MEMBER(bank_2xxx_3xxx_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_READ8_MEMBER(opcode_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(irqack_w);

	DECLARE_READ8_MEMBER(ieee_pia_pb_r);
	DECLARE_WRITE8_MEMBER(ieee_pia_pb_w);
	DECLARE_WRITE_LINE_MEMBER(ieee_pia_irq_a_func);

	DECLARE_WRITE8_MEMBER(video_pia_port_a_w);
	DECLARE_WRITE8_MEMBER(video_pia_port_b_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_out_cb2_dummy);
	DECLARE_WRITE_LINE_MEMBER(video_pia_irq_a_func);

	DECLARE_WRITE_LINE_MEMBER(serial_acia_irq_func);

	DECLARE_DRIVER_INIT(osborne1);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<acia6850_device> m_acia;
	required_device<mb8877_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<ieee488_device> m_ieee;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	TIMER_CALLBACK_MEMBER(video_callback);

	bool set_rom_mode(UINT8 value);
	bool set_bit_9(UINT8 value);
	void update_irq();
	void update_acia_rxc_txc();

	// user inputs
	required_ioport         m_keyb_row0;
	required_ioport         m_keyb_row1;
	required_ioport         m_keyb_row2;
	required_ioport         m_keyb_row3;
	required_ioport         m_keyb_row4;
	required_ioport         m_keyb_row5;
	required_ioport         m_keyb_row6;
	required_ioport         m_keyb_row7;
	required_ioport         m_btn_reset;

	// fake inputs for hardware configuration and things that need rewiring
	required_ioport         m_cnf;

	// pieces of memory
	required_memory_region  m_region_maincpu;
	required_memory_bank    m_bank_0xxx;
	required_memory_bank    m_bank_1xxx;
	required_memory_bank    m_bank_fxxx;

	// configuration (reloaded on reset)
	UINT8           m_screen_pac;
	UINT8           m_acia_rxc_txc_div;
	UINT8	        m_acia_rxc_txc_p_low;
	UINT8	        m_acia_rxc_txc_p_high;

	// bank switch control bits
	UINT8           m_ub4a_q;
	UINT8           m_ub6a_q;
	UINT8           m_rom_mode;
	UINT8           m_bit_9;

	// onboard video state
	UINT8           m_scroll_x;
	UINT8           m_scroll_y;
	UINT8           m_beep_state;
	emu_timer       *m_video_timer;
	bitmap_ind16    m_bitmap;
	UINT8           *m_p_chargen;

	// SCREEN-PAC registers
	UINT8           m_resolution;
	UINT8           m_hc_left;

	// serial state
	int             m_acia_irq_state;
	int             m_acia_rxc_txc_state;
	emu_timer       *m_acia_rxc_txc_timer;
};

#endif /* OSBORNE1_H_ */
