// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Pencil 2 Memory Packs

*******************************************************************************/

#include "emu.h"
#include "ram.h"


namespace {

class pencil2_mem16k_device : public device_t, public device_pencil2_memexp_interface
{
public:
	pencil2_mem16k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, PENCIL2_MEM16K, tag, owner, clock)
		, device_pencil2_memexp_interface(mconfig, *this)
		, m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE)
	{
	}

protected:
	virtual void device_start() override ATTR_COLD { }

	virtual u8 m2_r(offs_t offset) override { return m_ram[offset]; }
	virtual void m2_w(offs_t offset, u8 data) override { m_ram[offset] = data; }
	virtual u8 m4_r(offs_t offset) override { return m_ram[offset + 0x2000]; }
	virtual void m4_w(offs_t offset, u8 data) override { m_ram[offset + 0x2000] = data; }

private:
	memory_share_creator<u8> m_ram;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PENCIL2_MEM16K, device_pencil2_memexp_interface, pencil2_mem16k_device, "pencil2_mem16k", "Pencil 2 16K Memory Pack")
