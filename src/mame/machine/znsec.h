/*  CAT702 ZN security chip */

#pragma once

#ifndef __ZNSEC_H__
#define __ZNSEC_H__

#include "emu.h"

extern const device_type ZNSEC;

class znsec_device : public device_t
{
public:
	znsec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void init(const UINT8 *transform);
	void select(int select);
	UINT8 step(UINT8 input);

protected:
	void device_start();

private:
	UINT8 compute_sbox_coef(int sel, int bit);
	void apply_bit_sbox(int sel);
	void apply_sbox(const UINT8 *sbox);

	const UINT8 *m_transform;
	int m_select;
	UINT8 m_state;
	UINT8 m_bit;
};

#endif
