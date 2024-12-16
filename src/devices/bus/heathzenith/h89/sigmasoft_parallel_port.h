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

	template <typename T> void set_tlbc(T &&tag) { m_tlbc.set_tag(std::forward<T>(tag)); }

	virtual u8 read(u8 select_lines, u16 offset) override;
	virtual void write(u8 select_lines, u16 offset, u8 data) override;

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void video_mem_w(u8 val);
	u8 video_mem_r();

	void io_lo_addr_w(u8 val);
	void io_hi_addr_w(u8 val);

	void window_lo_addr_w(u8 val);
	void window_hi_addr_w(u8 val);

	void ctrl_w(u8 val);
	u8 ctrl_r();

	inline bool card_selected(u8 select_lines, u16 offset);

private:

	required_device<heath_tlb_connector> m_tlbc;
	required_ioport m_jumpers;

	// flag to allow multiple paralllel boards to be install (up to 3) and configure
	// only one to connect to the tlbc and igc boards.
	bool m_connected_tlbc;

	// physical jumper on the board to enable/disable entire board
	bool m_enabled;

	// base address of board configured by jumpers.
	u16 m_base_addr;

};


DECLARE_DEVICE_TYPE(H89BUS_SIGMASOFT_PARALLEL, device_h89bus_left_card_interface)

#endif // MAME_BUS_HEATHZENITH_H89_SIGMASOFT_PARALLEL_PORT_H
