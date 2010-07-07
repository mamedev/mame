/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/decocass.h"

#define LOG_CASSETTE_STATE		0

/* dongle type #1: jumpers C and D assignments */
#define MAKE_MAP(m0,m1,m2,m3,m4,m5,m6,m7)	\
	((UINT32)(m0)) | \
	((UINT32)(m1) << 3) | \
	((UINT32)(m2) << 6) | \
	((UINT32)(m3) << 9) | \
	((UINT32)(m4) << 12) | \
	((UINT32)(m5) << 15) | \
	((UINT32)(m6) << 18) | \
	((UINT32)(m7) << 21)

#define MAP0(m) ((m)&7)
#define MAP1(m) (((m)>>3)&7)
#define MAP2(m) (((m)>>6)&7)
#define MAP3(m) (((m)>>9)&7)
#define MAP4(m) (((m)>>12)&7)
#define MAP5(m) (((m)>>15)&7)
#define MAP6(m) (((m)>>18)&7)
#define MAP7(m) (((m)>>21)&7)


enum {
	TYPE3_SWAP_01,
	TYPE3_SWAP_12,
	TYPE3_SWAP_13,
	TYPE3_SWAP_24,
	TYPE3_SWAP_25,
	TYPE3_SWAP_34_0,
	TYPE3_SWAP_34_7,
	TYPE3_SWAP_23_56,
	TYPE3_SWAP_56,
	TYPE3_SWAP_67
};



static UINT8 tape_get_status_bits(running_device *device);
static UINT8 tape_is_present(running_device *device);
static void tape_change_speed(running_device *device, INT8 newspeed);


WRITE8_HANDLER( decocass_coin_counter_w )
{
}

WRITE8_HANDLER( decocass_sound_command_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	LOG(2,("CPU %s sound command -> $%02x\n", space->cpu->tag(), data));
	soundlatch_w(space, 0, data);
	state->sound_ack |= 0x80;
	/* remove snd cpu data ack bit. i don't see it in the schems, but... */
	state->sound_ack &= ~0x40;
	cpu_set_input_line(state->audiocpu, M6502_IRQ_LINE, ASSERT_LINE);
}

READ8_HANDLER( decocass_sound_data_r )
{
	UINT8 data = soundlatch2_r(space, 0);
	LOG(2,("CPU %s sound data    <- $%02x\n", space->cpu->tag(), data));
	return data;
}

READ8_HANDLER( decocass_sound_ack_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data = state->sound_ack;	/* D6+D7 */
	LOG(4,("CPU %s sound ack     <- $%02x\n", space->cpu->tag(), data));
	return data;
}

WRITE8_HANDLER( decocass_sound_data_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	LOG(2,("CPU %s sound data    -> $%02x\n", space->cpu->tag(), data));
	soundlatch2_w(space, 0, data);
	state->sound_ack |= 0x40;
}

READ8_HANDLER( decocass_sound_command_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data = soundlatch_r(space, 0);
	LOG(4,("CPU %s sound command <- $%02x\n", space->cpu->tag(), data));
	cpu_set_input_line(state->audiocpu, M6502_IRQ_LINE, CLEAR_LINE);
	state->sound_ack &= ~0x80;
	return data;
}

