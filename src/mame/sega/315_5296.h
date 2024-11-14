// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Charles MacDonald
/**********************************************************************

    Sega 315-5296 I/O chip

**********************************************************************/

#ifndef MAME_SEGA_315_5296_H
#define MAME_SEGA_315_5296_H

#pragma once


class sega_315_5296_device : public device_t
{
public:
	sega_315_5296_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto in_pa_callback() { return m_in_port_cb[0].bind(); }
	auto in_pb_callback() { return m_in_port_cb[1].bind(); }
	auto in_pc_callback() { return m_in_port_cb[2].bind(); }
	auto in_pd_callback() { return m_in_port_cb[3].bind(); }
	auto in_pe_callback() { return m_in_port_cb[4].bind(); }
	auto in_pf_callback() { return m_in_port_cb[5].bind(); }
	auto in_pg_callback() { return m_in_port_cb[6].bind(); }
	auto in_ph_callback() { return m_in_port_cb[7].bind(); }

	auto out_pa_callback() { return m_out_port_cb[0].bind(); }
	auto out_pb_callback() { return m_out_port_cb[1].bind(); }
	auto out_pc_callback() { return m_out_port_cb[2].bind(); }
	auto out_pd_callback() { return m_out_port_cb[3].bind(); }
	auto out_pe_callback() { return m_out_port_cb[4].bind(); }
	auto out_pf_callback() { return m_out_port_cb[5].bind(); }
	auto out_pg_callback() { return m_out_port_cb[6].bind(); }
	auto out_ph_callback() { return m_out_port_cb[7].bind(); }

	auto out_cnt0_callback() { return m_out_cnt_cb[0].bind(); }
	auto out_cnt1_callback() { return m_out_cnt_cb[1].bind(); }
	auto out_cnt2_callback() { return m_out_cnt_cb[2].bind(); }

	void set_ddr_override(u8 mask) { m_dir_override = mask; }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 debug_peek_output(offs_t offset) const { return m_output_latch[offset & 7]; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8::array<8> m_in_port_cb;
	devcb_write8::array<8> m_out_port_cb;
	devcb_write_line::array<3> m_out_cnt_cb;

	u8 m_output_latch[8];
	u8 m_cnt;
	u8 m_dir;
	u8 m_dir_override;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5296, sega_315_5296_device)

#endif // MAME_SEGA_315_5296_H
