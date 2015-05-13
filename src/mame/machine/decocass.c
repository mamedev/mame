// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/decocass.h"
#include "machine/decocass_tape.h"

/* dongle type #1: jumpers C and D assignments */
#define MAKE_MAP(m0,m1,m2,m3,m4,m5,m6,m7)   \
	((UINT32)(m0)) | \
	((UINT32)(m1) << 3) | \
	((UINT32)(m2) << 6) | \
	((UINT32)(m3) << 9) | \
	((UINT32)(m4) << 12) | \
	((UINT32)(m5) << 15) | \
	((UINT32)(m6) << 18) | \
	((UINT32)(m7) << 21)


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


WRITE8_MEMBER(decocass_state::decocass_coin_counter_w)
{
}

READ8_MEMBER( decocass_state::decocass_sound_command_main_r)
{
	// cgraplop2 needs to read something here or it will reset when you coin-up
	//  could do with further investigation
	return 0xc0;
}

WRITE8_MEMBER(decocass_state::decocass_sound_command_w)
{
	LOG(2,("CPU %s sound command -> $%02x\n", space.device().tag(), data));
	soundlatch_byte_w(space, 0, data);
	m_sound_ack |= 0x80;
	/* remove snd cpu data ack bit. i don't see it in the schems, but... */
	m_sound_ack &= ~0x40;
	m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}

READ8_MEMBER(decocass_state::decocass_sound_data_r)
{
	UINT8 data = soundlatch2_byte_r(space, 0);
	LOG(2,("CPU %s sound data    <- $%02x\n", space.device().tag(), data));
	return data;
}

READ8_MEMBER(decocass_state::decocass_sound_ack_r)
{
	UINT8 data = m_sound_ack;   /* D6+D7 */
	LOG(4,("CPU %s sound ack     <- $%02x\n", space.device().tag(), data));
	return data;
}

WRITE8_MEMBER(decocass_state::decocass_sound_data_w)
{
	LOG(2,("CPU %s sound data    -> $%02x\n", space.device().tag(), data));
	soundlatch2_byte_w(space, 0, data);
	m_sound_ack |= 0x40;
}

READ8_MEMBER(decocass_state::decocass_sound_command_r)
{
	UINT8 data = soundlatch_byte_r(space, 0);
	LOG(4,("CPU %s sound command <- $%02x\n", space.device().tag(), data));
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

WRITE8_MEMBER(decocass_state::decocass_sound_nmi_enable_w)
{
	m_audio_nmi_enabled = 1;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(decocass_state::decocass_sound_nmi_enable_r)
{
	m_audio_nmi_enabled = 1;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_nmi_enabled && m_audio_nmi_state) ? ASSERT_LINE : CLEAR_LINE);
	return 0xff;
}

READ8_MEMBER(decocass_state::decocass_sound_data_ack_reset_r)
{
	UINT8 data = 0xff;
	LOG(2,("CPU %s sound ack rst <- $%02x\n", space.device().tag(), data));
	m_sound_ack &= ~0x40;
	return data;
}

WRITE8_MEMBER(decocass_state::decocass_sound_data_ack_reset_w)
{
	LOG(2,("CPU %s sound ack rst -> $%02x\n", space.device().tag(), data));
	m_sound_ack &= ~0x40;
}

WRITE8_MEMBER(decocass_state::decocass_nmi_reset_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE );
}

WRITE8_MEMBER(decocass_state::decocass_quadrature_decoder_reset_w)
{
	/* just latch the analog controls here */
	m_quadrature_decoder[0] = ioport("AN0")->read();
	m_quadrature_decoder[1] = ioport("AN1")->read();
	m_quadrature_decoder[2] = ioport("AN2")->read();
	m_quadrature_decoder[3] = ioport("AN3")->read();
}

