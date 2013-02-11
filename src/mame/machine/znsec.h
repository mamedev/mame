/*  CAT702 ZN security chip */

#pragma once

#ifndef __ZNSEC_H__
#define __ZNSEC_H__

#include "cpu/psx/siodev.h"

extern const device_type ZNSEC;

class znsec_device : public psxsiodev_device
{
public:
	znsec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void init(const UINT8 *transform);
	void select(int select);

protected:
	virtual void device_start();

private:
	virtual void data_in( int data, int mask );

	UINT8 compute_sbox_coef(int sel, int bit);
	void apply_bit_sbox(int sel);
	void apply_sbox(const UINT8 *sbox);

	const UINT8 *m_transform;
	int m_select;
	UINT8 m_state;
	UINT8 m_bit;
};

#endif
