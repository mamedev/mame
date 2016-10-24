// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  AMERICAN MUSIC POKER V1.4
  1987 AMUSCO.

  Preliminary driver by Roberto Fresca.

*******************************************************************************

  Hardware Notes:

  1x Empty 40 pin socket (U57)    Suspected 8085/8086/8088 Processor (due to the D8259AC presence).

  1x NEC D8259AC          Programmable Interrupt Controller (PIC).
  1x AMD D8284A           Clock Generator and Driver for 8086/8088 Processors.
  1x M5L8253P-5           Programmable general-purpose timer device.

  1x MOS 6845 (U27)       CRT Controller.
  2x P8255                Programmable Peripheral Interface (I/O).

  2x SRM2264        8k X 8 CMOS Static RAM.

  1x 27128 (U35) ROM.   Handwritten sticker:  Char A U35.
  1x 27128 (U36) ROM.   Handwritten sticker:  Char B U36.
  1x 27128 (U37) ROM.   Handwritten sticker:  Char C U37.
  1x 27256 (U42) ROM.   Handwritten sticker:  PK V1.4 U42.

  1x TI SN76489AN   Digital Complex Sound Generator (DCSG).

  3x MMI PAL16L8ACN (U47, U48, U50)
  1x MMI PAL16R4 (U49)    <-- couldn't get a consistent read

  22.1184 MHz. Crystal
  15.000  MHz. Crystal

  An unidentified but similar Amusco PCB auctioned on eBay had an AMD P8088 filling the
  CPU socket. Some of the other chips on this board were replaced with clones (e.g.
  AMD P8253, SY6545-1).

  The program code reads from and writes to what must be a line printer and a RTC
  (probably a MSM5832), though neither is present on the main board. The I/O write
  patterns also suggest that each or both of these devices are accessed through an
  unknown interface chip or gate array (not i8255-compatible). The printer appears to
  use non-Epson control codes: ETB, DLE, DC4, DC2, SO, EM, DC1 and STX.

*****************************************************************************************

  DRIVER UPDATES:

  [2014-03-14]

  - Initial release.
  - Decoded graphics.
  - Preliminary memory map.
  - Added the CRTC 6845.
  - Added the SN76489 PSG.
  - Added technical notes.


  TODO:

  - Proper tile colors.
  - Determine PIT clocks for proper IRQ timing.
  - Make the 6845 transparent videoram addressing actually transparent.
    (IRQ1 changes the 6845 address twice but neither reads nor writes data?)
  - Add NVRAM in a way that won't trigger POST error message (needs NMI on shutdown?)
  - Identify outputs from first PPI (these include button lamps?)

*******************************************************************************/


#define MASTER_CLOCK        XTAL_22_1184MHz     /* confirmed */
#define SECOND_CLOCK        XTAL_15MHz          /* confirmed */

#define CPU_CLOCK           MASTER_CLOCK / 4    /* guess */
#define CRTC_CLOCK          SECOND_CLOCK / 8    /* guess */
#define SND_CLOCK           SECOND_CLOCK / 8    /* guess */

#include "emu.h"
#include "cpu/i86/i86.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/msm5832.h"


class amusco_state : public driver_device
{
public:
	amusco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_pic(*this, "pic8259"),
		m_rtc(*this, "rtc"),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen")
		{ }

	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_bg_tilemap;
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	virtual void machine_start() override;
	uint8_t hack_coin1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hack_coin2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hack_908_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mc6845_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mc6845_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void output_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void output_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void output_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lpt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lpt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rtc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rtc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	MC6845_UPDATE_ROW(update_row);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<pic8259_device> m_pic;
	required_device<msm5832_device> m_rtc;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	uint8_t m_mc6845_address;
	uint16_t m_video_update_address;
	uint8_t m_rtc_data;
};


/*************************
*     Video Hardware     *
*************************/

void amusco_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
/*  - bits -
    7654 3210
    ---- ----   bank select.
    ---- ----   color code.
    ---- ----   seems unused.
*/
	int code = m_videoram[tile_index * 2] | (m_videoram[tile_index * 2 + 1] << 8);
	int color = (code & 0x7000) >> 12;

	// 6845 cursor is only used for its blink state
	if (BIT(code, 15) && !m_crtc->cursor_state_r())
		code = 0;

	SET_TILE_INFO_MEMBER(
							0 /* bank */,
							code & 0x3ff,
							color,
							0
						);
}

void amusco_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(amusco_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 10, 74, 24);
}

void amusco_state::machine_start()
{
	m_rtc_data = 0;
}


/**************************
*  Read / Write Handlers  *
**************************/

uint8_t amusco_state::hack_coin1_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	// actually set by IRQ4
	return BIT(ioport("IN2")->read(), 1) ? 1 : 0;
}

