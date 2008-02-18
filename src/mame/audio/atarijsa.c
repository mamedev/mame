/***************************************************************************

Atari Audio Board II
--------------------

6502 MEMORY MAP

Function                                  Address     R/W  Data
---------------------------------------------------------------
Program RAM                               0000-1FFF   R/W  D0-D7

Music (YM-2151)                           2000-2001   R/W  D0-D7

Read 68010 Port (Input Buffer)            280A        R    D0-D7

Self-test                                 280C        R    D7
Output Buffer Full (@2A02) (Active High)              R    D5
Left Coin Switch                                      R    D1
Right Coin Switch                                     R    D0

Interrupt acknowledge                     2A00        W    xx
Write 68010 Port (Outbut Buffer)          2A02        W    D0-D7
Banked ROM select (at 3000-3FFF)          2A04        W    D6-D7
???                                       2A06        W

Effects                                   2C00-2C0F   R/W  D0-D7

Banked Program ROM (4 pages)              3000-3FFF   R    D0-D7
Static Program ROM (48K bytes)            4000-FFFF   R    D0-D7

****************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "sound/5220intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"


#define JSA_MASTER_CLOCK			XTAL_3_579545MHz


static UINT8 *bank_base;
static UINT8 *bank_source_data;

static UINT8 speech_data;
static UINT8 last_ctl;

static int cpu_num;
static UINT8 test_port;
static UINT16 test_mask;

static UINT8 has_pokey;
static UINT8 has_ym2151;
static UINT8 has_tms5220;
static UINT8 has_oki6295;

static UINT32 oki6295_bank_base;

static UINT8 overall_volume;
static UINT8 pokey_volume;
static UINT8 ym2151_volume;
static UINT8 tms5220_volume;
static UINT8 oki6295_volume;

static void update_all_volumes(running_machine *machine);

static READ8_HANDLER( jsa1_io_r );
static WRITE8_HANDLER( jsa1_io_w );
static READ8_HANDLER( jsa2_io_r );
static WRITE8_HANDLER( jsa2_io_w );
static READ8_HANDLER( jsa3_io_r );
static WRITE8_HANDLER( jsa3_io_w );


/*************************************
 *
 *  State save
 *
 *************************************/

