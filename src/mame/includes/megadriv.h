
#include "emu.h"
#include "coreutil.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh2/sh2.h"
#include "cpu/sh2/sh2comn.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/rf5c68.h"
#include "sound/sn76496.h"
#include "imagedev/chd_cd.h"
#include "machine/nvram.h"
#include "cpu/ssp1601/ssp1601.h"

#include "machine/megavdp.h"
#include "machine/mega32x.h"
#include "machine/megacd.h"
#include "video/315_5124.h"

#define MASTER_CLOCK_NTSC 53693175
#define MASTER_CLOCK_PAL  53203424
#define SEGACD_CLOCK      12500000


#define MAX_MD_CART_SIZE 0x800000

/* where a fresh copy of rom is stashed for reset and banking setup */
#define VIRGIN_COPY_GEN 0xd00000

#define MD_CPU_REGION_SIZE (MAX_MD_CART_SIZE + VIRGIN_COPY_GEN)


/*----------- defined in machine/megadriv.c -----------*/

INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );
INPUT_PORTS_EXTERN( megadri6 );
INPUT_PORTS_EXTERN( ssf2mdb );
INPUT_PORTS_EXTERN( mk3mdb );
INPUT_PORTS_EXTERN( megdsvp );

MACHINE_CONFIG_EXTERN( megadriv_timers );
MACHINE_CONFIG_EXTERN( md_ntsc );
MACHINE_CONFIG_EXTERN( md_pal );
MACHINE_CONFIG_EXTERN( md_svp );

MACHINE_CONFIG_EXTERN( megdsvppal );
MACHINE_CONFIG_EXTERN( megadriv );
MACHINE_CONFIG_EXTERN( megadpal );
MACHINE_CONFIG_EXTERN( megdsvp );
MACHINE_CONFIG_EXTERN( genesis_32x );
MACHINE_CONFIG_EXTERN( genesis_32x_pal );
MACHINE_CONFIG_EXTERN( genesis_scd );
MACHINE_CONFIG_EXTERN( genesis_scd_scd );
MACHINE_CONFIG_EXTERN( genesis_scd_mcd );
MACHINE_CONFIG_EXTERN( genesis_scd_mcdj );
MACHINE_CONFIG_EXTERN( genesis_32x_scd );
MACHINE_CONFIG_EXTERN( md_bootleg );	// for topshoot.c & hshavoc.c

extern UINT16* megadriv_backupram;
extern int megadriv_backupram_length;

extern UINT8 megatech_bios_port_cc_dc_r(running_machine &machine, int offset, int ctrl);
extern void megadriv_stop_scanline_timer(running_machine &machine);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine &machine, const char* tag);


/* These handlers are needed by megaplay.c */
extern READ16_HANDLER( megadriv_68k_io_read );
extern WRITE16_HANDLER( megadriv_68k_io_write );

/* These handlers are needed by puckpkmn.c for his memory map */
extern READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read);
extern WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write);

/* These are needed to create external input handlers (see e.g. MESS) */
/* Regs are also used by Megaplay! */
extern UINT8 (*megadrive_io_read_data_port_ptr)(running_machine &machine, int offset);
extern void (*megadrive_io_write_data_port_ptr)(running_machine &machine, int offset, UINT16 data);
extern UINT8 megadrive_io_data_regs[3];
extern UINT8 megadrive_io_ctrl_regs[3];

MACHINE_START( megadriv );
MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
SCREEN_UPDATE_RGB32( megadriv );
SCREEN_VBLANK( megadriv );





extern int megadrive_6buttons_pad;

/* Megaplay - Megatech specific */
/* It might be possible to move the following structs in the drivers */

#define MP_ROM  0x10
#define MP_GAME 0

class md_base_state : public driver_device
{
public:
	md_base_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_vdp(*this,"gen_vdp"),
		m_megadrive_ram(*this,"megadrive_ram")
	{ }
	required_device<sega_genesis_vdp_device> m_vdp;
	optional_shared_ptr<UINT16> m_megadrive_ram;

