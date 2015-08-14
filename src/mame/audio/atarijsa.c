// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarijsa.c

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
#include "audio/atarijsa.h"


#define JSA_MASTER_CLOCK            XTAL_3_579545MHz


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type ATARI_JSA_I = &device_creator<atari_jsa_i_device>;
extern const device_type ATARI_JSA_II = &device_creator<atari_jsa_ii_device>;
extern const device_type ATARI_JSA_III = &device_creator<atari_jsa_iii_device>;
extern const device_type ATARI_JSA_IIIS = &device_creator<atari_jsa_iiis_device>;



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

static ADDRESS_MAP_START( atarijsa1_map, AS_PROGRAM, 8, atari_jsa_i_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x01f9)                                                                      // N/C
	AM_RANGE(0x2802, 0x2802) AM_MIRROR(0x01f9) AM_DEVREAD("soundcomm", atari_sound_comm_device, sound_command_r)    // /RDP
	AM_RANGE(0x2804, 0x2804) AM_MIRROR(0x01f9) AM_READ(rdio_r)                                                      // /RDIO
	AM_RANGE(0x2806, 0x2806) AM_MIRROR(0x01f9) AM_DEVREADWRITE("soundcomm", atari_sound_comm_device, sound_irq_ack_r, sound_irq_ack_w)  // R/W=/IRQACK
	AM_RANGE(0x2a00, 0x2a00) AM_MIRROR(0x01f9) AM_WRITE(tms5220_voice)                                              // /VOICE
	AM_RANGE(0x2a02, 0x2a02) AM_MIRROR(0x01f9) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_response_w)  // /WRP
	AM_RANGE(0x2a04, 0x2a04) AM_MIRROR(0x01f9) AM_WRITE(wrio_w)                                                     // /WRIO
	AM_RANGE(0x2a06, 0x2a06) AM_MIRROR(0x01f9) AM_WRITE(mix_w)                                                      // /MIX
	AM_RANGE(0x2c00, 0x2c0f) AM_MIRROR(0x03f0) AM_READWRITE(pokey_r, pokey_w)
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("cpubank")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atarijsa2_map, AS_PROGRAM, 8, atari_jsa_ii_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x01f9) AM_READ(oki_r)                                                       // /RDV
	AM_RANGE(0x2802, 0x2802) AM_MIRROR(0x01f9) AM_DEVREAD("soundcomm", atari_sound_comm_device, sound_command_r)    // /RDP
	AM_RANGE(0x2804, 0x2804) AM_MIRROR(0x01f9) AM_READ(rdio_r)                                                      // /RDIO
	AM_RANGE(0x2806, 0x2806) AM_MIRROR(0x01f9) AM_DEVREADWRITE("soundcomm", atari_sound_comm_device, sound_irq_ack_r, sound_irq_ack_w)  // R/W=/IRQACK
	AM_RANGE(0x2a00, 0x2a00) AM_MIRROR(0x01f9) AM_WRITE(oki_w)                                                      // /WRV
	AM_RANGE(0x2a02, 0x2a02) AM_MIRROR(0x01f9) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_response_w)  // /WRP
	AM_RANGE(0x2a04, 0x2a04) AM_MIRROR(0x01f9) AM_WRITE(wrio_w)                                                     // /WRIO
	AM_RANGE(0x2a06, 0x2a06) AM_MIRROR(0x01f9) AM_WRITE(mix_w)                                                      // /MIX
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("cpubank")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


// full map verified from schematics and Batman GALs
static ADDRESS_MAP_START( atarijsa3_map, AS_PROGRAM, 8, atari_jsa_iii_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x07fe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2801) AM_MIRROR(0x05f8) AM_READWRITE(oki_r, overall_volume_w)                                // /RDV
	AM_RANGE(0x2802, 0x2802) AM_MIRROR(0x05f9) AM_DEVREAD("soundcomm", atari_sound_comm_device, sound_command_r)    // /RDP
	AM_RANGE(0x2804, 0x2804) AM_MIRROR(0x05f9) AM_READ(rdio_r)                                                      // /RDIO
	AM_RANGE(0x2806, 0x2806) AM_MIRROR(0x05f9) AM_DEVREADWRITE("soundcomm", atari_sound_comm_device, sound_irq_ack_r, sound_irq_ack_w)  // R/W=/IRQACK
	AM_RANGE(0x2a00, 0x2a01) AM_MIRROR(0x05f8) AM_WRITE(oki_w)                                                      // /WRV
	AM_RANGE(0x2a02, 0x2a02) AM_MIRROR(0x05f9) AM_DEVWRITE("soundcomm", atari_sound_comm_device, sound_response_w)  // /WRP
	AM_RANGE(0x2a04, 0x2a04) AM_MIRROR(0x05f9) AM_WRITE(wrio_w)                                                     // /WRIO
	AM_RANGE(0x2a06, 0x2a06) AM_MIRROR(0x05f9) AM_WRITE(mix_w)                                                      // /MIX
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("cpubank")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( jsa3_oki1_map, AS_0, 8, atari_jsa_iii_device )
	AM_RANGE(0x00000, 0x1ffff) AM_ROMBANK("oki1lo")
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("oki1hi")
ADDRESS_MAP_END


