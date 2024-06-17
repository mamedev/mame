// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

Toshiba TMP95C063 emulation

*******************************************************************/

#include "emu.h"
#include "tmp95c063.h"
#include "dasm900.h"

DEFINE_DEVICE_TYPE(TMP95C063, tmp95c063_device, "tmp95c063", "Toshiba TMP95C063")


tmp95c063_device::tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tlcs900h_device(mconfig, TMP95C063, tag, owner, clock),
	m_port1_read(*this, 0),
	m_port1_write(*this),
	m_port2_write(*this),
	m_port5_read(*this, 0),
	m_port5_write(*this),
	m_port6_read(*this, 0),
	m_port6_write(*this),
	m_port7_read(*this, 0),
	m_port7_write(*this),
	m_port8_read(*this, 0),
	m_port8_write(*this),
	m_port9_read(*this, 0),
	m_port9_write(*this),
	m_porta_read(*this, 0),
	m_porta_write(*this),
	m_portb_read(*this, 0),
	m_portb_write(*this),
	m_portc_read(*this, 0),
	m_portd_read(*this, 0),
	m_portd_write(*this),
	m_porte_read(*this, 0),
	m_porte_write(*this),
	m_an_read(*this, 0),
	m_port_latch{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_control{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_function{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t8run(0),
	m_t8_reg{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t8_mode{ 0, 0, 0, 0 },
	m_t8_invert{ 0, 0 },
	m_trdc(0),
	m_t16_reg{ 0, 0, 0, 0 },
	m_t16_cap{ 0, 0, 0, 0 },
	m_t16_mode{ 0, 0 },
	m_t16_invert{ 0, 0 },
	m_t89cr(0),
	m_t16run(0),
	m_pgreg{ 0, 0 },
	m_pg01cr(0),
	m_watchdog_mode(0),
	m_serial_control{ 0, 0 },
	m_serial_mode{ 0, 0 },
	m_baud_rate{ 0, 0 },
	m_od_enable(0),
	m_ad_mode1(0),
	m_ad_mode2(0),
	m_ad_result{ 0, 0, 0, 0 },
	m_int_reg{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_iimc(0),
	m_dma_vector{ 0, 0, 0, 0 },
	m_block_cs{ 0, 0, 0, 0 },
	m_external_cs(0),
	m_mem_start_reg{ 0, 0, 0, 0 },
	m_mem_start_mask{ 0, 0, 0, 0 },
	m_dram_refresh{ 0, 0 },
	m_dram_access{ 0, 0 },
	m_da_drive(0)
{
}

void tmp95c063_device::internal_mem(address_map &map)
{
	map(0x000001, 0x000001).rw(FUNC(tmp95c063_device::p1_r), FUNC(tmp95c063_device::p1_w));
	map(0x000004, 0x000004).w(FUNC(tmp95c063_device::p1cr_w));
	map(0x000006, 0x000006).rw(FUNC(tmp95c063_device::p2_r), FUNC(tmp95c063_device::p2_w));
	map(0x000009, 0x000009).w(FUNC(tmp95c063_device::p2fc_w));
	map(0x00000d, 0x00000d).rw(FUNC(tmp95c063_device::p5_r), FUNC(tmp95c063_device::p5_w));
	map(0x000010, 0x000010).w(FUNC(tmp95c063_device::p5cr_w));
	map(0x000011, 0x000011).w(FUNC(tmp95c063_device::p5fc_w));
	map(0x000012, 0x000012).rw(FUNC(tmp95c063_device::p6_r), FUNC(tmp95c063_device::p6_w));
	map(0x000013, 0x000013).rw(FUNC(tmp95c063_device::p7_r), FUNC(tmp95c063_device::p7_w));
	map(0x000015, 0x000015).w(FUNC(tmp95c063_device::p6fc_w));
	map(0x000016, 0x000016).w(FUNC(tmp95c063_device::p7cr_w));
	map(0x000017, 0x000017).w(FUNC(tmp95c063_device::p7fc_w));
	map(0x000018, 0x000018).rw(FUNC(tmp95c063_device::p8_r), FUNC(tmp95c063_device::p8_w));
	map(0x000019, 0x000019).rw(FUNC(tmp95c063_device::p9_r), FUNC(tmp95c063_device::p9_w));
	map(0x00001a, 0x00001a).w(FUNC(tmp95c063_device::p8cr_w));
	map(0x00001b, 0x00001b).w(FUNC(tmp95c063_device::p8fc_w));
	map(0x00001c, 0x00001c).w(FUNC(tmp95c063_device::p9cr_w));
	map(0x00001d, 0x00001d).w(FUNC(tmp95c063_device::p9fc_w));
	map(0x00001e, 0x00001e).rw(FUNC(tmp95c063_device::pa_r), FUNC(tmp95c063_device::pa_w));
	map(0x00001f, 0x00001f).rw(FUNC(tmp95c063_device::pb_r), FUNC(tmp95c063_device::pb_w));
	map(0x000020, 0x000020).rw(FUNC(tmp95c063_device::t8run_r), FUNC(tmp95c063_device::t8run_w));
	map(0x000021, 0x000021).rw(FUNC(tmp95c063_device::trdc_r), FUNC(tmp95c063_device::trdc_w));
	map(0x000022, 0x000023).w(FUNC(tmp95c063_device::treg01_w));
	map(0x000024, 0x000024).rw(FUNC(tmp95c063_device::t01mod_r), FUNC(tmp95c063_device::t01mod_w));
	map(0x000025, 0x000025).rw(FUNC(tmp95c063_device::t02ffcr_r), FUNC(tmp95c063_device::t02ffcr_w));
	map(0x000026, 0x000027).w(FUNC(tmp95c063_device::treg23_w));
	map(0x000028, 0x000028).rw(FUNC(tmp95c063_device::t23mod_r), FUNC(tmp95c063_device::t23mod_w));
	map(0x000029, 0x00002a).w(FUNC(tmp95c063_device::treg45_w));
	map(0x00002b, 0x00002b).rw(FUNC(tmp95c063_device::t45mod_r), FUNC(tmp95c063_device::t45mod_w));
	map(0x00002c, 0x00002c).rw(FUNC(tmp95c063_device::t46ffcr_r), FUNC(tmp95c063_device::t46ffcr_w));
	map(0x00002d, 0x00002e).w(FUNC(tmp95c063_device::treg67_w));
	map(0x00002f, 0x00002f).rw(FUNC(tmp95c063_device::t67mod_r), FUNC(tmp95c063_device::t67mod_w));
	map(0x000030, 0x000033).w(FUNC(tmp95c063_device::treg89_w));
	map(0x000034, 0x000037).r(FUNC(tmp95c063_device::cap12_r));
	map(0x000038, 0x000038).rw(FUNC(tmp95c063_device::t8mod_r), FUNC(tmp95c063_device::t8mod_w));
	map(0x000039, 0x000039).rw(FUNC(tmp95c063_device::t8ffcr_r), FUNC(tmp95c063_device::t8ffcr_w));
	map(0x00003a, 0x00003a).rw(FUNC(tmp95c063_device::t89cr_r), FUNC(tmp95c063_device::t89cr_w));
	map(0x00003b, 0x00003b).rw(FUNC(tmp95c063_device::t16run_r), FUNC(tmp95c063_device::t16run_w));
	map(0x000040, 0x000043).w(FUNC(tmp95c063_device::tregab_w));
	map(0x000044, 0x000047).r(FUNC(tmp95c063_device::cap34_r));
	map(0x000048, 0x000048).rw(FUNC(tmp95c063_device::t9mod_r), FUNC(tmp95c063_device::t9mod_w));
	map(0x000049, 0x000049).rw(FUNC(tmp95c063_device::t9ffcr_r), FUNC(tmp95c063_device::t9ffcr_w));
	map(0x00004a, 0x00004b).w(FUNC(tmp95c063_device::dareg_w));
	map(0x00004c, 0x00004d).rw(FUNC(tmp95c063_device::pgreg_r), FUNC(tmp95c063_device::pgreg_w));
	map(0x00004e, 0x00004e).rw(FUNC(tmp95c063_device::pg01cr_r), FUNC(tmp95c063_device::pg01cr_w));
	map(0x00004f, 0x00004f).rw(FUNC(tmp95c063_device::dadrv_r), FUNC(tmp95c063_device::dadrv_w));
	map(0x000050, 0x000050).rw(FUNC(tmp95c063_device::sc0buf_r), FUNC(tmp95c063_device::sc0buf_w));
	map(0x000051, 0x000051).rw(FUNC(tmp95c063_device::sc0cr_r), FUNC(tmp95c063_device::sc0cr_w));
	map(0x000052, 0x000052).rw(FUNC(tmp95c063_device::sc0mod_r), FUNC(tmp95c063_device::sc0mod_w));
	map(0x000053, 0x000053).rw(FUNC(tmp95c063_device::br0cr_r), FUNC(tmp95c063_device::br0cr_w));
	map(0x000054, 0x000054).rw(FUNC(tmp95c063_device::sc1buf_r), FUNC(tmp95c063_device::sc1buf_w));
	map(0x000055, 0x000055).rw(FUNC(tmp95c063_device::sc1cr_r), FUNC(tmp95c063_device::sc1cr_w));
	map(0x000056, 0x000056).rw(FUNC(tmp95c063_device::sc1mod_r), FUNC(tmp95c063_device::sc1mod_w));
	map(0x000057, 0x000057).rw(FUNC(tmp95c063_device::br1cr_r), FUNC(tmp95c063_device::br1cr_w));
	map(0x000058, 0x000058).rw(FUNC(tmp95c063_device::ode_r), FUNC(tmp95c063_device::ode_w));
	map(0x00005a, 0x00005d).w(FUNC(tmp95c063_device::dmav_w));
	map(0x00005e, 0x00005e).rw(FUNC(tmp95c063_device::admod1_r), FUNC(tmp95c063_device::admod1_w));
	map(0x00005f, 0x00005f).rw(FUNC(tmp95c063_device::admod2_r), FUNC(tmp95c063_device::admod2_w));
	map(0x000060, 0x000067).r(FUNC(tmp95c063_device::adreg_r));
	//map(0x00006a, 0x00006d).rw(FUNC(tmp95c063_device::sdmacr_r), FUNC(tmp95c063_device::sdmacr_w));
	map(0x00006e, 0x00006e).rw(FUNC(tmp95c063_device::wdmod_r), FUNC(tmp95c063_device::wdmod_w));
	map(0x00006f, 0x00006f).w(FUNC(tmp95c063_device::wdcr_w));
	map(0x000070, 0x00007e).rw(FUNC(tmp95c063_device::inte_r), FUNC(tmp95c063_device::inte_w));
	map(0x00007f, 0x00007f).w(FUNC(tmp95c063_device::iimc_w));
	map(0x000080, 0x000080).w(FUNC(tmp95c063_device::pacr_w));
	map(0x000081, 0x000081).w(FUNC(tmp95c063_device::pafc_w));
	map(0x000082, 0x000082).w(FUNC(tmp95c063_device::pbcr_w));
	map(0x000083, 0x000083).w(FUNC(tmp95c063_device::pbfc_w));
	map(0x000084, 0x000084).r(FUNC(tmp95c063_device::pc_r));
	map(0x000085, 0x000085).rw(FUNC(tmp95c063_device::pd_r), FUNC(tmp95c063_device::pd_w));
	map(0x000088, 0x000088).w(FUNC(tmp95c063_device::pdcr_w));
	map(0x00008a, 0x00008a).rw(FUNC(tmp95c063_device::pe_r), FUNC(tmp95c063_device::pe_w));
	map(0x00008c, 0x00008c).w(FUNC(tmp95c063_device::pecr_w));
	map(0x00008f, 0x00008f).w(FUNC(tmp95c063_device::bexcs_w));
	map(0x000090, 0x000093).w(FUNC(tmp95c063_device::bcs_w));
	map(0x000094, 0x00009b).rw(FUNC(tmp95c063_device::msar_r), FUNC(tmp95c063_device::msar_w));
	map(0x00009c, 0x00009c).rw(FUNC(tmp95c063_device::drefcr1_r), FUNC(tmp95c063_device::drefcr1_w));
	map(0x00009d, 0x00009d).rw(FUNC(tmp95c063_device::dmemcr1_r), FUNC(tmp95c063_device::dmemcr1_w));
	map(0x00009e, 0x00009e).rw(FUNC(tmp95c063_device::drefcr3_r), FUNC(tmp95c063_device::drefcr3_w));
	map(0x00009f, 0x00009f).rw(FUNC(tmp95c063_device::dmemcr3_r), FUNC(tmp95c063_device::dmemcr3_w));
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



#define TMP95C063_INTE0AD   0x0
#define TMP95C063_INTE12    0x1
#define TMP95C063_INTE34    0x2
#define TMP95C063_INTE56    0x3
#define TMP95C063_INTE78    0x4
#define TMP95C063_INTET01   0x5
#define TMP95C063_INTET23   0x6
#define TMP95C063_INTET45   0x7
#define TMP95C063_INTET67   0x8
#define TMP95C063_INTET89   0x9
#define TMP95C063_INTETAB   0xa
#define TMP95C063_INTES0    0xb
#define TMP95C063_INTES1    0xc
#define TMP95C063_INTETC01  0xd
#define TMP95C063_INTETC23  0xe


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
	if ( m_t8run & 0x80 )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( m_t8run & 0x01 )
	{
		switch( m_t8_mode[0] & 0x03 )
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
			m_timer[0] += 1;
			if ( m_timer[0] == m_t8_reg[0] )
			{
				if ( ( m_t8run & 0x02 ) && ( m_t8_mode[0] & 0x0c ) == 0x00 )
				{
					m_timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_t8_mode[0] & 0xc0 ) != 0x40 )
				{
					m_timer[0] = 0;
					m_int_reg[TMP95C063_INTET01] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( m_t8run & 0x02 )
	{
		switch( ( m_t8_mode[0] >> 2 ) & 0x03 )
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
			if ( m_timer[1] == m_t8_reg[1] )
			{
				m_timer[1] = 0;
				m_int_reg[TMP95C063_INTET01] |= 0x80;

				if ( m_t8_invert[0] & 0x02 )
				{
					//tlcs900_change_tff( 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( m_t8_mode[0] & 0xc0 ) == 0x40 )
				{
					m_timer[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( m_t8run & 0x04 )
	{
		switch( m_t8_mode[1] & 0x03 )
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
			if ( m_timer[2] == m_t8_reg[2] )
			{
				if ( ( m_t8run & 0x08 ) && ( m_t8_mode[1] & 0x0c ) == 0x00 )
				{
					m_timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_t8_mode[1] & 0xc0 ) != 0x40 )
				{
					m_timer[2] = 0;
					m_int_reg[TMP95C063_INTET23] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( m_t8run & 0x08 )
	{
		switch( ( m_t8_mode[1] >> 2 ) & 0x03 )
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
			if ( m_timer[3] == m_t8_reg[3] )
			{
				m_timer[3] = 0;
				m_int_reg[TMP95C063_INTET23] |= 0x80;

				if ( m_t8_invert[1] & 0x20 )
				{
					//tlcs900_change_tff( 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( m_t8_mode[1] & 0xc0 ) == 0x40 )
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
		if ( m_int_reg[tmp95c063_irq_vector_map[i].reg] & tmp95c063_irq_vector_map[i].iff )
		{
			switch( tmp95c063_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_int_reg[ tmp95c063_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_int_reg[ tmp95c063_irq_vector_map[i].reg ] & 0x07 ] = i;
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
		m_int_reg[ tmp95c063_irq_vector_map[irq].reg ] &= ~ tmp95c063_irq_vector_map[irq].iff;
	}
}


void tmp95c063_device::tlcs900_handle_ad()
{
	if ( m_ad_cycles_left > 0 )
	{
		m_ad_cycles_left -= m_cycles;
		if ( m_ad_cycles_left <= 0 )
		{
			/* Store A/D converted value */
			if ((m_ad_mode1 & 0x10) == 0)      // conversion channel fixed
			{
				m_ad_result[m_ad_mode2 & 0x03] = m_an_read[m_ad_mode2 & 0x07](0) & 0x3ff;
			}
			else            // conversion channel sweep
			{
				switch( m_ad_mode2 & 0x07 )
				{
					case 0x00:      // AN0
						m_ad_result[0] = m_an_read[0](0) & 0x3ff;
						break;
					case 0x01:      // AN0 -> AN1
						m_ad_result[0] = m_an_read[0](0) & 0x3ff;
						m_ad_result[1] = m_an_read[1](0) & 0x3ff;
						break;
					case 0x02:      // AN0 -> AN1 -> AN2
						m_ad_result[0] = m_an_read[0](0) & 0x3ff;
						m_ad_result[1] = m_an_read[1](0) & 0x3ff;
						m_ad_result[2] = m_an_read[2](0) & 0x3ff;
						break;
					case 0x03:      // AN0 -> AN1 -> AN2 -> AN3
						m_ad_result[0] = m_an_read[0](0) & 0x3ff;
						m_ad_result[1] = m_an_read[1](0) & 0x3ff;
						m_ad_result[2] = m_an_read[2](0) & 0x3ff;
						m_ad_result[3] = m_an_read[3](0) & 0x3ff;
						break;
					case 0x04:      // AN4
						m_ad_result[0] = m_an_read[4](0) & 0x3ff;
						break;
					case 0x05:      // AN4 -> AN5
						m_ad_result[0] = m_an_read[4](0) & 0x3ff;
						m_ad_result[1] = m_an_read[5](0) & 0x3ff;
						break;
					case 0x06:      // AN4 -> AN5 -> AN6
						m_ad_result[0] = m_an_read[4](0) & 0x3ff;
						m_ad_result[1] = m_an_read[5](0) & 0x3ff;
						m_ad_result[2] = m_an_read[6](0) & 0x3ff;
						break;
					case 0x07:      // AN4 -> AN5 -> AN6 -> AN7
						m_ad_result[0] = m_an_read[4](0) & 0x3ff;
						m_ad_result[1] = m_an_read[5](0) & 0x3ff;
						m_ad_result[2] = m_an_read[6](0) & 0x3ff;
						m_ad_result[3] = m_an_read[7](0) & 0x3ff;
						break;
				}
			}

			/* Clear BUSY flag, set END flag */
			m_ad_mode1 &= ~ 0x40;
			m_ad_mode1 |= 0x80;

			m_int_reg[TMP95C063_INTE0AD] |= 0x80;
			m_check_irqs = 1;
		}
	}
}


void tmp95c063_device::device_start()
{
	tlcs900h_device::device_start();

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_control));
	save_item(NAME(m_port_function));
	save_item(NAME(m_t8run));
	save_item(NAME(m_t8_reg));
	save_item(NAME(m_t8_mode));
	save_item(NAME(m_t8_invert));
	save_item(NAME(m_trdc));
	save_item(NAME(m_t16_reg));
	save_item(NAME(m_t16_cap));
	save_item(NAME(m_t16_mode));
	save_item(NAME(m_t16_invert));
	save_item(NAME(m_t89cr));
	save_item(NAME(m_t16run));
	save_item(NAME(m_pgreg));
	save_item(NAME(m_pg01cr));
	save_item(NAME(m_watchdog_mode));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_serial_mode));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_od_enable));
	save_item(NAME(m_ad_mode1));
	save_item(NAME(m_ad_mode2));
	save_item(NAME(m_ad_result));
	save_item(NAME(m_int_reg));
	save_item(NAME(m_iimc));
	save_item(NAME(m_dma_vector));
	save_item(NAME(m_block_cs));
	save_item(NAME(m_external_cs));
	save_item(NAME(m_mem_start_reg));
	save_item(NAME(m_mem_start_mask));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_dram_access));
	save_item(NAME(m_da_drive));

	m_nmi_state = CLEAR_LINE;
	for( int i = 0; i < TLCS900_NUM_INPUTS; i++ )
	{
		m_level[i] = CLEAR_LINE;
	}
}

void tmp95c063_device::device_reset()
{
	tlcs900h_device::device_reset();

	m_ad_cycles_left = 0;
	m_timer_pre = 0;
	m_timer_change[0] = 0;
	m_timer_change[1] = 0;
	m_timer_change[2] = 0;
	m_timer_change[3] = 0;

	m_port_latch[1] = 0x00;
	m_port_latch[2] = 0xff;
	m_port_latch[5] = 0x3d;
	m_port_latch[6] = 0x3b;
	m_port_latch[7] = 0xff;
	m_port_latch[8] = 0x3f;
	m_port_latch[0xa] = 0x0f;
	m_port_latch[0xb] = 0xff;
	std::fill_n(&m_port_control[0], 0xc, 0x00);
	std::fill_n(&m_port_function[0], 0xc, 0x00);
	m_t8run = 0x00;
	std::fill_n(&m_t8_mode[0], 4, 0x00);
	std::fill_n(&m_t8_invert[0], 2, 0xcc);
	m_trdc = 0x00;
	std::fill_n(&m_t16_mode[0], 2, 0x20);
	std::fill_n(&m_t16_invert[0], 2, 0x00);
	m_t89cr = 0x00;
	m_t16run = 0x00;
	m_pgreg[0] &= 0x0f;
	m_pgreg[1] &= 0x0f;
	m_pg01cr = 0x00;
	m_watchdog_mode = 0x80;
	for (int i = 0; i < 2; i++)
	{
		m_serial_control[i] &= 0x80;
		m_serial_mode[i] &= 0x80;
		m_baud_rate[i] = 0x00;
	}
	m_od_enable = 0x00;
	m_ad_mode1 = 0x00;
	m_ad_mode2 = 0x00;
	std::fill_n(&m_int_reg[0], 0xf, 0x00);
	m_iimc = 0x00;
	std::fill_n(&m_dma_vector[0], 4, 0x00);
	m_block_cs[0] = 0x00;
	m_block_cs[1] = 0x00;
	m_block_cs[2] = 0x10;
	m_block_cs[3] = 0x00;
	m_external_cs = 0x00;
	std::fill_n(&m_mem_start_reg[0], 4, 0xff);
	std::fill_n(&m_mem_start_mask[0], 4, 0xff);
	std::fill_n(&m_dram_refresh[0], 2, 0x00);
	std::fill_n(&m_dram_access[0], 2, 0x80);
	m_da_drive = 0x00;
}


uint8_t tmp95c063_device::p1_r()
{
	return m_port1_read(0);
}

void tmp95c063_device::p1_w(uint8_t data)
{
	m_port_latch[1] = data;
	m_port1_write(0, data, 0xff);
}

void tmp95c063_device::p1cr_w(uint8_t data)
{
	m_port_control[1] = data;
}

uint8_t tmp95c063_device::p2_r()
{
	return m_port_latch[2];
}

void tmp95c063_device::p2_w(uint8_t data)
{
	m_port_latch[2] = data;
	m_port2_write(0, data, 0xff);
}

void tmp95c063_device::p2fc_w(uint8_t data)
{
	m_port_control[2] = data;
}

uint8_t tmp95c063_device::p5_r()
{
	return m_port5_read(0);
}

void tmp95c063_device::p5_w(uint8_t data)
{
	m_port_latch[5] = data;
	m_port5_write(0, data, 0xff);
}

void tmp95c063_device::p5cr_w(uint8_t data)
{
	m_port_control[5] = data;
}

void tmp95c063_device::p5fc_w(uint8_t data)
{
	m_port_function[5] = data;
}

uint8_t tmp95c063_device::p6_r()
{
	return m_port6_read(0);
}

void tmp95c063_device::p6_w(uint8_t data)
{
	m_port_latch[6] = data;
	m_port6_write(0, data, 0xff);
}

void tmp95c063_device::p6fc_w(uint8_t data)
{
	m_port_function[6] = data;
}

uint8_t tmp95c063_device::p7_r()
{
	return m_port7_read(0);
}

void tmp95c063_device::p7_w(uint8_t data)
{
	m_port_latch[7] = data;
	m_port7_write(0, data, 0xff);
}

void tmp95c063_device::p7cr_w(uint8_t data)
{
	m_port_control[7] = data;
}

void tmp95c063_device::p7fc_w(uint8_t data)
{
	m_port_function[7] = data;
}

uint8_t tmp95c063_device::p8_r()
{
	return m_port8_read(0);
}

void tmp95c063_device::p8_w(uint8_t data)
{
	m_port_latch[8] = data;
	m_port8_write(0, data, 0xff);
}

void tmp95c063_device::p8cr_w(uint8_t data)
{
	m_port_control[8] = data;
}

void tmp95c063_device::p8fc_w(uint8_t data)
{
	m_port_function[8] = data;
}

uint8_t tmp95c063_device::p9_r()
{
	return m_port9_read(0);
}

void tmp95c063_device::p9_w(uint8_t data)
{
	m_port_latch[0x9] = data;
}

void tmp95c063_device::p9cr_w(uint8_t data)
{
	m_port_control[0x9] = data;
}

void tmp95c063_device::p9fc_w(uint8_t data)
{
	m_port_function[0x9] = data;
}

uint8_t tmp95c063_device::pa_r()
{
	return m_porta_read(0);
}

void tmp95c063_device::pa_w(uint8_t data)
{
	m_port_latch[0xa] = data;
}

void tmp95c063_device::pacr_w(uint8_t data)
{
	m_port_control[0xa] = data;
}

void tmp95c063_device::pafc_w(uint8_t data)
{
	m_port_function[0xa] = data;
}

uint8_t tmp95c063_device::pb_r()
{
	return m_portb_read(0);
}

void tmp95c063_device::pb_w(uint8_t data)
{
	m_port_latch[0xb] = data;
	m_portb_write(0, data, 0xff);
}

void tmp95c063_device::pbcr_w(uint8_t data)
{
	m_port_control[0xb] = data;
}

void tmp95c063_device::pbfc_w(uint8_t data)
{
	m_port_function[0xb] = data;
}

uint8_t tmp95c063_device::pc_r()
{
	return m_portc_read(0);
}

uint8_t tmp95c063_device::pd_r()
{
	return m_portd_read(0);
}

void tmp95c063_device::pd_w(uint8_t data)
{
	m_port_latch[0xd] = data;
	m_portd_write(0, data, 0xff);
}

void tmp95c063_device::pdcr_w(uint8_t data)
{
	m_port_control[0xd] = data;
}

uint8_t tmp95c063_device::pe_r()
{
	return m_porte_read(0);
}

void tmp95c063_device::pe_w(uint8_t data)
{
	m_port_latch[0xe] = data;
	m_porte_write(0, data, 0xff);
}

void tmp95c063_device::pecr_w(uint8_t data)
{
	m_port_control[0xe] = data;
}


uint8_t tmp95c063_device::t8run_r()
{
	return m_t8run;
}

void tmp95c063_device::t8run_w(uint8_t data)
{
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

	m_t8run = data;
}

void tmp95c063_device::treg01_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset] = data;
}

uint8_t tmp95c063_device::t01mod_r()
{
	return m_t8_mode[0];
}

void tmp95c063_device::t01mod_w(uint8_t data)
{
	m_t8_mode[0] = data;
}

uint8_t tmp95c063_device::t02ffcr_r()
{
	return m_t8_invert[0];
}

void tmp95c063_device::t02ffcr_w(uint8_t data)
{
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

	m_t8_invert[0] = data | 0xcc;
}

void tmp95c063_device::treg23_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset + 2] = data;
}

uint8_t tmp95c063_device::t23mod_r()
{
	return m_t8_mode[1];
}

void tmp95c063_device::t23mod_w(uint8_t data)
{
	m_t8_mode[1] = data;
}

uint8_t tmp95c063_device::trdc_r()
{
	return m_trdc;
}

void tmp95c063_device::trdc_w(uint8_t data)
{
	m_trdc = data;
}

void tmp95c063_device::treg45_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset + 4] = data;
}

uint8_t tmp95c063_device::t45mod_r()
{
	return m_t8_mode[2];
}

void tmp95c063_device::t45mod_w(uint8_t data)
{
	m_t8_mode[2] = data;
}

uint8_t tmp95c063_device::t46ffcr_r()
{
	return m_t8_invert[1];
}

void tmp95c063_device::t46ffcr_w(uint8_t data)
{
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

	m_t8_invert[1] = data | 0xcc;
}

void tmp95c063_device::treg67_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset + 6] = data;
}

uint8_t tmp95c063_device::t67mod_r()
{
	return m_t8_mode[3];
}

void tmp95c063_device::t67mod_w(uint8_t data)
{
	m_t8_mode[3] = data;
}

void tmp95c063_device::treg89_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_t16_reg[offset >> 1] = (m_t16_reg[offset >> 1] & 0x00ff) | uint16_t(data) << 8;
	else
		m_t16_reg[offset >> 1] = (m_t16_reg[offset >> 1] & 0xff00) | data;
}

uint8_t tmp95c063_device::cap12_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[offset >> 1] >> 8;
	else
		return m_t16_cap[offset >> 1] & 0x00ff;
}

