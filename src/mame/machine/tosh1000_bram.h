// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Toshiba 1000 Backup RAM

    Simulation of interface provided by 80C50 keyboard controller.

***************************************************************************/

#ifndef MAME_MACHINE_TOSH1000_BRAM_H
#define MAME_MACHINE_TOSH1000_BRAM_H

#pragma once


// ======================> tosh1000_bram_device

class tosh1000_bram_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	tosh1000_bram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	uint8_t read(int offset);
	void write(int offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	enum { BRAM_SIZE = 160 };

	uint8_t m_bram[BRAM_SIZE];
};

// device type definition
DECLARE_DEVICE_TYPE(TOSH1000_BRAM, tosh1000_bram_device)

#endif // MAME_MACHINE_TOSH1000_BRAM_H
