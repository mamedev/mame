// license:BSD-3-Clause
// copyright-holders: Luca Elia, David Haywood

/*
Sanma - San-nin Uchi Mahjong (Japan) by ANES
Ton Puu Mahjong (Japan) by ANES

TODO:
- refine blitter emulation;
- flip screen;
- player 2 inputs;
- verify sanma dips translations;

- 1x Z0840008PSC Z80 CPU
- 1x 16.000 XTAL near the Z80
- 1x YM2413 sound chip
- 1x 3.579545 XTAL near the YM2413
- 1x Xilinx XC7354 CPLD
- 1x 17128EPC configuration bitstream for the Xilinx
- 2x ISSI IS61C64AH 8k x8 SRAM
- 1x HM6265LK-70
- 1x unknown 160 pin device labeled "ANES ORIGINAL SEAL NO. A199." for tonpuu, "ANES ORIGINAL SEAL NO. A446." for sanma
- 4x banks of 8 DIP switches
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class anes_state : public driver_device
{
public:
	anes_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank"),
		m_blitrom(*this, "blitter"),
		m_key(*this, "KEY1.%u", 0U)
	{
	}

	void anes(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void vram_offset_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_rombank;
	required_region_ptr<uint8_t> m_blitrom;

	required_ioport_array<5> m_key;

	uint8_t m_inp_matrix;
	uint8_t m_bank;
	uint8_t m_bank_delay;

	bitmap_ind16 m_bitmap[2];
	uint8_t m_blit[16];
	uint16_t m_blit_addr[2];
	uint8_t m_blit_val[2];

	uint8_t m_palette_enable;
	uint8_t m_palette_offset;
	uint8_t m_palette_offset_msb;
	uint8_t m_palette_active_bank;
	uint8_t m_palette_active_bank_alt;
	uint8_t m_palette_data_lsb[0x2000];
	uint8_t m_palette_data_msb[0x2000];

	void do_blit_draw(bitmap_ind16 &bitmap, int &addr, int sx, int sy, int sw, int sh, int x, int y, bool flipx, bool flipy);
	void do_blit();
	void blit_w(offs_t offset, uint8_t data);
	void blit_rom_w(offs_t offset, uint8_t data);
	void blit_unk_w(uint8_t data);

	void rombank_w(uint8_t data);
	void matrix_w(uint8_t data);
	uint8_t key_r(offs_t offset);
	uint8_t m1_rom_r(offs_t offset);

	void palette_enable_w(uint8_t data);
	void palette_offset_w(uint8_t data);
	void palette_offset_msb_w(uint8_t data);
	void palette_active_bank_w(uint8_t data);
	uint8_t palette_data_lsb_r();
	uint8_t palette_data_msb_r();
	void palette_data_lsb_w(uint8_t data);
	void palette_data_msb_w(uint8_t data);
	void set_color(int offset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(interrupt);
};

uint32_t anes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const src = &m_bitmap[0].pix(y);
		uint16_t const *const src2 = &m_bitmap[1].pix(y);
		uint16_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (src2[x])
				dst[x] = src2[x] + (m_palette_active_bank * 0x100);
			else
				dst[x] = src[x] + (m_palette_active_bank_alt * 0x100);
		}
	}

	return 0;
}

/*
 blitter operation is:
 checks for bit 4 to be high in port $16
 writes either 0 or 3 to port $04
 writes a 0 to port $00
 writes an offset to ports $0c / $0d / $0e
 writes 1 to port $0b, writes to program space, writes 2 to port $0b, writes to program space
 writes a mode to port $0b, writes to trigger port $0a
 */

void anes_state::do_blit_draw(bitmap_ind16 &bitmap, int &addr, int sx, int sy, int sw, int sh, int x, int y, bool flipx, bool flipy)
{
	int drawx, drawy;

	if (!flipy)
		drawy = ((sy + y) & 0x1ff);
	else
		drawy = ((sy + (sh - 1) - y) & 0x1ff);

	if (!flipx)
		drawx = ((sx + x) & 0x1ff);
	else
		drawx = ((sx + (sw - 1) - x) & 0x1ff);

	int pen = m_blitrom[(addr) & (m_blitrom.bytes() - 1)];
	if (!(m_blit[0x0b] & 0x10))
		addr++;

	if (pen != 0xff)
	{
		//  if (pen != 0x00)
		bitmap.pix(drawy, drawx) = pen;
	}
	else
	{
		bitmap.pix(drawy, drawx) = 0x00;
	}
}

