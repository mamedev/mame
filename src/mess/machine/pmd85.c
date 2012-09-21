/***************************************************************************

  machine.c

  Functions to emulate general aspects of PMD-85 (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "imagedev/cassette.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "includes/pmd85.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/ram.h"



enum {PMD85_LED_1, PMD85_LED_2, PMD85_LED_3};
enum {PMD85_1, PMD85_2, PMD85_2A, PMD85_2B, PMD85_3, ALFA, MATO, C2717};



static void pmd851_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		UINT8 *mem = state->memregion("maincpu")->base();

		space.unmap_write(0x0000, 0x0fff);
		space.nop_write(0x1000, 0x1fff);
		space.unmap_write(0x2000, 0x2fff);
		space.nop_write(0x3000, 0x3fff);

		space.nop_read(0x1000, 0x1fff);
		space.nop_read(0x3000, 0x3fff);

		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank3")->set_base(mem + 0x010000);
		state->membank("bank5")->set_base(ram + 0xc000);

		state->membank("bank6")->set_base(mem + 0x010000);
		state->membank("bank7")->set_base(mem + 0x010000);
		state->membank("bank8")->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x1000, 0x1fff, "bank2");
		space.install_write_bank(0x2000, 0x2fff, "bank3");
		space.install_write_bank(0x3000, 0x3fff, "bank4");
		space.install_write_bank(0x4000, 0x7fff, "bank5");

		space.install_read_bank(0x1000, 0x1fff, "bank2");
		space.install_read_bank(0x3000, 0x3fff, "bank4");

		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x1000);
		state->membank("bank3")->set_base(ram + 0x2000);
		state->membank("bank4")->set_base(ram + 0x3000);
		state->membank("bank5")->set_base(ram + 0x4000);
	}
}

static void pmd852a_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		UINT8 *mem = state->memregion("maincpu")->base();

		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x2000, 0x2fff);

		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank2")->set_base(ram + 0x9000);
		state->membank("bank3")->set_base(mem + 0x010000);
		state->membank("bank4")->set_base(ram + 0xb000);
		state->membank("bank5")->set_base(ram + 0xc000);
		state->membank("bank6")->set_base(mem + 0x010000);
		state->membank("bank7")->set_base(ram + 0x9000);
		state->membank("bank8")->set_base(mem + 0x010000);
		state->membank("bank9")->set_base(ram + 0xb000);
		state->membank("bank10")->set_base(ram + 0xc000);

	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x2000, 0x2fff, "bank3");

		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x1000);
		state->membank("bank3")->set_base(ram + 0x2000);
		state->membank("bank4")->set_base(ram + 0x5000);
		state->membank("bank5")->set_base(ram + 0x4000);
	}
}

static void pmd853_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	UINT8 *mem = state->memregion("maincpu")->base();
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank2")->set_base(mem + 0x010000);
		state->membank("bank3")->set_base(mem + 0x010000);
		state->membank("bank4")->set_base(mem + 0x010000);
		state->membank("bank5")->set_base(mem + 0x010000);
		state->membank("bank6")->set_base(mem + 0x010000);
		state->membank("bank7")->set_base(mem + 0x010000);
		state->membank("bank8")->set_base(mem + 0x010000);
		state->membank("bank9")->set_base(ram);
		state->membank("bank10")->set_base(ram + 0x2000);
		state->membank("bank11")->set_base(ram + 0x4000);
		state->membank("bank12")->set_base(ram + 0x6000);
		state->membank("bank13")->set_base(ram + 0x8000);
		state->membank("bank14")->set_base(ram + 0xa000);
		state->membank("bank15")->set_base(ram + 0xc000);
		state->membank("bank16")->set_base(ram + 0xe000);
	}
	else
	{
		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x2000);
		state->membank("bank3")->set_base(ram + 0x4000);
		state->membank("bank4")->set_base(ram + 0x6000);
		state->membank("bank5")->set_base(ram + 0x8000);
		state->membank("bank6")->set_base(ram + 0xa000);
		state->membank("bank7")->set_base(ram + 0xc000);
		state->membank("bank8")->set_base(state->m_pmd853_memory_mapping ? mem + 0x010000 : ram + 0xe000);
	}
}

static void alfa_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		UINT8 *mem = state->memregion("maincpu")->base();

		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x1000, 0x33ff);
		space.nop_write(0x3400, 0x3fff);

		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank2")->set_base(mem + 0x011000);
		state->membank("bank4")->set_base(ram + 0xc000);
		state->membank("bank5")->set_base(mem + 0x010000);
		state->membank("bank6")->set_base(mem + 0x011000);
		state->membank("bank7")->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x1000, 0x33ff, "bank2");
		space.install_write_bank(0x3400, 0x3fff, "bank3");

		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x1000);
		state->membank("bank3")->set_base(ram + 0x3400);
		state->membank("bank4")->set_base(ram + 0x4000);
	}
}

static void mato_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		UINT8 *mem = state->memregion("maincpu")->base();

		space.unmap_write(0x0000, 0x3fff);

		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank2")->set_base(ram + 0xc000);
		state->membank("bank3")->set_base(mem + 0x010000);
		state->membank("bank4")->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, "bank1");

		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x4000);
	}
}

static void c2717_update_memory(running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *mem = state->memregion("maincpu")->base();
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x3fff);

		state->membank("bank1")->set_base(mem + 0x010000);
		state->membank("bank2")->set_base(ram + 0x4000);
		state->membank("bank3")->set_base(mem + 0x010000);
		state->membank("bank4")->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, "bank1");
		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x4000);
	}
}

/*******************************************************************************

    Motherboard 8255 (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    --------------------------------------------------------------
        keyboard, speaker, LEDs

*******************************************************************************/

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_porta_r )
{
	return 0xff;
}

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_portb_r )
{
	pmd85_state *state = device->machine().driver_data<pmd85_state>();
	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7",
		"KEY8", "KEY9", "KEY10", "KEY11", "KEY12", "KEY13", "KEY14", "KEY15"
	};

	return device->machine().root_device().ioport(keynames[(state->m_ppi_port_outputs[0][0] & 0x0f)])->read() & device->machine().root_device().ioport("KEY15")->read();
}

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_porta_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[0][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_portb_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[0][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_portc_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[0][2] = data;
	set_led_status(space.machine(), PMD85_LED_2, (data & 0x08) ? 1 : 0);
	set_led_status(space.machine(), PMD85_LED_3, (data & 0x04) ? 1 : 0);
}

/*******************************************************************************

    Motherboard 8255 (Mato)
    -----------------------
        keyboard, speaker, LEDs, tape

*******************************************************************************/

static  READ8_DEVICE_HANDLER ( mato_ppi_0_portb_r )
{
	pmd85_state *state = device->machine().driver_data<pmd85_state>();
	int i;
	UINT8 data = 0xff;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7" };

	for (i = 0; i < 8; i++)
	{
		if (!BIT(state->m_ppi_port_outputs[0][0], i))
			data &= device->machine().root_device().ioport(keynames[i])->read();
	}
	return data;
}

static  READ8_DEVICE_HANDLER ( mato_ppi_0_portc_r )
{
	return device->machine().root_device().ioport("KEY8")->read() | 0x8f;
}

static WRITE8_DEVICE_HANDLER ( mato_ppi_0_portc_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[0][2] = data;
	set_led_status(space.machine(), PMD85_LED_2, BIT(data, 3));
	set_led_status(space.machine(), PMD85_LED_3, BIT(data, 2));
}

/*******************************************************************************

    I/O board 8255
    --------------
        GPIO/0 (K3 connector), GPIO/1 (K4 connector)

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_porta_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[1][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_portb_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[1][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_portc_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[1][2] = data;
}

/*******************************************************************************

    I/O board 8255
    --------------
        IMS-2 (K5 connector)

    - 8251 - cassette recorder and V.24/IFSS (selectable by switch)

    - external interfaces connector (K2)

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_porta_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[2][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_portb_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[2][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_portc_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[2][2] = data;
}

/*******************************************************************************

    I/O board 8251
    --------------
        cassette recorder and V.24/IFSS (selectable by switch)

*******************************************************************************/

/*******************************************************************************

    I/O board 8253
    --------------

    Timer 0:
        OUT0    - external interfaces connector (K2)
        CLK0    - external interfaces connector (K2)
        GATE0   - external interfaces connector (K2), default = 1
    Timer 1:
        OUT0    - external interfaces connector (K2), i8251 (for V24 only)
        CLK0    - hardwired to 2 MHz system clock
        GATE0   - external interfaces connector (K2), default = 1
    Timer 2:
        OUT0    - unused
        CLK0    - hardwired to 1HZ signal generator
        GATE0   - hardwired to 5V, default = 1

*******************************************************************************/

const struct pit8253_config pmd85_pit8253_interface =
{
	{
		{ 0,		DEVCB_NULL,     DEVCB_NULL },
		{ 2000000,	DEVCB_NULL,     DEVCB_NULL },
		{ 1,		DEVCB_LINE_VCC, DEVCB_NULL }
	}
};

/*******************************************************************************

    I/O board external interfaces connector (K2)
    --------------------------------------------

*******************************************************************************/



/*******************************************************************************

    ROM Module 8255
    ---------------
        port A - data read
        ports B, C - address select

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_porta_r )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	if (state->memregion("user1")->base() != NULL)
		return state->memregion("user1")->base()[state->m_ppi_port_outputs[3][1] | (state->m_ppi_port_outputs[3][2] << 8)];
	else
		return 0;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_porta_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[3][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_portb_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[3][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_portc_w )
{
	pmd85_state *state = space.machine().driver_data<pmd85_state>();
	state->m_ppi_port_outputs[3][2] = data;
}

/*******************************************************************************

    I/O ports (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    -------------------------------------------------------

    I/O board
    1xxx11aa    external interfaces connector (K2)

    0xxx11aa    I/O board interfaces
        000111aa    8251 (casette recorder, V24)
        010011aa    8255 (GPIO/0, GPIO/1)
        010111aa    8253
        011111aa    8255 (IMS-2)

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs)
            PMD-85.3 memory banking

    ROM Module
    1xxx10aa    8255 (ROM reading)

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_io_r)
{
	i8251_device *uart = machine().device<i8251_device>("uart");
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							return machine().device<i8255_device>("ppi8255_0")->read(space, offset & 0x03);
				}
				break;
		case 0x08:	/* ROM module connector */
				switch (m_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (m_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:	/* ROM module 8255 */
									return machine().device<i8255_device>("ppi8255_3")->read(space, offset & 0x03);
							}
						}
						break;
				}
				break;
		case 0x0c:	/* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:	/* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:	/* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: return uart->data_r(space, offset & 0x01);
											case 0x01: return uart->status_r(space, offset & 0x01);
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/1) */
										return machine().device<i8255_device>("ppi8255_1")->read(space, offset & 0x03);
								case 0x50:	/* 8253 */
										return pit8253_r( machine().device("pit8253"), space, offset & 0x03);
								case 0x70:	/* 8255 (IMS-2) */
										return machine().device<i8255_device>("ppi8255_2")->read(space, offset & 0x03);
							}
							break;
					case 0x80:	/* external interfaces */
							break;
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_io_w)
{
	i8251_device *uart = machine().device<i8251_device>("uart");
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(*update_memory)(machine());
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							machine().device<i8255_device>("ppi8255_0")->write(space, offset & 0x03, data);
							/* PMD-85.3 memory banking */
							if ((offset & 0x03) == 0x03)
							{
								m_pmd853_memory_mapping = data & 0x01;
								(*update_memory)(machine());
							}
							break;
				}
				break;
		case 0x08:	/* ROM module connector */
				switch (m_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (m_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:	/* ROM module 8255 */
										machine().device<i8255_device>("ppi8255_3")->write(space, offset & 0x03, data);
										break;
							}
						}
						break;
				}
				break;
		case 0x0c:	/* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:	/* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:	/* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: uart->data_w(space, offset & 0x01, data); break;
											case 0x01: uart->control_w(space, offset & 0x01, data); break;
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/0) */
										machine().device<i8255_device>("ppi8255_1")->write(space, offset & 0x03, data);
										break;
								case 0x50:	/* 8253 */
										pit8253_w(machine().device("pit8253"), space, offset & 0x03, data);
										logerror ("8253 writing. Address: %02x, Data: %02x\n", offset, data);
										break;
								case 0x70:	/* 8255 (IMS-2) */
										machine().device<i8255_device>("ppi8255_2")->write(space, offset & 0x03, data);
										break;
							}
							break;
					case 0x80:	/* external interfaces */
							break;
				}
				break;
	}
}

