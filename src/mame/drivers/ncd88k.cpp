// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for MC88100-based NCD X terminals.

****************************************************************************/
/*
 * WIP notes
 *
 * 88k "base" (19c, 19g, 17cr, 19cp)
 *  - https://web-docs.gsi.de/~kraemer/COLLECTION/ftp.ncd.com/pub/ncd/Archive/NCD-Articles/NCD_X_Terminals/Memory_specs/NCD_88k_family_memory_specs
 *
 * MCX
 *  - 2MB code and 4MB data memory on motherboard
 *  - J10 SIMM slot for code, J11/J12 for data
 *  - Code memory can be added as 256Kx32, 512Kx32, 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs. Data memory can be added as 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs.
 *  - 256Kx32 and 512Kx32 SIMMs are only supported as code memory
 *
 */

#include "emu.h"
#include "cpu/m88000/m88000.h"
#include "machine/mc68681.h"
#include "machine/am79c90.h"
#include "machine/eepromser.h"

#include "bus/rs232/rs232.h"

#include "video/bt45x.h"
#include "video/bt47x.h"
#include "screen.h"

class ncd88k_state : public driver_device
{
public:
	ncd88k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
	{
	}

	void ncd19c(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void code_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<bt458_device> m_ramdac;

	required_shared_ptr<u32> m_vram;
};

u32 ncd88k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = m_ramdac->pen_color(u8(pixel_data >> 24));
			bitmap.pix(y, x + 1) = m_ramdac->pen_color(u8(pixel_data >> 16));
			bitmap.pix(y, x + 2) = m_ramdac->pen_color(u8(pixel_data >> 8));
			bitmap.pix(y, x + 3) = m_ramdac->pen_color(u8(pixel_data >> 0));
		}

		// compensate by 2048 - 1280 pixels per line
		pixel_pointer += 0xc0;
	}

	return 0;
}

void ncd88k_state::code_map(address_map &map)
{
	map(0x00000000, 0x0001cfff).rom().region("program", 0);
	map(0x04000000, 0x07ffffff).ram().share("coderam");
}

void ncd88k_state::data_map(address_map &map)
{
	map(0x00000000, 0x0001cfff).rom().region("program", 0);
	map(0x01000000, 0x0100003f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x01400000, 0x0140001f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff000000);

	map(0x04000000, 0x07ffffff).ram().share("coderam");
	map(0x08000000, 0x0d03ffff).ram().share("dataram");

	map(0x0e000000, 0x0e1fffff).ram().share("vram");
}

static INPUT_PORTS_START(ncd19c)
INPUT_PORTS_END

void ncd88k_state::ncd19c(machine_config &config)
{
	MC88100(config, m_maincpu, 15'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd88k_state::code_map);
	m_maincpu->set_addrmap(AS_DATA, &ncd88k_state::data_map);

	SCN2681(config, "duart", 3'686'400);

	BT458(config, m_ramdac, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(125'000'000, 1680, 0, 1280, 1063, 0, 1024); // 74.4 kHz horizontal, 70 Hz vertical
	m_screen->set_screen_update(FUNC(ncd88k_state::screen_update));
}

ROM_START(ncd19c)
	ROM_REGION32_BE(0x20000, "program", ROMREGION_ERASE00)
	// These dumps have very strange lengths. The actual ROMs should be standard EEPROM types.
	ROM_LOAD16_BYTE("ncd19c-e.rom", 0x0000, 0xb000, CRC(01e31b42) SHA1(28da6e4465415d00a739742ded7937a144129aad) BAD_DUMP)
	ROM_LOAD16_BYTE("ncd19c-o.rom", 0x0001, 0xb000, CRC(dfd9be7c) SHA1(2e99a325b039f8c3bb89833cd1940e6737b64d79) BAD_DUMP)
ROM_END

class ncdmcx_state : public driver_device
{
public:
	ncdmcx_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_eeprom(*this, "eeprom")
		, m_lance(*this, "lance")
		, m_duart(*this, "duart")
		, m_ramdac(*this, "ramdac")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
	{
	}

	void ncdmcx(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void code_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_cpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<am79c90_device> m_lance;
	required_device<scn2681_device> m_duart;
	required_device<bt477_device> m_ramdac;
	required_device<screen_device> m_screen;

	required_shared_ptr<u32> m_vram;
};

u32 ncdmcx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u32 *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = m_ramdac->pen_color(u8(pixel_data >> 24));
			bitmap.pix(y, x + 1) = m_ramdac->pen_color(u8(pixel_data >> 16));
			bitmap.pix(y, x + 2) = m_ramdac->pen_color(u8(pixel_data >> 8));
			bitmap.pix(y, x + 3) = m_ramdac->pen_color(u8(pixel_data >> 0));
		}

		// compensate by 2048 - 1280 pixels per line
		pixel_pointer += 0xc0;
	}

	return 0;
}

void ncdmcx_state::ncdmcx(machine_config &config)
{
	MC88100(config, m_cpu, 80_MHz_XTAL / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &ncdmcx_state::code_map);
	m_cpu->set_addrmap(AS_DATA, &ncdmcx_state::data_map);

	// CAT35C104P 256x16 or 512x8 4K-Bit SERIAL E2PROM
	EEPROM_93C66_8BIT(config, m_eeprom);

	// ICD2061ASC-1

	// 4100004
	AM79C90(config, m_lance, 20_MHz_XTAL);

	SCN2681(config, m_duart, 3'686'400);

	// ATT20C497-11
	BT477(config, m_ramdac, 125'000'000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(125'000'000, 1680, 0, 1280, 1063, 0, 1024); // 74.4 kHz horizontal, 70 Hz vertical
	m_screen->set_screen_update(FUNC(ncdmcx_state::screen_update));
}

void ncdmcx_state::code_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("program", 0);

	map(0x04000000, 0x07ffffff).ram().share("coderam");
}

void ncdmcx_state::data_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("program", 0);

	map(0x01000000, 0x0100003f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x01400000, 0x0140001f).m(m_ramdac, FUNC(bt477_device::map)).umask32(0xff000000);

	// bit 0 -> icd2061a clock
	// bit 1 -> icd2016a data
	map(0x01580000, 0x01580003).nopw();

	map(0x04000000, 0x07ffffff).ram().share("coderam"); // maximum 64M
	map(0x08000000, 0x0dffffff).ram().share("dataram"); // maximum 96M

	map(0x0e000000, 0x0e3fffff).ram().share("vram");
}

ROM_START(ncdmcx)
	ROM_REGION32_BE(0x40000, "program", 0)
	ROM_LOAD16_BYTE("ncd88k_mcx_bm.u3",  0x0000, 0x20000, CRC(70305680) SHA1(b10b250fe319e823cff28ba7b449b0a40755f5a2))
	ROM_LOAD16_BYTE("ncd88k_mcx_bm.u14", 0x0001, 0x20000, CRC(fc066464) SHA1(fa894de56b77bd4bc619040a2cf3a0d260914727))
ROM_END

COMP(1990, ncd19c, 0, 0, ncd19c, ncd19c, ncd88k_state, empty_init, "Network Computing Devices", "19c", MACHINE_IS_SKELETON)
COMP(1990, ncdmcx, 0, 0, ncdmcx, 0,      ncdmcx_state, empty_init, "Network Computing Devices", "MCX", MACHINE_IS_SKELETON)
