// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// DSP563xx unit tests

#include "emu.h"
#include "cpu/dsp563xx/dsp56311.h"
#include "cpu/dsp563xx/dsp563xxd.h"

#include "screen.h"
#include "speaker.h"

extern const uint8_t font_uismall[];

class ut_debug_buffer : public util::disasm_interface::data_buffer
{
public:
	const u32 *m_start;
	u32 m_size;

	ut_debug_buffer(const u32 *start, u32 size);
	virtual u8  r8 (offs_t pc) const override;
	virtual u16 r16(offs_t pc) const override;
	virtual u32 r32(offs_t pc) const override;
	virtual u64 r64(offs_t pc) const override;
};

ut_debug_buffer::ut_debug_buffer(const u32 *start, u32 size) : m_start(start), m_size(size)
{
}

u8  ut_debug_buffer::r8 (offs_t pc) const
{
	return 0;
}

u16 ut_debug_buffer::r16(offs_t pc) const
{
	return 0;
}

u32 ut_debug_buffer::r32(offs_t pc) const
{
	return pc < m_size ? m_start[pc] : 0;
}

u64 ut_debug_buffer::r64(offs_t pc) const
{
	return 0;
}


class utdsp563xx_state : public driver_device
{
public:
    utdsp563xx_state(const machine_config &mconfig, device_type type, const char *tag);

    void utdsp563xx(machine_config &config);

private:
	enum { TESTS_PER_COLUMN = 28 };
	enum { PRE_PER_COLUMN = 6, CODE_PER_COLUMN=9, POST_PER_COLUMN = 5 };

	enum {
		MODE_ALL,
		MODE_SINGLE
	};

	enum {
		F_CCR_C = 0x1000,
		F_CCR_V = 0x1001,
		F_CCR_Z = 0x1002,
		F_CCR_N = 0x1003,
		F_CCR_U = 0x1004,
		F_CCR_E = 0x1005,
		F_CCR_L = 0x1006,
		F_CCR_S = 0x1007,
		F_SR_S0 = 0x100b,
		F_SR_S1 = 0x100c,

		MEM_P   = 0x2000,
		MEM_X   = 0x2001,
		MEM_Y   = 0x2002,
	};

	struct testblock {
		const char *m_name;
		u32 m_testlist_start, m_testlist_end;
	};

	struct testlist {
		u32 m_code_start, m_code_end;
		u32 m_pre_start, m_pre_end;
		u32 m_post_start, m_post_end;
	};

	struct reg {
		u32 m_reg;
		u64 m_value;
	};

	static const testblock testblocks[];
	static const testlist testlists[];
	static const u32 code[];
	static const reg regs[];

    required_device<dsp56311_device> m_cpu;
	required_region_ptr<u16> m_font;
	required_ioport m_control;

	dsp563xx_disassembler m_disassembler;

	std::map<u32, u32> m_p_data;
	std::map<u32, u32> m_x_data;
	std::map<u32, u32> m_y_data;

	std::vector<u8> m_test_success;
	std::vector<u64> m_reg_results;

	std::array<u16, 128*64> m_chars;
	s32 m_first_fail;
	s32 m_selected_block;
	u32 m_code_start, m_code_end, m_pre_start, m_pre_end, m_post_start, m_post_end;
	u32 m_blocks, m_tests;
	u32 m_cur_block, m_cur_test;
	u8 m_cur_port;
	u8 m_mode;

	bool m_initial_run;

    virtual void machine_start() override ATTR_COLD;
    virtual void machine_reset() override ATTR_COLD;

	void p_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static std::pair<int, int> test2pos(int block);
	int test_count(int block) const;
	void clear();
	void xprint(int x, int y, std::string s, int c0, int c1);
	void draw_main_test_list();
	void draw_single_test();
	std::string reg_name(u32 index) const;
	int reg_value_size(u32 index) const;
	u64 reg_expected_value(u32 index) const;
	u64 reg_actual_value(u32 index) const;

