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
	wpc_lamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_lamp_device();

	void row_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void set_names(const char *const *lamp_names);

protected:
	uint8_t state[64];
	uint8_t col, row;
	emu_timer *timer;
	const char *const *names;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update();
};

extern const device_type WPC_LAMP;

#endif
