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

const device_type PC_FDC_XT = &device_creator<pc_fdc_xt_device>;
const device_type PC_FDC_AT = &device_creator<pc_fdc_at_device>;
const device_type PC_FDC_JR = &device_creator<pc_fdc_jr_device>;

static MACHINE_CONFIG_FRAGMENT( cfg )
	MCFG_UPD765A_ADD("upd765", false, false)
MACHINE_CONFIG_END

DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_family_device)
ADDRESS_MAP_END

// The schematics show address decoding is minimal
DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_xt_device)
	AM_RANGE(0x0, 0x0) AM_DEVREAD("upd765", upd765a_device, msr_r) AM_WRITE(dor_w)
	AM_RANGE(0x1, 0x1) AM_DEVREAD("upd765", upd765a_device, fifo_r) AM_WRITE(dor_fifo_w)
	AM_RANGE(0x2, 0x2) AM_WRITE(dor_w)
	AM_RANGE(0x3, 0x3) AM_WRITE(dor_w)
	AM_RANGE(0x4, 0x5) AM_DEVICE("upd765", upd765a_device, map)
ADDRESS_MAP_END


// Decoding is through a PAL, so presumably complete
DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_at_device)
	AM_RANGE(0x2, 0x2) AM_READWRITE(dor_r, dor_w)
	AM_RANGE(0x4, 0x5) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0x7, 0x7) AM_READWRITE(dir_r, ccr_w)
ADDRESS_MAP_END



pc_fdc_family_device::pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_interface(mconfig, type, name, tag, owner, clock), fdc(*this, "upd765")
{
}

void pc_fdc_family_device::setup_intrq_cb(line_cb cb)
{
	intrq_cb = cb;
}

void pc_fdc_family_device::setup_drq_cb(line_cb cb)
{
	drq_cb = cb;
}

void pc_fdc_family_device::tc_w(bool state)
{
	fdc->tc_w(state);
}

UINT8 pc_fdc_family_device::dma_r()
{
	return fdc->dma_r();
}

void pc_fdc_family_device::dma_w(UINT8 data)
{
	fdc->dma_w(data);
}

machine_config_constructor pc_fdc_family_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg);
}

void pc_fdc_family_device::device_start()
{
	fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(pc_fdc_family_device::irq_w), this));
	fdc->setup_drq_cb(upd765a_device::line_cb(FUNC(pc_fdc_family_device::drq_w), this));

	for(int i=0; i<4; i++) {
		char name[2] = {'0'+i, 0};
		floppy_connector *conn = subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : NULL;
	}

	irq = drq = false;
	fdc_irq = fdc_drq = false;
	dor = 0x00;
}

void pc_fdc_family_device::device_reset()
{
}

// Bits 0-1 select one of the 4 drives, but only if the associated
// motor bit is on

// Bit 2 is tied to the upd765 reset line

// Bit 3 enables the irq and drq lines

// Bit 4-7 control the drive motors

WRITE8_MEMBER( pc_fdc_family_device::dor_w )
{
	logerror("%s: dor = %02x\n", tag(), data);
	UINT8 pdor = dor;
	dor = data;

	for(int i=0; i<4; i++)
		if(floppy[i])
			floppy[i]->mon_w(!(dor & (0x10 << i)));

	int fid = dor & 3;
	if(dor & (0x10 << fid))
		fdc->set_floppy(floppy[fid]);
	else
		fdc->set_floppy(NULL);

	check_irq();
	check_drq();
	if((pdor^dor) & 4)
		fdc->reset();
}

READ8_MEMBER( pc_fdc_family_device::dor_r )
{
	return dor;
}

READ8_MEMBER( pc_fdc_family_device::dir_r )
{
	return do_dir_r();
}

WRITE8_MEMBER( pc_fdc_family_device::ccr_w )
{
	static const int rates[4] = { 500000, 300000, 250000, 1000000 };
	logerror("%s: ccr = %02x\n", tag(), data);
	fdc->set_rate(rates[data & 3]);
}

UINT8 pc_fdc_family_device::do_dir_r()
{
	if(floppy[dor & 3])
		return floppy[dor & 3]->dskchg_r() ? 0x00 : 0x80;
	return 0x00;
}

WRITE8_MEMBER( pc_fdc_xt_device::dor_fifo_w)
{
	fdc->fifo_w(space, 0, data, mem_mask);
	dor_w(space, 0, data, mem_mask);
}

void pc_fdc_family_device::irq_w(bool state)
{
	fdc_irq = state;
	check_irq();
}

void pc_fdc_family_device::drq_w(bool state)
{
	fdc_drq = state;
	check_drq();
}

void pc_fdc_family_device::check_irq()
{
	bool pirq = irq;
	irq = fdc_irq && (dor & 4) && (dor & 8);
	if(irq != pirq && !intrq_cb.isnull()) {
		logerror("%s: pc_irq = %d\n", tag(), irq);
		intrq_cb(irq);
	}
}

void pc_fdc_family_device::check_drq()
{
	bool pdrq = drq;
	drq = fdc_drq && (dor & 4) && (dor & 8);
	if(drq != pdrq && !drq_cb.isnull())
		drq_cb(drq);
}

pc_fdc_xt_device::pc_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_family_device(mconfig, PC_FDC_XT, "PC FDC XT", tag, owner, clock)
{
	m_shortname = "pc_fdc_xt";
}

pc_fdc_at_device::pc_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_family_device(mconfig, PC_FDC_AT, "PC FDC AT", tag, owner, clock)
{
	m_shortname = "pc_fdc_at";
}

pc_fdc_jr_device::pc_fdc_jr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_family_device(mconfig, PC_FDC_JR, "PC FDC JR", tag, owner, clock)
{
	m_shortname = "pc_fdc_jr";
}

