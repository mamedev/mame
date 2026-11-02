// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1551 Single Disk Drive emulation

**********************************************************************/

#include "emu.h"
#include "c1551.h"

#include "formats/d64_dsk.h"
#include "formats/g64_dsk.h"

#include <algorithm>



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6510T_TAG      "u2"
#define C64H156_TAG     "u6"
#define PLA_TAG         "u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1551, c1551_device, "c1551", "Commodore 1551")


//-------------------------------------------------
//  ROM( c1551 )
//-------------------------------------------------

ROM_START( c1551 ) // schematic 251860
	ROM_REGION( 0x4000, M6510T_TAG, 0 )
	ROM_LOAD( "318001-01.u4", 0x0000, 0x4000, CRC(6d16d024) SHA1(fae3c788ad9a6cc2dbdfbcf6c0264b2ca921d55e) )

	ROM_REGION( 0xf5, PLA_TAG, 0 ) // schematic 251925
	ROM_LOAD( "251641-03.u1", 0x00, 0xf5, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1551_device::device_rom_region() const
{
	return ROM_NAME( c1551 );
}


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t c1551_device::port_r()
{
	/*

	    bit     description

	    P0
	    P1
	    P2
	    P3
	    P4      WPS
	    P5
	    P6
	    P7      BYTE LTCHED

	*/

	uint8_t data = 0;

	// write protect sense
	data |= !m_floppy->wpt_r() << 4;

	// byte latched
	data |= m_ga->atn_r() << 7;

	return data;
}

void c1551_device::port_w(uint8_t data)
{
	/*

	    bit     description

	    P0      STP0A
	    P1      STP0B
	    P2      MTR0
	    P3      ACT0
	    P4
	    P5      DS0
	    P6      DS1
	    P7

	*/

	// stepper motor
	m_ga->stp_w(data & 0x03);

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// activity LED
	m_leds[LED_ACT] = BIT(data, 3);

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


//-------------------------------------------------
//  tpi6525_interface tpi0_intf
//-------------------------------------------------

uint8_t c1551_device::tcbm_data_r()
{
	/*

	    bit     description

	    PA0     TCBM PA0
	    PA1     TCBM PA1
	    PA2     TCBM PA2
	    PA3     TCBM PA3
	    PA4     TCBM PA4
	    PA5     TCBM PA5
	    PA6     TCBM PA6
	    PA7     TCBM PA7

	*/

	return tcbm_r(TCBM_DATA0) & tcbm_r(TCBM_DATA1);
}

template <int N> void c1551_device::tcbm_data_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     TCBM PA0
	    PA1     TCBM PA1
	    PA2     TCBM PA2
	    PA3     TCBM PA3
	    PA4     TCBM PA4
	    PA5     TCBM PA5
	    PA6     TCBM PA6
	    PA7     TCBM PA7

	*/

	tcbm_w(TCBM_DATA0 + N, data);
}

uint8_t c1551_device::tpi0_r(offs_t offset)
{
	if (((offset == 0) || (offset == 2)) && !tcbm_sample_ready())
		return 0xff;

	uint8_t data = m_tpi0->read(offset);

	if (!machine().side_effects_disabled())
	{
		m_ga->ted_w(0);
		m_ga->ted_w(1);
	}

	return data;
}

void c1551_device::tpi0_w(offs_t offset, uint8_t data)
{
	m_tpi0->write(offset, data);

	m_ga->ted_w(0);
	m_ga->ted_w(1);
}

uint8_t c1551_device::tpi0_pc_r()
{
	/*

	    bit     description

	    PC0     TCBM STATUS0
	    PC1     TCBM STATUS1
	    PC2
	    PC3
	    PC4
	    PC5     JP1
	    PC6     _SYNC
	    PC7     TCBM DAV

	*/

	uint8_t data = 0;

	// JP1
	data |= m_jp1->read() << 5;

	// SYNC detect line
	data |= m_ga->sync_r() << 6;

	// TCBM status
	data |= tcbm_r(TCBM_STATUS) & tcbm_r(TCBM_HOST_STATUS);

	// TCBM data valid
	data |= tcbm_r(TCBM_DAV) << 7;

	return data;
}

void c1551_device::tpi0_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0     TCBM STATUS0
	    PC1     TCBM STATUS1
	    PC2     TCBM DEV
	    PC3     TCBM ACK
	    PC4     MODE
	    PC5
	    PC6
	    PC7

	*/

	// TCBM status
	tcbm_w(TCBM_STATUS, data & 0x03);

	// TCBM device number
	m_dev = BIT(data, 2);

	// TCBM acknowledge
	tcbm_w(TCBM_ACK, BIT(data, 3));

	// read/write mode
	m_ga->oe_w(BIT(data, 4));
}

//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

