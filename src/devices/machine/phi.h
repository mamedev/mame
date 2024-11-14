// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    phi.h

    HP PHI (Processor-to-Hpib-Interface) (1AA6-6x04)

*********************************************************************/

#ifndef MAME_MACHINE_PHI_H
#define MAME_MACHINE_PHI_H


class phi_device : public device_t
{
public:
	// construction/destruction
	phi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// See ieee488.h
	enum phi_488_signal_t
	{
		PHI_488_EOI,
		PHI_488_DAV,
		PHI_488_NRFD,
		PHI_488_NDAC,
		PHI_488_IFC,
		PHI_488_SRQ,
		PHI_488_ATN,
		PHI_488_REN,
		PHI_488_SIGNAL_COUNT
	};

	// Set read and write callbacks to access DIO bus on IEEE-488
	auto dio_read_cb() { return m_dio_read_func.bind(); }
	auto dio_write_cb() { return m_dio_write_func.bind(); }
	// Set write callbacks to access uniline signals on IEEE-488
	auto eoi_write_cb() { return m_signal_wr_fns[PHI_488_EOI].bind(); }
	auto dav_write_cb() { return m_signal_wr_fns[PHI_488_DAV].bind(); }
	auto nrfd_write_cb() { return m_signal_wr_fns[PHI_488_NRFD].bind(); }
	auto ndac_write_cb() { return m_signal_wr_fns[PHI_488_NDAC].bind(); }
	auto ifc_write_cb() { return m_signal_wr_fns[PHI_488_IFC].bind(); }
	auto srq_write_cb() { return m_signal_wr_fns[PHI_488_SRQ].bind(); }
	auto atn_write_cb() { return m_signal_wr_fns[PHI_488_ATN].bind(); }
	auto ren_write_cb() { return m_signal_wr_fns[PHI_488_REN].bind(); }
	// Set write callback for INT signal
	auto int_write_cb() { return m_int_write_func.bind(); }
	// Set write callback for DMARQ signal
	auto dmarq_write_cb() { return m_dmarq_write_func.bind(); }
	// Set read callback for SYS_CNTRL signal
	auto sys_cntrl_read_cb() { return m_sys_cntrl_read_func.bind(); }

	void eoi_w(int state);
	void dav_w(int state);
	void nrfd_w(int state);
	void ndac_w(int state);
	void ifc_w(int state);
	void srq_w(int state);
	void atn_w(int state);
	void ren_w(int state);

	void bus_dio_w(uint8_t data);

	void set_ext_signal(phi_488_signal_t signal, int state);

	// Register read/write
	// Mapping of PHI register bits:
	// Reg. bit PHI bit
	// =================
	// 15       0
	// 14       1
	// 13       =0=
	// 12       =0=
	// 11       =0=
	// 10       =0=
	// 9        =0=
	// 8        =0=
	// 7        8
	// 6        9
	// 5        10
	// 4        11
	// 3        12
	// 2        13
	// 1        14
	// 0        15
	void reg16_w(offs_t offset, uint16_t data);
	uint16_t reg16_r(offs_t offset);
	void reg8_w(offs_t offset, uint8_t data);
	uint8_t reg8_r(offs_t offset);

protected:
	phi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(delayed_update);

private:
	// Depth of inbound/outbound FIFOs
	static constexpr unsigned FIFO_SIZE = 8;

	devcb_read8 m_dio_read_func;
	devcb_write8 m_dio_write_func;
	devcb_write_line::array<PHI_488_SIGNAL_COUNT> m_signal_wr_fns;
	devcb_write_line m_int_write_func;
	devcb_write_line m_dmarq_write_func;
	devcb_read_line m_sys_cntrl_read_func;
	bool m_int_line;
	bool m_dmarq_line;

	// Internal copy of bus signals
	// These signals have the "right" polarity (i.e. the opposite of bus signals, 1=L)
	uint8_t m_dio;
	bool m_signals[PHI_488_SIGNAL_COUNT];
	bool m_ext_signals[PHI_488_SIGNAL_COUNT];

	bool m_no_recursion;

	bool m_sys_controller;
	bool m_loopback;
	bool m_id_enabled;

	// SH (Source Handshake) states
	enum {
		PHI_SH_SIDS,    // & SIWS
		PHI_SH_SGNS,    // & SWNS
		PHI_SH_SDYS,
		PHI_SH_STRS
	};

