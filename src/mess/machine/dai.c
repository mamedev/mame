/***************************************************************************

  machine/dai.c

  Functions to emulate general aspects of DAI (RAM, ROM, interrupts, I/O ports)

  Krzysztof Strzecha
  Nathan Woods

***************************************************************************/

#include "emu.h"
#include "includes/dai.h"

#define DEBUG_DAI_PORTS	0

#define LOG_DAI_PORT_R(_port, _data, _comment) do { if (DEBUG_DAI_PORTS) logerror ("DAI port read : %04x, Data: %02x (%s)\n", _port, _data, _comment); } while (0)
#define LOG_DAI_PORT_W(_port, _data, _comment) do { if (DEBUG_DAI_PORTS) logerror ("DAI port write: %04x, Data: %02x (%s)\n", _port, _data, _comment); } while (0)

/* Discrete I/O devices */


/* Memory */

WRITE8_MEMBER(dai_state::dai_stack_interrupt_circuit_w)
{
	m_tms5501->set_sensor(1);
	m_tms5501->set_sensor(0);
}

static void dai_update_memory(running_machine &machine, int dai_rom_bank)
{
	dai_state *state = machine.driver_data<dai_state>();
	state->membank("bank2")->set_entry(dai_rom_bank);
}

static TIMER_CALLBACK(dai_bootstrap_callback)
{
	machine.device("maincpu")->state().set_pc(0xc000);
}


READ8_MEMBER(dai_state::dai_keyboard_r)
{
	UINT8 data = 0x00;
	static const char *const keynames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7" };

	for (int i = 0; i < 8; i++)
	{
		if (m_keyboard_scan_mask & (1 << i))
			data |= ioport(keynames[i])->read();
	}

	return data;
}

WRITE8_MEMBER(dai_state::dai_keyboard_w)
{
	m_keyboard_scan_mask = data;
}

static TMS5501_IRQ_CALLBACK(dai_interrupt_callback)
{
	if (intreq)
		device.machine().device("maincpu")->execute().set_input_line_and_vector(0, HOLD_LINE, vector);
	else
		device.machine().device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
}

TMS5501_INTERFACE( dai_tms5501_interface )
{
	DEVCB_DRIVER_MEMBER(dai_state, dai_keyboard_r),
	DEVCB_DRIVER_MEMBER(dai_state, dai_keyboard_w),
	dai_interrupt_callback
};

I8255A_INTERFACE( dai_ppi82555_intf )
{
	DEVCB_NULL,	/* Port A read */
	DEVCB_NULL,	/* Port A write */
	DEVCB_NULL,	/* Port B read */
	DEVCB_NULL,	/* Port B write */
	DEVCB_NULL,	/* Port C read */
	DEVCB_NULL	/* Port C write */
};

const struct pit8253_config dai_pit8253_intf =
{
	{
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER("custom", dai_sound_device, set_input_ch0),
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER("custom", dai_sound_device, set_input_ch1),
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER("custom", dai_sound_device, set_input_ch2),
		}
	}
};

static TIMER_CALLBACK( dai_timer )
{
	dai_state *state = machine.driver_data<dai_state>();
	state->m_tms5501->set_pio_bit_7((state->ioport("IN8")->read() & 0x04) ? 1:0);
}

void dai_state::machine_start()
{

	membank("bank2")->configure_entries(0, 4, memregion("maincpu")->base() + 0x010000, 0x1000);
	machine().scheduler().timer_set(attotime::zero, FUNC(dai_bootstrap_callback));
	machine().scheduler().timer_pulse(attotime::from_hz(100), FUNC(dai_timer));	/* timer for tms5501 */

	memset(machine().device<ram_device>(RAM_TAG)->pointer(), 0, machine().device<ram_device>(RAM_TAG)->size());
}

void dai_state::machine_reset()
{
	membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());
}

