// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    DKB 3128

    Zorro-III RAM Expansion with up to 128 MB RAM

    Notes:
    - Maximum size is default
    - We only emulate configurations with 4 equally sized modules for now

***************************************************************************/

#include "emu.h"
#include "dkb3128.h"

#include "machine/autoconfig.h"

#define VERBOSE 1
#include "logmacro.h"


namespace bus::amiga::zorro {


class dkb3128_device : public device_t, public device_zorro3_card_interface, public amiga_autoconfig
{
public:
	dkb3128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	static constexpr uint32_t RAM_SIZE_MAX = 0x08000000;

	required_ioport m_config;
	std::unique_ptr<uint32_t[]> m_ram;
	uint32_t m_ram_size = 0;
	uint32_t m_base_address = 0;
};

dkb3128_device::dkb3128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_DKB3128, tag, owner, clock),
	device_zorro3_card_interface(mconfig, *this),
	m_config(*this, "config")
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( dkb3128 )
	PORT_START("config")
	PORT_CONFNAME(0x03, 0x03, "Installed RAM")
	PORT_CONFSETTING(0x00, "16 MB")
	PORT_CONFSETTING(0x01, "32 MB")
	PORT_CONFSETTING(0x02, "64 MB")
	PORT_CONFSETTING(0x03, "128 MB")
INPUT_PORTS_END

ioport_constructor dkb3128_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dkb3128);
}


//**************************************************************************
//  ZORRO / AUTOCONFIG
//**************************************************************************

void dkb3128_device::busrst_w(int state)
{
	if (state)
		return;

	if (m_base_address)
		zorro3_space().unmap_readwrite(m_base_address, m_base_address + m_ram_size - 1);

	m_base_address = 0;
}

void dkb3128_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state)
		return;

	// setup ram from jumpers
	m_ram_size = 0x01000000U << (m_config->read() & 0x03);

	// setup autoconfig
	autoconfig_board_type(BOARD_TYPE_ZORRO3);
	autoconfig_board_subsize(BOARD_SUBSIZE_SAME);

	switch (m_ram_size)
	{
	case 0x01000000:
		autoconfig_board_size(BOARD_SIZE_16M);
		break;
	case 0x02000000:
		autoconfig_board_size(BOARD_SIZE_32M);
		break;
	case 0x04000000:
		autoconfig_board_size(BOARD_SIZE_64M);
		break;
	case 0x08000000:
		autoconfig_board_size(BOARD_SIZE_128M);
		break;
	}

	autoconfig_product(14);
	autoconfig_manufacturer(2012);
	autoconfig_serial(0x00000000);

	autoconfig_link_into_memory(true);
	autoconfig_rom_vector_valid(false);
	autoconfig_multi_device(false);
	autoconfig_8meg_preferred(true); // z3 memory device
	autoconfig_can_shutup(true);

	// zorro3 autoconfig handler
	zorro3_space().install_readwrite_handler(0xff000000, 0xff00ffff,
		read32_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read32)),
		write32_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write32)), 0xffffffff);
}

void dkb3128_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig address received: 0x%08x\n", uint32_t(address));

	// stop responding to default autoconfig
	zorro3_space().unmap_readwrite(0xff000000, 0xff00ffff);

	m_base_address = address;

	// install memory if we have a valid address
	if (m_base_address)
		zorro3_space().install_ram(m_base_address, m_base_address + m_ram_size - 1, m_ram.get());

	// we're done
	cfgout_w(0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void dkb3128_device::device_start()
{
	// allocate maximum ram
	m_ram = make_unique_clear<uint32_t[]>(RAM_SIZE_MAX / sizeof(uint32_t));

	// register for save states
	save_pointer(NAME(m_ram), RAM_SIZE_MAX / sizeof(uint32_t));
	save_item(NAME(m_ram_size));
	save_item(NAME(m_base_address));
}


} // bus::amiga::zorro


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_DKB3128, device_zorro3_card_interface, bus::amiga::zorro::dkb3128_device, "amiga_dkb3128", "DKB 3128 Memory Expansion Board")
