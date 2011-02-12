/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "devlegcy.h"
#include "machine/intelfsh.h"

class cps3_state : public driver_device
{
public:
	cps3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	fujitsu_29f016a_device *simm[7][8];
	UINT32* decrypted_bios;
	UINT32* decrypted_gamerom;
	UINT32 cram_gfxflash_bank;
	UINT32* nops;
	UINT32* tilemap20_regs_base;
	UINT32* tilemap30_regs_base;
	UINT32* tilemap40_regs_base;
	UINT32* tilemap50_regs_base;
	UINT32* _0xc0000000_ram;
	UINT32* _0xc0000000_ram_decrypted;
	UINT32* char_ram;
	UINT32* spriteram;
	UINT32* eeprom;
	UINT32* fullscreenzoom;
	UINT32 ss_pal_base;
	UINT32* colourram;
	UINT32 unk_vidregs[0x20/4];
	UINT32 ss_bank_base;
	UINT32 screenwidth;
	UINT32* mame_colours;
	bitmap_t *renderbuffer_bitmap;
	rectangle renderbuffer_clip;
	UINT8* user4region;
	UINT32 key1;
	UINT32 key2;
	int altEncryption;
	UINT32* ss_ram;
	UINT32 cram_bank;
	UINT16 current_eeprom_read;
	UINT32 paldma_source;
	UINT32 paldma_realsource;
	UINT32 paldma_dest;
	UINT32 paldma_fade;
	UINT32 paldma_other2;
	UINT32 paldma_length;
	UINT32 chardma_source;
	UINT32 chardma_other;
	UINT32 current_table_address;
	int rle_length;
	int last_normal_byte;
	unsigned short lastb;
	unsigned short lastb2;
	UINT32* mainram;
	UINT8* user5region;
};


/*----------- defined in audio/cps3.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(CPS3, cps3_sound);

WRITE32_DEVICE_HANDLER( cps3_sound_w );
READ32_DEVICE_HANDLER( cps3_sound_r );
