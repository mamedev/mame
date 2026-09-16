// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPL_CHX_H
#define MAME_MACHINE_GENERALPLUS_GPL_CHX_H

#pragma once

class gpl_chx_device : public device_t
{
public:
	gpl_chx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto cha_write_callback() { return m_cha_output_cb.bind(); }
	auto chb_write_callback() { return m_chb_output_cb.bind(); }
	auto updateirqs_callback() { return m_updateirqs_cb.bind(); }

	bool is_cha_fifo_empty_irq() { return (m_cha_ctrl & 0x8000) ? true : false; }
	bool is_chb_fifo_empty_irq() { return (m_chb_ctrl & 0x8000) ? true : false; }

	u16 cha_ctrl_r();
	void cha_ctrl_w(u16 data);
	u16 cha_data_r();
	void cha_data_w(u16 data);
	u16 cha_fifo_r();
	void cha_fifo_w(u16 data);
	u16 chb_ctrl_r();
	void chb_ctrl_w(u16 data);
	u16 chb_data_r();
	void chb_data_w(u16 data);
	u16 chb_fifo_r();
	void chb_fifo_w(u16 data);

	void check_cha_fifo_empty();
	void check_chb_fifo_empty();

	void process_cha_fifo();
	void process_chb_fifo();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 m_cha_ctrl;
	u16 m_chb_ctrl;

	u16 m_cha_fifo[16];
	u16 m_chb_fifo[16];

	u16 m_cha_fifo_reg;
	u16 m_chb_fifo_reg;

	u8 m_cha_fifo_readpos;
	u8 m_cha_fifo_writepos;
	u8 m_cha_fifo_entries;

	u8 m_chb_fifo_readpos;
	u8 m_chb_fifo_writepos;
	u8 m_chb_fifo_entries;

	devcb_write16 m_cha_output_cb;
	devcb_write16 m_chb_output_cb;
	devcb_write_line m_updateirqs_cb;
};

DECLARE_DEVICE_TYPE(GPL_CHX, gpl_chx_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL_CHX_H
