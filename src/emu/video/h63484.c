/***************************************************************************

	HD63484 ACRTC (rewrite in progress)

	TODO:
	- 8-bit support for FIFO, parameters and command values
	- convert to C++

***************************************************************************/

#include "emu.h"
#include "video/h63484.h"

#define LOG 1

typedef struct _h63484_state h63484_state;
struct _h63484_state
{
	h63484_display_pixels_func display_func;
	screen_device *screen;	/* screen */

	UINT8 *vram;
	UINT8 ar;
	UINT8 vreg[0x100];
	UINT8 sr;

	UINT8 fifo[16];					/* FIFO W data queue */
	int fifo_ptr;					/* FIFO W pointer */

	UINT8 fifo_r[16];				/* FIFO R data queue */
	int fifo_r_ptr;					/* FIFO R pointer */


	UINT16 cr;
	UINT16 pr[9];					/* parameter byte register */
	int param_ptr;					/* parameter pointer */

	UINT32 rwp;
	UINT8 rwp_dn;

	address_space		*space;		/* memory space */
};

enum
{
	FIFO_READ = 0,
	FIFO_WRITE
};

enum
{
	FIFO_EMPTY = -1,
	FIFO_PARAMETER,
	FIFO_COMMAND
};

#define H63484_SR_CER 0x80 // Command Error
#define H63484_SR_ARD 0x40 // Area Detect
#define H63484_SR_CED 0x20 // Command End
#define H63484_SR_LPD 0x10 // Light Pen Strobe Detect
#define H63484_SR_RFF 0x08 // Read FIFO Full
#define H63484_SR_RFR 0x04 // Read FIFO Ready
#define H63484_SR_WFR 0x02 // Write FIFO Ready
#define H63484_SR_WFE 0x01 // Write FIFO Empty


#define CCR 0x02
// Command Control
#define ABT 0x8000


