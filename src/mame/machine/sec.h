// license:BSD-3-Clause
// copyright-holders:David Haywood

/* commands */

#define SEC_REQUEST_STATUS     (0x20)
#define SEC_REQUEST_MARKET     (0x21)
#define SEC_REQEUST_LAST_ERROR (0x22)
#define SEC_REQUEST_VERSION    (0x23)
#define SEC_REQUEST_COUNT_VAL  (0x24)
#define SEC_REQUEST_LAST_CMD   (0x25)
#define SEC_REQUEST_FINGERPRNT (0x26)

#define SEC_SET_NUM_COUNTERS   (0x30)
#define SEC_SET_MARKET         (0x31)
#define SEC_SET_COUNTER_TXT    (0x32)

#define SEC_SHOW_TEXT          (0x40)
#define SEC_SHOW_COUNTER_VAL   (0x41)
#define SEC_SHOW_COUNTER_TXT   (0x42)
#define SEC_SHOW_BITPATTERN    (0x43)

#define SEC_COUNT_INC_SMALL    (0x50)
#define SEC_COUNT_INC_MED      (0x51)
#define SEC_COUNT_INC_LARGE    (0x52)

#define SEC_COUNT_CYCLE_DISP   (0x54)
#define SEC_STOP_CYCLE         (0x55)

#define SEC_SELF_TEST          (0x5c)

#define MAX_COUNTERS (32)

#define SEC_DAT  (0x60)
#define SEC_ACK  (0x61)

class SEC
{
public:
	SEC()
	{
		reset();
	}

// Internal registers

private:

	// stuff the SEC stores
	int m_counters[MAX_COUNTERS];
	char m_strings[MAX_COUNTERS][8];
	uint8_t m_market;
	uint8_t m_nocnt;
	uint8_t m_last;

	// serial comms
	uint8_t m_curbyte;
	uint8_t m_data;

	uint8_t m_clk;
	uint8_t m_clks;
	uint8_t m_rxpos;
	uint8_t m_rxclk;
	uint8_t m_rxdat;
	uint8_t m_rxlen;
	uint8_t chars_left;

	uint8_t n_reqpos;

	// communication buffer
	uint8_t m_request[12];
	uint8_t m_reply[8];

	bool             enabled;

	// execute command
	void             Do_Command(void);
	// command handlers
	void Cmd_NOP(void);
	void Cmd_Set_Txt(void);
	void Cmd_Inc_Sml(void);
	void Cmd_Inc_Med(void);
	void Cmd_Inc_Lrg(void);
	void Cmd_Set_Ncn(void);
	void Cmd_Set_Mrk(void);
	void Cmd_Get_Sta(void);
	void Cmd_Get_Mrk(void);
	void Cmd_Get_Err(void);
	void Cmd_Get_Fpr(void);
	void Cmd_Get_Lst(void);
	void Cmd_Get_Ver(void);
	void Cmd_Get_Cnt(void);

	uint8_t       CalcByteSum(int length);

public:
	void             reset(void);

	/* serial interface */
	void write_clock_line(uint8_t bit);
	void write_data_line(uint8_t bit);
	void write_cs_line(uint8_t bit);
	uint8_t read_data_line(void);
};
