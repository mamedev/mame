/***************************************************************************

    Skeleton driver for Applix 1616 computer

    See for docs: http;//www.microbee-mspp.org.au
    You need to sign up and make an introductory thread.
    Then you will be granted permission to visit the repository.

    First revealed to the world in December 1986 issue of Electronics Today
    International (ETI) an Australian electronics magazine which is now defunct.

    The main articles appeared in ETI February/March/April 1987, followed by
    other articles in various issues after that.

    TODO: everything!
    - Required device Z8530 Z80SCC (not emulated, but it is just a dual-channel
      serial controller, possibly just like the other ones)
    - Keyboard is a standard pc keyboard
    - Sound is stereo dac-sound, plus an analog output. Details unknown.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mc6845.h"
#include "machine/6522via.h"


class applix_state : public driver_device
{
public:
	applix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_via(*this, "via6522"),
		m_base(*this, "base"),
		m_expansion(*this, "expansion"){ }

	DECLARE_READ16_MEMBER(applix_inputs_r);
	DECLARE_WRITE16_MEMBER(applix_index_w);
	DECLARE_WRITE16_MEMBER(applix_register_w);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_WRITE16_MEMBER(analog_latch_w);
	DECLARE_WRITE16_MEMBER(dac_latch_w);
	DECLARE_WRITE16_MEMBER(video_latch_w);
	DECLARE_READ8_MEMBER(applix_pa_r);
	DECLARE_READ8_MEMBER(applix_pb_r);
	DECLARE_WRITE8_MEMBER(applix_pa_w);
	DECLARE_WRITE8_MEMBER(applix_pb_w);
	DECLARE_WRITE_LINE_MEMBER(vsync_w);
	UINT8 m_pa;
	UINT8 m_pb;
	UINT8 m_analog_latch;
	UINT8 m_dac_latch;
	UINT8 m_video_latch;
	UINT8 m_palette_latch[4];
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<via6522_device> m_via;
	required_shared_ptr<UINT16> m_base;
	required_shared_ptr<UINT16> m_expansion;
};

// dac-latch seems to be:
// bit 7 cassette relay, low=on
// bit 6,5,4 sound outputs
// bit 3 cassette LED, low=on
// bit 2,1,0 joystick
WRITE16_MEMBER( applix_state::dac_latch_w )
{//printf("%X ",data);
	if (ACCESSING_BITS_0_7)
		m_dac_latch = data;
}

WRITE16_MEMBER( applix_state::analog_latch_w )
{//printf("%X ",data);
	if (ACCESSING_BITS_0_7)
		m_analog_latch = data;
}

//cent = odd, video = even
WRITE16_MEMBER( applix_state::palette_w )
{
	offset >>= 4;
	if (ACCESSING_BITS_0_7)
		{} //centronics
	else
		m_palette_latch[offset] = (data >> 8) & 15;
}

WRITE16_MEMBER( applix_state::video_latch_w )
{//printf("%X ",data);
	if (ACCESSING_BITS_0_7)
		m_video_latch = data;
}

WRITE16_MEMBER( applix_state::applix_index_w )
{
	if (ACCESSING_BITS_8_15)
		m_crtc->address_w( space, offset, data >> 8 );
}

WRITE16_MEMBER( applix_state::applix_register_w )
{
	if (ACCESSING_BITS_8_15)
		m_crtc->register_w( space, offset, data >> 8 );
}

READ16_MEMBER( applix_state::applix_inputs_r )
{// as far as i can tell:
// bits 4,5,6,7 are SW0,1,2,3
// bits 2,3 joystick in
// bit 1 cassette in
// bit 0 something to do with audio
	return ioport("DSW")->read();
// set dips to Off,Off,Off,On for a video test.
}

READ8_MEMBER( applix_state::applix_pa_r )
{
	return m_pa;
}

READ8_MEMBER( applix_state::applix_pb_r )
{
	return m_pb;
}

WRITE8_MEMBER( applix_state::applix_pa_w )
{
	m_pa = data;
}

WRITE8_MEMBER( applix_state::applix_pb_w )
{//printf("%X ",data);
	m_pb = data;
}

static ADDRESS_MAP_START(applix_mem, AS_PROGRAM, 16, applix_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x3fffff) AM_RAM AM_SHARE("expansion") // Expansion
	AM_RANGE(0x400000, 0x47ffff) AM_RAM AM_MIRROR(0x80000) AM_SHARE("base") // Main ram
	AM_RANGE(0x500000, 0x51ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x600000, 0x60007f) AM_WRITE(palette_w)
	AM_RANGE(0x600080, 0x6000ff) AM_WRITE(dac_latch_w)
	AM_RANGE(0x600100, 0x60017f) AM_WRITE(video_latch_w) //video latch (=border colour, high nybble; video base, low nybble) (odd)
	AM_RANGE(0x600180, 0x6001ff) AM_WRITE(analog_latch_w) //analog multiplexer latch (odd)
	//AM_RANGE(0x700000, 0x700007) z80-scc (ch b control, ch b data, ch a control, ch a data) on even addresses
	AM_RANGE(0x700080, 0x7000ff) AM_READ(applix_inputs_r) //input port (odd)
	AM_RANGE(0x700100, 0x70011f) AM_MIRROR(0x60) AM_DEVREADWRITE8("via6522", via6522_device, read, write, 0xff00)
	AM_RANGE(0x700180, 0x700181) AM_MIRROR(0x7c) AM_WRITE(applix_index_w)
	AM_RANGE(0x700182, 0x700183) AM_MIRROR(0x7c) AM_WRITE(applix_register_w)
	//600000, 6FFFFF  io ports and latches
	//700000, 7FFFFF  peripheral chips and devices
	//800000, FFC000  optional roms
	//FFFFC0, FFFFFF  disk controller board
ADDRESS_MAP_END

// io priorities:
// 4 cassette
// 3 scc
// 2 via

/* Input ports */
static INPUT_PORTS_START( applix )
	PORT_START("DSW")
	PORT_BIT( 0xf, 0, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Switch 0") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "Switch 1") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "Switch 2") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch 3") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


