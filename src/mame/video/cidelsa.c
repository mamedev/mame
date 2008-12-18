#include "driver.h"
#include "video/cdp1869.h"
#include "cidelsa.h"

#define CIDELSA_PAGERAM_SIZE	0x400
#define DRACO_PAGERAM_SIZE		0x800
#define CIDELSA_CHARRAM_SIZE	0x800

#define CIDELSA_PAGERAM_MASK	0x3ff
#define DRACO_PAGERAM_MASK		0x7ff
#define CIDELSA_CHARRAM_MASK	0x7ff

#define SCREEN_TAG	"main"
#define CDP1869_TAG	"cdp1869"

/* Page RAM Access */

static CDP1869_PAGE_RAM_READ(cidelsa_pageram_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT16 addr = pma & CIDELSA_PAGERAM_MASK;

	if (BIT(pma, 10))
	{
		return 0xff;
	}

	return state->pageram[addr];
}

static CDP1869_PAGE_RAM_WRITE(cidelsa_pageram_w)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT16 addr = pma & CIDELSA_PAGERAM_MASK;

	if (BIT(pma, 10))
	{
		return;
	}

	state->pageram[addr] = data;
}

static CDP1869_PAGE_RAM_READ(draco_pageram_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT16 addr = pma & DRACO_PAGERAM_MASK;

	return state->pageram[addr];
}

static CDP1869_PAGE_RAM_WRITE(draco_pageram_w)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT16 addr = pma & DRACO_PAGERAM_MASK;

	state->pageram[addr] = data;
}

/* Character RAM Access */

static CDP1869_CHAR_RAM_READ(cidelsa_charram_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = BIT(pma, 10) ? 0xff : state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->charram[addr];
	state->cdp1869_pcb = state->pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE(cidelsa_charram_w)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = BIT(pma, 10) ? 0xff : state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->charram[addr] = data;
	state->pcbram[addr] = state->cdp1802_q;
}

static CDP1869_CHAR_RAM_READ(draco_charram_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->charram[addr];
	state->cdp1869_pcb = state->pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE(draco_charram_w)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->charram[addr] = data;
	state->pcbram[addr] = state->cdp1802_q;
}

/* Page Color Bit Access */

static CDP1869_PCB_READ(cidelsa_pcb_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->pcbram[addr];
}

static CDP1869_PCB_READ(draco_pcb_r)
{
	cidelsa_state *state = device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->pcbram[addr];
}

/* Predisplay Changed Handler */

static CDP1869_ON_PRD_CHANGED(cidelsa_prd_changed)
{
	cidelsa_state *state = device->machine->driver_data;

	// PRD is inverted

	cputag_set_input_line(device->machine, MAIN_CPU_TAG, INPUT_LINE_IRQ0, !prd);
	state->cdp1869_prd = !prd;
}

static CDP1869_ON_PRD_CHANGED(draco_prd_changed)
{
	cidelsa_state *state = device->machine->driver_data;

	state->cdp1869_prd = prd;
}

/* CDP1869 Interface */

static CDP1869_INTERFACE( destryer_cdp1869_intf )
{
	CDP1869_PAL,
	cidelsa_pageram_r,
	cidelsa_pageram_w,
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	cidelsa_prd_changed,
};

static CDP1869_INTERFACE( altair_cdp1869_intf )
{
	CDP1869_PAL,
	cidelsa_pageram_r,
	cidelsa_pageram_w,
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	cidelsa_prd_changed,
};

static CDP1869_INTERFACE( draco_cdp1869_intf )
{
	CDP1869_PAL,
	draco_pageram_r,
	draco_pageram_w,
	draco_pcb_r,
	draco_charram_r,
	draco_charram_w,
	draco_prd_changed,
};

/* Video Start */

static VIDEO_START(cidelsa)
{
	cidelsa_state *state = machine->driver_data;

	/* allocate memory */

	state->pageram = auto_malloc(CIDELSA_PAGERAM_SIZE);
	state->pcbram = auto_malloc(CIDELSA_CHARRAM_SIZE);
	state->charram = auto_malloc(CIDELSA_CHARRAM_SIZE);

	/* find devices */

	state->cdp1869 = devtag_get_device(machine, CDP1869_VIDEO, CDP1869_TAG);

	/* register for state saving */

	state_save_register_global(machine, state->cdp1869_prd);
	state_save_register_global(machine, state->cdp1869_pcb);
	state_save_register_global_pointer(machine, state->pageram, CIDELSA_PAGERAM_SIZE);
	state_save_register_global_pointer(machine, state->pcbram, CIDELSA_CHARRAM_SIZE);
	state_save_register_global_pointer(machine, state->charram, CIDELSA_CHARRAM_SIZE);
}

static VIDEO_START(draco)
{
	cidelsa_state *state = machine->driver_data;

	/* allocate memory */

	state->pageram = auto_malloc(DRACO_PAGERAM_SIZE);
	state->pcbram = auto_malloc(CIDELSA_CHARRAM_SIZE);
	state->charram = auto_malloc(CIDELSA_CHARRAM_SIZE);

	/* find devices */

	state->cdp1869 = devtag_get_device(machine, CDP1869_VIDEO, CDP1869_TAG);

	/* register for state saving */

	state_save_register_global(machine, state->cdp1869_prd);
	state_save_register_global(machine, state->cdp1869_pcb);
	state_save_register_global_pointer(machine, state->pageram, DRACO_PAGERAM_SIZE);
	state_save_register_global_pointer(machine, state->pcbram, CIDELSA_CHARRAM_SIZE);
	state_save_register_global_pointer(machine, state->charram, CIDELSA_CHARRAM_SIZE);
}

/* Video Update */

static VIDEO_UPDATE( cidelsa )
{
	cidelsa_state *state = screen->machine->driver_data;

	cdp1869_update(state->cdp1869, bitmap, cliprect);

	return 0;
}

/* Machine Drivers */

MACHINE_DRIVER_START( destryer_video )
	MDRV_PALETTE_LENGTH(CDP1869_PALETTE_LENGTH)
	MDRV_PALETTE_INIT(cdp1869)

	MDRV_VIDEO_START(cidelsa)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SCREEN_ADD(SCREEN_TAG, RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(DESTRYER_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_CDP1869_ADD(CDP1869_TAG, SCREEN_TAG, DESTRYER_CHR2, 0, MAIN_CPU_TAG, destryer_cdp1869_intf)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( altair_video )
	MDRV_PALETTE_LENGTH(CDP1869_PALETTE_LENGTH)
	MDRV_PALETTE_INIT(cdp1869)

	MDRV_VIDEO_START(cidelsa)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SCREEN_ADD(SCREEN_TAG, RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(ALTAIR_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_CDP1869_ADD(CDP1869_TAG, SCREEN_TAG, ALTAIR_CHR2, 0, MAIN_CPU_TAG, altair_cdp1869_intf)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( draco_video )
	MDRV_PALETTE_LENGTH(CDP1869_PALETTE_LENGTH)
	MDRV_PALETTE_INIT(cdp1869)

	MDRV_VIDEO_START(draco)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SCREEN_ADD(SCREEN_TAG, RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(DRACO_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.360, 0.024)

	MDRV_CDP1869_ADD(CDP1869_TAG, SCREEN_TAG, DRACO_CHR2, 0, MAIN_CPU_TAG, draco_cdp1869_intf)
MACHINE_DRIVER_END
