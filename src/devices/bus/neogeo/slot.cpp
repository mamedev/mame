// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 SNK Neo Geo cartslot emulation

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//
// fullset Project Neon
//
#include "fxpt_atan2.cpp"
#include "neo_rand.c"

//
//
//

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
	m_region_ym(*this, "^ymsnd:adpcma"),
	m_region_ymd(*this, "^ymsnd:adpcmb")
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
	device_cartrom_image_interface(mconfig, *this),
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


	//
	// fullset Project Neon custom cart type
	//
	{ NEOGEO_NEON, "fsng1_neon" },
};


static int neogeo_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
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

std::pair<std::error_condition, std::string> neogeo_cart_slot_device::call_load()
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

			len = get_software_region_length("ymsnd:adpcma");
			m_cart->ym_alloc(len);
			ROM8 = m_cart->get_ym_base();
			memcpy(ROM8, get_software_region("ymsnd:adpcma"), len);

			if (get_software_region("ymsnd:adpcmb") != nullptr)
			{
				len = get_software_region_length("ymsnd:adpcmb");
				m_cart->ymdelta_alloc(len);
				ROM8 = m_cart->get_ymdelta_base();
				memcpy(ROM8, get_software_region("ymsnd:adpcmb"), len);
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
		}
	}

	return std::make_pair(std::error_condition(), std::string());
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

//
// fullset Project Neon r/w port handlers
//
typedef enum {
    NEON_WIFI_DISCONNECTED = 0,
    NEON_WIFI_CONNECTING = 1,
    NEON_WIFI_CONNECTED = 2
} neon_wifi_status_t;

// Command codes:
#define CART_CMD_SMUL	0x01
#define CART_CMD_ATAN2	0x02

#define CART_CMD_SAVE	0x03
#define CART_CMD_LOAD	0x04

#define CART_CMD_SRAND	0x05
#define CART_CMD_RAND	0x06

#define CART_CMD_WIFI_STATUS	0x20
#define CART_CMD_WIFI_REFRESH	0x21
#define CART_CMD_WIFI_GET_NAME	0x22
#define CART_CMD_WIFI_CONNECT	0x23

#define CART_CMD_TEST	0xAA
#define CART_CMD_HRESET	0xFF

// Reply codes for CART_CMD_SAVE:
#define CART_SAVE_OK    0x0000
#define CART_SAVE_ERR   0x0001
#define CART_SAVE_NONE  0x0010

// Reply codes for CART_CMD_WIFI_GET_NAME:
#define CART_WIFI_OK    0x0000
#define CART_WIFI_INVALID   0xFFFF

#define NEON_FPGA_RAM_SIZE 40	// In words
#define NEON_RAM_DATA_MAX (NEON_FPGA_RAM_SIZE - 1)	// First word is always used by echo
#define NEON_MAX_SAVES 10
#define NEON_SAVE_RAM_SIZE 64 // in bytes

uint16_t neon_fpga_ver = 0x01;
uint16_t neon_mcu_id = 0x0A;
uint16_t neon_mcu_busy = 0x00;
uint16_t neon_bank = 0;				// 0~F
uint16_t neon_fix_bank = 0;			// 0~3
uint16_t neon_adpcm_bank = 0;		// 0~7
uint16_t neon_game_to_mcu_ram[NEON_FPGA_RAM_SIZE];
uint16_t neon_mcu_to_game_ram[NEON_FPGA_RAM_SIZE];
uint32_t neon_sw_rng_next = 1;
bool neon_debug_enabled = false;	// MAME-only debug
neon_wifi_status_t neon_status_wifi = NEON_WIFI_DISCONNECTED;

typedef struct {
	char name[40]; 		//max len of ssid is 32, but adding space for goofyness
	char password[70]; 	//max len of password is 64
} accessPoint_t;

accessPoint_t accessPoints[50]; //max 50 in csv for testing
uint16_t accessPointsCount;

