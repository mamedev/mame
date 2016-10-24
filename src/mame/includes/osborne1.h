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
		m_gfxdecode(*this, "gfxdecode"),
		m_speaker(*this, "speaker"),
		m_pia0(*this, "pia_0"),
		m_pia1(*this, "pia_1"),
		m_acia(*this, "acia"),
		m_fdc(*this, "mb8877"),
		m_ram(*this, RAM_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_floppy0(*this, "mb8877:0"),
		m_floppy1(*this, "mb8877:1"),
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
		m_video_timer(nullptr),
		m_p_chargen(nullptr),
		m_tilemap(nullptr),
		m_acia_rxc_txc_timer(nullptr)
	{ }


	void bank_0xxx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank_1xxx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bank_2xxx_3xxx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank_2xxx_3xxx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t opcode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqack_w(int state);

	uint8_t ieee_pia_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ieee_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ieee_pia_irq_a_func(int state);

	void video_pia_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_pia_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_pia_out_cb2_dummy(int state);
	void video_pia_irq_a_func(int state);

	void serial_acia_irq_func(int state);

	void init_osborne1();
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device>             m_maincpu;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<speaker_sound_device>   m_speaker;
	required_device<pia6821_device>         m_pia0;
	required_device<pia6821_device>         m_pia1;
	required_device<acia6850_device>        m_acia;
	required_device<mb8877_t>               m_fdc;
	required_device<ram_device>             m_ram;
	required_device<ieee488_device>         m_ieee;
	required_device<floppy_connector>       m_floppy0;
	required_device<floppy_connector>       m_floppy1;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void video_callback(void *ptr, int32_t param);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	bool set_rom_mode(uint8_t value);
	bool set_bit_9(uint8_t value);
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
	uint8_t           m_screen_pac;
	uint8_t           m_acia_rxc_txc_div;
	uint8_t           m_acia_rxc_txc_p_low;
	uint8_t           m_acia_rxc_txc_p_high;

	// bank switch control bits
	uint8_t           m_ub4a_q;
	uint8_t           m_ub6a_q;
	uint8_t           m_rom_mode;
	uint8_t           m_bit_9;

	// onboard video state
	uint8_t           m_scroll_x;
	uint8_t           m_scroll_y;
	uint8_t           m_beep_state;
	emu_timer       *m_video_timer;
	bitmap_ind16    m_bitmap;
	uint8_t           *m_p_chargen;
	tilemap_t       *m_tilemap;

	// SCREEN-PAC registers
	uint8_t           m_resolution;
	uint8_t           m_hc_left;

	// serial state
	uint8_t           m_acia_irq_state;
	uint8_t           m_acia_rxc_txc_state;
	emu_timer       *m_acia_rxc_txc_timer;
};

#endif /* OSBORNE1_H_ */
