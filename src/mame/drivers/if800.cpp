// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    if800

****************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "video/upd7220.h"


class if800_state : public driver_device
{
public:
	if800_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_hgdc(*this, "upd7220"),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette") { }

	required_device<upd7220_device> m_hgdc;

	required_shared_ptr<UINT16> m_video_ram;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
};

UPD7220_DISPLAY_PIXELS_MEMBER( if800_state::hgdc_display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	int xi,gfx;
	UINT8 pen;

	gfx = m_video_ram[address >> 1];

	for(xi=0;xi<16;xi++)
	{
		pen = ((gfx >> xi) & 1) ? 1 : 0;

		bitmap.pix32(y, x + xi) = palette[pen];
	}
}

static ADDRESS_MAP_START(if800_map, AS_PROGRAM, 16, if800_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x0ffff) AM_RAM
	AM_RANGE(0xfe000,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(if800_io, AS_IO, 16, if800_state)
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x0640, 0x065f) dma?
	AM_RANGE(0x0660, 0x0663) AM_DEVREADWRITE8("upd7220", upd7220_device, read, write,0x00ff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( if800 )
INPUT_PORTS_END

void if800_state::machine_start()
{
}


void if800_state::machine_reset()
{
}

static ADDRESS_MAP_START( upd7220_map, AS_0, 16, if800_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static MACHINE_CONFIG_START( if800, if800_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 8000000)
	MCFG_CPU_PROGRAM_MAP(if800_map)
	MCFG_CPU_IO_MAP(if800_io)


//  MCFG_PIC8259_ADD( "pic8259", if800_pic8259_config )
	MCFG_DEVICE_ADD("upd7220", UPD7220, 8000000/4)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(if800_state, hgdc_display_pixels)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)

	MCFG_PALETTE_ADD("palette", 8)
//  MCFG_PALETTE_INIT(black_and_white)

//  MCFG_VIDEO_START_OVERRIDE(if800_state,if800)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( if800 )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(36212491) SHA1(6eaa8885e2dccb6dd86def6c0c9be1870cee957f))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1985, if800,  0,      0,       if800,     if800, driver_device,    0,     "Oki Electric",   "if800 model 60", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
