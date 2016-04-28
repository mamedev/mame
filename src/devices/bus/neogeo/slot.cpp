// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 SNK Neo Geo cartslot emulation
 
 ***********************************************************************************************************/

#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NEOGEO_CART_SLOT = &device_creator<neogeo_cart_slot_device>;


//-------------------------------------------------
//  device_neogeo_cart_interface - constructor
//-------------------------------------------------

device_neogeo_cart_interface::device_neogeo_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	m_sprite_gfx_address_mask(0),
	m_region_rom(*this, "^maincpu"),
	m_region_fixed(*this, "^fixed"),
	m_region_audio(*this, "^audiocpu"),
	m_region_audiocrypt(*this, "^audiocrypt"),
	m_region_spr(*this, "^sprites"),
	m_region_spr_opt(*this, "^spr_opt"),
	m_region_ym(*this, "^ymsnd"),
	m_region_ymd(*this, "^ymsnd.deltat")
{
}

//-------------------------------------------------
//  ~device_neogeo_cart_interface - destructor
//-------------------------------------------------

device_neogeo_cart_interface::~device_neogeo_cart_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_cart_slot_device - constructor
//-------------------------------------------------
neogeo_cart_slot_device::neogeo_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
						device_t(mconfig, NEOGEO_CART_SLOT, "Neo Geo Cartridge Slot", tag, owner, clock, "neogeo_cart_slot", __FILE__),
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


//-------------------------------------------------
//  NEOGEO PCB
//-------------------------------------------------

struct neogeo_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const neogeo_slot slot_list[] =
{
	{ NEOGEO_STD, "rom" },
	{ NEOGEO_VLINER, "rom_vliner" },
	{ NEOGEO_FATFURY2, "rom_fatfur2" },
	{ NEOGEO_KOF98, "rom_kof98" },
	{ NEOGEO_MSLUGX, "rom_mslugx" },
	{ NEOGEO_ZUPAPA, "cmc42_zupapa" },
	{ NEOGEO_MSLUG3H, "cmc42_mslug3h" },
	{ NEOGEO_GANRYU, "cmc42_ganryu" },
	{ NEOGEO_S1945P, "cmc42_s1945p" },
	{ NEOGEO_PREIS2, "cmc42_preisle2" },
	{ NEOGEO_BANGBD, "cmc42_bangbead" },
	{ NEOGEO_NITD, "cmc42_nitd" },
	{ NEOGEO_SENGOK3, "cmc42_sengoku3" },
	{ NEOGEO_KOF99K, "cmc42_kof99k" },
	{ NEOGEO_KOF2K1, "cmc50_kof2001" },
	{ NEOGEO_KOF2KN, "cmc50_kof2000n" },
	{ NEOGEO_JOCKEYGP, "cmc50_jockeygp" },
	{ NEOGEO_KOF99, "sma_kof99" },
	{ NEOGEO_GAROU, "sma_garou" },
	{ NEOGEO_GAROUH, "sma_garouh" },
	{ NEOGEO_MSLUG3, "sma_mslug3" },
	{ NEOGEO_KOF2K, "sma_kof2k" },
	{ NEOGEO_MSLUG4, "pcm2_mslug4" },
	{ NEOGEO_MSLUG4P, "pcm2_ms4p" },
	{ NEOGEO_ROTD, "pcm2_rotd" },
	{ NEOGEO_PNYAA, "pcm2_pnyaa" },
	{ NEOGEO_KOF2K2, "k2k2_kof2k2" },
	{ NEOGEO_MATRIM, "k2k2_matrim" },
	{ NEOGEO_SAMSHO5, "k2k2_samsh5" },
	{ NEOGEO_SAMSHO5S, "k2k2_sams5s" },
	{ NEOGEO_KOF2K2P, "k2k2_kf2k2p" },
	{ NEOGEO_MSLUG5, "pvc_mslug5" },
	{ NEOGEO_SVC, "pvc_svc" },
	{ NEOGEO_KOF2K3, "pvc_kf2k3" },
	{ NEOGEO_KOF2K3H, "pvc_kf2k3h" },
	{ NEOGEO_CTHD2K3, "boot_cthd2k3" },
	{ NEOGEO_CT2K3SP, "boot_ct2k3sp" },
	{ NEOGEO_CT2K3SA, "boot_ct2k3sa" },
	{ NEOGEO_MATRIMBL, "boot_matrimbl" },
	{ NEOGEO_SVCBOOT, "boot_svcboot" },
	{ NEOGEO_SVCPLUS, "boot_svcplus" },
	{ NEOGEO_SVCPLUSA, "boot_svcplusa" },
	{ NEOGEO_SVCSPLUS, "boot_svcsplus" },
	{ NEOGEO_KOF2K2B, "boot_kf2k2b" },
	{ NEOGEO_KOF2K2MP, "boot_kf2k2mp" },
	{ NEOGEO_KOF2K2MP2, "boot_kf2k2mp2" },
	{ NEOGEO_KOF2K3B, "boot_kf2k3bl" },
	{ NEOGEO_KOF2K3P, "boot_kf2k3pl" },
	{ NEOGEO_KOF2K3UP, "boot_kf2k3upl" },
	{ NEOGEO_GAROUBL, "boot_garoubl" },
	{ NEOGEO_KOF97ORO, "boot_kof97oro" },
	{ NEOGEO_KF10THEP, "boot_kf10thep" },
	{ NEOGEO_KF2K5UNI, "boot_kf2k5uni" },
	{ NEOGEO_KF2K4SE, "boot_kf2k4se" },
	{ NEOGEO_LANS2K4, "boot_lans2004" },
	{ NEOGEO_SAMSHO5B, "boot_samsho5b" },
	{ NEOGEO_MSLUG3B6, "boot_mslug3b6" },
	{ NEOGEO_MSLUG5P, "boot_ms5plus" },
	{ NEOGEO_KOG, "boot_kog" },
	{ NEOGEO_SBP, "boot_sbp" },
	{ NEOGEO_KOF10TH, "boot_kf10th" },
};


