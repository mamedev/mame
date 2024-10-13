// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5649

    I/O Controller

***************************************************************************/

#ifndef MAME_SEGA_315_5649_H
#define MAME_SEGA_315_5649_H

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

	template <unsigned N> auto in_counter_callback() { return m_cnt_cb[N].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// callbacks
	devcb_read8::array<7> m_in_port_cb;
	devcb_write8::array<7> m_out_port_cb;
	devcb_read8::array<8> m_an_port_cb;
	devcb_read8::array<2> m_serial_rd_cb;
	devcb_write8::array<2> m_serial_wr_cb;
	devcb_read16::array<4> m_cnt_cb;

	uint8_t m_port_value[7];
	uint8_t m_port_config;
	uint8_t m_mode;
	int m_analog_channel;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5649, sega_315_5649_device)

#endif // MAME_SEGA_315_5649_H