TIMER_DEVICE_CALLBACK( decocass_audio_nmi_gen )
{
	decocass_state *state = (decocass_state *)timer.machine->driver_data;
	int scanline = param;
	state->audio_nmi_state = scanline & 8;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, (state->audio_nmi_enabled && state->audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_HANDLER( decocass_sound_nmi_enable_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	state->audio_nmi_enabled = 1;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, (state->audio_nmi_enabled && state->audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_HANDLER( decocass_sound_nmi_enable_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	state->audio_nmi_enabled = 1;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, (state->audio_nmi_enabled && state->audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	return 0xff;
}

READ8_HANDLER( decocass_sound_data_ack_reset_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data = 0xff;
	LOG(2,("CPU %s sound ack rst <- $%02x\n", space->cpu->tag(), data));
	state->sound_ack &= ~0x40;
	return data;
}

WRITE8_HANDLER( decocass_sound_data_ack_reset_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	LOG(2,("CPU %s sound ack rst -> $%02x\n", space->cpu->tag(), data));
	state->sound_ack &= ~0x40;
}

WRITE8_HANDLER( decocass_nmi_reset_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, CLEAR_LINE );
}

WRITE8_HANDLER( decocass_quadrature_decoder_reset_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;

	/* just latch the analog controls here */
	state->quadrature_decoder[0] = input_port_read(space->machine, "AN0");
	state->quadrature_decoder[1] = input_port_read(space->machine, "AN1");
	state->quadrature_decoder[2] = input_port_read(space->machine, "AN2");
	state->quadrature_decoder[3] = input_port_read(space->machine, "AN3");
}

WRITE8_HANDLER( decocass_adc_w )
{
}

/*
 * E6x0    inputs
 * E6x1    inputs
 * E6x2    coin inp
 * E6x3    quadrature decoder read
 * E6x4    ""
 * E6x5    ""
 * E6x6    ""
 * E6x7    a/d converter read
 */
READ8_HANDLER( decocass_input_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data = 0xff;
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };

	switch (offset & 7)
	{
	case 0: case 1: case 2:
		data = input_port_read(space->machine, portnames[offset & 7]);
		break;
	case 3: case 4: case 5: case 6:
		data = state->quadrature_decoder[(offset & 7) - 3];
		break;
	default:
		break;
	}

	return data;
}

/*
 * D0 - REQ/ data request     (8041 pin 34 port 1.7)
 * D1 - FNO/ function number  (8041 pin 21 port 2.0)
 * D2 - EOT/ end-of-tape      (8041 pin 22 port 2.1)
 * D3 - ERR/ error condition  (8041 pin 23 port 2.2)
 * D4 - BOT-EOT from tape
 * D5 -
 * D6 -
 * D7 - cassette present
 */

#define E5XX_MASK	0x02	/* use 0x0e for old style board */


WRITE8_HANDLER( decocass_reset_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	LOG(1,("%10s 6502-PC: %04x decocass_reset_w(%02x): $%02x\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
	state->decocass_reset = data;

	/* CPU #1 active high reset */
	cpu_set_input_line(state->audiocpu, INPUT_LINE_RESET, data & 0x01);

	/* on reset also disable audio NMI */
	if (data & 1)
	{
		state->audio_nmi_enabled = 0;
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, (state->audio_nmi_enabled && state->audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	}

	/* 8041 active low reset */
	cpu_set_input_line(state->mcu, INPUT_LINE_RESET, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
}


#ifdef MAME_DEBUG
static void decocass_fno( running_machine *machine, offs_t offset, UINT8 data )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	/* 8041ENA/ and is this a FNO write (function number)? */
	if (0 == (state->i8041_p2 & 0x01))
	{
		switch (data)
		{
		case 0x25: logerror("8041 FNO 25: write_block\n"); break;
		case 0x26: logerror("8041 FNO 26: rewind_block\n"); break;
		case 0x27: logerror("8041 FNO 27: read_block_a\n"); break;
		case 0x28: logerror("8041 FNO 28: read_block_b\n"); break;
		case 0x29: logerror("8041 FNO 29: tape_rewind_fast\n"); break;
		case 0x2a: logerror("8041 FNO 2a: tape_forward\n"); break;
		case 0x2b: logerror("8041 FNO 2b: tape_rewind\n"); break;
		case 0x2c: logerror("8041 FNO 2c: force_abort\n"); break;
		case 0x2d: logerror("8041 FNO 2d: tape_erase\n"); break;
		case 0x2e: logerror("8041 FNO 2e: search_tape_mark\n"); break;
		case 0x2f: logerror("8041 FNO 2f: search_eot\n"); break;
		case 0x30: logerror("8041 FNO 30: advance_block\n"); break;
		case 0x31: logerror("8041 FNO 31: write_tape_mark\n"); break;
		case 0x32: logerror("8041 FNO 32: reset_error\n"); break;
		case 0x33: logerror("8041 FNO 33: flag_status_report\n"); break;
		case 0x34: logerror("8041 FNO 34: report_status_to_main\n"); break;
		default:   logerror("8041 FNO %02x: invalid\n", data);
		}
	}
}
#endif

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Lock 'n Chase
 *  - Treasure Island
 *  - Super Astro Fighter
 *  - Lucky Poker
 *  - Terranian
 *  - Explorer
 *  - Pro Golf
 *
 * Latch bits 2 and 6, pass bit 3, invert bit 2.
 * Lookup PROM DE-0061 using bits 0, 1, 4, 5, and 7 as the
 * address bits; take PROM data 0-4 as data bits 0, 1, 4, 5, and 7.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type1_latch_26_pass_3_inv_2_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_latch_26_pass_3_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(space->machine, "dongle");

		if (state->firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			state->firsttime = 0;
			state->latch1 = 0;	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 0);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(state->type1_inmap)) & 1) << 0) |
			(((data >> MAP1(state->type1_inmap)) & 1) << 1) |
			(((data >> MAP4(state->type1_inmap)) & 1) << 2) |
			(((data >> MAP5(state->type1_inmap)) & 1) << 3) |
			(((data >> MAP7(state->type1_inmap)) & 1) << 4);
		/* latch bits 2 and 6, pass bit 3, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(state->type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(state->type1_outmap)) |
			((1 - ((state->latch1 >> MAP2(state->type1_inmap)) & 1)) << MAP2(state->type1_outmap)) |
			(((data >> MAP3(state->type1_inmap)) & 1)		   << MAP3(state->type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(state->type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(state->type1_outmap)) |
			(((state->latch1 >> MAP6(state->type1_inmap)) & 1)	   << MAP6(state->type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(state->type1_outmap));

		LOG(3,("%10s 6502-PC: %04x decocass_type1_latch_26_pass_3_inv_2_r(%02x): $%02x\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));

		state->latch1 = save;		/* latch the data for the next A0 == 0 read */
	}
	return data;
}


/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Test Tape
 *
 * Pass bits 1, 3, and 6. Lookup PROM DE-0061 using bits 0, 2, 4, 5, and 7
 * as the address bits; take PROM data 0-4 as data bits 0, 2, 4, 5, and 7.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type1_pass_136_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_pass_136_r(%02x): $%02x <- (%s %s)\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(space->machine, "dongle");

		if (state->firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			state->firsttime = 0;
			state->latch1 = 0;	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 0);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(state->type1_inmap)) & 1) << 0) |
			(((data >> MAP2(state->type1_inmap)) & 1) << 1) |
			(((data >> MAP4(state->type1_inmap)) & 1) << 2) |
			(((data >> MAP5(state->type1_inmap)) & 1) << 3) |
			(((data >> MAP7(state->type1_inmap)) & 1) << 4);
		/* latch bits 1 and 6, pass bit 3, invert bit 1 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(state->type1_outmap)) |
			(((data >> MAP1(state->type1_inmap)) & 1)         << MAP1(state->type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP2(state->type1_outmap)) |
			(((data >> MAP3(state->type1_inmap)) & 1)		   << MAP3(state->type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(state->type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(state->type1_outmap)) |
			(((data >> MAP6(state->type1_inmap)) & 1)	       << MAP6(state->type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(state->type1_outmap));

		LOG(3,("%10s 6502-PC: %04x decocass_type1_pass_136_r(%02x): $%02x\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));

		state->latch1 = save;		/* latch the data for the next A0 == 0 read */
	}
	return data;
}

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Highway Chase
 *
 * Latch bits 2 and 7, pass bit 3, invert bit 2 to the output.
 * Lookup PROM (Highway Chase) using data bits 0, 1, 4, 5, and 6 as the
 * address bits. Take PROM data 0-4 as data bits 0, 1, 4, 5, and 6.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type1_latch_27_pass_3_inv_2_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_latch_27_pass_3_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(space->machine, "dongle");

		if (state->firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			state->firsttime = 0;
			state->latch1 = 0;	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 0);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(state->type1_inmap)) & 1) << 0) |
			(((data >> MAP1(state->type1_inmap)) & 1) << 1) |
			(((data >> MAP4(state->type1_inmap)) & 1) << 2) |
			(((data >> MAP5(state->type1_inmap)) & 1) << 3) |
			(((data >> MAP6(state->type1_inmap)) & 1) << 4);
		/* latch bits 2 and 7, pass bit 3, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(state->type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(state->type1_outmap)) |
			((1 - ((state->latch1 >> MAP2(state->type1_inmap)) & 1)) << MAP2(state->type1_outmap)) |
			(((data >> MAP3(state->type1_inmap)) & 1)		   << MAP3(state->type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(state->type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(state->type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP6(state->type1_outmap)) |
			(((state->latch1 >> MAP7(state->type1_inmap)) & 1)	   << MAP7(state->type1_outmap));

		LOG(3,("%10s 6502-PC: %04x decocass_type1_latch_27_pass_3_inv_2_r(%02x): $%02x\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));

		state->latch1 = save;		/* latch the data for the next A0 == 0 read */
	}
	return data;
}

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Explorer
 *
 * Latch bits 2 and 6, pass bit 5, invert bit 2 to the output.
 * Lookup PROM (Explorer) using bits 0, 1, 3, 4, and 7 as the
 * address bits. Take PROM data 0-4 as data bits 0, 1, 3, 4, and 7.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type1_latch_26_pass_5_inv_2_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_latch_26_pass_5_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(space->machine, "dongle");

		if (state->firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			state->firsttime = 0;
			state->latch1 = 0;	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 0);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(state->type1_inmap)) & 1) << 0) |
			(((data >> MAP1(state->type1_inmap)) & 1) << 1) |
			(((data >> MAP3(state->type1_inmap)) & 1) << 2) |
			(((data >> MAP4(state->type1_inmap)) & 1) << 3) |
			(((data >> MAP7(state->type1_inmap)) & 1) << 4);
		/* latch bits 2 and 6, pass bit 5, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(state->type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(state->type1_outmap)) |
			((1 - ((state->latch1 >> MAP2(state->type1_inmap)) & 1)) << MAP2(state->type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP3(state->type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP4(state->type1_outmap)) |
			(((data >> MAP5(state->type1_inmap)) & 1)		   << MAP5(state->type1_outmap)) |
			(((state->latch1 >> MAP6(state->type1_inmap)) & 1)		   << MAP6(state->type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(state->type1_outmap));

		LOG(3,("%10s 6502-PC: %04x decocass_type1_latch_26_pass_5_inv_2_r(%02x): $%02x\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));

		state->latch1 = save;		/* latch the data for the next A0 == 0 read */
	}
	return data;
}



