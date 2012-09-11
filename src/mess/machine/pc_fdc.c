/**********************************************************************

    PC-style floppy disk controller emulation

    TODO:
        - check how the drive select from DOR register, and the drive select
        from the fdc are related !!!!
        - if all drives do not have a disk in them, and the fdc is reset, is a int generated?
        (if yes, indicates drives are ready without discs, if no indicates no drives are ready)
        - status register a, status register b

**********************************************************************/

#include "emu.h"
#include "machine/pc_fdc.h"
#include "machine/upd765.h"


/* if not 1, DACK and TC inputs to FDC are disabled, and DRQ and IRQ are held
 * at high impedance i.e they are not affective */
#define PC_FDC_FLAGS_DOR_DMA_ENABLED	(1<<3)
#define PC_FDC_FLAGS_DOR_FDC_ENABLED	(1<<2)
#define PC_FDC_FLAGS_DOR_MOTOR_ON		(1<<4)

#define LOG_FDC		0

/* registers etc */
struct pc_fdc
{
	int status_register_a;
	int status_register_b;
	int digital_output_register;
	int tape_drive_register;
	int data_rate_register;
	int digital_input_register;
	int configuration_control_register;

	/* stored tc state - state present at pins */
	int tc_state;
	/* stored dma drq state */
	int dma_state;
	/* stored int state */
	int int_state;

	/* PCJR watchdog timer */
	emu_timer	*watchdog;

	struct pc_fdc_interface fdc_interface;
};

static struct pc_fdc *fdc;

/* Prototypes */

static TIMER_CALLBACK( watchdog_timeout );

static WRITE_LINE_DEVICE_HANDLER(  pc_fdc_hw_interrupt );
static WRITE_LINE_DEVICE_HANDLER( pc_fdc_hw_dma_drq );
static UPD765_GET_IMAGE ( pc_fdc_get_image );
static UPD765_GET_IMAGE ( pcjr_fdc_get_image );

const upd765_interface pc_fdc_upd765_connected_interface =
{
	DEVCB_LINE(pc_fdc_hw_interrupt),
	DEVCB_LINE(pc_fdc_hw_dma_drq),
	pc_fdc_get_image,
	UPD765_RDY_PIN_CONNECTED,
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

const upd765_interface pc_fdc_upd765_connected_1_drive_interface =
{
	DEVCB_LINE(pc_fdc_hw_interrupt),
	DEVCB_LINE(pc_fdc_hw_dma_drq),
	pc_fdc_get_image,
	UPD765_RDY_PIN_CONNECTED,
	{FLOPPY_0, NULL, NULL, NULL}
};


const upd765_interface pc_fdc_upd765_not_connected_interface =
{
	DEVCB_LINE(pc_fdc_hw_interrupt),
	DEVCB_LINE(pc_fdc_hw_dma_drq),
	pc_fdc_get_image,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

const upd765_interface pcjr_fdc_upd765_interface =
{
	DEVCB_LINE(pc_fdc_hw_interrupt),
	DEVCB_NULL,
	pcjr_fdc_get_image,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{FLOPPY_0, NULL, NULL, NULL}
};


static device_t* pc_get_device(running_machine &machine)
{
	return (*fdc->fdc_interface.get_device)(machine);
}

void pc_fdc_reset(running_machine &machine)
{
	/* setup reset condition */
	fdc->data_rate_register = 2;
	fdc->digital_output_register = 0;

	/* bit 7 is disk change */
	fdc->digital_input_register = 0x07f;

	upd765_reset(pc_get_device(machine),0);

	/* set FDC at reset */
	upd765_reset_w(pc_get_device(machine), 1);
}



void pc_fdc_init(running_machine &machine, const struct pc_fdc_interface *iface)
{
	/* initialize fdc structure */
	fdc = auto_alloc_clear(machine, struct pc_fdc);

	/* copy specified interface */
	if (iface)
		memcpy(&fdc->fdc_interface, iface, sizeof(fdc->fdc_interface));

	fdc->watchdog = machine.scheduler().timer_alloc(FUNC(watchdog_timeout));

	pc_fdc_reset(machine);
}



static UPD765_GET_IMAGE ( pc_fdc_get_image )
{
	device_t *image = NULL;

	if (!fdc->fdc_interface.get_image)
	{
		image = floppy_get_device(device->machine(), (fdc->digital_output_register & 0x03));
	}
	else
	{
		image = fdc->fdc_interface.get_image(device->machine(), (fdc->digital_output_register & 0x03));
	}
	return image;
}

static UPD765_GET_IMAGE ( pcjr_fdc_get_image )
{
	device_t *image = NULL;

	if (!fdc->fdc_interface.get_image)
	{
		image = floppy_get_device(device->machine(), 0);
	}
	else
	{
		image = fdc->fdc_interface.get_image(device->machine(), 0);
	}
	return image;
}

void pc_fdc_set_tc_state(running_machine &machine, int state)
{
	/* store state */
	fdc->tc_state = state;

	/* if dma is not enabled, tc's are not acknowledged */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		upd765_tc_w(pc_get_device(machine), state);
	}
}



static WRITE_LINE_DEVICE_HANDLER(  pc_fdc_hw_interrupt )
{
	fdc->int_state = state;

	/* if dma is not enabled, irq's are masked */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)==0)
		return;

	/* send irq */
	if (fdc->fdc_interface.pc_fdc_interrupt)
		fdc->fdc_interface.pc_fdc_interrupt(device->machine(), state);
}



