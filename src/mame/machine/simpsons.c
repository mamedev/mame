#include "driver.h"
#include "video/konicdev.h"
#include "cpu/konami/konami.h"
#include "machine/eepromdev.h"
#include "sound/k053260.h"
#include "includes/simpsons.h"

int simpsons_firq_enabled;

/***************************************************************************

  EEPROM

***************************************************************************/

WRITE8_HANDLER( simpsons_eeprom_w )
{
	if (data == 0xff)
		return;

	input_port_write(space->machine, "EEPROMOUT", data, 0xff);

	simpsons_video_banking(space->machine, data & 3);

	simpsons_firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_HANDLER( simpsons_coin_counter_w )
{
	const device_config *k053246 = devtag_get_device(space->machine, "k053246");
	const device_config *k052109 = devtag_get_device(space->machine, "k052109");

	/* bit 0,1 coin counters */
	coin_counter_w(space->machine, 0,data & 0x01);
	coin_counter_w(space->machine, 1,data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	k053246_set_objcha_line(k053246, (~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
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