/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Astro Fantazia
 *
 * Latch bits 1 and 6, pass bit 3, invert bit 1.
 * Lookup PROM DE-0061 using bits 0, 2, 4, 5, and 7 as the
 * address bits; take PROM data 0-4 as data bits 0, 2, 4, 5, and 7.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type1_latch_16_pass_3_inv_1_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_latch_16_pass_3_inv_1_r(%02x): $%02x <- (%s %s)\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(space->machine, "dongle");

		if (state->firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			state->firsttime = 0;
			state->latch1 = 0;	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, 0);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(state->type1_inmap)) & 1) << 0) |
			(((data >> MAP2(state->type1_inmap)) & 1) << 1) |
			(((data >> MAP4(state->type1_inmap)) & 1) << 2) |
			(((data >> MAP5(state->type1_inmap)) & 1) << 3) |
			(((data >> MAP7(state->type1_inmap)) & 1) << 4);
		/* latch bits 1 and 6, pass bit 3, invert bit 1 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(state->type1_outmap)) |
			((1 - ((state->latch1 >> MAP1(state->type1_inmap)) & 1)) << MAP1(state->type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP2(state->type1_outmap)) |
			(((data >> MAP3(state->type1_inmap)) & 1)		   << MAP3(state->type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(state->type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(state->type1_outmap)) |
			(((state->latch1 >> MAP6(state->type1_inmap)) & 1)	   << MAP6(state->type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(state->type1_outmap));

		LOG(3,("%10s 6502-PC: %04x decocass_type1_latch_16_pass_3_inv_1_r(%02x): $%02x\n",
			attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));

		state->latch1 = save;		/* latch the data for the next A0 == 0 read */
	}
	return data;
}




/***************************************************************************
 *
 *  TYPE2 DONGLE (CS82-007)
 *  - Mission X
 *  - Disco No 1
 *  - Pro Tennis
 *  - Tornado
 *
 ***************************************************************************/
static READ8_HANDLER( decocass_type2_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == state->type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			UINT8 *prom = memory_region(space->machine, "dongle");
			data = prom[256 * state->type2_d2_latch + state->type2_promaddr];
			LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- prom[%03x]\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, 256 * state->type2_d2_latch + state->type2_promaddr));
		}
		else
		{
			data = 0xff;	/* floating input? */
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
			data = upi41_master_r(state->mcu, offset);
		else
			data = offset & 0xff;

		LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- 8041-%s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "STATUS" : "DATA"));
	}
	return data;
}

static WRITE8_HANDLER( decocass_type2_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (1 == state->type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			LOG(4,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM+D2 latch", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
		else
		{
			state->type2_promaddr = data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM addr $%02x\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, state->type2_promaddr));
			return;
		}
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s ", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "8041-CMND" : "8041 DATA"));
	}
	if (1 == (offset & 1))
	{
		if (0xc0 == (data & 0xf0))
		{
			state->type2_xx_latch = 1;
			state->type2_d2_latch = (data & 0x04) ? 1 : 0;
			LOG(3,("PROM:%s D2:%d", state->type2_xx_latch ? "on" : "off", state->type2_d2_latch));
		}
	}
	upi41_master_w(state->mcu, offset & 1, data);

#ifdef MAME_DEBUG
	decocass_fno(space->machine, offset, data);
#endif
}

/***************************************************************************
 *
 *  TYPE3 DONGLE
 *  - Bump 'n Jump
 *  - Burnin' Rubber
 *  - Burger Time
 *  - Graplop
 *  - Cluster Buster
 *  - LaPaPa
 *  - Fighting Ice Hockey
 *  - Pro Bowling
 *  - Night Star
 *  - Pro Soccer
 *  - Peter Pepper's Ice Cream Factory
 *
 ***************************************************************************/