	DECLARE_DRIVER_INIT(megadriv_c2);
	DECLARE_DRIVER_INIT(megadrie);
	DECLARE_DRIVER_INIT(megadriv);
	DECLARE_DRIVER_INIT(megadrij);
	DECLARE_DRIVER_INIT(mpnew);
	
	TILE_GET_INFO_MEMBER( get_stampmap_16x16_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_16x16_16x16_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_16x16_tile_info );
};

class md_boot_state : public md_base_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { m_protcount = 0;}

	// bootleg specific
	int m_aladmdb_mcu_port;

	int m_protcount;

	// jzth protection
	DECLARE_WRITE16_MEMBER( bl_710000_w )
	{
		int pc = space.device().safe_pc();

		logerror("%06x writing to bl_710000_w %04x %04x\n", pc, data, mem_mask);

		// protection value is read from  0x710000 after a series of writes.. and stored at ff0007
		// startup
		/*
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)
        */
		// before lv stage 3
		/*
        059ce0 writing to bl_710000_w 0008 ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0x4)
        */
		// start level 3
		/*
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000e ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0x5)

        // after end sequence
        059ce0 writing to bl_710000_w 0008 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)

        */
		m_protcount++;
	}


	DECLARE_READ16_MEMBER( bl_710000_r )
	{
		UINT16 ret;
		int pc = space.device().safe_pc();
		logerror("%06x reading from bl_710000_r\n", pc);

		if (m_protcount==6) { ret = 0xe; }
		else if (m_protcount==5) { ret = 0x5; }
		else if (m_protcount==4) { ret = 0x4; }
		else ret = 0xf;

		m_protcount = 0;
		return ret;
	}

	DECLARE_DRIVER_INIT(aladmdb);
	DECLARE_DRIVER_INIT(mk3mdb);
	DECLARE_DRIVER_INIT(ssf2mdb);
	DECLARE_DRIVER_INIT(srmdb);
	DECLARE_DRIVER_INIT(topshoot);
	DECLARE_DRIVER_INIT(puckpkmn);
};

class segac2_state : public md_base_state
{
public:
	segac2_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
	  m_paletteram(*this, "paletteram") { }

	// for Print Club only
	int m_cam_data;

	int m_segac2_enable_display;

	required_shared_ptr<UINT16> m_paletteram;

	/* internal states */
	UINT8		m_misc_io_data[0x10];	/* holds values written to the I/O chip */

	/* protection-related tracking */
	int (*m_prot_func)(int in);		/* emulation of protection chip */
	UINT8		m_prot_write_buf;		/* remembers what was written */
	UINT8		m_prot_read_buf;		/* remembers what was returned */

	/* palette-related variables */
	UINT8		m_segac2_alt_palette_mode;
	UINT8		m_palbank;
	UINT8		m_bg_palbase;
	UINT8		m_sp_palbase;

	/* sound-related variables */
	UINT8		m_sound_banks;		/* number of sound banks */

