// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_NCR53C810_H
#define MAME_MACHINE_NCR53C810_H

#pragma once

#include "nscsi_bus.h"

// NCR 53C8xx DMA/SCRIPTS core.  Host callbacks use byte addresses and
// little-endian, aligned 32-bit data with byte enables.
class ncr53c8xx_device_base : public device_t, public nscsi_device_interface
{
public:
	auto memory_read() { return m_memory_read.bind(); }
	auto memory_write() { return m_memory_write.bind(); }
	auto io_read() { return m_io_read.bind(); }
	auto io_write() { return m_io_write.bind(); }
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	void bus_master_w(int state);
	void cio_w(int state);
	void cm_w(int state);
	virtual void scsi_ctrl_changed() override;

protected:
	enum : u8
	{
		SCNTL0 = 0x00, SCNTL1 = 0x01, SCNTL2 = 0x02, SCNTL3 = 0x03,
		SCID = 0x04, SXFER = 0x05, SDID = 0x06, GPREG = 0x07,
		SOCL = 0x09, SSID = 0x0a, SBCL = 0x0b, SSTAT0 = 0x0d, SSTAT1 = 0x0e, SSTAT2 = 0x0f,
		SIEN0 = 0x40, SIEN1 = 0x41, SIST0 = 0x42, SIST1 = 0x43,
		SLPAR = 0x44, MACNTL = 0x46, GPCNTL = 0x47,
		STIME0 = 0x48, STIME1 = 0x49, RESPID = 0x4a,
		STEST0 = 0x4c, STEST1 = 0x4d, STEST2 = 0x4e, STEST3 = 0x4f,
		SODL = 0x54, SIDL = 0x50, SBDL = 0x58,
		SFBR = 0x08, DSTAT = 0x0c, DSA = 0x10, ISTAT = 0x14,
		CTEST0 = 0x18, CTEST1 = 0x19, CTEST2 = 0x1a, CTEST3 = 0x1b,
		TEMP = 0x1c, DFIFO = 0x20, CTEST4 = 0x21, CTEST5 = 0x22, CTEST6 = 0x23,
		DBC = 0x24, DCMD = 0x27, DNAD = 0x28,
		DSP = 0x2c, DSPS = 0x30, SCRATCHA = 0x34, DMODE = 0x38,
		DIEN = 0x39, DWT = 0x3a, DCNTL = 0x3b, SCRATCHB = 0x5c,
		HIGH_REGS = 0x60,
		DSTAT_IID = 0x01, DSTAT_SIR = 0x04, DSTAT_SSI = 0x08, DSTAT_ABRT = 0x10,
		SIST0_RST = 0x02, SIST0_UDC = 0x04, SIST0_RSL = 0x10, SIST0_CMP = 0x40, SIST0_MA = 0x80,
		SIST1_STO = 0x04
	};

	enum scsi_state : u8
	{
		IDLE, BUS_FREE, ARBITRATE, SELECT, TRANSFER, ACKNOWLEDGE,
		DISCONNECT, RESELECT, RESELECTED, PHASE
	};

	ncr53c8xx_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// chip identity: CTEST3 revision and MACNTL chip type, bits 7-4
	virtual u8 revision() const = 0;
	virtual u8 chip_type() const = 0;

	// operating registers 60h-7Fh
	virtual u8 high_reg_r(u8 offset);
	virtual void high_reg_w(u8 offset, u8 data);

	// returns false when the chip has no Load and Store instructions
	virtual bool load_store(u32 instruction);

	// block move bytes per REQ/ACK, 2 on a wide bus
	virtual u32 scsi_width() const;

	u32 word_r(u8 offset) const;
	void word_w(u8 offset, u32 data);
	void reset_registers();
	void start();
	void schedule();
	void dma_interrupt(u8 cause);
	void update_irq();
	void instruction_done();
	void fetch();
	void memory_move();
	void transfer_control(u32 instruction);
	void register_operation(u32 instruction);
	void scsi_io(u32 instruction);
	void block_move(u32 instruction);
	bool scsi_step();
	u8 dma_byte_r(u32 address);
	void dma_byte_w(u32 address, u8 data);
	void scsi_lines(u32 data, u32 mask);
	void drive_sodl();
	void scsi_interrupt(u8 sist0, u8 sist1 = 0);
	void promote_stacked_interrupts();
	bool scsi_pending() const;
	TIMER_CALLBACK_MEMBER(scsi_tick);
	TIMER_CALLBACK_MEMBER(execute);
	TIMER_CALLBACK_MEMBER(promote_interrupt);

	devcb_read32 m_memory_read;
	devcb_write32 m_memory_write;
	devcb_read32 m_io_read;
	devcb_write32 m_io_write;
	devcb_write_line m_irq_handler;

	emu_timer *m_timer;
	emu_timer *m_interrupt_timer;
	emu_timer *m_scsi_timer;
	u8 m_regs[HIGH_REGS];
	u32 m_move_source;
	u32 m_move_destination;
	u8 m_stacked_dstat;
	u8 m_stacked_sist0;
	u8 m_stacked_sist1;
	u8 m_scsi_state;
	u8 m_scsi_irq;
	u32 m_bus_control;
	bool m_waiting;
	bool m_first_byte;
	bool m_bus_master;
	bool m_cio;
	bool m_cm;
	bool m_running;
	bool m_moving;
	bool m_carry;
	bool m_dma_irq;
	bool m_irq;
};

class ncr53c810_device : public ncr53c8xx_device_base
{
public:
	ncr53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// ncr53c8xx_device_base implementation
	virtual u8 revision() const override;
	virtual u8 chip_type() const override;
};

DECLARE_DEVICE_TYPE(NCR53C810, ncr53c810_device)

#endif // MAME_MACHINE_NCR53C810_H
