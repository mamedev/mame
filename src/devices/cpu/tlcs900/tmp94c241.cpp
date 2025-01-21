// license:BSD-3-Clause
// copyright-holders:AJR,Wilbert Pol,Felipe Sanches
/****************************************************************************

    Toshiba TMP94C241 microcontroller

****************************************************************************/

#include "emu.h"
#include "tmp94c241.h"
#include "dasm900.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TMP94C241, tmp94c241_device, "tmp94c241", "Toshiba TMP94C241")

#define SFR_T01MOD	m_tmod[0]
#define SFR_T23MOD	m_tmod[1]
#define SFR_T4MOD	m_tmod[2]
#define SFR_T6MOD	m_tmod[3]
#define SFR_T8MOD	m_tmod[4]
#define SFR_TAMOD	m_tmod[5]
#define SFR_T8RUN	m_t8run
#define SFR_T16RUN	m_t16run
#define SFR_T02FFCR	m_ffcr[0]
#define SFR_T4FFCR	m_ffcr[1]
#define SFR_T6FFCR	m_ffcr[2]
#define SFR_T8FFCR	m_ffcr[3]
#define SFR_TAFFCR	m_ffcr[4]
#define SFR_MSAR0 m_mem_start_reg[0]
#define SFR_MSAR1 m_mem_start_reg[1]
#define SFR_MSAR2 m_mem_start_reg[2]
#define SFR_MSAR3 m_mem_start_reg[3]
#define SFR_MSAR4 m_mem_start_reg[4]
#define SFR_MSAR5 m_mem_start_reg[5]
#define SFR_MAMR0 m_mem_start_mask[0]
#define SFR_MAMR1 m_mem_start_mask[1]
#define SFR_MAMR2 m_mem_start_mask[2]
#define SFR_MAMR3 m_mem_start_mask[3]
#define SFR_MAMR4 m_mem_start_mask[4]
#define SFR_MAMR5 m_mem_start_mask[5]

#define TIMER_0_IS_RUNNING	BIT(SFR_T8RUN, 0)
#define TIMER_1_IS_RUNNING	BIT(SFR_T8RUN, 1)
#define TIMER_2_IS_RUNNING	BIT(SFR_T8RUN, 2)
#define TIMER_3_IS_RUNNING	BIT(SFR_T8RUN, 3)
#define TIMER_4_IS_RUNNING	BIT(SFR_T16RUN, 0)
#define TIMER_6_IS_RUNNING	BIT(SFR_T16RUN, 1)
#define TIMER_8_IS_RUNNING	BIT(SFR_T16RUN, 2)
#define TIMER_A_IS_RUNNING	BIT(SFR_T16RUN, 3)
#define PRESCALER_IS_ACTIVE	BIT(SFR_T16RUN, 7)

#define T0_INPUT_CLOCK			((SFR_T01MOD >> 0) & 0x03)
#define T1_INPUT_CLOCK			((SFR_T01MOD >> 2) & 0x03)
#define PWM1_INTERVAL_SELECTION	((SFR_T01MOD >> 4) & 0x03)
#define TO1_OPERATING_MODE		((SFR_T01MOD >> 6) & 0x03)
#define T2_INPUT_CLOCK			((SFR_T23MOD >> 0) & 0x03)
#define T3_INPUT_CLOCK			((SFR_T23MOD >> 2) & 0x03)
#define PWM2_INTERVAL_SELECTION	((SFR_T23MOD >> 4) & 0x03)
#define T23_OPERATING_MODE		((SFR_T23MOD >> 6) & 0x03)
#define T4_INPUT_CLOCK			((SFR_T4MOD >> 0) & 0x03)
#define UC4_CLEAR				((SFR_T4MOD >> 2) & 0x01)
#define T4_CAPTURE_TIMING		((SFR_T4MOD >> 3) & 0x03)
#define CAP4IN					((SFR_T4MOD >> 5) & 0x01)
#define T6_INPUT_CLOCK			((SFR_T6MOD >> 0) & 0x03)
#define T8_INPUT_CLOCK			((SFR_T8MOD >> 0) & 0x03)
#define TA_INPUT_CLOCK			((SFR_TAMOD >> 0) & 0x03)
#define EQ4T4 BIT(SFR_T4FFCR, 2)
#define EQ5T4 BIT(SFR_T4FFCR, 3)
#define EQ6T6 BIT(SFR_T6FFCR, 2)
#define EQ7T6 BIT(SFR_T6FFCR, 3)
#define EQ8T8 BIT(SFR_T8FFCR, 2)
#define EQ9T8 BIT(SFR_T8FFCR, 3)
#define EQATA BIT(SFR_TAFFCR, 2)
#define EQBTA BIT(SFR_TAFFCR, 3)

// Field values for timer mode selection on TnMOD SFRs:
#define MODE_8BIT_TIMER		0
#define MODE_16BIT_TIMER	1
#define MODE_PPG			2
#define MODE_PWM			3

#define UPCOUNTER_0		m_timer[0]
#define UPCOUNTER_1		m_timer[1]
#define UPCOUNTER_2		m_timer[2]
#define UPCOUNTER_3		m_timer[3]
#define UPCOUNTER_4		m_timer16[0]
#define UPCOUNTER_6		m_timer16[1]
#define UPCOUNTER_8		m_timer16[2]
#define UPCOUNTER_A		m_timer16[3]
#define SFR_TREG0	m_t8_reg[0]
#define SFR_TREG1	m_t8_reg[1]
#define SFR_TREG2	m_t8_reg[2]
#define SFR_TREG3	m_t8_reg[3]
#define SFR_TREG4	m_t16_reg[0]
#define SFR_TREG5	m_t16_reg[1]
#define SFR_TREG6	m_t16_reg[2]
#define SFR_TREG7	m_t16_reg[3]
#define SFR_TREG8	m_t16_reg[4]
#define SFR_TREG9	m_t16_reg[5]
#define SFR_TREGA	m_t16_reg[6]
#define SFR_TREGB	m_t16_reg[7]
#define TIMER_CHANGE_0	m_timer_change[0]
#define TIMER_CHANGE_1	m_timer_change[1]
#define TIMER_CHANGE_2	m_timer_change[2]
#define TIMER_CHANGE_3	m_timer_change[3]
#define TIMER_CHANGE_4	m_timer_change[4]
#define TIMER_CHANGE_6	m_timer_change[5]
#define TIMER_CHANGE_8	m_timer_change[6]
#define TIMER_CHANGE_A	m_timer_change[7]

enum
{
	INTE45,
	INTE67,
	INTE89,
	INTEAB,
	INTET01,
	INTET23,
	INTET45,
	INTET67,
	INTET89,
	INTETAB,
	INTES0,
	INTES1,
	INTETC01,
	INTETC23,
	INTETC45,
	INTETC67,
	INTE0AD,
	INTNMWDT
};

