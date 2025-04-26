// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

Toshiba TMP95C061 emulation

*******************************************************************/

#include "emu.h"
#include "tmp95c061.h"

DEFINE_DEVICE_TYPE(TMP95C061, tmp95c061_device, "tmp95c061", "Toshiba TMP95C061")


tmp95c061_device::tmp95c061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tlcs900h_device(mconfig, TMP95C061, tag, owner, clock),
	m_an_read(*this, 0),
	m_port_read(*this, 0),
	m_port_write(*this),
	m_port_latch{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_control{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_function{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_trun(0),
	m_t8_reg{ 0, 0, 0, 0 },
	m_t8_mode{ 0, 0 },
	m_t8_invert(0),
	m_trdc(0),
	m_to1(0),
	m_to3(0),
	m_t16_reg{ 0, 0, 0, 0 },
	m_t16_cap{ 0, 0, 0, 0 },
	m_t16_mode{ 0, 0 },
	m_t16_invert{ 0, 0 },
	m_t45cr(0),
	m_pgreg{ 0, 0 },
	m_pg01cr(0),
	m_watchdog_mode(0),
	m_serial_control{ 0, 0 },
	m_serial_mode{ 0, 0 },
	m_baud_rate{ 0, 0 },
	m_od_enable(0),
	m_ad_result{ 0, 0, 0, 0 },
	m_ad_mode(0),
	m_int_reg{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_iimc(0),
	m_dma_vector{ 0, 0, 0, 0 },
	m_block_cs{ 0, 0, 0, 0 },
	m_external_cs(0),
	m_mem_start_reg{ 0, 0, 0, 0 },
	m_mem_start_mask{ 0, 0, 0, 0 },
	m_dram_refresh(0),
	m_dram_access(0)
{
}

template <uint8_t P>
void tmp95c061_device::port_w(uint8_t data)
{
	m_port_latch[P] = data;
	m_port_write[P](0, data, 0xff);
}

template <uint8_t P>
uint8_t tmp95c061_device::port_r()
{
	return m_port_read[P](0);
}

template <uint8_t P>
void tmp95c061_device::port_cr_w(uint8_t data)
{
	m_port_control[P] = data;
}

template <uint8_t P>
void tmp95c061_device::port_fc_w(uint8_t data)
{
	m_port_function[P] = data;
}

template <>
void tmp95c061_device::port_w<tmp95c061_device::PORT_A>(uint8_t data)
{
	m_port_latch[PORT_A] = data;
	update_porta();
}

template <>
void tmp95c061_device::port_cr_w<tmp95c061_device::PORT_A>(uint8_t data)
{
	m_port_control[PORT_A] = data;
	update_porta();
}

template <>
void tmp95c061_device::port_fc_w<tmp95c061_device::PORT_A>(uint8_t data)
{
	m_port_function[PORT_A] = data;
	update_porta();
}

void tmp95c061_device::update_porta()
{
	int fc = (m_to1 << 2) | (m_to3 << 3);

	m_port_write[PORT_A](0, ((fc & m_port_function[PORT_A]) | (m_port_latch[PORT_A] & ~m_port_function[PORT_A])) & m_port_control[PORT_A], 0xff);
}

void tmp95c061_device::internal_mem(address_map &map)
{
	map(0x000001, 0x000001).rw(FUNC(tmp95c061_device::port_r<PORT_1>), FUNC(tmp95c061_device::port_w<PORT_1>));
	map(0x000004, 0x000004).w(FUNC(tmp95c061_device::port_cr_w<PORT_1>));
	map(0x000006, 0x000006).rw(FUNC(tmp95c061_device::port_r<PORT_2>), FUNC(tmp95c061_device::port_w<PORT_2>));
	map(0x000009, 0x000009).w(FUNC(tmp95c061_device::port_fc_w<PORT_2>));
	map(0x00000d, 0x00000d).rw(FUNC(tmp95c061_device::port_r<PORT_5>), FUNC(tmp95c061_device::port_w<PORT_5>));
	map(0x000010, 0x000010).w(FUNC(tmp95c061_device::port_cr_w<PORT_5>));
	map(0x000011, 0x000011).w(FUNC(tmp95c061_device::port_fc_w<PORT_5>));
	map(0x000012, 0x000012).rw(FUNC(tmp95c061_device::port_r<PORT_6>), FUNC(tmp95c061_device::port_w<PORT_7>));
	map(0x000013, 0x000013).rw(FUNC(tmp95c061_device::port_r<PORT_7>), FUNC(tmp95c061_device::port_w<PORT_7>));
	map(0x000015, 0x000015).w(FUNC(tmp95c061_device::port_fc_w<PORT_6>));
	map(0x000016, 0x000016).w(FUNC(tmp95c061_device::port_cr_w<PORT_7>));
	map(0x000017, 0x000017).w(FUNC(tmp95c061_device::port_fc_w<PORT_7>));
	map(0x000018, 0x000018).rw(FUNC(tmp95c061_device::port_r<PORT_8>), FUNC(tmp95c061_device::port_w<PORT_8>));
	map(0x000019, 0x000019).r(FUNC(tmp95c061_device::port_r<PORT_9>));
	map(0x00001a, 0x00001a).w(FUNC(tmp95c061_device::port_cr_w<PORT_8>));
	map(0x00001b, 0x00001b).w(FUNC(tmp95c061_device::port_fc_w<PORT_8>));
	map(0x00001e, 0x00001e).rw(FUNC(tmp95c061_device::port_r<PORT_A>), FUNC(tmp95c061_device::port_w<PORT_A>));
	map(0x00001f, 0x00001f).rw(FUNC(tmp95c061_device::port_r<PORT_B>), FUNC(tmp95c061_device::port_w<PORT_B>));
	map(0x000020, 0x000020).rw(FUNC(tmp95c061_device::trun_r), FUNC(tmp95c061_device::trun_w));
	map(0x000022, 0x000023).w(FUNC(tmp95c061_device::treg01_w));
	map(0x000024, 0x000024).w(FUNC(tmp95c061_device::t01mod_w));
	map(0x000025, 0x000025).rw(FUNC(tmp95c061_device::tffcr_r), FUNC(tmp95c061_device::tffcr_w));
	map(0x000026, 0x000027).w(FUNC(tmp95c061_device::treg23_w));
	map(0x000028, 0x000028).w(FUNC(tmp95c061_device::t23mod_w));
	map(0x000029, 0x000029).rw(FUNC(tmp95c061_device::trdc_r), FUNC(tmp95c061_device::trdc_w));
	map(0x00002c, 0x00002c).w(FUNC(tmp95c061_device::port_cr_w<PORT_A>));
	map(0x00002d, 0x00002d).w(FUNC(tmp95c061_device::port_fc_w<PORT_A>));
	map(0x00002e, 0x00002e).w(FUNC(tmp95c061_device::port_cr_w<PORT_B>));
	map(0x00002f, 0x00002f).w(FUNC(tmp95c061_device::port_fc_w<PORT_B>));
	map(0x000030, 0x000033).w(FUNC(tmp95c061_device::treg45_w));
	map(0x000034, 0x000037).r(FUNC(tmp95c061_device::cap12_r));
	map(0x000038, 0x000038).rw(FUNC(tmp95c061_device::t4mod_r), FUNC(tmp95c061_device::t4mod_w));
	map(0x000039, 0x000039).rw(FUNC(tmp95c061_device::t4ffcr_r), FUNC(tmp95c061_device::t4ffcr_w));
	map(0x00003a, 0x00003a).rw(FUNC(tmp95c061_device::t45cr_r), FUNC(tmp95c061_device::t45cr_w));
	map(0x00003c, 0x00003f).rw(FUNC(tmp95c061_device::msar01_r), FUNC(tmp95c061_device::msar01_w));
	map(0x000040, 0x000043).w(FUNC(tmp95c061_device::treg67_w));
	map(0x000044, 0x000047).r(FUNC(tmp95c061_device::cap34_r));
	map(0x000048, 0x000048).rw(FUNC(tmp95c061_device::t5mod_r), FUNC(tmp95c061_device::t5mod_w));
	map(0x000049, 0x000049).rw(FUNC(tmp95c061_device::t5ffcr_r), FUNC(tmp95c061_device::t5ffcr_w));
	map(0x00004c, 0x00004d).rw(FUNC(tmp95c061_device::pgreg_r), FUNC(tmp95c061_device::pgreg_w));
	map(0x00004e, 0x00004e).rw(FUNC(tmp95c061_device::pg01cr_r), FUNC(tmp95c061_device::pg01cr_w));
	map(0x000050, 0x000050).rw(FUNC(tmp95c061_device::sc0buf_r), FUNC(tmp95c061_device::sc0buf_w));
	map(0x000051, 0x000051).rw(FUNC(tmp95c061_device::sc0cr_r), FUNC(tmp95c061_device::sc0cr_w));
	map(0x000052, 0x000052).rw(FUNC(tmp95c061_device::sc0mod_r), FUNC(tmp95c061_device::sc0mod_w));
	map(0x000053, 0x000053).rw(FUNC(tmp95c061_device::br0cr_r), FUNC(tmp95c061_device::br0cr_w));
	map(0x000054, 0x000054).rw(FUNC(tmp95c061_device::sc1buf_r), FUNC(tmp95c061_device::sc1buf_w));
	map(0x000055, 0x000055).rw(FUNC(tmp95c061_device::sc1cr_r), FUNC(tmp95c061_device::sc1cr_w));
	map(0x000056, 0x000056).rw(FUNC(tmp95c061_device::sc1mod_r), FUNC(tmp95c061_device::sc1mod_w));
	map(0x000057, 0x000057).rw(FUNC(tmp95c061_device::br1cr_r), FUNC(tmp95c061_device::br1cr_w));
	map(0x000058, 0x000058).rw(FUNC(tmp95c061_device::ode_r), FUNC(tmp95c061_device::ode_w));
	map(0x00005a, 0x00005a).rw(FUNC(tmp95c061_device::drefcr_r), FUNC(tmp95c061_device::drefcr_w));
	map(0x00005b, 0x00005b).rw(FUNC(tmp95c061_device::dmemcr_r), FUNC(tmp95c061_device::dmemcr_w));
	map(0x00005c, 0x00005f).rw(FUNC(tmp95c061_device::msar23_r), FUNC(tmp95c061_device::msar23_w));
	map(0x000060, 0x000067).r(FUNC(tmp95c061_device::adreg_r));
	map(0x000068, 0x00006b).w(FUNC(tmp95c061_device::bcs_w));
	map(0x00006c, 0x00006c).w(FUNC(tmp95c061_device::bexcs_w));
	map(0x00006d, 0x00006d).rw(FUNC(tmp95c061_device::admod_r), FUNC(tmp95c061_device::admod_w));
	map(0x00006e, 0x00006e).rw(FUNC(tmp95c061_device::wdmod_r), FUNC(tmp95c061_device::wdmod_w));
	map(0x00006f, 0x00006f).w(FUNC(tmp95c061_device::wdcr_w));
	map(0x000070, 0x00007a).rw(FUNC(tmp95c061_device::inte_r), FUNC(tmp95c061_device::inte_w));
	map(0x00007b, 0x00007b).w(FUNC(tmp95c061_device::iimc_w));
	map(0x00007c, 0x00007f).w(FUNC(tmp95c061_device::dmav_w));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tmp95c061_device::device_config_complete()
{
	if (m_am8_16 == 0)
	{
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(tmp95c061_device::internal_mem), this));
	}
	else
	{
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 8, 24, 0, address_map_constructor(FUNC(tmp95c061_device::internal_mem), this));
	}
}


void tmp95c061_device::device_resolve_objects()
{
	m_nmi_state = CLEAR_LINE;
	for( int i = 0; i < TLCS900_NUM_INPUTS; i++ )
	{
		m_level[i] = CLEAR_LINE;
	}
}


void tmp95c061_device::device_start()
{
	tlcs900h_device::device_start();

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_control));
	save_item(NAME(m_port_function));
	save_item(NAME(m_trun));
	save_item(NAME(m_t8_reg));
	save_item(NAME(m_t8_mode));
	save_item(NAME(m_t8_invert));
	save_item(NAME(m_trdc));
	save_item(NAME(m_to1));
	save_item(NAME(m_to3));
	save_item(NAME(m_t16_reg));
	save_item(NAME(m_t16_cap));
	save_item(NAME(m_t16_mode));
	save_item(NAME(m_t16_invert));
	save_item(NAME(m_t45cr));
	save_item(NAME(m_pgreg));
	save_item(NAME(m_pg01cr));
	save_item(NAME(m_watchdog_mode));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_serial_mode));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_od_enable));
	save_item(NAME(m_ad_result));
	save_item(NAME(m_ad_mode));
	save_item(NAME(m_int_reg));
	save_item(NAME(m_iimc));
	save_item(NAME(m_dma_vector));
	save_item(NAME(m_block_cs));
	save_item(NAME(m_external_cs));
	save_item(NAME(m_mem_start_reg));
	save_item(NAME(m_mem_start_mask));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_dram_access));
}