uint16_t neon_save_ram(uint16_t save_number) {
    uint16_t result = CART_SAVE_OK; //CART_SAVE_ERR

    std::string filename = string_format("nvram%sneon_save0%d.bin", PATH_SEPARATOR, save_number);
    const char* filenamePointer = filename.c_str(); 
    
	if (neon_debug_enabled) {
		printf("Saving ram file :%s\n", filename.c_str());
	}

    FILE *file_ptr;
    file_ptr = fopen(filenamePointer, "wb");

	if (file_ptr == NULL) {
		if (neon_debug_enabled) {
			printf("Could not open ram file: %s\n", filename.c_str());
		}
		return CART_SAVE_ERR;
	}

    uint16_t saveResult = fwrite(&neon_game_to_mcu_ram[2], NEON_SAVE_RAM_SIZE, 1, file_ptr);

    fclose(file_ptr);

	if (saveResult != 1) {
		result = CART_SAVE_ERR;
		if (neon_debug_enabled) {
			printf("Writing ram file failed with result: %d\n", saveResult);
		}
	} else {
		if (neon_debug_enabled) {
			printf("Save file completed successfuly\n");
		}
	}

    return result;
}

uint16_t neon_load_ram(uint16_t save_number) {
    uint16_t result = CART_SAVE_OK; //CART_SAVE_ERR

    std::string filename = string_format("nvram%sneon_save0%d.bin", PATH_SEPARATOR, save_number);
    const char* filenamePointer = filename.c_str(); 
    
	if (neon_debug_enabled) {
		printf("Loading ram file :%s\n", filename.c_str());
	}

    FILE *file_ptr;
    file_ptr = fopen(filenamePointer, "rb");

	if (file_ptr == NULL) {
		if (neon_debug_enabled) {
			printf("Could not open ram file :%s\n", filename.c_str());
		}
		return CART_SAVE_ERR;
	}

	size_t loadResult = fread(&neon_mcu_to_game_ram[2], 1, NEON_SAVE_RAM_SIZE, file_ptr);

	result = loadResult == NEON_SAVE_RAM_SIZE ? CART_SAVE_OK : CART_SAVE_ERR;

	fclose(file_ptr);

	if (result == CART_SAVE_ERR) {
		if (neon_debug_enabled) {
			printf("Loading ram file failed with result: %lld\n", loadResult);
		}
	} else {
		if (neon_debug_enabled) {
			printf("Loading ram file completed successfuly\n");
		}
	}

    return result;
}

uint16_t neon_load_wifi_list() {
    size_t count = 0;

    std::string filename = string_format("nvram%sneon_wifi_list.csv", PATH_SEPARATOR);
    const char* filenamePointer = filename.c_str(); 

    FILE *file_ptr;
    file_ptr = fopen(filenamePointer, "r");

    if (file_ptr == NULL) {
		if (neon_debug_enabled) {
			printf("Could not open ram file :%s\n", filename.c_str());
		}
        return 0;
    }

    while (fscanf(file_ptr, " %[^,],%[^\n]", accessPoints[count].name, accessPoints[count].password) == 2) {
        count++;
		
		if (count > 50) {
			break;
		}
    }

	if (neon_debug_enabled) {
		printf("WIFI Networks\n");
		printf("--------------------------------\n");
		for (uint16_t i = 0; i < count; i++) {
			printf("Name :%s\n", accessPoints[i].name);
			printf("Pass :%s\n", accessPoints[i].password);
		}
		printf("--------------------------------\n");
	}

    fclose(file_ptr);

	return count;
}

void neogeo_cart_slot_device::neon_enable_debug_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	neon_debug_enabled = data != 0x00;
	
	if (neon_debug_enabled) {
		printf("neon mcu debug enabled\n");
	}
}