	u32 code_r(offs_t address);
	u32 done_r();
	u32 jmp0_r();
	void trigger_test(bool stop = false);

	void any_w(const char *name, std::map<u32, u32> &area, offs_t offset, u32 data);
	u32 any_r(const char *name, std::map<u32, u32> &area, offs_t offset);

	void p_w(offs_t offset, u32 data);
	u32 p_r(offs_t offset);
	void x_w(offs_t offset, u32 data);
	u32 x_r(offs_t offset);
	void y_w(offs_t offset, u32 data);
	u32 y_r(offs_t offset);

	void tick(int state);
};

utdsp563xx_state::utdsp563xx_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_cpu(*this, "cpu")
	, m_font(*this, "font")
	, m_control(*this, "CONTROL")
{
}

u32 utdsp563xx_state::code_r(offs_t address)
{
	auto i = m_p_data.find(address);
	if(i != m_p_data.end())
		return i->second;

	u32 clen = m_code_end - m_code_start;
	if(address >= clen)
		return 0x0c0fff;
	return code[m_code_start + address];
}

u32 utdsp563xx_state::done_r()
{
	if(machine().side_effects_disabled())
		return 0;
	m_cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	if(m_initial_run) {
		int result = 2;
		for(int r = m_post_start; r != m_post_end; r++) {
			u64 val = reg_actual_value(r);
			m_reg_results[r] = val;
			if(val != reg_expected_value(r))
				result = 1;
		}
		if(result == 1 && m_first_fail == -1)
			m_first_fail = m_cur_block;
		
		m_test_success[testblocks[m_cur_block].m_testlist_start + m_cur_test] = result;
		m_cur_test++;

		while(m_cur_test == test_count(m_cur_block)) {
			m_cur_test = 0;
			m_cur_block ++;
			if(m_cur_block == m_blocks) {
				m_initial_run = false;
				m_cur_block = 0;
				break;
			}
		}
		if(m_initial_run)
			trigger_test();
	}

	return 0;
}

u32 utdsp563xx_state::jmp0_r()
{
	for(int i = m_pre_start; i != m_pre_end; i++) {
		u64 val = reg_expected_value(i);
		if(regs[i].m_reg >= MEM_P) {
			auto &area = regs[i].m_reg == MEM_P ? m_p_data : regs[i].m_reg == MEM_X ? m_x_data : m_y_data;
			area[regs[i].m_value >> 24] = val;

		} else if(regs[i].m_reg >= F_CCR_C) {
			u32 sr = m_cpu->state_int(DSP563XX_CCR);
			if(val)
				sr |= 1 << (regs[i].m_reg & 15);
			else
				sr &= ~(1 << (regs[i].m_reg & 15));
			m_cpu->set_state_int(DSP563XX_SR, sr);

		} else
			m_cpu->set_state_int(regs[i].m_reg, val);
	}
	return 0x0c0000; // jmp 0
}

void utdsp563xx_state::p_map(address_map &map)
{
	map(0x000000*4, 0xffffff*4).rw(FUNC(utdsp563xx_state::p_r), FUNC(utdsp563xx_state::p_w));
	map(0x000000*4, 0x000ffe*4).r(FUNC(utdsp563xx_state::code_r));
	map(0x000fff*4, 0x000fff*4).r(FUNC(utdsp563xx_state::done_r));
	map(0xc00000*4, 0xc00000*4).r(FUNC(utdsp563xx_state::jmp0_r));
}

void utdsp563xx_state::any_w(const char *name, std::map<u32, u32> &area, offs_t offset, u32 data)
{
	area[offset] = data;
}

u32 utdsp563xx_state::any_r(const char *name, std::map<u32, u32> &area, offs_t offset)
{
	auto i = area.find(offset);
	if(i != area.end())
		return i->second;
	if(!machine().side_effects_disabled())
		logerror("Unmapped read from %s address %06x\n", name, offset);
	return 0;
}

