// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for MC88100-based NCD X terminals.

****************************************************************************/
/*
 * WIP notes
 *
 * There are two basic machines, NCD88k and Modular Color X (MCX). They share
 * the same firmware and have very similar hardware, however the system memory
 * maps differ. Marketing material for the MCX systems describes 3D and audio
 * hardware as enhancements over the prior NCD88k, and the latter system board
 * includes an additional custom ASIC. Various models of terminal differ only
 * by the type of monitor supplied.
 *
 * 88k "base" (19c, 19g, 17cr, 17g, 19cp)
 *  - no on-board memory
 *  - J6 and J7 are code memory SIMM sockets (up to 64M)
 *  - J8, J9 and J10 are data memory SIMM sockets (up to 96M)
 *  - 19c shipped with 2M code memory (1991)
 *  - 17g 1280x1024 gray-scale (1992)
 *  - 17cr 1280x1024 color (1992)
 *
 * MCX
 *  - 2MB code and 4MB data memory on motherboard
 *  - J10 SIMM slot for code, J11/J12 for data
 *  - Code memory can be added as 256Kx32, 512Kx32, 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs. Data memory can be added as 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs.
 *  - 256Kx32 and 512Kx32 SIMMs are only supported as code memory
 *  - ports: ethernet (TP & AUI), monitor, aux, mouse, keyboard, audio in, audio out
 *  - 50kHz 16-bit sound
 *
 * Model    Type, Colors/Grayscales           Res.      Mem.  CPU/GCC     List US$
 * -------------------------------------------------------------------------------
 * MCX-L   base only, VGA color, 100dpi       1152x900  6.0   88100-20       2,295
 * MCX14   14" color, 103dpi/70Hz     640x480,1024x768  6.0   88100-20       3,295
 * MCX15   15" color, 100dpi/70Hz     640x480,1152x900  6.0   88100-20       3,495
 * MCX17   17" color, 87dpi/75Hz     1024x768,1152x900  6.0   88100-20       4,295
 * MCX-L19 19" color, 84dpi/72Hz              1152x900  6.0   88100-20       4,695
 *
 * Sources:
 *  - https://web-docs.gsi.de/~kraemer/COLLECTION/ftp.ncd.com/pub/ncd/Archive/NCD-Articles/NCD_X_Terminals/Memory_specs/NCD_88k_family_memory_specs
 *  - http://books.google.com/books?id=ujsEAAAAMBAJ&lpg=PA24&pg=PA24#v=onepage&q&f=false
 *
 * TODO:
 *  - ram control/sizing
 *  - keyboard and mouse
 *  - interrupts and mask
 */

#include "emu.h"
#include "cpu/m88000/m88000.h"
#include "machine/mc68681.h"
#include "machine/am79c90.h"
#include "machine/eepromser.h"
#include "machine/pckeybrd.h"

#include "bus/rs232/rs232.h"

#include "video/bt45x.h"
#include "video/bt47x.h"
#include "screen.h"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

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

	void code_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

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
	map(0x00000000, 0x0001cfff).rom().region("prom", 0);
	map(0x04000000, 0x07ffffff).ram().share("cram");
}

void ncd88k_state::data_map(address_map &map)
{
	map(0x00000000, 0x0001cfff).rom().region("prom", 0);
	map(0x01000000, 0x0100003f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x01400000, 0x0140001f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff000000);

	map(0x04000000, 0x07ffffff).ram().share("cram");
	map(0x08000000, 0x0d03ffff).ram().share("dram");

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
	ROM_REGION32_BE(0x20000, "prom", ROMREGION_ERASE00)
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
		, m_kbd(*this, "kbd")
		, m_dram(*this, "dram")
		, m_vram(*this, "vram")
	{
	}

	void ncdmcx(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void code_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	template <unsigned N> void irq_w(int state);

	required_device<cpu_device> m_cpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<am7990_device> m_lance;
	required_device<scn2681_device> m_duart;
	required_device<bt477_device> m_ramdac;
	required_device<screen_device> m_screen;

	required_device<at_keyboard_device> m_kbd;

	required_shared_ptr<u32> m_dram;
	required_shared_ptr<u32> m_vram;

	u8 m_int;
	u8 m_msk;
};

void ncdmcx_state::machine_reset()
{
	m_int = 0;
	m_msk = 0;

	m_duart->ip2_w(0);
}

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

	EEPROM_93C66_16BIT(config, m_eeprom); // CAT35C104P
	m_eeprom->default_value(0);
	m_eeprom->do_callback().set(m_duart, FUNC(scn2681_device::ip2_w));

	// ICD2061ASC-1

	AM7990(config, m_lance, 20_MHz_XTAL / 2); // 4100004
	m_lance->intr_out().set(FUNC(ncdmcx_state::irq_w<2>)).invert();
	m_lance->dma_in().set([this](offs_t offset) { return util::big_endian_cast<const u16>(m_dram.target())[offset >> 1]; });
	m_lance->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&util::big_endian_cast<u16>(m_dram.target())[offset >> 1]); });

	SCN2681(config, m_duart, 3'686'400);
	m_duart->irq_cb().set(FUNC(ncdmcx_state::irq_w<3>));
	m_duart->outport_cb().set(
		[this](u8 data)
		{
			m_eeprom->cs_write(BIT(data, 5));
			m_eeprom->di_write(BIT(data, 4));
			m_eeprom->clk_write(BIT(data, 6));
		});

	BT477(config, m_ramdac, 125'000'000); // ATT20C497-11

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(125'000'000, 1680, 0, 1280, 1063, 0, 1024); // 74.4 kHz horizontal, 70 Hz vertical
	m_screen->set_screen_update(FUNC(ncdmcx_state::screen_update));
	m_screen->screen_vblank().set(FUNC(ncdmcx_state::irq_w<4>));

	AT_KEYB(config, m_kbd);
	m_kbd->set_type(at_keyboard_device::KEYBOARD_TYPE::AT, 3);
	m_kbd->keypress().set(FUNC(ncdmcx_state::irq_w<1>));
}

