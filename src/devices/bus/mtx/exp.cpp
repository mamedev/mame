// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MTX expansion emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_EXP_SLOT, mtx_exp_slot_device, "mtx_exp_slot", "MTX expansion slot")


//**************************************************************************
//  DEVICE MTX_BUS PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mtx_exp_interface - constructor
//-------------------------------------------------

device_mtx_exp_interface::device_mtx_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "mtxexp")
	, m_rom(nullptr)
	, m_rom_size(0)
{
	m_slot = dynamic_cast<mtx_exp_slot_device *>(device.owner());
}


//-------------------------------------------------
//  rom_alloc - alloc the space for the ROM
//-------------------------------------------------

void device_mtx_exp_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(":cart:rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_exp_slot_device - constructor
//-------------------------------------------------

mtx_exp_slot_device::mtx_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MTX_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_mtx_exp_interface>(mconfig, *this)
	, device_cartrom_image_interface(mconfig, *this)
	, m_program(*this, finder_base::DUMMY_TAG, -1)
	, m_io(*this, finder_base::DUMMY_TAG, -1)
	, m_busreq_handler(*this)
	, m_int_handler(*this)
	, m_nmi_handler(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_exp_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_busreq_handler.resolve_safe();
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result mtx_exp_slot_device::call_load()
{
	if (m_card)
	{
		uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		if (size % 0x2000)
		{
			seterror(image_error::INVALIDIMAGE, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

		m_card->rom_alloc(size, tag());

		if (!loaded_through_softlist())
			fread(m_card->get_rom_base(), size);
		else
			memcpy(m_card->get_rom_base(), get_software_region("rom"), size);
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string mtx_exp_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return

		if (len == 0x80000)
			return software_get_default_slot("magrom");
	}

	return software_get_default_slot("rompak");
}


void mtx_exp_slot_device::bankswitch(uint8_t data)
{
	if (m_card)
		m_card->bankswitch(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( mtx_exp_devices )
//-------------------------------------------------


// slot devices
#include "cfx.h"
//#include "fdx.h"
#include "magrom.h"
#include "rompak.h"
#include "sdx.h"


void mtx_int_expansion_devices(device_slot_interface &device)
{
	device.option_add("cfx", MTX_CFX);
	//device.option_add("fdx", MTX_FDX);         /* FDX Floppy Disc System */
	device.option_add_internal("magrom", MTX_MAGROM);
	device.option_add("sdxcpm", MTX_SDXCPM);   /* SDX Floppy Disc System (CP/M ROM and 80 column card) */
}

void mtx_ext_expansion_devices(device_slot_interface &device)
{
	device.option_add("cfx", MTX_CFX);
	device.option_add_internal("magrom", MTX_MAGROM);
	device.option_add_internal("rompak", MTX_ROMPAK);
	device.option_add("sdxbas", MTX_SDXBAS);   /* SDX Floppy Disc System (SDX ROM)*/
}