void anes_state::do_blit()
{
	int src = (m_blit[0x0c] << 16) + (m_blit[0x0d] << 8) + m_blit[0x0e];

	logerror("%s: src %06x, xpos %04X width %02X, ypos %04X height %02X | %02X %02X %02X %02X %02X %02X %02X | %02X %02X | %02X\n",
			machine().describe_context(),
			src, m_blit_addr[0], m_blit_val[0], m_blit_addr[1], m_blit_val[1],
			m_blit[0x00], m_blit[0x01], m_blit[0x02], m_blit[0x03], m_blit[0x04], m_blit[0x05], m_blit[0x06],
			m_blit[0x0a], m_blit[0x0b],
			m_blit[0x0f]);

	bool flipx = m_blit[0x04] & 0x01;
	bool flipy = m_blit[0x04] & 0x02;

	bitmap_ind16 &bitmap = m_bitmap[(m_blit[0x0b] & 0x04) ? 1 : 0];

	int sx = m_blit_addr[0];
	int sy = m_blit_val[0];
	int16_t sw = m_blit_addr[1] - m_blit_addr[0] + 1;
	int16_t sh = m_blit_val[1] - m_blit_val[0] + 1;

	if (m_blit[0x0b] & 0x40) // flipx x/y scan order on output?
	{
		for (int x = 0; x < sw; x++)
		{
			for (int y = 0; y < sh; y++)
			{
				do_blit_draw(bitmap, src, sx, sy, sw, sh, x, y, flipx, flipy);
			}
		}
	}
	else
	{
		for (int y = 0; y < sh; y++)
		{
			for (int x = 0; x < sw; x++)
			{
				do_blit_draw(bitmap, src, sx, sy, sw, sh, x, y, flipx, flipy);
			}
		}
	}
}

void anes_state::rombank_w(uint8_t data)
{
	if (data & (~0x07))
		logerror("%s: Unknown ROM bank bit written %02X\n", machine().describe_context(), data);

	m_bank = data & 0x07;

	// the bank change must happen AFTER the next instruction, be it a jmp or a ret
	m_bank_delay = 2;
}

void anes_state::blit_w(offs_t offset, uint8_t data)
{
	m_blit[offset] = data;
	if (offset == 0x0a)
		do_blit();
}

void anes_state::blit_rom_w(offs_t offset, uint8_t data)
{
	uint8_t which = m_blit[0x0b];
//  logerror("%s: blit ROM written %02X (%04X <- %02X)\n", machine().describe_context(), which, offset, data);
	if (which == 1)
	{
		m_blit_addr[0] = offset;
		m_blit_val[0] = data;
	}
	else if (which == 2)
	{
		m_blit_addr[1] = offset;
		m_blit_val[1] = data;
	}
	else
		logerror("%s: Unknown blit ROM which %02X\n", machine().describe_context(), which);
}


void anes_state::matrix_w(uint8_t data)
{
	m_inp_matrix = data;
	if (m_inp_matrix & (~0x1f))
		logerror("%s: Unknown mux bit written %02X\n", machine().describe_context(), data);
}

uint8_t anes_state::key_r(offs_t offset)
{
	uint8_t data = 0xff;

	// read key matrix
	for (int i = 0; i < 5; i++)
		if (!BIT(m_inp_matrix, i))
			data &= m_key[i]->read();

	return data;
}

uint8_t anes_state::m1_rom_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_bank_delay)
	{
		if (--m_bank_delay == 0)
			m_rombank->set_entry(m_bank);
	}

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void anes_state::palette_enable_w(uint8_t data)
{
	m_palette_enable = data;
}

void anes_state::set_color(int offset)
{
	uint16_t dat = (m_palette_data_msb[offset] << 8) | m_palette_data_lsb[offset];
	m_palette->set_pen_color(offset, rgb_t(pal5bit(dat >> 0), pal5bit(dat >> 5), pal5bit(dat >> 10)));
}

void anes_state::palette_offset_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		logerror("write to palette_offset_w when not enabled %02x (enable %02x)\n", data, m_palette_enable);
	}
	else
	{
		m_palette_offset = data;
	}
}

void anes_state::palette_offset_msb_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		//logerror("%s: write to palette_offset_msb_w when not enabled %02x (enable %02x)\n", machine().describe_context(), data, m_palette_enable);
		m_palette_active_bank_alt = data;
	}
	else
	{
		m_palette_offset_msb = data;
	}
}

void anes_state::palette_active_bank_w(uint8_t data)
{
	m_palette_active_bank = data & 0x1f;
}

