// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista

/*
Sanma - San-nin Uchi Mahjong (Japan) by ANES
Ton Puu Mahjong (Japan) by ANES

TODO:
- ROM banking;
- blitter, 8bpp with hardcoded palette & writes to ROM area!?
- inputs;
- dip sheets are available for sanma.

- 1x Z0840008PSC Z80 CPU
- 1x 16.000 XTAL near the Z80
- 1x YM2413 sound chip
- 1x 3.579545 XTAL near the YM2413
- 1x Xilinx XC7354 CPLD
- 1x 17128EPC configuration bitstream for the Xilinx
- 2x ISSI IS61C64AH 8k x8 SRAM
- 1x HM6265LK-70
- 1x unknown 160 pin device labeled "ANES ORIGINAL SEAL NO. A199." for tonpuu, "ANES ORIGINAL SEAL NO. A446." for sanma
- 4x bank of 8 dip-switches
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
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
		m_palette(*this, "palette"),
		m_banktimer(*this, "banktimer"),
		m_rombank(*this, "rombank"),
		m_blitrom(*this, "blitter"),
		m_coin(*this, "COIN"),
		m_key{ { *this, "KEY1.%u", 0U }, { *this, "KEY2.%u", 0U } }
	{
	}

	void anes(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;


private:
	void vram_offset_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_banktimer;

	required_memory_bank m_rombank;
	required_region_ptr<uint8_t> m_blitrom;

	required_ioport m_coin;
	required_ioport_array<5> m_key[2];

	uint8_t m_mux;
	uint8_t m_bank;
	uint8_t m_bank_delay;

	bitmap_ind16 m_bitmap[2][2];
	uint8_t m_blit[16];
	uint16_t m_blit_addr[2];
	uint8_t m_blit_val[2];

	uint8_t m_palette_enable;
	uint8_t m_palette_offset;
	uint8_t m_palette_active_bank;

	uint8_t m_palette_offset_msb;
	uint8_t m_palette_data2[0x2000];
	uint8_t m_palette_data3[0x2000];

	void do_blit();
	void blit_w(offs_t offset, uint8_t data);
	void blit_rom_w(offs_t offset, uint8_t data);
	uint8_t blit_status_r();
	void rombank_w(uint8_t data);
	void mux_w(uint8_t data);
	uint8_t key_r(offs_t offset);
	uint8_t m1_rom_r(offs_t offset);

	void palette_enable_w(uint8_t data);
	void palette_offset_w(uint8_t data);
	void palette_offset_msb_w(uint8_t data);
	void palette_active_bank_w(uint8_t data);
	void palette_data2_w(uint8_t data);
	void palette_data3_w(uint8_t data);
	void set_color(int offset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void prg_map(address_map &map);
	void opcodes_map(address_map &map);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(bank_timer_expired);
};

uint32_t anes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 *srcbitmap = &m_bitmap[0][0];

	for (int y = cliprect.min_y; y < cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			uint16_t* src = &srcbitmap->pix(y, x);
			uint16_t* dst = &bitmap.pix(y, x);

			dst[0] = src[0] + m_palette_active_bank * 0x100;
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

void anes_state::do_blit()
{
	//	int m4      =   m_blit[0x04];
	//	int m0      =   m_blit[0x00];
	int which = m_blit[0x0b];
	int src = (m_blit[0x0c] << 16) + (m_blit[0x0d] << 8) + m_blit[0x0e];

	//	logerror("%s: blit s %02x, y %02x, src %06x, addr1 %04X %02X, addr2 %04X %02X, which %02X\n", machine().describe_context(),
	//			m4, m0, src, m_blit_addr[0], m_blit_val[0], m_blit_addr[1], m_blit_val[1], which );

	int layer = 0;
	int buffer = 0;
	bitmap_ind16& bitmap = m_bitmap[layer][buffer];

	int sx = m_blit_addr[0];
	int sy = m_blit_val[0];

	int16_t sw = m_blit_addr[1] - m_blit_addr[0] + 1;
	int16_t sh = m_blit_val[1] - m_blit_val[0] + 1;

	int addr = src;
	for (int y = 0; y < sh; y++)
	{
		for (int x = 0; x < sw; x++)
		{
			int pen = (which & 0x10) ? 0 : m_blitrom[(addr++) & (m_blitrom.bytes() - 1)];

			//          if (pen != 0xff)
			bitmap.pix(((sy + y) & 0x1ff), ((sx + x) & 0x1ff)) = pen;
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

uint8_t anes_state::blit_status_r()
{
	// bits 0-3 = ?
	// bit    4 = busy
	return 0x00;
}

void anes_state::mux_w(uint8_t data)
{
	m_mux = data;
	if (m_mux & (~0x1f))
		logerror("%s: Unknown mux bit written %02X\n", machine().describe_context(), data);
}

uint8_t anes_state::key_r(offs_t offset)
{
	for (int i = 0; i < 5; i++)
		if (!BIT(m_mux, i))
			return m_key[offset][i]->read();

	//  logerror("%s: Unknown key read with mux = %02X\n", machine().describe_context(), m_mux);
	//  return 0xff;
	return offset ? m_coin->read() : 0xff;
}


uint8_t anes_state::m1_rom_r(offs_t offset)
{
	uint8_t ret = m_maincpu->space(AS_PROGRAM).read_byte(offset);
	if (!machine().side_effects_disabled())
	{
		if (m_bank_delay)
		{
			m_banktimer->adjust(attotime::from_usec(1));
		}
	}
	return ret;
}

void anes_state::palette_enable_w(uint8_t data)
{
	m_palette_enable = data;
}

void anes_state::set_color(int offset)
{
	uint16_t dat = (m_palette_data3[offset] << 8) | m_palette_data2[offset];
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
		logerror("%s: write to palette_offset_msb_w when not enabled %02x (enable %02x)\n", machine().describe_context(), data, m_palette_enable);
	}
	else
	{
		m_palette_offset_msb = data;
	}
}

void anes_state::palette_active_bank_w(uint8_t data)
{
	m_palette_active_bank = data & 0xf;
}

void anes_state::palette_data2_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		logerror("write to palette_data2_w when not enabled %02x\n", data);
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		m_palette_data2[offs] = data;
		set_color(offs);
	}
}

void anes_state::palette_data3_w(uint8_t data)
{
	if (m_palette_enable != 0x01)
	{
		logerror("write to palette_data3_w when not enabled %02x\n", data);
	}
	else
	{
		int offs = (m_palette_offset + (m_palette_offset_msb << 8)) & 0x1fff;
		m_palette_data3[offs] = data;
		set_color(offs);
	}
}


void anes_state::opcodes_map(address_map &map)
{
	map(0x0000, 0xefff).r(FUNC(anes_state::m1_rom_r));
}

void anes_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xefff).bankr(m_rombank);
	map(0x0000, 0xefff).w(FUNC(anes_state::blit_rom_w));
	map(0xf000, 0xf7ff).ram();
	map(0xf900, 0xffff).ram();
}

void anes_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x0f).w(FUNC(anes_state::blit_w));

	map(0x07, 0x07).w(FUNC(anes_state::mux_w));
	map(0x08, 0x09).w("ym", FUNC(ym2413_device::write));

	map(0x10, 0x10).portr("SW1");
	map(0x11, 0x11).portr("SW2");
	map(0x12, 0x12).portr("SW3");
	map(0x13, 0x13).portr("SW4");
	map(0x14, 0x15).r(FUNC(anes_state::key_r));
	map(0x16, 0x16).r(FUNC(anes_state::blit_status_r));
	map(0x50, 0x50).w(FUNC(anes_state::palette_enable_w));
	map(0x51, 0x51).w(FUNC(anes_state::palette_offset_w));
	map(0x52, 0x52).w(FUNC(anes_state::palette_offset_msb_w));
	map(0x53, 0x53).w(FUNC(anes_state::palette_active_bank_w));
	map(0x54, 0x54).w(FUNC(anes_state::palette_data2_w));
	map(0x55, 0x55).w(FUNC(anes_state::palette_data3_w));
	map(0xfe, 0xfe).w(FUNC(anes_state::rombank_w));
}


static INPUT_PORTS_START( anes )
	PORT_START("COIN")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )

	PORT_START("KEY1.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_START("KEY1.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_START("KEY1.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_START("KEY1.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_START("KEY1.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("KEY2.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_START("KEY2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_START("KEY2.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_START("KEY2.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_START("KEY2.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )


	PORT_START("SW1")
	PORT_START("SW2")

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
	PORT_DIPNAME( 0x40, 0x40, "Use Rand" )               PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Twice" )               PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW4") // port 0x13
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void anes_state::machine_start()
{
	uint8_t* rom = memregion("maincpu")->base();
	for (int i = 0; i < (memregion("maincpu")->bytes() - 0x2000) / 0xc000; i++)
		m_rombank->configure_entry(i, &rom[0x2000 + (0xc000 * i)]);

	m_mux = 0;
	m_bank = 0;
	m_bank_delay = 0;
	m_palette_active_bank = 0;

	save_item(NAME(m_mux));
	save_item(NAME(m_bank));
	save_item(NAME(m_bank_delay));
}

void anes_state::video_start()
{
	for (int layer = 0; layer < 2; layer++)
	{
		for (int buffer = 0; buffer < 2; buffer++)
		{
			m_bitmap[layer][buffer].allocate(512, 512);
			m_bitmap[layer][buffer].fill(0);
		}
	}

	std::fill(std::begin(m_blit), std::end(m_blit), 0);
	std::fill(std::begin(m_blit_addr), std::end(m_blit_addr), 0);
	std::fill(std::begin(m_blit_val), std::end(m_blit_val), 0);

	std::fill(std::begin(m_palette_data2), std::end(m_palette_data2), 0);
	std::fill(std::begin(m_palette_data3), std::end(m_palette_data3), 0);	

	save_item(NAME(m_blit));
	save_item(NAME(m_blit_addr));
	save_item(NAME(m_blit_val));

	save_item(NAME(m_palette_offset));
	save_item(NAME(m_palette_offset_msb));
	save_item(NAME(m_palette_data2));
	save_item(NAME(m_palette_data3));
	save_item(NAME(m_palette_active_bank));
}


INTERRUPT_GEN_MEMBER(anes_state::interrupt)
{
	if (!m_bank_delay)
		device.execute().set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(anes_state::bank_timer_expired)
{
	m_rombank->set_entry(m_bank);
	m_bank_delay = 0;
}

void anes_state::anes(machine_config &config)
{
	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", XTAL(16'000'000) / 2)); // Z0840008PSC
	maincpu.set_addrmap(AS_PROGRAM, &anes_state::prg_map);
	maincpu.set_addrmap(AS_OPCODES, &anes_state::opcodes_map);
	maincpu.set_addrmap(AS_IO, &anes_state::io_map);
	maincpu.set_vblank_int("screen", FUNC(anes_state::interrupt));

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 46*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(anes_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x2000);

	TIMER(config, m_banktimer).configure_generic(FUNC(anes_state::bank_timer_expired));

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


GAME( 2001, sanma,  0, anes, anes, anes_state, empty_init, ROT0, "ANES", "Sanma - San-nin Uchi Mahjong [BET] (Japan, version 2.60)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // flyer says 2000, manual says 2001 version 2.60
GAME( 200?, tonpuu, 0, anes, anes, anes_state, empty_init, ROT0, "ANES", "Ton Puu Mahjong [BET] (Japan)",                            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