static const char *const acrtc_regnames[0x100/2] =
{
	"FIFO Entry", // 0x00
	"Command Control (CCR)", // 0x02
	"Operation Mode (OMR)", // 0x04
	"Display Control (DCR)", // 0x06
	"(Undefined)", // 0x08
	"(Undefined)", // 0x0a
	"(Undefined)", // 0x0c
	"(Undefined)", // 0x0e
	"(Undefined)", // 0x10
	"(Undefined)", // 0x12
	"(Undefined)", // 0x14
	"(Undefined)", // 0x16
	"(Undefined)", // 0x18
	"(Undefined)", // 0x1a
	"(Undefined)", // 0x1c
	"(Undefined)", // 0x1e
	"(Undefined)", // 0x20
	"(Undefined)", // 0x22
	"(Undefined)", // 0x24
	"(Undefined)", // 0x26
	"(Undefined)", // 0x28
	"(Undefined)", // 0x2a
	"(Undefined)", // 0x2c
	"(Undefined)", // 0x2e
	"(Undefined)", // 0x30
	"(Undefined)", // 0x32
	"(Undefined)", // 0x34
	"(Undefined)", // 0x36
	"(Undefined)", // 0x38
	"(Undefined)", // 0x3a
	"(Undefined)", // 0x3c
	"(Undefined)", // 0x3e
	"(Undefined)", // 0x40
	"(Undefined)", // 0x42
	"(Undefined)", // 0x44
	"(Undefined)", // 0x46
	"(Undefined)", // 0x48
	"(Undefined)", // 0x4a
	"(Undefined)", // 0x4c
	"(Undefined)", // 0x4e
	"(Undefined)", // 0x50
	"(Undefined)", // 0x52
	"(Undefined)", // 0x54
	"(Undefined)", // 0x56
	"(Undefined)", // 0x58
	"(Undefined)", // 0x5a
	"(Undefined)", // 0x5c
	"(Undefined)", // 0x5e
	"(Undefined)", // 0x60
	"(Undefined)", // 0x62
	"(Undefined)", // 0x64
	"(Undefined)", // 0x66
	"(Undefined)", // 0x68
	"(Undefined)", // 0x6a
	"(Undefined)", // 0x6c
	"(Undefined)", // 0x6e
	"(Undefined)", // 0x70
	"(Undefined)", // 0x72
	"(Undefined)", // 0x74
	"(Undefined)", // 0x76
	"(Undefined)", // 0x78
	"(Undefined)", // 0x7a
	"(Undefined)", // 0x7c
	"(Undefined)", // 0x7e
	"Raster Count (RCR)", // 0x80
	"Horizontal Sync (HSR)", // 0x82
	"Horizontal Display (HDR)", // 0x84
	"Vertical Sync (VSR)", // 0x86
	"Vertical Display (VDR)", // 0x88
	"Split Screen Width (SSW) 0x8a", // 0x8a
	"Split Screen Width (SSW) 0x8c", // 0x8c
	"Split Screen Width (SSW) 0x8e", // 0x8e
	"Blink Control (BCR)", // 0x90
	"Horizontal Window Display (HWR)", // 0x92
	"Vertical Window Display (VWR) 0x94", // 0x94
	"Vertical Window Display (VWR) 0x96", // 0x96
	"Graphic Cursor (GCR0)", // 0x98
	"Graphic Cursor (GCR1)", // 0x9a
	"Graphic Cursor (GCR2)", // 0x9c
	"(Undefined)", // 0x9e
	"(Undefined)", // 0xa0
	"(Undefined)", // 0xa2
	"(Undefined)", // 0xa4
	"(Undefined)", // 0xa6
	"(Undefined)", // 0xa8
	"(Undefined)", // 0xaa
	"(Undefined)", // 0xac
	"(Undefined)", // 0xae
	"(Undefined)", // 0xb0
	"(Undefined)", // 0xb2
	"(Undefined)", // 0xb4
	"(Undefined)", // 0xb6
	"(Undefined)", // 0xb8
	"(Undefined)", // 0xba
	"(Undefined)", // 0xbc
	"(Undefined)", // 0xbe
	// upper screen
	"Raster Address 0 (RAR0)", // 0xc0
	"Memory Width 0 (MWR0)", // 0xc2
	"Start Address 0 (SAR0) 0xc4", // 0xc4
	"Start Address 0 (SAR0) 0xc6", // 0xc6
	// base screen
	"Raster Address 1 (RAR1)", // 0xc8
	"Memory Width 1 (MWR1)", // 0xca
	"Start Address 1 (SAR1) 0xcc", // 0xcc
	"Start Address 1 (SAR1) 0xce", // 0xce
	// lower screen
	"Raster Address 2 (RAR2)", // 0xd0
	"Memory Width 2 (MWR2)", // 0xd2
	"Start Address 2 (SAR2) 0xd4", // 0xd4
	"Start Address 2 (SAR2) 0xd6", // 0xd6
	// window screen
	"Raster Address 3 (RAR3)", // 0xd8
	"Memory Width 3 (MWR3)", // 0xda
	"Start Address 3 (SAR3) 0xdc", // 0xdc
	"Start Address 3 (SAR3) 0xde", // 0xde
	// block cursor 1
	"Block Cursor 1 (BCUR1) 0xe0",
	"Block Cursor 1 (BCUR1) 0xe2",
	// block cursor 2
	"Block Cursor 2 (BCUR2) 0xe4",
	"Block Cursor 2 (BCUR2) 0xe6",
	"Cursor Definition (CDR)",
	"Zoom Factor (ZFR)",
	"Lightpen Address (LPAR) 0xec",
	"Lightpen Address (LPAR) 0xee",
	"(Undefined)", // 0xf0
	"(Undefined)", // 0xf2
	"(Undefined)", // 0xf4
	"(Undefined)", // 0xf6
	"(Undefined)", // 0xf8
	"(Undefined)", // 0xfa
	"(Undefined)", // 0xfc
	"(Undefined)"  // 0xfe
};

static const char *const wpr_regnames[0x20] =
{
	"Color 0 (CL0)",
	"Color 1 (CL1)",
	"Color Comparison (COMP)",
	"Edge Color (EDG)",
	"Mask (MASK)",
	"Pattern RAM Control (PRC) 0x05",
	"Pattern RAM Control (PRC) 0x06",
	"Pattern RAM Control (PRC) 0x07",
	"Area Definition (ADR) 0x08",
	"Area Definition (ADR) 0x09",
	"Area Definition (ADR) 0x0a",
	"Area Definition (ADR) 0x0b",
	"Read Write Pointer (RWP) H",
	"Read Write Pointer (RWP) L",
	"(Undefined)",
	"(Undefined)",
	"Drawing Pointer (DP) H",
	"Drawing Pointer (DP) L",
	"Current Pointer (CP) H",
	"Current Pointer (CP) L",
	"(Undefined)", // 0x14
	"(Undefined)",
	"(Undefined)", // 0x16
	"(Undefined)",
	"(Undefined)", // 0x18
	"(Undefined)",
	"(Undefined)", // 0x1a
	"(Undefined)",
	"(Undefined)", // 0x1c
	"(Undefined)",
	"(Undefined)", // 0x1e
	"(Undefined)"
};

