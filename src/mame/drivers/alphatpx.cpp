// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Triumph-Adler Alphatronic Px series

    TODO: Doesn't do much more than showing the initial boot message

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "video/tms9927.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class alphatpx_state : public driver_device
{
public:
	alphatpx_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_bankdev(*this, "bankdev"),
	m_palette(*this, "palette"),
	m_vram(*this, "vram"),
	m_gfx(*this, "gfx")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override;

private:
	required_device<address_map_bank_device> m_bankdev;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<u8> m_gfx;

	std::unique_ptr<uint8_t[]> m_ram;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( alphatp3_mem, AS_PROGRAM, 8, alphatpx_state )
	AM_RANGE(0x0000, 0xffff) AM_DEVICE("bankdev", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( alphatp3_map, AS_PROGRAM, 8, alphatpx_state )
	AM_RANGE(0x00000, 0x00fff) AM_READ_BANK("rom") AM_WRITE_BANK("ram_0000")
	AM_RANGE(0x01000, 0x02fff) AM_RAMBANK("ram_1000")
	AM_RANGE(0x03000, 0x037ef) AM_MIRROR(0x800) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x037f0, 0x037ff) AM_MIRROR(0x800) AM_DEVREADWRITE("crtc", crt5037_device, read, write)
	AM_RANGE(0x04000, 0x0ffff) AM_RAMBANK("ram_4000")
	AM_RANGE(0x10000, 0x1ffff) AM_RAMBANK("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( alphatp3_io, AS_IO, 8, alphatpx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( alphatp3 )
INPUT_PORTS_END


//**************************************************************************
//  FLOPPY
//**************************************************************************

static SLOT_INTERFACE_START( alphatp3_floppies )
	SLOT_INTERFACE("525qd", FLOPPY_525_QD)
SLOT_INTERFACE_END


//**************************************************************************
//  VIDEO
//**************************************************************************

static const gfx_layout charlayout =
{
	8, 12,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START( alphatp3 )
	GFXDECODE_ENTRY("gfx", 0, charlayout, 0, 1)
GFXDECODE_END

uint32_t alphatpx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	for (int y = 0; y < 24; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[(y * 24) + x] & 0x7f;

			// draw 12 lines of the character
			for (int line = 0; line < 12; line++)
			{
				uint8_t data = m_gfx[(code * 16) + line];

				bitmap.pix32(y * 12 + line, x * 8 + 0) = pen[BIT(data, 0) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 1) = pen[BIT(data, 1) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 2) = pen[BIT(data, 2) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 3) = pen[BIT(data, 3) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 4) = pen[BIT(data, 4) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 5) = pen[BIT(data, 5) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 6) = pen[BIT(data, 6) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 7) = pen[BIT(data, 7) ^ BIT(code, 7)];
			}
		}
	}

	return 0;
}


//**************************************************************************
//  SOUND
//**************************************************************************

// Beeper


//**************************************************************************
//  MACHINE
//**************************************************************************

void alphatpx_state::machine_start()
{
	// allocate memory
	m_ram = std::make_unique<uint8_t[]>(0x10000);

	// setup banking
	membank("rom")->set_base(memregion("boot")->base());
	membank("ram_0000")->set_base(m_ram.get() + 0x0000);
	membank("ram_1000")->set_base(m_ram.get() + 0x1000);
	membank("ram_4000")->set_base(m_ram.get() + 0x4000);
	membank("ram")->set_base(m_ram.get());

	// register for save states
	save_pointer(NAME(m_ram.get()), 0x10000);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static MACHINE_CONFIG_START( alphatp3 )
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(alphatp3_mem)
	MCFG_CPU_IO_MAP(alphatp3_io)

	MCFG_CPU_ADD("mcu", I8041, XTAL_6MHz)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(alphatp3_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(17)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS(XTAL_12_8544MHz, 824, 0, 640, 312, 0, 288)
	MCFG_SCREEN_UPDATE_DRIVER(alphatpx_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("crtc", CRT5037, XTAL_12_8544MHz)
	MCFG_TMS9927_CHAR_WIDTH(8)
	MCFG_TMS9927_VSYN_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE)) MCFG_DEVCB_XOR(1)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", alphatp3)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 1000) // frequency unknown
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	// XTAL_4_9152MHz serial clock

	MCFG_FD1791_ADD("fdc", XTAL_4MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( alphatp3 )
	ROM_REGION(0x1000, "boot", 0)
	ROM_LOAD("caap36_02_19.bin", 0x0000, 0x1000, CRC(23df6666) SHA1(5ea04cd299dec9951425eb91ecceb4818c4c6378))

	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("p3_8041.bin", 0x000, 0x400, CRC(97206ad7) SHA1(e4e6b2ebf87ae9dc0b051f3f478496109d124896))

	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  COMPAT   MACHINE   INPUT     CLASS           INIT  COMPANY          FULLNAME  FLAGS
COMP( 1982, alphatp3,  0,      0,       alphatp3, alphatp3, alphatpx_state, 0,    "Triumph-Adler", "alphatronic P3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
