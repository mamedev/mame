/* Todo, reorganise, cleanup etc.*/

#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "sound/2612intf.h"
#include "sound/upd7759.h"

/*----------- defined in drivers/megaplay.c -----------*/

extern UINT16 *ic36_ram;
extern UINT8 bios_6204;
extern UINT8 bios_ctrl[6];

/*----------- defined in drivers/megatech.c -----------*/

extern UINT32 bios_ctrl_inputs;

/*----------- defined in drivers/genesis.c -----------*/

extern UINT8 *genesis_z80_ram;
extern UINT16 *genesis_68k_ram;
extern MACHINE_START( genesis );
extern MACHINE_RESET( genesis );
extern WRITE16_HANDLER ( genesis_io_w );
extern UINT16 *genesis_io_ram;
extern READ16_HANDLER(genesis_ctrl_r);
extern READ16_HANDLER ( megaplay_68k_to_z80_r );
extern READ8_HANDLER ( genesis_z80_r );
extern READ8_HANDLER ( genesis_z80_bank_r );
extern WRITE8_HANDLER ( genesis_z80_w );
extern WRITE16_HANDLER(genesis_ctrl_w);
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
