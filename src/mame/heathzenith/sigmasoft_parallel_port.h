// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board

****************************************************************************/

#ifndef MAME_HEATHKIT_SIGMASOFT_PARALLEL_PORT_H
#define MAME_HEATHKIT_SIGMASOFT_PARALLEL_PORT_H

#pragma once


class sigmasoft_parallel_port : public device_t
{
public:
	sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t reg, u8 val);
	u8 read(offs_t reg);

	auto ctrl_r_cb() { return m_ctrl_r.bind(); }
	auto video_mem_r_cb() { return m_video_mem_r.bind(); }

	auto video_mem_cb() { return m_video_mem_w.bind(); }
	auto io_lo_cb() { return m_io_lo_addr.bind(); }
	auto io_hi_cb() { return m_io_hi_addr.bind(); }
	auto window_lo_cb() { return m_window_lo_addr.bind(); }
	auto window_hi_cb() { return m_window_hi_addr.bind(); }
	auto ctrl_cb() { return m_ctrl_w.bind(); }

protected:

	virtual void device_start() override;

	u8 video_mem_r();
	void video_mem_w(u8 val);

	void io_lo_addr_w(u8 val);
	void io_hi_addr_w(u8 val);

	void window_lo_addr_w(u8 val);
	void window_hi_addr_w(u8 val);

	void ctrl_w(u8 val);
	u8 ctrl_r();

private:

	// Reads
	devcb_read8  m_ctrl_r;
	devcb_read8  m_video_mem_r;

	// Writes
	devcb_write8 m_video_mem_w;
	devcb_write8 m_io_lo_addr;
	devcb_write8 m_io_hi_addr;
	devcb_write8 m_window_lo_addr;
	devcb_write8 m_window_hi_addr;
	devcb_write8 m_ctrl_w;
};

DECLARE_DEVICE_TYPE(SIGMASOFT_PARALLEL_PORT, sigmasoft_parallel_port)


#endif // MAME_HEATHKIT_SIGMASOFT_PARALLEL_PORT_H