void applix_state::machine_reset()
{
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_expansion, ROM, 8);
	m_maincpu->reset();
}


void applix_state::palette_init()
{ // shades need to be verified - the names on the right are from the manual
	const UINT8 colors[16*3] = {
	0x00, 0x00, 0x00,   //  0 Black
	0x40, 0x40, 0x40,   //  1 Dark Grey
	0x00, 0x00, 0x80,   //  2 Dark Blue
	0x00, 0x00, 0xff,   //  3 Mid Blue
	0x00, 0x80, 0x00,   //  4 Dark Green
	0x00, 0xff, 0x00,   //  5 Green
	0x00, 0xff, 0xff,   //  6 Blue Grey
	0x00, 0x7f, 0x7f,   //  7 Light Blue
	0x7f, 0x00, 0x00,   //  8 Dark Red
	0xff, 0x00, 0x00,   //  9 Red
	0x7f, 0x00, 0x7f,   // 10 Dark Violet
	0xff, 0x00, 0xff,   // 11 Violet
	0x7f, 0x7f, 0x00,   // 12 Brown
	0xff, 0xff, 0x00,   // 13 Yellow
	0xbf, 0xbf, 0xbf,   // 14 Light Grey
	0xff, 0xff, 0xff }; // 15 White

	UINT8 r, b, g, i, color_count = 0;

	for (i = 0; i < 48; color_count++)
	{
		r = colors[i++]; g = colors[i++]; b = colors[i++];
		palette_set_color(machine(), color_count, MAKE_RGB(r, g, b));
	}
}


void applix_state::video_start()
{
}