static const struct {
	uint8_t reg;
	uint8_t iff;
	uint8_t vector;
	uint8_t dma_start_vector;
} tmp94c241_irq_vector_map[] =
{
	{ INTE0AD,  0x08, 0x28, 0x0a},  // INT0 Pin
	{ INTE45,   0x08, 0x2c, 0x0b},  // INT4 Pin
	{ INTE45,   0x80, 0x30, 0x0c},  // INT5 Pin
	{ INTE67,   0x08, 0x34, 0x0d},  // INT6 Pin
	{ INTE67,   0x80, 0x38, 0x0e},  // INT7 Pin
									// 0x3c - reserved
	{ INTE89,   0x08, 0x40, 0x10},  // INT8 Pin
	{ INTE89,   0x80, 0x44, 0x11},  // INT9 Pin
	{ INTEAB,   0x08, 0x48, 0x12},  // INTA Pin
	{ INTEAB,   0x80, 0x4c, 0x13},  // INTB Pin
	{ INTET01,  0x08, 0x50, 0x14},  // INTT0: 8-bit timer (Timer 0)
	{ INTET01,  0x80, 0x54, 0x15},  // INTT1: 8-bit timer (Timer 1)
	{ INTET23,  0x08, 0x58, 0x16},  // INTT2: 8-bit timer (Timer 2)
	{ INTET23,  0x80, 0x5c, 0x17},  // INTT3: 8-bit timer (Timer 3)
	{ INTET45,  0x08, 0x60, 0x18},  // INTTR4: 16-bit timer (Treg 4)
	{ INTET45,  0x80, 0x64, 0x19},  // INTTR5: 16-bit timer (Treg 5)
	{ INTET67,  0x08, 0x68, 0x1a},  // INTTR6: 16-bit timer (Treg 6)
	{ INTET67,  0x80, 0x6c, 0x1b},  // INTTR7: 16-bit timer (Treg 7)
	{ INTET89,  0x08, 0x70, 0x1c},  // INTTR8: 16-bit timer (Treg 8)
	{ INTET89,  0x80, 0x74, 0x1d},  // INTTR9: 16-bit timer (Treg 9)
	{ INTETAB,  0x08, 0x78, 0x1e},  // INTTRA: 16-bit timer (Treg A)
	{ INTETAB,  0x80, 0x7c, 0x1f},  // INTTRB: 16-bit timer (Treg B)
	{ INTES0,   0x08, 0x80, 0x20},  // INTRX0: Serial receive 0
	{ INTES0,   0x80, 0x84, 0x21},  // INTTX0: Serial send 0
	{ INTES1,   0x08, 0x88, 0x22},  // INTRX1: Serial receive 1
	{ INTES1,   0x80, 0x8c, 0x23},  // INTTX1: Serial send 1
	{ INTE0AD,  0x80, 0x90, 0x24},  // INTAD: AD conversion completion
	{ INTETC01, 0x08, 0x94, 0x25},  // INTTC0: micro-DMA completion Ch.0
	{ INTETC01, 0x80, 0x98, 0x26},  // INTTC1: micro-DMA completion Ch.1
	{ INTETC23, 0x08, 0x9c, 0x27},  // INTTC2: micro-DMA completion Ch.2
	{ INTETC23, 0x80, 0xa0, 0x28},  // INTTC3: micro-DMA completion Ch.3
	{ INTETC45, 0x08, 0xa4, 0x29},  // INTTC4: micro-DMA completion Ch.4
	{ INTETC45, 0x80, 0xa8, 0x2a},  // INTTC5: micro-DMA completion Ch.5
	{ INTETC67, 0x08, 0xac, 0x2b},  // INTTC6: micro-DMA completion Ch.6
	{ INTETC67, 0x80, 0xb0, 0x2c},  // INTTC7: micro-DMA completion Ch.7
									/* 0xb4 ... 0xfc (Reserved) */
};
static constexpr u8 NUM_MASKABLE_IRQS = sizeof(tmp94c241_irq_vector_map) / 4;


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  tmp94c241_device - constructor
//-------------------------------------------------

tmp94c241_device::tmp94c241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tlcs900h_device(mconfig, TMP94C241, tag, owner, clock),
	m_an_read(*this, 0),
	m_port_read(*this, 0),
	m_port_write(*this),
	m_port_latch{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_control{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_port_function{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t8run(0),
	m_t8_reg{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_tmod{ 0, 0, 0, 0, 0, 0 },
	m_ffcr{ 0, 0, 0, 0, 0 },
	m_trdc(0),
	m_t16_reg{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t16_cap{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t16run(0),
	m_timer16{ 0, 0, 0, 0 },
	m_watchdog_mode(0),
	m_serial_control{ 0, 0 },
	m_serial_mode{ 0, 0 },
	m_baud_rate{ 0, 0 },
	m_od_enable(0),
	m_ad_mode1(0),
	m_ad_mode2(0),
	m_ad_result{ 0, 0, 0, 0 },
	m_int_reg{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
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

//-------------------------------------------------
//  device_config_complete - device-specific startup
//-------------------------------------------------

void tmp94c241_device::device_config_complete()
{
	if (m_am8_16 == 0)
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(tmp94c241_device::internal_mem), this));
	else
		m_program_config = address_space_config("program", ENDIANNESS_LITTLE, 8, 24, 0, address_map_constructor(FUNC(tmp94c241_device::internal_mem), this));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmp94c241_device::device_start()
{
	tlcs900h_device::device_start();

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_control));
	save_item(NAME(m_port_function));
	save_item(NAME(m_t8run));
	save_item(NAME(m_t8_reg));
	save_item(NAME(m_tmod));
	save_item(NAME(m_trdc));
	save_item(NAME(m_t16_reg));
	save_item(NAME(m_t16_cap));
	save_item(NAME(m_ffcr));
	save_item(NAME(m_t16run));
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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmp94c241_device::device_reset()
{
	tlcs900h_device::device_reset();

	m_ad_cycles_left = 0;
	m_timer_pre = 0;
	TIMER_CHANGE_0 = 0;
	TIMER_CHANGE_1 = 0;
	TIMER_CHANGE_2 = 0;
	TIMER_CHANGE_3 = 0;
	TIMER_CHANGE_4 = 0;
	TIMER_CHANGE_6 = 0;
	TIMER_CHANGE_8 = 0;
	TIMER_CHANGE_A = 0;

	m_port_latch[PORT_0] = 0x00;
	m_port_latch[PORT_1] = 0x00;
	m_port_latch[PORT_2] = 0x00;
	m_port_latch[PORT_3] = 0x00;
	m_port_latch[PORT_4] = 0x00;
	m_port_latch[PORT_5] = 0x00;
	m_port_latch[PORT_6] = 0x00;
	m_port_latch[PORT_7] = 0x7f;
	m_port_latch[PORT_8] = 0x3b;
	m_port_latch[PORT_A] = 0x1f;
	m_port_latch[PORT_B] = 0x1f;
	m_port_latch[PORT_C] = 0x00;
	m_port_latch[PORT_D] = 0x00;
	m_port_latch[PORT_E] = 0x00;
	m_port_latch[PORT_F] = 0x00;
	m_port_latch[PORT_H] = 0x00;
	m_port_latch[PORT_Z] = 0x00;

	m_port_function[PORT_0] = 0x01;
	m_port_function[PORT_1] = 0x01;
	m_port_function[PORT_2] = 0x01;
	m_port_function[PORT_3] = 0x01;
	m_port_function[PORT_4] = 0xff;
	m_port_function[PORT_5] = 0xff;
	m_port_function[PORT_6] = 0xff;
	m_port_function[PORT_7] = 0x01;
	m_port_function[PORT_8] = 0x00;
	m_port_function[PORT_A] = 0x00;
	m_port_function[PORT_B] = 0x00;
	m_port_function[PORT_C] = 0x00;
	m_port_function[PORT_D] = 0x00;
	m_port_function[PORT_E] = 0x00;
	m_port_function[PORT_F] = 0x00;
	m_port_function[PORT_H] = 0x00;
	m_port_function[PORT_Z] = 0x00;

	std::fill_n(&m_port_control[0], NUM_PORTS, 0x00);
	m_t8run = 0x00;
	m_trdc = 0x00;
	std::fill_n(&m_tmod[0], 8, 0x00);
	std::fill_n(&m_ffcr[0], 6, 0x00);
	std::fill_n(&m_timer[0], 4, 0x00);
	std::fill_n(&m_timer16[0], 4, 0x00);
	m_watchdog_mode = 0x80;
	for( int i = 0; i < 2; i++ )
	{
		m_serial_control[i] &= 0x80;
		m_serial_mode[i] &= 0x80;
		m_baud_rate[i] = 0x00;
	}
	m_od_enable = 0x00;
	m_ad_mode1 = 0x00;
	m_ad_mode2 = 0x00;
	std::fill_n(&m_int_reg[0], 18, 0x00);
	m_iimc = 0x00;
	std::fill_n(&m_dma_vector[0], 4, 0x00);
	m_block_cs[0] = 0x0000;
	m_block_cs[1] = 0x0000;
	m_block_cs[2] = 0x1000; //FIXME!
	m_block_cs[3] = 0x0000;
	m_block_cs[4] = 0x0000;
	m_block_cs[5] = 0x0000;
	m_external_cs = 0x0000;
	std::fill_n(&m_mem_start_reg[0], 6, 0xff);
	std::fill_n(&m_mem_start_mask[0], 6, 0xff);
	std::fill_n(&m_dram_refresh[0], 2, 0x00);
	std::fill_n(&m_dram_access[0], 2, 0x80);
	m_da_drive = 0x00;
}

uint8_t tmp94c241_device::inte_r(offs_t offset)
{
	return m_int_reg[offset];
}

void tmp94c241_device::inte_w(offs_t offset, uint8_t data)
{
	if ( data & 0x80 )
		data = ( data & 0x7f ) | ( m_int_reg[offset] & 0x80 );
	if ( data & 0x08 )
		data = ( data & 0xf7 ) | ( m_int_reg[offset] & 0x08 );

	m_int_reg[offset] = data;
	m_check_irqs = 1;
}

uint8_t tmp94c241_device::intnmwdt_r(offs_t offset)
{
	return m_int_reg[INTNMWDT];
}

void tmp94c241_device::intnmwdt_w(offs_t offset, uint8_t data)
{
	if ( data & 0x80 )
		data = ( data & 0x7f ) | ( m_int_reg[INTNMWDT] & 0x80 );
	if ( data & 0x08 )
		data = ( data & 0xf7 ) | ( m_int_reg[INTNMWDT] & 0x08 );

	m_int_reg[INTNMWDT] = data;
	m_check_irqs = 1;
}

void tmp94c241_device::iimc_w(uint8_t data)
{
	m_iimc = data;
	m_check_irqs = 1;
}

void tmp94c241_device::intclr_w(uint8_t data)
{
	for( int i = 0; i < NUM_MASKABLE_IRQS; i++ )
	{
		if ( data == tmp94c241_irq_vector_map[i].dma_start_vector )
		{
			// clear interrupt request
			m_int_reg[tmp94c241_irq_vector_map[i].reg] &= ~ tmp94c241_irq_vector_map[i].iff;
			return;
		}
	}
}

void tmp94c241_device::dmav_w(offs_t offset, uint8_t data)
{
	m_dma_vector[offset] = data;
}

void tmp94c241_device::b0cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[0] = (m_block_cs[0] & 0xff) | (data << 8);
	else
		m_block_cs[0] = (m_block_cs[0] & 0xff00) | data;
}

void tmp94c241_device::b1cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[1] = (m_block_cs[1] & 0xff) | (data << 8);
	else
		m_block_cs[1] = (m_block_cs[1] & 0xff00) | data;
}

void tmp94c241_device::b2cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[2] = (m_block_cs[2] & 0xff) | (data << 8);
	else
		m_block_cs[2] = (m_block_cs[2] & 0xff00) | data;
}

void tmp94c241_device::b3cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[3] = (m_block_cs[3] & 0xff) | (data << 8);
	else
		m_block_cs[3] = (m_block_cs[3] & 0xff00) | data;
}

void tmp94c241_device::b4cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[4] = (m_block_cs[4] & 0xff) | (data << 8);
	else
		m_block_cs[4] = (m_block_cs[4] & 0xff00) | data;
}

