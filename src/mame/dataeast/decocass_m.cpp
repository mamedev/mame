// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "decocass.h"
#include "decocass_tape.h"

/* dongle type #1: jumpers C and D assignments */
#define MAKE_MAP(m0,m1,m2,m3,m4,m5,m6,m7)   \
	((uint32_t)(m0)) | \
	((uint32_t)(m1) << 3) | \
	((uint32_t)(m2) << 6) | \
	((uint32_t)(m3) << 9) | \
	((uint32_t)(m4) << 12) | \
	((uint32_t)(m5) << 15) | \
	((uint32_t)(m6) << 18) | \
	((uint32_t)(m7) << 21)


#define T1MAP(x, m) (((m)>>(x*3))&7)



enum {
	TYPE3_SWAP_01,
	TYPE3_SWAP_12,
	TYPE3_SWAP_13,
	TYPE3_SWAP_24,
	TYPE3_SWAP_25,
	TYPE3_SWAP_34_0,
	TYPE3_SWAP_34_7,
	TYPE3_SWAP_45,
	TYPE3_SWAP_23_56,
	TYPE3_SWAP_56,
	TYPE3_SWAP_67
};


void decocass_state::decocass_coin_counter_w(uint8_t data)
{
}

uint8_t decocass_state::decocass_sound_command_main_r()
{
	// cgraplop2 needs to read something here or it will reset when you coin-up
	//  could do with further investigation
	return 0xc0;
}

void decocass_state::decocass_sound_command_w(uint8_t data)
{
	LOG(2,("CPU %s sound command -> $%02x\n", m_maincpu->tag(), data));
	m_soundlatch->write(data);
	m_sound_ack |= 0x80;
	/* remove snd cpu data ack bit. i don't see it in the schems, but... */
	m_sound_ack &= ~0x40;
	m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}

uint8_t decocass_state::decocass_sound_data_r()
{
	uint8_t data = m_soundlatch2->read();
	LOG(2,("CPU %s sound data    <- $%02x\n", m_maincpu->tag(), data));
	return data;
}

uint8_t decocass_state::decocass_sound_ack_r()
{
	uint8_t data = m_sound_ack;   /* D6+D7 */
	LOG(4,("CPU %s sound ack     <- $%02x\n", m_maincpu->tag(), data));
	return data;
}

void decocass_state::decocass_sound_data_w(uint8_t data)
{
	LOG(2,("CPU %s sound data    -> $%02x\n", m_audiocpu->tag(), data));
	m_soundlatch2->write(data);
	m_sound_ack |= 0x40;
}

uint8_t decocass_state::decocass_sound_command_r()
{
	uint8_t data = m_soundlatch->read();
	LOG(4,("CPU %s sound command <- $%02x\n", m_audiocpu->tag(), data));
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	m_sound_ack &= ~0x80;
	return data;
}

TIMER_DEVICE_CALLBACK_MEMBER(decocass_state::decocass_audio_nmi_gen)
{
	int scanline = param;
	m_audio_nmi_state = scanline & 8;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

void decocass_state::decocass_sound_nmi_enable_w(uint8_t data)
{
	m_audio_nmi_enabled = 1;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t decocass_state::decocass_sound_nmi_enable_r()
{
	m_audio_nmi_enabled = 1;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	return 0xff;
}

uint8_t decocass_state::decocass_sound_data_ack_reset_r()
{
	uint8_t data = 0xff;
	LOG(2,("CPU %s sound ack rst <- $%02x\n", m_audiocpu->tag(), data));
	m_sound_ack &= ~0x40;
	return data;
}

void decocass_state::decocass_sound_data_ack_reset_w(uint8_t data)
{
	LOG(2,("CPU %s sound ack rst -> $%02x\n", m_audiocpu->tag(), data));
	m_sound_ack &= ~0x40;
}

void decocass_state::decocass_nmi_reset_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE );
}

void decocass_state::decocass_quadrature_decoder_reset_w(uint8_t data)
{
	/* just latch the analog controls here */
	m_quadrature_decoder[0] = ioport("AN0")->read();
	m_quadrature_decoder[1] = ioport("AN1")->read();
	m_quadrature_decoder[2] = ioport("AN2")->read();
	m_quadrature_decoder[3] = ioport("AN3")->read();
}

