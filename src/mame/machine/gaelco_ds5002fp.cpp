// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "machine/gaelco_ds5002fp.h"

DEFINE_DEVICE_TYPE(GAELCO_DS5002FP,       gaelco_ds5002fp_device,       "gaelco_ds5002fp",      "Gaelco DS5002FP")
DEFINE_DEVICE_TYPE(GAELCO_DS5002FP_WRALLY, gaelco_ds5002fp_wrally_device, "gaelco_ds5002fp_wr", "Gaelco DS5002FP (World Rally)")

gaelco_ds5002fp_device_base::gaelco_ds5002fp_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_shareram(*this, ":shareram"),
	m_mcu_ram(*this, "sram")	
{
}

READ8_MEMBER(gaelco_ds5002fp_device_base::dallas_share_r)
{
	uint8_t *shareram = (uint8_t *)m_shareram.target();
	return shareram[BYTE_XOR_BE(offset)];
}

WRITE8_MEMBER(gaelco_ds5002fp_device_base::dallas_share_w)
{
	uint8_t *shareram = (uint8_t *)m_shareram.target();
	shareram[BYTE_XOR_BE(offset)] = data;
}

READ8_MEMBER(gaelco_ds5002fp_device_base::dallas_ram_r)
{
	return m_mcu_ram[offset];
}

WRITE8_MEMBER(gaelco_ds5002fp_device_base::dallas_ram_w)
{
	m_mcu_ram[offset] = data;
}

void gaelco_ds5002fp_device_base::device_start()
{
}

void gaelco_ds5002fp_device_base::device_reset()
{
}

gaelco_ds5002fp_device::gaelco_ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: gaelco_ds5002fp_device_base(mconfig, GAELCO_DS5002FP, tag, owner, clock)
{
}

static ADDRESS_MAP_START( dallas_rom, AS_PROGRAM, 8, gaelco_ds5002fp_device )
	AM_RANGE(0x00000, 0x07fff) AM_READ(dallas_ram_r) /* Code in NVRAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dallas_ram, AS_IO, 8, gaelco_ds5002fp_device )
	AM_RANGE(0x08000, 0x0ffff) AM_READWRITE(dallas_share_r, dallas_share_w) /* confirmed that 0x8000 - 0xffff is a window into 68k shared RAM */
	AM_RANGE(0x10000, 0x17fff) AM_RAM AM_SHARE("sram") /* yes, the games access it as data and use it for temporary storage!! */
ADDRESS_MAP_END

MACHINE_CONFIG_MEMBER(gaelco_ds5002fp_device::device_add_mconfig)
	MCFG_CPU_ADD("mcu", DS5002FP, DERIVED_CLOCK(1, 1))
	MCFG_CPU_PROGRAM_MAP(dallas_rom)
	MCFG_CPU_IO_MAP(dallas_ram)

	MCFG_QUANTUM_PERFECT_CPU("mcu")

	MCFG_NVRAM_ADD_0FILL("sram")
MACHINE_CONFIG_END


gaelco_ds5002fp_wrally_device::gaelco_ds5002fp_wrally_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: gaelco_ds5002fp_device_base(mconfig, GAELCO_DS5002FP_WRALLY, tag, owner, clock)
{
}

static ADDRESS_MAP_START( dallas_rom_wr, AS_PROGRAM, 8, gaelco_ds5002fp_wrally_device )
	AM_RANGE(0x00000, 0x07fff) AM_READ(dallas_ram_r) /* Code in NVRAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dallas_ram_wr, AS_IO, 8, gaelco_ds5002fp_wrally_device )
	AM_RANGE(0x00000, 0x0ffff) AM_READWRITE(dallas_share_r, dallas_share_w)   AM_MASK(0x3fff)     /* Shared RAM with the main CPU */
	AM_RANGE(0x10000, 0x17fff) AM_RAM AM_SHARE("sram") /*don't think World Rally actually uses it for storage tho */
ADDRESS_MAP_END

MACHINE_CONFIG_MEMBER(gaelco_ds5002fp_wrally_device::device_add_mconfig)
	MCFG_CPU_ADD("mcu", DS5002FP, DERIVED_CLOCK(1, 1))
	MCFG_CPU_PROGRAM_MAP(dallas_rom_wr)
	MCFG_CPU_IO_MAP(dallas_ram_wr)

	MCFG_QUANTUM_PERFECT_CPU("mcu")

	MCFG_NVRAM_ADD_0FILL("sram")
MACHINE_CONFIG_END

