// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * symbfac2.h
 *
 *  Created on: 2/08/2014
 */

#ifndef SYMBFAC2_H_
#define SYMBFAC2_H_

#include "emu.h"
#include "machine/ataintf.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"
#include "cpcexp.h"

class cpc_symbiface2_device  : public device_t,
								public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_symbiface2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER(ide_cs0_r);
	DECLARE_WRITE8_MEMBER(ide_cs0_w);
	DECLARE_READ8_MEMBER(ide_cs1_r);
	DECLARE_WRITE8_MEMBER(ide_cs1_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(mouse_r);
	DECLARE_READ8_MEMBER(rom_rewrite_r);
	DECLARE_WRITE8_MEMBER(rom_rewrite_w);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_x);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_y);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_buttons);

	enum
	{
		PS2_MOUSE_IDLE = 0,
		PS2_MOUSE_X,
		PS2_MOUSE_Y,
		PS2_MOUSE_BUTTONS,
		PS2_MOUSE_SCROLL
	};
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;
	required_device<ata_interface_device> m_ide;
	required_device<ds12885_device> m_rtc;
	required_device<nvram_device> m_nvram;

	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_buttons;

	dynamic_buffer m_rom_space;

	bool m_iohigh;
	UINT16 m_ide_data;

	UINT8 m_mouse_state;
	UINT8 m_input_x;
	UINT8 m_input_y;

	// stores backup pointers so that mapping can be restored
	UINT8* m_4xxx_ptr_r;
	UINT8* m_4xxx_ptr_w;
	UINT8* m_6xxx_ptr_r;
	UINT8* m_6xxx_ptr_w;
};

// device type definition
extern const device_type CPC_SYMBIFACE2;


#endif /* SYMBFAC2_H_ */
