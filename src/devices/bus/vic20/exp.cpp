// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_EXPANSION_SLOT, vic20_expansion_slot_device, "vic20_expansion_slot", "VIC-20 expansion port")



//**************************************************************************
//  DEVICE VIC20_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic20_expansion_card_interface - constructor
//-------------------------------------------------

device_vic20_expansion_card_interface::device_vic20_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_blk1(*this, "blk1"),
		m_blk2(*this, "blk2"),
		m_blk3(*this, "blk3"),
		m_blk5(*this, "blk5"),
		m_nvram(*this, "nvram")
{
	m_slot = dynamic_cast<vic20_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vic20_expansion_card_interface - destructor
//-------------------------------------------------

device_vic20_expansion_card_interface::~device_vic20_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_expansion_slot_device - constructor
//-------------------------------------------------

vic20_expansion_slot_device::vic20_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC20_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_res(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_validity_check -
//-------------------------------------------------

void vic20_expansion_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<device_vic20_expansion_card_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement device_vic20_expansion_card_interface\n", carddev->tag(), carddev->name());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_expansion_slot_device::device_start()
{
	device_t *const carddev = get_card_device();
	m_card = dynamic_cast<device_vic20_expansion_card_interface *>(carddev);
	if (carddev && !m_card)
		fatalerror("Card device %s (%s) does not implement device_vic20_expansion_card_interface\n", carddev->tag(), carddev->name());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_res.resolve_safe();

	// inherit bus clock
	if (clock() == 0)
	{
		vic20_expansion_slot_device *root = machine().device<vic20_expansion_slot_device>(VIC20_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result vic20_expansion_slot_device::call_load()
{
	if (m_card)
	{
		if (!loaded_through_softlist())
		{
			if (is_filetype("20")) fread(m_card->m_blk1, 0x2000);
			else if (is_filetype("40")) fread(m_card->m_blk2, 0x2000);
			else if (is_filetype("60")) fread(m_card->m_blk3, 0x2000);
			else if (is_filetype("70")) fread(m_card->m_blk3, 0x2000, 0x1000);
			else if (is_filetype("a0")) fread(m_card->m_blk5, 0x2000);
			else if (is_filetype("b0")) fread(m_card->m_blk5, 0x2000, 0x1000);
			else if (is_filetype("crt"))
			{
				// read the header
				uint8_t header[2];
				fread(&header, 2);
				uint16_t address = (header[1] << 8) | header[0];

				switch (address)
				{
				case 0x2000: fread(m_card->m_blk1, 0x2000); break;
				case 0x4000: fread(m_card->m_blk2, 0x2000); break;
				case 0x6000: fread(m_card->m_blk3, 0x2000); break;
				case 0x7000: fread(m_card->m_blk3, 0x2000, 0x1000); break;
				case 0xa000: fread(m_card->m_blk5, 0x2000); break;
				case 0xb000: fread(m_card->m_blk5, 0x2000, 0x1000); break;
				default: return image_init_result::FAIL;
				}
			}
		}
		else
		{
			load_software_region("blk1", m_card->m_blk1);
			load_software_region("blk2", m_card->m_blk2);
			load_software_region("blk3", m_card->m_blk3);
			load_software_region("blk5", m_card->m_blk5);
		}
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string vic20_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic20_expansion_slot_device::cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (m_card != nullptr)
	{
		data = m_card->vic20_cd_r(offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void vic20_expansion_slot_device::cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (m_card != nullptr)
	{
		m_card->vic20_cd_w(offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}
}

void vic20_expansion_slot_device::add_passthrough(machine_config &config, const char *_tag)
{
	vic20_expansion_slot_device &slot(VIC20_EXPANSION_SLOT(config, _tag, DERIVED_CLOCK(1, 1), vic20_expansion_cards, nullptr));
	slot.irq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::irq_w));
	slot.nmi_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::nmi_w));
	slot.res_wr_callback().set(DEVICE_SELF_OWNER, FUNC(vic20_expansion_slot_device::res_w));
}


//-------------------------------------------------
//  SLOT_INTERFACE( vic20_expansion_cards )
//-------------------------------------------------

// slot devices
#include "fe3.h"
#include "megacart.h"
#include "std.h"
#include "vic1010.h"
#include "vic1110.h"
#include "vic1111.h"
#include "vic1112.h"
#include "vic1210.h"
#include "videopak.h"
#include "speakeasy.h"

void vic20_expansion_cards(device_slot_interface &device)
{
	device.option_add("exp", VIC1010);
	device.option_add("3k", VIC1210);
	device.option_add("8k", VIC1110);
	device.option_add("16k", VIC1111);
	device.option_add("fe3", VIC20_FE3);
	device.option_add("speakez", VIC20_SPEAKEASY);
	device.option_add("videopak", VIC20_VIDEO_PAK);

	// the following need ROMs from the software list
	device.option_add_internal("standard", VIC20_STD);
	device.option_add_internal("ieee488", VIC1112);
	device.option_add_internal("megacart", VIC20_MEGACART);
}
