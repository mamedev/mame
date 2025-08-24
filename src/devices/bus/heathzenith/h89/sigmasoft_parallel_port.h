// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H89_SIGMASOFT_PARALLEL_PORT_H
#define MAME_BUS_HEATHZENITH_H89_SIGMASOFT_PARALLEL_PORT_H

#pragma once

#include "h89bus.h"

#include "bus/heathzenith/h19/tlb.h"

class sigmasoft_parallel_port : public device_t, public device_h89bus_left_card_interface
{
public:
	sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	sigmasoft_parallel_port(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	virtual void igc_w(u8 offset, u8 val);
	virtual u8 igc_r(u8 offset);

private:

	required_ioport m_jumpers;

	bool m_installed;

	// physical jumper on the board to enable/disable entire board
	bool m_enabled;

	// base address of board configured by jumpers.
	u16 m_base_addr;
};

class sigmasoft_parallel_port_igc : public sigmasoft_parallel_port
{
public:
	sigmasoft_parallel_port_igc(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_tlbc(T &&tag) { m_tlbc.set_tag(std::forward<T>(tag)); }

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void igc_w(u8 offset, u8 val) override;
	virtual u8 igc_r(u8 offset) override;

private:
	required_device<heath_tlb_connector> m_tlbc;

};

DECLARE_DEVICE_TYPE(H89BUS_SIGMASOFT_PARALLEL,     device_h89bus_left_card_interface)
DECLARE_DEVICE_TYPE(H89BUS_SIGMASOFT_PARALLEL_IGC, device_h89bus_left_card_interface)

#endif // MAME_BUS_HEATHZENITH_H89_SIGMASOFT_PARALLEL_PORT_H
