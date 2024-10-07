// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

    Atari Tomcat prototype hardware

    Driver by Mariusz Wojcieszek

    Notes:
    - game has no sound, while sound hardware was developed, sound program was
      not prepared

    TODO:
    - add proper timing of interrupts and framerate (currently commented out,
      as they cause test mode to hang)
    - vector quality appears to be worse than original game (compared to original
      game videos)
    - verify controls
    - implement game linking (after MAME supports network)
    - current implementation of 68010 <-> tms32010 is a little bit hacky, after
      tms32010 is started by 68010, 68010 is suspended until tms32010 reads command
      and starts executing
    - hook up tms5220 - it is currently not used at all

*/

#include "emu.h"

#include "cpu/m68000/m68010.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/m6502/m6502.h"
#include "video/avgdvg.h"
#include "video/vector.h"
#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/mos6530.h"
#include "machine/timekpr.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/ymopm.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tomcat_state : public driver_device
{
public:
	tomcat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tms(*this, "tms")
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_adc(*this, "adc")
		, m_mainlatch(*this, "mainlatch")
	{ }

	void tomcat(machine_config &config);

private:
	void adcon_w(uint8_t data);
	uint16_t tomcat_inputs_r(offs_t offset, uint16_t mem_mask = ~0);
	void main_latch_w(offs_t offset, uint16_t data);
	void lnkmode_w(int state);
	void err_w(int state);
	void ack_w(int state);
	void txbuff_w(int state);
	void sndres_w(int state);
	void mres_w(int state);
	void tomcat_irqclr_w(uint16_t data);
	uint16_t tomcat_inputs2_r();
	uint16_t tomcat_320bio_r();
	uint8_t tomcat_nvram_r(offs_t offset);
	void tomcat_nvram_w(offs_t offset, uint8_t data);
	int dsp_bio_r();
	void soundlatches_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void tomcat_map(address_map &map) ATTR_COLD;

	required_device<tms5220_device> m_tms;
	required_shared_ptr<uint16_t> m_shared_ram;
	uint8_t m_nvram[0x800]{};
	int m_dsp_bio = 0;
	int m_dsp_idle = 0;

	required_device<cpu_device> m_maincpu;
	required_device<tms32010_device> m_dsp;
	required_device<adc0808_device> m_adc;
	required_device<ls259_device> m_mainlatch;
};



void tomcat_state::adcon_w(uint8_t data)
{
	m_adc->address_w(data & 7);
	m_adc->start_w(BIT(data, 3));
}

uint16_t tomcat_state::tomcat_inputs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t result = 0;
	if (ACCESSING_BITS_8_15)
		result |= ioport("IN0")->read() << 8;

	return result;
}

void tomcat_state::main_latch_w(offs_t offset, uint16_t data)
{
	// A1-A3 = address, A4 = data
	m_mainlatch->write_bit(offset & 7, BIT(offset, 3));
}

void tomcat_state::lnkmode_w(int state)
{
	// Link Mode
	// When Low: Master does not respond to Interrupts
}

void tomcat_state::err_w(int state)
{
	// Link Error Flag
}

void tomcat_state::ack_w(int state)
{
	// Link ACK Flag
}

void tomcat_state::txbuff_w(int state)
{
	// Link Buffer Control
	// When High: Turn off TX (Link) Buffer
}

void tomcat_state::sndres_w(int state)
{
	// Sound Reset
	// When Low: Reset Sound System
	// When High: Release reset of sound system
}

