// license:BSD-3-Clause
// copyright-holders:smf
/*  CAT702 security chip */

#ifndef MAME_MACHINE_CAT702_H
#define MAME_MACHINE_CAT702_H

#pragma once


DECLARE_DEVICE_TYPE(CAT702, cat702_device)

class cat702_device : public device_t
{
public:
	cat702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto dataout_handler() { return m_dataout_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER(write_select);
	DECLARE_WRITE_LINE_MEMBER(write_datain);
	DECLARE_WRITE_LINE_MEMBER(write_clock);

protected:
	virtual void device_start() override;

private:
	uint8_t compute_sbox_coef(int sel, int bit);
	void apply_bit_sbox(int sel);
	void apply_sbox(const uint8_t *sbox);

	optional_memory_region m_region;
	uint8_t m_transform[8];

	int m_select;
	int m_clock;
	int m_datain;
	uint8_t m_state;
	uint8_t m_bit;

	devcb_write_line m_dataout_handler;
};

#endif // MAME_MACHINE_CAT702_H
