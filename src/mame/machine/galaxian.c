/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "machine/7474.h"
#include "includes/galaxian.h"

static int irq_line;
static emu_timer *int_timer;

static UINT8 kingball_speech_dip;
static UINT8 kingball_sound;

static UINT8 _4in1_bank;
static UINT8 gmgalax_selected_game;


static void galaxian_7474_9M_2_callback(void)
{
	/* Q bar clocks the other flip-flop,
       Q is VBLANK (not visible to the CPU) */
	TTL7474_clock_w(1, TTL7474_output_comp_r(0));
	TTL7474_update(1);
}

static void galaxian_7474_9M_1_callback(void)
{
	/* Q goes to the NMI line */
	cpunum_set_input_line(Machine, 0, irq_line, TTL7474_output_r(1) ? CLEAR_LINE : ASSERT_LINE);
}

static const struct TTL7474_interface galaxian_7474_9M_2_intf =
{
	galaxian_7474_9M_2_callback
};

static const struct TTL7474_interface galaxian_7474_9M_1_intf =
{
	galaxian_7474_9M_1_callback
};


WRITE8_HANDLER( galaxian_nmi_enable_w )
{
	TTL7474_preset_w(1, data);
	TTL7474_update(1);
}


static TIMER_CALLBACK( interrupt_timer )
{
	/* 128V, 64V and 32V go to D */
	TTL7474_d_w(0, (param & 0xe0) != 0xe0);

	/* 16V clocks the flip-flop */
	TTL7474_clock_w(0, param & 0x10);

	param = (param + 0x10) & 0xff;

	timer_adjust_oneshot(int_timer, video_screen_get_time_until_pos(0, param, 0), param);

	TTL7474_update(0);
}


static void machine_reset_common( int line )
{
	irq_line = line;

	/* initalize main CPU interrupt generator flip-flops */
	TTL7474_config(0, &galaxian_7474_9M_2_intf);
	TTL7474_preset_w(0, 1);
	TTL7474_clear_w (0, 1);

	TTL7474_config(1, &galaxian_7474_9M_1_intf);
	TTL7474_clear_w (1, 1);
	TTL7474_d_w     (1, 0);
	TTL7474_preset_w(1, 0);

	/* start a timer to generate interrupts */
	int_timer = timer_alloc(interrupt_timer, NULL);
	timer_adjust_oneshot(int_timer, video_screen_get_time_until_pos(0, 0, 0), 0);
}

MACHINE_RESET( galaxian )
{
	machine_reset_common(INPUT_LINE_NMI);
}

MACHINE_RESET( devilfsg )
{
	machine_reset_common(0);
}



WRITE8_HANDLER( galaxian_coin_lockout_w )
{
	coin_lockout_global_w(~data & 1);
}


WRITE8_HANDLER( galaxian_coin_counter_w )
{
	coin_counter_w(offset, data & 0x01);
}

WRITE8_HANDLER( galaxian_coin_counter_1_w )
{
	coin_counter_w(1, data & 0x01);
}

WRITE8_HANDLER( galaxian_coin_counter_2_w )
{
	coin_counter_w(2, data & 0x01);
}


WRITE8_HANDLER( galaxian_leds_w )
{
	set_led_status(offset,data & 1);
}


READ8_HANDLER( jumpbug_protection_r )
{
	switch (offset)
	{
	case 0x0114:  return 0x4f;
	case 0x0118:  return 0xd3;
	case 0x0214:  return 0xcf;
	case 0x0235:  return 0x02;
	case 0x0311:  return 0x00;  /* not checked */
	default:
		logerror("Unknown protection read. Offset: %04X  PC=%04X\n",0xb000+offset,activecpu_get_pc());
	}

	return 0;
}



static READ8_HANDLER( checkmaj_protection_r )
{
	switch (activecpu_get_pc())
	{
	case 0x0f15:  return 0xf5;
	case 0x0f8f:  return 0x7c;
	case 0x10b3:  return 0x7c;
	case 0x10e0:  return 0x00;
	case 0x10f1:  return 0xaa;
	case 0x1402:  return 0xaa;
	default:
		logerror("Unknown protection read. PC=%04X\n",activecpu_get_pc());
	}

	return 0;
}