	DECLARE_DRIVER_INIT(c2boot);
	DECLARE_DRIVER_INIT(bloxeedc);
	DECLARE_DRIVER_INIT(columns);
	DECLARE_DRIVER_INIT(columns2);
	DECLARE_DRIVER_INIT(tfrceac);
	DECLARE_DRIVER_INIT(tfrceacb);
	DECLARE_DRIVER_INIT(borench);
	DECLARE_DRIVER_INIT(twinsqua);
	DECLARE_DRIVER_INIT(ribbit);
	DECLARE_DRIVER_INIT(puyo);
	DECLARE_DRIVER_INIT(tantr);
	DECLARE_DRIVER_INIT(tantrkor);
	DECLARE_DRIVER_INIT(potopoto);
	DECLARE_DRIVER_INIT(stkclmns);
	DECLARE_DRIVER_INIT(stkclmnj);
	DECLARE_DRIVER_INIT(ichir);
	DECLARE_DRIVER_INIT(ichirk);
	DECLARE_DRIVER_INIT(ichirj);
	DECLARE_DRIVER_INIT(ichirjbl);
	DECLARE_DRIVER_INIT(puyopuy2);
	DECLARE_DRIVER_INIT(zunkyou);
	DECLARE_DRIVER_INIT(pclub);
	DECLARE_DRIVER_INIT(pclubjv2);
	DECLARE_DRIVER_INIT(pclubjv4);
	DECLARE_DRIVER_INIT(pclubjv5);
	void segac2_common_init(running_machine& machine, int (*func)(int in));
	DECLARE_VIDEO_START(segac2_new);
	DECLARE_MACHINE_START(segac2);
	DECLARE_MACHINE_RESET(segac2);

};

class mplay_state : public md_base_state
{
public:
	mplay_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
	m_ic3_ram(*this, "ic3_ram"),
	m_vdp1(*this, "vdp1"),
	m_bioscpu(*this, "mtbios")


	{ }

	UINT32 m_bios_mode;  // determines whether ROM banks or Game data
	// is to read from 0x8000-0xffff

	UINT32 m_bios_bank; // ROM bank selection
	UINT16 m_game_banksel;  // Game bank selection
	UINT32 m_readpos;  // serial bank selection position (9-bit)
	UINT32 m_mp_bios_bank_addr;

	UINT32 m_bios_width;  // determines the way the game info ROM is read
	UINT8 m_bios_ctrl[6];
	UINT8 m_bios_6600;
	UINT8 m_bios_6403;
	UINT8 m_bios_6404;

	UINT16 *m_genesis_io_ram;
	required_shared_ptr<UINT8> m_ic3_ram;
	optional_device<sega315_5124_device> m_vdp1;
	required_device<cpu_device>          m_bioscpu;
	UINT8* m_ic37_ram;
	UINT16 *m_ic36_ram;
	DECLARE_WRITE_LINE_MEMBER( int_callback );
	DECLARE_DRIVER_INIT(megaplay);
	DECLARE_VIDEO_START(megplay);
	DECLARE_MACHINE_RESET(megaplay);
};

class mtech_state : public md_base_state
{
public:
	mtech_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
		m_vdp1(*this, "vdp1"),
		m_bioscpu(*this, "mtbios")
	{ }


	required_device<sega315_5124_device> m_vdp1;
	required_device<cpu_device>          m_bioscpu;


	DECLARE_WRITE_LINE_MEMBER( int_callback );


	UINT8 m_mt_cart_select_reg;
	UINT32 m_bios_port_ctrl;
	int m_current_game_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	UINT32 m_bios_ctrl_inputs;
	UINT8 m_bios_ctrl[6];

	int m_mt_bank_addr;

	int m_cart_is_genesis[8];

	/* Megatech BIOS specific */
	UINT8* m_megatech_banked_ram;
	DECLARE_DRIVER_INIT(mt_crt);
	DECLARE_DRIVER_INIT(mt_slot);
	DECLARE_VIDEO_START(mtnew);
	DECLARE_MACHINE_RESET(mtnew);
};

struct megadriv_cart
{
	int type;

	// SRAM related
	UINT16 *sram;
	int last_loaded_image_length;
	int sram_start, sram_end;
	int sram_active, sram_readonly;
	int sram_handlers_installed;
	int sram_detected;

	// EEPROM related
	int has_serial_eeprom;

	// I2C related
	UINT8 i2c_mem, i2c_clk;

	// mapper related (mostly for pirate carts)
	UINT16 squirrel_king_extra;
	UINT16 lion2_prot1_data, lion2_prot2_data;
	UINT16 realtec_bank_addr, realtec_bank_size, realtec_old_bank_addr;
	UINT16 l3alt_pdat, l3alt_pcmd;
	int ssf2_lastoff, ssf2_lastdata;
};