/*******************************************************************************

    I/O ports (Mato)
    ----------------

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs, tape)

*******************************************************************************/

READ8_MEMBER(pmd85_state::mato_io_r)
{
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							return machine().device<i8255_device>("ppi8255_0")->read(space, offset & 0x03);
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::mato_io_w)
{
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(*update_memory)(machine());
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							return machine().device<i8255_device>("ppi8255_0")->write(space, offset & 0x03, data);
				}
				break;
	}
}

const i8255_interface pmd85_ppi8255_interface[4] =
{
	{
		DEVCB_HANDLER(pmd85_ppi_0_porta_r),
		DEVCB_HANDLER(pmd85_ppi_0_porta_w),
		DEVCB_HANDLER(pmd85_ppi_0_portb_r),
		DEVCB_HANDLER(pmd85_ppi_0_portb_w),
		DEVCB_HANDLER(pmd85_ppi_0_portc_r),
		DEVCB_HANDLER(pmd85_ppi_0_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_1_porta_r),
		DEVCB_HANDLER(pmd85_ppi_1_porta_w),
		DEVCB_HANDLER(pmd85_ppi_1_portb_r),
		DEVCB_HANDLER(pmd85_ppi_1_portb_w),
		DEVCB_HANDLER(pmd85_ppi_1_portc_r),
		DEVCB_HANDLER(pmd85_ppi_1_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_2_porta_r),
		DEVCB_HANDLER(pmd85_ppi_2_porta_w),
		DEVCB_HANDLER(pmd85_ppi_2_portb_r),
		DEVCB_HANDLER(pmd85_ppi_2_portb_w),
		DEVCB_HANDLER(pmd85_ppi_2_portc_r),
		DEVCB_HANDLER(pmd85_ppi_2_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_3_porta_r),
		DEVCB_HANDLER(pmd85_ppi_3_porta_w),
		DEVCB_HANDLER(pmd85_ppi_3_portb_r),
		DEVCB_HANDLER(pmd85_ppi_3_portb_w),
		DEVCB_HANDLER(pmd85_ppi_3_portc_r),
		DEVCB_HANDLER(pmd85_ppi_3_portc_w)
	}
};