void tmp94c241_device::b5cs_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_block_cs[5] = (m_block_cs[5] & 0xff) | (data << 8);
	else
		m_block_cs[5] = (m_block_cs[5] & 0xff00) | data;
}

void tmp94c241_device::mamr0_w(offs_t offset, uint8_t data)
{
	SFR_MAMR0 = data;
}

void tmp94c241_device::msar0_w(offs_t offset, uint8_t data)
{
	SFR_MSAR0 = data;
}

uint8_t tmp94c241_device::mamr0_r()
{
	return SFR_MAMR0;
}

uint8_t tmp94c241_device::msar0_r()
{
	return SFR_MSAR0;
}

void tmp94c241_device::mamr1_w(offs_t offset, uint8_t data)
{
	SFR_MAMR1 = data;
}

void tmp94c241_device::msar1_w(offs_t offset, uint8_t data)
{
	SFR_MSAR1 = data;
}

uint8_t tmp94c241_device::mamr1_r()
{
	return SFR_MAMR1;
}

uint8_t tmp94c241_device::msar1_r()
{
	return SFR_MSAR1;
}

void tmp94c241_device::mamr2_w(offs_t offset, uint8_t data)
{
	SFR_MAMR2 = data;
}

void tmp94c241_device::msar2_w(offs_t offset, uint8_t data)
{
	SFR_MSAR2 = data;
}

uint8_t tmp94c241_device::mamr2_r()
{
	return SFR_MAMR2;
}

uint8_t tmp94c241_device::msar2_r()
{
	return SFR_MSAR2;
}

void tmp94c241_device::mamr3_w(offs_t offset, uint8_t data)
{
	SFR_MAMR3 = data;
}

void tmp94c241_device::msar3_w(offs_t offset, uint8_t data)
{
	SFR_MSAR3 = data;
}

uint8_t tmp94c241_device::mamr3_r()
{
	return SFR_MAMR3;
}

uint8_t tmp94c241_device::msar3_r()
{
	return SFR_MSAR3;
}

void tmp94c241_device::mamr4_w(offs_t offset, uint8_t data)
{
	SFR_MAMR4 = data;
}

void tmp94c241_device::msar4_w(offs_t offset, uint8_t data)
{
	SFR_MSAR4 = data;
}

uint8_t tmp94c241_device::mamr4_r()
{
	return SFR_MAMR4;
}

uint8_t tmp94c241_device::msar4_r()
{
	return SFR_MSAR4;
}

void tmp94c241_device::mamr5_w(offs_t offset, uint8_t data)
{
	SFR_MAMR5 = data;
}

void tmp94c241_device::msar5_w(offs_t offset, uint8_t data)
{
	SFR_MSAR5 = data;
}

uint8_t tmp94c241_device::mamr5_r()
{
	return SFR_MAMR5;
}

uint8_t tmp94c241_device::msar5_r()
{
	return SFR_MSAR5;
}

uint8_t tmp94c241_device::t8run_r()
{
	return SFR_T8RUN;
}

void tmp94c241_device::t8run_w(uint8_t data)
{
	SFR_T8RUN = data;

	if ( !TIMER_0_IS_RUNNING )
	{
		UPCOUNTER_0 = 0;
		TIMER_CHANGE_0 = 0;
	}
	if ( !TIMER_1_IS_RUNNING )
	{
		UPCOUNTER_1 = 0;
		TIMER_CHANGE_1 = 0;
	}
	if ( !TIMER_2_IS_RUNNING )
	{
		UPCOUNTER_2 = 0;
		TIMER_CHANGE_2 = 0;
	}
	if ( !TIMER_3_IS_RUNNING )
	{
		UPCOUNTER_3 = 0;
		TIMER_CHANGE_3 = 0;
	}
}

void tmp94c241_device::treg01_w(offs_t offset, uint8_t data)
{

    if (BIT(offset, 0))
		SFR_TREG1 = data;
	else
		SFR_TREG0 = data;
}

uint8_t tmp94c241_device::t01mod_r()
{
	return SFR_T01MOD;
}

void tmp94c241_device::t01mod_w(uint8_t data)
{
	logerror("T01MOD = %02X\n", data);
	SFR_T01MOD = data;
}

uint8_t tmp94c241_device::t02ffcr_r()
{
	return SFR_T02FFCR;
}

enum
{
	FF_INVERT,
	FF_SET,
	FF_CLEAR,
	FF_DONTCARE
};

