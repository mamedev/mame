// license:BSD-3-Clause
// copyright-holders:smf
/*  CAT702 security chip */

#ifndef MAME_MACHINE_CAT702_H
#define MAME_MACHINE_CAT702_H

#pragma once


DECLARE_DEVICE_TYPE(CAT702, cat702_device)
DECLARE_DEVICE_TYPE(CAT702_PIU, cat702_piu_device)

class cat702_device_base : public device_t
{
public:
	// configuration helpers
	auto dataout_handler() { return m_dataout_handler.bind(); }

	void write_datain(int state);

protected:
	cat702_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	void apply_bit_sbox(int sel);
	void apply_sbox(const uint8_t *sbox);

	int m_select;
	int m_clock;
	int m_datain;
	uint8_t m_state;
	uint8_t m_bit;

	devcb_write_line m_dataout_handler;

private:
	uint8_t compute_sbox_coef(int sel, int bit);

	optional_memory_region m_region;
	uint8_t m_transform[8];
};

class cat702_device : public cat702_device_base
{
public:
	cat702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_select(int state);
	void write_clock(int state);
};

class cat702_piu_device : public cat702_device_base
{
public:
	cat702_piu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_select(int state);
	void write_clock(int state);
};

#endif // MAME_MACHINE_CAT702_H
