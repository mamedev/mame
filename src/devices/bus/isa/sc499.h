// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * sc499.h - Archive Cartridge tape controller SC-499
 *
 *  Created on: February 21, 2011
 *      Author: Hans Ostermeyer
 *
 */

#ifndef MAME_BUS_ISA_SC499_H
#define MAME_BUS_ISA_SC499_H

#pragma once

#include "isa.h"

#include "imagedev/magtape.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sc499_ctape_image_device

class sc499_ctape_image_device : public microtape_image_device
{
public:
	// construction/destruction
	sc499_ctape_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "sc499_cass"; }
	virtual const char *file_extensions() const noexcept override { return "act,ct"; }
	virtual const char *image_type_name() const noexcept override { return "ctape"; }
	virtual const char *image_brief_type_name() const noexcept override { return "ct"; }

	uint8_t *read_block(int block_num);
	void write_block(int block_num, uint8_t *ptr);
	uint64_t tapelen() { return m_ctape_data.size(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	std::vector<uint8_t> m_ctape_data;
};

// ======================> sc499_device

class sc499_device: public device_t, public device_isa8_card_interface
{
public:
	// construction/destruction
	sc499_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	required_ioport m_iobase;
	required_ioport m_irqdrq;

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// ISA overrides
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line,uint8_t data) override;
	virtual void eop_w(int state) override;

	std::string cpu_context() const;

	void tape_status_clear(uint16_t value);
	void tape_status_set(uint16_t value);

	// device register I/O
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void check_tape();

	void set_interrupt(enum line_state state);

	void log_command(uint8_t data);
	void do_command(uint8_t data);
	void do_reset();

	void set_dma_drq(enum line_state state);

	void write_command_port( uint8_t data);
	uint8_t read_data_port();
	void write_control_port( uint8_t data);
	uint8_t read_status_port();
	void write_dma_go( uint8_t data);
	void write_dma_reset( uint8_t data);

	void log_block(const char * text);
	void read_block();
	void write_block();
	int block_is_filemark();
	void block_set_filemark();

	TIMER_CALLBACK_MEMBER(timer_func);

	uint8_t m_data;
	uint8_t m_command;
	uint8_t m_status;
	uint8_t m_control;

	uint8_t m_has_cartridge;
	uint8_t m_is_writable;

	uint8_t m_current_command;

	uint8_t m_first_block_hack;
	uint8_t m_nasty_readahead;
	uint8_t m_read_block_pending;

	uint16_t m_data_index;

	uint16_t m_tape_status;           /* Drive exception flags */
	uint16_t m_data_error_counter;    /* data error count: nr of blocks rewritten/soft read errors */
	uint16_t m_underrun_counter;      /* underrun count: nr of times streaming was interrupted */

	uint32_t m_tape_pos;
	uint32_t m_ctape_block_count;
	uint32_t m_ctape_block_index;
	uint64_t m_image_length;

	std::vector<uint8_t> m_ctape_block_buffer;
	required_device<sc499_ctape_image_device> m_image;

	enum line_state irq_state;
	enum line_state dma_drq_state;

	emu_timer * m_timer; // timer to delay functions
	emu_timer * m_timer1; // timer to delay functions
	int m_irq, m_drq;

	bool m_installed;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_SC499, sc499_device)

#endif // MAME_BUS_ISA_SC499_H
