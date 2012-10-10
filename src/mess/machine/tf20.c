/***************************************************************************

    Epson TF-20

    Dual floppy drive with HX-20 factory option


    Status: Boots from system disk, missing ??PD7201 emulation

***************************************************************************/

#include "emu.h"
#include "tf20.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/upd7201.h"
#include "machine/upd765.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define XTAL_CR1	XTAL_8MHz
#define XTAL_CR2	XTAL_4_9152MHz


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tf20_state
{
	device_t *cpu;
	ram_device *ram;
	upd765a_device *upd765a;
	upd7201_device *upd7201;
	floppy_image_device *floppy_0;
	floppy_image_device *floppy_1;

	void fdc_int(bool state) {
		cpu->execute().set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	}
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tf20_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TF20);

	return (tf20_state *)downcast<tf20_device *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* serial clock, 38400 baud by default */
static TIMER_DEVICE_CALLBACK( serial_clock )
{
	tf20_state *tf20 = get_safe_token(timer.owner());

	tf20->upd7201->rxca_w(ASSERT_LINE);
	tf20->upd7201->txca_w(ASSERT_LINE);
	tf20->upd7201->rxcb_w(ASSERT_LINE);
	tf20->upd7201->txcb_w(ASSERT_LINE);
}

/* a read from this location disables the rom */
static READ8_HANDLER( tf20_rom_disable )
{
	tf20_state *tf20 = get_safe_token(space.device().owner());
	address_space &prg = space.device().memory().space(AS_PROGRAM);

	/* switch in ram */
	prg.install_ram(0x0000, 0x7fff, tf20->ram->pointer());

	return 0xff;
}

static READ8_HANDLER( tf20_dip_r )
{
	logerror("%s: tf20_dip_r\n", space.machine().describe_context());

	return space.machine().root_device().ioport("tf20_dip")->read();
}

static TIMER_CALLBACK( tf20_upd765_tc_reset )
{
	static_cast<upd765a_device *>(ptr)->tc_w(false);
}

static READ8_DEVICE_HANDLER( tf20_upd765_tc_r )
{
	tf20_state *tf20 = get_safe_token(device->owner());
	logerror("%s: tf20_upd765_tc_r\n", device->machine().describe_context());

	/* toggle tc on read */
	tf20->upd765a->tc_w(true);
	space.machine().scheduler().timer_set(attotime::zero, FUNC(tf20_upd765_tc_reset), 0, device);

	return 0xff;
}

static WRITE8_HANDLER( tf20_fdc_control_w )
{
	tf20_state *tf20 = get_safe_token(space.device().owner());
	logerror("%s: tf20_fdc_control_w %02x\n", space.machine().describe_context(), data);

	/* bit 0, motor on signal */
	tf20->floppy_0->mon_w(!BIT(data, 0));
	tf20->floppy_1->mon_w(!BIT(data, 0));
}

static IRQ_CALLBACK( tf20_irq_ack )
{
	return 0x00;
}


/***************************************************************************
    EXTERNAL INTERFACE
***************************************************************************/

/* serial output signal (to the host computer) */
READ_LINE_DEVICE_HANDLER( tf20_rxs_r )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_rxs_r\n", device->machine().describe_context());

	return tf20->upd7201->txda_r();
}

READ_LINE_DEVICE_HANDLER( tf20_pins_r )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_pins_r\n", device->machine().describe_context());

	return tf20->upd7201->dtra_r();
}

/* serial input signal (from host computer) */
WRITE_LINE_DEVICE_HANDLER( tf20_txs_w )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_txs_w %u\n", device->machine().describe_context(), state);

	tf20->upd7201->rxda_w(state);
}

WRITE_LINE_DEVICE_HANDLER( tf20_pouts_w )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_pouts_w %u\n", device->machine().describe_context(), state);

	tf20->upd7201->ctsa_w(state);
}

#ifdef UNUSED_FUNCTION
/* serial output signal (to another terminal) */
WRITE_LINE_DEVICE_HANDLER( tf20_txc_w )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_txc_w %u\n", device->machine().describe_context(), state);

	tf20->upd7201->rxda_w(state);
}

/* serial input signal (from another terminal) */
READ_LINE_DEVICE_HANDLER( tf20_rxc_r )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_rxc_r\n", device->machine().describe_context());

	return tf20->upd7201->txda_r();
}

WRITE_LINE_DEVICE_HANDLER( tf20_poutc_w )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_poutc_w %u\n", device->machine().describe_context(), state);

	tf20->upd7201->ctsa_w(state);
}

READ_LINE_DEVICE_HANDLER( tf20_pinc_r )
{
	tf20_state *tf20 = get_safe_token(device);
	logerror("%s: tf20_pinc_r\n", device->machine().describe_context());

	return tf20->upd7201->dtra_r();
}
#endif


/*****************************************************************************
    ADDRESS MAPS
*****************************************************************************/

