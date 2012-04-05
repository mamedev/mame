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

TODO:
JSA-i: stereo gating for POKEY/TMS5220C is currently only mono, only looking at ym2151_ct1;
  the two commented-out lines would be correct if stereo volume-set functions were written.
ALL: the LPF (low pass filter) bit which selectively places a lowpass filter in the output
  path for all channels is currently unimplemented; someone who knows analog magic will need
  to handle this.

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/tms5220.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"


#define JSA_MASTER_CLOCK			XTAL_3_579545MHz


static UINT8 *bank_base;
static UINT8 *bank_source_data;

static cpu_device *jsacpu;
static const char *test_port;
static UINT16 test_mask;

static pokey_device *pokey;
static ym2151_device *ym2151;
static device_t *tms5220;
static okim6295_device *oki6295;
static okim6295_device *oki6295_l, *oki6295_r;

static UINT8 overall_volume;
static UINT8 pokey_volume;
static UINT8 ym2151_volume;
static UINT8 tms5220_volume;
static UINT8 oki6295_volume;
static UINT8 ym2151_ct1;
static UINT8 ym2151_ct2;

static void update_all_volumes(running_machine &machine);

static READ8_HANDLER( jsa1_io_r );
static WRITE8_HANDLER( jsa1_io_w );
static READ8_HANDLER( jsa2_io_r );
static WRITE8_HANDLER( jsa2_io_w );
static READ8_HANDLER( jsa3_io_r );
static WRITE8_HANDLER( jsa3_io_w );


/*************************************
 *
 *  OKI banked address map
 *
 *************************************/

static ADDRESS_MAP_START( jsa3_oki_map, AS_0, 8, driver_device )
	AM_RANGE(0x00000, 0x1ffff) AM_ROMBANK("bank12")
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank13")
ADDRESS_MAP_END

static ADDRESS_MAP_START( jsa3_oki2_map, AS_0, 8, driver_device )
	AM_RANGE(0x00000, 0x1ffff) AM_ROMBANK("bank14")
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank15")
ADDRESS_MAP_END



/*************************************
 *
 *  State save
 *
 *************************************/

