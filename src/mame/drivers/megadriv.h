extern DRIVER_INIT( megadrie );
extern DRIVER_INIT( megadriv );
extern DRIVER_INIT( megadrij );
extern DRIVER_INIT( megadsvp );

INPUT_PORTS_EXTERN(megadriv);
INPUT_PORTS_EXTERN(aladbl);
INPUT_PORTS_EXTERN(megadri6);
INPUT_PORTS_EXTERN(ssf2ghw);
INPUT_PORTS_EXTERN(megdsvp);

MACHINE_DRIVER_EXTERN(megadriv);
MACHINE_DRIVER_EXTERN(megadpal);
MACHINE_DRIVER_EXTERN(_32x);
MACHINE_DRIVER_EXTERN(megdsvp);

extern UINT16* megadriv_backupram;
extern int megadriv_backupram_length;
extern UINT16* megadrive_ram;
extern UINT8 megatech_bios_port_cc_dc_r(int offset, int ctrl);
extern void megadriv_stop_scanline_timer(void);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine);

extern READ8_HANDLER (megatech_sms_ioport_dc_r);
extern READ8_HANDLER (megatech_sms_ioport_dd_r);

extern READ16_HANDLER( megadriv_vdp_r );
extern WRITE16_HANDLER( megadriv_vdp_w );

MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
VIDEO_UPDATE( megadriv );
VIDEO_EOF( megadriv );

/* Needed to create external input handlers (see e.g. MESS) */

extern UINT8 (*megadrive_io_read_data_port_ptr)(int offset);
extern void (*megadrive_io_write_data_port_ptr)(int offset, UINT16 data);
extern UINT8 megadrive_io_data_regs[3];
extern UINT8 megadrive_io_ctrl_regs[3];
