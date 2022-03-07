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
	i2cmem_device & set_address(int address) { m_slave_address = address; return *this; }
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
	i2cmem_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int read_page_size, int write_page_size, int data_size);

	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read( util::read_stream &file ) override;
	virtual bool nvram_write( util::write_stream &file ) override;

	// configuration helpers
	void set_devsel_address_low(bool devsel_address_low) { m_devsel_address_low = devsel_address_low; }

	// internal helpers
	int address_mask();
	int select_device();
	int data_offset();
	bool skip_addresshigh() const { return m_data_size <= 0x800; }

	optional_memory_region m_region;

	// internal state
	std::unique_ptr<uint8_t[]> m_data;
	int m_slave_address;
	int m_read_page_size;
	int m_write_page_size;
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
	int m_addresshigh;
	int m_byteaddr;
	std::vector<uint8_t> m_page;
	int m_page_offset;
	int m_page_written_size;
	bool m_devsel_address_low;
};

#define DECLARE_I2C_DEVICE(name) \
	class i2c_##name##_device : public i2cmem_device \
	{ \
	public: \
		i2c_##name##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0); \
	};

DECLARE_I2C_DEVICE(x24c01);
DECLARE_I2C_DEVICE(24c01);
DECLARE_I2C_DEVICE(pcf8570);
DECLARE_I2C_DEVICE(pcd8572);
DECLARE_I2C_DEVICE(pcf8582);
DECLARE_I2C_DEVICE(24c02);
DECLARE_I2C_DEVICE(m24c02);
DECLARE_I2C_DEVICE(24c04);
DECLARE_I2C_DEVICE(x2404p);
DECLARE_I2C_DEVICE(24c08);
DECLARE_I2C_DEVICE(24c16);
DECLARE_I2C_DEVICE(24c64);
DECLARE_I2C_DEVICE(24c512);

// device type definition
DECLARE_DEVICE_TYPE(I2C_X24C01,  i2c_x24c01_device)
DECLARE_DEVICE_TYPE(I2C_24C01,   i2c_24c01_device)
DECLARE_DEVICE_TYPE(I2C_PCF8570, i2c_pcf8570_device)
DECLARE_DEVICE_TYPE(I2C_PCD8572, i2c_pcd8572_device)
DECLARE_DEVICE_TYPE(I2C_PCF8582, i2c_pcf8582_device)
DECLARE_DEVICE_TYPE(I2C_24C02,   i2c_24c02_device)
DECLARE_DEVICE_TYPE(I2C_M24C02,  i2c_m24c02_device)
DECLARE_DEVICE_TYPE(I2C_24C04,   i2c_24c04_device)
DECLARE_DEVICE_TYPE(I2C_X2404P,  i2c_x2404p_device)
DECLARE_DEVICE_TYPE(I2C_24C08,   i2c_24c08_device)
DECLARE_DEVICE_TYPE(I2C_24C16,   i2c_24c16_device)
DECLARE_DEVICE_TYPE(I2C_24C64,   i2c_24c64_device)
DECLARE_DEVICE_TYPE(I2C_24C512,  i2c_24c512_device)

#endif // MAME_MACHINE_I2CMEM_H