void ncdmcx_state::code_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("prom", 0);

	map(0x04000000, 0x07ffffff).ram().share("cram"); // maximum 64M
}

void ncdmcx_state::data_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("prom", 0);

	map(0x00c00000, 0x00c00003).rw(m_lance, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).mirror(0x000fff04);
	map(0x01000000, 0x0100003f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x01400000, 0x0140001f).m(m_ramdac, FUNC(bt477_device::map)).umask32(0xff000000);

	// 014c0000.b - reset?

	// bit 0 -> icd2061a clock
	// bit 1 -> icd2016a data
	map(0x01580000, 0x01580003).nopw();

	map(0x01d80000, 0x01d80000).lrw8(
		[this]()
		{
			return m_int;
		}, "int_r",
		[this](u8 data)
		{
			LOG("msk_w 0x%02x (%s)\n", data, machine().describe_context());

			m_msk = data;
			m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
		}, "msk_w");

	// 0x64 -> ack or clear irq 4?
	map(0x01d80001, 0x01d80001).lrw8(
		[]()
		{
			return 0;
		}, "int?_r",
		[this](u8 data)
		{
			LOG("ack_w 0x%02x int 0x%02x (%s)\n", data, m_int, machine().describe_context());

			// 0x20?
			// 0x65

			// 0x60 - clr irq 0? (soft interrupt)
			// 0x61 - set irq 0? (soft interrupt)
			// 0x63 - clr irq 6?
			// 0x64 - ack irq 4?
			// 0x65 - 0x0400582c?
			switch (data)
			{
			case 0x60: m_int &= ~0x01; break;
			case 0x61: m_int |= 0x41; break;
			//case 0x62: m_int &= ~0x40; break;
			case 0x63: m_int &= ~0x40; break;
			case 0x64: m_int &= ~0x10; break;
			}

			m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
		}, "ack_w");

	map(0x01d80003, 0x01d80003).lr8(
		[this]()
		{
			irq_w<1>(0);

			// TODO: this might be keyboard rx?
			return m_kbd->read();
		}, "kbd_r");

	map(0x02000000, 0x02ffffff).lw8(
		[this](offs_t offset, u8 data)
		{
			LOG("dram_ctrl 0x%02x\n", offset >> 16);
		}, "dram_ctrl_w");
	map(0x03000000, 0x03ffffff).lw8(
		[this](offs_t offset, u8 data)
		{
			LOG("cram_ctrl 0x%02x\n", offset >> 16);
		}, "cram_ctrl_w");

	map(0x04000000, 0x07ffffff).ram().share("cram"); // maximum 64M
	map(0x08000000, 0x0dffffff).ram().share("dram"); // maximum 96M

	map(0x0e000000, 0x0e3fffff).ram().share("vram");
}

template <unsigned N> void ncdmcx_state::irq_w(int state)
{
	if (state)
		m_int |= 1U << N;
	else
		m_int &= ~(1U << N);

	m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
}

ROM_START(ncdmcx)
	ROM_REGION32_BE(0x40000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "v2.7.3", "v2.7.3")
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0e.u3",  0x0000, 0x20000, CRC(70305680) SHA1(b10b250fe319e823cff28ba7b449b0a40755f5a2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0o.u14", 0x0001, 0x20000, CRC(fc066464) SHA1(fa894de56b77bd4bc619040a2cf3a0d260914727), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "v2.6.0", "v2.6.0")
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0e.u3",  0x0000, 0x20000, CRC(99644196) SHA1(d5091fd4f096000de4970ae778112ff3c01ac340), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0o.u14", 0x0001, 0x20000, CRC(db2ed336) SHA1(8be4e08bf097d2b85be84da62b2a24c6e55661d9), ROM_BIOS(1) | ROM_SKIP(1))
ROM_END

} // anonymous namespace

COMP(1991, ncd19c, 0, 0, ncd19c, ncd19c, ncd88k_state, empty_init, "Network Computing Devices", "19c", MACHINE_IS_SKELETON)
COMP(1993, ncdmcx, 0, 0, ncdmcx, 0,      ncdmcx_state, empty_init, "Network Computing Devices", "MCX", MACHINE_IS_SKELETON)
