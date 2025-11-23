// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Krikzz "SEGA SSF" mapper

https://krikzz.com/pub/support/mega-everdrive/v1/dev/extended_ssf-v2.txt

**************************************************************************************************/

#include "emu.h"
#include "ssf.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_SSF, megadrive_hb_ssf_device, "megadrive_hb_ssf", "Megadrive Krikzz Sega SSF cart")

megadrive_hb_ssf_device::megadrive_hb_ssf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_ssf2_device(mconfig, type, tag, owner, clock)
{
}

megadrive_hb_ssf_device::megadrive_hb_ssf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_hb_ssf_device(mconfig, MEGADRIVE_HB_SSF, tag, owner, clock)
{
}

// NOTE: same as Sega mapper (Demons of Asteborg wants it this way)
void megadrive_hb_ssf_device::time_f0_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (BIT(data, 1))
		m_sram_view.disable();
	else
		m_sram_view.select(0);
	m_nvram_write_protect = !!BIT(data, 0);
}


void megadrive_hb_ssf_device::time_io_map(address_map &map)
{
	map(0xf0, 0xf1).w(FUNC(megadrive_hb_ssf_device::time_f0_w));
	map(0xf3, 0xf3).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[1]->set_entry(data & 0x1f); }));
	map(0xf5, 0xf5).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[2]->set_entry(data & 0x1f); }));
	map(0xf7, 0xf7).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[3]->set_entry(data & 0x1f); }));
	map(0xf9, 0xf9).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[4]->set_entry(data & 0x1f); }));
	map(0xfb, 0xfb).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[5]->set_entry(data & 0x1f); }));
	map(0xfd, 0xfd).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[6]->set_entry(data & 0x1f); }));
	map(0xff, 0xff).lw8(NAME([this] (offs_t offset, u8 data) { m_rom_bank[7]->set_entry(data & 0x1f); }));
}

/*
 * Demons of Asteborg
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_SSF_SRAM, megadrive_hb_ssf_sram_device, "megadrive_hb_ssf_sram", "Megadrive Krikzz Sega SSF + SRAM cart")

megadrive_hb_ssf_sram_device::megadrive_hb_ssf_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_hb_ssf_device(mconfig, MEGADRIVE_HB_SSF_SRAM, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

void megadrive_hb_ssf_sram_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void megadrive_hb_ssf_sram_device::device_start()
{
	megadrive_hb_ssf_device::device_start();
	const u32 nvram_size = 0x8000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

u16 megadrive_hb_ssf_sram_device::nvram_r(offs_t offset)
{
	const u32 nvram_offset = offset & 0xffff;
	return 0xff00 | m_nvram_ptr[nvram_offset];
}

void megadrive_hb_ssf_sram_device::nvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		const u32 nvram_offset = offset & 0xffff;
		m_nvram_ptr[nvram_offset] = data & 0xff;
	}
}

void megadrive_hb_ssf_sram_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).m(*this, FUNC(megadrive_hb_ssf_sram_device::cart_bank_map));
	map(0x20'0000, 0x20'ffff).view(m_sram_view);
	m_sram_view[0](0x20'0000, 0x20'ffff).rw(FUNC(megadrive_hb_ssf_sram_device::nvram_r), FUNC(megadrive_hb_ssf_sram_device::nvram_w));
}

/*
 * krikzz SSF actual extended SSF
 *
 * TODO:
 * - SD card;
 * - USB (2.0?);
 * - LED;
 * - is math unit signed or unsigned? Sample ROM doesn't seem to care;
 * - what "support 16-bit or 32-bit" really mean? Are args reset after use? Sample ROM doesn't care
 *   again;
 * - division by zero actual values;
 *
 */


DEFINE_DEVICE_TYPE(MEGADRIVE_HB_SSF_EX, megadrive_hb_ssf_ex_device, "megadrive_hb_ssf_ex", "Megadrive Krikzz Sega SSF Extended cart")