WRITE8_MEMBER(decocass_state::decocass_adc_w)
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
READ8_MEMBER(decocass_state::decocass_input_r)
{
	UINT8 data = 0xff;
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };

	switch (offset & 7)
	{
	case 0: case 1: case 2:
		data = space.machine().root_device().ioport(portnames[offset & 7])->read();
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


WRITE8_MEMBER(decocass_state::decocass_reset_w)
{
	LOG(1,("%10s 6502-PC: %04x decocass_reset_w(%02x): $%02x\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
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
void decocass_state::decocass_fno( offs_t offset, UINT8 data )
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


READ8_MEMBER(decocass_state::decocass_type1_r)
{
	if (!m_type1_map)
		return 0x00;

	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
			data = m_mcu->upi41_master_r(space,1);
		else
			data = 0xff;

		data = (BIT(data, 0) << 0) | (BIT(data, 1) << 1) | 0x7c;
		LOG(4,("%10s 6502-PC: %04x decocass_type1_r(%02x): $%02x <- (%s %s)\n",
			space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data,
			(data & 1) ? "OBF" : "-",
			(data & 2) ? "IBF" : "-"));
	}
	else
	{
		offs_t promaddr;
		UINT8 save;
		UINT8 *prom = space.machine().root_device().memregion("dongle")->base();

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
			data = m_mcu->upi41_master_r(space,0);
		else
			data = 0xff;

		save = data;    /* save the unmodifed data for the latch */

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
			space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));

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

static UINT8 type1_latch_26_pass_3_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1DIRECT,T1PROM, T1PROM,T1LATCH,T1PROM };

/***************************************************************************
 *
 *  TYPE1 DONGLE (DE-0061)
 *  - Test Tape
 *
 * Pass bits 1, 3, and 6. Lookup PROM DE-0061 using bits 0, 2, 4, 5, and 7
 * as the address bits; take PROM data 0-4 as data bits 0, 2, 4, 5, and 7.
 *
 ***************************************************************************/

static UINT8 type1_pass_136_table[8] ={ T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM,T1PROM,T1DIRECT,T1PROM };

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

static UINT8 type1_latch_xab_pass_x54_table[8] = { T1PROM,T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM,T1DIRECT,T1PROM };

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

static UINT8 type1_latch_27_pass_3_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1DIRECT,T1PROM,T1PROM,T1PROM,T1LATCH };

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

static UINT8 type1_latch_26_pass_5_inv_2_table[8] = { T1PROM,T1PROM,T1LATCHINV,T1PROM,T1PROM,T1DIRECT,T1LATCH,T1PROM };

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

static UINT8 type1_latch_16_pass_3_inv_1_table[8] = { T1PROM,T1LATCHINV,T1PROM,T1DIRECT,T1PROM,T1PROM,T1LATCH,T1PROM };

/***************************************************************************
 *
 *  TYPE2 DONGLE (CS82-007)
 *  - Disco No 1
 *  - Tornado
 *  - Mission X
 *  - Pro Tennis
 *
 ***************************************************************************/
READ8_MEMBER(decocass_state::decocass_type2_r)
{
	UINT8 data;

	if (1 == m_type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			UINT8 *prom = memregion("dongle")->base();
			data = prom[256 * m_type2_d2_latch + m_type2_promaddr];
			LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- prom[%03x]\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, 256 * m_type2_d2_latch + m_type2_promaddr));
		}
		else
		{
			data = 0xff;    /* floating input? */
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
			data = m_mcu->upi41_master_r(space,offset);
		else
			data = offset & 0xff;

		LOG(3,("%10s 6502-PC: %04x decocass_type2_r(%02x): $%02x <- 8041-%s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "STATUS" : "DATA"));
	}
	return data;
}

WRITE8_MEMBER(decocass_state::decocass_type2_w)
{
	if (1 == m_type2_xx_latch)
	{
		if (1 == (offset & 1))
		{
			LOG(4,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM+D2 latch", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
		else
		{
			m_type2_promaddr = data;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> set PROM addr $%02x\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, m_type2_promaddr));
			return;
		}
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s ", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041 DATA"));
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
	m_mcu->upi41_master_w(space,offset & 1, data);

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
READ8_MEMBER(decocass_state::decocass_type3_r)
{
	UINT8 data, save;

	if (1 == (offset & 1))
	{
		if (1 == m_type3_pal_19)
		{
			UINT8 *prom = memregion("dongle")->base();
			data = prom[m_type3_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- prom[$%03x]\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, m_type3_ctrs));
			if (++m_type3_ctrs == 4096)
				m_type3_ctrs = 0;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(space,1);
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- 8041 STATUS\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
			}
		}
	}
	else
	{
		if (1 == m_type3_pal_19)
		{
			save = data = 0xff;    /* open data bus? */
			LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x <- open bus", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				save = m_mcu->upi41_master_r(space,0);
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
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- 8041-DATA\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
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
				LOG(3,("%10s 6502-PC: %04x decocass_type3_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
				m_type3_d0_latch = save & 1;
			}
		}
	}

	return data;
}

