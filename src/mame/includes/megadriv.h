#define MASTER_CLOCK_NTSC 53693175
#define MASTER_CLOCK_PAL  53203424
#define SEGACD_CLOCK      12500000

/*----------- defined in machine/megadriv.c -----------*/

extern DRIVER_INIT( megadriv_c2 );
extern DRIVER_INIT( megadrie );
extern DRIVER_INIT( megadriv );
extern DRIVER_INIT( megadrij );
extern DRIVER_INIT( _32x );
extern DRIVER_INIT( mpnew );

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
extern UINT16* megadrive_ram;

extern UINT8 megatech_bios_port_cc_dc_r(running_machine *machine, int offset, int ctrl);
extern void megadriv_stop_scanline_timer(void);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine, const char* tag);

extern READ16_HANDLER( megadriv_vdp_r );
extern WRITE16_HANDLER( megadriv_vdp_w );

/* These handlers are needed by megaplay.c */
extern READ16_HANDLER( megadriv_68k_io_read );
extern WRITE16_HANDLER( megadriv_68k_io_write );

/* These handlers are needed by puckpkmn.c for his memory map */
extern READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read);
extern WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write);

/* These are needed to create external input handlers (see e.g. MESS) */
/* Regs are also used by Megaplay! */
extern UINT8 (*megadrive_io_read_data_port_ptr)(running_machine *machine, int offset);
extern void (*megadrive_io_write_data_port_ptr)(running_machine *machine, int offset, UINT16 data);
extern UINT8 megadrive_io_data_regs[3];
extern UINT8 megadrive_io_ctrl_regs[3];

MACHINE_START( megadriv );
MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
SCREEN_UPDATE( megadriv );
SCREEN_EOF( megadriv );


extern UINT16* megadrive_vdp_palette_lookup;
extern UINT16* megadrive_vdp_palette_lookup_sprite; // for C2
extern UINT16* megadrive_vdp_palette_lookup_shadow;
extern UINT16* megadrive_vdp_palette_lookup_highlight;

extern int segac2_bg_pal_lookup[4];
extern int segac2_sp_pal_lookup[4];

extern int genvdp_use_cram;
extern int genesis_always_irq6;
extern int genesis_other_hacks;

extern int megadrive_6buttons_pad;
extern int megadrive_region_export;
extern int megadrive_region_pal;

/* Megaplay - Megatech specific */
/* It might be possible to move the following structs in the drivers */

#define MP_ROM  0x10
#define MP_GAME 0

class md_base_state : public driver_device
{
public:
	md_base_state(running_machine &machine, const driver_device_config_base &config)
	: driver_device(machine, config) { }
};

class md_boot_state : public md_base_state
{
public:
	md_boot_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }

	// bootleg specific
	int aladmdb_mcu_port;
};

class segac2_state : public md_base_state
{
public:
	segac2_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }

	// for Print Club only
	int cam_data;

	int segac2_enable_display;

	UINT16* paletteram;

	/* internal states */
	UINT8		misc_io_data[0x10];	/* holds values written to the I/O chip */

	/* protection-related tracking */
	int (*prot_func)(int in);		/* emulation of protection chip */
	UINT8		prot_write_buf;		/* remembers what was written */
	UINT8		prot_read_buf;		/* remembers what was returned */

	/* palette-related variables */
	UINT8		segac2_alt_palette_mode;
	UINT8		palbank;
	UINT8		bg_palbase;
	UINT8		sp_palbase;

	/* sound-related variables */
	UINT8		sound_banks;		/* number of sound banks */
};

class mplay_state : public md_base_state
{
public:
	mplay_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }

	UINT32 bios_mode;  // determines whether ROM banks or Game data
	// is to read from 0x8000-0xffff

	UINT32 bios_bank; // ROM bank selection
	UINT16 game_banksel;  // Game bank selection
	UINT32 readpos;  // serial bank selection position (9-bit)
	UINT32 mp_bios_bank_addr;

	UINT32 bios_width;  // determines the way the game info ROM is read
	UINT8 bios_ctrl[6];
	UINT8 bios_6600;
	UINT8 bios_6403;
	UINT8 bios_6404;

	UINT16 *genesis_io_ram;
	UINT8* ic3_ram;
	UINT8* ic37_ram;
	UINT16 *ic36_ram;
};

class mtech_state : public md_base_state
{
public:
	mtech_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }

	UINT8 mt_cart_select_reg;
	UINT32 bios_port_ctrl;
	int current_game_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	UINT32 bios_ctrl_inputs;
	UINT8 bios_ctrl[6];

	int mt_bank_addr;

	int cart_is_genesis[8];

	/* Megatech BIOS specific */
	UINT8* megatech_banked_ram;
};

typedef struct _megadriv_cart  megadriv_cart;
struct _megadriv_cart
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
	md_cons_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }

	emu_timer *mess_io_timeout[3];
	int mess_io_stage[3];
	UINT8 jcart_io_data[2];

	megadriv_cart md_cart;
};

class pico_state : public md_cons_state
{
public:
	pico_state(running_machine &machine, const driver_device_config_base &config)
	: md_cons_state(machine, config) { }

	UINT8 page_register;
};

class mdsvp_state : public md_cons_state
{
public:
	mdsvp_state(running_machine &machine, const driver_device_config_base &config)
	: md_cons_state(machine, config) { }

	UINT8 *iram; // IRAM (0-0x7ff)
	UINT8 *dram; // [0x20000];
	UINT32 pmac_read[6];	// read modes/addrs for PM0-PM5
	UINT32 pmac_write[6];	// write ...
	PAIR pmc;
	UINT32 emu_status;
	UINT16 XST;		// external status, mapped at a15000 and a15002 on 68k side.
	UINT16 XST2;		// status of XST (bit1 set when 68k writes to XST)
};

class _32x_state : public md_base_state
{
public:
	_32x_state(running_machine &machine, const driver_device_config_base &config)
	: md_base_state(machine, config) { }
};

class segacd_state : public _32x_state	// use _32x_state as base to make easier the combo 32X + SCD
{
public:
	segacd_state(running_machine &machine, const driver_device_config_base &config)
	: _32x_state(machine, config) { }
};


/*----------- defined in machine/md_cart.c -----------*/

MACHINE_CONFIG_EXTERN( genesis_cartslot );
MACHINE_CONFIG_EXTERN( _32x_cartslot );
MACHINE_CONFIG_EXTERN( pico_cartslot );
MACHINE_START( md_sram );

/*----------- defined in drivers/megadriv.c -----------*/

/* These are needed to handle J-Cart inputs */
extern WRITE16_HANDLER( jcart_ctrl_w );
extern READ16_HANDLER( jcart_ctrl_r );
