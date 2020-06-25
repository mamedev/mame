// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 SNK Neo Geo cartslot emulation

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NEOGEO_CART_SLOT, neogeo_cart_slot_device, "neogeo_cart_slot", "Neo Geo Cartridge Slot")


//-------------------------------------------------
//  device_neogeo_cart_interface - constructor
//-------------------------------------------------

device_neogeo_cart_interface::device_neogeo_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "neogeocart"),
	m_region_rom(*this, "^maincpu"),
	m_region_fixed(*this, "^fixed"),
	m_region_audio(*this, "^audiocpu"),
	m_region_audiocrypt(*this, "^audiocrypt"),
	m_region_spr(*this, "^sprites"),
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

uint32_t device_neogeo_cart_interface::get_region_mask(uint8_t* rgn, uint32_t rgn_size)
{
	// get mask based on the length rounded up to the nearest power of 2
	uint32_t mask = 0xffffffff;
	uint32_t len = rgn_size;

	for (uint32_t bit = 0x80000000; bit != 0; bit >>= 1)
	{
		if ((len * 2 - 1) & bit)
			break;

		mask >>= 1;
	}

	return mask;
}

void device_neogeo_cart_interface::optimize_sprites(uint8_t* region_sprites, uint32_t region_sprites_size)
{
	// convert the sprite graphics data into a format that allows faster blitting
	uint32_t spritegfx_address_mask = get_region_mask(region_sprites, region_sprites_size);
	uint8_t *src = region_sprites;

	m_sprites_opt.resize(spritegfx_address_mask + 1);
	uint8_t *dest = &m_sprites_opt[0];

	for (unsigned i = 0; i < region_sprites_size; i += 0x80, src += 0x80)
	{
		for (unsigned y = 0; y < 0x10; y++)
		{
			for (unsigned x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x43 | (y << 2)] >> x) & 0x01) << 3) |
				(((src[0x41 | (y << 2)] >> x) & 0x01) << 2) |
				(((src[0x42 | (y << 2)] >> x) & 0x01) << 1) |
				(((src[0x40 | (y << 2)] >> x) & 0x01) << 0);
			}

			for (unsigned x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x03 | (y << 2)] >> x) & 0x01) << 3) |
				(((src[0x01 | (y << 2)] >> x) & 0x01) << 2) |
				(((src[0x02 | (y << 2)] >> x) & 0x01) << 1) |
				(((src[0x00 | (y << 2)] >> x) & 0x01) << 0);
			}
		}
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  neogeo_cart_slot_device - constructor
//-------------------------------------------------
neogeo_cart_slot_device::neogeo_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	device_t(mconfig, NEOGEO_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_neogeo_cart_interface>(mconfig, *this),
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
	{ NEOGEO_MSLUG3A, "sma_mslug3a" },
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
	{ NEOGEO_MSLUG5B, "boot_mslug5b" },
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


image_init_result neogeo_cart_slot_device::call_load()
{
	if (m_cart)
	{
		if (loaded_through_softlist())
		{
			uint16_t *ROM16;
			uint8_t *ROM8;
			uint32_t len;

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
				(uint8_t*)m_cart->get_rom_base(), m_cart->get_rom_size(),
				m_cart->get_sprites_base(), m_cart->get_sprites_size(),
				m_cart->get_fixed_base(), m_cart->get_fixed_size(),
				m_cart->get_ym_base(), m_cart->get_ym_size(),
				m_cart->get_ymdelta_base(), m_cart->get_ymdelta_size(),
				m_cart->get_audio_base(), m_cart->get_audio_size(),
				m_cart->get_audiocrypt_base(), m_cart->get_audiocrypt_size());

			// SPEED UP WORKAROUND: to speed up sprite drawing routine, let us store the sprite data in
			// a different format (we then always access such alt format for drawing)
			m_cart->optimize_sprites(m_cart->get_sprites_base(), m_cart->get_sprites_size());

			return image_init_result::PASS;
		}
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void neogeo_cart_slot_device::call_unload()
{
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string neogeo_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}

/*-------------------------------------------------
 rom_r
 -------------------------------------------------*/

uint16_t neogeo_cart_slot_device::rom_r(offs_t offset)
{
	if (m_cart)
		return m_cart->rom_r(offset);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_banksel
 -------------------------------------------------*/

void neogeo_cart_slot_device::banksel_w(uint16_t data)
{
	if (m_cart)
		m_cart->banksel_w(data);
}

/*-------------------------------------------------
 ram_r
 -------------------------------------------------*/

uint16_t neogeo_cart_slot_device::ram_r(offs_t offset)
{
	if (m_cart)
		return m_cart->ram_r(offset);
	else
		return 0xffff;
}

/*-------------------------------------------------
 ram_w
 -------------------------------------------------*/

void neogeo_cart_slot_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cart)
		m_cart->ram_w(offset, data, mem_mask);
}

/*-------------------------------------------------
 addon_r
 -------------------------------------------------*/

uint16_t neogeo_cart_slot_device::addon_r(offs_t offset)
{
	if (m_cart)
		return m_cart->addon_r(offset);
	else
		return 0xffff;
}

/*-------------------------------------------------
 protection_r
 -------------------------------------------------*/

uint16_t neogeo_cart_slot_device::protection_r(address_space &space, offs_t offset)
{
	if (m_cart)
		return m_cart->protection_r(space, offset);
	else
		return 0xffff;
}

/*-------------------------------------------------
 protection_w
 -------------------------------------------------*/

void neogeo_cart_slot_device::protection_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cart)
		m_cart->protection_w(offset, data, mem_mask);
}