int	pc_fdc_dack_r(running_machine &machine)
{
	int data;

	/* what is output if dack is not acknowledged? */
	data = 0x0ff;

	/* if dma is not enabled, dacks are not acknowledged */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		data = upd765_dack_r(pc_get_device(machine), 0);
	}

	return data;
}



void pc_fdc_dack_w(running_machine &machine, int data)
{
	/* if dma is not enabled, dacks are not issued */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		/* dma acknowledge - and send byte to fdc */
		upd765_dack_w(pc_get_device(machine), 0,data);
	}
}



static WRITE_LINE_DEVICE_HANDLER( pc_fdc_hw_dma_drq )
{
	fdc->dma_state = state;

	/* if dma is not enabled, drqs are masked */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)==0)
		return;

	if (fdc->fdc_interface.pc_fdc_dma_drq)
		fdc->fdc_interface.pc_fdc_dma_drq(device->machine(), state);
}



static void pc_fdc_data_rate_w(running_machine &machine, UINT8 data)
{
	if ((data & 0x080)!=0)
	{
		/* set ready state */
		upd765_ready_w(pc_get_device(machine),1);

		/* toggle reset state */
		upd765_reset_w(pc_get_device(machine), 1);
		upd765_reset_w(pc_get_device(machine), 0);

		/* bit is self-clearing */
		data &= ~0x080;
	}

	fdc->data_rate_register = data;
}



/*  FDC Digitial Output Register (DOR)

    |7|6|5|4|3|2|1|0|
     | | | | | | `------ floppy drive select (0=A, 1=B, 2=floppy C, ...)
     | | | | | `-------- 1 = FDC enable, 0 = hold FDC at reset
     | | | | `---------- 1 = DMA & I/O interface enabled
     | | | `------------ 1 = turn floppy drive A motor on
     | | `-------------- 1 = turn floppy drive B motor on
     | `---------------- 1 = turn floppy drive C motor on
     `------------------ 1 = turn floppy drive D motor on
 */

