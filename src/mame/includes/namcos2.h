/***************************************************************************

  namcos2.h

  Common functions & declarations for the Namco System 2 driver

***************************************************************************/

/* CPU reference numbers */

#define CPU_MASTER	0
#define CPU_SLAVE	1
#define CPU_SOUND	2
#define CPU_MCU 	3
#define CPU_GPU 	5


/*********************************************/
/* IF GAME SPECIFIC HACKS ARE REQUIRED THEN  */
/* USE THE namcos2_gametype VARIABLE TO FIND */
/* OUT WHAT GAME IS RUNNING                  */
/*********************************************/

enum
{
	/* Namco System 2 */
	NAMCOS2_ASSAULT = 0x1000,
	NAMCOS2_ASSAULT_JP,
	NAMCOS2_ASSAULT_PLUS,
	NAMCOS2_BUBBLE_TROUBLE,
	NAMCOS2_BURNING_FORCE,
	NAMCOS2_COSMO_GANG,
	NAMCOS2_COSMO_GANG_US,
	NAMCOS2_DIRT_FOX,
	NAMCOS2_DIRT_FOX_JP,
	NAMCOS2_DRAGON_SABER,
	NAMCOS2_FINAL_LAP,
	NAMCOS2_FINAL_LAP_2,
	NAMCOS2_FINAL_LAP_3,
	NAMCOS2_FINEST_HOUR,
	NAMCOS2_FOUR_TRAX,
	NAMCOS2_GOLLY_GHOST,
	NAMCOS2_LUCKY_AND_WILD,
	NAMCOS2_MARVEL_LAND,
	NAMCOS2_METAL_HAWK,
	NAMCOS2_MIRAI_NINJA,
	NAMCOS2_ORDYNE,
	NAMCOS2_PHELIOS,
	NAMCOS2_ROLLING_THUNDER_2,
	NAMCOS2_STEEL_GUNNER,
	NAMCOS2_STEEL_GUNNER_2,
	NAMCOS2_SUPER_WSTADIUM,
	NAMCOS2_SUPER_WSTADIUM_92,
	NAMCOS2_SUPER_WSTADIUM_92T,
	NAMCOS2_SUPER_WSTADIUM_93,
	NAMCOS2_SUZUKA_8_HOURS,
	NAMCOS2_SUZUKA_8_HOURS_2,
	NAMCOS2_VALKYRIE,
	NAMCOS2_KYUUKAI_DOUCHUUKI,

	/* Namco System21 */
	NAMCOS21_AIRCOMBAT,
	NAMCOS21_STARBLADE,
	NAMCOS21_CYBERSLED,
	NAMCOS21_SOLVALOU,
	NAMCOS21_WINRUN91,
	NAMCOS21_DRIVERS_EYES,

	/* Namco NB1 */
	NAMCONB1_NEBULRAY,
	NAMCONB1_GUNBULET,
	NAMCONB1_GSLGR94U,
	NAMCONB1_GSLGR94J,
	NAMCONB1_SWS95,
	NAMCONB1_SWS96,
	NAMCONB1_SWS97,
	NAMCONB1_VSHOOT,

	/* Namco NB2 */
	NAMCONB2_OUTFOXIES,
	NAMCONB2_MACH_BREAKERS,

	/* Namco System FL */
	NAMCOFL_SPEED_RACER,
	NAMCOFL_FINAL_LAP_R
};