void utdsp563xx_state::p_w(offs_t offset, u32 data)
{
	any_w("P", m_p_data, offset, data);
}

u32 utdsp563xx_state::p_r(offs_t offset)
{
	return any_r("P", m_p_data, offset);
}

void utdsp563xx_state::x_w(offs_t offset, u32 data)
{
	any_w("X", m_x_data, offset, data);
}

u32 utdsp563xx_state::x_r(offs_t offset)
{
	return any_r("X", m_x_data, offset);
}

void utdsp563xx_state::y_w(offs_t offset, u32 data)
{
	any_w("Y", m_y_data, offset, data);
}

u32 utdsp563xx_state::y_r(offs_t offset)
{
	return any_r("Y", m_y_data, offset);
}

std::pair<int, int> utdsp563xx_state::test2pos(int block)
{
	int y = 2+(block % TESTS_PER_COLUMN);
	int x = (block / TESTS_PER_COLUMN)*40;
	return std::make_pair(x, y);
}

int utdsp563xx_state::test_count(int block) const
{
	return testblocks[block].m_testlist_end - testblocks[block].m_testlist_start;
}

void utdsp563xx_state::clear()
{
	std::fill(m_chars.begin(), m_chars.end(), 0x3820);
}

void utdsp563xx_state::xprint(int x, int y, std::string s, int c0, int c1)
{
	u16 mask = (c1 << (8+3)) | (c0 << 8);
	for(int c : s) {
		m_chars[x+y*128] = c | mask;
		x++;
	}
}

void utdsp563xx_state::draw_main_test_list()
{
	clear();
	xprint(0, 0, "DSP563xx unit testing", 0, 6);
	xprint(60, 0, util::string_format("%d blocks", m_blocks), 0, 6);
	xprint(90, 0, util::string_format("%d tests", m_tests), 0, 6);
	
	for(int i=0; i != m_blocks; i++) {
		auto [xo, yo] = test2pos(i);
		bool sel = i == m_selected_block;
		xprint(xo, yo, testblocks[i].m_name, sel ? 7 : 0, sel ? 1 : 7);
		int fails = 0, count = 0;
		for(int j=testblocks[i].m_testlist_start; j != testblocks[i].m_testlist_end; j++) {
			if(m_test_success[j] == 0)
				goto skip;
			else if(m_test_success[j] == 1)
				fails ++;
			count++;
		}
		if(count != 0) {
			if(fails == 0)
				xprint(xo+20, yo, "PASS", 0, 2);
			else
				xprint(xo+20, yo, util::string_format("FAIL %4d/%4d", fails, count), 0, 4);
		}
	skip:;
	}
	xprint(0, 32, "Arrows to move, Button 1 to select", 0, 3);
}

std::string utdsp563xx_state::reg_name(u32 index) const
{
	if(regs[index].m_reg >= MEM_P)
		return util::string_format("%c:%06x", "PXY"[regs[index].m_reg & 3], regs[index].m_value >> 24);

	if(regs[index].m_reg >= F_SR_S0)
		return util::string_format("SR.S%d", regs[index].m_reg - F_SR_S0);

	if(regs[index].m_reg >= F_CCR_C)
		return util::string_format("CCR.%c", "CVZNUELS"[regs[index].m_reg - F_CCR_C]);

	return m_cpu->state_find_entry(regs[index].m_reg)->symbol();
}

int utdsp563xx_state::reg_value_size(u32 index) const
{
	if(regs[index].m_reg >= MEM_P)
		return 6;
	if(regs[index].m_reg >= F_CCR_C)
		return 1;
	switch(regs[index].m_reg) {
	case DSP563XX_A: case DSP563XX_B: return 14;
	case DSP563XX_X: case DSP563XX_Y: return 12;
	case DSP563XX_A2: case DSP563XX_B2: return 8;
	default: return 6;
	}
}

u64 utdsp563xx_state::reg_expected_value(u32 index) const
{
	if(regs[index].m_reg >= MEM_P)
		return regs[index].m_value & 0xffffff;
	else
		return regs[index].m_value;
}

