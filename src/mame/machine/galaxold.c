/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "machine/7474.h"
#include "includes/galaxold.h"

static int irq_line;
static const device_config *int_timer;

static UINT8 _4in1_bank;

void galaxold_7474_9m_2_callback(const device_config *device)
{
	/* Q bar clocks the other flip-flop,
       Q is VBLANK (not visible to the CPU) */
    const device_config *target = devtag_get_device(device->machine, "7474_9m_1");
	ttl7474_clock_w(target, ttl7474_output_comp_r(device));
	ttl7474_update(target);
}

void galaxold_7474_9m_1_callback(const device_config *device)
{
	/* Q goes to the NMI line */
	cputag_set_input_line(device->machine, "maincpu", irq_line, ttl7474_output_r(device) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_HANDLER( galaxold_nmi_enable_w )
{
    const device_config *target = devtag_get_device(space->machine, "7474_9m_1");
	ttl7474_preset_w(target, data);
	ttl7474_update(target);
}


TIMER_DEVICE_CALLBACK( galaxold_interrupt_timer )
{
    const device_config *target = devtag_get_device(timer->machine, "7474_9m_2");

	/* 128V, 64V and 32V go to D */
	ttl7474_d_w(target, (param & 0xe0) != 0xe0);

	/* 16V clocks the flip-flop */
	ttl7474_clock_w(target, param & 0x10);

	param = (param + 0x10) & 0xff;

	timer_device_adjust_oneshot(int_timer, video_screen_get_time_until_pos(timer->machine->primary_screen, param, 0), param);

	ttl7474_update(target);
}


static void machine_reset_common(running_machine *machine, int line)
{
    const device_config *ttl7474_9m_1 = devtag_get_device(machine, "7474_9m_1");
    const device_config *ttl7474_9m_2 = devtag_get_device(machine, "7474_9m_2");
	irq_line = line;

	/* initalize main CPU interrupt generator flip-flops */
	ttl7474_preset_w(ttl7474_9m_2, 1);
	ttl7474_clear_w (ttl7474_9m_2, 1);

	ttl7474_clear_w (ttl7474_9m_1, 1);
	ttl7474_d_w     (ttl7474_9m_1, 0);
	ttl7474_preset_w(ttl7474_9m_1, 0);

	/* start a timer to generate interrupts */
	int_timer = devtag_get_device(machine, "int_timer");
	timer_device_adjust_oneshot(int_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);
}

MACHINE_RESET( galaxold )
{
	machine_reset_common(machine, INPUT_LINE_NMI);
}

MACHINE_RESET( devilfsg )
{
	machine_reset_common(machine, 0);
}


WRITE8_HANDLER( galaxold_coin_lockout_w )
{
	coin_lockout_global_w(space->machine, ~data & 1);
}


WRITE8_HANDLER( galaxold_coin_counter_w )
{
	coin_counter_w(space->machine, offset, data & 0x01);
}

WRITE8_HANDLER( galaxold_coin_counter_1_w )
{
	coin_counter_w(space->machine, 1, data & 0x01);
}

WRITE8_HANDLER( galaxold_coin_counter_2_w )
{
	coin_counter_w(space->machine, 2, data & 0x01);
}


WRITE8_HANDLER( galaxold_leds_w )
{
	set_led_status(space->machine, offset,data & 1);
}


#ifdef UNUSED_FUNCTION
static READ8_HANDLER( checkmaj_protection_r )
{
	switch (cpu_get_pc(space->cpu))
	{
	case 0x0f15:  return 0xf5;
	case 0x0f8f:  return 0x7c;
	case 0x10b3:  return 0x7c;
	case 0x10e0:  return 0x00;
	case 0x10f1:  return 0xaa;
	case 0x1402:  return 0xaa;
	default:
		logerror("Unknown protection read. PC=%04X\n",cpu_get_pc(space->cpu));
	}

	return 0;
}


/* Zig Zag can swap ROMs 2 and 3 as a form of copy protection */
WRITE8_HANDLER( zigzag_sillyprotection_w )
{
	if (data)
	{
		/* swap ROM 2 and 3! */
		memory_set_bank(space->machine, "bank1", 1);
		memory_set_bank(space->machine, "bank2", 0);
	}
	else
	{
		memory_set_bank(space->machine, "bank1", 0);
		memory_set_bank(space->machine, "bank2", 1);
	}
}

DRIVER_INIT( zigzag )
{
	UINT8 *RAM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 2, &RAM[0x2000], 0x1000);
	memory_configure_bank(machine, "bank2", 0, 2, &RAM[0x2000], 0x1000);
	memory_set_bank(machine, "bank1", 0);
	memory_set_bank(machine, "bank2", 1);
}



static READ8_HANDLER( dingo_3000_r )
{
	return 0xaa;
}

static READ8_HANDLER( dingo_3035_r )
{
	return 0x8c;
}

static READ8_HANDLER( dingoe_3001_r )
{
	return 0xaa;
}


DRIVER_INIT( dingoe )
{
	offs_t i;
	UINT8 *rom = memory_region(machine, "maincpu");

	for (i = 0; i < 0x3000; i++)
	{
		UINT8 data_xor;

		/* XOR bit 2 with 4 and 5 with 0 */
		data_xor = BIT(rom[i], 2) << 4 | BIT(rom[i], 5) << 0;
		rom[i] ^= data_xor;


		/* Invert bit 1 */
		if (~rom[i] & 0x02)
			rom[i] = rom[i] | 0x02;
		else
			rom[i] = rom[i] & 0xfd;


		/* Swap bit0 with bit4 */
		if ((i & 0x0f) == 0x02 || (i & 0x0f) == 0x0a || (i & 0x0f) == 0x03 || (i & 0x0f) == 0x0b || (i & 0x0f) == 0x06 || (i & 0x0f) == 0x0e || (i & 0x0f) == 0x07 || (i & 0x0f) == 0x0f)	/* Swap Bit 0 and 4 */
			rom[i] = BITSWAP8(rom[i],7,6,5,0,3,2,1,4);
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3001, 0x3001, 0, 0, dingoe_3001_r);	/* Protection check */

}
#endif


READ8_HANDLER( scramblb_protection_1_r )
{
	switch (cpu_get_pc(space->cpu))
	{
	case 0x01da: return 0x80;
	case 0x01e4: return 0x00;
	default:
		logerror("%04x: read protection 1\n",cpu_get_pc(space->cpu));
		return 0;
	}
}

READ8_HANDLER( scramblb_protection_2_r )
{
	switch (cpu_get_pc(space->cpu))
	{
	case 0x01ca: return 0x90;
	default:
		logerror("%04x: read protection 2\n",cpu_get_pc(space->cpu));
		return 0;
	}
}


WRITE8_HANDLER( _4in1_bank_w )
{
	_4in1_bank = data & 0x03;
	galaxold_gfxbank_w(space, 0, _4in1_bank);
	memory_set_bank(space->machine, "bank1", _4in1_bank);
}

CUSTOM_INPUT( _4in1_fake_port_r )
{
	static const char *const portnames[] = { "FAKE1", "FAKE2", "FAKE3", "FAKE4" };
	int bit_mask = (FPTR)param;

	return (input_port_read(field->port->machine, portnames[_4in1_bank]) & bit_mask) ? 0x01 : 0x00;
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT( pisces )
{
	/* the coin lockout was replaced */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6002, 0x6002, 0, 0, galaxold_gfxbank_w);
}

DRIVER_INIT( checkmaj )
{
	/* for the title screen */
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3800, 0x3800, 0, 0, checkmaj_protection_r);
}

