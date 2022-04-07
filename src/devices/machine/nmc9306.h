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

#ifndef MAME_MACHINE_NMC9306_H
#define MAME_MACHINE_NMC9306_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> nmc9306_device

class nmc9306_device :  public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	nmc9306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( sk_w );
	DECLARE_WRITE_LINE_MEMBER( di_w );
	DECLARE_READ_LINE_MEMBER( do_r );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	inline uint16_t read(offs_t offset);
	inline void write(offs_t offset, uint16_t data);
	inline void erase(offs_t offset);

	uint16_t m_register[16];

	int m_bits;
	int m_state;
	uint8_t m_command;
	uint8_t m_address;
	uint16_t m_data;
	bool m_ewen;
	int m_cs;
	int m_sk;
	int m_do;
	int m_di;
};


// device type definition
DECLARE_DEVICE_TYPE(NMC9306, nmc9306_device)

#endif // MAME_MACHINE_NMC9306_H
