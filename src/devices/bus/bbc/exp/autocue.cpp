// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Autocue RAM Disc

  The RAM is accessible through JIM (page &FD). One page is visible in JIM at a time.
  The selected page is controlled by the two paging registers:

  &FCFE       Paging register MSB
  &FCFF       Paging register LSB

  256K board has 1024 pages &000 to &3FF
  512K board has 2048 pages &000 to &7FF

**********************************************************************/

#include "emu.h"
#include "autocue.h"

#include "machine/nvram.h"


namespace {

class bbc_autocue_device : public device_t, public device_bbc_exp_interface
{
public:
	bbc_autocue_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_AUTOCUE, tag, owner, clock)
		, device_bbc_exp_interface(mconfig, *this)
		, m_nvram(*this, "nvram")
		, m_ram_page(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_ram;

	uint16_t m_ram_page;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_autocue_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_autocue_device::device_start()
{
	// ram disk - board with 8 x HM62256LFP-12 - 256K expandable to 512K
	m_ram = make_unique_clear<uint8_t[]>(0x40000);
	m_nvram->set_base(m_ram.get(), 0x40000);

	// register for save states
	save_item(NAME(m_ram_page));
	save_pointer(NAME(m_ram), 0x40000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_autocue_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xfe: m_ram_page = (m_ram_page & 0x00ff) | (data << 8); break;
	case 0xff: m_ram_page = (m_ram_page & 0xff00) | (data << 0); break;
	}
}

uint8_t bbc_autocue_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_ram_page < 0x400)
	{
		data = m_ram[(m_ram_page << 8) | offset];
	}

	return data;
}

void bbc_autocue_device::jim_w(offs_t offset, uint8_t data)
{
	if (m_ram_page < 0x400)
	{
		m_ram[(m_ram_page << 8) | offset] = data;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_AUTOCUE, device_bbc_exp_interface, bbc_autocue_device, "bbc_autocue", "Autocue RAM Disc");