u64 utdsp563xx_state::reg_actual_value(u32 index) const
{
	u64 rval = regs[index].m_value;
	if(regs[index].m_reg >= MEM_P) {
		offs_t offset = rval >> 24;
		rval &= 0xffffff;
		const auto &area = regs[index].m_reg == MEM_P ? m_p_data : regs[index].m_reg == MEM_X ? m_x_data : m_y_data;
		auto i = area.find(offset);
		if(i != area.end())
			return i->second;
		else
			return ~u64(0);

	} else if(regs[index].m_reg >= F_CCR_C) {
		u32 sr = m_cpu->state_int(DSP563XX_SR);
		return BIT(sr, regs[index].m_reg & 31);

	} else
		return m_cpu->state_int(regs[index].m_reg);
}

void utdsp563xx_state::draw_single_test()
{
	clear();
	int tidx = testblocks[m_cur_block].m_testlist_start + m_cur_test;

	xprint(0, 0, "DSP563xx unit testing", 0, 6);
	xprint(60, 0, testblocks[m_cur_block].m_name, 0, 6);
	if(m_test_success[tidx] == 2)
		xprint(85, 0, "PASS", 0, 2);
	else
		xprint(85, 0, "FAIL", 0, 4);
	xprint(90, 0, util::string_format("%d/%d", m_cur_test+1, test_count(m_cur_block)), 0, 6);

	const testlist &tl = testlists[tidx];

	u32 col = 0;
	u32 line = 0;
	for(u32 i = tl.m_pre_start; i != tl.m_pre_end; i++) {
		std::string rn = reg_name(i);
		xprint(40*col, 2+line, rn, 0, 7);
		xprint(40*col+9, 2+line, util::string_format("%0*x", reg_value_size(i), reg_expected_value(i)), 0, 7);
		line ++;
		if(line == PRE_PER_COLUMN) {
			line = 0;
			col ++;
		}
	}


	offs_t code_len = tl.m_code_end - tl.m_code_start;
	ut_debug_buffer data(code+tl.m_code_start, code_len);

	offs_t pos = 0;
	col = 0;
	line = 0;
	while(pos != code_len) {
		std::ostringstream os;
		offs_t off = m_disassembler.disassemble(os, pos, data, data);
		xprint(40*col, 2+PRE_PER_COLUMN+1+line, util::string_format("%02x: %06x %s", pos, data.r32(pos), os.str()), 0, 7);
		line ++;
		if(line == CODE_PER_COLUMN) {
			line = 0;
			col ++;
		}
		pos += off & util::disasm_interface::LENGTHMASK;
	}


	col = 0;
	line = 0;
	for(u32 i = tl.m_post_start; i != tl.m_post_end; i++) {
		std::string rn = reg_name(i);
		xprint(40*col, 2+PRE_PER_COLUMN+1+CODE_PER_COLUMN+1+line, rn, 0, 7);
		u64 rval = reg_expected_value(i);
		u64 val = m_reg_results[i];
		int rs = reg_value_size(i);
		if(rval == val)
			xprint(40*col+9, 2+PRE_PER_COLUMN+1+CODE_PER_COLUMN+1+line, util::string_format("%0*x", rs, rval), 0, 2);
		else {
			xprint(40*col+9, 2+PRE_PER_COLUMN+1+CODE_PER_COLUMN+1+line, util::string_format("%0*x", rs, val), 0, 4);
			xprint(40*col+9+15, 2+PRE_PER_COLUMN+1+CODE_PER_COLUMN+1+line, util::string_format("%0*x", rs, rval), 0, 7);
		}
		line ++;
		if(line == POST_PER_COLUMN) {
			line = 0;
			col ++;
		}
	}
	
	xprint(0, 32, "Left/right to select test, Button 1 to machine().debug_break(), Button 2 to return to main list", 0, 3);
}

