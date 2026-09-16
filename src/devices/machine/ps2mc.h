// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 memory card device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_PS2MC_H
#define MAME_MACHINE_PS2MC_H

#pragma once

class ps2_mc_device : public device_t
{
public:
	ps2_mc_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: ps2_mc_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	ps2_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_mc_device() override;

	void recv_fifo_push(uint8_t data); // TODO: Turn me into a bus interface!
	uint8_t xmit_fifo_pop();
	uint8_t xmit_fifo_depth() const { return m_end_xmit - m_curr_xmit; }
	void process_fifos();

	uint8_t status() const { return m_status; }

	static const uint8_t SIO_DEVICE_ID = 0x81;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void xmit_fifo_push(uint8_t data);
	uint8_t recv_fifo_pop();
	uint8_t recv_fifo_depth() const { return m_end_recv - m_curr_recv; }
	void process_command(uint8_t data);

	void cmd_init();
	void cmd_get_term();

	enum : uint8_t
	{
		CMD_INIT        = 0x11,
		CMD_GET_TERM    = 0x28,
		CMD_UNKNOWN_F3  = 0xf3,
	};

	uint8_t m_recv_buf[512]; // Buffer size is a guess
	uint8_t m_xmit_buf[512];
	uint32_t m_curr_recv;
	uint32_t m_curr_xmit;
	uint32_t m_end_recv;
	uint32_t m_end_xmit;

	uint8_t m_cmd;
	uint32_t m_cmd_size;
	uint8_t m_terminator;
	uint8_t m_status;

	static const size_t BUFFER_SIZE;
	static const uint8_t DEFAULT_TERMINATOR;
};

DECLARE_DEVICE_TYPE(SONYPS2_MC, ps2_mc_device)

#endif // MAME_MACHINE_PS2MC_H
