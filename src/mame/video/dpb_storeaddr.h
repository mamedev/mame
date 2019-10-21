// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_storeaddr.h
    DPB-7000/1 - Store Address Card

***************************************************************************/

#ifndef MAME_VIDEO_DPB_STOREADDR_H
#define MAME_VIDEO_DPB_STOREADDR_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_storeaddr_card_device

class dpb7000_storeaddr_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_storeaddr_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void reg_w(uint16_t data);
	void s_type_w(int state);
	void cen_w(int state);

	void cxd_w(int state);
	void cxen_w(int state);
	void cxld_w(int state);
	void cxck_w(int state);
	void cxod_w(int state);
	void cxoen_w(int state);

	void cyd_w(int state);
	void cyen_w(int state);
	void cyld_w(int state);
	void cyck_w(int state);
	void cyod_w(int state);
	void cyoen_w(int state);

	void clrc_w(int state);
	void selvideo_w(int state);
	void creq_w(int state);
	void cr_w(int state);

	void prot_a_w(int state);
	void prot_b_w(int state);

	void preread_w(int state);

	void rvl_w(int state);
	void rhr_w(int state);
	void plt_w(int state);
	void zb_w(int state);
	void rppck_w(int state);
	void rb_w(int state);
	void pflag_w(int state);
	void b26_w(int state);

	void ipen_w(int state);

	auto ipsel() { return m_ipsel_out.bind(); }
	auto csel() { return m_csel_out.bind(); }
	auto rck() { return m_rck_out.bind(); }
	auto cck() { return m_cck_out.bind(); }
	auto ra() { return m_ra_out.bind(); }
	auto opstr() { return m_opstr_out.bind(); }
	auto opwa() { return m_opwa_out.bind(); }
	auto opwb() { return m_opwb_out.bind(); }
	auto opra() { return m_opra_out.bind(); }
	auto oprb() { return m_oprb_out.bind(); }
	auto blk() { return m_blk_out.bind(); }
	auto a() { return m_addr_out.bind(); }
	auto r_busy() { return m_r_busy_out.bind(); }
	auto ras() { return m_ras_out.bind(); }
	auto cas() { return m_cas_out.bind(); }
	auto write() { return m_w_out.bind(); }
	auto cbusy() { return m_cbusy_out.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static constexpr device_timer_id DELAY_TIMER = 0;

	void set_cxpos(uint16_t data);

	void tick_cxck();
	void tick_cyck();

	void update_r_busy(bool old_ras);

	void update_blanking_pal();
	void update_prot_proms();
	void update_rck();
	void update_v0();
	void update_opwa();
	void update_opstr();
	void update_req_clears();
	void update_cck();

	void update_addr_mux_inputs();
	void update_addr_mux_outputs();
	void update_addr_select_inputs();
	void update_addr_select_outputs();

	void check_cycle_start();
	void tick_delay_step();

	void request_r_read();
	void check_r_read();
	void request_c_read();
	void check_c_read();

	void opwb_w(bool state);

	// Timed control signals
	void mxr_w(bool state);
	void ras_w(bool state);
	void cas_w(bool state);
	void laac_w(bool state);
	void t6_w(bool state);
	void clrw_w(bool state);

	uint8_t *m_bb_base;
	uint8_t *m_bc_base;
	uint8_t *m_bd_base;
	uint8_t *m_protx_base;
	uint8_t *m_proty_base;
	uint8_t *m_blanking_base;

	uint8_t m_bb_out;
	uint8_t m_bc_out;
	uint8_t m_bd_out;
	bool m_protx;
	bool m_proty;

	emu_timer *m_delay_timer;
	uint8_t m_delay_step;

	uint8_t m_df_in[2];
	uint8_t m_df_out;
	uint8_t m_ee_in[2];
	uint8_t m_ee_out;

	uint8_t m_dg_in[2];
	uint8_t m_eg_in[2];
	uint8_t m_fg_in[2];
	uint8_t m_gg_in[2];
	uint8_t m_addr;

	uint16_t m_rhscr;
	uint16_t m_rvscr;
	uint8_t m_rzoom;
	uint8_t m_fld_sel;

	uint8_t m_hzoom_count;
	uint8_t m_vzoom_count;

	int8_t m_orig_cx_stripe_addr;
	int16_t m_orig_cx_stripe_num;
	int16_t m_orig_cy_addr;
	int8_t m_cx_stripe_addr;
	int16_t m_cx_stripe_num;
	int16_t m_cy_addr;

	uint8_t m_rhscr_stripe_addr;
	uint8_t m_rhscr_stripe_num;

	uint16_t m_rvscr_counter;
	uint16_t m_rvscr_with_v0;

	int m_s_type;

	bool m_cen;

	bool m_cxd;
	bool m_cxen;
	bool m_cxld;
	bool m_cxck;
	bool m_cxod;
	bool m_cxoen;

	bool m_cyd;
	bool m_cyen;
	bool m_cyld;
	bool m_cyck;
	bool m_cyod;
	bool m_cyoen;

	bool m_clrc;
	bool m_selvideo;
	bool m_creq;
	bool m_cread;

	bool m_prot_a;
	bool m_prot_b;

	bool m_preread;
	bool m_rreq_pending;
	bool m_rreq_active;
	bool m_creq_pending;
	bool m_creq_active;
	bool m_store_busy;

	bool m_rvl;
	bool m_rhr;
	bool m_plt;
	bool m_zb;
	bool m_rppck;
	bool m_rb;
	bool m_pflag;

	bool m_mxr;
	bool m_ras;
	bool m_cas;
	bool m_laac;
	bool m_t6;
	bool m_clrw;
	bool m_opstr;
	bool m_cck_clear;

	bool m_creq_sel;

	bool m_write_active;

	bool m_window_enable;
	bool m_b26;

	bool m_blank_d;
	bool m_blank_a;
	bool m_blank_b;
	uint8_t m_blank_q;

	bool m_crc;
	bool m_ipen;

	// Output Signals
	bool m_ipsel;
	bool m_rck;
	uint8_t m_ra;
	bool m_opra;
	bool m_opwa;
	bool m_opwb;
	bool m_cck;
	bool m_csel;

	// Output Handlers
	devcb_write_line m_ipsel_out;
	devcb_write_line m_rck_out;
	devcb_write8 m_ra_out;
	devcb_write_line m_opra_out;
	devcb_write_line m_oprb_out;
	devcb_write_line m_blk_out;
	devcb_write8 m_addr_out;
	devcb_write_line m_r_busy_out;
	devcb_write_line m_ras_out;
	devcb_write_line m_cas_out;
	devcb_write_line m_opwb_out;
	devcb_write_line m_opstr_out;
	devcb_write_line m_w_out;
	devcb_write_line m_opwa_out;
	devcb_write_line m_csel_out;
	devcb_write_line m_cck_out;
	devcb_write_line m_cbusy_out;

	// Devices
	required_memory_region m_x_prom;
	required_memory_region m_protx_prom;
	required_memory_region m_proty_prom;
	required_memory_region m_blanking_pal;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_STOREADDR, dpb7000_storeaddr_card_device)

#endif // MAME_VIDEO_DPB_STOREADDR_H