uint8_t anes_state::palette_data_lsb_r()
{
	if (m_palette_enable != 0x01)
	{
		logerror("read from palette_data_lsb_r when not enabled\n");
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		return m_palette_data_lsb[offs];
	}
	return 0x00;
}

uint8_t anes_state::palette_data_msb_r()
{
	if (m_palette_enable != 0x01)
	{
		logerror("read from palette_data_msb_r when not enabled\n");
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		return m_palette_data_msb[offs];
	}
	return 0x00;
}



void anes_state::palette_data_lsb_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		logerror("write to palette_data_lsb_w when not enabled %02x\n", data);
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		m_palette_data_lsb[offs] = data;
		set_color(offs);
	}
}

void anes_state::palette_data_msb_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		logerror("write to palette_data_msb_w when not enabled %02x\n", data);
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		m_palette_data_msb[offs] = data;
		set_color(offs);
	}
}

void anes_state::blit_unk_w(uint8_t data)
{
	if (data != 0x1f)
		logerror("write to blit_unk_w %02x\n", data);
}

void anes_state::opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(anes_state::m1_rom_r));
}

void anes_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xefff).bankr(m_rombank);
	map(0x0000, 0x01ff).w(FUNC(anes_state::blit_rom_w));
	map(0x1000, 0x11ff).w(FUNC(anes_state::blit_rom_w)); // does writing with 0x1000 set on the address have a different meaning?
	map(0xf000, 0xffff).ram().share("nvram"); // there might be a hole at 0xf800 - 0xf8ff, battery backed RAM may be less
}

void anes_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x0f).w(FUNC(anes_state::blit_w));

	map(0x07, 0x07).w(FUNC(anes_state::matrix_w));
	map(0x08, 0x09).w("ym", FUNC(ym2413_device::write));

	map(0x10, 0x10).portr("SW1");
	map(0x11, 0x11).portr("SW2");
	map(0x12, 0x12).portr("SW3");
	map(0x13, 0x13).portr("SW4");
	map(0x14, 0x14).r(FUNC(anes_state::key_r));
	map(0x15, 0x15).portr("COIN");
	map(0x16, 0x16).portr("SYSTEM").w(FUNC(anes_state::blit_unk_w));
	map(0x50, 0x50).w(FUNC(anes_state::palette_enable_w));
	map(0x51, 0x51).w(FUNC(anes_state::palette_offset_w));
	map(0x52, 0x52).w(FUNC(anes_state::palette_offset_msb_w));
	map(0x53, 0x53).w(FUNC(anes_state::palette_active_bank_w));
	map(0x54, 0x54).rw(FUNC(anes_state::palette_data_lsb_r), FUNC(anes_state::palette_data_lsb_w));
	map(0x55, 0x55).rw(FUNC(anes_state::palette_data_msb_r), FUNC(anes_state::palette_data_msb_w));
	map(0xfe, 0xfe).w(FUNC(anes_state::rombank_w));
}


static INPUT_PORTS_START( anes )
	PORT_START("COIN")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) // test
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) // analyzer
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) // reset
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) // note
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // the following ones have no effect in test mode
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEY1.0")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("SW3") // port 0x12
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Bonus Chance" )          PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(    0x38, "10" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x28, "2" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x30, "1/rand<2" )  // 2
	PORT_DIPSETTING(    0x10, "1/rand<3" )  // 3
	PORT_DIPSETTING(    0x20, "1/rand<5" )  // 5
	PORT_DIPSETTING(    0x00, "1/rand<10" ) // 10
	PORT_DIPNAME( 0x40, 0x40, "Use Rand" )             PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Twice" )                PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW4") // port 0x13
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( tonpuu )
	PORT_INCLUDE( anes )

	// dips and defaults not verified from manual
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x0f, 0x09, "Main Rate" )      PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "50%" )
	PORT_DIPSETTING(    0x07, "56%" )
	PORT_DIPSETTING(    0x0b, "59%" )
	PORT_DIPSETTING(    0x03, "62%" )
	PORT_DIPSETTING(    0x0d, "65%" )
	PORT_DIPSETTING(    0x05, "68%" )
	PORT_DIPSETTING(    0x09, "70%" )
	PORT_DIPSETTING(    0x01, "72%" )
	PORT_DIPSETTING(    0x0e, "74%" )
	PORT_DIPSETTING(    0x06, "76%" )
	PORT_DIPSETTING(    0x0a, "78%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x0c, "85%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x08, "95%" )
	PORT_DIPSETTING(    0x00, "99%" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus Rate" )    PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "5%" )
	PORT_DIPSETTING(    0x10, "10%" )
	PORT_DIPSETTING(    0x20, "15%" )
	PORT_DIPSETTING(    0x00, "20%" )
	PORT_DIPNAME( 0xc0, 0x00, "Max Bet" )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_MODIFY("SW4") // port 0x13
	PORT_DIPNAME( 0x04, 0x04, "Background Style" )    PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, "Textured" )
	PORT_DIPSETTING(    0x00, "Flat")