static READ8_HANDLER( decocass_type3_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data, save;

	if (1 == (offset & 1))
	{
		if (1 == state->type3_pal_19)
		{
			UINT8 *prom = memory_region(space->machine, "dongle");
			data = prom[state->type3_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- prom[$%03x]\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, state->type3_ctrs));
			if (++state->type3_ctrs == 4096)
				state->type3_ctrs = 0;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = upi41_master_r(state->mcu, 1);
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- 8041 STATUS\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
			}
		}
	}
	else
	{
		if (1 == state->type3_pal_19)
		{
			save = data = 0xff;    /* open data bus? */
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				save = upi41_master_r(state->mcu, 0);
				switch (state->type3_swap)
				{
				case TYPE3_SWAP_01:
					data =
						(BIT(save, 1) << 0) |
						(state->type3_d0_latch << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_12:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 2) << 1) |
						(BIT(save, 1) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_13:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 3) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 1) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_24:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 1) << 1) |
						(BIT(save, 4) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 2) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_25:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 1) << 1) |
						(BIT(save, 5) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 2) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_34_0:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 4) |
						(BIT(save, 4) << 3) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_34_7:
					data =
						(BIT(save, 7) << 0) |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 4) << 3) |
						(BIT(save, 3) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(state->type3_d0_latch << 7);
					break;
				case TYPE3_SWAP_23_56:
					data =
						(state->type3_d0_latch << 0) |
						(BIT(save, 1) << 1) |
						(BIT(save, 3) << 2) |
						(BIT(save, 2) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 6) << 5) |
						(BIT(save, 5) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_56:
					data =
						state->type3_d0_latch |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 6) << 5) |
						(BIT(save, 5) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_67:
					data =
						state->type3_d0_latch |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 7) << 6) |
						(BIT(save, 6) << 7);
					break;
				default:
					data =
						state->type3_d0_latch |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
				}
				state->type3_d0_latch = save & 1;
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- 8041-DATA\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				save = 0xff;	/* open data bus? */
				data =
					state->type3_d0_latch |
					(BIT(save, 1) << 1) |
					(BIT(save, 2) << 2) |
					(BIT(save, 3) << 3) |
					(BIT(save, 4) << 4) |
					(BIT(save, 5) << 5) |
					(BIT(save, 6) << 7) |
					(BIT(save, 7) << 6);
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
				state->type3_d0_latch = save & 1;
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type3_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (1 == (offset & 1))
	{
		if (1 == state->type3_pal_19)
		{
			state->type3_ctrs = data << 4;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, "LDCTRS"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			state->type3_pal_19 = 1;
	}
	else
	{
		if (1 == state->type3_pal_19)
		{
			/* write nowhere?? */
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	upi41_master_w(state->mcu, offset, data);
}

/***************************************************************************
 *
 *  TYPE4 DONGLE
 *  - Scrum Try
 *  Contains a 32K (EP)ROM that can be read from any byte
 *  boundary sequentially. The EPROM is enable after writing
 *  1100xxxx to E5x1 once. Then an address is written LSB
 *  to E5x0 MSB to E5x1 and every read from E5x1 returns the
 *  next byte of the contents.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type4_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = upi41_master_r(state->mcu, 1);
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- 8041 STATUS\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
	}
	else
	{
		if (state->type4_latch)
		{
			UINT8 *prom = memory_region(space->machine, "dongle");

			data = prom[state->type4_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- PROM[%04x]\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.', state->type4_ctrs));
			state->type4_ctrs = (state->type4_ctrs + 1) & 0x7fff;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = upi41_master_r(state->mcu, 0);
				LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type4_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (1 == (offset & 1))
	{
		if (1 == state->type4_latch)
		{
			state->type4_ctrs = (state->type4_ctrs & 0x00ff) | ((data & 0x7f) << 8);
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS MSB (%04x)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, state->type4_ctrs));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
		{
			state->type4_latch = 1;
		}
	}
	else
	{
		if (state->type4_latch)
		{
			state->type4_ctrs = (state->type4_ctrs & 0xff00) | data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS LSB (%04x)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, state->type4_ctrs));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	upi41_master_w(state->mcu, offset, data);
}

/***************************************************************************
 *
 *  TYPE5 DONGLE
 *  - Boulder Dash
 *  Actually a NOP dongle returning 0x55 after triggering a latch
 *  by writing 1100xxxx to E5x1
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_type5_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = upi41_master_r(state->mcu, 1);
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- 8041 STATUS\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
	}
	else
	{
		if (state->type5_latch)
		{
			data = 0x55;	/* Only a fixed value? It looks like this is all we need to do */
			LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- fixed value???\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = upi41_master_r(state->mcu, 0);
				LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type5_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (1 == (offset & 1))
	{
		if (1 == state->type5_latch)
		{
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, "latch #2??"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			state->type5_latch = 1;
	}
	else
	{
		if (state->type5_latch)
		{
			/* write nowhere?? */
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	upi41_master_w(state->mcu, offset, data);
}

/***************************************************************************
 *
 *  NO DONGLE
 *  - Flying Ball
 *  A NOP dongle returning the data read from cassette as is.
 *
 ***************************************************************************/

static READ8_HANDLER( decocass_nodong_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = upi41_master_r(state->mcu, 1);
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- 8041 STATUS\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = upi41_master_r(state->mcu, 0);
			LOG(3,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
		}
	}

	return data;
}

/***************************************************************************
 *
 *  Main dongle and 8041 interface
 *
 ***************************************************************************/

READ8_HANDLER( decocass_e5xx_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	/* E5x2-E5x3 and mirrors */
	if (2 == (offset & E5XX_MASK))
	{
		UINT8 bot_eot = (tape_get_status_bits(state->cassette) >> 5) & 1;

		data =
			(BIT(state->i8041_p1, 7)	  << 0) |	/* D0 = P17 - REQ/ */
			(BIT(state->i8041_p2, 0)	  << 1) |	/* D1 = P20 - FNO/ */
			(BIT(state->i8041_p2, 1)	  << 2) |	/* D2 = P21 - EOT/ */
			(BIT(state->i8041_p2, 2)	  << 3) |	/* D3 = P22 - ERR/ */
			((bot_eot)	          << 4) |	/* D4 = BOT/EOT (direct from drive) */
			(1					  << 5) |	/* D5 floating input */
			(1					  << 6) |	/* D6 floating input */
			(!tape_is_present(state->cassette) << 7);	/* D7 = cassette present */

		LOG(4,("%10s 6502-PC: %04x decocass_e5xx_r(%02x): $%02x <- STATUS (%s%s%s%s%s%s%s%s)\n",
			attotime_string(timer_get_time(space->machine), 6),
			cpu_get_previouspc(space->cpu),
			offset, data,
			data & 0x01 ? "" : "REQ/",
			data & 0x02 ? "" : " FNO/",
			data & 0x04 ? "" : " EOT/",
			data & 0x08 ? "" : " ERR/",
			data & 0x10 ? " [BOT-EOT]" : "",
			data & 0x20 ? " [BIT5?]" : "",
			data & 0x40 ? " [BIT6?]" : "",
			data & 0x80 ? "" : " [CASS-PRESENT/]"));
	}
	else
	{
		if (state->dongle_r)
			data = (*state->dongle_r)(space, offset);
		else
			data = 0xff;
	}
	return data;
}

WRITE8_HANDLER( decocass_e5xx_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (state->dongle_w)
	{
		(*state->dongle_w)(space, offset, data);
		return;
	}

	if (0 == (offset & E5XX_MASK))
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
		upi41_master_w(state->mcu, offset & 1, data);
#ifdef MAME_DEBUG
		decocass_fno(space->machine, offset, data);
#endif
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> dongle\n", attotime_string(timer_get_time(space->machine), 6), cpu_get_previouspc(space->cpu), offset, data));
	}
}

/***************************************************************************
 *
 *  DE-0091xx daughter board handler
 *
 *  The DE-0091xx daughter board seems to be a read-only ROM board with
 *  two times five 4K ROMs. The only game using it (so far) is
 *  Treasure Island, which has 4 ROMs.
 *  The board's ROMs are mapped into view for reads between addresses
 *  0x6000 and 0xafff by setting bit0 of address 0xe900.
 *
 ***************************************************************************/

WRITE8_HANDLER( decocass_e900_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	state->de0091_enable = data & 1;
	memory_set_bank(space->machine, "bank1", data & 1);
	/* Perhaps the second row of ROMs is enabled by another bit.
     * There is no way to verify this yet, so for now just look
     * at bit 0 to enable the daughter board at reads between
     * 0x6000 and 0xafff.
     */
}

WRITE8_HANDLER( decocass_de0091_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	/* don't allow writes to the ROMs */
	if (!state->de0091_enable)
		decocass_charram_w(space, offset, data);
}

/***************************************************************************
 *
 *  state save setup
 *
 ***************************************************************************/
/* To be called once from driver_init, i.e. decocass_init */
void decocass_machine_state_save_init( running_machine *machine )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	state_save_register_global(machine, state->firsttime);
	state_save_register_global(machine, state->decocass_reset);
	state_save_register_global(machine, state->i8041_p1);
	state_save_register_global(machine, state->i8041_p2);
	state_save_register_global(machine, state->de0091_enable);
	state_save_register_global(machine, state->type1_inmap);
	state_save_register_global(machine, state->type1_outmap);
	state_save_register_global(machine, state->type2_d2_latch);
	state_save_register_global(machine, state->type2_xx_latch);
	state_save_register_global(machine, state->type2_promaddr);
	state_save_register_global(machine, state->type3_ctrs);
	state_save_register_global(machine, state->type3_d0_latch);
	state_save_register_global(machine, state->type3_pal_19);
	state_save_register_global(machine, state->type3_swap);
	state_save_register_global(machine, state->type4_ctrs);
	state_save_register_global(machine, state->type4_latch);
	state_save_register_global(machine, state->type5_latch);
	state_save_register_global(machine, state->sound_ack);

	state_save_register_global_array(machine, state->quadrature_decoder);
	state_save_register_global(machine, state->latch1);
	state_save_register_global(machine, state->audio_nmi_enabled);
	state_save_register_global(machine, state->audio_nmi_state);
	state_save_register_global(machine, state->i8041_p1_write_latch);
	state_save_register_global(machine, state->i8041_p2_write_latch);
	state_save_register_global(machine, state->i8041_p1_read_latch);
	state_save_register_global(machine, state->i8041_p2_read_latch);
}

/***************************************************************************
 *
 *  init machine functions (select dongle and determine tape image size)
 *
 ***************************************************************************/

MACHINE_START( decocass )
{
	decocass_state *state = (decocass_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->mcu = machine->device("mcu");
	state->cassette = machine->device("cassette");
}

static void decocass_reset_common( running_machine *machine )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	state->firsttime = 1;
	state->latch1 = 0;

	state->dongle_r = NULL;
	state->dongle_w = NULL;

	state->decocass_reset = 0;
	state->i8041_p1 = 0xff;
	state->i8041_p2 = 0xff;
	state->i8041_p1_write_latch = 0xff;
	state->i8041_p2_write_latch = 0xff;
	state->i8041_p1_read_latch = 0xff;
	state->i8041_p2_read_latch = 0xff;
	state->de0091_enable = 0;

	state->type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	state->type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);

	state->type2_d2_latch = 0;
	state->type2_xx_latch = 0;
	state->type2_promaddr = 0;

	state->type3_ctrs = 0;
	state->type3_d0_latch = 0;
	state->type3_pal_19 = 0;
	state->type3_swap = 0;

	state->type4_ctrs = 0;
	state->type4_latch = 0;

	state->type5_latch = 0;

	memset(state->quadrature_decoder, 0, sizeof(state->quadrature_decoder));
	state->sound_ack = 0;
	state->audio_nmi_enabled = 0;
	state->audio_nmi_state = 0;

	/* video-related */
	state->watchdog_flip = 0;
	state->color_missiles = 0;
	state->color_center_bot = 0;
	state->mode_set = 0;
	state->back_h_shift = 0;
	state->back_vl_shift = 0;
	state->back_vr_shift = 0;
	state->part_h_shift = 0;
	state->part_v_shift = 0;
	state->center_h_shift_space = 0;
	state->center_v_shift = 0;
}