static void pc_fdc_dor_w(running_machine &machine, UINT8 data)
{
	int selected_drive;
	int floppy_count;

	floppy_count = floppy_get_count(machine);

	if (floppy_count > (fdc->digital_output_register & 0x03))
		floppy_drive_set_ready_state(floppy_get_device(machine, fdc->digital_output_register & 0x03), 1, 0);

	fdc->digital_output_register = data;

	selected_drive = data & 0x03;

	/* set floppy drive motor state */
	if (floppy_count > 0)
		floppy_mon_w(floppy_get_device(machine, 0), !BIT(data, 4));
	if (floppy_count > 1)
		floppy_mon_w(floppy_get_device(machine, 1), !BIT(data, 5));
	if (floppy_count > 2)
		floppy_mon_w(floppy_get_device(machine, 2), !BIT(data, 6));
	if (floppy_count > 3)
		floppy_mon_w(floppy_get_device(machine, 3), !BIT(data, 7));

	if ((data>>4) & (1<<selected_drive))
	{
		if (floppy_count > selected_drive)
			floppy_drive_set_ready_state(floppy_get_device(machine, selected_drive), 1, 0);
	}

	/* changing the DMA enable bit, will affect the terminal count state
    from reaching the fdc - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_set_tc_state(machine, fdc->tc_state);

	/* changing the DMA enable bit, will affect the dma drq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_dma_drq(pc_get_device(machine), fdc->dma_state);

	/* changing the DMA enable bit, will affect the irq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_interrupt(pc_get_device(machine), fdc->int_state);

	/* reset? */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_FDC_ENABLED)==0)
	{
		/* yes */

			/* pc-xt expects a interrupt to be generated
            when the fdc is reset.
            In the FDC docs, it states that a INT will
            be generated if READY input is true when the
            fdc is reset.

                It also states, that outputs to drive are set to 0.
                Maybe this causes the drive motor to go on, and therefore
                the ready line is set.

            This in return causes a int?? ---


        what is not yet clear is if this is a result of the drives ready state
        changing...
        */
			upd765_ready_w(pc_get_device(machine),1);

		/* set FDC at reset */
		upd765_reset_w(pc_get_device(machine), 1);
	}
	else
	{
		pc_fdc_set_tc_state(machine, 0);

		/* release reset on fdc */
		upd765_reset_w(pc_get_device(machine), 0);
	}
}

/*  PCJr FDC Digitial Output Register (DOR)

    On a PC Jr the DOR is wired up a bit differently:
    |7|6|5|4|3|2|1|0|
     | | | | | | | `--- Drive enable ( 0 = off, 1 = on )
     | | | | | | `----- Reserved
     | | | | | `------- Reserved
     | | | | `--------- Reserved
     | | | `----------- Reserved
     | | `------------- Watchdog Timer Enable ( 0 = watchdog disabled, 1 = watchdog enabled )
     | `--------------- Watchdog Timer Trigger ( on a 1->0 transition to strobe the trigger )
     `----------------- FDC Reset ( 0 = hold reset, 1 = release reset )
 */

static TIMER_CALLBACK( watchdog_timeout )
{
	/* Trigger a watchdog timeout signal */
	if ( fdc->fdc_interface.pc_fdc_interrupt && ( fdc->digital_output_register & 0x20 )  )
	{
		fdc->fdc_interface.pc_fdc_interrupt(machine, 1 );
	}
	else
	{
		fdc->fdc_interface.pc_fdc_interrupt(machine, 0 );
	}
}

