// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 6566/6567/6569 Video Interface Chip II (VIC-II) emulation

****************************************************************************
                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A13
                  _IRQ   8 |             | 33  A12
                    LP   9 |             | 32  A11
                   _CS  10 |   MOS6566   | 31  A10
                   R/W  11 |             | 30  A9
                    BA  12 |             | 29  A8
                   Vdd  13 |             | 28  A7
                 COLOR  14 |             | 27  A6
                 S/LUM  15 |             | 26  A5
                   AEC  16 |             | 25  A4
                   PH0  17 |             | 24  A3
                  PHIN  18 |             | 23  A2
                 PHCOL  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A10
                  _IRQ   8 |             | 33  A9
                    LP   9 |   MOS6567   | 32  A8
                   _CS  10 |   MOS6569   | 31  A7
                   R/W  11 |   MOS8562   | 30  A6
                    BA  12 |   MOS8565   | 29  A5/A13
                   Vdd  13 |             | 28  A4/A12
                 COLOR  14 |             | 27  A3/A11
                 S/LUM  15 |             | 26  A2/A10
                   AEC  16 |             | 25  A1/A9
                   PH0  17 |             | 24  A0/A8
                  _RAS  18 |             | 23  A11
                   CAS  19 |             | 22  PHIN
                   Vss  20 |_____________| 21  PHCL

                            _____   _____
                    D6   1 |*    \_/     | 48  Vcc
                    D5   2 |             | 47  D7
                    D4   3 |             | 46  D8
                    D3   4 |             | 45  D9
                    D2   5 |             | 44  D10
                    D1   6 |             | 43  D11
                    D0   7 |             | 42  MA10
                  _IRQ   8 |             | 41  MA9
                   _LP   9 |             | 40  MA8
                    BA  10 |             | 39  A7
              _DMARQST  11 |             | 38  A6
                   AEC  12 |   MOS8564   | 37  MA5
                   _CS  13 |   MOS8566   | 36  MA4
                   R/W  14 |             | 35  MA3
               _DMAACK  15 |             | 34  MA2
                CHROMA  16 |             | 33  MA1
              SYNC/LUM  17 |             | 32  MA0
                 1 MHZ  18 |             | 31  MA11
                  _RAS  19 |             | 30  PHI IN
                  _CAS  20 |             | 29  PHI COLOR
                   MUX  21 |             | 28  K2
                _IOACC  22 |             | 27  K1
                 2 MHZ  23 |             | 26  K0
                   Vss  24 |_____________| 25  Z80 PHI

***************************************************************************/

#ifndef MAME_VIDEO_MOS6566_H
#define MAME_VIDEO_MOS6566_H

#pragma once


class mos6566_device : public device_t,
					public device_memory_interface,
					public device_video_interface,
					public device_execute_interface
{
public:
	static constexpr XTAL VIC6566_CLOCK = XTAL(8'000'000) / 8; // 1000000
	static constexpr XTAL VIC6567_CLOCK = XTAL(14'318'181) / 14; // 1022727
	static constexpr XTAL VIC6569_CLOCK = XTAL(17'734'472) / 18; // 985248

	static constexpr int VIC6566_LINES = 262;
	static constexpr int VIC6567_LINES = 263;
	static constexpr int VIC6567R56A_LINES = 262;
	static constexpr int VIC6569_LINES = 312;

	static constexpr int VIC6567_VISIBLELINES = 247;
	static constexpr int VIC6569_VISIBLELINES = 272;

	static constexpr int VIC6567_FIRST_DISP_LINE = 0x1c;
	static constexpr int VIC6569_FIRST_DISP_LINE = 0x10;

	static constexpr int VIC6569_LAST_DISP_LINE = VIC6569_FIRST_DISP_LINE + VIC6569_VISIBLELINES - 1;

	static constexpr int VIC6567_VISIBLECOLUMNS = 384;
	static constexpr int VIC6569_VISIBLECOLUMNS = 384;

	static constexpr int VIC6566_COLUMNS = 512;
	static constexpr int VIC6567_COLUMNS = 520;
	static constexpr int VIC6567R56A_COLUMNS = 512;
	static constexpr int VIC6569_COLUMNS = 504;

	static constexpr int VIC6567_FIRST_COLUMN = 92;
	static constexpr int VIC6569_FIRST_COLUMN = 92;

	mos6566_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class T> void set_cpu(T &&tag) { m_cpu.set_tag(tag); }
	void set_palette(const rgb_t (&palette)[16]) { m_palette = palette; }
	auto irq_callback() { return m_write_irq.bind(); }
	auto ba_callback() { return m_write_ba.bind(); }
	auto aec_callback() { return m_write_aec.bind(); }
	auto k_callback() { return m_write_k.bind(); }
	auto charrom_callback() { return m_read_charrom.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void lp_w(int state);

	// time until the chip's own raster counters (as latched by lp_w into LPX/LPY) reach the given position
	attotime time_until_pos(int rasterline, int raster_x = 0) const;

	// same, but taking a light pen crosshair position in 0-255 fractional-of-visible-picture units
	attotime time_until_lightpen_pos(int x255, int y255) const;

	int phi0_r() const { return m_phi0; }
	int ba_r() const { return m_ba; }
	int aec_r() const { return m_aec; }
	uint8_t bus_r() { return m_last_data; }

	void cpu_access(int ioacc);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TYPE_6566,  // NTSC-M (SRAM)
		TYPE_6567R56A,  // NTSC-M (NMOS, 64 cycles per line)
		TYPE_6567,  // NTSC-M (NMOS)
		TYPE_8562,  // NTSC-M (HMOS)
		TYPE_8564,  // NTSC-M VIC-IIe (C128)

		TYPE_6569,  // PAL-B
		TYPE_8565,  // PAL-B (HMOS)
		TYPE_8566   // PAL-B VIC-IIe (C128)
	};

	mos6566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;
	virtual void execute_run() override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return is_viciie() ? clocks / 8 : clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return is_viciie() ? cycles * 8 : cycles; }

