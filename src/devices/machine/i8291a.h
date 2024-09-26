// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

    i8291a.h

    I8291A GPIB controller

*********************************************************************/

#ifndef MAME_MACHINE_I8291A_H
#define MAME_MACHINE_I8291A_H

#pragma once

class i8291a_device : public device_t
{
public:
	// construction/destruction
	i8291a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_write() { return m_int_write_func.bind(); }
	auto dreq_write() { return m_dreq_write_func.bind(); }
	auto trig_write() { return m_trig_write_func.bind(); }
	auto eoi_write() { return m_eoi_write_func.bind(); }
	auto dav_write() { return m_dav_write_func.bind(); }
	auto nrfd_write() { return m_nrfd_write_func.bind(); }
	auto ndac_write() { return m_ndac_write_func.bind(); }
	auto srq_write() { return m_srq_write_func.bind(); }
	auto dio_write() { return m_dio_write_func.bind(); }
	auto dio_read() { return m_dio_read_func.bind(); }

	// Signal inputs
	void reset_w(int state);
	void dack_w(int state);

	// GPIB port
	void eoi_w(int state);
	void dav_w(int state);
	void nrfd_w(int state);
	void ndac_w(int state);
	void ifc_w(int state);
	void srq_w(int state);
	void atn_w(int state);
	void ren_w(int state);
	void dio_w(uint8_t data); // declared but not defined?

	// register r/w functions

	void dout_w(uint8_t data);
	void ie1_w(uint8_t data);
	void ie2_w(uint8_t data);
	void spoll_mode_w(uint8_t data);
	void addr_mode_w(uint8_t data);
	void aux_mode_w(uint8_t data);
	void addr01_w(uint8_t data);
	void eos_w(uint8_t data);

	uint8_t din_r();
	uint8_t ints1_r();
	uint8_t ints2_r();
	uint8_t spoll_stat_r();
	uint8_t addr_stat_r();
	uint8_t cpt_r();
	uint8_t addr0_r();
	uint8_t addr1_r();
	void map(address_map &map) ATTR_COLD;

private:

	// signal output
	devcb_write_line m_int_write_func;
	devcb_write_line m_dreq_write_func;
	devcb_write_line m_trig_write_func;
	devcb_write_line m_eoi_write_func;
	devcb_write_line m_dav_write_func;
	devcb_write_line m_nrfd_write_func;
	devcb_write_line m_ndac_write_func;
	devcb_write_line m_srq_write_func;

	devcb_write8 m_dio_write_func;
	devcb_read8 m_dio_read_func;

	void set_dav(bool state);
	void set_nrfd(bool state);
	void set_ndac(bool state);
	void set_eoi(bool state);
	void set_srq(bool state);


	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_int();
	void handle_command();
	void run_fsm();
	void run_sh_fsm();
	void run_ah_fsm();
	void run_dt_fsm();
	void run_dc_fsm();
	void run_l_fsm();
	void run_lp_fsm();
	void run_t_fsm();
	void run_tp_fsm();
	void run_tsp_fsm();
	void run_sp_fsm();
	void run_pp_fsm();
	void run_rl_fsm();

	// registers

	uint8_t m_din;
	uint8_t m_dout;
	uint8_t m_ints1;
	uint8_t m_ints2;
	uint8_t m_ie1;
	uint8_t m_ie2;
	uint8_t m_address0;
	uint8_t m_address1;
	uint8_t m_eos;
	uint8_t m_spoll_mode;
	uint8_t m_address_mode;
	uint8_t m_address_status;
	uint8_t m_cpt;

	uint8_t m_auxa;
	uint8_t m_auxb;

	bool m_atn;
	bool m_ren;
	bool m_nrfd;
	bool m_ndac;
	bool m_dav;
	bool m_srq;
	bool m_ifc;
	bool m_eoi;
	uint8_t m_dio;

	bool m_nrfd_out;
	bool m_ndac_out;
	bool m_dav_out;
	bool m_srq_out;
	bool m_eoi_out;

	// internal signals

	bool m_pon;
	bool m_rdy;
	bool m_lpe;
	bool m_ist;
	bool m_rtl;
	bool m_apt_flag;
	bool m_cpt_flag;
	bool m_din_flag;
	bool m_nba;

	bool m_pp_sense;
	uint8_t m_pp_line;

