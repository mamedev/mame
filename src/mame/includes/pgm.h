
#include "machine/v3021.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/arm7/arm7.h"
#include "sound/ics2115.h"
#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"

#define PGMARM7LOGERROR 0

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
	// ASIC 3 (oriental legends protection)
	UINT8         m_asic3_reg;
	UINT8         m_asic3_latch[3];
	UINT8         m_asic3_x;
	UINT8         m_asic3_y;
	UINT8         m_asic3_z;
	UINT8         m_asic3_h1;
	UINT8         m_asic3_h2;
	UINT16        m_asic3_hold;

	UINT32*       m_arm_ram;

};

/* for machine/pgmprot1.c type games */
class pgm_kovarmsim_state : public pgm_state
{
public:
	pgm_kovarmsim_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

		m_ddp3internal_slot = 0;
	}

	UINT16 m_value0;
	UINT16 m_value1;
	UINT16 m_valuekey;
	UINT16 m_ddp3lastcommand;
	UINT32 m_valueresponse;
	int m_ddp3internal_slot;
	UINT32 m_ddp3slots[0x100];

	// pstars / oldsplus
	UINT16        m_pstar_e7;
	UINT16        m_pstar_b1;
	UINT16        m_pstar_ce;
	UINT16        m_extra_ram[0x100];

	typedef void (*pgm_arm_sim_command_handler)(pgm_kovarmsim_state *state, int pc);

	pgm_arm_sim_command_handler arm_sim_handler;
};



extern UINT16 *pgm_mainram;	// used by nvram handler, we cannot move it to driver data struct

/*----------- defined in drivers/pgm.c -----------*/

void pgm_basic_init( running_machine &machine, bool set_bank  = true );

INPUT_PORTS_EXTERN( pgm );

/* we only need half of these because CavePGM has it's own MACHINE DRIVER in pgmprot1.c - refactor */
TIMER_DEVICE_CALLBACK( pgm_interrupt );

GFXDECODE_EXTERN( pgm );

MACHINE_CONFIG_EXTERN( pgm );

ADDRESS_MAP_EXTERN( pgm_z80_mem, 8 );
ADDRESS_MAP_EXTERN( pgm_z80_io, 8 );
void pgm_sound_irq( device_t *device, int level );

ADDRESS_MAP_EXTERN( pgm_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_basic_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_base_mem, 16 );

MACHINE_START( pgm );
MACHINE_RESET( pgm );

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

DRIVER_INIT( orlegend );

INPUT_PORTS_EXTERN( orlegend );
INPUT_PORTS_EXTERN( orld105k );

/*----------- defined in machine/pgmprot1.c -----------*/

/* emulations */
DRIVER_INIT( photoy2k );
DRIVER_INIT( kovsh );
DRIVER_INIT( kovshp );
DRIVER_INIT( kovlsqh2 );
DRIVER_INIT( kovqhsgs );

MACHINE_CONFIG_EXTERN( kov );

/* simulations */
DRIVER_INIT( ddp3 );
DRIVER_INIT( ket );
DRIVER_INIT( espgal );
DRIVER_INIT( puzzli2 );
DRIVER_INIT( py2k2 );
DRIVER_INIT( pstar );
DRIVER_INIT( kov );
DRIVER_INIT( kovboot );
DRIVER_INIT( oldsplus );

MACHINE_CONFIG_EXTERN( kov_simulated_arm );
MACHINE_CONFIG_EXTERN( cavepgm );

INPUT_PORTS_EXTERN( sango );
INPUT_PORTS_EXTERN( sango_ch );
INPUT_PORTS_EXTERN( photoy2k );
INPUT_PORTS_EXTERN( oldsplus );
INPUT_PORTS_EXTERN( pstar );
INPUT_PORTS_EXTERN( py2k2 );
INPUT_PORTS_EXTERN( puzzli2 );
INPUT_PORTS_EXTERN( kovsh );

/*----------- defined in machine/pgmprot2.c -----------*/

/* emulations */
MACHINE_CONFIG_EXTERN( kov2 );

DRIVER_INIT( kov2 );
DRIVER_INIT( kov2p );
DRIVER_INIT( martmast );
DRIVER_INIT( ddp2 );

/* simulations (or missing) */

DRIVER_INIT( dw2001 );

INPUT_PORTS_EXTERN( ddp2 );
INPUT_PORTS_EXTERN( kov2 );
INPUT_PORTS_EXTERN( martmast );
INPUT_PORTS_EXTERN( dw2001 );

/*----------- defined in machine/pgmprot3.c -----------*/

MACHINE_CONFIG_EXTERN( svg );

DRIVER_INIT( theglad );
DRIVER_INIT( svg );
DRIVER_INIT( svgpcb );
DRIVER_INIT( killbldp );
DRIVER_INIT( dmnfrnt );
DRIVER_INIT( happy6 );

/*----------- defined in machine/pgmprot4.c -----------*/

MACHINE_CONFIG_EXTERN( killbld );
MACHINE_CONFIG_EXTERN( dw3 );

DRIVER_INIT( killbld );
DRIVER_INIT( drgw3 );

INPUT_PORTS_EXTERN( killbld );
INPUT_PORTS_EXTERN( dw3 );

/*----------- defined in machine/pgmprot5.c -----------*/

DRIVER_INIT( drgw2 );
DRIVER_INIT( dw2v100x );
DRIVER_INIT( drgw2c );
DRIVER_INIT( drgw2j );

/*----------- defined in machine/pgmprot6.c -----------*/

MACHINE_CONFIG_EXTERN( olds );

DRIVER_INIT( olds );

INPUT_PORTS_EXTERN( olds );


/*----------- defined in video/pgm.c -----------*/

WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );

VIDEO_START( pgm );
SCREEN_VBLANK( pgm );
SCREEN_UPDATE_IND16( pgm );
