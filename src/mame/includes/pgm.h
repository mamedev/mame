
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


};


/* for machine/pgmprot.c type games */
class pgm_asic3_state : public pgm_state
{
public:
	pgm_asic3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

	}

	// ASIC 3 (oriental legends protection)
	UINT8         m_asic3_reg;
	UINT8         m_asic3_latch[3];
	UINT8         m_asic3_x;
	UINT8         m_asic3_y;
	UINT8         m_asic3_z;
	UINT8         m_asic3_h1;
	UINT8         m_asic3_h2;
	UINT16        m_asic3_hold;
};


/* for machine/pgmprot1.c type games */
class pgm_arm_type1_state : public pgm_state
{
public:
	pgm_arm_type1_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

		m_curslots = 0;
		m_puzzli_54_trigger = 0;
	}

	/////////////// simulations
	UINT16 m_value0;
	UINT16 m_value1;
	UINT16 m_valuekey;
	UINT16 m_ddp3lastcommand;
	UINT32 m_valueresponse;
	int m_curslots;
	UINT32 m_slots[0x100];

	// pstars / oldsplus / kov
	UINT16 m_pstar_e7_value;
	UINT16 m_pstar_b1_value;
	UINT16 m_pstar_ce_value;
	UINT16 m_kov_c0_value;
	UINT16 m_kov_cb_value;
	UINT16 m_kov_fe_value;
	UINT16 m_extra_ram[0x100];
	// puzzli2
	INT32 m_puzzli_54_trigger;

	typedef void (*pgm_arm_sim_command_handler)(pgm_arm_type1_state *state, int pc);

	pgm_arm_sim_command_handler arm_sim_handler;

	/////////////// emulation
	UINT16        m_pgm_arm_type1_highlatch_arm_w;
	UINT16        m_pgm_arm_type1_lowlatch_arm_w;
	UINT16        m_pgm_arm_type1_highlatch_68k_w;
	UINT16        m_pgm_arm_type1_lowlatch_68k_w;
	UINT32        m_pgm_arm_type1_counter;
	UINT32 *      m_arm7_shareram;

	cpu_device *m_prot;
};

/* for machine/pgmprot2.c type games */
class pgm_arm_type2_state : public pgm_state
{
public:
	pgm_arm_type2_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

	}
	// kov2
	UINT32        m_kov2_latchdata_68k_w;
	UINT32        m_kov2_latchdata_arm_w;

	UINT32*       m_arm_ram;
	UINT32 *      m_arm7_shareram;

	cpu_device *m_prot;
};



/* for machine/pgmprot3.c type games */
class pgm_arm_type3_state : public pgm_state
{
public:
	pgm_arm_type3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

	}
	// svg
	int           m_svg_ram_sel;
	UINT32 *      m_svg_shareram[2];	//for 5585G MACHINE

	UINT32        m_svg_latchdata_68k_w;
	UINT32        m_svg_latchdata_arm_w;
	UINT32*       m_arm_ram;

	cpu_device *m_prot;
};


/* for machine/pgmprot4.c type games */
class pgm_022_025_state : public pgm_state
{
public:
	pgm_022_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

	}
	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	int			  m_kb_region_sequence_position;
	UINT32        m_kb_regs[0x10];
	UINT16 *      m_sharedprotram;

};

/* for machine/pgmprot6.c type games */
class pgm_028_025_state : public pgm_state
{
public:
	pgm_028_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

	}
	// olds
	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	UINT16        m_olds_bs;
	UINT16        m_olds_cmd3;
	UINT16 *      m_sharedprotram;

};




extern UINT16 *pgm_mainram;	// used by nvram handler, we cannot move it to driver data struct

/*----------- defined in drivers/pgm.c -----------*/

void pgm_basic_init( running_machine &machine, bool set_bank  = true );

INPUT_PORTS_EXTERN( pgm );

TIMER_DEVICE_CALLBACK( pgm_interrupt );

GFXDECODE_EXTERN( pgm );

MACHINE_CONFIG_EXTERN( pgm );
MACHINE_CONFIG_EXTERN( pgmbase );

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

MACHINE_CONFIG_EXTERN( pgm_asic3 );

/*----------- defined in machine/pgmprot1.c -----------*/

/* emulations */
DRIVER_INIT( photoy2k );
DRIVER_INIT( kovsh );
DRIVER_INIT( kovshp );
DRIVER_INIT( kovlsqh2 );
DRIVER_INIT( kovqhsgs );


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

MACHINE_CONFIG_EXTERN( pgm_arm_type1 );
MACHINE_CONFIG_EXTERN( pgm_arm_type1_sim );
MACHINE_CONFIG_EXTERN( pgm_arm_type1_cave );

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
MACHINE_CONFIG_EXTERN( pgm_arm_type2 );

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

MACHINE_CONFIG_EXTERN( pgm_arm_type3 );

DRIVER_INIT( theglad );
DRIVER_INIT( svg );
DRIVER_INIT( svgpcb );
DRIVER_INIT( killbldp );
DRIVER_INIT( dmnfrnt );
DRIVER_INIT( happy6 );

/*----------- defined in machine/pgmprot4.c -----------*/

MACHINE_CONFIG_EXTERN( pgm_022_025_kb );
MACHINE_CONFIG_EXTERN( pgm_022_025_dw );

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

MACHINE_CONFIG_EXTERN( pgm_028_025_ol );

DRIVER_INIT( olds );

INPUT_PORTS_EXTERN( olds );


/*----------- defined in video/pgm.c -----------*/

WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );

VIDEO_START( pgm );
SCREEN_VBLANK( pgm );
SCREEN_UPDATE_IND16( pgm );