uint8_t tmp95c063_device::t8mod_r()
{
	return m_t16_mode[0];
}

void tmp95c063_device::t8mod_w(uint8_t data)
{
	m_t16_mode[0] = data | 0x20;
}

uint8_t tmp95c063_device::t8ffcr_r()
{
	return m_t16_invert[0];
}

void tmp95c063_device::t8ffcr_w(uint8_t data)
{
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

	m_t16_invert[0] = data | 0xc3;
}

uint8_t tmp95c063_device::t89cr_r()
{
	return m_t89cr;
}

void tmp95c063_device::t89cr_w(uint8_t data)
{
	m_t89cr = data;
}

uint8_t tmp95c063_device::t16run_r()
{
	return m_t16run;
}

void tmp95c063_device::t16run_w(uint8_t data)
{
	m_t16run = data;
}

void tmp95c063_device::tregab_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_t16_reg[(offset >> 1) + 2] = (m_t16_reg[(offset >> 1) + 2] & 0x00ff) | uint16_t(data) << 8;
	else
		m_t16_reg[(offset >> 1) + 2] = (m_t16_reg[(offset >> 1) + 2] & 0xff00) | data;
}

uint8_t tmp95c063_device::cap34_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 2] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 2] & 0x00ff;
}

uint8_t tmp95c063_device::t9mod_r()
{
	return m_t16_mode[1];
}