uint8_t amusco_state::hack_coin2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	// actually set by IRQ4
	return BIT(ioport("IN2")->read(), 2) ? 1 : 0;
}

uint8_t amusco_state::hack_908_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	// IRQ4 and other routines wait for bits of this to be set by IRQ2
	return 0xff;
}

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( amusco_mem_map, AS_PROGRAM, 8, amusco_state )
	AM_RANGE(0x006a6, 0x006a6) AM_READ(hack_coin1_r)
	AM_RANGE(0x006a8, 0x006a8) AM_READ(hack_coin2_r)
	AM_RANGE(0x00908, 0x00908) AM_READ(hack_908_r)
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
	AM_RANGE(0xec000, 0xecfff) AM_RAM AM_SHARE("videoram")  // placeholder
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END

uint8_t amusco_state::mc6845_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if(offset & 1)
		return m_crtc->register_r(space, 0);

	return m_crtc->status_r(space,0); // not a plain 6845, requests update bit here ...
}

void amusco_state::mc6845_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if(offset & 1)
	{
		m_crtc->register_w(space, 0,data);
		if(m_mc6845_address == 0x12)
			m_video_update_address = ((data & 0xff) << 8) | (m_video_update_address & 0x00ff);
		if(m_mc6845_address == 0x13)
			m_video_update_address = ((data & 0xff) << 0) | (m_video_update_address & 0xff00);
	}
	else
	{
		m_crtc->address_w(space,0,data);
		m_mc6845_address = data;
	}
}

void amusco_state::output_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("Writing %02Xh to PPI output A\n", data);
}

void amusco_state::output_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("Writing %02Xh to PPI output B\n", data);
}

void amusco_state::output_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("Writing %02Xh to PPI output C\n", data);
}

void amusco_state::vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[m_video_update_address * 2 + offset] = data;
	m_bg_tilemap->mark_tile_dirty(m_video_update_address);
//  printf("%04x %04x\n",m_video_update_address,data);
}

uint8_t amusco_state::lpt_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	switch (offset)
	{
		case 2:
			// Bit 0 = busy
			// Bit 1 = paper jam (inverted)
			// Bit 3 = out of paper
			// Bit 4 = low paper
			return 2;

		default:
			logerror("Reading from printer port 28%dh\n", offset);
			return 0;
	}
}

void amusco_state::lpt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	logerror("Writing %02Xh to printer port 28%dh\n", data, offset);
}

uint8_t amusco_state::rtc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	switch (offset)
	{
		case 3:
			return m_rtc->data_r(space, 0);

		default:
			logerror("Reading from RTC port 38%dh\n", offset);
			return 0;
	}
}

void amusco_state::rtc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch (offset)
	{
		case 1:
			m_rtc->address_w(data & 0x0f);
			m_rtc->cs_w(BIT(data, 6));
			m_rtc->hold_w(BIT(data, 6));
			m_rtc->write_w(BIT(data, 5));
			if (BIT(data, 5))
				m_rtc->data_w(space, 0, m_rtc_data);
			m_rtc->read_w(BIT(data, 4));
			break;

		case 3:
			m_rtc_data = data;
			break;

		default:
			logerror("Writing %02Xh to RTC port 38%dh\n", data, offset);
			break;
	}
}

static ADDRESS_MAP_START( amusco_io_map, AS_IO, 8, amusco_state )
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(mc6845_r, mc6845_w)
	AM_RANGE(0x0010, 0x0011) AM_DEVWRITE("pic8259", pic8259_device, write)
	AM_RANGE(0x0020, 0x0023) AM_DEVWRITE("pit8253", pit8253_device, write)
	AM_RANGE(0x0030, 0x0033) AM_DEVREADWRITE("ppi_outputs", i8255_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("ppi_inputs", i8255_device, read, write)
	AM_RANGE(0x0060, 0x0060) AM_DEVWRITE("sn", sn76489a_device, write)
	AM_RANGE(0x0070, 0x0071) AM_WRITE(vram_w)
	AM_RANGE(0x0280, 0x0283) AM_READWRITE(lpt_r, lpt_w)
	AM_RANGE(0x0380, 0x0383) AM_READWRITE(rtc_r, rtc_w)
ADDRESS_MAP_END

/* I/O byte R/W

  0000 writes CRTC register (high nibble)    screen size: 88*8 27*10  -  visible scr: 74*8 24*10
  0000 writes CRTC address (low nibble)      reg values: 57, 4a, 4b, 0b, 1a, 08, 18, 19, 48, 09, 40, 00, 00, 00, 00, 00, 00.

   -----------------

   unknown writes:


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( amusco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play Credit")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // actual name uncertain; next screen in service mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // decrement in service mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw") // previous screen in service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) // actual name uncertain; increment in service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) // move left in service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // move right in service mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Cash Door")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Logic Door")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // move down in service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) // move up in service mode

	PORT_START("IN2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) //PORT_WRITE_LINE_DEVICE_MEMBER("pic8259", pic8259_device, ir4_w)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) //PORT_WRITE_LINE_DEVICE_MEMBER("pic8259", pic8259_device, ir4_w)
	PORT_BIT( 0xf9, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 10,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	16*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( amusco )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, /*8*/ 1 ) // current palette has only 8 colors...
