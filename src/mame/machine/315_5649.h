// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5649

    I/O Controller

***************************************************************************/

#ifndef MAME_MACHINE_315_5649_H
#define MAME_MACHINE_315_5649_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5649_device : public device_t
{
public:
	// construction/destruction
	sega_315_5649_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
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

	template <unsigned N> auto an_port_callback() { return m_an_port_cb[N].bind(); }

	auto serial_ch1_rd_callback() { return m_serial_rd_cb[0].bind(); }
	auto serial_ch2_rd_callback() { return m_serial_rd_cb[1].bind(); }

	auto serial_ch1_wr_callback() { return m_serial_wr_cb[0].bind(); }
	auto serial_ch2_wr_callback() { return m_serial_wr_cb[1].bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_in_port_cb[7];
	devcb_write8 m_out_port_cb[7];
	devcb_read8 m_an_port_cb[8];
	devcb_read8 m_serial_rd_cb[2];
	devcb_write8 m_serial_wr_cb[2];

	uint8_t m_port_value[7];
	uint8_t m_port_config;
	int m_analog_channel;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5649, sega_315_5649_device)

#endif // MAME_MACHINE_315_5649_H