INPUT_PORTS_END

static INPUT_PORTS_START( sanma )
	PORT_INCLUDE( anes )

	// dips and defaults from manual version 2.60
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x07, 0x06, "Main Rate" )      PORT_DIPLOCATION("SW1:1,2,3") // spelt 'Mein Rate' in the manual
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x05, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x02, "75%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x00, "Bonus Rate" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "20%" )
	PORT_DIPSETTING(    0x00, "30%" )
	PORT_DIPNAME( 0x10, 0x10, "Tile Arrangement" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Bad" ) // TODO: check this translation
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Customer Service" )    PORT_DIPLOCATION("SW1:7,8") // TODO: check this translation
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x80, "Small" )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0xc0, "Big" )

	PORT_MODIFY("SW2")
	PORT_DIPNAME( 0x03, 0x01, "Odds Rate" )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1,2,3,5,8,12,24,36 / 2,3,5,8,12,18,36,58" )
	PORT_DIPSETTING(    0x01, "1,2,3,5,8,12,24,60 / 2,3,5,8,12,18,36,100" )
	PORT_DIPSETTING(    0x02, "1,2,3,5,8,15,30,50" )
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPNAME( 0x04, 0x00, "Max Bet" )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10")
	PORT_DIPNAME( 0x08, 0x08, "Minimum Bet" )    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPNAME( 0x30, 0x00, "Coin Limit" )     PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "100" )
	PORT_DIPSETTING(    0x10, "300" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x00, "9999" )
	PORT_DIPNAME( 0xc0, 0x00, "Credit Limit" )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0x00, "9999" )

	PORT_MODIFY("SW3") // port 0x12
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Notes" )          PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(    0x00, "10 Notes/1 Credit" )
	PORT_DIPSETTING(    0x20, "5 Notes/1 Credit" )
	PORT_DIPSETTING(    0x10, "3 Notes/1 Credit" )
	PORT_DIPSETTING(    0x30, "2 Notes/1 Credit" )
	PORT_DIPSETTING(    0x08, "1 Note/1 Credit" )
	PORT_DIPSETTING(    0x28, "1 Note/2 Credits" )
	PORT_DIPSETTING(    0x18, "1 Note/5 Credits" )
	PORT_DIPSETTING(    0x38, "1 Note/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )             PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, "Coin" )
	PORT_DIPSETTING(    0x00, "Note" )
	PORT_DIPNAME( 0x80, 0x80, "Pay Out Speed" )                PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_MODIFY("SW4") // port 0x13
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )                PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "8 Renso Initial Setting" )                PORT_DIPLOCATION("SW4:2") // TODO: check this translation
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )                PORT_DIPLOCATION("SW4:3") // TODO: needs translation
	PORT_DIPSETTING(    0x04, "Every 2 Bets" )
	PORT_DIPSETTING(    0x00, "Every 3 Bets" )
	PORT_DIPNAME( 0x08, 0x08, "Credit Timer" )       PORT_DIPLOCATION("SW4:4") // TODO: check this translation
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" ) // last 4 dips are unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END


void anes_state::machine_start()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0; i < (memregion("maincpu")->bytes() - 0x2000) / 0xc000; i++)
		m_rombank->configure_entry(i, &rom[0x2000 + (0xc000 * i)]);

	m_inp_matrix = 0;
	m_bank = 0;
	m_bank_delay = 0;

	save_item(NAME(m_inp_matrix));
	save_item(NAME(m_bank));
	save_item(NAME(m_bank_delay));
}