MACHINE_RESET( decocass )
{
	decocass_reset_common(machine);
}

MACHINE_RESET( ctsttape )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061)\n"));
	state->dongle_r = decocass_type1_pass_136_r;
}

MACHINE_RESET( chwy )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	state->dongle_r = decocass_type1_latch_27_pass_3_inv_2_r;
}

MACHINE_RESET( clocknch )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 flip 2-3)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(0,1,3,2,4,5,6,7);
	state->type1_outmap = MAKE_MAP(0,1,3,2,4,5,6,7);
}

MACHINE_RESET( ctisland )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 flip 0-2)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(2,1,0,3,4,5,6,7);
	state->type1_outmap = MAKE_MAP(2,1,0,3,4,5,6,7);
}

MACHINE_RESET( csuperas )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 flip 4-5)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(0,1,2,3,5,4,6,7);
	state->type1_outmap = MAKE_MAP(0,1,2,3,5,4,6,7);
}

MACHINE_RESET( castfant )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061)\n"));
	state->dongle_r = decocass_type1_latch_16_pass_3_inv_1_r;
}

MACHINE_RESET( cluckypo )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 flip 1-3)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(0,3,2,1,4,5,6,7);
	state->type1_outmap = MAKE_MAP(0,3,2,1,4,5,6,7);
}

MACHINE_RESET( cterrani )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 straight)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	state->type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

MACHINE_RESET( cexplore )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_5_inv_2_r;
}

MACHINE_RESET( cprogolf )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #1 (DE-0061 flip 0-1)\n"));
	state->dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	state->type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	state->type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET( cmissnx )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #2 (CS82-007)\n"));
	state->dongle_r = decocass_type2_r;
	state->dongle_w = decocass_type2_w;
}

MACHINE_RESET( cdiscon1 )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #2 (CS82-007)\n"));
	state->dongle_r = decocass_type2_r;
	state->dongle_w = decocass_type2_w;
}

MACHINE_RESET( cptennis )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #2 (CS82-007)\n"));
	state->dongle_r = decocass_type2_r;
	state->dongle_w = decocass_type2_w;
}

MACHINE_RESET( ctornado )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #2 (CS82-007)\n"));
	state->dongle_r = decocass_type2_r;
	state->dongle_w = decocass_type2_w;
}

MACHINE_RESET( cbnj )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( cburnrub )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( cbtime )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_12;
}

MACHINE_RESET( cgraplop )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_56;
}

MACHINE_RESET( cgraplop2 )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( clapapa )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_34_7;
}

MACHINE_RESET( cfghtice )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_25;
}

MACHINE_RESET( cprobowl )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_34_0;
}

MACHINE_RESET( cnightst )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_13;
}

MACHINE_RESET( cprosocc )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_24;
}

MACHINE_RESET( cppicf )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_01;
}

MACHINE_RESET( cscrtry )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #4 (32K ROM)\n"));
	state->dongle_r = decocass_type4_r;
	state->dongle_w = decocass_type4_w;
}

MACHINE_RESET( cbdash )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("dongle type #5 (NOP)\n"));
	state->dongle_r = decocass_type5_r;
	state->dongle_w = decocass_type5_w;
}

