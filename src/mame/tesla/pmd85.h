// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * includes/pmd85.h
 *
 ****************************************************************************/
#ifndef MAME_TESLA_PMD85_H
#define MAME_TESLA_PMD85_H

#pragma once

#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "machine/ram.h"
#include "emupal.h"


class pmd85_state : public driver_device
{
public:
	pmd85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_user1(*this, "user1")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_pit(*this, "pit")
		, m_uart(*this, "uart")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_ppi3(*this, "ppi3")
		, m_bank(*this, "bank%d", 0U)
		, m_io_dsw0(*this, "DSW0")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "KEY%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void pmd85(machine_config &config, bool with_uart = true);
	void pmd851(machine_config &config);
	void pmd853(machine_config &config);
	void pmd852a(machine_config &config);
	void alfa(machine_config &config);
	void c2717(machine_config &config);
	void mato(machine_config &config);

	void init_mato();
	void init_pmd852a();
	void init_pmd851();
	void init_pmd852();
	void init_pmd853();
	void init_alfa();
	void init_c2717();

	DECLARE_INPUT_CHANGED_MEMBER(pmd85_reset);

private:
	bool m_txd = false;
	bool m_rts = false;
	uint8_t m_rom_module_present = 0;
	uint8_t m_ppi_port_outputs[4][3]{};
	uint8_t m_startup_mem_map = 0;
	uint8_t m_pmd853_memory_mapping = 0;
	bool m_previous_level = false;
	bool m_clk_level = false;
	bool m_clk_level_tape = false;
	uint8_t m_model = 0;
	emu_timer * m_cassette_timer = nullptr;
	void (pmd85_state::*update_memory)();
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t mato_io_r(offs_t offset);
	void mato_io_w(offs_t offset, uint8_t data);

	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cassette_timer_callback);
	uint8_t ppi0_porta_r();
	uint8_t ppi0_portb_r();
	uint8_t ppi0_portc_r();
	void ppi0_porta_w(uint8_t data);
	void ppi0_portb_w(uint8_t data);
	void ppi0_portc_w(uint8_t data);
	uint8_t mato_ppi0_portb_r();
	uint8_t mato_ppi0_portc_r();
	void mato_ppi0_portc_w(uint8_t data);
	uint8_t ppi1_porta_r();
	uint8_t ppi1_portb_r();
	uint8_t ppi1_portc_r();
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);
	uint8_t ppi2_porta_r();
	uint8_t ppi2_portb_r();
	uint8_t ppi2_portc_r();
	void ppi2_porta_w(uint8_t data);
	void ppi2_portb_w(uint8_t data);
	void ppi2_portc_w(uint8_t data);
	uint8_t ppi3_porta_r();
	uint8_t ppi3_portb_r();
	uint8_t ppi3_portc_r();
	void ppi3_porta_w(uint8_t data);
	void ppi3_portb_w(uint8_t data);
	void ppi3_portc_w(uint8_t data);

	void alfa_mem(address_map &map) ATTR_COLD;
	void c2717_mem(address_map &map) ATTR_COLD;
	void mato_io(address_map &map) ATTR_COLD;
	void mato_mem(address_map &map) ATTR_COLD;
	void pmd852a_mem(address_map &map) ATTR_COLD;
	void pmd853_mem(address_map &map) ATTR_COLD;
	void pmd85_io(address_map &map) ATTR_COLD;
	void pmd85_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	void pmd851_update_memory();
	void pmd852a_update_memory();
	void pmd853_update_memory();
	void alfa_update_memory();
	void mato_update_memory();
	void c2717_update_memory();
	void common_driver_init();

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<ram_device> m_ram;
	optional_memory_region m_user1;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	optional_device<i8251_device> m_uart;
	optional_device<i8255_device> m_ppi0;
	optional_device<i8255_device> m_ppi1;
	optional_device<i8255_device> m_ppi2;
	optional_device<i8255_device> m_ppi3;
	optional_memory_bank_array<17> m_bank;
	optional_ioport m_io_dsw0;
	required_device<palette_device> m_palette;
	optional_ioport_array<16> m_io_keyboard;
	output_finder<3> m_leds;
};


#endif // MAME_TESLA_PMD85_H