static ADDRESS_MAP_START( jsa3_oki2_map, AS_0, 8, atari_jsa_iiis_device )
	AM_RANGE(0x00000, 0x1ffff) AM_ROMBANK("oki2lo")
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("oki2hi")
ADDRESS_MAP_END



//**************************************************************************
//  MACHINE FRAGMENTS
//**************************************************************************

// Fully populated JSA-I, not used by anyone
MACHINE_CONFIG_FRAGMENT( jsa_i_config )

	// basic machine hardware
	MCFG_CPU_ADD("cpu", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa1_map)
	MCFG_DEVICE_PERIODIC_INT_DEVICE("soundcomm", atari_sound_comm_device, sound_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	// sound hardware
	MCFG_ATARI_SOUND_COMM_ADD("soundcomm", "cpu", WRITELINE(atari_jsa_base_device, main_int_write_line))

	MCFG_YM2151_ADD("ym2151", JSA_MASTER_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("soundcomm", atari_sound_comm_device, ym2151_irq_gen))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(atari_jsa_base_device, ym2151_port_w))
	MCFG_MIXER_ROUTE(0, DEVICE_SELF_OWNER, 0.60, 0)
	MCFG_MIXER_ROUTE(1, DEVICE_SELF_OWNER, 0.60, 1)

	MCFG_SOUND_ADD("pokey", POKEY, JSA_MASTER_CLOCK/2)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.40, 0)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.40, 1)

	MCFG_SOUND_ADD("tms", TMS5220C, JSA_MASTER_CLOCK*2/11) // potentially JSA_MASTER_CLOCK/9 as well
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 0)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 1)
MACHINE_CONFIG_END


// Fully populated JSA-II
MACHINE_CONFIG_FRAGMENT( jsa_ii_config )

	// basic machine hardware
	MCFG_CPU_ADD("cpu", M6502, JSA_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(atarijsa2_map)
	MCFG_DEVICE_PERIODIC_INT_DEVICE("soundcomm", atari_sound_comm_device, sound_irq_gen, (double)JSA_MASTER_CLOCK/4/16/16/14)

	// sound hardware
	MCFG_ATARI_SOUND_COMM_ADD("soundcomm", "cpu", WRITELINE(atari_jsa_base_device, main_int_write_line))

	MCFG_YM2151_ADD("ym2151", JSA_MASTER_CLOCK)
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("soundcomm", atari_sound_comm_device, ym2151_irq_gen))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(atari_jsa_base_device, ym2151_port_w))
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.60, 0)

	MCFG_OKIM6295_ADD("oki1", JSA_MASTER_CLOCK/3, OKIM6295_PIN7_HIGH)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.75, 0)
MACHINE_CONFIG_END


// Fully populated JSA-III
MACHINE_CONFIG_DERIVED( jsa_iii_config, jsa_ii_config )

	// basic machine hardware
	MCFG_CPU_MODIFY("cpu")
	MCFG_CPU_PROGRAM_MAP(atarijsa3_map)

	MCFG_DEVICE_MODIFY("oki1")
	MCFG_DEVICE_ADDRESS_MAP(AS_0, jsa3_oki1_map)
MACHINE_CONFIG_END


