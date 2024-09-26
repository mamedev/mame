// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    X the Ball

    driver by Aaron Giles

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/74259.h"
#include "machine/ticket.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "video/tlc34076.h"
#include "screen.h"
#include "speaker.h"


namespace {

class xtheball_state : public driver_device
{
public:
	xtheball_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_tlc34076(*this, "tlc34076"),
		m_ticket(*this, "ticket"),
		m_vram_bg(*this, "vrabg"),
		m_vram_fg(*this, "vrafg"),
		m_analog_x(*this, "ANALOGX"),
		m_analog_y(*this, "ANALOGY")
	{ }

	void xtheball(machine_config &config);

private:
	required_device<tms34010_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<tlc34076_device> m_tlc34076;
	required_device<ticket_dispenser_device> m_ticket;

	required_shared_ptr<uint16_t> m_vram_bg;
	required_shared_ptr<uint16_t> m_vram_fg;

	bool m_foreground_mode;

	required_ioport m_analog_x;
	required_ioport m_analog_y;

	void foreground_mode_w(int state);
	uint16_t analogx_r();
	uint16_t analogy_watchdog_r();

	virtual void machine_start() override ATTR_COLD;

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
	void main_map(address_map &map) ATTR_COLD;
};


void xtheball_state::machine_start()
{
	save_item(NAME(m_foreground_mode));
}

/*************************************
 *
 *  Video update
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(xtheball_state::scanline_update)
{
	uint16_t const *const srcbg = &m_vram_bg[(params->rowaddr << 8) & 0xff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	pen_t const *const pens = m_tlc34076->pens();
	int coladdr = params->coladdr;

	/* bit stored at 3040130 controls which foreground mode to use */
	if (!m_foreground_mode)
	{
		/* mode 0: foreground is the same as background */
		uint16_t const *const srcfg = &m_vram_fg[(params->rowaddr << 8) & 0xff00];

		for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
		{
			uint16_t fgpix = srcfg[coladdr & 0xff];
			uint16_t bgpix = srcbg[coladdr & 0xff];

			dest[x + 0] = pens[((fgpix & 0x00ff) != 0) ? (fgpix & 0xff) : (bgpix & 0xff)];
			dest[x + 1] = pens[((fgpix & 0xff00) != 0) ? (fgpix >> 8) : (bgpix >> 8)];
		}
	}
	else
	{
		/* mode 1: foreground is half background resolution in */
		/* X and supports two pages */
		uint16_t const *const srcfg = &m_vram_fg[(params->rowaddr << 7) & 0xff00];

		for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
		{
			uint16_t fgpix = srcfg[(coladdr >> 1) & 0xff] >> (8 * (coladdr & 1));
			uint16_t bgpix = srcbg[coladdr & 0xff];

			dest[x + 0] = pens[((fgpix & 0x00ff) != 0) ? (fgpix & 0xff) : (bgpix & 0xff)];
			dest[x + 1] = pens[((fgpix & 0x00ff) != 0) ? (fgpix & 0xff) : (bgpix >> 8)];
		}
	}

}



/*************************************
 *
 *  Shift register transfers
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(xtheball_state::to_shiftreg)
{
	if (address >= 0x01000000 && address <= 0x010fffff)
		memcpy(shiftreg, &m_vram_bg[(address & 0xff000) >> 4], 0x200);
	else if (address >= 0x02000000 && address <= 0x020fffff)
		memcpy(shiftreg, &m_vram_fg[(address & 0xff000) >> 4], 0x200);
	else
		logerror("%s:to_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(xtheball_state::from_shiftreg)
{
	if (address >= 0x01000000 && address <= 0x010fffff)
		memcpy(&m_vram_bg[(address & 0xff000) >> 4], shiftreg, 0x200);
	else if (address >= 0x02000000 && address <= 0x020fffff)
		memcpy(&m_vram_fg[(address & 0xff000) >> 4], shiftreg, 0x200);
	else
		logerror("%s:from_shiftreg(%08X)\n", machine().describe_context(), address);
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

void xtheball_state::foreground_mode_w(int state)
{
	m_foreground_mode = state;
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

uint16_t xtheball_state::analogx_r()
{
	return (m_analog_x->read() << 8) | 0x00ff;
}


uint16_t xtheball_state::analogy_watchdog_r()
{
	/* doubles as a watchdog address */
	if (!machine().side_effects_disabled())
		m_watchdog->watchdog_reset();

	return (m_analog_y->read() << 8) | 0x00ff;
}