static MC6845_UPDATE_ROW( applix_update_row )
{
// The display is bitmapped. 2 modes are supported here, 320x200x16 and 640x200x4.
// Need to display a border colour.
// There is a monochrome mode, but no info found as yet.
	applix_state *state = device->machine().driver_data<applix_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 i;
	UINT16 chr,x;
	UINT32 mem, vidbase = (state->m_video_latch & 15) << 14, *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{

		if (BIT(state->m_pa, 3))
		// 640 x 200 x 4of16 mode
		{
			mem = vidbase + ma + x + ((y%4)<<12);
			chr = state->m_base[mem];
			for (i = 0; i < 8; i++)
			{
				*p++ = palette[state->m_palette_latch[chr>>14]];
				chr <<= 2;
			}
		}
		else
		// 320 x 200 x 16 mode
		{
			mem = vidbase + ma + x + ((y%4)<<12);
			chr = state->m_expansion[mem];
			for (i = 0; i < 4; i++)
			{
				*p++ = palette[chr>>12];
				*p++ = palette[chr>>12];
				chr <<= 4;
			}
		}
	}
}

static MC6845_INTERFACE( applix_crtc )
{
	"screen",           /* name of screen */
	false,
	8,          /* number of dots per character */
	NULL,
	applix_update_row,      /* handler to display a scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(applix_state, vsync_w),
	NULL
};

WRITE_LINE_MEMBER( applix_state::vsync_w )
{
	m_via->write_ca2(state);
}

static const via6522_interface applix_via =
{
	DEVCB_DRIVER_MEMBER(applix_state, applix_pa_r), // in port A
	DEVCB_DRIVER_MEMBER(applix_state, applix_pb_r), // in port B
	DEVCB_NULL, // in CA1 cent ack
	DEVCB_NULL, // in CB1 kbd clk
	DEVCB_NULL, // in CA2 vsync
	DEVCB_NULL, // in CB2 kdb data
	DEVCB_DRIVER_MEMBER(applix_state, applix_pa_w),// out Port A
	DEVCB_DRIVER_MEMBER(applix_state, applix_pb_w), // out port B
	DEVCB_NULL, // out CA1
	DEVCB_NULL, // out CB1
	DEVCB_NULL, // out CA2
	DEVCB_NULL, // out CB2
	DEVCB_CPU_INPUT_LINE("maincpu", M68K_IRQ_2) //IRQ
};

static MACHINE_CONFIG_START( applix, applix_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 7500000)
	MCFG_CPU_PROGRAM_MAP(applix_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_LENGTH(16)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, 1875000, applix_crtc) // 6545
	MCFG_VIA6522_ADD("via6522", 0, applix_via)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( applix )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD16_BYTE( "1616oshv.044", 0x00000, 0x10000, CRC(4a1a90d3) SHA1(4df504bbf6fc5dad76c29e9657bfa556500420a6) )
	ROM_LOAD16_BYTE( "1616oslv.044", 0x00001, 0x10000, CRC(ef619994) SHA1(ff16fe9e2c99a1ffc855baf89278a97a2a2e881a) )

	ROM_REGION(0x50000, "user1", 0)
	ROM_LOAD16_BYTE( "1616oshv.045", 0x00000, 0x10000, CRC(9dfb3224) SHA1(5223833a357f90b147f25826c01713269fc1945f) )
	ROM_LOAD16_BYTE( "1616oslv.045", 0x00001, 0x10000, CRC(951bd441) SHA1(e0a38c8d0d38d84955c1de3f6a7d56ce06b063f6) )
	ROM_LOAD( "1616osv.045",  0x20000, 0x20000, CRC(b9f75432) SHA1(278964e2a02b1fe26ff34f09dc040e03c1d81a6d) )
	ROM_LOAD( "1616ssdv.022", 0x40000, 0x08000, CRC(6d8e413a) SHA1(fc27d92c34f231345a387b06670f36f8c1705856) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1986, applix, 0,       0,     applix, applix, driver_device, 0, "Applix Pty Ltd", "Applix 1616", GAME_NOT_WORKING | GAME_NO_SOUND)
