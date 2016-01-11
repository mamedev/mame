// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 ***********************************************************************************************************/


#include "emu.h"
#include "neogeo_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NEOGEO_CART_SLOT = &device_creator<neogeo_cart_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_cart_slot_device - constructor
//-------------------------------------------------
neogeo_cart_slot_device::neogeo_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
						device_t(mconfig, NEOGEO_CART_SLOT, "NeoGeo Cartridge Slot", tag, owner, clock, "neogeo_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_cart(nullptr)
{
}


//-------------------------------------------------
//  neogeo_cart_slot_device - destructor
//-------------------------------------------------

neogeo_cart_slot_device::~neogeo_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void neogeo_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_neogeo_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void neogeo_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool neogeo_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT16 *ROM;
		UINT8* ROM8;
		UINT32 len;

		if (software_entry() != nullptr)
		{
			// create memory regions
			len = get_software_region_length("maincpu");
			m_cart->rom_alloc(len); ROM = m_cart->get_rom_base();
			memcpy(ROM, get_software_region("maincpu"), len);

			len = get_software_region_length("fixed");
			m_cart->fixed_alloc(len);   ROM8 = m_cart->get_fixed_base();
			memcpy(ROM8, get_software_region("fixed"), len);

			if (get_software_region("audiocpu") != nullptr)
			{
				len = get_software_region_length("audiocpu");
				m_cart->audio_alloc(len + 0x10000); ROM8 = m_cart->get_audio_base();
				memcpy(ROM8, get_software_region("audiocpu"), len);
				memcpy(ROM8 + 0x10000, get_software_region("audiocpu"), len); // avoid reloading in XML, should just improve banking instead tho?
			}

			len = get_software_region_length("ymsnd");
			m_cart->ym_alloc(len);  ROM8 = m_cart->get_ym_base();
			memcpy(ROM8, get_software_region("ymsnd"), len);

			if (get_software_region("ymsnd.deltat") != nullptr)
			{
				len = get_software_region_length("ymsnd.deltat");
				m_cart->ymdelta_alloc(len); ROM8 = m_cart->get_ymdelta_base();
				memcpy(ROM8, get_software_region("ymsnd.deltat"), len);
			}
			else
			{
				// ensure there is no delta-t region
			}


			len = get_software_region_length("sprites");
			m_cart->sprites_alloc(len); ROM8 = m_cart->get_sprites_base();
			memcpy(ROM8, get_software_region("sprites"), len);

			if (get_software_region("audiocrypt") != nullptr)  // encrypted Z80 code
			{
				len = get_software_region_length("audiocrypt");
				m_cart->audiocrypt_alloc(len);  ROM8 = m_cart->get_audiocrypt_base();
				memcpy(ROM8, get_software_region("audiocrypt"), len);
				// allocate the audiocpu region to decrypt data into
				m_cart->audio_alloc(len + 0x10000);
			}

			m_cart->decrypt_all(
				(UINT8*)m_cart->get_rom_base(), m_cart->get_rom_size(),
				m_cart->get_sprites_base(), m_cart->get_sprites_size(),
				m_cart->get_fixed_base(), m_cart->get_fixed_size(),
				m_cart->get_ym_base(), m_cart->get_ym_size(),
				m_cart->get_ymdelta_base(), m_cart->get_ymdelta_size(),
				m_cart->get_audio_base(), m_cart->get_audio_size(),
				m_cart->get_audiocrypt_base(), m_cart->get_audiocrypt_size());


			// create optimized sprite cache
			m_cart->m_sprite_gfx_address_mask = neogeohelper_optimize_sprite_data(m_cart->get_sprites_optimized_arr(), m_cart->get_sprites_base(), m_cart->get_sprites_size());


			return IMAGE_INIT_PASS;
		}
	}

	return IMAGE_INIT_PASS;
}

void neogeo_cart_slot_device::setup_memory_banks(running_machine &machine)
{
	UINT16* base16 =  get_rom_base();
	UINT32 size = get_rom_size();
	machine.memory().region_free(":maincpu");
	machine.memory().region_alloc(":maincpu",size,2, ENDIANNESS_BIG);
	memcpy(memregion(":maincpu")->base(),(UINT8*)base16,size);

	UINT8* base =  get_audio_base();
	size = get_audio_size();
	machine.memory().region_free(":audiocpu");
	machine.memory().region_alloc(":audiocpu",size,1, ENDIANNESS_LITTLE);
	memcpy(memregion(":audiocpu")->base(),base,size);


	base =  get_ym_base();
	size = get_ym_size();

	machine.memory().region_free(":ymsnd");
	machine.memory().region_alloc(":ymsnd",size,1, ENDIANNESS_LITTLE);
	memcpy(memregion(":ymsnd")->base(),base,size);


	base = get_ymdelta_base();
	size = get_ymdelta_size();
	machine.memory().region_free(":ymsnd.deltat");

	if(base)
	{
		machine.memory().region_alloc(":ymsnd.deltat",size,1, ENDIANNESS_LITTLE);
		memcpy(memregion(":ymsnd.deltat")->base(),base,size);
	}


}

/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void neogeo_cart_slot_device::call_unload()
{
}

/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool neogeo_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string neogeo_cart_slot_device::get_default_card_software()
{
	return software_get_default_slot("rom");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset, mem_mask);
	else
		return 0xffff;
}
