// license:BSD-3-Clause
// copyright-holders:David Haywood, XingXing

#include "emu.h"

#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "cpu/xa/xa.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/v3021.h"
#include "sound/ics2115.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class igs_fear_state : public driver_device
{
public:
	igs_fear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_xa(*this, "xa"),
		m_videoram(*this, "videoram"),
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfx1"),
		m_ticket(*this, "ticket"),
		m_io_dsw(*this, "DSW%u", 1U),
		m_io_trackball(*this, "AN%u", 0)
	{ }

	void igs_fear(machine_config &config);

	void init_igs_fear();
	void init_igs_superkds();

protected:
	virtual void video_start() override;

private:
	void main_map(address_map &map);

	void sound_irq(int state);
	void vblank_irq(int state);

	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int flipx, int romoffset);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u32 igs027_gpio_r(offs_t offset);
	void igs027_gpio_w(offs_t offset, u32 data, u32 mem_mask);

	TIMER_CALLBACK_MEMBER(igs027_timer0);
	TIMER_CALLBACK_MEMBER(igs027_timer1);

	void igs027_periph_init(void);
	void igs027_trigger_irq(int num);
	u32 igs027_periph_r(offs_t offset);
	void igs027_periph_w(offs_t offset, u32 data, u32 mem_mask);

	u32 xa_r(offs_t offset);
	void xa_w(offs_t offset, u32 data, u32 mem_mask);
	void cpld_w(offs_t offset, u32 data, u32 mem_mask);

	u32 m_gpio_o;
	u32 m_irq_enable;
	u32 m_irq_pending;
	emu_timer *m_timer0;
	emu_timer *m_timer1;

	u32 m_xa_cmd;
	int m_trackball_cnt;
	int m_trackball_axis[2], m_trackball_axis_pre[2], m_trackball_axis_diff[2];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<xa_cpu_device> m_xa;
	required_shared_ptr<u32> m_videoram;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_gfxrom;

	required_device<ticket_dispenser_device> m_ticket;
	required_ioport_array<2> m_io_dsw;
	required_ioport_array<2> m_io_trackball;
};


void igs_fear_state::video_start()
{
	igs027_periph_init();
}

