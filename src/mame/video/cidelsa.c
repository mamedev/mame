#include "emu.h"
#include "sound/cdp1869.h"
#include "sound/ay8910.h"
#include "includes/cidelsa.h"

/* Page RAM Access */

static READ8_DEVICE_HANDLER( cidelsa_pageram_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT16 addr = offset & CIDELSA_PAGERAM_MASK;

	if (BIT(offset, 10))
	{
		return 0xff;
	}

	return state->pageram[addr];
}

static WRITE8_DEVICE_HANDLER( cidelsa_pageram_w )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT16 addr = offset & CIDELSA_PAGERAM_MASK;

	if (BIT(offset, 10))
	{
		return;
	}

	state->pageram[addr] = data;
}

static READ8_DEVICE_HANDLER( draco_pageram_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT16 addr = offset & DRACO_PAGERAM_MASK;

	return state->pageram[addr];
}

static WRITE8_DEVICE_HANDLER( draco_pageram_w )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT16 addr = offset & DRACO_PAGERAM_MASK;

	state->pageram[addr] = data;
}

/* Character RAM Access */

static CDP1869_CHAR_RAM_READ( cidelsa_charram_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = BIT(pma, 10) ? 0xff : state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->charram[addr];
	state->cdp1869_pcb = state->pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE( cidelsa_charram_w )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = BIT(pma, 10) ? 0xff : state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->charram[addr] = data;
	state->pcbram[addr] = state->cdp1802_q;
}

static CDP1869_CHAR_RAM_READ( draco_charram_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	UINT8 data = state->charram[addr];
	state->cdp1869_pcb = state->pcbram[addr];

	return data;
}

static CDP1869_CHAR_RAM_WRITE( draco_charram_w )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	state->charram[addr] = data;
	state->pcbram[addr] = state->cdp1802_q;
}

/* Page Color Bit Access */

static CDP1869_PCB_READ( cidelsa_pcb_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = state->pageram[pma & CIDELSA_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->pcbram[addr];
}

static CDP1869_PCB_READ( draco_pcb_r )
{
	cidelsa_state *state = (cidelsa_state *)device->machine->driver_data;

	UINT8 column = state->pageram[pma & DRACO_PAGERAM_MASK];
	UINT16 addr = ((column << 3) | (cma & 0x07)) & CIDELSA_CHARRAM_MASK;

	return state->pcbram[addr];
}

/* Predisplay Changed Handler */

static WRITE_LINE_DEVICE_HANDLER( cidelsa_prd_w )
{
	cidelsa_state *driver_state = (cidelsa_state *)device->machine->driver_data;

	/* invert PRD signal */
	cpu_set_input_line(driver_state->cdp1802, INPUT_LINE_IRQ0, !state);
	driver_state->cdp1869_prd = !state;
}

/* CDP1869 Interface */

static CDP1869_INTERFACE( destryer_cdp1869_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	DEVCB_HANDLER(cidelsa_pageram_r),
	DEVCB_HANDLER(cidelsa_pageram_w),
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	DEVCB_LINE(cidelsa_prd_w)
};

static CDP1869_INTERFACE( altair_cdp1869_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	DEVCB_HANDLER(cidelsa_pageram_r),
	DEVCB_HANDLER(cidelsa_pageram_w),
	cidelsa_pcb_r,
	cidelsa_charram_r,
	cidelsa_charram_w,
	DEVCB_LINE(cidelsa_prd_w)
};

static CDP1869_INTERFACE( draco_cdp1869_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	0,
	CDP1869_PAL,
	DEVCB_HANDLER(draco_pageram_r),
	DEVCB_HANDLER(draco_pageram_w),
	draco_pcb_r,
	draco_charram_r,
	draco_charram_w,
	DEVCB_NULL
};

/* Video Start */

static void video_start(running_machine *machine, UINT16 pageram_size)
{
	cidelsa_state *state = (cidelsa_state *)machine->driver_data;

	/* allocate memory */
	state->pageram = auto_alloc_array(machine, UINT8, pageram_size);
	state->pcbram = auto_alloc_array(machine, UINT8, CIDELSA_CHARRAM_SIZE);
	state->charram = auto_alloc_array(machine, UINT8, CIDELSA_CHARRAM_SIZE);

	/* find devices */
	state->cdp1869 = machine->device(CDP1869_TAG);

	/* register for state saving */
	state_save_register_global(machine, state->cdp1869_pcb);
	state_save_register_global_pointer(machine, state->pageram, pageram_size);
	state_save_register_global_pointer(machine, state->pcbram, CIDELSA_CHARRAM_SIZE);
	state_save_register_global_pointer(machine, state->charram, CIDELSA_CHARRAM_SIZE);
}

static VIDEO_START(cidelsa)
{
	video_start(machine, CIDELSA_PAGERAM_SIZE);
}

static VIDEO_START(draco)
{
	video_start(machine, DRACO_PAGERAM_SIZE);
}

/* Video Update */

static VIDEO_UPDATE( cidelsa )
{
	cidelsa_state *state = (cidelsa_state *)screen->machine->driver_data;

	cdp1869_update(state->cdp1869, bitmap, cliprect);

	return 0;
}

/* AY-3-8910 */

static WRITE8_DEVICE_HANDLER( draco_ay8910_port_b_w )
{
	/*

      bit   description

        0   RELE0
        1   RELE1
        2   sound output -> * -> 22K capacitor -> GND
        3   sound output -> * -> 220K capacitor -> GND
        4   5V -> 1K resistor -> * -> 10uF capacitor -> GND (volume pot voltage adjustment)
        5   not connected
        6   not connected
        7   not connected

    */
}

static const ay8910_interface ay8910_config =
{
	AY8910_SINGLE_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(draco_ay8910_port_b_w)
};

/* Machine Drivers */

MACHINE_DRIVER_START( destryer_video )
	MDRV_CDP1869_SCREEN_PAL_ADD(SCREEN_TAG, DESTRYER_CHR2)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_VIDEO_START(cidelsa)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_CDP1869_ADD(CDP1869_TAG, DESTRYER_CHR2, destryer_cdp1869_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( altair_video )
	MDRV_CDP1869_SCREEN_PAL_ADD(SCREEN_TAG, ALTAIR_CHR2)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_VIDEO_START(cidelsa)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_CDP1869_ADD(CDP1869_TAG, ALTAIR_CHR2, altair_cdp1869_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( draco_video )
	MDRV_CDP1869_SCREEN_PAL_ADD(SCREEN_TAG, DRACO_CHR2)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.360, 0.024)

	MDRV_VIDEO_START(draco)
	MDRV_VIDEO_UPDATE(cidelsa)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_CDP1869_ADD(CDP1869_TAG, DRACO_CHR2, draco_cdp1869_intf)
	MDRV_SOUND_ADD(AY8910_TAG, AY8910, DRACO_SND_CHR1)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END
