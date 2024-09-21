// license:BSD-3-Clause
// copyright-holders:David Haywood, XingXing

// default bookkeeping passwords
// fearless 1234
// superkds (unknown)

#include "emu.h"

#include "igs027a.h"
#include "pgmcrypt.h"
#include "xamcu.h"

#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/v3021.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define LOG_DEBUG       (1U << 1)
//#define VERBOSE         (LOG_DEBUG)
#include "logmacro.h"


namespace {

class igs_fear_state : public driver_device
{
public:
	igs_fear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_gfxrom(*this, "gfx1"),
		m_sram(*this, "sram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_xa(*this, "xa"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ticket(*this, "ticket"),
		m_io_dsw(*this, "DSW%u", 1U),
		m_io_trackball(*this, "AN%u", 0)
	{ }

	void igs_fear(machine_config &config) ATTR_COLD;
	void igs_fear_xor(machine_config &config) ATTR_COLD;

	void init_igs_fear() ATTR_COLD;
	void init_igs_icescape() ATTR_COLD;
	void init_igs_superkds() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_xor_map(address_map &map) ATTR_COLD;

	void vblank_irq(int state);

	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int flipx, int romoffset);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u32 external_rom_r(offs_t offset);

	void xor_table_w(offs_t offset, u8 data);

	u32 igs027_gpio_r();
	void igs027_gpio_w(u8 data);

	u32 xa_r(offs_t offset, u32 mem_mask);
	void cpld_w(offs_t offset, u32 data, u32 mem_mask);

	required_region_ptr<u32> m_external_rom;
	required_region_ptr<u8> m_gfxrom;
	required_shared_ptr<uint32_t> m_sram;
	required_shared_ptr<u32> m_videoram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<igs_xa_mcu_ics_sound_device> m_xa;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_device<ticket_dispenser_device> m_ticket;

	required_ioport_array<2> m_io_dsw;
	optional_ioport_array<2> m_io_trackball;

	u32 m_xor_table[0x100];

	u8 m_gpio_o;

	int m_trackball_cnt;
	int m_trackball_axis[2], m_trackball_axis_pre[2], m_trackball_axis_diff[2];
};


void igs_fear_state::video_start()
{
}

void igs_fear_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_xor_table));

	save_item(NAME(m_gpio_o));
}

void igs_fear_state::machine_reset()
{
}

void igs_fear_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int flipx, int romoffset)
{
	if ((romoffset != 0) && (romoffset != 0xffffffff))
	{
		//LOGMASKED(LOG_DEBUG, "x=%d, y=%d, w=%d pix, h=%d pix, c=0x%02x, romoffset=0x%08x\n", xpos, ypos, width, height, palette, romoffset << 2);
		const u8 *gfxrom = &m_gfxrom[romoffset << 2];
		const int x_base = flipx ? (xpos + width - 1) : xpos;
		const int x_inc = flipx ? (-1) : 1;
		palette = (palette & 0x3f) << 7;

		for (int y = 0; y < height; y++)
		{
			u16 *dest = &bitmap.pix(ypos + y);
			int x_index = x_base;
			for (int x = 0; x < width; x++)
			{
				u8 pix = *gfxrom++;
				if (pix)
				{
					if (cliprect.contains(x_index, ypos + y))
						dest[x_index] = pix | palette;
				}
				x_index += x_inc;
			}
		}
	}
}

u32 igs_fear_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3ff, cliprect);

	for (int i = 0; i < 0x2000 / 0x10; i++)
	{
		int       xpos    = (m_videoram[(i * 4) + 0] & 0x0000ffff) >> 0;
		int       ypos    = (m_videoram[(i * 4) + 0] & 0xffff0000) >> 16;
		int       height  = (m_videoram[(i * 4) + 1] & 0x0000ffff) >> 0;
		int       width   = (m_videoram[(i * 4) + 1] & 0xffff0000) >> 16;
		const int palette = (m_videoram[(i * 4) + 2] & 0x000000ff) >> 0;
		const int flipx   = (m_videoram[(i * 4) + 2] & 0x00000100) == 0;
		const int rom_msb = (m_videoram[(i * 4) + 2] & 0xffff0000) >> 16;
		const int rom_lsb = (m_videoram[(i * 4) + 3] & 0x0000ffff) >> 0;

		const int romoffset = rom_msb + (rom_lsb << 16);

		if (xpos & 0x8000)
			xpos -= 0x10000;
		if (ypos & 0x8000)
			ypos -= 0x10000;
		width = (width + 1) << 2;
		height = height + 1;

		draw_sprite(bitmap, cliprect, xpos, ypos, height, width, palette, flipx, romoffset);
	}

	return 0;
}