void tmp95c061_device::device_reset()
{
	tlcs900h_device::device_reset();

	m_to1 = 0;
	m_to3 = 0;

	m_ad_cycles_left = 0;
	m_timer_pre = 0;
	m_timer_change[0] = 0;
	m_timer_change[1] = 0;
	m_timer_change[2] = 0;
	m_timer_change[3] = 0;

	m_port_latch[PORT_1] = 0x00;
	m_port_latch[PORT_2] = 0xff;
	m_port_latch[PORT_5] = 0x3d;
	m_port_latch[PORT_6] = 0x3b;
	m_port_latch[PORT_7] = 0xff;
	m_port_latch[PORT_8] = 0x3f;
	m_port_latch[PORT_A] = 0x0f;
	m_port_latch[PORT_B] = 0xff;
	std::fill_n(&m_port_control[0], NUM_PORTS, 0x00);
	std::fill_n(&m_port_function[0], NUM_PORTS, 0x00);
	m_port_control[PORT_A] = 0x0c; // HACK ngpc needs this but should be zero
	m_port_function[PORT_A] = 0x0c; // HACK ngpc needs this but should be zero
	m_trun = 0x00;
	std::fill_n(&m_t8_mode[0], 2, 0x00);
	m_t8_invert = 0xcc;
	m_trdc = 0x00;
	std::fill_n(&m_t16_mode[0], 2, 0x20);
	std::fill_n(&m_t16_invert[0], 2, 0x00);
	m_t45cr = 0x00;
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
	m_ad_mode = 0x00;
	std::fill_n(&m_int_reg[0], 0xb, 0x00);
	m_iimc = 0x00;
	std::fill_n(&m_dma_vector[0], 4, 0x00);
	m_block_cs[0] = 0x00;
	m_block_cs[1] = 0x00;
	m_block_cs[2] = 0x10;
	m_block_cs[3] = 0x00;
	m_external_cs = 0x00;
	std::fill_n(&m_mem_start_reg[0], 4, 0xff);
	std::fill_n(&m_mem_start_mask[0], 4, 0xff);
	m_dram_refresh = 0x00;
	m_dram_access = 0x80;
}

