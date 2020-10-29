// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mockingboard.h

    Sweet Micro Systems Mockingboard and compatibles

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2MOCKINGBOARD_H
#define MAME_BUS_A2BUS_A2MOCKINGBOARD_H

#include "a2bus.h"
#include "machine/6522via.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"
#include "sound/votrax.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ayboard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( via2_irq_w );
	void via1_out_a(uint8_t data);
	virtual void via1_out_b(uint8_t data);
	void via2_out_a(uint8_t data);
	virtual void via2_out_b(uint8_t data);

protected:
	// construction/destruction
	a2bus_ayboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override { return 0xff; }
	virtual void write_c0nx(uint8_t offset, uint8_t data) override { }
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;

	void add_common_devices(machine_config &config);

	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;

	uint8_t m_porta1;
	uint8_t m_porta2;
};

class a2bus_mockingboard_device : public a2bus_ayboard_device
{
public:
	a2bus_mockingboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void via1_out_b(uint8_t data) override;
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_reset() override;

	required_device<votrax_sc01_device> m_sc01;

private:
	DECLARE_WRITE_LINE_MEMBER(write_via1_cb2);

	uint8_t m_portb1;
	int m_last_cb2_state;
};

class a2bus_phasor_device : public a2bus_ayboard_device
{
public:
	a2bus_phasor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void via1_out_b(uint8_t data) override;
	void via2_out_b(uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;

	required_device<ay8913_device> m_ay3;
	required_device<ay8913_device> m_ay4;

private:
	void set_clocks();

	bool m_native;
};

class a2bus_echoplus_device : public a2bus_ayboard_device
{
public:
	a2bus_echoplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<tms5220_device> m_tms;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_MOCKINGBOARD, a2bus_mockingboard_device)
DECLARE_DEVICE_TYPE(A2BUS_PHASOR,       a2bus_phasor_device)
DECLARE_DEVICE_TYPE(A2BUS_ECHOPLUS,     a2bus_echoplus_device)

#endif  // MAME_BUS_A2BUS_A2MOCKINGBOARD_H
