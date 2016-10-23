// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef NAMCO50_H
#define NAMCO50_H

#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_50XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_50XX, _clock)


/* device get info callback */
class namco_50xx_device : public device_t
{
public:
	namco_50xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	WRITE_LINE_MEMBER(read_request);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask);

	uint8_t K_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t R0_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t R2_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void O_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	TIMER_CALLBACK_MEMBER( latch_callback );
	TIMER_CALLBACK_MEMBER( readrequest_callback );
	TIMER_CALLBACK_MEMBER( irq_clear );
	void irq_set();
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t                   m_latched_cmd;
	uint8_t                   m_latched_rw;
	uint8_t                   m_portO;
};

extern const device_type NAMCO_50XX;

#endif  /* NAMCO50_H */
