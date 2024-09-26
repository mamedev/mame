// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    remote488.h

*********************************************************************/

#ifndef MAME_BUS_IEEE488_REMOTE488_H
#define MAME_BUS_IEEE488_REMOTE488_H

#pragma once

#include "ieee488.h"
#include "imagedev/bitbngr.h"

class remote488_device : public device_t,
						 public device_ieee488_interface
{
public:
	// construction/destruction
	remote488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_ieee488_interface overrides
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// Position of signals in "S/R" msgs
	enum signal_bit {
		SIGNAL_ATN_BIT, // Bit 0
		SIGNAL_IFC_BIT, // Bit 1
		SIGNAL_REN_BIT, // Bit 2
		SIGNAL_SRQ_BIT, // Bit 3
		SIGNAL_COUNT
	};

	// Source handshake states
	enum {
		REM_SH_SIDS,
		REM_SH_SDYS,
		REM_SH_STRS
	};

	// Acceptor handshake states
	enum {
		REM_AH_AIDS,
		REM_AH_ACRS,
		REM_AH_ACDS,
		REM_AH_AWNS
	};

	// Stream rx states
	enum {
		REM_RX_WAIT_CH,
		REM_RX_WAIT_COLON,
		REM_RX_WAIT_1ST_HEX,
		REM_RX_WAIT_2ND_HEX,
		REM_RX_WAIT_SEP,
		REM_RX_WAIT_WS
	};

	required_device<bitbanger_device> m_stream;
	uint8_t m_in_signals;
	uint8_t m_out_signals;
	bool m_no_propagation;
	int m_sh_state;
	int m_ah_state;
	int m_rx_state;
	char m_rx_ch;
	uint8_t m_rx_data;
	bool m_flush_bytes;
	bool m_ibf;
	uint8_t m_ib;
	bool m_ib_eoi;
	emu_timer *m_poll_timer;
	emu_timer *m_hb_timer;
	emu_timer *m_ah_timer;
	unsigned m_connect_cnt;
	bool m_connected;
	uint8_t m_pp_data;
	bool m_pp_requested;
	uint8_t m_pp_dio;
	uint8_t m_sh_dio;
	bool m_waiting_checkpoint;


	TIMER_CALLBACK_MEMBER(process_input_msgs);
	TIMER_CALLBACK_MEMBER(heartbeat_tick);
	TIMER_CALLBACK_MEMBER(checkpoint_timeout_tick);

	void bus_reset();
	void set_connection(bool state);
	void recvd_data_byte(uint8_t data , bool eoi);
	void flush_data();
	void update_signals_from_rem(uint8_t to_set , uint8_t to_clear);
	void update_signal(signal_bit bit , int state);
	void update_state(uint8_t new_signals);
	void send_update(char type , uint8_t data);
	static bool a2hex(char c , uint8_t& out);
	static bool is_msg_type(char c);
	static bool is_terminator(char c);
	static bool is_space(char c);
	char recv_update(uint8_t& data);
	bool is_local_atn_active() const;
	void ah_checkpoint();
	void update_ah_fsm();
	void update_sh_fsm();
	bool is_local_pp_active() const;
	void update_pp();
	void update_pp_dio();
	void update_dio();
};

// device type definition
DECLARE_DEVICE_TYPE(REMOTE488, remote488_device)

#endif // MAME_BUS_IEEE488_REMOTE488_H
