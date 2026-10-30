// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88_ALU_H
#define MAME_NEC_PC88_ALU_H

#pragma once

class pc88_alu_device : public device_t
{
public:
	pc88_alu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ctrl1_w(u8 data);
	void ctrl2_w(u8 data);

	u8 alu_r(offs_t offset);
	void alu_w(offs_t offset, u8 data);

	auto gvram_read_cb()  { return m_gvram_read.bind(); }
	auto gvram_write_cb() { return m_gvram_write.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	devcb_read8  m_gvram_read;
	devcb_write8 m_gvram_write;

	u8 m_ctrl1;
	u8 m_ctrl2;
	u8 m_reg[3];
};

DECLARE_DEVICE_TYPE(PC88_ALU,   pc88_alu_device)

#endif // MAME_NEC_PC88_ALU_H
