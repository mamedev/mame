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
	cpc_symbiface2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t ide_cs0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ide_cs0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ide_cs1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ide_cs1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rtc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rtc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mouse_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rom_rewrite_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rom_rewrite_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mouse_change_x(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void mouse_change_y(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void mouse_change_buttons(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

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

	std::vector<uint8_t> m_rom_space;

	bool m_iohigh;
	uint16_t m_ide_data;

	uint8_t m_mouse_state;
	uint8_t m_input_x;
	uint8_t m_input_y;

	// stores backup pointers so that mapping can be restored
	uint8_t* m_4xxx_ptr_r;
	uint8_t* m_4xxx_ptr_w;
	uint8_t* m_6xxx_ptr_r;
	uint8_t* m_6xxx_ptr_w;
};

// device type definition
extern const device_type CPC_SYMBIFACE2;


#endif /* SYMBFAC2_H_ */
