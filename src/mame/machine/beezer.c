#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "includes/beezer.h"

static int pbus;

static READ8_HANDLER( b_via_0_pb_r );
static WRITE8_HANDLER( b_via_0_pa_w );
static WRITE8_HANDLER( b_via_0_pb_w );
static READ8_HANDLER( b_via_0_ca2_r );
static WRITE8_HANDLER( b_via_0_ca2_w );
static void b_via_0_irq (running_machine *machine, int level);

static READ8_HANDLER( b_via_1_pa_r );
static READ8_HANDLER( b_via_1_pb_r );
static WRITE8_HANDLER( b_via_1_pa_w );
static WRITE8_HANDLER( b_via_1_pb_w );
static void b_via_1_irq (running_machine *machine, int level);

static const struct via6522_interface b_via_0_interface =
{
	/*inputs : A/B         */ 0, b_via_0_pb_r,
	/*inputs : CA/B1,CA/B2 */ 0, via_1_ca2_r, b_via_0_ca2_r, via_1_ca1_r,
	/*outputs: A/B         */ b_via_0_pa_w, b_via_0_pb_w,
	/*outputs: CA/B1,CA/B2 */ 0, 0, b_via_0_ca2_w, via_1_ca1_w,
	/*irq                  */ b_via_0_irq
};

static const struct via6522_interface b_via_1_interface =
{
	/*inputs : A/B         */ b_via_1_pa_r, b_via_1_pb_r,
	/*inputs : CA/B1,CA/B2 */ via_0_cb2_r, 0, via_0_cb1_r, 0,
	/*outputs: A/B         */ b_via_1_pa_w, b_via_1_pb_w,
	/*outputs: CA/B1,CA/B2 */ 0, 0, via_0_cb1_w, 0,
	/*irq                  */ b_via_1_irq
};

static READ8_HANDLER( b_via_0_ca2_r )
{
	return 0;
}

static WRITE8_HANDLER( b_via_0_ca2_w )
{
}

static void b_via_0_irq (running_machine *machine, int level)
{
	cpunum_set_input_line(machine, 0, M6809_IRQ_LINE, level);
}

static READ8_HANDLER( b_via_0_pb_r )
{
	return pbus;
}

static WRITE8_HANDLER( b_via_0_pa_w )
{
	if ((data & 0x08) == 0)
		cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
	else
		cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, CLEAR_LINE);

	if ((data & 0x04) == 0)
	{
		switch (data & 0x03)
		{
		case 0:
			pbus = input_port_read(machine, "IN0");
			break;
		case 1:
			pbus = input_port_read(machine, "IN1") | (input_port_read(machine, "IN2") << 4);
			break;
		case 2:
			pbus = input_port_read(machine, "DSWB");
			break;
		case 3:
			pbus = 0xff;
			break;
		}
	}
}

static WRITE8_HANDLER( b_via_0_pb_w )
{
	pbus = data;
}

static void b_via_1_irq (running_machine *machine, int level)
{
	cpunum_set_input_line(machine, 1, M6809_IRQ_LINE, level);
}

static READ8_HANDLER( b_via_1_pa_r )
{
	return pbus;
}

static READ8_HANDLER( b_via_1_pb_r )
{
	return 0xff;
}

static WRITE8_HANDLER( b_via_1_pa_w )
{
	pbus = data;
}

static WRITE8_HANDLER( b_via_1_pb_w )
{
}

DRIVER_INIT( beezer )
{
	via_config(0, &b_via_0_interface);
	via_config(1, &b_via_1_interface);
	via_reset();
	pbus = 0;
}

WRITE8_HANDLER( beezer_bankswitch_w )
{
	if ((data & 0x07) == 0)
	{
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc600, 0xc7ff, 0, 0, watchdog_reset_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc800, 0xc9ff, 0, 0, beezer_map_w);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xca00, 0xcbff, 0, 0, beezer_line_r);
		memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xce00, 0xcfff, 0, 0, via_0_r, via_0_w);
	}
	else
	{
		UINT8 *rom = memory_region(machine, "main") + 0x10000;
		memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xcfff, 0, 0, SMH_BANK1, SMH_BANK1);
		memory_set_bankptr(1, rom + (data & 0x07) * 0x2000 + ((data & 0x08) ? 0x1000: 0));
	}
}