static void init_save_state(void)
{
	state_save_register_global(speech_data);
	state_save_register_global(last_ctl);

	state_save_register_global(oki6295_bank_base);

	state_save_register_global(overall_volume);
	state_save_register_global(pokey_volume);
	state_save_register_global(ym2151_volume);
	state_save_register_global(tms5220_volume);
	state_save_register_global(oki6295_volume);
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

void atarijsa_init(running_machine *machine, int testport, int testmask)
{
	int i;

	/* copy in the parameters */
	cpu_num = mame_find_cpu_index(machine, "jsa");
	assert_always(cpu_num != -1, "Could not find JSA CPU!");
	test_port = testport;
	test_mask = testmask;

	/* predetermine the bank base */
	bank_base = &memory_region(REGION_CPU1+cpu_num)[0x03000];
	bank_source_data = &memory_region(REGION_CPU1+cpu_num)[0x10000];

	/* determine which sound hardware is installed */
	has_tms5220 = has_oki6295 = has_pokey = has_ym2151 = 0;
	for (i = 0; i < MAX_SOUND; i++)
	{
		switch (Machine->config->sound[i].type)
		{
			case SOUND_TMS5220:
				has_tms5220 = 1;
				break;
			case SOUND_OKIM6295:
				has_oki6295 = 1;
				break;
			case SOUND_POKEY:
				has_pokey = 1;
				break;
			case SOUND_YM2151:
				has_ym2151 = 1;
				break;
			default:
				break;
		}
	}

	/* install POKEY memory handlers */
	if (has_pokey)
	{
		memory_install_read8_handler(cpu_num, ADDRESS_SPACE_PROGRAM, 0x2c00, 0x2c0f, 0, 0, pokey1_r);
		memory_install_write8_handler(cpu_num, ADDRESS_SPACE_PROGRAM, 0x2c00, 0x2c0f, 0, 0, pokey1_w);
	}

	init_save_state();
	atarijsa_reset();
}


void atarijsa3_init_adpcm(int region)
{
	UINT8 *base = memory_region(region);

	/* expand the ADPCM data to avoid lots of memcpy's during gameplay */
	/* the upper 128k is fixed, the lower 128k is bankswitched */
	memcpy(&base[0x00000], &base[0x80000], 0x20000);
	memcpy(&base[0x40000], &base[0x80000], 0x20000);
	memcpy(&base[0x80000], &base[0xa0000], 0x20000);

	memcpy(&base[0x20000], &base[0xe0000], 0x20000);
	memcpy(&base[0x60000], &base[0xe0000], 0x20000);
	memcpy(&base[0xa0000], &base[0xe0000], 0x20000);
}


void atarijsa_reset(void)
{
	/* reset the sound I/O system */
	atarigen_sound_io_reset(cpu_num);

	/* reset the static states */
	speech_data = 0;
	last_ctl = 0;
	oki6295_bank_base = 0x00000;
	overall_volume = 100;
	pokey_volume = 100;
	ym2151_volume = 100;
	tms5220_volume = 100;
	oki6295_volume = 100;

	/* Guardians of the Hood assumes we're reset to bank 0 on startup */
	memcpy(bank_base, &bank_source_data[0x0000], 0x1000);
}



/*************************************
 *
 *  JSA I I/O handlers
 *
 *************************************/

static READ8_HANDLER( jsa1_io_r )
{
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* n/c */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(offset);
			break;

		case 0x004:		/* /RDIO */
			/*
                0x80 = self test
                0x40 = NMI line state (active low)
                0x20 = sound output full
                0x10 = TMS5220 ready (active low)
                0x08 = +5V
                0x04 = +5V
                0x02 = coin 2
                0x01 = coin 1
            */
			result = readinputportbytag("JSAI");
			if (!(readinputport(test_port) & test_mask)) result ^= 0x80;
			if (atarigen_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen_sound_to_cpu_ready) result ^= 0x20;
			if (!has_tms5220 || tms5220_ready_r()) result ^= 0x10;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /VOICE */
		case 0x202:		/* /WRP */
		case 0x204:		/* /WRIO */
		case 0x206:		/* /MIX */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;
	}

	return result;
}


static WRITE8_HANDLER( jsa1_io_w )
{
	switch (offset & 0x206)
	{
		case 0x000:		/* n/c */
		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /VOICE */
			speech_data = data;
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(offset, data);
			break;

		case 0x204:		/* WRIO */
			/*
                0xc0 = bank address
                0x20 = coin counter 2
                0x10 = coin counter 1
                0x08 = squeak (tweaks the 5220 frequency)
                0x04 = TMS5220 reset (active low)
                0x02 = TMS5220 write strobe (active low)
                0x01 = YM2151 reset (active low)
            */

			/* handle TMS5220 I/O */
			if (has_tms5220)
			{
				int count;

				if (((data ^ last_ctl) & 0x02) && (data & 0x02))
					tms5220_data_w(0, speech_data);
				count = 5 | ((data >> 2) & 2);
				tms5220_set_frequency(JSA_MASTER_CLOCK*2 / (16 - count));
			}

			/* coin counters */
			coin_counter_w(1, (data >> 5) & 1);
			coin_counter_w(0, (data >> 4) & 1);

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
			last_ctl = data;
			break;

		case 0x206:		/* MIX */
			/*
                0xc0 = TMS5220 volume (0-3)
                0x30 = POKEY volume (0-3)
                0x0e = YM2151 volume (0-7)
                0x01 = low-pass filter enable
            */
			tms5220_volume = ((data >> 6) & 3) * 100 / 3;
			pokey_volume = ((data >> 4) & 3) * 100 / 3;
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			update_all_volumes(Machine);
			break;
	}
}



/*************************************
 *
 *  JSA II I/O handlers
 *
 *************************************/