static ADDRESS_MAP_START( tf20_mem, AS_PROGRAM, 8, tf20_device )
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank21")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank22")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tf20_io, AS_IO, 8, tf20_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("3a", upd7201_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0xf6, 0xf6) AM_READ_LEGACY(tf20_rom_disable)
	AM_RANGE(0xf7, 0xf7) AM_READ_LEGACY(tf20_dip_r)
	AM_RANGE(0xf8, 0xf8) AM_DEVREAD_LEGACY("5a", tf20_upd765_tc_r) AM_WRITE_LEGACY(tf20_fdc_control_w)
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("5a", upd765a_device, map)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

INPUT_PORTS_START( tf20 )
	PORT_START("tf20_dip")
	PORT_DIPNAME(0x0f, 0x0f, "Drive extension")
	PORT_DIPLOCATION("TF-20 TFX:8,7,6,5")
	PORT_DIPSETTING(0x0f, "A & B Drive")
	PORT_DIPSETTING(0x07, "C & D Drive")
INPUT_PORTS_END


/*****************************************************************************
    MACHINE CONFIG
*****************************************************************************/

static UPD7201_INTERFACE( tf20_upd7201_intf )
{
	DEVCB_NULL,				/* interrupt: nc */
	{
		{
			XTAL_CR2 / 128,		/* receive clock: 38400 baud (default) */
			XTAL_CR2 / 128,		/* transmit clock: 38400 baud (default) */
			DEVCB_NULL,			/* receive DRQ */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_NULL,			/* clear to send */
			DEVCB_LINE_GND,		/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output: nc */
		}, {
			XTAL_CR2 / 128,		/* receive clock: 38400 baud (default) */
			XTAL_CR2 / 128,		/* transmit clock: 38400 baud (default) */
			DEVCB_NULL,			/* receive DRQ: nc */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_LINE_GND,		/* clear to send */
			DEVCB_LINE_GND,		/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready: nc */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output: nc */
		}
	}
};

static const floppy_format_type tf20_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( tf20_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( tf20 )
	MCFG_CPU_ADD("tf20", Z80, XTAL_CR1 / 2) /* uPD780C */
	MCFG_CPU_PROGRAM_MAP(tf20_mem)
	MCFG_CPU_IO_MAP(tf20_io)

	/* 64k internal ram */
	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("64k")

	/* upd765a floppy controller */
	MCFG_UPD765A_ADD("5a", false, true)

	/* upd7201 serial interface */
	MCFG_UPD7201_ADD("3a", XTAL_CR1 / 2, tf20_upd7201_intf)
	MCFG_TIMER_ADD_PERIODIC("serial_timer", serial_clock, attotime::from_hz(XTAL_CR2 / 128))

	/* 2 floppy drives */
	MCFG_FLOPPY_DRIVE_ADD("5a:0", tf20_floppies, "525dd", 0, tf20_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("5a:1", tf20_floppies, "525dd", 0, tf20_floppy_formats)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( tf20 )
	ROM_REGION(0x0800, "tf20", 0)
	ROM_LOAD("tfx.15e", 0x0000, 0x0800, CRC(af34f084) SHA1(c9bdf393f757ba5d8f838108ceb2b079be1d616e))
ROM_END


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tf20 )
{
	tf20_state *tf20 = get_safe_token(device);
	tf20->cpu = device->subdevice("tf20");
	address_space &prg = tf20->cpu->memory().space(AS_PROGRAM);

	tf20->cpu->execute().set_irq_acknowledge_callback(tf20_irq_ack);

	/* ram device */
	tf20->ram = device->subdevice<ram_device>("ram");

	/* make sure its already running */
	if (!tf20->ram->started())
		throw device_missing_dependencies();

	/* locate child devices */
	tf20->upd765a = device->subdevice<upd765a_device>("5a");
	tf20->upd7201 = downcast<upd7201_device *>(device->subdevice("3a"));
	tf20->floppy_0 = device->subdevice<floppy_connector>("5a:0")->get_device();
	tf20->floppy_1 = device->subdevice<floppy_connector>("5a:1")->get_device();

	/* hookup the irq */
	tf20->upd765a->setup_intrq_cb(upd765a_device::line_cb(FUNC(tf20_state::fdc_int), tf20));

	/* enable second half of ram */
	prg.install_ram(0x8000, 0xffff, tf20->ram->pointer() + 0x8000);
}

static DEVICE_RESET( tf20 )
{
	device_t *cpu = device->subdevice("tf20");
	address_space &prg = cpu->memory().space(AS_PROGRAM);

	/* enable rom */
	prg.install_rom(0x0000, 0x07ff, 0, 0x7800, cpu->region()->base());
}

const device_type TF20 = &device_creator<tf20_device>;

tf20_device::tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TF20, "TF-20", tag, owner, clock)
{
	m_token = global_alloc_clear(tf20_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tf20_device::device_config_complete()
{
	m_shortname = "tf20";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tf20_device::device_start()
{
	DEVICE_START_NAME( tf20 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tf20_device::device_reset()
{
	DEVICE_RESET_NAME( tf20 )(this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor tf20_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tf20  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *tf20_device::device_rom_region() const
{
	return ROM_NAME(tf20 );
}