/* Zig Zag can swap ROMs 2 and 3 as a form of copy protection */
WRITE8_HANDLER( zigzag_sillyprotection_w )
{
	if (data)
	{
		/* swap ROM 2 and 3! */
		memory_set_bank(1, 1);
		memory_set_bank(2, 0);
	}
	else
	{
		memory_set_bank(1, 0);
		memory_set_bank(2, 1);
	}
}

DRIVER_INIT( zigzag )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	memory_configure_bank(1, 0, 2, &RAM[0x2000], 0x1000);
	memory_configure_bank(2, 0, 2, &RAM[0x2000], 0x1000);
	memory_set_bank(1, 0);
	memory_set_bank(2, 1);
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
	UINT8 *rom = memory_region(REGION_CPU1);

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

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3001, 0x3001, 0, 0, dingoe_3001_r);	/* Protection check */

}



/* Hack? If $b003 is high, we'll check our "fake" speech dipswitch (marked as SLAM) */
static READ8_HANDLER( kingball_IN0_r )
{
	if (kingball_speech_dip)
		return (readinputport(0) & ~0x40) | ((readinputport(3) & 0x01) << 6);
	else
		return readinputport(0);
}

static READ8_HANDLER( kingball_IN1_r )
{
	/* bit 5 is the NOISE line from the sound circuit.  The code just verifies
       that it's working, doesn't actually use return value, so we can just use
       rand() */

	return (readinputport(1) & ~0x20) | (mame_rand(Machine) & 0x20);
}

WRITE8_HANDLER( kingball_speech_dip_w )
{
	kingball_speech_dip = data;
}

WRITE8_HANDLER( kingball_sound1_w )
{
	kingball_sound = (kingball_sound & ~0x01) | data;
}

WRITE8_HANDLER( kingball_sound2_w )
{
	kingball_sound = (kingball_sound & ~0x02) | (data << 1);
	soundlatch_w (machine, 0, kingball_sound | 0xf0);
}


READ8_HANDLER( scramblb_protection_1_r )
{
	switch (activecpu_get_pc())
	{
	case 0x01da: return 0x80;
	case 0x01e4: return 0x00;
	default:
		logerror("%04x: read protection 1\n",activecpu_get_pc());
		return 0;
	}
}

READ8_HANDLER( scramblb_protection_2_r )
{
	switch (activecpu_get_pc())
	{
	case 0x01ca: return 0x90;
	default:
		logerror("%04x: read protection 2\n",activecpu_get_pc());
		return 0;
	}
}


static READ8_HANDLER( azurian_IN1_r )
{
	return (readinputport(1) & ~0x40) | ((readinputport(3) & 0x01) << 6);
}

static READ8_HANDLER( azurian_IN2_r )
{
	return (readinputport(2) & ~0x04) | ((readinputport(3) & 0x02) << 1);
}


WRITE8_HANDLER( _4in1_bank_w )
{
	_4in1_bank = data & 0x03;
	galaxian_gfxbank_w(machine, 0, _4in1_bank);
	memory_set_bank(1, _4in1_bank);
}

READ8_HANDLER( _4in1_input_port_1_r )
{
	return (readinputport(1) & ~0xc0) | (readinputport(3+_4in1_bank) & 0xc0);
}

READ8_HANDLER( _4in1_input_port_2_r )
{
	return (readinputport(2) & 0x04) | (readinputport(3+_4in1_bank) & ~0xc4);
}


static void gmgalax_select_game(int game)
{
	gmgalax_selected_game = game;

	memory_set_bank(1, game);

	galaxian_gfxbank_w(Machine, 0, gmgalax_selected_game);
}

READ8_HANDLER( gmgalax_input_port_0_r )
{
	return readinputport(gmgalax_selected_game ? 3 : 0);
}

READ8_HANDLER( gmgalax_input_port_1_r )
{
	return readinputport(gmgalax_selected_game ? 4 : 1);
}

READ8_HANDLER( gmgalax_input_port_2_r )
{
	return readinputport(gmgalax_selected_game ? 5 : 2);
}