void igs_fear_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int height, int width, int palette, int flipx, int romoffset)
{
	if ((romoffset != 0) && (romoffset != 0xffffffff))
	{
		//logerror("x=%d, y=%d, w=%d pix, h=%d pix, c=0x%02x, romoffset=0x%08x\n", xpos, ypos, width, height, palette, romoffset << 2);
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
	map(0x00000000, 0x00003fff).rom(); /* Internal ROM */
	map(0x08000000, 0x0807ffff).rom().region("user1", 0);/* Game ROM */
	map(0x10000000, 0x100003ff).ram().share("iram");
	map(0x18000000, 0x1800ffff).ram().share("sram");
	map(0x40000000, 0x400003ff).rw(FUNC(igs_fear_state::igs027_gpio_r), FUNC(igs_fear_state::igs027_gpio_w));
	map(0x50000000, 0x500003ff).ram().share("xortab");
	map(0x70000000, 0x700003ff).rw(FUNC(igs_fear_state::igs027_periph_r), FUNC(igs_fear_state::igs027_periph_w));

	map(0x28000000, 0x28000003).rw("rtc", FUNC(v3021_device::read), FUNC(v3021_device::write));
	map(0x38000000, 0x38001fff).ram().share("videoram");
	map(0x38004000, 0x38007fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x38008500, 0x380085ff).rw(FUNC(igs_fear_state::xa_r), FUNC(igs_fear_state::xa_w));
	map(0x58000000, 0x58000003).portr("IN0");
	map(0x58100000, 0x58100003).portr("IN1");
	map(0x68000000, 0x6800000f).w(FUNC(igs_fear_state::cpld_w));

}

static INPUT_PORTS_START( fear )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

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

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(20)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(20)
INPUT_PORTS_END

INPUT_PORTS_START( superkds )
	PORT_INCLUDE ( fear )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Scene" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "Volcano" )
	PORT_DIPSETTING(    0x02, "Jungle" )
	PORT_DIPSETTING(    0x01, "Ice Field" )
	PORT_DIPSETTING(    0x00, "Ice Field" )
	PORT_DIPNAME( 0x04, 0x00, "Ticket" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0xf8, 0x00, "Ticket Table" ) PORT_DIPLOCATION("SW1:4,5,6,7,8")
	PORT_DIPSETTING(    0xf8, "Table1" )
	PORT_DIPSETTING(    0xf0, "Table2" )
	PORT_DIPSETTING(    0xe8, "Table3" )
	PORT_DIPSETTING(    0xe0, "Table4" )
	PORT_DIPSETTING(    0xd8, "Table5" )
	PORT_DIPSETTING(    0xd0, "Table6" )
	PORT_DIPSETTING(    0xc8, "Table7" )
	PORT_DIPSETTING(    0xc0, "Table8" )
	PORT_DIPSETTING(    0xb8, "Table9" )
	PORT_DIPSETTING(    0xb0, "Table10" )
	PORT_DIPSETTING(    0xa8, "Table11" )
	PORT_DIPSETTING(    0xa0, "Table12" )
	PORT_DIPSETTING(    0x98, "Table13" )
	PORT_DIPSETTING(    0x90, "Table14" )
	PORT_DIPSETTING(    0x88, "Table15" )
	PORT_DIPSETTING(    0x80, "Table16" )
	PORT_DIPSETTING(    0x78, "Table17" )
	PORT_DIPSETTING(    0x70, "Table18" )
	PORT_DIPSETTING(    0x68, "Table19" )
	PORT_DIPSETTING(    0x60, "Table20" )
	PORT_DIPSETTING(    0x58, "Table21" )
	PORT_DIPSETTING(    0x50, "Table22" )
	PORT_DIPSETTING(    0x48, "Table23" )
	PORT_DIPSETTING(    0x40, "Table24" )
	PORT_DIPSETTING(    0x38, "Table25" )
	PORT_DIPSETTING(    0x30, "Table26" )
	PORT_DIPSETTING(    0x28, "Table27" )
	PORT_DIPSETTING(    0x20, "Table28" )
	PORT_DIPSETTING(    0x18, "Table29" )
	PORT_DIPSETTING(    0x10, "Table30" )
	PORT_DIPSETTING(    0x08, "Table31" )
	PORT_DIPSETTING(    0x00, "Table32" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Free Play" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin/Credit" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x08, "Demo BGM" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x70, 0x00, "Slave" ) PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "0" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x50, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x00, "Single" )
	PORT_DIPNAME( 0x80, 0x00, "Language" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Chinese" )
	PORT_DIPSETTING(    0x00, "English" )
INPUT_PORTS_END

void igs_fear_state::sound_irq(int state)
{
}

void igs_fear_state::vblank_irq(int state)
{
	if (state)
		m_maincpu->pulse_input_line(ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
}

u32 igs_fear_state::igs027_gpio_r(offs_t offset)
{
	u32 data = ~u32(0);
	switch (offset * 4)
	{
	case 0xc:
		{
			u8 ret = 0xff;
			if (!BIT(m_gpio_o, 0)) ret &= m_io_dsw[0]->read();
			if (!BIT(m_gpio_o, 1)) ret &= m_io_dsw[1]->read();
			data = 0x2000 | (u32(ret) << 3);
		}
		break;
	}
	return data;
}

void igs_fear_state::igs027_gpio_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
	case 0x18:
		m_gpio_o = data;
		break;
	}
}

void igs_fear_state::igs027_periph_init()
{
	m_irq_enable = 0xff;
	m_irq_pending = 0xff;
	m_timer0 = timer_alloc(FUNC(igs_fear_state::igs027_timer0), this);
	m_timer1 = timer_alloc(FUNC(igs_fear_state::igs027_timer1), this);
}