static void init_save_state(running_machine &machine)
{
	state_save_register_global(machine, overall_volume);
	state_save_register_global(machine, pokey_volume);
	state_save_register_global(machine, ym2151_volume);
	state_save_register_global(machine, tms5220_volume);
	state_save_register_global(machine, oki6295_volume);
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

void atarijsa_init(running_machine &machine, const char *testport, int testmask)
{
	UINT8 *rgn;

	/* copy in the parameters */
	jsacpu = machine.device<cpu_device>("jsa");
	assert_always(jsacpu != NULL, "Could not find JSA CPU!");
	test_port = testport;
	test_mask = testmask;

	/* predetermine the bank base */
	rgn = machine.region("jsa")->base();
	bank_base = &rgn[0x03000];
	bank_source_data = &rgn[0x10000];

	/* determine which sound hardware is installed */
	tms5220 = machine.device("tms");
	ym2151 = machine.device<ym2151_device>("ymsnd");
	pokey = machine.device<pokey_device>("pokey");
	oki6295 = machine.device<okim6295_device>("adpcm");
	oki6295_l = machine.device<okim6295_device>("adpcml");
	oki6295_r = machine.device<okim6295_device>("adpcmr");

	/* install POKEY memory handlers */
	if (pokey != NULL)
		jsacpu->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(*pokey, 0x2c00, 0x2c0f, FUNC(pokey_r), FUNC(pokey_w));

	init_save_state(machine);
	atarijsa_reset();

	/* initialize JSA III ADPCM */
	{
		static const char *const regions[] = { "adpcm", "adpcml", "adpcmr" };
		int rgn;

		/* expand the ADPCM data to avoid lots of memcpy's during gameplay */
		/* the upper 128k is fixed, the lower 128k is bankswitched */
		for (rgn = 0; rgn < ARRAY_LENGTH(regions); rgn++)
		{
			UINT8 *base = machine.region(regions[rgn])->base();
			if (base != NULL && machine.region(regions[rgn])->bytes() >= 0x80000)
			{
				const char *bank = (rgn != 2) ? "bank12" : "bank14";
				const char *bank_plus_1 = (rgn != 2) ? "bank13" : "bank15";
				memory_configure_bank(machine, bank, 0, 2, base + 0x00000, 0x00000);
				memory_configure_bank(machine, bank, 2, 2, base + 0x20000, 0x20000);
				memory_set_bankptr(machine, bank_plus_1, base + 0x60000);
			}
		}
	}
}


void atarijsa_reset(void)
{
	/* reset the sound I/O system */
	atarigen_sound_io_reset(jsacpu);

	/* reset the static states */
	overall_volume = 100;
	pokey_volume = 100;
	ym2151_volume = 100;
	tms5220_volume = 100;
	oki6295_volume = 100;
	ym2151_ct1 = 0;
	ym2151_ct2 = 0;

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
	atarigen_state *atarigen = space->machine().driver_data<atarigen_state>();
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* n/c */
			logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(space, offset);
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
			result = input_port_read(space->machine(), "JSAI");
			if (!(input_port_read(space->machine(), test_port) & test_mask)) result ^= 0x80;
			if (atarigen->m_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen->m_sound_to_cpu_ready) result ^= 0x20;
			if ((tms5220 != NULL) && (tms5220_readyq_r(tms5220) == 0))
				result |= 0x10;
			else
				result &= ~0x10;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
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
			atarigen_6502_irq_ack_r(space, 0);
			break;

		case 0x200:		/* /VOICE */
			if (tms5220 != NULL)
				tms5220_data_w(tms5220, 0, data);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(space, offset, data);
			break;

		case 0x204:		/* WRIO */
			/*
                0xc0 = bank address
                0x20 = coin counter 2
                0x10 = coin counter 1
                0x08 = squeak (tweaks the 5220 frequency)
                0x04 = TMS5220 reset (actually the read strobe) (active low)
                0x02 = TMS5220 write strobe (active low)
                0x01 = YM2151 reset (active low)
            */

			/* handle TMS5220 I/O */
			if (tms5220 != NULL)
			{
				int count;
				tms5220_wsq_w(tms5220, (data&0x02)>>1);
				tms5220_rsq_w(tms5220, (data&0x04)>>2);
				count = 5 | ((data >> 2) & 2);
				tms5220_set_frequency(tms5220, JSA_MASTER_CLOCK*2 / (16 - count));
			}

			/* reset the YM2151 if needed */
			if ((data&1) == 0) devtag_reset(space->machine(), "ymsnd");

			/* coin counters */
			coin_counter_w(space->machine(), 1, (data >> 5) & 1);
			coin_counter_w(space->machine(), 0, (data >> 4) & 1);

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);
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
			update_all_volumes(space->machine());
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
	atarigen_state *atarigen = space->machine().driver_data<atarigen_state>();
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (oki6295 != NULL)
				result = oki6295->read(*space, offset);
			else
				logerror("atarijsa: Unknown read at %04X\n", offset & 0x206);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(space, offset);
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
			result = input_port_read(space->machine(), "JSAII");
			if (!(input_port_read(space->machine(), test_port) & test_mask)) result ^= 0x80;
			if (atarigen->m_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen->m_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
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
			atarigen_6502_irq_ack_r(space, 0);
			break;

		case 0x200:		/* /WRV */
			if (oki6295 != NULL)
				oki6295->write(*space, offset, data);
			else
				logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(space, offset, data);
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

			/* reset the YM2151 if needed */
			if ((data&1) == 0) devtag_reset(space->machine(), "ymsnd");

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);

			/* coin counters */
			coin_counter_w(space->machine(), 1, (data >> 5) & 1);
			coin_counter_w(space->machine(), 0, (data >> 4) & 1);

			/* update the OKI frequency */
			if (oki6295 != NULL)
				oki6295->set_pin7(data & 8);
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
			update_all_volumes(space->machine());
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
	atarigen_state *atarigen = space->machine().driver_data<atarigen_state>();
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (oki6295 != NULL)
				result = oki6295->read(*space, offset);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(space, offset);
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
			result = input_port_read(space->machine(), "JSAIII");
			if (!(input_port_read(space->machine(), test_port) & test_mask)) result ^= 0x90;
			if (atarigen->m_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen->m_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
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
			update_all_volumes(space->machine());
			break;

		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
			break;

		case 0x200:		/* /WRV */
			if (oki6295 != NULL)
				oki6295->write(*space, offset, data);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(space, offset, data);
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

			/* reset the YM2151 if needed */
			if ((data&1) == 0) devtag_reset(space->machine(), "ymsnd");

			/* update the OKI bank */
			if (oki6295 != NULL)
				memory_set_bank(space->machine(), "bank12", (space->machine().memory().bank("bank12") & 2) | ((data >> 1) & 1));

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);

			/* coin counters */
			coin_counter_w(space->machine(), 1, (data >> 5) & 1);
			coin_counter_w(space->machine(), 0, (data >> 4) & 1);

			/* update the OKI frequency */
			if (oki6295 != NULL) oki6295->set_pin7(data & 8);
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
			if (oki6295 != NULL)
				memory_set_bank(space->machine(), "bank12", (space->machine().memory().bank("bank12") & 1) | ((data >> 3) & 2));

			/* update the volumes */
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			oki6295_volume = 50 + (data & 1) * 50;
			update_all_volumes(space->machine());
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
	atarigen_state *atarigen = space->machine().driver_data<atarigen_state>();
	int result = 0xff;

	switch (offset & 0x206)
	{
		case 0x000:		/* /RDV */
			if (oki6295_l != NULL)
				result = ((offset & 1) ? oki6295_r : oki6295_l)->read(*space, offset);
			break;

		case 0x002:		/* /RDP */
			result = atarigen_6502_sound_r(space, offset);
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
			result = input_port_read(space->machine(), "JSAIII");
			if (!(input_port_read(space->machine(), test_port) & test_mask)) result ^= 0x90;
			if (atarigen->m_cpu_to_sound_ready) result ^= 0x40;
			if (atarigen->m_sound_to_cpu_ready) result ^= 0x20;
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
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
			update_all_volumes(space->machine());
			break;

		case 0x002:		/* /RDP */
		case 0x004:		/* /RDIO */
			logerror("atarijsa: Unknown write (%02X) at %04X\n", data & 0xff, offset & 0x206);
			break;

		case 0x006:		/* /IRQACK */
			atarigen_6502_irq_ack_r(space, 0);
			break;

		case 0x200:		/* /WRV */
			if (oki6295_l != NULL)
				((offset & 1) ? oki6295_r : oki6295_l)->write(*space, 0, data);
			break;

		case 0x202:		/* /WRP */
			atarigen_6502_sound_w(space, offset, data);
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

			/* reset the YM2151 if needed */
			if ((data&1) == 0) devtag_reset(space->machine(), "ymsnd");

			/* update the OKI bank */
			memory_set_bank(space->machine(), "bank12", (space->machine().memory().bank("bank12") & 2) | ((data >> 1) & 1));

			/* update the bank */
			memcpy(bank_base, &bank_source_data[0x1000 * ((data >> 6) & 3)], 0x1000);

			/* coin counters */
			coin_counter_w(space->machine(), 1, (data >> 5) & 1);
			coin_counter_w(space->machine(), 0, (data >> 4) & 1);

			/* update the OKI frequency */
			oki6295_l->set_pin7(data & 8);
			oki6295_r->set_pin7(data & 8);
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
			memory_set_bank(space->machine(), "bank12", (space->machine().memory().bank("bank12") & 1) | ((data >> 3) & 2));
			memory_set_bank(space->machine(), "bank14", data >> 6);

			/* update the volumes */
			ym2151_volume = ((data >> 1) & 7) * 100 / 7;
			oki6295_volume = 50 + (data & 1) * 50;
			update_all_volumes(space->machine());
			break;
	}
}

