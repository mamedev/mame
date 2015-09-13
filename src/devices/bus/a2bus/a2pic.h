// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2pic.h

    Apple II Parallel Interface Card

*********************************************************************/

#ifndef __A2BUS_PIC__
#define __A2BUS_PIC__

#include "a2bus.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_pic_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_pic_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;

	required_ioport m_dsw1;

	DECLARE_WRITE_LINE_MEMBER( ack_w );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset);

	required_device<centronics_device> m_ctx;
	required_device<input_buffer_device> m_ctx_data_in;
	required_device<output_latch_device> m_ctx_data_out;

	void start_strobe();
	void clear_strobe();

private:
	UINT8 *m_rom;
	bool m_started;
	UINT8 m_ack;
	bool m_irqenable;
	bool m_autostrobe;
	emu_timer *m_timer;
};

// device type definition
extern const device_type A2BUS_PIC;

#endif  /* __A2BUS_PIC__ */
