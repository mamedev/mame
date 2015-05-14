// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor NMC9306 256-Bit Serial EEPROM emulation

**********************************************************************
                            _____   _____
                    CS   1 |*    \_/     | 8   Vcc
                    SK   2 |             | 7   NC
                    DI   3 |             | 6   NC
                    DO   4 |_____________| 5   GND

**********************************************************************/

#pragma once

#ifndef __NMC9306__
#define __NMC9306__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NMC9306_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NMC9306, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> nmc9306_device

class nmc9306_device :  public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	nmc9306_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( sk_w );
	DECLARE_WRITE_LINE_MEMBER( di_w );
	DECLARE_READ_LINE_MEMBER( do_r );

protected:
	// device-level overrides
	virtual void device_start();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

private:
	inline UINT16 read(offs_t offset);
	inline void write(offs_t offset, UINT16 data);
	inline void erase(offs_t offset);

	UINT16 m_register[16];

	int m_bits;
	int m_state;
	UINT8 m_command;
	UINT8 m_address;
	UINT16 m_data;
	bool m_ewen;
	int m_cs;
	int m_sk;
	int m_do;
	int m_di;
};


// device type definition
extern const device_type NMC9306;


#endif
