// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heath's WH-88-16 16k RAM board


****************************************************************************/

#include "emu.h"

#include "wh_88_16.h"

#include "machine/ram.h"

#define LOG_MEM_READ      (1U << 1)
#define LOG_MEM_WRITE     (1U << 2)

//#define VERBOSE (LOG_MEM_READ | LOG_MEM_WRITE)
#include "logmacro.h"

#define LOGMEMREAD(...)      LOGMASKED(LOG_MEM_READ,     __VA_ARGS__)
#define LOGMEMWRITE(...)     LOGMASKED(LOG_MEM_WRITE,    __VA_ARGS__)

namespace {

class wh_88_16_device : public device_t, public device_h89bus_left_card_interface
{
public:
	wh_88_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 mem_read(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset) override;
	virtual void mem_write(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset, u8 data) override;

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	inline bool card_selected(u8 pri_select_lines);

private:

	required_device<ram_device> m_ram;

};


/**
 * Heath's WH-88-16
 */
wh_88_16_device::wh_88_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_WH_88_16, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}

inline bool wh_88_16_device::card_selected(u8 pri_select_lines)
{
	return bool(pri_select_lines & h89bus_device::H89_MEM_PRI_RD6);
}

void wh_88_16_device::mem_write(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset, u8 data)
{

	if (!card_selected(pri_select_lines))
	{
		return;
	}

	LOGMEMWRITE("write Card selected - 0x%02x - offset: 0x%04x data: 0x%02x\n", pri_select_lines, offset, data);
	m_ram->write(offset & 0x3fff, data);
}

u8 wh_88_16_device::mem_read(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset)
{
	if (!card_selected(pri_select_lines))
	{
		return 0;
	}

	u8 val = m_ram->read(offset & 0x3fff);
	LOGMEMREAD("read Card selected - 0x%02x offset: 0x%04x val: 0x%02x\n", pri_select_lines, offset, val);

	return val;
}

void wh_88_16_device::device_start()
{
}

void wh_88_16_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_ram).set_default_size("16K");
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_WH_88_16, device_h89bus_left_card_interface, wh_88_16_device, "h89_wh_88_16", "WH-88-16 16k RAM");
