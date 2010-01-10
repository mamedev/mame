#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "includes/beezer.h"

static int pbus;

static READ8_DEVICE_HANDLER( b_via_0_pb_r );
static WRITE8_DEVICE_HANDLER( b_via_0_pa_w );
static WRITE8_DEVICE_HANDLER( b_via_0_pb_w );
static READ_LINE_DEVICE_HANDLER( b_via_0_ca2_r );
static WRITE_LINE_DEVICE_HANDLER( b_via_0_ca2_w );

static READ8_DEVICE_HANDLER( b_via_1_pa_r );
static READ8_DEVICE_HANDLER( b_via_1_pb_r );
static WRITE8_DEVICE_HANDLER( b_via_1_pa_w );
static WRITE8_DEVICE_HANDLER( b_via_1_pb_w );

const via6522_interface b_via_0_interface =
{
	/*inputs : A/B         */ DEVCB_NULL, DEVCB_HANDLER(b_via_0_pb_r),
	/*inputs : CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_DEVICE_LINE("via6522_1", via_ca2_r), DEVCB_LINE(b_via_0_ca2_r), DEVCB_DEVICE_LINE("via6522_1", via_ca1_r),
	/*outputs: A/B         */ DEVCB_HANDLER(b_via_0_pa_w), DEVCB_HANDLER(b_via_0_pb_w),
	/*outputs: CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_LINE(b_via_0_ca2_w), DEVCB_DEVICE_LINE("via6522_1", via_ca1_w),
	/*irq                  */ DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)
};

const via6522_interface b_via_1_interface =
{
	/*inputs : A/B         */ DEVCB_HANDLER(b_via_1_pa_r), DEVCB_HANDLER(b_via_1_pb_r),
	/*inputs : CA/B1,CA/B2 */ DEVCB_DEVICE_LINE("via6522_0", via_cb2_r), DEVCB_NULL, DEVCB_DEVICE_LINE("via6522_0", via_cb1_r), DEVCB_NULL,
	/*outputs: A/B         */ DEVCB_HANDLER(b_via_1_pa_w), DEVCB_HANDLER(b_via_1_pb_w),
	/*outputs: CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_DEVICE_LINE("via6522_0", via_cb1_w), DEVCB_NULL,
	/*irq                  */ DEVCB_CPU_INPUT_LINE("audiocpu", M6809_IRQ_LINE)
};

static READ_LINE_DEVICE_HANDLER( b_via_0_ca2_r )
{
	return 0;
}

static WRITE_LINE_DEVICE_HANDLER( b_via_0_ca2_w )
{
}

static READ8_DEVICE_HANDLER( b_via_0_pb_r )
{
	return pbus;
}

static WRITE8_DEVICE_HANDLER( b_via_0_pa_w )
{
	if ((data & 0x08) == 0)
		cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
	else
		cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);

	if ((data & 0x04) == 0)
	{
		switch (data & 0x03)
		{
		case 0:
			pbus = input_port_read(device->machine, "IN0");
			break;
		case 1:
			pbus = input_port_read(device->machine, "IN1") | (input_port_read(device->machine, "IN2") << 4);
			break;
		case 2:
			pbus = input_port_read(device->machine, "DSWB");
			break;
		case 3:
			pbus = 0xff;
			break;
		}
	}
}

static WRITE8_DEVICE_HANDLER( b_via_0_pb_w )
{
	pbus = data;
}

static READ8_DEVICE_HANDLER( b_via_1_pa_r )
{
	return pbus;
}

static READ8_DEVICE_HANDLER( b_via_1_pb_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( b_via_1_pa_w )
{
	pbus = data;
}

static WRITE8_DEVICE_HANDLER( b_via_1_pb_w )
{
}

DRIVER_INIT( beezer )
{
	pbus = 0;
}

WRITE8_HANDLER( beezer_bankswitch_w )
{
	if ((data & 0x07) == 0)
	{
		const device_config *via_0 = devtag_get_device(space->machine, "via6522_0");
		memory_install_write8_handler(space, 0xc600, 0xc7ff, 0, 0, watchdog_reset_w);
		memory_install_write8_handler(space, 0xc800, 0xc9ff, 0, 0, beezer_map_w);
		memory_install_read8_handler(space, 0xca00, 0xcbff, 0, 0, beezer_line_r);
		memory_install_readwrite8_device_handler(space, via_0, 0xce00, 0xcfff, 0, 0, via_r, via_w);
	}
	else
	{
		UINT8 *rom = memory_region(space->machine, "maincpu") + 0x10000;
		memory_install_ram(space, 0xc000, 0xcfff, 0, 0, rom + (data & 0x07) * 0x2000 + ((data & 0x08) ? 0x1000: 0));
	}
}