enum
{
	INTE0AD,
	INTE45,
	INTE67,
	INTET10,
	INTET32,
	INTET54,
	INTET76,
	INTES0,
	INTES1,
	INTETC10,
	INTETC32
};

static const struct {
	uint8_t reg;
	uint8_t iff;
	uint8_t vector;
} tmp95c061_irq_vector_map[] =
{
	{ INTETC32, 0x80, 0x80 },   // INTTC3
	{ INTETC32, 0x08, 0x7c },   // INTTC2
	{ INTETC10, 0x80, 0x78 },   // INTTC1
	{ INTETC10, 0x08, 0x74 },   // INTTC0
	{ INTE0AD,  0x80, 0x70 },   // INTAD
	{ INTES1,   0x80, 0x6c },   // INTTX1
	{ INTES1,   0x08, 0x68 },   // INTRX1
	{ INTES0,   0x80, 0x64 },   // INTTX0
	{ INTES0,   0x08, 0x60 },   // INTRX0
	{ INTET76,  0x80, 0x5c },   // INTTR7
	{ INTET76,  0x08, 0x58 },   // INTTR6
	{ INTET54,  0x80, 0x54 },   // INTTR5
	{ INTET54,  0x08, 0x50 },   // INTTR4
	{ INTET32,  0x80, 0x4c },   // INTT3
	{ INTET32,  0x08, 0x48 },   // INTT2
	{ INTET10,  0x80, 0x44 },   // INTT1
	{ INTET10,  0x08, 0x40 },   // INTT0
								// 0x3c - reserved
	{ INTE67,   0x80, 0x38 },   // INT7
	{ INTE67,   0x08, 0x34 },   // INT6
	{ INTE45,   0x80, 0x30 },   // INT5
	{ INTE45,   0x08, 0x2c },   // INT4
	{ INTE0AD,  0x08, 0x28 }    // INT0
};
static constexpr u8 NUM_MASKABLE_IRQS = sizeof(tmp95c061_irq_vector_map) / 3;


