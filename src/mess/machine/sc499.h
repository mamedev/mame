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

/***************************************************************************
 FUNCTION PROTOTYPES
 ***************************************************************************/

READ8_DEVICE_HANDLER( sc499_r );
WRITE8_DEVICE_HANDLER( sc499_w );

void sc499_set_tc_state(running_machine *machine, int state);
UINT8 sc499_dack_r(running_machine *machine);
void sc499_dack_w(running_machine *machine, UINT8 data);

void sc499_set_verbose(int on_off);

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SC499_ADD(_tag, _interface) \
	MCFG_FRAGMENT_ADD( sc499_ctape ) \
	MCFG_DEVICE_ADD(_tag, SC499, 0) \
	sc499_device::static_set_interface(*device, _interface);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef void (*sc499_set_irq)(const device_t *, int);
typedef void (*sc499_dma_drq)(const device_t *,int);

struct sc499_interface
{
	sc499_set_irq set_irq;
	sc499_dma_drq dma_drq;
	device_t *(*get_device)(running_machine*);
};

#define SC499_INTERFACE(name) const struct sc499_interface (name)

// ======================> sc499_device

class sc499_device: public device_t, public sc499_interface
{
public:
	// construction/destruction
	sc499_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const sc499_interface &interface);

	int sc499_receive(const UINT8 data[], int length);
	// device register I/O
	UINT8 read_port(offs_t offset);
	void write_port(offs_t offset, UINT8 data);

	void set_tc_state(int state);
	UINT8 dack_read();
	void dack_write( UINT8 data);

	// static pointer to myself (nasty: used for logging and dma ...)
	static sc499_device *m_device;

	virtual void device_reset();

private:
	// device-level overrides
	virtual void device_start();

	const char *cpu_context();

	void tape_status_clear(UINT16 value);
	void tape_status_set(UINT16 value);

	void check_tape();

	void timer_func(int param);
	static TIMER_CALLBACK( static_timer_func );

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

	UINT16 m_tape_status;			/* Drive exception flags */
	UINT16 m_data_error_counter;	/* data error count: nr of blocks rewritten/soft read errors */
	UINT16 m_underrun_counter;		/* underrun count: nr of times streaming was interrupted */

	UINT32 m_tape_pos;
	UINT32 m_ctape_block_count;
	UINT32 m_ctape_block_index;
	UINT64 m_image_length;

    UINT8 *m_ctape_block_buffer;

	device_image_interface *m_image;

	enum line_state irq_state;
	enum line_state dma_drq_state;

	emu_timer * m_timer; // timer to delay functions
	emu_timer * m_timer1; // timer to delay functions
	int m_timer_type;
};

// device type definition
extern const device_type SC499;

MACHINE_CONFIG_EXTERN( sc499_ctape );

#endif /* SC499_H_ */
