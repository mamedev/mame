/***************************************************************************

	HD63484 ACRTC (rewrite in progress)

***************************************************************************/

#include "emu.h"
#include "video/h63484.h"

#define LOG 1

typedef struct _h63484_state h63484_state;
struct _h63484_state
{
	h63484_display_pixels_func display_func;
	screen_device *screen;	/* screen */

	UINT8 vram[0x40000];
	UINT8 ar;
	UINT8 vreg[0x100];
	UINT8 sr;

	UINT8 fifo[16];					/* FIFO data queue */
	int fifo_flag[16];				/* FIFO flag queue */
	int fifo_ptr;					/* FIFO pointer */
	int fifo_dir;					/* FIFO direction */

	UINT8 cr;
	UINT8 pr[17];					/* parameter byte register */
	int param_ptr;					/* parameter pointer */
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

enum
{
	COMMAND_INVALID = -1
};

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


INLINE void fifo_clear(h63484_state *h63484)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		h63484->fifo[i] = 0;
		h63484->fifo_flag[i] = FIFO_EMPTY;
	}

	h63484->fifo_ptr = -1;

	h63484->sr |= H63484_SR_WFR;
	h63484->sr |= H63484_SR_WFE;
}

INLINE int fifo_param_count(h63484_state *h63484)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		if (h63484->fifo_flag[i] != FIFO_PARAMETER) break;
	}

	return i;
}

INLINE void fifo_set_direction(h63484_state *h63484, int dir)
{
	if (h63484->fifo_dir != dir)
	{
		fifo_clear(h63484);
	}

	h63484->fifo_dir = dir;
}

INLINE void queue(h63484_state *h63484, UINT8 data, int flag)
{
	if (h63484->fifo_ptr < 15)
	{
		h63484->fifo_ptr++;

		h63484->fifo[h63484->fifo_ptr] = data;
		h63484->fifo_flag[h63484->fifo_ptr] = flag;

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

INLINE void dequeue(h63484_state *h63484, UINT8 *data, int *flag)
{
	int i;

	*data = h63484->fifo[0];
	*flag = h63484->fifo_flag[0];

	if (h63484->fifo_ptr > -1)
	{
		for (i = 0; i < 15; i++)
		{
			h63484->fifo[i] = h63484->fifo[i + 1];
			h63484->fifo_flag[i] = h63484->fifo_flag[i + 1];
		}

		h63484->fifo[15] = 0;
		h63484->fifo_flag[15] = 0;

		h63484->fifo_ptr--;

		h63484->sr |= H63484_SR_WFR;

		if (h63484->fifo_ptr == -1)
			h63484->sr |= H63484_SR_WFE;

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

static int translate_command(UINT8 data)
{
	int command = COMMAND_INVALID;

	switch (data)
	{

	}

	return command;
}

static void process_fifo(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);
	UINT8 data;
	int flag;

	dequeue(h63484, &data, &flag);

	if (flag == FIFO_COMMAND)
	{
		h63484->cr = (data & 0xff00) >> 8;
		h63484->param_ptr = 1;
		h63484->sr &= ~H63484_SR_CED;
		dequeue(h63484, &data, &flag);
		h63484->pr[h63484->param_ptr] = data & 0xff;
		h63484->param_ptr++;
	}
	else
	{
		h63484->pr[h63484->param_ptr] = (data & 0xff00) >> 8;
		h63484->param_ptr++;
		dequeue(h63484, &data, &flag);
		h63484->pr[h63484->param_ptr] = data & 0xff;
		h63484->param_ptr++;
	}

	switch (translate_command(h63484->cr))
	{
		// ...
	}
}

static void exec_abort_sequence(device_t *device)
{
	h63484_state *h63484 = get_safe_token(device);

	fifo_clear(h63484);
	h63484->sr = H63484_SR_WFR | H63484_SR_WFE | H63484_SR_CED; // set to 0x23
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
		fifo_set_direction(h63484, FIFO_WRITE);
		queue(h63484, (data & 0xff00) >> 8, (h63484->sr & H63484_SR_CED) >> 5);
		queue(h63484, (data & 0x00ff) >> 0, 0);
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
		case DEVINFO_INT_ADDRBUS_WIDTH_0:		info->i = 19;									break;
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
