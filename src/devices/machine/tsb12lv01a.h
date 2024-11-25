// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************

  Texas Instruments TSB12LV01A/TSB12LV01AI IEEE 1394-1995
  High-Speed Serial-Bus Link-Layer Controller

  Skeleton device

**************************************************************/

#ifndef MAME_MACHINE_TSB12LV01A_H
#define MAME_MACHINE_TSB12LV01A_H

#pragma once

class tsb12lv01a_device : public device_t
{
public:
	tsb12lv01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask);

	auto int_cb() { return m_int_cb.bind(); }
	auto phy_read() { return m_phy_read_cb.bind(); }
	auto phy_write() { return m_phy_write_cb.bind(); }

	void phy_reset_w(int state);

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_interrupt(uint32_t bit);
	void check_interrupts();

	void reset_tx();
	void reset_rx();

	void clear_atf();
	void clear_itf();
	void clear_grf();

	enum
	{
		NODE_ADDR_BUSNUM_SHIFT  = 22,
		NODE_ADDR_BUSNUM_MASK   = 0xffc00000,
		NODE_ADDR_NODENUM_SHIFT = 16,
		NODE_ADDR_NODENUM_MASK  = 0x003f0000,
		NODE_ADDR_ROOT          = 0x8000,
		NODE_ADDR_ATACK_SHIFT   = 4,
		NODE_ADDR_ATACK_MASK    = 0x000001f0,
		NODE_ADDR_ACKV          = 0x00000001,

		CTRL_IDVAL      = 0x80000000,
		CTRL_RXSID      = 0x40000000,
		CTRL_BSYCTRL    = 0x20000000,
		CTRL_RAI        = 0x10000000,
		CTRL_RCVCYST    = 0x08000000,
		CTRL_TXAEN      = 0x04000000,
		CTRL_RXAEN      = 0x02000000,
		CTRL_TXIEN      = 0x01000000,
		CTRL_RXIEN      = 0x00800000,
		CTRL_ACKCEN     = 0x00400000,
		CTRL_RSTTX      = 0x00200000,
		CTRL_RSTRX      = 0x00100000,
		CTRL_CYMAS      = 0x00000800,
		CTRL_CYSRC      = 0x00000400,
		CTRL_CYTEN      = 0x00000200,
		CTRL_TRGEN      = 0x00000100,
		CTRL_IRP1EN     = 0x00000080,
		CTRL_IRP2EN     = 0x00000040,
		CTRL_FHBAD      = 0x00000001,
		CTRL_RW_BITS    = CTRL_IDVAL | CTRL_RXSID | CTRL_BSYCTRL | CTRL_RAI | CTRL_RCVCYST |
						  CTRL_TXAEN | CTRL_RXAEN | CTRL_TXIEN | CTRL_RXIEN | CTRL_ACKCEN |
						  CTRL_CYMAS | CTRL_CYSRC | CTRL_CYTEN | CTRL_TRGEN | CTRL_IRP1EN |
						  CTRL_IRP2EN | CTRL_FHBAD,

		INT_INT     = 0x80000000,
		INT_PHINT   = 0x40000000,
		INT_PHYRRX  = 0x20000000,
		INT_PHRST   = 0x10000000,
		INT_SIDCOMP = 0x08000000,
		INT_TXRDY   = 0x04000000,
		INT_RXDTA   = 0x02000000,
		INT_CMDRST  = 0x01000000,
		INT_ACKRCV  = 0x00800000,
		INT_ITBADF  = 0x00100000,
		INT_ATBADF  = 0x00080000,
		INT_SNTRJ   = 0x00020000,
		INT_HDRDR   = 0x00010000,
		INT_TCERR   = 0x00008000,
		INT_CYTMOUT = 0x00001000,
		INT_CYSEC   = 0x00000800,
		INT_CYST    = 0x00000400,
		INT_CYDNE   = 0x00000200,
		INT_CYPND   = 0x00000100,
		INT_CYLST   = 0x00000080,
		INT_CARBFL  = 0x00000040,
		INT_ARBGP   = 0x00000004,
		INT_FRGP    = 0x00000002,
		INT_IARBFL  = 0x00000001,

		CYTMR_SEC_COUNT_SHIFT   = 25,
		CYTMR_SEC_COUNT_MASK    = 0xfe000000,
		CYTMR_CYC_COUNT_SHIFT   = 12,
		CYTMR_CYC_COUNT_MASK    = 0x01fff000,
		CYTMR_CYC_OFFSET_SHIFT  = 0,
		CYTMR_CYC_OFFSET_MASK   = 0x00000fff,

		ISOCH_PORT_TAG_SHIFT        = 30,
		ISOCH_PORT_TAG_MASK         = 0xc0000000,
		ISOCH_PORT_IRPORT1_SHIFT    = 24,
		ISOCH_PORT_IRPORT1_MASK     = 0x3f000000,
		ISOCH_PORT_TAG2_SHIFT       = 22,
		ISOCH_PORT_TAG2_MASK        = 0x00c00000,
		ISOCH_PORT_IRPORT2_SHIFT    = 16,
		ISOCH_PORT_IRPORT2_MASK     = 0x003f0000,
		ISOCH_PORT_MON_TAG          = 0x00000001,

		FIFO_CTRL_CLRATF            = 0x80000000,
		FIFO_CTRL_CLRITF            = 0x40000000,
		FIFO_CTRL_CLRGRF            = 0x20000000,
		FIFO_CTRL_TRIG_SIZE_SHIFT   = 18,
		FIFO_CTRL_TRIG_SIZE_MASK    = 0x07fc0000,
		FIFO_CTRL_ATF_SIZE_SHIFT    = 9,
		FIFO_CTRL_ATF_SIZE_MASK     = 0x0003fe00,
		FIFO_CTRL_ITF_SIZE_SHIFT    = 0,
		FIFO_CTRL_ITF_SIZE_MASK     = 0x000001ff,
		FIFO_CTRL_RW_BITS           = (FIFO_CTRL_TRIG_SIZE_MASK | FIFO_CTRL_ATF_SIZE_MASK | FIFO_CTRL_ITF_SIZE_MASK),

		PHY_RDPHY           = 0x80000000,
		PHY_WRPHY           = 0x40000000,
		PHY_PHYRGAD_SHIFT   = 24,
		PHY_PHYRGAD_MASK    = 0x0f000000,
		PHY_PHYRGDATA_SHIFT = 16,
		PHY_PHYRGDATA_MASK  = 0x00ff0000,
		PHY_PHYRXAD_SHIFT   = 8,
		PHY_PHYRXAD_MASK    = 0x00000f00,
		PHY_PHYRXDATA_SHIFT = 0,
		PHY_PHYRXDATA_MASK  = 0x000000ff,
		PHY_RW_BITS         = 0x0fff0fff,

		ATF_STATUS_FULL             = 0x80000000,
		ATF_STATUS_EMPTY            = 0x40000000,
		ATF_STATUS_CONERR           = 0x20000000,
		ATF_STATUS_ADRCLR           = 0x10000000,
		ATF_STATUS_CONTROL          = 0x08000000,
		ATF_STATUS_RAMTEST          = 0x04000000,
		ATF_STATUS_ADRCOUNTER_SHIFT = 17,
		ATF_STATUS_ADRCOUNTER_MASK  = 0x03fe0000,
		ATF_STATUS_ATFSPACE_SHIFT   = 0,
		ATF_STATUS_ATFSPACE_MASK    = 0x000001ff,

		ITF_STATUS_FULL             = 0x80000000,
		ITF_STATUS_EMPTY            = 0x40000000,
		ITF_STATUS_ITFSPACE_SHIFT   = 0,
		ITF_STATUS_ITFSPACE_MASK    = 0x000001ff,

		GRF_STATUS_EMPTY            = 0x80000000,
		GRF_STATUS_CD               = 0x40000000,
		GRF_STATUS_PACCOM           = 0x20000000,
		GRF_STATUS_GRFTOTAL_SHIFT   = 19,
		GRF_STATUS_GRFTOTAL_MASK    = 0x1ff80000,
		GRF_STATUS_GRFSIZE_SHIFT    = 9,
		GRF_STATUS_GRFSIZE_MASK     = 0x0007fe00,
		GRF_STATUS_WRITECOUNT_SHIFT = 0,
		GRF_STATUS_WRITECOUNT_MASK  = 0x000001ff
	};

	uint32_t m_version;
	uint32_t m_node_address;
	uint32_t m_ctrl;
	uint32_t m_int_status;
	uint32_t m_int_mask;
	uint32_t m_cycle_timer;
	uint32_t m_isoch_port_num;
	uint32_t m_fifo_ctrl;
	uint32_t m_diag_ctrl;
	uint32_t m_phy_access;
	uint32_t m_atf_status;
	uint32_t m_itf_status;
	uint32_t m_grf_status;

	devcb_write_line m_int_cb;
	devcb_read8 m_phy_read_cb;
	devcb_write8 m_phy_write_cb;
};

//device type definition
DECLARE_DEVICE_TYPE(TSB12LV01A, tsb12lv01a_device)

#endif // MAME_MACHINE_TSB12LV01A_H