void utdsp563xx_state::machine_start()
{
	save_item(NAME(m_chars));
	save_item(NAME(m_code_start));
	save_item(NAME(m_code_end));
	save_item(NAME(m_pre_start));
	save_item(NAME(m_pre_end));
	save_item(NAME(m_post_start));
	save_item(NAME(m_post_end));
	save_item(NAME(m_initial_run));
	save_item(NAME(m_cur_block));
	save_item(NAME(m_cur_test));
	save_item(NAME(m_selected_block));
	save_item(NAME(m_first_fail));
	save_item(NAME(m_mode));

	m_blocks = 0;
	m_tests = 0;
	while(testblocks[m_blocks].m_name) {
		m_tests += test_count(m_blocks);
		m_blocks ++;
	}

	m_test_success.resize(m_tests, 0);
	save_item(NAME(m_test_success));
	m_reg_results.resize(testlists[m_tests-1].m_post_end, 0);
}

void utdsp563xx_state::machine_reset()
{
	m_selected_block = -1;
	m_first_fail = -1;
	m_mode = MODE_ALL;

	m_initial_run = true;

	// Have to do it there otherwise the internal maps override our stuff
	m_cpu->space(dsp563xx_device::AS_P).install_device(0, 0xffffff, *this, &utdsp563xx_state::p_map);
	m_cpu->space(dsp563xx_device::AS_X).install_readwrite_handler(0, 0xffffff, emu::rw_delegate(*this, FUNC(utdsp563xx_state::x_r)), emu::rw_delegate(*this, FUNC(utdsp563xx_state::x_w)));
	m_cpu->space(dsp563xx_device::AS_Y).install_readwrite_handler(0, 0xffffff, emu::rw_delegate(*this, FUNC(utdsp563xx_state::y_r)), emu::rw_delegate(*this, FUNC(utdsp563xx_state::y_w)));
	m_cur_block = 0;
	m_cur_test = 0;
	trigger_test();
}

uint32_t utdsp563xx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for(int y=0; y != 33; y++) {
		for(int x=0; x != 120; x++) {
			u16 c = m_chars[x + y*128];
			u32 color0 = 0;
			if(c & 0x0100)
				color0 |= 0x0000ff;
			if(c & 0x0200)
				color0 |= 0x00ff00;
			if(c & 0x0400)
				color0 |= 0xff0000;
			u32 color1 = 0;
			if(c & 0x0800)
				color1 |= 0x0000ff;
			if(c & 0x1000)
				color1 |= 0x00ff00;
			if(c & 0x2000)
				color1 |= 0xff0000;
			const u16 *cdata = m_font + (c & 0xff)*32;
			for(int yy=0; yy != 32; yy++) {
				u32 *dest = &bitmap.pix(y*32+yy+12, x*16);
				u16 pix = *cdata++;
				for(int xx=0; xx != 16; xx++)
					if(BIT(pix, 15-xx))
						*dest++ = color1;
					else
						*dest++ = color0;
			}
		}
	}
	return 0;
}

