// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_ram.cpp

    Code for emulating the Disto RAM cartridge

    This cartridge came in several forms: 256K, 512K, 768K, and 1024K.

***************************************************************************/

#include "emu.h"
#include "coco_ram.h"
#include "cococart.h"
#include "machine/ram.h"

#define STATICRAM_TAG   "static_ram"


// #define VERBOSE (LOG_GENERAL )
#include "logmacro.h"

#define RAM_SIZE_IN_K 1024
#define BUFFER_SIZE (RAM_SIZE_IN_K * 1024)

#define STRINGIZE_HELPER(expr) #expr
#define STRINGIZE(expr) STRINGIZE_HELPER(expr)

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> coco_pak_device

	class coco_pak_ram_device :
			public device_t,
			public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
		virtual void device_add_mconfig(machine_config &config) override;

	protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void scs_write(offs_t offset, u8 data) override;
		virtual u8 scs_read(offs_t offset) override;

	private:
		required_device<ram_device>             m_staticram;
		int                                     m_offset;
	};
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_PAK_RAM, device_cococart_interface, coco_pak_ram_device, "cocopakram", "Disto " STRINGIZE(RAM_SIZE_IN_K) "K RAM Cartridge")



//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_ram_device::coco_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_PAK_RAM, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_staticram(*this, STATICRAM_TAG)
	, m_offset(0)
{
}



//**************************************************************************
//  MACHINE FRAGMENTS AND ADDRESS MAPS
//**************************************************************************

void coco_pak_ram_device::device_add_mconfig(machine_config &config)
{
	RAM(config, STATICRAM_TAG).set_default_size(STRINGIZE(RAM_SIZE_IN_K) "K").set_default_value(0);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_ram_device::device_start()
{
	// initial state
	m_offset = 0;

	// save state
	save_item(NAME(m_offset));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_pak_ram_device::device_reset()
{
	m_offset = 0;
}



//-------------------------------------------------
//    scs_write
//-------------------------------------------------

void coco_pak_ram_device::scs_write(offs_t offset, u8 data)
{
//  int idata = data;

	switch(offset)
	{
		case 0:
			m_offset = ((m_offset & 0xffff00) + data);
			break;
		case 1:
			m_offset = ((m_offset & 0xff00ff) + (data << 8));
			break;
		case 2:
			m_offset = ((m_offset & 0x00ffff) + (data << 16));
			break;
		case 3:
			if( m_offset < BUFFER_SIZE )
			{
				m_staticram->write(m_offset, data);
			}
			break;
	}

	LOG("scs_write: %s: %06x, %02x, %02x\n", machine().describe_context(), m_offset, offset, data);
}



//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_pak_ram_device::scs_read(offs_t offset)
{
	u8 data = 0x00;

	switch (offset)
	{
		case 0:
			data = (m_offset) & 0xff;
			break;
		case 1:
			data = (m_offset & 0xff00ff) >> 8;
			break;
		case 2:
			data = (m_offset & 0xff0000) >> 16;
			break;
		case 3:
			if( m_offset < BUFFER_SIZE )
			{
				data = m_staticram->read(m_offset);
			}

			break;
	}

	LOG("scs_read:  %s: %06x, %02x, %02x\n", machine().describe_context(), m_offset, offset, data);
	return data;
}