void igs_fear_state::igs027_trigger_irq(int num)
{
	if (!BIT(m_irq_enable, num))
	{
		m_irq_pending &= ~(u32(1) << num);
		m_maincpu->pulse_input_line(ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time());
	}
}

TIMER_CALLBACK_MEMBER(igs_fear_state::igs027_timer0)
{
	igs027_trigger_irq(0);
}

TIMER_CALLBACK_MEMBER(igs_fear_state::igs027_timer1)
{
	igs027_trigger_irq(1);
}

void igs_fear_state::igs027_periph_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
	case 0x100:
		// TODO: verify the timer interval
		m_timer0->adjust(attotime::from_hz(data), 0, attotime::from_hz(data));
		break;

	case 0x104:
		m_timer1->adjust(attotime::from_hz(data), 0, attotime::from_hz(data));
		break;

	case 0x200:
		m_irq_enable = data;
		break;
	}
}

u32 igs_fear_state::igs027_periph_r(offs_t offset)
{
	u32 data = ~u32(0);
	switch (offset * 4)
	{
	case 0x200:
		data = m_irq_pending;
		m_irq_pending = 0xff;
		break;
	}
	return data;
}

// TODO: ICS2115 & trackball support in XA
u32 igs_fear_state::xa_r(offs_t offset)
{
	u32 data = ~u32(0);

	switch (offset * 4)
	{
	case 0:
	{
		if (m_xa_cmd == 0xa301)
		{
			switch (m_trackball_cnt++)
			{
			case 0:
				data = 0x80;
				for (int i = 0; i < 2; i++)
				{
					m_trackball_axis_pre[i] = m_trackball_axis[i];
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
		data = 0;
		break;
	}
	return data;
}

void igs_fear_state::xa_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0)
	{
		m_xa_cmd = data;
		igs027_trigger_irq(3);
	}
}

void igs_fear_state::cpld_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
	case 0x8:
		m_ticket->motor_w(BIT(data, 7));
		break;
	}
}

void igs_fear_state::igs_fear(machine_config &config)
{
	ARM7(config, m_maincpu, 50000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_fear_state::main_map);

	MX10EXA(config, m_xa, 50000000/3); // MX10EXAQC (Philips 80C51 XA)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(igs_fear_state::screen_update));
	screen.screen_vblank().set(FUNC(igs_fear_state::vblank_irq));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 0x4000/2);

	V3021(config, "rtc");

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH );

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ics2115_device &ics(ICS2115(config, "ics", 33.8688_MHz_XTAL)); // TODO : Correct?
	ics.irq().set(FUNC(igs_fear_state::sound_irq));
	ics.add_route(ALL_OUTPUTS, "mono", 5.0);
}


ROM_START( fearless )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "fearless_igs027a.bin", 0x00000, 0x4000, CRC(2121f01b) SHA1(de44016b590fdcf6bfb63e3f8dbbbd679938fe87) ) // sticker marked 'F1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "fearlessp_v-101us.u37", 0x000000, 0x80000, CRC(2522873c) SHA1(8db709877311b6d2796353fc9a44a820937e35c2) )

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "fearlessp_07.u33", 0x000000, 0x10000, CRC(7dae4900) SHA1(bbf7ba7c9e95ff2ffeb1dc0fc7ccedd4da274d01) )

	ROM_REGION( 0x3000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "fearlessp_u7_cg-0l.u7",   0x0000000, 0x800000, CRC(ca254db4) SHA1(f5670c2ff0720c84c9aff3cea95b118b6044e469) )
	ROM_LOAD32_WORD( "fearlessp_u6_cg-0h.u6",   0x0000002, 0x800000, CRC(02d8bbbf) SHA1(7cf36c909a5d76096a725ffe0a697bcbafbcf985) )
	ROM_LOAD32_WORD( "fearlessp_u14_cg-1l.u14", 0x1000000, 0x800000, CRC(7fe312d2) SHA1(c0add22d9fc4c0e32a03922cb709b947bfff429d) )
	ROM_LOAD32_WORD( "fearlessp_u13_cg-1h.u13", 0x1000002, 0x800000, CRC(c9d2a56d) SHA1(91d1665623bca743f68f15a27bbf433e2ffc0808) )
	ROM_LOAD32_WORD( "fearlessp_u18_cg-2l.u18", 0x2000000, 0x800000, CRC(07623d66) SHA1(041d5e44917bc16caa720ea98bdc0a4f5fb4b8e0) )
	ROM_LOAD32_WORD( "fearlessp_u17_cg-2h.u17", 0x2000002, 0x800000, CRC(756fe1f2) SHA1(48ee81c5fa4808406b57b2521b836db3ff5a7fa9) )

	ROM_REGION( 0x800000, "ics", 0 )
	ROM_LOAD( "fearlessp_u25_music0.u25", 0x000000, 0x400000, CRC(a015b9b1) SHA1(7b129c59acd523dec82e58a75d873bbc5341fb28) )
	ROM_LOAD( "fearlessp_u26_music1.u26", 0x400000, 0x400000, CRC(9d5f18da) SHA1(42e5224c1af0898cc2e02b2e051ea8b629d5fb6d) )