static WRITE8_DEVICE_HANDLER( ym2151_ctl_w )
{
	ym2151_ct1 = data&0x1;
	ym2151_ct2 = (data&0x2)>>1;
	update_all_volumes(device->machine());
}


/*************************************
 *
 *  Volume helpers
 *
 *************************************/

static void update_all_volumes(running_machine &machine )
{
	if (pokey != NULL) atarigen_set_pokey_vol(machine, (overall_volume * pokey_volume / 100) * ym2151_ct1);
	//if (pokey != NULL) atarigen_set_pokey_stereo_vol(machine, (overall_volume * pokey_volume / 100) * ym2151_ct1, (overall_volume * pokey_volume / 100) * ym2151_ct2);
	if (ym2151 != NULL) atarigen_set_ym2151_vol(machine, overall_volume * ym2151_volume / 100);
	if (tms5220 != NULL) atarigen_set_tms5220_vol(machine, (overall_volume * tms5220_volume / 100) * ym2151_ct1);
	//if (tms5220 != NULL) atarigen_set_tms5220_stereo_vol(machine, (overall_volume * tms5220_volume / 100) * ym2151_ct1, (overall_volume * tms5220_volume / 100) * ym2151_ct2);
	if (oki6295 != NULL || oki6295_l != NULL || oki6295_r != NULL) atarigen_set_oki6295_vol(machine, overall_volume * oki6295_volume / 100);
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( atarijsa1_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2bff) AM_READWRITE_LEGACY(jsa1_io_r, jsa1_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atarijsa2_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2bff) AM_READWRITE_LEGACY(jsa2_io_r, jsa2_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* full map verified from schematics and Batman GALs */
static ADDRESS_MAP_START( atarijsa3_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07fe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2fff) AM_READWRITE_LEGACY(jsa3_io_r, jsa3_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atarijsa3s_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07fe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2fff) AM_READWRITE_LEGACY(jsa3s_io_r, jsa3s_io_w)
	AM_RANGE(0x3000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const ym2151_interface ym2151_config =
{
	atarigen_ym2151_irq_gen,
	ym2151_ctl_w
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

/* Used by Blasteroids */
MACHINE_CONFIG_FRAGMENT( jsa_i_stereo )

	/* basic machine hardware */
	MCFG_CPU_ADD("jsa", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa1_map)
	MCFG_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, JSA_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)
MACHINE_CONFIG_END


/* Used by Xybots */
MACHINE_CONFIG_DERIVED( jsa_i_stereo_swapped, jsa_i_stereo )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_SOUND_REPLACE("ymsnd", YM2151, JSA_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.60)
MACHINE_CONFIG_END


/* Used by Toobin', Vindicators */
MACHINE_CONFIG_DERIVED( jsa_i_stereo_pokey, jsa_i_stereo )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_SOUND_ADD("pokey", POKEY, JSA_MASTER_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END


/* Used by Escape from the Planet of the Robot Monsters */
MACHINE_CONFIG_FRAGMENT( jsa_i_mono_speech )

	/* basic machine hardware */
	MCFG_CPU_ADD("jsa", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa1_map)
	MCFG_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, JSA_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)

	MCFG_SOUND_ADD("tms", TMS5220C, JSA_MASTER_CLOCK*2/11) /* potentially JSA_MASTER_CLOCK/9 as well */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/* Used by Cyberball 2072, STUN Runner, Skull & Crossbones, ThunderJaws, Hydra, Pit Fighter */
MACHINE_CONFIG_FRAGMENT( jsa_ii_mono )

	/* basic machine hardware */
	MCFG_CPU_ADD("jsa", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa2_map)
	MCFG_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, JSA_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)

	MCFG_OKIM6295_ADD("adpcm", JSA_MASTER_CLOCK/3, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


/* Used by Batman, Guardians of the 'Hood, Road Riot 4WD, Steel Talons */
MACHINE_CONFIG_DERIVED( jsa_iii_mono, jsa_ii_mono )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("jsa")
	MCFG_CPU_PROGRAM_MAP(atarijsa3_map)

	MCFG_DEVICE_MODIFY("adpcm")
	MCFG_DEVICE_ADDRESS_MAP(AS_0, jsa3_oki_map)
MACHINE_CONFIG_END


/* Used by Off the Wall */
MACHINE_CONFIG_DERIVED( jsa_iii_mono_noadpcm, jsa_iii_mono )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_DEVICE_REMOVE("adpcm")
MACHINE_CONFIG_END


/* Used by Space Lords, Moto Frenzy, Road Riot's Revenge Rally */
MACHINE_CONFIG_FRAGMENT( jsa_iiis_stereo )

	/* basic machine hardware */
	MCFG_CPU_ADD("jsa", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa3s_map)
	MCFG_CPU_PERIODIC_INT(atarigen_6502_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, JSA_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_OKIM6295_ADD("adpcml", JSA_MASTER_CLOCK/3, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, jsa3_oki_map)

	MCFG_OKIM6295_ADD("adpcmr", JSA_MASTER_CLOCK/3, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, jsa3_oki2_map)
MACHINE_CONFIG_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

INPUT_PORTS_START( atarijsa_i )
	PORT_START("JSAI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* speech chip ready */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */
INPUT_PORTS_END

INPUT_PORTS_START( atarijsa_ii )
	PORT_START("JSAII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */
INPUT_PORTS_END

INPUT_PORTS_START( atarijsa_iii )
	PORT_START("JSAIII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )	/* input buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */
INPUT_PORTS_END
