// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Wizard Sidewinder Rom Expansion Board

    TODO: Fire button doesn't seem to be recognised.

**********************************************************************/

#include "emu.h"
#include "sidewndr.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/vcs_ctrl/ctrl.h"


namespace {

class electron_sidewndr_device
	: public device_t
	, public device_electron_expansion_interface
{
public:
	electron_sidewndr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ELECTRON_SIDEWNDR, tag, owner, clock)
		, device_electron_expansion_interface(mconfig, *this)
		, m_exp(*this, "exp")
		, m_exp_rom(*this, "exp_rom")
		, m_rom(*this, "rom%u", 1)
		, m_joy(*this, "joy")
		, m_romsel(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom[2]); }

	required_device<electron_expansion_slot_device> m_exp;
	required_memory_region m_exp_rom;
	required_device_array<generic_slot_device, 3> m_rom;
	required_device<vcs_control_port_device> m_joy;

	uint8_t m_romsel;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sidewndr )
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("joyrom.rom", 0x0000, 0x2000, CRC(a7320cda) SHA1(771664dbf23bb14febeb414a0272762cd6091ead))
ROM_END

const tiny_rom_entry *electron_sidewndr_device::device_rom_region() const
{
	return ROM_NAME( sidewndr );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_sidewndr_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 6
	m_rom[0]->set_device_load(FUNC(electron_sidewndr_device::rom1_load));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 7
	m_rom[1]->set_device_load(FUNC(electron_sidewndr_device::rom2_load));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 8
	m_rom[2]->set_device_load(FUNC(electron_sidewndr_device::rom3_load));

	VCS_CONTROL_PORT(config, m_joy, vcs_control_port_devices, "joy");

	ELECTRON_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), electron_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_sidewndr_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 5:
			data = m_exp_rom->base()[offset & 0x0fff];
			break;
		case 6:
		case 7:
		case 8:
			data = m_rom[m_romsel - 6]->read_rom(offset & 0x3fff);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			if (offset == 0xfc05)
			{
				data = bitswap<8>(m_joy->read_joy(), 7, 6, 4, 5, 3, 2, 1, 0);
			}
			break;

		case 0xfd:
			data = m_exp_rom->base()[offset & 0x1dff];
			break;
		}
		break;
	}

	data &= m_exp->expbus_r(offset);

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_sidewndr_device::expbus_w(offs_t offset, uint8_t data)
{
	m_exp->expbus_w(offset, data);

	if ((offset == 0xfe05) && !(data & 0xf0))
	{
		m_romsel = data & 0x0f;
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

std::pair<std::error_condition, std::string> electron_sidewndr_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t const size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size: Only 8K/16K is supported");

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *const crt = slot->get_rom_base();
	if (size <= 0x2000)
		memcpy(crt + 0x2000, crt, 0x2000);

	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ELECTRON_SIDEWNDR, device_electron_expansion_interface, electron_sidewndr_device, "electron_sidewndr", "Wizard Sidewinder Rom Expansion Board")
