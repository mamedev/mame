/*\
* * Linus Akesson's "Craft"
* *
* * Driver by MooglyGuy
\*/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"

#define VERBOSE_LEVEL	(0)

#define ENABLE_VERBOSE_LOG (0)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif

#define MASTER_CLOCK	20000000

#define VISIBLE_CYCLES		480
#define HSYNC_CYCLES		155
#define LINE_CYCLES 		(VISIBLE_CYCLES + HSYNC_CYCLES)
#define VISIBLE_LINES		480
#define VSYNC_LINES			45
#define LINES_PER_FRAME		(VISIBLE_LINES + VSYNC_LINES)
#define CYCLES_PER_FRAME	(LINES_PER_FRAME * LINE_CYCLES)
#define PIXELS_PER_FRAME	(CYCLES_PER_FRAME)

/****************************************************\
* I/O defines                                        *
\****************************************************/

#define AVR8_PORTD				(state->m_regs[AVR8_REGIDX_PORTD])
#define AVR8_DDRD				(state->m_regs[AVR8_REGIDX_DDRD])
#define AVR8_PORTC				(state->m_regs[AVR8_REGIDX_PORTC])
#define AVR8_DDRC				(state->m_regs[AVR8_REGIDX_DDRC])
#define AVR8_PORTB				(state->m_regs[AVR8_REGIDX_PORTB])
#define AVR8_DDRB				(state->m_regs[AVR8_REGIDX_DDRB])

#define AVR8_SPSR				(state->m_regs[AVR8_REGIDX_SPSR])
#define AVR8_SPSR_SPR2X			(AVR8_SPSR & AVR8_SPSR_SPR2X_MASK)

#define AVR8_SPCR				(state->m_regs[AVR8_REGIDX_SPCR])
#define AVR8_SPCR_SPIE			((AVR8_SPCR & AVR8_SPCR_SPIE_MASK) >> 7)
#define AVR8_SPCR_SPE			((AVR8_SPCR & AVR8_SPCR_SPE_MASK) >> 6)
#define AVR8_SPCR_DORD			((AVR8_SPCR & AVR8_SPCR_DORD_MASK) >> 5)
#define AVR8_SPCR_MSTR			((AVR8_SPCR & AVR8_SPCR_MSTR_MASK) >> 4)
#define AVR8_SPCR_CPOL			((AVR8_SPCR & AVR8_SPCR_CPOL_MASK) >> 3)
#define AVR8_SPCR_CPHA			((AVR8_SPCR & AVR8_SPCR_CPHA_MASK) >> 2)
#define AVR8_SPCR_SPR			(AVR8_SPCR & AVR8_SPCR_SPR_MASK)


/****************************************************\
* I/O devices                                        *
\****************************************************/

class craft_state : public driver_device
{
public:
	craft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu")
	{
	}

	virtual void machine_start();

    dac_device* dac;

	UINT8 m_regs[0x100];
    UINT8* m_eeprom;
    UINT32 m_last_cycles;

    bool m_spi_pending;
    UINT64 m_spi_start_cycle;

    UINT64 m_frame_start_cycle;

    UINT8 m_pixels[PIXELS_PER_FRAME];

