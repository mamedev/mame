#ifndef MAME_CPU_M6502_ST2XXX_H
#define MAME_CPU_M6502_ST2XXX_H

#pragma once

#include "r65c02.h"

class st2xxx_device : public r65c02_device {
public:
	enum {
		ST_PDA = M6502_IR + 1,
		ST_PDB,
		ST_PDC,
		ST_PDD,
		ST_PDE,
		ST_PDF,
		ST_PDL,
		ST_PCA,
		ST_PCB,
		ST_PCC,
		ST_PCD,
		ST_PCE,
		ST_PCF,
		ST_PCL,
		ST_PSA,
		ST_PSB,
		ST_PSC,
		ST_PSD,
		ST_PSE,
		ST_PSF,
		ST_PSL,
		ST_PFC,
		ST_PFD,
		ST_PMCR,
		ST_SYS,
		ST_IRR,
		ST_PRR,
		ST_DRR,
		ST_IREQ,
		ST_IENA,
		ST_LSSA,
		ST_LVPW,
		ST_LXMAX,
		ST_LYMAX
	};

	auto in_pa_callback() { return m_in_port_cb[0].bind(); }
	auto out_pa_callback() { return m_out_port_cb[0].bind(); }
	auto in_pb_callback() { return m_in_port_cb[1].bind(); }
	auto out_pb_callback() { return m_out_port_cb[1].bind(); }
	auto in_pc_callback() { return m_in_port_cb[2].bind(); }
	auto out_pc_callback() { return m_out_port_cb[2].bind(); }
	auto in_pd_callback() { return m_in_port_cb[3].bind(); }
	auto out_pd_callback() { return m_out_port_cb[3].bind(); }
	auto in_pe_callback() { return m_in_port_cb[4].bind(); }
	auto out_pe_callback() { return m_out_port_cb[4].bind(); }
	auto in_pf_callback() { return m_in_port_cb[5].bind(); }
	auto out_pf_callback() { return m_out_port_cb[5].bind(); }
	auto in_pl_callback() { return m_in_port_cb[6].bind(); }
	auto out_pl_callback() { return m_out_port_cb[6].bind(); }

protected:
	st2xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map, int data_bits, u16 ireq_mask);

	virtual space_config_vector memory_space_config() const override;
	virtual void device_resolve_objects() override;
	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	class mi_st2xxx : public memory_interface {
	public:
		virtual u8 read_vector(u16 adr) = 0;

		address_space *data;
		memory_access_cache<0, 0, ENDIANNESS_LITTLE> *dcache;
		bool irq_service;
	};

	u8 read_vector(u16 adr) { return downcast<mi_st2xxx &>(*mintf).read_vector(adr); }
	void set_irq_service(bool state) { downcast<mi_st2xxx &>(*mintf).irq_service = state; }

	void update_irq_state() { irq_state = (m_ireq & m_iena) != 0; }
	u8 acknowledge_irq();

	u8 pdata_r(offs_t offset);
	void pdata_w(offs_t offset, u8 data);
	u8 pctrl_r(offs_t offset);
	void pctrl_w(offs_t offset, u8 data);
	u8 pl_r();
	void pl_w(u8 data);
	u8 pcl_r();
	void pcl_w(u8 data);
	u8 psc_r();
	void psc_w(u8 data);
	u8 pse_r();
	void pse_w(u8 data);
	u8 pfc_r();
	void pfc_w(u8 data);
	u8 pfd_r();
	void pfd_w(u8 data);

	u8 ireql_r();
	void ireql_w(u8 data);
	u8 ireqh_r();
	void ireqh_w(u8 data);
	u8 ienal_r();
	void ienal_w(u8 data);
	u8 ienah_r();
	void ienah_w(u8 data);

	void lssal_w(u8 data);
	void lssah_w(u8 data);
	void lvpw_w(u8 data);
	u8 lxmax_r();
	void lxmax_w(u8 data);
	u8 lymax_r();
	void lymax_w(u8 data);

#define O(o) void o ## _full(); void o ## _partial()

	O(brk_st_imp);
	O(rti_st_imp);
	O(reset_st);

#undef O

	address_space_config m_data_config;

	devcb_read8 m_in_port_cb[7];
	devcb_write8 m_out_port_cb[7];

	u8 m_pdata[7];
	u8 m_pctrl[7];
	u8 m_psel[7];
	u8 m_pfun[2];
	u8 m_pmcr;
	u16 m_ireq;
	u16 m_iena;
	const u16 m_ireq_mask;
	u8 m_sys;
	u16 m_lssa;
	u8 m_lvpw;
	u8 m_lxmax;
	u8 m_lymax;
};

#endif // MAME_CPU_M6502_ST2XXX_H