WRITE8_MEMBER(decocass_state::decocass_type3_w)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type3_pal_19)
		{
			m_type3_ctrs = data << 4;
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, "LDCTRS"));
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
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(space,offset, data);
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

READ8_MEMBER(decocass_state::decocass_type4_r)
{
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(space,1);
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- 8041 STATUS\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
	}
	else
	{
		if (m_type4_latch)
		{
			UINT8 *prom = space.machine().root_device().memregion("dongle")->base();

			data = prom[m_type4_ctrs];
			LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- PROM[%04x]\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.', m_type4_ctrs));
			m_type4_ctrs = (m_type4_ctrs + 1) & 0x7fff;
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(space,0);
				LOG(3,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type4_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
			}
		}
	}

	return data;
}

WRITE8_MEMBER(decocass_state::decocass_type4_w)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type4_latch)
		{
			m_type4_ctrs = (m_type4_ctrs & 0x00ff) | ((data & 0x7f) << 8);
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS MSB (%04x)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, m_type4_ctrs));
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
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> CTRS LSB (%04x)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, m_type4_ctrs));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(space,offset, data);
}

/***************************************************************************
 *
 *  TYPE4 DONGLE
 *  - Boulder Dash
 *  Actually a NOP dongle returning 0x55 after triggering a latch
 *  by writing 1100xxxx to E5x1
 *
 ***************************************************************************/

READ8_MEMBER(decocass_state::decocass_type5_r)
{
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(space,1);
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- 8041 STATUS\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
	}
	else
	{
		if (m_type5_latch)
		{
			data = 0x55;    /* Only a fixed value? It looks like this is all we need to do */
			LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- fixed value???\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			if (0 == (offset & E5XX_MASK))
			{
				data = m_mcu->upi41_master_r(space,0);
				LOG(3,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
			}
			else
			{
				data = 0xff;    /* open data bus? */
				LOG(4,("%10s 6502-PC: %04x decocass_type5_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
			}
		}
	}

	return data;
}

WRITE8_MEMBER(decocass_state::decocass_type5_w)
{
	if (1 == (offset & 1))
	{
		if (1 == m_type5_latch)
		{
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, "latch #2??"));
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
			LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, "nowhere?"));
			return;
		}
	}
	LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
	m_mcu->upi41_master_w(space,offset, data);
}

/***************************************************************************
 *
 *  NO DONGLE
 *  - Flying Ball
 *  A NOP dongle returning the data read from cassette as is.
 *
 ***************************************************************************/

READ8_MEMBER(decocass_state::decocass_nodong_r)
{
	UINT8 data;

	if (1 == (offset & 1))
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(space,1);
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- 8041 STATUS\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
	}
	else
	{
		if (0 == (offset & E5XX_MASK))
		{
			data = m_mcu->upi41_master_r(space,0);
			LOG(3,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x '%c' <- open bus (D0 replaced with latch)\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, (data >= 32) ? data : '.'));
		}
		else
		{
			data = 0xff;    /* open data bus? */
			LOG(4,("%10s 6502-PC: %04x decocass_nodong_r(%02x): $%02x <- open bus\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
		}
	}

	return data;
}

/***************************************************************************
 *
 *  Main dongle and 8041 interface
 *
 ***************************************************************************/