DRIVER_INIT( dingo )
{
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3000, 0x3000, 0, 0, dingo_3000_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3035, 0x3035, 0, 0, dingo_3035_r);
}


static UINT8 decode_mooncrst(UINT8 data,offs_t addr)
{
	UINT8 res;

	res = data;
	if (BIT(data,1)) res ^= 0x40;
	if (BIT(data,5)) res ^= 0x04;
	if ((addr & 1) == 0)
		res = (res & 0xbb) | (BIT(res,6) << 2) | (BIT(res,2) << 6);
	return res;
}

DRIVER_INIT( mooncrsu )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa000, 0xa002, 0, 0, galaxold_gfxbank_w);
}

DRIVER_INIT( mooncrst )
{
	offs_t i, len = memory_region_length(machine, "maincpu");
	UINT8 *rom = memory_region(machine, "maincpu");


	for (i = 0;i < len;i++)
		rom[i] = decode_mooncrst(rom[i],i);

	DRIVER_INIT_CALL(mooncrsu);
}

DRIVER_INIT( mooncrgx )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x6002, 0, 0, galaxold_gfxbank_w);
}

DRIVER_INIT( moonqsr )
{
	offs_t i;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x8000);

	memory_set_decrypted_region(space, 0x0000, 0x7fff, decrypt);

	for (i = 0;i < 0x8000;i++)
		decrypt[i] = decode_mooncrst(rom[i],i);
}

