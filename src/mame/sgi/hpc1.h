// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC1 "High-performance Peripheral Controller" emulation

**********************************************************************/

#ifndef MAME_SGI_HPC1_H
#define MAME_SGI_HPC1_H

#pragma once

#include "machine/edlc.h"

class hpc1_device
	: public device_t
	, public device_memory_interface
{
public:
	hpc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_gio(T &&tag, int spacenum) { m_gio.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_enet(T &&tag) { m_enet.set_tag(std::forward<T>(tag)); }

	auto int_w() { return m_int_w.bind(); }
	template <unsigned N> auto dma_r_cb() { return m_dma_r[N].bind(); }
	template <unsigned N> auto dma_w_cb() { return m_dma_w[N].bind(); }
	auto eeprom_dati() { return m_eeprom_dati.bind(); }
	auto eeprom_out() { return m_eeprom_out.bind(); }

	template <unsigned N> void write_drq(int state);
	void write_int(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// dma
	void enet_dma();
	void scsi_dma();
	void scsi_chain();

	// ethernet read handlers
	u32 cxbp_r() { return m_enet_cxbp; }
	u32 nxbdp_r() { return m_enet_nxbdp; }
	u32 xbc_r() { return m_enet_xbc; }
	u32 cxbdp_r() { return m_enet_cxbdp; }
	u32 cpfxbdp_r() { return m_enet_cpfxbdp; }
	u32 ppfxbdp_r() { return m_enet_ppfxbdp; }
	u32 intdelay_r() { return m_enet_intdelay; }
	u32 trstat_r();
	u32 rcvstat_r();
	u32 ctl_r() { return m_enet_ctrl; }
	u32 rbc_r() { return m_enet_rbc; }
	u32 crbp_r() { return m_enet_crbp; }
	u32 nrbdp_r() { return m_enet_nrbdp; }
	u32 crbdp_r() { return m_enet_crbdp; }

	// ethernet write handlers
	void cxbp_w(u32 data);
	void nxbdp_w(u32 data);
	void xbc_w(u32 data);
	void cxbdp_w(u32 data);
	void cpfxbdp_w(u32 data);
	void ppfxbdp_w(u32 data);
	void intdelay_w(u32 data);
	void trstat_w(u32 data);
	void rcvstat_w(u32 data);
	void ctl_w(u32 data);
	void rbc_w(u32 data);
	void crbp_w(u32 data);
	void nrbdp_w(u32 data);
	void crbdp_w(u32 data);

	// scsi read handlers
	u32 scsi_bc_r() { return m_scsi_bc; }
	u32 scsi_cbp_r() { return m_scsi_cbp; }
	u32 scsi_nbdp_r() { return m_scsi_nbdp; }
	u32 scsi_ctrl_r() { return m_scsi_ctrl; }

	// scsi write handlers
	void scsi_bc_w(u32 data) { m_scsi_bc = data & 0x1fffU; }
	void scsi_cbp_w(u32 data) { m_scsi_cbp = data & 0x8fff'ffffU; }
	void scsi_nbdp_w(u32 data);
	void scsi_ctrl_w(u32 data);

	u16 dsp_bc_r() { return m_dsp_bc; }
	void dsp_bc_w(u16 data) { m_dsp_bc = data; }

	u8 aux_r();
	void aux_w(u8 data);

	void pbus_map(address_map &map) ATTR_COLD;

private:
	address_space_config const m_pbus;
	required_address_space m_gio;

	optional_device<seeq8003_device> m_enet;

	devcb_write_line m_int_w;
	devcb_read_line m_eeprom_dati;
	devcb_read8::array<2> m_dma_r;
	devcb_write8::array<2> m_dma_w;
	devcb_write8 m_eeprom_out;

	u8 m_drq;

	u32 m_enet_cxbp;     // enet current transmit buffer pointer
	u32 m_enet_nxbdp;    // enet current transmit buffer descriptor pointer
	u32 m_enet_xbc;      // enet transmit byte count
	u32 m_enet_cxbdp;    // enet current transmit buffer descriptor pointer
	u32 m_enet_cpfxbdp;  // enet current packet first transmit buffer descriptor pointer
	u32 m_enet_ppfxbdp;  // enet previous packet first transmit buffer descriptor pointer
	u32 m_enet_intdelay; // enet interrupt delay count
	u32 m_enet_txs;      // enet transmit status
	u32 m_enet_rxs;      // enet receive status
	u32 m_enet_ctrl;     // enet interrupt, channel reset, buffer overflow
	u32 m_enet_rbc;      // enet receive byte count
	u32 m_enet_crbp;     // enet current receive buffer pointer
	u32 m_enet_nrbdp;    // enet next receive buffer desriptor pointer
	u32 m_enet_crbdp;    // enet current receive buffer descriptor pointer

	u32 m_scsi_bc;       // scsi byte count
	u32 m_scsi_cbp;      // scsi current buffer pointer
	u32 m_scsi_nbdp;     // scsi next buffer descriptor pointer
	u32 m_scsi_ctrl;     // scsi control

	u16 m_dsp_bc;

	u8 m_aux;
};

DECLARE_DEVICE_TYPE(SGI_HPC1, hpc1_device)

#endif // MAME_SGI_HPC1_H
