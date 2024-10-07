// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

    Tasc Final ChessCard cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_FCC_H
#define MAME_BUS_C64_FCC_H

#pragma once

#include "exp.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/gen_latch.h"



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
	c64_final_chesscard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_c64_expansion_card_interface implementation
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<m65sc02_device> m_maincpu;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<generic_latch_8_device> m_sublatch;

	uint8_t m_bank;
	int m_hidden;

	void mainlatch_int(int state) { m_slot->nmi_w(state); }
	uint8_t rom_r(offs_t offset) { return m_romx[offset]; } // cartridge CPU ROM
	uint8_t nvram_r(offs_t offset) { return m_nvram[offset & 0x1fff]; }
	void nvram_w(offs_t offset, uint8_t data) { m_nvram[offset & 0x1fff] = data; }

	void c64_fcc_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_FCC, c64_final_chesscard_device)


#endif // MAME_BUS_C64_FCC_H
