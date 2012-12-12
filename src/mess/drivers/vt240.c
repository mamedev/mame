/***************************************************************************

        DEC VT240

        31/03/2010 Skeleton driver.

    TODO:
    - understand how PCG works, it should be a funky i/o $30 to uPD7220 DMA
      transfer;
    - hook-up T11, rst65 irq + $20 reads are latches for that

    ROM POST notes:
    0x0139: ROM test
    0x015f: RAM test
    0x0071: RAM fill to 0x00
    0x1c8f: UPD7220

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/t11/t11.h"
#include "machine/ram.h"
#include "video/upd7220.h"


class vt240_state : public driver_device
{
public:
	vt240_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hgdc(*this, "upd7220")
		,
		m_video_ram(*this, "video_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc;
	DECLARE_READ8_MEMBER( test_r );
	DECLARE_READ8_MEMBER( pcg_r );
	DECLARE_WRITE8_MEMBER( pcg_w );

	//UINT16 m_pcg_addr;
	//UINT8 m_pcg_internal_addr;
	//UINT8 *m_char_rom;

	required_shared_ptr<UINT8> m_video_ram;
	DECLARE_DRIVER_INIT(vt240);
	virtual void machine_reset();
	INTERRUPT_GEN_MEMBER(vt240_irq);
};

/* TODO */
static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{
	//vt240_state *state = device->machine().driver_data<a5105_state>();
	//int x;
	//int xi,yi;
	//int tile,color;
	//UINT8 tile_data;

	#if 0
	for( x = 0; x < pitch; x++ )
	{
		tile = (vram[(addr+x)*2] & 0xff);
		color = (vram[(addr+x)*2+1] & 0x0f);

		for( yi = 0; yi < lr; yi++)
		{
			tile_data = state->m_char_rom[(tile*8+yi) & 0x7ff];

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			for( xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;
				int pen = (tile_data >> xi) & 1 ? color : 0;

				if(yi >= 8) { pen = 0; }

				res_x = x * 8 + xi;
				res_y = y * lr + yi;

				if(res_x > screen_max_x || res_y > screen_max_y)
					continue;

				bitmap.pix16(res_y, res_x) = pen;
			}
		}
	}
	#endif
}


/* presumably communication with T11 */
READ8_MEMBER( vt240_state::test_r )
{
	//machine().device("maincpu")->execute().set_input_line(I8085_RST65_LINE, CLEAR_LINE);

	return rand();
}


static ADDRESS_MAP_START(vt240_mem, AS_PROGRAM, 8, vt240_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("ipl", 0)
	AM_RANGE(0x4000, 0x5fff) AM_ROM AM_REGION("ipl", 0x8000)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt240_io, AS_IO, 8, vt240_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)
	AM_RANGE(0x20, 0x20) AM_READ(test_r)
	//AM_RANGE(0x30, 0x30) AM_READWRITE(pcg_r,pcg_w) // 0x30 PCG
ADDRESS_MAP_END


static ADDRESS_MAP_START( upd7220_map, AS_0, 8, vt240_state)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vt240 )
INPUT_PORTS_END


void vt240_state::machine_reset()
{
}

static UPD7220_INTERFACE( hgdc_intf )
{
	"screen",
	NULL,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

INTERRUPT_GEN_MEMBER(vt240_state::vt240_irq)
{
	//device.execute().set_input_line(I8085_RST65_LINE, ASSERT_LINE);
}

static const gfx_layout vt240_chars_8x8 =
{
	8,10,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16
};

static GFXDECODE_START( vt240 )
	GFXDECODE_ENTRY( "ipl", 0x0000, vt240_chars_8x8, 0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_START( vt240, vt240_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(vt240_mem)
	MCFG_CPU_IO_MAP(vt240_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vt240_state, vt240_irq)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
//  MCFG_VIDEO_START_OVERRIDE(vt240_state,vt240)
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
	MCFG_GFXDECODE(vt240)

	MCFG_UPD7220_ADD("upd7220", XTAL_4MHz / 4, hgdc_intf, upd7220_map) //unknown clock
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mc7105 )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "027.bin", 0x8000, 0x8000, CRC(a159b412) SHA1(956097ccc2652d494258b3682498cfd3096d7d4f))
	ROM_LOAD( "028.bin", 0x0000, 0x8000, CRC(b253151f) SHA1(22ffeef8eb5df3c38bfe91266f26d1e7822cdb53))

	ROM_REGION( 0x20000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "029.bin", 0x00000, 0x8000, CRC(4a6db217) SHA1(47637325609ea19ffab61fe31e2700d72fa50729))
	ROM_LOAD16_BYTE( "031.bin", 0x00001, 0x8000, CRC(47129579) SHA1(39de9e2e26f90c5da5e72a09ff361c1a94b9008a))
	ROM_LOAD16_BYTE( "030.bin", 0x10000, 0x8000, CRC(05fd7b75) SHA1(2ad8c14e76accfa1b9b8748c58e9ebbc28844a47))
	ROM_LOAD16_BYTE( "032.bin", 0x10001, 0x8000, CRC(e81d93c4) SHA1(982412a7a6e65d6f6b4f66bd093e54ee16f31384))
ROM_END

/* Driver */
DRIVER_INIT_MEMBER(vt240_state,vt240)
{
	UINT8 *ROM = machine().root_device().memregion("ipl")->base();

	/* patch T11 check */
	ROM[0x09d] = 0x00;
	ROM[0x09e] = 0x00;
	ROM[0x09f] = 0x00;

	/* ROM checksum*/
	ROM[0x15c] = 0x00;
	ROM[0x15d] = 0x00;
	ROM[0x15e] = 0x00;
}

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                      FULLNAME       FLAGS */
//COMP( 1983, vt240,  0,      0,       vt220,     vt220, driver_device,   0,  "Digital Equipment Corporation", "VT240", GAME_NOT_WORKING | GAME_NO_SOUND)
//COMP( 1983, vt241,  0,      0,       vt220,     vt220, driver_device,   0,  "Digital Equipment Corporation", "VT241", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1983, mc7105, 0,      0,       vt240,     vt240, vt240_state,   vt240,  "Elektronika",                  "MC7105", GAME_NOT_WORKING | GAME_NO_SOUND)