enum
{
	COMMAND_INVALID = -1,
	COMMAND_ORG,
	COMMAND_WPR,
	COMMAND_RPR,
	COMMAND_WPTN,
	COMMAND_RPTN,
	COMMAND_DRD,
	COMMAND_DWT,
	COMMAND_DMOD,
	COMMAND_RD,
	COMMAND_WT,
	COMMAND_MOD,
	COMMAND_CLR,
	COMMAND_SCLR,
	COMMAND_CPY,
	COMMAND_SCPY,
	COMMAND_AMOVE,
	COMMAND_RMOVE,
	COMMAND_ALINE,
	COMMAND_RLINE,
	COMMAND_ARCT,
	COMMAND_RRCT,
	COMMAND_APLL,
	COMMAND_RPLL,
	COMMAND_APLG,
	COMMAND_RPLG,
	COMMAND_CRCL,
	COMMAND_ELPS,
	COMMAND_AARC,
	COMMAND_RARC,
	COMMAND_AEARC,
	COMMAND_REARC,
	COMMAND_AFRCT,
	COMMAND_RFRCT,
	COMMAND_PAINT,
	COMMAND_DOT,
	COMMAND_PTN,
	COMMAND_AGCPY,
	COMMAND_RGCPY
};

#define H63484_COMMAND_ORG		0x0400	// 				p: 2
#define H63484_COMMAND_WPR		0x0800	// & ~0x1f  	p: 1
#define H63484_COMMAND_RPR		0x0c00	// & ~0x1f  	p: 0
#define H63484_COMMAND_WPTN		0x1800	// & ~0xf   	p: 1 + n
#define H63484_COMMAND_RPTN		0x1c00	// & ~0xf		p: 1
#define H63484_COMMAND_DRD		0x2400	//				p: 2
#define H63484_COMMAND_DWT		0x2800	// 				p: 2
#define H63484_COMMAND_DMOD		0x2c00	// & ~3			p: 2
#define H63484_COMMAND_RD		0x4400	//				p: 0
#define H63484_COMMAND_WT		0x4800	//				p: 1
#define H63484_COMMAND_MOD		0x4c00	// & ~3			p: 1
#define H63484_COMMAND_CLR		0x5800	//				p: 3
#define H63484_COMMAND_SCLR		0x5c00	// & ~3			p: 3
#define H63484_COMMAND_CPY		0x6000	// & ~0x0f03	p: 4
#define H63484_COMMAND_SCPY		0x7000	// & ~0x0f03	p: 4
#define H63484_COMMAND_AMOVE	0x8000	//				p: 2
#define H63484_COMMAND_RMOVE	0x8400	//				p: 2
#define H63484_COMMAND_ALINE	0x8800	// & ~0x00ff	p: 2
#define H63484_COMMAND_RLINE	0x8c00	// & ~0x00ff	p: 2
#define H63484_COMMAND_ARCT		0x9000	// & ~0x00ff	p: 2
#define H63484_COMMAND_RRCT		0x9400	// & ~0x00ff	p: 2
#define H63484_COMMAND_APLL		0x9800	// & ~0x00ff	p: 1 + n
#define H63484_COMMAND_RPLL		0x9c00	// & ~0x00ff	p: 1 + n
#define H63484_COMMAND_APLG		0xa000	// & ~0x00ff	p: 1 + n
#define H63484_COMMAND_RPLG		0xa400	// & ~0x00ff	p: 1 + n
#define H63484_COMMAND_CRCL		0xa800	// & ~0x01ff	p: 1
#define H63484_COMMAND_ELPS		0xac00	// & ~0x01ff	p: 3
#define H63484_COMMAND_AARC		0xb000	// & ~0x01ff	p: 4
#define H63484_COMMAND_RARC		0xb400	// & ~0x01ff	p: 4
#define H63484_COMMAND_AEARC	0xb800	// & ~0x01ff	p: 6
#define H63484_COMMAND_REARC	0xbc00	// & ~0x01ff	p: 6
#define H63484_COMMAND_AFRCT	0xc000	// & ~0x00ff	p: 2
#define H63484_COMMAND_RFRCT	0xc400	// & ~0x00ff	p: 2
#define H63484_COMMAND_PAINT	0xc800	// & ~0x01ff	p: 0
#define H63484_COMMAND_DOT		0xcc00	// & ~0x00ff	p: 0
#define H63484_COMMAND_PTN		0xd000	// & ~0x0fff	p: 1
#define H63484_COMMAND_AGCPY	0xe000	// & ~0x0fff	p: 4
#define H63484_COMMAND_RGCPY	0xf000	// & ~0x0fff	p: 4