void decocass_state::decocass_adc_w(uint8_t data)
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
uint8_t decocass_state::decocass_input_r(offs_t offset)
{
	uint8_t data = 0xff;
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };

	switch (offset & 7)
	{
	case 0: case 1: case 2:
		data = machine().root_device().ioport(portnames[offset & 7])->read();
		break;
	case 3: case 4: case 5: case 6:
		data = m_quadrature_decoder[(offset & 7) - 3];
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

#define E5XX_MASK   0x02    /* use 0x0e for old style board */


void decocass_state::decocass_reset_w(offs_t offset, uint8_t data)
{
	LOG(1,("%10s 6502-PC: %04x decocass_reset_w(%02x): $%02x\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
	m_decocass_reset = data;

	/* CPU #1 active high reset */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, data & 0x01);

	/* on reset also disable audio NMI */
	if (data & 1)
	{
		m_audio_nmi_enabled = 0;
		m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	}

	/* 8041 active low reset */
	m_mcu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
}


#ifdef MAME_DEBUG
void decocass_state::decocass_fno( offs_t offset, uint8_t data )
{
	/* 8041ENA/ and is this a FNO write (function number)? */
	if (0 == (m_i8041_p2 & 0x01))
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


uint8_t decocass_type1_state::decocass_type1_r(offs_t offset)
{
	if (!m_type1_map)
		return 0x00;

	uint8_t data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = m_mcu->upi41_master_r(1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_r(%02x): $%02x <- (%s %s)\n",
			machine().time().as_string(6), m_maincpu->pcbase(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		uint8_t save;
		uint8_t *prom = machine().root_device().memregion("dongle")->base();

		if (m_firsttime)
		{
			LOG(3,("prom data:\n"));
			for (promaddr = 0; promaddr < 32; promaddr++)
			{
				if (promaddr % 8 == 0)
					LOG(3,("  %02x:", promaddr));
				LOG(3,(" %02x%s", prom[promaddr], (promaddr % 8) == 7 ? "\n" : ""));
			}
			m_firsttime = 0;
			m_latch1 = 0;    /* reset latch (??) */
		}

		if (0 == (offset & E5XX_MASK))
			data = m_mcu->upi41_master_r(0);
		else
			data = 0xff;

		save = data;    /* save the unmodified data for the latch */

		promaddr = 0;
		int promshift = 0;

		for (int i=0;i<8;i++)
		{
			if (m_type1_map[i] == T1PROM) { promaddr |= (((data >> T1MAP(i,m_type1_inmap)) & 1) << promshift); promshift++; }
		}

		if (promshift!=5)
			printf("promshift != 5? (you specified more/less than 5 prom source bits)");

		data = 0;
		promshift = 0;

		for (int i=0;i<8;i++)
		{
			if (m_type1_map[i] == T1PROM)     { data |= (((prom[promaddr] >> promshift) & 1)               << T1MAP(i,m_type1_outmap)); promshift++; }
			if (m_type1_map[i] == T1LATCHINV) { data |= ((1 - ((m_latch1 >> T1MAP(i,m_type1_inmap)) & 1)) << T1MAP(i,m_type1_outmap)); }
			if (m_type1_map[i] == T1LATCH)    { data |= (((m_latch1 >> T1MAP(i,m_type1_inmap)) & 1)    << T1MAP(i,m_type1_outmap)); }
			if (m_type1_map[i] == T1DIRECT)   { data |= (((save >> T1MAP(i,m_type1_inmap)) & 1)        << T1MAP(i,m_type1_outmap)); }
		}

		LOG(3,("%10s 6502-PC: %04x decocass_type1_r(%02x): $%02x\n",
			machine().time().as_string(6), m_maincpu->pcbase(), offset, data));

		m_latch1 = save;        /* latch the data for the next A0 == 0 read */
	}
	return data;
}

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Terranian
 *  - Super Astro Fighter
 *  - Lock 'n Chase
 *  - Pro Golf
 *  - Lucky Poker
 *  - Treasure Island
 *
 * Latch bits 2 and 6, pass bit 3, invert bit 2.
 * Lookup PROM DE-0061 using bits 0, 1, 4, 5, and 7 as the
 * address bits; take PROM data 0-4 as data bits 0, 1, 4, 5, and 7.
 *
 ***************************************************************************/

static uint8_t type1_latch_26_pass_3_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1DIRECT,T1PROM, T1PROM,T1LATCH,T1PROM };

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061?)
 *  - Treasure Island (Region D)
 *
 ***************************************************************************/

static uint8_t type1_latch_ctisland3[8] = { T1LATCHINV,T1PROM,T1PROM,T1DIRECT,T1PROM, T1PROM,T1LATCH,T1PROM };

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Test Tape
 *
 * Pass bits 1, 3, and 6. Lookup PROM DE-0061 using bits 0, 2, 4, 5, and 7
 * as the address bits; take PROM data 0-4 as data bits 0, 2, 4, 5, and 7.
 *
 ***************************************************************************/

static uint8_t type1_pass_136_table[8] ={ T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM,T1PROM,T1DIRECT,T1PROM };

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Manhattan
 *
 * Input bits that are passed uninverted = $54 (3 true bits)
 * Input bits that are passed inverted   = $00 (0 inverted bits)
 * Remaining bits for addressing PROM    = $AB (5 bits)
 *
 ***************************************************************************/

static uint8_t type1_latch_xab_pass_x54_table[8] = { T1PROM,T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM };

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

static uint8_t type1_latch_27_pass_3_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1DIRECT,T1PROM,T1PROM,T1PROM,T1LATCH };

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

static uint8_t type1_latch_26_pass_5_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1PROM,T1PROM,T1DIRECT,T1LATCH,T1PROM };

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

