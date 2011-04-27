/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "devlegcy.h"
#include "machine/intelfsh.h"

class cps3_state : public driver_device
{
public:
	cps3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	fujitsu_29f016a_device *m_simm[7][8];
	UINT32* m_decrypted_bios;
	UINT32* m_decrypted_gamerom;
	UINT32 m_cram_gfxflash_bank;
	UINT32* m_nops;
	UINT32* m_tilemap20_regs_base;
	UINT32* m_tilemap30_regs_base;
	UINT32* m_tilemap40_regs_base;
	UINT32* m_tilemap50_regs_base;
	UINT32* m_0xc0000000_ram;
	UINT32* m_0xc0000000_ram_decrypted;
	UINT32* m_char_ram;
	UINT32* m_spriteram;
	UINT32* m_eeprom;
	UINT32* m_fullscreenzoom;
	UINT32 m_ss_pal_base;
	UINT32* m_colourram;
	UINT32 m_unk_vidregs[0x20/4];
	UINT32 m_ss_bank_base;
	UINT32 m_screenwidth;
	UINT32* m_mame_colours;
	bitmap_t *m_renderbuffer_bitmap;
	rectangle m_renderbuffer_clip;
	UINT8* m_user4region;
	UINT32 m_key1;
	UINT32 m_key2;
	int m_altEncryption;
	UINT32* m_ss_ram;
	UINT32 m_cram_bank;
	UINT16 m_current_eeprom_read;
	UINT32 m_paldma_source;
	UINT32 m_paldma_realsource;
	UINT32 m_paldma_dest;
	UINT32 m_paldma_fade;
	UINT32 m_paldma_other2;
	UINT32 m_paldma_length;
	UINT32 m_chardma_source;
	UINT32 m_chardma_other;
	UINT32 m_current_table_address;
	int m_rle_length;
	int m_last_normal_byte;
	unsigned short m_lastb;
	unsigned short m_lastb2;
	UINT32* m_mainram;
	UINT8* m_user5region;
};


/*----------- defined in audio/cps3.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(CPS3, cps3_sound);

WRITE32_DEVICE_HANDLER( cps3_sound_w );
READ32_DEVICE_HANDLER( cps3_sound_r );