DRIVER_INIT( checkman )
{
/*
                     Encryption Table
                     ----------------
+---+---+---+------+------+------+------+------+------+------+------+
|A2 |A1 |A0 |D7    |D6    |D5    |D4    |D3    |D2    |D1    |D0    |
+---+---+---+------+------+------+------+------+------+------+------+
| 0 | 0 | 0 |D7    |D6    |D5    |D4    |D3    |D2    |D1    |D0^^D6|
| 0 | 0 | 1 |D7    |D6    |D5    |D4    |D3    |D2    |D1^^D5|D0    |
| 0 | 1 | 0 |D7    |D6    |D5    |D4    |D3    |D2^^D4|D1^^D6|D0    |
| 0 | 1 | 1 |D7    |D6    |D5    |D4^^D2|D3    |D2    |D1    |D0^^D5|
| 1 | 0 | 0 |D7    |D6^^D4|D5^^D1|D4    |D3    |D2    |D1    |D0    |
| 1 | 0 | 1 |D7    |D6^^D0|D5^^D2|D4    |D3    |D2    |D1    |D0    |
| 1 | 1 | 0 |D7    |D6    |D5    |D4    |D3    |D2^^D0|D1    |D0    |
| 1 | 1 | 1 |D7    |D6    |D5    |D4^^D1|D3    |D2    |D1    |D0    |
+---+---+---+------+------+------+------+------+------+------+------+

For example if A2=1, A1=1 and A0=0 then D2 to the CPU would be an XOR of
D2 and D0 from the ROM's. Note that D7 and D3 are not encrypted.

Encryption PAL 16L8 on cardridge
         +--- ---+
    OE --|   U   |-- VCC
 ROMD0 --|       |-- D0
 ROMD1 --|       |-- D1
 ROMD2 --|VER 5.2|-- D2
    A0 --|       |-- NOT USED
    A1 --|       |-- A2
 ROMD4 --|       |-- D4
 ROMD5 --|       |-- D5
 ROMD6 --|       |-- D6
   GND --|       |-- M1 (NOT USED)
         +-------+
Pin layout is such that links can replace the PAL if encryption is not used.

*/
	static const UINT8 xortable[8][4] =
	{
		{ 6,0,6,0 },
		{ 5,1,5,1 },
		{ 4,2,6,1 },
		{ 2,4,5,0 },
		{ 4,6,1,5 },
		{ 0,6,2,5 },
		{ 0,2,0,2 },
		{ 1,4,1,4 }
	};

	offs_t i, len = memory_region_length(machine, "maincpu");
	UINT8 *rom = memory_region(machine, "maincpu");


	for (i = 0; i < len; i++)
	{
		UINT8 data_xor;
		int line = i & 0x07;

		data_xor = (BIT(rom[i],xortable[line][0]) << xortable[line][1]) |
				   (BIT(rom[i],xortable[line][2]) << xortable[line][3]);

		rom[i] ^= data_xor;
	}
}
#endif

DRIVER_INIT( 4in1 )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	offs_t i, len = memory_region_length(machine, "maincpu");
	UINT8 *RAM = memory_region(machine, "maincpu");

	/* Decrypt Program Roms */
	for (i = 0; i < len; i++)
		RAM[i] = RAM[i] ^ (i & 0xff);

	/* games are banked at 0x0000 - 0x3fff */
	memory_configure_bank(machine, "bank1", 0, 4, &RAM[0x10000], 0x4000);

	_4in1_bank_w(space, 0, 0); /* set the initial CPU bank */

	state_save_register_global(machine, _4in1_bank);
}

INTERRUPT_GEN( hunchbks_vh_interrupt )
{
	generic_pulse_irq_line_and_vector(device,0,0x03);
}

DRIVER_INIT( ladybugg )
{
/* Doesn't actually use the bank, but it mustn't have a coin lock! */
memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6002, 0x6002, 0, 0, galaxold_gfxbank_w);
}