READ8_MEMBER(decocass_state::decocass_e5xx_r)
{
	UINT8 data;

	/* E5x2-E5x3 and mirrors */
	if (2 == (offset & E5XX_MASK))
	{
		UINT8 bot_eot = (m_cassette->get_status_bits() >> 5) & 1;

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
			space.machine().time().as_string(6),
			space.device().safe_pcbase(),
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
			data = (m_dongle_r)(space, offset, mem_mask);
		else
			data = 0xff;
	}
	return data;
}

WRITE8_MEMBER(decocass_state::decocass_e5xx_w)
{
	if (!m_dongle_w.isnull())
	{
		(m_dongle_w)(space, offset, data, mem_mask);
		return;
	}

	if (0 == (offset & E5XX_MASK))
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> %s\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data, offset & 1 ? "8041-CMND" : "8041-DATA"));
		m_mcu->upi41_master_w(space,offset & 1, data);
#ifdef MAME_DEBUG
		decocass_fno(offset, data);
#endif
	}
	else
	{
		LOG(3,("%10s 6502-PC: %04x decocass_e5xx_w(%02x): $%02x -> dongle\n", space.machine().time().as_string(6), space.device().safe_pcbase(), offset, data));
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

WRITE8_MEMBER(decocass_state::decocass_e900_w)
{
	m_de0091_enable = data & 1;
	membank("bank1")->set_entry(data & 1);
	/* Perhaps the second row of ROMs is enabled by another bit.
	 * There is no way to verify this yet, so for now just look
	 * at bit 0 to enable the daughter board at reads between
	 * 0x6000 and 0xafff.
	 */
}

WRITE8_MEMBER(decocass_state::decocass_de0091_w)
{
	/* don't allow writes to the ROMs */
	if (!m_de0091_enable)
		decocass_charram_w(space, offset, data);
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
	save_item(NAME(m_type1_inmap));
	save_item(NAME(m_type1_outmap));
	save_item(NAME(m_type2_d2_latch));
	save_item(NAME(m_type2_xx_latch));
	save_item(NAME(m_type2_promaddr));
	save_item(NAME(m_type3_ctrs));
	save_item(NAME(m_type3_d0_latch));
	save_item(NAME(m_type3_pal_19));
	save_item(NAME(m_type3_swap));
	save_item(NAME(m_type4_ctrs));
	save_item(NAME(m_type4_latch));
	save_item(NAME(m_type5_latch));
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

	m_dongle_r = read8_delegate();
	m_dongle_w = write8_delegate();

	m_decocass_reset = 0;
	m_i8041_p1 = 0xff;
	m_i8041_p2 = 0xff;
	m_i8041_p1_write_latch = 0xff;
	m_i8041_p2_write_latch = 0xff;
	m_i8041_p1_read_latch = 0xff;
	m_i8041_p2_read_latch = 0xff;
	m_de0091_enable = 0;

	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);

	m_type2_d2_latch = 0;
	m_type2_xx_latch = 0;
	m_type2_promaddr = 0;

	m_type3_ctrs = 0;
	m_type3_d0_latch = 0;
	m_type3_pal_19 = 0;
	m_type3_swap = 0;

	m_type4_ctrs = 0;
	m_type4_latch = 0;

	m_type5_latch = 0;

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

MACHINE_RESET_MEMBER(decocass_state,ctsttape)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_pass_136_table;
}

MACHINE_RESET_MEMBER(decocass_state,chwy)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_27_pass_3_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_state,cdsteljn)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (A-0061)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_27_pass_3_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_state,cterrani)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 straight)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,castfant)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_16_pass_3_inv_1_table;
}

MACHINE_RESET_MEMBER(decocass_state,csuperas)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 4-5)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,2,3,5,4,6,7);
	m_type1_outmap = MAKE_MAP(0,1,2,3,5,4,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,cmanhat)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_xab_pass_x54_table;
}