void igs_fear_state::main_map(address_map &map)
{
	map(0x08000000, 0x0807ffff).rom().region("user1", 0); // Game ROM
	map(0x10000000, 0x100003ff).ram().share("iram");
	map(0x18000000, 0x1800ffff).ram().share(m_sram);
	map(0x28000000, 0x28000003).rw("rtc", FUNC(v3021_device::read), FUNC(v3021_device::write));

	map(0x38000000, 0x38001fff).ram().share(m_videoram);
	map(0x38004000, 0x38007fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x38008500, 0x38008503).umask32(0x0000ffff).w(m_xa, FUNC(igs_xa_mcu_ics_sound_device::cmd_w));
	map(0x38008500, 0x380085ff).r(FUNC(igs_fear_state::xa_r));

	map(0x50000000, 0x500003ff).umask32(0x000000ff).w(FUNC(igs_fear_state::xor_table_w));

	map(0x58000000, 0x58000003).portr("IN0");
	map(0x58100000, 0x58100003).portr("IN1");

	map(0x68000000, 0x6800000f).w(FUNC(igs_fear_state::cpld_w));
}

void igs_fear_state::main_xor_map(address_map &map)
{
	main_map(map);

	map(0x08000000, 0x0807ffff).r(FUNC(igs_fear_state::external_rom_r)); // Game ROM
}


static INPUT_PORTS_START( fear )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW1:7")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW2:7")
INPUT_PORTS_END

INPUT_PORTS_START( superkds )
	PORT_INCLUDE ( fear )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x01, "Scene" )                   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Volcano" )
	PORT_DIPSETTING(    0x02, "Jungle" )
	PORT_DIPSETTING(    0x01, "Ice Field" )
	PORT_DIPSETTING(    0x00, "Ice Field (duplicate)" )
	PORT_DIPNAME( 0x04, 0x00, "Ticket Dispenser" )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0xf8, 0x00, "Ticket Payout Table" )     PORT_DIPLOCATION("SW1:4,5,6,7,8")
	PORT_DIPSETTING(    0xf8, "3 2 2 1 1 0 0 0" )
	PORT_DIPSETTING(    0xf0, "3 2 2 2 1 1 1 1" )
	PORT_DIPSETTING(    0xe8, "3 2 2 2 2 2 2 2" )
	PORT_DIPSETTING(    0xe0, "3 3 3 3 3 3 3 3" )
	PORT_DIPSETTING(    0xd8, "6 5 4 3 2 1 0 0" )
	PORT_DIPSETTING(    0xd0, "6 5 4 3 3 2 1 1" )
	PORT_DIPSETTING(    0xc8, "6 5 4 4 3 3 2 2" )
	PORT_DIPSETTING(    0xc0, "6 5 5 4 4 3 3 3" )
	PORT_DIPSETTING(    0xb8, "9 7 6 5 3 2 1 0" )
	PORT_DIPSETTING(    0xb0, "9 7 6 5 4 3 2 1" )
	PORT_DIPSETTING(    0xa8, "9 8 7 6 5 4 3 2" )
	PORT_DIPSETTING(    0xa0, "9 8 7 6 5 4 3 3" )
	PORT_DIPSETTING(    0x98, "12 10 8 6 5 3 1 0" )
	PORT_DIPSETTING(    0x90, "12 10 8 7 5 4 2 1" )
	PORT_DIPSETTING(    0x88, "12 10 9 7 6 4 3 2" )
	PORT_DIPSETTING(    0x80, "12 10 9 8 6 5 4 3" )
	PORT_DIPSETTING(    0x78, "20 17 14 11 8 5 2 0" )
	PORT_DIPSETTING(    0x70, "20 17 14 11 9 6 3 1" )
	PORT_DIPSETTING(    0x68, "20 17 14 12 9 7 4 2" )
	PORT_DIPSETTING(    0x60, "20 17 15 12 10 7 5 3" )
	PORT_DIPSETTING(    0x58, "30 25 21 17 12 8 4 0" )
	PORT_DIPSETTING(    0x50, "30 25 21 17 13 9 5 1" )
	PORT_DIPSETTING(    0x48, "30 26 22 18 14 10 6 2" )
	PORT_DIPSETTING(    0x40, "30 26 22 18 14 10 6 3" )
	PORT_DIPSETTING(    0x38, "40 34 28 22 17 11 5 0" )
	PORT_DIPSETTING(    0x30, "40 34 28 23 17 12 6 1" )
	PORT_DIPSETTING(    0x28, "40 34 29 23 18 12 7 2" )
	PORT_DIPSETTING(    0x20, "40 34 29 24 18 13 8 3" )
	PORT_DIPSETTING(    0x18, "50 42 35 28 21 14 7 0" )
	PORT_DIPSETTING(    0x10, "50 43 36 29 22 15 8 1" )
	PORT_DIPSETTING(    0x08, "50 43 36 29 22 15 8 2" )
	PORT_DIPSETTING(    0x00, "50 43 36 29 23 16 9 3" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Free_Play) )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR(Coin_A) )           PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR(4C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(3C_1C) )
	PORT_DIPSETTING(    0x04, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x06, DEF_STR(1C_1C) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR(Demo_Sounds) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x00, "Slave ID" )                PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "0" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x50, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x00, "Single" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR(Language) )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR(Chinese) )
	PORT_DIPSETTING(    0x00, DEF_STR(English) )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(20)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(20)