class namcos2_state : public driver_device
{
public:
	namcos2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_dpram(*this, "dpram"),
		  m_spriteram(*this, "spriteram"),
		  m_paletteram(*this, "paletteram"),
		  m_rozram(*this, "rozram")
	{ }
	DECLARE_READ16_MEMBER(dpram_word_r);
	DECLARE_WRITE16_MEMBER(dpram_word_w);
	DECLARE_READ8_MEMBER(dpram_byte_r);
	DECLARE_WRITE8_MEMBER(dpram_byte_w);
	DECLARE_DRIVER_INIT(cosmogng);
	DECLARE_DRIVER_INIT(sgunner2);
	DECLARE_DRIVER_INIT(kyukaidk);
	DECLARE_DRIVER_INIT(bubbletr);
	DECLARE_DRIVER_INIT(suzuk8h2);
	DECLARE_DRIVER_INIT(burnforc);
	DECLARE_DRIVER_INIT(gollygho);
	DECLARE_DRIVER_INIT(rthun2j);
	DECLARE_DRIVER_INIT(sws);
	DECLARE_DRIVER_INIT(finehour);
	DECLARE_DRIVER_INIT(finallap);
	DECLARE_DRIVER_INIT(dirtfoxj);
	DECLARE_DRIVER_INIT(marvlanj);
	DECLARE_DRIVER_INIT(sws92);
	DECLARE_DRIVER_INIT(dsaber);
	DECLARE_DRIVER_INIT(assault);
	DECLARE_DRIVER_INIT(mirninja);
	DECLARE_DRIVER_INIT(finalap2);
	DECLARE_DRIVER_INIT(valkyrie);
	DECLARE_DRIVER_INIT(fourtrax);
	DECLARE_DRIVER_INIT(finalap3);
	DECLARE_DRIVER_INIT(luckywld);
	DECLARE_DRIVER_INIT(assaultj);
	DECLARE_DRIVER_INIT(dsaberj);
	DECLARE_DRIVER_INIT(suzuka8h);
	DECLARE_DRIVER_INIT(phelios);
	DECLARE_DRIVER_INIT(sws93);
	DECLARE_DRIVER_INIT(metlhawk);
	DECLARE_DRIVER_INIT(sws92g);
	DECLARE_DRIVER_INIT(assaultp_hack);
	DECLARE_DRIVER_INIT(assaultp);
	DECLARE_DRIVER_INIT(ordyne);
	DECLARE_DRIVER_INIT(marvland);
	DECLARE_DRIVER_INIT(rthun2);
	
	virtual void video_start();
	void video_start_finallap();
	void video_start_luckywld();
	void video_start_metlhawk();
	void video_start_sgunner();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_finallap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_luckywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_metlhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sgunner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	DECLARE_READ16_MEMBER( paletteram_word_r );
	DECLARE_WRITE16_MEMBER( paletteram_word_w );
	DECLARE_READ16_MEMBER( spriteram_word_r );
	DECLARE_WRITE16_MEMBER( spriteram_word_w );
	DECLARE_READ16_MEMBER( rozram_word_r );
	DECLARE_WRITE16_MEMBER( rozram_word_w );
	DECLARE_READ16_MEMBER( roz_ctrl_word_r );
	DECLARE_WRITE16_MEMBER( roz_ctrl_word_w );