const i8255_interface alfa_ppi8255_interface[3] =
{
	{
		DEVCB_HANDLER(pmd85_ppi_0_porta_r),
		DEVCB_HANDLER(pmd85_ppi_0_porta_w),
		DEVCB_HANDLER(pmd85_ppi_0_portb_r),
		DEVCB_HANDLER(pmd85_ppi_0_portb_w),
		DEVCB_HANDLER(pmd85_ppi_0_portc_r),
		DEVCB_HANDLER(pmd85_ppi_0_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_1_porta_r),
		DEVCB_HANDLER(pmd85_ppi_1_porta_w),
		DEVCB_HANDLER(pmd85_ppi_1_portb_r),
		DEVCB_HANDLER(pmd85_ppi_1_portb_w),
		DEVCB_HANDLER(pmd85_ppi_1_portc_r),
		DEVCB_HANDLER(pmd85_ppi_1_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_2_porta_r),
		DEVCB_HANDLER(pmd85_ppi_2_porta_w),
		DEVCB_HANDLER(pmd85_ppi_2_portb_r),
		DEVCB_HANDLER(pmd85_ppi_2_portb_w),
		DEVCB_HANDLER(pmd85_ppi_2_portc_r),
		DEVCB_HANDLER(pmd85_ppi_2_portc_w)
	}
};