void tmp95c063_device::t9mod_w(uint8_t data)
{
	m_t16_mode[1] = data | 0x20;
}

uint8_t tmp95c063_device::t9ffcr_r()
{
	return m_t16_invert[1];
}

void tmp95c063_device::t9ffcr_w(uint8_t data)
{
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

	m_t16_invert[1] = data | 0xc3;
}


uint8_t tmp95c063_device::pgreg_r(offs_t offset)
{
	return m_pgreg[offset];
}

void tmp95c063_device::pgreg_w(offs_t offset, uint8_t data)
{
	m_pgreg[offset] = data;
}

uint8_t tmp95c063_device::pg01cr_r()
{
	return m_pg01cr;
}

void tmp95c063_device::pg01cr_w(uint8_t data)
{
	m_pg01cr = data;
}


uint8_t tmp95c063_device::wdmod_r()
{
	return m_watchdog_mode;
}

void tmp95c063_device::wdmod_w(uint8_t data)
{
	m_watchdog_mode = data;
}

void tmp95c063_device::wdcr_w(uint8_t data)
{
}


uint8_t tmp95c063_device::sc0buf_r()
{
	return 0;
}

void tmp95c063_device::sc0buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[TMP95C063_INTES0] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp95c063_device::sc0cr_r()
{
	uint8_t reg = m_serial_control[0];
	if (!machine().side_effects_disabled())
		m_serial_control[0] &= 0xe3;
	return reg;
}

