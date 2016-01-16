// license:BSD-3-Clause
// copyright-holders:smf
/*  CAT702 security chip */

#pragma once

#ifndef __CAT702_H__
#define __CAT702_H__

#include "emu.h"

extern const device_type CAT702;

#define MCFG_CAT702_DATAOUT_HANDLER(_devcb) \
	devcb = &cat702_device::set_dataout_handler(*device, DEVCB_##_devcb);

class cat702_device : public device_t
{
public:
	cat702_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_dataout_handler(device_t &device, _Object object) { return downcast<cat702_device &>(device).m_dataout_handler.set_callback(object); }

	void init(const UINT8 *transform); // TODO: region

	DECLARE_WRITE_LINE_MEMBER(write_select);
	DECLARE_WRITE_LINE_MEMBER(write_datain);
	DECLARE_WRITE_LINE_MEMBER(write_clock);

protected:
	virtual void device_start() override;

private:
	UINT8 compute_sbox_coef(int sel, int bit);
	void apply_bit_sbox(int sel);
	void apply_sbox(const UINT8 *sbox);

	const UINT8 *m_transform;
	int m_select;
	int m_clock;
	int m_datain;
	UINT8 m_state;
	UINT8 m_bit;

	devcb_write_line m_dataout_handler;
};

#endif
