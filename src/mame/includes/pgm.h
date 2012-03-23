
class pgm_state : public driver_device
{
public:
	pgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{
			m_irq4_disabled = 0;
		}

	/* memory pointers */
//  UINT16 *      m_mainram;  // currently this is also used by nvram handler
	UINT16 *      m_bg_videoram;
	UINT16 *      m_tx_videoram;
	UINT16 *      m_videoregs;
	UINT16 *      m_rowscrollram;
	UINT16 *      m_videoram;
	UINT8  *      m_z80_mainram;
	UINT32 *      m_arm7_shareram;
	UINT32 *      m_svg_shareram[2];	//for 5585G MACHINE
	UINT16 *      m_sharedprotram;		// killbld & olds
	UINT8  *      m_sprite_a_region;
	size_t        m_sprite_a_region_size;
	UINT16 *      m_spritebufferram; // buffered spriteram
//  UINT16 *      m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t       *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT16        *m_sprite_temp_render;
	bitmap_rgb32      m_tmppgmbitmap;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_soundcpu;
	cpu_device *m_prot;
	device_t *m_ics;

	/* used by rendering */
	UINT8 *m_bdata;
	size_t  m_bdatasize;
	int m_aoffset;
	int m_boffset;

	/* hack */
	int m_irq4_disabled;

	/* calendar */
	UINT8        m_cal_val;
	UINT8        m_cal_mask;
	UINT8        m_cal_com;
	UINT8        m_cal_cnt;
	system_time  m_systime;

	/* protection handling */
	// kov2
	UINT32        m_kov2_latchdata_68k_w;
	UINT32        m_kov2_latchdata_arm_w;
	// kovsh
	UINT16        m_kovsh_highlatch_arm_w;
	UINT16        m_kovsh_lowlatch_arm_w;
	UINT16        m_kovsh_highlatch_68k_w;
	UINT16        m_kovsh_lowlatch_68k_w;
	UINT32        m_kovsh_counter;
	// svg
	int           m_svg_ram_sel;
	// killbld & olds
	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	int			  m_kb_region_sequence_position;
	UINT32        m_kb_regs[0x10];
	UINT16        m_olds_bs;
	UINT16        m_olds_cmd3;
	// pstars
	UINT16        m_pstars_key;
	UINT16        m_pstars_int[2];
	UINT32        m_pstars_regs[16];
	UINT32        m_pstars_val;
	UINT16        m_pstar_e7;
	UINT16        m_pstar_b1;
	UINT16        m_pstar_ce;
	UINT16        m_pstar_ram[3];
	// ASIC 3 (oriental legends protection)
	UINT8         m_asic3_reg;
	UINT8         m_asic3_latch[3];
	UINT8         m_asic3_x;
	UINT8         m_asic3_y;
	UINT8         m_asic3_z;
	UINT8         m_asic3_h1;
	UINT8         m_asic3_h2;
	UINT16        m_asic3_hold;
	// ASIC28
	UINT16        m_asic28_key;
	UINT16        m_asic28_regs[10];
	UINT16        m_asic_params[256];
	UINT16        m_asic28_rcnt;
	UINT32        m_eoregs[16];
	// Oldsplus simulation
	UINT16        m_oldsplus_key;
	UINT16        m_oldsplus_int[2];
	UINT32        m_oldsplus_val;
	UINT16        m_oldsplus_ram[0x100];
	UINT32        m_oldsplus_regs[0x100];

	UINT32*       m_arm_ram;

};





extern UINT16 *pgm_mainram;	// used by nvram handler, we cannot move it to driver data struct

/*----------- defined in machine/pgmcrypt.c -----------*/

void pgm_kov_decrypt(running_machine &machine);
void pgm_kovsh_decrypt(running_machine &machine);
void pgm_kov2_decrypt(running_machine &machine);
void pgm_kov2p_decrypt(running_machine &machine);
void pgm_mm_decrypt(running_machine &machine);
void pgm_dw2_decrypt(running_machine &machine);
void pgm_photoy2k_decrypt(running_machine &machine);
void pgm_py2k2_decrypt(running_machine &machine);
void pgm_dw3_decrypt(running_machine &machine);
void pgm_killbld_decrypt(running_machine &machine);
void pgm_pstar_decrypt(running_machine &machine);
void pgm_puzzli2_decrypt(running_machine &machine);
void pgm_theglad_decrypt(running_machine &machine);
void pgm_ddp2_decrypt(running_machine &machine);
void pgm_dfront_decrypt(running_machine &machine);
void pgm_oldsplus_decrypt(running_machine &machine);
void pgm_kovshp_decrypt(running_machine &machine);
void pgm_killbldp_decrypt(running_machine &machine);
void pgm_svg_decrypt(running_machine &machine);
void pgm_svgpcb_decrypt(running_machine &machine);
void pgm_ket_decrypt(running_machine &machine);
void pgm_espgal_decrypt(running_machine &machine);
void pgm_happy6_decrypt(running_machine &machine);

/*----------- defined in machine/pgmprot.c -----------*/

READ16_HANDLER( pstars_protram_r );
READ16_HANDLER( pstars_r );
WRITE16_HANDLER( pstars_w );

READ16_HANDLER( pgm_asic3_r );
WRITE16_HANDLER( pgm_asic3_w );
WRITE16_HANDLER( pgm_asic3_reg_w );

READ16_HANDLER( sango_protram_r );
READ16_HANDLER( asic28_r );
WRITE16_HANDLER( asic28_w );

READ16_HANDLER( dw2_d80000_r );


READ16_HANDLER( oldsplus_protram_r );
READ16_HANDLER( oldsplus_r );
WRITE16_HANDLER( oldsplus_w );

MACHINE_RESET( kov );
void install_protection_asic_sim_kov(running_machine &machine);

/*----------- defined in video/pgm.c -----------*/

WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );

VIDEO_START( pgm );
SCREEN_VBLANK( pgm );
SCREEN_UPDATE_IND16( pgm );
