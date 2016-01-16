// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tasc Final ChessCard cartridge emulation

**********************************************************************/

#pragma once

#ifndef __FCC__
#define __FCC__

#include "emu.h"
#include "exp.h"
#include "cpu/m6502/m65sc02.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_final_chesscard_device

class c64_final_chesscard_device : public device_t,
									public device_c64_expansion_card_interface,
									public device_nvram_interface
{
public:
	// construction/destruction
	c64_final_chesscard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER( nvram_r );
	DECLARE_WRITE8_MEMBER( nvram_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual void nvram_read(emu_file &file) override { if (m_nvram != nullptr) { file.read(m_nvram, m_nvram.bytes()); } }
	virtual void nvram_write(emu_file &file) override { if (m_nvram != nullptr) { file.write(m_nvram, m_nvram.bytes()); } }

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<m65sc02_device> m_maincpu;

	UINT8 m_bank;
	int m_ramen;
};


// device type definition
extern const device_type C64_FCC;



#endif
