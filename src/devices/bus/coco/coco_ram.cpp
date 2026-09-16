// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_ram.cpp

    Code for emulating the Disto RAM cartridge

    This cartridge came in several forms: 256K, 512K, 768K, and 1024K.
    It also includes a mini expansion bus.

***************************************************************************/

#include "emu.h"
#include "coco_ram.h"
#include "meb_intrf.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define RAM_SIZE 1024 * 1024
#define MEB_TAG "meb"


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> coco_pak_ram_device

	class coco_pak_ram_device
		: public device_t
		, public device_cococart_interface
	{
		public:
			// construction/destruction
			coco_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
			virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		protected:
			// device-level overrides
			virtual void device_start() override ATTR_COLD;
			virtual void device_reset() override ATTR_COLD;
			virtual void scs_write(offs_t offset, u8 data) override;
			virtual u8 scs_read(offs_t offset) override;
			required_device<distomeb_slot_device> m_slot;

		private:
			std::unique_ptr<uint8_t[]> m_staticram;
			u32 m_offset;
	};
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_PAK_RAM, device_cococart_interface, coco_pak_ram_device, "cocopakram", "Disto RAM Cartridge")



//-------------------------------------------------
//  coco_pak_ram_device - constructor
//-------------------------------------------------

coco_pak_ram_device::coco_pak_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_PAK_RAM, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_slot(*this, MEB_TAG)
{
}



//**************************************************************************
//  MACHINE FRAGMENTS AND ADDRESS MAPS
//**************************************************************************

static void disto_meb_slot(device_slot_interface &device)
{
	disto_meb_add_basic_devices(device);
}

void coco_pak_ram_device::device_add_mconfig(machine_config &config)
{
	DISTOMEB_SLOT(config, m_slot, DERIVED_CLOCK(1, 1), disto_meb_slot, "rtime");
	m_slot->cart_callback().set([this](int state) { set_line_value(line::CART, state); });
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_ram_device::device_start()
{
	// initial state
	m_offset = 0;
	m_staticram = std::make_unique<uint8_t[]>(RAM_SIZE);

	// save state
	save_item(NAME(m_offset));
	save_pointer(NAME(m_staticram), RAM_SIZE);
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
	if (offset > 0x0f && offset < 0x18)
		m_slot->meb_write(offset - 0x10, data);
	else
	{
		switch (offset)
		{
			case 0:
				m_offset = (m_offset & 0xffff00) | u32(data);
				break;
			case 1:
				m_offset = (m_offset & 0xff00ff) | (u32(data) << 8);
				break;
			case 2:
				m_offset = (m_offset & 0x00ffff) | (u32(data) << 16);
				break;
			case 3:
				if (m_offset < RAM_SIZE)
				{
					m_staticram[m_offset] = data;
				}
				break;
		}

		LOG("scs_write: %s: %06x, %02x, %02x\n", machine().describe_context(), m_offset, offset, data);
	}
}



//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_pak_ram_device::scs_read(offs_t offset)
{
	u8 data = 0x00;

	if (offset > 0x0f && offset < 0x18)
	{
		data =  m_slot->meb_read(offset - 0x10);
	}
	else
	{
		switch (offset)
		{
			case 0:
				data = u8(m_offset & 0x00ff);
				break;
			case 1:
				data = u8((m_offset >> 8) & 0x00ff);
				break;
			case 2:
				data = u8((m_offset >> 16) & 0x00ff);
				break;
			case 3:
				if (m_offset < RAM_SIZE)
				{
					data = m_staticram[m_offset];
				}
				break;
		}

		LOG("scs_read:\t%s: %06x, %02x, %02x\n", machine().describe_context(), m_offset, offset, data);
	}

	return data;
}