void tomcat_state::mres_w(int state)
{
	// 320 Reset
	// When Low: Reset TMS320
	// When High: Release reset of TMS320
	if (state)
		m_dsp_bio = 0;
	m_dsp->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

void tomcat_state::tomcat_irqclr_w(uint16_t data)
{
	// Clear IRQ Latch          (Address Strobe)
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

uint16_t tomcat_state::tomcat_inputs2_r()
{
/*
*       D15 LNKFLAG     (Game Link)
*       D14 PC3        "    "
*       D13 PC2        "    "
*       D12 PC0        "    "
*       D11 MAINFLAG    (Sound System)
*       D10 SOUNDFLAG      "    "
*       D9  /IDLE*      (TMS320 System)
*       D8
*/
	return m_dsp_idle ? 0 : (1 << 9);
}

uint16_t tomcat_state::tomcat_320bio_r()
{
	m_dsp_bio = 1;
	m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
	return 0;
}

int tomcat_state::dsp_bio_r()
{
	if (m_dsp->pc() == 0x0001)
	{
		if (m_dsp_idle == 0)
		{
			m_dsp_idle = 1;
			m_dsp_bio = 0;
		}
		return !m_dsp_bio;
	}
	else if (m_dsp->pc() == 0x0003)
	{
		if (m_dsp_bio == 1)
		{
			m_dsp_idle = 0;
			m_dsp_bio = 0;
			m_maincpu->resume(SUSPEND_REASON_SPIN);
			return 0;
		}
		else
		{
			assert(0);
			return 0;
		}
	}
	else
	{
		return !m_dsp_bio;
	}
}

uint8_t tomcat_state::tomcat_nvram_r(offs_t offset)
{
	return m_nvram[offset];
}

void tomcat_state::tomcat_nvram_w(offs_t offset, uint8_t data)
{
	m_nvram[offset] = data;
}

void tomcat_state::tomcat_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x402001, 0x402001).r("adc", FUNC(adc0808_device::data_r)).w(FUNC(tomcat_state::adcon_w));
	map(0x404000, 0x404001).r(FUNC(tomcat_state::tomcat_inputs_r)).w("avg", FUNC(avg_device::go_word_w));
	map(0x406000, 0x406001).w("avg", FUNC(avg_device::reset_word_w));
	map(0x408000, 0x408001).r(FUNC(tomcat_state::tomcat_inputs2_r)).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x40a000, 0x40a001).rw(FUNC(tomcat_state::tomcat_320bio_r), FUNC(tomcat_state::tomcat_irqclr_w));
	map(0x40e000, 0x40e01f).w(FUNC(tomcat_state::main_latch_w));
	map(0x800000, 0x803fff).ram();
	map(0xffa000, 0xffbfff).ram().share("shared_ram");
	map(0xffc000, 0xffcfff).ram();
	map(0xffd000, 0xffdfff).rw("m48t02", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);
	map(0xffd000, 0xffdfff).rw(FUNC(tomcat_state::tomcat_nvram_r), FUNC(tomcat_state::tomcat_nvram_w)).umask16(0x00ff);
}

void tomcat_state::dsp_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("shared_ram");
}


void tomcat_state::soundlatches_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x00: break; // XLOAD 0    Write the Sequential ROM counter Low Byte
		case 0x20: break; // XLOAD 1    Write the Sequential ROM counter High Byte
		case 0x40: break; // SOUNDWR    Write to Sound Interface Latch (read by Main)
		case 0x60: break; // unused
		case 0x80: break; // XREAD      Read the Sequential ROM (64K bytes) and increment the counter
		case 0xa0: break; // unused
		case 0xc0: break; // SOUNDREAD  Read the Sound Interface Latch (written by Main)
	}
}

void tomcat_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x3000, 0x30df).w(FUNC(tomcat_state::soundlatches_w));
	map(0x30e0, 0x30e0).noprw(); // COINRD Inputs: D7 = Coin L, D6 = Coin R, D5 = SOUNDFLAG
	map(0x5000, 0x507f).m("riot", FUNC(mos6532_device::ram_map));
	map(0x5080, 0x509f).m("riot", FUNC(mos6532_device::io_map));
	map(0x6000, 0x601f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x7000, 0x701f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x8000, 0xffff).noprw(); // main sound program rom
}