void anes_state::video_start()
{
	for (int layer = 0; layer < 2; layer++)
	{
		m_bitmap[layer].allocate(512, 512);
		m_bitmap[layer].fill(0);
		m_screen->register_screen_bitmap(m_bitmap[layer]);
	}

	save_item(NAME(m_bitmap[0]));
	save_item(NAME(m_bitmap[1]));

	std::fill(std::begin(m_blit), std::end(m_blit), 0);
	std::fill(std::begin(m_blit_addr), std::end(m_blit_addr), 0);
	std::fill(std::begin(m_blit_val), std::end(m_blit_val), 0);

	save_item(NAME(m_blit));
	save_item(NAME(m_blit_addr));
	save_item(NAME(m_blit_val));

	m_palette_enable = 0;
	m_palette_offset = 0;
	m_palette_offset_msb = 0;
	m_palette_active_bank = 0;
	m_palette_active_bank_alt = 0;

	std::fill(std::begin(m_palette_data_lsb), std::end(m_palette_data_lsb), 0);
	std::fill(std::begin(m_palette_data_msb), std::end(m_palette_data_msb), 0);

	save_item(NAME(m_palette_enable));
	save_item(NAME(m_palette_offset));
	save_item(NAME(m_palette_offset_msb));
	save_item(NAME(m_palette_active_bank));
	save_item(NAME(m_palette_active_bank_alt));
	save_item(NAME(m_palette_data_lsb));
	save_item(NAME(m_palette_data_msb));
}


INTERRUPT_GEN_MEMBER(anes_state::interrupt)
{
	if (!m_bank_delay)
		device.execute().set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

void anes_state::anes(machine_config &config)
{
	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", XTAL(16'000'000) / 2)); // Z0840008PSC
	maincpu.set_addrmap(AS_PROGRAM, &anes_state::prg_map);
	maincpu.set_addrmap(AS_OPCODES, &anes_state::opcodes_map);
	maincpu.set_addrmap(AS_IO, &anes_state::io_map);
	maincpu.set_vblank_int("screen", FUNC(anes_state::interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO: set_raw
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 46*8-1, 0*8, 31*8-1);
	m_screen->set_screen_update(FUNC(anes_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x2000);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ym", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( sanma ) // exact ROM type unknown, dumped multiple times as various sized chips and 27C040 seems correct. Program ROM might actually be 27C020.
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "anes_s26.u32", 0x00000, 0x80000, CRC(2c14ed16) SHA1(8ce0ddee9501896f76dab63d57326812ba4a9a6c) ) // 27C040?, 1ST AND 2ND HALF IDENTICAL

	ROM_REGION(0x200000, "blitter", 0)
	ROM_LOAD( "anes_301.u33", 0x000000, 0x80000, CRC(d2ba943d) SHA1(9aea0bda5186c0a746bf86eeaa2bd7910d3d5e03) ) // 27C040?
	ROM_LOAD( "anes_302.u34", 0x080000, 0x80000, CRC(92cac418) SHA1(3f17a7a95cc7d9ca3e1eb638276c40c8dab9f12d) ) // 27C040?
	ROM_LOAD( "anes_333.u36", 0x100000, 0x80000, CRC(f46db452) SHA1(b545b071a72323009110aecfda6c434468851a63) ) // 27C040?
	ROM_LOAD( "anes_324.u35", 0x180000, 0x80000, CRC(7f6a7af5) SHA1(c8657dc3053d3e125c9e92ade7467ffa31e48a34) ) // 27C040?

	ROM_REGION(0x4001, "fpga_bitstream", 0)
	ROM_LOAD( "17128epc.u41", 0x0000, 0x4001, CRC(95e38a6c) SHA1(ba2f563f6aa6de7d6439f73ccc6d82346e453e22) )
ROM_END

ROM_START( tonpuu )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "201.u32", 0x00000, 0x20000, CRC(ace857bb) SHA1(3f65976883c0c514abf73eeed9223ca52a2be410) ) // 27C010

	ROM_REGION(0x100000, "blitter", 0)
	ROM_LOAD( "202.u33", 0x00000, 0x80000, CRC(4d62a358) SHA1(6edff8e031272cd5a466d9767454093870a0f90a) ) // 27C4001
	ROM_LOAD( "203.u34", 0x80000, 0x80000, CRC(a6068528) SHA1(c988bd1fc2f91befa9d0d39995ba98ef86b5d854) ) // 27C4001

	ROM_REGION(0x4001, "fpga_bitstream", 0)
	ROM_LOAD( "17128epc.u41", 0x0000, 0x4001, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2001, sanma,  0, anes, sanma,  anes_state, empty_init, ROT0, "ANES", "Sanma - San-nin Uchi Mahjong (Japan, version 2.60)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // flyer says 2000, manual says 2001 version 2.60
GAME( 1997, tonpuu, 0, anes, tonpuu, anes_state, empty_init, ROT0, "ANES", "Ton Puu Mahjong Version 2.0 RX (Japan)",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE  )