static READ8_HANDLER( jsa2_io_r )
{
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (has_oki6295)
				result = OKIM6295_status_0_r(offset);
			else
				logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(offset);
			break;

		case 0x004:		/* /RDIO */
			/*
                0x80 = self test
                0x40 = NMI line state (active low)
                0x20 = sound output full
                0x10 = +5V
                0x08 = +5V
                0x04 = +5V
                0x02 = coin 2
                0x01 = coin 1
            */
			result = readinputportbytag("JSAII");
			if (!(readinputport(test_port) & test_mask)) result ^= 0x80;
			if (atarigen_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
		case 0x202:		/* /WRP */
		case 0x204:		/* /WRIO */
		case 0x206:		/* /MIX */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;
	}

	return result;
}


static WRITE8_HANDLER( jsa2_io_w )
{
	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
			if (has_oki6295)
				OKIM6295_data_0_w(offset, data);
			else
				logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(offset, data);
			break;

		case 0x204:		/* /WRIO */
			/*
                0xc0 = bank address
                0x20 = coin counter 2
                0x10 = coin counter 1
                0x08 = voice frequency (tweaks the OKI6295 frequency)
                0x04 = OKI6295 reset (active low)
                0x02 = n/c
                0x01 = YM2151 reset (active low)
            */

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
			last_ctl = data;

			/* coin counters */
			coin_counter_w(1, (data >> 5) & 1);
			coin_counter_w(0, (data >> 4) & 1);

			/* update the OKI frequency */
			if (has_oki6295) OKIM6295_set_pin7(0, data & 8);
			break;

		case 0x206:		/* /MIX */
			/*
                0xc0 = n/c
                0x20 = low-pass filter enable
                0x10 = n/c
                0x0e = YM2151 volume (0-7)
                0x01 = OKI6295 volume (0-1)
            */
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			oki6295_volume = 50 + (data & 1) * 50;
			update_all_volumes(Machine);
			break;
	}
}



/*************************************
 *
 *  JSA III I/O handlers
 *
 *************************************/

static READ8_HANDLER( jsa3_io_r )
{
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (has_oki6295)
				result = OKIM6295_status_0_r(offset);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(offset);
			break;

		case 0x004:		/* /RDIO */
			/*
                0x80 = self test (active high)
                0x40 = NMI line state (active high)
                0x20 = sound output full (active high)
                0x10 = self test (active high)
                0x08 = service (active high)
                0x04 = tilt (active high)
                0x02 = coin L (active high)
                0x01 = coin R (active high)
            */
			result = readinputportbytag("JSAIII");
			if (!(readinputport(test_port) & test_mask)) result ^= 0x90;
			if (atarigen_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
		case 0x202:		/* /WRP */
		case 0x204:		/* /WRIO */
		case 0x206:		/* /MIX */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;
	}

	return result;
}


static WRITE8_HANDLER( jsa3_io_w )
{
	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			overall_volume = data * 100 / 127;
			update_all_volumes(Machine);
			break;

		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
			if (has_oki6295)
				OKIM6295_data_0_w(offset, data);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(offset, data);
			break;

		case 0x204:		/* /WRIO */
			/*
                0xc0 = bank address
                0x20 = coin counter 2
                0x10 = coin counter 1
                0x08 = voice frequency (tweaks the OKI6295 frequency)
                0x04 = OKI6295 reset (active low)
                0x02 = OKI6295 bank bit 0
                0x01 = YM2151 reset (active low)
            */

			/* update the OKI bank */

			oki6295_bank_base = (0x40000 * ((data >> 1) & 1)) | (oki6295_bank_base & 0x80000);
			if (has_oki6295) OKIM6295_set_bank_base(0, oki6295_bank_base);

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
			last_ctl = data;

			/* coin counters */
			coin_counter_w(1, (data >> 5) & 1);
			coin_counter_w(0, (data >> 4) & 1);

			/* update the OKI frequency */
			if (has_oki6295) OKIM6295_set_pin7(0, data & 8);
			break;

		case 0x206:		/* /MIX */
			/*
                0xc0 = n/c
                0x20 = low-pass filter enable
                0x10 = OKI6295 bank bit 1
                0x0e = YM2151 volume (0-7)
                0x01 = OKI6295 volume (0-1)
            */

			/* update the OKI bank */
			oki6295_bank_base = (0x80000 * ((data >> 4) & 1)) | (oki6295_bank_base & 0x40000);
			if (has_oki6295) OKIM6295_set_bank_base(0, oki6295_bank_base);

			/* update the volumes */
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			oki6295_volume = 50 + (data & 1) * 50;
			update_all_volumes(Machine);
			break;
	}
}



