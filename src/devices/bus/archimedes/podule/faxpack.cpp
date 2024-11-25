// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Concepts Fax-Pack

    https://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_FaxPack.html

    TODO:
    - add R96DFX modem.

**********************************************************************/

#include "emu.h"
#include "faxpack.h"
#include "machine/mb8421.h"
//#include "machine/r96dfx.h"


namespace {

class arc_faxpack_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_faxpack_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_device<mcs51_cpu_device> m_mcu;
	required_memory_region m_podule_rom;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void arc_faxpack_device::ioc_map(address_map &map)
{
	map(0x0000, 0x0fff).rw("dpram", FUNC(idt7130_device::right_r), FUNC(idt7130_device::right_w)).umask32(0x000000ff);
}


void arc_faxpack_device::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("podule_rom", 0);
}

void arc_faxpack_device::io_map(address_map &map)
{
	//map(0x2000, 0x201f).rw("modem", FUNC(r96dfx_device::read), FUNC(r96dfx_device::write));
	map(0x6000, 0x63ff).rw("dpram", FUNC(idt7130_device::left_r), FUNC(idt7130_device::left_w));
}


//-------------------------------------------------
//  ROM( faxpack )
//-------------------------------------------------

ROM_START( faxpack )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("faxpack_3873.rom", 0x0000, 0x8000, CRC(4bb7a925) SHA1(bbd1c4560c3bde1e61f81ae6505cd5d4658c6cb8))
ROM_END

const tiny_rom_entry *arc_faxpack_device::device_rom_region() const
{
	return ROM_NAME( faxpack );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_faxpack_device::device_add_mconfig(machine_config &config)
{
	I8031(config, m_mcu, 11.0592_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &arc_faxpack_device::mem_map);
	m_mcu->set_addrmap(AS_IO, &arc_faxpack_device::io_map);

	idt7130_device &dpram(IDT7130(config, "dpram"));
	dpram.intl_callback().set_inputline(m_mcu, MCS51_INT0_LINE);
	dpram.intr_callback().set([this](int state) { set_pirq(state); });

	//R96DFX(config, "modem", 24.00014_MHz_XTAL);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_faxpack_device - constructor
//-------------------------------------------------

arc_faxpack_device::arc_faxpack_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_FAXPACK, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_faxpack_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_FAXPACK, device_archimedes_podule_interface, arc_faxpack_device, "arc_faxpack", "Computer Concepts FaxPack Modem")