void neogeo_cart_slot_device::neon_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	// TODO: Implement real P ROM banks - Project Neon uses none
	neon_bank = data & 0x0f;
	if (neon_debug_enabled) {
		if (neon_bank > 7)
			printf("neon_bank_w: FPGA RAM\n");
		else if (!neon_bank)
			printf("neon_bank_w: ROM\n");
		else
			printf("neon_bank_w(%d) ** not implemented in mame **\n", data);
	}
}

uint16_t neogeo_cart_slot_device::neon_mcu_status_r(offs_t offset) {
	// TODO: Implement real P ROM banks - Project Neon uses none
	u16 result;
    
	if (neon_bank > 7) {
		// FPGA / MCU status
		result = (neon_fpga_ver << 12) + (neon_mcu_id << 8) + (neon_mcu_busy << 7) + ((neon_bank & 7) << 0); 
		if (neon_debug_enabled) {
			printf("neon_mcu_status_r: 0x%X\n", result);
		}
	} else {
		//need to return proper rom data from selected bank here, but not sure what to do for that yet
		result = 0xffff;
	}

	return result;
}

void neogeo_cart_slot_device::neon_auxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	// TODO: Implement real S and V ROM banks - Project Neon uses none
	if (data & 0x8000) {
		neon_adpcm_bank = data & 7;
		if (neon_debug_enabled) {
			printf("neon_auxbank_w: ADCPM bank %u ** not implemented in mame **\n", neon_adpcm_bank);
		}
	} else {
		neon_fix_bank = data & 3;
		if (neon_debug_enabled) {
			printf("neon_fixbank_w: Fix bank %u ** not implemented in mame **\n", neon_fix_bank);
		}
	}
}

