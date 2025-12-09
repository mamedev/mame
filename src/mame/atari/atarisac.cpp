// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarisac.cpp

    Functions to emulate the Atari "SAC" audio board

***************************************************************************/

#include "emu.h"
#include "atarisac.h"

#include "cpu/m68000/m68000.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ATARI_SAC, atari_sac_device, "atarisac", "Atari SAC Sound Board")



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void atari_sac_device::sac_6502_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).mirror(0x7fe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x3f9).w(m_datin, FUNC(generic_latch_8_device::write));
	map(0x2802, 0x2802).mirror(0x3f9).rw(FUNC(atari_sac_device::sound_irq_ack_r), FUNC(atari_sac_device::sound_irq_ack_w));
	map(0x2804, 0x2804).mirror(0x3f9).w(m_soundcomm, FUNC(atari_sound_comm_device::sound_response_w));
	map(0x2806, 0x2806).mirror(0x3f9).w(FUNC(atari_sac_device::wrio_w));
	map(0x2c00, 0x2c00).mirror(0x3f9).r(m_soundcomm, FUNC(atari_sound_comm_device::sound_command_r));
	map(0x2c02, 0x2c02).mirror(0x3f9).r(FUNC(atari_sac_device::rdio_r));
	map(0x2c04, 0x2c04).mirror(0x3f9).r(m_datout, FUNC(generic_latch_8_device::read));
	map(0x2c06, 0x2c06).mirror(0x3f9).r(FUNC(atari_sac_device::pstat_r));
	map(0x3000, 0x3fff).bankr("cpubank");
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  68000 Sound CPU memory handlers
 *
 *************************************/

void atari_sac_device::sac_68k_map(address_map &map)
{
	map.global_mask(0x87ffff);
	map(0x000000, 0x03ffff).rom();
	map(0x860000, 0x860001).mirror(0x187fe).r(FUNC(atari_sac_device::rdp8_r));
	map(0x860800, 0x860800).mirror(0x187fe).w(m_datout, FUNC(generic_latch_8_device::write));
	map(0x861000, 0x861000).mirror(0x187fe).w(m_datin, FUNC(generic_latch_8_device::acknowledge_w));
	map(0x861800, 0x861801).mirror(0x187ee).select(0x10).w(FUNC(atari_sac_device::dac_w));
	map(0x864000, 0x867fff).mirror(0x18000).ram(); // 8Kx8x2
}



//**************************************************************************
//  I/O PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START( sac_ioports )
	PORT_START("SAC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::sound_to_main_ready)) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::main_to_sound_ready)) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(atari_sac_device::main_test_read_line)) // self test
INPUT_PORTS_END



//**************************************************************************
//  SAC-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_sac_device: Constructor
//-------------------------------------------------

atari_sac_device::atari_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: atari_jsa_base_device(mconfig, ATARI_SAC, tag, owner, clock)
	, m_daccpu(*this, "dac")
	, m_datin(*this, "datin")
	, m_datout(*this, "datout")
	, m_rdac(*this, "rdac")
	, m_ldac(*this, "ldac")
	, m_inputs(*this, "SAC")
	, m_68k_reset(false)
	, m_10k_int(false)
{
}


//-------------------------------------------------
//  rdio_r: Handle reads from the general I/O
//  port on a SAC board
//-------------------------------------------------

u8 atari_sac_device::rdio_r()
{
	//
	//  0x80 = self test
	//  0x40 = NMI line state (active low)
	//  0x20 = sound output full
	//  0x10 = +5V
	//  0x08 = coin 4
	//  0x04 = coin 3
	//  0x02 = coin 2
	//  0x01 = coin 1
	//

	u8 result = m_inputs->read();
	if (!m_test_read_cb())
		result ^= 0x80;

	return result;
}


//-------------------------------------------------
//  wrio_w: Handle writes to the general I/O
//  port on a SAC board
//-------------------------------------------------

void atari_sac_device::wrio_w(u8 data)
{
	//
	//  0xc0 = bank address
	//  0x20 = coin counter 2
	//  0x10 = coin counter 1
	//  0x08 = 68K reset (active low)
	//  0x06 = not used
	//  0x01 = YM2151 reset (active low)
	//

	// update the bank
	m_cpu_bank->set_entry(BIT(data, 6, 2));

	// coin counters
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));

	// reset the 68K
	m_68k_reset = !BIT(data, 3);
	m_daccpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	// reset the YM2151
	m_ym2151->reset_w(BIT(data, 0));
}