/***************************************************************************

    Discrete Devices IO

    FD00    POR1:   IN  bit 0   -
                bit 1   -
                bit 2   PIPGE: Page signal
                bit 3   PIDTR: Serial output ready
                bit 4   PIBU1: Button on paddle 1 (1 = closed)
                bit 5   PIBU1: Button on paddle 2 (1 = closed)
                bit 6   PIRPI: Random data
                bit 7   PICAI: Cassette input data

    FD01    PDLST:  IN  Single pulse used to trigger paddle timer circuit

    FD04    POR1:   OUT bit 0-3 Volume oscillator channel 0
                bit 4-7 Volume oscillator channel 1

    FD05    POR1:   OUT bit 0-3 Volume oscillator channel 2
                bit 4-7 Volume random noise generator

    FD06    POR0:   OUT bit 0   POCAS: Cassette data output
                bit 1-2 PDLMSK: Paddle select
                bit 3   PDPNA:  Paddle enable
                bit 4   POCM1:  Cassette 1 motor control (0 = run)
                bit 5   POCM2:  Cassette 2 motor control (0 = run)
                bit 6-7         ROM bank switching
***************************************************************************/

READ8_MEMBER(dai_state::dai_io_discrete_devices_r)
{
	UINT8 data = 0x00;

	switch(offset & 0x000f) {
	case 0x00:
		data = ioport("IN8")->read();
		data |= 0x08;			// serial ready
		if (machine().rand()&0x01)
			data |= 0x40;		// random number generator
		if (machine().device<cassette_image_device>(CASSETTE_TAG)->input() > 0.01)
			data |= 0x80;		// tape input
		break;

	default:
		data = 0xff;
		LOG_DAI_PORT_R (offset, data, "discrete devices - unmapped");

		break;
	}
	return data;
}

WRITE8_MEMBER(dai_state::dai_io_discrete_devices_w)
{
	switch(offset & 0x000f) {
	case 0x04:
		m_sound->set_volume(space, offset, data);
		LOG_DAI_PORT_W (offset, data&0x0f, "discrete devices - osc. 0 volume");
		LOG_DAI_PORT_W (offset, (data&0xf0)>>4, "discrete devices - osc. 1 volume");
		break;

	case 0x05:
		m_sound->set_volume(space, offset, data);
		LOG_DAI_PORT_W (offset, data&0x0f, "discrete devices - osc. 2 volume");
		LOG_DAI_PORT_W (offset, (data&0xf0)>>4, "discrete devices - noise volume");
		break;

	case 0x06:
		m_paddle_select = (data&0x06)>>2;
		m_paddle_enable = (data&0x08)>>3;
		m_cassette_motor[0] = (data&0x10)>>4;
		m_cassette_motor[1] = (data&0x20)>>5;
		machine().device<cassette_image_device>(CASSETTE_TAG)->change_state(m_cassette_motor[0]?CASSETTE_MOTOR_DISABLED:CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		machine().device<cassette_image_device>(CASSETTE_TAG)->output((data & 0x01) ? -1.0 : 1.0);
		dai_update_memory (machine(), (data&0xc0)>>6);
		LOG_DAI_PORT_W (offset, (data&0x06)>>2, "discrete devices - paddle select");
		LOG_DAI_PORT_W (offset, (data&0x08)>>3, "discrete devices - paddle enable");
		LOG_DAI_PORT_W (offset, (data&0x10)>>4, "discrete devices - cassette motor 1");
		LOG_DAI_PORT_W (offset, (data&0x20)>>5, "discrete devices - cassette motor 2");
		LOG_DAI_PORT_W (offset, (data&0xc0)>>6, "discrete devices - ROM bank");
		break;

	default:
		LOG_DAI_PORT_W (offset, data, "discrete devices - unmapped");
		break;
	}
}

/***************************************************************************

    PIT8253

    Offset need to be shifted by 1 to right, because the PIT is
    connected to A1 and A2

***************************************************************************/

READ8_MEMBER(dai_state::dai_pit_r)
{
	return pit8253_r(m_pit, space, (offset>>1) & 3);
}

WRITE8_MEMBER(dai_state::dai_pit_w)
{
	pit8253_w(m_pit, space, (offset>>1) & 3, data);
}

/***************************************************************************

    AMD 9911 mathematical coprocesor

***************************************************************************/

READ8_MEMBER(dai_state::dai_amd9511_r)
{
	/* optional and no present at this moment */
	return 0xff;
}

WRITE8_MEMBER(dai_state::dai_amd9511_w)
{
	logerror ("Writing to AMD9511 math chip, %04x, %02x\n", offset, data);
}