	bool m_send_eoi;

	static constexpr int REG_INTS1_BI = 1 << 0;
	static constexpr int REG_INTS1_BO = 1 << 1;
	static constexpr int REG_INTS1_ERR = 1 << 2;
	static constexpr int REG_INTS1_DEC = 1 << 3;
	static constexpr int REG_INTS1_END = 1 << 4;
	static constexpr int REG_INTS1_GET = 1 << 5;
	static constexpr int REG_INTS1_APT = 1 << 6;
	static constexpr int REG_INTS1_CPT = 1 << 7;

	static constexpr int REG_INTS2_ADSC = 1 << 0;
	static constexpr int REG_INTS2_REMC = 1 << 1;
	static constexpr int REG_INTS2_LLOC = 1 << 2;
	static constexpr int REG_INTS2_SPC = 1 << 3;
	static constexpr int REG_INTS2_REM = 1 << 4;
	static constexpr int REG_INTS2_LLO = 1 << 5;
	static constexpr int REG_INTS2_SPAS = 1 << 6;
	static constexpr int REG_INTS2_INT = 1 << 7;

	static constexpr int REG_IE2_DMAI = 1 << 4;
	static constexpr int REG_IE2_DMAO = 1 << 5;

	static constexpr int REG_ADDRESS01_ARS = 1 << 7;
	static constexpr int REG_ADDRESS0_INT = 1 << 7;
	static constexpr int REG_ADDRESS_DT = 1 << 6;
	static constexpr int REG_ADDRESS_DL = 1 << 5;

	static constexpr int REG_ADDRESS_STATUS_MJMN = 1 << 0;
	static constexpr int REG_ADDRESS_STATUS_TA = 1 << 1;
	static constexpr int REG_ADDRESS_STATUS_LA = 1 << 2;
	static constexpr int REG_ADDRESS_STATUS_TPAS = 1 << 3;
	static constexpr int REG_ADDRESS_STATUS_LPAS = 1 << 4;
	static constexpr int REG_ADDRESS_STATUS_EOI = 1 << 5;
	static constexpr int REG_ADDRESS_STATUS_LON = 1 << 6;
	static constexpr int REG_ADDRESS_STATUS_TON = 1 << 7;

	static constexpr int REG_AUXB_CPT_ENABLE = 1 << 0;
	static constexpr int REG_AUXB_EOI_SPAS_ENABLE = 1 << 1;
	static constexpr int REG_AUXB_HS_TRANSFER = 1 << 2;
	static constexpr int REG_AUXB_INT_ACTIVE_LOW = 1 << 3;
	static constexpr int REG_AUXB_RFD_HOLDOFF_GET_DEC = 1 << 4;

	static constexpr int REG_AUXA_RFD_HOLDOFF_DATA = 1 << 0;
	static constexpr int REG_AUXA_RFD_HOLDOFF_END = 1 << 1;
	static constexpr int REG_AUXA_END_ON_EOS = 1 << 2;
	static constexpr int REG_AUXA_EOI_ON_EOS = 1 << 3;
	static constexpr int REG_AUXA_EOS_8BIT = 1 << 4;

	// AUX CMDs
	static constexpr int AUXCMD_IMMEDIATE_EXEC_PON = 0;
	static constexpr int AUXCMD_CLEAR_PP = 1;
	static constexpr int AUXCMD_CHIP_RESET = 2;
	static constexpr int AUXCMD_FINISH_HANDSHAKE = 3;
	static constexpr int AUXCMD_TRIGGER = 4;
	static constexpr int AUXCMD_CLEAR_RTL = 5;
	static constexpr int AUXCMD_SEND_EOI = 6;
	static constexpr int AUXCMD_NON_VALID_SA = 7;
	static constexpr int AUXCMD_PON = 8;
	static constexpr int AUXCMD_SET_PP = 9;
	static constexpr int AUXCMD_SET_RTL = 13;
	static constexpr int AUXCMD_VALID_SA = 15;

	// Interface commands
	// TODO: stolen from tms9914, move to common header file. PHI also defines this