void tmp95c063_device::sc0cr_w(uint8_t data)
{
	m_serial_control[0] = data;
}

uint8_t tmp95c063_device::sc0mod_r()
{
	return m_serial_mode[0];
}

void tmp95c063_device::sc0mod_w(uint8_t data)
{
	m_serial_mode[0] = data;
}

uint8_t tmp95c063_device::br0cr_r()
{
	return m_baud_rate[0];
}

void tmp95c063_device::br0cr_w(uint8_t data)
{
	m_baud_rate[0] = data;
}

uint8_t tmp95c063_device::sc1buf_r()
{
	return 0;
}

void tmp95c063_device::sc1buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[TMP95C063_INTES1] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp95c063_device::sc1cr_r()
{
	uint8_t reg = m_serial_control[1];
	if (!machine().side_effects_disabled())
		m_serial_control[1] &= 0xe3;
	return reg;
}

void tmp95c063_device::sc1cr_w(uint8_t data)
{
	m_serial_control[1] = data;
}

uint8_t tmp95c063_device::sc1mod_r()
{
	return m_serial_mode[1];
}

void tmp95c063_device::sc1mod_w(uint8_t data)
{
	m_serial_mode[1] = data;
}

uint8_t tmp95c063_device::br1cr_r()
{
	return m_baud_rate[1];
}

