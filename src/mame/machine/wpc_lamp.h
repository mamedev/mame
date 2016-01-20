// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller lamp control

#ifndef WPC_LAMP_H
#define WPC_LAMP_H

#define MCFG_WPC_LAMP_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, WPC_LAMP, 0 )

class wpc_lamp_device : public device_t
{
public:
	wpc_lamp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~wpc_lamp_device();

	DECLARE_WRITE8_MEMBER(row_w);
	DECLARE_WRITE8_MEMBER(col_w);

	void set_names(const char *const *lamp_names);

protected:
	UINT8 state[64];
	UINT8 col, row;
	emu_timer *timer;
	const char *const *names;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update();
};

extern const device_type WPC_LAMP;

#endif