static uint8_t type1_latch_16_pass_3_inv_1_table[8] = { T1PROM,T1LATCHINV,T1PROM,T1DIRECT,T1PROM,T1PROM,T1LATCH,T1PROM };

/***************************************************************************
 *
 *  TYPE1 DONGLE DP-1100/DP-2100 map for Ocean to Ocean
 *
 * Latched bits                          = $44 (2 latch bits)
 * Input bits that are passed uninverted = $10 (1 true bits)
 * Input bits that are passed inverted   = $00 (0 inverted bits)
 * Remaining bits for addressing PROM    = $AB (5 bits)
 * Latched bit #0:
 * Input bit position  = 2
 * Output bit position = 2
 * Type                = Inverting latch
 * Latched bit #1:
 * Input bit position  = 6
 * Output bit position = 6
 * Type                = Non-inverting latch
 *
 ***************************************************************************/

static uint8_t type1_map1100[8] = { T1PROM,T1PROM,T1LATCHINV,T1PROM,T1DIRECT,T1PROM,T1LATCH,T1PROM };

MACHINE_RESET_MEMBER(decocass_type1_state,cocean1a) /* 10 */
{
	machine_reset();
	LOG(0,("dongle type #1 (DP-1100 map)\n"));
	m_type1_map = type1_map1100;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

 /***************************************************************************
 *
 *  TYPE1 DONGLE DP-1120/DP-2120 map for Flash Boy/The Deco Kid
 *
 * Latched bits                          = $24 (2 latch bits)
 * Input bits that are passed uninverted = $08 (1 true bits)
 * Input bits that are passed inverted   = $00 (0 inverted bits)
 * Remaining bits for addressing PROM    = $D3 (5 bits)
 * Latched bit #0:
 * Input bit position  = 2
 * Output bit position = 2
 * Type                = Inverting latch
 * Latched bit #1:
 * Input bit position  = 5
 * Output bit position = 5
 * Type                = Non-inverting latch
 *
 ***************************************************************************/

static uint8_t type1_map1120[8] = { T1PROM,T1PROM,T1LATCHINV,T1DIRECT,T1PROM,T1LATCH,T1PROM,T1PROM };


MACHINE_RESET_MEMBER(decocass_type1_state,cfboy0a1) /* 12 */
{
	machine_reset();
	LOG(0,("dongle type #1 (DP-1120 map)\n"));
	m_type1_map = type1_map1120;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

/*

TYPE 1
* Latched bits                          = $48 (2 latch bits)
* Input bits that are passed uninverted = $04 (1 true bits)
* Input bits that are passed inverted   = $00 (0 inverted bits)
* Remaining bits for addressing PROM    = $B3 (5 bits)
* Latched bit #0:
- Input bit position  = 3
- Output bit position = 3
- Type                = Inverting latch
* Latched bit #1:
- Input bit position  = 6
- Output bit position = 6
- Type                = Non-inverting latch

*/

static uint8_t type1_map_clocknchj[8] = { T1PROM,T1PROM,T1DIRECT,T1LATCHINV,T1PROM,T1PROM,T1LATCH,T1PROM };

MACHINE_RESET_MEMBER(decocass_type1_state,clocknchj) /* 11 */
{
	machine_reset();
	LOG(0,("dongle type #1 (type1_map_clocknchj map)\n"));
	m_type1_map = type1_map_clocknchj;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}


/***************************************************************************
 *
 *  TYPE2 DONGLE (CS82-007)
 *  - Disco No 1
 *  - Tornado
 *  - Mission X
 *  - Pro Tennis
 *
 ***************************************************************************/
uint8_t decocass_type2_state::decocass_type2_r(offs_t offset)
{
	uint8_t data;

	if (1 == m_type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			uint8_t *prom = memregion("dongle")->base();
			data = prom[256 * m_type2_d2_latch + m_type2_promaddr];
			LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- prom[%03x]\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, 256 * m_type2_d2_latch + m_type2_promaddr));
		}
		else
		{
			data = 0xff;    /* floating input? */
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
			data = m_mcu->upi41_master_r(offset);
		else
			data = offset & 0xff;

		LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- 8041-%s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "STATUS" : "DATA"));
	}
	return data;
}

