// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Sharp MZ-6500



****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/upd765.h"
#include "video/upd7220.h"

class mz6500_state : public driver_device
{
public:
	mz6500_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_hgdc(*this, "upd7220"),
		m_fdc(*this, "upd765"),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette") { }

	required_device<upd7220_device> m_hgdc;
	required_device<upd765a_device> m_fdc;
	DECLARE_READ8_MEMBER(mz6500_vram_r);
	DECLARE_WRITE8_MEMBER(mz6500_vram_w);
	void fdc_irq(bool state);
	void fdc_drq(bool state);
	required_shared_ptr<UINT16> m_video_ram;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
};

UPD7220_DISPLAY_PIXELS_MEMBER( mz6500_state::hgdc_display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int gfx[3];
	UINT8 i,pen;

	gfx[0] = m_video_ram[(address + 0x00000) >> 1];
	gfx[1] = m_video_ram[(address + 0x10000) >> 1];
	gfx[2] = m_video_ram[(address + 0x20000) >> 1];

	for(i=0; i<16; i++)
	{
		pen = (BIT(gfx[0], i)) | (BIT(gfx[1], i) << 1) | (BIT(gfx[2], i) << 2);

		bitmap.pix32(y, x + i) = palette[pen];
	}
}


void mz6500_state::video_start()
{
}


READ8_MEMBER( mz6500_state::mz6500_vram_r )
{
	return m_video_ram[offset >> 1] >> ((offset & 1) ? 8 : 0);
}

WRITE8_MEMBER( mz6500_state::mz6500_vram_w )
{
	int mask = (offset & 1) ? 8 : 0;
	offset >>= 1;
	m_video_ram[offset] &= 0xff00 >> mask;
	m_video_ram[offset] |= data << mask;
}

static ADDRESS_MAP_START(mz6500_map, AS_PROGRAM, 16, mz6500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x9ffff) AM_RAM
//  AM_RANGE(0xa0000,0xbffff) kanji/dictionary ROM
	AM_RANGE(0xc0000,0xeffff) AM_READWRITE8(mz6500_vram_r,mz6500_vram_w,0xffff)
	AM_RANGE(0xfc000,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mz6500_io, AS_IO, 16, mz6500_state)
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x0000, 0x000f) i8237 dma
//  AM_RANGE(0x0010, 0x001f) i8255
	AM_RANGE(0x0020, 0x0021) AM_MIRROR(0xe) AM_DEVICE8("upd765", upd765a_device, map, 0xffff)
//  AM_RANGE(0x0030, 0x003f) i8259 master
//  AM_RANGE(0x0040, 0x004f) i8259 slave
//  AM_RANGE(0x0050, 0x0050) segment byte for DMA
//  AM_RANGE(0x0060, 0x0060) system port A
//  AM_RANGE(0x0070, 0x0070) system port C
//  AM_RANGE(0x00cd, 0x00cd) MZ-1R32
	AM_RANGE(0x0100, 0x0103) AM_MIRROR(0xc) AM_DEVREADWRITE8("upd7220", upd7220_device, read, write, 0x00ff)
//  AM_RANGE(0x0110, 0x011f) video address / data registers (priority)
//  AM_RANGE(0x0120, 0x012f) video registers
//  AM_RANGE(0x0130, 0x013f) video register
//  AM_RANGE(0x0140, 0x015f) palette pens
//  AM_RANGE(0x0200, 0x020f) z80sio
//  AM_RANGE(0x0210, 0x021f) z80ctc
//  AM_RANGE(0x0220, 0x022f) rp5c01
//  AM_RANGE(0x0230, 0x023f) ay-3-8912
//  AM_RANGE(0x0240, 0x0240) z80ctc vector ack
//  AM_RANGE(0x0250, 0x0250) z80sio vector ack
//  AM_RANGE(0x0270, 0x0270) system port B
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mz6500 )
INPUT_PORTS_END


void mz6500_state::machine_reset()
{
}

void mz6500_state::fdc_irq(bool state)
{
	//printf("%02x IRQ\n",state);
}

void mz6500_state::fdc_drq(bool state)
{
	//printf("%02x DRQ\n",state);
}

static SLOT_INTERFACE_START( mz6500_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 16, mz6500_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


static MACHINE_CONFIG_START( mz6500, mz6500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 8000000) //unk clock
	MCFG_CPU_PROGRAM_MAP(mz6500_map)
	MCFG_CPU_IO_MAP(mz6500_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_PALETTE_ADD("palette", 8)

	/* Devices */
	MCFG_DEVICE_ADD("upd7220", UPD7220, 8000000/6) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(mz6500_state, hgdc_display_pixels)

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", mz6500_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", mz6500_floppies, "525hd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mz6500 )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000,CRC(6c978ac4) SHA1(7872d7e6d9cda2ed9f47ed4833a5caa4dfe0e55c))

	ROM_REGION( 0x40000, "dictionary", ROMREGION_ERASEFF )
	ROM_LOAD( "dict.rom", 0x0000, 0x40000, CRC(2df3cfd3) SHA1(d420ede09658c2626b0bb650a063d88b1783e554))

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x40000, CRC(b618e25d) SHA1(1da93337fecde6c0f8a5bd68f3f0b3222a38d63e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 198?, mz6500,  0,      0,       mz6500,     mz6500, driver_device,    0,     "Sharp",   "MZ-6500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