void utdsp563xx_state::tick(int state)
{
	if(!state)
		return;
	if(m_initial_run) {
		draw_main_test_list();
		return;
	}
	if(m_selected_block == -1) {
		m_selected_block = m_first_fail == -1 ? 0 : m_first_fail;
		draw_main_test_list();
	}

	u8 iport = m_control->read();
	u8 ctrl = iport & ~m_cur_port;
	m_cur_port = iport;

	if(m_mode == MODE_ALL) {
		if(ctrl & 0x01) {
			if((m_selected_block % TESTS_PER_COLUMN) != 0)
				m_selected_block --;
			else {
				m_selected_block = (m_selected_block / TESTS_PER_COLUMN + 1)*TESTS_PER_COLUMN - 1;
				if(m_selected_block >= m_blocks)
					m_selected_block = m_blocks - 1;
			}
		}
		if(ctrl & 0x02) {
			if((m_selected_block % TESTS_PER_COLUMN) != (TESTS_PER_COLUMN-1)) {
				m_selected_block ++;
				if(m_selected_block == m_blocks)
					m_selected_block = (m_blocks-1)/TESTS_PER_COLUMN*TESTS_PER_COLUMN;
			} else
				m_selected_block = m_selected_block/TESTS_PER_COLUMN*TESTS_PER_COLUMN;
		}
		
		if(ctrl & 0x04) {
			if((m_selected_block - TESTS_PER_COLUMN) >= 0)
				m_selected_block -= TESTS_PER_COLUMN;
			else {
				while(m_selected_block + TESTS_PER_COLUMN < m_blocks)
					m_selected_block += TESTS_PER_COLUMN;
			}
		}
		if(ctrl & 0x08) {
			if((m_selected_block + TESTS_PER_COLUMN) < m_blocks)
				m_selected_block += TESTS_PER_COLUMN;
			else
				m_selected_block %= TESTS_PER_COLUMN;
		}
		if(ctrl & 0x10) {
			m_mode = MODE_SINGLE;
			m_cur_block = m_selected_block;
			const testblock &tb = testblocks[m_selected_block];
			m_cur_test = 0;
			for(u32 i = tb.m_testlist_start; i != tb.m_testlist_end; i++)
				if(m_test_success[i] == 1) {
					m_cur_test = i - tb.m_testlist_start;
					break;
				}
			draw_single_test();

		} else if(ctrl)
			draw_main_test_list();

	} else {
		if(ctrl & 0x04) {
			if(m_cur_test != 0)
				m_cur_test --;
			else
				m_cur_test = test_count(m_cur_block)-1;
		}
		if(ctrl & 0x08) {
			m_cur_test ++;
			if(m_cur_test == test_count(m_cur_block))
				m_cur_test = 0;
		}
		if(ctrl & 0x10)
			trigger_test(true);
		if(ctrl & 0x20) {
			m_mode = MODE_ALL;
			draw_main_test_list();
		}
		if(m_mode == MODE_SINGLE && (ctrl & 0x1c))
			draw_single_test();
	}
}

void utdsp563xx_state::trigger_test(bool stop)
{
	const testblock &tb = testblocks[m_cur_block];
	const testlist &tl = testlists[tb.m_testlist_start + m_cur_test];
	m_code_start = tl.m_code_start;
	m_code_end = tl.m_code_end;
	m_pre_start = tl.m_pre_start;
	m_pre_end = tl.m_pre_end;
	m_post_start = tl.m_post_start;
	m_post_end = tl.m_post_end;
	m_p_data.clear();
	m_x_data.clear();
	m_y_data.clear();
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	if(stop)
		machine().debug_break();
}

void utdsp563xx_state::utdsp563xx(machine_config &config)
{
    DSP56311(config, m_cpu, 66_MHz_XTAL);
	m_cpu->set_hard_omr(0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_native_aspect();
	screen.set_refresh_hz(60);
	screen.set_vblank_time(0);
	screen.set_size(1920, 1080);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(utdsp563xx_state::screen_update));
	screen.screen_vblank().set(FUNC(utdsp563xx_state::tick));

	SPEAKER(config, "speaker", 2).front();
}

#include "dsp563xx.ipp"

static INPUT_PORTS_START( utdsp563xx  )
	PORT_START("CONTROL")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)
INPUT_PORTS_END

ROM_START( utdsp563xx )
	ROM_REGION16_BE(32*2*256, "font", 0)
	// Characters 0-255 of the spleen 16x32 font, BSD-2
	// See https://github.com/fcambus/spleen/
	ROM_LOAD("spleen_16_32_256.bin", 0, 32*2*256, CRC(d95ea771) SHA1(4aacc4e088165bc4a83bfca3bbb94507efb5d617))
ROM_END

SYST( 2025, utdsp563xx, 0, 0, utdsp563xx, utdsp563xx, utdsp563xx_state, empty_init, "MAME", "DSP563xx unit tests", MACHINE_SUPPORTS_SAVE )
