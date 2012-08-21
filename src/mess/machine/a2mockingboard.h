/*********************************************************************

    a2mockingboard.h

    Sweet Micro Systems Mockingboard and compatibles

*********************************************************************/

#ifndef __A2BUS_MOCKINGBOARD__
#define __A2BUS_MOCKINGBOARD__

#include "emu.h"
#include "machine/a2bus.h"
#include "machine/6522via.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ayboard_device:
    public device_t,
    public device_a2bus_card_interface
{
public:
    // construction/destruction
    a2bus_ayboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

    // optional information overrides
    virtual machine_config_constructor device_mconfig_additions() const;

    DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
    DECLARE_WRITE_LINE_MEMBER( via2_irq_w );
    DECLARE_READ8_MEMBER(via1_in_a);
    DECLARE_READ8_MEMBER(via1_in_b);
    DECLARE_WRITE8_MEMBER(via1_out_a);
    DECLARE_WRITE8_MEMBER(via1_out_b);
    DECLARE_READ8_MEMBER(via2_in_a);
    DECLARE_READ8_MEMBER(via2_in_b);
    DECLARE_WRITE8_MEMBER(via2_out_a);
    DECLARE_WRITE8_MEMBER(via2_out_b);

protected:
    virtual void device_start();
    virtual void device_reset();

    // overrides of standard a2bus slot functions
    virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
    virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
    virtual UINT8 read_cnxx(address_space &space, UINT8 offset);
    virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data);

	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
    required_device<device_t> m_ay1;
    required_device<device_t> m_ay2;
    optional_device<device_t> m_ay3;
    optional_device<device_t> m_ay4;

    bool m_isPhasor, m_PhasorNative;

private:
    UINT8 m_porta1, m_porta2;
};

class a2bus_mockingboard_device : public a2bus_ayboard_device
{
public:
    a2bus_mockingboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class a2bus_phasor_device : public a2bus_ayboard_device
{
public:
    a2bus_phasor_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    virtual machine_config_constructor device_mconfig_additions() const;
};

// device type definition
extern const device_type A2BUS_MOCKINGBOARD;
extern const device_type A2BUS_PHASOR;

#endif  /* __A2BUS_MOCKINGBOARD__ */

