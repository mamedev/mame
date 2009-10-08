extern DRIVER_INIT( megadriv_c2 );
extern DRIVER_INIT( megadrie );
extern DRIVER_INIT( megadriv );
extern DRIVER_INIT( megadrij );
extern DRIVER_INIT( _32x );
extern DRIVER_INIT( mpnew );

INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );
INPUT_PORTS_EXTERN( aladbl );
INPUT_PORTS_EXTERN( megadri6 );
INPUT_PORTS_EXTERN( ssf2ghw );
INPUT_PORTS_EXTERN( megdsvp );

MACHINE_DRIVER_EXTERN( megdsvppal );
MACHINE_DRIVER_EXTERN( megadriv );
MACHINE_DRIVER_EXTERN( megadpal );
MACHINE_DRIVER_EXTERN( megdsvp );
MACHINE_DRIVER_EXTERN( genesis_32x );
MACHINE_DRIVER_EXTERN( genesis_32x_pal );
MACHINE_DRIVER_EXTERN( genesis_scd );
MACHINE_DRIVER_EXTERN( genesis_32x_scd );

extern UINT16* megadriv_backupram;
extern int megadriv_backupram_length;
extern UINT16* megadrive_ram;
extern UINT8 megatech_bios_port_cc_dc_r(running_machine *machine, int offset, int ctrl);
extern void megadriv_stop_scanline_timer(void);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine, const char* tag);

extern READ8_HANDLER (megatech_sms_ioport_dc_r);
extern READ8_HANDLER (megatech_sms_ioport_dd_r);

extern READ16_HANDLER( megadriv_vdp_r );
extern WRITE16_HANDLER( megadriv_vdp_w );


MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
VIDEO_UPDATE( megadriv );
VIDEO_EOF( megadriv );

/* Needed to create external input handlers (see e.g. MESS) */

extern UINT8 (*megadrive_io_read_data_port_ptr)(running_machine *machine, int offset);
extern void (*megadrive_io_write_data_port_ptr)(running_machine *machine, int offset, UINT16 data);
extern UINT8 megadrive_io_data_regs[3];
extern UINT8 megadrive_io_ctrl_regs[3];

extern UINT16* megadrive_vdp_palette_lookup;
extern UINT16* megadrive_vdp_palette_lookup_sprite; // for C2
extern UINT16* megadrive_vdp_palette_lookup_shadow;
extern UINT16* megadrive_vdp_palette_lookup_highlight;

extern int segac2_bg_pal_lookup[4];
extern int segac2_sp_pal_lookup[4];

extern int genvdp_use_cram;
extern int genesis_always_irq6;
extern int genesis_other_hacks;


// megaplay & megatech

READ8_HANDLER( megaplay_bios_6402_r );
WRITE8_HANDLER( megaplay_bios_6402_w );
READ8_HANDLER( megaplay_bios_6204_r );
WRITE8_HANDLER( megaplay_bios_width_w );
WRITE16_HANDLER( megaplay_io_write );
READ16_HANDLER( megaplay_io_read );

/* Megaplay BIOS specific */
#define MP_ROM  0x10
#define MP_GAME 0

struct _mplay_bios	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
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
};

extern struct _mplay_bios mplay_bios;	// defined in megaplay.c

struct _mtech_bios	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
	UINT8 mt_cart_select_reg;
	UINT32 bios_port_ctrl;
	int current_game_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	UINT32 bios_ctrl_inputs;
	UINT8 bios_ctrl[6];

	int mt_bank_bank_pos;
	int mt_bank_partial;
	int mt_bank_addr;
};
