// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    tms9914.h

    Texas Instruments TMS9914(A) GPIB Controller

                            _____   _____
                ACCRQ/   1 |*    \_/     | 40  Vcc
                ACCGR/   2 |             | 39  TR
                   CE/   3 |             | 38  DIO1
                   WE/   4 |             | 37  DIO2
                  DBIN   5 |             | 36  DIO3
                   RS0   6 |             | 35  DIO4
                   RS1   7 |             | 34  DIO5
                   RS2   8 |             | 33  DIO6
                  INT/   9 |             | 32  DIO7
                    D7  10 |  TMS9914    | 31  DIO8
                    D6  11 |  TMS9914A   | 30  CONT/
                    D5  12 |             | 29  SRQ
                    D4  13 |             | 28  ATN
                    D3  14 |             | 27  EOI
                    D2  15 |             | 26  DAV
                    D1  16 |             | 25  NRFD
                    D0  17 |             | 24  NDAC
                    O/  18 |             | 23  IFC
                RESET/  19 |             | 22  REN
                   Vss  20 |_____________| 21  TE

**********************************************************************/

#ifndef MAME_MACHINE_TMS9914_H
#define MAME_MACHINE_TMS9914_H

#pragma once

class tms9914_device : public device_t
{
public:
	// construction/destruction
	tms9914_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// See ieee488.h
	enum ieee_488_signal_t {
		IEEE_488_EOI,
		IEEE_488_DAV,
		IEEE_488_NRFD,
		IEEE_488_NDAC,
		IEEE_488_IFC,
		IEEE_488_SRQ,
		IEEE_488_ATN,
		IEEE_488_REN,
		IEEE_488_SIGNAL_COUNT
	};

	// Set read and write callbacks to access DIO bus on IEEE-488
	auto dio_read_cb() { return m_dio_read_func.bind(); }
	auto dio_write_cb() { return m_dio_write_func.bind(); }

	// Set write callbacks to access uniline signals on IEEE-488
	auto eoi_write_cb() { return m_signal_wr_fns[IEEE_488_EOI].bind(); }
	auto dav_write_cb() { return m_signal_wr_fns[IEEE_488_DAV].bind(); }
	auto nrfd_write_cb() { return m_signal_wr_fns[IEEE_488_NRFD].bind(); }
	auto ndac_write_cb() { return m_signal_wr_fns[IEEE_488_NDAC].bind(); }
	auto ifc_write_cb() { return m_signal_wr_fns[IEEE_488_IFC].bind(); }
	auto srq_write_cb() { return m_signal_wr_fns[IEEE_488_SRQ].bind(); }
	auto atn_write_cb() { return m_signal_wr_fns[IEEE_488_ATN].bind(); }
	auto ren_write_cb() { return m_signal_wr_fns[IEEE_488_REN].bind(); }

	// Set write callback for INT signal
	auto int_write_cb() { return m_int_write_func.bind(); }

	// Set write callback for ACCRQ signal
	auto accrq_write_cb() { return m_accrq_write_func.bind(); }

	void eoi_w(int state);
	void dav_w(int state);
	void nrfd_w(int state);
	void ndac_w(int state);
	void ifc_w(int state);
	void srq_w(int state);
	void atn_w(int state);
	void ren_w(int state);

	// Register access
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	// CONT output: true when 9914 is current controller-in-charge
	int cont_r();

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(fsm_tick);

	devcb_read8 m_dio_read_func;
	devcb_write8 m_dio_write_func;
	devcb_write_line::array<IEEE_488_SIGNAL_COUNT> m_signal_wr_fns;
	devcb_write_line m_int_write_func;
	devcb_write_line m_accrq_write_func;
	bool m_int_line;
	bool m_accrq_line;

	uint8_t m_dio;
	bool m_signals[ IEEE_488_SIGNAL_COUNT ];
	bool m_ext_signals[ IEEE_488_SIGNAL_COUNT ];
	bool m_no_reflection;
	bool m_ext_state_change;