MACHINE_RESET( cflyball )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	decocass_reset_common(machine);
	LOG(0,("no dongle\n"));
	state->dongle_r = decocass_nodong_r;
}

MACHINE_RESET( czeroize )
{
	decocass_state *state = (decocass_state *)machine->driver_data;
	UINT8 *mem = memory_region(machine, "dongle");
	decocass_reset_common(machine);
	LOG(0,("dongle type #3 (PAL)\n"));
	state->dongle_r = decocass_type3_r;
	state->dongle_w = decocass_type3_w;
	state->type3_swap = TYPE3_SWAP_23_56;

	/*
     * FIXME: remove if the original ROM is available.
     * The Zeroize 6502 code at 0x3707 issues LODCTRS with 0x8a,
     * and expects to read 0x18 from 0x08a0 ff. within 7 bytes
     * and 0xf7 from 0x8a1 (which 0xd is subtracted from presumably in order
     * to form a NOP of 0xea).
     * This hack seems to be sufficient to get around
     * the missing dongle ROM contents and play the game.
     */
	memset(mem, 0x00, 0x1000);
	mem[0x08a0] = 0x18;
	mem[0x08a1] = 0xf7;
}

/***************************************************************************
 *
 *  8041 port handlers
 *
 ***************************************************************************/

WRITE8_HANDLER( i8041_p1_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (data != state->i8041_p1_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_string(timer_get_time(space->machine), 6),
			cpu_get_previouspc(space->cpu),
			data,
			data & 0x01 ? "" : "DATA-WRT",
			data & 0x02 ? "" : " DATA-CLK",
			data & 0x04 ? "" : " FAST",
			data & 0x08 ? "" : " BIT3",
			data & 0x10 ? "" : " REW",
			data & 0x20 ? "" : " FWD",
			data & 0x40 ? "" : " WREN",
			data & 0x80 ? "" : " REQ"));
		state->i8041_p1_write_latch = data;
	}

	/* change in FAST/REW/FWD signals? */
	if ((data ^ state->i8041_p1) & 0x34)
	{
		int newspeed = 0;

		if ((data & 0x30) == 0x20)
			newspeed = (data & 0x04) ? -1 : -7;
		else if ((data & 0x30) == 0x10)
			newspeed = (data & 0x04) ? 1 : 7;
		tape_change_speed(state->cassette, newspeed);
	}

	state->i8041_p1 = data;
}

READ8_HANDLER( i8041_p1_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data = state->i8041_p1;

	if (data != state->i8041_p1_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_string(timer_get_time(space->machine), 6),
			cpu_get_previouspc(space->cpu),
			data,
			data & 0x01 ? "" : "DATA-WRT",
			data & 0x02 ? "" : " DATA-CLK",
			data & 0x04 ? "" : " FAST",
			data & 0x08 ? "" : " BIT3",
			data & 0x10 ? "" : " REW",
			data & 0x20 ? "" : " FWD",
			data & 0x40 ? "" : " WREN",
			data & 0x80 ? "" : " REQ"));
		state->i8041_p1_read_latch = data;
	}
	return data;
}

WRITE8_HANDLER( i8041_p2_w )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	if (data != state->i8041_p2_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_string(timer_get_time(space->machine), 6),
			cpu_get_previouspc(space->cpu),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		state->i8041_p2_write_latch = data;
	}
	state->i8041_p2 = (state->i8041_p2 & 0xe0) | (data & ~0xe0);
}

READ8_HANDLER( i8041_p2_r )
{
	decocass_state *state = (decocass_state *)space->machine->driver_data;
	UINT8 data;

	data = (state->i8041_p2 & ~0xe0) | tape_get_status_bits(state->cassette);

	if (data != state->i8041_p2_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_string(timer_get_time(space->machine), 6),
			cpu_get_previouspc(space->cpu),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		state->i8041_p2_read_latch = data;
	}
	return data;
}



/***************************************************************************
    CASSETTE DEVICE INTERFACE
***************************************************************************/

/* regions within the virtual tape */
enum _tape_region
{
	REGION_LEADER,				/* in clear leader section */
	REGION_LEADER_GAP,			/* in gap between leader and BOT */
	REGION_BOT,					/* in BOT hole */
	REGION_BOT_GAP,				/* in gap between BOT hole and data */
	REGION_DATA_BLOCK_0,		/* in data block 0 */
	REGION_DATA_BLOCK_255 = REGION_DATA_BLOCK_0 + 255,
	REGION_EOT_GAP,				/* in gap between data and EOT hole */
	REGION_EOT,					/* in EOT hole */
	REGION_TRAILER_GAP,			/* in gap between trailer and EOT */
	REGION_TRAILER				/* in clear trailer section */
};
typedef enum _tape_region tape_region;


/* bytes within a data block on a virtual tape */
enum _tape_byte
{
	BYTE_PRE_GAP_0,				/* 34 bytes of gap, clock held to 0, no data */
	BYTE_PRE_GAP_33 = BYTE_PRE_GAP_0 + 33,
	BYTE_LEADIN,				/* 1 leadin byte, clocked value 0x00 */
	BYTE_HEADER,				/* 1 header byte, clocked value 0xAA */
	BYTE_DATA_0,				/* 256 bytes of data, clocked */
	BYTE_DATA_255 = BYTE_DATA_0 + 255,
	BYTE_CRC16_MSB,				/* 2 bytes of CRC, clocked MSB first, then LSB */
	BYTE_CRC16_LSB,
	BYTE_TRAILER,				/* 1 trailer byte, clocked value 0xAA */
	BYTE_LEADOUT,				/* 1 leadout byte, clocked value 0x00 */
	BYTE_LONGCLOCK,				/* 1 longclock byte, clock held to 1, no data */
	BYTE_POSTGAP_0,				/* 34 bytes of gap, no clock, no data */
	BYTE_POSTGAP_33 = BYTE_POSTGAP_0 + 33,
	BYTE_BLOCK_TOTAL			/* total number of bytes in block */
};
typedef enum _tape_byte tape_byte;


/* state of the tape */
typedef struct _tape_state tape_state;
struct _tape_state
{
	running_machine *	machine;			/* pointer back to the machine */
	emu_timer *			timer;				/* timer for running the tape */
	INT8				speed;				/* speed: <-1=fast rewind, -1=reverse, 0=stopped, 1=normal, >1=fast forward */
	tape_region			region;				/* current region */
	tape_byte			bytenum;			/* byte number within a datablock */
	UINT8				bitnum;				/* bit number within a byte */
	UINT32				clockpos;			/* the current clock position of the tape */
	UINT32				numclocks;			/* total number of clocks on the entire tape */
	UINT16				crc16[256];			/* CRC16 for each block */
};