void tmp94c241_device::change_timer_flipflop(uint8_t flipflop, uint8_t operation)
{
	switch(operation)
	{
	case FF_INVERT:
		switch( flipflop )
		{
		case 0x1: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] ^ 0x01); break;
		case 0x3: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] ^ 0x02); break;
		case 0x4: if ((m_port_function[PORT_D] & 0x01) == 0x01) port_w<PORT_D>(m_port_latch[PORT_D] ^ 0x01); break;
		case 0x6: if ((m_port_function[PORT_D] & 0x10) == 0x10) port_w<PORT_D>(m_port_latch[PORT_D] ^ 0x10); break;
		case 0x7: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] ^ 0x01); break;
		case 0x8: if ((m_port_function[PORT_E] & 0x01) == 0x01) port_w<PORT_E>(m_port_latch[PORT_E] ^ 0x01); break;
		case 0xa: if ((m_port_function[PORT_E] & 0x10) == 0x10) port_w<PORT_E>(m_port_latch[PORT_E] ^ 0x10); break;
		case 0xb: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] ^ 0x02); break;
		}
		break;
	case FF_SET:
		switch( flipflop )
		{
		case 0x1: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] & 0x01); break;
		case 0x3: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] & 0x02); break;
		case 0x4: if ((m_port_function[PORT_D] & 0x01) == 0x01) port_w<PORT_D>(m_port_latch[PORT_D] & 0x01); break;
		case 0x6: if ((m_port_function[PORT_D] & 0x10) == 0x10) port_w<PORT_D>(m_port_latch[PORT_D] & 0x10); break;
		case 0x7: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] & 0x01); break;
		case 0x8: if ((m_port_function[PORT_E] & 0x01) == 0x01) port_w<PORT_E>(m_port_latch[PORT_E] & 0x01); break;
		case 0xa: if ((m_port_function[PORT_E] & 0x10) == 0x10) port_w<PORT_E>(m_port_latch[PORT_E] & 0x10); break;
		case 0xb: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] & 0x02); break;
		}
		break;
	case FF_CLEAR:
		switch( flipflop )
		{
		case 0x1: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] | ~0x01); break;
		case 0x3: if ((m_port_function[PORT_C] & 0x03) == 0x02) port_w<PORT_C>(m_port_latch[PORT_C] | ~0x02); break;
		case 0x4: if ((m_port_function[PORT_D] & 0x01) == 0x01) port_w<PORT_D>(m_port_latch[PORT_D] | ~0x01); break;
		case 0x6: if ((m_port_function[PORT_D] & 0x10) == 0x10) port_w<PORT_D>(m_port_latch[PORT_D] | ~0x10); break;
		case 0x7: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] | ~0x01); break;
		case 0x8: if ((m_port_function[PORT_E] & 0x01) == 0x01) port_w<PORT_E>(m_port_latch[PORT_E] | ~0x01); break;
		case 0xa: if ((m_port_function[PORT_E] & 0x10) == 0x10) port_w<PORT_E>(m_port_latch[PORT_E] | ~0x10); break;
		case 0xb: if ((m_port_function[PORT_C] & 0x03) == 0x03) port_w<PORT_C>(m_port_latch[PORT_C] | ~0x02); break;
		}
		break;
	}
}

void tmp94c241_device::t02ffcr_w(uint8_t data)
{
	switch( data & 0x0c )
	{
	case 0x00:
		change_timer_flipflop( 1, FF_INVERT );
		break;
	case 0x04:
		change_timer_flipflop( 1, FF_SET );
		break;
	case 0x08:
		change_timer_flipflop( 1, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		change_timer_flipflop( 3, FF_INVERT );
		break;
	case 0x40:
		change_timer_flipflop( 3, FF_SET );
		break;
	case 0x80:
		change_timer_flipflop( 3, FF_CLEAR );
		break;
	}

	SFR_T02FFCR = data | 0xcc;
}

void tmp94c241_device::treg23_w(offs_t offset, uint8_t data)
{

    if (BIT(offset, 0))
		SFR_TREG3 = data;
	else
		SFR_TREG2 = data;
}

uint8_t tmp94c241_device::t23mod_r()
{
	return SFR_T23MOD;
}

void tmp94c241_device::t23mod_w(uint8_t data)
{
	logerror("T23MOD = %02X\n", data);

	SFR_T23MOD = data;
}

uint8_t tmp94c241_device::trdc_r()
{
	return m_trdc;
}

void tmp94c241_device::trdc_w(uint8_t data)
{
	m_trdc = data;
}

void tmp94c241_device::treg45_w(offs_t offset, uint16_t data)
{
	if (offset < 2)
		SFR_TREG4 = data;
	else
		SFR_TREG5 = data;
}

void tmp94c241_device::treg67_w(offs_t offset, uint16_t data)
{
	if (offset < 2)
		SFR_TREG6 = data;
	else
		SFR_TREG7 = data;
}

void tmp94c241_device::treg89_w(offs_t offset, uint16_t data)
{
	if (offset < 2)
		SFR_TREG8 = data;
	else
		SFR_TREG9 = data;
}

void tmp94c241_device::tregab_w(offs_t offset, uint16_t data)
{
	if (offset < 2)
		SFR_TREGA = data;
	else
		SFR_TREGB = data;
}

uint8_t tmp94c241_device::t4mod_r()
{
	return SFR_T4MOD;
}

void tmp94c241_device::t4mod_w(uint8_t data)
{
	logerror("T4MOD = %02X\n", data);
	SFR_T4MOD = data;
}

uint8_t tmp94c241_device::t6mod_r()
{
	return SFR_T6MOD;
}

void tmp94c241_device::t6mod_w(uint8_t data)
{
	logerror("T6MOD = %02X\n", data);
	SFR_T6MOD = data;
}

uint8_t tmp94c241_device::t8mod_r()
{
	return SFR_T8MOD;
}

void tmp94c241_device::t8mod_w(uint8_t data)
{
	logerror("T8MOD = %02X\n", data);
	SFR_T8MOD = data;
}

uint8_t tmp94c241_device::tamod_r()
{
	return SFR_TAMOD;
}

void tmp94c241_device::tamod_w(uint8_t data)
{
	logerror("TAMOD = %02X\n", data);
	SFR_TAMOD = data;
}


uint8_t tmp94c241_device::t4ffcr_r()
{
	return SFR_T4FFCR;
}

void tmp94c241_device::t4ffcr_w(uint8_t data)
{
	switch( data & 0x03 )
	{
	case 0x00:
		change_timer_flipflop( 4, FF_INVERT );
		break;
	case 0x01:
		change_timer_flipflop( 4, FF_SET );
		break;
	case 0x02:
		change_timer_flipflop( 4, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		change_timer_flipflop( 5, FF_INVERT );
		break;
	case 0x40:
		change_timer_flipflop( 5, FF_SET );
		break;
	case 0x80:
		change_timer_flipflop( 5, FF_CLEAR );
		break;
	}

	SFR_T4FFCR = data | 0xc3;
}

uint8_t tmp94c241_device::t8ffcr_r()
{
	return SFR_T8FFCR;
}

void tmp94c241_device::t8ffcr_w(uint8_t data)
{
	switch( data & 0x03 )
	{
	case 0x00:
		change_timer_flipflop( 8, FF_INVERT );
		break;
	case 0x01:
		change_timer_flipflop( 8, FF_SET );
		break;
	case 0x02:
		change_timer_flipflop( 8, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		change_timer_flipflop( 9, FF_INVERT );
		break;
	case 0x40:
		change_timer_flipflop( 9, FF_SET );
		break;
	case 0x80:
		change_timer_flipflop( 9, FF_CLEAR );
		break;
	}

	SFR_T8FFCR = data | 0xc3;
}


uint8_t tmp94c241_device::t6ffcr_r()
{
	return SFR_T6FFCR;
}

void tmp94c241_device::t6ffcr_w(uint8_t data)
{
	switch( data & 0x03 )
	{
	case 0x00:
		change_timer_flipflop( 6, FF_INVERT );
		break;
	case 0x01:
		change_timer_flipflop( 6, FF_SET );
		break;
	case 0x02:
		change_timer_flipflop( 6, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		change_timer_flipflop( 7, FF_INVERT );
		break;
	case 0x40:
		change_timer_flipflop( 7, FF_SET );
		break;
	case 0x80:
		change_timer_flipflop( 7, FF_CLEAR );
		break;
	}

	SFR_T6FFCR = data | 0xc3;
}

uint8_t tmp94c241_device::taffcr_r()
{
	return SFR_TAFFCR;
}

void tmp94c241_device::taffcr_w(uint8_t data)
{
	switch( data & 0x03 )
	{
	case 0x00:
		change_timer_flipflop( 0xa, FF_INVERT );
		break;
	case 0x01:
		change_timer_flipflop( 0xa, FF_SET );
		break;
	case 0x02:
		change_timer_flipflop( 0xa, FF_CLEAR );
		break;
	}
	switch( data & 0xc0 )
	{
	case 0x00:
		change_timer_flipflop( 0xb, FF_INVERT );
		break;
	case 0x40:
		change_timer_flipflop( 0xb, FF_SET );
		break;
	case 0x80:
		change_timer_flipflop( 0xb, FF_CLEAR );
		break;
	}

	SFR_TAFFCR = data | 0xc3;
}

uint8_t tmp94c241_device::t16run_r()
{
	return SFR_T16RUN;
}

void tmp94c241_device::t16run_w(uint8_t data)
{
	SFR_T16RUN = data;

	if ( !TIMER_4_IS_RUNNING )
	{
		UPCOUNTER_4 = 0;
		TIMER_CHANGE_4 = 0;
	}
	if ( !TIMER_6_IS_RUNNING )
	{
		UPCOUNTER_6 = 0;
		TIMER_CHANGE_6 = 0;
	}
	if ( !TIMER_8_IS_RUNNING )
	{
		UPCOUNTER_8 = 0;
		TIMER_CHANGE_8 = 0;
	}
	if ( !TIMER_A_IS_RUNNING )
	{
		UPCOUNTER_A = 0;
		TIMER_CHANGE_A = 0;
	}
}


uint8_t tmp94c241_device::cap45_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 0] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 0] & 0x00ff;
}

uint8_t tmp94c241_device::cap67_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 2] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 2] & 0x00ff;
}

