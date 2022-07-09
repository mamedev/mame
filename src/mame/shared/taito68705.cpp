// license:BSD-3-Clause
// copyright-holders:David Haywood, Vas Crabb
#include "emu.h"
#include "taito68705.h"

#include "cpu/z80/z80.h"

/*
   Most Taito 68705s share a similar (often identical) hookup.
   This file encapsulates that.

    used by:
    40love.cpp   - 40love
    bigevglf.cpp - bigevglf
    bking.cpp    - bking3
    bublbobl.cpp - tokio
    buggychl.cpp - buggychl
    flstory.cpp  - flstory
    lkage.cpp    - lkage
    lsasquad.cpp - lsasquad, daikaiju
    matmania.cpp - maniach
    nycaptor.cpp - nycaptor
    renegade.cpp - renegade
    retofinv.cpp - retofinv
    slapfght.cpp - slapfght
    xain.cpp     - xsleena

    and the following with slight changes:
    slapfght.cpp - tigerh (inverted status bits read on port C)
    arkanoid.cpp - arkanoid (latch control on port C, inputs on port B)
    taito_l.cpp - puzznic (latch control on port C)

    not hooked up here, but possible (needs investigating)
    pitnrun.cpp - have more functionality on portB, currently using 'instant timers' for latches
    taitosj.cpp - ^^

    68705 sets in Taito drivers that are NOT suitable for hookup here?
    bublbobl.cpp - bub68705 - this is a bootleg, not an official Taito hookup
    changela.cpp - changela - looks like an ancestor of arkanoid without automatic semaphores
    mexico86.cpp - knightb, mexico86 - bootleg 68705s
    sqix.cpp - hotsmash - kaneko hookup, different from Taito ones.

    there are other drivers (and games in existing drivers) that could hookup here, but currently lack MCU dumps.

*/


DEFINE_DEVICE_TYPE(TAITO68705_MCU,       taito68705_mcu_device,       "taito68705",      "Taito MC68705 MCU Interface")
DEFINE_DEVICE_TYPE(TAITO68705_MCU_TIGER, taito68705_mcu_tiger_device, "taito68705tiger", "Taito MC68705 MCU Interface (Tiger-Heli)")
DEFINE_DEVICE_TYPE(ARKANOID_68705P3,     arkanoid_68705p3_device,     "arkanoid68705p3", "Arkanoid MC68705P3 Interface")
DEFINE_DEVICE_TYPE(ARKANOID_68705P5,     arkanoid_68705p5_device,     "arkanoid68705p5", "Arkanoid MC68705P5 Interface")


u8 taito68705_mcu_device_base::data_r()
{
	// clear MCU semaphore flag and return data
	u8 const result(m_mcu_latch);
	if (!machine().side_effects_disabled())
	{
		m_mcu_flag = false;
		m_semaphore_cb(CLEAR_LINE);
	}
	return result;
}

