// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro E00 DFS emulation

    Comprises of 8K ROM and 2K/4K? RAM on a carrier board, with flying lead
    to RW line to enable writing to RAM.

***************************************************************************/

#include "emu.h"
#include "dfs.h"


namespace {

class bbc_dfse00_device : public device_t, public device_bbc_rom_interface
{
public:
	bbc_dfse00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_DFSE00, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
		, m_ram(*this, "ram", 0x800, ENDIANNESS_LITTLE)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override
	{
		if (offset & 0x2000)
			return m_ram[offset & 0x7ff];
		else
			return get_rom_base()[offset];
	}

	virtual void write(offs_t offset, uint8_t data) override
	{
		if (offset & 0x2000)
			m_ram[offset & 0x7ff] = data;
	}

private:
	memory_share_creator<uint8_t> m_ram;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_DFSE00, device_bbc_rom_interface, bbc_dfse00_device, "bbc_dfse00", "BBC Micro E00 DFS")