int tmp95c061_device::tlcs900_process_hdma( int channel )
{
	uint8_t vector = ( m_dma_vector[channel] & 0x1f ) << 2;

	/* Check if any HDMA actions should be performed */
	if ( vector >= 0x28 && vector != 0x3c && vector < 0x74 )
	{
		int irq = 0;

		while( irq < NUM_MASKABLE_IRQS && tmp95c061_irq_vector_map[irq].vector != vector )
			irq++;

		/* Check if our interrupt flip-flop is set */
		if ( irq < NUM_MASKABLE_IRQS && m_int_reg[tmp95c061_irq_vector_map[irq].reg] & tmp95c061_irq_vector_map[irq].iff )
		{
			switch( m_dmam[channel].b.l & 0x1f )
			{
			case 0x00:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmad[channel].d += 1;
				m_cycles += 8;
				break;
			case 0x01:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmad[channel].d += 2;
				m_cycles += 8;
				break;
			case 0x02:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmad[channel].d += 4;
				m_cycles += 12;
				break;
			case 0x04:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmad[channel].d -= 1;
				m_cycles += 8;
				break;
			case 0x05:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmad[channel].d -= 2;
				m_cycles += 8;
				break;
			case 0x06:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmad[channel].d -= 4;
				m_cycles += 12;
				break;
			case 0x08:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmas[channel].d += 1;
				m_cycles += 8;
				break;
			case 0x09:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d += 2;
				m_cycles += 8;
				break;
			case 0x0a:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmas[channel].d += 4;
				m_cycles += 12;
				break;
			case 0x0c:
				WRMEM( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d -= 1;
				m_cycles += 8;
				break;
			case 0x0d:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d -= 2;
				m_cycles += 8;
				break;
			case 0x0e:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmas[channel].d -= 4;
				m_cycles += 12;
				break;
			case 0x10:
				WRMEM( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_cycles += 8;
				break;
			case 0x11:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_cycles += 8;
				break;
			case 0x12:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_cycles += 12;
				break;
			case 0x14:
				m_dmas[channel].d += 1;
				m_cycles += 5;
				break;
			}

			m_dmac[channel].w.l -= 1;

			if ( m_dmac[channel].w.l == 0 )
			{
				m_dma_vector[channel] = 0;
				switch( channel )
				{
				case 0:
					m_int_reg[INTETC10] |= 0x08;
					break;
				case 1:
					m_int_reg[INTETC10] |= 0x80;
					break;
				case 2:
					m_int_reg[INTETC32] |= 0x08;
					break;
				case 3:
					m_int_reg[INTETC32] |= 0x80;
					break;
				}
			}

			/* Clear the interrupt flip-flop */
			m_int_reg[tmp95c061_irq_vector_map[irq].reg] &= ~tmp95c061_irq_vector_map[irq].iff;

			return 1;
		}
	}
	return 0;
}


void tmp95c061_device::tlcs900_check_hdma()
{
	/* HDMA can only be performed if interrupts are allowed */
	if ( ( m_sr.b.h & 0x70 ) != 0x70 )
	{
		if ( ! tlcs900_process_hdma( 0 ) )
		{
			if ( ! tlcs900_process_hdma( 1 ) )
			{
				if ( ! tlcs900_process_hdma( 2 ) )
				{
					tlcs900_process_hdma( 3 );
				}
			}
		}
	}
}


void tmp95c061_device::tlcs900_check_irqs()
{
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
	int irq_vectors[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	for( int i = 0; i < NUM_MASKABLE_IRQS; i++ )
	{
		if ( m_int_reg[tmp95c061_irq_vector_map[i].reg] & tmp95c061_irq_vector_map[i].iff )
		{
			switch( tmp95c061_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_int_reg[ tmp95c061_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_int_reg[ tmp95c061_irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	int irq = -1;
	int level = 0;
	for ( int i = std::max( 1, ( ( m_sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
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
		uint8_t vector = tmp95c061_irq_vector_map[irq].vector;

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
		m_int_reg[ tmp95c061_irq_vector_map[irq].reg ] &= ~ tmp95c061_irq_vector_map[irq].iff;
	}
}


void tmp95c061_device::tlcs900_handle_ad()
{
	if ( m_ad_cycles_left > 0 )
	{
		m_ad_cycles_left -= m_cycles;
		if ( m_ad_cycles_left <= 0 )
		{
			/* Store A/D converted value */
			if ( ( m_ad_mode & 0x10 ) == 0 )
			{
				/* conversion channel fixed */
				m_ad_result[m_ad_mode & 0x03] = m_an_read[m_ad_mode & 0x03](0) & 0x3ff;
			}
			else
			{
				/* conversion channel sweep */
				switch( m_ad_mode & 0x03 )
				{
				case 0x03:  /* AN3 */
					m_ad_result[3] = m_an_read[3](0) & 0x3ff;
					[[fallthrough]];
				case 0x02:  /* AN2 */
					m_ad_result[2] = m_an_read[2](0) & 0x3ff;
					[[fallthrough]];
				case 0x01:  /* AN1 */
					m_ad_result[1] = m_an_read[1](0) & 0x3ff;
					[[fallthrough]];
				case 0x00:  /* AN0 */
					m_ad_result[0] = m_an_read[0](0) & 0x3ff;
					break;
				}
			}

			/* Clear BUSY flag, set END flag */
			m_ad_mode &= ~ 0x40;
			m_ad_mode |= 0x80;

			m_int_reg[INTE0AD] |= 0x80;
			m_check_irqs = 1;

			/* AD repeat mode */
			if ( m_ad_mode & 0x20 )
				m_ad_cycles_left = ( m_ad_mode & 0x08 ) ? 320 : 160;
		}
	}
}


enum ff_change
{
	FF_CLEAR,
	FF_SET,
	FF_INVERT
};


void tmp95c061_device::tlcs900_change_tff( int which, int change )
{
	switch( which )
	{
	case 1:
		switch( change )
		{
		case FF_CLEAR:
			m_to1 = 0;
			break;
		case FF_SET:
			m_to1 = 1;
			break;
		case FF_INVERT:
			m_to1 ^= 1;
			break;
		}
		break;

	case 3:
		switch( change )
		{
		case FF_CLEAR:
			m_to3 = 0;
			break;
		case FF_SET:
			m_to3 = 1;
			break;
		case FF_INVERT:
			m_to3 ^= 1;
			break;
		}
		break;
	}

	update_porta();
}


void tmp95c061_device::tlcs900_handle_timers()
{
	uint32_t  old_pre = m_timer_pre;

	/* Is the pre-scaler active */
	if ( m_trun & 0x80 )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( m_trun & 0x01 )
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
			m_timer_8[0] += 1;
			if ( m_timer_8[0] == m_t8_reg[0] )
			{
				if ( ( m_trun & 0x02 ) && ( m_t8_mode[0] & 0x0c ) == 0x00 )
				{
					m_timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_t8_mode[0] & 0xc0 ) != 0x40 )
				{
					m_timer_8[0] = 0;
					m_int_reg[INTET10] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( m_trun & 0x02 )
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
			m_timer_8[1] += 1;
			if ( m_timer_8[1] == m_t8_reg[1] )
			{
				m_timer_8[1] = 0;
				m_int_reg[INTET10] |= 0x80;

				if ( m_t8_invert & 0x02 )
				{
					tlcs900_change_tff( 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( m_t8_mode[0] & 0xc0 ) == 0x40 )
				{
					m_timer_8[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( m_trun & 0x04 )
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
			m_timer_8[2] += 1;
			if ( m_timer_8[2] == m_t8_reg[2] )
			{
				if ( ( m_trun & 0x08 ) && ( m_t8_mode[1] & 0x0c ) == 0x00 )
				{
					m_timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_t8_mode[1] & 0xc0 ) != 0x40 )
				{
					m_timer_8[2] = 0;
					m_int_reg[INTET32] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( m_trun & 0x08 )
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
			m_timer_8[3] += 1;
			if ( m_timer_8[3] == m_t8_reg[3] )
			{
				m_timer_8[3] = 0;
				m_int_reg[INTET32] |= 0x80;

				if ( m_t8_invert & 0x20 )
				{
					tlcs900_change_tff( 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( m_t8_mode[1] & 0xc0 ) == 0x40 )
				{
					m_timer_8[2] = 0;
				}
			}
		}
	}

	m_timer_pre &= 0xffffff;
}


void tmp95c061_device::execute_set_input(int input, int level)
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
		if ( m_iimc & 0x04 )
		{
			if ( m_iimc & 0x02 )
			{
				/* Rising edge detect */
				if ( m_level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE )
				{
					/* Leave HALT state */
					m_halted = 0;
					m_int_reg[INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if ( level == ASSERT_LINE )
					m_int_reg[INTE0AD] |= 0x08;
				else
					m_int_reg[INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( m_port_control[PORT_B] & 0x01 ) )
		{
			if ( m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_int_reg[INTE45] |= 0x08;
			}
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( m_port_control[PORT_B] & 0x02 ) )
		{
			if ( m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_int_reg[INTE45] |= 0x80;
			}
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_TIO:   /* External timer input for timer 0 */
		if ( ( m_trun & 0x01 ) && ( m_t8_mode[0] & 0x03 ) == 0x00 )
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

uint8_t tmp95c061_device::trun_r()
{
	return m_trun;
}

void tmp95c061_device::trun_w(uint8_t data)
{
	if ( ! ( data & 0x01 ) )
	{
		m_timer_8[0] = 0;
		m_timer_change[0] = 0;
	}
	if ( ! ( data & 0x02 ) )
	{
		m_timer_8[1] = 0;
		m_timer_change[1] = 0;
	}
	if ( ! ( data & 0x04 ) )
	{
		m_timer_8[2] = 0;
		m_timer_change[2] = 0;
	}
	if ( ! ( data & 0x08 ) )
	{
		m_timer_8[3] = 0;
		m_timer_change[3] = 0;
	}
	if ( ! ( data & 0x10 ) )
		m_timer_8[4] = 0;
	if ( ! ( data & 0x20 ) )
		m_timer_8[5] = 0;

	m_trun = data;
}

void tmp95c061_device::treg01_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset] = data;
}

void tmp95c061_device::t01mod_w(uint8_t data)
{
	m_t8_mode[0] = data;
}

uint8_t tmp95c061_device::tffcr_r()
{
	return m_t8_invert;
}

void tmp95c061_device::tffcr_w(uint8_t data)
{
	switch( data & 0x0c )
	{
	case 0x00:
		tlcs900_change_tff( 1, FF_INVERT );
		break;
	case 0x04:
		tlcs900_change_tff( 1, FF_SET );
		break;
	case 0x08:
		tlcs900_change_tff( 1, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		tlcs900_change_tff( 3, FF_INVERT );
		break;
	case 0x40:
		tlcs900_change_tff( 3, FF_SET );
		break;
	case 0x80:
		tlcs900_change_tff( 3, FF_CLEAR );
		break;
	}

	m_t8_invert = data | 0xcc;
}

void tmp95c061_device::treg23_w(offs_t offset, uint8_t data)
{
	m_t8_reg[offset + 2] = data;
}

void tmp95c061_device::t23mod_w(uint8_t data)
{
	m_t8_mode[1] = data;
}

uint8_t tmp95c061_device::trdc_r()
{
	return m_trdc;
}

void tmp95c061_device::trdc_w(uint8_t data)
{
	m_trdc = data;
}

void tmp95c061_device::treg45_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_t16_reg[offset >> 1] = (m_t16_reg[offset >> 1] & 0x00ff) | uint16_t(data) << 8;
	else
		m_t16_reg[offset >> 1] = (m_t16_reg[offset >> 1] & 0xff00) | data;
}

uint8_t tmp95c061_device::cap12_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[offset >> 1] >> 8;
	else
		return m_t16_cap[offset >> 1] & 0x00ff;
}

uint8_t tmp95c061_device::t4mod_r()
{
	return m_t16_mode[0];
}

void tmp95c061_device::t4mod_w(uint8_t data)
{
	m_t16_mode[0] = data | 0x20;
}

uint8_t tmp95c061_device::t4ffcr_r()
{
	return m_t16_invert[0];
}

void tmp95c061_device::t4ffcr_w(uint8_t data)
{
	m_t16_invert[0] = data | 0xc3;
}

uint8_t tmp95c061_device::t45cr_r()
{
	return m_t45cr;
}

void tmp95c061_device::t45cr_w(uint8_t data)
{
	m_t45cr = data;
}

void tmp95c061_device::treg67_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_t16_reg[(offset >> 1) + 2] = (m_t16_reg[(offset >> 1) + 2] & 0x00ff) | uint16_t(data) << 8;
	else
		m_t16_reg[(offset >> 1) + 2] = (m_t16_reg[(offset >> 1) + 2] & 0xff00) | data;
}

uint8_t tmp95c061_device::cap34_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 2] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 2] & 0x00ff;
}

uint8_t tmp95c061_device::t5mod_r()
{
	return m_t16_mode[1];
}

void tmp95c061_device::t5mod_w(uint8_t data)
{
	m_t16_mode[1] = data | 0x20;
}

uint8_t tmp95c061_device::t5ffcr_r()
{
	return m_t16_invert[1];
}

void tmp95c061_device::t5ffcr_w(uint8_t data)
{
	m_t16_invert[1] = data | 0xc3;
}


uint8_t tmp95c061_device::pgreg_r(offs_t offset)
{
	return m_pgreg[offset];
}

void tmp95c061_device::pgreg_w(offs_t offset, uint8_t data)
{
	m_pgreg[offset] = data;
}

uint8_t tmp95c061_device::pg01cr_r()
{
	return m_pg01cr;
}

void tmp95c061_device::pg01cr_w(uint8_t data)
{
	m_pg01cr = data;
}


uint8_t tmp95c061_device::wdmod_r()
{
	return m_watchdog_mode;
}

void tmp95c061_device::wdmod_w(uint8_t data)
{
	m_watchdog_mode = data;
}

void tmp95c061_device::wdcr_w(uint8_t data)
{
}


uint8_t tmp95c061_device::sc0buf_r()
{
	return 0;
}

void tmp95c061_device::sc0buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[INTES0] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp95c061_device::sc0cr_r()
{
	uint8_t reg = m_serial_control[0];
	if (!machine().side_effects_disabled())
		m_serial_control[0] &= 0xe3;
	return reg;
}

void tmp95c061_device::sc0cr_w(uint8_t data)
{
	m_serial_control[0] = data;
}

uint8_t tmp95c061_device::sc0mod_r()
{
	return m_serial_mode[0];
}

void tmp95c061_device::sc0mod_w(uint8_t data)
{
	m_serial_mode[0] = data;
}

uint8_t tmp95c061_device::br0cr_r()
{
	return m_baud_rate[0];
}

void tmp95c061_device::br0cr_w(uint8_t data)
{
	m_baud_rate[0] = data;
}

uint8_t tmp95c061_device::sc1buf_r()
{
	return 0;
}

void tmp95c061_device::sc1buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[INTES1] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp95c061_device::sc1cr_r()
{
	uint8_t reg = m_serial_control[1];
	if (!machine().side_effects_disabled())
		m_serial_control[1] &= 0xe3;
	return reg;
}

void tmp95c061_device::sc1cr_w(uint8_t data)
{
	m_serial_control[1] = data;
}

uint8_t tmp95c061_device::sc1mod_r()
{
	return m_serial_mode[1];
}

void tmp95c061_device::sc1mod_w(uint8_t data)
{
	m_serial_mode[1] = data;
}

uint8_t tmp95c061_device::br1cr_r()
{
	return m_baud_rate[1];
}

void tmp95c061_device::br1cr_w(uint8_t data)
{
	m_baud_rate[1] = data;
}

uint8_t tmp95c061_device::ode_r()
{
	return m_od_enable;
}

void tmp95c061_device::ode_w(uint8_t data)
{
	m_od_enable = data;
}


uint8_t tmp95c061_device::adreg_r(offs_t offset)
{
	// ADMOD EOCF is cleared to 0 when reading any ADREG0..3
	m_ad_mode &= ~0x80;

	if (BIT(offset, 0))
		return m_ad_result[offset >> 1] >> 2;

	// Reading data from the upper 8 bits clears INTE0AD IADC
	m_int_reg[INTE0AD] &= ~0x80;
	return m_ad_result[offset >> 1] << 6 | 0x3f;
}

uint8_t tmp95c061_device::admod_r()
{
	return m_ad_mode;
}

void tmp95c061_device::admod_w(uint8_t data)
{
	// Preserve read-only bits
	data = ( m_ad_mode & 0xc0 ) | ( data & 0x3f );

	// Check for A/D request start */
	if ( data & 0x04 )
	{
		data &= ~0x04;
		data |= 0x40;
		m_ad_cycles_left = ( data & 0x08 ) ? 320 : 160;
	}

	m_ad_mode = data;
}


uint8_t tmp95c061_device::inte_r(offs_t offset)
{
	return m_int_reg[offset];
}

void tmp95c061_device::inte_w(offs_t offset, uint8_t data)
{
	if ( data & 0x80 )
		data = ( data & 0x7f ) | ( m_int_reg[offset] & 0x80 );
	if ( data & 0x08 )
		data = ( data & 0xf7 ) | ( m_int_reg[offset] & 0x08 );

	m_int_reg[offset] = data;
	m_check_irqs = 1;
}

void tmp95c061_device::iimc_w(uint8_t data)
{
	m_iimc = data;
	m_check_irqs = 1;
}

void tmp95c061_device::dmav_w(offs_t offset, uint8_t data)
{
	m_dma_vector[offset] = data;
}


void tmp95c061_device::bcs_w(offs_t offset, uint8_t data)
{
	m_block_cs[offset] = data;
}

void tmp95c061_device::bexcs_w(uint8_t data)
{
	m_external_cs = data;
}

uint8_t tmp95c061_device::msar01_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_mem_start_mask[offset >> 1];
	else
		return m_mem_start_reg[offset >> 1];
}

void tmp95c061_device::msar01_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_mem_start_mask[offset >> 1] = data;
	else
		m_mem_start_reg[offset >> 1] = data;
}

uint8_t tmp95c061_device::msar23_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_mem_start_mask[(offset >> 1) + 2];
	else
		return m_mem_start_reg[(offset >> 1) + 2];
}

void tmp95c061_device::msar23_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_mem_start_mask[(offset >> 1) + 2] = data;
	else
		m_mem_start_reg[(offset >> 1) + 2] = data;
}


uint8_t tmp95c061_device::drefcr_r()
{
	return m_dram_refresh;
}

void tmp95c061_device::drefcr_w(uint8_t data)
{
	m_dram_refresh = data;
}

uint8_t tmp95c061_device::dmemcr_r()
{
	return m_dram_access;
}

void tmp95c061_device::dmemcr_w(uint8_t data)
{
	m_dram_access = data;
}
