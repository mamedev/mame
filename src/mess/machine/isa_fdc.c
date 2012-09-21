/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/

#include "emu.h"
#include "machine/isa_fdc.h"
#include "machine/upd765.h"
#include "imagedev/flopdrv.h"
#include "formats/pc_dsk.h"
#include "machine/idectrl.h"

static DECLARE_READ8_DEVICE_HANDLER ( pc_fdc_r );
static DECLARE_WRITE8_DEVICE_HANDLER ( pc_fdc_w );

/* if not 1, DACK and TC inputs to FDC are disabled, and DRQ and IRQ are held
 * at high impedance i.e they are not affective */
#define PC_FDC_FLAGS_DOR_DMA_ENABLED	(1<<3)
#define PC_FDC_FLAGS_DOR_FDC_ENABLED	(1<<2)
#define PC_FDC_FLAGS_DOR_MOTOR_ON		(1<<4)

#define LOG_FDC		0

static const floppy_interface ibmpc_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pc),
	"floppy_5_25",
	NULL
};

static WRITE_LINE_DEVICE_HANDLER( pc_fdc_hw_interrupt );
static WRITE_LINE_DEVICE_HANDLER( pc_fdc_hw_dma_drq );
static UPD765_GET_IMAGE ( pc_fdc_get_image );

const upd765_interface pc_fdc_upd765_not_connected_interface =
{
	DEVCB_LINE(pc_fdc_hw_interrupt),
	DEVCB_LINE(pc_fdc_hw_dma_drq),
	pc_fdc_get_image,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

static MACHINE_CONFIG_FRAGMENT( fdc_config )
	MCFG_UPD765A_ADD("upd765", pc_fdc_upd765_not_connected_interface)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(ibmpc_floppy_interface)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_FDC = &device_creator<isa8_fdc_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fdc_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_fdc_device - constructor
//-------------------------------------------------

isa8_fdc_device::isa8_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_FDC, "Diskette Drive Adapter", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this),
		m_upd765(*this, "upd765")
{
}

isa8_fdc_device::isa8_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock),
		device_isa8_card_interface(mconfig, *this),
		m_upd765(*this, "upd765")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_fdc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(this, 0x03f0, 0x03f7, 0, 0, FUNC(pc_fdc_r), FUNC(pc_fdc_w) );
	m_isa->set_dma_channel(2, this, TRUE);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_fdc_device::device_reset()
{
	status_register_a = 0;
	status_register_b = 0;
	digital_output_register = 0;
	tape_drive_register = 0;
	data_rate_register = 2;
	digital_input_register = 0x07f;
	configuration_control_register = 0;
	tc_state = 0;
	dma_state = 0;
	int_state = 0;

	upd765_reset(m_upd765,0);

	/* set FDC at reset */
	upd765_reset_w(m_upd765, 1);
}

static device_t *get_floppy_subdevice(device_t *device, int drive)
{
	switch(drive) {
		case 0 : return device->subdevice(FLOPPY_0);
		case 1 : return device->subdevice(FLOPPY_1);
		case 2 : return device->subdevice(FLOPPY_2);
		case 3 : return device->subdevice(FLOPPY_3);
	}
	return NULL;
}

static UPD765_GET_IMAGE ( pc_fdc_get_image )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device->owner());

	return get_floppy_subdevice(fdc, fdc->digital_output_register & 0x03);
}

static WRITE_LINE_DEVICE_HANDLER( pc_fdc_set_tc_state)
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device);
	/* store state */
	fdc->tc_state = state;

	/* if dma is not enabled, tc's are not acknowledged */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		upd765_tc_w(fdc->m_upd765, state);
	}
}

static WRITE_LINE_DEVICE_HANDLER(  pc_fdc_hw_interrupt )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device->owner());

	fdc->int_state = state;

	/* if dma is not enabled, irq's are masked */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)==0)
		return;

	// not masked, send interrupt request
	fdc->m_isa->irq6_w(state);
}