GFXDECODE_END


/***********************
*    CRTC Interface    *
************************/

MC6845_ON_UPDATE_ADDR_CHANGED(amusco_state::crtc_addr)
{
//  m_video_update_address = address;
}

MC6845_UPDATE_ROW(amusco_state::update_row)
{
	const rectangle rowrect(0, 8 * x_count - 1, y, y);
	m_bg_tilemap->mark_all_dirty();
	m_bg_tilemap->draw(*m_screen, bitmap, rowrect, 0, 0);
}

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( amusco, amusco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, CPU_CLOCK)        // 5 MHz ?
	MCFG_CPU_PROGRAM_MAP(amusco_mem_map)
	MCFG_CPU_IO_MAP(amusco_io_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NOOP)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	//MCFG_PIT8253_CLK0(nnn)
	//MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	//MCFG_PIT8253_CLK1(nnn)
	//MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir2_w))

	MCFG_DEVICE_ADD("ppi_outputs", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(amusco_state, output_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(amusco_state, output_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(amusco_state, output_c_w))

	MCFG_DEVICE_ADD("ppi_inputs", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_MSM5832_ADD("rtc", XTAL_32_768kHz)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(88*8, 27*10)                           // screen size: 88*8 27*10
	MCFG_SCREEN_VISIBLE_AREA(0*8, 74*8-1, 0*10, 24*10-1)    // visible scr: 74*8 24*10
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", amusco)

	MCFG_PALETTE_ADD_3BIT_GBR("palette")

	MCFG_MC6845_ADD("crtc", R6545_1, "screen", CRTC_CLOCK) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_ADDR_CHANGED_CB(amusco_state, crtc_addr)
	MCFG_MC6845_OUT_DE_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w)) // IRQ1 sets 0x918 bit 3
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w)) // IRQ0 sets 0x665 to 0xff
	MCFG_MC6845_UPDATE_ROW_CB(amusco_state, update_row)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn", SN76489A, SND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( amusco )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "pk_v1.4_u42.u42",  0xf8000, 0x08000, CRC(bf57d7b1) SHA1(fc8b062b12c241c6c096325f728305316b80be8b) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "char_a_u35.u35",  0x0000, 0x4000, CRC(ded67ef6) SHA1(da7326c190211e956e5a5f763d5045615bb8ffb3) )
	ROM_LOAD( "char_b_u36.u36",  0x4000, 0x4000, CRC(55523513) SHA1(97fd221c298698628d4f6564389d96eb89e55927) )
	ROM_LOAD( "char_c_u37.u37",  0x8000, 0x4000, CRC(d26f3b94) SHA1(e58af4f6f1a9091c23827997d03b91f02bb07856) )
//  ROM_LOAD( "char_a_u35.u35",  0x8000, 0x4000, CRC(ded67ef6) SHA1(da7326c190211e956e5a5f763d5045615bb8ffb3) )
//  ROM_LOAD( "char_b_u36.u36",  0x4000, 0x4000, CRC(55523513) SHA1(97fd221c298698628d4f6564389d96eb89e55927) )
//  ROM_LOAD( "char_c_u37.u37",  0x0000, 0x4000, CRC(d26f3b94) SHA1(e58af4f6f1a9091c23827997d03b91f02bb07856) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8a.u47", 0x0000, 0x0104, CRC(554b4286) SHA1(26bc991f2cc58644cd2d9ce5c1867a94455b95a8) )
	ROM_LOAD( "pal16l8a.u48", 0x0200, 0x0104, CRC(d8d1fb4b) SHA1(7a722420324d7efbe500279cbff6e08b7eeb4f22) )
	ROM_LOAD( "pal16r4a.u49", 0x0400, 0x0104, CRC(97813a68) SHA1(be4c7f2d38b7c5eec13dd803b78293d8e5f1c2ff) )
	ROM_LOAD( "pal16l8a.u50", 0x0600, 0x0104, CRC(f5d80001) SHA1(ba0e55ebb45eceec256d432aee6d4123365a0af2) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT  ROT    COMPANY   FULLNAME                      FLAGS */
GAME( 1987, amusco,   0,      amusco,   amusco,   driver_device, 0,    ROT0, "Amusco", "American Music Poker (V1.4)", MACHINE_IMPERFECT_COLORS | MACHINE_NODEVICE_PRINTER ) // runs much too fast; palette totally wrong
