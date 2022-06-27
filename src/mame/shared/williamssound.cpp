// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    williams.h

    Functions to emulate general the various Williams/Midway sound cards.

****************************************************************************

    Midway/Williams Audio Boards
    ----------------------------

    6809 MEMORY MAP

    Function                                  Address     R/W  Data
    ---------------------------------------------------------------
    Program RAM                               0000-07FF   R/W  D0-D7

    Music (YM-2151)                           2000-2001   R/W  D0-D7

    6821 PIA                                  4000-4003   R/W  D0-D7

    HC55516 clock low, digit latch            6000        W    D0
    HC55516 clock high                        6800        W    xx

    Bank select                               7800        W    D0-D2

    Banked Program ROM                        8000-FFFF   R    D0-D7

****************************************************************************/

#include "emu.h"
#include "williamssound.h"

#include "machine/6821pia.h"
#include "machine/rescap.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"


#define NARC_MASTER_CLOCK       XTAL(8'000'000)
#define NARC_FM_CLOCK           XTAL(3'579'545)

#define CVSD_MASTER_CLOCK       XTAL(8'000'000)
#define CVSD_FM_CLOCK           XTAL(3'579'545)

#define ADPCM_MASTER_CLOCK      XTAL(8'000'000)
#define ADPCM_FM_CLOCK          XTAL(3'579'545)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(WILLIAMS_CVSD_SOUND, williams_cvsd_sound_device, "wmscvsd", "Williams CVSD Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_NARC_SOUND, williams_narc_sound_device, "wmsnarc", "Williams NARC Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_ADPCM_SOUND, williams_adpcm_sound_device, "wmsadpcm", "Williams ADPCM Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_S4_SOUND, williams_s4_sound_device, "wmss4", "Williams System 4 Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_S6_SOUND, williams_s6_sound_device, "wmss6", "Williams System 6 Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_S9_SOUND, williams_s9_sound_device, "wmss9", "Williams System 9 Sound Board")
DEFINE_DEVICE_TYPE(WILLIAMS_S11_SOUND, williams_s11_sound_device, "wmss11", "Williams System 11 Sound Board")



//**************************************************************************
//  CVSD SOUND BOARD
//**************************************************************************

//-------------------------------------------------
//  williams_cvsd_sound_device - constructor
//-------------------------------------------------

