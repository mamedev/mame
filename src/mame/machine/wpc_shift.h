// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Shift-based protection simulation

#ifndef WPC_SHIFT_H
#define WPC_SHIFT_H

#define MCFG_WPC_SHIFT_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, WPC_SHIFT, 0 )

class wpc_shift_device : public device_t
{
public:
	wpc_shift_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~wpc_shift_device();

	DECLARE_ADDRESS_MAP(registers, 8);

	DECLARE_READ8_MEMBER(adrh_r);
	DECLARE_WRITE8_MEMBER(adrh_w);
	DECLARE_READ8_MEMBER(adrl_r);
	DECLARE_WRITE8_MEMBER(adrl_w);
	DECLARE_READ8_MEMBER(val1_r);
	DECLARE_WRITE8_MEMBER(val1_w);
	DECLARE_READ8_MEMBER(val2_r);
	DECLARE_WRITE8_MEMBER(val2_w);

protected:
	UINT16 adr;
	UINT8 val1, val2;

	virtual void device_start() override;
	virtual void device_reset() override;
};

extern const device_type WPC_SHIFT;

#endif