DRIVER_INIT( pisces )
{
	/* the coin lockout was replaced */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6002, 0x6002, 0, 0, galaxian_gfxbank_w);
}

DRIVER_INIT( checkmaj )
{
	/* for the title screen */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3800, 0x3800, 0, 0, checkmaj_protection_r);
}

DRIVER_INIT( dingo )
{
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3000, 0x3000, 0, 0, dingo_3000_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3035, 0x3035, 0, 0, dingo_3035_r);
}

DRIVER_INIT( kingball )
{
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa000, 0xa000, 0, 0, kingball_IN0_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa800, 0xa800, 0, 0, kingball_IN1_r);

	state_save_register_global(kingball_speech_dip);
	state_save_register_global(kingball_sound);
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
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa000, 0xa002, 0, 0, galaxian_gfxbank_w);
}

DRIVER_INIT( mooncrst )
{
	offs_t i;
	UINT8 *rom = memory_region(REGION_CPU1);


	for (i = 0;i < memory_region_length(REGION_CPU1);i++)
		rom[i] = decode_mooncrst(rom[i],i);

	DRIVER_INIT_CALL(mooncrsu);
}

DRIVER_INIT( mooncrgx )
{
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6002, 0, 0, galaxian_gfxbank_w);
}

DRIVER_INIT( moonqsr )
{
	offs_t i;
	UINT8 *rom = memory_region(REGION_CPU1);
	UINT8 *decrypt = auto_malloc(0x8000);

	memory_set_decrypted_region(0, 0x0000, 0x7fff, decrypt);

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

	offs_t i;
	UINT8 *rom = memory_region(REGION_CPU1);


	for (i = 0; i < memory_region_length(REGION_CPU1); i++)
	{
		UINT8 data_xor;
		int line = i & 0x07;

		data_xor = (BIT(rom[i],xortable[line][0]) << xortable[line][1]) |
				   (BIT(rom[i],xortable[line][2]) << xortable[line][3]);

		rom[i] ^= data_xor;
	}
}

DRIVER_INIT( gteikob2 )
{
	DRIVER_INIT_CALL(pisces);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x7006, 0x7006, 0, 0, gteikob2_flip_screen_x_w);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x7007, 0x7007, 0, 0, gteikob2_flip_screen_y_w);
}

DRIVER_INIT( azurian )
{
	DRIVER_INIT_CALL(pisces);

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6800, 0x6800, 0, 0, azurian_IN1_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x7000, 0x7000, 0, 0, azurian_IN2_r);
}

DRIVER_INIT( 4in1 )
{
	offs_t i;
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* Decrypt Program Roms */
	for (i = 0; i < memory_region_length(REGION_CPU1); i++)
		RAM[i] = RAM[i] ^ (i & 0xff);

	/* games are banked at 0x0000 - 0x3fff */
	memory_configure_bank(1, 0, 4, &RAM[0x10000], 0x4000);

	_4in1_bank_w(machine, 0, 0); /* set the initial CPU bank */

	state_save_register_global(_4in1_bank);
}

INTERRUPT_GEN( hunchbks_vh_interrupt )
{
	cpunum_set_input_line_and_vector(machine, 0,0,PULSE_LINE,0x03);
}

DRIVER_INIT( ladybugg )
{
/* Doesn't actually use the bank, but it mustn't have a coin lock! */
memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6002, 0x6002, 0, 0, galaxian_gfxbank_w);
}

DRIVER_INIT( gmgalax )
{
	/* games are banked at 0x0000 - 0x3fff */
	UINT8 *RAM=memory_region(REGION_CPU1);
	memory_configure_bank(1, 0, 2, &RAM[0x10000], 0x4000);

	state_save_register_global(gmgalax_selected_game);

	gmgalax_select_game(input_port_6_r(machine, 0) & 0x01);
}

INTERRUPT_GEN( gmgalax_vh_interrupt )
{
	// reset the cpu if the selected game changed
	int new_game = input_port_6_r(machine, 0) & 0x01;

	if (gmgalax_selected_game != new_game)
	{
		gmgalax_select_game(new_game);

		/* Ghost Muncher never clears this */
		galaxian_stars_enable_w(machine, 0, 0);

		cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, ASSERT_LINE);
	}
}