/*************************************
 *
 *  JSA IIIS I/O handlers
 *
 *************************************/

static READ8_HANDLER( jsa3s_io_r )
{
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (has_oki6295)
			{
				if (offset & 1)
					result = OKIM6295_status_1_r(offset);
				else
					result = OKIM6295_status_0_r(offset);
			}
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(offset);
			break;

		case 0x004:		/* /RDIO */
			/*
                0x80 = self test (active high)
                0x40 = NMI line state (active high)
                0x20 = sound output full (active high)
                0x10 = self test (active high)
                0x08 = service (active high)
                0x04 = tilt (active high)
                0x02 = coin L (active high)
                0x01 = coin R (active high)
            */
			result = readinputportbytag("JSAIII");
			if (!(readinputport(test_port) & test_mask)) result ^= 0x90;
			if (atarigen_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
		case 0x202:		/* /WRP */
		case 0x204:		/* /WRIO */
		case 0x206:		/* /MIX */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;
	}

	return result;
}


static WRITE8_HANDLER( jsa3s_io_w )
{
	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			overall_volume = data * 100 / 127;
			update_all_volumes(Machine);
			break;

		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(0);
			break;

		case 0x200:		/* /WRV */
			if (has_oki6295)
			{
				if (offset & 1)
					OKIM6295_data_1_w(offset, data);
				else
					OKIM6295_data_0_w(offset, data);
			}
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(offset, data);
			break;

		case 0x204:		/* /WRIO */
			/*
                0xc0 = bank address
                0x20 = coin counter 2
                0x10 = coin counter 1
                0x08 = voice frequency (tweaks the OKI6295 frequency)
                0x04 = OKI6295 reset (active low)
                0x02 = left OKI6295 bank bit 0
                0x01 = YM2151 reset (active low)
            */

			/* update the OKI bank */
			oki6295_bank_base = (0x40000 * ((data >> 1) & 1)) | (oki6295_bank_base & 0x80000);
			OKIM6295_set_bank_base(0, oki6295_bank_base);

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
			last_ctl = data;

			/* coin counters */
			coin_counter_w(1, (data >> 5) & 1);
			coin_counter_w(0, (data >> 4) & 1);

			/* update the OKI frequency */
			OKIM6295_set_pin7(0, data & 8);
			OKIM6295_set_pin7(1, data & 8);
			break;

		case 0x206:		/* /MIX */
			/*
                0xc0 = right OKI6295 bank bits 0-1
                0x20 = low-pass filter enable
                0x10 = left OKI6295 bank bit 1
                0x0e = YM2151 volume (0-7)
                0x01 = OKI6295 volume (0-1)
            */

			/* update the OKI bank */
			oki6295_bank_base = (0x80000 * ((data >> 4) & 1)) | (oki6295_bank_base & 0x40000);
			OKIM6295_set_bank_base(0, oki6295_bank_base);
			OKIM6295_set_bank_base(1, 0x40000 * (data >> 6));

			/* update the volumes */
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			oki6295_volume = 50 + (data & 1) * 50;
			update_all_volumes(Machine);
			break;
	}
}



/*************************************
 *
 *  Volume helpers
 *
 *************************************/

