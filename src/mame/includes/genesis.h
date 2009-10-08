/* Todo, reorganise, cleanup etc.*/

/*----------- defined in drivers/genesis.c -----------*/

extern UINT8 *genesis_z80_ram;
extern UINT16 *genesis_68k_ram;
extern MACHINE_START( genesis );
extern MACHINE_RESET( genesis );
extern READ8_HANDLER ( genesis_z80_r );
extern READ8_HANDLER ( genesis_z80_bank_r );
extern WRITE8_HANDLER ( genesis_z80_w );
extern WRITE16_HANDLER ( genesis_68k_to_z80_w );
extern READ16_HANDLER ( genesis_68k_to_z80_r );
extern INTERRUPT_GEN( genesis_vblank_interrupt );
extern void genesis_irq2_interrupt(const device_config *device, int state);

/*----------- defined in video/genesis.c -----------*/

extern UINT8		genesis_vdp_regs[];
extern UINT16		genesis_bg_pal_lookup[];
extern UINT16		genesis_sp_pal_lookup[];

VIDEO_START( genesis );
VIDEO_START( segac2 );

VIDEO_UPDATE( genesis );
VIDEO_UPDATE( segac2 );
VIDEO_UPDATE( megaplay );

void segac2_enable_display(running_machine *machine, int enable);

void system18_vdp_start(running_machine *machine);
void system18_vdp_update(bitmap_t *bitmap, const rectangle *cliprect);

READ16_HANDLER ( genesis_vdp_r );
WRITE16_HANDLER( genesis_vdp_w );