/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void xtheball_state::main_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram().share("nvram");
	map(0x01000000, 0x010fffff).ram().share("vrabg");
	map(0x02000000, 0x020fffff).ram().share("vrafg");
	map(0x03000000, 0x030000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0x03020000, 0x0302005f).unmaprw(); // looks like a CRTC of some sort
	map(0x03040000, 0x0304007f).w("latch1", FUNC(hc259_device::write_d0)).umask16(0x00ff);
	map(0x03040080, 0x0304008f).portr("DSW");
	map(0x03040080, 0x030400ff).w("latch2", FUNC(hc259_device::write_d0)).umask16(0x00ff);
	map(0x03040100, 0x0304010f).r(FUNC(xtheball_state::analogx_r));
	map(0x03040110, 0x0304011f).portr("COIN1");
	map(0x03040130, 0x0304013f).portr("SERVICE2");
	map(0x03040140, 0x0304014f).portr("COIN3");
	map(0x03040150, 0x0304015f).portr("BUTTON1");
	map(0x03040160, 0x0304016f).portr("SERVICE");
	map(0x03040170, 0x0304017f).portr("SERVICE1");
	map(0x03040100, 0x0304017f).w("latch3", FUNC(hc259_device::write_d0)).umask16(0x00ff);
	map(0x03040180, 0x0304018f).r(FUNC(xtheball_state::analogy_watchdog_r)).nopw();
	map(0x03060000, 0x0306000f).w("dac", FUNC(dac_byte_interface::data_w)).umask16(0xff00);
	map(0x04000000, 0x057fffff).rom().region("user2", 0);
	map(0xfff80000, 0xffffffff).rom().region("user1", 0);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( xtheball )
	PORT_START("DSW")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0700, 0x0000, "Target Tickets")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0300, "6" )
	PORT_DIPSETTING(      0x0400, "7" )
	PORT_DIPSETTING(      0x0500, "8" )
	PORT_DIPSETTING(      0x0600, "9" )
	PORT_DIPSETTING(      0x0700, "10" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x1000, 0x1000, "Automatic 1 Ticket" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x1000, DEF_STR( On ))
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x2000, DEF_STR( On ))
	PORT_DIPNAME( 0x4000, 0x4000, "Game Tune" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x4000, DEF_STR( On ))
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x8000, DEF_STR( On ))

	/* service mode is funky:
	    hit F2 to enter bookkeeping mode; hit service1 (9) to exit
	    hold service 1 (9) and hit F2 to enter test mode; hit service2 (0) to exit
	*/
	PORT_START("COIN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTON1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_HIGH )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOGX")
	PORT_BIT( 0x00ff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("ANALOGY")
	PORT_BIT( 0x00ff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void xtheball_state::xtheball(machine_config &config)
{
	TMS34010(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &xtheball_state::main_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(10000000);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(xtheball_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(xtheball_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(xtheball_state::from_shiftreg));
	m_maincpu->set_periodic_int(FUNC(xtheball_state::irq1_line_hold), attotime::from_hz(15000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	hc259_device &latch1(HC259(config, "latch1"));
	latch1.q_out_cb<7>().set("ticket", FUNC(ticket_dispenser_device::motor_w));
	// Q4 = meter, Q6 = tickets, Q7 = tickets?

	hc259_device &latch2(HC259(config, "latch2"));
	latch2.q_out_cb<0>().set_output("led0"); // start lamp
	// Q1-Q7 = more lamps?

	hc259_device &latch3(HC259(config, "latch3"));
	latch3.q_out_cb<3>().set(FUNC(xtheball_state::foreground_mode_w));
	// Q3 = video foreground control?

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(100));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10000000, 640, 114, 626, 257, 24, 248);
	screen.set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_rgb32));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	ZN428E(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( xtheball )
	ROM_REGION16_LE( 0x10000, "user1", 0 )  /* 34010 code */
	ROM_LOAD16_BYTE( "aces18-lo.ic13",  0x000000, 0x8000, CRC(c0e80004) SHA1(d79c2e7301857148674fffad349c7a2a98fa1ee2) )
	ROM_LOAD16_BYTE( "aces18-hi.ic19",  0x000001, 0x8000, CRC(5a682f92) SHA1(ed77c02cbdcff9eac32760cee67e3a784efacac7) )

	ROM_REGION16_LE( 0x300000, "user2", 0 ) /* 34010 code */
	ROM_LOAD16_BYTE( "xtb-ic6.bin", 0x000000, 0x40000, CRC(a3cc01b8) SHA1(49d42bb17c314609f371df7d7ace57e54fdf6335) )
	ROM_LOAD16_BYTE( "xtb-ic7.bin", 0x000001, 0x40000, CRC(8dfa6c1b) SHA1(a32940b3f9501a44e1d1ef1628f8a64b32aa2183) )
	ROM_LOAD16_BYTE( "xtb-1l.ic8",  0x100000, 0x80000, CRC(df52c00f) SHA1(9a89d780ad394b55ce9540a5743bbe571543288f) )
	ROM_LOAD16_BYTE( "xtb-1h.ic9",  0x100001, 0x80000, CRC(9ce7785b) SHA1(9a002ba492cdc35391df2b55dfe90c55b7d48ad1) )
	ROM_LOAD16_BYTE( "xtb-2l.ic10", 0x200000, 0x80000, CRC(e2545a19) SHA1(c2fe5adf7a174cec6aad63baa1b92761ef21d5a4) )
	ROM_LOAD16_BYTE( "xtb-2h.ic11", 0x200001, 0x80000, CRC(50c27558) SHA1(ecfb7d918868d35a8cde45f7d04fdfc3ffc06328) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1991, xtheball, 0, xtheball, xtheball, xtheball_state, empty_init, ROT0, "Rare", "X the Ball", MACHINE_SUPPORTS_SAVE )
