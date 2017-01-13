// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria, David Haywood, Vas Crabb
#include "emu.h"
#include "machine/taito68705interface.h"

#include "cpu/z80/z80.h"

/*
   Most Taito 68705s share a similar (often identical) hookup.
   This file encapsulates that.

    used by:
    buggychl.cpp - buggychl
    bking.cpp - bking3
    40love.cpp - 40love
    bublbobl.cpp - tokio
    flstory.cpp - flstory
    nycaptor.cpp - nycaptor
    lsasquad.cpp - lsasquad
                 - daikaiju
    lkage.cpp    - lkage

    and the following with slight changes:
    slapfght.cpp - tigerh (inverted status bits read on portC)
                 - slapfght (extended outputs for scrolling)
    bigevglf.cpp - writes to mcu aren't latched(?)f


    not hooked up here, but possible (needs investigating)
    pitnrun.cpp - have more functionality on portB, currently using 'instant timers' for latches
    taitosj.cpp - ^^
    changela.cpp - ^^
    xain.cpp - not a Taito game (licensed to Taito?) but MCU hookup looks almost the same
    renegade.cpp - ^^
    matmania.cpp - ^^

    68705 sets in Taito drivers that are NOT suitable for hookup here?
    bublbobl.cpp - bub68705 - this is a bootleg, not an official Taito hookup
    mexico86.cpp - knightb, mexico86 - bootleg 68705s
    retofinv.cpp - the current MCU dump is a bootleg at least
    sqix.cpp - hotsmash - kaneko hookup, different from Taito ones.

    there are other drivers (and games in existing drivers) that could hookup here, but currently lack MCU dumps.

*/


namespace {

MACHINE_CONFIG_FRAGMENT( taito68705 )
	MCFG_CPU_ADD("mcu", M68705P5, DERIVED_CLOCK(1, 1))
	MCFG_M68705_PORTA_R_CB(READ8(taito68705_mcu_device, mcu_porta_r))
	MCFG_M68705_PORTA_W_CB(WRITE8(taito68705_mcu_device, mcu_porta_w))
	MCFG_M68705_PORTB_W_CB(WRITE8(taito68705_mcu_device, mcu_portb_w))
	MCFG_M68705_PORTC_R_CB(READ8(taito68705_mcu_device, mcu_portc_r))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( arkanoid_68705p3 )
	MCFG_CPU_ADD("mcu", M68705P3, DERIVED_CLOCK(1, 1))
	MCFG_M68705_PORTA_R_CB(READ8(arkanoid_mcu_device_base, mcu_pa_r))
	MCFG_M68705_PORTB_R_CB(READ8(arkanoid_mcu_device_base, mcu_pb_r))
	MCFG_M68705_PORTC_R_CB(READ8(arkanoid_mcu_device_base, mcu_pc_r))
	MCFG_M68705_PORTA_W_CB(WRITE8(arkanoid_mcu_device_base, mcu_pa_w))
	MCFG_M68705_PORTC_W_CB(WRITE8(arkanoid_mcu_device_base, mcu_pc_w))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( arkanoid_68705p5 )
	MCFG_CPU_ADD("mcu", M68705P5, DERIVED_CLOCK(1, 1))
	MCFG_M68705_PORTA_R_CB(READ8(arkanoid_mcu_device_base, mcu_pa_r))
	MCFG_M68705_PORTB_R_CB(READ8(arkanoid_mcu_device_base, mcu_pb_r))
	MCFG_M68705_PORTC_R_CB(READ8(arkanoid_mcu_device_base, mcu_pc_r))
	MCFG_M68705_PORTA_W_CB(WRITE8(arkanoid_mcu_device_base, mcu_pa_w))
	MCFG_M68705_PORTC_W_CB(WRITE8(arkanoid_mcu_device_base, mcu_pc_w))
MACHINE_CONFIG_END

} // anonymous namespace


