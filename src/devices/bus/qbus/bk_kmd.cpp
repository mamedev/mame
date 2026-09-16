// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK0011 floppy controller (device driver BY.SYS)

    Also compatible with BK0010.

    ROM revisions:
    - 253 -- shipped with BK0011; supports 10-sector format only (800 KB)
    - 326 -- shipped with BK0011M; also supports 9-sector format (720 KB)
    - 327v12 -- 3rd party; also supports PC formats (without Index Mark)

***************************************************************************/

#include "emu.h"
#include "bk_kmd.h"

#include "machine/1801vp128.h"

#include "formats/bk0010_dsk.h"


namespace {

ROM_START(bk_kmd)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_DEFAULT_BIOS("326")
	ROM_SYSTEM_BIOS(0, "253", "mask 253")
	ROMX_LOAD("disk_253.rom", 0, 0x1000, CRC(7a21b887) SHA1(f2c08147558816c59f0ef59e7c40ccb4b555e887), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "326", "mask 326")
	ROMX_LOAD("disk_326.rom", 0, 0x1000, CRC(fde7583d) SHA1(14f29dcb0fa81f7f1a8efade0bda66b042e47aba), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "327v12", "327 v12") // "FDD/HDD BIOS 327 V12 (c) JD/DWG!" -- modified mask 326
	ROMX_LOAD("disk_327.rom", 0, 0x1000, CRC(ed8a43ae) SHA1(28eefbb63047b26e4aec104aeeca74e2f9d0276c), ROM_BIOS(2))
ROM_END


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> bk_kmd_device

class bk_kmd_device : public device_t,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	bk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<k1801vp128_device> m_fdc;

private:
	static void floppy_formats(format_registration &fr);
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_kmd_device - constructor
//-------------------------------------------------

bk_kmd_device::bk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_KMD, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
{
}

void bk_kmd_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

static void bk_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

const tiny_rom_entry *bk_kmd_device::device_rom_region() const
{
	return ROM_NAME(bk_kmd);
}

void bk_kmd_device::device_add_mconfig(machine_config &config)
{
	K1801VP128(config, m_fdc, XTAL(4'000'000));
	m_fdc->ds_in_callback().set(
			[] (uint16_t data) -> uint16_t
			{
				switch (data & 15)
				{
					case 1: return 0;
					case 2: return 1;
					default: return -1;
				}
			});
	FLOPPY_CONNECTOR(config, "fdc:0", bk_floppies, "525qd", bk_kmd_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", bk_floppies, "525qd", bk_kmd_device::floppy_formats);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bk_kmd_device::device_start()
{
	m_bus->install_device(0177130, 0177133,
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::read)),
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::write)));
	m_bus->program_space().install_rom(0160000, 0167777, memregion("maincpu")->base());
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_KMD, device_qbus_card_interface, bk_kmd_device, "bk_kmd", "BK0010 floppy")