static int neogeo_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}
	
	return 0;
}


void neogeo_cart_slot_device::set_cart_type(const char *slot)
{
	m_type = neogeo_get_pcb_id(slot);
}

/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool neogeo_cart_slot_device::call_load()
{
	if (m_cart)
	{
		if (software_entry() != nullptr)
		{
			UINT16 *ROM16;
			UINT8 *ROM8;
			UINT32 len;

			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				set_cart_type(pcb_name);

			// create memory regions
			len = get_software_region_length("maincpu");
			m_cart->rom_alloc(len);
			ROM16 = m_cart->get_rom_base();
			memcpy(ROM16, get_software_region("maincpu"), len);

			len = get_software_region_length("fixed");
			m_cart->fixed_alloc(len);
			ROM8 = m_cart->get_fixed_base();
			memcpy(ROM8, get_software_region("fixed"), len);

			if (get_software_region("audiocpu") != nullptr)
			{
				len = get_software_region_length("audiocpu");
				m_cart->audio_alloc(len + 0x10000);
				ROM8 = m_cart->get_audio_base();
				memcpy(ROM8, get_software_region("audiocpu"), len);
				memcpy(ROM8 + 0x10000, get_software_region("audiocpu"), len); // avoid reloading in XML, should just improve banking instead tho?
			}

			len = get_software_region_length("ymsnd");
			m_cart->ym_alloc(len);
			ROM8 = m_cart->get_ym_base();
			memcpy(ROM8, get_software_region("ymsnd"), len);

			if (get_software_region("ymsnd.deltat") != nullptr)
			{
				len = get_software_region_length("ymsnd.deltat");
				m_cart->ymdelta_alloc(len);
				ROM8 = m_cart->get_ymdelta_base();
				memcpy(ROM8, get_software_region("ymsnd.deltat"), len);
			}
			else
			{
				// ensure there is no delta-t region
			}


			len = get_software_region_length("sprites");
			m_cart->sprites_alloc(len);
			ROM8 = m_cart->get_sprites_base();
			memcpy(ROM8, get_software_region("sprites"), len);

			if (get_software_region("audiocrypt") != nullptr)  // encrypted Z80 code
			{
				len = get_software_region_length("audiocrypt");
				m_cart->audiocrypt_alloc(len);
				ROM8 = m_cart->get_audiocrypt_base();
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
			
			// create optimized sprite cache  [NOT GOOD FOR NOSLOT!!!!]
			m_cart->m_sprite_gfx_address_mask = neogeohelper_optimize_sprite_data(m_cart->get_sprites_optimized_arr(), m_cart->get_sprites_base(), m_cart->get_sprites_size());

			return IMAGE_INIT_PASS;
		}
	}

	return IMAGE_INIT_PASS;
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
 rom_r
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cart_slot_device::rom_r)
{
	if (m_cart)
		return m_cart->rom_r(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_banksel
 -------------------------------------------------*/

WRITE16_MEMBER(neogeo_cart_slot_device::banksel_w)
{
	if (m_cart)
		m_cart->banksel_w(space, offset, data, mem_mask);
}

/*-------------------------------------------------
 ram_r
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cart_slot_device::ram_r)
{
	if (m_cart)
		return m_cart->ram_r(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 ram_w
 -------------------------------------------------*/

WRITE16_MEMBER(neogeo_cart_slot_device::ram_w)
{
	if (m_cart)
		m_cart->ram_w(space, offset, data, mem_mask);
}

/*-------------------------------------------------
 addon_r
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cart_slot_device::addon_r)
{
	if (m_cart)
		return m_cart->addon_r(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 protection_r
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cart_slot_device::protection_r)
{
	if (m_cart)
		return m_cart->protection_r(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 protection_w
 -------------------------------------------------*/

WRITE16_MEMBER(neogeo_cart_slot_device::protection_w)
{
	if (m_cart)
		m_cart->protection_w(space, offset, data, mem_mask);
}