const device_type TAITO68705_MCU = &device_creator<taito68705_mcu_device>;
const device_type TAITO68705_MCU_SLAP = &device_creator<taito68705_mcu_slap_device>;
const device_type TAITO68705_MCU_TIGER = &device_creator<taito68705_mcu_tiger_device>;
const device_type TAITO68705_MCU_BEG = &device_creator<taito68705_mcu_beg_device>;
const device_type ARKANOID_68705P3 = &device_creator<arkanoid_68705p3_device>;
const device_type ARKANOID_68705P5 = &device_creator<arkanoid_68705p5_device>;


taito68705_mcu_device::taito68705_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITO68705_MCU, "Taito M68705 MCU Interface", tag, owner, clock, "taito68705", __FILE__),
	m_mcu_sent(false),
	m_main_sent(false),
	m_from_main(0),
	m_from_mcu(0),
	m_from_mcu_latch(0),
	m_to_mcu_latch(0),
	m_old_portB(0),
	m_mcu(*this, "mcu")
{
}

taito68705_mcu_device::taito68705_mcu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_mcu_sent(false),
	m_main_sent(false),
	m_from_main(0),
	m_from_mcu(0),
	m_from_mcu_latch(0),
	m_to_mcu_latch(0),
	m_old_portB(0),
	m_mcu(*this, "mcu")
{
}




machine_config_constructor taito68705_mcu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( taito68705 );
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void taito68705_mcu_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito68705_mcu_device::device_start()
{
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));
	save_item(NAME(m_from_mcu_latch));
	save_item(NAME(m_to_mcu_latch));
	save_item(NAME(m_old_portB));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taito68705_mcu_device::device_reset()
{
	m_mcu_sent = false;
	m_main_sent = false;
	m_from_main = 0;
	m_from_mcu = 0;
	m_from_mcu_latch = 0;
	m_to_mcu_latch = 0;
	m_old_portB = 0;

	m_mcu->set_input_line(0, CLEAR_LINE);
}


/***************************************************************************

 Buggy Challenge 68705 protection interface

 This is accurate. FairyLand Story seems to be identical.

***************************************************************************/


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





READ8_MEMBER(taito68705_mcu_device::mcu_r)
{
	m_mcu_sent = false;

//  logerror("%s: mcu_r %02x\n", space.machine().describe_context(), m_from_mcu);

	return m_from_mcu;
}

WRITE8_MEMBER(taito68705_mcu_device::mcu_w)
{
//  logerror("%s: mcu_w %02x\n", space.machine().describe_context(), data);

	m_from_main = data;
	m_main_sent = true;
	m_mcu->set_input_line(0, ASSERT_LINE);

}


READ8_MEMBER(taito68705_mcu_device::mcu_porta_r)
{
//  logerror("mcu_porta_r\n");
	return m_to_mcu_latch;
}

WRITE8_MEMBER(taito68705_mcu_device::mcu_porta_w)
{
//  logerror("mcu_porta_w %02x\n", data);
	m_from_mcu_latch = data;
}


READ8_MEMBER(taito68705_mcu_device::mcu_portc_r)
{
	uint8_t ret = 0;

	if (m_main_sent)
		ret |= 0x01;
	if (!m_mcu_sent)
		ret |= 0x02;

//  logerror("%s: mcu_portc_r %02x\n", space.machine().describe_context(), ret);

	return ret;
}


WRITE8_MEMBER(taito68705_mcu_device::mcu_portb_w)
{
//  logerror("mcu_portb_w %02x\n", data);

	if ((mem_mask & 0x02) && (~data & 0x02) && (m_old_portB & 0x02))
	{
		if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);

		m_to_mcu_latch = m_from_main;
		m_main_sent = false;
	}
	if ((mem_mask & 0x04) && (data & 0x04) && (~m_old_portB & 0x04))
	{
		m_from_mcu = m_from_mcu_latch;
		m_mcu_sent = true;
	//  logerror("sent %02x\n", m_from_mcu);
	}

	m_old_portB = data;

}

/* Status readbacks for MAIN cpu - these hook up in various ways depending on the host (provide 2 lines instead?) */