void tmp95c063_device::br1cr_w(uint8_t data)
{
	m_baud_rate[1] = data;
}

uint8_t tmp95c063_device::ode_r()
{
	return m_od_enable;
}

void tmp95c063_device::ode_w(uint8_t data)
{
	m_od_enable = data;
}


uint8_t tmp95c063_device::admod1_r()
{
	return m_ad_mode1;
}

void tmp95c063_device::admod1_w(uint8_t data)
{
	// Preserve read-only bits
	data = ( m_ad_mode1 & 0xc0 ) | ( data & 0x34 );

	// Check for A/D conversion start
	if (data & 0x04)
	{
		data &= ~0x04;
		data |= 0x40;

		switch ((m_ad_mode2 >> 4) & 3)
		{
		case 0: m_ad_cycles_left = 160; break;
		case 1: m_ad_cycles_left = 320; break;
		case 2: m_ad_cycles_left = 640; break;
		case 3: m_ad_cycles_left = 1280; break;
		}
	}

	m_ad_mode1 = data;
}

uint8_t tmp95c063_device::admod2_r()
{
	return m_ad_mode2;
}

void tmp95c063_device::admod2_w(uint8_t data)
{
	m_ad_mode2 = data;
}

uint8_t tmp95c063_device::adreg_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_ad_result[offset >> 1] >> 2;
	else
		return m_ad_result[offset >> 1] << 6 | 0x3f;
}


