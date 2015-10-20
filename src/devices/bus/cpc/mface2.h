// license:BSD-3-Clause
// copyright-holders:Kevin Thacker, Barry Rodewald
/*
 * mface2.h  --  Romantic Robot Multiface II expansion device for the Amstrad CPC/CPC+
 *
 *  Created on: 31/07/2011
 *
 *  I/O Ports:
 *    -  FEE8: Enables Multiface ROM and RAM
 *    -  FEEA: Disables Multiface ROM and RAM
 *
 *  When enabled, Multiface ROM is mapped to &0000, and RAM to &2000
 *
 *  When the Stop button is pressed, the Multiface II will generate an NMI
 *  (I guess the ROM/RAM is enabled when you do this also?)
 *
 *  It also monitors all I/O port writes, so that it can restore them when resuming the current application.
 */

#ifndef MFACE2_H_
#define MFACE2_H_

#include "emu.h"
#include "cpcexp.h"

/* stop button has been pressed */
#define MULTIFACE_STOP_BUTTON_PRESSED   0x0001
/* ram/rom is paged into memory space */
#define MULTIFACE_RAM_ROM_ENABLED       0x0002
/* when visible OUT commands are performed! */
#define MULTIFACE_VISIBLE               0x0004


class cpc_multiface2_device :   public device_t,
								public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_multiface2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	int multiface_hardware_enabled();
	void multiface_rethink_memory();
	void multiface_stop();
	int multiface_io_write(UINT16 offset, UINT8 data);
	void check_button_state();
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;

	DIRECT_UPDATE_MEMBER( amstrad_default );
	DIRECT_UPDATE_MEMBER( amstrad_multiface_directoverride );

	unsigned char *m_multiface_ram;
	unsigned long m_multiface_flags;

	UINT8 m_romdis;
};

// device type definition
extern const device_type CPC_MFACE2;

#endif /* MFACE2_H_ */