void decocass_type2_state::decocass_type2_w(offs_t offset, uint8_t data)
{
	if (1 == m_type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			LOG(4,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM+D2 latch", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			m_type2_promaddr = data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM addr $%02x\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_type2_promaddr));
			return;
		}
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s ", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041 DATA"));
	}
	if (1 == (offset & 1))
	{
		if (0xc0 == (data & 0xf0))
		{
			m_type2_xx_latch = 1;
			m_type2_d2_latch = (data & 0x04) ? 1 : 0;
			LOG(3,("PROM:%s D2:%d", m_type2_xx_latch ? "on" : "off", m_type2_d2_latch));
		}
	}
	m_mcu->upi41_master_w(offset & 1, data);

#ifdef MAME_DEBUG
	decocass_fno(offset, data);
#endif
}

/***************************************************************************
 *
 *  TYPE3 DONGLE
 *  - Burger Time
 *  - Bump 'n Jump
 *  - Burnin' Rubber
 *  - Graplop
 *  - Cluster Buster
 *  - LaPaPa
 *  - Skater
 *  - Pro Bowling
 *  - Night Star
 *  - Pro Soccer
 *  - Super Doubles Tennis
 *  - Peter Pepper's Ice Cream Factory
 *  - Fighting Ice Hockey
 *
 ***************************************************************************/
uint8_t decocass_type3_state::decocass_type3_r(offs_t offset)
{
	uint8_t data, save;

	if (1 == (offset & 1))
	{
		if (1 == m_type3_pal_19)
		{
			uint8_t *prom = memregion("dongle")->base();
			data = prom[m_type3_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- prom[$%03x]\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_type3_ctrs));
			if (++m_type3_ctrs == 4096)
				m_type3_ctrs = 0;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(1);
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- 8041 STATUS\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
			}
		}
	}
	else
	{
		if (1 == m_type3_pal_19)
		{
			save = data = 0xff;    /* open data bus? */
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				save = m_mcu->upi41_master_r(0);
				switch (m_type3_swap)
				{
				case TYPE3_SWAP_01:
					data =
						(BIT(save, 1) << 0) |
						(m_type3_d0_latch << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_12:
					data =
						(m_type3_d0_latch << 0) |
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
						(m_type3_d0_latch << 0) |
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
						(m_type3_d0_latch << 0) |
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
						(m_type3_d0_latch << 0) |
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
						(m_type3_d0_latch << 0) |
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
						(m_type3_d0_latch << 7);
					break;
				case TYPE3_SWAP_45:
					data =
						m_type3_d0_latch |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 5) << 4) |
						(BIT(save, 4) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
					break;
				case TYPE3_SWAP_23_56:
					data =
						(m_type3_d0_latch << 0) |
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
						m_type3_d0_latch |
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
						m_type3_d0_latch |
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
						m_type3_d0_latch |
						(BIT(save, 1) << 1) |
						(BIT(save, 2) << 2) |
						(BIT(save, 3) << 3) |
						(BIT(save, 4) << 4) |
						(BIT(save, 5) << 5) |
						(BIT(save, 6) << 6) |
						(BIT(save, 7) << 7);
				}
				m_type3_d0_latch = save & 1;
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- 8041-DATA\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				save = 0xff;    /* open data bus? */
				data =
					m_type3_d0_latch |
					(BIT(save, 1) << 1) |
					(BIT(save, 2) << 2) |
					(BIT(save, 3) << 3) |
					(BIT(save, 4) << 4) |
					(BIT(save, 5) << 5) |
					(BIT(save, 6) << 7) |
					(BIT(save, 7) << 6);
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
				m_type3_d0_latch = save & 1;
			}
		}
	}

	return data;
}

void decocass_type3_state::decocass_type3_w(offs_t offset, uint8_t data)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type3_pal_19)
		{
			m_type3_ctrs = data << 4;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, "LDCTRS"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			m_type3_pal_19 = 1;
	}
	else
	{
		if (1 == m_type3_pal_19)
		{
			/* write nowhere?? */
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(offset, data);
}

/***************************************************************************
 *
 *  TYPE4 DONGLE
 *  - Scrum Try
 *  - Oozumou/The Grand Sumo
 *  Contains a 32K (EP)ROM that can be read from any byte
 *  boundary sequentially. The EPROM is enable after writing
 *  1100xxxx to E5x1 once. Then an address is written LSB
 *  to E5x0 MSB to E5x1 and every read from E5x1 returns the
 *  next byte of the contents.
 *
 ***************************************************************************/

uint8_t decocass_type4_state::decocass_type4_r(offs_t offset)
{
	uint8_t data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(1);
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- 8041 STATUS\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
	}
	else
	{
		if (m_type4_latch)
		{
			uint8_t *prom = machine().root_device().memregion("dongle")->base();

			data = prom[m_type4_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- PROM[%04x]\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.', m_type4_ctrs));
			m_type4_ctrs = (m_type4_ctrs + 1) & 0x7fff;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(0);
				LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
			}
		}
	}

	return data;
}