I8255_INTERFACE( mato_ppi8255_interface )
{
	DEVCB_HANDLER(pmd85_ppi_0_porta_r),
	DEVCB_HANDLER(pmd85_ppi_0_porta_w),
	DEVCB_HANDLER(mato_ppi_0_portb_r),
	DEVCB_HANDLER(pmd85_ppi_0_portb_w),
	DEVCB_HANDLER(mato_ppi_0_portc_r),
	DEVCB_HANDLER(mato_ppi_0_portc_w)
};


static TIMER_CALLBACK(pmd85_cassette_timer_callback)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	i8251_device *uart = machine.device<i8251_device>("uart");
	serial_source_device *ser = machine.device<serial_source_device>("sercas");
	int data;
	int current_level;

	if (!(machine.root_device().ioport("DSW0")->read() & 0x02))	/* V.24 / Tape Switch */
	{
		/* tape reading */
		if (machine.device<cassette_image_device>(CASSETTE_TAG)->get_state()&CASSETTE_PLAY)
		{
			switch (state->m_model)
			{
				case PMD85_1:
					if (state->m_clk_level_tape)
					{
						state->m_previous_level = ((machine.device<cassette_image_device>(CASSETTE_TAG))->input() > 0.038) ? 1 : 0;
						state->m_clk_level_tape = 0;
					}
					else
					{
						current_level = ((machine.device<cassette_image_device>(CASSETTE_TAG))->input() > 0.038) ? 1 : 0;

						if (state->m_previous_level!=current_level)
						{
							data = (!state->m_previous_level && current_level) ? 1 : 0;

							ser->send_bit(data);
							uart->receive_clock();

							state->m_clk_level_tape = 1;
						}
					}
					return;
				case PMD85_2:
				case PMD85_2A:
				case C2717:
				case PMD85_3:
				case ALFA:
					/* not hardware data decoding */
					return;
			}
		}

		/* tape writing */
		if (machine.device<cassette_image_device>(CASSETTE_TAG)->get_state()&CASSETTE_RECORD)
		{
			data = ser->get_in_data_bit();
			data ^= state->m_clk_level_tape;
			machine.device<cassette_image_device>(CASSETTE_TAG)->output(data&0x01 ? 1 : -1);

			if (!state->m_clk_level_tape)
				uart->transmit_clock();

			state->m_clk_level_tape = state->m_clk_level_tape ? 0 : 1;

			return;
		}

		state->m_clk_level_tape = 1;

		if (!state->m_clk_level)
			uart->transmit_clock();
		state->m_clk_level = state->m_clk_level ? 0 : 1;
	}
}

