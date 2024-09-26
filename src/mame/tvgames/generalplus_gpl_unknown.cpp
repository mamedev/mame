// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
   NOTES:

   Exact size of internal ROM is unknown, but it appears to occupy at least 0x4000 - 0xffff (word addresses)
   in unSP space and is different for each game.  Data is pulled for jump tables, where the addresses are in
   internal ROM and the code in external

   For correctly offset disassembly use
   unidasm xxx.bin -arch unsp12 -xchbytes -basepc 200000 >palace.txt

   Multiple different units appear to share the same ROM with a jumper to select game.
   It should be verified in each case that the external ROM was not changed

   It is confirmed that in some cases the external ROMs contain both versions for Micro Arcade and Tiny Arcade
   units, with different sound, but in some cases the expected game behavior differs too, so the code revisions
   could be different.  (for example the Micro Arcade Pac-Man doesn't display points when you eat a ghost)
*/


#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "logmacro.h"


namespace {

class generalplus_gpl_unknown_state : public driver_device
{
public:
	generalplus_gpl_unknown_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		//m_mainrom(*this, "maincpu"),
		//m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_dac(*this, "dac"),
		m_spirom(*this, "spi"),
		m_testio(*this, "TEST")
	{ }

	void generalplus_gpl_unknown(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<unsp_12_device> m_maincpu;
	//required_region_ptr<uint16_t> m_mainrom;
	//required_shared_ptr<uint16_t> m_mainram;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<dac_word_device_base> m_dac;

	required_region_ptr<uint16_t> m_spirom;
	required_ioport m_testio;

	uint16_t reg3001_r(offs_t offset);
	void reg3001_w(offs_t offset, uint16_t data);
	uint16_t reg3002_r(offs_t offset);
	void reg3002_w(offs_t offset, uint16_t data);
	uint16_t reg3003_r(offs_t offset);
	void reg3003_w(offs_t offset, uint16_t data);
	uint16_t reg3004_r(offs_t offset);
	uint16_t reg3005_r(offs_t offset);
	void reg3005_w(offs_t offset, uint16_t data);
	uint16_t reg3006_r(offs_t offset);
	uint16_t reg3007_r(offs_t offset);

	uint16_t reg300f_r(offs_t offset);

	uint16_t reg3016_r(offs_t offset);

	void reg3034_w(offs_t offset, uint16_t data);
	void reg3041_audiodac_w(offs_t offset, uint16_t data);

	uint16_t reg3050_r(offs_t offset);
	void reg3050_w(offs_t offset, uint16_t data);
	void reg3051_w(offs_t offset, uint16_t data);
	uint16_t reg3052_r(offs_t offset);
	uint16_t reg3053_r(offs_t offset);

	uint16_t reg3090_r(offs_t offset);
	uint16_t reg3091_r(offs_t offset);
	void reg3092_lcd_w(offs_t offset, uint16_t data);
	uint16_t reg3094_r(offs_t offset);
	uint16_t reg3095_r(offs_t offset);

	void reg30e0_w(offs_t offset, uint16_t data);
	void reg30e1_w(offs_t offset, uint16_t data);
	void reg30e2_w(offs_t offset, uint16_t data);
	void reg30e3_w(offs_t offset, uint16_t data);
	uint16_t reg30e4_r(offs_t offset);
	uint16_t reg30e5_r(offs_t offset);

	TIMER_DEVICE_CALLBACK_MEMBER(timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer2);
	TIMER_DEVICE_CALLBACK_MEMBER(timer3);

	uint16_t m_display[128*2 * 128];
	int m_displayposx;
	int m_displayposy;
	uint16_t m_3001;
	uint16_t m_3003;
	uint16_t m_3005;
	uint16_t m_3050;

	void map(address_map &map) ATTR_COLD;
};

uint32_t generalplus_gpl_unknown_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	for (int y = 0; y < 128; y++)
	{
		u16* dst = &bitmap.pix(y);

		for (int x = 0; x < 128; x++)
		{
			uint8_t pix1 = m_display[count++];
			uint8_t pix2 = m_display[count++];

			uint16_t pal = ((pix1<<8) | pix2);

			dst[x] = pal;
		}
	}
	return 0;
}


static INPUT_PORTS_START( generalplus_gpl_unknown )
	PORT_START("TEST")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

uint16_t generalplus_gpl_unknown_state::reg3001_r(offs_t offset)
{
	return m_3001;
}

void generalplus_gpl_unknown_state::reg3001_w(offs_t offset, uint16_t data)
{
	m_3001 = data;
	logerror("%s: reg3001_w %04x\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg3002_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

void generalplus_gpl_unknown_state::reg3002_w(offs_t offset, uint16_t data)
{
	//logerror("%s: reg3002_w %04x\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg3003_r(offs_t offset)
{
	return m_3003;
}

void generalplus_gpl_unknown_state::reg3003_w(offs_t offset, uint16_t data)
{
	m_3003 = data;
	logerror("%s: reg3003_w %04x\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg3004_r(offs_t offset)
{
	return m_testio->read();
}

uint16_t generalplus_gpl_unknown_state::reg3005_r(offs_t offset)
{
	return m_3005;
}

void generalplus_gpl_unknown_state::reg3005_w(offs_t offset, uint16_t data)
{
	m_3005 = data;
	//logerror("%s: reg3005_w %04x\n", machine().describe_context(), data);
}


uint16_t generalplus_gpl_unknown_state::reg3006_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

uint16_t generalplus_gpl_unknown_state::reg3007_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

uint16_t generalplus_gpl_unknown_state::reg300f_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

uint16_t generalplus_gpl_unknown_state::reg3016_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}



void generalplus_gpl_unknown_state::reg3034_w(offs_t offset, uint16_t data)
{
	// writes a lot of 0x5555 here in a block
	logerror("%s: reg3034_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpl_unknown_state::reg3041_audiodac_w(offs_t offset, uint16_t data)
{
//  logerror("%s: reg3041_audiodac_w %04x\n", machine().describe_context(), data);

	// mapacman only writes 0000 / 7fff / 8000, but is known to have more limited sound than other units

	m_dac->data_w(data);
}

uint16_t generalplus_gpl_unknown_state::reg3050_r(offs_t offset)
{
	return m_3050;
}

void generalplus_gpl_unknown_state::reg3050_w(offs_t offset, uint16_t data)
{
	uint16_t old = m_3050;
	m_3050 = data;

	// probably not, but does happen at the end of a line
	if ((old & 0x0100) != (m_3050 & 0x0100))
	{
		if ((m_3050 & 0x0100) == 0x0000)
		{
			m_displayposx = 0;
			m_displayposy++;
			if (m_displayposy == 128)
			{
				m_displayposy = 0;
			}
		}
	}

	//m_displaypos = 0;
	logerror("%s: reg3050_w %04x\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg3052_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

uint16_t generalplus_gpl_unknown_state::reg3053_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

void generalplus_gpl_unknown_state::reg30e0_w(offs_t offset, uint16_t data)
{
	//logerror("%s: reg30e0_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpl_unknown_state::reg30e1_w(offs_t offset, uint16_t data)
{
	//logerror("%s: reg30e1_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpl_unknown_state::reg30e2_w(offs_t offset, uint16_t data)
{
	/* this appears to be querying the SPI Flash, eg.
	   command 0xab = Read Electonic Signature
	   command 0x9f = Read ID of the SPI Flash

	[:] ':maincpu' (0075DD): reg3001_w 1c00
	[:] ':maincpu' (00769B): reg3001_w 0c00
	[:] ':maincpu' (00769F): reg30e2_w 00ab (WITHOUT m_3001 & 0x1000)
	[:] ':maincpu' (0076A5): reg3001_w 1c00
	[:] ':maincpu' (0076A7): reg30e4_r
	[:] ':maincpu' (0076AF): reg3001_w 0c00
	[:] ':maincpu' (0076B3): reg30e2_w 009f (WITHOUT m_3001 & 0x1000)
	[:] ':maincpu' (0076B6): reg30e2_w 0000 (WITHOUT m_3001 & 0x1000)
	[:] ':maincpu' (0076B8): reg30e2_w 0000 (WITHOUT m_3001 & 0x1000)
	[:] ':maincpu' (0076BA): reg30e2_w 0000 (WITHOUT m_3001 & 0x1000)
	[:] ':maincpu' (0076C0): reg30e4_r
	[:] ':maincpu' (0076C2): reg30e4_r
	[:] ':maincpu' (0076C4): reg30e4_r
	[:] ':maincpu' (0076C6): reg30e4_r
	*/
	if (!(m_3001 & 0x1000))
	{
		logerror("%s: reg30e2_w %04x (WITHOUT m_3001 & 0x1000)\n", machine().describe_context(), data);
	}
	else
	{
		logerror("%s: reg30e2_w %04x (WITH m_3001 & 0x1000)\n", machine().describe_context(), data);
	}
}

void generalplus_gpl_unknown_state::reg30e3_w(offs_t offset, uint16_t data)
{
	//logerror("%s: reg30e3_w %04x\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg30e4_r(offs_t offset)
{
	logerror("%s: reg30e4_r\n", machine().describe_context());
	return machine().rand() & 0x01;
}

uint16_t generalplus_gpl_unknown_state::reg30e5_r(offs_t offset)
{
	// status flag: loops on bit 0x0010 after writes to 0x30e2
	// and before reading from 0x30e4

	// clrb [3001],12 is also usually executed before write operations
	// to 0x30e2 and setb [3001],12 befoe the result is read back

	return 0x0000;
}

/*
mapacman vectors

004015: 4844        - just reti
        4845 fiq    - has code, acks by writing 8000 to 3051, very similar to irq0?)
        4809 boot vector
        4854 irq 0  - has code, acks by writing 8000 to 3051
        4863 irq 1  - just acks by writing 4000 to 3051, no payload
        486A irq 2 (used - needed to pass check on value in RAM changing in boot) acks with 2000 to 3051
        487D irq 3 (used - needed to pass check on value in RAM changing shortly after) acks with 0100 to 3051?
        4886 irq 4 - just acks by writing 10 to 3051, no payload
        488C irq 5 - push/pop/return
        488F irq 6 - push/pop/return
        4892 irq 7 - minimal payload, no 3051 ack?
*/

void generalplus_gpl_unknown_state::reg3051_w(offs_t offset, uint16_t data)
{
	if (data & 0x8000)
		m_maincpu->set_input_line(UNSP_IRQ0_LINE, CLEAR_LINE);

	if (data & 0x2000)
		m_maincpu->set_input_line(UNSP_IRQ2_LINE, CLEAR_LINE);

	if (data & 0x0100)
		m_maincpu->set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);


	//logerror("%s: reg3051_w %04x (IRQ Ack?)\n", machine().describe_context(), data);
}

uint16_t generalplus_gpl_unknown_state::reg3090_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

uint16_t generalplus_gpl_unknown_state::reg3091_r(offs_t offset)
{
	return 0x0000;//machine().rand();
}

void generalplus_gpl_unknown_state::reg3092_lcd_w(offs_t offset, uint16_t data)
{
	if (m_3005 & 0x1000)
	{
		//if (m_3005 & 0x0800) // also always set for video writes?
		{
			//logerror("%s: reg3092_lcd_w %04x (Video?)\n", machine().describe_context(), data);
			if ((m_displayposx < 256) && (m_displayposy < 256))
				m_display[(m_displayposy * 256) + m_displayposx] = data;

			if (data & 0xff00)
				fatalerror("upper data bits set?\n");

			m_displayposx++;

			/*
			if (m_displayposx == 256)
			{
			    m_displayposy++;
			    m_displayposx = 0;
			}
			*/
			/*
			if (m_displayposy == 128)
			{
			    m_displayposy = 0;
			    m_displayposx = 0;
			}
			*/
		}
	}
}

uint16_t generalplus_gpl_unknown_state::reg3094_r(offs_t offset)
{
	return 0x0000;
}

uint16_t generalplus_gpl_unknown_state::reg3095_r(offs_t offset)
{
	// loops on bit 0x0010 after writing data to 0x3092

	// clrb [3005],12 is used before writing non-pixel data to 0x3092
	// setb [3005],12 is used after reading from 0x3094
	return 0x0000;
}


void generalplus_gpl_unknown_state::map(address_map &map)
{
	map(0x000000, 0x000fff).ram(); // RAM
	//map(0x001000, 0x0017ff).ram(); // acts like open bus?

	//map(0x003000, 0x003fff).ram(); // system regs
	map(0x003001, 0x003001).rw(FUNC(generalplus_gpl_unknown_state::reg3001_r), FUNC(generalplus_gpl_unknown_state::reg3001_w));
	map(0x003002, 0x003002).rw(FUNC(generalplus_gpl_unknown_state::reg3002_r), FUNC(generalplus_gpl_unknown_state::reg3002_w));
	map(0x003003, 0x003003).rw(FUNC(generalplus_gpl_unknown_state::reg3003_r), FUNC(generalplus_gpl_unknown_state::reg3003_w));
	map(0x003004, 0x003004).r(FUNC(generalplus_gpl_unknown_state::reg3004_r));  // input from here is acted upon
	map(0x003005, 0x003005).rw(FUNC(generalplus_gpl_unknown_state::reg3005_r), FUNC(generalplus_gpl_unknown_state::reg3005_w));
	map(0x003006, 0x003006).r(FUNC(generalplus_gpl_unknown_state::reg3006_r));
	map(0x003007, 0x003007).r(FUNC(generalplus_gpl_unknown_state::reg3007_r));

	map(0x00300f, 0x00300f).r(FUNC(generalplus_gpl_unknown_state::reg300f_r));

	map(0x003016, 0x003016).r(FUNC(generalplus_gpl_unknown_state::reg3016_r));

	map(0x003034, 0x003034).w(FUNC(generalplus_gpl_unknown_state::reg3034_w));

	map(0x003041, 0x003041).w(FUNC(generalplus_gpl_unknown_state::reg3041_audiodac_w));

	map(0x003050, 0x003050).rw(FUNC(generalplus_gpl_unknown_state::reg3050_r), FUNC(generalplus_gpl_unknown_state::reg3050_w));
	map(0x003051, 0x003051).w(FUNC(generalplus_gpl_unknown_state::reg3051_w));
	map(0x003052, 0x003052).r(FUNC(generalplus_gpl_unknown_state::reg3052_r));
	map(0x003053, 0x003053).r(FUNC(generalplus_gpl_unknown_state::reg3053_r));

	map(0x003090, 0x003090).r(FUNC(generalplus_gpl_unknown_state::reg3090_r));
	map(0x003091, 0x003091).r(FUNC(generalplus_gpl_unknown_state::reg3091_r));
	map(0x003092, 0x003092).w(FUNC(generalplus_gpl_unknown_state::reg3092_lcd_w));
	map(0x003094, 0x003094).r(FUNC(generalplus_gpl_unknown_state::reg3094_r));  // potential interesting
	map(0x003095, 0x003095).r(FUNC(generalplus_gpl_unknown_state::reg3095_r));  // mostly a status flag

	map(0x0030e0, 0x0030e0).w(FUNC(generalplus_gpl_unknown_state::reg30e0_w));
	map(0x0030e1, 0x0030e1).w(FUNC(generalplus_gpl_unknown_state::reg30e1_w));
	map(0x0030e2, 0x0030e2).w(FUNC(generalplus_gpl_unknown_state::reg30e2_w));
	map(0x0030e3, 0x0030e3).w(FUNC(generalplus_gpl_unknown_state::reg30e3_w));
	map(0x0030e4, 0x0030e4).r(FUNC(generalplus_gpl_unknown_state::reg30e4_r));  // potentially interesting (must not return 0x0000 or 'flash error')
	map(0x0030e5, 0x0030e5).r(FUNC(generalplus_gpl_unknown_state::reg30e5_r));  // potentially interesting (also status flag)

	map(0x004000, 0x00bfff).rom().region("maincpu", 0x0000);
	map(0x00c000, 0x00ffff).rom().region("maincpu", 0x0000);

	map(0x200000, 0x3fffff).rom().region("spi", 0x0000); // has direct access to SPI ROM
}


void generalplus_gpl_unknown_state::machine_start()
{
	for (int color = 0; color < 0x10000; color++)
	{
		const u8 r = pal5bit(color >> 11);
		const u8 g = pal6bit(color >> 5);
		const u8 b = pal5bit(color & 0x1f);
		m_palette->set_pen_color(color, rgb_t(r, g, b));
	}

	save_item(NAME(m_display));
	save_item(NAME(m_displayposx));
	save_item(NAME(m_displayposy));
	save_item(NAME(m_3001));
	save_item(NAME(m_3003));
	save_item(NAME(m_3005));
	save_item(NAME(m_3050));
}

void generalplus_gpl_unknown_state::machine_reset()
{
	m_displayposx = 0;
	m_displayposy = 0;
	m_3001 = 0;
	m_3003 = 0;
	m_3005 = 0;
	m_3050 = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpl_unknown_state::timer )
{
	m_maincpu->set_input_line(UNSP_IRQ3_LINE, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpl_unknown_state::timer2 )
{
	m_maincpu->set_input_line(UNSP_IRQ2_LINE, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpl_unknown_state::timer3 )
{
	m_maincpu->set_input_line(UNSP_IRQ0_LINE, ASSERT_LINE);
}



void generalplus_gpl_unknown_state::generalplus_gpl_unknown(machine_config &config)
{
	UNSP_20(config, m_maincpu, 96000000); // internal ROM uses unsp2.0 opcodes, unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gpl_unknown_state::map);
	m_maincpu->set_vectorbase(0x4010); // there is also a set of vectors for what looks to be a burn-in test at 4000, maybe external pin selects?

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(128, 128);
	m_screen->set_visarea(0, 128-1, 0, 128-1); // not the correct resolution
	m_screen->set_screen_update(FUNC(generalplus_gpl_unknown_state::screen_update));
	//m_screen->screen_vblank().set(FUNC(generalplus_gpl_unknown_state::screen_vblank));
	m_screen->set_palette(m_palette);

	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC

	TIMER(config, "timer").configure_periodic(FUNC(generalplus_gpl_unknown_state::timer), attotime::from_hz(200000)); // draw timer (pushes pixels to the display in the IRQ)
	TIMER(config, "timer2").configure_periodic(FUNC(generalplus_gpl_unknown_state::timer2), attotime::from_hz(1000)); // game speed?
	TIMER(config, "timer3").configure_periodic(FUNC(generalplus_gpl_unknown_state::timer3), attotime::from_hz(20000)); // audio

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);
}


ROM_START( mapacman ) // this is the single game (no games hidden behind solder pads) release
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mapacman_internal.rom", 0x000000, 0x10000, CRC(9ea69d2a) SHA1(17f5001794f4454bf5856cfa170834509d68bed0) )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END

ROM_START( taspinv )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcade_spaceinvaders.bin", 0x000000, 0x200000, CRC(11ac4c77) SHA1(398d5eff83a4e94487ed810819085a0e44582908) )
ROM_END

ROM_START( tagalaga )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadegalaga_fm25q16a_a14015.bin", 0x000000, 0x200000, CRC(2a91460c) SHA1(ce297642d2d51ce568e93c0c57432446633b2077) )
ROM_END

ROM_START( parcade )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "palacearcade_gpr25l3203_c22016.bin", 0x000000, 0x400000, CRC(98fbd2a1) SHA1(ffc19aadd53ead1f9f3472475606941055ca09f9) )
ROM_END

ROM_START( taturtf )
	ROM_REGION16_BE( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadeturtlefighter_25q32bst16_684016.bin", 0x000000, 0x400000, CRC(8e046f2d) SHA1(e48492cf953f22a47fa2b88a8f96a1e459b8c487) )
ROM_END

} // anonymous namespace


// The 'Micro Arcade' units are credit card sized handheld devices
CONS( 2017, mapacman,      0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Pac-Man (Micro Arcade)", MACHINE_IS_SKELETON )

// The 'Tiny Arcade' units are arcade cabinets on a keyring.
CONS( 2017, taspinv,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Space Invaders (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2017, tagalaga,      0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Galaga (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2017, parcade,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Hasbro", "Palace Arcade (Tiny Arcade)", MACHINE_IS_SKELETON )

CONS( 2019, taturtf,       0,       0,      generalplus_gpl_unknown,   generalplus_gpl_unknown, generalplus_gpl_unknown_state, empty_init, "Super Impulse", "Teenage Mutant Ninja Turtles - Turtle Fighter (Tiny Arcade)", MACHINE_IS_SKELETON )


