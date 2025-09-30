// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Econet Module

**********************************************************************/

#include "emu.h"
#include "econet.h"

#include "bus/econet/econet.h"
#include "machine/mc6854.h"


namespace {

// ======================> arc_econet_adf10_device

class arc_econet_device : public device_t, public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, ARC_ECONET, tag, owner, clock)
		, device_archimedes_econet_interface(mconfig, *this)
		, m_adlc(*this, "mc6854")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u8 read(offs_t offset) override
	{
		return m_adlc->read(offset);
	}

	virtual void write(offs_t offset, u8 data) override
	{
		m_adlc->write(offset, data);
	}

private:
	required_device<mc6854_device> m_adlc;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_econet_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("econet", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(DEVICE_SELF_OWNER, FUNC(archimedes_econet_slot_device::efiq_w));

	econet_device &econet(ECONET(config, "econet", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", "econet", econet_devices);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ARC_ECONET, device_archimedes_econet_interface, arc_econet_device, "arc_econet", "Acorn ADF10/AEH52 Econet Module");