void decocass_type4_state::decocass_type4_w(offs_t offset, uint8_t data)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type4_latch)
		{
			m_type4_ctrs = (m_type4_ctrs & 0x00ff) | ((data & 0x7f) << 8);
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS MSB (%04x)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_type4_ctrs));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
		{
			m_type4_latch = 1;
		}
	}
	else
	{
		if (m_type4_latch)
		{
			m_type4_ctrs = (m_type4_ctrs & 0xff00) | data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS LSB (%04x)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_type4_ctrs));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(offset, data);
}

/***************************************************************************
 *
 *  TYPE4 DONGLE
 *  - Boulder Dash
 *  Actually a NOP dongle returning 0x55 after triggering a latch
 *  by writing 1100xxxx to E5x1
 *
 ***************************************************************************/

uint8_t decocass_type5_state::decocass_type5_r(offs_t offset)
{
	uint8_t data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(1);
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- 8041 STATUS\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
	}
	else
	{
		if (m_type5_latch)
		{
			data = 0x55;    /* Only a fixed value? It looks like this is all we need to do */
			LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- fixed value???\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(0);
				LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
			}
		}
	}

	return data;
}

void decocass_type5_state::decocass_type5_w(offs_t offset, uint8_t data)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type5_latch)
		{
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, "latch #2??"));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
			m_type5_latch = 1;
	}
	else
	{
		if (m_type5_latch)
		{
			/* write nowhere?? */
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(offset, data);
}

/***************************************************************************
 *
 *  NO DONGLE
 *  - Flying Ball
 *  A NOP dongle returning the data read from cassette as is.
 *
 ***************************************************************************/

uint8_t decocass_nodong_state::decocass_nodong_r(offs_t offset)
{
	uint8_t data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(1);
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- 8041 STATUS\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(0);
			LOG(3,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
	}

	return data;
}


/***************************************************************************
 *
 *  Widel Multigame Dongle
 *   - provides access to a large ROM
 *
 ***************************************************************************/

uint8_t decocass_widel_state::decocass_widel_r(offs_t offset)
{
	uint8_t data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			if (m_widel_latch && !machine().side_effects_disabled())
				m_widel_ctrs = (m_widel_ctrs + 0x100) & 0xfffff;
			data = m_mcu->upi41_master_r(1);
			LOG(4,("%10s 6502-PC: %04x decocass_widel_r(%02x): $%02x <- 8041 STATUS\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_widel_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
		}
	}
	else
	{
		if (m_widel_latch)
		{
			uint8_t *prom = machine().root_device().memregion("dongle")->base();

			data = prom[m_widel_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_widel_r(%02x): $%02x '%c' <- PROM[%04x]\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.', m_widel_ctrs));

			if (!machine().side_effects_disabled())
				m_widel_ctrs = (m_widel_ctrs + 1) & 0xfffff;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(0);
				LOG(3,("%10s 6502-PC: %04x decocass_widel_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_widel_r(%02x): $%02x <- open bus\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
			}
		}
	}

	return data;
}

void decocass_widel_state::decocass_widel_w(offs_t offset, uint8_t data)
{
	if (1 == (offset & 1))
	{
		if (1 == m_widel_latch)
		{
			// BIOS follows writes to here by counting out a lot of dummy reads from the
			// same location, probably to advance a 74HC4040 or similar counter.
			// Counterintuitive though it may seem, the value written is probably just ignored.
			// Treasure Island depends on this clearing the lower bits as well.
			m_widel_ctrs = 0;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS MSB (%04x)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_widel_ctrs));
			return;
		}
		else
		if (0xc0 == (data & 0xf0))
		{
			m_widel_latch = 1;
		}
	}
	else
	{
		if (m_widel_latch)
		{
			m_widel_ctrs = (m_widel_ctrs & 0xfff00) | data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS LSB (%04x)\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, m_widel_ctrs));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(offset, data);
}

/***************************************************************************
 *
 *  Main dongle and 8041 interface
 *
 ***************************************************************************/

uint8_t decocass_state::decocass_e5xx_r(offs_t offset)
{
	uint8_t data;

	/* E5x2-E5x3 and mirrors */
	if (2 == (offset & E5XX_MASK))
	{
		uint8_t bot_eot = (m_cassette->get_status_bits() >> 5) & 1;

		data =
			(BIT(m_i8041_p1, 7)   << 0) |   /* D0 = P17 - REQ/ */
			(BIT(m_i8041_p2, 0)   << 1) |   /* D1 = P20 - FNO/ */
			(BIT(m_i8041_p2, 1)   << 2) |   /* D2 = P21 - EOT/ */
			(BIT(m_i8041_p2, 2)   << 3) |   /* D3 = P22 - ERR/ */
			((bot_eot)            << 4) |   /* D4 = BOT/EOT (direct from drive) */
			(1                    << 5) |   /* D5 floating input */
			(1                    << 6) |   /* D6 floating input */
			(!m_cassette->is_present() << 7);    /* D7 = cassette present */

		LOG(4,("%10s 6502-PC: %04x decocass_e5xx_r(%02x): $%02x <- STATUS (%s%s%s%s%s%s%s%s)\n",
			machine().time().as_string(6),
			m_maincpu->pcbase(),
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
		if (!m_dongle_r.isnull())
			data = m_dongle_r(offset);
		else
			data = 0xff;
	}
	return data;
}

void decocass_state::decocass_e5xx_w(offs_t offset, uint8_t data)
{
	if (!m_dongle_w.isnull())
	{
		m_dongle_w(offset, data);
		return;
	}

	if (0 == (offset & E5XX_MASK))
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
		m_mcu->upi41_master_w(offset & 1, data);
#ifdef MAME_DEBUG
		decocass_fno(offset, data);
#endif
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> dongle\n", machine().time().as_string(6), m_maincpu->pcbase(), offset, data));
	}
}

/***************************************************************************
 *
 *  DE-0091xx daughter board handler
 *
 *  The DE-0091xx daughter board seems to be a read-only ROM board with
 *  two times five 4K ROMs.
 *
 *  The board's ROMs are mapped into view for reads between addresses
 *  0x6000 and 0xafff by setting bits 0 and 1 of address 0xe900.
 *
 ***************************************************************************/

void decocass_state::decocass_e900_w(uint8_t data)
{
	m_de0091_enable = data & 3;

	if (m_de0091_enable == 0x3) // invalid
		return;

	membank("bank1")->set_entry(data & 3);
}

void decocass_state::decocass_de0091_w(offs_t offset, uint8_t data)
{
	/* don't allow writes to the ROMs - actually cexplore requires us to allow them */
	//if (!m_de0091_enable)
		decocass_charram_w(offset, data);
}

/***************************************************************************
 *
 *  state save setup
 *
 ***************************************************************************/
/* To be called once from driver_init, i.e. decocass_init */
void decocass_state::decocass_machine_state_save_init()
{
	save_item(NAME(m_firsttime));
	save_item(NAME(m_decocass_reset));
	save_item(NAME(m_i8041_p1));
	save_item(NAME(m_i8041_p2));
	save_item(NAME(m_de0091_enable));
	save_item(NAME(m_sound_ack));

	save_item(NAME(m_quadrature_decoder));
	save_item(NAME(m_latch1));
	save_item(NAME(m_audio_nmi_enabled));
	save_item(NAME(m_audio_nmi_state));
	save_item(NAME(m_i8041_p1_write_latch));
	save_item(NAME(m_i8041_p2_write_latch));
	save_item(NAME(m_i8041_p1_read_latch));
	save_item(NAME(m_i8041_p2_read_latch));
}

/***************************************************************************
 *
 *  init machine functions (select dongle and determine tape image size)
 *
 ***************************************************************************/

void decocass_state::machine_start()
{
}


void decocass_state::machine_reset()
{
	m_firsttime = 1;
	m_latch1 = 0;

	m_dongle_r = read8sm_delegate(*this);
	m_dongle_w = write8sm_delegate(*this);

	m_decocass_reset = 0;
	m_i8041_p1 = 0xff;
	m_i8041_p2 = 0xff;
	m_i8041_p1_write_latch = 0xff;
	m_i8041_p2_write_latch = 0xff;
	m_i8041_p1_read_latch = 0xff;
	m_i8041_p2_read_latch = 0xff;
	m_de0091_enable = 0;

	memset(m_quadrature_decoder, 0, sizeof(m_quadrature_decoder));
	m_sound_ack = 0;
	m_audio_nmi_enabled = 0;
	m_audio_nmi_state = 0;

	/* video-related */
	m_watchdog_flip = 0;
	m_color_missiles = 0;
	m_color_center_bot = 0;
	m_mode_set = 0;
	m_back_h_shift = 0;
	m_back_vl_shift = 0;
	m_back_vr_shift = 0;
	m_part_h_shift = 0;
	m_part_v_shift = 0;
	m_center_h_shift_space = 0;
	m_center_v_shift = 0;
}

void decocass_type1_state::machine_start()
{
	save_item(NAME(m_type1_inmap));
	save_item(NAME(m_type1_outmap));
}

void decocass_type1_state::machine_reset()
{
	decocass_state::machine_reset();

	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_type1_state::decocass_type1_r));
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

void decocass_type2_state::machine_start()
{
	save_item(NAME(m_type2_d2_latch));
	save_item(NAME(m_type2_xx_latch));
	save_item(NAME(m_type2_promaddr));
}

void decocass_type2_state::machine_reset()
{
	decocass_state::machine_reset();

	LOG(0,("dongle type #2 (CS82-007)\n"));
	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_type2_state::decocass_type2_r));
	m_dongle_w = write8sm_delegate(*this, FUNC(decocass_type2_state::decocass_type2_w));

	m_type2_d2_latch = 0;
	m_type2_xx_latch = 0;
	m_type2_promaddr = 0;
}

