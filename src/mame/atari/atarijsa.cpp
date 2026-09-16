// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarijsa.cpp

    Functions to emulate the Atari "JSA" audio boards

****************************************************************************

    JSA I (stereo): used by:
        * Blasteroids (YM2151 only)
        * Toobin' (YM2151 + POKEY)
        * Vindicators (YM2151 + POKEY)
        * Escape from the Planets of the Robot Monsters (YM2151 + TMS5220)

    JSA II (mono), used by:
        * Cyberball 2072
        * STUN Runner
        * Skull & Crossbones
        * ThunderJaws
        * Hydra
        * Pit Fighter

    JSA III (mono), used by:
        * Off the Wall (YM2151 only)
        * Batman
        * Guardians of the 'Hood
        * Road Riot 4WD
        * Steel Talons

    JSA IIIs (stereo), used by:
        * Space Lords
        * Moto Frenzy
        * Road Riot's Revenge Rally

****************************************************************************

Atari Audio Board II
--------------------

6502 MEMORY MAP

Function                                  Address     R/W  Data
---------------------------------------------------------------
Program RAM                               0000-1FFF   R/W  D0-D7

Music (YM-2151)                           2000-2001   R/W  D0-D7

Read 68010 Port (Input Buffer)            280A        R    D0-D7

Self-test                                 280C        R    D7
Output Buffer Full (@2A02) (Active High)              R    D5
Left Coin Switch                                      R    D1
Right Coin Switch                                     R    D0

Interrupt acknowledge                     2A00        W    xx
Write 68010 Port (Outbut Buffer)          2A02        W    D0-D7
Banked ROM select (at 3000-3FFF)          2A04        W    D6-D7
???                                       2A06        W

Effects                                   2C00-2C0F   R/W  D0-D7

Banked Program ROM (4 pages)              3000-3FFF   R    D0-D7
Static Program ROM (48K bytes)            4000-FFFF   R    D0-D7

TODO:
JSA-i: stereo gating for POKEY/TMS5220C is currently only mono, only looking at ym2151_ct1;
  the two commented-out lines would be correct if stereo volume-set functions were written.
ALL: the LPF (low pass filter) bit which selectively places a lowpass filter in the output
  path for all channels is currently unimplemented; someone who knows analog magic will need
  to handle this.

***************************************************************************/

#include "emu.h"
#include "atarijsa.h"


#define JSA_MASTER_CLOCK            XTAL(3'579'545)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ATARI_JSA_I,    atari_jsa_i_device,    "atjsa1",  "Atari JSA I Sound Board")
DEFINE_DEVICE_TYPE(ATARI_JSA_II,   atari_jsa_ii_device,   "atjsa2",  "Atari JSA II Sound Board")
DEFINE_DEVICE_TYPE(ATARI_JSA_III,  atari_jsa_iii_device,  "atjsa3",  "Atari JSA III Sound Board")
DEFINE_DEVICE_TYPE(ATARI_JSA_IIIS, atari_jsa_iiis_device, "atjsa3s", "Atari JSA IIIs Sound Board")



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

void atari_jsa_i_device::atarijsa1_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x01f9);                                                                      // N/C
	map(0x2802, 0x2802).mirror(0x01f9).r(m_soundcomm, FUNC(atari_sound_comm_device::sound_command_r));    // /RDP
	map(0x2804, 0x2804).mirror(0x01f9).r(FUNC(atari_jsa_i_device::rdio_r));                                                      // /RDIO
	map(0x2806, 0x2806).mirror(0x01f9).rw(FUNC(atari_jsa_i_device::sound_irq_ack_r), FUNC(atari_jsa_i_device::sound_irq_ack_w));  // R/W=/IRQACK
	map(0x2a00, 0x2a00).mirror(0x01f9).w(FUNC(atari_jsa_i_device::tms5220_voice));                                              // /VOICE
	map(0x2a02, 0x2a02).mirror(0x01f9).w(m_soundcomm, FUNC(atari_sound_comm_device::sound_response_w));  // /WRP
	map(0x2a04, 0x2a04).mirror(0x01f9).w(FUNC(atari_jsa_i_device::wrio_w));                                                     // /WRIO
	map(0x2a06, 0x2a06).mirror(0x01f9).w(FUNC(atari_jsa_i_device::mix_w));                                                      // /MIX
	map(0x2c00, 0x2c0f).mirror(0x03f0).rw(FUNC(atari_jsa_i_device::pokey_r), FUNC(atari_jsa_i_device::pokey_w));
	map(0x3000, 0x3fff).bankr("cpubank");
	map(0x4000, 0xffff).rom();
}


