// license:BSD-3-Clause
// copyright-holders:smf
/*  CAT702 security chip */

#ifndef MAME_MACHINE_CAT702_H
#define MAME_MACHINE_CAT702_H

#pragma once


DECLARE_DEVICE_TYPE(CAT702, cat702_device)
DECLARE_DEVICE_TYPE(CAT702_PIU, cat702_piu_device)

class cat702_device : public device_t
{
public:
	cat702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto dataout_handler() { return m_dataout_handler.bind(); }

	void write_select(int state);
	void write_datain(int state);
	void write_clock(int state);

protected:
	cat702_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

	uint8_t compute_sbox_coef(int sel, int bit);
	void apply_bit_sbox(int sel);
	void apply_sbox(const uint8_t *sbox);

	static constexpr uint8_t initial_sbox[8] = { 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x7f };

	optional_memory_region m_region;
	uint8_t m_transform[8];

	int m_select;
	int m_clock;
	int m_datain;
	uint8_t m_state;
	uint8_t m_bit;

	devcb_write_line m_dataout_handler;
};

class cat702_piu_device : public cat702_device
{
public:
	cat702_piu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: cat702_device(mconfig, CAT702_PIU, tag, owner, clock) {}

	void write_select(int state);
	void write_clock(int state);
};

#endif // MAME_MACHINE_CAT702_H
