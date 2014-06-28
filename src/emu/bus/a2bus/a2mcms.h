// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mcms.h

    Implementation of the Mountain Computer Music System.
    This was sold standalone and also used as part of the alphaSyntauri
    and SoundChaser systems.

*********************************************************************/

#ifndef __A2BUS_MCMS__
#define __A2BUS_MCMS__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class mcms_device;

// card 1
class a2bus_mcms1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_mcms1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// comms from card 2 (oscillator parameter writes)
	mcms_device *get_engine(void);

	DECLARE_WRITE_LINE_MEMBER(irq_w);

	required_device<mcms_device> m_mcms;

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset);
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data);
	virtual bool take_c800() { return false; }
};

// card 2
class a2bus_mcms2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_mcms2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data);
	virtual bool take_c800() { return false; }

private:
	a2bus_mcms1_device *m_card1;	// card 1 for passthrough
	class mcms_device *m_engine;
};

// device type definition
extern const device_type A2BUS_MCMS1;
extern const device_type A2BUS_MCMS2;

#endif /* __A2BUS_MCMS__ */
