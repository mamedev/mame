// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mockingboard.h

    Sweet Micro Systems Mockingboard and compatibles

*********************************************************************/

#ifndef __A2BUS_MOCKINGBOARD__
#define __A2BUS_MOCKINGBOARD__

#include "emu.h"
#include "a2bus.h"
#include "machine/6522via.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ayboard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ayboard_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via2_irq_w );
	DECLARE_WRITE8_MEMBER(via1_out_a);
	DECLARE_WRITE8_MEMBER(via1_out_b);
	DECLARE_WRITE8_MEMBER(via2_out_a);
	DECLARE_WRITE8_MEMBER(via2_out_b);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;

	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;
	optional_device<ay8913_device> m_ay3;
	optional_device<ay8913_device> m_ay4;

	bool m_isPhasor, m_PhasorNative;

private:
	UINT8 m_porta1, m_porta2;
};

class a2bus_mockingboard_device : public a2bus_ayboard_device
{
public:
	a2bus_mockingboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class a2bus_phasor_device : public a2bus_ayboard_device
{
public:
	a2bus_phasor_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};

class a2bus_echoplus_device : public a2bus_ayboard_device
{
public:
	a2bus_echoplus_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;

	required_device<tms5220_device> m_tms;

	virtual machine_config_constructor device_mconfig_additions() const override;
};

// device type definition
extern const device_type A2BUS_MOCKINGBOARD;
extern const device_type A2BUS_PHASOR;
extern const device_type A2BUS_ECHOPLUS;

#endif  /* __A2BUS_MOCKINGBOARD__ */
