// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/
#ifndef MAME_MIDWAY_MIDVUNIT_H
#define MAME_MIDWAY_MIDVUNIT_H

#pragma once

#include "midwayic.h"

#include "dcs.h"

#include "bus/ata/ataintf.h"
#include "machine/adc0844.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"


struct midvunit_object_data
{
	uint16_t *    destbase = nullptr;
	uint8_t *     texbase = 0;
	uint16_t      pixdata = 0;
	uint8_t       dither = 0;
};

class midvunit_base_state;

class midvunit_renderer : public poly_manager<float, midvunit_object_data, 2>
{
public:
	midvunit_renderer(midvunit_base_state &state);
	void process_dma_queue();
	void make_vertices_inclusive(vertex_t *vert);

private:
	void render_flat(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_tex(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textrans(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textransmask(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);

	midvunit_base_state &m_state;
};

class midvunit_base_state : public driver_device
{
public:
	uint16_t m_page_control = 0;
	uint16_t m_dma_data[16]{};
	uint8_t m_video_changed = 0;

	memory_share_creator<uint16_t> m_videoram;
	required_shared_ptr<uint32_t> m_textureram;
	required_device<screen_device> m_screen;

protected:
	static inline constexpr XTAL MIDVUNIT_VIDEO_CLOCK = 33.333333_MHz_XTAL;

	midvunit_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram", 0x200000, ENDIANNESS_LITTLE)
		, m_textureram(*this, "textureram")
		, m_screen(*this, "screen")
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_palette(*this, "palette")
		, m_timer(*this, "timer%u", 0U)
		, m_dcs(*this, "dcs")
		, m_paletteram(*this, "paletteram")
		, m_ram_base(*this, "ram_base")
		, m_tms32031_control(*this, "32031_control")
	{ }

	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void cmos_protect_w(uint32_t data);
	void dma_queue_w(uint32_t data);
	uint32_t dma_queue_entries_r();
	uint32_t dma_trigger_r(offs_t offset);
	void page_control_w(uint32_t data);
	uint32_t page_control_r();
	void video_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t scanline_r();
	void videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t videoram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void textureram_w(offs_t offset, uint32_t data);
	uint32_t textureram_r(offs_t offset);
	void control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void sound_w(uint32_t data);
	uint32_t tms32031_control_r(offs_t offset);
	void tms32031_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t generic_speedup_r(offs_t offset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(scanline_timer_cb);
	TIMER_CALLBACK_MEMBER(eoi_timer_cb);

	void midvcommon(machine_config &config);

	required_device<tms32031_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<palette_device> m_palette;
	required_device_array<timer_device, 2> m_timer;
	required_device<dcs_audio_device> m_dcs;
	required_shared_ptr<uint32_t> m_paletteram;
	required_shared_ptr<uint32_t> m_ram_base;
	required_shared_ptr<uint32_t> m_tms32031_control;

	uint8_t m_cmos_protected = 0;
	uint16_t m_control_data = 0;
	double m_timer_rate = 0;
	uint32_t *m_generic_speedup = nullptr;
	uint16_t m_video_regs[16]{};
	uint8_t m_dma_data_index = 0;
	emu_timer *m_scanline_timer = nullptr;
	emu_timer *m_eoi_timer = nullptr;
	std::unique_ptr<midvunit_renderer> m_poly;
};

class midvunit_state : public midvunit_base_state
{
public:
	void midvunit(machine_config &config);

protected:
	midvunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midvunit_base_state(mconfig, type, tag)
		, m_adc(*this, "adc")
		, m_nvram(*this, "nvram")
		, m_optional_drivers(*this, "lamp%u", 0U)
		, m_wheel_motor(*this, "wheel")
		, m_in0(*this, "IN0")
		, m_in1(*this, "IN1")
		, m_dsw(*this, "DSW")
	{ }

	virtual void machine_start() override ATTR_COLD;

	uint32_t port0_r();
	uint32_t adc_r();
	void adc_w(uint32_t data);
	void cmos_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cmos_r(offs_t offset);
	uint32_t wheel_board_r();
	void wheel_board_w(uint32_t data);
	uint32_t intcs_r();
	uint32_t comcs_r(offs_t offset);
	void comcs_w(offs_t offset, uint32_t data);
	void set_input(const char *s);

	uint16_t comm_bus_out();
	uint16_t comm_bus_in();

	void midvunit_map(address_map &map) ATTR_COLD;

	required_device<adc0844_device> m_adc;

	required_shared_ptr<uint32_t> m_nvram;

	output_finder<8> m_optional_drivers;
	output_finder<> m_wheel_motor;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_dsw;

	uint8_t m_adc_shift = 0;
	uint16_t m_last_port0 = 0;
	uint8_t m_shifter_state = 0;
	uint8_t m_galil_input_index = 0;
	uint8_t m_galil_input_length = 0;
	const char *m_galil_input = nullptr;
	uint8_t m_galil_output_index = 0;
	char m_galil_output[450]{};
	uint8_t m_wheel_board_output = 0;
	uint32_t m_wheel_board_last = 0;
	uint32_t m_wheel_board_u8_latch = 0;
	uint8_t m_comm_flags = 0;
	uint16_t m_comm_data = 0;
};

class crusnusa_state : public midvunit_state
{
public:
	crusnusa_state(const machine_config &mconfig, device_type type, const char *tag)
		: midvunit_state(mconfig, type, tag)
		, m_motion(*this, "MOTION")
	{ }

	void init_crusnu40();
	void init_crusnu21();
	void init_crusnusa();

	ioport_value motion_r();

protected:
	void init_crusnusa_common(offs_t speedup);

	required_ioport m_motion;
};

class crusnwld_state : public midvunit_state
{
public:
	crusnwld_state(const machine_config &mconfig, device_type type, const char *tag)
		: midvunit_state(mconfig, type, tag)
		, m_midway_serial_pic2(*this, "serial_pic2")
	{ }

	void crusnwld(machine_config &config);
	void offroadc(machine_config &config);

	void init_crusnwld();
	void init_offroadc();

protected:
	virtual void machine_start() override ATTR_COLD;

	void crusnwld_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t crusnwld_serial_status_r();
	uint32_t crusnwld_serial_data_r();
	void crusnwld_serial_data_w(uint32_t data);
	uint32_t bit_data_r(offs_t offset);
	void bit_reset_w(uint32_t data);
	void init_crusnwld_common(offs_t speedup);

	void crusnwld_map(address_map &map) ATTR_COLD;
	void offroadc_map(address_map &map) ATTR_COLD;

	required_device<midway_serial_pic2_device> m_midway_serial_pic2;

	uint16_t m_bit_index = 0;
};

class midvplus_state : public midvunit_base_state
{
public:
	midvplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: midvunit_base_state(mconfig, type, tag)
		, m_midway_ioasic(*this, "ioasic")
		, m_ata(*this, "ata")
		, m_fastram_base(*this, "fastram_base")
		, m_midvplus_misc(*this, "midvplus_misc")
	{ }

	void midvplus(machine_config &config);

	void init_wargods();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t midvplus_misc_r(offs_t offset);
	void midvplus_misc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void midvplus_xf1_w(uint8_t data);

	void midvplus_map(address_map &map) ATTR_COLD;

	required_device<midway_ioasic_device> m_midway_ioasic;
	required_device<ata_interface_device> m_ata;
	required_shared_ptr<uint32_t> m_fastram_base;
	required_shared_ptr<uint32_t> m_midvplus_misc;

	int m_lastval = 0;
};

#endif // MAME_MIDWAY_MIDVUNIT_H