static WRITE_LINE_DEVICE_HANDLER( pc_fdc_hw_dma_drq )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device->owner());
	fdc->dma_state = state;

	/* if dma is not enabled, drqs are masked */
	if ((fdc->digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)==0)
		return;

	// not masked, send dma request
	fdc->m_isa->drq2_w(state);
}



static WRITE8_DEVICE_HANDLER( pc_fdc_data_rate_w )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device);
	if ((data & 0x080)!=0)
	{
		/* set ready state */
		upd765_ready_w(fdc->m_upd765,1);

		/* toggle reset state */
		upd765_reset_w(fdc->m_upd765, 1);
		upd765_reset_w(fdc->m_upd765, 0);

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

static WRITE8_DEVICE_HANDLER( pc_fdc_dor_w )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device);
	int selected_drive;
	int floppy_count;

	floppy_count = floppy_get_count(space.machine());

	if (floppy_count > (fdc->digital_output_register & 0x03))
		floppy_drive_set_ready_state(get_floppy_subdevice(device, fdc->digital_output_register & 0x03), 1, 0);

	fdc->digital_output_register = data;

	selected_drive = data & 0x03;

	/* set floppy drive motor state */
	if (floppy_count > 0)
		floppy_mon_w(get_floppy_subdevice(device, 0), !BIT(data, 4));
	if (floppy_count > 1)
		floppy_mon_w(get_floppy_subdevice(device, 1), !BIT(data, 5));
	if (floppy_count > 2)
		floppy_mon_w(get_floppy_subdevice(device, 2), !BIT(data, 6));
	if (floppy_count > 3)
		floppy_mon_w(get_floppy_subdevice(device, 3), !BIT(data, 7));

	if ((data>>4) & (1<<selected_drive))
	{
		if (floppy_count > selected_drive)
			floppy_drive_set_ready_state(get_floppy_subdevice(device, selected_drive), 1, 0);
	}

	/* changing the DMA enable bit, will affect the terminal count state
    from reaching the fdc - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_set_tc_state(device, fdc->tc_state);

	/* changing the DMA enable bit, will affect the dma drq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_dma_drq(fdc->m_upd765, fdc->dma_state);

	/* changing the DMA enable bit, will affect the irq state
    from reaching us - if dma is enabled this will send it through
    otherwise it will be ignored */
	pc_fdc_hw_interrupt(fdc->m_upd765, fdc->int_state);

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
		upd765_ready_w(fdc->m_upd765, 1);

		/* set FDC at reset */
		upd765_reset_w(fdc->m_upd765, 1);
	}
	else
	{
		pc_fdc_set_tc_state(device, 0);

		/* release reset on fdc */
		upd765_reset_w(fdc->m_upd765, 0);
	}
}

#define RATE_250  2
#define RATE_300  1
#define RATE_500  0
#define RATE_1000 3

static void pc_fdc_check_data_rate(isa8_fdc_device *fdc, running_machine &machine)
{
	device_t *device = get_floppy_subdevice(fdc, fdc->digital_output_register & 0x03);
	floppy_image_legacy *image;
	int tracks, sectors, rate;

	upd765_set_bad(fdc->m_upd765, 0); // unset in case format is unknown
	if (!device) return;
	image = flopimg_get_image(device);
	if (!image) return;
	tracks = floppy_get_tracks_per_disk(image);
	tracks -= (tracks % 10); // ignore extra tracks
	floppy_get_sector_count(image, 0, 0, &sectors);

	if (tracks == 40) {
		if ((fdc->data_rate_register != RATE_250) && (fdc->data_rate_register != RATE_300))
			upd765_set_bad(fdc->m_upd765, 1);
		return;
	} else if (tracks == 80) {
		if (sectors <= 14)      rate = RATE_250;    // 720KB 5 1/4 and 3 1/2
		else if (sectors <= 24) rate = RATE_500;    // 1.2MB 5 1/4 and 1.44MB 3 1/2
		else                    rate = RATE_1000;   // 2.88MB 3 1/2
	} else return;

	if (rate != (fdc->data_rate_register & 3))
		upd765_set_bad(fdc->m_upd765, 1);
}