megadrive_hb_ssf_ex_device::megadrive_hb_ssf_ex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_hb_ssf_device(mconfig, MEGADRIVE_HB_SSF_EX, tag, owner, clock)
{
}

void megadrive_hb_ssf_ex_device::device_start()
{
	megadrive_hb_ssf_device::device_start();
	save_pointer(STRUCT_MEMBER(m_math, arg), 2);
	save_pointer(STRUCT_MEMBER(m_math, mul), 2);
	save_pointer(STRUCT_MEMBER(m_math, div), 2);
}

void megadrive_hb_ssf_ex_device::device_reset()
{
	megadrive_hb_ssf_device::device_reset();
	m_sram_view.select(0);
	// undefined at /VRES
	for (int i = 0; i < 2; i++)
	{
		m_math[i].arg = 0xffff;
		m_math[i].mul = 0xffff;
		m_math[i].div = 0xffff;
	}
}

/*
 * needs to be a word write
 * 1--- ---- ---- ---- unlock writes to this register
 * -x-- ---- ---- ---- 32X mode
 * --x- ---- ---- ---- ROM memory write protect
 * ---x ---- ---- ---- LED
 * ---- ---- ---x xxxx ROM bank [0]
 */
void megadrive_hb_ssf_ex_device::time_f0_w(offs_t offset, u16 data, u16 mem_mask)
 {
	logerror("time_f0_w %04x & %04x (%s)\n", data, mem_mask, BIT(data, 15) ? "valid" : "ignored");
	if (BIT(data, 15))
	{
		m_sram_view.select(BIT(data, 13));

		// m_led_output = BIT(data, 12);

		m_rom_bank[0]->set_entry(data & 0x1f);
	}
}

void megadrive_hb_ssf_ex_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).view(m_sram_view);
	m_sram_view[0](0x00'0000, 0x3f'ffff).m(*this, FUNC(megadrive_hb_ssf_ex_device::cart_bank_map));
	for (int i = 0; i < 8; i++)
	{
		const u32 page_size = 0x08'0000;

		m_sram_view[1](0x00'0000 | (page_size * i), 0x07'ffff | (page_size * i)).bankrw(m_rom_bank[i]);
	}
}

void megadrive_hb_ssf_ex_device::time_io_map(address_map &map)
{
	megadrive_hb_ssf_device::time_io_map(map);
	map(0xd0, 0xd3).lrw16(
		NAME([this] (offs_t offset) {
			return m_math[offset].arg;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_math[offset].arg);
		})
	);
	map(0xd4, 0xd7).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_math[offset].mul);
			if (offset)
			{
				const u32 arg = (m_math[0].arg << 16) | m_math[1].arg;
				const u32 mul = (m_math[0].mul << 16) | m_math[1].mul;
				const u32 res = arg * mul;
				m_math[0].arg = res >> 16;
				m_math[1].arg = res & 0xffff;
				logerror("mul unit: %08x * %08x = %08x\n", arg, mul, res);
			}
		})
	);
	map(0xd8, 0xdb).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_math[offset].div);
			if (offset)
			{
				const u32 arg = (m_math[0].arg << 16) | m_math[1].arg;
				const u32 div = (m_math[0].div << 16) | m_math[1].div;
				const u32 res = div ? arg / div : 0xffff'ffff;
				m_math[0].arg = res >> 16;
				m_math[1].arg = res & 0xffff;
				logerror("div unit: %08x / %08x = %08x\n", arg, div, res);
			}
		})
	);
//  map(0xe0, 0xe1).rw SD card data
//  map(0xe2, 0xe3).rw USB data (8-bit)
//  map(0xe4, 0xe5).r (14) SPI controller ready
//                     (2) USB FIFO write ready
//                     (1) USB FIFO read ready
//                     (0) SPI controller ready
//  map(0xe6, 0xe7).w  (2) SPI auto read (16-bits only)
//                     (1) 16bit SPI mode
//                     (0) SD card chip select
}
