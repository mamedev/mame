// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

	Toshiba 1000 Backup RAM

	Simulation of interface provided by 80C50 keyboard controller.

***************************************************************************/

#ifndef __TOSH1000_BRAM_H__
#define __TOSH1000_BRAM_H__

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TOSH1000_BRAM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TOSH1000_BRAM, 0)


#define BRAM_SIZE  160

// ======================> tosh1000_bram_device

class tosh1000_bram_device :   public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	tosh1000_bram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

public:
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	uint8_t read(int offset);
	void write(int offset, uint8_t data);

private:
	uint8_t m_bram[BRAM_SIZE];
};

// device type definition
extern const device_type TOSH1000_BRAM;

#endif
