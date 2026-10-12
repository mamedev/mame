// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 6560/6561 Video Interface Chip (VIC) emulation

****************************************************************************
                            _____   _____
                   N/C   1 |*    \_/     | 40  Vdd
                CHROMA   2 |             | 39  phi1
             LUMA/SYNC   3 |             | 38  phi2
                   R/W   4 |             | 37  OPTION
                   D11   5 |             | 36  Pphi2
                   D10   6 |             | 35  Pphi1
                    D9   7 |             | 34  A13
                    D8   8 |             | 33  A12
                    D7   9 |             | 32  A11
                    D6  10 |   MOS6560   | 31  A10
                    D5  11 |   MOS6561   | 30  A9
                    D4  12 |             | 29  A8
                    D3  13 |             | 28  A7
                    D2  14 |             | 27  A6
                    D1  15 |             | 26  A5
                    D0  16 |             | 25  A4
                 POT X  17 |             | 24  A3
                 POT Y  18 |             | 23  A2
                 AUDIO  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

***************************************************************************/

#ifndef MAME_SOUND_MOS6560_H
#define MAME_SOUND_MOS6560_H

#pragma once

#include "screen.h"


//***************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************

// ======================> mos6560_device

class mos6560_device : public device_t,
						public device_memory_interface,
						public device_sound_interface,
						public device_video_interface
{
public:
	static constexpr XTAL CLOCK = 14.318181_MHz_XTAL / 14;
	static constexpr uint16_t CYCLES = 65;
	static constexpr uint16_t LINES = 261;
	static constexpr uint16_t HTOTAL = CYCLES * 4;
	static constexpr uint16_t HBEND = 4;
	static constexpr uint16_t HBSTART = 204;
	static constexpr uint16_t VBEND = 28;

	mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto potx_rd_callback() { return m_read_potx.bind(); }
	auto poty_rd_callback() { return m_read_poty.bind(); }

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t bus_r();
	void bus_w(uint8_t data);
	void bus_w(uint8_t data, uint8_t high);
	uint8_t fetch_open_bus_r() const { return m_vbus & (0xf0 | m_vbus_high); }

	void lp_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mos6560_colorram_map(address_map &map) ATTR_COLD;
	void mos6560_videoram_map(address_map &map) ATTR_COLD;

protected:
	mos6560_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cycles_per_line, int lines, int raster_offset, const rgb_t *palette, bool ntsc, bool interlace, bool attack_ufo);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	enum : uint8_t
	{
		AREA_IDLE,
		AREA_PENDING,
		AREA_DISPLAY,
		AREA_DONE
	};

	enum : uint8_t
	{
		FETCH_IDLE,
		FETCH_START,
		FETCH_MATRIX,
		FETCH_CHARGEN,
		FETCH_DONE
	};

	TIMER_CALLBACK_MEMBER(line_tick);

	uint64_t current_cycle();
	uint64_t cycle_at(const attotime &time) const;
	void update_cycles();
	void cycle();
	void end_of_line();
	void end_of_frame();
	void open_v();
	void close_v();
	void open_h();
	void handle_vmc();
	void count_vmc();
	void fetch();
	uint8_t fetch_byte(offs_t addr);
	void output_pixels();
	void trigger_light_pen();
	int raster_line(uint64_t cycle, int *pos = nullptr) const;
	int last_line_cycles(int line) const;

	const address_space_config m_videoram_space_config;
	const address_space_config m_colorram_space_config;

	const int m_cycles_per_line;
	const int m_lines;
	const int m_raster_offset;
	const rgb_t *const m_palette;
	const bool m_ntsc;
	const bool m_interlace_capable;
	const bool m_attack_ufo;

	devcb_read8 m_read_potx;
	devcb_read8 m_read_poty;

	sound_stream *m_stream;
	emu_timer *m_line_timer;

	bitmap_rgb32 m_bitmap[2];
	int32_t m_draw;

	attoseconds_t m_period;
	attotime m_base_time;
	uint64_t m_base_cycle;
	uint64_t m_cycle_count;
	uint64_t m_frame_start;
	int32_t m_frame_lines;

	uint8_t m_reg[16];

	// raster
	int32_t m_line;
	int32_t m_cyc;
	uint8_t m_area;
	uint8_t m_fetch_state;
	int32_t m_start_delay;
	int32_t m_buf_offset;
	int32_t m_text_cols;
	int32_t m_pending_text_cols;
	int32_t m_text_lines;
	int32_t m_char_height;
	int32_t m_ycounter;
	int32_t m_row_counter;
	int32_t m_vmc;
	int32_t m_vmb;
	int32_t m_vmc_left;
	bool m_vmc_inc;
	bool m_vmc_wrap_inc;
	bool m_blank_this_line;
	bool m_line_was_blank;
	bool m_interlace;
	int32_t m_field;

	// fetch and shifter
	uint8_t m_vbuf;
	uint8_t m_cbuf;
	uint8_t m_gload[3];
	uint8_t m_cload[3];
	bool m_load[3];
	uint8_t m_gdata;
	uint8_t m_cdata;
	int32_t m_shift_pos;
	uint8_t m_vbus;
	uint8_t m_vbus_high;

	// output registers
	uint8_t m_out_bg;
	uint8_t m_out_border;
	uint8_t m_out_aux;
	bool m_out_reverse;
	int32_t m_pend_reg;
	uint8_t m_pend_data;

	// light pen
	bool m_lp_state;
	bool m_lp_triggered;
	uint64_t m_lp_trigger;
	uint8_t m_lp_x;
	uint8_t m_lp_y;
	uint8_t m_lp_extra;

	// sound
	uint8_t m_snd_div;
	uint8_t m_snd_cnt[4];
	uint8_t m_snd_sreg[4];
	uint16_t m_noise_lfsr;
	bool m_noise_lfsr0;
	sound_stream::sample_t m_dac[(4 * 7 + 1) * 15 + 1];
};


// ======================> mos6561_device

class mos6561_device : public mos6560_device
{
public:
	static constexpr XTAL CLOCK = 4.433619_MHz_XTAL / 4;
	static constexpr uint16_t CYCLES = 71;
	static constexpr uint16_t LINES = 312;
	static constexpr uint16_t HTOTAL = CYCLES * 4;
	static constexpr uint16_t HBEND = 24;
	static constexpr uint16_t HBSTART = 248;
	static constexpr uint16_t VBEND = 28;

	// construction/destruction
	mos6561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> mos656x_attack_ufo_device

class mos656x_attack_ufo_device : public mos6560_device
{
public:
	// construction/destruction
	mos656x_attack_ufo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definitions
DECLARE_DEVICE_TYPE(MOS6560,            mos6560_device)
DECLARE_DEVICE_TYPE(MOS6561,            mos6561_device)
DECLARE_DEVICE_TYPE(MOS656X_ATTACK_UFO, mos656x_attack_ufo_device)

#endif // MAME_SOUND_MOS6560_H
