// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK "Altpro" combo floppy/ATA controller series

    CSRs 177130, 177740.  No vector.

    Compatible with standard floppy controller.  ATA port has software
    support in ANDOS and MK-DOS.

    Not implemented: memory expansion (SMK models).

***************************************************************************/

#include "emu.h"
#include "bk_altpro.h"

#include "bus/ata/ataintf.h"
#include "cpu/t11/t11.h"
#include "imagedev/harddriv.h"
#include "machine/1801vp128.h"

#include "formats/bk0010_dsk.h"


namespace {

ROM_START(bk_altpro)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_DEFAULT_BIOS("smk00")
	ROM_SYSTEM_BIOS(0, "smk00", "SMK00 1.41")
	ROMX_LOAD("disk_smk00_v1.41.rom", 0, 0x1000, CRC(cf41889f) SHA1(d34480e35a400316a1f0b6a9d2f3208f2ff2a6de), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "smk64", "SMK64 2.05")
	ROMX_LOAD("disk_smk64_v2.05.rom", 0, 0x1000, CRC(f5ac331d) SHA1(f14f54eb7a1f3327ceb7d6a30c484f6a0d5226ad), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "smk512", "SMK512 2.05")
	ROMX_LOAD("disk_smk512_v2.05.rom", 0, 0x1000, CRC(6ca562a7) SHA1(4796f373e343460b2c9acf769fd4ddfa46e6e90f), ROM_BIOS(2))
ROM_END

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> bk_altpro_device

class bk_altpro_device : public device_t,
					public device_qbus_card_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	// construction/destruction
	bk_altpro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t ata_r(offs_t offset, uint16_t mem_mask = ~0);
	void ata_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<k1801vp128_device> m_fdc;
	required_device<ata_interface_device> m_ata;

private:
	static void floppy_formats(format_registration &fr);

	bool odd;

	std::unique_ptr<uint8_t[]> m_ram;
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_altpro_device - constructor
//-------------------------------------------------

bk_altpro_device::bk_altpro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_ALTPRO, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_ata(*this, "ata")
{
}

void bk_altpro_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

static void bk_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

const tiny_rom_entry *bk_altpro_device::device_rom_region() const
{
	return ROM_NAME(bk_altpro);
}

void bk_altpro_device::device_add_mconfig(machine_config &config)
{
	K1801VP128(config, m_fdc, XTAL(4'000'000));
	m_fdc->ds_in_callback().set(
			[] (uint16_t data)
			{
				switch (data & 15)
				{
					case 1: return 0;
					case 2: return 1;
					default: return -1;
				}
			});
	FLOPPY_CONNECTOR(config, "fdc:0", bk_floppies, "525qd", bk_altpro_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", bk_floppies, "525qd", bk_altpro_device::floppy_formats);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bk_altpro_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(4096);
	m_bus->install_device(0177130, 0177133,
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::read)),
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::write)));
	m_bus->program_space().install_readwrite_handler(0177740, 0177757,
			emu::rw_delegate(*this, FUNC(bk_altpro_device::ata_r)),
			emu::rw_delegate(*this, FUNC(bk_altpro_device::ata_w)),
			0, 0, t11_device::UNALIGNED_WORD);
	m_bus->program_space().install_rom(0160000, 0167777, memregion("maincpu")->base());
	m_bus->program_space().unmap_write(0160000, 0167777);
	m_bus->program_space().install_ram(0170000, 0176777, &m_ram[0]);
}

void bk_altpro_device::device_reset()
{
	odd = false;
}

uint16_t bk_altpro_device::ata_r(offs_t offset, uint16_t mem_mask)
{
	if (mem_mask == 0xff00) // unaligned access
	{
		odd = true;
		return ((m_ata->cs1_r(7 - offset) << 8) ^ 0xffff);
	}
	else if (odd)
	{
		odd = false;
		return 0;
	}
	else if (offset == 7)
		return m_ata->cs0_r(7 - offset) ^ 0xffff;
	else
		return (m_ata->cs0_r(7 - offset) ^ 0xffff) & 0xff;
}

void bk_altpro_device::ata_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask == 0xff00) // unaligned access
	{
		odd = true;
		m_ata->cs1_w(7 - offset, ((data >> 8) ^ 0xffff) & 0xff);
	}
	else if (odd)
	{
		odd = false;
		return;
	}
	else if (offset == 7)
		m_ata->cs0_w(7 - offset, (data ^ 0xffff));
	else
		m_ata->cs0_w(7 - offset, (data ^ 0xffff) & 0xff);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_ALTPRO, device_qbus_card_interface, bk_altpro_device, "bk_altpro", "BK Altpro floppy/ATA")