READ8_MEMBER( taito68705_mcu_device::mcu_status_r )
{
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	//logerror("%s: mcu_status_r\n",machine().describe_context());
	if (!m_main_sent)
		res |= 0x01;
	if (m_mcu_sent)
		res |= 0x02;

	return res;
}

CUSTOM_INPUT_MEMBER(taito68705_mcu_device::mcu_sent_r)
{
	if (!m_mcu_sent) return 0;
	else return 1;
}

CUSTOM_INPUT_MEMBER(taito68705_mcu_device::main_sent_r)
{
	if (!m_main_sent) return 0;
	else return 1;
}

/* The Slap Fight interface has some extensions, handle them here */

taito68705_mcu_slap_device::taito68705_mcu_slap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: taito68705_mcu_device(mconfig, TAITO68705_MCU_SLAP, "Taito M68705 MCU Interface (Slap Fight)", tag, owner, clock, "taito68705slap", __FILE__),
	m_extension_cb_w(*this)
{
}

WRITE8_MEMBER(taito68705_mcu_slap_device::mcu_portb_w)
{
	if ((mem_mask & 0x08) && (~data & 0x08) && (m_old_portB & 0x08))
	{
		m_extension_cb_w(0,m_from_mcu_latch, 0xff); // m_scrollx_lo
	}
	if ((mem_mask & 0x10) && (~data & 0x10) && (m_old_portB & 0x10))
	{
		m_extension_cb_w(1,m_from_mcu_latch, 0xff); // m_scrollx_hi
	}

	taito68705_mcu_device::mcu_portb_w(space,offset,data);
}

void taito68705_mcu_slap_device::device_start()
{
	taito68705_mcu_device::device_start();
	m_extension_cb_w.resolve_safe();
}


/* The Tiger Heli interface has some extensions, handle them here */

taito68705_mcu_tiger_device::taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: taito68705_mcu_device(mconfig, TAITO68705_MCU_TIGER, "Taito M68705 MCU Interface (Tiger Heli)", tag, owner, clock, "taito68705tiger", __FILE__)
{
}

READ8_MEMBER(taito68705_mcu_tiger_device::mcu_portc_r)
{
	uint8_t ret = taito68705_mcu_device::mcu_portc_r(space,offset);

	// Tiger Heli has these status bits inverted MCU-side
	ret ^= 0x3;

	return ret;
}


/* Big Event Golf has some things switched around, handle them here */

taito68705_mcu_beg_device::taito68705_mcu_beg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: taito68705_mcu_device(mconfig, TAITO68705_MCU_BEG, "Taito M68705 MCU Interface (Big Event Golf)", tag, owner, clock, "taito68705bigevglf", __FILE__)
{
}

WRITE8_MEMBER(taito68705_mcu_beg_device::mcu_portb_w)
{
	// transitions are reversed
	if ((mem_mask & 0x02) && (data & 0x02) && (~m_old_portB & 0x02) ) /* positive going transition of the clock */
	{
		//if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);

		//m_to_mcu_latch = m_from_main; // this is weird, no latching?!
		m_main_sent = false;
	}
	if ((mem_mask & 0x04) && (data & 0x04) && (~m_old_portB & 0x04)  ) /* positive going transition of the clock */
	{
		m_from_mcu = m_from_mcu_latch;
		m_mcu_sent = true;
	//  logerror("sent %02x\n", m_from_mcu);
	}

	m_old_portB = data;
}

WRITE8_MEMBER(taito68705_mcu_beg_device::mcu_w)
{
//  logerror("%s: mcu_w %02x\n", space.machine().describe_context(), data);

	m_to_mcu_latch = data; // this is weird, no latching?!
	m_main_sent = true;
	m_mcu->set_input_line(0, ASSERT_LINE);

}


// Arkanoid/Puzznic (completely different)

READ8_MEMBER(arkanoid_mcu_device_base::data_r)
{
	// clear MCU semaphore flag and return data
	u8 const result(m_mcu_latch);
	m_mcu_flag = false;
	m_semaphore_cb(CLEAR_LINE);
	return result;
}

