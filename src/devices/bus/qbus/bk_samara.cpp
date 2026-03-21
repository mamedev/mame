// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK "Samara" combo floppy/ATA controller

    CSRs 177130, 177620, 177640.  No vector.

    Compatible with standard floppy controller.  ATA port has software
    support in CSI-DOS.

***************************************************************************/

#include "emu.h"
#include "bk_samara.h"

#include "bus/ata/ataintf.h"
#include "imagedev/harddriv.h"
#include "machine/1801vp128.h"

#include "formats/bk0010_dsk.h"


namespace {

ROM_START(bk_samara)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_DEFAULT_BIOS("326")
	ROM_SYSTEM_BIOS(0, "326", "mask 326 modded")
	ROMX_LOAD("disk_samara_hdd+fis.rom", 0, 0x0ff4, CRC(3415a928) SHA1(abd1a48fbc1ec3a2bcb83e8d847dbce747453a1b), ROM_BIOS(0))
ROM_END

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> bk_samara_device

class bk_samara_device : public device_t,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	bk_samara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<k1801vp128_device> m_fdc;
	required_device<ata_interface_device> m_ata;

private:
	static void floppy_formats(format_registration &fr);
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_samara_device - constructor
//-------------------------------------------------

bk_samara_device::bk_samara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_SAMARA, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_ata(*this, "ata")
{
}

void bk_samara_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

static void bk_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

const tiny_rom_entry *bk_samara_device::device_rom_region() const
{
	return ROM_NAME(bk_samara);
}

void bk_samara_device::device_add_mconfig(machine_config &config)
{
	K1801VP128(config, m_fdc, XTAL(4'000'000));
	m_fdc->ds_in_callback().set(
			[] (uint16_t data)
			{
				switch (data & 15)
				{
					case 1: return 0;
					case 2: return 1;
					case 4: return 2;
					case 8: return 3;
					default: return -1;
				}
			});
	FLOPPY_CONNECTOR(config, "fdc:0", bk_floppies, "525qd", bk_samara_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", bk_floppies, "525qd", bk_samara_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", bk_floppies, "525qd", bk_samara_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", bk_floppies, "525qd", bk_samara_device::floppy_formats);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bk_samara_device::device_start()
{
	m_bus->install_device(0177130, 0177133,
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::read)),
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::write)));
	m_bus->program_space().install_readwrite_handler(0177620, 0177623,
			emu::rw_delegate(*this, FUNC(bk_samara_device::cs1_r)),
			emu::rw_delegate(*this, FUNC(bk_samara_device::cs1_w)));
	m_bus->program_space().install_readwrite_handler(0177640, 0177657,
			emu::rw_delegate(*this, FUNC(bk_samara_device::cs0_r)),
			emu::rw_delegate(*this, FUNC(bk_samara_device::cs0_w)));
	m_bus->program_space().install_rom(0160000, 0167777, memregion("maincpu")->base());
	m_bus->program_space().unmap_write(0160000, 0167777);
}

uint16_t bk_samara_device::cs0_r(offs_t offset, uint16_t mem_mask)
{
	if (offset == 7)
		return m_ata->cs0_r(7 - offset) ^ 0xffff;
	else
		return (m_ata->cs0_r(7 - offset) ^ 0xffff) & 0xff;
}

uint16_t bk_samara_device::cs1_r(offs_t offset, uint16_t mem_mask)
{
	return (m_ata->cs1_r(7 - offset) ^ 0xffff) & 0xff;
}

void bk_samara_device::cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 7)
		m_ata->cs0_w(7 - offset, (data ^ 0xffff));
	else
		m_ata->cs0_w(7 - offset, (data ^ 0xffff) & 0xff);
}

void bk_samara_device::cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata->cs1_w(7 - offset, (data ^ 0xffff) & 0xff);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_SAMARA, device_qbus_card_interface, bk_samara_device, "bk_samara", "BK Samara floppy/ATA")