uint8_t tmp94c241_device::cap89_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 4] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 4] & 0x00ff;
}

uint8_t tmp94c241_device::capab_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_t16_cap[(offset >> 1) + 6] >> 8;
	else
		return m_t16_cap[(offset >> 1) + 6] & 0x00ff;
}

uint8_t tmp94c241_device::wdmod_r()
{
	return m_watchdog_mode;
}

void tmp94c241_device::wdmod_w(uint8_t data)
{
	m_watchdog_mode = data;
}

void tmp94c241_device::wdcr_w(uint8_t data)
{
}


uint8_t tmp94c241_device::sc0buf_r()
{
	return 0;
}

void tmp94c241_device::sc0buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[INTES0] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp94c241_device::sc0cr_r()
{
	uint8_t reg = m_serial_control[0];
	if (!machine().side_effects_disabled())
		m_serial_control[0] &= 0xe3;
	return reg;
}

void tmp94c241_device::sc0cr_w(uint8_t data)
{
	m_serial_control[0] = data;
}

uint8_t tmp94c241_device::sc0mod_r()
{
	return m_serial_mode[0];
}

void tmp94c241_device::sc0mod_w(uint8_t data)
{
	m_serial_mode[0] = data;
}

uint8_t tmp94c241_device::br0cr_r()
{
	return m_baud_rate[0];
}

void tmp94c241_device::br0cr_w(uint8_t data)
{
	m_baud_rate[0] = data;
}

uint8_t tmp94c241_device::sc1buf_r()
{
	return 0;
}

void tmp94c241_device::sc1buf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[INTES1] |= 0x80;
	m_check_irqs = 1;
}

uint8_t tmp94c241_device::sc1cr_r()
{
	uint8_t reg = m_serial_control[1];
	if (!machine().side_effects_disabled())
		m_serial_control[1] &= 0xe3;
	return reg;
}

void tmp94c241_device::sc1cr_w(uint8_t data)
{
	m_serial_control[1] = data;
}

uint8_t tmp94c241_device::sc1mod_r()
{
	return m_serial_mode[1];
}

void tmp94c241_device::sc1mod_w(uint8_t data)
{
	m_serial_mode[1] = data;
}

uint8_t tmp94c241_device::br1cr_r()
{
	return m_baud_rate[1];
}

void tmp94c241_device::br1cr_w(uint8_t data)
{
	m_baud_rate[1] = data;
}

uint8_t tmp94c241_device::ode_r()
{
	return m_od_enable;
}

void tmp94c241_device::ode_w(uint8_t data)
{
	m_od_enable = data;
}

uint8_t tmp94c241_device::admod1_r()
{
	return m_ad_mode1;
}

void tmp94c241_device::admod1_w(uint8_t data)
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

uint8_t tmp94c241_device::admod2_r()
{
	return m_ad_mode2;
}

void tmp94c241_device::admod2_w(uint8_t data)
{
	m_ad_mode2 = data;
}

uint8_t tmp94c241_device::adreg_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_ad_result[offset >> 1] >> 2;
	else
		return m_ad_result[offset >> 1] << 6 | 0x3f;
}

uint8_t tmp94c241_device::dadrv_r()
{
	return m_da_drive;
}

void tmp94c241_device::dadrv_w(uint8_t data)
{
	m_da_drive = data;
}

void tmp94c241_device::dareg_w(offs_t offset, uint8_t data)
{
}

template <uint8_t P>
void tmp94c241_device::port_w(uint8_t data)
{
	m_port_latch[P] = data;
	m_port_write[P](0, data, 0xff);
}

template <uint8_t P>
uint8_t tmp94c241_device::port_r()
{
	return m_port_read[P](0);
}

template <uint8_t P>
void tmp94c241_device::port_cr_w(uint8_t data)
{
	m_port_control[P] = data;
}

template <uint8_t P>
void tmp94c241_device::port_fc_w(uint8_t data)
{
	m_port_function[P] = data;
}

//**************************************************************************
//  INTERNAL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  internal_mem - memory map for internal RAM and
//  I/O registers
//-------------------------------------------------

