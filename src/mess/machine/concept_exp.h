#ifndef __CONCEPT_EXP__
#define __CONCEPT_EXP__

#include "emu.h"
#include "machine/wd17xx.h"
#include "machine/corvushd.h"

// FIXME: Concept expansion ports should just use the Apple II Bus device!
// The code below is outdated and inaccurate!

class concept_exp_card_device;


class concept_exp_port_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	concept_exp_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( reg_r );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( reg_w );
	DECLARE_WRITE8_MEMBER( rom_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	concept_exp_card_device *m_card;
};


// device type definition
extern const ATTR_DEPRECATED device_type CONCEPT_EXP_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CONCEPT_EXP_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, CONCEPT_EXP_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)





//**************************************************************************
//  CARD INTERFACE
//**************************************************************************


class concept_exp_card_device : public device_slot_card_interface
{
public:
	// construction/destruction
	concept_exp_card_device(const machine_config &mconfig, device_t &device);

	DECLARE_READ8_MEMBER( reg_r ) { return 0xff; }
	DECLARE_READ8_MEMBER( rom_r ) { return 0xff; }
	DECLARE_WRITE8_MEMBER( reg_w ) {}
	DECLARE_WRITE8_MEMBER( rom_w ) {}

protected:
};


class concept_fdc_device : public device_t,
							public concept_exp_card_device
{
public:
	// construction/destruction
	concept_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(reg_r);
	DECLARE_READ8_MEMBER(rom_r);
	DECLARE_WRITE8_MEMBER(reg_w);

	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	fd1793_device *m_wd179x;

	UINT8 m_fdc_local_status;
	UINT8 m_fdc_local_command;
};

class concept_hdc_device : public device_t,
							public concept_exp_card_device
{
public:
	// construction/destruction
	concept_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( reg_r );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( reg_w );

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	required_device<corvus_hdc_t> m_hdc;
};


extern const ATTR_DEPRECATED device_type CONCEPT_FDC;
extern const ATTR_DEPRECATED device_type CONCEPT_HDC;

#endif