WRITE8_MEMBER(arkanoid_mcu_device_base::data_w)
{
	// set host semaphore flag and latch data
	m_host_flag = true;
	m_host_latch = data;
	m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}

CUSTOM_INPUT_MEMBER(arkanoid_mcu_device_base::semaphore_r)
{
	// bit 0 is host semaphore flag, bit 1 is MCU semaphore flag (both active low)
	return (m_host_flag ? 0x00 : 0x01) | (m_mcu_flag ? 0x00 : 0x02) | 0xfc;
}

WRITE_LINE_MEMBER(arkanoid_mcu_device_base::reset_w)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: determine whether host CPU controlled reset also clears the semaphore flags
}

READ8_MEMBER(arkanoid_mcu_device_base::mcu_pa_r)
{
	// PC2 controls whether host host latch drives the port
	return BIT(m_pc_output, 2) ? 0xff : m_host_latch;
}

READ8_MEMBER(arkanoid_mcu_device_base::mcu_pb_r)
{
	return m_portb_r_cb(space, offset, mem_mask);
}

READ8_MEMBER(arkanoid_mcu_device_base::mcu_pc_r)
{
	// PC0 is the host semaphore flag (active high)
	// PC1 is the MCU semaphoe flag (active low)
	return (m_host_flag ? 0x01 : 0x00) | (m_mcu_flag ? 0x00 : 0x02) | 0xfc;
}

WRITE8_MEMBER(arkanoid_mcu_device_base::mcu_pa_w)
{
	m_pa_output = data;
}

WRITE8_MEMBER(arkanoid_mcu_device_base::mcu_pc_w)
{
	// rising edge on PC2 clears the host semaphore flag
	if (BIT(data, 2) && !BIT(m_pc_output, 2))
	{
		m_host_flag = false;
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	// PC3 sets the MCU semaphore when low
	if (!BIT(data, 3))
	{
		m_mcu_flag = true;

		// data is latched on falling edge
		if (BIT(m_pc_output, 3))
			m_mcu_latch = m_pa_output & (BIT(m_pc_output, 2) ? 0xff : m_host_latch);
	}

	m_pc_output = data;
	if (!BIT(data, 3))
		m_semaphore_cb(ASSERT_LINE);
}

arkanoid_mcu_device_base::arkanoid_mcu_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		char const *shortname,
		char const *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_mcu(*this, "mcu")
	, m_semaphore_cb(*this)
	, m_portb_r_cb(*this)
	, m_host_flag(false)
	, m_mcu_flag(false)
	, m_host_latch(0xff)
	, m_mcu_latch(0xff)
	, m_pa_output(0xff)
	, m_pc_output(0xff)
{
}

void arkanoid_mcu_device_base::device_start()
{
	m_semaphore_cb.resolve_safe();
	m_portb_r_cb.resolve_safe(0xff);

	save_item(NAME(m_host_flag));
	save_item(NAME(m_mcu_flag));
	save_item(NAME(m_host_latch));
	save_item(NAME(m_mcu_latch));
	save_item(NAME(m_pa_output));
	save_item(NAME(m_pc_output));

	m_host_latch = 0xff;
	m_mcu_latch = 0xff;
	m_pa_output = 0xff;
	m_pc_output = 0xff;
}

void arkanoid_mcu_device_base::device_reset()
{
	m_host_flag = false;
	m_mcu_flag = false;

	m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	m_semaphore_cb(CLEAR_LINE);
}


arkanoid_68705p3_device::arkanoid_68705p3_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: arkanoid_mcu_device_base(mconfig, ARKANOID_68705P3, "Arkanoid MC68705P3 Interface", tag, owner, clock, "arkanoid_68705p3", __FILE__)
{
}

machine_config_constructor arkanoid_68705p3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(arkanoid_68705p3);
}


arkanoid_68705p5_device::arkanoid_68705p5_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: arkanoid_mcu_device_base(mconfig, ARKANOID_68705P5, "Arkanoid MC68705P5 Interface", tag, owner, clock, "arkanoid_68705p5", __FILE__)
{
}

machine_config_constructor arkanoid_68705p5_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(arkanoid_68705p5);
}
