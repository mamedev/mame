// license: BSD-3-Clause
// copyright-holders: Peter Ferrie, Angelo Salese

#ifndef MAME_BUS_CENTRONICS_HASP_SAVQUEST_H
#define MAME_BUS_CENTRONICS_HASP_SAVQUEST_H

#pragma once

#include "ctronics.h"

class hasp_savquest_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	hasp_savquest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_data0(int state) override { update_bit(0, state); }
	virtual void input_data1(int state) override { update_bit(1, state); }
	virtual void input_data2(int state) override { update_bit(2, state); }
	virtual void input_data3(int state) override { update_bit(3, state); }
	virtual void input_data4(int state) override { update_bit(4, state); }
	virtual void input_data5(int state) override { update_bit(5, state); }
	virtual void input_data6(int state) override { update_bit(6, state); }
	virtual void input_data7(int state) override { update_bit(7, state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void input_select_in(int state) override;

private:
	void update_bit(int bit, int state);
	TIMER_CALLBACK_MEMBER(deferred_update);
	void process_byte(uint8_t data8);
	void request_transition(uint8_t prev, uint8_t data8);
	uint8_t get_prodinfo_bit(int prodind) const;

	uint8_t m_data_in;
	uint8_t m_prev_data;
	bool m_deferred_pending;
	uint32_t m_deferred_token;
	int m_select_in_state;

	int m_haspind;
	int m_haspstate;
	enum hasp_states
	{
		HASPSTATE_NONE,
		HASPSTATE_PASSBEG,
		HASPSTATE_PASSEND,
		HASPSTATE_READ
	};
	int m_hasp_passind;
	uint8_t m_hasp_tmppass[0x29];
	int m_hasp_passmode;
	int m_hasp_prodind;
	uint8_t m_port379_bit5;

	// passmode 1 product request, decoded from pin transitions
	int m_req_stage;
	bool m_req_prefix_ok;
	int m_req_bits;
	int m_req_acc;
};

// device type definition
DECLARE_DEVICE_TYPE(HASP_SAVQUEST, hasp_savquest_device)

#endif // MAME_BUS_CENTRONICS_HASP_SAVQUEST_H
