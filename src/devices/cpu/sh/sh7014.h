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

class sh7014_device : public sh2_device
{
public:
	sh7014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<int Sci> auto sci_tx_w() {
		return m_sci[Sci].lookup()->write_sci_tx();
	}

	template<int Sci> void sci_set_external_clock_period(const attotime &period) {
		m_sci[Sci].lookup()->set_external_clock_period(period);
	}

	template<int Sci> void sci_set_send_full_data_transmit_on_sync_hack(bool enabled) {
		m_sci[Sci].lookup()->set_send_full_data_transmit_on_sync_hack(enabled);
	}

	auto read_porta()  { return m_port.lookup()->port_a_read_callback(); }
	auto write_porta() { return m_port.lookup()->port_a_write_callback(); }

	auto read_portb()  { return m_port.lookup()->port_b_read_callback(); }
	auto write_portb() { return m_port.lookup()->port_b_write_callback(); }

	auto read_porte()  { return m_port.lookup()->port_e_read_callback(); }
	auto write_porte() { return m_port.lookup()->port_e_write_callback(); }

	auto read_portf()  { return m_port.lookup()->port_f_read_callback(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void sh2_exception_internal(const char *message, int irqline, int vector) override;

private:
	void sh7014_map(address_map &map) ATTR_COLD;

	void set_irq(int vector, int level, bool is_internal);

	void notify_dma_source(uint32_t source);

	uint16_t ccr_r();
	void ccr_w(offs_t offset, uint16_t dat, uint16_t mem_mask = ~0);

	required_device_array<sh7014_sci_device, 2> m_sci;
	required_device<sh7014_bsc_device> m_bsc;
	required_device<sh7014_dmac_device> m_dmac;
	required_device<sh7014_intc_device> m_intc;
	required_device<sh7014_mtu_device> m_mtu;
	required_device<sh7014_port_device> m_port;

	devcb_write_line::array<2> m_sci_tx_cb;

	uint16_t m_ccr;
};

DECLARE_DEVICE_TYPE(SH7014,  sh7014_device)

#endif // MAME_CPU_SH_SH7014_H