void decocass_type3_state::machine_start()
{
	save_item(NAME(m_type3_ctrs));
	save_item(NAME(m_type3_d0_latch));
	save_item(NAME(m_type3_pal_19));
	save_item(NAME(m_type3_swap));
}

void decocass_type3_state::machine_reset()
{
	decocass_state::machine_reset();

	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_type3_state::decocass_type3_r));
	m_dongle_w = write8sm_delegate(*this, FUNC(decocass_type3_state::decocass_type3_w));

	m_type3_ctrs = 0;
	m_type3_d0_latch = 0;
	m_type3_pal_19 = 0;
	m_type3_swap = 0;
}

void decocass_type4_state::machine_start()
{
	save_item(NAME(m_type4_ctrs));
	save_item(NAME(m_type4_latch));
}

void decocass_type4_state::machine_reset()
{
	decocass_state::machine_reset();

	LOG(0,("dongle type #4 (32K ROM)\n"));
	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_type4_state::decocass_type4_r));
	m_dongle_w = write8sm_delegate(*this, FUNC(decocass_type4_state::decocass_type4_w));

	m_type4_ctrs = 0;
	m_type4_latch = 0;
}

void decocass_type5_state::machine_start()
{
	save_item(NAME(m_type5_latch));
}

void decocass_type5_state::machine_reset()
{
	decocass_state::machine_reset();

	LOG(0,("dongle type #5 (NOP)\n"));
	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_type5_state::decocass_type5_r));
	m_dongle_w = write8sm_delegate(*this, FUNC(decocass_type5_state::decocass_type5_w));

	m_type5_latch = 0;
}

