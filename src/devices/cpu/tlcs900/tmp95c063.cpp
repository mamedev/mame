// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

Toshiba TMP95C063 emulation

*******************************************************************/

#include "emu.h"
#include "tmp95c063.h"

DEFINE_DEVICE_TYPE(TMP95C063, tmp95c063_device, "tmp95c063", "Toshiba TMP95C063")


tmp95c063_device::tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tlcs900h_device(mconfig, TMP95C063, tag, owner, clock),
	m_port1_read(*this),
	m_port1_write(*this),
	m_port2_write(*this),
	m_port5_read(*this),
	m_port5_write(*this),
	m_port6_read(*this),
	m_port6_write(*this),
	m_port7_read(*this),
	m_port7_write(*this),
	m_port8_read(*this),
	m_port8_write(*this),
	m_port9_read(*this),
	m_port9_write(*this),
	m_porta_read(*this),
	m_porta_write(*this),
	m_portb_read(*this),
	m_portb_write(*this),
	m_portc_read(*this),
	m_portd_read(*this),
	m_portd_write(*this),
	m_porte_read(*this),
	m_porte_write(*this),
	m_an_read(*this)
{
}

void tmp95c063_device::internal_mem(address_map &map)
{
	map(0x000000, 0x00009f).rw(FUNC(tmp95c063_device::internal_r), FUNC(tmp95c063_device::internal_w));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tmp95c063_device::device_config_complete()
{
	if (m_am8_16 == 0)
	{
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(tmp95c063_device::internal_mem), this));
	}
	else
	{
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 8, 24, 0, address_map_constructor(FUNC(tmp95c063_device::internal_mem), this));
	}
}


/* TMP95C063 Internal register defines */

