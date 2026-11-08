// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    DIMM "Dual In-line Memory Module"

***************************************************************************/

#include "emu.h"
#include "modules.h"
#include "dimm.h"


//**************************************************************************
//  SDR SDRAM
//**************************************************************************

// Vanguard VS46417801BTGC-8L
// 32 MB (2 MB x8), 2 Ranks, 64-bit Non-ECC
// PC100 3-2-2-5
// PC66 2-2-2-4

class vanguard_vs46417801btgc_8l_device : public spd_dimm_device
{
public:
	vanguard_vs46417801btgc_8l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_rank0;
	memory_share_creator<uint8_t> m_rank1;
};

vanguard_vs46417801btgc_8l_device::vanguard_vs46417801btgc_8l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spd_dimm_device(mconfig, DIMM_VANGUARD_32MB_PC100, tag, owner, clock),
	m_rank0(*this, "^rank0", 16 * 1024 * 1024, ENDIANNESS_NATIVE),
	m_rank1(*this, "^rank1", 16 * 1024 * 1024, ENDIANNESS_NATIVE)
{
}

ROM_START( vanguard_vs46417801btgc_8l )
	ROM_REGION( 0x100, "spd", 0 )
	ROM_LOAD( "vanguard_vs46417801btgc_8l.bin", 0x000, 0x100, CRC(de37a12d) SHA1(fa9df8861e3e5f12aa1e3851c3ace7f025ff0648) )
ROM_END

const tiny_rom_entry *vanguard_vs46417801btgc_8l_device::device_rom_region() const
{
	return ROM_NAME(vanguard_vs46417801btgc_8l);
}

DEFINE_DEVICE_TYPE_PRIVATE(DIMM_VANGUARD_32MB_PC100, spd_dimm_device, vanguard_vs46417801btgc_8l_device, "vanguard_vs46417801btgc_8l", "Vanguard VS46417801BTGC-8L (SDR, 32 MB, PC100)")

// Micron MT18LSDT1664AG-133D2
// 128 MB (16 MB x8), 1 Rank, 64-bit Non-ECC
// PC133 3-3-3-6
// PC100 2-2-2-5

class micron_mt18lsdt1664ag_133d2_device : public spd_dimm_device
{
public:
	micron_mt18lsdt1664ag_133d2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_rank0;
};

micron_mt18lsdt1664ag_133d2_device::micron_mt18lsdt1664ag_133d2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spd_dimm_device(mconfig, DIMM_MICRON_128MB_PC133, tag, owner, clock),
	m_rank0(*this, "^rank0", 128 * 1024 * 1024, ENDIANNESS_NATIVE)
{
}

ROM_START( micron_mt18lsdt1664ag_133d2 )
	ROM_REGION( 0x100, "spd", 0 )
	ROM_LOAD( "micron_mt18lsdt1664ag_133d2.bin", 0x000, 0x100, CRC(bc46f925) SHA1(e3092577622729ca9ca13de17cdbde8cb5e26666) )
ROM_END

const tiny_rom_entry *micron_mt18lsdt1664ag_133d2_device::device_rom_region() const
{
	return ROM_NAME(micron_mt18lsdt1664ag_133d2);
}

DEFINE_DEVICE_TYPE_PRIVATE(DIMM_MICRON_128MB_PC133, spd_dimm_device, micron_mt18lsdt1664ag_133d2_device, "micron_mt18lsdt1664ag_133d2", "Micron MT18LSDT1664AG-133D2 (SDR, 128 MB, PC133)")


//**************************************************************************
//  DDR SDRAM
//**************************************************************************

// Infineon HYS64D32309GU-6
// 256 MB (32 MB x8), 1 Rank, 64-bit Non-ECC
// PC2700/DDR333 2.5-3-3-7
// PC2100/DDR266 2-3-3-6

class infineon_hys64d32309gu_6_device : public spd_dimm_device
{
public:
	infineon_hys64d32309gu_6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_rank0;
};

infineon_hys64d32309gu_6_device::infineon_hys64d32309gu_6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spd_dimm_device(mconfig, DIMM_INFINEON_256MB_PC333, tag, owner, clock),
	m_rank0(*this, "^rank0", 256 * 1024 * 1024, ENDIANNESS_NATIVE)
{
}

ROM_START( infineon_hys64d32309gu_6 )
	ROM_REGION( 0x100, "spd", 0 )
	ROM_LOAD( "infineon_hys64d32309gu_6.bin", 0x000, 0x100, CRC(f47cf389) SHA1(90e2d649f76d9f36eaa41559498d3eea8f223cf6) )
ROM_END

const tiny_rom_entry *infineon_hys64d32309gu_6_device::device_rom_region() const
{
	return ROM_NAME(infineon_hys64d32309gu_6);
}

DEFINE_DEVICE_TYPE_PRIVATE(DIMM_INFINEON_256MB_PC333, spd_dimm_device, infineon_hys64d32309gu_6_device, "infineon_hys64d32309gu_6", "Infineon HYS64D32309GU-6 (DDR, 256 MB, PC333)")


//**************************************************************************
//  SLOT OPTION LISTS
//**************************************************************************

void sdr_sdram_modules(device_slot_interface &device)
{
	device.option_add("vanguard_32mb_pc100", DIMM_VANGUARD_32MB_PC100);
	device.option_add("micron_128mb_pc133", DIMM_MICRON_128MB_PC133);
}

void ddr_sdram_modules(device_slot_interface &device)
{
	device.option_add("infineon_256mb_pc333", DIMM_INFINEON_256MB_PC333);
}
