/***************************************************************************

    Namco 54XX

    This custom chip is a Fujitsu MB8844 MCU programmed to act as a noise
    generator. It is used for explosions, the shoot sound in Bosconian,
    and the tire screech sound in Pole Position.

    CMD = command from main CPU
    OUTn = sound outputs (3 channels)

    The chip reads the command when the /IRQ is pulled down.

                       +------+
                     EX|1   28|Vcc
                      X|2   27|K3 (CMD7)
                 /RESET|3   26|K2 (CMD6)
            (OUT0.0) O0|4   25|K1 (CMD5)
            (OUT0.1) O1|5   24|K0 (CMD4)
            (OUT0.2) O2|6   23|R10/IRQ
            (OUT0.3) O3|7   22|R9/TC
            (OUT1.0) O4|8   21|R8
            (OUT1.1) O5|9   20|R7 (OUT2.3)
            (OUT1.2) O6|10  19|R6 (OUT2.2)
            (OUT1.3) O7|11  18|R5 (OUT2.1)
              (CMD0) R0|12  17|R4 (OUT2.0)
              (CMD1) R1|13  16|R3 (CMD3)
                    GND|14  15|R2 (CMD2)
                       +------+

    [1] The RNG that drives the type A output is output on pin 21, and
    the one that drives the type B output is output on pin 22, but those
    pins are not connected on the board.


    The command format is very simple:

    0x: nop
    1x: play sound type A
    2x: play sound type B
    3x: set parameters (type A) (followed by 4 bytes)
    4x: set parameters (type B) (followed by 4 bytes)
    5x: play sound type C
    6x: set parameters (type C) (followed by 5 bytes)
    7x: set volume for sound type C to x
    8x-Fx: nop

***************************************************************************/

#include "emu.h"
#include "namco54.h"
#include "cpu/mb88xx/mb88xx.h"

struct namco_54xx_state
{
	device_t *m_cpu;
	device_t *m_discrete;
	int m_basenode;
	UINT8 m_latched_cmd;
};

INLINE namco_54xx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_54XX);

	return (namco_54xx_state *)downcast<namco_54xx_device *>(device)->token();
}



static TIMER_CALLBACK( namco_54xx_latch_callback )
{
	namco_54xx_state *state = get_safe_token((device_t *)ptr);
	state->m_latched_cmd = param;
}

static READ8_HANDLER( namco_54xx_K_r )
{
	namco_54xx_state *state = get_safe_token(space.device().owner());
	return state->m_latched_cmd >> 4;
}

static READ8_HANDLER( namco_54xx_R0_r )
{
	namco_54xx_state *state = get_safe_token(space.device().owner());
	return state->m_latched_cmd & 0x0f;
}


static WRITE8_HANDLER( namco_54xx_O_w )
{
	namco_54xx_state *state = get_safe_token(space.device().owner());
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		discrete_sound_w(state->m_discrete, space, NAMCO_54XX_1_DATA(state->m_basenode), out);
	else
		discrete_sound_w(state->m_discrete, space, NAMCO_54XX_0_DATA(state->m_basenode), out);
}

static WRITE8_HANDLER( namco_54xx_R1_w )
{
	namco_54xx_state *state = get_safe_token(space.device().owner());
	UINT8 out = (data & 0x0f);

	discrete_sound_w(state->m_discrete, space, NAMCO_54XX_2_DATA(state->m_basenode), out);
}




static TIMER_CALLBACK( namco_54xx_irq_clear )
{
	namco_54xx_state *state = get_safe_token((device_t *)ptr);
	state->m_cpu->execute().set_input_line(0, CLEAR_LINE);
}

WRITE8_DEVICE_HANDLER( namco_54xx_write )
{
	namco_54xx_state *state = get_safe_token(device);

	space.machine().scheduler().synchronize(FUNC(namco_54xx_latch_callback), data, (void *)device);

	state->m_cpu->execute().set_input_line(0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	space.machine().scheduler().timer_set(attotime::from_usec(21), FUNC(namco_54xx_irq_clear), 0, (void *)device);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_54xx_map_io, AS_IO, 8, namco_54xx_device )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ_LEGACY(namco_54xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE_LEGACY(namco_54xx_O_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ_LEGACY(namco_54xx_R0_r)
	AM_RANGE(MB88_PORTR1, MB88_PORTR1) AM_WRITE_LEGACY(namco_54xx_R1_w)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_NOP
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_54xx )
	MCFG_CPU_ADD("mcu", MB8844, DERIVED_CLOCK(1,1))     /* parent clock, internally divided by 6 */
	MCFG_CPU_IO_MAP(namco_54xx_map_io)
MACHINE_CONFIG_END


ROM_START( namco_54xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "54xx.bin",     0x0000, 0x0400, CRC(ee7357e0) SHA1(01bdf984a49e8d0cc8761b2cc162fd6434d5afbe) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_54xx )
{
	namco_54xx_config *config = (namco_54xx_config *)device->static_config();
	namco_54xx_state *state = get_safe_token(device);
	astring tempstring;

	/* find our CPU */
	state->m_cpu = device->subdevice("mcu");
	assert(state->m_cpu != NULL);

	/* find the attached discrete sound device */
	assert(config->discrete != NULL);
	state->m_discrete = device->machine().device(config->discrete);
	assert(state->m_discrete != NULL);
	state->m_basenode = config->firstnode;
}


const device_type NAMCO_54XX = &device_creator<namco_54xx_device>;

namco_54xx_device::namco_54xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_54XX, "Namco 54xx", tag, owner, clock)
{
	m_token = global_alloc_clear(namco_54xx_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void namco_54xx_device::device_config_complete()
{
	m_shortname = "namco54";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_54xx_device::device_start()
{
	DEVICE_START_NAME( namco_54xx )(this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_54xx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_54xx  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *namco_54xx_device::device_rom_region() const
{
	return ROM_NAME(namco_54xx );
}
