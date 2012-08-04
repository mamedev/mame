/***************************************************************************

	Space Invaders / Qix Silver Anniversary Edition (c) 2003 Taito Corporation

	TODO:
	- HD6312394TE20 == H8S/2394 (unsupported)
	- "H8/3xx: Unknown opcode (PC=11146) 218"

***************************************************************************/


#include "emu.h"
#include "cpu/h83002/h8.h"
//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_20MHz

class invqix_state : public driver_device
{
public:
	invqix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	UINT32 m_test_x, m_test_y,m_start_offs;

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void invqix_state::video_start()
{

}

UINT32 invqix_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int x,y,count;
	const UINT8 *blit_ram = memregion("maincpu")->base();

	if(screen.machine().input().code_pressed_once(KEYCODE_Z))
		m_test_x++;

	if(screen.machine().input().code_pressed_once(KEYCODE_X))
		m_test_x--;

	if(screen.machine().input().code_pressed(KEYCODE_A))
		m_test_y++;

	if(screen.machine().input().code_pressed(KEYCODE_S))
		m_test_y--;

	if(m_test_x == 0)
		m_test_x = 256;

	if(m_test_y == 0)
		m_test_y = 256;

	if(screen.machine().input().code_pressed(KEYCODE_Q))
		m_start_offs+=0x200;

	if(screen.machine().input().code_pressed(KEYCODE_W))
		m_start_offs-=0x200;

	if(screen.machine().input().code_pressed(KEYCODE_E))
		m_start_offs++;

	if(screen.machine().input().code_pressed(KEYCODE_R))
		m_start_offs--;

	popmessage("%d %d %04x",m_test_x,m_test_y,m_start_offs);

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	count = (m_start_offs);

	for(y=0;y<m_test_y;y++)
	{
		for(x=0;x<m_test_x;x++)
		{
			UINT8 r,g,b;
			int pen_data;

			pen_data = (blit_ram[count]) | (blit_ram[count+1]) << 8;
			b = (pen_data & 0x001f);
			g = (pen_data & 0x03e0) >> 5;
			r = (pen_data & 0x7c00) >> 10;
			r = (r << 3) | (r & 0x7);
			g = (g << 3) | (g & 0x7);
			b = (b << 3) | (b & 0x7);

			if(cliprect.contains(x, y))
				bitmap.pix32(y, x) = r << 16 | g << 8 | b;

			count+=2;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( invqix_map, AS_PROGRAM, 16, invqix_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0xff0000, 0xffbfff) AM_RAM // unknown boundaries
ADDRESS_MAP_END

static ADDRESS_MAP_START( invqix_io, AS_IO, 16, invqix_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( invqix )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void invqix_state::machine_start()
{
}

void invqix_state::machine_reset()
{
}


static PALETTE_INIT( invqix )
{
}

static MACHINE_CONFIG_START( invqix, invqix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",H8S2323,MAIN_CLOCK) /* !!! H8S/2394 !!! */
	MCFG_CPU_PROGRAM_MAP(invqix_map)
	MCFG_CPU_IO_MAP(invqix_io)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(invqix_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_INIT(invqix)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( invqix )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f34-02.ic2",   0x000000, 0x200000, CRC(035ace40) SHA1(e61f180024102c7a136b1c7f974c71e5dc698a1e) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "f34-01.ic13",  0x000000, 0x200000, CRC(7b055722) SHA1(8152bf04a58de15aefc4244e40733275e21818e1) )

	ROM_REGION( 0x080, "default_eep", 0 )
	ROM_LOAD( "93c46.ic6",    0x000000, 0x000080, CRC(564b744e) SHA1(4d9ea7dc253797c513258d07a936dfb63d8ed18c) )
ROM_END


GAME( 2003, invqix,  0,   invqix,  invqix, invqix_state,  0,       ROT270, "Taito Corporation",      "Space Invaders / Qix Silver Anniversary Edition (Ver. 2.03)", GAME_NOT_WORKING | GAME_NO_SOUND )
