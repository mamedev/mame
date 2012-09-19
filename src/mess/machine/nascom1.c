/**********************************************************************

  machine/nascom1.c

**********************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/nascom1.h"

/* Components */
#include "cpu/z80/z80.h"
#include "machine/wd17xx.h"
#include "machine/ay31015.h"

/* Devices */
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"

#define NASCOM1_KEY_RESET	0x02
#define NASCOM1_KEY_INCR	0x01



/*************************************
 *
 *  Global variables
 *
 *************************************/




/*************************************
 *
 *  Floppy
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( nascom2_fdc_intrq_w )
{
	nascom1_state *drvstate = device->machine().driver_data<nascom1_state>();
	drvstate->m_nascom2_fdc.irq = state;
}

static WRITE_LINE_DEVICE_HANDLER( nascom2_fdc_drq_w )
{
	nascom1_state *drvstate = device->machine().driver_data<nascom1_state>();
	drvstate->m_nascom2_fdc.drq = state;
}

const wd17xx_interface nascom2_wd17xx_interface =
{
	DEVCB_LINE_VCC,
	DEVCB_LINE(nascom2_fdc_intrq_w),
	DEVCB_LINE(nascom2_fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};


READ8_MEMBER(nascom1_state::nascom2_fdc_select_r)
{
	return m_nascom2_fdc.select | 0xa0;
}


WRITE8_MEMBER(nascom1_state::nascom2_fdc_select_w)
{
	device_t *fdc = machine().device("wd1793");
	m_nascom2_fdc.select = data;

	logerror("nascom2_fdc_select_w: %02x\n", data);

	if (data & 0x01) wd17xx_set_drive(fdc,0);
	if (data & 0x02) wd17xx_set_drive(fdc,1);
	if (data & 0x04) wd17xx_set_drive(fdc,2);
	if (data & 0x08) wd17xx_set_drive(fdc,3);
	if (data & 0x10) wd17xx_set_side(fdc,(data & 0x10) >> 4);
}


/*
 * D0 -- WD1793 IRQ
 * D1 -- NOT READY
 * D2 to D6 -- 0
 * D7 -- WD1793 DRQ
 *
 */
READ8_MEMBER(nascom1_state::nascom2_fdc_status_r)
{
	return (m_nascom2_fdc.drq << 7) | m_nascom2_fdc.irq;
}

/*************************************
 *
 *  Keyboard
 *
 *************************************/

READ8_MEMBER(nascom1_state::nascom1_port_00_r)
{
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8" };

	if (m_portstat.stat_count < 9)
		return (ioport(keynames[m_portstat.stat_count])->read() | ~0x7f);

	return (0xff);
}


WRITE8_MEMBER(nascom1_state::nascom1_port_00_w)
{

	machine().device<cassette_image_device>(CASSETTE_TAG)->change_state(
		( data & 0x10 ) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR );

	if (!(data & NASCOM1_KEY_RESET))
	{
		if (m_portstat.stat_flags & NASCOM1_KEY_RESET)
			m_portstat.stat_count = 0;
	}
	else
		m_portstat.stat_flags = NASCOM1_KEY_RESET;

	if (!(data & NASCOM1_KEY_INCR))
	{
		if (m_portstat.stat_flags & NASCOM1_KEY_INCR)
			m_portstat.stat_count++;
	}
	else
		m_portstat.stat_flags = NASCOM1_KEY_INCR;
}




/*************************************
 *
 *  Cassette
 *
 *************************************/


READ8_MEMBER(nascom1_state::nascom1_port_01_r)
{
	return ay31015_get_received_data( m_hd6402 );
}


WRITE8_MEMBER(nascom1_state::nascom1_port_01_w)
{
	ay31015_set_transmit_data( m_hd6402, data );
}

READ8_MEMBER(nascom1_state::nascom1_port_02_r)
{
	UINT8 data = 0x31;

	ay31015_set_input_pin( m_hd6402, AY31015_SWE, 0 );
	data |= ay31015_get_output_pin( m_hd6402, AY31015_OR ) ? 0x02 : 0;
	data |= ay31015_get_output_pin( m_hd6402, AY31015_PE ) ? 0x04 : 0;
	data |= ay31015_get_output_pin( m_hd6402, AY31015_FE ) ? 0x08 : 0;
	data |= ay31015_get_output_pin( m_hd6402, AY31015_TBMT ) ? 0x40 : 0;
	data |= ay31015_get_output_pin( m_hd6402, AY31015_DAV ) ? 0x80 : 0;
	ay31015_set_input_pin( m_hd6402, AY31015_SWE, 1 );

	return data;
}


READ8_DEVICE_HANDLER( nascom1_hd6402_si )
{
	return 1;
}


WRITE8_DEVICE_HANDLER( nascom1_hd6402_so )
{
}


DEVICE_IMAGE_LOAD( nascom1_cassette )
{
	nascom1_state *state = image.device().machine().driver_data<nascom1_state>();
	state->m_tape_size = image.length();
	state->m_tape_image = (UINT8*)image.ptr();
	if (!state->m_tape_image)
		return IMAGE_INIT_FAIL;

	state->m_tape_index = 0;
	return IMAGE_INIT_PASS;
}


DEVICE_IMAGE_UNLOAD( nascom1_cassette )
{
	nascom1_state *state = image.device().machine().driver_data<nascom1_state>();
	state->m_tape_image = NULL;
	state->m_tape_size = state->m_tape_index = 0;
}



/*************************************
 *
 *  Snapshots
 *
 *  ASCII .nas format
 *
 *************************************/

SNAPSHOT_LOAD( nascom1 )
{
	UINT8 line[35];

	while (image.fread( &line, sizeof(line)) == sizeof(line))
	{
		int addr, b0, b1, b2, b3, b4, b5, b6, b7, dummy;

		if (sscanf((char *)line, "%x %x %x %x %x %x %x %x %x %x\010\010\n",
			&addr, &b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7, &dummy) == 10)
		{
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b0);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b1);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b2);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b3);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b4);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b5);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b6);
			image.device().machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte(addr++, b7);
		}
	}

	return IMAGE_INIT_PASS;
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

void nascom1_state::machine_reset()
{
	m_hd6402 = machine().device("hd6402");

	/* Set up hd6402 pins */
	ay31015_set_input_pin( m_hd6402, AY31015_SWE, 1 );

	ay31015_set_input_pin( m_hd6402, AY31015_CS, 0 );
	ay31015_set_input_pin( m_hd6402, AY31015_NP, 1 );
	ay31015_set_input_pin( m_hd6402, AY31015_NB1, 1 );
	ay31015_set_input_pin( m_hd6402, AY31015_NB2, 1 );
	ay31015_set_input_pin( m_hd6402, AY31015_EPS, 1 );
	ay31015_set_input_pin( m_hd6402, AY31015_TSB, 1 );
	ay31015_set_input_pin( m_hd6402, AY31015_CS, 1 );
}

DRIVER_INIT_MEMBER(nascom1_state,nascom1)
{
	switch (machine().device<ram_device>(RAM_TAG)->size())
	{
	case 1 * 1024:
		machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(
			0x1400, 0x9000);
		break;

	case 16 * 1024:
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(
			0x1400, 0x4fff, "bank1");
		machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(
			0x5000, 0xafff);
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());
		break;

	case 32 * 1024:
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(
			0x1400, 0x8fff, "bank1");
		machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(
			0x9000, 0xafff);
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());
		break;

	case 40 * 1024:
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(
			0x1400, 0xafff, "bank1");
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());
		break;
	}
}