static void pcjr_fdc_dor_w(running_machine &machine, UINT8 data)
{
	int floppy_count;

	floppy_count = floppy_get_count(machine);

	/* set floppy drive motor state */
	if (floppy_count > 0)
		floppy_mon_w(floppy_get_device(machine, 0), BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	if ( data & 0x01 )
	{
		if ( floppy_count )
			floppy_drive_set_ready_state(floppy_get_device(machine, 0), 1, 0);
	}

	/* Is the watchdog timer disabled */
	if ( ! ( data & 0x20 ) )
	{
		fdc->watchdog->adjust( attotime::never );
		if ( fdc->fdc_interface.pc_fdc_interrupt )
		{
			fdc->fdc_interface.pc_fdc_interrupt(machine, 0 );
		}
	} else {
		/* Check for 1->0 watchdog trigger */
		if ( ( fdc->digital_output_register & 0x40 ) && ! ( data & 0x40 ) )
		{
			/* Start watchdog timer here */
			fdc->watchdog->adjust( attotime::from_seconds(3) );
		}
	}

	/* reset? */
	if ( ! (data & 0x80) )
	{
		/* yes */

			/* pc-xt expects a interrupt to be generated
            when the fdc is reset.
            In the FDC docs, it states that a INT will
            be generated if READY input is true when the
            fdc is reset.

                It also states, that outputs to drive are set to 0.
                Maybe this causes the drive motor to go on, and therefore
                the ready line is set.

            This in return causes a int?? ---


        what is not yet clear is if this is a result of the drives ready state
        changing...
        */
			upd765_ready_w(pc_get_device(machine),1);

		/* set FDC at reset */
		upd765_reset_w(pc_get_device(machine), 1);
	}
	else
	{
		pc_fdc_set_tc_state(machine, 0);

		/* release reset on fdc */
		upd765_reset_w(pc_get_device(machine), 0);
	}

	logerror("pcjr_fdc_dor_w: changing dor from %02x to %02x\n", fdc->digital_output_register, data);

	fdc->digital_output_register = data;
}

#define RATE_250  2
#define RATE_300  1
#define RATE_500  0
#define RATE_1000 3

static void pc_fdc_check_data_rate(running_machine &machine)
{
	device_t *device = floppy_get_device(machine, fdc->digital_output_register & 0x03);
	floppy_image_legacy *image;
	int tracks, sectors, rate;

	upd765_set_bad(pc_get_device(machine), 0); // unset in case format is unknown
	if (!device) return;
	image = flopimg_get_image(device);
	if (!image) return;
	tracks = floppy_get_tracks_per_disk(image);
	tracks -= (tracks % 10); // ignore extra tracks
	floppy_get_sector_count(image, 0, 0, &sectors);

	if (tracks == 40) {
		if ((fdc->data_rate_register != RATE_250) && (fdc->data_rate_register != RATE_300))
			upd765_set_bad(pc_get_device(machine), 1);
		return;
	} else if (tracks == 80) {
		if (sectors <= 14)      rate = RATE_250;    // 720KB 5 1/4 and 3 1/2
		else if (sectors <= 24) rate = RATE_500;    // 1.2MB 5 1/4 and 1.44MB 3 1/2
		else                    rate = RATE_1000;   // 2.88MB 3 1/2
	} else return;

	if (rate != (fdc->data_rate_register & 3))
		upd765_set_bad(pc_get_device(machine), 1);
}

READ8_HANDLER ( pc_fdc_r )
{
	UINT8 data = 0xff;

	switch(offset)
	{
		case 0: /* status register a */
		case 1: /* status register b */
			data = 0x00;
			break;
		case 2:
			data = fdc->digital_output_register;
			break;
		case 3: /* tape drive select? */
			break;
		case 4:
			data = upd765_status_r(pc_get_device(space->machine()), 0);
			break;
		case 5:
			data = upd765_data_r(pc_get_device(space->machine()), offset);
			break;
		case 6: /* FDC reserved */
			break;
		case 7:
			device_t *dev = floppy_get_device(space->machine(), fdc->digital_output_register & 0x03);
			data = fdc->digital_input_register;
			if(dev) data |= (!floppy_dskchg_r(dev)<<7);
			break;
    }

	if (LOG_FDC)
		logerror("pc_fdc_r(): pc=0x%08x offset=%d result=0x%02X\n", (unsigned) space->machine().firstcpu->pc(), offset, data);
	return data;
}



WRITE8_HANDLER ( pc_fdc_w )
{
	if (LOG_FDC)
		logerror("pc_fdc_w(): pc=0x%08x offset=%d data=0x%02X\n", (unsigned) space->machine().firstcpu->pc(), offset, data);

	pc_fdc_check_data_rate(space->machine());  // check every time a command may start
	switch(offset)
	{
		case 0:	/* n/a */
		case 1:	/* n/a */
			break;
		case 2:
			pc_fdc_dor_w(space->machine(), data);
			break;
		case 3:
			/* tape drive select? */
			break;
		case 4:
			pc_fdc_data_rate_w(space->machine(), data);
			break;
		case 5:
			upd765_data_w(pc_get_device(space->machine()), 0, data);
			break;
		case 6:
			/* FDC reserved */
			break;
		case 7:
			/* Configuration Control Register
             *
             * Currently unimplemented; bits 1-0 are supposed to control data
             * flow rates:
             *      0 0      500 kbps
             *      0 1      300 kbps
             *      1 0      250 kbps
             *      1 1     1000 kbps
             */
			pc_fdc_data_rate_w(space->machine(), data & 3);
			break;
	}
}

WRITE8_HANDLER ( pcjr_fdc_w )
{
	if (LOG_FDC)
		logerror("pcjr_fdc_w(): pc=0x%08x offset=%d data=0x%02X\n", (unsigned) space->machine().firstcpu->pc(), offset, data);

	switch(offset)
	{
		case 2:
			pcjr_fdc_dor_w( space->machine(), data );
			break;
		case 4:
		case 7:
			break;
		default:
			pc_fdc_w( space, offset, data );
			break;
	}
}