void atari_jsa_ii_device::atarijsa2_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x01f9).r(FUNC(atari_jsa_ii_device::oki_r));                                                       // /RDV
	map(0x2802, 0x2802).mirror(0x01f9).r(m_soundcomm, FUNC(atari_sound_comm_device::sound_command_r));    // /RDP
	map(0x2804, 0x2804).mirror(0x01f9).r(FUNC(atari_jsa_ii_device::rdio_r));                                                      // /RDIO
	map(0x2806, 0x2806).mirror(0x01f9).rw(FUNC(atari_jsa_ii_device::sound_irq_ack_r), FUNC(atari_jsa_ii_device::sound_irq_ack_w));  // R/W=/IRQACK
	map(0x2a00, 0x2a00).mirror(0x01f9).w(FUNC(atari_jsa_ii_device::oki_w));                                                      // /WRV
	map(0x2a02, 0x2a02).mirror(0x01f9).w(m_soundcomm, FUNC(atari_sound_comm_device::sound_response_w));  // /WRP
	map(0x2a04, 0x2a04).mirror(0x01f9).w(FUNC(atari_jsa_ii_device::wrio_w));                                                     // /WRIO
	map(0x2a06, 0x2a06).mirror(0x01f9).w(FUNC(atari_jsa_ii_device::mix_w));                                                      // /MIX
	map(0x3000, 0x3fff).bankr("cpubank");
	map(0x4000, 0xffff).rom();
}


// full map verified from schematics and Batman GALs
void atari_jsa_iii_device::atarijsa3_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).mirror(0x07fe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2801).mirror(0x05f8).rw(FUNC(atari_jsa_iii_device::oki_r), FUNC(atari_jsa_iii_device::overall_volume_w));                                // /RDV
	map(0x2802, 0x2802).mirror(0x05f9).r(m_soundcomm, FUNC(atari_sound_comm_device::sound_command_r));    // /RDP
	map(0x2804, 0x2804).mirror(0x05f9).r(FUNC(atari_jsa_iii_device::rdio_r));                                                      // /RDIO
	map(0x2806, 0x2806).mirror(0x05f9).rw(FUNC(atari_jsa_iii_device::sound_irq_ack_r), FUNC(atari_jsa_iii_device::sound_irq_ack_w));  // R/W=/IRQACK
	map(0x2a00, 0x2a01).mirror(0x05f8).w(FUNC(atari_jsa_iii_device::oki_w));                                                      // /WRV
	map(0x2a02, 0x2a02).mirror(0x05f9).w(m_soundcomm, FUNC(atari_sound_comm_device::sound_response_w));  // /WRP
	map(0x2a04, 0x2a04).mirror(0x05f9).w(FUNC(atari_jsa_iii_device::wrio_w));                                                     // /WRIO
	map(0x2a06, 0x2a06).mirror(0x05f9).w(FUNC(atari_jsa_iii_device::mix_w));                                                      // /MIX
	map(0x3000, 0x3fff).bankr("cpubank");
	map(0x4000, 0xffff).rom();
}


void atari_jsa_iii_device::jsa3_oki1_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("oki1lo");
	map(0x20000, 0x3ffff).bankr("oki1hi");
}


void atari_jsa_iiis_device::jsa3_oki2_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("oki2lo");
	map(0x20000, 0x3ffff).bankr("oki2hi");
}


//**************************************************************************
//  I/O PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START( jsa_i_ioports )
	PORT_START("JSAI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )    // speech chip ready
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::sound_to_main_ready)) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::main_to_sound_ready)) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(atari_jsa_base_device::main_test_read_line)) // self test
INPUT_PORTS_END

