// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Nichibutsu 1412M2 device emulation

***************************************************************************/

#ifndef MAME_MACHINE_NB1412M2_H
#define MAME_MACHINE_NB1412M2_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NB1412M2_ADD(tag, freq) \
		MCFG_DEVICE_ADD((tag), NB1412M2, (freq))


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nb1412m2_device

class nb1412m2_device : public device_t
{
public:
	// construction/destruction
	nb1412m2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( data_r );

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const override;
//  virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_command;
	uint16_t m_rom_address;

	required_region_ptr<uint8_t> m_data;
};


// device type definition
DECLARE_DEVICE_TYPE(NB1412M2, nb1412m2_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_NB1412M2_H