private:
	static constexpr int MAX_CYCLES_PER_LINE = 65;

	struct reg_write
	{
		uint8_t reg;
		uint8_t data;
		uint8_t mask;
		uint64_t when;
	};

	struct raster_timing;
	struct process_traits;

	struct cycle_decode
	{
		uint16_t strobes;
		int8_t spr_pointer;
		int8_t spr_data;
		uint8_t spr_ba;
	};

	TIMER_CALLBACK_MEMBER(fast_changed);

	template <int Space> void default_map(address_map &map) ATTR_COLD;

	bool is_ntsc() const { return m_variant == TYPE_6566 || m_variant == TYPE_6567R56A || m_variant == TYPE_6567 || m_variant == TYPE_8562 || m_variant == TYPE_8564; }
	bool is_viciie() const { return m_variant == TYPE_8564 || m_variant == TYPE_8566; }
	bool fast_mode() const { return is_viciie() && BIT(m_reg[0x30], 0); }

	static const raster_timing &raster_timing_for(uint32_t variant) ATTR_COLD;
	static const process_traits &process_traits_for(uint32_t variant) ATTR_COLD;

	void build_decode() ATTR_COLD;
	int sprite_cycle(int sprite) const;
	int phase_x(int phase) const;
	void set_ba(int state);
	void update_irq();
	void raise_irq(uint8_t mask);
	void check_raster_irq();
	void trigger_lightpen(int cycle);
	uint8_t fetch(offs_t address);
	uint8_t fetch_phi2(offs_t address);
	uint8_t read_color(offs_t offset);

	void line_start();
	void update_badline();
	void sprite_dma_check();
	bool sprite_phase1(const cycle_decode &decode);
	void sprite_phase2(const cycle_decode &decode);
	void graphics_fetch(bool display, bool dma_delay);
	void matrix_fetch();
	void queue_register(uint8_t reg, uint8_t data);
	void queue_register(uint8_t reg, uint8_t data, uint8_t mask, int seen);
	void queue_edges(uint8_t reg, uint8_t data, uint8_t mask, int rise_seen, int fall_seen);
	int mcm_fall_lag(uint8_t cr1) const;
	uint8_t fetch_mode() const;
	void apply_register(uint8_t reg, uint8_t data, uint8_t mask);
	void apply_registers(uint64_t dot);
	void draw_until(uint64_t dot);
	void draw_dot();
	void border_unit(int x, uint8_t cr1, uint8_t cr2);
	bool graphics_sequencer(uint8_t cr1, uint8_t cr2, int &color);
	uint8_t sprite_sequencer(int x, int (&color)[8]);
	void collision_unit(uint8_t sprite_mask, bool fg);
	void output_stage(int color);

	int m_icount;
	const int m_variant;
	const raster_timing &m_timing;
	const process_traits &m_process;

	const address_space_config m_space_config[2];

	devcb_write_line m_write_irq;
	devcb_write_line m_write_ba;
	devcb_write_line m_write_aec;
	devcb_write8 m_write_k;
	devcb_read8 m_read_charrom;

	required_device<cpu_device> m_cpu;

	const rgb_t *m_palette;

	emu_timer *m_fast_timer;

	int32_t m_phi0;
	int32_t m_ba;
	int32_t m_aec;
	uint8_t m_aec_delay;

	uint8_t m_reg[0x40];

	int32_t m_rasterline;
	uint8_t m_cycle;
	uint16_t m_raster_x;
	uint8_t m_last_data;
	int8_t m_bus_slot;
	int32_t m_lp;
	bool m_lp_latched_this_frame;

	bitmap_rgb32 m_bitmap;

	cycle_decode m_decode[MAX_CYCLES_PER_LINE + 1];
	uint16_t m_phase_x[2 * MAX_CYCLES_PER_LINE];

	int8_t m_ba_out;
	int8_t m_aec_out;

	uint16_t m_beam_line;
	uint8_t m_irq_flags;
	uint8_t m_irq_enable;
	uint8_t m_irq_out;
	bool m_raster_irq_done;
	bool m_lp_pending;

	bool m_badline;
	bool m_badlines_enabled;

	bool m_display_state;
	uint8_t m_fetch_cr1;
	uint16_t m_vc;
	uint16_t m_vcbase;
	uint8_t m_vmli;
	uint8_t m_rc;
	uint8_t m_refresh;
	uint8_t m_matrix[64];
	uint8_t m_color[64];

	uint8_t m_spr_dma;
	uint8_t m_spr_disp;
	uint8_t m_spr_yff;
	uint8_t m_spr_mcbase[8];
	uint8_t m_spr_mc[8];
	uint8_t m_spr_pointer[8];
	uint8_t m_spr_byte[8][3];

	uint8_t m_spr_pending;
	uint8_t m_spr_active;
	uint8_t m_spr_halt;
	uint32_t m_spr_shift_data[8];
	uint8_t m_spr_xphase;
	uint8_t m_spr_mcphase;
	uint8_t m_spr_pixel[8];

	uint8_t m_dreg[0x40];
	uint8_t m_pixel_color[4];
	uint16_t m_pixel_row[4];
	uint16_t m_pixel_col[4];
	uint8_t m_pixel_index;
	uint8_t m_pixel_count;
	uint8_t m_color_write;
	uint8_t m_coll_mm[8];
	uint8_t m_coll_md[8];
	uint8_t m_coll_index;
	uint64_t m_coll_clear[2];
	reg_write m_write_queue[32];
	uint8_t m_write_count;
	uint64_t m_write_last[0x40];
	uint64_t m_dot;
	uint64_t m_cycle_dot;
	uint16_t m_draw_row;
	uint16_t m_draw_col;
	uint8_t m_draw_pos;

	uint8_t m_latch_gfx[2];
	uint16_t m_latch_vbuf[2];
	uint8_t m_seq_shift;
	uint16_t m_seq_vbuf;
	uint8_t m_seq_count;
	uint8_t m_seq_mc;
	bool m_seq_mcm;
	bool m_seq_cell_mc;
	uint8_t m_seq_bmm;
	uint8_t m_seq_xscroll;

	bool m_main_border;
	bool m_vert_border;
	bool m_vert_ff;
};


class mos6567_device : public mos6566_device
{
public:
	mos6567_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mos6567_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);
};


class mos6567r56a_device : public mos6567_device
{
public:
	mos6567r56a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mos8562_device : public mos6567_device
{
public:
	mos8562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mos8564_device : public mos6567_device
{
public:
	mos8564_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mos6569_device : public mos6566_device
{
public:
	mos6569_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mos6569_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);
};


class mos8565_device : public mos6569_device
{
public:
	mos8565_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mos8566_device : public mos6569_device
{
public:
	mos8566_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(MOS6566, mos6566_device)
DECLARE_DEVICE_TYPE(MOS6567, mos6567_device)
DECLARE_DEVICE_TYPE(MOS6567R56A, mos6567r56a_device)
DECLARE_DEVICE_TYPE(MOS8562, mos8562_device)
DECLARE_DEVICE_TYPE(MOS8564, mos8564_device)
DECLARE_DEVICE_TYPE(MOS6569, mos6569_device)
DECLARE_DEVICE_TYPE(MOS8565, mos8565_device)
DECLARE_DEVICE_TYPE(MOS8566, mos8566_device)

#endif // MAME_VIDEO_MOS6566_H
