/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "devlegcy.h"
#include "machine/intelfsh.h"

class cps3_state : public driver_device
{
public:
	cps3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_colourram(*this, "colourram"),
		m_tilemap20_regs_base(*this, "tmap20_regs"),
		m_tilemap30_regs_base(*this, "tmap30_regs"),
		m_tilemap40_regs_base(*this, "tmap40_regs"),
		m_tilemap50_regs_base(*this, "tmap50_regs"),
		m_fullscreenzoom(*this, "fullscreenzoom"),
		m_0xc0000000_ram(*this, "0xc0000000_ram"){ }

	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_colourram;
	required_shared_ptr<UINT32> m_tilemap20_regs_base;
	required_shared_ptr<UINT32> m_tilemap30_regs_base;
	required_shared_ptr<UINT32> m_tilemap40_regs_base;
	required_shared_ptr<UINT32> m_tilemap50_regs_base;
	required_shared_ptr<UINT32> m_fullscreenzoom;
	required_shared_ptr<UINT32> m_0xc0000000_ram;

	fujitsu_29f016a_device *m_simm[7][8];
	UINT32* m_decrypted_bios;
	UINT32* m_decrypted_gamerom;
	UINT32 m_cram_gfxflash_bank;
	UINT32* m_nops;
	UINT32* m_0xc0000000_ram_decrypted;
	UINT32* m_char_ram;
	UINT32* m_eeprom;
	UINT32 m_ss_pal_base;
	UINT32 m_unk_vidregs[0x20/4];
	UINT32 m_ss_bank_base;
	UINT32 m_screenwidth;
	UINT32* m_mame_colours;
	bitmap_rgb32 m_renderbuffer_bitmap;
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
	UINT8* m_user5region;
	DECLARE_READ32_MEMBER(cps3_ssram_r);
	DECLARE_WRITE32_MEMBER(cps3_ssram_w);
	DECLARE_WRITE32_MEMBER(cps3_0xc0000000_ram_w);
	DECLARE_WRITE32_MEMBER(cram_bank_w);
	DECLARE_READ32_MEMBER(cram_data_r);
	DECLARE_WRITE32_MEMBER(cram_data_w);
	DECLARE_READ32_MEMBER(cps3_gfxflash_r);
	DECLARE_WRITE32_MEMBER(cps3_gfxflash_w);
	DECLARE_READ32_MEMBER(cps3_flash1_r);
	DECLARE_READ32_MEMBER(cps3_flash2_r);
	DECLARE_WRITE32_MEMBER(cps3_flash1_w);
	DECLARE_WRITE32_MEMBER(cps3_flash2_w);
	DECLARE_WRITE32_MEMBER(cram_gfxflash_bank_w);
	DECLARE_READ32_MEMBER(cps3_vbl_r);
	DECLARE_READ32_MEMBER(cps3_unk_io_r);
	DECLARE_READ32_MEMBER(cps3_40C0000_r);
	DECLARE_READ32_MEMBER(cps3_40C0004_r);
	DECLARE_READ32_MEMBER(cps3_eeprom_r);
	DECLARE_WRITE32_MEMBER(cps3_eeprom_w);
	DECLARE_WRITE32_MEMBER(cps3_ss_bank_base_w);
	DECLARE_WRITE32_MEMBER(cps3_ss_pal_base_w);
	DECLARE_WRITE32_MEMBER(cps3_palettedma_w);
	DECLARE_WRITE32_MEMBER(cps3_characterdma_w);
	DECLARE_WRITE32_MEMBER(cps3_irq10_ack_w);
	DECLARE_WRITE32_MEMBER(cps3_irq12_ack_w);
	DECLARE_WRITE32_MEMBER(cps3_unk_vidregs_w);
	DECLARE_READ32_MEMBER(cps3_colourram_r);
	DECLARE_WRITE32_MEMBER(cps3_colourram_w);
	DECLARE_DIRECT_UPDATE_MEMBER(cps3_direct_handler);
	DECLARE_DRIVER_INIT(sfiii3);
	DECLARE_DRIVER_INIT(sfiii);
	DECLARE_DRIVER_INIT(redearth);
	DECLARE_DRIVER_INIT(jojo);
	DECLARE_DRIVER_INIT(jojoba);
	DECLARE_DRIVER_INIT(sfiii2);
};


/*----------- defined in audio/cps3.c -----------*/

class cps3_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	cps3_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~cps3_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type CPS3;


WRITE32_DEVICE_HANDLER( cps3_sound_w );
READ32_DEVICE_HANDLER( cps3_sound_r );
