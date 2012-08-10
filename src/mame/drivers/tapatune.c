/*

Tap-A-Tune
1994 CES?
---------

Driver by Mariusz Wojcieszek and Phil Bennett

Top board notable:

- Hitachi HD68HC000-12 68000 CPU (24 MHz crystal)
- Hitachi HD46505SP-2 CRTC
- 2x Sony CXK581000P-10L RAM
- 2x Mosel MS6264L-10PC RAM
- 2x Mosel MS62256L-10PC RAM
- rom0.u3 / rom1.u12 - 68000 program
- rom2.u4 / rom3.u13 / rom4.u5 / rom5.u14 - graphics

Bottom board notable:

- Zilog Z0840006PSC Z80 CPU (24 MHz crystal, clock unknown)
- BSMT2000 custom audio IC
- rom.u8 Z80 program
- arom1.u16 BSMT2000 samples
- 2 banks of 8-position DIP switches
- red/green/yellow LEDs
- many connectors for I/O

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/bsmt2000.h"
#include "machine/ticket.h"

class tapatune_state : public driver_device
{
public:
	tapatune_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_videoram(*this, "videoram") {}

	UINT8	m_paletteram[0x300];
	UINT16	m_palette_write_address;
	rgb_t	m_pens[0x100];

	required_shared_ptr<UINT16> m_videoram;

	UINT8	m_controls_mux;
	UINT8	m_z80_to_68k_index;
	UINT8	m_z80_to_68k_data;

	UINT8	m_68k_to_z80_index;
	UINT8	m_68k_to_z80_data;

	UINT16	m_bsmt_data;
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_READ16_MEMBER(read_from_z80);
	DECLARE_WRITE16_MEMBER(write_to_z80);
	DECLARE_READ16_MEMBER(irq_ack_r);
	DECLARE_READ8_MEMBER(sound_irq_clear);
	DECLARE_WRITE8_MEMBER(controls_mux);
	DECLARE_READ8_MEMBER(controls_r);
	DECLARE_WRITE8_MEMBER(write_index_to_68k);
	DECLARE_WRITE8_MEMBER(write_data_to_68k);
	DECLARE_READ8_MEMBER(read_index_from_68k);
	DECLARE_READ8_MEMBER(read_data_from_68k);
	DECLARE_READ8_MEMBER(bsmt_status_r);
	DECLARE_WRITE8_MEMBER(bsmt_data_lo_w);
	DECLARE_WRITE8_MEMBER(bsmt_data_hi_w);
	DECLARE_WRITE8_MEMBER(bsmt_reg_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
};

WRITE16_MEMBER(tapatune_state::palette_w)
{

	//logerror("Palette write: offset = %02x, data = %04x, mask = %04x\n", offset, data, mem_mask );
	switch(offset)
	{
		case 0: // address register
			m_palette_write_address = ((data >> 8) & 0xff) * 3;
			break;
		case 1: // palette data
			m_paletteram[m_palette_write_address++] = (data >> 8) & 0xff;
			break;
		case 2: // unknown?
			break;
	}
}

READ16_MEMBER(tapatune_state::read_from_z80)
{

	//logerror("Reading data from Z80: index = %02x, data = %02x\n", m_z80_to_68k_index, m_z80_to_68k_data );

	switch( offset )
	{
		case 0:
			return ((UINT16)m_z80_to_68k_data << 8) | (m_z80_to_68k_index);
		default:
			return 0;
	}
}

WRITE16_MEMBER(tapatune_state::write_to_z80)
{

	switch( offset )
	{
		case 0:
			//if ( (data >> 8) & 0xff )
			//  logerror("Command to Z80: %04x\n", data);
			m_68k_to_z80_index = data & 0xff;
			m_68k_to_z80_data = (data >> 8) & 0xff;
			cputag_set_input_line(machine(), "maincpu", 3, CLEAR_LINE);
			break;
		case 1:
			break;
	}
}

READ16_MEMBER(tapatune_state::irq_ack_r)
{
	cputag_set_input_line(machine(), "maincpu", 2, CLEAR_LINE);
	return 0;
}

static ADDRESS_MAP_START( tapatune_map, AS_PROGRAM, 16, tapatune_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM // program rom and graphics roms
	AM_RANGE(0x300000, 0x31ffff) AM_RAM AM_SHARE("videoram") // hardware video buffer
	AM_RANGE(0x320000, 0x327fff) AM_RAM // workram
	AM_RANGE(0x328000, 0x32ffff) AM_RAM
	AM_RANGE(0x330000, 0x337fff) AM_RAM // ram used as system video buffer
	AM_RANGE(0x338000, 0x33ffff) AM_RAM
	AM_RANGE(0x400000, 0x400003) AM_READWRITE(read_from_z80, write_to_z80)
	AM_RANGE(0x400010, 0x400011) AM_READ(irq_ack_r)
	AM_RANGE(0x600000, 0x600005) AM_WRITE(palette_w)
	AM_RANGE(0x800000, 0x800001) AM_DEVWRITE8("crtc", mc6845_device, address_w, 0xff00)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
ADDRESS_MAP_END

READ8_MEMBER(tapatune_state::sound_irq_clear)
{
	cputag_set_input_line(machine(), "soundcpu", 0, CLEAR_LINE);
	return 0;
}

WRITE8_MEMBER(tapatune_state::controls_mux)
{
	//logerror("Controls mux written with %02x\n", data);
	m_controls_mux = data;
}

READ8_MEMBER(tapatune_state::controls_r)
{
	switch( m_controls_mux )
	{
		case 0x07: return ioport("DSW1")->read();
		case 0x08: return ioport("DSW2")->read();
		case 0x09: return ioport("IN0")->read();
		default: return 0xff;
	}
}

WRITE8_MEMBER(tapatune_state::write_index_to_68k)
{
	m_z80_to_68k_index = data;
}

WRITE8_MEMBER(tapatune_state::write_data_to_68k)
{
	m_z80_to_68k_data = data;
	//logerror("Writing data from Z80: index = %02x, data = %02x\n", m_z80_to_68k_index, m_z80_to_68k_data );
	cputag_set_input_line(machine(), "maincpu", 3, ASSERT_LINE);
}

READ8_MEMBER(tapatune_state::read_index_from_68k)
{
	return m_68k_to_z80_index;
}

READ8_MEMBER(tapatune_state::read_data_from_68k)
{
	//if ( m_68k_to_z80_data != 0 )
	//  logerror("Load command from 68K: %02x %02x\n", m_68k_to_z80_index, m_68k_to_z80_data);
	return m_68k_to_z80_data;
}

READ8_MEMBER(tapatune_state::bsmt_status_r)
{
	bsmt2000_device *bsmt = machine().device<bsmt2000_device>("bsmt");
	return (bsmt->read_status() << 7) ^ 0x80;
}

WRITE8_MEMBER(tapatune_state::bsmt_data_lo_w)
{
	m_bsmt_data = (m_bsmt_data & 0xff00) | data;
}

WRITE8_MEMBER(tapatune_state::bsmt_data_hi_w)
{
	m_bsmt_data = (m_bsmt_data & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tapatune_state::bsmt_reg_w)
{
	bsmt2000_device *bsmt = machine().device<bsmt2000_device>("bsmt");


	//logerror("Writing BSMT reg: %02X data: %04X\n", data, m_bsmt_data);
	bsmt->write_reg(data);
	bsmt->write_data(m_bsmt_data);
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tapatune_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START ( sound_io_map, AS_IO, 8, tapatune_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(bsmt_data_lo_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(bsmt_data_hi_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(bsmt_reg_w)
	AM_RANGE(0x18, 0x18) AM_WRITE(controls_mux)
	AM_RANGE(0x20, 0x20) AM_READ(sound_irq_clear)
	AM_RANGE(0x28, 0x28) AM_READ(bsmt_status_r)
	AM_RANGE(0x30, 0x30) AM_READ(controls_r)
	AM_RANGE(0x38, 0x38) AM_READ_PORT("COINS")
	AM_RANGE(0x60, 0x60) AM_WRITE(write_index_to_68k)
	AM_RANGE(0x61, 0x61) AM_WRITE(write_data_to_68k)
	AM_RANGE(0x63, 0x63) AM_WRITENOP // leds? lamps?
	AM_RANGE(0x68, 0x68) AM_READ(read_index_from_68k)
	AM_RANGE(0x69, 0x69) AM_READ(read_data_from_68k)
	AM_RANGE(0x6b, 0x6b) AM_READ_PORT("BUTTONS")
ADDRESS_MAP_END


static INPUT_PORTS_START( tapatune )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Dispense" )
	PORT_DIPSETTING(    0x01, "Tickets" )
	PORT_DIPSETTING(    0x00, "Capsules" )
	PORT_DIPNAME( 0x02, 0x02, "Award per" )
	PORT_DIPSETTING(    0x02, "Game" )
	PORT_DIPSETTING(    0x00, "Tune" )
	PORT_DIPNAME( 0x1c, 0x1c, "Tickets per game/tune" )
	PORT_DIPSETTING(    0x1c, "Disabled" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x14, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x60, 0x60, "Tune bonus tickets" )
	PORT_DIPSETTING(    0x60, "Disabled" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )


	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SW2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SW3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // pink
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // purple
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // blue
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // d. green
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) // l. green
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) // yellow
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) // orange
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) // red

INPUT_PORTS_END

const pen_t* get_pens(tapatune_state *state)
{
	offs_t i;

	for (i = 0; i < 0x100; i++)
	{
		int r, g, b;

		r = state->m_paletteram[3 * i + 0];
		g = state->m_paletteram[3 * i + 1];
		b = state->m_paletteram[3 * i + 2];

		r = pal6bit(r);
		g = pal6bit(g);
		b = pal6bit(b);

		state->m_pens[i] = MAKE_RGB(r, g, b);
	}

	return state->m_pens;
}

static MC6845_BEGIN_UPDATE( begin_update )
{
	tapatune_state *state = device->machine().driver_data<tapatune_state>();
	/* create the pens */
	get_pens(state);

	return state->m_pens;
}