void decocass_nodong_state::machine_reset()
{
	decocass_state::machine_reset();
	LOG(0, ("no dongle\n"));
	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_nodong_state::decocass_nodong_r));
}

MACHINE_RESET_MEMBER(decocass_type1_state,ctsttape)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_type1_map = type1_pass_136_table;
}

MACHINE_RESET_MEMBER(decocass_type1_state,chwy)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	m_type1_map = type1_latch_27_pass_3_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_type1_state,cdsteljn)
{
	machine_reset();
	LOG(0,("dongle type #1 (A-0061)\n"));
	m_type1_map = type1_latch_27_pass_3_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_type1_state,cterrani)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 straight)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,castfant)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_type1_map = type1_latch_16_pass_3_inv_1_table;
}

MACHINE_RESET_MEMBER(decocass_type1_state,csuperas)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 4-5)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,2,3,5,4,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,5,4,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,cmanhat)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_type1_map = type1_latch_xab_pass_x54_table;
}

MACHINE_RESET_MEMBER(decocass_type1_state,clocknch)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 2-3)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,3,2,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,3,2,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,cprogolf)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 0-1)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,cprogolfj)
{
	machine_reset();
	LOG(0,("dongle type #1 (A-0061 flip 0-1)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,cluckypo)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 1-3)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,3,2,1,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,3,2,1,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,ctisland)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 0-2)\n"));
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(2,1,0,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(2,1,0,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_type1_state,ctisland3)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 custom)\n"));
	m_type1_map = type1_latch_ctisland3;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);  // correct for handcrafted prom
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7); // ^
}

MACHINE_RESET_MEMBER(decocass_type1_state,cexplore)
{
	machine_reset();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	m_type1_map = type1_latch_26_pass_5_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cfishing)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_01;

}

