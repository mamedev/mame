#ifndef MAME_MACHINE_PS2M50_H
#define MAME_MACHINE_PS2M50_H

#include "machine/ibmps2.h"

class ps2_m50_t1_mb_device : public ps2_mb_device
{
public:
	ps2_m50_t1_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint16_t sram_size, uint16_t pos_id)
		: ps2_mb_device(mconfig, tag, owner, clock, sram_size, pos_id, false)
	{
	}

    ps2_m50_t1_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    	: ps2_mb_device(mconfig, tag, owner, clock, 0x0000, 0xfbff, false)
	{
	}

    virtual void 	map(address_map &map) override;
	
protected:
	virtual void 	device_start() override;
	virtual void 	device_reset() override;
	virtual void 	device_add_mconfig(machine_config &config) override;
    virtual void 	device_config_complete() override;

	virtual uint8_t planar_pos_r(offs_t offset) override;
	virtual void 	planar_pos_w(offs_t offset, uint8_t data) override;

	virtual void	update_memory_control_register(uint8_t data);

	uint8_t			m_system_board_memory_enabled;
private:
};

class ps2_m60_mb_device : public ps2_m50_t1_mb_device
{
public:
    ps2_m60_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    	: ps2_m50_t1_mb_device(mconfig, tag, owner, clock, 0x0800, 0xf7ff)
	{
	}

protected:
	virtual void 	device_start() override;
	virtual void 	device_reset() override;
	virtual void 	device_add_mconfig(machine_config &config) override;

private:
};

DECLARE_DEVICE_TYPE(PS2_M50_T1_MB, ps2_m50_t1_mb_device)
DECLARE_DEVICE_TYPE(PS2_M60_MB, ps2_m60_mb_device)

#endif