MACHINE_RESET_MEMBER(decocass_state,clocknch)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 2-3)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,1,3,2,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,1,3,2,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,cprogolf)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 0-1)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,cprogolfj)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (A-0061 flip 0-1)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(1,0,2,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(1,0,2,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,cluckypo)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 1-3)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(0,3,2,1,4,5,6,7);
	m_type1_outmap = MAKE_MAP(0,3,2,1,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,ctisland)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 flip 0-2)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_3_inv_2_table;
	m_type1_inmap = MAKE_MAP(2,1,0,3,4,5,6,7);
	m_type1_outmap = MAKE_MAP(2,1,0,3,4,5,6,7);
}

MACHINE_RESET_MEMBER(decocass_state,cexplore)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #1 (DE-0061 own PROM)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type1_r),this);
	m_type1_map = type1_latch_26_pass_5_inv_2_table;
}

MACHINE_RESET_MEMBER(decocass_state,cdiscon1)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type2_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type2_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,ctornado)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type2_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type2_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,cmissnx)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type2_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type2_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,cptennis)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #2 (CS82-007)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type2_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type2_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,cfishing)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_01;

}

MACHINE_RESET_MEMBER(decocass_state,cbtime)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_12;

}

MACHINE_RESET_MEMBER(decocass_state,cburnrub)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET_MEMBER(decocass_state,cgraplop)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_56;
}

MACHINE_RESET_MEMBER(decocass_state,cgraplop2)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_67;
}

MACHINE_RESET_MEMBER(decocass_state,clapapa)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_34_7;
}

MACHINE_RESET_MEMBER(decocass_state,cskater)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_45;
}

MACHINE_RESET_MEMBER(decocass_state,cprobowl)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_34_0;
}

MACHINE_RESET_MEMBER(decocass_state,cnightst)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_13;
}

MACHINE_RESET_MEMBER(decocass_state,cpsoccer)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_24;
}

MACHINE_RESET_MEMBER(decocass_state,csdtenis)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_23_56;
}

MACHINE_RESET_MEMBER(decocass_state,czeroize)
{
	UINT8 *mem = memregion("dongle")->base();
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
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

MACHINE_RESET_MEMBER(decocass_state,cppicf)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_01;
}

MACHINE_RESET_MEMBER(decocass_state,cfghtice)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #3 (PAL)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type3_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type3_w),this);
	m_type3_swap = TYPE3_SWAP_25;
}

MACHINE_RESET_MEMBER(decocass_state,type4)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #4 (32K ROM)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type4_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type4_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,cbdash)
{
	decocass_state::machine_reset();
	LOG(0,("dongle type #5 (NOP)\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_type5_r),this);
	m_dongle_w = write8_delegate(FUNC(decocass_state::decocass_type5_w),this);
}

MACHINE_RESET_MEMBER(decocass_state,cflyball)
{
	decocass_state::machine_reset();
	LOG(0,("no dongle\n"));
	m_dongle_r = read8_delegate(FUNC(decocass_state::decocass_nodong_r),this);
}

/***************************************************************************
 *
 *  8041 port handlers
 *
 ***************************************************************************/

WRITE8_MEMBER(decocass_state::i8041_p1_w)
{
	if (data != m_i8041_p1_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			space.machine().time().as_string(6),
			space.device().safe_pcbase(),
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

READ8_MEMBER(decocass_state::i8041_p1_r)
{
	UINT8 data = m_i8041_p1;

	if (data != m_i8041_p1_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p1_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			space.machine().time().as_string(6),
			space.device().safe_pcbase(),
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

WRITE8_MEMBER(decocass_state::i8041_p2_w)
{
	if (data != m_i8041_p2_write_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_w: $%02x (%s%s%s%s%s%s%s%s)\n",
			space.machine().time().as_string(6),
			space.device().safe_pcbase(),
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

READ8_MEMBER(decocass_state::i8041_p2_r)
{
	UINT8 data;

	data = (m_i8041_p2 & ~0xe0) | m_cassette->get_status_bits();

	if (data != m_i8041_p2_read_latch)
	{
		LOG(4,("%10s 8041-PC: %03x i8041_p2_r: $%02x (%s%s%s%s%s%s%s%s)\n",
			space.machine().time().as_string(6),
			space.device().safe_pcbase(),
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
