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

	// Timers
	enum {
		TMR_ID_POLL,
		TMR_ID_HEARTBEAT,
		TMR_ID_AH
	};

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

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
	uint8_t m_in_signals = 0;
	uint8_t m_out_signals = 0;
	bool m_no_propagation = false;
	int m_sh_state = 0;
	int m_ah_state = 0;
	int m_rx_state = 0;
	char m_rx_ch = 0;
	uint8_t m_rx_data = 0;
	bool m_flush_bytes = false;
	bool m_ibf = false;
	uint8_t m_ib = 0;
	bool m_ib_eoi = false;
	emu_timer *m_poll_timer = nullptr;
	emu_timer *m_hb_timer = nullptr;
	emu_timer *m_ah_timer = nullptr;
	unsigned m_connect_cnt = 0;
	bool m_connected = false;
	uint8_t m_pp_data = 0;
	bool m_pp_requested = false;
	uint8_t m_pp_dio = 0;
	uint8_t m_sh_dio = 0;
	bool m_waiting_cp = false;

	void bus_reset();
	void process_input_msgs();
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