    required_device<cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER(avr8_read);
	DECLARE_WRITE8_MEMBER(avr8_write);
	DECLARE_DRIVER_INIT(craft);
	virtual void machine_reset();
	UINT32 screen_update_craft(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(avr8_timer0_tick);
	TIMER_CALLBACK_MEMBER(avr8_timer1_tick);
	TIMER_CALLBACK_MEMBER(avr8_timer2_tick);
};

void craft_state::machine_start()
{
}

READ8_MEMBER(craft_state::avr8_read)
{
    switch( offset )
    {
		case AVR8_REGIDX_EEDR:
		case AVR8_REGIDX_PORTC:
		case AVR8_REGIDX_PORTD:
			return m_regs[offset];

        default:
            verboselog(machine(), 0, "AVR8: Unrecognized register read: %02x\n", offset );
    }

    return 0;
}

static UINT8 avr8_get_ddr(running_machine &machine, int reg)
{
    craft_state *state = machine.driver_data<craft_state>();

	switch(reg)
	{
		case AVR8_REG_B:
			return AVR8_DDRB;

		case AVR8_REG_C:
			return AVR8_DDRC;

		case AVR8_REG_D:
			return AVR8_DDRD;

		default:
			verboselog(machine, 0, "avr8_get_ddr: Unsupported register retrieval: %c\n", avr8_reg_name[reg]);
			break;
	}

	return 0;
}

static void avr8_change_ddr(running_machine &machine, int reg, UINT8 data)
{
    //craft_state *state = machine.driver_data<craft_state>();

	UINT8 oldddr = avr8_get_ddr(machine, reg);
	UINT8 newddr = data;
	UINT8 changed = newddr ^ oldddr;
	// TODO: When AVR8 is converted to emu/machine, this should be factored out to 8 single-bit callbacks per port
	if(changed)
	{
		// TODO
		verboselog(machine, 0, "avr8_change_ddr: DDR%c lines %02x changed\n", avr8_reg_name[reg], changed);
	}
}

static void avr8_video_update(running_machine &machine)
{
    craft_state *state = machine.driver_data<craft_state>();

	UINT64 cycles = avr8_get_elapsed_cycles(state->m_maincpu);
	UINT32 frame_cycles = (UINT32)(cycles - state->m_frame_start_cycle);

	if (state->m_last_cycles < frame_cycles)
	{
		for (UINT32 pixidx = state->m_last_cycles; pixidx < frame_cycles; pixidx++)
		{
			UINT8 value = AVR8_PORTC & 0x3f;
			if (state->m_spi_pending)
			{
				if (pixidx >= state->m_spi_start_cycle && pixidx < (state->m_spi_start_cycle + 16))
				{
					UINT8 bitidx = 7 - ((pixidx - state->m_spi_start_cycle) >> 1);
					value = ((state->m_regs[AVR8_REGIDX_SPDR] & (1 << bitidx)) ? value : 0x3f);
					if (pixidx == (state->m_spi_start_cycle + 15))
					{
						state->m_spi_pending = false;
						state->m_regs[AVR8_REGIDX_SPDR] = 0;
					}
				}
			}
			state->m_pixels[pixidx] = value;
		}
	}
	else
	{
		memset(state->m_pixels + state->m_last_cycles, 0, sizeof(state->m_pixels) - state->m_last_cycles);
		memset(state->m_pixels, 0, frame_cycles);
	}

	state->m_last_cycles = frame_cycles;
}

static void avr8_change_port(running_machine &machine, int reg, UINT8 data)
{
    craft_state *state = machine.driver_data<craft_state>();

	UINT8 oldport = avr8_get_ddr(machine, reg);
	UINT8 newport = data;
	UINT8 changed = newport ^ oldport;

	// TODO: When AVR8 is converted to emu/machine, this should be factored out to 8 single-bit callbacks per port
	if(changed)
	{
		// TODO
		//verboselog(machine, 0, "avr8_change_port: PORT%c lines %02x changed\n", avr8_reg_name[reg], changed);
	}

	switch(reg)
	{
		case AVR8_REG_A:
			// Unhandled
			break;

		case AVR8_REG_B:
			AVR8_PORTB = data;
			if(newport & changed & 0x02)
			{
				state->m_frame_start_cycle = avr8_get_elapsed_cycles(state->m_maincpu);
			}
			break;

		case AVR8_REG_C:
			avr8_video_update(machine);
			AVR8_PORTC = data;
			break;

		case AVR8_REG_D:
		{
			UINT8 audio_sample = (data & 0x02) | ((data & 0xf4) >> 2);
			state->dac->write_unsigned8(audio_sample << 2);
			AVR8_PORTD = data;
			break;
		}
	}
}

/****************/
/* SPI Handling */
/****************/

static void avr8_enable_spi(running_machine &machine)
{
	// TODO
	verboselog(machine, 0, "avr8_enable_spi: TODO\n");
}

static void avr8_disable_spi(running_machine &machine)
{
	// TODO
	verboselog(machine, 0, "avr8_disable_spi: TODO\n");
}

static void avr8_spi_update_masterslave_select(running_machine &machine)
{
	// TODO
    //craft_state *state = machine.driver_data<craft_state>();
	//verboselog(machine, 0, "avr8_spi_update_masterslave_select: TODO; AVR is %s\n", AVR8_SPCR_MSTR ? "Master" : "Slave");
}

static void avr8_spi_update_clock_polarity(running_machine &machine)
{
	// TODO
    //craft_state *state = machine.driver_data<craft_state>();
	//verboselog(machine, 0, "avr8_spi_update_clock_polarity: TODO; SCK is Active-%s\n", AVR8_SPCR_CPOL ? "Low" : "High");
}

static void avr8_spi_update_clock_phase(running_machine &machine)
{
	// TODO
    //craft_state *state = machine.driver_data<craft_state>();
	//verboselog(machine, 0, "avr8_spi_update_clock_phase: TODO; Sampling edge is %s\n", AVR8_SPCR_CPHA ? "Trailing" : "Leading");
}

static const UINT8 avr8_spi_clock_divisor[8] = { 4, 16, 64, 128, 2, 8, 32, 64 };

static void avr8_spi_update_clock_rate(running_machine &machine)
{
	// TODO
    //craft_state *state = machine.driver_data<craft_state>();
	//verboselog(machine, 0, "avr8_spi_update_clock_rate: TODO; New clock rate should be f/%d\n", avr8_spi_clock_divisor[AVR8_SPCR_SPR] / (AVR8_SPSR_SPR2X ? 2 : 1));
}

static void avr8_change_spcr(running_machine &machine, UINT8 data)
{
    craft_state *state = machine.driver_data<craft_state>();

	UINT8 oldspcr = AVR8_SPCR;
	UINT8 newspcr = data;
	UINT8 changed = newspcr ^ oldspcr;
	UINT8 high_to_low = ~newspcr & oldspcr;
	UINT8 low_to_high = newspcr & ~oldspcr;

	AVR8_SPCR = data;

	if(changed & AVR8_SPCR_SPIE_MASK)
	{
		// Check for SPI interrupt condition
		avr8_update_interrupt(state->m_maincpu, AVR8_INTIDX_SPI);
	}

	if(low_to_high & AVR8_SPCR_SPE_MASK)
	{
		avr8_enable_spi(machine);
	}
	else if(high_to_low & AVR8_SPCR_SPE_MASK)
	{
		avr8_disable_spi(machine);
	}

	if(changed & AVR8_SPCR_MSTR_MASK)
	{
		avr8_spi_update_masterslave_select(machine);
	}

	if(changed & AVR8_SPCR_CPOL_MASK)
	{
		avr8_spi_update_clock_polarity(machine);
	}

	if(changed & AVR8_SPCR_CPHA_MASK)
	{
		avr8_spi_update_clock_phase(machine);
	}

	if(changed & AVR8_SPCR_SPR_MASK)
	{
		avr8_spi_update_clock_rate(machine);
	}
}

static void avr8_change_spsr(running_machine &machine, UINT8 data)
{
    craft_state *state = machine.driver_data<craft_state>();

	UINT8 oldspsr = AVR8_SPSR;
	UINT8 newspsr = data;
	UINT8 changed = newspsr ^ oldspsr;
	//UINT8 high_to_low = ~newspsr & oldspsr;
	//UINT8 low_to_high = newspsr & ~oldspsr;

	AVR8_SPSR &= ~1;
	AVR8_SPSR |= data & 1;

	if(changed & AVR8_SPSR_SPR2X_MASK)
	{
		avr8_spi_update_clock_rate(machine);
	}
}

WRITE8_MEMBER(craft_state::avr8_write)
{
    switch( offset )
    {
		case AVR8_REGIDX_SPSR:
			avr8_change_spsr(machine(), data);
			break;

		case AVR8_REGIDX_SPCR:
			avr8_change_spcr(machine(), data);
			break;

		case AVR8_REGIDX_SPDR:
			avr8_video_update(machine());
			m_regs[offset] = data;
			m_spi_pending = true;
			m_spi_start_cycle = avr8_get_elapsed_cycles(m_maincpu) - m_frame_start_cycle;
			break;

		case AVR8_REGIDX_EECR:
			if (data & AVR8_EECR_EERE)
			{
				UINT16 addr = (m_regs[AVR8_REGIDX_EEARH] & AVR8_EEARH_MASK) << 8;
				addr |= m_regs[AVR8_REGIDX_EEARL];
				m_regs[AVR8_REGIDX_EEDR] = m_eeprom[addr];
			}
			break;

        case AVR8_REGIDX_EEARL:
        case AVR8_REGIDX_EEARH:
			m_regs[offset] = data;
			break;

        case AVR8_REGIDX_PORTD:
            avr8_change_port(machine(), AVR8_REG_D, data);
            break;

		case AVR8_REGIDX_DDRD:
			avr8_change_ddr(machine(), AVR8_REG_D, data);
			break;

		case AVR8_REGIDX_PORTC:
			avr8_change_port(machine(), AVR8_REG_C, data);
			break;

		case AVR8_REGIDX_DDRC:
			avr8_change_ddr(machine(), AVR8_REG_C, data);
			break;

		case AVR8_REGIDX_PORTB:
			avr8_change_port(machine(), AVR8_REG_B, data);
			break;

		case AVR8_REGIDX_DDRB:
			avr8_change_ddr(machine(), AVR8_REG_B, data);
			break;

        default:
            verboselog(machine(), 0, "AVR8: Unrecognized register write: %02x = %02x\n", offset, data );
    }
}

/****************************************************\
* Address maps                                       *
\****************************************************/

static ADDRESS_MAP_START( craft_prg_map, AS_PROGRAM, 8, craft_state )
    AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( craft_io_map, AS_IO, 8, craft_state )
    AM_RANGE(0x0000, 0x00ff) AM_READWRITE(avr8_read, avr8_write)
    AM_RANGE(0x0100, 0x04ff) AM_RAM
ADDRESS_MAP_END

/****************************************************\
* Input ports                                        *
\****************************************************/

static INPUT_PORTS_START( craft )
    PORT_START("MAIN")
        PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

/****************************************************\
* Video hardware                                     *
\****************************************************/

UINT32 craft_state::screen_update_craft(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for(int y = 0; y < LINES_PER_FRAME; y++)
	{
		UINT32 *line = &bitmap.pix32(y);
		for(int x = 0; x < LINE_CYCLES; x++)
		{
			UINT8 pixel = m_pixels[y * LINE_CYCLES + x];
			UINT8 r = 0x55 * ((pixel & 0x30) >> 4);
			UINT8 g = 0x55 * ((pixel & 0x0c) >> 2);
			UINT8 b = 0x55 * (pixel & 0x03);
			line[x] = 0xff000000 | (r << 16) | (g << 8) | b;
		}
	}
    return 0;
}

/****************************************************\
* Machine definition                                 *
\****************************************************/

DRIVER_INIT_MEMBER(craft_state,craft)
{
}

void craft_state::machine_reset()
{
    craft_state *state = machine().driver_data<craft_state>();

    state->dac = machine().device<dac_device>("dac");

    state->dac->write_unsigned8(0x00);

    state->m_eeprom = memregion("eeprom")->base();

    state->m_frame_start_cycle = 0;
    state->m_last_cycles = 0;

    state->m_spi_pending = false;

    state->m_spi_start_cycle = 0;
}

static MACHINE_CONFIG_START( craft, craft_state )

    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu", ATMEGA88, MASTER_CLOCK)
    MCFG_CPU_PROGRAM_MAP(craft_prg_map)
    MCFG_CPU_IO_MAP(craft_io_map)


    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(59.99)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1429)) /* accurate */
    MCFG_SCREEN_SIZE(635, 525)
    MCFG_SCREEN_VISIBLE_AREA(47, 526, 36, 515)
	MCFG_SCREEN_UPDATE_DRIVER(craft_state, screen_update_craft)
    MCFG_PALETTE_LENGTH(0x1000)

    /* sound hardware */
    MCFG_SPEAKER_STANDARD_MONO("avr8")
    MCFG_SOUND_ADD("dac", DAC, 0)
    MCFG_SOUND_ROUTE(0, "avr8", 1.00)
MACHINE_CONFIG_END

ROM_START( craft )
	ROM_REGION( 0x2000, "maincpu", 0 )  /* Main program store */
	ROM_LOAD( "craft.bin", 0x0000, 0x2000, CRC(2e6f9ad2) SHA1(75e495bf18395d74289ca7ee2649622fc4010457) )
	ROM_REGION( 0x200, "eeprom", 0 )  /* on-die eeprom */
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(e18a2af9) SHA1(81fc6f2d391edfd3244870214fac37929af0ac0c) )
ROM_END

/*   YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY          FULLNAME */
CONS(2008, craft,    0,        0,        craft,    craft, craft_state,    craft,    "Linus Akesson", "Craft", GAME_IMPERFECT_GRAPHICS)