	static constexpr uint8_t IFCMD_MASK       = 0x7f;  // Mask of valid bits in if. commands
	static constexpr uint8_t IFCMD_ACG_MASK   = 0x70;  // Mask of ACG commands
	static constexpr uint8_t IFCMD_ACG_VALUE  = 0x00;  // Value of ACG commands
	static constexpr uint8_t IFCMD_UCG_MASK   = 0x70;  // Mask of UCG commands
	static constexpr uint8_t IFCMD_UCG_VALUE  = 0x10;  // Value of UCG commands
	static constexpr uint8_t IFCMD_GROUP_MASK = 0x60;  // Mask of group id
	static constexpr uint8_t IFCMD_LAG_VALUE  = 0x20;  // Value of LAG commands
	static constexpr uint8_t IFCMD_TAG_VALUE  = 0x40;  // Value of TAG commands
	static constexpr uint8_t IFCMD_SCG_VALUE  = 0x60;  // Value of SCG commands
	static constexpr uint8_t IFCMD_GTL        = 0x01;  // Go to local
	static constexpr uint8_t IFCMD_SDC        = 0x04;  // Selected device clear
	static constexpr uint8_t IFCMD_GET        = 0x08;  // Group execute trigger
	static constexpr uint8_t IFCMD_TCT        = 0x09;  // Take control
	static constexpr uint8_t IFCMD_LLO        = 0x11;  // Local lock-out
	static constexpr uint8_t IFCMD_DCL        = 0x14;  // Device clear
	static constexpr uint8_t IFCMD_SPE        = 0x18;  // Serial poll enable
	static constexpr uint8_t IFCMD_SPD        = 0x19;  // Serial poll disable
	static constexpr uint8_t IFCMD_UNL        = 0x3f;  // Unlisten
	static constexpr uint8_t IFCMD_UNT        = 0x5f;  // Untalk

	enum class source_handshake_state : uint8_t {
		SIDS,
		SGNS,
		SDYS,
		STRS
	};

	enum class acceptor_handshake_state : uint8_t {
		AIDS,
		ANRS,
		ACRS,
		AWNS,
		ADYS,
		ACDS
	};

	enum class talker_state : uint8_t {
		TIDS,
		TADS,
		SPAS,
		TACS
	};

	enum class talker_primary_state : uint8_t {
		TPIS,
		TPAS
	};

	enum class listener_primary_state : uint8_t {
		LPIS,
		LPAS
	};

	enum class talker_serial_poll_state : uint8_t {
		SPIS,
		SPMS
	};

	enum class serial_poll_state : uint8_t {
		NPRS,
		SRQS,
		APRS
	};

	enum class listener_state : uint8_t {
		LIDS,
		LADS,
		LACS
	};

	enum class remote_local_state : uint8_t {
		LOCS,
		REMS,
		RWLS,
		LWLS
	};

	enum class parallel_poll_state : uint8_t {
		PPIS,
		PPSS,
		PPAS
	};

	enum class device_clear_state : uint8_t {
		DCIS,
		DCAS
	};

	enum class device_trigger_state : uint8_t {
		DTIS,
		DTAS
	};

	static const char *get_state_name(acceptor_handshake_state state);
	static const char *get_state_name(source_handshake_state state);
	static const char *get_state_name(talker_state state);
	static const char *get_state_name(talker_primary_state state);
	static const char *get_state_name(talker_serial_poll_state state);
	static const char *get_state_name(listener_state state);
	static const char *get_state_name(listener_primary_state state);
	static const char *get_state_name(device_clear_state state);
	static const char *get_state_name(device_trigger_state state);
	static const char *get_state_name(parallel_poll_state state);
	static const char *get_state_name(serial_poll_state state);
	static const char *get_state_name(remote_local_state state);

	template<typename T> void update_state(T &name, T state);

	source_handshake_state m_sh_state;
	acceptor_handshake_state m_ah_state;
	talker_state m_t_state;
	talker_primary_state m_tp_state;
	talker_serial_poll_state m_tsp_state;
	listener_state m_l_state;
	listener_primary_state m_lp_state;
	remote_local_state m_rl_state;
	serial_poll_state m_sp_state;
	parallel_poll_state m_pp_state;
	device_clear_state m_dc_state;
	device_trigger_state m_dt_state;
	bool m_state_changed;
	bool m_ignore_ext_signals;
	bool m_intr_out;
	bool m_dreq_out;
};

// device type definition
DECLARE_DEVICE_TYPE(I8291A, i8291a_device)

#endif // MAME_MACHINE_I8291A_H