#define TMP95C063_P1        0x01
#define TMP95C063_P1CR      0x04
#define TMP95C063_P2        0x06
#define TMP95C063_P2FC      0x09
#define TMP95C063_P5        0x0d
#define TMP95C063_P5CR      0x10
#define TMP95C063_P5FC      0x11
#define TMP95C063_P6        0x12
#define TMP95C063_P7        0x13
#define TMP95C063_P6FC      0x15
#define TMP95C063_P7CR      0x16
#define TMP95C063_P7FC      0x17
#define TMP95C063_P8        0x18
#define TMP95C063_P9        0x19
#define TMP95C063_P8CR      0x1a
#define TMP95C063_P8FC      0x1b
#define TMP95C063_P9CR      0x1c
#define TMP95C063_P9FC      0x1d
#define TMP95C063_PA        0x1e
#define TMP95C063_PB        0x1f
#define TMP95C063_T8RUN     0x20
#define TMP95C063_TRDC      0x21
#define TMP95C063_TREG0     0x22
#define TMP95C063_TREG1     0x23
#define TMP95C063_T01MOD    0x24
#define TMP95C063_T02FFCR   0x25
#define TMP95C063_TREG2     0x26
#define TMP95C063_TREG3     0x27
#define TMP95C063_T23MOD    0x28
#define TMP95C063_TREG4     0x29
#define TMP95C063_TREG5     0x2a
#define TMP95C063_T45MOD    0x2b
#define TMP95C063_T46FFCR   0x2c
#define TMP95C063_TREG6     0x2d
#define TMP95C063_TREG7     0x2e
#define TMP95C063_T67MOD    0x2f
#define TMP95C063_TREG8L    0x30
#define TMP95C063_TREG8H    0x31
#define TMP95C063_TREG9L    0x32
#define TMP95C063_TREG9H    0x33
#define TMP95C063_CAP1L     0x34
#define TMP95C063_CAP1H     0x35
#define TMP95C063_CAP2L     0x36
#define TMP95C063_CAP2H     0x37
#define TMP95C063_T8MOD     0x38
#define TMP95C063_T8FFCR    0x39
#define TMP95C063_T89CR     0x3a
#define TMP95C063_T16RUN    0x3b
#define TMP95C063_TREGAL    0x40
#define TMP95C063_TREGAH    0x41
#define TMP95C063_TREGBL    0x42
#define TMP95C063_TREGBH    0x43
#define TMP95C063_CAP3L     0x44
#define TMP95C063_CAP3H     0x45
#define TMP95C063_CAP4L     0x46
#define TMP95C063_CAP4H     0x47
#define TMP95C063_T9MOD     0x48
#define TMP95C063_T9FFCR    0x49
#define TMP95C063_DAREG0    0x4a
#define TMP95C063_DAREG1    0x4b
#define TMP95C063_PG0REG    0x4c
#define TMP95C063_PG1REG    0x4d
#define TMP95C063_PG01CR    0x4e
#define TMP95C063_DADRV     0x4f
#define TMP95C063_SC0BUF    0x50
#define TMP95C063_SC0CR     0x51
#define TMP95C063_SC0MOD    0x52
#define TMP95C063_BR0CR     0x53
#define TMP95C063_SC1BUF    0x54
#define TMP95C063_SC1CR     0x55
#define TMP95C063_SC1MOD    0x56
#define TMP95C063_BR1CR     0x57
#define TMP95C063_ODE       0x58
#define TMP95C063_DMA0V     0x5a
#define TMP95C063_DMA1V     0x5b
#define TMP95C063_DMA2V     0x5c
#define TMP95C063_DMA3V     0x5d
#define TMP95C063_ADMOD1    0x5e
#define TMP95C063_ADMOD2    0x5f
#define TMP95C063_ADREG04L  0x60
#define TMP95C063_ADREG04H  0x61
#define TMP95C063_ADREG15L  0x62
#define TMP95C063_ADREG15H  0x63
#define TMP95C063_ADREG26L  0x64
#define TMP95C063_ADREG26H  0x65
#define TMP95C063_ADREG37L  0x66
#define TMP95C063_ADREG37H  0x67
#define TMP95C063_SDMACR0   0x6a
#define TMP95C063_SDMACR1   0x6b
#define TMP95C063_SDMACR2   0x6c
#define TMP95C063_SDMACR3   0x6d
#define TMP95C063_WDMOD     0x6e
#define TMP95C063_WDCR      0x6f
#define TMP95C063_INTE0AD   0x70
#define TMP95C063_INTE12    0x71
#define TMP95C063_INTE34    0x72
#define TMP95C063_INTE56    0x73
#define TMP95C063_INTE78    0x74
#define TMP95C063_INTET01   0x75
#define TMP95C063_INTET23   0x76
#define TMP95C063_INTET45   0x77
#define TMP95C063_INTET67   0x78
#define TMP95C063_INTET89   0x79
#define TMP95C063_INTETAB   0x7a
#define TMP95C063_INTES0    0x7b
#define TMP95C063_INTES1    0x7c
#define TMP95C063_INTETC01  0x7d
#define TMP95C063_INTETC23  0x7e
#define TMP95C063_IIMC      0x7f
#define TMP95C063_PACR      0x80
#define TMP95C063_PAFC      0x81
#define TMP95C063_PBCR      0x82
#define TMP95C063_PBFC      0x83
#define TMP95C063_PC        0x84
#define TMP95C063_PD        0x85
#define TMP95C063_PDCR      0x88
#define TMP95C063_PE        0x8a
#define TMP95C063_PECR      0x8c
#define TMP95C063_BEXCS     0x8f
#define TMP95C063_B0CS      0x90
#define TMP95C063_B1CS      0x91
#define TMP95C063_B2CS      0x92
#define TMP95C063_B3CS      0x93
#define TMP95C063_MSAR0     0x94
#define TMP95C063_MAMR0     0x95
#define TMP95C063_MSAR1     0x96
#define TMP95C063_MAMR1     0x97
#define TMP95C063_MSAR2     0x98
#define TMP95C063_MAMR2     0x99
#define TMP95C063_MSAR3     0x9a
#define TMP95C063_MAMR3     0x9b
#define TMP95C063_DREFCR1   0x9c
#define TMP95C063_DMEMCR1   0x9d
#define TMP95C063_DREFCR3   0x9e
#define TMP95C063_DMEMCR3   0x9f


