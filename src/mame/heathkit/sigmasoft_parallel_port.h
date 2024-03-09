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
	sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void    write(offs_t reg, uint8_t val);
	uint8_t read(offs_t reg);

	auto ctrl_r_cb()      { return m_ctrl_r.bind(); }
	auto video_mem_r_cb() { return m_video_mem_r.bind(); }

	auto video_mem_cb()   { return m_video_mem_w.bind(); }
	auto io_lo_cb()       { return m_io_lo_addr.bind(); }
	auto io_hi_cb()       { return m_io_hi_addr.bind(); }
	auto window_lo_cb()   { return m_window_lo_addr.bind(); }
	auto window_hi_cb()   { return m_window_hi_addr.bind(); }
	auto ctrl_cb()        { return m_ctrl_w.bind(); }

protected:

	virtual void device_start() override;

	uint8_t video_mem_r();
	void video_mem_w(uint8_t val);

	void io_lo_addr_w(uint8_t val);
	void io_hi_addr_w(uint8_t val);

	void window_lo_addr_w(uint8_t val);
	void window_hi_addr_w(uint8_t val);

	void ctrl_w(uint8_t val);
	uint8_t ctrl_r();

private:

	// Reads
	devcb_read8 m_ctrl_r;
	devcb_read8 m_video_mem_r;

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
