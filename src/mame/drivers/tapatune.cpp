// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Philip Bennett
/***************************************************************************

    Tap a Tune

    driver by Mariusz Wojcieszek and Phil Bennett


    PCB Notes:

    Top board notable (video)

    - Hitachi HD68HC000-12 68000 CPU (24 MHz crystal)
    - Hitachi HD46505SP-2 CRTC
    - 2x Sony CXK581000P-10L RAM
    - 2x Mosel MS6264L-10PC RAM
    - 2x Mosel MS62256L-10PC RAM
    - rom0.u3 / rom1.u12 - 68000 program
    - rom2.u4 / rom3.u13 / rom4.u5 / rom5.u14 - graphics

    Bottom board notable (main / sound / io)

    - Zilog Z0840006PSC Z80 CPU (24 MHz crystal)
    - BSMT2000 custom audio IC
    - rom.u8 Z80 program
    - arom1.u16 BSMT2000 samples
    - 2 banks of 8-position DIP switches
    - red/green/yellow LEDs
    - many connectors for I/O

    The sound and I/O board is used by other redemption games such as
    Colorama and Wheel 'Em In, Super Rock and Bowl

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/bsmt2000.h"
#include "video/mc6845.h"


/*************************************
 *
 *  Driver state
 *
 *************************************/

class tapatune_state : public driver_device
{
public:
	tapatune_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videocpu(*this, "videocpu"),
		m_bsmt(*this, "bsmt"),
		m_videoram(*this, "videoram") {}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_videocpu;
	required_device<bsmt2000_device> m_bsmt;

	optional_shared_ptr<UINT16> m_videoram;

	UINT8   m_paletteram[0x300];
	UINT16  m_palette_write_addr;
	rgb_t   m_pens[0x100];
	UINT8   m_controls_mux;
	UINT8   m_z80_to_68k_index;
	UINT8   m_z80_to_68k_data;
	UINT8   m_68k_to_z80_index;
	UINT8   m_68k_to_z80_data;
	UINT8   m_z80_data_available;
	UINT8   m_68k_data_available;
	UINT8   m_bsmt_data_l;
	UINT8   m_bsmt_data_h;
	bool    m_bsmt_reset;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);

	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_READ16_MEMBER(read_from_z80);
	DECLARE_WRITE16_MEMBER(write_to_z80);

	DECLARE_READ8_MEMBER(sound_irq_clear);
	DECLARE_WRITE8_MEMBER(controls_mux);
	DECLARE_READ8_MEMBER(controls_r);
	DECLARE_WRITE8_MEMBER(write_index_to_68k);
	DECLARE_WRITE8_MEMBER(write_data_to_68k);
	DECLARE_READ8_MEMBER(read_index_from_68k);
	DECLARE_READ8_MEMBER(read_data_from_68k);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(bsmt_data_lo_w);
	DECLARE_WRITE8_MEMBER(bsmt_data_hi_w);
	DECLARE_WRITE8_MEMBER(bsmt_reg_w);
	DECLARE_READ8_MEMBER(special_r);

	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
};


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void tapatune_state::machine_start()
{
	save_item(NAME(m_paletteram));
	save_item(NAME(m_palette_write_addr));
	save_item(NAME(m_pens));
	save_item(NAME(m_controls_mux));
	save_item(NAME(m_z80_to_68k_index));
	save_item(NAME(m_z80_to_68k_data));
	save_item(NAME(m_68k_to_z80_index));
	save_item(NAME(m_68k_to_z80_data));
	save_item(NAME(m_z80_data_available));
	save_item(NAME(m_68k_data_available));
	save_item(NAME(m_bsmt_data_l));
	save_item(NAME(m_bsmt_data_h));
	save_item(NAME(m_bsmt_reset));
}


