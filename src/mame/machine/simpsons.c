#include "driver.h"
#include "video/konamiic.h"
#include "cpu/konami/konami.h"
#include "machine/eeprom.h"
#include "sound/k053260.h"
#include "includes/simpsons.h"

int simpsons_firq_enabled;

/***************************************************************************

  EEPROM

***************************************************************************/

static int init_eeprom_count;


static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	0,				/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

NVRAM_HANDLER( simpsons )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(machine, &eeprom_intf);

		if (file)
		{
			init_eeprom_count = 0;
			eeprom_load(file);
		}
		else
			init_eeprom_count = 10;
	}
}

READ8_HANDLER( simpsons_eeprom_r )
{
	int res;

	res = (eeprom_read_bit() << 4);

	res |= 0x20;//konami_eeprom_ack() << 5; /* add the ack */

	res |= input_port_read(space->machine, "TEST") & 1; /* test switch */

	if (init_eeprom_count)
	{
		init_eeprom_count--;
		res &= 0xfe;
	}
	return res;
}

WRITE8_HANDLER( simpsons_eeprom_w )
{
	if ( data == 0xff )
		return;

	eeprom_write_bit(data & 0x80);
	eeprom_set_cs_line((data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	eeprom_set_clock_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	simpsons_video_banking( space->machine, data & 3 );

	simpsons_firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_HANDLER( simpsons_coin_counter_w )
{
	/* bit 0,1 coin counters */
	coin_counter_w(space->machine, 0,data & 0x01);
	coin_counter_w(space->machine, 1,data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	K053246_set_OBJCHA_line((~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_HANDLER( simpsons_sound_interrupt_r )
{
	cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff );
	return 0x00;
}

READ8_DEVICE_HANDLER( simpsons_sound_r )
{
	return k053260_r(device, 2 + offset);
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( simpsons_banking )
{
	memory_set_bank(device->machine, "bank1", lines & 0x3f);
}

MACHINE_RESET( simpsons )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	konami_configure_set_lines(cputag_get_cpu(machine, "maincpu"), simpsons_banking);

	machine->generic.paletteram.u8 = &RAM[0x88000];
	simpsons_xtraram = &RAM[0x89000];
	machine->generic.spriteram.u16 = (UINT16 *)&RAM[0x8a000];

	simpsons_firq_enabled = 0;

	/* init the default banks */
	memory_configure_bank(machine, "bank1", 0, 64, memory_region(machine, "maincpu") + 0x10000, 0x2000);
	memory_set_bank(machine, "bank1", 0);

	memory_configure_bank(machine, "bank2", 0, 2, memory_region(machine, "audiocpu") + 0x10000, 0);
	memory_configure_bank(machine, "bank2", 2, 6, memory_region(machine, "audiocpu") + 0x10000, 0x4000);
	memory_set_bank(machine, "bank2", 0);

	simpsons_video_banking( machine, 0 );
}
