// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SK-1100 keyboard link cable emulation

The cable is used only to link two Mark III's through keyboard, what
is supported by the game F-16 Fighting Falcon for its 2 players mode.

**********************************************************************/

#ifndef MAME_BUS_SG1000_EXP_SK1100_KBLINK_H
#define MAME_BUS_SG1000_EXP_SK1100_KBLINK_H

#pragma once


#include "sk1100prn.h"
#include "imagedev/bitbngr.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sk1100_link_cable_device

class sk1100_link_cable_device : public device_t,
	public device_sk1100_printer_port_interface
{
public:
	// construction/destruction
	sk1100_link_cable_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_sk1100_link_cable_interface overrides
	virtual void input_data(int state) override { m_data = state; set_data_transfer(); }
	virtual void input_reset(int state) override { m_reset = state; set_data_transfer(); }
	virtual void input_feed(int state) override { m_feed = state; set_data_transfer(); }

	virtual int output_fault() override { set_data_read(); return m_fault; }
	virtual int output_busy() override { set_data_read(); return m_busy; }

	TIMER_CALLBACK_MEMBER(update_queue);
	TIMER_CALLBACK_MEMBER(send_tick);
	TIMER_CALLBACK_MEMBER(read_tick);

private:
	static constexpr int TIMER_POLL = 1;
	static constexpr int TIMER_SEND = 2;
	static constexpr int TIMER_READ = 3;

	void queue();
	void set_data_transfer();
	void set_data_read();

	required_device<bitbanger_device> m_stream;

	u8 m_input_buffer[1000];
	u32 m_input_count;
	u32 m_input_index;
	emu_timer *m_timer_poll;
	emu_timer *m_timer_send;
	emu_timer *m_timer_read;
	bool m_update_received_data;
	int m_data;
	int m_reset;
	int m_feed;
	int m_busy;
	int m_fault;
};


// device type definition
DECLARE_DEVICE_TYPE(SK1100_LINK_CABLE, sk1100_link_cable_device)


#endif // MAME_BUS_SG1000_EXP_SK1100_KBLINK_H
