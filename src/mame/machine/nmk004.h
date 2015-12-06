// license:BSD-3-Clause
// copyright-holders:David Haywood,Alex Marshall
/***************************************************************************

 NMK004 emulation

***************************************************************************/

#ifndef NMK004_H
#define NMK004_H

#include "cpu/tlcs90/tlcs90.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"

#define MCFG_NMK004_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NMK004, _clock)


/* device get info callback */
class nmk004_device : public device_t
{
public:
	nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	WRITE8_MEMBER( write );
	READ8_MEMBER( read );

	DECLARE_WRITE8_MEMBER(nmk004_port4_w);
	DECLARE_WRITE8_MEMBER(nmk004_oki0_bankswitch_w);
	DECLARE_WRITE8_MEMBER(nmk004_oki1_bankswitch_w);
	DECLARE_READ8_MEMBER(nmk004_tonmk004_r);
	DECLARE_WRITE8_MEMBER(nmk004_tomain_w);
	void ym2203_irq_handler(int irq);
	required_device<tlcs90_device> m_cpu;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;


private:
	// internal state
	required_device<cpu_device> m_systemcpu;
	UINT8 to_nmk004;
	UINT8 to_main;

};

extern const device_type NMK004;

#endif  /* NMK004_H */
