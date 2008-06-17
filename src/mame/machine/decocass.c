/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "cpu/i8x41/i8x41.h"
#include "machine/decocass.h"

/* tape direction, speed and timing (used also in video/decocass.c) */
INT32 tape_dir;
INT32 tape_speed;
attotime tape_time0;
emu_timer *tape_timer;

static INT32 firsttime = 1;
static INT32 tape_present;
static INT32 tape_blocks;
static INT32 tape_length;
static INT32 tape_bot_eot;
static UINT8 crc16_lsb;
static UINT8 crc16_msb;

/* pre-calculated crc16 of the tape blocks */
static UINT8 tape_crc16_lsb[256];
static UINT8 tape_crc16_msb[256];

static read8_machine_func decocass_dongle_r;
static write8_machine_func decocass_dongle_w;

static UINT8 decocass_reset;
static UINT8 i8041_p1;
static UINT8 i8041_p2;

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

static UINT32 type1_inmap;
static UINT32 type1_outmap;

static INT32 de0091_enable;	/* DE-0091xx daughter board enable */

/* dongle type #2: status of the latches */
static INT32 type2_d2_latch;	/* latched 8041-STATUS D2 value */
static INT32 type2_xx_latch;	/* latched value (D7-4 == 0xc0) ? 1 : 0 */
static INT32 type2_promaddr;	/* latched PROM address A0-A7 */

/* dongle type #3: status and patches */
static INT32 type3_ctrs;		/* 12 bit counter stage */
static INT32 type3_d0_latch;	/* latched 8041-D0 value */
static INT32 type3_pal_19;		/* latched 1 for PAL input pin-19 */
static INT32 type3_swap;
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

/* dongle type #4: status */
static INT32 type4_ctrs;	/* latched PROM address (E5x0 LSB, E5x1 MSB) */
static INT32 type4_latch; 	/* latched enable PROM (1100xxxx written to E5x1) */

/* dongle type #5: status */
static INT32 type5_latch; 	/* latched enable PROM (1100xxxx written to E5x1) */

/* four inputs from the quadrature decoder (H1, V1, H2, V2) */
static UINT8 decocass_quadrature_decoder[4];

/* sound latches, ACK status bits and NMI timer */
static UINT8 decocass_sound_ack;
static emu_timer *decocass_sound_timer;

WRITE8_HANDLER( decocass_coin_counter_w )
{
}

WRITE8_HANDLER( decocass_sound_command_w )
{
	LOG(2,("CPU #%d sound command -> $%02x\n", cpu_getactivecpu(), data));
	soundlatch_w(machine,0,data);
	decocass_sound_ack |= 0x80;
	/* remove snd cpu data ack bit. i don't see it in the schems, but... */
	decocass_sound_ack &= ~0x40;
	cpunum_set_input_line(machine, 1, M6502_IRQ_LINE, ASSERT_LINE);
}

READ8_HANDLER( decocass_sound_data_r )
{
	UINT8 data = soundlatch2_r(machine, 0);
	LOG(2,("CPU #%d sound data    <- $%02x\n", cpu_getactivecpu(), data));
	return data;
}

READ8_HANDLER( decocass_sound_ack_r )
{
	UINT8 data = decocass_sound_ack;	/* D6+D7 */
	LOG(4,("CPU #%d sound ack     <- $%02x\n", cpu_getactivecpu(), data));
	return data;
}

WRITE8_HANDLER( decocass_sound_data_w )
{
	LOG(2,("CPU #%d sound data    -> $%02x\n", cpu_getactivecpu(), data));
	soundlatch2_w(machine, 0, data);
	decocass_sound_ack |= 0x40;
}

READ8_HANDLER( decocass_sound_command_r )
{
	UINT8 data = soundlatch_r(machine, 0);
	LOG(4,("CPU #%d sound command <- $%02x\n", cpu_getactivecpu(), data));
	cpunum_set_input_line(machine, 1, M6502_IRQ_LINE, CLEAR_LINE);
	decocass_sound_ack &= ~0x80;
	return data;
}

static TIMER_CALLBACK( decocass_sound_nmi_pulse )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_HANDLER( decocass_sound_nmi_enable_w )
{
	LOG(2,("CPU #%d sound NMI enb -> $%02x\n", cpu_getactivecpu(), data));
	timer_adjust_periodic(decocass_sound_timer, ATTOTIME_IN_HZ(256 * 57 / 8 / 2), 0, ATTOTIME_IN_HZ(256 * 57 / 8 / 2));
}

READ8_HANDLER( decocass_sound_nmi_enable_r )
{
	UINT8 data = 0xff;
	LOG(2,("CPU #%d sound NMI enb <- $%02x\n", cpu_getactivecpu(), data));
	timer_adjust_periodic(decocass_sound_timer, ATTOTIME_IN_HZ(256 * 57 / 8 / 2), 0, ATTOTIME_IN_HZ(256 * 57 / 8 / 2));
	return data;
}

READ8_HANDLER( decocass_sound_data_ack_reset_r )
{
	UINT8 data = 0xff;
	LOG(2,("CPU #%d sound ack rst <- $%02x\n", cpu_getactivecpu(), data));
	decocass_sound_ack &= ~0x40;
	return data;
}

WRITE8_HANDLER( decocass_sound_data_ack_reset_w )
{
	LOG(2,("CPU #%d sound ack rst -> $%02x\n", cpu_getactivecpu(), data));
	decocass_sound_ack &= ~0x40;
}

WRITE8_HANDLER( decocass_nmi_reset_w )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, CLEAR_LINE );
}