/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE h63484_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == H63484);

	return (h63484_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const h63484_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == H63484);
	return (const h63484_interface *) device->static_config();
}


INLINE void fifo_w_clear(h63484_state *h63484)
{
	int i;

	for (i = 0; i < 16; i++)
		h63484->fifo[i] = 0;

	h63484->fifo_ptr = -1;

	h63484->sr |= H63484_SR_WFR;
	h63484->sr |= H63484_SR_WFE;
}

INLINE void queue_w(h63484_state *h63484, UINT8 data)
{
	if (h63484->fifo_ptr < 15)
	{
		h63484->fifo_ptr++;

		h63484->fifo[h63484->fifo_ptr] = data;

		if (h63484->fifo_ptr == 16)
			h63484->sr &= ~H63484_SR_WFR;

		h63484->sr &= ~H63484_SR_WFE;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

INLINE void dequeue_w(h63484_state *h63484, UINT8 *data)
{
	int i;

	*data = h63484->fifo[0];

	//h63484->sr &= ~H63484_SR_RFF;

	if (h63484->fifo_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			h63484->fifo[i] = h63484->fifo[i + 1];

		h63484->fifo[15] = 0;

		h63484->fifo_ptr--;

		h63484->sr |= H63484_SR_WFR;

		if (h63484->fifo_ptr == -1)
			h63484->sr |= H63484_SR_WFE;

	}
}

INLINE void fifo_r_clear(h63484_state *h63484)
{
	int i;

	for (i = 0; i < 16; i++)
		h63484->fifo_r[i] = 0;

	h63484->fifo_r_ptr = -1;

	h63484->sr &= ~H63484_SR_RFR;
	h63484->sr &= ~H63484_SR_RFF;
}

INLINE void queue_r(h63484_state *h63484, UINT8 data)
{
	if (h63484->fifo_ptr < 15)
	{
		h63484->fifo_r_ptr++;

		h63484->fifo_r[h63484->fifo_r_ptr] = data;

		if (h63484->fifo_r_ptr == 16)
			h63484->sr |= H63484_SR_RFF;

		h63484->sr |= H63484_SR_RFR;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

INLINE void dequeue_r(h63484_state *h63484, UINT8 *data)
{
	int i;

	*data = h63484->fifo_r[0];

	if (h63484->fifo_r_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			h63484->fifo_r[i] = h63484->fifo_r[i + 1];

		h63484->fifo_r[15] = 0;

		h63484->fifo_r_ptr--;

		h63484->sr &= ~H63484_SR_RFF;

		if (h63484->fifo_ptr == -1)
			h63484->sr &= H63484_SR_RFR;

	}
}

/*-------------------------------------------------
    ROM( h63484 )
-------------------------------------------------*/

ROM_START( h63484 )
	ROM_REGION( 0x100, "h63484", 0 )
	ROM_LOAD( "h63484.bin", 0x000, 0x100, NO_DUMP ) /* internal control ROM */
ROM_END

/*-------------------------------------------------
    ADDRESS_MAP( upd7220 )
-------------------------------------------------*/

READ8_DEVICE_HANDLER( h63484_vram_r )
{
	h63484_state *h63484 = get_safe_token(device);

	return h63484->vram[offset];
}

WRITE8_DEVICE_HANDLER( h63484_vram_w )
{
	h63484_state *h63484 = get_safe_token(device);

	h63484->vram[offset] = data;
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static int translate_command(UINT16 data)
{
	/* annoying switch-case sequence, but it's the only way to get invalid commands ... */
	switch (data)
	{
		case H63484_COMMAND_ORG: 	return COMMAND_ORG;
		case H63484_COMMAND_DRD: 	return COMMAND_DRD;
		case H63484_COMMAND_DWT: 	return COMMAND_DWT;
		case H63484_COMMAND_RD:  	return COMMAND_RD;
		case H63484_COMMAND_WT:  	return COMMAND_WT;
		case H63484_COMMAND_CLR: 	return COMMAND_CLR;
		case H63484_COMMAND_AMOVE:	return COMMAND_AMOVE;
		case H63484_COMMAND_RMOVE:	return COMMAND_RMOVE;
	}

	switch(data & ~0x3)
	{
		case H63484_COMMAND_DMOD:	return COMMAND_DMOD;
		case H63484_COMMAND_MOD: 	return COMMAND_MOD;
		case H63484_COMMAND_SCLR:	return COMMAND_SCLR;
	}

	switch(data & ~0xf)
	{
		case H63484_COMMAND_WPTN:	return COMMAND_WPTN;
		case H63484_COMMAND_RPTN:	return COMMAND_RPTN;
	}

	switch(data & ~0x1f)
	{
		case H63484_COMMAND_WPR:	return COMMAND_WPR;
		case H63484_COMMAND_RPR:	return COMMAND_RPR;
	}

	switch(data & ~0x0f03)
	{
		case H63484_COMMAND_CPY:	return COMMAND_CPY;
		case H63484_COMMAND_SCPY:	return COMMAND_SCPY;
	}

	switch(data & ~0x00ff)
	{
		case H63484_COMMAND_ALINE:	return COMMAND_ALINE;
		case H63484_COMMAND_RLINE:	return COMMAND_RLINE;
		case H63484_COMMAND_ARCT:	return COMMAND_ARCT;
		case H63484_COMMAND_RRCT:	return COMMAND_RRCT;
		case H63484_COMMAND_APLL:	return COMMAND_APLL;
		case H63484_COMMAND_RPLL:	return COMMAND_RPLL;
		case H63484_COMMAND_APLG:	return COMMAND_APLG;
		case H63484_COMMAND_RPLG:	return COMMAND_RPLG;
		case H63484_COMMAND_AFRCT:	return COMMAND_AFRCT;
		case H63484_COMMAND_RFRCT:	return COMMAND_RFRCT;
		case H63484_COMMAND_DOT:	return COMMAND_DOT;
	}

	switch(data & ~0x01ff)
	{
		case H63484_COMMAND_CRCL:	return COMMAND_CRCL;
		case H63484_COMMAND_ELPS:	return COMMAND_ELPS;
		case H63484_COMMAND_AARC:	return COMMAND_AARC;
		case H63484_COMMAND_RARC:	return COMMAND_RARC;
		case H63484_COMMAND_AEARC:	return COMMAND_AEARC;
		case H63484_COMMAND_REARC:	return COMMAND_REARC;
		case H63484_COMMAND_PAINT:	return COMMAND_PAINT;
	}

	switch(data & ~0x0fff)
	{
		case H63484_COMMAND_PTN:	return COMMAND_PTN;
		case H63484_COMMAND_AGCPY:	return COMMAND_AGCPY;
		case H63484_COMMAND_RGCPY:	return COMMAND_RGCPY;
	}

	return COMMAND_INVALID;
}

static void command_end_seq(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);

	//h63484->param_ptr = 0;
	h63484->sr |= H63484_SR_CED;

	/* TODO: we might need to be more aggressive and clear the params in there */
}

static void command_wpr_exec(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);

	if(LOG) printf("%s -> %02x\n",wpr_regnames[h63484->cr & 0x1f],h63484->pr[0]);

	switch(h63484->cr & 0x1f)
	{
		case 0x0c: // Read Write Pointer H
			h63484->rwp_dn = (h63484->pr[0] & 0xc000) >> 14;
			h63484->rwp = (h63484->rwp & 0x00fff) | ((h63484->pr[0] & 0x00ff) << 12);
			break;
		case 0x0d: // Read Write Pointer L
			h63484->rwp = (h63484->rwp & 0xff000) | ((h63484->pr[0] & 0xfff0) >> 4);
			break;
	}
}

static void process_fifo(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);
	UINT8 data;

	dequeue_w(h63484, &data);

	if (h63484->sr & H63484_SR_CED)
	{
		h63484->cr = (data & 0xff) << 8;
		dequeue_w(h63484, &data);
		h63484->cr |= data & 0xff;
		h63484->param_ptr = 0;
		h63484->sr &= ~H63484_SR_CED;
	}
	else
	{
		h63484->pr[h63484->param_ptr] = (data & 0xff) << 8;
		dequeue_w(h63484, &data);
		h63484->pr[h63484->param_ptr] |= (data & 0xff);
		h63484->param_ptr++;
	}

	switch (translate_command(h63484->cr))
	{
		case COMMAND_INVALID:
			printf("H63484 '%s' Invalid Command Byte %02x\n", device->tag(), h63484->cr);
			h63484->sr |= H63484_SR_CER; // command error
			break;

		case COMMAND_WPR: // 0x0800 & ~0x1f
			if (h63484->param_ptr == 1)
			{
				printf("%04x\n",h63484->pr[0]);

				command_wpr_exec(device);
				command_end_seq(device);
			}
			break;

		case COMMAND_RD:
			if (h63484->param_ptr == 0)
			{
				printf("%08x %02x %02x\n", h63484->rwp,h63484->space->direct().read_raw_byte((h63484->rwp+0) & 0xfffff), h63484->space->direct().read_raw_byte((h63484->rwp+1) & 0xfffff));

				queue_r(h63484, h63484->space->read_byte((h63484->rwp+0) & 0xfffff));
				queue_r(h63484, h63484->space->read_byte((h63484->rwp+1) & 0xfffff));
				h63484->rwp+=2;
				command_end_seq(device);
			}
			break;
	}
}

static void exec_abort_sequence(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);

	fifo_w_clear(h63484);
	fifo_r_clear(h63484);
	h63484->sr = H63484_SR_WFR | H63484_SR_WFE | H63484_SR_CED; // hard-set to 0x23
}

static void check_video_registers(device_t *device, int offset)
{
	h63484_state *h63484 = get_safe_token(device);
	UINT16 vreg_data;

	vreg_data = (h63484->vreg[offset]<<8)|(h63484->vreg[offset+1]&0xff);

	switch(offset)
	{
		case 0x00: // FIFO entry, not covered there
			break;
		case CCR: // Command Entry
			if(vreg_data & ABT) // abort sequence
				exec_abort_sequence(device);
			break;
	}
}

READ16_DEVICE_HANDLER( h63484_status_r )
{
	h63484_state *h63484 = get_safe_token(device);

	return h63484->sr;
}

READ16_DEVICE_HANDLER( h63484_data_r )
{
	//h63484_state *h63484 = get_safe_token(device);
	int res;

	res = 0xffff;

	return res;
}

WRITE16_DEVICE_HANDLER( h63484_address_w )
{
	h63484_state *h63484 = get_safe_token(device);

	if(ACCESSING_BITS_0_7)
		h63484->ar = data & 0xff;
}

WRITE16_DEVICE_HANDLER( h63484_data_w )
{
	h63484_state *h63484 = get_safe_token(device);

	if(LOG) printf("%s -> %02x\n",acrtc_regnames[h63484->ar/2],data);

	if(ACCESSING_BITS_8_15)
		h63484->vreg[h63484->ar] = (data & 0xff00) >> 8;

	if(ACCESSING_BITS_0_7)
		h63484->vreg[h63484->ar+1] = (data & 0xff);

	if(h63484->ar == 0)
	{
		queue_w(h63484, (data & 0xff00) >> 8);
		queue_w(h63484, (data & 0x00ff) >> 0);
		process_fifo(device);
	}
	else
		check_video_registers(device,h63484->ar);

	if(h63484->ar & 0x80)
	{
		h63484->ar+=2;
		h63484->ar &= 0xff; // TODO: what happens if it overflows?
	}
}

static DEVICE_START( h63484 )
{
	h63484_state *h63484 = get_safe_token(device);
	const h63484_interface *intf = get_interface(device);

	h63484->display_func = intf->display_func;

	h63484->screen = device->machine().device<screen_device>(intf->screen_tag);
	assert(h63484->screen != NULL);

	h63484->space = device->memory().space(AS_0);
	h63484->vram = auto_alloc_array_clear(device->machine(), UINT8, 1 << 20);
}

static DEVICE_RESET( h63484 )
{
	//h63484_state *h63484 = get_safe_token(device);

	//h63484->fifo_counter = 0;
}

DEVICE_GET_INFO( h63484 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(h63484_state);					break;
		case DEVINFO_INT_DATABUS_WIDTH_0:		info->i = 8;									break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:		info->i = 20;									break;
		case DEVINFO_INT_ADDRBUS_SHIFT_0:		info->i = -1;									break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(h63484);				break;

		/* --- the following bits of info are returned as pointers to data --- */
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:	info->default_map8 = NULL; 						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(h63484);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(h63484);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Hitachi 63484");				break;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, "h63484");						break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Hitachi 63484 ACRTC");			break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_MEMORY_DEVICE(H63484, h63484);