static TIMER_CALLBACK( pmd_reset )
{
	machine.schedule_soft_reset();
}

DIRECT_UPDATE_MEMBER(pmd85_state::pmd85_opbaseoverride)
{
	if (ioport("RESET")->read() & 0x01)
		machine().scheduler().timer_set(attotime::from_usec(10), FUNC(pmd_reset));
	return address;
}

static void pmd85_common_driver_init (running_machine &machine)
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	state->m_previous_level = 0;
	state->m_clk_level = state->m_clk_level_tape = 1;
	state->m_cassette_timer = machine.scheduler().timer_alloc(FUNC(pmd85_cassette_timer_callback));
	state->m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(2400));
}

DRIVER_INIT_MEMBER(pmd85_state,pmd851)
{
	m_model = PMD85_1;
	update_memory = pmd851_update_memory;
	pmd85_common_driver_init(machine());
}

DRIVER_INIT_MEMBER(pmd85_state,pmd852a)
{
	m_model = PMD85_2A;
	update_memory = pmd852a_update_memory;
	pmd85_common_driver_init(machine());
}

DRIVER_INIT_MEMBER(pmd85_state,pmd853)
{
	m_model = PMD85_3;
	update_memory = pmd853_update_memory;
	pmd85_common_driver_init(machine());
}

DRIVER_INIT_MEMBER(pmd85_state,alfa)
{
	m_model = ALFA;
	update_memory = alfa_update_memory;
	pmd85_common_driver_init(machine());
}

DRIVER_INIT_MEMBER(pmd85_state,mato)
{
	m_model = MATO;
	update_memory = mato_update_memory;
}

DRIVER_INIT_MEMBER(pmd85_state,c2717)
{
	m_model = C2717;
	update_memory = c2717_update_memory;
	pmd85_common_driver_init(machine());
}

static TIMER_CALLBACK( setup_machine_state )
{
	pmd85_state *state = machine.driver_data<pmd85_state>();
	if (state->m_model != MATO)
	{
		i8251_device *uart = machine.device<i8251_device>("uart");
		serial_source_device *ser = machine.device<serial_source_device>("sercas");
		uart->connect(ser);
	}
}


void pmd85_state::machine_reset()
{
	int i, j;

	/* checking for Rom Module */
	switch (m_model)
	{
		case PMD85_1:
		case PMD85_2A:
		case PMD85_3:
		case C2717:
			m_rom_module_present = (machine().root_device().ioport("DSW0")->read() & 0x01) ? 1 : 0;
			break;
		case ALFA:
		case MATO:
			break;
	}

	for (i = 0; i < 4; i++)
		for (j = 0; j < 3; j++)
			m_ppi_port_outputs[i][j] = 0;

	/* memory initialization */
	memset(machine().device<ram_device>(RAM_TAG)->pointer(), 0, sizeof(unsigned char)*0x10000);
	m_pmd853_memory_mapping = 1;
	m_startup_mem_map = 1;
	update_memory(machine());

	machine().scheduler().timer_set(attotime::zero, FUNC(setup_machine_state));

	machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(pmd85_state::pmd85_opbaseoverride), this));
}