/* number of tape clock pulses per second */
#define TAPE_CLOCKRATE					4800
#define TAPE_CLOCKS_PER_BIT				2
#define TAPE_CLOCKS_PER_BYTE			(8 * TAPE_CLOCKS_PER_BIT)
#define TAPE_MSEC_TO_CLOCKS(x)			((x) * TAPE_CLOCKRATE / 1000)


/* Note on a tapes leader-BOT-data-EOT-trailer format:
 * A cassette has a transparent piece of tape on both ends,
 * leader and trailer. And data tapes also have BOT and EOT
 * holes, shortly before the the leader and trailer.
 * The holes and clear tape are detected using a photo-resitor.
 * When rewinding, the BOT/EOT signal will show a short
 * pulse and if rewind continues a constant high signal later.
 * The specs say the holes are "> 2ms" in length.
 */

/* duration of the clear LEADER (and trailer) of the tape */
#define REGION_LEADER_START_CLOCK		0
#define REGION_LEADER_LEN_CLOCKS		TAPE_MSEC_TO_CLOCKS(1000)	/* 1s */
#define REGION_LEADER_END_CLOCK			(REGION_LEADER_START_CLOCK+REGION_LEADER_LEN_CLOCKS)

/* duration of the GAP between leader and BOT/EOT */
#define REGION_LEADER_GAP_START_CLOCK	REGION_LEADER_END_CLOCK
#define REGION_LEADER_GAP_LEN_CLOCKS	TAPE_MSEC_TO_CLOCKS(1500)	/* 1.5s */
#define REGION_LEADER_GAP_END_CLOCK		(REGION_LEADER_GAP_START_CLOCK+REGION_LEADER_GAP_LEN_CLOCKS)

/* duration of BOT/EOT holes */
#define REGION_BOT_START_CLOCK			REGION_LEADER_GAP_END_CLOCK
#define REGION_BOT_LEN_CLOCKS			TAPE_MSEC_TO_CLOCKS(2.5)	/* 0.0025s */
#define REGION_BOT_END_CLOCK			(REGION_BOT_START_CLOCK+REGION_BOT_LEN_CLOCKS)

/* gap between BOT/EOT and first/last data block */
#define REGION_BOT_GAP_START_CLOCK		REGION_BOT_END_CLOCK
#define REGION_BOT_GAP_LEN_CLOCKS		TAPE_MSEC_TO_CLOCKS(300)	/* 300ms */
#define REGION_BOT_GAP_END_CLOCK		(REGION_BOT_GAP_START_CLOCK+REGION_BOT_GAP_LEN_CLOCKS)


/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE tape_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == DECOCASS_TAPE);

	return (tape_state *)downcast<legacy_device_base *>(device)->token();
}


/*-------------------------------------------------
    tape_crc16_byte - accumulate 8 bits worth of
    CRC data
-------------------------------------------------*/

static UINT16 tape_crc16_byte(UINT16 crc, UINT8 data)
{
	int bit;

	for (bit = 0; bit < 8; bit++)
	{
		crc = (crc >> 1) | (crc << 15);
		crc ^= (data << 7) & 0x80;
		if (crc & 0x80)
			crc ^= 0x0120;
		data >>= 1;
	}
	return crc;
}


/*-------------------------------------------------
    tape_describe_state - create a string that
    describes the state of the tape
-------------------------------------------------*/