williams_cvsd_sound_device::williams_cvsd_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_CVSD_SOUND, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia(*this, "pia"),
		m_ym2151(*this, "ym2151"),
		m_hc55516(*this, "cvsd"),
		m_rombank(*this, "rombank"),
		m_talkback(0)
{
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

void williams_cvsd_sound_device::write(u16 data)
{
	m_sync_write_timer->adjust(attotime::zero, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(williams_cvsd_sound_device::reset_write)
{
	// going high halts the CPU
	if (state)
	{
		bank_select_w(0);
		device_reset();
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	// going low resets and reactivates the CPU
	else
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//-------------------------------------------------
//  bank_select_w - change memory banks
//-------------------------------------------------

void williams_cvsd_sound_device::bank_select_w(u8 data)
{
	m_rombank->set_entry(data & 0x0f);
}


//-------------------------------------------------
//  talkback_w - write to the talkback latch
//-------------------------------------------------

void williams_cvsd_sound_device::talkback_w(u8 data)
{
	m_talkback = data;
	logerror("CVSD Talkback = %02X\n", data);
}


//-------------------------------------------------
//  cvsd_digit_clock_clear_w - clear the clock on
//  the HC55516 and clock the data
//-------------------------------------------------

void williams_cvsd_sound_device::cvsd_digit_clock_clear_w(u8 data)
{
	m_hc55516->clock_w(0);
	m_hc55516->digit_w(data&1);
}


//-------------------------------------------------
//  cvsd_clock_set_w - set the clock on the HC55516
//-------------------------------------------------

void williams_cvsd_sound_device::cvsd_clock_set_w(u8 data)
{
	m_hc55516->clock_w(1);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

void williams_cvsd_sound_device::williams_cvsd_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw("ym2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6000).mirror(0x07ff).w(FUNC(williams_cvsd_sound_device::cvsd_digit_clock_clear_w));
	map(0x6800, 0x6800).mirror(0x07ff).w(FUNC(williams_cvsd_sound_device::cvsd_clock_set_w));
	map(0x7800, 0x7800).mirror(0x07ff).w(FUNC(williams_cvsd_sound_device::bank_select_w));
	map(0x8000, 0xffff).bankr("rombank");
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void williams_cvsd_sound_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, CVSD_MASTER_CLOCK / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &williams_cvsd_sound_device::williams_cvsd_map);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia->writepb_handler().set(FUNC(williams_cvsd_sound_device::talkback_w));
	m_pia->ca2_handler().set(m_ym2151, FUNC(ym2151_device::reset_w));
	m_pia->irqa_handler().set_inputline(m_cpu, M6809_FIRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, INPUT_LINE_NMI);

	YM2151(config, m_ym2151, CVSD_FM_CLOCK);
	m_ym2151->irq_handler().set(m_pia, FUNC(pia6821_device::ca1_w)).invert(); // IRQ is not true state
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.10);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);

	HC55516(config, m_hc55516, 0);
	m_hc55516->add_route(ALL_OUTPUTS, *this, 0.60);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void williams_cvsd_sound_device::device_start()
{
	// configure master CPU banks
	u8 *rom = memregion("cpu")->base();
	for (int bank = 0; bank < 16; bank++)
	{
		//
		//  D0/D1 -> selects: 0=U4 1=U19 2=U20 3=n/c
		//  D2 -> A15
		//  D3 -> A16
		//
		offs_t offset = 0x8000 * ((bank >> 2) & 3) + 0x20000 * (bank & 3);
		m_rombank->configure_entry(bank, &rom[0x10000 + offset]);
	}
	m_rombank->set_entry(0);

	// reset the IRQ state
	m_pia->ca1_w(1);

	// register for save states
	save_item(NAME(m_talkback));

	// allocate timers
	m_sync_write_timer = timer_alloc(FUNC(williams_cvsd_sound_device::sync_write), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void williams_cvsd_sound_device::device_reset()
{
	// reset interrupt states
	m_cpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


//-------------------------------------------------
//  sync_write
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(williams_cvsd_sound_device::sync_write)
{
	// process incoming data write
	m_pia->portb_w(param & 0xff);
	m_pia->cb1_w((param >> 8) & 1);
	m_pia->cb2_w((param >> 9) & 1);
}



//**************************************************************************
//  NARC SOUND BOARD
//**************************************************************************

//-------------------------------------------------
//  williams_narc_sound_device - constructor
//-------------------------------------------------

williams_narc_sound_device::williams_narc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_NARC_SOUND, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu%u", 0U),
		m_hc55516(*this, "cvsd"),
		m_masterbank(*this, "masterbank"),
		m_slavebank(*this, "slavebank"),
		m_latch(0),
		m_latch2(0),
		m_talkback(0),
		m_audio_sync(0),
		m_sound_int_state(0)
{
}


//-------------------------------------------------
//  read - return the talkback register with the
//  SYNC bits in bits 8 and 9
//-------------------------------------------------

u16 williams_narc_sound_device::read()
{
	return m_talkback | (m_audio_sync << 8);
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

void williams_narc_sound_device::write(u16 data)
{
	m_sync_master_timer->adjust(attotime::zero, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(williams_narc_sound_device::reset_write)
{
	// going high halts the CPU
	if (state)
	{
		master_bank_select_w(0);
		slave_bank_select_w(0);
		device_reset();
		m_cpu[0]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_cpu[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	// going low resets and reactivates the CPU
	else
	{
		m_cpu[0]->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_cpu[1]->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  master_bank_select_w - select the bank for the
//  master CPU
//-------------------------------------------------

void williams_narc_sound_device::master_bank_select_w(u8 data)
{
	m_masterbank->set_entry(data & 0x0f);
}


//-------------------------------------------------
//  slave_bank_select_w - select the bank for the
//  slave CPU
//-------------------------------------------------

void williams_narc_sound_device::slave_bank_select_w(u8 data)
{
	m_slavebank->set_entry(data & 0x0f);
}


//-------------------------------------------------
//  command_r - read command written by external
//  agent
//-------------------------------------------------

u8 williams_narc_sound_device::command_r()
{
	m_cpu[0]->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_sound_int_state = 0;
	return m_latch;
}


//-------------------------------------------------
//  command2_w - write command from master CPU to
//  slave CPU
//-------------------------------------------------

void williams_narc_sound_device::command2_w(u8 data)
{
	m_sync_slave_timer->adjust(attotime::zero, data);
}


//-------------------------------------------------
//  command2_r - read command written by master
//  CPU
//-------------------------------------------------

u8 williams_narc_sound_device::command2_r()
{
	m_cpu[1]->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return m_latch2;
}


//-------------------------------------------------
//  master_talkback_w - handle writes to the
//  talkback latch from the master CPU
//-------------------------------------------------

void williams_narc_sound_device::master_talkback_w(u8 data)
{
	m_talkback = data;
	logerror("Master Talkback = %02X\n", data);
}


//-------------------------------------------------
//  master_sync_w - handle writes to the master
//  SYNC register
//-------------------------------------------------

void williams_narc_sound_device::master_sync_w(u8 data)
{
	m_sync_clear_timer->adjust(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), 0x01);
	m_audio_sync |= 0x01;
	logerror("Master sync = %02X\n", data);
}


//-------------------------------------------------
//  slave_talkback_w - handle writes to the
//  talkback latch from the slave CPU
//-------------------------------------------------

void williams_narc_sound_device::slave_talkback_w(u8 data)
{
	logerror("Slave Talkback = %02X\n", data);
}


//-------------------------------------------------
//  slave_sync_w - handle writes to the slave
//  SYNC register
//-------------------------------------------------

void williams_narc_sound_device::slave_sync_w(u8 data)
{
	m_sync_clear_timer->adjust(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), 0x02);
	m_audio_sync |= 0x02;
	logerror("Slave sync = %02X\n", data);
}


//-------------------------------------------------
//  cvsd_digit_clock_clear_w - clear the clk pin on
//  the HC555xx and clock the data latch
//-------------------------------------------------

void williams_narc_sound_device::cvsd_digit_clock_clear_w(u8 data)
{
	m_hc55516->clock_w(0);
	m_hc55516->digit_w(data&1);
}


//---------------------------------------------------
//  cvsd_clock_set_w - set the clk pin on the HC555xx
//---------------------------------------------------

void williams_narc_sound_device::cvsd_clock_set_w(u8 data)
{
	m_hc55516->clock_w(1);
}


//-------------------------------------------------
//  master CPU map
//-------------------------------------------------

void williams_narc_sound_device::williams_narc_master_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).mirror(0x03fe).rw("ym2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x03ff).w(FUNC(williams_narc_sound_device::master_talkback_w));
	map(0x2c00, 0x2c00).mirror(0x03ff).w(FUNC(williams_narc_sound_device::command2_w));
	map(0x3000, 0x3000).mirror(0x03ff).w("dac1", FUNC(dac_byte_interface::data_w));
	map(0x3400, 0x3400).mirror(0x03ff).r(FUNC(williams_narc_sound_device::command_r));
	map(0x3800, 0x3800).mirror(0x03ff).w(FUNC(williams_narc_sound_device::master_bank_select_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(williams_narc_sound_device::master_sync_w));
	map(0x4000, 0xbfff).bankr("masterbank");
	map(0xc000, 0xffff).bankr("masterupper");
}


//-------------------------------------------------
//  slave CPU map
//-------------------------------------------------

void williams_narc_sound_device::williams_narc_slave_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x03ff).w(FUNC(williams_narc_sound_device::cvsd_clock_set_w));
	map(0x2400, 0x2400).mirror(0x03ff).w(FUNC(williams_narc_sound_device::cvsd_digit_clock_clear_w));
	map(0x2800, 0x2800).mirror(0x03ff).w(FUNC(williams_narc_sound_device::slave_talkback_w));
	map(0x3000, 0x3000).mirror(0x03ff).w("dac2", FUNC(dac_byte_interface::data_w));
	map(0x3400, 0x3400).mirror(0x03ff).r(FUNC(williams_narc_sound_device::command2_r));
	map(0x3800, 0x3800).mirror(0x03ff).w(FUNC(williams_narc_sound_device::slave_bank_select_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(williams_narc_sound_device::slave_sync_w));
	map(0x4000, 0xbfff).bankr("slavebank");
	map(0xc000, 0xffff).bankr("slaveupper");
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------


void williams_narc_sound_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu[0], NARC_MASTER_CLOCK / 4);
	m_cpu[0]->set_addrmap(AS_PROGRAM, &williams_narc_sound_device::williams_narc_master_map);

	MC6809E(config, m_cpu[1], NARC_MASTER_CLOCK / 4);
	m_cpu[1]->set_addrmap(AS_PROGRAM, &williams_narc_sound_device::williams_narc_slave_map);

	ym2151_device &ym2151(YM2151(config, "ym2151", NARC_FM_CLOCK));
	ym2151.irq_handler().set_inputline("cpu0", M6809_FIRQ_LINE);
	ym2151.add_route(ALL_OUTPUTS, *this, 0.10);

	AD7224(config, "dac1", 0).add_route(ALL_OUTPUTS, *this, 0.25);
	AD7224(config, "dac2", 0).add_route(ALL_OUTPUTS, *this, 0.25);

	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, *this, 0.60);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void williams_narc_sound_device::device_start()
{
	// configure master CPU banks
	u8 *rom = memregion("cpu0")->base();
	for (int bank = 0; bank < 16; bank++)
	{
		//
		//  D0 -> A15
		//  D1/D2 -> selects: 0=n/c 1=U3 2=U4 3=U5
		//  D3 -> A16
		//
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		m_masterbank->configure_entry(bank, &rom[0x10000 + offset]);
	}
	membank("masterupper")->set_base(&rom[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	// configure slave CPU banks
	rom = memregion("cpu1")->base();
	for (int bank = 0; bank < 16; bank++)
	{
		//
		//  D0 -> A15
		//  D1/D2 -> selects: 0=U35 1=U36 2=U37 3=U38
		//  D3 -> A16
		//
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		m_slavebank->configure_entry(bank, &rom[0x10000 + offset]);
	}
	membank("slaveupper")->set_base(&rom[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	// register for save states
	save_item(NAME(m_latch));
	save_item(NAME(m_latch2));
	save_item(NAME(m_talkback));
	save_item(NAME(m_audio_sync));
	save_item(NAME(m_sound_int_state));

	// allocate timers
	m_sync_master_timer = timer_alloc(FUNC(williams_narc_sound_device::sync_master_command), this);
	m_sync_slave_timer = timer_alloc(FUNC(williams_narc_sound_device::sync_slave_command), this);
	m_sync_clear_timer = timer_alloc(FUNC(williams_narc_sound_device::sync_clear), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void williams_narc_sound_device::device_reset()
{
	// reset interrupt states
	m_sound_int_state = 0;
	m_cpu[0]->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_cpu[0]->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_cpu[0]->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_cpu[1]->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_cpu[1]->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_cpu[1]->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


//-------------------------------------------------
//  timer callbacks
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(williams_narc_sound_device::sync_master_command)
{
	m_latch = param & 0xff;
	m_cpu[0]->set_input_line(INPUT_LINE_NMI, (param & 0x100) ? CLEAR_LINE : ASSERT_LINE);
	if ((param & 0x200) == 0)
	{
		m_cpu[0]->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_sound_int_state = 1;
	}
}

TIMER_CALLBACK_MEMBER(williams_narc_sound_device::sync_slave_command)
{
	m_latch2 = param & 0xff;
	m_cpu[1]->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(williams_narc_sound_device::sync_clear)
{
	m_audio_sync &= ~param;
}


//**************************************************************************
//  ADPCM SOUND BOARD
//**************************************************************************

//-------------------------------------------------
//  williams_adpcm_sound_device - constructor
//-------------------------------------------------

williams_adpcm_sound_device::williams_adpcm_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_ADPCM_SOUND, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_rombank(*this, "rombank"),
		m_okibank(*this, "okibank"),
		m_latch(0),
		m_talkback(0),
		m_sound_int_state(0)
{
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

void williams_adpcm_sound_device::write(u16 data)
{
	m_sync_command_timer->adjust(attotime::zero, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(williams_adpcm_sound_device::reset_write)
{
	// going high halts the CPU
	if (state)
	{
		bank_select_w(0);
		device_reset();
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	// going low resets and reactivates the CPU
	else
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//-------------------------------------------------
//  irq_read - read the sound IRQ state
//-------------------------------------------------

READ_LINE_MEMBER(williams_adpcm_sound_device::irq_read)
{
	return m_sound_int_state;
}


//-------------------------------------------------
//  bank_select_w - select the sound CPU memory
//  bank
//-------------------------------------------------

void williams_adpcm_sound_device::bank_select_w(u8 data)
{
	m_rombank->set_entry(data & 0x07);
}


//-------------------------------------------------
//  oki6295_bank_select_w - select the OKI6295
//  memory bank
//-------------------------------------------------

void williams_adpcm_sound_device::oki6295_bank_select_w(u8 data)
{
	m_okibank->set_entry(data & 7);
}


//-------------------------------------------------
//  command_r - read the command from the external
//  latch
//-------------------------------------------------

u8 williams_adpcm_sound_device::command_r()
{
	m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);

	// don't clear the external IRQ state for a short while; this allows the
	// self-tests to pass
	m_irq_clear_timer->adjust(attotime::from_usec(10));
	return m_latch;
}


//-------------------------------------------------
//  talkback_w - write to the talkback latch
//-------------------------------------------------

void williams_adpcm_sound_device::talkback_w(u8 data)
{
	m_talkback = data;
	logerror("ADPCM Talkback = %02X\n", data);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

void williams_adpcm_sound_device::williams_adpcm_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x03ff).w(FUNC(williams_adpcm_sound_device::bank_select_w));
	map(0x2400, 0x2401).mirror(0x03fe).rw("ym2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x03ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2c00, 0x2c00).mirror(0x03ff).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x3000, 0x3000).mirror(0x03ff).r(FUNC(williams_adpcm_sound_device::command_r));
	map(0x3400, 0x3400).mirror(0x03ff).w(FUNC(williams_adpcm_sound_device::oki6295_bank_select_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(williams_adpcm_sound_device::talkback_w));
	map(0x4000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).bankr("romupper");
}


//-------------------------------------------------
//  OKI6295 map
//-------------------------------------------------

void williams_adpcm_sound_device::williams_adpcm_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("okibank");
	map(0x20000, 0x3ffff).rom().region("oki", 0x60000);
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void williams_adpcm_sound_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, ADPCM_MASTER_CLOCK / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &williams_adpcm_sound_device::williams_adpcm_map);

	ym2151_device &ym2151(YM2151(config, "ym2151", ADPCM_FM_CLOCK));
	ym2151.irq_handler().set_inputline("cpu", M6809_FIRQ_LINE);
	ym2151.add_route(ALL_OUTPUTS, *this, 0.10);

	AD7524(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.10);

	okim6295_device &oki(OKIM6295(config, "oki", ADPCM_MASTER_CLOCK/8, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &williams_adpcm_sound_device::williams_adpcm_oki_map);
	oki.add_route(ALL_OUTPUTS, *this, 0.15);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void williams_adpcm_sound_device::device_start()
{
	// configure banks
	u8 *rom = memregion("cpu")->base();
	m_rombank->configure_entries(0, 8, &rom[0x10000], 0x8000);
	membank("romupper")->set_base(&rom[0x10000 + 0x4000 + 7 * 0x8000]);

	// expand ADPCM data
	rom = memregion("oki")->base();
	// it is assumed that U12 is loaded @ 0x00000 and U13 is loaded @ 0x40000
	m_okibank->configure_entry(0, &rom[0x40000]);
	m_okibank->configure_entry(1, &rom[0x40000]);
	m_okibank->configure_entry(2, &rom[0x20000]);
	m_okibank->configure_entry(3, &rom[0x00000]);
	m_okibank->configure_entry(4, &rom[0xe0000]);
	m_okibank->configure_entry(5, &rom[0xc0000]);
	m_okibank->configure_entry(6, &rom[0xa0000]);
	m_okibank->configure_entry(7, &rom[0x80000]);

	// register for save states
	save_item(NAME(m_latch));
	save_item(NAME(m_talkback));
	save_item(NAME(m_sound_int_state));

	m_sync_command_timer = timer_alloc(FUNC(williams_adpcm_sound_device::sync_command), this);
	m_irq_clear_timer = timer_alloc(FUNC(williams_adpcm_sound_device::irq_clear), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void williams_adpcm_sound_device::device_reset()
{
	// reset interrupt states
	m_sound_int_state = 0;
	m_cpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_sync_command_timer->adjust(attotime::never);
	m_irq_clear_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  timer callbacks
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(williams_adpcm_sound_device::sync_command)
{
	m_latch = param & 0xff;
	if (!(param & 0x200))
	{
		m_cpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_sound_int_state = 1;
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
	}
}

TIMER_CALLBACK_MEMBER(williams_adpcm_sound_device::irq_clear)
{
	m_sound_int_state = 0;
}



//**************************************************************************
//  S4 SOUND BOARD (simple sound card used in system 3/4 pinballs)
//**************************************************************************

//-------------------------------------------------
//  williams_s4_sound_device - constructor
//-------------------------------------------------
williams_s4_sound_device::williams_s4_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_S4_SOUND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pia(*this, "pia")
{
}

//-------------------------------------------------
//  write - handle an external write
//-------------------------------------------------
void williams_s4_sound_device::write(u8 data)
{
	// Handle S2 (electronic or tones)
	data &= ioport("S4")->read();
	if ((data & 0x9f) != 0x9f)
	{
		m_pia->portb_w(data);
		m_pia->cb1_w(0);
	}
	m_pia->cb1_w(1);
}

//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------
void williams_s4_sound_device::williams_s4_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x00ff).ram();
	map(0x0400, 0x0403).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0800, 0x0fff).rom().region("audiocpu", 0);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
void williams_s4_sound_device::device_add_mconfig(machine_config &config)
{
	M6808(config, m_cpu, 3580000);
	m_cpu->set_addrmap(AS_PROGRAM, &williams_s4_sound_device::williams_s4_map);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.5);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia->irqa_handler().set_inputline(m_cpu, M6808_IRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, M6808_IRQ_LINE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void williams_s4_sound_device::device_start()
{
	// register for save states
	save_item(NAME(m_dummy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void williams_s4_sound_device::device_reset()
{
	// reset interrupt states
	m_cpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
}

INPUT_PORTS_START( williams_s4 )
	PORT_START("S4")
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x40, 0x00, "Sounds" )
	PORT_DIPSETTING(    0x00, "Set 1" )
	PORT_DIPSETTING(    0x40, "Set 2" )
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, williams_s4_sound_device, audio_nmi, 1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( williams_s4_sound_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------
ioport_constructor williams_s4_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( williams_s4 );
}



//**************************************************************************
//  S6 SOUND BOARD (s4 with speech, used in system 6/6a/7 pinballs)
//**************************************************************************

//-------------------------------------------------
//  williams_s6_sound_device - constructor
//-------------------------------------------------
williams_s6_sound_device::williams_s6_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_S6_SOUND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pia(*this, "pia")
	, m_hc(*this, "hc")
{
}

//-------------------------------------------------
//  write - handle an external write
//-------------------------------------------------
void williams_s6_sound_device::write(u8 data)
{
	data = bitswap<8>(data, 6, 7, 5, 4, 3, 2, 1, 0) | 0x40;
	// Handle dips
	data &= ioport("S6")->read();
	if ((data & 0x9f) != 0x9f)
	{
		m_pia->portb_w(data);
		m_pia->cb1_w(0);
	}
	m_pia->cb1_w(1);
}

//-------------------------------------------------
//  pb_w - acknowledge interrupt
//-------------------------------------------------
void williams_s6_sound_device::pb_w(u8 data)
{
	m_pia->cb1_w(1);
}

//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------
void williams_s6_sound_device::williams_s6_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram();
	map(0x0400, 0x0403).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3000, 0x7fff).rom().region("audiocpu", 0);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
void williams_s6_sound_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, 3580000);
	m_cpu->set_addrmap(AS_PROGRAM, &williams_s6_sound_device::williams_s6_map);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.5);

	HC55516(config, m_hc, 0).add_route(ALL_OUTPUTS, *this, 1.00);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia->writepb_handler().set(FUNC(williams_s6_sound_device::pb_w));
	m_pia->ca2_handler().set(m_hc, FUNC(hc55516_device::digit_w));
	m_pia->cb2_handler().set(m_hc, FUNC(hc55516_device::clock_w));
	m_pia->irqa_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void williams_s6_sound_device::device_start()
{
	// register for save states
	save_item(NAME(m_dummy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void williams_s6_sound_device::device_reset()
{
	// reset interrupt states
	m_cpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
}

INPUT_PORTS_START( williams_s6 )
	PORT_START("S6")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x20, 0x00, "Speech" )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Sounds" )
	PORT_DIPSETTING(    0x00, "Tones" )
	PORT_DIPSETTING(    0x40, "Synth" )
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, williams_s6_sound_device, audio_nmi, 1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( williams_s6_sound_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------
ioport_constructor williams_s6_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( williams_s6 );
}



//**************************************************************************
//  S9 SOUND BOARD (s6 with different interface, used in system 9 pinballs)
//**************************************************************************

//-------------------------------------------------
//  williams_s9_sound_device - constructor
//-------------------------------------------------
williams_s9_sound_device::williams_s9_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_S9_SOUND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pia(*this, "pia")
	, m_hc(*this, "hc")
{
}

//-------------------------------------------------
//  write - handle an external write
//-------------------------------------------------
void williams_s9_sound_device::write(u8 data)
{
	m_pia->porta_w(data);
}

//-------------------------------------------------
//  strobe - tell PIA to process the input
//-------------------------------------------------
WRITE_LINE_MEMBER(williams_s9_sound_device::strobe)
{
	m_pia->ca1_w(state);
}

//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------
void williams_s9_sound_device::williams_s9_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2003).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom().region("audiocpu", 0 );
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
void williams_s9_sound_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, XTAL(4'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &williams_s9_sound_device::williams_s9_map);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.5);

	HC55516(config, m_hc, 0).add_route(ALL_OUTPUTS, *this, 1.00);

	PIA6821(config, m_pia, 0);
	m_pia->set_port_a_input_overrides_output_mask(0xff);
	m_pia->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia->ca2_handler().set(m_hc, FUNC(hc55516_device::clock_w));
	m_pia->cb2_handler().set(m_hc, FUNC(hc55516_device::digit_w));
	m_pia->irqa_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void williams_s9_sound_device::device_start()
{
	// register for save states
	save_item(NAME(m_dummy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void williams_s9_sound_device::device_reset()
{
	// reset interrupt states
	m_cpu->set_input_line(M6802_IRQ_LINE, CLEAR_LINE);
}

INPUT_PORTS_START( williams_s9 )
	PORT_START("S9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, williams_s9_sound_device, audio_nmi, 1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( williams_s9_sound_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------
ioport_constructor williams_s9_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( williams_s9 );
}



//**************************************************************************
//  S11 SOUND BOARD (s9 with banked roms, used in system 11/a/b/c pinballs)
//**************************************************************************

//-------------------------------------------------
//  williams_s11_sound_device - constructor
//-------------------------------------------------
williams_s11_sound_device::williams_s11_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WILLIAMS_S11_SOUND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pia(*this, "pia")
	, m_hc(*this, "hc")
{
}

//-------------------------------------------------
//  write - handle an external write
//-------------------------------------------------
void williams_s11_sound_device::write(u8 data)
{
	m_pia->porta_w(data);
}

//-------------------------------------------------
//  strobe - tell PIA to process the input
//-------------------------------------------------
WRITE_LINE_MEMBER(williams_s11_sound_device::strobe)
{
	m_pia->ca1_w(state);
}

//-------------------------------------------------
//  bank_w - bankswitch
//-------------------------------------------------
void williams_s11_sound_device::bank_w(u8 data)
{
	membank("bank0")->set_entry(BIT(data, 1));
	membank("bank1")->set_entry(BIT(data, 0));
}

//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------
void williams_s11_sound_device::williams_s11_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x1000, 0x1fff).w(FUNC(williams_s11_sound_device::bank_w));
	map(0x2000, 0x2003).mirror(0x0ffc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xbfff).bankr("bank0");
	map(0xc000, 0xffff).bankr("bank1");
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
void williams_s11_sound_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, XTAL(4'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &williams_s11_sound_device::williams_s11_map);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.5);

	HC55516(config, m_hc, 0).add_route(ALL_OUTPUTS, *this, 1.00);

	PIA6821(config, m_pia, 0);
	m_pia->set_port_a_input_overrides_output_mask(0xff);
	m_pia->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia->ca2_handler().set(m_hc, FUNC(hc55516_device::clock_w));
	m_pia->cb2_handler().set(m_hc, FUNC(hc55516_device::digit_w));
	m_pia->irqa_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, M6802_IRQ_LINE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void williams_s11_sound_device::device_start()
{
	u8 *ROM = memregion("audiocpu")->base();
	membank("bank0")->configure_entries(0, 2, &ROM[0x0000], 0x4000);
	membank("bank1")->configure_entries(0, 2, &ROM[0x8000], 0x4000);

	// register for save states
	save_item(NAME(m_dummy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void williams_s11_sound_device::device_reset()
{
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);

	// reset interrupt states
	m_cpu->set_input_line(M6802_IRQ_LINE, CLEAR_LINE);
}

INPUT_PORTS_START( williams_s11 )
	PORT_START("S11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, williams_s11_sound_device, audio_nmi, 1)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( williams_s11_sound_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------
ioport_constructor williams_s11_sound_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( williams_s11 );
}