void neogeo_cart_slot_device::neon_irq_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	uint32_t seed;
	bool valid = true;
	uint8_t save_number, index;
	uint16_t result, modulo;
	char pwd[64 + 1];
	
	neon_mcu_busy = 0x01;
	if (neon_debug_enabled) {
		printf("neon_irq_w() ** MCU set to BUSY **\n");
	}
	
	uint16_t first_word = neon_game_to_mcu_ram[0];
	uint8_t count = first_word;
	uint16_t* data_in = &neon_game_to_mcu_ram[1];
	uint16_t* data_out = &neon_mcu_to_game_ram[1];
	
	// TODO: Simulate execution delays for each command ?
	switch (first_word >> 8) {
		case CART_CMD_SMUL: // SMUL(a, b) -- 16x16:32 signed multiply
			if (count > (NEON_RAM_DATA_MAX / 2)) {
				printf("mcu SMUL count %d exceeds RAM capacity\n", count);
				valid = false;
				break;
			}
			
			for (uint8_t c = 0; c < count; c += 2) {
				int32_t result = data_in[c] * data_in[c + 1];
				data_out[c] = result >> 16;
				data_out[c + 1] = result;
			}

			if (neon_debug_enabled) {
				printf("mcu SMUL %d operations\n", count);
			}
			break;

		case CART_CMD_ATAN2: // ATAN2(y, x) -- Standard 16bit atan2
			if (count > (NEON_RAM_DATA_MAX / 2)) {
				printf("mcu ATAN2 count %d exceeds RAM capacity\n", count);
				valid = false;
				break;
			}
			
			for (uint8_t s = 0, d = 0; d < count; s += 2, d++) {
				data_out[d] = fxpt_atan2(data_in[s], data_in[s + 1]);
				if (neon_debug_enabled && !d) {
					// Only print result for first operation
					printf("mcu ATAN2(%d, %d) = %d\n", data_in[s], data_in[s + 1], data_out[d]);
				}
			}
			
			if (neon_debug_enabled) {
				printf("mcu ATAN2 %d operations\n", count);
			}
			break;

		case CART_CMD_SAVE: // SAVE(n, data) -- Non-volatile mem save
			save_number = data_in[0];
			if (save_number >= NEON_MAX_SAVES) {
				printf("mcu save number %d invalid\n", save_number);
				data_out[0] = CART_SAVE_ERR;	// Valid, but return error code
				break;
			}

			if (neon_debug_enabled) {
				printf("Saving MCU RAM number %d\n", save_number);
			}

			data_out[0] = neon_save_ram(save_number);
			
			if (neon_debug_enabled) {
				printf("mcu SAVE(%d)\n", save_number);
			}
			break;

		case CART_CMD_LOAD: // LOAD(n) -- Non-volatile mem load
			save_number = data_in[0];
			if (save_number >= NEON_MAX_SAVES) {
				printf("mcu save number %d invalid\n", save_number);
				data_out[0] = CART_SAVE_ERR;	// Valid, but return error code
				break;
			}
			
			if (neon_debug_enabled) {
				printf("Loading MCU RAM number %d\n", save_number);
			}

			data_out[0] = neon_load_ram(save_number);

			if (neon_debug_enabled) {
				printf("mcu LOAD(%d)\n", save_number);
			}
			break;

		case CART_CMD_SRAND: // SRAND(s) -- Seed rng
			seed = (data_in[0] << 16) + data_in[1];
			//srand(seed);
			neon_sw_rng_next = seed;

			if (neon_debug_enabled) {
				printf("mcu SRAND(%d)\n", seed);
			}
			break;

		case CART_CMD_RAND: // RAND(m) -- Generate random numbers mod m
			if (count > NEON_RAM_DATA_MAX) {
				printf("mcu RAND count %d exceeds RAM capacity\n", count);
				valid = false;
				break;
			}
			
			modulo = data_in[0];
			if (!modulo) modulo = 1;	// Guard against divide by zero
			
			for (uint8_t c = 0; c < count; c++) {
				for (uint8_t pass = 0; pass < 19; pass++) {
	                if (neon_sw_rng_next & 0x80000000UL) {
	                    neon_sw_rng_next <<= 1;
	                    neon_sw_rng_next ^= 0xFFFFFFAFUL;
	                } else {
	                    neon_sw_rng_next <<= 1;
					}
	            }
	
				data_out[c] = (uint16_t)(neon_sw_rng_next % modulo);
			}

			if (neon_debug_enabled) {
				printf("mcu %d RAND(%d)'s, last: %d\n", count, modulo, neon_sw_rng_next);
			}
			break;
			
		case CART_CMD_WIFI_STATUS:
			data_out[0] = neon_status_wifi;
			
			if (neon_debug_enabled) {
				printf("mcu WIFI_STATUS\n");
			}
			break;

		case CART_CMD_WIFI_REFRESH:		// List WiFi access points
			accessPointsCount = neon_load_wifi_list();
			
			data_out[0] = accessPointsCount; // Found one access point
			
			if (neon_debug_enabled) {
				printf("mcu WIFI_REFRESH\n");
			}
			break;
		
		case CART_CMD_WIFI_GET_NAME:	// Get infos about a previously listed WiFi access point
			index = data_in[0];
			accessPointsCount = neon_load_wifi_list();
			
			if (accessPointsCount == 0 || index > accessPointsCount) {
				printf("mcu WIFI_GET_NAME index %d out of bounds\n", index);
				data_out[0] = CART_WIFI_INVALID;
				break;
			}
			
		  //data_out[0] = CART_WIFI_OK;
			data_out[0] = (30 << 8) | 3;	// rssi = 30, authmode = WIFI_AUTH_WPA2_PSK
			data_out[1] = 11 << 8;			// Channel 11

			for (int i=0; i<18; i++) { data_out[2 + i] = 0x00; }
			memcpy((char *)&data_out[2], accessPoints[index].name, 32);
			for (int i=0; i<16; i++) { data_out[2 + i] = __builtin_bswap16(data_out[2 + i]); }

			if (neon_debug_enabled) {
				printf("mcu WIFI_GET_NAME(%d)\n", index);
			}
			break;
			
		case CART_CMD_WIFI_CONNECT:		// Connected to previously listed WiFi access point
			index = data_in[0];
			accessPointsCount = neon_load_wifi_list();
			
			if (accessPointsCount == 0 || index > accessPointsCount) {
				printf("mcu WIFI_CONNECT index %d out of bounds\n", index);
				data_out[0] = CART_SAVE_ERR;	// Valid, but return error code
				break;
			}
			
			memcpy(pwd, (uint8_t*)&data_in[1], 64);
			pwd[64] = 0;

			if (strcmp(pwd, accessPoints[index].password) == 0) {
				neon_status_wifi = NEON_WIFI_CONNECTED;
				data_out[0] = CART_WIFI_OK;
				if (neon_debug_enabled) {
					printf("mcu WIFI_CONNECT(%d) to \"%s\" succeeded with pwd \"%s\"\n", index, accessPoints[index].name, pwd);
				}
			} else {
				neon_status_wifi = NEON_WIFI_DISCONNECTED;
				data_out[0] = CART_WIFI_INVALID;
				if (neon_debug_enabled) {
					printf("mcu WIFI_CONNECT(%d) to \"%s\" failed with pwd \"%s\"\n", index, accessPoints[index].name, pwd);
				}
			}		
			break;
		
		case CART_CMD_TEST: // MCU test -- Returns 16-bit word sum of data words
			if (count > NEON_RAM_DATA_MAX) {
				printf("mcu TEST count %d exceeds RAM capacity\n", count);
				valid = false;
				break;
			}
			
			result = 0;
			for (uint8_t c = 0; c < count; c++) {
				result += data_in[c];
			}
			
			data_out[0] = result;
			
			if (neon_debug_enabled) {
				printf("** mcu TEST **\n");
			}
			break;

		case CART_CMD_HRESET: // HW reset
			if ((count == 1) && (data_in[0] == 0xaa55)) {
				// trigger system reset here
				if (neon_debug_enabled) {
					printf("** mcu HRESET **\n");
				}
			}
			break;

		default:
			valid = false;
			
			if (neon_debug_enabled) {
				printf("** Unhandled MCU command 0x%04X\n", first_word);
			}
			break;
	}

	if (!valid)
		neon_mcu_to_game_ram[0] = 0;
	else
		neon_mcu_to_game_ram[0] = first_word;	// Echo means command valid
		
	neon_mcu_busy = 0x00;
	
	if (neon_debug_enabled) {
		printf("neon_irq_w() ** MCU irq done **\n");
	}
}