void tmp94c241_device::internal_mem(address_map &map)
{
	map(0x000000, 0x000000).rw(FUNC(tmp94c241_device::port_r<PORT_0>), FUNC(tmp94c241_device::port_w<PORT_0>));
	map(0x000002, 0x000002).w(FUNC(tmp94c241_device::port_cr_w<PORT_0>));
	map(0x000003, 0x000003).w(FUNC(tmp94c241_device::port_fc_w<PORT_0>));
	map(0x000004, 0x000004).rw(FUNC(tmp94c241_device::port_r<PORT_1>), FUNC(tmp94c241_device::port_w<PORT_1>));
	map(0x000006, 0x000006).w(FUNC(tmp94c241_device::port_cr_w<PORT_1>));
	map(0x000007, 0x000007).w(FUNC(tmp94c241_device::port_fc_w<PORT_1>));
	map(0x000008, 0x000008).rw(FUNC(tmp94c241_device::port_r<PORT_2>), FUNC(tmp94c241_device::port_w<PORT_2>));
	map(0x00000a, 0x00000a).w(FUNC(tmp94c241_device::port_cr_w<PORT_2>));
	map(0x00000b, 0x00000b).w(FUNC(tmp94c241_device::port_fc_w<PORT_2>));
	map(0x00000c, 0x00000c).rw(FUNC(tmp94c241_device::port_r<PORT_3>), FUNC(tmp94c241_device::port_w<PORT_3>));
	map(0x00000e, 0x00000e).w(FUNC(tmp94c241_device::port_cr_w<PORT_3>));
	map(0x00000f, 0x00000f).w(FUNC(tmp94c241_device::port_fc_w<PORT_3>));
	map(0x000010, 0x000010).rw(FUNC(tmp94c241_device::port_r<PORT_4>), FUNC(tmp94c241_device::port_w<PORT_4>));
	map(0x000012, 0x000012).w(FUNC(tmp94c241_device::port_cr_w<PORT_4>));
	map(0x000013, 0x000013).w(FUNC(tmp94c241_device::port_fc_w<PORT_4>));
	map(0x000014, 0x000014).rw(FUNC(tmp94c241_device::port_r<PORT_5>), FUNC(tmp94c241_device::port_w<PORT_5>));
	map(0x000016, 0x000016).w(FUNC(tmp94c241_device::port_cr_w<PORT_5>));
	map(0x000017, 0x000017).w(FUNC(tmp94c241_device::port_fc_w<PORT_5>));
	map(0x000018, 0x000018).rw(FUNC(tmp94c241_device::port_r<PORT_6>), FUNC(tmp94c241_device::port_w<PORT_6>));
	map(0x00001a, 0x00001a).w(FUNC(tmp94c241_device::port_cr_w<PORT_6>));
	map(0x00001b, 0x00001b).w(FUNC(tmp94c241_device::port_fc_w<PORT_6>));
	map(0x00001c, 0x00001c).rw(FUNC(tmp94c241_device::port_r<PORT_7>), FUNC(tmp94c241_device::port_w<PORT_7>));
	map(0x00001e, 0x00001e).w(FUNC(tmp94c241_device::port_cr_w<PORT_7>));
	map(0x00001f, 0x00001f).w(FUNC(tmp94c241_device::port_fc_w<PORT_7>));
	map(0x000020, 0x000020).rw(FUNC(tmp94c241_device::port_r<PORT_8>), FUNC(tmp94c241_device::port_w<PORT_8>));
	map(0x000022, 0x000022).w(FUNC(tmp94c241_device::port_cr_w<PORT_8>));
	map(0x000023, 0x000023).w(FUNC(tmp94c241_device::port_fc_w<PORT_8>));
	map(0x000028, 0x000028).rw(FUNC(tmp94c241_device::port_r<PORT_A>), FUNC(tmp94c241_device::port_w<PORT_A>));
	map(0x00002b, 0x00002b).w(FUNC(tmp94c241_device::port_fc_w<PORT_A>));
	map(0x00002c, 0x00002c).rw(FUNC(tmp94c241_device::port_r<PORT_B>), FUNC(tmp94c241_device::port_w<PORT_B>));
	map(0x00002f, 0x00002f).w(FUNC(tmp94c241_device::port_fc_w<PORT_B>));
	map(0x000030, 0x000030).rw(FUNC(tmp94c241_device::port_r<PORT_C>), FUNC(tmp94c241_device::port_w<PORT_C>));
	map(0x000032, 0x000032).w(FUNC(tmp94c241_device::port_cr_w<PORT_C>));
	map(0x000033, 0x000033).w(FUNC(tmp94c241_device::port_fc_w<PORT_C>));
	map(0x000034, 0x000034).rw(FUNC(tmp94c241_device::port_r<PORT_D>), FUNC(tmp94c241_device::port_w<PORT_D>));
	map(0x000036, 0x000036).w(FUNC(tmp94c241_device::port_cr_w<PORT_D>));
	map(0x000037, 0x000037).w(FUNC(tmp94c241_device::port_fc_w<PORT_D>));
	map(0x000038, 0x000038).rw(FUNC(tmp94c241_device::port_r<PORT_E>), FUNC(tmp94c241_device::port_w<PORT_E>));
	map(0x00003a, 0x00003a).w(FUNC(tmp94c241_device::port_cr_w<PORT_E>));
	map(0x00003b, 0x00003b).w(FUNC(tmp94c241_device::port_fc_w<PORT_E>));
	map(0x00003c, 0x00003c).rw(FUNC(tmp94c241_device::port_r<PORT_F>), FUNC(tmp94c241_device::port_w<PORT_F>));
	map(0x00003e, 0x00003e).w(FUNC(tmp94c241_device::port_cr_w<PORT_F>));
	map(0x00003f, 0x00003f).w(FUNC(tmp94c241_device::port_fc_w<PORT_F>));
	map(0x000040, 0x000040).r(FUNC(tmp94c241_device::port_r<PORT_G>));
	map(0x000044, 0x000044).rw(FUNC(tmp94c241_device::port_r<PORT_H>), FUNC(tmp94c241_device::port_w<PORT_H>));
	map(0x000046, 0x000046).w(FUNC(tmp94c241_device::port_cr_w<PORT_H>));
	map(0x000047, 0x000047).w(FUNC(tmp94c241_device::port_fc_w<PORT_H>));
	map(0x000068, 0x000068).rw(FUNC(tmp94c241_device::port_r<PORT_Z>), FUNC(tmp94c241_device::port_w<PORT_Z>));
	map(0x00006a, 0x00006a).w(FUNC(tmp94c241_device::port_cr_w<PORT_Z>));
	map(0x000080, 0x000080).rw(FUNC(tmp94c241_device::t8run_r), FUNC(tmp94c241_device::t8run_w));
	map(0x000081, 0x000081).rw(FUNC(tmp94c241_device::trdc_r), FUNC(tmp94c241_device::trdc_w));
	map(0x000082, 0x000082).rw(FUNC(tmp94c241_device::t02ffcr_r), FUNC(tmp94c241_device::t02ffcr_w));
	map(0x000084, 0x000084).rw(FUNC(tmp94c241_device::t01mod_r), FUNC(tmp94c241_device::t01mod_w));
	map(0x000085, 0x000085).rw(FUNC(tmp94c241_device::t23mod_r), FUNC(tmp94c241_device::t23mod_w));
	map(0x000088, 0x000089).w(FUNC(tmp94c241_device::treg01_w));
	map(0x00008a, 0x00008b).w(FUNC(tmp94c241_device::treg23_w));
	map(0x000090, 0x000093).w(FUNC(tmp94c241_device::treg45_w));
	map(0x000094, 0x000097).r(FUNC(tmp94c241_device::cap45_r));
	map(0x000098, 0x000098).rw(FUNC(tmp94c241_device::t4mod_r), FUNC(tmp94c241_device::t4mod_w));
	map(0x000099, 0x000099).rw(FUNC(tmp94c241_device::t4ffcr_r), FUNC(tmp94c241_device::t4ffcr_w));
	map(0x00009e, 0x00009e).rw(FUNC(tmp94c241_device::t16run_r), FUNC(tmp94c241_device::t16run_w));
	map(0x0000a0, 0x0000a3).w(FUNC(tmp94c241_device::treg67_w));
	map(0x0000a4, 0x0000a7).r(FUNC(tmp94c241_device::cap67_r));
	map(0x0000a8, 0x0000a8).rw(FUNC(tmp94c241_device::t6mod_r), FUNC(tmp94c241_device::t6mod_w));
	map(0x0000a9, 0x0000a9).rw(FUNC(tmp94c241_device::t6ffcr_r), FUNC(tmp94c241_device::t6ffcr_w));
	map(0x0000b0, 0x0000b3).w(FUNC(tmp94c241_device::treg89_w));
	map(0x0000b4, 0x0000b7).r(FUNC(tmp94c241_device::cap89_r));
	map(0x0000b8, 0x0000b8).rw(FUNC(tmp94c241_device::t8mod_r), FUNC(tmp94c241_device::t8mod_w));
	map(0x0000b9, 0x0000b9).rw(FUNC(tmp94c241_device::t8ffcr_r), FUNC(tmp94c241_device::t8ffcr_w));
	map(0x0000c0, 0x0000c3).w(FUNC(tmp94c241_device::tregab_w));
	map(0x0000c4, 0x0000c7).r(FUNC(tmp94c241_device::capab_r));
	map(0x0000c8, 0x0000c8).rw(FUNC(tmp94c241_device::tamod_r), FUNC(tmp94c241_device::tamod_w));
	map(0x0000c9, 0x0000c9).rw(FUNC(tmp94c241_device::taffcr_r), FUNC(tmp94c241_device::taffcr_w));
	map(0x0000d0, 0x0000d0).rw(FUNC(tmp94c241_device::sc0buf_r), FUNC(tmp94c241_device::sc0buf_w));
	map(0x0000d1, 0x0000d1).rw(FUNC(tmp94c241_device::sc0cr_r), FUNC(tmp94c241_device::sc0cr_w));
	map(0x0000d2, 0x0000d2).rw(FUNC(tmp94c241_device::sc0mod_r), FUNC(tmp94c241_device::sc0mod_w));
	map(0x0000d3, 0x0000d3).rw(FUNC(tmp94c241_device::br0cr_r), FUNC(tmp94c241_device::br0cr_w));
	map(0x0000d4, 0x0000d4).rw(FUNC(tmp94c241_device::sc1buf_r), FUNC(tmp94c241_device::sc1buf_w));
	map(0x0000d5, 0x0000d5).rw(FUNC(tmp94c241_device::sc1cr_r), FUNC(tmp94c241_device::sc1cr_w));
	map(0x0000d6, 0x0000d6).rw(FUNC(tmp94c241_device::sc1mod_r), FUNC(tmp94c241_device::sc1mod_w));
	map(0x0000d7, 0x0000d7).rw(FUNC(tmp94c241_device::br1cr_r), FUNC(tmp94c241_device::br1cr_w));
	map(0x0000e0, 0x0000f0).rw(FUNC(tmp94c241_device::inte_r), FUNC(tmp94c241_device::inte_w));
	map(0x0000f6, 0x0000f6).w(FUNC(tmp94c241_device::iimc_w));
	map(0x0000f7, 0x0000f7).rw(FUNC(tmp94c241_device::intnmwdt_r), FUNC(tmp94c241_device::intnmwdt_w));
	map(0x0000f8, 0x0000f8).w(FUNC(tmp94c241_device::intclr_w));
	map(0x000100, 0x000103).w(FUNC(tmp94c241_device::dmav_w));
	map(0x000110, 0x000110).rw(FUNC(tmp94c241_device::wdmod_r), FUNC(tmp94c241_device::wdmod_w));
	map(0x000111, 0x000111).w(FUNC(tmp94c241_device::wdcr_w));
	map(0x000120, 0x000127).r(FUNC(tmp94c241_device::adreg_r));
	map(0x000128, 0x000128).rw(FUNC(tmp94c241_device::admod1_r), FUNC(tmp94c241_device::admod1_w));
	map(0x000129, 0x000129).rw(FUNC(tmp94c241_device::admod2_r), FUNC(tmp94c241_device::admod2_w));
	map(0x000130, 0x000131).w(FUNC(tmp94c241_device::dareg_w));
	map(0x000132, 0x000132).rw(FUNC(tmp94c241_device::dadrv_r), FUNC(tmp94c241_device::dadrv_w));
	map(0x000140, 0x000141).w(FUNC(tmp94c241_device::b0cs_w));
	map(0x000142, 0x000142).rw(FUNC(tmp94c241_device::mamr0_r), FUNC(tmp94c241_device::mamr0_w));
	map(0x000143, 0x000143).rw(FUNC(tmp94c241_device::msar0_r), FUNC(tmp94c241_device::msar0_w));
	map(0x000144, 0x000145).w(FUNC(tmp94c241_device::b1cs_w));
	map(0x000146, 0x000146).rw(FUNC(tmp94c241_device::mamr1_r), FUNC(tmp94c241_device::mamr1_w));
	map(0x000147, 0x000147).rw(FUNC(tmp94c241_device::msar1_r), FUNC(tmp94c241_device::msar1_w));
	map(0x000148, 0x000149).w(FUNC(tmp94c241_device::b2cs_w));
	map(0x00014a, 0x00014a).rw(FUNC(tmp94c241_device::mamr2_r), FUNC(tmp94c241_device::mamr2_w));
	map(0x00014b, 0x00014b).rw(FUNC(tmp94c241_device::msar2_r), FUNC(tmp94c241_device::msar2_w));
	map(0x00014c, 0x00014d).w(FUNC(tmp94c241_device::b3cs_w));
	map(0x00014e, 0x00014e).rw(FUNC(tmp94c241_device::mamr3_r), FUNC(tmp94c241_device::mamr3_w));
	map(0x00014f, 0x00014f).rw(FUNC(tmp94c241_device::msar3_r), FUNC(tmp94c241_device::msar3_w));
	map(0x000150, 0x000151).w(FUNC(tmp94c241_device::b4cs_w));
	map(0x000152, 0x000152).rw(FUNC(tmp94c241_device::mamr4_r), FUNC(tmp94c241_device::mamr4_w));
	map(0x000153, 0x000153).rw(FUNC(tmp94c241_device::msar4_r), FUNC(tmp94c241_device::msar4_w));
	map(0x000154, 0x000155).w(FUNC(tmp94c241_device::b5cs_w));
	map(0x000156, 0x000156).rw(FUNC(tmp94c241_device::mamr5_r), FUNC(tmp94c241_device::mamr5_w));
	map(0x000157, 0x000157).rw(FUNC(tmp94c241_device::msar5_r), FUNC(tmp94c241_device::msar5_w));
	map(0x000400, 0x000bff).ram();
}