INPUT_PORTS_END

void igs_fear_state::vblank_irq(int state)
{
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, (state && m_screen->frame_number() & 1) ? 1 : 0);
}


u32 igs_fear_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}


void igs_fear_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}


u32 igs_fear_state::igs027_gpio_r()
{
	u8 dsw = 0xff;
	if (!BIT(m_gpio_o, 0)) dsw &= m_io_dsw[0]->read();
	if (!BIT(m_gpio_o, 1)) dsw &= m_io_dsw[1]->read();
	return 0x00400 | dsw;
}

void igs_fear_state::igs027_gpio_w(u8 data)
{
	m_gpio_o = data;
}

// TODO: trackball support in XA
u32 igs_fear_state::xa_r(offs_t offset, u32 mem_mask)
{
	u32 data = ~u32(0);

	switch (offset * 4)
	{
	case 0:
	{
		data = m_xa->response_low_r();
		// TODO: This should be remove when we implement serial trackball support in XA
		if (m_xa->cmd_r() == 0xa301)
		{
			switch (m_trackball_cnt++)
			{
			case 0:
				data = 0x80;
				for (int i = 0; i < 2; i++)
				{
					m_trackball_axis_pre[i] = m_trackball_axis[i];
					if (m_io_trackball[i])
						m_trackball_axis[i] = m_io_trackball[i]->read();

					if (m_trackball_axis[i] & 0x80)
						m_trackball_axis[i] -= 0x100;
					m_trackball_axis_diff[i] = m_trackball_axis[i] - m_trackball_axis_pre[i];
					if (m_trackball_axis_diff[i] >= 128)
						m_trackball_axis_diff[i] -= 256;
					if (m_trackball_axis_diff[i] <= -128)
						m_trackball_axis_diff[i] += 256;
				}
				break;
			case 1:
				data = 0x80;
				break;
			case 2:
				data = (m_trackball_axis_diff[0] - m_trackball_axis_diff[1]) / 2;
				break;
			case 3:
				data = (m_trackball_axis_diff[0] + m_trackball_axis_diff[1]) / 2;
				break;
			default:
				m_trackball_cnt = 0;
				break;
			}
		}
		break;
	}
	case 0x80:
		data = u32(m_xa->response_high_r()) << 16;
		break;
	}
	return data;
}