#define TMP95C063_NUM_MASKABLE_IRQS 30
static const struct {
	uint8_t reg;
	uint8_t iff;
	uint8_t vector;
} tmp95c063_irq_vector_map[TMP95C063_NUM_MASKABLE_IRQS] =
{
	{ TMP95C063_INTETC23, 0x80, 0xa0 },     /* INTTC3 */
	{ TMP95C063_INTETC23, 0x08, 0x9c },     /* INTTC2 */
	{ TMP95C063_INTETC01, 0x80, 0x98 },     /* INTTC1 */
	{ TMP95C063_INTETC01, 0x08, 0x94 },     /* INTTC0 */
	{ TMP95C063_INTE0AD, 0x80, 0x90 },      /* INTAD */
	{ TMP95C063_INTES1, 0x80, 0x8c },       /* INTTX1 */
	{ TMP95C063_INTES1, 0x08, 0x88 },       /* INTRX1 */
	{ TMP95C063_INTES0, 0x80, 0x84 },       /* INTTX0 */
	{ TMP95C063_INTES0, 0x08, 0x80 },       /* INTRX0 */
	{ TMP95C063_INTETAB, 0x80, 0x7c },      /* INTTRB */
	{ TMP95C063_INTETAB, 0x08, 0x78 },      /* INTTRA */
	{ TMP95C063_INTET89, 0x80, 0x74 },      /* INTTR9 */
	{ TMP95C063_INTET89, 0x80, 0x70 },      /* INTTR8 */
	{ TMP95C063_INTET67, 0x80, 0x6c },      /* INTT7 */
	{ TMP95C063_INTET67, 0x08, 0x68 },      /* INTT6 */
	{ TMP95C063_INTET45, 0x80, 0x64 },      /* INTT5 */
	{ TMP95C063_INTET45, 0x08, 0x60 },      /* INTT4 */
	{ TMP95C063_INTET23, 0x80, 0x5c },      /* INTT3 */
	{ TMP95C063_INTET23, 0x08, 0x58 },      /* INTT2 */
	{ TMP95C063_INTET01, 0x80, 0x54 },      /* INTT1 */
	{ TMP95C063_INTET01, 0x08, 0x50 },      /* INTT0 */
	{ TMP95C063_INTE78, 0x80, 0x4c },       /* int8_t */
	{ TMP95C063_INTE78, 0x08, 0x48 },       /* INT7 */
	{ TMP95C063_INTE56, 0x80, 0x44 },       /* INT6 */
	{ TMP95C063_INTE56, 0x08, 0x40 },       /* INT5 */
								/* 0x3c - reserved */
	{ TMP95C063_INTE34, 0x80, 0x38 },       /* INT4 */
	{ TMP95C063_INTE34, 0x08, 0x34 },       /* INT3 */
	{ TMP95C063_INTE12, 0x80, 0x30 },       /* INT2 */
	{ TMP95C063_INTE12, 0x08, 0x2c },       /* INT1 */
	{ TMP95C063_INTE0AD, 0x08, 0x28 }       /* INT0 */
};