MACHINE_RESET_MEMBER(decocass_type3_state,cbtime)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_12;

}

MACHINE_RESET_MEMBER(decocass_type3_state,cburnrub)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cgraplop)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_56;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cgraplop2)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET_MEMBER(decocass_type3_state,clapapa)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_34_7;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cskater)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_45;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cprobowl)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_34_0;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cnightst)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_13;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cpsoccer)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_24;
}

MACHINE_RESET_MEMBER(decocass_type3_state,csdtenis)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_23_56;
}

MACHINE_RESET_MEMBER(decocass_type3_state,czeroize)
{
	uint8_t *mem = memregion("dongle")->base();
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_23_56;

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

MACHINE_RESET_MEMBER(decocass_type3_state,cppicf)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_01;
}

MACHINE_RESET_MEMBER(decocass_type3_state,cfghtice)
{
	machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_type3_swap = TYPE3_SWAP_25;
}




void decocass_widel_state::machine_start()
{
	decocass_state::machine_start();

	save_item(NAME(m_widel_ctrs));
	save_item(NAME(m_widel_latch));
}

void decocass_widel_state::machine_reset()
{
	decocass_state::machine_reset();
	LOG(0,("Deco Multigame Dongle\n"));
	m_dongle_r = read8sm_delegate(*this, FUNC(decocass_widel_state::decocass_widel_r));
	m_dongle_w = write8sm_delegate(*this, FUNC(decocass_widel_state::decocass_widel_w));

	m_widel_ctrs = 0;
	m_widel_latch = 0;
}


/***************************************************************************
 *
 *  8041 port handlers
 *
 ***************************************************************************/

void decocass_state::i8041_p1_w(uint8_t data)
{
	if (data != m_i8041_p1_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_w: $%02x (%s%s%s%s%s%s%s%s)\n",
				machine().time().as_string(6),
				m_maincpu->pcbase(),
				data,
				data & 0x01 ? "" : "DATA-WRT",
				data & 0x02 ? "" : " DATA-CLK",
				data & 0x04 ? "" : " FAST",
				data & 0x08 ? "" : " BIT3",
				data & 0x10 ? "" : " REW",
				data & 0x20 ? "" : " FWD",
				data & 0x40 ? "" : " WREN",
				data & 0x80 ? "" : " REQ"));
		m_i8041_p1_write_latch = data;
	}

	/* change in FAST/REW/FWD signals? */
	if ((data ^ m_i8041_p1) & 0x34)
	{
		int newspeed = 0;

		if ((data & 0x30) == 0x20)
			newspeed = (data & 0x04) ? -1 : -7;
		else if ((data & 0x30) == 0x10)
			newspeed = (data & 0x04) ? 1 : 7;
		m_cassette->change_speed(newspeed);
	}

	m_i8041_p1 = data;
}

uint8_t decocass_state::i8041_p1_r()
{
	uint8_t data = m_i8041_p1;

	if (data != m_i8041_p1_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			machine().time().as_string(6),
			m_maincpu->pcbase(),
			data,
			data & 0x01 ? "" : "DATA-WRT",
			data & 0x02 ? "" : " DATA-CLK",
			data & 0x04 ? "" : " FAST",
			data & 0x08 ? "" : " BIT3",
			data & 0x10 ? "" : " REW",
			data & 0x20 ? "" : " FWD",
			data & 0x40 ? "" : " WREN",
			data & 0x80 ? "" : " REQ"));
		m_i8041_p1_read_latch = data;
	}
	return data;
}

void decocass_state::i8041_p2_w(uint8_t data)
{
	if (data != m_i8041_p2_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			machine().time().as_string(6),
			m_maincpu->pcbase(),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		m_i8041_p2_write_latch = data;
	}
	m_i8041_p2 = (m_i8041_p2 & 0xe0) | (data & ~0xe0);
}

uint8_t decocass_state::i8041_p2_r()
{
	uint8_t data;

	data = (m_i8041_p2 & ~0xe0) | m_cassette->get_status_bits();

	if (data != m_i8041_p2_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			machine().time().as_string(6),
			m_maincpu->pcbase(),
			data,
			data & 0x01 ? "" : "FNO/",
			data & 0x02 ? "" : " EOT/",
			data & 0x04 ? "" : " ERR/",
			data & 0x08 ? "" : " OUT3?/",
			data & 0x10 ? " [IN4]" : "",
			data & 0x20 ? " [BOT-EOT]" : "",
			data & 0x40 ? " [RCLK]" : "",
			data & 0x80 ? " [RDATA]" : ""));
		m_i8041_p2_read_latch = data;
	}
	return data;
}
