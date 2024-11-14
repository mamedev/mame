// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

    Custom 100-pin QFP LSI. Supports 7 8-bit input/output ports and can
    directly interact with dual port RAM. Also supports a master/slave
    configuration where one controller acts as master and sends commands
    and data over a serial link to another controller.

    TODO:
    - Serial
    - Slave mode
    - and probably lots more

***************************************************************************/

#ifndef MAME_SEGA_315_5338A_H
#define MAME_SEGA_315_5338A_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5338a_device : public device_t
{
public:
	// construction/destruction
	sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto read_callback() { return m_read_cb.bind(); }

	auto write_callback() { return m_write_cb.bind(); }

	auto in_pa_callback() { return m_in_port_cb[0].bind(); }
	auto in_pb_callback() { return m_in_port_cb[1].bind(); }
	auto in_pc_callback() { return m_in_port_cb[2].bind(); }
	auto in_pd_callback() { return m_in_port_cb[3].bind(); }
	auto in_pe_callback() { return m_in_port_cb[4].bind(); }
	auto in_pf_callback() { return m_in_port_cb[5].bind(); }
	auto in_pg_callback() { return m_in_port_cb[6].bind(); }

	auto out_pa_callback() { return m_out_port_cb[0].bind(); }
	auto out_pb_callback() { return m_out_port_cb[1].bind(); }
	auto out_pc_callback() { return m_out_port_cb[2].bind(); }
	auto out_pd_callback() { return m_out_port_cb[3].bind(); }
	auto out_pe_callback() { return m_out_port_cb[4].bind(); }
	auto out_pf_callback() { return m_out_port_cb[5].bind(); }
	auto out_pg_callback() { return m_out_port_cb[6].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// callbacks
	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
	devcb_read8::array<7> m_in_port_cb;
	devcb_write8::array<7> m_out_port_cb;

	uint8_t m_port_value[7];
	uint8_t m_port_config;
	uint8_t m_serial_output;
	uint16_t m_address;
	uint8_t m_cmd;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device)

#endif // MAME_SEGA_315_5338A_H
