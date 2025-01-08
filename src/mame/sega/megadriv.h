// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_SEGA_MEGADRIV_H
#define MAME_SEGA_MEGADRIV_H

#pragma once

#include "mdioport.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/ymopn.h"
#include "video/315_5313.h"

#define MD_CPU_REGION_SIZE 0x800000


INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );


class md_core_state : public driver_device
{
protected:
	static inline constexpr XTAL MASTER_CLOCK_NTSC = 53.693175_MHz_XTAL;
	static inline constexpr XTAL MASTER_CLOCK_PAL  = 53.203424_MHz_XTAL;

	md_core_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_scan_timer(*this, "md_scan_timer"),
		m_vdp(*this,"gen_vdp"),
		m_screen(*this,"megadriv"),
		m_io_reset(*this, "RESET")
	{
	}

	virtual void machine_reset() override ATTR_COLD;

	void md_core_ntsc(machine_config &config);
	void md_core_pal(machine_config &config);

	void megadriv_tas_callback(offs_t offset, uint8_t data);

	uint32_t screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_megadriv(int state);

	required_device<m68000_device> m_maincpu;
	required_device<timer_device> m_scan_timer;
	required_device<sega315_5313_device> m_vdp;
	optional_device<screen_device> m_screen;

	optional_ioport m_io_reset;

private:
	IRQ_CALLBACK_MEMBER(genesis_int_callback);

	void vdp_lv6irqline_callback_genesis_68k(int state);
	void vdp_lv4irqline_callback_genesis_68k(int state);

	void megadriv_timers(machine_config &config);
};


class md_base_state : public md_core_state
{
public:
	void init_megadrie();
	void init_megadriv();
	void init_megadrij();

protected:
	struct genesis_z80_vars
	{
		int z80_is_reset = 0;
		int z80_has_bus = 0;
		uint32_t z80_bank_addr = 0;
		std::unique_ptr<uint8_t[]> z80_prgram;
		emu_timer *z80_run_timer = nullptr;
	};

	md_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_core_state(mconfig, type, tag),
		m_z80snd(*this,"genesis_snd_z80"),
		m_ymsnd(*this,"ymsnd"),
		m_megadrive_ram(*this,"megadrive_ram"),
		m_ioports(*this, "ioport%u", 1U)
	{ }

	uint8_t megadriv_68k_YM2612_read(offs_t offset, uint8_t mem_mask = ~0);
	void megadriv_68k_YM2612_write(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

	void megadriv_init_common();

	void megadriv_z80_bank_w(uint16_t data);
	void megadriv_68k_z80_bank_write(uint16_t data);
	void megadriv_z80_z80_bank_w(uint8_t data);
	uint16_t megadriv_68k_read_z80_ram(offs_t offset, uint16_t mem_mask = ~0);
	void megadriv_68k_write_z80_ram(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t megadriv_68k_check_z80_bus(offs_t offset, uint16_t mem_mask = ~0);
	void megadriv_68k_req_z80_bus(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void megadriv_68k_req_z80_reset(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t z80_read_68k_banked_data(offs_t offset);
	void z80_write_68k_banked_data(offs_t offset, uint8_t data);
	void megadriv_z80_vdp_write(offs_t offset, uint8_t data);
	uint8_t megadriv_z80_vdp_read(offs_t offset);
	uint8_t megadriv_z80_unmapped_read();
	TIMER_CALLBACK_MEMBER(megadriv_z80_run_state);

	void vdp_sndirqline_callback_genesis_z80(int state);

	void megadriv_stop_scanline_timer();

	void md_ntsc(machine_config &config);
	void md2_ntsc(machine_config &config);
	void md_pal(machine_config &config);
	void md2_pal(machine_config &config);
	void md_bootleg(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void megadriv_68k_base_map(address_map &map) ATTR_COLD;
	void megadriv_68k_map(address_map &map) ATTR_COLD;
	void megadriv_z80_io_map(address_map &map) ATTR_COLD;
	void megadriv_z80_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_z80snd;
	required_device<ym_generic_device> m_ymsnd;
	optional_shared_ptr<uint16_t> m_megadrive_ram;

	genesis_z80_vars m_genz80;
	int m_version_hi_nibble;

	required_device_array<megadrive_io_port_device, 3> m_ioports;

private:
	uint16_t m68k_version_read();
	uint16_t m68k_ioport_data_read(offs_t offset);
	uint16_t m68k_ioport_ctrl_read(offs_t offset);
	template <unsigned N> uint16_t m68k_ioport_txdata_read();
	template <unsigned N> uint16_t m68k_ioport_rxdata_read();
	template <unsigned N> uint16_t m68k_ioport_s_ctrl_read();

	void m68k_ioport_data_write(offs_t offset, uint16_t data);
	void m68k_ioport_ctrl_write(offs_t offset, uint16_t data);
	template <unsigned N> void m68k_ioport_txdata_write(uint16_t data);
	template <unsigned N> void m68k_ioport_s_ctrl_write(uint16_t data);

	void megadriv_ioports(machine_config &config);
};


class md_ctrl_state : public md_base_state
{
protected:
	md_ctrl_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_base_state(mconfig, type, tag),
		m_io_pad(*this, "PAD%u", 1U)
	{
	}

	void ctrl1_3button(machine_config &config);
	void ctrl2_3button(machine_config &config);
	void ctrl1_6button(machine_config &config);
	void ctrl2_6button(machine_config &config);

	virtual void machine_start() override ATTR_COLD;

private:
	template <unsigned N> uint8_t ioport_in_3button();
	template <unsigned N> uint8_t ioport_in_6button();

	template <unsigned N> void ioport_out_3button(uint8_t data, uint8_t mem_mask);
	template <unsigned N> void ioport_out_6button(uint8_t data, uint8_t mem_mask);

	TIMER_CALLBACK_MEMBER(ioport_timeout);

	optional_ioport_array<2> m_io_pad;

	emu_timer *m_ioport_idle[2];

	uint8_t m_ioport_th[2];
	uint8_t m_ioport_phase[2];
};

#endif // MAME_SEGA_MEGADRIV_H
