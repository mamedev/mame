// license:BSD-3-Clause
// copyright-holders:AJR
/**************************************************************************************************

Sharp MZ-1R18 RAM File Board

This 64KB RAM disk is specific to one dedicated slot on the MZ-800, likely because that slot
supplies its DRAMs with the Z80-generated refresh strobe not ordinarily present on the bus.

List of all ICs:    8x M5K4164AP-15
                    4x M74LS193P
                    2x M74LS257AP
                       M74LS367AP
                       M74LS74AP
                       SN74LS04N
                       M74LS30P
                       M74LS42P
                       M74LS00P

**************************************************************************************************/

#include "emu.h"
#include "mz1r18.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class mz1r18_device : public device_t, public device_mz80_exp_interface
{
public:
	// device type constructor
	mz1r18_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	u8 ram_data_r();
	void ram_data_w(u8 data);
	void ram_address_w(offs_t offset, u8 data);

	std::unique_ptr<u8 []> m_ram;
	u16 m_ram_address;
};

mz1r18_device::mz1r18_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MZ1R18, tag, owner, clock)
	, device_mz80_exp_interface(mconfig, *this)
	, m_ram_address(0)
{
}

void mz1r18_device::device_start()
{
	m_ram = util::make_unique_clear<u8 []>(0x10000);

	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_ram_address));
}

void mz1r18_device::io_map(address_map &map)
{
	map(0xea, 0xea).mirror(0xff00).rw(FUNC(mz1r18_device::ram_data_r), FUNC(mz1r18_device::ram_data_w));
	map(0xeb, 0xeb).select(0xff00).w(FUNC(mz1r18_device::ram_address_w));
}

u8 mz1r18_device::ram_data_r()
{
	u8 data = m_ram[m_ram_address];
	if (!machine().side_effects_disabled())
	{
		LOG("%s: Reading %02X from RAM disk @ %04X\n", machine().describe_context(), data, m_ram_address);
		m_ram_address++;
	}
	return data;
}

void mz1r18_device::ram_data_w(u8 data)
{
	m_ram[m_ram_address] = data;
	if (!machine().side_effects_disabled())
	{
		LOG("%s: Writing %02X to RAM disk @ %04X\n", machine().describe_context(), data, m_ram_address);
		m_ram_address++;
	}
}

void mz1r18_device::ram_address_w(offs_t offset, u8 data)
{
	m_ram_address = (offset & 0xff00) | data;
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(MZ1R18, device_mz80_exp_interface, mz1r18_device, "mz1r18", "Sharp MZ-1R18 RAM File")