	// Registers
	uint8_t m_reg_int0_status;
	uint8_t m_reg_int0_mask;
	uint8_t m_reg_int1_status;
	uint8_t m_reg_int1_mask;
	uint8_t m_reg_address;
	uint8_t m_reg_serial_p;
	uint8_t m_reg_2nd_serial_p;
	uint8_t m_reg_parallel_p;
	uint8_t m_reg_2nd_parallel_p;
	uint8_t m_reg_di;
	uint8_t m_reg_do;
	bool m_reg_ulpa;

	// Auxiliary cmd states
	bool m_swrst;
	bool m_hdfa;
	bool m_hdfe;
	bool m_rtl;
	bool m_gts;
	bool m_rpp;
	bool m_sic;
	bool m_sre;
	bool m_dai;
	bool m_pts;
	bool m_stdl;
	bool m_shdw;
	bool m_vstdl;
	bool m_rsvd2;

	// AH (Acceptor Handshake) states
	enum {
		FSM_AH_AIDS,
		FSM_AH_ANRS,
		FSM_AH_ACRS,
		FSM_AH_ACDS1,
		FSM_AH_ACDS2,
		FSM_AH_AWNS
	};

	int m_ah_state;
	bool m_ah_adhs;
	bool m_ah_anhs;
	bool m_ah_aehs;

	// SH (Source Handshake) states
	enum {
		FSM_SH_SIDS,
		FSM_SH_SGNS,
		FSM_SH_SDYS,
		FSM_SH_SERS,
		FSM_SH_STRS
	};

	int m_sh_state;
	bool m_sh_shfs;
	bool m_sh_vsts;

	// T (Talker) states
	enum {
		FSM_T_TIDS,
		FSM_T_TADS,
		FSM_T_TACS,
		FSM_T_SPAS
	};

	int m_t_state;
	bool m_t_tpas;
	bool m_t_spms;

	// Talker EOI generator states
	enum {
		FSM_T_ENIS,
		FSM_T_ENRS,
		FSM_T_ERAS,
		FSM_T_ENAS
	};

	int m_t_eoi_state;

	// L (Listener) states
	enum {
		FSM_L_LIDS,
		FSM_L_LADS,
		FSM_L_LACS
	};

	int m_l_state;
	bool m_l_lpas;

	// SR (Service request) states
	enum {
		FSM_SR_NPRS,
		FSM_SR_SRQS,
		FSM_SR_APRS1,
		FSM_SR_APRS2
	};

	int m_sr_state;

	// RL (Remote Local) states
	enum {
		FSM_RL_LOCS,
		FSM_RL_REMS,
		FSM_RL_RWLS,
		FSM_RL_LWLS
	};

	int m_rl_state;

	// PP (Parallel poll) states
	bool m_pp_ppas;

	// C (Controller) states
	enum {
		FSM_C_CIDS,
		FSM_C_CADS,
		FSM_C_CACS,
		FSM_C_CSBS,
		FSM_C_CWAS,
		FSM_C_CSHS,
		FSM_C_CSWS,
		FSM_C_CAWS,
		FSM_C_CPWS
	};

	int m_c_state;

	// Timers
	emu_timer *m_sh_dly_timer;
	emu_timer *m_ah_dly_timer;
	emu_timer *m_c_dly_timer;

	uint8_t get_dio();
	void set_dio(uint8_t data);
	bool get_signal(ieee_488_signal_t signal) const;
	bool get_ifcin() const;
	void set_ext_signal(ieee_488_signal_t signal , int state);
	void set_signal(ieee_488_signal_t signal , bool state);
	void do_swrst();
	bool listener_reset() const;
	bool talker_reset() const;
	bool controller_reset() const;
	bool sh_active() const;
	void update_fsm();
	bool is_my_address(uint8_t addr);
	void do_LAF();
	void do_TAF();
	void if_cmd_received(uint8_t if_cmd);
	void dab_received(uint8_t dab , bool eoi);
	void do_aux_cmd(unsigned cmd , bool set_bit);
	void set_int0_bit(unsigned bit_no);
	void set_int1_bit(unsigned bit_no);
	void update_int();
	void update_ifc();
	void update_ren();
	void set_accrq(bool state);
	bool m_next_eoi;
};

// device type definition
DECLARE_DEVICE_TYPE(TMS9914, tms9914_device)

#endif // MAME_MACHINE_TMS9914_H