static INPUT_PORTS_START( tomcat )
	PORT_START("IN0")   /* INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("avg", avg_device, done_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED ) // SPARE
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON5 ) // DIAGNOSTIC
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) // R FIRE
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) // L FIRE
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON4 ) // R THUMB
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 ) // L THUMB

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END

void tomcat_state::machine_start()
{
	((uint16_t*)m_shared_ram)[0x0000] = 0xf600;
	((uint16_t*)m_shared_ram)[0x0001] = 0x0000;
	((uint16_t*)m_shared_ram)[0x0002] = 0xf600;
	((uint16_t*)m_shared_ram)[0x0003] = 0x0000;

	subdevice<nvram_device>("nvram")->set_base(m_nvram, 0x800);

	save_item(NAME(m_nvram));
	save_item(NAME(m_dsp_bio));
	save_item(NAME(m_dsp_idle));

	m_dsp_bio = 0;
}

void tomcat_state::tomcat(machine_config &config)
{
	M68010(config, m_maincpu, 12.096_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tomcat_state::tomcat_map);
	m_maincpu->set_periodic_int(FUNC(tomcat_state::irq1_line_assert), attotime::from_hz(5*60));
	//m_maincpu->set_periodic_int(FUNC(tomcat_state::irq1_line_assert), attotime::from_hz(12.096_MHz_XTAL / 16 / 16 / 16 / 12));

	TMS32010(config, m_dsp, 16_MHz_XTAL);
	m_dsp->set_addrmap(AS_PROGRAM, &tomcat_state::dsp_map);
	m_dsp->bio().set(FUNC(tomcat_state::dsp_bio_r));

	m6502_device &soundcpu(M6502(config, "soundcpu", 14.318181_MHz_XTAL / 8));
	soundcpu.set_disable();
	soundcpu.set_addrmap(AS_PROGRAM, &tomcat_state::sound_map);

	ADC0809(config, m_adc, 12.096_MHz_XTAL / 16);
	m_adc->in_callback<0>().set_ioport("STICKY");
	m_adc->in_callback<1>().set_ioport("STICKX");

	MOS6532(config, "riot", 14.318181_MHz_XTAL / 8);
	/*
	 PA0 = /WS   OUTPUT  (TMS-5220 WRITE STROBE)
	 PA1 = /RS   OUTPUT  (TMS-5220 READ STROBE)
	 PA2 = /READY    INPUT   (TMS-5220 READY FLAG)
	 PA3 = FSEL  OUTPUT  Select TMS5220 clock;
	 0 = 325 KHz (8 KHz sampling)
	 1 = 398 KHz (10 KHz sampling)
	 PA4 = /CC1  OUTPUT  Coin Counter 1
	 PA5 = /CC2  OUTPUT  Coin Counter 2
	 PA6 = /MUSRES   OUTPUT  (Reset the Yamaha)
	 PA7 = MAINFLAG  INPUT
	 */
	// OUTB PB0 - PB7   OUTPUT  Speech Data
	// IRQ CB connected to IRQ line of 6502

	config.set_maximum_quantum(attotime::from_hz(4000));

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set_output("led1").invert();
	m_mainlatch->q_out_cb<1>().set_output("led2").invert();
	m_mainlatch->q_out_cb<2>().set(FUNC(tomcat_state::mres_w));
	m_mainlatch->q_out_cb<3>().set(FUNC(tomcat_state::sndres_w));
	m_mainlatch->q_out_cb<4>().set(FUNC(tomcat_state::lnkmode_w));
	m_mainlatch->q_out_cb<5>().set(FUNC(tomcat_state::err_w));
	m_mainlatch->q_out_cb<6>().set(FUNC(tomcat_state::ack_w));
	m_mainlatch->q_out_cb<7>().set(FUNC(tomcat_state::txbuff_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	M48T02(config, "m48t02", 0);

	VECTOR(config, "vector", 0);
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_VECTOR));
	screen.set_refresh_hz(40);
	//screen.set_refresh_hz((double)XTAL(12'000'000) / 16 / 16 / 16 / 12  / 5 );
	screen.set_size(400, 300);
	screen.set_visarea(0, 280, 0, 250);
	screen.set_screen_update("vector", FUNC(vector_device::screen_update));

	avg_device &avg(AVG_STARWARS(config, "avg", 0));
	avg.set_vector("vector");
	avg.set_memory(m_maincpu, AS_PROGRAM, 0x800000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	POKEY(config, "pokey1", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "lspeaker", 0.20);

	POKEY(config, "pokey2", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "rspeaker", 0.20);

	TMS5220(config, m_tms, 325000);
	m_tms->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_tms->add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	YM2151(config, "ymsnd", XTAL(14'318'181)/4).add_route(0, "lspeaker", 0.60).add_route(1, "rspeaker", 0.60);
}

ROM_START( tomcat )
	ROM_REGION( 0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE( "rom1k.bin", 0x00001, 0x8000, CRC(5535a1ff) SHA1(b9807c749a8e6b5ddec3ff494130abda09f0baab) )
	ROM_LOAD16_BYTE( "rom2k.bin", 0x00000, 0x8000, CRC(021a01d2) SHA1(01d99aab54ad57a664e8aaa91296bb879fc6e422) )

	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "136021-105.1l",   0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */
ROM_END

} // anonymous namespace

GAME( 1985, tomcat, 0,        tomcat, tomcat, tomcat_state, empty_init, ROT0, "Atari", "TomCat (prototype)", MACHINE_SUPPORTS_SAVE )
