// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NCR5380_H
#define MAME_MACHINE_NCR5380_H

#pragma once

#include "machine/nscsi_bus.h"

class ncr5380_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	ncr5380_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// device configuration
	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	// register access
	void map(address_map &map) ATTR_COLD;
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// dma access
	void eop_w(int state);
	u8 dma_r();
	void dma_w(u8 val);

protected:
	ncr5380_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, bool has_lbs = false);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ncsci_device overrides
	virtual void scsi_ctrl_changed() override;

	// register read handlers
	u8 csdata_r();
	u8 icmd_r();
	u8 mode_r();
	u8 tcmd_r();
	u8 csstat_r();
	u8 bas_r();
	u8 idata_r();
	u8 rpi_r();

	// register write handlers
	void odata_w(u8 data);
	void icmd_w(u8 data);
	void mode_w(u8 data);
	void tcmd_w(u8 data);
	void selen_w(u8 data);
	void sds_w(u8 data);
	void sdtr_w(u8 data);
	void sdir_w(u8 data);

	// state machine
	void state_timer(s32 param);
	int state_step();

	// other helpers
	void scsi_data_w(u8 data);
	void set_irq(bool irq_state);
	void set_drq(bool drq_state);

private:
	enum icmd_mask : u8
	{
		IC_RST   = 0x80, // assert R̅S̅T̅
		IC_TEST  = 0x40, // test mode (wo)
		IC_AIP   = 0x40, // arbitration in progress (ro)
		IC_LA    = 0x20, // lost arbitration (ro)
		IC_ACK   = 0x10, // assert A̅C̅K̅
		IC_BSY   = 0x08, // assert B̅S̅Y̅
		IC_SEL   = 0x04, // assert S̅E̅L̅
		IC_ATN   = 0x02, // assert A̅T̅N̅
		IC_DBUS  = 0x01, // assert data bus

		IC_PHASE = 0x9e,
		IC_WRITE = 0x9f,
	};
	enum mode_mask : u8
	{
		MODE_BLOCKDMA  = 0x80,
		MODE_TARGET    = 0x40,
		MODE_PARITYCHK = 0x20,
		MODE_PARITYIRQ = 0x10,
		MODE_EOPIRQ    = 0x08,
		MODE_BSYIRQ    = 0x04,
		MODE_DMA       = 0x02,
		MODE_ARBITRATE = 0x01,
	};
	enum tcmd_mask : u8
	{
		TC_LBS   = 0x80, // last byte sent
		TC_REQ   = 0x08, // assert R̅E̅Q̅
		TC_MSG   = 0x04, // assert M̅S̅G̅
		TC_CD    = 0x02, // assert C̅/D
		TC_IO    = 0x01, // assert I̅/O

		TC_PHASE = 0x07,
	};
	enum csstat_mask : u8
	{
		ST_RST = 0x80,
		ST_BSY = 0x40,
		ST_REQ = 0x20,
		ST_MSG = 0x10,
		ST_CD  = 0x08,
		ST_IO  = 0x04,
		ST_SEL = 0x02,
		ST_DBP = 0x01,
	};
	enum bas_mask : u8
	{
		BAS_ENDOFDMA    = 0x80,
		BAS_DMAREQUEST  = 0x40,
		BAS_PARITYERROR = 0x20,
		BAS_IRQACTIVE   = 0x10,
		BAS_PHASEMATCH  = 0x08,
		BAS_BUSYERROR   = 0x04,
		BAS_ATN         = 0x02,
		BAS_ACK         = 0x01,
	};

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

	// state machine
	emu_timer *m_state_timer;
	enum state : uint32_t
	{
		IDLE,

		// arbitration
		ARB_BUS_FREE,
		ARB_START,
		ARB_EVALUATE,

		// dma transfer
		DMA_IN_REQ,
		DMA_IN_ACK,
		DMA_OUT_REQ,
		DMA_OUT_DRQ,
		DMA_OUT_ACK,
	}
	m_state;

	// registers
	u8 m_odata;
	u8 m_icmd;
	u8 m_mode;
	u8 m_tcmd;
	u8 m_bas;
	u8 m_idata;

	// line state
	u32 m_scsi_ctrl;
	bool m_irq_state;
	bool m_drq_state;

	bool const m_has_lbs;
};

class ncr53c80_device : public ncr5380_device
{
public:
	ncr53c80_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
};

class cxd1180_device : public ncr5380_device
{
public:
	cxd1180_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(NCR5380, ncr5380_device)
DECLARE_DEVICE_TYPE(NCR53C80, ncr53c80_device)
DECLARE_DEVICE_TYPE(CXD1180, cxd1180_device)

#endif // MAME_MACHINE_NCR5380_H
