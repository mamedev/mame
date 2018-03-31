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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I2CMEM_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, I2CMEM, 0 )

#define MCFG_I2CMEM_ADDRESS( address ) \
	downcast<i2cmem_device &>(*device).set_address(address);
#define MCFG_I2CMEM_PAGE_SIZE( page_size ) \
	downcast<i2cmem_device &>(*device).set_page_size(page_size);
#define MCFG_I2CMEM_DATA_SIZE(data_size) \
	downcast<i2cmem_device &>(*device).set_data_size(data_size);
#define MCFG_I2CMEM_E0(e0) \
	downcast<i2cmem_device &>(*device).set_e0(e0);
#define MCFG_I2CMEM_E1(e1) \
	downcast<i2cmem_device &>(*device).set_e1(e1);
#define MCFG_I2CMEM_E2(e2) \
	downcast<i2cmem_device &>(*device).set_e2(e2);
#define MCFG_I2CMEM_WC(wc) \
	downcast<i2cmem_device &>(*device).set_wc(wc);

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
	public device_nvram_interface
{
public:
	// construction/destruction
	i2cmem_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock );

	void set_address(int address) { m_slave_address = address; }
	void set_page_size(int page_size) { m_page_size = page_size; }
	void set_data_size(int data_size) { m_data_size = data_size; }
	void set_e0(int e0) { m_e0 = e0; }
	void set_e1(int e1) { m_e1 = e1; }
	void set_e2(int e2) { m_e2 = e2; }
	void set_wc(int wc) { m_wc = wc; }

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


// device type definition
DECLARE_DEVICE_TYPE(I2CMEM, i2cmem_device)

#endif // MAME_MACHINE_I2CMEM_H