INPUT_PORTS_START( jsa_i_ioports_swapped_coins )
	PORT_INCLUDE( jsa_i_ioports )

	PORT_MODIFY("JSAI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

INPUT_PORTS_START( jsa_ii_ioports )
	PORT_START("JSAII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::sound_to_main_ready)) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::main_to_sound_ready)) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(atari_jsa_base_device::main_test_read_line)) // self test
INPUT_PORTS_END

INPUT_PORTS_START( jsa_ii_ioports_swapped_coins )
	PORT_INCLUDE( jsa_ii_ioports )

	PORT_MODIFY("JSAII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

INPUT_PORTS_START( jsa_iii_ioports )
	PORT_START("JSAIII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(atari_jsa_base_device::main_test_read_line)) // self test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::sound_to_main_ready)) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", FUNC(atari_sound_comm_device::main_to_sound_ready)) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(atari_jsa_base_device::main_test_read_line)) // self test
INPUT_PORTS_END

INPUT_PORTS_START( jsa_iii_ioports_swapped_coins )
	PORT_INCLUDE( jsa_iii_ioports )

	PORT_MODIFY("JSAIII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

//**************************************************************************
//  BASE DEVICE CLASS
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_base_device - constructor
//-------------------------------------------------

atari_jsa_base_device::atari_jsa_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, devtype, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_soundcomm(*this, "soundcomm"),
		m_jsacpu(*this, "cpu"),
		m_ym2151(*this, "ym2151"),
		m_cpu_region(*this, "cpu"),
		m_cpu_bank(*this, "cpubank"),
		m_test_read_cb(*this, 0),
		m_main_int_cb(*this),
		m_timed_int(false),
		m_ym2151_int(false),
		m_ym2151_volume(1.0),
		m_ym2151_ct1(0),
		m_ym2151_ct2(0),
		m_swapped_coins(false)
{
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_jsa_base_device::device_start()
{
	// configure CPU bank
	m_cpu_bank->configure_entries(0, 4, m_cpu_region->base(), 0x1000);

	// save states
	save_item(NAME(m_timed_int));
	save_item(NAME(m_ym2151_int));
	save_item(NAME(m_ym2151_volume));
	save_item(NAME(m_ym2151_ct1));
	save_item(NAME(m_ym2151_ct2));
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void atari_jsa_base_device::device_reset()
{
	// reset the static states
	m_ym2151_volume = 1.0;
	m_ym2151_ct1 = 0;
	m_ym2151_ct2 = 0;

	// Guardians of the Hood assumes we're reset to bank 0 on startup
	m_cpu_bank->set_entry(0);
}


//-------------------------------------------------
//  main_command_w: Handle command writes
//-------------------------------------------------

void atari_jsa_base_device::main_command_w(uint8_t data)
{
	m_soundcomm->main_command_w(data);
}


//-------------------------------------------------
//  main_response_r: Handle response reads
//-------------------------------------------------

uint8_t atari_jsa_base_device::main_response_r()
{
	return m_soundcomm->main_response_r();
}


//-------------------------------------------------
//  sound_reset_w: Reset the sound board
//-------------------------------------------------

void atari_jsa_base_device::sound_reset_w(uint16_t data)
{
	m_soundcomm->sound_reset_w();
}


//-------------------------------------------------
//  ym2151_port_w: Handle writes from the YM2151
//  output port
//-------------------------------------------------

void atari_jsa_base_device::ym2151_port_w(uint8_t data)
{
	m_ym2151_ct1 = (data >> 0) & 1;
	m_ym2151_ct2 = (data >> 1) & 1;
	update_all_volumes();
}


//-------------------------------------------------
//  main_test_read_line: Return the state of the
//  main's test line, provided by a callback
//-------------------------------------------------

int atari_jsa_base_device::main_test_read_line()
{
	return !m_test_read_cb();
}


//-------------------------------------------------
//  main_int_write_line: Forward interrupt signals
//  from the comm device to the owning callback
//-------------------------------------------------

void atari_jsa_base_device::main_int_write_line(int state)
{
	m_main_int_cb(state);
}


//-------------------------------------------------
//  sound_irq_gen: Generates an IRQ signal to the
//  6502 sound processor.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atari_jsa_base_device::sound_irq_gen)
{
	m_timed_int = 1;
	update_sound_irq();
}


//-------------------------------------------------
//  sound_irq_ack_r: Resets the IRQ signal to the
//  6502 sound processor. Both reads and writes
//  can be used.
//-------------------------------------------------

u8 atari_jsa_base_device::sound_irq_ack_r()
{
	if (!machine().side_effects_disabled())
	{
		m_timed_int = 0;
		update_sound_irq();
	}
	return 0;
}

void atari_jsa_base_device::sound_irq_ack_w(u8 data)
{
	m_timed_int = 0;
	update_sound_irq();
}


//-------------------------------------------------
//  ym2151_irq_gen: Sets the state of the
//  YM2151's IRQ line.
//-------------------------------------------------

void atari_jsa_base_device::ym2151_irq_gen(int state)
{
	m_ym2151_int = state;
	update_sound_irq();
}


//-------------------------------------------------
//  update_sound_irq: Called whenever the IRQ state
//  changes. An interrupt is generated if either
//  sound_irq_gen() was called, or if the YM2151
//  generated an interrupt via the
//  ym2151_irq_gen() callback.
//-------------------------------------------------

void atari_jsa_base_device::update_sound_irq()
{
	if (m_timed_int || m_ym2151_int)
		m_jsacpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
	else
		m_jsacpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}



//**************************************************************************
//  BASE DEVICE CLASS FOR OKI6295-BASED VERSIONS
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_oki_base_device: Constructor
//-------------------------------------------------

atari_jsa_oki_base_device::atari_jsa_oki_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_base_device(mconfig, devtype, tag, owner, clock),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_oki1_region(*this, "oki1"),
		m_oki2_region(*this, "oki2"),
		m_oki1_banklo(*this, "oki1lo"),
		m_oki1_bankhi(*this, "oki1hi"),
		m_oki2_banklo(*this, "oki2lo"),
		m_oki2_bankhi(*this, "oki2hi"),
		m_oki6295_volume(1.0),
		m_overall_volume(1.0)
{
}


//-------------------------------------------------
//  oki_r: Handle reads from the OKI chip(s)
//  on the JSA II, III, and IIIs boards
//-------------------------------------------------

uint8_t atari_jsa_oki_base_device::oki_r(offs_t offset)
{
	// JSA IIIs selects the 2nd OKI via the low bit, so select it
	if (m_oki2 != nullptr && offset == 1)
		return m_oki2->read();

	// OKI may not be populated at all
	else if (m_oki1 != nullptr)
		return m_oki1->read();

	// if not present, return all 0xff
	return 0xff;
}


//-------------------------------------------------
//  oki_w: Handle writes to the OKI chip(s)
//  on the JSA II, III, and IIIs boards
//-------------------------------------------------

void atari_jsa_oki_base_device::oki_w(offs_t offset, uint8_t data)
{
	// JSA IIIs selects the 2nd OKI via the low bit, so select it
	if (m_oki2 != nullptr && offset == 1)
		m_oki2->write(data);

	// OKI may not be populated at all
	else if (m_oki1 != nullptr)
		m_oki1->write(data);
}


//-------------------------------------------------
//  wrio_w: Handle writes to the general
//  I/O port on JSA II, III, and IIIs boards
//-------------------------------------------------

void atari_jsa_oki_base_device::wrio_w(uint8_t data)
{
	//
	//  0xc0 = bank address
	//  0x20 = coin counter 2
	//  0x10 = coin counter 1
	//  0x08 = voice frequency (tweaks the OKI6295 frequency)
	//  0x04 = OKI6295 reset (active low)
	//  0x02 = OKI6295 #1 bank bit 0 (JSA III/IIIs only)
	//  0x01 = YM2151 reset (active low)
	//

	// update the bank
	m_cpu_bank->set_entry((data >> 6) & 3);

	// coin counters
	machine().bookkeeping().coin_counter_w(1, (data >> 5) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 4) & 1);

	// update the OKI frequency
	if (m_oki1 != nullptr)
	{
		m_oki1->set_pin7(data & 8);
		if ((data & 4) == 0)
			m_oki1->reset();
	}

	// same for the 2nd OKI (JSA IIIs only)
	if (m_oki2 != nullptr)
	{
		m_oki2->set_pin7(data & 8);
		if ((data & 4) == 0)
			m_oki2->reset();
	}

	// update the (left) OKI bank (JSA III/IIIs only)
	if (m_oki1_banklo != nullptr)
		m_oki1_banklo->set_entry((m_oki1_banklo->entry() & 2) | ((data >> 1) & 1));

	// reset the YM2151 if needed
	m_ym2151->reset_w(BIT(data, 0));
}


//-------------------------------------------------
//  mix_w: Handle writes to the mixing
//  register on JSA II, III, and IIIs boards
//-------------------------------------------------

void atari_jsa_oki_base_device::mix_w(uint8_t data)
{
	//
	//  0xc0 = right OKI6295 bank bits 0-1 (JSA IIIs only)
	//  0x20 = low-pass filter enable
	//  0x10 = OKI6295 #1 bank bit 1
	//  0x0e = YM2151 volume (0-7)
	//  0x01 = OKI6295 volume (0-1)
	//

	// update the right OKI bank (JSA IIIs only)
	if (m_oki2_banklo != nullptr)
		m_oki2_banklo->set_entry((data >> 6) & 3);

	// TODO: emulate the low pass filter!

	// update the (left) OKI bank (JSA III/IIIs only)
	if (m_oki1_banklo != nullptr)
		m_oki1_banklo->set_entry((m_oki1_banklo->entry() & 1) | ((data >> 3) & 2));

	// update the volumes
	m_ym2151_volume = ((data >> 1) & 7) / 7.0;
	m_oki6295_volume = (data & 1) ? 1.0 : 0.5;
	update_all_volumes();
}


//-------------------------------------------------
//  overall_volume_w: Handle writes to control the
//  total sound volume on JSA III/IIIs boards
//-------------------------------------------------

void atari_jsa_oki_base_device::overall_volume_w(uint8_t data)
{
	m_overall_volume = data / 127.0;
	update_all_volumes();
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_jsa_oki_base_device::device_start()
{
	// call the parent
	atari_jsa_base_device::device_start();

	// save states
	save_item(NAME(m_oki6295_volume));
	save_item(NAME(m_overall_volume));

	// configure JSA III ADPCM banking
	if (m_oki1_banklo.found() && m_oki1_bankhi.found() && m_oki1_region->bytes() >= 0x80000)
	{
		m_oki1_banklo->configure_entries(0, 2, m_oki1_region->base() + 0x00000, 0x00000);
		m_oki1_banklo->configure_entries(2, 2, m_oki1_region->base() + 0x20000, 0x20000);
		m_oki1_bankhi->set_base(m_oki1_region->base() + 0x60000);
	}

	// configure JSA IIIs ADPCM banking
	if (m_oki2_banklo.found() && m_oki2_bankhi.found() && m_oki2_region->bytes() >= 0x80000)
	{
		m_oki2_banklo->configure_entries(0, 2, m_oki2_region->base() + 0x00000, 0x00000);
		m_oki2_banklo->configure_entries(2, 2, m_oki2_region->base() + 0x20000, 0x20000);
		m_oki2_bankhi->set_base(m_oki2_region->base() + 0x60000);
	}
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void atari_jsa_oki_base_device::device_reset()
{
	// call the parent
	atari_jsa_base_device::device_reset();

	// reset the static states
	m_oki6295_volume = 1.0;
	m_overall_volume = 1.0;
	update_all_volumes();
}


//-------------------------------------------------
//  update_all_volumes: Update volumes for all
//  chips
//-------------------------------------------------

void atari_jsa_oki_base_device::update_all_volumes()
{
	if (m_oki1.found())
		m_oki1->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_oki6295_volume * m_ym2151_ct1);
	if (m_oki2.found())
		m_oki2->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_oki6295_volume * m_ym2151_ct1);
	m_ym2151->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_ym2151_volume);
}



//**************************************************************************
//  JSA I-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_i_device: Constructor
//-------------------------------------------------

atari_jsa_i_device::atari_jsa_i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_base_device(mconfig, ATARI_JSA_I, tag, owner, clock),
		m_pokey(*this, "pokey"),
		m_tms5220(*this, "tms"),
		m_jsai(*this, "JSAI"),
		m_pokey_volume(1.0),
		m_tms5220_volume(1.0)
{
}


//-------------------------------------------------
//  rdio_r: Handle reads from the general I/O
//  port on a JSA I board
//-------------------------------------------------

uint8_t atari_jsa_i_device::rdio_r()
{
	//
	//  0x80 = self test
	//  0x40 = NMI line state (active low)
	//  0x20 = sound output full
	//  0x10 = TMS5220 ready (active low)
	//  0x08 = +5V
	//  0x04 = +5V
	//  0x02 = coin 2
	//  0x01 = coin 1
	//

	uint8_t result = m_jsai->read();
	if (!m_test_read_cb())
		result ^= 0x80;
	if (m_tms5220 != nullptr && m_tms5220->readyq_r() == 0)
		result |= 0x10;
	else
		result &= ~0x10;

	return result;
}


//-------------------------------------------------
//  wrio_w: Handle writes to the general I/O
//  port on a JSA I board
//-------------------------------------------------

void atari_jsa_i_device::wrio_w(uint8_t data)
{
	//
	//  0xc0 = bank address
	//  0x20 = coin counter 2
	//  0x10 = coin counter 1
	//  0x08 = squeak (tweaks the 5220 frequency)
	//  0x04 = TMS5220 reset (actually the read strobe) (active low)
	//  0x02 = TMS5220 write strobe (active low)
	//  0x01 = YM2151 reset (active low)
	//

	// update the bank
	m_cpu_bank->set_entry((data >> 6) & 3);

	// coin counters
	machine().bookkeeping().coin_counter_w(1, (data >> 5) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 4) & 1);

	// handle TMS5220 I/O
	if (m_tms5220 != nullptr)
	{
		int count = 5 | ((data >> 2) & 2);
		m_tms5220->set_unscaled_clock(JSA_MASTER_CLOCK*2 / (16 - count));
		m_tms5220->wsq_w((data >> 1) & 1);
		m_tms5220->rsq_w((data >> 2) & 1);
	}

	// reset the YM2151 if needed
	m_ym2151->reset_w(BIT(data, 0));
}


//-------------------------------------------------
//  mix_w: Handle writes to the mixing register
//  on a JSA I board
//-------------------------------------------------

void atari_jsa_i_device::mix_w(uint8_t data)
{
	//
	//  0xc0 = TMS5220 volume (0-3)
	//  0x30 = POKEY volume (0-3)
	//  0x0e = YM2151 volume (0-7)
	//  0x01 = low-pass filter enable
	//

	m_tms5220_volume = ((data >> 6) & 3) / 3.0;
	m_pokey_volume = ((data >> 4) & 3) / 3.0;
	m_ym2151_volume = ((data >> 1) & 7) / 7.0;
	update_all_volumes();
}


//-------------------------------------------------
//  tms5220_voice: Handle writes to the TMS5220
//  voice data register
//-------------------------------------------------

void atari_jsa_i_device::tms5220_voice(uint8_t data)
{
	if (m_tms5220 != nullptr)
		m_tms5220->data_w(data);
}


//-------------------------------------------------
//  pokey_r: Handle reads from the POKEY if
//  present
//-------------------------------------------------

uint8_t atari_jsa_i_device::pokey_r(offs_t offset)
{
	if (m_pokey != nullptr)
		return m_pokey->read(offset);
	return 0xff;
}


//-------------------------------------------------
//  pokey_w: Handle writes to the POKEY if
//  present
//-------------------------------------------------

void atari_jsa_i_device::pokey_w(offs_t offset, uint8_t data)
{
	if (m_pokey != nullptr)
		m_pokey->write(offset, data);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// Fully populated JSA-I, not used by anyone
void atari_jsa_i_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_jsacpu, JSA_MASTER_CLOCK/2);
	m_jsacpu->set_addrmap(AS_PROGRAM, &atari_jsa_i_device::atarijsa1_map);
	m_jsacpu->set_periodic_int(FUNC(atari_jsa_i_device::sound_irq_gen), attotime::from_hz(JSA_MASTER_CLOCK/4/16/16/14));

	// sound hardware
	ATARI_SOUND_COMM(config, m_soundcomm, m_jsacpu)
		.int_callback().set(FUNC(atari_jsa_base_device::main_int_write_line));

	YM2151(config, m_ym2151, JSA_MASTER_CLOCK);
	m_ym2151->irq_handler().set(FUNC(atari_jsa_i_device::ym2151_irq_gen));
	m_ym2151->port_write_handler().set(FUNC(atari_jsa_base_device::ym2151_port_w));
	m_ym2151->add_route(0, *this, 0.60, 0);
	m_ym2151->add_route(1, *this, 0.60, 1);

	POKEY(config, m_pokey, JSA_MASTER_CLOCK/2);
	m_pokey->add_route(ALL_OUTPUTS, *this, 0.40, 0);
	m_pokey->add_route(ALL_OUTPUTS, *this, 0.40, 1);

	TMS5220C(config, m_tms5220, JSA_MASTER_CLOCK*2/11); // potentially JSA_MASTER_CLOCK/9 as well
	m_tms5220->add_route(ALL_OUTPUTS, *this, 1.0, 0);
	m_tms5220->add_route(ALL_OUTPUTS, *this, 1.0, 1);
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_i_device::device_input_ports() const
{
	return m_swapped_coins ? INPUT_PORTS_NAME( jsa_i_ioports_swapped_coins ) : INPUT_PORTS_NAME( jsa_i_ioports );
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_jsa_i_device::device_start()
{
	// call the parent
	atari_jsa_base_device::device_start();

	// save states
	save_item(NAME(m_pokey_volume));
	save_item(NAME(m_tms5220_volume));
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void atari_jsa_i_device::device_reset()
{
	// call the parent
	atari_jsa_base_device::device_reset();

	// reset the static states
	m_pokey_volume = 1.0;
	m_tms5220_volume = 1.0;
	update_all_volumes();
}


//-------------------------------------------------
//  update_all_volumes: Update volumes for all
//  chips
//-------------------------------------------------

void atari_jsa_i_device::update_all_volumes()
{
	if (m_tms5220 != nullptr)
		m_tms5220->set_output_gain(ALL_OUTPUTS, m_tms5220_volume * m_ym2151_ct1);
	if (m_pokey != nullptr)
		m_pokey->set_output_gain(ALL_OUTPUTS, m_pokey_volume * m_ym2151_ct1);
	m_ym2151->set_output_gain(ALL_OUTPUTS, m_ym2151_volume);
}



//**************************************************************************
//  JSA II-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_ii_device: Constructor
//-------------------------------------------------

atari_jsa_ii_device::atari_jsa_ii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_oki_base_device(mconfig, ATARI_JSA_II, tag, owner, clock)
	, m_jsaii(*this, "JSAII")
{
}


//-------------------------------------------------
//  rdio_r: Handle reads from the general I/O
//  port on a JSA II board
//-------------------------------------------------

uint8_t atari_jsa_ii_device::rdio_r()
{
	//
	//  0x80 = self test
	//  0x40 = NMI line state (active low)
	//  0x20 = sound output full
	//  0x10 = +5V
	//  0x08 = +5V
	//  0x04 = +5V
	//  0x02 = coin 2
	//  0x01 = coin 1
	//

	uint8_t result = m_jsaii->read();
	if (!m_test_read_cb())
		result ^= 0x80;

	return result;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// Fully populated JSA-II
void atari_jsa_ii_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_jsacpu, JSA_MASTER_CLOCK/2);
	m_jsacpu->set_addrmap(AS_PROGRAM, &atari_jsa_ii_device::atarijsa2_map);
	m_jsacpu->set_periodic_int(FUNC(atari_jsa_ii_device::sound_irq_gen), attotime::from_hz(JSA_MASTER_CLOCK/4/16/16/14));

	// sound hardware
	ATARI_SOUND_COMM(config, m_soundcomm, m_jsacpu)
		.int_callback().set(FUNC(atari_jsa_base_device::main_int_write_line));

	YM2151(config, m_ym2151, JSA_MASTER_CLOCK);
	m_ym2151->irq_handler().set(FUNC(atari_jsa_ii_device::ym2151_irq_gen));
	m_ym2151->port_write_handler().set(FUNC(atari_jsa_base_device::ym2151_port_w));
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.60, 0);

	OKIM6295(config, m_oki1, JSA_MASTER_CLOCK/3, okim6295_device::PIN7_HIGH);
	m_oki1->add_route(ALL_OUTPUTS, *this, 0.75, 0);
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_ii_device::device_input_ports() const
{
	return m_swapped_coins ? INPUT_PORTS_NAME( jsa_ii_ioports_swapped_coins ) : INPUT_PORTS_NAME( jsa_ii_ioports );
}



//**************************************************************************
//  JSA III-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_iii_device: Constructor
//-------------------------------------------------

atari_jsa_iii_device::atari_jsa_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_iii_device(mconfig, ATARI_JSA_III, tag, owner, clock)
{
}

atari_jsa_iii_device::atari_jsa_iii_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_oki_base_device(mconfig, devtype, tag, owner, clock)
	, m_jsaiii(*this, "JSAIII")
{
}


//-------------------------------------------------
//  jsa_iii_rdio: Handle reads from the general I/O
//  port on a JSA III/IIIs board
//-------------------------------------------------

uint8_t atari_jsa_iii_device::rdio_r()
{
	//
	//  0x80 = self test (active high)
	//  0x40 = NMI line state (active high)
	//  0x20 = sound output full (active high)
	//  0x10 = self test (active high)
	//  0x08 = service (active high)
	//  0x04 = tilt (active high)
	//  0x02 = coin L (active high)
	//  0x01 = coin R (active high)
	//

	uint8_t result = m_jsaiii->read();
	if (!m_test_read_cb())
		result ^= 0x90;
	return result;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// Fully populated JSA-III
void atari_jsa_iii_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_jsacpu, JSA_MASTER_CLOCK/2);
	m_jsacpu->set_addrmap(AS_PROGRAM, &atari_jsa_iii_device::atarijsa3_map);
	m_jsacpu->set_periodic_int(FUNC(atari_jsa_iii_device::sound_irq_gen), attotime::from_hz(JSA_MASTER_CLOCK/4/16/16/14));

	// sound hardware
	ATARI_SOUND_COMM(config, m_soundcomm, m_jsacpu)
		.int_callback().set(FUNC(atari_jsa_base_device::main_int_write_line));

	YM2151(config, m_ym2151, JSA_MASTER_CLOCK);
	m_ym2151->irq_handler().set(FUNC(atari_jsa_iii_device::ym2151_irq_gen));
	m_ym2151->port_write_handler().set(FUNC(atari_jsa_base_device::ym2151_port_w));
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.60, 0);

	OKIM6295(config, m_oki1, JSA_MASTER_CLOCK/3, okim6295_device::PIN7_HIGH);
	m_oki1->set_addrmap(0, &atari_jsa_iii_device::jsa3_oki1_map);
	m_oki1->add_route(ALL_OUTPUTS, *this, 0.75, 0);
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_iii_device::device_input_ports() const
{
	return m_swapped_coins ? INPUT_PORTS_NAME( jsa_iii_ioports_swapped_coins ) : INPUT_PORTS_NAME( jsa_iii_ioports );
}



//**************************************************************************
//  JSA IIIS-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_iiis_device: Constructor
//-------------------------------------------------

atari_jsa_iiis_device::atari_jsa_iiis_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_jsa_iii_device(mconfig, ATARI_JSA_IIIS, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// Fully populated JSA_IIIs
void atari_jsa_iiis_device::device_add_mconfig(machine_config &config)
{
	atari_jsa_iii_device::device_add_mconfig(config);

	m_ym2151->reset_routes();
	m_ym2151->add_route(0, *this, 0.60, 0);
	m_ym2151->add_route(1, *this, 0.60, 1);

	OKIM6295(config, m_oki2, JSA_MASTER_CLOCK/3, okim6295_device::PIN7_HIGH);
	m_oki2->add_route(ALL_OUTPUTS, *this, 0.75, 1);
	m_oki2->set_addrmap(0, &atari_jsa_iiis_device::jsa3_oki2_map);
}
