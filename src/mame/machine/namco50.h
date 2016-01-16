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
	namco_50xx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	WRITE8_MEMBER( write );
	WRITE_LINE_MEMBER(read_request);
	READ8_MEMBER( read );

	READ8_MEMBER( K_r );
	READ8_MEMBER( R0_r );
	READ8_MEMBER( R2_r );
	WRITE8_MEMBER( O_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	TIMER_CALLBACK_MEMBER( latch_callback );
	TIMER_CALLBACK_MEMBER( readrequest_callback );
	TIMER_CALLBACK_MEMBER( irq_clear );
	void irq_set();
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	UINT8                   m_latched_cmd;
	UINT8                   m_latched_rw;
	UINT8                   m_portO;
};

extern const device_type NAMCO_50XX;

#endif  /* NAMCO50_H */