//**************************************************************************
//  EXECUTION CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  tlcs900_check_hdma -
//-------------------------------------------------

void tmp94c241_device::tlcs900_check_hdma()
{
}


//-------------------------------------------------
//  tlcs900_check_irqs -
//-------------------------------------------------

void tmp94c241_device::tlcs900_check_irqs()
{
	int irq_vectors[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
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
	// the smaller the vector value, the higher the priority
	for( i = NUM_MASKABLE_IRQS - 1; i >= 0; i-- )
	{
		if ( m_int_reg[tmp94c241_irq_vector_map[i].reg] & tmp94c241_irq_vector_map[i].iff )
		{
			switch( tmp94c241_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_int_reg[ tmp94c241_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_int_reg[ tmp94c241_irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	for( i = std::max( 1, ( ( m_sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
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
		uint8_t vector = tmp94c241_irq_vector_map[irq].vector;

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
		m_int_reg[ tmp94c241_irq_vector_map[irq].reg ] &= ~ tmp94c241_irq_vector_map[irq].iff;
	}
}


//-------------------------------------------------
//  tlcs900_handle_ad -
//-------------------------------------------------

void tmp94c241_device::tlcs900_handle_ad()
{
}


//-------------------------------------------------
//  tlcs900_handle_timers -
//-------------------------------------------------

void tmp94c241_device::tlcs900_handle_timers()
{
	uint32_t  old_pre = m_timer_pre;

	if ( PRESCALER_IS_ACTIVE )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( TIMER_0_IS_RUNNING )
	{
		switch( T0_INPUT_CLOCK )
		{
		case 0:  /* TIO */
			break;
		case 1:  /* T1 */
			TIMER_CHANGE_0 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 2:  /* T4 */
			TIMER_CHANGE_0 += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 3:  /* T16 */
			TIMER_CHANGE_0 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_0 > 0; TIMER_CHANGE_0-- )
		{
			UPCOUNTER_0++;
			if ( UPCOUNTER_0 == SFR_TREG0 )
			{
				if ( TO1_OPERATING_MODE == MODE_8BIT_TIMER )
					TIMER_CHANGE_1++;

				if ( (m_ffcr[0] & 0x03) == 0b10 )
					change_timer_flipflop( 1, FF_INVERT );

				/* In 16bit timer mode the timer should not be reset */
				if ( TO1_OPERATING_MODE != MODE_16BIT_TIMER )
				{
					UPCOUNTER_0 = 0;
					m_int_reg[INTET01] |= 0x08;
					m_check_irqs = 1;
				}
			}
		}
	}

	/* Timer 1 */
	if ( TIMER_1_IS_RUNNING )
	{
		switch( T1_INPUT_CLOCK )
		{
		case 0x00:  /* TO0TRG */
			break;
		case 0x01:  /* T1 */
			TIMER_CHANGE_1 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 0x02:  /* T16 */
			TIMER_CHANGE_1 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x03:  /* T256 */
			TIMER_CHANGE_1 += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; TIMER_CHANGE_1 > 0; TIMER_CHANGE_1-- )
		{
			UPCOUNTER_1 += 1;
			if ( UPCOUNTER_1 == SFR_TREG1 )
			{
				UPCOUNTER_1 = 0;
				m_int_reg[INTET01] |= 0x80;
				m_check_irqs = 1;

				if ( (m_ffcr[0] & 0x03) == 0b11 )
					change_timer_flipflop( 1, FF_INVERT );

				/* In 16bit timer mode also reset timer 0 */
				if ( TO1_OPERATING_MODE == MODE_16BIT_TIMER )
					UPCOUNTER_1 = 0;
			}
		}
	}

	/* Timer 2 */
	if ( TIMER_2_IS_RUNNING )
	{
		switch( T2_INPUT_CLOCK )
		{
		case 0: /* invalid */
		case 1: /* T1 */
			TIMER_CHANGE_2 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 2: /* T4 */
			TIMER_CHANGE_2 += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 3: /* T16 */
			TIMER_CHANGE_2 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_2 > 0; TIMER_CHANGE_2-- )
		{
			UPCOUNTER_2++;
			if ( UPCOUNTER_2 == SFR_TREG2 )
			{
				if ( T23_OPERATING_MODE == MODE_8BIT_TIMER )
					TIMER_CHANGE_3++;

				if ( ((m_ffcr[0] & 0x30) >> 4) == 0b10 )
					change_timer_flipflop( 3, FF_INVERT );

				/* In 16bit timer mode the timer should not be reset */
				if ( T23_OPERATING_MODE != MODE_16BIT_TIMER )
				{
					UPCOUNTER_2 = 0;
					m_int_reg[INTET23] |= 0x08;
					m_check_irqs = 1;
				}
			}
		}
	}

	/* Timer 3 */
	if ( TIMER_3_IS_RUNNING )
	{
		switch( T3_INPUT_CLOCK )
		{
		case 0: /* TO2TRG */
			break;
		case 1: /* T1 */
			TIMER_CHANGE_3 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 2: /* T16 */
			TIMER_CHANGE_3 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 3: /* T256 */
			TIMER_CHANGE_3 += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; TIMER_CHANGE_3 > 0; TIMER_CHANGE_3-- )
		{
			UPCOUNTER_3++;
			if ( UPCOUNTER_3 == SFR_TREG3 )
			{
				UPCOUNTER_3 = 0;
				m_int_reg[INTET23] |= 0x80;
				m_check_irqs = 1;

				if ( ((m_ffcr[0] & 0x30) >> 4) == 0b11 )
					change_timer_flipflop( 3, FF_INVERT );

				/* In 16bit timer mode also reset timer 2 */
				if ( T23_OPERATING_MODE == MODE_16BIT_TIMER )
					UPCOUNTER_2 = 0;
			}
		}
	}

	/* Timer 4 */
	if ( TIMER_4_IS_RUNNING )
	{
		switch( T4_INPUT_CLOCK )
		{
		case 0x00:  /* TIA */
			// TODO: implement-me!
			break;
		case 0x01:  /* T1 */
			TIMER_CHANGE_4 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 0x02:  /* T4 */
			TIMER_CHANGE_4 += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 0x03:  /* T16 */
			TIMER_CHANGE_4 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_4 > 0; TIMER_CHANGE_4-- )
		{
			UPCOUNTER_4++;
			if ( ((UPCOUNTER_4 == SFR_TREG5) && EQ5T4) ||
				((UPCOUNTER_4 == SFR_TREG4) && EQ4T4) )
			{
				change_timer_flipflop( 4, FF_INVERT );
				UPCOUNTER_4 = 0;
				m_int_reg[INTET45] |= 0x08;
				m_check_irqs = 1;
			}
		}
	}


	/* Timer 6 */
	if ( TIMER_6_IS_RUNNING )
	{
		switch( T6_INPUT_CLOCK )
		{
		case 0: /* TIA */
			// TODO: implement-me!
			break;
		case 1: /* T1 */
			TIMER_CHANGE_6 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 2: /* T4 */
			TIMER_CHANGE_6 += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 3: /* T16 */
			TIMER_CHANGE_6 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_6 > 0; TIMER_CHANGE_6-- )
		{
			UPCOUNTER_6++;
			if ( ((UPCOUNTER_6 == SFR_TREG7) && EQ7T6) ||
				((UPCOUNTER_6 == SFR_TREG6) && EQ6T6) )
			{
				change_timer_flipflop( 6, FF_INVERT );
				UPCOUNTER_6 = 0;
				m_int_reg[INTET67] |= 0x08;
				m_check_irqs = 1;
			}
		}
	}


	/* Timer 8 */
	if ( TIMER_8_IS_RUNNING )
	{
		switch( T8_INPUT_CLOCK )
		{
		case 0x00:  /* TIA */
			// TODO: implement-me!
			break;
		case 0x01:  /* T1 */
			TIMER_CHANGE_8 += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 0x02:  /* T4 */
			TIMER_CHANGE_8 += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 0x03:  /* T16 */
			TIMER_CHANGE_8 += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_8 > 0; TIMER_CHANGE_8-- )
		{
			UPCOUNTER_8++;
			if ( ((UPCOUNTER_8 == SFR_TREG9) && EQ9T8) ||
				((UPCOUNTER_8 == SFR_TREG8) && EQ8T8) )
			{
				change_timer_flipflop( 8, FF_INVERT );
				UPCOUNTER_8 = 0;
				m_int_reg[INTET89] |= 0x08;
				m_check_irqs = 1;
			}
		}
	}


	/* Timer A */
	if ( TIMER_A_IS_RUNNING )
	{
		switch( TA_INPUT_CLOCK )
		{
		case 0: /* TIA */
			// TODO: implement-me!
			break;
		case 1: /* T1 */
			TIMER_CHANGE_A += ( m_timer_pre >> 3 ) - ( old_pre >> 3 );
			break;
		case 2: /* T4 */
			TIMER_CHANGE_A += ( m_timer_pre >> 5 ) - ( old_pre >> 5 );
			break;
		case 3: /* T16 */
			TIMER_CHANGE_A += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		}

		for( ; TIMER_CHANGE_A > 0; TIMER_CHANGE_A-- )
		{
			UPCOUNTER_A++;
			if ( ((UPCOUNTER_A == SFR_TREGA) && EQATA) ||
				((UPCOUNTER_A == SFR_TREGB) && EQBTA) )
			{
				change_timer_flipflop( 0xa, FF_INVERT );
				UPCOUNTER_A = 0;
				m_int_reg[INTETAB] |= 0x08;
				m_check_irqs = 1;
			}
		}
	}

	m_timer_pre &= 0xffffff;
}


//-------------------------------------------------
//  execute_set_input - called when a synchronized
//  input is changed
//-------------------------------------------------

void tmp94c241_device::execute_set_input(int input, int level)
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
					m_int_reg[INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if (level == ASSERT_LINE)
					m_int_reg[INTE0AD] |= 0x08;
				else
					m_int_reg[INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT4:
		if (m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE45] |= 0x08;
		}
		else if (m_level[TLCS900_INT4] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE45] &= ~0x08;
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if (m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE45] |= 0x80;
		}
		else if (m_level[TLCS900_INT5] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE45] &= ~0x80;
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_INT6:
		if (m_level[TLCS900_INT6] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE67] |= 0x08;
		}
		else if (m_level[TLCS900_INT6] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE67] &= ~0x08;
		}
		m_level[TLCS900_INT3] = level;
		break;

	case TLCS900_INT7:
		if (m_level[TLCS900_INT7] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE67] |= 0x80;
		}
		else if (m_level[TLCS900_INT7] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE67] &= ~0x80;
		}
		m_level[TLCS900_INT7] = level;
		break;

	case TLCS900_INT8:
		if (m_level[TLCS900_INT8] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE89] |= 0x08;
		}
		else if (m_level[TLCS900_INT8] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE89] &= ~0x08;
		}
		m_level[TLCS900_INT8] = level;
		break;

	case TLCS900_INT9:
		if (m_level[TLCS900_INT9] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTE89] |= 0x80;
		}
		else if (m_level[TLCS900_INT9] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTE89] &= ~0x80;
		}
		m_level[TLCS900_INT9] = level;
		break;

	case TLCS900_INTA:
		if (m_level[TLCS900_INTA] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTEAB] |= 0x08;
		}
		else if (m_level[TLCS900_INTA] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTEAB] &= ~0x08;
		}
		m_level[TLCS900_INTA] = level;
		break;

	case TLCS900_INTB:
		if (m_level[TLCS900_INTB] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_int_reg[INTEAB] |= 0x80;
		}
		else if (m_level[TLCS900_INTB] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_int_reg[INTEAB] &= ~0x80;
		}
		m_level[TLCS900_INTB] = level;
		break;
	}
	m_check_irqs = 1;
}

std::unique_ptr<util::disasm_interface> tmp94c241_device::create_disassembler()
{
	return std::make_unique<tlcs900_disassembler>();
}

