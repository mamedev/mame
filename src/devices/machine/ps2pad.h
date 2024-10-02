// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony DualShock 2 device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_PS2PAD_H
#define MAME_MACHINE_PS2PAD_H

#pragma once

class ps2_pad_device : public device_t
{
public:
	ps2_pad_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: ps2_pad_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	ps2_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_pad_device() override;

	void recv_fifo_push(uint8_t data); // TODO: Turn me into a bus interface!
	uint8_t xmit_fifo_pop();
	uint8_t xmit_fifo_depth() const { return m_end_xmit - m_curr_xmit; }
	void process_fifos();

	static const uint8_t SIO_DEVICE_ID = 0x01;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void xmit_fifo_push(uint8_t data);
	uint8_t recv_fifo_pop();
	uint8_t recv_fifo_depth() const { return m_end_recv - m_curr_recv; }
	void process_command(uint8_t data);

	void cmd_read_buttons();
	void cmd_config();
	void cmd_get_model();
	void cmd_get_act();
	void cmd_get_comb();
	void cmd_get_mode();

	enum : uint8_t
	{
		CMD_READ_BUTTONS    = 0x42,
		CMD_CONFIG          = 0x43,
		CMD_GET_MODEL       = 0x45,
		CMD_GET_ACT         = 0x46,
		CMD_GET_COMB        = 0x47,
		CMD_GET_MODE        = 0x4c,
	};

	uint8_t m_recv_buf[64]; // Buffer size is a guess
	uint8_t m_xmit_buf[64];
	uint8_t m_curr_recv;
	uint8_t m_curr_xmit;
	uint8_t m_end_recv;
	uint8_t m_end_xmit;

	uint8_t m_cmd;
	uint8_t m_cmd_size;
	bool m_configuring;

	static const size_t BUFFER_SIZE;
};

DECLARE_DEVICE_TYPE(SONYPS2_PAD, ps2_pad_device)

#endif // MAME_MACHINE_PS2PAD_H
