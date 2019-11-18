// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 CMC42 & CMC50 encrypted cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "cmc.h"


//-------------------------------------------------
//  neogeo_cmc_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_CMC_CART, neogeo_cmc_cart_device, "neocart_cmc", "Neo Geo CMC Cart")


neogeo_cmc_cart_device::neogeo_cmc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_prot(*this, "cmc_prot")
{
}

neogeo_cmc_cart_device::neogeo_cmc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_cmc_cart_device::device_start()
{
}

void neogeo_cmc_cart_device::device_reset()
{
}


void neogeo_cmc_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_prot);
}


/*************************************************
 zupapa
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_ZUPAPA_CART, neogeo_cmc_zupapa_cart_device, "neocart_zupapa", "Neo Geo Zupapa CMC42 Cart")

neogeo_cmc_zupapa_cart_device::neogeo_cmc_zupapa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_ZUPAPA_CART, tag, owner, clock)
{
}

void neogeo_cmc_zupapa_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, ZUPAPA_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 mslug3h
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_MSLUG3H_CART, neogeo_cmc_mslug3h_cart_device, "neocart_mslug3h", "Neo Geo Metal Slug 3 AES CMC42 Cart")

neogeo_cmc_mslug3h_cart_device::neogeo_cmc_mslug3h_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_MSLUG3H_CART, tag, owner, clock)
{
}

void neogeo_cmc_mslug3h_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 ganryu
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_GANRYU_CART, neogeo_cmc_ganryu_cart_device, "neocart_ganryu", "Neo Geo Ganryu CMC42 Cart")

neogeo_cmc_ganryu_cart_device::neogeo_cmc_ganryu_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_GANRYU_CART, tag, owner, clock)
{
}

void neogeo_cmc_ganryu_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GANRYU_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 s1945p
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_S1945P_CART, neogeo_cmc_s1945p_cart_device, "neocart_s1945p", "Neo Geo Strikers 1945 Plus CMC42 Cart")

neogeo_cmc_s1945p_cart_device::neogeo_cmc_s1945p_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_S1945P_CART, tag, owner, clock)
{
}

void neogeo_cmc_s1945p_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, S1945P_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 preisle2
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_PREISLE2_CART, neogeo_cmc_preisle2_cart_device, "neocart_preisle2", "Neo Geo Prehistoric Isle 2 CMC42 Cart")

neogeo_cmc_preisle2_cart_device::neogeo_cmc_preisle2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_PREISLE2_CART, tag, owner, clock)
{
}

void neogeo_cmc_preisle2_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, PREISLE2_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 bangbead
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_BANGBEAD_CART, neogeo_cmc_bangbead_cart_device, "neocart_bangbead", "Neo Geo Bangbead CMC42 Cart")

neogeo_cmc_bangbead_cart_device::neogeo_cmc_bangbead_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_BANGBEAD_CART, tag, owner, clock)
{
}

void neogeo_cmc_bangbead_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, BANGBEAD_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 ntd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_NITD_CART, neogeo_cmc_nitd_cart_device, "neocart_nitd", "Neo Geo NITD CMC42 Cart")

neogeo_cmc_nitd_cart_device::neogeo_cmc_nitd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_NITD_CART, tag, owner, clock)
{
}

void neogeo_cmc_nitd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, NITD_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 sengoku3
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_SENGOKU3_CART, neogeo_cmc_sengoku3_cart_device, "neocart_sengoku3", "Neo Geo Sengoku 3 CMC42 Cart")

neogeo_cmc_sengoku3_cart_device::neogeo_cmc_sengoku3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_SENGOKU3_CART, tag, owner, clock)
{
}

void neogeo_cmc_sengoku3_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, SENGOKU3_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 kof99k
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_KOF99K_CART, neogeo_cmc_kof99k_cart_device, "neocart_kof99k", "Neo Geo KoF 99 Korea CMC42 Cart")

neogeo_cmc_kof99k_cart_device::neogeo_cmc_kof99k_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_KOF99K_CART, tag, owner, clock)
{
}

void neogeo_cmc_kof99k_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, KOF99_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 kof2001
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_KOF2001_CART, neogeo_cmc_kof2001_cart_device, "neocart_kof2001", "Neo Geo KoF 2001 CMC50 Cart")

neogeo_cmc_kof2001_cart_device::neogeo_cmc_kof2001_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_KOF2001_CART, tag, owner, clock)
{
}

void neogeo_cmc_kof2001_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2001_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 kof2000n
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_KOF2000N_CART, neogeo_cmc_kof2000n_cart_device, "neocart_kof2000n", "Neo Geo KoF 2000 CMC50 Cart")

neogeo_cmc_kof2000n_cart_device::neogeo_cmc_kof2000n_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_KOF2000N_CART, tag, owner, clock)
{
}

void neogeo_cmc_kof2000n_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2000_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 jockeygp
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CMC_JOCKEYGP_CART, neogeo_cmc_jockeygp_cart_device, "neocart_jockeygp", "Neo Geo Jockey GP CMC50 Cart")

neogeo_cmc_jockeygp_cart_device::neogeo_cmc_jockeygp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cmc_cart_device(mconfig, NEOGEO_CMC_JOCKEYGP_CART, tag, owner, clock),
	m_nvram(*this, "nvram")
{
}


void neogeo_cmc_jockeygp_cart_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(0x2000/2);
	m_nvram->set_base(m_ram.get(), 0x2000);
	save_pointer(NAME(m_ram), 0x2000/2);
}

void neogeo_cmc_jockeygp_cart_device::device_add_mconfig(machine_config &config)
{
	neogeo_cmc_cart_device::device_add_mconfig(config);
	NVRAM(config, m_nvram);
}

void neogeo_cmc_jockeygp_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, JOCKEYGP_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
