/*
 * sc499.h - Archive Cartridge tape controller SC-499
 *
 *  Created on: February 21, 2011
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 */

#pragma once

#ifndef SC499_H_
#define SC499_H_

#include "emu.h"
#include "bus/isa/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sc499_device

class sc499_device: public device_t, public device_isa8_card_interface
{
public:
	// construction/destruction
	sc499_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	int sc499_receive(const UINT8 data[], int length);

	required_ioport m_iobase;
	required_ioport m_irqdrq;

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// ISA overrides
	virtual UINT8 dack_r(int line);
	virtual void dack_w(int line,UINT8 data);
	virtual void eop_w(int state);

	const char *cpu_context();

	void tape_status_clear(UINT16 value);
	void tape_status_set(UINT16 value);

	// device register I/O
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void check_tape();

	void set_interrupt(enum line_state state);

	void log_command(UINT8 data);
	void do_command(UINT8 data);
	void do_reset();

	void set_dma_drq(enum line_state state);

	void write_command_port( UINT8 data);
	UINT8 read_data_port();
	void write_control_port( UINT8 data);
	UINT8 read_status_port();
	void write_dma_go( UINT8 data);
	void write_dma_reset( UINT8 data);

	void log_block(const char * text);
	void read_block();
	void write_block();
	int block_is_filemark();
	void block_set_filemark();

	UINT8 m_data;
	UINT8 m_command;
	UINT8 m_status;
	UINT8 m_control;

	UINT8 m_has_cartridge;
	UINT8 m_is_writable;

	UINT8 m_current_command;

	UINT8 m_first_block_hack;
	UINT8 m_nasty_readahead;
	UINT8 m_read_block_pending;

	UINT16 m_data_index;

	UINT16 m_tape_status;           /* Drive exception flags */
	UINT16 m_data_error_counter;    /* data error count: nr of blocks rewritten/soft read errors */
	UINT16 m_underrun_counter;      /* underrun count: nr of times streaming was interrupted */

	UINT32 m_tape_pos;
	UINT32 m_ctape_block_count;
	UINT32 m_ctape_block_index;
	UINT64 m_image_length;

	dynamic_buffer m_ctape_block_buffer;

	device_image_interface *m_image;

	enum line_state irq_state;
	enum line_state dma_drq_state;

	emu_timer * m_timer; // timer to delay functions
	emu_timer * m_timer1; // timer to delay functions
	int m_timer_type;
	int m_irq, m_drq;

	bool m_installed;
};

// device type definition
extern const device_type ISA8_SC499;

#endif /* SC499_H_ */