uint8_t c1551_device::tpi1_pb_r()
{
	/*

	    bit     description

	    PB0     STATUS0
	    PB1     STATUS1
	    PB2
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	return tcbm_r(TCBM_STATUS) & tcbm_r(TCBM_HOST_STATUS);
}

void c1551_device::tpi1_pb_w(uint8_t data)
{
	tcbm_w(TCBM_HOST_STATUS, data & 0x03);
}

uint8_t c1551_device::tpi1_pc_r()
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6
	    PC7     TCBM ACK

	*/

	uint8_t data = 0;

	// TCBM acknowledge
	data |= tcbm_r(TCBM_ACK) << 7;

	return data;
}

void c1551_device::tpi1_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6     TCBM DAV
	    PC7

	*/

	// TCBM data valid
	tcbm_w(TCBM_DAV, BIT(data, 6));
}

uint8_t c1551_device::tpi1_r(offs_t offset)
{
	if ((offset < 3) && !tcbm_sample_ready())
		return 0xff;

	return m_tpi1->read(offset);
}


//-------------------------------------------------
//  tcbm_w - drive a TCBM line; port outputs
//  change half a cycle after the write
//-------------------------------------------------

void c1551_device::tcbm_w(int line, uint8_t state)
{
	uint8_t &count = m_tcbm_count[line];
	attotime *const times = m_tcbm_time[line];
	uint8_t *const states = m_tcbm_state[line];
	if (states[count - 1] == state)
		return;

	attotime time = machine().time();
	device_execute_interface *const exec = machine().scheduler().currently_executing();
	if (exec)
	{
		attotime const cycle = exec->cycles_to_attotime(1);
		if (!cycle.is_never())
			time += cycle / 2;
	}
	time = std::max(time, times[count - 1]);

	if (times[count - 1] == time)
	{
		if ((count > 1) && (states[count - 2] == state))
			count--;
		else
			states[count - 1] = state;
	}
	else
	{
		if (count == EDGE_COUNT)
		{
			std::copy(times + 1, times + count, times);
			std::copy(states + 1, states + count, states);
			count--;
		}
		times[count] = time;
		states[count] = state;
		count++;
	}
}


//-------------------------------------------------
//  tcbm_r - TCBM line state at the current time
//-------------------------------------------------

uint8_t c1551_device::tcbm_r(int line) const
{
	attotime const time = machine().time();
	int i = m_tcbm_count[line] - 1;
	while ((i > 0) && (m_tcbm_time[line][i] > time))
		i--;

	if ((i == 0) && (m_tcbm_time[line][0] > time) && !machine().side_effects_disabled())
		logerror("TCBM line %d history overflow\n", line);

	return m_tcbm_state[line][i];
}


//-------------------------------------------------
//  tcbm_sample_ready - hold a CPU's TCBM port
//  read until every other CPU has passed the
//  point where its outputs could still change it
//-------------------------------------------------

bool c1551_device::tcbm_sample_ready()
{
	if (machine().side_effects_disabled())
		return true;

	cpu_device *const host = &m_slot->cpu();
	cpu_device *const drive = m_maincpu;
	device_execute_interface *const executing = machine().scheduler().currently_executing();
	if ((executing != host) && (executing != drive))
		return true;

	cpu_device *const reader = (executing == host) ? host : drive;
	cpu_device *const other = (executing == host) ? drive : host;
	if (other->suspended(SUSPEND_ANY_REASON & ~SUSPEND_REASON_TIMESLICE))
		return true;

	attotime const cycle = other->cycles_to_attotime(1);
	if ((other->local_time() + (cycle / 2)) <= reader->local_time())
	{
		if ((other->local_time() + cycle) > reader->local_time())
		{
			reader->yield();
			machine().scheduler().perfect_quantum(cycle);
		}
		reader->retry_access();
		return false;
	}

	return true;
}

//-------------------------------------------------
//  ADDRESS_MAP( c1551_mem )
//-------------------------------------------------