	void update_palette();
	void apply_clip( rectangle &clip, const rectangle &cliprect );
	void draw_roz(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control );
	void draw_sprites_metalhawk(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	UINT16 get_palette_register( int which );

	required_shared_ptr<UINT8> m_dpram;	/* 2Kx8 */
	optional_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_rozram;

	UINT16 m_roz_ctrl[0x8];
	tilemap_t *m_tilemap_roz;

};

/*----------- defined in video/namcos2.c -----------*/

#define NAMCOS21_NUM_COLORS 0x8000

VIDEO_START( namcos2 );

VIDEO_START( finallap );

VIDEO_START( luckywld );

VIDEO_START( metlhawk );

VIDEO_START( sgunner );

int namcos2_GetPosIrqScanline( running_machine &machine );

WRITE16_HANDLER( namcos2_gfx_ctrl_w );
READ16_HANDLER( namcos2_gfx_ctrl_r );

/**************************************************************/
/*  Shared video palette function handlers                    */
/**************************************************************/

#define VIRTUAL_PALETTE_BANKS 30

/**************************************************************/
/*  ROZ - Rotate & Zoom memory function handlers              */
/**************************************************************/

/*----------- defined in machine/namcos2.c -----------*/

extern void (*namcos2_kickstart)(running_machine &machine, int internal);
extern int namcos2_gametype;

MACHINE_START( namcos2 );
MACHINE_RESET( namcos2 );

READ16_HANDLER( namcos2_flap_prot_r );

/**************************************************************/
/*  EEPROM memory function handlers                           */
/**************************************************************/
#define NAMCOS2_68K_eeprom_W	namcos2_68k_eeprom_w
#define NAMCOS2_68K_eeprom_R	namcos2_68k_eeprom_r
WRITE16_HANDLER( namcos2_68k_eeprom_w );
READ16_HANDLER( namcos2_68k_eeprom_r );

/**************************************************************/
/*  Shared data ROM memory handlerhandlers                    */
/**************************************************************/
READ16_HANDLER( namcos2_68k_data_rom_r );

/**************************************************************/
/* Shared serial communications processory (CPU5 ????)        */
/**************************************************************/
READ16_HANDLER( namcos2_68k_serial_comms_ram_r );
WRITE16_HANDLER( namcos2_68k_serial_comms_ram_w );
READ16_HANDLER( namcos2_68k_serial_comms_ctrl_r );
WRITE16_HANDLER( namcos2_68k_serial_comms_ctrl_w );

extern UINT16 *namcos2_68k_serial_comms_ram;

/**************************************************************/
/* Shared protection/random number generator                  */
/**************************************************************/
READ16_HANDLER( namcos2_68k_key_r );
WRITE16_HANDLER( namcos2_68k_key_w );

/**************************************************************/
/* Non-shared memory custom IO device - IRQ/Inputs/Outputs   */
/**************************************************************/

#define NAMCOS2_C148_0			0		/* 0x1c0000 */
#define NAMCOS2_C148_1			1		/* 0x1c2000 */
#define NAMCOS2_C148_2			2		/* 0x1c4000 */
#define NAMCOS2_C148_CPUIRQ 	3		/* 0x1c6000 */
#define NAMCOS2_C148_EXIRQ		4		/* 0x1c8000 */
#define NAMCOS2_C148_POSIRQ 	5		/* 0x1ca000 */
#define NAMCOS2_C148_SERIRQ 	6		/* 0x1cc000 */
#define NAMCOS2_C148_VBLANKIRQ	7		/* 0x1ce000 */

WRITE16_HANDLER( namcos2_68k_master_C148_w );
READ16_HANDLER( namcos2_68k_master_C148_r );
INTERRUPT_GEN( namcos2_68k_master_vblank );

WRITE16_HANDLER( namcos2_68k_slave_C148_w );
READ16_HANDLER( namcos2_68k_slave_C148_r );
INTERRUPT_GEN( namcos2_68k_slave_vblank );

WRITE16_HANDLER( namcos2_68k_gpu_C148_w );
READ16_HANDLER( namcos2_68k_gpu_C148_r );
INTERRUPT_GEN( namcos2_68k_gpu_vblank );

void namcos2_adjust_posirq_timer( running_machine &machine, int scanline );

/**************************************************************/
/* MASTER CPU RAM MEMORY                                      */
/**************************************************************/

#define NAMCOS2_68K_MASTER_RAM	"bank3"

/**************************************************************/
/* SLAVE CPU RAM MEMORY                                       */
/**************************************************************/

#define NAMCOS2_68K_SLAVE_RAM	"bank4"

/**************************************************************/
/*                                                            */
/**************************************************************/
#define BANKED_SOUND_ROM		"bank6"

/**************************************************************/
/* Sound CPU support handlers - 6809                          */
/**************************************************************/

WRITE8_HANDLER( namcos2_sound_bankselect_w );

/**************************************************************/
/* MCU Specific support handlers - HD63705                    */
/**************************************************************/

WRITE8_HANDLER( namcos2_mcu_analog_ctrl_w );
READ8_HANDLER( namcos2_mcu_analog_ctrl_r );

WRITE8_HANDLER( namcos2_mcu_analog_port_w );
READ8_HANDLER( namcos2_mcu_analog_port_r );

WRITE8_HANDLER( namcos2_mcu_port_d_w );
READ8_HANDLER( namcos2_mcu_port_d_r );

READ8_HANDLER( namcos2_input_port_0_r );
READ8_HANDLER( namcos2_input_port_10_r );
READ8_HANDLER( namcos2_input_port_12_r );