uint8_t tmp95c063_device::inte_r(offs_t offset)
{
	return m_int_reg[offset];
}

void tmp95c063_device::inte_w(offs_t offset, uint8_t data)
{
	if ( data & 0x80 )
		data = ( data & 0x7f ) | ( m_int_reg[offset] & 0x80 );
	if ( data & 0x08 )
		data = ( data & 0xf7 ) | ( m_int_reg[offset] & 0x08 );

	m_int_reg[offset] = data;
	m_check_irqs = 1;
}

void tmp95c063_device::iimc_w(uint8_t data)
{
	m_iimc = data;
	m_check_irqs = 1;
}

void tmp95c063_device::dmav_w(offs_t offset, uint8_t data)
{
	m_dma_vector[offset] = data;
}


void tmp95c063_device::bcs_w(offs_t offset, uint8_t data)
{
	m_block_cs[offset] = data;
}

void tmp95c063_device::bexcs_w(uint8_t data)
{
	m_external_cs = data;
}

uint8_t tmp95c063_device::msar_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_mem_start_mask[offset >> 1];
	else
		return m_mem_start_reg[offset >> 1];
}

void tmp95c063_device::msar_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_mem_start_mask[offset >> 1] = data;
	else
		m_mem_start_reg[offset >> 1] = data;
}


