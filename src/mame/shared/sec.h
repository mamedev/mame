// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SHARED_SEC_H
#define MAME_SHARED_SEC_H

#pragma once

#include "emu.h"

/* commands */

enum
{
	SEC_REQUEST_STATUS     = 0x20,
	SEC_REQUEST_MARKET     = 0x21,
	SEC_REQEUST_LAST_ERROR = 0x22,
	SEC_REQUEST_VERSION    = 0x23,
	SEC_REQUEST_COUNT_VAL  = 0x24,
	SEC_REQUEST_LAST_CMD   = 0x25,
	SEC_REQUEST_FINGERPRNT = 0x26,

	SEC_SET_NUM_COUNTERS   = 0x30,
	SEC_SET_MARKET         = 0x31,
	SEC_SET_COUNTER_TXT    = 0x32,

	SEC_SHOW_TEXT          = 0x40,
	SEC_SHOW_COUNTER_VAL   = 0x41,
	SEC_SHOW_COUNTER_TXT   = 0x42,
	SEC_SHOW_BITPATTERN    = 0x43,

	SEC_COUNT_INC_SMALL    = 0x50,
	SEC_COUNT_INC_MED      = 0x51,
	SEC_COUNT_INC_LARGE    = 0x52,

	SEC_COUNT_CYCLE_DISP   = 0x54,
	SEC_STOP_CYCLE         = 0x55,

	SEC_SELF_TEST          = 0x5c,

	SEC_DAT = 0x60,
	SEC_ACK = 0x61
};

class sec_device : public device_t
{
public:
	// construction/destruction
	sec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	/* serial interface */
	void clk_w(int state);
	void data_w(int state);
	void cs_w(int state);

	int data_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const size_t MAX_COUNTERS = 32;

	// stuff the SEC stores
	int         m_counters[MAX_COUNTERS]{};
	char        m_strings[MAX_COUNTERS][8]{};
	uint8_t     m_market = 0;
	uint8_t     m_nocnt = 0;
	uint8_t     m_last = 0;

	// serial comms
	uint8_t     m_curbyte = 0;
	uint8_t     m_data = 0;

	uint8_t     m_clk = 0;
	uint8_t     m_clks = 0;
	uint8_t     m_rxpos = 0;
	uint8_t     m_rxclk = 0;
	uint8_t     m_rxdat = 0;
	uint8_t     m_rxlen = 0;
	uint8_t     m_chars_left = 0;

	uint8_t     m_reqpos = 0;

	// communication buffer
	uint8_t     m_request[12]{};
	uint8_t     m_reply[8]{};

	bool        m_enabled = false;

	// execute command
	void        do_command(void);
	// command handlers
	void cmd_nop(void);
	void cmd_set_txt(void);
	void cmd_inc_sml(void);
	void cmd_inc_med(void);
	void cmd_inc_lrg(void);
	void cmd_set_ncn(void);
	void cmd_set_mrk(void);
	void cmd_get_sta(void);
	void cmd_get_mrk(void);
	void cmd_get_err(void);
	void cmd_get_fpr(void);
	void cmd_get_lst(void);
	void cmd_get_ver(void);
	void cmd_get_cnt(void);

	uint8_t calc_byte_sum(int length);
};

DECLARE_DEVICE_TYPE(SEC, sec_device)

#endif // MAME_SHARED_SEC_H
