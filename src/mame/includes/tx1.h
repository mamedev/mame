/*************************************************************************

    TX-1/Buggy Boy hardware

*************************************************************************/
#include "machine/8255ppi.h"
#include "sound/custom.h"

#define TX1_PIXEL_CLOCK		6000000
#define TX1_HBSTART			256
#define TX1_HBEND			0
#define TX1_HTOTAL			384
#define TX1_VBSTART			240
#define TX1_VBEND			0
#define TX1_VTOTAL			264

#define BB_PIXEL_CLOCK		6000000
#define BB_HBSTART			256
#define BB_HBEND			0
#define BB_HTOTAL			384
#define BB_VBSTART			240
#define BB_VBEND			0
#define BB_VTOTAL			288

/* Buggy Boy PCBs could use 8086s or V30s */
#define BUGGYBOY_CPU_TYPE	I8086
#define BUGGYBOY_ZCLK		7500000


/*----------- defined in drivers/tx1.c -----------*/
extern UINT16 *tx1_math_ram;
extern ppi8255_interface tx1_ppi8255_intf;
extern ppi8255_interface buggyboy_ppi8255_intf;

/*----------- defined in machine/tx1.c -----------*/
READ16_HANDLER( tx1_spcs_rom_r );
READ16_HANDLER( tx1_spcs_ram_r );
WRITE16_HANDLER( tx1_spcs_ram_w );
READ16_HANDLER( tx1_math_r );
WRITE16_HANDLER( tx1_math_w );
MACHINE_START( tx1 );
MACHINE_RESET( tx1 );


READ16_HANDLER( buggyboy_spcs_rom_r );
READ16_HANDLER( buggyboy_spcs_ram_r );
WRITE16_HANDLER( buggyboy_spcs_ram_w );
READ16_HANDLER( buggyboy_math_r );
WRITE16_HANDLER( buggyboy_math_w );
MACHINE_RESET( buggybjr );
MACHINE_RESET( buggyboy );
MACHINE_START( buggybjr );
MACHINE_START( buggyboy );

/*----------- defined in audio/tx1.c -----------*/
READ8_HANDLER( pit8253_r );
WRITE8_HANDLER( pit8253_w );

WRITE8_HANDLER( bb_ym1_a_w );
WRITE8_HANDLER( bb_ym1_b_w );
void *buggyboy_sh_start(int clock, const struct CustomSound_interface *config);
void buggyboy_sh_reset(void *token);

void *tx1_sh_start(int clock, const struct CustomSound_interface *config);
void tx1_sh_reset(void *token);


/*----------- defined in video/tx1.c -----------*/
READ16_HANDLER( tx1_crtc_r );
WRITE16_HANDLER( tx1_crtc_w );

extern UINT16 *tx1_vram;
extern UINT16 *tx1_objram;
extern UINT16 *tx1_rcram;
extern size_t tx1_objram_size;
PALETTE_INIT( tx1 );
WRITE16_HANDLER( tx1_vram_w );
VIDEO_START( tx1 );
VIDEO_UPDATE( tx1 );
VIDEO_EOF( tx1 );
WRITE16_HANDLER( tx1_slincs_w );
WRITE16_HANDLER( tx1_slock_w );
WRITE16_HANDLER( tx1_scolst_w );
WRITE16_HANDLER( tx1_bankcs_w );
WRITE16_HANDLER( tx1_flgcs_w );

extern UINT16 *buggyboy_objram;
extern UINT16 *buggyboy_rcram;
extern UINT16 *buggyboy_vram;
extern size_t buggyboy_objram_size;
extern size_t buggyboy_rcram_size;
PALETTE_INIT( buggyboy );
WRITE16_HANDLER( buggyboy_vram_w );
VIDEO_START( buggyboy );
VIDEO_UPDATE( buggyboy );
VIDEO_EOF( buggyboy );

extern UINT16 *buggybjr_vram;
PALETTE_INIT( buggybjr );
WRITE16_HANDLER( buggybjr_vram_w );
VIDEO_START( buggybjr );
VIDEO_UPDATE( buggybjr );
WRITE16_HANDLER( buggyboy_slincs_w );
WRITE16_HANDLER( buggyboy_scolst_w );
WRITE16_HANDLER( buggyboy_gas_w );
WRITE16_HANDLER( buggyboy_sky_w );