void c1551_device::c1551_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x4000, 0x4007).mirror(0x3ff8).rw(FUNC(c1551_device::tpi0_r), FUNC(c1551_device::tpi0_w));
	map(0xc000, 0xffff).rom().region(M6510T_TAG, 0);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c1551_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D64_FORMAT);
	fr.add(FLOPPY_G64_FORMAT);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1551_device::device_add_mconfig(machine_config &config)
{
	M6510T(config, m_maincpu, XTAL(16'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1551_device::c1551_mem);
	m_maincpu->read_callback().set(FUNC(c1551_device::port_r));
	m_maincpu->write_callback().set(FUNC(c1551_device::port_w));

	PLS100(config, m_pla);

	TPI6525(config, m_tpi0);
	m_tpi0->in_pa_cb().set(FUNC(c1551_device::tcbm_data_r));
	m_tpi0->out_pa_cb().set(FUNC(c1551_device::tcbm_data_w<0>));
	m_tpi0->in_pb_cb().set(m_ga, FUNC(c64h156_device::yb_r));
	m_tpi0->out_pb_cb().set(m_ga, FUNC(c64h156_device::yb_w));
	m_tpi0->in_pc_cb().set(FUNC(c1551_device::tpi0_pc_r));
	m_tpi0->out_pc_cb().set(FUNC(c1551_device::tpi0_pc_w));

	TPI6525(config, m_tpi1);
	m_tpi1->in_pa_cb().set(FUNC(c1551_device::tcbm_data_r));
	m_tpi1->out_pa_cb().set(FUNC(c1551_device::tcbm_data_w<1>));
	m_tpi1->in_pb_cb().set(FUNC(c1551_device::tpi1_pb_r));
	m_tpi1->out_pb_cb().set(FUNC(c1551_device::tpi1_pb_w));
	m_tpi1->in_pc_cb().set(FUNC(c1551_device::tpi1_pc_r));
	m_tpi1->out_pc_cb().set(FUNC(c1551_device::tpi1_pc_w));

	C64H156(config, m_ga, XTAL(16'000'000));
	m_ga->byte_callback().set(m_ga, FUNC(c64h156_device::atni_w));

	floppy_connector &connector(FLOPPY_CONNECTOR(config, C64H156_TAG":0"));
	connector.option_add("525ssqd", FLOPPY_525_SSQD);
	connector.set_default_option("525ssqd");
	connector.set_fixed(true);
	connector.set_formats(c1551_device::floppy_formats);
	connector.enable_sound(true);

	plus4_expansion_slot_device::add_passthrough(config, "exp");
}


//-------------------------------------------------
//  INPUT_PORTS( c1551 )
//-------------------------------------------------

static INPUT_PORTS_START( c1551 )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "Device Number" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1551_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1551 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1551_device - constructor
//-------------------------------------------------

c1551_device::c1551_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, C1551, tag, owner, clock)
	, device_plus4_expansion_card_interface(mconfig, *this)
	, m_maincpu(*this, M6510T_TAG)
	, m_tpi0(*this, "u3")
	, m_tpi1(*this, "ci_u2")
	, m_ga(*this, C64H156_TAG)
	, m_pla(*this, PLA_TAG)
	, m_floppy(*this, C64H156_TAG":0:525ssqd")
	, m_jp1(*this, "JP1")
	, m_leds(*this, "led%u", 0U)
	, m_dev(0)
	, m_irq_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1551_device::device_start()
{
	// allocate timers
	m_irq_timer = timer_alloc(FUNC(c1551_device::irq_timer_tick), this);
	m_irq_timer->adjust(attotime::zero, CLEAR_LINE);

	// install image callbacks
	m_ga->set_floppy(m_floppy);

	static constexpr uint8_t TCBM_INIT[TCBM_COUNT] = { 0xff, 0xff, 1, 3, 1, 1 };
	for (int line = 0; line < TCBM_COUNT; line++)
	{
		std::fill(std::begin(m_tcbm_time[line]), std::end(m_tcbm_time[line]), attotime::zero);
		std::fill(std::begin(m_tcbm_state[line]), std::end(m_tcbm_state[line]), TCBM_INIT[line]);
		m_tcbm_count[line] = 1;
	}

	// register for state saving
	save_item(NAME(m_tcbm_time));
	save_item(NAME(m_tcbm_state));
	save_item(NAME(m_tcbm_count));
	save_item(NAME(m_dev));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1551_device::device_reset()
{
	// initialize gate array
	m_ga->test_w(1);
	m_ga->soe_w(1);
	m_ga->accl_w(1);
	m_ga->atna_w(1);

	offs_t const base = m_jp1->read() ? 0x1c0 : 0x1e0;
	m_slot->io().install_readwrite_handler(base, base + 0x07, 0x18, emu::rw_delegate(*this, FUNC(c1551_device::tpi1_r)), emu::rw_delegate(*m_tpi1, FUNC(tpi6525_device::write)));
}


//-------------------------------------------------
//  irq_timer_tick - update IRQ line state
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(c1551_device::irq_timer_tick)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, param);

	if (param == ASSERT_LINE)
	{
		// Ts = 0.7*R2*C1 = 0.7*100R*0.1uF = 7us
		m_irq_timer->adjust(attotime::from_usec(7), CLEAR_LINE);
	}
	else
	{
		// Tm = 0.7*(R1+R2)*C1 = 0.7*(120K+100R)*0.1uF = 0.008407s
		m_irq_timer->adjust(attotime::from_usec(8407), ASSERT_LINE);
	}
}