uint8_t tmp95c063_device::drefcr1_r()
{
	return m_dram_refresh[0];
}

void tmp95c063_device::drefcr1_w(uint8_t data)
{
	m_dram_refresh[0] = data;
}

uint8_t tmp95c063_device::dmemcr1_r()
{
	return m_dram_access[0];
}

void tmp95c063_device::dmemcr1_w(uint8_t data)
{
	m_dram_access[0] = data;
}

uint8_t tmp95c063_device::drefcr3_r()
{
	return m_dram_refresh[1];
}

void tmp95c063_device::drefcr3_w(uint8_t data)
{
	m_dram_refresh[1] = data;
}

uint8_t tmp95c063_device::dmemcr3_r()
{
	return m_dram_access[1];
}

void tmp95c063_device::dmemcr3_w(uint8_t data)
{
	m_dram_access[1] = data;
}


uint8_t tmp95c063_device::dadrv_r()
{
	return m_da_drive;
}

void tmp95c063_device::dadrv_w(uint8_t data)
{
	m_da_drive = data;
}

void tmp95c063_device::dareg_w(offs_t offset, uint8_t data)
{
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
		if (m_iimc & 0x04)
		{
			if (m_iimc & 0x02)
			{
				/* Rising edge detect */
				if (m_level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE)
				{
					/* Leave HALT state */
					m_halted = 0;
					m_int_reg[TMP95C063_INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if (level == ASSERT_LINE)
					m_int_reg[TMP95C063_INTE0AD] |= 0x08;
				else
					m_int_reg[TMP95C063_INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT1:
		if (m_level[TLCS900_INT1] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[TMP95C063_INTE12] |= 0x08;
		}
		else if (m_level[TLCS900_INT1] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[TMP95C063_INTE12] &= ~0x08;
		}
		m_level[TLCS900_INT1] = level;
		break;

	case TLCS900_INT2:
		if (m_level[TLCS900_INT2] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[TMP95C063_INTE12] |= 0x80;
		}
		else if (m_level[TLCS900_INT2] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[TMP95C063_INTE12] &= ~0x80;
		}
		m_level[TLCS900_INT2] = level;
		break;

	case TLCS900_INT3:
		if (m_level[TLCS900_INT3] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[TMP95C063_INTE34] |= 0x08;
		}
		else if (m_level[TLCS900_INT3] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[TMP95C063_INTE34] &= ~0x08;
		}
		m_level[TLCS900_INT3] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( m_port_control[0xb] & 0x01 ) )
		{
			if ( m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_int_reg[TMP95C063_INTE34] |= 0x80;
			}
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( m_port_control[0xb] & 0x02 ) )
		{
			if ( m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_int_reg[TMP95C063_INTE56] |= 0x08;
			}
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_INT6:
		if (m_level[TLCS900_INT6] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[TMP95C063_INTE56] |= 0x80;
		}
		else if (m_level[TLCS900_INT6] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[TMP95C063_INTE56] &= ~0x80;
		}
		m_level[TLCS900_INT6] = level;
		break;

	case TLCS900_TIO:   /* External timer input for timer 0 */
		if ( ( m_t8run & 0x01 ) && ( m_t8_mode[0] & 0x03 ) == 0x00 )
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

std::unique_ptr<util::disasm_interface> tmp95c063_device::create_disassembler()
{
	return std::make_unique<tmp95c063_disassembler>();
}