void tmp95c063_device::tlcs900_handle_timers()
{
	// TODO: implement timers 4-7

	uint32_t  old_pre = m_timer_pre;

	/* Is the pre-scaler active */
	if ( m_reg[TMP95C063_T8RUN] & 0x80 )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( m_reg[TMP95C063_T8RUN] & 0x01 )
	{
		switch( m_reg[TMP95C063_T01MOD] & 0x03 )
		{
		case 0x00:  /* TIO */
			break;
		case 0x01:  /* T1 */
			m_timer_change[0] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[0] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[0] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[0] > 0; m_timer_change[0]-- )
		{
//printf("timer0 = %02x, TREG0 = %02x\n", m_timer[0], m_reg[TREG0] );
			m_timer[0] += 1;
			if ( m_timer[0] == m_reg[TMP95C063_TREG0] )
			{
				if ( ( m_reg[TMP95C063_T01MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TMP95C063_T01MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[0] = 0;
					m_reg[TMP95C063_INTET01] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( m_reg[TMP95C063_T8RUN] & 0x02 )
	{
		switch( ( m_reg[TMP95C063_T01MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO0TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[1] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[1] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[1] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[1] > 0; m_timer_change[1]-- )
		{
			m_timer[1] += 1;
			if ( m_timer[1] == m_reg[TMP95C063_TREG1] )
			{
				m_timer[1] = 0;
				m_reg[TMP95C063_INTET01] |= 0x80;

				if ( m_reg[TMP95C063_T02FFCR] & 0x02 )
				{
					//tlcs900_change_tff( 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( m_reg[TMP95C063_T01MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( m_reg[TMP95C063_T8RUN] & 0x04 )
	{
		switch( m_reg[TMP95C063_T23MOD] & 0x03 )
		{
		case 0x00:  /* invalid */
		case 0x01:  /* T1 */
			m_timer_change[2] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[2] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[2] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[2] > 0; m_timer_change[2]-- )
		{
			m_timer[2] += 1;
			if ( m_timer[2] == m_reg[TMP95C063_TREG2] )
			{
				if ( ( m_reg[TMP95C063_T23MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TMP95C063_T23MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[2] = 0;
					m_reg[TMP95C063_INTET23] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( m_reg[TMP95C063_T8RUN] & 0x08 )
	{
		switch( ( m_reg[TMP95C063_T23MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO2TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[3] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[3] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[3] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[3] > 0; m_timer_change[3]-- )
		{
			m_timer[3] += 1;
			if ( m_timer[3] == m_reg[TMP95C063_TREG3] )
			{
				m_timer[3] = 0;
				m_reg[TMP95C063_INTET23] |= 0x80;

				if ( m_reg[TMP95C063_T02FFCR] & 0x20 )
				{
					//tlcs900_change_tff( 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( m_reg[TMP95C063_T23MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[2] = 0;
				}
			}
		}
	}

	m_timer_pre &= 0xffffff;
}

void tmp95c063_device::tlcs900_check_hdma()
{
	// TODO
}

void tmp95c063_device::tlcs900_check_irqs()
{
	int irq_vectors[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int level = 0;
	int irq = -1;
	int i;

	/* Check for NMI */
	if ( m_nmi_state == ASSERT_LINE )
	{
		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );
		m_pc.d = RDMEML( 0xffff00 + 0x20 );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		m_nmi_state = CLEAR_LINE;

		return;
	}

	/* Check regular irqs */
	for( i = 0; i < TMP95C063_NUM_MASKABLE_IRQS; i++ )
	{
		if ( m_reg[tmp95c063_irq_vector_map[i].reg] & tmp95c063_irq_vector_map[i].iff )
		{
			switch( tmp95c063_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_reg[ tmp95c063_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_reg[ tmp95c063_irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	for ( i = std::max( 1, ( ( m_sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
	{
		if ( irq_vectors[i] >= 0 )
		{
			irq = irq_vectors[i];
			level = i + 1;
		}
	}

	/* Take irq */
	if ( irq >= 0 )
	{
		uint8_t vector = tmp95c063_irq_vector_map[irq].vector;

		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );

		/* Mask off any lower priority interrupts  */
		m_sr.b.h = ( m_sr.b.h & 0x8f ) | ( level << 4 );

		m_pc.d = RDMEML( 0xffff00 + vector );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		/* Clear taken IRQ */
		m_reg[ tmp95c063_irq_vector_map[irq].reg ] &= ~ tmp95c063_irq_vector_map[irq].iff;
	}
}


void tmp95c063_device::tlcs900_handle_ad()
{
	if ( m_ad_cycles_left > 0 )
	{
		m_ad_cycles_left -= m_cycles;
		if ( m_ad_cycles_left <= 0 )
		{
			int ad_value;

			/* Store A/D converted value */
			if ((m_reg[TMP95C063_ADMOD1] & 0x10) == 0)      // conversion channel fixed
			{
				switch( m_reg[TMP95C063_ADMOD2] & 0x07 )
				{
				case 0x00:  // AN0
					ad_value = m_an_read[0](0) & 0x3ff;
					m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
					break;
				case 0x01:  // AN1
					ad_value = m_an_read[1](0) & 0x3ff;
					m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
					break;
				case 0x02:  // AN2
					ad_value = m_an_read[2](0) & 0x3ff;
					m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
					break;
				case 0x03:  // AN3
					ad_value = m_an_read[3](0) & 0x3ff;
					m_reg[TMP95C063_ADREG37L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG37H] = (ad_value >> 2) & 0xff;
					break;
				case 0x04:  // AN4
					ad_value = m_an_read[4](0) & 0x3ff;
					m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
					break;
				case 0x05:  // AN5
					ad_value = m_an_read[5](0) & 0x3ff;
					m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
					break;
				case 0x06:  // AN6
					ad_value = m_an_read[6](0) & 0x3ff;
					m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
					break;
				case 0x07:  // AN7
					ad_value = m_an_read[7](0) & 0x3ff;
					m_reg[TMP95C063_ADREG37L] = (ad_value & 0x3) << 6;
					m_reg[TMP95C063_ADREG37H] = (ad_value >> 2) & 0xff;
					break;
				}
			}
			else            // conversion channel sweep
			{
				switch( m_reg[TMP95C063_ADMOD2] & 0x07 )
				{
					case 0x00:      // AN0
						ad_value = m_an_read[0](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						break;
					case 0x01:      // AN0 -> AN1
						ad_value = m_an_read[0](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[1](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						break;
					case 0x02:      // AN0 -> AN1 -> AN2
						ad_value = m_an_read[0](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[1](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[2](0) & 0x3ff;
						m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
						break;
					case 0x03:      // AN0 -> AN1 -> AN2 -> AN3
						ad_value = m_an_read[0](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[1](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[2](0) & 0x3ff;
						m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[3](0) & 0x3ff;
						m_reg[TMP95C063_ADREG37L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG37H] = (ad_value >> 2) & 0xff;
						break;
					case 0x04:      // AN4
						ad_value = m_an_read[4](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						break;
					case 0x05:      // AN4 -> AN5
						ad_value = m_an_read[4](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[5](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						break;
					case 0x06:      // AN4 -> AN5 -> AN6
						ad_value = m_an_read[4](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[5](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[6](0) & 0x3ff;
						m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
						break;
					case 0x07:      // AN4 -> AN5 -> AN6 -> AN7
						ad_value = m_an_read[4](0) & 0x3ff;
						m_reg[TMP95C063_ADREG04L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG04H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[5](0) & 0x3ff;
						m_reg[TMP95C063_ADREG15L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG15H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[6](0) & 0x3ff;
						m_reg[TMP95C063_ADREG26L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG26H] = (ad_value >> 2) & 0xff;
						ad_value = m_an_read[7](0) & 0x3ff;
						m_reg[TMP95C063_ADREG37L] = (ad_value & 0x3) << 6;
						m_reg[TMP95C063_ADREG37H] = (ad_value >> 2) & 0xff;
						break;
				}
			}

			/* Clear BUSY flag, set END flag */
			m_reg[TMP95C063_ADMOD1] &= ~ 0x40;
			m_reg[TMP95C063_ADMOD1] |= 0x80;

			m_reg[TMP95C063_INTE0AD] |= 0x80;
			m_check_irqs = 1;
		}
	}
}


void tmp95c063_device::device_start()
{
	tlcs900h_device::device_start();

	m_port1_read.resolve_safe(0);
	m_port1_write.resolve_safe();
	m_port2_write.resolve_safe();
	m_port5_read.resolve_safe(0);
	m_port5_write.resolve_safe();
	m_port6_read.resolve_safe(0);
	m_port6_write.resolve_safe();
	m_port7_read.resolve_safe(0);
	m_port7_write.resolve_safe();
	m_port8_read.resolve_safe(0);
	m_port8_write.resolve_safe();
	m_port9_read.resolve_safe(0);
	m_port9_write.resolve_safe();
	m_porta_read.resolve_safe(0);
	m_porta_write.resolve_safe();
	m_portb_read.resolve_safe(0);
	m_portb_write.resolve_safe();
	m_portc_read.resolve_safe(0);
	m_portd_read.resolve_safe(0);
	m_portd_write.resolve_safe();
	m_porte_read.resolve_safe(0);
	m_porte_write.resolve_safe();
	m_an_read.resolve_all_safe(0);
}

void tmp95c063_device::device_reset()
{
	m_pc.b.l = RDMEM( 0xFFFF00 );
	m_pc.b.h = RDMEM( 0xFFFF01 );
	m_pc.b.h2 = RDMEM( 0xFFFF02 );
	m_pc.b.h3 = 0;
	/* system mode, iff set to 111, max mode, register bank 0 */
	m_sr.d = 0xF800;
	m_regbank = 0;
	m_xssp.d = 0x0100;
	m_halted = 0;
	m_check_irqs = 0;
	m_ad_cycles_left = 0;
	m_nmi_state = CLEAR_LINE;
	m_timer_pre = 0;
	m_timer_change[0] = 0;
	m_timer_change[1] = 0;
	m_timer_change[2] = 0;
	m_timer_change[3] = 0;

	m_reg[TMP95C063_P1] = 0x00;
	m_reg[TMP95C063_P1CR] = 0x00;
	m_reg[TMP95C063_P2] = 0xff;
	m_reg[TMP95C063_P2FC] = 0x00;
	m_reg[TMP95C063_P5] = 0x3d;
	m_reg[TMP95C063_P5CR] = 0x00;
	m_reg[TMP95C063_P5FC] = 0x00;
	m_reg[TMP95C063_P6] = 0x3b;
	m_reg[TMP95C063_P6FC] = 0x00;
	m_reg[TMP95C063_P7] = 0xff;
	m_reg[TMP95C063_P7CR] = 0x00;
	m_reg[TMP95C063_P7FC] = 0x00;
	m_reg[TMP95C063_P8] = 0x3f;
	m_reg[TMP95C063_P8CR] = 0x00;
	m_reg[TMP95C063_P8FC] = 0x00;
	m_reg[TMP95C063_PA] = 0x0f;
	m_reg[TMP95C063_PACR] = 0x00;
	m_reg[TMP95C063_PAFC] = 0x00;
	m_reg[TMP95C063_PB] = 0xff;
	m_reg[TMP95C063_PBCR] = 0x00;
	m_reg[TMP95C063_PBFC] = 0x00;
	m_reg[TMP95C063_MSAR0] = 0xff;
	m_reg[TMP95C063_MSAR1] = 0xff;
	m_reg[TMP95C063_MSAR2] = 0xff;
	m_reg[TMP95C063_MSAR3] = 0xff;
	m_reg[TMP95C063_MAMR0] = 0xff;
	m_reg[TMP95C063_MAMR1] = 0xff;
	m_reg[TMP95C063_MAMR2] = 0xff;
	m_reg[TMP95C063_MAMR3] = 0xff;
	m_reg[TMP95C063_DREFCR1] = 0x00;
	m_reg[TMP95C063_DMEMCR1] = 0x80;
	m_reg[TMP95C063_DREFCR3] = 0x00;
	m_reg[TMP95C063_DMEMCR3] = 0x80;
	m_reg[TMP95C063_T01MOD] = 0x00;
	m_reg[TMP95C063_T23MOD] = 0x00;
	m_reg[TMP95C063_T02FFCR] = 0x00;
	m_reg[TMP95C063_T46FFCR] = 0x00;
	m_reg[TMP95C063_T8RUN] = 0x00;
	m_reg[TMP95C063_TRDC] = 0x00;
	m_reg[TMP95C063_T45MOD] = 0x20;
	m_reg[TMP95C063_T46FFCR] = 0x00;
	m_reg[TMP95C063_PG01CR] = 0x00;
	m_reg[TMP95C063_PG0REG] = 0x00;
	m_reg[TMP95C063_PG1REG] = 0x00;
	m_reg[TMP95C063_SC0MOD] = 0x00;
	m_reg[TMP95C063_SC0CR] = 0x00;
	m_reg[TMP95C063_BR0CR] = 0x00;
	m_reg[TMP95C063_SC1MOD] = 0x00;
	m_reg[TMP95C063_SC1CR] = 0x00;
	m_reg[TMP95C063_BR1CR] = 0x00;
	m_reg[TMP95C063_P8FC] = 0x00;
	m_reg[TMP95C063_ODE] = 0x00;
	m_reg[TMP95C063_ADMOD1] = 0x00;
	m_reg[TMP95C063_ADMOD2] = 0x00;
	m_reg[TMP95C063_ADREG04L] = 0x3f;
	m_reg[TMP95C063_ADREG04H] = 0x00;
	m_reg[TMP95C063_ADREG15L] = 0x3f;
	m_reg[TMP95C063_ADREG15H] = 0x00;
	m_reg[TMP95C063_ADREG26L] = 0x3f;
	m_reg[TMP95C063_ADREG26H] = 0x00;
	m_reg[TMP95C063_ADREG37L] = 0x3f;
	m_reg[TMP95C063_ADREG37H] = 0x00;
	m_reg[TMP95C063_WDMOD] = 0x80;

	for (int i = 0; i < TLCS900_NUM_INPUTS; i++)
		m_level[i] = CLEAR_LINE;

	m_prefetch_clear = true;
}


uint8_t tmp95c063_device::internal_r(offs_t offset)
{
	switch (offset)
	{
		case TMP95C063_P1: m_reg[offset] = m_port1_read(0); break;
		case TMP95C063_P5: m_reg[offset] = m_port5_read(0); break;
		case TMP95C063_P6: m_reg[offset] = m_port6_read(0); break;
		case TMP95C063_P7: m_reg[offset] = m_port7_read(0); break;
		case TMP95C063_P8: m_reg[offset] = m_port8_read(0); break;
		case TMP95C063_P9: m_reg[offset] = m_port9_read(0); break;
		case TMP95C063_PA: m_reg[offset] = m_porta_read(0); break;
		case TMP95C063_PB: m_reg[offset] = m_portb_read(0); break;
		case TMP95C063_PC: m_reg[offset] = m_portc_read(0); break;
		case TMP95C063_PD: m_reg[offset] = m_portd_read(0); break;
		case TMP95C063_PE: m_reg[offset] = m_porte_read(0); break;
	}
	return m_reg[ offset ];
}


void tmp95c063_device::internal_w(offs_t offset, uint8_t data)
{
	switch ( offset )
	{
	case TMP95C063_T8RUN:
		if ( ! ( data & 0x01 ) )
		{
			m_timer[0] = 0;
			m_timer_change[0] = 0;
		}
		if ( ! ( data & 0x02 ) )
		{
			m_timer[1] = 0;
			m_timer_change[1] = 0;
		}
		if ( ! ( data & 0x04 ) )
		{
			m_timer[2] = 0;
			m_timer_change[2] = 0;
		}
		if ( ! ( data & 0x08 ) )
		{
			m_timer[3] = 0;
			m_timer_change[3] = 0;
		}
		if ( ! ( data & 0x10 ) )
			m_timer[4] = 0;
		if ( ! ( data & 0x20 ) )
			m_timer[5] = 0;
		break;

	case TMP95C063_T02FFCR:
		switch( data & 0x0c )
		{
		case 0x00:
			//tlcs900_change_tff( 1, FF_INVERT );
			break;
		case 0x04:
			//tlcs900_change_tff( 1, FF_SET );
			break;
		case 0x08:
			//tlcs900_change_tff( 1, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			//tlcs900_change_tff( 3, FF_INVERT );
			break;
		case 0x40:
			//tlcs900_change_tff( 3, FF_SET );
			break;
		case 0x80:
			//tlcs900_change_tff( 3, FF_CLEAR );
			break;
		}
		break;

	case TMP95C063_T46FFCR:
		switch( data & 0x0c )
		{
		case 0x00:
			//tlcs900_change_tff( 5, FF_INVERT );
			break;
		case 0x04:
			//tlcs900_change_tff( 5, FF_SET );
			break;
		case 0x08:
			//tlcs900_change_tff( 5, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			//tlcs900_change_tff( 7, FF_INVERT );
			break;
		case 0x40:
			//tlcs900_change_tff( 7, FF_SET );
			break;
		case 0x80:
			//tlcs900_change_tff( 7, FF_CLEAR );
			break;
		}
		break;

	case TMP95C063_T8FFCR:
		switch( data & 0x03 )
		{
		case 0x00:
			//tlcs900_change_tff( 8, FF_INVERT );
			break;
		case 0x01:
			//tlcs900_change_tff( 8, FF_SET );
			break;
		case 0x02:
			//tlcs900_change_tff( 8, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			//tlcs900_change_tff( 9, FF_INVERT );
			break;
		case 0x40:
			//tlcs900_change_tff( 9, FF_SET );
			break;
		case 0x80:
			//tlcs900_change_tff( 9, FF_CLEAR );
			break;
		}
		break;

	case TMP95C063_T9FFCR:
		switch( data & 0x03 )
		{
		case 0x00:
			//tlcs900_change_tff( 0xa, FF_INVERT );
			break;
		case 0x01:
			//tlcs900_change_tff( 0xa, FF_SET );
			break;
		case 0x02:
			//tlcs900_change_tff( 0xa, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			//tlcs900_change_tff( 0xb, FF_INVERT );
			break;
		case 0x40:
			//tlcs900_change_tff( 0xb, FF_SET );
			break;
		case 0x80:
			//tlcs900_change_tff( 0xb, FF_CLEAR );
			break;
		}
		break;

	case TMP95C063_MSAR0:
	case TMP95C063_MAMR0:
	case TMP95C063_MSAR1:
	case TMP95C063_MAMR1:
		break;

	case TMP95C063_WDMOD:
	case TMP95C063_WDCR:
		break;

	case TMP95C063_INTE0AD:
	case TMP95C063_INTE12:
	case TMP95C063_INTE34:
	case TMP95C063_INTE56:
	case TMP95C063_INTE78:
	case TMP95C063_INTET01:
	case TMP95C063_INTET23:
	case TMP95C063_INTET45:
	case TMP95C063_INTET67:
	case TMP95C063_INTET89:
	case TMP95C063_INTETAB:
	case TMP95C063_INTES0:
	case TMP95C063_INTES1:
	case TMP95C063_INTETC01:
	case TMP95C063_INTETC23:
		if ( data & 0x80 )
			data = ( data & 0x7f ) | ( m_reg[offset] & 0x80 );
		if ( data & 0x08 )
			data = ( data & 0xf7 ) | ( m_reg[offset] & 0x08 );
		break;

	case TMP95C063_IIMC:
		break;

	case TMP95C063_ADMOD1:
		// conversion start
		if (data & 0x04)
		{
			data &= ~0x04;
			data |= 0x40;

			switch ((m_reg[TMP95C063_ADMOD2] >> 4) & 3)
			{
				case 0: m_ad_cycles_left = 160; break;
				case 1: m_ad_cycles_left = 320; break;
				case 2: m_ad_cycles_left = 640; break;
				case 3: m_ad_cycles_left = 1280; break;
			}
		}
		break;

	case TMP95C063_ADMOD2:
		break;

	default:
		break;
	}

	m_check_irqs = 1;
	m_reg[ offset ] = data;

	switch (offset)
	{
		case TMP95C063_P1: m_port1_write(0, data, 0xff); break;
		case TMP95C063_P2: m_port2_write(0, data, 0xff); break;
		case TMP95C063_P5: m_port5_write(0, data, 0xff); break;
		case TMP95C063_P6: m_port6_write(0, data, 0xff); break;
		case TMP95C063_P7: m_port7_write(0, data, 0xff); break;
		case TMP95C063_P8: m_port8_write(0, data, 0xff); break;
		case TMP95C063_P9: m_port9_write(0, data, 0xff); break;
		case TMP95C063_PA: m_porta_write(0, data, 0xff); break;
		case TMP95C063_PB: m_portb_write(0, data, 0xff); break;
		//case TMP95C063_PC: m_portc_write(0, data, 0xff); break;
		case TMP95C063_PD: m_portd_write(0, data, 0xff); break;
		case TMP95C063_PE: m_porte_write(0, data, 0xff); break;
	}
}


void tmp95c063_device::execute_set_input(int input, int level)
{
	switch( input )
	{
	case INPUT_LINE_NMI:
	case TLCS900_NMI:
		if ( m_level[TLCS900_NMI] == CLEAR_LINE && level == ASSERT_LINE )
		{
			m_nmi_state = level;
		}
		m_level[TLCS900_NMI] = level;
		break;

	case TLCS900_INTWD:
		break;

	case TLCS900_INT0:
		/* Is INT0 functionality enabled? */
		if (m_reg[TMP95C063_IIMC] & 0x04)
		{
			if (m_reg[TMP95C063_IIMC] & 0x02)
			{
				/* Rising edge detect */
				if (m_level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE)
				{
					/* Leave HALT state */
					m_halted = 0;
					m_reg[TMP95C063_INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if (level == ASSERT_LINE)
					m_reg[TMP95C063_INTE0AD] |= 0x08;
				else
					m_reg[TMP95C063_INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT1:
		if (m_level[TLCS900_INT1] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE12] |= 0x08;
		}
		else if (m_level[TLCS900_INT1] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE12] &= ~0x08;
		}
		m_level[TLCS900_INT1] = level;
		break;

	case TLCS900_INT2:
		if (m_level[TLCS900_INT2] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE12] |= 0x80;
		}
		else if (m_level[TLCS900_INT2] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE12] &= ~0x80;
		}
		m_level[TLCS900_INT2] = level;
		break;

	case TLCS900_INT3:
		if (m_level[TLCS900_INT3] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE34] |= 0x08;
		}
		else if (m_level[TLCS900_INT3] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE34] &= ~0x08;
		}
		m_level[TLCS900_INT3] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( m_reg[TMP95C063_PBCR] & 0x01 ) )
		{
			if ( m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TMP95C063_INTE34] |= 0x80;
			}
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( m_reg[TMP95C063_PBCR] & 0x02 ) )
		{
			if ( m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TMP95C063_INTE56] |= 0x08;
			}
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_INT6:
		if (m_level[TLCS900_INT6] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE56] |= 0x80;
		}
		else if (m_level[TLCS900_INT6] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE56] &= ~0x80;
		}
		m_level[TLCS900_INT6] = level;
		break;

	case TLCS900_TIO:   /* External timer input for timer 0 */
		if ( ( m_reg[TMP95C063_T8RUN] & 0x01 ) && ( m_reg[TMP95C063_T01MOD] & 0x03 ) == 0x00 )
		{
			if ( m_level[TLCS900_TIO] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_timer_change[0] += 1;
			}
		}
		m_level[TLCS900_TIO] = level;
		break;
	}
	m_check_irqs = 1;
}
