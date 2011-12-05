
#include "video/generic.h"

VIDEO_START( pce );
SCREEN_UPDATE( pce );
WRITE8_HANDLER ( vdc_0_w );
WRITE8_HANDLER ( vdc_1_w );
 READ8_HANDLER ( vdc_0_r );
 READ8_HANDLER ( vdc_1_r );
PALETTE_INIT( vce );
 READ8_HANDLER ( vce_r );
WRITE8_HANDLER ( vce_w );
WRITE8_HANDLER( vpc_w );
 READ8_HANDLER( vpc_r );
WRITE8_HANDLER( sgx_vdc_w );
 READ8_HANDLER( sgx_vdc_r );
TIMER_DEVICE_CALLBACK( pce_interrupt );
TIMER_DEVICE_CALLBACK( sgx_interrupt );

/* Screen timing stuff */

#define VDC_WPF			684		/* width of a line in frame including blanking areas */
#define VDC_LPF         262     /* number of lines in a single frame */

/* Bits in the VDC status register */

#define VDC_BSY         0x40    /* Set when the VDC accesses VRAM */
#define VDC_VD          0x20    /* Set when in the vertical blanking period */
#define VDC_DV          0x10    /* Set when a VRAM > VRAM DMA transfer is done */
#define VDC_DS          0x08    /* Set when a VRAM > SATB DMA transfer is done */
#define VDC_RR          0x04    /* Set when the current scanline equals the RCR register */
#define VDC_OR          0x02    /* Set when there are more than 16 sprites on a line */
#define VDC_CR          0x01    /* Set when sprite #0 overlaps with another sprite */

/* Bits in the CR register */

#define CR_BB           0x80    /* Background blanking */
#define CR_SB           0x40    /* Object blanking */
#define CR_VR           0x08    /* Interrupt on vertical blank enable */
#define CR_RC           0x04    /* Interrupt on line compare enable */
#define CR_OV           0x02    /* Interrupt on sprite overflow enable */
#define CR_CC           0x01    /* Interrupt on sprite #0 collision enable */

/* Bits in the DCR regsiter */

#define DCR_DSR         0x10    /* VRAM > SATB auto-transfer enable */
#define DCR_DID         0x08    /* Destination diretion */
#define DCR_SID         0x04    /* Source direction */
#define DCR_DVC         0x02    /* VRAM > VRAM EOT interrupt enable */
#define DCR_DSC         0x01    /* VRAM > SATB EOT interrupt enable */

/* just to keep things simple... */
enum vdc_regs {MAWR = 0, MARR, VxR, reg3, reg4, CR, RCR, BXR, BYR, MWR, HSR, HDR, VPR, VDW, VCR, DCR, SOUR, DESR, LENR, DVSSR };