void neogeo_cart_slot_device::neon_mcu_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	if (offset < NEON_FPGA_RAM_SIZE) {
		if (neon_debug_enabled) {
			printf("neon_mcu_ram_w(%x) = %04x (%d)\n", 0x210000 + (offset * 2), data, data);
		}
		neon_game_to_mcu_ram[offset] = data;
	} else {
		if (neon_debug_enabled) {
			printf("neon_mcu_ram_w() - out of bounds write to %X\n", offset);
		}
	}
}

uint16_t neogeo_cart_slot_device::neon_mcu_ram_r(offs_t offset) {
	// TODO: Implement real P ROM banks - Project Neon uses none
	
	if (neon_bank > 7) {
		if (offset < NEON_FPGA_RAM_SIZE) {
			if (neon_debug_enabled) {
				printf("neon_mcu_ram_r(%x) = %04x (%d)\n", 0x210000 + (offset * 2), neon_mcu_to_game_ram[offset], neon_mcu_to_game_ram[offset]);
			}
			return neon_mcu_to_game_ram[offset];
		} else {
			if (neon_debug_enabled) {
				printf("neon_mcu_ram_r(%x) out of bounds\n", 0x210000 + (offset * 2));
			}
			return 0xffff;
		}
	} else {
		//need to return proper rom data from selected bank here, but not sure what to do for that yet
		return 0xffff;
	}
}