static void update_all_volumes(running_machine *machine )
{
	if (has_pokey) atarigen_set_pokey_vol(machine, overall_volume * pokey_volume / 100);
	if (has_ym2151) atarigen_set_ym2151_vol(machine, overall_volume * ym2151_volume / 100);
	if (has_tms5220) atarigen_set_tms5220_vol(machine, overall_volume * tms5220_volume / 100);
	if (has_oki6295) atarigen_set_oki6295_vol(machine, overall_volume * oki6295_volume / 100);
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( atarijsa1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x2000, 0x2001) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x2800, 0x2bff) AM_READWRITE(jsa1_io_r, jsa1_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atarijsa2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x2000, 0x2001) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x2800, 0x2bff) AM_READWRITE(jsa2_io_r, jsa2_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* full map verified from schematics and Batman GALs */
static ADDRESS_MAP_START( atarijsa3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07fe) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07fe) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07fe) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x2800, 0x2fff) AM_READWRITE(jsa3_io_r, jsa3_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atarijsa3s_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07fe) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07fe) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07fe) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x2800, 0x2fff) AM_READWRITE(jsa3s_io_r, jsa3s_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const struct YM2151interface ym2151_interface =
{
	atarigen_ym2151_irq_gen
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

/* Used by Blasteroids */
MACHINE_DRIVER_START( jsa_i_stereo )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("jsa", M6502, JSA_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(atarijsa1_map,0)
	MDRV_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD_TAG("ym", YM2151, JSA_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.60)
	MDRV_SOUND_ROUTE(1, "right", 0.60)
MACHINE_DRIVER_END


/* Used by Xybots */
MACHINE_DRIVER_START( jsa_i_stereo_swapped )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(jsa_i_stereo)

	/* sound hardware */
	MDRV_SOUND_REPLACE("ym", YM2151, JSA_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "right", 0.60)
	MDRV_SOUND_ROUTE(1, "left", 0.60)
MACHINE_DRIVER_END


/* Used by Toobin', Vindicators */
MACHINE_DRIVER_START( jsa_i_stereo_pokey )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(jsa_i_stereo)

	/* sound hardware */
	MDRV_SOUND_ADD(POKEY, JSA_MASTER_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.40)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.40)
MACHINE_DRIVER_END


/* Used by Escape from the Planet of the Robot Monsters */
MACHINE_DRIVER_START( jsa_i_mono_speech )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("jsa", M6502, JSA_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(atarijsa1_map,0)
	MDRV_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD_TAG("ym", YM2151, JSA_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.60)
	MDRV_SOUND_ROUTE(1, "mono", 0.60)

	MDRV_SOUND_ADD(TMS5220, JSA_MASTER_CLOCK*2/11) /* potentially JSA_MASTER_CLOCK/9 as well */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/* Used by Cyberball 2072, STUN Runner, Skull & Crossbones, ThunderJaws, Hydra, Pit Fighter */
MACHINE_DRIVER_START( jsa_ii_mono )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("jsa", M6502, JSA_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(atarijsa2_map,0)
	MDRV_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD_TAG("ym", YM2151, JSA_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.60)
	MDRV_SOUND_ROUTE(1, "mono", 0.60)

	MDRV_SOUND_ADD_TAG("adpcm", OKIM6295, JSA_MASTER_CLOCK/3)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END


/* Used by Batman, Guardians of the 'Hood, Road Riot 4WD */
MACHINE_DRIVER_START( jsa_iii_mono )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(jsa_ii_mono)
	MDRV_CPU_MODIFY("jsa")
	MDRV_CPU_PROGRAM_MAP(atarijsa3_map,0)
MACHINE_DRIVER_END


/* Used by Off the Wall */
MACHINE_DRIVER_START( jsa_iii_mono_noadpcm )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(jsa_iii_mono)

	/* sound hardware */
	MDRV_SOUND_REMOVE("adpcm")
MACHINE_DRIVER_END


/* Used by Space Lords, Moto Frenzy, Steel Talons, Road Riot's Revenge Rally */
MACHINE_DRIVER_START( jsa_iiis_stereo )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("jsa", M6502, JSA_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(atarijsa3s_map,0)
	MDRV_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD_TAG("ym", YM2151, JSA_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.60)
	MDRV_SOUND_ROUTE(1, "right", 0.60)

	MDRV_SOUND_ADD(OKIM6295, JSA_MASTER_CLOCK/3)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.75)

	MDRV_SOUND_ADD(OKIM6295, JSA_MASTER_CLOCK/3)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.75)
MACHINE_DRIVER_END
