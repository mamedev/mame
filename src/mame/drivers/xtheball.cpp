// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    X the Ball

    driver by Aaron Giles

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "machine/ticket.h"
#include "machine/nvram.h"
#include "sound/dac.h"


class xtheball_state : public driver_device
{
public:
	xtheball_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_tlc34076(*this, "tlc34076"),
			m_vram_bg(*this, "vrabg"),
			m_vram_fg(*this, "vrafg"),
			m_analog_x(*this, "ANALOGX"),
			m_analog_y(*this, "ANALOGY") { }

	required_device<cpu_device> m_maincpu;
	required_device<tlc34076_device> m_tlc34076;

	required_shared_ptr<UINT16> m_vram_bg;
	required_shared_ptr<UINT16> m_vram_fg;

	required_ioport m_analog_x;
	required_ioport m_analog_y;

	UINT8 m_bitvals[32];

	DECLARE_WRITE16_MEMBER(bit_controls_w);
	DECLARE_READ16_MEMBER(analogx_r);
	DECLARE_READ16_MEMBER(analogy_watchdog_r);

	virtual void machine_start() override;

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
};


void xtheball_state::machine_start()
{
	save_item(NAME(m_bitvals));
}

/*************************************
 *
 *  Video update
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(xtheball_state::scanline_update)
{
	UINT16 *srcbg = &m_vram_bg[(params->rowaddr << 8) & 0xff00];
	UINT32 *dest = &bitmap.pix32(scanline);
	const rgb_t *pens = m_tlc34076->get_pens();
	int coladdr = params->coladdr;
	int x;

	/* bit value 0x13 controls which foreground mode to use */
	if (!m_bitvals[0x13])
	{
		/* mode 0: foreground is the same as background */
		UINT16 *srcfg = &m_vram_fg[(params->rowaddr << 8) & 0xff00];

		for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
		{
			UINT16 fgpix = srcfg[coladdr & 0xff];
			UINT16 bgpix = srcbg[coladdr & 0xff];

			dest[x + 0] = pens[((fgpix & 0x00ff) != 0) ? (fgpix & 0xff) : (bgpix & 0xff)];
			dest[x + 1] = pens[((fgpix & 0xff00) != 0) ? (fgpix >> 8) : (bgpix >> 8)];
		}
	}
	else
	{
		/* mode 1: foreground is half background resolution in */
		/* X and supports two pages */
		UINT16 *srcfg = &m_vram_fg[(params->rowaddr << 7) & 0xff00];

		for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
		{
			UINT16 fgpix = srcfg[(coladdr >> 1) & 0xff] >> (8 * (coladdr & 1));
			UINT16 bgpix = srcbg[coladdr & 0xff];

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
		memcpy(shiftreg, &m_vram_bg[TOWORD(address & 0xff000)], TOBYTE(0x1000));
	else if (address >= 0x02000000 && address <= 0x020fffff)
		memcpy(shiftreg, &m_vram_fg[TOWORD(address & 0xff000)], TOBYTE(0x1000));
	else
		logerror("%s:to_shiftreg(%08X)\n", space.machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(xtheball_state::from_shiftreg)
{
	if (address >= 0x01000000 && address <= 0x010fffff)
		memcpy(&m_vram_bg[TOWORD(address & 0xff000)], shiftreg, TOBYTE(0x1000));
	else if (address >= 0x02000000 && address <= 0x020fffff)
		memcpy(&m_vram_fg[TOWORD(address & 0xff000)], shiftreg, TOBYTE(0x1000));
	else
		logerror("%s:from_shiftreg(%08X)\n", space.machine().describe_context(), address);
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE16_MEMBER(xtheball_state::bit_controls_w)
{
	UINT8 *bitvals = m_bitvals;
	if (ACCESSING_BITS_0_7)
	{
		if (bitvals[offset] != (data & 1))
		{
			logerror("%08x:bit_controls_w(%x,%d)\n", space.device().safe_pc(), offset, data & 1);

			switch (offset)
			{
				case 7:
					machine().device<ticket_dispenser_device>("ticket")->write(space, 0, data << 7);
					break;

				case 8:
					output().set_led_value(0, data & 1);
					break;
			}
		}
		bitvals[offset] = data & 1;
	}
//  popmessage("%d%d%d%d-%d%d%d%d--%d%d%d%d-%d%d%d%d",
/*
        bitvals[0],
        bitvals[1],
        bitvals[2],
        bitvals[3],
        bitvals[4],     // meter
        bitvals[5],
        bitvals[6],     // tickets
        bitvals[7],     // tickets
        bitvals[8],     // start lamp
        bitvals[9],     // lamp
        bitvals[10],    // lamp
        bitvals[11],    // lamp
        bitvals[12],    // lamp
        bitvals[13],    // lamp
        bitvals[14],    // lamp
        bitvals[15]     // lamp
*/
/*      bitvals[16],
        bitvals[17],
        bitvals[18],
        bitvals[19],    // video foreground control
        bitvals[20],
        bitvals[21],
        bitvals[22],
        bitvals[23],
        bitvals[24],
        bitvals[25],
        bitvals[26],
        bitvals[27],
        bitvals[28],
        bitvals[29],
        bitvals[30],
        bitvals[31]
*/
//  );
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

READ16_MEMBER(xtheball_state::analogx_r)
{
	return (m_analog_x->read() << 8) | 0x00ff;
}


READ16_MEMBER(xtheball_state::analogy_watchdog_r)
{
	/* doubles as a watchdog address */
	watchdog_reset_w(space,0,0);
	return (m_analog_y->read() << 8) | 0x00ff;
}



/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, xtheball_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x01000000, 0x010fffff) AM_RAM AM_SHARE("vrabg")
	AM_RANGE(0x02000000, 0x020fffff) AM_RAM AM_SHARE("vrafg")
	AM_RANGE(0x03000000, 0x030000ff) AM_DEVREADWRITE8("tlc34076", tlc34076_device, read, write, 0x00ff)
	AM_RANGE(0x03040000, 0x030401ff) AM_WRITE(bit_controls_w)
	AM_RANGE(0x03040080, 0x0304008f) AM_READ_PORT("DSW")
	AM_RANGE(0x03040100, 0x0304010f) AM_READ(analogx_r)
	AM_RANGE(0x03040110, 0x0304011f) AM_READ_PORT("COIN1")
	AM_RANGE(0x03040130, 0x0304013f) AM_READ_PORT("SERVICE2")
	AM_RANGE(0x03040140, 0x0304014f) AM_READ_PORT("COIN3")
	AM_RANGE(0x03040150, 0x0304015f) AM_READ_PORT("BUTTON1")
	AM_RANGE(0x03040160, 0x0304016f) AM_READ_PORT("SERVICE")
	AM_RANGE(0x03040170, 0x0304017f) AM_READ_PORT("SERVICE1")
	AM_RANGE(0x03040180, 0x0304018f) AM_READ(analogy_watchdog_r)
	AM_RANGE(0x03060000, 0x0306000f) AM_DEVWRITE8("dac", dac_device, write_unsigned8, 0xff00)
	AM_RANGE(0x04000000, 0x057fffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("maincpu", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( xtheball )
	PORT_START("DSW")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
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

static MACHINE_CONFIG_START( xtheball, xtheball_state )

	MCFG_CPU_ADD("maincpu", TMS34010, 40000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(10000000) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(xtheball_state, scanline_update)     /* scanline updater (rgb32) */
	MCFG_TMS340X0_TO_SHIFTREG_CB(xtheball_state, to_shiftreg)  /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(xtheball_state, from_shiftreg) /* read from shiftreg function */
	MCFG_CPU_PERIODIC_INT_DRIVER(xtheball_state, irq1_line_hold,  15000)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(10000000, 640, 114, 626, 257, 24, 248)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", tms34010_device, tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



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



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1991, xtheball, 0, xtheball, xtheball, driver_device, 0,  ROT0, "Rare", "X the Ball", MACHINE_SUPPORTS_SAVE )