//-------------------------------------------------
//  pstat_r: Check internal communications status
//  from the 6502 side
//-------------------------------------------------

u8 atari_sac_device::pstat_r()
{
	int temp = 0xff;
	if (m_datin->pending_r()) temp ^= 0x80;
	if (m_datout->pending_r()) temp ^= 0x40;
	if (m_68k_reset) temp ^= 0x20;
	return temp;
}


//-------------------------------------------------
//  int_10k_gen: Generate 10kHz interrupt for
//  driving the DAC
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atari_sac_device::int_10k_gen)
{
	if (!m_10k_int)
	{
		m_10k_int = true;
		m_daccpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
}


//-------------------------------------------------
//  rdp8_r: Read data sent by the 6502 and check
//  communications status
//-------------------------------------------------

u16 atari_sac_device::rdp8_r()
{
	int temp = (m_datin->read() << 8) | 0xff;

	if (m_datin->pending_r()) temp ^= 0x08;
	if (m_datout->pending_r()) temp ^= 0x04;
	return temp;
}


//-------------------------------------------------
//  dac_w: Write a sample to one of the DAC
//  channels
//-------------------------------------------------

void atari_sac_device::dac_w(offs_t offset, u16 data)
{
	//int clip = BIT(data, 15);
	//int off0b = BIT(data, 13) | BIT(data, 14);
	//int off4b = BIT(data, 13) & BIT(data, 14);
	u16 sample = ((data >> 3) & 0x800) | ((data >> 2) & 0x7ff);

	if (offset & 8)
		m_ldac->write(sample);
	else
		m_rdac->write(sample);

	if (m_10k_int)
	{
		m_10k_int = false;
		m_daccpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atari_sac_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_jsacpu, 14.318181_MHz_XTAL/8);
	m_jsacpu->set_addrmap(AS_PROGRAM, &atari_sac_device::sac_6502_map);
	m_jsacpu->set_periodic_int(FUNC(atari_sac_device::sound_irq_gen), attotime::from_hz(14.318181_MHz_XTAL/4/4/16/16/14));

	M68000(config, m_daccpu, 14.318181_MHz_XTAL/2);
	m_daccpu->set_addrmap(AS_PROGRAM, &atari_sac_device::sac_68k_map);
	m_daccpu->set_periodic_int(FUNC(atari_sac_device::int_10k_gen), attotime::from_hz(10000));
	// TODO: determine exact frequency (controlled by a PAL16R4)

	GENERIC_LATCH_8(config, m_datin);
	m_datin->data_pending_callback().set_inputline(m_daccpu, M68K_IRQ_2);
	m_datin->set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, m_datout);

	// sound hardware
	ATARI_SOUND_COMM(config, m_soundcomm, m_jsacpu)
		.int_callback().set(FUNC(atari_sac_device::main_int_write_line));

	YM2151(config, m_ym2151, 14.318181_MHz_XTAL/4);
	m_ym2151->irq_handler().set(FUNC(atari_sac_device::ym2151_irq_gen));
	m_ym2151->port_write_handler().set(FUNC(atari_sac_device::ym2151_port_w));
	m_ym2151->add_route(0, *this, 0.60, 0);
	m_ym2151->add_route(1, *this, 0.60, 1);

	// FIXME: there is actually only one DAC (plus some analog switches)
	AM6012(config, m_rdac).add_route(ALL_OUTPUTS, *this, 0.5, 1); // AM6012.6j
	AM6012(config, m_ldac).add_route(ALL_OUTPUTS, *this, 0.5, 0); // AM6012.6j
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_sac_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sac_ioports );
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_sac_device::device_start()
{
	// call the parent
	atari_jsa_base_device::device_start();

	// save states
	save_item(NAME(m_68k_reset));
	save_item(NAME(m_10k_int));
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void atari_sac_device::device_reset()
{
	// call the parent
	atari_jsa_base_device::device_reset();

	// hold 68K in reset
	m_68k_reset = true;
	m_daccpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


//-------------------------------------------------
//  update_all_volumes: Update volumes for all
//  chips
//-------------------------------------------------

void atari_sac_device::update_all_volumes()
{
	// TODO: CT1 and CT2 control LPFs on left and right channels
}
