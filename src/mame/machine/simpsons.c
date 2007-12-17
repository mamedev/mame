#include "driver.h"
#include "video/konamiic.h"
#include "cpu/konami/konami.h"
#include "machine/eeprom.h"
#include "sound/k053260.h"

/* from video */
extern void simpsons_video_banking( int select );
extern UINT8 *simpsons_xtraram;

int simpsons_firq_enabled;

/***************************************************************************

  EEPROM

***************************************************************************/

static int init_eeprom_count;


static struct EEPROM_interface eeprom_interface =
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
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface);

		if (file)
		{
			init_eeprom_count = 0;
			EEPROM_load(file);
		}
		else
			init_eeprom_count = 10;
	}
}

READ8_HANDLER( simpsons_eeprom_r )
{
	int res;

	res = (EEPROM_read_bit() << 4);

	res |= 0x20;//konami_eeprom_ack() << 5; /* add the ack */

	res |= readinputport( 5 ) & 1; /* test switch */

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

	EEPROM_write_bit(data & 0x80);
	EEPROM_set_cs_line((data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	EEPROM_set_clock_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	simpsons_video_banking( data & 3 );

	simpsons_firq_enabled = data & 0x04;
}

/***************************************************************************

  Coin Counters, Sound Interface

***************************************************************************/

WRITE8_HANDLER( simpsons_coin_counter_w )
{
	/* bit 0,1 coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);
	/* bit 2 selects mono or stereo sound */
	/* bit 3 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	/* bit 4 = INIT (unknown) */
	/* bit 5 = enable sprite ROM reading */
	K053246_set_OBJCHA_line((~data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_HANDLER( simpsons_sound_interrupt_r )
{
	cpunum_set_input_line_and_vector( 1, 0, HOLD_LINE, 0xff );
	return 0x00;
}

READ8_HANDLER( simpsons_sound_r )
{
	return K053260_0_r(2 + offset);
}


/***************************************************************************

  Banking, initialization

***************************************************************************/

static void simpsons_banking( int lines )
{
	memory_set_bank(1, lines & 0x3f);
}

MACHINE_RESET( simpsons )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	cpunum_set_info_fct(0, CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)simpsons_banking);

	paletteram = &RAM[0x88000];
	simpsons_xtraram = &RAM[0x89000];
	spriteram16 = (UINT16 *)&RAM[0x8a000];

	simpsons_firq_enabled = 0;

	/* init the default banks */
	memory_configure_bank(1, 0, 64, memory_region(REGION_CPU1) + 0x10000, 0x2000);
	memory_set_bank(1, 0);

	memory_configure_bank(2, 0, 2, memory_region(REGION_CPU2) + 0x10000, 0);
	memory_configure_bank(2, 2, 6, memory_region(REGION_CPU2) + 0x10000, 0x4000);
	memory_set_bank(2, 0);

	simpsons_video_banking( 0 );
}