// Fully populated JSA_IIIs
MACHINE_CONFIG_DERIVED( jsa_iiis_config, jsa_iii_config )

	MCFG_DEVICE_MODIFY("ym2151")
	MCFG_MIXER_ROUTE(1, DEVICE_SELF_OWNER, 0.60, 1)

	MCFG_OKIM6295_ADD("oki2", JSA_MASTER_CLOCK/3, OKIM6295_PIN7_HIGH)
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.75, 1)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, jsa3_oki2_map)
MACHINE_CONFIG_END



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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, sound_to_main_ready) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, main_to_sound_ready) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, atari_jsa_base_device, main_test_read_line) // self test
INPUT_PORTS_END

INPUT_PORTS_START( jsa_ii_ioports )
	PORT_START("JSAII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, sound_to_main_ready) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, main_to_sound_ready) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, atari_jsa_base_device, main_test_read_line) // self test
INPUT_PORTS_END

INPUT_PORTS_START( jsa_iii_ioports )
	PORT_START("JSAIII")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, atari_jsa_base_device, main_test_read_line) // self test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, sound_to_main_ready) // output buffer full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("soundcomm", atari_sound_comm_device, main_to_sound_ready) // input buffer full
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, atari_jsa_base_device, main_test_read_line) // self test
INPUT_PORTS_END



//**************************************************************************
//  BASE DEVICE CLASS
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_base_device - constructor
//-------------------------------------------------

atari_jsa_base_device::atari_jsa_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels)
	: device_t(mconfig, devtype, name, tag, owner, clock, shortname, __FILE__),
		device_mixer_interface(mconfig, *this, channels),
		m_soundcomm(*this, "soundcomm"),
		m_jsacpu(*this, "cpu"),
		m_ym2151(*this, "ym2151"),
		m_cpu_bank(*this, "cpubank"),
		m_test_read_cb(*this),
		m_main_int_cb(*this),
		m_ym2151_volume(1.0),
		m_ym2151_ct1(0),
		m_ym2151_ct2(0)
{
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_jsa_base_device::device_start()
{
	// configure CPU bank
	m_cpu_bank->configure_entries(0, 4, m_jsacpu->region()->base() + 0x10000, 0x1000);

	// resolve devices
	m_test_read_cb.resolve_safe(0);
	m_main_int_cb.resolve_safe();

	// save states
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

WRITE8_MEMBER( atari_jsa_base_device::main_command_w )
{
	m_soundcomm->main_command_w(space, offset, data);
}


//-------------------------------------------------
//  main_response_r: Handle response reads
//-------------------------------------------------

READ8_MEMBER( atari_jsa_base_device::main_response_r )
{
	return m_soundcomm->main_response_r(space, offset);
}


//-------------------------------------------------
//  sound_reset_w: Reset the sound board
//-------------------------------------------------

WRITE16_MEMBER( atari_jsa_base_device::sound_reset_w )
{
	m_soundcomm->sound_reset_w(space, offset, data);
}


//-------------------------------------------------
//  ym2151_port_w: Handle writes from the YM2151
//  output port
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_base_device::ym2151_port_w )
{
	m_ym2151_ct1 = (data >> 0) & 1;
	m_ym2151_ct2 = (data >> 1) & 1;
	update_all_volumes();
}


//-------------------------------------------------
//  main_test_read_line: Return the state of the
//  main's test line, provided by a callback
//-------------------------------------------------

READ_LINE_MEMBER(atari_jsa_base_device::main_test_read_line)
{
	return !m_test_read_cb();
}


//-------------------------------------------------
//  main_int_write_line: Forward interrupt signals
//  from the comm device to the owning callback
//-------------------------------------------------

WRITE_LINE_MEMBER( atari_jsa_base_device::main_int_write_line )
{
	m_main_int_cb(state);
}



//**************************************************************************
//  BASE DEVICE CLASS FOR OKI6295-BASED VERSIONS
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_oki_base_device: Constructor
//-------------------------------------------------

atari_jsa_oki_base_device::atari_jsa_oki_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels)
	: atari_jsa_base_device(mconfig, devtype, name, tag, owner, clock, shortname, channels),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
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

READ8_MEMBER( atari_jsa_oki_base_device::oki_r )
{
	// JSA IIIs selects the 2nd OKI via the low bit, so select it
	if (m_oki2 != NULL && offset == 1)
		return m_oki2->read(space, offset);

	// OKI may not be populated at all
	else if (m_oki1 != NULL)
		return m_oki1->read(space, offset);

	// if not present, return all 0xff
	return 0xff;
}


