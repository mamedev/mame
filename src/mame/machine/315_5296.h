// license:BSD-3-Clause
// copyright-holders:hap, Charles MacDonald
/**********************************************************************

    Sega 315-5296 I/O chip

**********************************************************************/

#ifndef MAME_MACHINE_315_5296_H
#define MAME_MACHINE_315_5296_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sega_315_5296_device

class sega_315_5296_device : public device_t
{
public:
	sega_315_5296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto in_pa_callback() { return m_in_pa_cb.bind(); }
	auto in_pb_callback() { return m_in_pb_cb.bind(); }
	auto in_pc_callback() { return m_in_pc_cb.bind(); }
	auto in_pd_callback() { return m_in_pd_cb.bind(); }
	auto in_pe_callback() { return m_in_pe_cb.bind(); }
	auto in_pf_callback() { return m_in_pf_cb.bind(); }
	auto in_pg_callback() { return m_in_pg_cb.bind(); }
	auto in_ph_callback() { return m_in_ph_cb.bind(); }

	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }
	auto out_pc_callback() { return m_out_pc_cb.bind(); }
	auto out_pd_callback() { return m_out_pd_cb.bind(); }
	auto out_pe_callback() { return m_out_pe_cb.bind(); }
	auto out_pf_callback() { return m_out_pf_cb.bind(); }
	auto out_pg_callback() { return m_out_pg_cb.bind(); }
	auto out_ph_callback() { return m_out_ph_cb.bind(); }

	auto out_cnt0_callback() { return m_out_cnt0_cb.bind(); }
	auto out_cnt1_callback() { return m_out_cnt1_cb.bind(); }
	auto out_cnt2_callback() { return m_out_cnt2_cb.bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	uint8_t debug_peek_output(offs_t offset) const { return m_output_latch[offset & 7]; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read8 m_in_pa_cb;
	devcb_read8 m_in_pb_cb;
	devcb_read8 m_in_pc_cb;
	devcb_read8 m_in_pd_cb;
	devcb_read8 m_in_pe_cb;
	devcb_read8 m_in_pf_cb;
	devcb_read8 m_in_pg_cb;
	devcb_read8 m_in_ph_cb;

	devcb_write8 m_out_pa_cb;
	devcb_write8 m_out_pb_cb;
	devcb_write8 m_out_pc_cb;
	devcb_write8 m_out_pd_cb;
	devcb_write8 m_out_pe_cb;
	devcb_write8 m_out_pf_cb;
	devcb_write8 m_out_pg_cb;
	devcb_write8 m_out_ph_cb;

	devcb_write_line m_out_cnt0_cb;
	devcb_write_line m_out_cnt1_cb;
	devcb_write_line m_out_cnt2_cb;

	devcb_read8 *m_in_port_cb[8];
	devcb_write8 *m_out_port_cb[8];
	devcb_write_line *m_out_cnt_cb[3];

	uint8_t m_output_latch[8];
	uint8_t m_cnt;
	uint8_t m_dir;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5296, sega_315_5296_device)

#endif // MAME_MACHINE_315_5296_H
