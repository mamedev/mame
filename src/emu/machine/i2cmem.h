// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    i2cmem.h

    I2C Memory

***************************************************************************/

#pragma once

#ifndef __I2CMEM_H__
#define __I2CMEM_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define I2CMEM_SLAVE_ADDRESS ( 0xa0 )
#define I2CMEM_SLAVE_ADDRESS_ALT ( 0xb0 )


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I2CMEM_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, I2CMEM, 0 )

#define MCFG_I2CMEM_ADDRESS( address ) \
	i2cmem_device::set_address(*device, address);
#define MCFG_I2CMEM_PAGE_SIZE( page_size ) \
	i2cmem_device::set_page_size(*device, page_size);
#define MCFG_I2CMEM_DATA_SIZE(data_size) \
	i2cmem_device::set_data_size(*device, data_size);
#define MCFG_I2CMEM_E0(e0) \
	i2cmem_device::set_e0(*device, e0);
#define MCFG_I2CMEM_E1(e1) \
	i2cmem_device::set_e1(*device, e1);
#define MCFG_I2CMEM_E2(e2) \
	i2cmem_device::set_e2(*device, e2);
#define MCFG_I2CMEM_WC(wc) \
	i2cmem_device::set_wc(*device, wc);

#define MCFG_X2404P_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_PAGE_SIZE(8) \
	MCFG_I2CMEM_DATA_SIZE(0x200)

#define MCFG_24C01_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_PAGE_SIZE(4) \
	MCFG_I2CMEM_DATA_SIZE(0x80)

#define MCFG_24C02_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_PAGE_SIZE(4) \
	MCFG_I2CMEM_DATA_SIZE(0x100)

#define MCFG_24C08_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_DATA_SIZE(0x400)

#define MCFG_24C16_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_PAGE_SIZE(8) \
	MCFG_I2CMEM_DATA_SIZE(0x800)

#define MCFG_24C16A_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_DATA_SIZE(0x800)

#define MCFG_24C64_ADD( _tag ) \
	MCFG_I2CMEM_ADD( _tag ) \
	MCFG_I2CMEM_PAGE_SIZE(8) \
	MCFG_I2CMEM_DATA_SIZE(0x2000)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i2cmem_device

class i2cmem_device :
	public device_t,
	public device_memory_interface,
	public device_nvram_interface
{
public:
	// construction/destruction
	i2cmem_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock );

	static void set_address(device_t &device, int address) { downcast<i2cmem_device &>(device).m_slave_address = address; }
	static void set_page_size(device_t &device, int page_size) { downcast<i2cmem_device &>(device).m_page_size = page_size; }
	static void set_data_size(device_t &device, int data_size) { downcast<i2cmem_device &>(device).m_data_size = data_size; }
	static void set_e0(device_t &device, int e0) { downcast<i2cmem_device &>(device).m_e0 = e0; }
	static void set_e1(device_t &device, int e1) { downcast<i2cmem_device &>(device).m_e1 = e1; }
	static void set_e2(device_t &device, int e2) { downcast<i2cmem_device &>(device).m_e2 = e2; }
	static void set_wc(device_t &device, int wc) { downcast<i2cmem_device &>(device).m_wc = wc; }

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( write_e0 );
	DECLARE_WRITE_LINE_MEMBER( write_e1 );
	DECLARE_WRITE_LINE_MEMBER( write_e2 );
	DECLARE_WRITE_LINE_MEMBER( write_sda );
	DECLARE_WRITE_LINE_MEMBER( write_scl );
	DECLARE_WRITE_LINE_MEMBER( write_wc );
	DECLARE_READ_LINE_MEMBER( read_sda );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config( address_spacenum spacenum = AS_0 ) const;

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read( emu_file &file );
	virtual void nvram_write( emu_file &file );

	// internal helpers
	int address_mask();
	int select_device();
	int data_offset();

	// device-specific configuration
	address_space_config m_space_config;

	// internal state
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
	dynamic_buffer m_page;
	int m_page_offset;
};


// device type definition
extern const device_type I2CMEM;

#endif  /* __I2CMEM_H__ */