//-------------------------------------------------
//  oki_w: Handle writes to the OKI chip(s)
//  on the JSA II, III, and IIIs boards
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_oki_base_device::oki_w )
{
	// JSA IIIs selects the 2nd OKI via the low bit, so select it
	if (m_oki2 != NULL && offset == 1)
		m_oki2->write(space, offset, data);

	// OKI may not be populated at all
	else if (m_oki1 != NULL)
		m_oki1->write(space, offset, data);
}


//-------------------------------------------------
//  wrio_w: Handle writes to the general
//  I/O port on JSA II, III, and IIIs boards
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_oki_base_device::wrio_w )
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
	coin_counter_w(space.machine(), 1, (data >> 5) & 1);
	coin_counter_w(space.machine(), 0, (data >> 4) & 1);

	// update the OKI frequency
	if (m_oki1 != NULL)
	{
		m_oki1->set_pin7(data & 8);
		if ((data & 4) == 0)
			m_oki1->reset();
	}

	// same for the 2nd OKI (JSA IIIs only)
	if (m_oki2 != NULL)
	{
		m_oki2->set_pin7(data & 8);
		if ((data & 4) == 0)
			m_oki2->reset();
	}

	// update the (left) OKI bank (JSA III/IIIs only)
	if (m_oki1_banklo != NULL)
		m_oki1_banklo->set_entry((m_oki1_banklo->entry() & 2) | ((data >> 1) & 1));

	// reset the YM2151 if needed
	if ((data & 1) == 0)
		m_ym2151->reset();
}


//-------------------------------------------------
//  mix_w: Handle writes to the mixing
//  register on JSA II, III, and IIIs boards
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_oki_base_device::mix_w )
{
	//
	//  0xc0 = right OKI6295 bank bits 0-1 (JSA IIIs only)
	//  0x20 = low-pass filter enable
	//  0x10 = OKI6295 #1 bank bit 1
	//  0x0e = YM2151 volume (0-7)
	//  0x01 = OKI6295 volume (0-1)
	//

	// update the right OKI bank (JSA IIIs only)
	if (m_oki2_banklo != NULL)
		m_oki2_banklo->set_entry((data >> 6) & 3);

	// update the (left) OKI bank (JSA III/IIIs only)
	if (m_oki1_banklo != NULL)
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

WRITE8_MEMBER( atari_jsa_oki_base_device::overall_volume_w )
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
	if (m_oki1_banklo != NULL && m_oki1_bankhi != NULL && m_oki1->region()->bytes() >= 0x80000)
	{
		m_oki1_banklo->configure_entries(0, 2, m_oki1->region()->base() + 0x00000, 0x00000);
		m_oki1_banklo->configure_entries(2, 2, m_oki1->region()->base() + 0x20000, 0x20000);
		m_oki1_bankhi->set_base(m_oki1->region()->base() + 0x60000);
	}

	// configure JSA IIIs ADPCM banking
	if (m_oki2_banklo != NULL && m_oki2_bankhi != NULL && m_oki2->region()->bytes() >= 0x80000)
	{
		m_oki2_banklo->configure_entries(0, 2, m_oki2->region()->base() + 0x00000, 0x00000);
		m_oki2_banklo->configure_entries(2, 2, m_oki2->region()->base() + 0x20000, 0x20000);
		m_oki2_bankhi->set_base(m_oki1->region()->base() + 0x60000);
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
	if (m_oki1 != NULL)
		m_oki1->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_oki6295_volume * m_ym2151_ct1);
	if (m_oki2 != NULL)
		m_oki2->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_oki6295_volume * m_ym2151_ct1);
	m_ym2151->set_output_gain(ALL_OUTPUTS, m_overall_volume * m_ym2151_volume);
}



//**************************************************************************
//  JSA I-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_i_device: Constructor
//-------------------------------------------------

atari_jsa_i_device::atari_jsa_i_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atari_jsa_base_device(mconfig, ATARI_JSA_I, "Atari JSA I Sound Board", tag, owner, clock, "atjsa1", 2),
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

READ8_MEMBER( atari_jsa_i_device::rdio_r )
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

	UINT8 result = m_jsai->read();
	if (!m_test_read_cb())
		result ^= 0x80;
	if (m_tms5220 != NULL && m_tms5220->readyq_r() == 0)
		result |= 0x10;
	else
		result &= ~0x10;

	return result;
}


