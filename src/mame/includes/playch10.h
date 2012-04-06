typedef struct
{
	int writable;	// 1 for RAM, 0 for ROM
	UINT8* chr;		// direct access to the memory
} chr_bank;

class playch10_state : public driver_device
{
public:
	playch10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_ram_8w;
	UINT8 *m_work_ram;
	int m_up_8w;
	UINT8 *m_timedata;
	int m_pc10_nmi_enable;
	int m_pc10_dog_di;
	int m_pc10_sdcs;
	int m_pc10_dispmask;
	int m_pc10_int_detect;
	int m_pc10_game_mode;
	int m_pc10_dispmask_old;
	int m_pc10_gun_controller;
	int m_cart_sel;
	int m_cntrl_mask;
	int m_input_latch[2];
	int m_mirroring;
	int m_MMC2_bank[4];
	int m_MMC2_bank_latch[2];
	UINT8* m_vrom;
	UINT8* m_vram;
	UINT8* m_nametable[4];
	UINT8* m_nt_ram;
	chr_bank m_chr_page[8];
	int m_mmc1_shiftreg;
	int m_mmc1_shiftcount;
	int m_mmc1_rom_mask;
	int m_gboard_scanline_counter;
	int m_gboard_scanline_latch;
	int m_gboard_banks[2];
	int m_gboard_4screen;
	int m_gboard_last_bank;
	int m_gboard_command;
	int m_pc10_bios;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(up8w_w);
	DECLARE_READ8_MEMBER(ram_8w_r);
	DECLARE_WRITE8_MEMBER(ram_8w_w);
	DECLARE_WRITE8_MEMBER(sprite_dma_w);
	DECLARE_WRITE8_MEMBER(time_w);
	DECLARE_WRITE8_MEMBER(pc10_SDCS_w);
	DECLARE_WRITE8_MEMBER(pc10_CNTRLMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_DISPMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_SOUNDMASK_w);
	DECLARE_WRITE8_MEMBER(pc10_NMIENABLE_w);
	DECLARE_WRITE8_MEMBER(pc10_DOGDI_w);
	DECLARE_WRITE8_MEMBER(pc10_GAMERES_w);
	DECLARE_WRITE8_MEMBER(pc10_GAMESTOP_w);
	DECLARE_WRITE8_MEMBER(pc10_PPURES_w);
	DECLARE_READ8_MEMBER(pc10_detectclr_r);
	DECLARE_WRITE8_MEMBER(pc10_CARTSEL_w);
	DECLARE_READ8_MEMBER(pc10_prot_r);
	DECLARE_WRITE8_MEMBER(pc10_prot_w);
	DECLARE_WRITE8_MEMBER(pc10_in0_w);
	DECLARE_READ8_MEMBER(pc10_in0_r);
	DECLARE_READ8_MEMBER(pc10_in1_r);
	DECLARE_WRITE8_MEMBER(pc10_nt_w);
	DECLARE_READ8_MEMBER(pc10_nt_r);
	DECLARE_WRITE8_MEMBER(pc10_chr_w);
	DECLARE_READ8_MEMBER(pc10_chr_r);
	DECLARE_WRITE8_MEMBER(mmc1_rom_switch_w);
	DECLARE_WRITE8_MEMBER(aboard_vrom_switch_w);
	DECLARE_WRITE8_MEMBER(bboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(cboard_vrom_switch_w);
	DECLARE_WRITE8_MEMBER(eboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(gboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(iboard_rom_switch_w);
	DECLARE_WRITE8_MEMBER(hboard_rom_switch_w);
	void pc10_set_mirroring(int mirroring);
	DECLARE_WRITE8_MEMBER(playch10_videoram_w);
};


/*----------- defined in machine/playch10.c -----------*/

MACHINE_RESET( pc10 );
MACHINE_START( pc10 );
MACHINE_START( playch10_hboard );
DRIVER_INIT( playch10 );	/* standard games */
DRIVER_INIT( pc_gun );	/* gun games */
DRIVER_INIT( pc_hrz );	/* horizontal games */
DRIVER_INIT( pcaboard );	/* a-board games */
DRIVER_INIT( pcbboard );	/* b-board games */
DRIVER_INIT( pccboard );	/* c-board games */
DRIVER_INIT( pcdboard );	/* d-board games */
DRIVER_INIT( pcdboard_2 );	/* d-board games with extra ram */
DRIVER_INIT( pceboard );	/* e-board games */
DRIVER_INIT( pcfboard );	/* f-board games */
DRIVER_INIT( pcfboard_2 );	/* f-board games with extra ram */
DRIVER_INIT( pcgboard );	/* g-board games */
DRIVER_INIT( pcgboard_type2 ); /* g-board games with 4 screen mirror */
DRIVER_INIT( pchboard );	/* h-board games */
DRIVER_INIT( pciboard );	/* i-board games */
DRIVER_INIT( pckboard );	/* k-board games */
CUSTOM_INPUT( pc10_int_detect_r );



/*----------- defined in video/playch10.c -----------*/

extern const ppu2c0x_interface playch10_ppu_interface;
extern const ppu2c0x_interface playch10_ppu_interface_hboard;

PALETTE_INIT( playch10 );
VIDEO_START( playch10 );
VIDEO_START( playch10_hboard );
SCREEN_UPDATE_IND16( playch10_top );
SCREEN_UPDATE_IND16( playch10_bottom );
