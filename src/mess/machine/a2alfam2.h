/*********************************************************************

    a2alfam2.h

    Implementation of the ALF Apple Music II card

*********************************************************************/

#ifndef __A2BUS_ALFAM2__
#define __A2BUS_ALFAM2__

#include "emu.h"
#include "machine/a2bus.h"
#include "sound/sn76496.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_alfam2_device:
    public device_t,
    public device_a2bus_card_interface
{
public:
    // construction/destruction
    a2bus_alfam2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
    a2bus_alfam2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    // optional information overrides
    virtual machine_config_constructor device_mconfig_additions() const;

    required_device<sn76489_device> m_sn1;
    required_device<sn76489_device> m_sn2;
    required_device<sn76489_device> m_sn3;

protected:
    virtual void device_start();
    virtual void device_reset();

    // overrides of standard a2bus slot functions
    virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
    virtual bool take_c800();
};

// device type definition
extern const device_type A2BUS_ALFAM2;

#endif /* __A2BUS_ALFAM2__ */