//-------------------------------------------------
//  wrio_w: Handle writes to the general I/O
//  port on a JSA I board
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_i_device::wrio_w )
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
	coin_counter_w(machine(), 1, (data >> 5) & 1);
	coin_counter_w(machine(), 0, (data >> 4) & 1);

	// handle TMS5220 I/O
	if (m_tms5220 != NULL)
	{
		int count = 5 | ((data >> 2) & 2);
		m_tms5220->set_frequency(JSA_MASTER_CLOCK*2 / (16 - count));
		m_tms5220->wsq_w((data >> 1) & 1);
		m_tms5220->rsq_w((data >> 2) & 1);
	}

	// reset the YM2151 if needed
	if ((data & 1) == 0)
		m_ym2151->reset();
}


//-------------------------------------------------
//  mix_w: Handle writes to the mixing register
//  on a JSA I board
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_i_device::mix_w )
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

WRITE8_MEMBER( atari_jsa_i_device::tms5220_voice )
{
	if (m_tms5220 != NULL)
		m_tms5220->data_w(space, 0, data);
}


//-------------------------------------------------
//  pokey_r: Handle reads from the POKEY if
//  present
//-------------------------------------------------

READ8_MEMBER( atari_jsa_i_device::pokey_r )
{
	if (m_pokey != NULL)
		return m_pokey->read(space, offset);
	return 0xff;
}


//-------------------------------------------------
//  pokey_w: Handle writes to the POKEY if
//  present
//-------------------------------------------------

WRITE8_MEMBER( atari_jsa_i_device::pokey_w )
{
	if (m_pokey != NULL)
		m_pokey->write(space, offset, data);
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor atari_jsa_i_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jsa_i_config );
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_i_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jsa_i_ioports );
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
	if (m_tms5220 != NULL)
		m_tms5220->set_output_gain(ALL_OUTPUTS, m_tms5220_volume * m_ym2151_ct1);
	if (m_pokey != NULL)
		m_pokey->set_output_gain(ALL_OUTPUTS, m_pokey_volume * m_ym2151_ct1);
	m_ym2151->set_output_gain(ALL_OUTPUTS, m_ym2151_volume);
}



//**************************************************************************
//  JSA II-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_ii_device: Constructor
//-------------------------------------------------

atari_jsa_ii_device::atari_jsa_ii_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atari_jsa_oki_base_device(mconfig, ATARI_JSA_II, "Atari JSA II Sound Board", tag, owner, clock, "atjsa2", 1)
	, m_jsaii(*this, "JSAII")
{
}


//-------------------------------------------------
//  rdio_r: Handle reads from the general I/O
//  port on a JSA II board
//-------------------------------------------------

READ8_MEMBER( atari_jsa_ii_device::rdio_r )
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

	UINT8 result = m_jsaii->read();
	if (!m_test_read_cb())
		result ^= 0x80;

	return result;
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor atari_jsa_ii_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jsa_ii_config );
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_ii_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jsa_ii_ioports );
}



//**************************************************************************
//  JSA III-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_iii_device: Constructor
//-------------------------------------------------

atari_jsa_iii_device::atari_jsa_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atari_jsa_oki_base_device(mconfig, ATARI_JSA_III, "Atari JSA III Sound Board", tag, owner, clock, "atjsa3", 1)
	, m_jsaiii(*this, "JSAIII")
{
}

atari_jsa_iii_device::atari_jsa_iii_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels)
	: atari_jsa_oki_base_device(mconfig, devtype, name, tag, owner, clock, shortname, channels)
	, m_jsaiii(*this, "JSAIII")
{
}


//-------------------------------------------------
//  jsa_iii_rdio: Handle reads from the general I/O
//  port on a JSA III/IIIs board
//-------------------------------------------------

READ8_MEMBER( atari_jsa_iii_device::rdio_r )
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

	UINT8 result = m_jsaiii->read();
	if (!m_test_read_cb())
		result ^= 0x90;
	return result;
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor atari_jsa_iii_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jsa_iii_config );
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor atari_jsa_iii_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jsa_iii_ioports );
}



//**************************************************************************
//  JSA IIIS-SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  atari_jsa_iiis_device: Constructor
//-------------------------------------------------

atari_jsa_iiis_device::atari_jsa_iiis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atari_jsa_iii_device(mconfig, ATARI_JSA_IIIS, "Atari JSA IIIs Sound Board", tag, owner, clock, "atjsa3s", 2)
{
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor atari_jsa_iiis_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jsa_iiis_config );
}