#if 0


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



int	pc_fdc_dack_r(running_machine &machine, address_space &space)
{
	int data;

	/* what is output if dack is not acknowledged? */
	data = 0x0ff;

	/* if dma is not enabled, dacks are not acknowledged */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		data = upd765_dack_r(pc_get_device(machine), space, 0);
	}

	return data;
}



void pc_fdc_dack_w(running_machine &machine, address_space &space, int data)
{
	/* if dma is not enabled, dacks are not issued */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		/* dma acknowledge - and send byte to fdc */
		upd765_dack_w(pc_get_device(machine), space, 0,data);
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

static WRITE8_HANDLER( pc_fdc_dor_w )
{
	int selected_drive;
	int floppy_count;

	floppy_count = floppy_get_count(space.machine());

	if (floppy_count > (fdc->digital_output_register & 0x03))
		floppy_drive_set_ready_state(floppy_get_device(space.machine(), fdc->digital_output_register & 0x03), 1, 0);

	fdc->digital_output_register = data;

	selected_drive = data & 0x03;

	/* set floppy drive motor state */
	if (floppy_count > 0)
		floppy_mon_w(floppy_get_device(space.machine(), 0), !BIT(data, 4));
	if (floppy_count > 1)
		floppy_mon_w(floppy_get_device(space.machine(), 1), !BIT(data, 5));
	if (floppy_count > 2)
		floppy_mon_w(floppy_get_device(space.machine(), 2), !BIT(data, 6));
	if (floppy_count > 3)
		floppy_mon_w(floppy_get_device(space.machine(), 3), !BIT(data, 7));

	if ((data>>4) & (1<<selected_drive))
	{
		if (floppy_count > selected_drive)
			floppy_drive_set_ready_state(floppy_get_device(space.machine(), selected_drive), 1, 0);
	}

	/* changing the DMA enable bit, will affect the terminal count state
    from reaching the fdc - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_set_tc_state(space.machine(), fdc->tc_state);

	/* changing the DMA enable bit, will affect the dma drq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_dma_drq(pc_get_device(space.machine()), fdc->dma_state);

	/* changing the DMA enable bit, will affect the irq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_interrupt(pc_get_device(space.machine()), fdc->int_state);

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
			upd765_ready_w(pc_get_device(space.machine()),1);

		/* set FDC at reset */
		upd765_reset_w(pc_get_device(space.machine()), 1);
	}
	else
	{
		pc_fdc_set_tc_state(space.machine(), 0);

		/* release reset on fdc */
		upd765_reset_w(pc_get_device(space.machine()), 0);
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

static WRITE8_HANDLER( pcjr_fdc_dor_w )
{
	int floppy_count;

	floppy_count = floppy_get_count(space.machine());

	/* set floppy drive motor state */
	if (floppy_count > 0)
		floppy_mon_w(floppy_get_device(space.machine(), 0), BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	if ( data & 0x01 )
	{
		if ( floppy_count )
			floppy_drive_set_ready_state(floppy_get_device(space.machine(), 0), 1, 0);
	}

	/* Is the watchdog timer disabled */
	if ( ! ( data & 0x20 ) )
	{
		fdc->watchdog->adjust( attotime::never );
		if ( fdc->fdc_interface.pc_fdc_interrupt )
		{
			fdc->fdc_interface.pc_fdc_interrupt(space.machine(), 0 );
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
			upd765_ready_w(pc_get_device(space.machine()),1);

		/* set FDC at reset */
		upd765_reset_w(pc_get_device(space.machine()), 1);
	}
	else
	{
		pc_fdc_set_tc_state(space.machine(), 0);

		/* release reset on fdc */
		upd765_reset_w(pc_get_device(space.machine()), 0);
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
			data = upd765_status_r(pc_get_device(space.machine()), space, 0);
			break;
		case 5:
			data = upd765_data_r(pc_get_device(space.machine()), space, offset);
			break;
		case 6: /* FDC reserved */
			break;
		case 7:
			device_t *dev = floppy_get_device(space.machine(), fdc->digital_output_register & 0x03);
			data = fdc->digital_input_register;
			if(dev) data |= (!floppy_dskchg_r(dev)<<7);
			break;
    }

	if (LOG_FDC)
		logerror("pc_fdc_r(): pc=0x%08x offset=%d result=0x%02X\n", (unsigned) space.machine().firstcpu->pc(), offset, data);
	return data;
}



WRITE8_HANDLER ( pc_fdc_w )
{
	if (LOG_FDC)
		logerror("pc_fdc_w(): pc=0x%08x offset=%d data=0x%02X\n", (unsigned) space.machine().firstcpu->pc(), offset, data);

	pc_fdc_check_data_rate(space.machine());  // check every time a command may start
	switch(offset)
	{
		case 0:	/* n/a */
		case 1:	/* n/a */
			break;
		case 2:
			pc_fdc_dor_w(space, 0, data, mem_mask);
			break;
		case 3:
			/* tape drive select? */
			break;
		case 4:
			pc_fdc_data_rate_w(space.machine(), data);
			break;
		case 5:
			upd765_data_w(pc_get_device(space.machine()), space, 0, data);
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
			pc_fdc_data_rate_w(space.machine(), data & 3);
			break;
	}
}

WRITE8_HANDLER ( pcjr_fdc_w )
{
	if (LOG_FDC)
		logerror("pcjr_fdc_w(): pc=0x%08x offset=%d data=0x%02X\n", (unsigned) space.machine().firstcpu->pc(), offset, data);

	switch(offset)
	{
		case 2:
			pcjr_fdc_dor_w( space, 0, data, mem_mask );
			break;
		case 4:
		case 7:
			break;
		default:
			pc_fdc_w( space, offset, data );
			break;
	}
}

#endif
