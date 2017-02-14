// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2alfam2.h

    Implementation of the ALF Apple Music II card and compatibles

*********************************************************************/

#ifndef __A2BUS_ALFAM2__
#define __A2BUS_ALFAM2__

#include "a2bus.h"
#include "sound/sn76496.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_sn76489_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_sn76489_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<sn76489_device> m_sn1;
	required_device<sn76489_device> m_sn2;
	required_device<sn76489_device> m_sn3;
	optional_device<sn76489_device> m_sn4;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	uint8_t m_latch0, m_latch1, m_latch2, m_latch3;

protected:
	bool m_has4thsn;
};

class a2bus_alfam2_device : public a2bus_sn76489_device
{
public:
	a2bus_alfam2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_aesms_device : public a2bus_sn76489_device
{
public:
	a2bus_aesms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};

// device type definition
extern const device_type A2BUS_ALFAM2;
extern const device_type A2BUS_AESMS;

#endif /* __A2BUS_ALFAM2__ */