void igs_fear_state::cpld_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
	case 0x8:
		m_ticket->motor_w(BIT(data, 7));
		break;

	default:
		LOGMASKED(LOG_DEBUG, "%s: unhandled cpld_w %04x %08x (%08x)\n", machine().describe_context(), offset * 4, data, mem_mask);
		break;
	}
}


void igs_fear_state::igs_fear(machine_config &config)
{
	IGS027A(config, m_maincpu, 50'000'000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_fear_state::main_map);
	m_maincpu->in_port().set(FUNC(igs_fear_state::igs027_gpio_r));
	m_maincpu->out_port().set(FUNC(igs_fear_state::igs027_gpio_w));

	config.set_maximum_quantum(attotime::from_hz(600));

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(igs_fear_state::screen_update));
	m_screen->screen_vblank().set(FUNC(igs_fear_state::vblank_irq));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 0x4000/2);

	V3021(config, "rtc");

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(200));

	/* sound hardware */
	IGS_XA_ICS_SOUND(config, m_xa, 50'000'000/3);
	m_xa->irq().set_inputline(m_maincpu, arm7_cpu_device::ARM7_IRQ_LINE);
}

void igs_fear_state::igs_fear_xor(machine_config &config)
{
	igs_fear(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs_fear_state::main_xor_map);
}


ROM_START( fearless )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "fearless_igs027a.bin", 0x00000, 0x4000, CRC(2121f01b) SHA1(de44016b590fdcf6bfb63e3f8dbbbd679938fe87) ) // sticker marked 'F1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "fearlessp_v-101us.u37", 0x000000, 0x80000, CRC(2522873c) SHA1(8db709877311b6d2796353fc9a44a820937e35c2) )

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "fearlessp_07.u33", 0x000000, 0x10000, CRC(7dae4900) SHA1(bbf7ba7c9e95ff2ffeb1dc0fc7ccedd4da274d01) )

	ROM_REGION( 0x3000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "fearlessp_u7_cg-0l.u7",   0x0000000, 0x800000, CRC(ca254db4) SHA1(f5670c2ff0720c84c9aff3cea95b118b6044e469) )
	ROM_LOAD32_WORD( "fearlessp_u6_cg-0h.u6",   0x0000002, 0x800000, CRC(02d8bbbf) SHA1(7cf36c909a5d76096a725ffe0a697bcbafbcf985) )
	ROM_LOAD32_WORD( "fearlessp_u14_cg-1l.u14", 0x1000000, 0x800000, CRC(7fe312d2) SHA1(c0add22d9fc4c0e32a03922cb709b947bfff429d) )
	ROM_LOAD32_WORD( "fearlessp_u13_cg-1h.u13", 0x1000002, 0x800000, CRC(c9d2a56d) SHA1(91d1665623bca743f68f15a27bbf433e2ffc0808) )
	ROM_LOAD32_WORD( "fearlessp_u18_cg-2l.u18", 0x2000000, 0x800000, CRC(07623d66) SHA1(041d5e44917bc16caa720ea98bdc0a4f5fb4b8e0) )
	ROM_LOAD32_WORD( "fearlessp_u17_cg-2h.u17", 0x2000002, 0x800000, CRC(756fe1f2) SHA1(48ee81c5fa4808406b57b2521b836db3ff5a7fa9) )

	ROM_REGION( 0x800000, "xa:ics", 0 )
	ROM_LOAD( "fearlessp_u25_music0.u25", 0x000000, 0x400000, CRC(a015b9b1) SHA1(7b129c59acd523dec82e58a75d873bbc5341fb28) )
	ROM_LOAD( "fearlessp_u26_music1.u26", 0x400000, 0x400000, CRC(9d5f18da) SHA1(42e5224c1af0898cc2e02b2e051ea8b629d5fb6d) )
ROM_END