ROM_END

ROM_START( superkds )
	ROM_REGION( 0x04000, "maincpu", 0 ) // Internal rom of IGS027A ARM based MCU
	ROM_LOAD( "superkids_igs027a.bin", 0x00000, 0x4000, CRC(9a8e790d) SHA1(ab020a04a4ed0c0e5ec8c979f206fe57572d2304) ) // sticker marked 'F5'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "superkids_s019cn.u37", 0x000000, 0x80000, CRC(1a7f17dd) SHA1(ba20c0f521bff2f5ae2103ea49bd413b0e6459ba) )

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU) marked 07, not read protected
	ROM_LOAD( "superkids_mx10exa.u33", 0x000000, 0x10000, CRC(8baf5ba2) SHA1(2f8c2c48e756264e593bce7c09260e50d5cac827) ) // sticker marked G6

	ROM_REGION( 0x2000000, "gfx1", 0 ) // FIXED BITS (0xxxxxxx) (graphics are 7bpp)
	ROM_LOAD32_WORD( "superkids_cg-0l.u7",  0x0000000, 0x800000, CRC(6c4eff23) SHA1(1089222bfedcd562f2a3c43c90512bbb9e22a8bf) )
	ROM_LOAD32_WORD( "superkids_cg-0h.u6",  0x0000002, 0x800000, CRC(b797b0b6) SHA1(4298564d963fdaeca94cc0e739fbcae401168949) )
	ROM_LOAD32_WORD( "superkids_cg-1l.u14", 0x1000000, 0x800000, CRC(57081c96) SHA1(886ac14ad1c9ce8c7a67bbfc6c00e7c75be634dc) )
	ROM_LOAD32_WORD( "superkids_cg-1h.u13", 0x1000002, 0x800000, CRC(cd1e41ef) SHA1(a40bcbd97fa3e742e8f9c7b7c7d8879175bf10ee) )

	ROM_REGION( 0x800000, "ics", 0 )
	ROM_LOAD( "superkids_music0.u25", 0x000000, 0x400000, CRC(d7c37216) SHA1(ffcf7f1bf3093eb34ad0ae2cc89062de45b9d420) )
	ROM_LOAD( "superkids_music1.u26", 0x400000, 0x400000, CRC(5f080dbf) SHA1(f02330db3336f6606aae9f5a9eca819701caa3bf) )
ROM_END

void igs_fear_state::init_igs_fear()
{
	fearless_decrypt(machine());
}

void igs_fear_state::init_igs_superkds()
{
	superkds_decrypt(machine());
}

} // anonymous namespace

GAME( 2005, superkds, 0, igs_fear, superkds, igs_fear_state, init_igs_superkds, ROT0, "IGS", "Super Kids (S019CN)",           MACHINE_IS_SKELETON )
GAME( 2006, fearless, 0, igs_fear, fear,     igs_fear_state, init_igs_fear,     ROT0, "IGS", "Fearless Pinocchio (V101US)",   MACHINE_IS_SKELETON )
