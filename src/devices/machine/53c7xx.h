// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*********************************************************************

    NCR 53C700/53C710 SCSI I/O Processors

*********************************************************************/

#ifndef MAME_MACHINE_53C7XX_H
#define MAME_MACHINE_53C7XX_H

#pragma once

#include "machine/nscsi_bus.h"


class ncr53c700_device : public device_t, public nscsi_device_interface, public device_execute_interface
{
public:
	// construction/destruction
	ncr53c700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	auto host_write() { return m_host_write.bind(); }
	auto host_read() { return m_host_read.bind(); }

	// our API
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// construction/destruction
	ncr53c700_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	TIMER_CALLBACK_MEMBER(step_timer);

	enum scripts_state
	{
		SCRIPTS_IDLE,
		SCRIPTS_WAIT_MANUAL_START,
		SCRIPTS_FETCH,
		SCRIPTS_EXECUTE
	};

	void set_scripts_state(scripts_state state);
	void illegal();
	[[noreturn]] void unimplemented(char const *operation) const ATTR_COLD;

	uint32_t host_memory_read(offs_t address, uint32_t mem_mask);
	void host_memory_write(offs_t address, uint32_t data, uint32_t mem_mask);

	virtual std::string disassemble_scripts();

	int m_icount;
	int m_scripts_state;

	// SCSI registers
	uint8_t   m_scntl[2];
	uint8_t   m_sdid;
	uint8_t   m_sien;
	uint8_t   m_scid;
	uint8_t   m_sxfer;
	uint8_t   m_sodl;
	uint8_t   m_socl;
	uint8_t   m_sfbr;
	uint8_t   m_sidl;
	uint8_t   m_sbdl;
	uint8_t   m_sbcl;
	uint8_t   m_dstat;
	uint8_t   m_sstat[3];
	uint8_t   m_ctest[8];
	uint32_t  m_temp;
	uint8_t   m_dfifo;
	uint8_t   m_istat;
	uint32_t  m_dbc;
	uint8_t   m_dcmd;
	uint32_t  m_dnad;
	uint32_t  m_dsp;
	uint32_t  m_dsps;
	uint8_t   m_dmode;
	uint8_t   m_dien;
	uint8_t   m_dwt;
	uint8_t   m_dcntl;

private:
	static constexpr uint8_t DMODE_PIPE = 0x02;
	static constexpr uint8_t DCNTL_RST = 0x01;

	enum
	{
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00
	};

	enum
	{
		MODE_I,
		MODE_T,
		MODE_D
	};

	enum scsi_state
	{
		IDLE,
		FINISHED,
		ARBITRATE_WAIT_FREE,
		ARBITRATE_CHECK_FREE,
		ARBITRATE_EXAMINE_BUS,
		ARBITRATE_SELECT_DEST,
		ARBITRATE_ASSERT_SEL,
		ARBITRATE_RELEASE_BSY,
		ARBITRATE_WAIT_BSY,
		ARBITRATE_DESKEW_WAIT,
		INIT_XFER,
		INIT_XFER_WAIT_REQ,
		INIT_XFER_SEND_BYTE,
		INIT_XFER_RECV_PAD,
		INIT_XFER_RECV_BYTE_ACK,
		INIT_XFER_RECV_BYTE_NACK,
		SEND_WAIT_REQ_0,
		SEND_WAIT_SETTLE,
		RECV_WAIT_SETTLE,
		RECV_WAIT_REQ_0,
		RECV_WAIT_REQ_1
	};

	void update_irqs();
	void set_scsi_state(int state);
	void delay(const attotime &delay);
	void scsi_ctrl_changed() override;
	void send_byte();
	void recv_byte();
	void step(bool timeout);
	virtual void dcntl_w(uint8_t data);
	virtual void istat_w(uint8_t data);
	virtual unsigned host_byte_shift(offs_t address);
	virtual uint32_t transfer_control_address();

	void scripts_yield();
	void scripts_decode_bm(void);
	void scripts_decode_io(void);
	void scripts_decode_tc(void);
	virtual unsigned block_move_opcode() const;
	virtual void configure_block_move();
	virtual void load_io_operands();
	virtual bool scripts_decode_read_write();
	virtual void scripts_decode_memory_move();

	void bm_t_move();
	void bm_i_wmov();
	void io_t_reselect();
	void io_t_disconnect();
	void io_t_waitselect();
	void io_t_set();
	void io_t_clear();
	void io_i_select();
	void io_i_waitdisconnect();
	virtual void io_i_waitreselect();
	void io_i_set();
	void io_i_clear();
	void tc_jump();
	void tc_call();
	void tc_return();
	void tc_int();
	bool scripts_data_compare() const;

private:
	// other state
	int     m_scsi_state;
	bool    m_connected;
	bool    m_finished;
	bool    m_first_byte_received;
	uint32_t  m_xfr_phase;
	emu_timer *m_tm;

	//int     m_scripts_substate;
	void    (ncr53c700_device::*m_scripts_op)();

	// callbacks
	devcb_write_line m_irq_handler;
	devcb_write32 m_host_write;
	devcb_read32 m_host_read;
};

class ncr53c710_device : public ncr53c700_device
{
public:
	ncr53c710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto big_lit_handler() { return m_big_lit_handler.bind(); }
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr uint8_t DCNTL_EA = 0x20;
	static constexpr uint8_t ISTAT_RST = 0x40;
	static constexpr uint8_t ISTAT_SIGP = 0x20;
	static constexpr uint8_t DMODE_FC1 = 0x10;
	static constexpr uint8_t CTEST8_REVISION = 0x10;
	static constexpr uint8_t CTEST8_CLF = 0x04;
	static constexpr uint8_t CTEST8_WRITABLE = 0x0b;
	static constexpr uint8_t CTEST2_SIGP = 0x40;

	virtual void dcntl_w(uint8_t data) override;
	virtual void istat_w(uint8_t data) override;
	virtual unsigned host_byte_shift(offs_t address) override;
	virtual uint32_t transfer_control_address() override;
	virtual unsigned block_move_opcode() const override;
	virtual void configure_block_move() override;
	virtual void load_io_operands() override;
	virtual bool scripts_decode_read_write() override;
	virtual void scripts_decode_memory_move() override;
	virtual void io_i_waitreselect() override;
	virtual std::string disassemble_scripts() override;
	uint8_t scripts_register_read(uint8_t address);
	void scripts_register_write(uint8_t address, uint8_t data);

	uint32_t m_dsa;
	uint8_t m_ctest8;
	uint8_t m_lcrc;
	uint32_t m_scratch;
	bool m_carry;

	devcb_read_line m_big_lit_handler;
};

// device type definitions
DECLARE_DEVICE_TYPE(NCR53C700, ncr53c700_device)
DECLARE_DEVICE_TYPE(NCR53C710, ncr53c710_device)

#endif // MAME_MACHINE_53C7XX_H