	int m_sh_state;

	// AH (Acceptor Handshake) states
	enum {
		PHI_AH_AIDS,
		PHI_AH_ANRS,
		PHI_AH_ACRS,
		PHI_AH_ACDS,
		PHI_AH_ACDS_FROZEN, // Non-standard state: IF CMD rejected because of even parity
		PHI_AH_AWNS
	};

	int m_ah_state;

	// T (Talker) states
	enum {
		PHI_T_TIDS,
		PHI_T_TADS,
		PHI_T_SPAS,
		PHI_T_TACS,
		// The following are non-standard states for IDENTIFY sequencing
		PHI_T_ID1,  // Addressed by secondary address
		PHI_T_ID2,  // Sending 1st byte
		PHI_T_ID3,  // Waiting to send 2nd byte
		PHI_T_ID4,  // Sending 2nd byte
		PHI_T_ID5   // 2nd byte sent, end of sequence
	};

	int m_t_state;
	bool m_t_spms;  // False: SPIS, true: SPMS

	// L (Listener) states
	enum {
		PHI_L_LIDS,
		PHI_L_LADS,
		PHI_L_LACS
	};

	int m_l_state;

	// SR (Service Request) states
	enum {
		PHI_SR_NPRS,
		PHI_SR_SRQS,
		PHI_SR_APRS
	};

	int m_sr_state;

	// RL (Remote Local) states
	bool m_rl_rems; // false: LOCS, true: REMS

	// PP (Parallel poll) states
	enum {
		PHI_PP_PPIS,
		PHI_PP_PPSS,
		PHI_PP_PPAS
	};

	int m_pp_state;
	uint8_t m_ppr_msg;
	bool m_s_sense;

	// C (Controller) states
	enum {
		PHI_C_CIDS,
		PHI_C_CADS,
		PHI_C_CACS,
		PHI_C_CPPS,
		PHI_C_CSBS,
		PHI_C_CSHS,
		PHI_C_CAWS,
		PHI_C_CTRS,
		PHI_C_CSWS
	};

	int m_c_state;

	// Secondary address decoder states
	enum {
		PHI_SA_NONE,
		PHI_SA_PACS,
		PHI_SA_TPAS,
		PHI_SA_LPAS,
		PHI_SA_UNT
	};

	int m_sa_state;

	uint8_t m_be_counter;
	uint16_t m_reg_status;
	uint16_t m_reg_int_cond;
	uint16_t m_reg_int_mask;
	uint16_t m_reg_1st_id;
	uint16_t m_reg_2nd_id;
	uint16_t m_reg_control;
	uint16_t m_reg_address;
	util::fifo<uint16_t, FIFO_SIZE> m_fifo_in;
	util::fifo<uint16_t, FIFO_SIZE> m_fifo_out;

	typedef enum {
		NBA_NONE,
		NBA_CMD_FROM_OFIFO,
		NBA_BYTE_FROM_OFIFO,
		NBA_FROM_SPAS,
		NBA_FROM_ID2,
		NBA_FROM_ID4
	} nba_origin_t;

	int m_nba_origin;

	// Timers
	emu_timer *m_sh_dly_timer;
	emu_timer *m_c_dly_timer;

	void int_reg_w(offs_t offset, uint16_t data);

	uint8_t get_dio(void);
	void set_dio(uint8_t data);
	bool get_signal(phi_488_signal_t signal);
	void set_signal(phi_488_signal_t signal, bool state);

	void pon_msg(void);
	void update_488(void);
	void update_fsm(void);
	nba_origin_t nba_msg(uint8_t& new_byte, bool& new_eoi) const;
	void clear_nba(nba_origin_t origin);
	bool if_cmd_received(uint8_t byte);
	bool byte_received(uint8_t byte, bool eoi);
	void rx_n_data_freeze(uint16_t word);
	bool ton_msg(void) const;
	bool lon_msg(void) const;
	bool odd_parity(uint8_t byte) const;
	uint8_t my_address(void) const;
	bool tcs_msg(void) const;
	bool rpp_msg(void) const;
	uint8_t get_pp_response();
	bool controller_in_charge(void) const;
	void configure_pp_response();
	void update_pp();
	void update_interrupt();
	void update_dmarq();
};

// device type definition
DECLARE_DEVICE_TYPE(PHI, phi_device)

#endif // MAME_MACHINE_PHI_H