class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { }

	emu_timer *m_mess_io_timeout[3];
	int m_mess_io_stage[3];
	UINT8 m_jcart_io_data[2];

	megadriv_cart m_md_cart;

	DECLARE_DRIVER_INIT(hshavoc);
	DECLARE_DRIVER_INIT(topshoot);

	DECLARE_DRIVER_INIT(genesis);
	DECLARE_DRIVER_INIT(mess_md_common);
	DECLARE_DRIVER_INIT(md_eur);
	DECLARE_DRIVER_INIT(md_jpn);

};


class mdsvp_state : public md_cons_state
{
public:
	mdsvp_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_cons_state(mconfig, type, tag) { }

	UINT8 *m_iram; // IRAM (0-0x7ff)
	UINT8 *m_dram; // [0x20000];
	UINT32 m_pmac_read[6];	// read modes/addrs for PM0-PM5
	UINT32 m_pmac_write[6];	// write ...
	PAIR m_pmc;
	UINT32 m_emu_status;
	UINT16 m_XST;		// external status, mapped at a15000 and a15002 on 68k side.
	UINT16 m_XST2;		// status of XST (bit1 set when 68k writes to XST)
};

ADDRESS_MAP_EXTERN( svp_ssp_map, driver_device );
ADDRESS_MAP_EXTERN( svp_ext_map, driver_device );
extern void svp_init(running_machine &machine);
extern cpu_device *_svp_cpu;



UINT8 megadrive_io_read_data_port_3button(running_machine &machine, int portnum);

class _32x_state : public md_base_state
{
public:
	_32x_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { }

};



extern cpu_device *_32x_master_cpu;
extern cpu_device *_32x_slave_cpu;

// called from out main scanline timers...





class segacd_state : public _32x_state	// use _32x_state as base to make easier the combo 32X + SCD
{
public:
	segacd_state(const machine_config &mconfig, device_type type, const char *tag)
	: _32x_state(mconfig, type, tag),
	  m_font_bits(*this,"segacd_font") { }
	
	required_shared_ptr<UINT16> m_font_bits;
};

extern int sega_cd_connected;
extern int segacd_wordram_mapped;
extern cpu_device *_segacd_68k_cpu;
extern MACHINE_RESET( segacd );
ADDRESS_MAP_EXTERN( segacd_map, driver_device);
extern TIMER_DEVICE_CALLBACK( scd_dma_timer_callback );
extern timer_device* scd_dma_timer;
extern void segacd_init_main_cpu( running_machine& machine );

/*----------- defined in machine/md_cart.c -----------*/

MACHINE_CONFIG_EXTERN( genesis_cartslot );
MACHINE_CONFIG_EXTERN( _32x_cartslot );
MACHINE_CONFIG_EXTERN( pico_cartslot );
MACHINE_START( md_sram );

/*----------- defined in drivers/megadriv.c -----------*/

/* These are needed to handle J-Cart inputs */
extern WRITE16_HANDLER( jcart_ctrl_w );
extern READ16_HANDLER( jcart_ctrl_r );

/* machine/megavdp.c */
extern UINT16 (*vdp_get_word_from_68k_mem)(running_machine &machine, UINT32 source, address_space* space);
extern UINT16 vdp_get_word_from_68k_mem_default(running_machine &machine, UINT32 source, address_space* space);
extern int megadriv_framerate;
extern int megadrive_total_scanlines;
extern int megadrive_vblank_flag;
extern int genesis_scanline_counter;
extern UINT16* megadrive_vdp_palette_lookup;

extern int megadrive_region_export;
extern int megadrive_region_pal;

/* machine/megadriv.c */
extern TIMER_DEVICE_CALLBACK( megadriv_scanline_timer_callback );
extern timer_device* megadriv_scanline_timer;