void taito68705_mcu_device_base::data_w(u8 data)
{
	// set host semaphore flag and latch data
	if (!m_reset_input)
		m_host_flag = true;
	m_host_latch = data;
	if (m_latch_driven)
		m_mcu->pa_w(data);
	m_mcu->set_input_line(M68705_IRQ_LINE, m_host_flag ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(taito68705_mcu_device_base::reset_w)
{
	m_reset_input = ASSERT_LINE == state;
	if (CLEAR_LINE != state)
	{
		m_host_flag = false;
		m_mcu_flag = false;
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}
	m_mcu->set_input_line(INPUT_LINE_RESET, state);
}

taito68705_mcu_device_base::taito68705_mcu_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_semaphore_cb(*this)
	, m_latch_driven(false)
	, m_reset_input(false)
	, m_host_flag(false)
	, m_mcu_flag(false)
	, m_host_latch(0xff)
	, m_mcu_latch(0xff)
	, m_pa_output(0xff)
{
}

void taito68705_mcu_device_base::mcu_pa_w(u8 data)
{
	m_pa_output = data;
}

void taito68705_mcu_device_base::device_start()
{
	m_semaphore_cb.resolve_safe();

	save_item(NAME(m_latch_driven));
	save_item(NAME(m_reset_input));
	save_item(NAME(m_host_flag));
	save_item(NAME(m_mcu_flag));
	save_item(NAME(m_host_latch));
	save_item(NAME(m_mcu_latch));
	save_item(NAME(m_pa_output));

	m_latch_driven = false;
	m_reset_input = false;
	m_host_latch = 0xff;
	m_mcu_latch = 0xff;
	m_pa_output = 0xff;
}

void taito68705_mcu_device_base::device_reset()
{
	m_host_flag = false;
	m_mcu_flag = false;

	m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	m_semaphore_cb(CLEAR_LINE);
}

void taito68705_mcu_device_base::latch_control(u8 data, u8 &value, unsigned host_bit, unsigned mcu_bit)
{
	// save this here to simulate latch propagation delays
	u8 const old_pa_value(pa_value());

	// rising edge clears the host semaphore flag
	if (BIT(data, host_bit))
	{
		m_latch_driven = false;
		m_mcu->pa_w(0xff);
		if (!BIT(value, host_bit))
		{
			m_host_flag = false;
			m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
		}
	}
	else
	{
		m_latch_driven = true;
		m_mcu->pa_w(m_host_latch);
	}

	// PB2 sets the MCU semaphore when low
	if (!BIT(data, mcu_bit))
	{
		if (!m_reset_input)
			m_mcu_flag = true;

		// data is latched on falling edge
		if (BIT(value, mcu_bit))
			m_mcu_latch = old_pa_value;
	}

	value = data;
	if (!BIT(data, mcu_bit) && !m_reset_input)
		m_semaphore_cb(ASSERT_LINE);
}


taito68705_mcu_device::taito68705_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: taito68705_mcu_device(mconfig, TAITO68705_MCU, tag, owner, clock)
{
}

taito68705_mcu_device::taito68705_mcu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: taito68705_mcu_device_base(mconfig, type, tag, owner, clock)
	, m_aux_out_cb(*this)
	, m_aux_strobe_cb(*this)
	, m_pb_output(0xff)
{
}

void taito68705_mcu_device::device_add_mconfig(machine_config &config)
{
	M68705P5(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->porta_w().set(FUNC(taito68705_mcu_device::mcu_pa_w));
	m_mcu->portb_w().set(FUNC(taito68705_mcu_device::mcu_portb_w));
	m_mcu->portc_r().set(FUNC(taito68705_mcu_device::mcu_portc_r));
}

void taito68705_mcu_device::device_start()
{
	taito68705_mcu_device_base::device_start();

	m_aux_out_cb.resolve_all_safe();
	m_aux_strobe_cb.resolve_safe();

	save_item(NAME(m_pb_output));

	m_pb_output = 0xff;
}


/*
 *  Port B connections:
 *  parts in [ ] are optional (not used by buggychl)
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   n.c.
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 *  3   W  to Z80 BUSRQ (put it on hold?)
 *  4   W  n.c.
 *  5   W  [selects Z80 memory access direction (0 = write 1 = read)]
 *  6   W  [loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access]
 *  7   W  [loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access]
 */


u8 taito68705_mcu_device::mcu_portc_r()
{
	// PC0 is the host semaphore flag (active high)
	// PC1 is the MCU semaphore flag (active low)
	return (host_flag() ? 0x01 : 0x00) | (mcu_flag() ? 0x00 : 0x02) | 0xfc;
}

void taito68705_mcu_device::mcu_portb_w(offs_t offset, u8 data, u8 mem_mask)
{
	// some games have additional peripherals strobed on falling edge
	u8 const old_pa_value(pa_value());
	u8 const aux_changed((data ^ m_pb_output) >> 2);
	u8 const aux_strobes((mem_mask & ~data & m_pb_output) >> 2);

	// rising edge on PB1 clears the host semaphore flag
	// PB2 sets the MCU semaphore when low
	latch_control(data, m_pb_output, 1, 2);

	// callbacks for other peripherals
	for (unsigned i = 0; i < 6; ++i)
	{
		if (BIT(aux_changed, i))
			m_aux_out_cb[i](BIT(data, i + 2));
		if (BIT(aux_strobes, i))
			m_aux_strobe_cb(i, old_pa_value, 0xff);
	}
}


/* The Tiger-Heli interface has some extensions, handle them here */

taito68705_mcu_tiger_device::taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: taito68705_mcu_device(mconfig, TAITO68705_MCU_TIGER, tag, owner, clock)
{
}

u8 taito68705_mcu_tiger_device::mcu_portc_r()
{
	// Tiger-Heli has these status bits inverted MCU-side
	return taito68705_mcu_device::mcu_portc_r() ^ 0x03;
}


// Arkanoid/Puzznic (latch control on PC2 and PC3 instead of PB1 and PB2)

arkanoid_mcu_device_base::arkanoid_mcu_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: taito68705_mcu_device_base(mconfig, type, tag, owner, clock)
	, m_portb_r_cb(*this)
	, m_pc_output(0xff)
{
}

u8 arkanoid_mcu_device_base::mcu_pb_r()
{
	return m_portb_r_cb();
}

u8 arkanoid_mcu_device_base::mcu_pc_r()
{
	// PC0 is the host semaphore flag (active high)
	// PC1 is the MCU semaphore flag (active low)
	return (host_flag() ? 0x01 : 0x00) | (mcu_flag() ? 0x00 : 0x02) | 0xfc;
}

void arkanoid_mcu_device_base::mcu_pc_w(u8 data)
{
	// rising edge on PC2 clears the host semaphore flag
	// PC3 sets the MCU semaphore when low
	latch_control(data, m_pc_output, 2, 3);
}

void arkanoid_mcu_device_base::device_start()
{
	taito68705_mcu_device_base::device_start();

	m_portb_r_cb.resolve_safe(0xff);

	save_item(NAME(m_pc_output));

	m_pc_output = 0xff;
}


arkanoid_68705p3_device::arkanoid_68705p3_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock)
	: arkanoid_mcu_device_base(mconfig, ARKANOID_68705P3, tag, owner, clock)
{
}

void arkanoid_68705p3_device::device_add_mconfig(machine_config &config)
{
	M68705P3(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->portb_r().set(FUNC(arkanoid_68705p3_device::mcu_pb_r));
	m_mcu->portc_r().set(FUNC(arkanoid_68705p3_device::mcu_pc_r));
	m_mcu->porta_w().set(FUNC(arkanoid_68705p3_device::mcu_pa_w));
	m_mcu->portc_w().set(FUNC(arkanoid_68705p3_device::mcu_pc_w));
}


arkanoid_68705p5_device::arkanoid_68705p5_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock)
	: arkanoid_mcu_device_base(mconfig, ARKANOID_68705P5, tag, owner, clock)
{
}

void arkanoid_68705p5_device::device_add_mconfig(machine_config &config)
{
	M68705P5(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->portb_r().set(FUNC(arkanoid_68705p5_device::mcu_pb_r));
	m_mcu->portc_r().set(FUNC(arkanoid_68705p5_device::mcu_pc_r));
	m_mcu->porta_w().set(FUNC(arkanoid_68705p5_device::mcu_pa_w));
	m_mcu->portc_w().set(FUNC(arkanoid_68705p5_device::mcu_pc_w));
}