ROM_START( superkds )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "superkids_igs027a.bin", 0x00000, 0x4000, CRC(9a8e790d) SHA1(ab020a04a4ed0c0e5ec8c979f206fe57572d2304) ) // sticker marked 'F5'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "superkids_s019cn.u37", 0x000000, 0x80000, CRC(1a7f17dd) SHA1(ba20c0f521bff2f5ae2103ea49bd413b0e6459ba) )

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "superkids_mx10exa.u33", 0x000000, 0x10000, CRC(8baf5ba2) SHA1(2f8c2c48e756264e593bce7c09260e50d5cac827) ) // sticker marked G6

	ROM_REGION( 0x2000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "superkids_cg-0l.u7",  0x0000000, 0x800000, CRC(6c4eff23) SHA1(1089222bfedcd562f2a3c43c90512bbb9e22a8bf) )
	ROM_LOAD32_WORD( "superkids_cg-0h.u6",  0x0000002, 0x800000, CRC(b797b0b6) SHA1(4298564d963fdaeca94cc0e739fbcae401168949) )
	ROM_LOAD32_WORD( "superkids_cg-1l.u14", 0x1000000, 0x800000, CRC(57081c96) SHA1(886ac14ad1c9ce8c7a67bbfc6c00e7c75be634dc) )
	ROM_LOAD32_WORD( "superkids_cg-1h.u13", 0x1000002, 0x800000, CRC(cd1e41ef) SHA1(a40bcbd97fa3e742e8f9c7b7c7d8879175bf10ee) )

	ROM_REGION( 0x800000, "xa:ics", 0 )
	ROM_LOAD( "superkids_music0.u25", 0x000000, 0x400000, CRC(d7c37216) SHA1(ffcf7f1bf3093eb34ad0ae2cc89062de45b9d420) )
	ROM_LOAD( "superkids_music1.u26", 0x400000, 0x400000, CRC(5f080dbf) SHA1(f02330db3336f6606aae9f5a9eca819701caa3bf) )
ROM_END

ROM_START( icescape ) // IGS PCB-0433-16-GK (same PCB as Fearless Pinocchio) - Has IGS027A, MX10EXAQC, 2x Actel A54SX32A, ICS2115, 2x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "a7.bin", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'A7', unreadable location

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "icescape_v-104fa.u37", 0x000000, 0x80000, CRC(e3552726) SHA1(bac34ac4fce1519c1bc8020064090e77b5c2a629) ) // TMS27C240

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked O7
	ROM_LOAD( "o7.u33", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x2000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "icescape_fa_cg_u7.u7",   0x0000000, 0x800000, NO_DUMP )
	ROM_LOAD32_WORD( "icescape_fa_cg_u6.u6",   0x0000002, 0x800000, NO_DUMP )
	ROM_LOAD32_WORD( "icescape_fa_cg_u14.u14", 0x1000000, 0x800000, NO_DUMP )
	ROM_LOAD32_WORD( "icescape_fa_cg_u13.u13", 0x1000002, 0x800000, NO_DUMP )
	// u17 and u18 not populated

	ROM_REGION( 0x400000, "xa:ics", 0 )
	ROM_LOAD( "icescape_fa_sp_u25.u25", 0x000000, 0x200000, CRC(a01febd6) SHA1(6abe8b700c5725909939421e2493940421fc823f) ) // M27C160
	ROM_LOAD( "icescape_fa_sp_u26.u26", 0x200000, 0x200000, CRC(35085613) SHA1(bdc6ecf5ee6fd095a56e33e8ce893fe05bcb426c) ) // M27C160
ROM_END

void igs_fear_state::init_igs_fear()
{
	fearless_decrypt(machine());
}

void igs_fear_state::init_igs_superkds()
{
	superkds_decrypt(machine());
}

void igs_fear_state::init_igs_icescape()
{
	icescape_decrypt(machine());
}

} // anonymous namespace

GAME( 2005, superkds, 0, igs_fear_xor, superkds, igs_fear_state, init_igs_superkds, ROT0, "IGS (Golden Dragon Amusement license)", "Super Kids / Jiu Nan Xiao Yingxiong (S019CN)", MACHINE_NODEVICE_LAN )
GAME( 2006, fearless, 0, igs_fear_xor, fear,     igs_fear_state, init_igs_fear,     ROT0, "IGS (American Alpha license)",          "Fearless Pinocchio (V101US)",                  0 )
GAME( 2006, icescape, 0, igs_fear,     fear,     igs_fear_state, init_igs_icescape, ROT0, "IGS",                                   "Icescape (V104FA)",                            MACHINE_IS_SKELETON ) // IGS FOR V104FA 2006-11-02