static MC6845_UPDATE_ROW( update_row )
{
	tapatune_state *state = device->machine().driver_data<tapatune_state>();
	UINT32 *dest = &bitmap.pix32(y);
	UINT16 x;

	pen_t *pens = (pen_t *)param;

	offs_t offs = (ma*2 + ra*0x40)*4;

	UINT8 *videoram = reinterpret_cast<UINT8 *>(state->m_videoram.target());
	for (x = 0; x < x_count*4; x++)
	{
		UINT8 pix = videoram[BYTE_XOR_BE(offs + x)];
		dest[2*x] = pens[((pix >> 4) & 0x0f)];
		dest[2*x + 1] = pens[(pix & 0x0f)];
	}
}

static VIDEO_START( tapatune )
{
}

WRITE_LINE_MEMBER(tapatune_state::crtc_vsync)
{
	cputag_set_input_line(machine(), "maincpu", 2, state ? ASSERT_LINE : CLEAR_LINE);
}

static const mc6845_interface h46505_intf =
{
	"screen",	/* screen we are acting on */
	5,			/* number of pixels per video memory address */
	begin_update,/* before pixel update callback */
	update_row, /* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_DRIVER_LINE_MEMBER(tapatune_state,crtc_vsync),	/* VSYNC callback */
	NULL		/* update address callback */
};