WRITE8_HANDLER( decocass_quadrature_decoder_reset_w )
{
	/* just latch the analog controls here */
	decocass_quadrature_decoder[0] = input_port_read(machine, "AN0");
	decocass_quadrature_decoder[1] = input_port_read(machine, "AN1");
	decocass_quadrature_decoder[2] = input_port_read(machine, "AN2");
	decocass_quadrature_decoder[3] = input_port_read(machine, "AN3");
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
	UINT8 data = 0xff;
	static const char *portnames[] = { "IN0", "IN1", "IN2" };
	
	switch (offset & 7)
	{
	case 0: case 1: case 2:
		data = input_port_read(machine, portnames[offset & 7]);
		break;
	case 3: case 4: case 5: case 6:
		data = decocass_quadrature_decoder[(offset & 7) - 3];
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
/* Note on a tapes leader-BOT-data-EOT-trailer format:
 * A cassette has a transparent piece of tape on both ends,
 * leader and trailer. And data tapes also have BOT and EOT
 * holes, shortly before the the leader and trailer.
 * The holes and clear tape are detected using a photo-resitor.
 * When rewinding, the BOT/EOT signal will show a short
 * pulse and if rewind continues a constant high signal later.
 * The specs say the holes are "> 2ms" in length.
 */

#define TAPE_CLOCKRATE	4800	/* clock pulses per second */

/* duration of the clear LEADER (and trailer) of the tape */
#define TAPE_LEADER 	TAPE_CLOCKRATE		/* 1s */
/* duration of the GAP between leader and BOT/EOT */
#define TAPE_GAP		TAPE_CLOCKRATE*3/2	/* 1.5s */
/* duration of BOT/EOT holes */
#define TAPE_HOLE		TAPE_CLOCKRATE/400	/* 0.0025s */

/* byte offset of the tape chunks (8 clocks per byte = 16 samples) */
/* 300 ms GAP between BOT and first data block (doesn't work.. thus /2) */
#define TAPE_PRE_GAP	34
#define TAPE_LEADIN 	(TAPE_PRE_GAP + 1)
#define TAPE_HEADER 	(TAPE_LEADIN + 1)
#define TAPE_BLOCK		(TAPE_HEADER + 256)
#define TAPE_CRC16_MSB	(TAPE_BLOCK + 1)
#define TAPE_CRC16_LSB	(TAPE_CRC16_MSB + 1)
#define TAPE_TRAILER	(TAPE_CRC16_LSB + 1)
#define TAPE_LEADOUT	(TAPE_TRAILER + 1)
#define TAPE_LONGCLOCK	(TAPE_LEADOUT + 1)
#define TAPE_POST_GAP	(TAPE_LONGCLOCK + 34)

/* size of a tape chunk (block) including gaps */
#define TAPE_CHUNK		TAPE_POST_GAP

#define E5XX_MASK	0x02	/* use 0x0e for old style board */

#define BIT0(x) ((x)&1)
#define BIT1(x) (((x)>>1)&1)
#define BIT2(x) (((x)>>2)&1)
#define BIT3(x) (((x)>>3)&1)
#define BIT4(x) (((x)>>4)&1)
#define BIT5(x) (((x)>>5)&1)
#define BIT6(x) (((x)>>6)&1)
#define BIT7(x) (((x)>>7)&1)

WRITE8_HANDLER( decocass_reset_w )
{
	LOG(1,("%9.7f 6502-PC: %04x decocass_reset_w(%02x): $%02x\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
	decocass_reset = data;

	/* CPU #1 active hight reset */
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, data & 0x01 );

	/* on reset also remove the sound timer */
	if (data & 1)
		timer_adjust_oneshot(decocass_sound_timer, attotime_never, 0);

	/* 8041 active low reset */
	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, (data & 0x08) ^ 0x08 );
}

static const char *dirnm(int speed)
{
	if (speed <  -1) return "fast rewind";
	if (speed == -1) return "rewind";
	if (speed ==  0) return "stop";
	if (speed ==  1) return "forward";
	return "fast forward";
}

static void tape_crc16(UINT8 data)
{
	UINT8 c0, c1;
	UINT8 old_lsb = crc16_lsb;
	UINT8 old_msb = crc16_msb;
	UINT8 feedback;

	feedback = ((data >> 7) ^ crc16_msb) & 1;

	/* rotate 16 bits */
	c0 = crc16_lsb & 1;
	c1 = crc16_msb & 1;
	crc16_msb = (crc16_msb >> 1) | (c0 << 7);
	crc16_lsb = (crc16_lsb >> 1) | (c1 << 7);

	/* feedback into bit 7 */
	if (feedback)
		crc16_lsb |= 0x80;
	else
		crc16_lsb &= ~0x80;

	/* feedback to bit 6 into bit 5 */
	if (((old_lsb >> 6) ^ feedback) & 1)
		crc16_lsb |= 0x20;
	else
		crc16_lsb &= ~0x20;

	/* feedback to bit 1 into bit 0 */
	if (((old_msb >> 1) ^ feedback) & 1)
		crc16_msb |= 0x01;
	else
		crc16_msb &= ~0x01;
}


attotime decocass_adjust_tape_time(attotime tape_time)
{
	attotime ret = tape_time;

	if (tape_timer)
	{
		attotime elapsed = timer_timeelapsed(tape_timer);

		if (tape_dir > 0)
		{
			ret = attotime_add(tape_time, elapsed);

			if (attotime_compare(ret, ATTOTIME_IN_MSEC(999900)) > 0)
				ret = ATTOTIME_IN_MSEC(999900);
		}

		if (tape_dir < 0)
		{
			if (attotime_compare(tape_time, elapsed) > 0)
				ret = attotime_sub(tape_time, elapsed);
			else
				ret = attotime_zero;
		}
	}

	return ret;
}


static void tape_update(void)
{
	static int last_byte;
	int offset, rclk, rdata, tape_bit, tape_byte, tape_block;

	attotime tape_time = decocass_adjust_tape_time(tape_time0);

	offset = (int)(attotime_to_double(attotime_mul(tape_time, TAPE_CLOCKRATE)) + 0.499995);

	/* reset RCLK and RDATA inputs */
	rclk = 0;
	rdata = 0;

	if (offset < TAPE_LEADER)
	{
		if (offset < 0)
			offset = 0;
		/* LEADER area */
		if (0 == tape_bot_eot)
		{
			tape_bot_eot = 1;
			set_led_status(1, 1);
			LOG(5,("tape %5.4fs: %s found LEADER\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
	}
	else
	if (offset < TAPE_LEADER + TAPE_GAP)
	{
		/* GAP between LEADER and BOT hole */
		if (1 == tape_bot_eot)
		{
			tape_bot_eot = 0;
			set_led_status(1, 0);
			LOG(5,("tape %5.4fs: %s between BOT + LEADER\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
	}
	else
	if (offset < TAPE_LEADER + TAPE_GAP + TAPE_HOLE)
	{
		/* during BOT hole */
		if (0 == tape_bot_eot)
		{
			tape_bot_eot = 1;
			set_led_status(1, 1);
			LOG(5,("tape %5.4fs: %s found BOT\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
	}
	else
	if (offset < tape_length - TAPE_LEADER - TAPE_GAP - TAPE_HOLE)
	{
		offset -= TAPE_LEADER + TAPE_GAP + TAPE_HOLE;

		/* data area */
		if (1 == tape_bot_eot)
		{
			tape_bot_eot = 0;
			set_led_status(1, 0);
			LOG(5,("tape %5.4fs: %s data area\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
		rclk = (offset ^ 1) & 1;
		tape_bit = (offset / 2) % 8;
		tape_byte = (offset / 16) % TAPE_CHUNK;
		tape_block = offset / 16 / TAPE_CHUNK;

		if (tape_byte < TAPE_PRE_GAP)
		{
			rclk = 0;
			rdata = 0;
		}
		else
		if (tape_byte < TAPE_LEADIN)
		{
			rdata = (0x00 >> tape_bit) & 1;
			if (tape_byte != last_byte)
			{
				LOG(5,("tape %5.4fs: LEADIN $00\n", attotime_to_double(tape_time)));
				set_led_status(2, 1);
			}
		}
		else
		if (tape_byte < TAPE_HEADER)
		{
			rdata = (0xaa >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(5,("tape %5.4fs: HEADER $aa\n", attotime_to_double(tape_time)));
		}
		else
		if (tape_byte < TAPE_BLOCK)
		{
			UINT8 *ptr = memory_region(REGION_USER2) + tape_block * 256 + tape_byte - TAPE_HEADER;
			rdata = (*ptr >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(4,("tape %5.4fs: DATA(%02x) $%02x\n", attotime_to_double(tape_time), tape_byte - TAPE_HEADER, *ptr));
		}
		else
		if (tape_byte < TAPE_CRC16_MSB)
		{
			rdata = (tape_crc16_msb[tape_block] >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(4,("tape %5.4fs: CRC16 MSB $%02x\n", attotime_to_double(tape_time), tape_crc16_msb[tape_block]));
		}
		else
		if (tape_byte < TAPE_CRC16_LSB)
		{
			rdata = (tape_crc16_lsb[tape_block] >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(4,("tape %5.4fs: CRC16 LSB $%02x\n", attotime_to_double(tape_time), tape_crc16_lsb[tape_block]));
		}
		else
		if (tape_byte < TAPE_TRAILER)
		{
			rdata = (0xaa >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(4,("tape %5.4fs: TRAILER $aa\n", attotime_to_double(tape_time)));
		}
		else
		if (tape_byte < TAPE_LEADOUT)
		{
			rdata = (0x00 >> tape_bit) & 1;
			if (tape_byte != last_byte)
				LOG(4,("tape %5.4fs: LEADOUT $00\n", attotime_to_double(tape_time)));
		}
		else
		if (tape_byte < TAPE_LONGCLOCK)
		{
			if (tape_byte != last_byte)
			{
				LOG(4,("tape %5.4fs: LONG CLOCK\n", attotime_to_double(tape_time)));
				set_led_status(2, 0);
			}
			rclk = 1;
			rdata = 0;
		}
		last_byte = tape_byte;
	}
	else
	if (offset < tape_length - TAPE_LEADER - TAPE_GAP)
	{
		/* during EOT hole */
		if (0 == tape_bot_eot)
		{
			tape_bot_eot = 1;
			set_led_status(1, 1);
			LOG(5,("tape %5.4fs: %s found EOT\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
	}
	else
	if (offset < tape_length - TAPE_LEADER)
	{
		/* GAP between EOT and trailer */
		if (1 == tape_bot_eot)
		{
			tape_bot_eot = 0;
			set_led_status(1, 0);
			LOG(5,("tape %5.4fs: %s EOT and TRAILER\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
	}
	else
	{
		/* TRAILER area */
		if (0 == tape_bot_eot)
		{
			tape_bot_eot = 1;
			set_led_status(1, 1);
			LOG(5,("tape %5.4fs: %s found TRAILER\n", attotime_to_double(tape_time), dirnm(tape_dir)));
		}
		offset = tape_length - 1;
	}

	i8041_p2 = (i8041_p2 & ~0xe0) | (tape_bot_eot << 5) | (rclk << 6) | (rdata << 7);
}

#ifdef MAME_DEBUG
static void decocass_fno(offs_t offset, UINT8 data)
{
		/* 8041ENA/ and is this a FNO write (function number)? */
		if (0 == (i8041_p2 & 0x01))
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
	static UINT8 latch1;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_STAT);
		else
			data = 0xff;

		data = (BIT0(data) << 0) | (BIT1(data) << 1) | 0x7c;
		LOG(4,("%9.7f 6502-PC: %04x decocass_type1_latch_26_pass_3_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(REGION_USER1);

		if (firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			firsttime = 0;
			latch1 = 0; 	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_DATA);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(type1_inmap)) & 1) << 0) |
			(((data >> MAP1(type1_inmap)) & 1) << 1) |
			(((data >> MAP4(type1_inmap)) & 1) << 2) |
			(((data >> MAP5(type1_inmap)) & 1) << 3) |
			(((data >> MAP7(type1_inmap)) & 1) << 4);
		/* latch bits 2 and 6, pass bit 3, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(type1_outmap)) |
			((1 - ((latch1 >> MAP2(type1_inmap)) & 1)) << MAP2(type1_outmap)) |
			(((data >> MAP3(type1_inmap)) & 1)		   << MAP3(type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(type1_outmap)) |
			(((latch1 >> MAP6(type1_inmap)) & 1)	   << MAP6(type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(type1_outmap));

		LOG(3,("%9.7f 6502-PC: %04x decocass_type1_latch_26_pass_3_inv_2_r(%02x): $%02x\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));

		latch1 = save;		/* latch the data for the next A0 == 0 read */
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
	static UINT8 latch1;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_STAT);
		else
			data = 0xff;

		data = (BIT0(data) << 0) | (BIT1(data) << 1) | 0x7c;
		LOG(4,("%9.7f 6502-PC: %04x decocass_type1_pass_136_r(%02x): $%02x <- (%s %s)\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(REGION_USER1);

		if (firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			firsttime = 0;
			latch1 = 0; 	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_DATA);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(type1_inmap)) & 1) << 0) |
			(((data >> MAP2(type1_inmap)) & 1) << 1) |
			(((data >> MAP4(type1_inmap)) & 1) << 2) |
			(((data >> MAP5(type1_inmap)) & 1) << 3) |
			(((data >> MAP7(type1_inmap)) & 1) << 4);
		/* latch bits 1 and 6, pass bit 3, invert bit 1 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(type1_outmap)) |
			(((data >> MAP1(type1_inmap)) & 1)         << MAP1(type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP2(type1_outmap)) |
			(((data >> MAP3(type1_inmap)) & 1)		   << MAP3(type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(type1_outmap)) |
			(((data >> MAP6(type1_inmap)) & 1)	       << MAP6(type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(type1_outmap));

		LOG(3,("%9.7f 6502-PC: %04x decocass_type1_pass_136_r(%02x): $%02x\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));

		latch1 = save;		/* latch the data for the next A0 == 0 read */
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
	static UINT8 latch1;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_STAT);
		else
			data = 0xff;

		data = (BIT0(data) << 0) | (BIT1(data) << 1) | 0x7c;
		LOG(4,("%9.7f 6502-PC: %04x decocass_type1_latch_27_pass_3_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(REGION_USER1);

		if (firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			firsttime = 0;
			latch1 = 0; 	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_DATA);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(type1_inmap)) & 1) << 0) |
			(((data >> MAP1(type1_inmap)) & 1) << 1) |
			(((data >> MAP4(type1_inmap)) & 1) << 2) |
			(((data >> MAP5(type1_inmap)) & 1) << 3) |
			(((data >> MAP6(type1_inmap)) & 1) << 4);
		/* latch bits 2 and 7, pass bit 3, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(type1_outmap)) |
			((1 - ((latch1 >> MAP2(type1_inmap)) & 1)) << MAP2(type1_outmap)) |
			(((data >> MAP3(type1_inmap)) & 1)		   << MAP3(type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP6(type1_outmap)) |
			(((latch1 >> MAP7(type1_inmap)) & 1)	   << MAP7(type1_outmap));

		LOG(3,("%9.7f 6502-PC: %04x decocass_type1_latch_27_pass_3_inv_2_r(%02x): $%02x\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));

		latch1 = save;		/* latch the data for the next A0 == 0 read */
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
	static UINT8 latch1;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_STAT);
		else
			data = 0xff;

		data = (BIT0(data) << 0) | (BIT1(data) << 1) | 0x7c;
		LOG(4,("%9.7f 6502-PC: %04x decocass_type1_latch_26_pass_5_inv_2_r(%02x): $%02x <- (%s %s)\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(REGION_USER1);

		if (firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			firsttime = 0;
			latch1 = 0; 	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_DATA);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(type1_inmap)) & 1) << 0) |
			(((data >> MAP1(type1_inmap)) & 1) << 1) |
			(((data >> MAP3(type1_inmap)) & 1) << 2) |
			(((data >> MAP4(type1_inmap)) & 1) << 3) |
			(((data >> MAP7(type1_inmap)) & 1) << 4);
		/* latch bits 2 and 6, pass bit 5, invert bit 2 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP1(type1_outmap)) |
			((1 - ((latch1 >> MAP2(type1_inmap)) & 1)) << MAP2(type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP3(type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP4(type1_outmap)) |
			(((data >> MAP5(type1_inmap)) & 1)		   << MAP5(type1_outmap)) |
			(((latch1 >> MAP6(type1_inmap)) & 1)		   << MAP6(type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(type1_outmap));

		LOG(3,("%9.7f 6502-PC: %04x decocass_type1_latch_26_pass_5_inv_2_r(%02x): $%02x\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));

		latch1 = save;		/* latch the data for the next A0 == 0 read */
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
	static UINT8 latch1;
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_STAT);
		else
			data = 0xff;

		data = (BIT0(data) << 0) | (BIT1(data) << 1) | 0x7c;
		LOG(4,("%9.7f 6502-PC: %04x decocass_type1_latch_16_pass_3_inv_1_r(%02x): $%02x <- (%s %s)\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = memory_region(REGION_USER1);

		if (firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			firsttime = 0;
			latch1 = 0; 	 /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, I8X41_DATA);
		else
			data = 0xff;

		save = data;	/* save the unmodifed data for the latch */

		promaddr =
			(((data >> MAP0(type1_inmap)) & 1) << 0) |
			(((data >> MAP2(type1_inmap)) & 1) << 1) |
			(((data >> MAP4(type1_inmap)) & 1) << 2) |
			(((data >> MAP5(type1_inmap)) & 1) << 3) |
			(((data >> MAP7(type1_inmap)) & 1) << 4);
		/* latch bits 1 and 6, pass bit 3, invert bit 1 */
		data =
			(((prom[promaddr] >> 0) & 1)			   << MAP0(type1_outmap)) |
			((1 - ((latch1 >> MAP1(type1_inmap)) & 1)) << MAP1(type1_outmap)) |
			(((prom[promaddr] >> 1) & 1)			   << MAP2(type1_outmap)) |
			(((data >> MAP3(type1_inmap)) & 1)		   << MAP3(type1_outmap)) |
			(((prom[promaddr] >> 2) & 1)			   << MAP4(type1_outmap)) |
			(((prom[promaddr] >> 3) & 1)			   << MAP5(type1_outmap)) |
			(((latch1 >> MAP6(type1_inmap)) & 1)	   << MAP6(type1_outmap)) |
			(((prom[promaddr] >> 4) & 1)			   << MAP7(type1_outmap));

		LOG(3,("%9.7f 6502-PC: %04x decocass_type1_latch_16_pass_3_inv_1_r(%02x): $%02x\n",
			attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));

		latch1 = save;		/* latch the data for the next A0 == 0 read */
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
	UINT8 data;

	if (1 == type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			UINT8 *prom = memory_region(REGION_USER1);
			data = prom[256 * type2_d2_latch + type2_promaddr];
			LOG(3,("%9.7f 6502-PC: %04x decocass_type2_r(%02x): $%02x <- prom[%03x]\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, 256 * type2_d2_latch + type2_promaddr));
		}
		else
		{
			data = 0xff;	/* floating input? */
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
			data = cpunum_get_reg(2, offset & 1 ? I8X41_STAT : I8X41_DATA);
		else
			data = offset & 0xff;

		LOG(3,("%9.7f 6502-PC: %04x decocass_type2_r(%02x): $%02x <- 8041-%s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "STATUS" : "DATA"));
	}
	return data;
}

static WRITE8_HANDLER( decocass_type2_w )
{
	if (1 == type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			LOG(4,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM+D2 latch", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
		else
		{
			type2_promaddr = data;
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM addr $%02x\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, type2_promaddr));
			return;
		}
	}
	else
	{
		LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s ", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "8041-CMND" : "8041 DATA"));
	}
	if (1 == (offset & 1))
	{
		if (0xc0 == (data & 0xf0))
		{
			type2_xx_latch = 1;
			type2_d2_latch = (data & 0x04) ? 1 : 0;
			LOG(3,("PROM:%s D2:%d", type2_xx_latch ? "on" : "off", type2_d2_latch));
		}
	}
	cpunum_set_reg(2, offset & 1 ? I8X41_CMND : I8X41_DATA, data);

#ifdef MAME_DEBUG
	decocass_fno(offset, data);
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
	UINT8 data, save;

	if (1 == (offset & 1))
	{
		if (1 == type3_pal_19)
		{
			UINT8 *prom = memory_region(REGION_USER1);
			data = prom[type3_ctrs];
			LOG(3,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x <- prom[$%03x]\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, type3_ctrs));
			if (++type3_ctrs == 4096)
				type3_ctrs = 0;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = cpunum_get_reg(2, I8X41_STAT);
				LOG(4,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x <- 8041 STATUS\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
			}
		}
	}
	else
	{
		if (1 == type3_pal_19)
		{
			save = data = 0xff;    /* open data bus? */
			LOG(3,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				save = cpunum_get_reg(2, I8X41_DATA);
				switch (type3_swap)
				{
				case TYPE3_SWAP_01:
					data =
						(BIT1(save) << 0) |
						(type3_d0_latch << 1) |
						(BIT2(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_12:
					data =
						(type3_d0_latch << 0) |
						(BIT2(save) << 1) |
						(BIT1(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_13:
					data =
						(type3_d0_latch << 0) |
						(BIT3(save) << 1) |
						(BIT2(save) << 2) |
						(BIT1(save) << 3) |
						(BIT4(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_24:
					data =
						(type3_d0_latch << 0) |
						(BIT1(save) << 1) |
						(BIT4(save) << 2) |
						(BIT3(save) << 3) |
						(BIT2(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_25:
					data =
						(type3_d0_latch << 0) |
						(BIT1(save) << 1) |
						(BIT5(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT2(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_34_0:
					data =
						(type3_d0_latch << 0) |
						(BIT1(save) << 1) |
						(BIT2(save) << 2) |
						(BIT3(save) << 4) |
						(BIT4(save) << 3) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_34_7:
					data =
						(BIT7(save) << 0) |
						(BIT1(save) << 1) |
						(BIT2(save) << 2) |
						(BIT4(save) << 3) |
						(BIT3(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(type3_d0_latch << 7);
					break;
				case TYPE3_SWAP_23_56:
					data =
						(type3_d0_latch << 0) |
						(BIT1(save) << 1) |
						(BIT3(save) << 2) |
						(BIT2(save) << 3) |
						(BIT4(save) << 4) |
						(BIT6(save) << 5) |
						(BIT5(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_56:
					data =
						type3_d0_latch |
						(BIT1(save) << 1) |
						(BIT2(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT6(save) << 5) |
						(BIT5(save) << 6) |
						(BIT7(save) << 7);
					break;
				case TYPE3_SWAP_67:
					data =
						type3_d0_latch |
						(BIT1(save) << 1) |
						(BIT2(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT5(save) << 5) |
						(BIT7(save) << 6) |
						(BIT6(save) << 7);
					break;
				default:
					data =
						type3_d0_latch |
						(BIT1(save) << 1) |
						(BIT2(save) << 2) |
						(BIT3(save) << 3) |
						(BIT4(save) << 4) |
						(BIT5(save) << 5) |
						(BIT6(save) << 6) |
						(BIT7(save) << 7);
				}
				type3_d0_latch = save & 1;
				LOG(3,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- 8041-DATA\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				save = 0xff;	/* open data bus? */
				data =
					type3_d0_latch |
					(BIT1(save) << 1) |
					(BIT2(save) << 2) |
					(BIT3(save) << 3) |
					(BIT4(save) << 4) |
					(BIT5(save) << 5) |
					(BIT6(save) << 7) |
					(BIT7(save) << 6);
				LOG(3,("%9.7f 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
				type3_d0_latch = save & 1;
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type3_w )
{
	if (1 == (offset & 1))
	{
		if (1 == type3_pal_19)
		{
			type3_ctrs = data << 4;
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, "LDCTRS"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			type3_pal_19 = 1;
	}
	else
	{
		if (1 == type3_pal_19)
		{
			/* write nowhere?? */
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	cpunum_set_reg(2, offset & 1 ? I8X41_CMND : I8X41_DATA, data);
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
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = cpunum_get_reg(2, I8X41_STAT);
			LOG(4,("%9.7f 6502-PC: %04x decocass_type4_r(%02x): $%02x <- 8041 STATUS\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%9.7f 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
	}
	else
	{
		if (type4_latch)
		{
			UINT8 *prom = memory_region(REGION_USER1);

			data = prom[type4_ctrs];
			LOG(3,("%9.7f 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- PROM[%04x]\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.', type4_ctrs));
			type4_ctrs = (type4_ctrs+1) & 0x7fff;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = cpunum_get_reg(2, I8X41_DATA);
				LOG(3,("%9.7f 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%9.7f 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type4_w )
{
	if (1 == (offset & 1))
	{
		if (1 == type4_latch)
		{
			type4_ctrs = (type4_ctrs & 0x00ff) | ((data & 0x7f) << 8);
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS MSB (%04x)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, type4_ctrs));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
		{
			type4_latch = 1;
		}
	}
	else
	{
		if (type4_latch)
		{
			type4_ctrs = (type4_ctrs & 0xff00) | data;
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS LSB (%04x)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, type4_ctrs));
			return;
		}
	}
	LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	cpunum_set_reg(2, offset & 1 ? I8X41_CMND : I8X41_DATA, data);
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
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = cpunum_get_reg(2, I8X41_STAT);
			LOG(4,("%9.7f 6502-PC: %04x decocass_type5_r(%02x): $%02x <- 8041 STATUS\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%9.7f 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
	}
	else
	{
		if (type5_latch)
		{
			data = 0x55;	/* Only a fixed value? It looks like this is all we need to do */
			LOG(3,("%9.7f 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- fixed value???\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = cpunum_get_reg(2, I8X41_DATA);
				LOG(3,("%9.7f 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;	/* open data bus? */
				LOG(4,("%9.7f 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
			}
		}
	}

	return data;
}

static WRITE8_HANDLER( decocass_type5_w )
{
	if (1 == (offset & 1))
	{
		if (1 == type5_latch)
		{
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, "latch #2??"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			type5_latch = 1;
	}
	else
	{
		if (type5_latch)
		{
			/* write nowhere?? */
			LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	cpunum_set_reg(2, offset & 1 ? I8X41_CMND : I8X41_DATA, data);
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
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = cpunum_get_reg(2, I8X41_STAT);
			LOG(4,("%9.7f 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- 8041 STATUS\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%9.7f 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = cpunum_get_reg(2, I8X41_DATA);
			LOG(3,("%9.7f 6502-PC: %04x decocass_nodong_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			data = 0xff;	/* open data bus? */
			LOG(4,("%9.7f 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
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
	UINT8 data;

	/* E5x2-E5x3 and mirrors */
	if (2 == (offset & E5XX_MASK))
	{
		data =
			(BIT7(i8041_p1) 	  << 0) |	/* D0 = P17 - REQ/ */
			(BIT0(i8041_p2) 	  << 1) |	/* D1 = P20 - FNO/ */
			(BIT1(i8041_p2) 	  << 2) |	/* D2 = P21 - EOT/ */
			(BIT2(i8041_p2) 	  << 3) |	/* D3 = P22 - ERR/ */
			((tape_bot_eot) 	  << 4) |	/* D4 = BOT/EOT (direct from drive) */
			(1					  << 5) |	/* D5 floating input */
			(1					  << 6) |	/* D6 floating input */
			((1 - tape_present)   << 7);	/* D7 = cassette present */

		LOG(4,("%9.7f 6502-PC: %04x decocass_e5xx_r(%02x): $%02x <- STATUS (%s%s%s%s%s%s%s%s)\n",
			attotime_to_double(timer_get_time()),
			activecpu_get_previouspc(),
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
		if (decocass_dongle_r)
			data = (*decocass_dongle_r)(machine, offset);
		else
			data = 0xff;
	}
	return data;
}

WRITE8_HANDLER( decocass_e5xx_w )
{
	if (decocass_dongle_w)
	{
		(*decocass_dongle_w)(machine, offset, data);
		return;
	}

	if (0 == (offset & E5XX_MASK))
	{
		LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
		cpunum_set_reg(2, offset & 1 ? I8X41_CMND : I8X41_DATA, data);
#ifdef MAME_DEBUG
		decocass_fno(offset, data);
#endif
	}
	else
	{
		LOG(3,("%9.7f 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> dongle\n", attotime_to_double(timer_get_time()), activecpu_get_previouspc(), offset, data));
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
	de0091_enable = data & 1;
	memory_set_bank(1, data & 1);
	/* Perhaps the second row of ROMs is enabled by another bit.
     * There is no way to verify this yet, so for now just look
     * at bit 0 to enable the daughter board at reads between
     * 0x6000 and 0xafff.
     */
}

WRITE8_HANDLER( decocass_de0091_w )
{
	/* don't allow writes to the ROMs */
	if (!de0091_enable)
		decocass_charram_w(machine, offset, data);
}

/***************************************************************************
 *
 *  state save setup
 *
 ***************************************************************************/
static STATE_POSTLOAD( decocass_state_save_postload )
{
#if 0
	/* fix me - this won't work anymore */
	int A;
	UINT8 *mem = memory_region(REGION_CPU1);
	int diff = memory_region_length(REGION_CPU1) / 2;

	memory_set_opcode_base(0, mem + diff);

	for (A = 0;A < 0x10000; A++)
		decocass_w(A, mem[A]);
	/* restart the timer if the tape was playing */
	if (0 != tape_dir)
		timer_adjust_oneshot(tape_timer, attotime_never, 0);
#endif
}

/* To be called once from driver_init, i.e. decocass_init */
void decocass_machine_state_save_init(running_machine *machine)
{
	state_save_register_postload(machine, decocass_state_save_postload, NULL);
	state_save_register_global(tape_dir);
	state_save_register_global(tape_speed);
	state_save_register_global(tape_time0.seconds);
	state_save_register_global(tape_time0.attoseconds);
	state_save_register_global(firsttime);
	state_save_register_global(tape_present);
	state_save_register_global(tape_blocks);
	state_save_register_global(tape_length);
	state_save_register_global(tape_bot_eot);
	state_save_register_global(crc16_lsb);
	state_save_register_global(crc16_msb);
	state_save_register_global_array(tape_crc16_lsb);
	state_save_register_global_array(tape_crc16_msb);
	state_save_register_global(decocass_reset);
	state_save_register_global(i8041_p1);
	state_save_register_global(i8041_p2);
	state_save_register_global(de0091_enable);
	state_save_register_global(type1_inmap);
	state_save_register_global(type1_outmap);
	state_save_register_global(type2_d2_latch);
	state_save_register_global(type2_xx_latch);
	state_save_register_global(type2_promaddr);
	state_save_register_global(type3_ctrs);
	state_save_register_global(type3_d0_latch);
	state_save_register_global(type3_pal_19);
	state_save_register_global(type3_swap);
	state_save_register_global(type4_ctrs);
	state_save_register_global(type4_latch);
	state_save_register_global(type5_latch);
	state_save_register_global(decocass_sound_ack);
}

/***************************************************************************
 *
 *  init machine functions (select dongle and determine tape image size)
 *
 ***************************************************************************/

static void decocass_init_common(void)
{
	UINT8 *image = memory_region(REGION_USER2);
	int i, offs;

	tape_dir = 0;
	tape_speed = 0;
	tape_timer = timer_alloc(NULL, NULL);

	firsttime = 1;
	tape_present = 1;
	tape_blocks = 0;
	for (i = memory_region_length(REGION_USER2) / 256 - 1; !tape_blocks && i > 0; i--)
		for (offs = 256 * i; !tape_blocks && offs < 256 * i + 256; offs++)
			if (image[offs])
				tape_blocks = i+1;
	for (i = 0; i < tape_blocks; i++)
	{
		crc16_lsb = 0;
		crc16_msb = 0;
		for (offs = 256 * i; offs < 256 * i + 256; offs++)
		{
			tape_crc16(image[offs] << 7);
			tape_crc16(image[offs] << 6);
			tape_crc16(image[offs] << 5);
			tape_crc16(image[offs] << 4);
			tape_crc16(image[offs] << 3);
			tape_crc16(image[offs] << 2);
			tape_crc16(image[offs] << 1);
			tape_crc16(image[offs] << 0);
		}
		tape_crc16_lsb[i] = crc16_lsb;
		tape_crc16_msb[i] = crc16_msb;
	}

	tape_length = tape_blocks * TAPE_CHUNK * 8 * 2 + 2 * (TAPE_LEADER + TAPE_GAP + TAPE_HOLE);
	tape_time0 = attotime_mul(ATTOTIME_IN_HZ(TAPE_CLOCKRATE), TAPE_LEADER + TAPE_GAP - TAPE_HOLE);
	LOG(0,("tape: %d blocks\n", tape_blocks));
	tape_bot_eot = 0;

	decocass_dongle_r = NULL;
	decocass_dongle_w = NULL;

	decocass_reset = 0;
	i8041_p1 = 0xff;
	i8041_p2 = 0xff;

	type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);

	type2_d2_latch = 0;
	type2_xx_latch = 0;
	type2_promaddr = 0;

	type3_ctrs = 0;
	type3_d0_latch = 0;
	type3_pal_19 = 0;
	type3_swap = 0;

	memset(decocass_quadrature_decoder, 0, sizeof(decocass_quadrature_decoder));
	decocass_sound_ack = 0;
	decocass_sound_timer = timer_alloc(decocass_sound_nmi_pulse, NULL);
}

MACHINE_RESET( decocass )
{
	decocass_init_common();
}

MACHINE_RESET( ctsttape )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	decocass_dongle_r = decocass_type1_pass_136_r;
}

MACHINE_RESET( chwy )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	decocass_dongle_r = decocass_type1_latch_27_pass_3_inv_2_r;
}

MACHINE_RESET( clocknch )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 flip 2-3)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(0,1,3,2,4,5,6,7);
	type1_outmap = MAKE_MAP(0,1,3,2,4,5,6,7);
}

MACHINE_RESET( ctisland )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 flip 0-2)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(2,1,0,3,4,5,6,7);
	type1_outmap = MAKE_MAP(2,1,0,3,4,5,6,7);
}

MACHINE_RESET( csuperas )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 flip 4-5)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(0,1,2,3,5,4,6,7);
	type1_outmap = MAKE_MAP(0,1,2,3,5,4,6,7);
}

MACHINE_RESET( castfant )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	decocass_dongle_r = decocass_type1_latch_16_pass_3_inv_1_r;
}

MACHINE_RESET( cluckypo )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 flip 1-3)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(0,3,2,1,4,5,6,7);
	type1_outmap = MAKE_MAP(0,3,2,1,4,5,6,7);
}

MACHINE_RESET( cterrani )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 straight)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

MACHINE_RESET( cexplore )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_5_inv_2_r;
}

MACHINE_RESET( cprogolf )
{
	decocass_init_common();
	LOG(0,("dongle type #1 (DE-0061 flip 0-1)\n"));
	decocass_dongle_r = decocass_type1_latch_26_pass_3_inv_2_r;
	type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET( cmissnx )
{
	decocass_init_common();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	decocass_dongle_r = decocass_type2_r;
	decocass_dongle_w = decocass_type2_w;
}

MACHINE_RESET( cdiscon1 )
{
	decocass_init_common();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	decocass_dongle_r = decocass_type2_r;
	decocass_dongle_w = decocass_type2_w;
}

MACHINE_RESET( cptennis )
{
	decocass_init_common();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	decocass_dongle_r = decocass_type2_r;
	decocass_dongle_w = decocass_type2_w;
}

MACHINE_RESET( ctornado )
{
	decocass_init_common();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	decocass_dongle_r = decocass_type2_r;
	decocass_dongle_w = decocass_type2_w;
}

MACHINE_RESET( cbnj )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( cburnrub )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( cbtime )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_12;
}

MACHINE_RESET( cgraplop )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_56;
}

MACHINE_RESET( cgraplp2 )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET( clapapa )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_34_7;
}

MACHINE_RESET( cfghtice )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_25;
}

MACHINE_RESET( cprobowl )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_34_0;
}

MACHINE_RESET( cnightst )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_13;
}

MACHINE_RESET( cprosocc )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_24;
}

MACHINE_RESET( cppicf )
{
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_01;
}

MACHINE_RESET( cscrtry )
{
	decocass_init_common();
	LOG(0,("dongle type #4 (32K ROM)\n"));
	decocass_dongle_r = decocass_type4_r;
	decocass_dongle_w = decocass_type4_w;
}

MACHINE_RESET( cbdash )
{
	decocass_init_common();
	LOG(0,("dongle type #5 (NOP)\n"));
	decocass_dongle_r = decocass_type5_r;
	decocass_dongle_w = decocass_type5_w;
}

MACHINE_RESET( cflyball )
{
	decocass_init_common();
	LOG(0,("no dongle\n"));
	decocass_dongle_r = decocass_nodong_r;
}

MACHINE_RESET( czeroize )
{
	UINT8 *mem = memory_region(REGION_USER1);
	decocass_init_common();
	LOG(0,("dongle type #3 (PAL)\n"));
	decocass_dongle_r = decocass_type3_r;
	decocass_dongle_w = decocass_type3_w;
	type3_swap = TYPE3_SWAP_23_56;

	/*
     * FIXME: remove if the original ROM is available.
     * The Zeroize 6502 code at 0x3707 issues LODCTRS with 0x8a,
     * and expects to read 0x18 from 0x08a0 ff. within 7 bytes.
     * This hack seems to be sufficient to get around
     * the missing dongle ROM contents and play the game.
     */
    memset(mem,0x00,0x1000);
	mem[0x08a0] = 0x18;
}

/***************************************************************************
 *
 *  8041 port handlers
 *
 ***************************************************************************/

static void tape_stop(void)
{
	/* remember time */
	tape_time0 = decocass_adjust_tape_time(tape_time0);

	timer_adjust_oneshot(tape_timer, attotime_never, 0);
}


WRITE8_HANDLER( i8041_p1_w )
{
	static int i8041_p1_old;

	if (data != i8041_p1_old)
	{
		LOG(4,("%9.7f 8041-PC: %03x i8041_p1_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_to_double(timer_get_time()),
			activecpu_get_previouspc(),
			data,
			data & 0x01 ? "" : "DATA-WRT",
			data & 0x02 ? "" : " DATA-CLK",
			data & 0x04 ? "" : " FAST",
			data & 0x08 ? "" : " BIT3",
			data & 0x10 ? "" : " REW",
			data & 0x20 ? "" : " FWD",
			data & 0x40 ? "" : " WREN",
			data & 0x80 ? "" : " REQ"));
		i8041_p1_old = data;
	}

	/* change in REW signal ? */
	if ((data ^ i8041_p1) & 0x10)
	{
		tape_stop();
		if (0 == (data & 0x10))
		{
			LOG(2,("tape %5.4fs: rewind\n", attotime_to_double(tape_time0)));
			tape_dir = -1;
			timer_adjust_oneshot(tape_timer, attotime_never, 0);
			set_led_status(0, 1);
		}
		else
		{
			tape_dir = 0;
			tape_speed = 0;
			LOG(2,("tape %5.4fs: stopped\n", attotime_to_double(tape_time0)));
#if TAPE_UI_DISPLAY
			popmessage("   [%05.1fs]   ", attotime_to_double(tape_time0));
#endif
			set_led_status(0, 0);
		}
	}

	/* change in FWD signal ? */
	if ((data ^ i8041_p1) & 0x20)
	{
		tape_stop();
		if (0 == (data & 0x20))
		{
			LOG(2,("tape %5.4fs: forward\n", attotime_to_double(tape_time0)));
			tape_dir = +1;
			timer_adjust_oneshot(tape_timer, attotime_never, 0);
			set_led_status(0, 1);
		}
		else
		{
			tape_dir = 0;
			tape_speed = 0;
			LOG(2,("tape %5.4fs: stopped\n", attotime_to_double(tape_time0)));
#if TAPE_UI_DISPLAY
			popmessage("   [%05.1fs]   ", attotime_to_double(tape_time0));
#endif
			set_led_status(0, 0);
		}
	}

	/* change in FAST signal ? */
	if (tape_timer && (data ^ i8041_p1) & 0x04)
	{
		tape_stop();
		tape_speed = (0 == (data & 0x04)) ? 1 : 0;

		if (tape_dir < 0)
		{
			LOG(2,("tape: fast rewind %s\n", (0 == (data & 0x04)) ? "on" : "off"));
			tape_dir = (tape_speed) ? -7 : -1;
			timer_adjust_oneshot(tape_timer, attotime_never, 0);
		}
		else
		if (tape_dir > 0)
		{
			LOG(2,("tape: fast forward %s\n", (0 == (data & 0x04)) ? "on" : "off"));
			tape_dir = (tape_speed) ? +7 : +1;
			timer_adjust_oneshot(tape_timer, attotime_never, 0);
		}
	}

	i8041_p1 = data;
}

READ8_HANDLER( i8041_p1_r )
{
	UINT8 data = i8041_p1;
	static int i8041_p1_old;

	if (data != i8041_p1_old)
	{
		LOG(4,("%9.7f 8041-PC: %03x i8041_p1_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_to_double(timer_get_time()),
			activecpu_get_previouspc(),
			data,
			data & 0x01 ? "" : "DATA-WRT",
			data & 0x02 ? "" : " DATA-CLK",
			data & 0x04 ? "" : " FAST",
			data & 0x08 ? "" : " BIT3",
			data & 0x10 ? "" : " REW",
			data & 0x20 ? "" : " FWD",
			data & 0x40 ? "" : " WREN",
			data & 0x80 ? "" : " REQ"));
		i8041_p1_old = data;
	}
	return data;
}

WRITE8_HANDLER( i8041_p2_w )
{
	static int i8041_p2_old;

	if (data != i8041_p2_old)
	{
		LOG(4,("%9.7f 8041-PC: %03x i8041_p2_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_to_double(timer_get_time()),
			activecpu_get_previouspc(),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		i8041_p2_old = data;
	}
	i8041_p2 = data;
}

READ8_HANDLER( i8041_p2_r )
{
	UINT8 data;
	static int i8041_p2_old;

	tape_update();

	data = i8041_p2;

	if (data != i8041_p2_old)
	{
		LOG(4,("%9.7f 8041-PC: %03x i8041_p2_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			attotime_to_double(timer_get_time()),
			activecpu_get_previouspc(),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		i8041_p2_old = data;
	}
	return data;
}