void tapatune_state::machine_reset()
{
	// The BSMT2000 is held in reset until the Z80 writes to P1
	m_bsmt_reset = true;
	m_z80_data_available = 0;
	m_68k_data_available = 0;
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

MC6845_BEGIN_UPDATE( tapatune_state::crtc_begin_update )
{
	// Create the pens
	for (UINT32 i = 0; i < 0x100; i++)
	{
		int r = m_paletteram[3 * i + 0];
		int g = m_paletteram[3 * i + 1];
		int b = m_paletteram[3 * i + 2];

		r = pal6bit(r);
		g = pal6bit(g);
		b = pal6bit(b);

		m_pens[i] = rgb_t(r, g, b);
	}
}


MC6845_UPDATE_ROW( tapatune_state::crtc_update_row )
{
	UINT32 *dest = &bitmap.pix32(y);
	offs_t offs = (ma*2 + ra*0x40)*4;

	UINT8 *videoram = reinterpret_cast<UINT8 *>(m_videoram.target());

	for (UINT32 x = 0; x < x_count*4; x++)
	{
		UINT8 pix = videoram[BYTE_XOR_BE(offs + x)];
		dest[2*x] = m_pens[((pix >> 4) & 0x0f)];
		dest[2*x + 1] = m_pens[(pix & 0x0f)];
	}
}


WRITE16_MEMBER(tapatune_state::palette_w)
{
	//logerror("Palette write: offset = %02x, data = %04x, mask = %04x\n", offset, data, mem_mask );
	switch(offset)
	{
		case 0: // address register
			m_palette_write_addr = ((data >> 8) & 0xff) * 3;
			break;
		case 1: // palette data
			m_paletteram[m_palette_write_addr++] = (data >> 8) & 0xff;
			break;
		case 2: // unknown?
			break;
	}
}


WRITE_LINE_MEMBER(tapatune_state::crtc_vsync)
{
	m_videocpu->set_input_line(2, state ? HOLD_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  68000 <-> Z80 comms
 *
 *************************************/

READ16_MEMBER(tapatune_state::read_from_z80)
{
	m_z80_data_available = 0;
	return ((UINT16)m_z80_to_68k_data << 8) | (m_z80_to_68k_index);
}


WRITE16_MEMBER(tapatune_state::write_to_z80)
{
	m_68k_to_z80_index = data & 0xff;
	m_68k_to_z80_data = (data >> 8) & 0xff;

	if (m_68k_data_available)
	{
		fatalerror("68000 overwrote old Z80 data");
	}
	m_68k_data_available = 1;
}


/*************************************
 *
 *  Z80 <-> 68000 comms
 *
 *************************************/

WRITE8_MEMBER(tapatune_state::write_index_to_68k)
{
	m_z80_to_68k_index = data;
}


WRITE8_MEMBER(tapatune_state::write_data_to_68k)
{
	// todo, use callback as this will hook up elsewhere on non-video games
	if (m_videocpu)
	{
		m_z80_to_68k_data = data;
		m_z80_data_available = 1;
		m_videocpu->set_input_line(1, HOLD_LINE);
	}
}


READ8_MEMBER(tapatune_state::read_index_from_68k)
{
	return m_68k_to_z80_index;
}


READ8_MEMBER(tapatune_state::read_data_from_68k)
{
	m_68k_data_available = 0;

	return m_68k_to_z80_data;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( video_map, AS_PROGRAM, 16, tapatune_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
	AM_RANGE(0x300000, 0x31ffff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x320000, 0x33ffff) AM_RAM
	AM_RANGE(0x400000, 0x400003) AM_READWRITE(read_from_z80, write_to_z80)
	AM_RANGE(0x400010, 0x400011) AM_NOP // Watchdog?
	AM_RANGE(0x600000, 0x600005) AM_WRITE(palette_w)
	AM_RANGE(0x800000, 0x800001) AM_DEVWRITE8("crtc", mc6845_device, address_w, 0xff00)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
ADDRESS_MAP_END


static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, tapatune_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


static ADDRESS_MAP_START ( maincpu_io_map, AS_IO, 8, tapatune_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(bsmt_data_lo_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(bsmt_data_hi_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(bsmt_reg_w)
	AM_RANGE(0x18, 0x18) AM_WRITE(controls_mux)
	AM_RANGE(0x20, 0x20) AM_READ(sound_irq_clear)
	AM_RANGE(0x28, 0x28) AM_READ(status_r)
	AM_RANGE(0x30, 0x30) AM_READ(controls_r)
	AM_RANGE(0x38, 0x38) AM_READ_PORT("COINS")
	AM_RANGE(0x60, 0x60) AM_WRITE(write_index_to_68k)
	AM_RANGE(0x61, 0x61) AM_WRITE(write_data_to_68k)
	AM_RANGE(0x63, 0x63) AM_WRITE(lamps_w)
	AM_RANGE(0x68, 0x68) AM_READ(read_index_from_68k)
	AM_RANGE(0x69, 0x69) AM_READ(read_data_from_68k)
	AM_RANGE(0x6b, 0x6b) AM_READ(special_r)
ADDRESS_MAP_END


/*************************************
 *
 *  I/O
 *
 *************************************/

READ8_MEMBER(tapatune_state::sound_irq_clear)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0;
}


WRITE8_MEMBER(tapatune_state::controls_mux)
{
	/*
	    Input multiplexer select and outputs:

	    76543210
	    .......x    Mux A
	    ......x.    Mux B (/Red LED)
	    .....x..    Mux C (/Yellow LED)
	    ....x...    Mux D (/Green LED)
	    ...x....    DOUT1 - High current driver 0
	    ..x.....    DOUT2 - High current driver 1
	    .x......    DOUT3 - High current driver 2
	    x.......    DOUT4 - Ticket dispenser
	*/

	m_controls_mux = data;
}


READ8_MEMBER(tapatune_state::controls_r)
{
	switch (m_controls_mux & 0xf)
	{
		case 0x07: return ioport("SW4")->read();
		case 0x08: return ioport("SW5")->read();
		case 0x09: return ioport("IN0")->read();
		default:
			return 0xff;
	}
}


READ8_MEMBER(tapatune_state::special_r)
{
	// Not sure if this is actually correct
	if (m_z80_data_available)
		return m_68k_data_available ? 0x80 : 0;
	else
		return ioport("BUTTONS")->read();
}


WRITE8_MEMBER(tapatune_state::lamps_w)
{
	/*
	    Button Lamps:

	    7654 3210
	    .... ...x   Pink
	    .... ..x.   Purple
	    .... .x..   Blue
	    .... x...   Dark Green
	    ...x ....   Light Green
	    ..x. ....   Yellow
	    .x.. ....   Orange
	    x... ....   Red
	*/
}


/*************************************
 *
 *  BSMT communications
 *
 *************************************/

READ8_MEMBER(tapatune_state::status_r)
{
	return !m_bsmt->read_status() << 7;
}


WRITE8_MEMBER(tapatune_state::bsmt_data_lo_w)
{
	m_bsmt_data_l = data;
}


WRITE8_MEMBER(tapatune_state::bsmt_data_hi_w)
{
	m_bsmt_data_h = data;

	if (m_bsmt_reset)
	{
		m_bsmt->reset();
		m_bsmt_reset = false;
	}
}


WRITE8_MEMBER(tapatune_state::bsmt_reg_w)
{
	m_bsmt->write_reg(data);
	m_bsmt->write_data((m_bsmt_data_h << 8) | m_bsmt_data_l);
}


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( tapatune )
	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x01, "Dispense" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "Tickets" )
	PORT_DIPSETTING(    0x00, "Capsules" )
	PORT_DIPNAME( 0x02, 0x02, "Award per" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, "Game" )
	PORT_DIPSETTING(    0x00, "Tune" )
	PORT_DIPNAME( 0x1c, 0x1c, "Tickets/capsules per game/tune" ) PORT_DIPLOCATION("SW4:3,4,5")
	PORT_DIPSETTING(    0x1c, "Disabled" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x14, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x60, 0x60, "Tune bonus tickets/capsules" ) PORT_DIPLOCATION("SW4:6,7")
	PORT_DIPSETTING(    0x60, "Disabled" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW5")
	PORT_DIPNAME( 0x07, 0x03, "Tune Timer" ) PORT_DIPLOCATION("SW5:1,2,3")
	PORT_DIPSETTING(    0x07, "5 secs" )
	PORT_DIPSETTING(    0x06, "10 secs" )
	PORT_DIPSETTING(    0x05, "15 secs" )
	PORT_DIPSETTING(    0x04, "20 secs" )
	PORT_DIPSETTING(    0x03, "25 secs" )
	PORT_DIPSETTING(    0x02, "30 secs" )
	PORT_DIPSETTING(    0x01, "35 secs" )
	PORT_DIPSETTING(    0x00, "40 secs" )
	PORT_DIPNAME( 0x18, 0x10, "Bad note penalty" ) PORT_DIPLOCATION("SW5:4,5")
	PORT_DIPSETTING(    0x18, "0 secs" )
	PORT_DIPSETTING(    0x10, "1 secs" )
	PORT_DIPSETTING(    0x08, "5 secs" )
	PORT_DIPSETTING(    0x00, "10 secs" )
	PORT_DIPNAME( 0x20, 0x20, "Coins per game" ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Burn in Mode" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Advance (SW3)") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service (SW2)")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pink")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Purple")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Blue")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Dark Green")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Light Green")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Yellow")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Orange")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Red")
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( tapatune_base, tapatune_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz / 4)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_CPU_IO_MAP(maincpu_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(tapatune_state, irq0_line_assert, XTAL_24MHz / 4 / 4 / 4096)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("bsmt", BSMT2000, XTAL_24MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tapatune, tapatune_base )
	MCFG_CPU_ADD("videocpu", M68000, XTAL_24MHz / 2)
	MCFG_CPU_PROGRAM_MAP(video_map)

	MCFG_QUANTUM_PERFECT_CPU("videocpu")

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_24MHz / 16)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(5)
	MCFG_MC6845_BEGIN_UPDATE_CB(tapatune_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(tapatune_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(tapatune_state, crtc_vsync))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz / 16 * 5, 500, 0, 320, 250, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tapatune )
	ROM_REGION( 0x300000, "videocpu", 0 )
	ROM_LOAD16_BYTE("rom1.u12", 0x000000, 0x80000, CRC(1d3ed3f9) SHA1(a42997dffcd4a3e83a5cec75d5bb0c295bd18a8d) )
	ROM_LOAD16_BYTE("rom0.u3",  0x000001, 0x80000, CRC(d76e5dec) SHA1(11b5fa6019e8b891550d37667aa156b113266b9b) )
	ROM_LOAD16_BYTE("rom3.u13", 0x100000, 0x80000, CRC(798e004a) SHA1(8e40ea7bf6e9a67d1c182c3b132a71d1334e1ae8) )
	ROM_LOAD16_BYTE("rom2.u4",  0x100001, 0x80000, CRC(3f073ff7) SHA1(6c34103c49c6c04dad51415037fa9a0c63d98cc2) )
	ROM_LOAD16_BYTE("rom5.u14", 0x280000, 0x40000, CRC(5d3b8765) SHA1(049a115eb28554cc3f5ca813441c42d6b834fc6f) )
	ROM_LOAD16_BYTE("rom4.u5",  0x280001, 0x40000, CRC(2a2eda6a) SHA1(ce86f0da2a41e23a842b3aa1659aad4817de333f) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.u8", 0x0000, 0x10000, CRC(f5c571d7) SHA1(cb5ef3b2bce9a579b54678962082d0e2fc0f1cd9) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom1.u16",  0x000000, 0x20000, CRC(e51696bc) SHA1(b002f8705ad1877f91a860dddb0ae16b2e73dd15) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated
ROM_END

ROM_START( srockbwl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super_rnb.u8", 0x0000, 0x10000, CRC(ad264e73) SHA1(9f950e7e8ac02619d86d6144e187eddbe299580d) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "super_rnb_v1.1_4-7-94.u16",  0x000000, 0x20000, CRC(2fe5d1e2) SHA1(5ca18bf369a3021d84a776ce62db44c498119fc9) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated
ROM_END


/*

Smart Toss 'Em

romset was marked "Smart Toss 'em"

one of the roms contains the string

SMART INDUSTRIES SMARTBALL   V2.0

a reference to Smart Toss 'em can be found at
http://www.museumofplay.org/online-collections/22/67/109.17072

also contains

"Creative Electronics Software"

*/


ROM_START( smartoss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s-tossem.u8", 0x0000, 0x10000,CRC(ce07c837) SHA1(5a474cc9d3163385a3dac6da935bc77af67ac85a) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "s-tossem.u16",  0x000000, 0x20000, CRC(633222dc) SHA1(f95713902920737c9d20aafbdeb975daf98c7078) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated?

	// for a different sub-board maybe? or banked CPU data?
	ROM_REGION( 0x40000, "unkdata", 0 )
	ROM_LOAD( "s-tossem.u27", 0x0000, 0x40000, CRC(703d19b7) SHA1(75641d885885a67bd66afa38577c7907fa505b0b) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME(1994, tapatune, 0, tapatune,      tapatune, driver_device, 0, ROT0, "Moloney Manufacturing Inc. / Creative Electronics and Software", "Tap a Tune", MACHINE_SUPPORTS_SAVE )

// below appear to be mechanical games with the same Z80 board as the above
GAME(1994, srockbwl, 0, tapatune_base, tapatune, driver_device, 0, ROT0, "Bromley",                                                        "Super Rock and Bowl (V1.1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(199?, smartoss, 0, tapatune_base, tapatune, driver_device, 0, ROT0, "Smart Industries / Creative Electronics and Software",           "Smart Toss 'em / Smartball (Ver 2.0)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
