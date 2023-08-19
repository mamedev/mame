// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH-2 SH7014

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_H
#define MAME_CPU_SH_SH7014_H

#pragma once

#include "sh2.h"
#include "sh7014_bsc.h"
#include "sh7014_dmac.h"
#include "sh7014_intc.h"
#include "sh7014_mtu.h"
#include "sh7014_port.h"
#include "sh7014_sci.h"

class sh2_sh7014_device : public sh2_device
{
public:
	sh2_sh7014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<int Sci> auto sci_tx_w() { return m_sci_tx_cb[Sci].bind(); }

	template<int Sci> void sci_set_external_clock_period(const attotime &period) {
		m_sci[Sci].lookup()->set_external_clock_period(period);
	}

	template<int Sci> void sci_set_send_full_data_transmit_on_sync_hack(bool enabled) {
		m_sci[Sci].lookup()->set_send_full_data_transmit_on_sync_hack(enabled);
	}

	auto read_porta()  { return m_port_read[PORT_A].bind(); }
	auto write_porta() { return m_port_write[PORT_A].bind(); }

	auto read_portb()  { return m_port_read[PORT_B].bind(); }
	auto write_portb() { return m_port_write[PORT_B].bind(); }

	auto read_porte()  { return m_port_read[PORT_E].bind(); }
	auto write_porte() { return m_port_write[PORT_E].bind(); }

	auto read_portf()  { return m_port_read[PORT_F].bind(); }

	void sh7014_map(address_map &map);

protected:
	enum {
		PORT_A,
		PORT_B,
		PORT_E,
		PORT_F,
		PORT_COUNT
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void sh2_exception_internal(const char *message, int irqline, int vector) override;

private:
	void set_irq(int vector, int level, bool is_internal);

	void notify_dma_source(uint32_t source);

	required_device_array<sh7014_sci_device, 2> m_sci;
	required_device<sh7014_bsc_device> m_bsc;
	required_device<sh7014_dmac_device> m_dmac;
	required_device_array<sh7014_dmac_channel_device, 2> m_dmac_chan;
	required_device<sh7014_intc_device> m_intc;
	required_device<sh7014_mtu_device> m_mtu;
	required_device_array<sh7014_mtu_channel_device, 3> m_mtu_chan;
	required_device<sh7014_port_device> m_port;

	devcb_write_line::array<2> m_sci_tx_cb;

	devcb_read16::array<PORT_COUNT> m_port_read;
	devcb_write16::array<PORT_COUNT> m_port_write;

	// CAC
	uint16_t ccr_r();
	void ccr_w(offs_t offset, uint16_t dat, uint16_t mem_mask = ~0);

	uint16_t m_ccr;
};

DECLARE_DEVICE_TYPE(SH2_SH7014,  sh2_sh7014_device)

#endif // MAME_CPU_SH_SH7014_H
