// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina

#include "raiden2cop.h"



class seibu_cop_bootleg_device : public device_t
{
public:
seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( copdxbl_0_r );
	DECLARE_WRITE16_MEMBER( copdxbl_0_w );
protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 *m_cop_mcu_ram;




	required_device<raiden2cop_device> m_raiden2cop;

};

extern const device_type SEIBU_COP_BOOTLEG;

#define MCFG_SEIBU_COP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_BOOTLEG, 0)