static READ8_DEVICE_HANDLER ( pc_fdc_r )
{
	UINT8 data = 0xff;

	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device);
	device_t *hdd = NULL;
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
			data = upd765_status_r(fdc->m_upd765, space, 0);
			break;
		case 5:
			data = upd765_data_r(fdc->m_upd765, space, offset);
			break;
		case 6: /* FDC reserved */
			hdd = space.machine().device(":board3:ide:ide");
			if (hdd)
				data = ide_controller16_r(hdd, space, 0x3f6/2, 0x00ff);
			break;
		case 7:
			device_t *dev = get_floppy_subdevice(device, fdc->digital_output_register & 0x03);
			data = fdc->digital_input_register;
			if(dev!=NULL) data |= (!floppy_dskchg_r(dev)<<7);
			break;
    }

	if (LOG_FDC)
		logerror("pc_fdc_r(): pc=0x%08x offset=%d result=0x%02X\n", (unsigned) space.machine().firstcpu->pc(), offset, data);
	return data;
}



static WRITE8_DEVICE_HANDLER ( pc_fdc_w )
{
	isa8_fdc_device	*fdc  = downcast<isa8_fdc_device *>(device);

	if (LOG_FDC)
		logerror("pc_fdc_w(): pc=0x%08x offset=%d data=0x%02X\n", (unsigned) space.machine().firstcpu->pc(), offset, data);
	pc_fdc_check_data_rate(fdc,space.machine());  // check every time a command may start
	device_t *hdd = NULL;

	switch(offset)
	{
		case 0:	/* n/a */
		case 1:	/* n/a */
			break;
		case 2:
			pc_fdc_dor_w(device, space, 0, data, mem_mask);
			break;
		case 3:
			/* tape drive select? */
			break;
		case 4:
			pc_fdc_data_rate_w(device, space, 0, data, mem_mask);
			break;
		case 5:
			upd765_data_w(fdc->m_upd765, space, 0, data);
			break;
		case 6:
			/* FDC reserved */
			hdd = space.machine().device(":board3:ide:ide");
			if (hdd)
				ide_controller16_w(hdd, space, 0x3f6/2, data, 0x00ff);
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
			pc_fdc_data_rate_w(device, space, 0, data & 3, mem_mask);
			break;
	}
}

UINT8 isa8_fdc_device::dack_r(int line)
{
	/* what is output if dack is not acknowledged? */
	int data = 0x0ff;

	/* if dma is not enabled, dacks are not acknowledged */
	if ((digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		data = upd765_dack_r(m_upd765, machine().driver_data()->generic_space(), 0);
	}

	return data;
}

void isa8_fdc_device::dack_w(int line,UINT8 data)
{
	/* if dma is not enabled, dacks are not issued */
	if ((digital_output_register & PC_FDC_FLAGS_DOR_DMA_ENABLED)!=0)
	{
		/* dma acknowledge - and send byte to fdc */
		upd765_dack_w(m_upd765, machine().driver_data()->generic_space(), 0,data);
	}
}
void isa8_fdc_device::eop_w(int state)
{
	pc_fdc_set_tc_state( this, state);
}

static MACHINE_CONFIG_FRAGMENT( fdc_smc_config )
	MCFG_SMC37C78_ADD("upd765", pc_fdc_upd765_not_connected_interface)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(ibmpc_floppy_interface)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_FDC_SMC = &device_creator<isa8_fdc_smc_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_fdc_smc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fdc_smc_config );
}

//-------------------------------------------------
//  isa8_fdc_smc_device - constructor
//-------------------------------------------------

isa8_fdc_smc_device::isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        isa8_fdc_device(mconfig, ISA8_FDC_SMC, "Diskette Drive Adapter (SMC37C78)", tag, owner, clock)
{
}

