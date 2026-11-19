// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-17 Floppy controller

    The H89 version of the card, model number H-88-1. It plugs into the
    right-hand P506 slot, where the /FLPY select line decodes its four ports
    and the FMWE line write-enables the 1k of floppy RAM on the CPU board.

    The controller logic is shared with the H8's H-8-17 card, see
    bus/heathzenith/h8x_common/h17_fdc_base.cpp.

****************************************************************************/

#include "emu.h"
#include "h17_fdc.h"

#include "bus/heathzenith/h8x_common/h17_fdc_base.h"


namespace {

class h_88_1_device : public heath_h17_fdc_base_device, public device_h89bus_right_card_interface
{
public:
	h_88_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void bus_clock_changed(u32 clock) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void set_ram_write_enable(int state) override;
	virtual u32 bus_clock() override { return h89bus().clock(); }

	bool m_installed;

	u32 m_bus_clock;
};


h_88_1_device::h_88_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_h17_fdc_base_device(mconfig, H89BUS_H_17_FDC, tag, owner, 0)
	, device_h89bus_right_card_interface(mconfig, *this)
{
}

void h_88_1_device::set_ram_write_enable(int state)
{
	set_slot_fmwe(state);
}

void h_88_1_device::device_start()
{
	heath_h17_fdc_base_device::device_start();

	m_installed = false;
	m_bus_clock = h89bus().clock();

	save_item(NAME(m_installed));
	save_item(NAME(m_bus_clock));
}

void h_88_1_device::device_reset()
{
	if (!m_installed)
	{
		h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_FLPY, m_p506_signals);

		if (addr_ranges.size() == 1)
		{
			h89bus::addr_range range = addr_ranges.front();

			h89bus().install_io_device(range.first, range.second,
				read8sm_delegate(*this, FUNC(h_88_1_device::read)),
				write8sm_delegate(*this, FUNC(h_88_1_device::write)));
		}

		m_installed = true;
	}

	m_bus_clock = h89bus().clock();

	heath_h17_fdc_base_device::device_reset();
}

void h_88_1_device::bus_clock_changed(u32 clock)
{
	// loading a state reports the clock it restored, leave the restored timing alone
	if (clock == m_bus_clock)
		return;

	m_bus_clock = clock;

	start_tx_clock();
	reset_rx_separator();
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_H_17_FDC, device_h89bus_right_card_interface, h_88_1_device, "h89_h17_fdc", "Heath H-17 Hard-sectored Controller (H-88-1)");
