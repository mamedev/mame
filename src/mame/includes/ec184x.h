// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*****************************************************************************
 *
 * includes/ec184x.h
 *
 ****************************************************************************/

#ifndef EC184X_H_
#define EC184X_H_

#include "includes/genpc.h"

#define MCFG_EC1841_MOTHERBOARD_ADD(_tag, _cputag) \
	MCFG_DEVICE_ADD(_tag, EC1841_MOTHERBOARD, 0) \
	ec1841_mb_device::static_set_cputag(*device, _cputag);

// ======================> ibm5150_mb_device
class ec1841_mb_device : public ibm5160_mb_device
{
public:
	// construction/destruction
	ec1841_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

public:
	virtual DECLARE_READ8_MEMBER ( pc_ppi_portc_r );
	virtual DECLARE_WRITE8_MEMBER( pc_ppi_portb_w );
};


// device type definition
extern const device_type EC1841_MOTHERBOARD;

#endif /* EC184X_H_ */
