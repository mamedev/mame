// license:BSD-3-Clause
// copyright-holders:AJR,Felipe Sanches
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
	m_timer_flipflops{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t8run(0),
	m_t01mod(0),
	m_t23mod(0),
	m_t4mod(0),
	m_t6mod(0),
	m_t8mod(0),
	m_tamod(0),
	m_tffcr(0),
	m_t4ffcr(0),
	m_t6ffcr(0),
	m_t8ffcr(0),
	m_taffcr(0),
	m_trdc(0),
	m_t16run(0),
	m_treg_8{ 0, 0, 0, 0 },
	m_treg_16{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_t16_cap{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_timer_16{ 0, 0, 0, 0 },
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
	m_msar{ 0, 0, 0, 0, 0, 0 },
	m_mamr{ 0, 0, 0, 0, 0, 0 },
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


void tmp94c241_device::device_resolve_objects()
{
	m_nmi_state = CLEAR_LINE;
	for (int i = 0; i < TLCS900_NUM_INPUTS; i++)
	{
		m_level[i] = CLEAR_LINE;
	}
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
	save_item(NAME(m_timer_flipflops));
	save_item(NAME(m_t8run));
	save_item(NAME(m_t01mod));
	save_item(NAME(m_t23mod));
	save_item(NAME(m_t4mod));
	save_item(NAME(m_t6mod));
	save_item(NAME(m_t8mod));
	save_item(NAME(m_tamod));
	save_item(NAME(m_trdc));
	save_item(NAME(m_treg_8));
	save_item(NAME(m_treg_16));
	save_item(NAME(m_t16_cap));
	save_item(NAME(m_tffcr));
	save_item(NAME(m_t4ffcr));
	save_item(NAME(m_t6ffcr));
	save_item(NAME(m_t8ffcr));
	save_item(NAME(m_taffcr));
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
	save_item(NAME(m_msar));
	save_item(NAME(m_mamr));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_dram_access));
	save_item(NAME(m_da_drive));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmp94c241_device::device_reset()
{
	tlcs900h_device::device_reset();

	m_ad_cycles_left = 0;
	m_timer_pre = 0;

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
	std::fill_n(&m_timer_flipflops[0], 12, 0x00);
	m_t8run = 0x00;
	m_trdc = 0x00;
	m_t01mod = 0x00;
	m_t23mod = 0x00;
	m_t4mod = 0x00;
	m_t6mod = 0x00;
	m_t8mod = 0x00;
	m_tamod = 0x00;
	m_tffcr = 0x00;
	m_t4ffcr = 0x00;
	m_t6ffcr = 0x00;
	m_t8ffcr = 0x00;
	m_taffcr = 0x00;
	std::fill_n(&m_timer_change[0], 8, 0x00);
	std::fill_n(&m_timer_8[0], 4, 0x00);
	std::fill_n(&m_timer_16[0], 4, 0x00);
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
	std::fill_n(&m_msar[0], 6, 0xff);
	std::fill_n(&m_mamr[0], 6, 0xff);
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
	if (data & 0x80)
		data = (data & 0x7f) | (m_int_reg[offset] & 0x80);
	if (data & 0x08)
		data = (data & 0xf7) | (m_int_reg[offset] & 0x08);

	m_int_reg[offset] = data;
	m_check_irqs = 1;
}

uint8_t tmp94c241_device::intnmwdt_r(offs_t offset)
{
	return m_int_reg[INTNMWDT];
}

void tmp94c241_device::intnmwdt_w(offs_t offset, uint8_t data)
{
	if (data & 0x80)
		data = (data & 0x7f) | (m_int_reg[INTNMWDT] & 0x80);
	if ( data & 0x08 )
		data = (data & 0xf7) | (m_int_reg[INTNMWDT] & 0x08);

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
	for (int i = 0; i < NUM_MASKABLE_IRQS; i++)
	{
		if (data == tmp94c241_irq_vector_map[i].dma_start_vector)
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

template <uint8_t N>
void tmp94c241_device::bNcs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_block_cs[N]);
}

template <uint8_t N>
void tmp94c241_device::mamr_w(uint8_t data)
{
	m_mamr[N] = data;
}

template <uint8_t N>
void tmp94c241_device::msar_w(uint8_t data)
{
	m_msar[N] = data;
}

template <uint8_t N>
uint8_t tmp94c241_device::mamr_r()
{
	return m_mamr[N];
}

template <uint8_t N>
uint8_t tmp94c241_device::msar_r()
{
	return m_msar[N];
}

uint8_t tmp94c241_device::t8run_r()
{
	return m_t8run;
}

void tmp94c241_device::t8run_w(uint8_t data)
{
	m_t8run = data;
	for (int i = 0; i < 4; i++)
	{
		// These correspond to UP_COUNTER and TIMER_CHANGE for 8-bit timers 0, 1, 2 and 3
		if (!BIT(m_t8run, i)) // Timer isn't running
		{
			m_timer_8[i] = 0;
			m_timer_change[i] = 0;
		}
	}
}

uint8_t tmp94c241_device::t01mod_r()
{
	return m_t01mod;
}

void tmp94c241_device::t01mod_w(uint8_t data)
{
	m_t01mod = data;
}

uint8_t tmp94c241_device::tffcr_r()
{
	return m_tffcr;
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
	/* First we update the timer flip-flop */
	bool &ff_state = m_timer_flipflops[flipflop];

	switch (operation)
	{
		case FF_INVERT:
			ff_state = !ff_state;
			break;
		case FF_SET:
			ff_state = true;
			break;
		case FF_CLEAR:
			ff_state = false;
			break;
		default:
			// invalid operation
			return;
	}

	/* The value of the flipflop is only exposed to a pin in certain modes of operation
	   determined by fields of the port function registers.

	   So here we bail out if the flipflop is not routed to its corresponding port bit:
	*/
	switch (flipflop)
	{
		case 0x1:
			if (!BIT(m_port_function[PORT_C], 0) || BIT(m_port_control[PORT_C], 0)) return;
			break;
		case 0x7:
			if (!BIT(m_port_function[PORT_C], 0) || !BIT(m_port_control[PORT_C], 0)) return;
			break;
		case 0x3:
			if (!BIT(m_port_function[PORT_C], 1) || BIT(m_port_control[PORT_C], 1)) return;
			break;
		case 0xb:
			if (!BIT(m_port_function[PORT_C], 1) || !BIT(m_port_control[PORT_C], 1)) return;
			break;
		case 0x4:
			if (!BIT(m_port_function[PORT_D], 0)) return;
			break;
		case 0x6:
			if (!BIT(m_port_function[PORT_D], 4)) return;
			break;
		case 0x8:
			if (!BIT(m_port_function[PORT_E], 0)) return;
			break;
		case 0xA:
			if (!BIT(m_port_function[PORT_E], 4)) return;
			break;
		default:
			// invalid flip flop
			return;
	}

	// And here we actually send the value to the corresponding pin
	uint8_t new_port_value = 0;
	switch (flipflop)
	{
		case 0x1:
		case 0x7:
			new_port_value = m_port_latch[PORT_C] & 0xfe;
			if (ff_state) new_port_value |= 0x01;
			port_w<PORT_C>(new_port_value);
			break;
		case 0x3:
		case 0xb:
			new_port_value = m_port_latch[PORT_C] & 0xfd;
			if (ff_state) new_port_value |= 0x02;
			port_w<PORT_C>(new_port_value);
			break;
		case 0x4:
			new_port_value = m_port_latch[PORT_D] & 0xfe;
			if (ff_state) new_port_value |= 0x01;
			port_w<PORT_D>(new_port_value);
			break;
		case 0x6:
			new_port_value = m_port_latch[PORT_D] & 0xef;
			if (ff_state) new_port_value |= 0x10;
			port_w<PORT_D>(new_port_value);
			break;
		case 0x8:
			new_port_value = m_port_latch[PORT_E] & 0xfe;
			if (ff_state) new_port_value |= 0x01;
			port_w<PORT_E>(new_port_value);
			break;
		case 0xa:
			new_port_value = m_port_latch[PORT_E] & 0xef;
			if (ff_state) new_port_value |= 0x10;
			port_w<PORT_E>(new_port_value);
			break;
	}
}

void tmp94c241_device::tffcr_w(uint8_t data)
{
	change_timer_flipflop( 1, (data >> 2) & 3 );
	change_timer_flipflop( 3, (data >> 6) & 3 );
	m_tffcr = data | 0xcc;
}

uint8_t tmp94c241_device::t23mod_r()
{
	return m_t23mod;
}

void tmp94c241_device::t23mod_w(uint8_t data)
{
	m_t23mod = data;
}

uint8_t tmp94c241_device::trdc_r()
{
	return m_trdc;
}

void tmp94c241_device::trdc_w(uint8_t data)
{
	m_trdc = data;
}

template <uint8_t Timer>
void tmp94c241_device::treg_8_w(uint8_t data)
{
	m_treg_8[Timer] = data;
}

template <uint8_t Timer>
void tmp94c241_device::treg_16_w(uint16_t data)
{
	m_treg_16[Timer] = data;
}

uint8_t tmp94c241_device::t4mod_r()
{
	return m_t4mod;
}

void tmp94c241_device::t4mod_w(uint8_t data)
{
	m_t4mod = data;
}

uint8_t tmp94c241_device::t6mod_r()
{
	return m_t6mod;
}

void tmp94c241_device::t6mod_w(uint8_t data)
{
	m_t6mod = data;
}

uint8_t tmp94c241_device::t8mod_r()
{
	return m_t8mod;
}

void tmp94c241_device::t8mod_w(uint8_t data)
{
	m_t8mod = data;
}

uint8_t tmp94c241_device::tamod_r()
{
	return m_tamod;
}

void tmp94c241_device::tamod_w(uint8_t data)
{
	m_tamod = data;
}

uint8_t tmp94c241_device::t4ffcr_r()
{
	return m_t4ffcr;
}

void tmp94c241_device::t4ffcr_w(uint8_t data)
{
	change_timer_flipflop( 4, data & 3 );
	change_timer_flipflop( 5, (data >> 6) & 3 );
	m_t4ffcr = data | 0xc3;
}

uint8_t tmp94c241_device::t6ffcr_r()
{
	return m_t6ffcr;
}

void tmp94c241_device::t6ffcr_w(uint8_t data)
{
	change_timer_flipflop( 6, data & 3 );
	change_timer_flipflop( 7, (data >> 6) & 3 );
	m_t6ffcr = data | 0xc3;
}

uint8_t tmp94c241_device::t8ffcr_r()
{
	return m_t8ffcr;
}

void tmp94c241_device::t8ffcr_w(uint8_t data)
{
	change_timer_flipflop( 8, data & 3 );
	change_timer_flipflop( 9, (data >> 6) & 3 );
	m_t8ffcr = data | 0xc3;
}

uint8_t tmp94c241_device::taffcr_r()
{
	return m_taffcr;
}

void tmp94c241_device::taffcr_w(uint8_t data)
{
	change_timer_flipflop( 0xa, data & 3 );
	change_timer_flipflop( 0xb, (data >> 6) & 3 );
	m_taffcr = data | 0xc3;
}

uint8_t tmp94c241_device::t16run_r()
{
	return m_t16run;
}

void tmp94c241_device::t16run_w(uint8_t data)
{
	m_t16run = data;
	for (int i = 0; i < 4; i++)
	{
		if (!BIT(m_t16run, i)) // Timer isn't running
		{
			// These correspond to UP_COUNTER and TIMER_CHANGE for 16-bit timers 4, 6, 8 and A
			m_timer_16[i] = 0;
			m_timer_change[4 + i] = 0;
		}
	}
}

template <uint8_t Timer>
uint16_t tmp94c241_device::cap_r()
{
	return m_t16_cap[Timer];
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

template <uint8_t Channel>
uint8_t tmp94c241_device::scNbuf_r()
{
	return 0;
}

template <uint8_t Channel>
void tmp94c241_device::scNbuf_w(uint8_t data)
{
	// Fake finish sending data
	m_int_reg[(Channel == 0) ? INTES0 : INTES1] |= 0x80;
	m_check_irqs = 1;
	logerror("sc%dbuf write: %02X\n", Channel, data);
	//machine().debugger().debug_break();
}

template <uint8_t Channel>
uint8_t tmp94c241_device::scNcr_r()
{
	uint8_t reg = m_serial_control[Channel];
	if (!machine().side_effects_disabled())
		m_serial_control[Channel] &= 0xe3;
	return reg;
}

template <uint8_t Channel>
void tmp94c241_device::scNcr_w(uint8_t data)
{
	m_serial_control[Channel] = data;
}

template <uint8_t Channel>
uint8_t tmp94c241_device::scNmod_r()
{
	return m_serial_mode[Channel];
}

template <uint8_t Channel>
void tmp94c241_device::scNmod_w(uint8_t data)
{
	m_serial_mode[Channel] = data;
}

template <uint8_t Channel>
uint8_t tmp94c241_device::brNcr_r()
{
	return m_baud_rate[Channel];
}

template <uint8_t Channel>
void tmp94c241_device::brNcr_w(uint8_t data)
{
	m_baud_rate[Channel] = data;
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
	data = (m_ad_mode1 & 0xc0) | ( data & 0x34 );

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
	map(0x000082, 0x000082).rw(FUNC(tmp94c241_device::tffcr_r), FUNC(tmp94c241_device::tffcr_w));
	map(0x000084, 0x000084).rw(FUNC(tmp94c241_device::t01mod_r), FUNC(tmp94c241_device::t01mod_w));
	map(0x000085, 0x000085).rw(FUNC(tmp94c241_device::t23mod_r), FUNC(tmp94c241_device::t23mod_w));
	map(0x000088, 0x000088).w(FUNC(tmp94c241_device::treg_8_w<TREG0>));
	map(0x000089, 0x000089).w(FUNC(tmp94c241_device::treg_8_w<TREG1>));
	map(0x00008a, 0x00008a).w(FUNC(tmp94c241_device::treg_8_w<TREG2>));
	map(0x00008b, 0x00008b).w(FUNC(tmp94c241_device::treg_8_w<TREG3>));
	map(0x000090, 0x000091).w(FUNC(tmp94c241_device::treg_16_w<TREG4>));
	map(0x000092, 0x000093).w(FUNC(tmp94c241_device::treg_16_w<TREG5>));
	map(0x000094, 0x000095).r(FUNC(tmp94c241_device::cap_r<CAP4>));
	map(0x000096, 0x000097).r(FUNC(tmp94c241_device::cap_r<CAP5>));
	map(0x000098, 0x000098).rw(FUNC(tmp94c241_device::t4mod_r), FUNC(tmp94c241_device::t4mod_w));
	map(0x000099, 0x000099).rw(FUNC(tmp94c241_device::t4ffcr_r), FUNC(tmp94c241_device::t4ffcr_w));
	map(0x00009e, 0x00009e).rw(FUNC(tmp94c241_device::t16run_r), FUNC(tmp94c241_device::t16run_w));
	map(0x0000a0, 0x0000a1).w(FUNC(tmp94c241_device::treg_16_w<TREG6>));
	map(0x0000a2, 0x0000a3).w(FUNC(tmp94c241_device::treg_16_w<TREG7>));
	map(0x0000a4, 0x0000a5).r(FUNC(tmp94c241_device::cap_r<CAP6>));
	map(0x0000a6, 0x0000a7).r(FUNC(tmp94c241_device::cap_r<CAP7>));
	map(0x0000a8, 0x0000a8).rw(FUNC(tmp94c241_device::t6mod_r), FUNC(tmp94c241_device::t6mod_w));
	map(0x0000a9, 0x0000a9).rw(FUNC(tmp94c241_device::t6ffcr_r), FUNC(tmp94c241_device::t6ffcr_w));
	map(0x0000b0, 0x0000b1).w(FUNC(tmp94c241_device::treg_16_w<TREG8>));
	map(0x0000b2, 0x0000b3).w(FUNC(tmp94c241_device::treg_16_w<TREG9>));
	map(0x0000b4, 0x0000b5).r(FUNC(tmp94c241_device::cap_r<CAP8>));
	map(0x0000b6, 0x0000b7).r(FUNC(tmp94c241_device::cap_r<CAP9>));
	map(0x0000b8, 0x0000b8).rw(FUNC(tmp94c241_device::t8mod_r), FUNC(tmp94c241_device::t8mod_w));
	map(0x0000b9, 0x0000b9).rw(FUNC(tmp94c241_device::t8ffcr_r), FUNC(tmp94c241_device::t8ffcr_w));
	map(0x0000c0, 0x0000c1).w(FUNC(tmp94c241_device::treg_16_w<TREGA>));
	map(0x0000c2, 0x0000c3).w(FUNC(tmp94c241_device::treg_16_w<TREGB>));
	map(0x0000c4, 0x0000c5).r(FUNC(tmp94c241_device::cap_r<CAPA>));
	map(0x0000c6, 0x0000c7).r(FUNC(tmp94c241_device::cap_r<CAPB>));
	map(0x0000c8, 0x0000c8).rw(FUNC(tmp94c241_device::tamod_r), FUNC(tmp94c241_device::tamod_w));
	map(0x0000c9, 0x0000c9).rw(FUNC(tmp94c241_device::taffcr_r), FUNC(tmp94c241_device::taffcr_w));
	map(0x0000d0, 0x0000d0).rw(FUNC(tmp94c241_device::scNbuf_r<0>), FUNC(tmp94c241_device::scNbuf_w<0>));
	map(0x0000d1, 0x0000d1).rw(FUNC(tmp94c241_device::scNcr_r<0>), FUNC(tmp94c241_device::scNcr_w<0>));
	map(0x0000d2, 0x0000d2).rw(FUNC(tmp94c241_device::scNmod_r<0>), FUNC(tmp94c241_device::scNmod_w<0>));
	map(0x0000d3, 0x0000d3).rw(FUNC(tmp94c241_device::brNcr_r<0>), FUNC(tmp94c241_device::brNcr_w<0>));
	map(0x0000d4, 0x0000d4).rw(FUNC(tmp94c241_device::scNbuf_r<1>), FUNC(tmp94c241_device::scNbuf_w<1>));
	map(0x0000d5, 0x0000d5).rw(FUNC(tmp94c241_device::scNcr_r<1>), FUNC(tmp94c241_device::scNcr_w<1>));
	map(0x0000d6, 0x0000d6).rw(FUNC(tmp94c241_device::scNmod_r<1>), FUNC(tmp94c241_device::scNmod_w<1>));
	map(0x0000d7, 0x0000d7).rw(FUNC(tmp94c241_device::brNcr_r<1>), FUNC(tmp94c241_device::brNcr_w<1>));
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
	map(0x000140, 0x000141).w(FUNC(tmp94c241_device::bNcs_w<0>));
	map(0x000142, 0x000142).rw(FUNC(tmp94c241_device::mamr_r<0>), FUNC(tmp94c241_device::mamr_w<0>));
	map(0x000143, 0x000143).rw(FUNC(tmp94c241_device::msar_r<0>), FUNC(tmp94c241_device::msar_w<0>));
	map(0x000144, 0x000145).w(FUNC(tmp94c241_device::bNcs_w<1>));
	map(0x000146, 0x000146).rw(FUNC(tmp94c241_device::mamr_r<1>), FUNC(tmp94c241_device::mamr_w<1>));
	map(0x000147, 0x000147).rw(FUNC(tmp94c241_device::msar_r<1>), FUNC(tmp94c241_device::msar_w<1>));
	map(0x000148, 0x000149).w(FUNC(tmp94c241_device::bNcs_w<2>));
	map(0x00014a, 0x00014a).rw(FUNC(tmp94c241_device::mamr_r<2>), FUNC(tmp94c241_device::mamr_w<2>));
	map(0x00014b, 0x00014b).rw(FUNC(tmp94c241_device::msar_r<2>), FUNC(tmp94c241_device::msar_w<2>));
	map(0x00014c, 0x00014d).w(FUNC(tmp94c241_device::bNcs_w<3>));
	map(0x00014e, 0x00014e).rw(FUNC(tmp94c241_device::mamr_r<3>), FUNC(tmp94c241_device::mamr_w<3>));
	map(0x00014f, 0x00014f).rw(FUNC(tmp94c241_device::msar_r<3>), FUNC(tmp94c241_device::msar_w<3>));
	map(0x000150, 0x000151).w(FUNC(tmp94c241_device::bNcs_w<4>));
	map(0x000152, 0x000152).rw(FUNC(tmp94c241_device::mamr_r<4>), FUNC(tmp94c241_device::mamr_w<4>));
	map(0x000153, 0x000153).rw(FUNC(tmp94c241_device::msar_r<4>), FUNC(tmp94c241_device::msar_w<4>));
	map(0x000154, 0x000155).w(FUNC(tmp94c241_device::bNcs_w<5>));
	map(0x000156, 0x000156).rw(FUNC(tmp94c241_device::mamr_r<5>), FUNC(tmp94c241_device::mamr_w<5>));
	map(0x000157, 0x000157).rw(FUNC(tmp94c241_device::msar_r<5>), FUNC(tmp94c241_device::msar_w<5>));
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
	// Check for NMI
	if (m_nmi_state == ASSERT_LINE)
	{
		m_xssp.d -= 4;
		WRMEML(m_xssp.d, m_pc.d);
		m_xssp.d -= 2;
		WRMEMW(m_xssp.d, m_sr.w.l);
		m_pc.d = RDMEML( 0xffff00 + 0x20 );
		m_cycles += 18;
		m_prefetch_clear = true;
		m_halted = 0;
		m_nmi_state = CLEAR_LINE;
		return;
	}

	/* Check regular IRQs
	   The smaller the vector value, the higher the priority. */
	int irq_vectors[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	for (int i = NUM_MASKABLE_IRQS - 1; i >= 0; i--)
	{
		if (m_int_reg[tmp94c241_irq_vector_map[i].reg] & tmp94c241_irq_vector_map[i].iff)
		{
			switch (tmp94c241_irq_vector_map[i].iff)
			{
				case 0x80:
					irq_vectors[(m_int_reg[tmp94c241_irq_vector_map[i].reg] >> 4) & 0x07] = i;
					break;
				case 0x08:
					irq_vectors[m_int_reg[tmp94c241_irq_vector_map[i].reg] & 0x07] = i;
					break;
			}
		}
	}

	// Check highest allowed priority IRQ
	int irq = -1;
	int level = 0;
	for (int i = std::max(1, (m_sr.b.h & 0x70) >> 4); i < 7; i++)
	{
		if (irq_vectors[i] >= 0)
		{
			irq = irq_vectors[i];
			level = i + 1;
		}
	}

	// Take IRQ
	if (irq >= 0)
	{
		uint8_t vector = tmp94c241_irq_vector_map[irq].vector;

		m_xssp.d -= 4;
		WRMEML(m_xssp.d, m_pc.d);
		m_xssp.d -= 2;
		WRMEMW(m_xssp.d, m_sr.w.l);

		// Mask off any lower priority interrupts
		m_sr.b.h = (m_sr.b.h & 0x8f) | (level << 4);

		m_pc.d = RDMEML(0xffff00 + vector);

		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		// Clear taken IRQ
		m_int_reg[tmp94c241_irq_vector_map[irq].reg] &= ~ tmp94c241_irq_vector_map[irq].iff;
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

// Prescaler shift amounts corresponding to each possible timer input clock source:
static constexpr uint8_t T1 = 3;
static constexpr uint8_t T4 = 5;
static constexpr uint8_t T16 = 7;
static constexpr uint8_t T256 = 11;

void tmp94c241_device::tlcs900_handle_timers()
{
	auto const update_timer_count =
			[this, old_pre = m_timer_pre] (
					uint8_t timer_index,
					uint8_t input_clk_select,
					uint8_t s1,
					uint8_t s2,
					uint8_t s3)
			{
				switch (input_clk_select)
				{
					case 0:
					/* Not yet implemented.
					    - For the 8 bit timers: TIO, TO0TRG, invalid and TO2TRG
					    - For all 16 bit timers: TIA
					*/
					break;
					case 1: m_timer_change[timer_index] += ((m_timer_pre >> s1) - (old_pre >> s1)); break;
					case 2: m_timer_change[timer_index] += ((m_timer_pre >> s2) - (old_pre >> s2)); break;
					case 3: m_timer_change[timer_index] += ((m_timer_pre >> s3) - (old_pre >> s3)); break;
				}
			};

	auto const timer_8bits =
			[this] (
					uint8_t timer_index,
					uint8_t timer_reg,
					uint8_t interrupt,
					uint8_t interrupt_mask,
					uint8_t operating_mode,
					bool invert)
			{
				for ( ; m_timer_change[timer_index] > 0; m_timer_change[timer_index]--)
				{
					m_timer_8[timer_index]++;
					if (m_timer_8[timer_index] == m_treg_8[timer_reg])
					{
						if (BIT(timer_index, 0) == 0)
						{
							if (operating_mode == 0) // mode == MODE_8BIT_TIMER
								m_timer_change[timer_index | 1]++;

							// In 16-bit timer mode the timer should not be reset
							if (operating_mode != 1) // mode != MODE_16BIT_TIMER
							{
								m_timer_8[timer_index] = 0;
								m_int_reg[interrupt] |= interrupt_mask;
								m_check_irqs = 1;
							}
						}
						else
						{
							m_timer_8[timer_index] = 0;
							m_int_reg[interrupt] |= interrupt_mask;
							m_check_irqs = 1;

							// In 16-bit timer mode also reset its 8-bit counterpart (timer N-1)
							if (operating_mode == 1) // mode == MODE_16BIT_TIMER
								m_timer_8[timer_index & ~1] = 0;
						}

						if (invert)
							change_timer_flipflop(timer_index | 1, FF_INVERT);
					}
				}
			};

	auto const timer_16bits =
			[this] (
					uint8_t timer_id,
					uint8_t timer_reg_low,
					uint8_t timer_reg_high,
					uint8_t tffcr,
					uint8_t interrupt)
			{
				/*
				    timer_id 4  =>  m_timer_16[0]  m_timer_change[4]
				    timer_id 6  =>  m_timer_16[1]  m_timer_change[5]
				    timer_id 8  =>  m_timer_16[2]  m_timer_change[6]
				    timer_id A  =>  m_timer_16[3]  m_timer_change[7]
				*/
				uint8_t timer_index = (timer_id - 4)/2;

				for ( ; m_timer_change[timer_index + 4] > 0; m_timer_change[timer_index + 4]--)
				{
					m_timer_16[timer_index]++;
					// TODO: also check for criteria of up counter matching CAPn registers
					if (((m_timer_16[timer_index] == m_treg_16[timer_reg_high]) && BIT(tffcr, 3)) ||
							((m_timer_16[timer_index] == m_treg_16[timer_reg_low]) && BIT(tffcr, 2)) )
					{
						change_timer_flipflop(timer_id, FF_INVERT);
						m_timer_16[timer_index] = 0;
						m_int_reg[interrupt] |= 0x08;
						m_check_irqs = 1;
					}
				}
			};

	if (BIT(m_t16run, 7)) // prescaler is active
		m_timer_pre += m_cycles;

	if (BIT(m_t8run, 0)) // Timer 0 is running
	{
		update_timer_count(0, m_t01mod & 3, T1, T4, T16);
		timer_8bits(
				0, TREG0, INTET01, 0x08,
				(m_t01mod >> 6) & 3, // TO1_OPERATING_MODE
				(m_tffcr & 3) == 2); // "FF1 Invert Enable" && "Invert by 8-bit timer 0"
	}

	if (BIT(m_t8run, 1)) // Timer 1 is running
	{
		update_timer_count(1, (m_t01mod >> 2) & 3, T1, T16, T256);
		timer_8bits(
				1, TREG1, INTET01, 0x80,
				(m_t01mod >> 6) & 3, // TO1_OPERATING_MODE
				(m_tffcr & 3) == 3); // "FF1 Invert Enable" && "Invert by 8-bit timer 1"
	}

	if (BIT(m_t8run, 2)) // Timer 2 is running
	{
		update_timer_count(2, m_t23mod & 3, T1, T4, T16);
		timer_8bits(
				2, TREG2, INTET23, 0x08,
				(m_t23mod >> 6) & 3, // T23_OPERATING_MODE
				((m_tffcr >> 4) & 3) == 2); // "FF3 Invert Enable" && "Invert by 8-bit timer 2"
	}

	if (BIT(m_t8run, 3)) // Timer 3 is running
	{
		update_timer_count(3, (m_t23mod >> 2) & 3, T1, T16, T256);
		timer_8bits(
				3, TREG3, INTET23, 0x80,
				(m_t23mod >> 6) & 3, // T23_OPERATING_MODE
				((m_tffcr >> 4) & 3) == 3); // "FF3 Invert Enable" && "Invert by 8-bit timer 3"
	}

	if (BIT(m_t16run, 0)) // Timer 4 is running
	{
		update_timer_count(4, m_t4mod & 3, T1, T4, T16);
		timer_16bits(4, TREG4, TREG5, m_t4ffcr, INTET45);
	}

	if (BIT(m_t16run, 1)) // Timer 6 is running
	{
		update_timer_count(5, m_t6mod & 3, T1, T4, T16);
		timer_16bits(6, TREG6, TREG7, m_t6ffcr, INTET67);
	}

	if (BIT(m_t16run, 2)) // Timer 8 is running
	{
		update_timer_count(6, m_t8mod & 3, T1, T4, T16);
		timer_16bits(8, TREG8, TREG9, m_t8ffcr, INTET89);
	}

	if (BIT(m_t16run, 3)) // Timer A is running
	{
		update_timer_count(7, m_tamod & 3, T1, T4, T16);
		timer_16bits(0xa, TREGA, TREGB, m_taffcr, INTETAB);
	}

	m_timer_pre &= 0xffffff;
}


//-------------------------------------------------
//  execute_set_input - called when a synchronized
//  input is changed
//-------------------------------------------------

void tmp94c241_device::execute_set_input(int input, int level)
{
	auto const update_int_reg =
			[this, level, input] (uint8_t reg, uint8_t mask)
			{
				if (level != m_level[input])
				{
					m_level[input] = level;
					if (level == ASSERT_LINE)
						m_int_reg[reg] |= mask;
					else
						m_int_reg[reg] &= ~mask;
				}
			};

	switch (input)
	{
		case INPUT_LINE_NMI:
		case TLCS900_NMI:
			if (level != m_level[TLCS900_NMI])
			{
				m_level[TLCS900_NMI] = level;
				if (level == ASSERT_LINE)
					m_nmi_state = ASSERT_LINE;
			}
			break;

		case TLCS900_INTWD:
			break;

		case TLCS900_INT0:
			if (m_iimc & 0x02)
			{
				// Rising edge detect
				if (level != m_level[TLCS900_INT0] && level == ASSERT_LINE)
				{
					// Leave HALT state
					m_halted = 0;
					m_int_reg[INTE0AD] |= 0x08;
				}
				m_level[TLCS900_INT0] = level;
			}
			else
			{
				// Level detect
				update_int_reg(INTE0AD, 0x08);
			}
			break;

		case TLCS900_INT4: update_int_reg(INTE45, 0x08); break;
		case TLCS900_INT5: update_int_reg(INTE45, 0x80); break;
		case TLCS900_INT6: update_int_reg(INTE67, 0x08); break;
		case TLCS900_INT7: update_int_reg(INTE67, 0x80); break;
		case TLCS900_INT8: update_int_reg(INTE89, 0x08); break;
		case TLCS900_INT9: update_int_reg(INTE89, 0x80); break;
		case TLCS900_INTA: update_int_reg(INTEAB, 0x08); break;
		case TLCS900_INTB: update_int_reg(INTEAB, 0x80); break;

		default:
			// invalid
			return;
	}
	m_check_irqs = 1;
}

std::unique_ptr<util::disasm_interface> tmp94c241_device::create_disassembler()
{
	return std::make_unique<tlcs900_disassembler>();
}