static MACHINE_CONFIG_START( tapatune, tapatune_state )

	MCFG_CPU_ADD("maincpu", M68000, 24000000/2 )
	MCFG_CPU_PROGRAM_MAP(tapatune_map)

	MCFG_CPU_ADD("soundcpu", Z80, 24000000/6 )
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_PERIODIC_INT(irq0_line_assert, 183)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)

	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(tapatune)

	MCFG_MC6845_ADD("crtc", H46505, 24000000/16, h46505_intf)	/* H46505 */

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("bsmt", BSMT2000, XTAL_24MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START(tapatune)
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE("rom1.u12", 0x000000, 0x80000, CRC(1d3ed3f9) SHA1(a42997dffcd4a3e83a5cec75d5bb0c295bd18a8d) )
	ROM_LOAD16_BYTE("rom0.u3",  0x000001, 0x80000, CRC(d76e5dec) SHA1(11b5fa6019e8b891550d37667aa156b113266b9b) )
	ROM_LOAD16_BYTE("rom3.u13", 0x100000, 0x80000, CRC(798e004a) SHA1(8e40ea7bf6e9a67d1c182c3b132a71d1334e1ae8) )
	ROM_LOAD16_BYTE("rom2.u4",  0x100001, 0x80000, CRC(3f073ff7) SHA1(6c34103c49c6c04dad51415037fa9a0c63d98cc2) )
	ROM_LOAD16_BYTE("rom5.u14", 0x280000, 0x40000, CRC(5d3b8765) SHA1(049a115eb28554cc3f5ca813441c42d6b834fc6f) )
	ROM_LOAD16_BYTE("rom4.u5",  0x280001, 0x40000, CRC(2a2eda6a) SHA1(ce86f0da2a41e23a842b3aa1659aad4817de333f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rom.u8", 0x0000, 0x10000, CRC(f5c571d7) SHA1(cb5ef3b2bce9a579b54678962082d0e2fc0f1cd9) )

	ROM_REGION(0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom1.u16",  0x000000, 0x80000,  CRC(e51696bc) SHA1(b002f8705ad1877f91a860dddb0ae16b2e73dd15) )
ROM_END

GAME(1994, tapatune, 0, tapatune, tapatune, driver_device, 0, ROT0, "Creative Electronics And Software", "Tap-a-Tune", GAME_NOT_WORKING | GAME_NO_SOUND )