static const char *tape_describe_state(tape_state *tape)
{
	static char buffer[40];
	char temprname[40];
	const char *rname = temprname;

	if (tape->region == REGION_LEADER)
		rname = "LEAD";
	else if (tape->region == REGION_LEADER_GAP)
		rname = "LGAP";
	else if (tape->region == REGION_BOT)
		rname = "BOT ";
	else if (tape->region == REGION_BOT_GAP)
		rname = "BGAP";
	else if (tape->region == REGION_TRAILER)
		rname = "TRLR";
	else if (tape->region == REGION_TRAILER_GAP)
		rname = "TGAP";
	else if (tape->region == REGION_EOT)
		rname = "EOT ";
	else if (tape->region == REGION_EOT_GAP)
		rname = "EGAP";
	else
	{
		char tempbname[40];
		const char *bname = tempbname;
		int clk;

		if (tape->bytenum <= BYTE_PRE_GAP_33)
			sprintf(tempbname, "PR%02d", tape->bytenum - BYTE_PRE_GAP_0);
		else if (tape->bytenum == BYTE_LEADIN)
			bname = "LDIN";
		else if (tape->bytenum == BYTE_HEADER)
			bname = "HEAD";
		else if (tape->bytenum <= BYTE_DATA_255)
			sprintf(tempbname, "BY%02X", tape->bytenum - BYTE_DATA_0);
		else if (tape->bytenum == BYTE_CRC16_MSB)
			bname = "CRCM";
		else if (tape->bytenum == BYTE_CRC16_LSB)
			bname = "CRCL";
		else if (tape->bytenum == BYTE_TRAILER)
			bname = "TRLR";
		else if (tape->bytenum == BYTE_LEADOUT)
			bname = "LOUT";
		else if (tape->bytenum == BYTE_LONGCLOCK)
			bname = "LONG";
		else
			sprintf(tempbname, "PO%02d", tape->bytenum - BYTE_POSTGAP_0);

		/* in the main data area, the clock alternates at the clock rate */
		if (tape->bytenum >= BYTE_LEADIN && tape->bytenum <= BYTE_LEADOUT)
			clk = ((UINT32)(tape->clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0 : 1;
		else if (tape->bytenum == BYTE_LONGCLOCK)
			clk = 1;
		else
			clk = 0;

		sprintf(temprname, "BL%02X.%4s.%d.%d", tape->region - REGION_DATA_BLOCK_0, bname, tape->bitnum, clk);
	}

	sprintf(buffer, "{%9d=%s}", tape->clockpos, rname);
	return buffer;
}


/*-------------------------------------------------
    tape_clock_callback - called once per clock
    to increment/decrement the tape location
-------------------------------------------------*/

static TIMER_CALLBACK( tape_clock_callback )
{
	running_device *device = (running_device *)ptr;
	tape_state *tape = get_safe_token(device);

	/* advance by one clock in the desired direction */
	if (tape->speed < 0 && tape->clockpos > 0)
		tape->clockpos--;
	else if (tape->speed > 0 && tape->clockpos < tape->numclocks)
		tape->clockpos++;

	/* look for states before the start of data */
	if (tape->clockpos < REGION_LEADER_END_CLOCK)
		tape->region = REGION_LEADER;
	else if (tape->clockpos < REGION_LEADER_GAP_END_CLOCK)
		tape->region = REGION_LEADER_GAP;
	else if (tape->clockpos < REGION_BOT_END_CLOCK)
		tape->region = REGION_BOT;
	else if (tape->clockpos < REGION_BOT_GAP_END_CLOCK)
		tape->region = REGION_BOT_GAP;

	/* look for states after the end of data */
	else if (tape->clockpos >= tape->numclocks - REGION_LEADER_END_CLOCK)
		tape->region = REGION_TRAILER;
	else if (tape->clockpos >= tape->numclocks - REGION_LEADER_GAP_END_CLOCK)
		tape->region = REGION_TRAILER_GAP;
	else if (tape->clockpos >= tape->numclocks - REGION_BOT_END_CLOCK)
		tape->region = REGION_EOT;
	else if (tape->clockpos >= tape->numclocks - REGION_BOT_GAP_END_CLOCK)
		tape->region = REGION_EOT_GAP;

	/* everything else is data */
	else
	{
		UINT32 dataclock = tape->clockpos - REGION_BOT_GAP_END_CLOCK;

		/* compute the block number */
		tape->region = (tape_region)(REGION_DATA_BLOCK_0 + dataclock / (TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL));
		dataclock -= (tape->region - REGION_DATA_BLOCK_0) * TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL;

		/* compute the byte within the block */
		tape->bytenum = (tape_byte)(dataclock / TAPE_CLOCKS_PER_BYTE);
		dataclock -= tape->bytenum * TAPE_CLOCKS_PER_BYTE;

		/* compute the bit within the byte */
		tape->bitnum = dataclock / TAPE_CLOCKS_PER_BIT;
	}

	/* log */
	if (LOG_CASSETTE_STATE)
		tape_describe_state(tape);
}


/*-------------------------------------------------
    tape_get_status_bits - return the 3 status
    bits from the tape
-------------------------------------------------*/

static UINT8 tape_get_status_bits(running_device *device)
{
	tape_state *tape = get_safe_token(device);
	UINT8 tape_bits = 0;

	/* bit 0x20 is the BOT/EOT signal, which is also set in the leader/trailer area */
	if (tape->region == REGION_LEADER || tape->region == REGION_BOT || tape->region == REGION_EOT || tape->region == REGION_TRAILER)
		tape_bits |= 0x20;

	/* bit 0x40 is the clock, which is only valid in some areas of the data block */
	/* bit 0x80 is the data, which is only valid in some areas of the data block */
	if (tape->region >= REGION_DATA_BLOCK_0 && tape->region <= REGION_DATA_BLOCK_255)
	{
		int blocknum = tape->region - REGION_DATA_BLOCK_0;
		UINT8 byteval = 0x00;

		/* in the main data area, the clock alternates at the clock rate */
		if (tape->bytenum >= BYTE_LEADIN && tape->bytenum <= BYTE_LEADOUT)
			tape_bits |= ((UINT32)(tape->clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0x00 : 0x40;

		/* in the longclock area, the clock holds high */
		else if (tape->bytenum == BYTE_LONGCLOCK)
			tape_bits |= 0x40;

		/* everywhere else, the clock holds to 0 */
		else
			;

		/* lead-in and lead-out bytes are 0xAA */
		if (tape->bytenum == BYTE_HEADER || tape->bytenum == BYTE_TRAILER)
			byteval = 0xaa;

		/* data block bytes are data */
		else if (tape->bytenum >= BYTE_DATA_0 && tape->bytenum <= BYTE_DATA_255)
			byteval = static_cast<UINT8 *>(*device->region())[blocknum * 256 + (tape->bytenum - BYTE_DATA_0)];

		/* CRC MSB */
		else if (tape->bytenum == BYTE_CRC16_MSB)
			byteval = tape->crc16[blocknum] >> 8;

		/* CRC LSB */
		else if (tape->bytenum == BYTE_CRC16_LSB)
			byteval = tape->crc16[blocknum];

		/* select the appropriate bit from the byte and move to the upper bit */
		if ((byteval >> tape->bitnum) & 1)
			tape_bits |= 0x80;
	}
	return tape_bits;
}


/*-------------------------------------------------
    tape_is_present - return TRUE if the tape is
    present
-------------------------------------------------*/

static UINT8 tape_is_present(running_device *device)
{
	return device->region() != NULL;
}


/*-------------------------------------------------
    tape_change_speed - alter the speed of tape
    playback
-------------------------------------------------*/

static void tape_change_speed(running_device *device, INT8 newspeed)
{
	tape_state *tape = get_safe_token(device);
	attotime newperiod;
	INT8 absnewspeed;

	/* do nothing if speed has not changed */
	if (tape->speed == newspeed)
		return;

	/* compute how fast to run the tape timer */
	absnewspeed = (newspeed < 0) ? -newspeed : newspeed;
	if (newspeed == 0)
		newperiod = attotime_never;
	else
		newperiod = ATTOTIME_IN_HZ(TAPE_CLOCKRATE * absnewspeed);

	/* set the new speed */
	timer_adjust_periodic(tape->timer, newperiod, 0, newperiod);
	tape->speed = newspeed;
}


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( decocass_tape )
{
	tape_state *tape = get_safe_token(device);
	int curblock, offs, numblocks;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->baseconfig().static_config() == NULL);
	assert(downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config() == NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* fetch the data pointer */
	tape->timer = timer_alloc(device->machine, tape_clock_callback, (void *)device);
	if (device->region() == NULL)
		return;
	UINT8 *regionbase = *device->region();

	/* scan for the first non-empty block in the image */
	for (offs = device->region()->bytes() - 1; offs >= 0; offs--)
		if (regionbase[offs] != 0)
			break;
	numblocks = ((offs | 0xff) + 1) / 256;
	assert(numblocks < ARRAY_LENGTH(tape->crc16));

	/* compute the total length */
	tape->numclocks = REGION_BOT_GAP_END_CLOCK + numblocks * BYTE_BLOCK_TOTAL * 16 + REGION_BOT_GAP_END_CLOCK;

	/* compute CRCs for each block */
	for (curblock = 0; curblock < numblocks; curblock++)
	{
		UINT16 crc = 0;
		int testval;

		/* first CRC the 256 bytes of data */
		for (offs = 256 * curblock; offs < 256 * curblock + 256; offs++)
			crc = tape_crc16_byte(crc, regionbase[offs]);

		/* then find a pair of bytes that will bring the CRC to 0 (any better way than brute force?) */
		for (testval = 0; testval < 0x10000; testval++)
			if (tape_crc16_byte(tape_crc16_byte(crc, testval >> 8), testval) == 0)
				break;
		tape->crc16[curblock] = testval;
	}

	/* register states */
	state_save_register_device_item(device, 0, tape->speed);
	state_save_register_device_item(device, 0, tape->bitnum);
	state_save_register_device_item(device, 0, tape->clockpos);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( decocass_tape )
{
	/* turn the tape off */
	tape_change_speed(device, 0);
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( decocass_tape )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tape_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(decocass_tape); break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(decocass_tape);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "DECO Cassette Tape");	break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Tape Controller");		break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_DEVICE(DECOCASS_TAPE, decocass_tape);
