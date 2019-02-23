// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    i2cmem.h

    I2C Memory

***************************************************************************/

#ifndef MAME_MACHINE_I2CMEM_H
#define MAME_MACHINE_I2CMEM_H

#pragma once


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define I2CMEM_SLAVE_ADDRESS ( 0xa0 )
#define I2CMEM_SLAVE_ADDRESS_ALT ( 0xb0 )


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i2cmem_device

class i2cmem_device :
	public device_t,
	public device_nvram_interface
{
public:
	// construction/destruction
	i2cmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	i2cmem_device & set_address(int address) { m_slave_address = address; return *this; }
	i2cmem_device & set_page_size(int page_size) { m_page_size = page_size; return *this; }
	i2cmem_device & set_data_size(int data_size) { m_data_size = data_size; return *this; }
	i2cmem_device & set_e0(int e0) { m_e0 = e0; return *this; }
	i2cmem_device & set_e1(int e1) { m_e1 = e1; return *this; }
	i2cmem_device & set_e2(int e2) { m_e2 = e2; return *this; }
	i2cmem_device & set_wc(int wc) { m_wc = wc; return *this; }

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( write_e0 );
	DECLARE_WRITE_LINE_MEMBER( write_e1 );
	DECLARE_WRITE_LINE_MEMBER( write_e2 );
	DECLARE_WRITE_LINE_MEMBER( write_sda );
	DECLARE_WRITE_LINE_MEMBER( write_scl );
	DECLARE_WRITE_LINE_MEMBER( write_wc );
	DECLARE_READ_LINE_MEMBER( read_sda );

protected:
	// construction/destruction
	i2cmem_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int page_size, int data_size);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read( emu_file &file ) override;
	virtual void nvram_write( emu_file &file ) override;

	// internal helpers
	int address_mask();
	int select_device();
	int data_offset();

	optional_memory_region m_region;

	// internal state
	std::unique_ptr<uint8_t[]> m_data;
	int m_slave_address;
	int m_page_size;
	int m_data_size;
	int m_scl;
	int m_sdaw;
	int m_e0;
	int m_e1;
	int m_e2;
	int m_wc;
	int m_sdar;
	int m_state;
	int m_bits;
	int m_shift;
	int m_devsel;
	int m_byteaddr;
	std::vector<uint8_t> m_page;
	int m_page_offset;
};

#define DECLARE_I2C_DEVICE(name) \
	class i2c_##name##_device : public i2cmem_device \
	{ \
	public: \
		i2c_##name##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0); \
	};

DECLARE_I2C_DEVICE(x2404p)
DECLARE_I2C_DEVICE(24c01)
DECLARE_I2C_DEVICE(24c02)
DECLARE_I2C_DEVICE(24c08)
DECLARE_I2C_DEVICE(24c16);
DECLARE_I2C_DEVICE(24c16a);
DECLARE_I2C_DEVICE(24c64);

// device type definition
DECLARE_DEVICE_TYPE(I2CMEM,     i2cmem_device)
DECLARE_DEVICE_TYPE(I2C_X2404P, i2c_x2404p_device)
DECLARE_DEVICE_TYPE(I2C_24C01,  i2c_24c01_device)
DECLARE_DEVICE_TYPE(I2C_24C02,  i2c_24c02_device)
DECLARE_DEVICE_TYPE(I2C_24C08,  i2c_24c08_device)
DECLARE_DEVICE_TYPE(I2C_24C16,  i2c_24c16_device)
DECLARE_DEVICE_TYPE(I2C_24C16A, i2c_24c16a_device)
DECLARE_DEVICE_TYPE(I2C_24C64,  i2c_24c64_device)

#endif // MAME_MACHINE_I2CMEM_H
