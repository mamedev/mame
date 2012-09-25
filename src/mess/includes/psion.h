/***************************************************************************

        Psion Organiser II series

****************************************************************************/

#pragma once

#ifndef _PSION_H_
#define _PSION_H_

#include "cpu/m6800/m6800.h"
#include "machine/psion_pack.h"
#include "video/hd44780.h"
#include "sound/beep.h"

#define MCFG_PSION_CUSTOM_LCDC_ADD( _tag , _config) \
	MCFG_DEVICE_ADD( _tag, PSION_CUSTOM_LCDC, 0 ) \
	MCFG_DEVICE_CONFIG(_config)

// ======================> psion_custom_lcdc

class psion_custom_lcdc :	public hd44780_device
{

public:
	// construction/destruction
	psion_custom_lcdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// hd44780_device overrides
	virtual DECLARE_WRITE8_MEMBER(control_write);
	virtual UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


// ======================> psion_state

class psion_state : public driver_device
{
public:
	psion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_lcdc(*this, "hd44780"),
		  m_beep(*this, BEEPER_TAG),
		  m_pack1(*this, "pack1"),
		  m_pack2(*this, "pack2"),
		m_sys_register(*this, "sys_register"),
		m_ram(*this, "ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<device_t> m_beep;
	required_device<datapack_device> m_pack1;
	required_device<datapack_device> m_pack2;

	UINT16 m_kb_counter;
	UINT8 m_enable_nmi;
	optional_shared_ptr<UINT8> m_sys_register;
	UINT8 m_tcsr_value;
	UINT8 m_stby_pwr;
	UINT8 m_pulse;

	UINT8 m_port2_ddr;	// datapack i/o ddr
	UINT8 m_port2;		// datapack i/o data bus
	UINT8 m_port6_ddr;	// datapack control lines ddr
	UINT8 m_port6;		// datapack control lines

	// RAM/ROM banks
	required_shared_ptr<UINT8> m_ram;
	UINT8 *m_paged_ram;
	UINT8 m_rom_bank;
	UINT8 m_ram_bank;
	UINT8 m_ram_bank_count;
	UINT8 m_rom_bank_count;

	virtual void machine_start();
	virtual void machine_reset();

	UINT8 kb_read(running_machine &machine);
	void update_banks(running_machine &machine);
	DECLARE_WRITE8_MEMBER( hd63701_int_reg_w );
	DECLARE_READ8_MEMBER( hd63701_int_reg_r );
	void io_rw(address_space &space, UINT16 offset);
	DECLARE_WRITE8_MEMBER( io_w );
	DECLARE_READ8_MEMBER( io_r );
	virtual void palette_init();
	DECLARE_INPUT_CHANGED_MEMBER(psion_on);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer);
};

// device type definition
extern const device_type PSION_CUSTOM_LCDC;

#endif	// _PSION_H_
