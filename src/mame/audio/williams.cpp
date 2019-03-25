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
#include "williams.h"
#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "sound/hc55516.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"


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
	synchronize(0, data);
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
	m_hc55516->digit_w(data);
	m_hc55516->clock_w(0);
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
	m_pia->irqa_handler().set_inputline(m_cpu, M6809_FIRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_cpu, INPUT_LINE_NMI);

	ym2151_device &ym(YM2151(config, "ym2151", CVSD_FM_CLOCK));
	ym.irq_handler().set(m_pia, FUNC(pia6821_device::ca1_w)).invert(); // IRQ is not true state
	ym.add_route(ALL_OUTPUTS, *this, 0.10);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

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
//  device_timer - timer callbacks
//-------------------------------------------------

void williams_cvsd_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// process incoming data write
	m_pia->write_portb(param & 0xff);
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
	synchronize(TID_MASTER_COMMAND, data);
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
	synchronize(TID_SLAVE_COMMAND, data);
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
	timer_set(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), TID_SYNC_CLEAR, 0x01);
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
	timer_set(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), TID_SYNC_CLEAR, 0x02);
	m_audio_sync |= 0x02;
	logerror("Slave sync = %02X\n", data);
}


//-------------------------------------------------
//  cvsd_digit_clock_clear_w - clear the clock on
//  the HC55516 and clock the data
//-------------------------------------------------

void williams_narc_sound_device::cvsd_digit_clock_clear_w(u8 data)
{
	m_hc55516->digit_w(data);
	m_hc55516->clock_w(0);
}


//-------------------------------------------------
//  cvsd_clock_set_w - set the clock on the HC55516
//-------------------------------------------------

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
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac1", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac1", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac2", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac2", -1.0, DAC_VREF_NEG_INPUT);

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
//  device_timer - timer callbacks
//-------------------------------------------------

void williams_narc_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_MASTER_COMMAND:
			m_latch = param & 0xff;
			m_cpu[0]->set_input_line(INPUT_LINE_NMI, (param & 0x100) ? CLEAR_LINE : ASSERT_LINE);
			if ((param & 0x200) == 0)
			{
				m_cpu[0]->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
				m_sound_int_state = 1;
			}
			break;

		case TID_SLAVE_COMMAND:
			m_latch2 = param & 0xff;
			m_cpu[1]->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
			break;

		case TID_SYNC_CLEAR:
			m_audio_sync &= ~param;
			break;
	}
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
	synchronize(TID_COMMAND, data);
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
	timer_set(attotime::from_usec(10), TID_IRQ_CLEAR);
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
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

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
}


//-------------------------------------------------
//  device_timer - timer callbacks
//-------------------------------------------------

void williams_adpcm_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_COMMAND:
			m_latch = param & 0xff;
			if (!(param & 0x200))
			{
				m_cpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
				m_sound_int_state = 1;
				machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
			}
			break;

		case TID_IRQ_CLEAR:
			m_sound_int_state = 0;
			break;
	}
}
