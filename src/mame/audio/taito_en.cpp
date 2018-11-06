// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, hap, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

    TODO:

    * ES5510 ESP emulation is not perfect
    * ES5510 Input Clock and ES5505 Output channels are same in other PCBs?
      (Currently these are verified from Gun Buster schematics)
    * Where does the MB8421 go? Taito F3 (and Super Chase) have 2 of them on
      the sound area, Taito JC has one.

****************************************************************************/

#include "emu.h"
#include "taito_en.h"
#include "speaker.h"
#include <algorithm>


DEFINE_DEVICE_TYPE(TAITO_EN, taito_en_device, "taito_en", "Taito Ensoniq Sound System")

taito_en_device::taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITO_EN, tag, owner, clock)
	, m_audiocpu(*this, "audiocpu")
	, m_ensoniq(*this, "ensoniq")
	, m_esp(*this, "esp")
	, m_pump(*this, "pump")
	, m_duart68681(*this, "duart68681")
	, m_mb87078(*this, "mb87078")
	, m_osram(*this, "osram")
	, m_osrom(*this, "audiocpu")
	, m_cpubank(*this, "cpubank%u", 1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito_en_device::device_start()
{
	// tell the pump about the ESP chips
	m_pump->set_esp(m_esp);
	uint8_t *ROM = m_osrom->base();
	uint32_t max = (m_osrom->bytes() - 0x100000) / 0x20000;
	for (int i = 0; i < 3; i++)
		m_cpubank[i]->configure_entries(0, max, &ROM[0x100000], 0x20000);

	m_bankmask = (memregion(":ensoniq.0")->bytes()/0x200000)-1;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taito_en_device::device_reset()
{
	/* Sound cpu program loads to 0xc00000 so we use a bank */
	uint32_t max = (m_osrom->bytes() - 0x100000) / 0x20000;
	for (int i = 0; i < 3; i++)
		m_cpubank[i]->set_entry(i % max);

	uint16_t *ROM = (uint16_t *)m_osrom->base();
	std::copy(&ROM[0x80000], &ROM[0x80004], &m_osram[0]); /* Stack and Reset vectors */

	/* reset CPU to catch any banking of startup vectors */
	m_audiocpu->reset();
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


/*************************************
 *
 *  Handlers
 *
 *************************************/

WRITE16_MEMBER( taito_en_device::en_es5505_bank_w )
{
	/* mask out unused bits */
	data &= m_bankmask;
	m_ensoniq->voice_bank_w(offset,data<<20);
}

WRITE8_MEMBER( taito_en_device::en_volume_w )
{
	m_mb87078->data_w(offset ^ 1, data);
}


/*************************************
 *
 *  68000 memory map
 *
 *************************************/

void taito_en_device::en_sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().mirror(0x30000).share("osram");
	map(0x140000, 0x140fff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w)).umask16(0xff00);
	map(0x200000, 0x20001f).rw("ensoniq", FUNC(es5505_device::read), FUNC(es5505_device::write));
	map(0x260000, 0x2601ff).rw("esp", FUNC(es5510_device::host_r), FUNC(es5510_device::host_w)).umask16(0x00ff);
	map(0x280000, 0x28001f).rw("duart68681", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x300000, 0x30003f).w(FUNC(taito_en_device::en_es5505_bank_w));
	map(0x340000, 0x340003).w(FUNC(taito_en_device::en_volume_w)).umask16(0xff00);
	map(0xc00000, 0xc1ffff).bankr("cpubank1");
	map(0xc20000, 0xc3ffff).bankr("cpubank2");
	map(0xc40000, 0xc7ffff).bankr("cpubank3");
	map(0xff0000, 0xffffff).ram().share("osram");  // mirror
}


/*************************************
 *
 *  MB87078 callback
 *
 *************************************/

WRITE8_MEMBER(taito_en_device::mb87078_gain_changed)
{
	if (offset > 1)
	{
		// TODO : ES5505 Volume control is correct?
		m_ensoniq->set_output_gain(offset & 1, data / 100.0);
		m_ensoniq->set_output_gain(2|(offset & 1), data / 100.0);
		m_ensoniq->set_output_gain(4|(offset & 1), data / 100.0);
		m_ensoniq->set_output_gain(6|(offset & 1), data / 100.0);
		m_pump->set_output_gain(offset & 1, data / 100.0);
	}
}


/*************************************
 *
 *  M68681 callback
 *
 *************************************/

WRITE_LINE_MEMBER(taito_en_device::duart_irq_handler)
{
	if (state == ASSERT_LINE)
	{
		m_audiocpu->set_input_line_vector(M68K_IRQ_6, m_duart68681->get_irq_vector());
		m_audiocpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
	else
	{
		m_audiocpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}


/*************************************
 *
 *  Device interfaces
 *
 *************************************/

/*
    68681 I/O pin assignments
    (according to Gun Buster schematics):

    IP0: 5V         OP0-OP5: N/C
    IP1: 5V         OP6: ESPHALT
    IP2: 1MHz       OP7: N/C
    IP3: 0.5MHz
    IP4: 0.5MHz
    IP5: 1MHz
*/

WRITE8_MEMBER(taito_en_device::duart_output)
{
	if (data & 0x40)
	{
		if (!m_pump->get_esp_halted())
		{
			logerror("Asserting ESPHALT\n");
			m_pump->set_esp_halted(true);
		}
	}
	else
	{
		if (m_pump->get_esp_halted())
		{
			logerror("Clearing ESPHALT\n");
			m_pump->set_esp_halted(false);
		}
	}
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(taito_en_device::device_add_mconfig)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("audiocpu", M68000, XTAL(30'476'100) / 2)
	MCFG_DEVICE_PROGRAM_MAP(en_sound_map)

	MCFG_DEVICE_ADD("esp", ES5510, XTAL(10'000'000)) // from Gun Buster schematics
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("duart68681", MC68681, XTAL(16'000'000) / 4)
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(XTAL(16'000'000)/2/8, XTAL(16'000'000)/2/16, XTAL(16'000'000)/2/16, XTAL(16'000'000)/2/8)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(*this, taito_en_device, duart_irq_handler))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(*this, taito_en_device, duart_output))

	MB87078(config, m_mb87078);
	m_mb87078->gain_changed().set(FUNC(taito_en_device::mb87078_gain_changed));

	MCFG_DEVICE_ADD("dpram", MB8421, 0) // host accesses this from the other side

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("pump", ESQ_5505_5510_PUMP, XTAL(30'476'100) / (2 * 16 * 32))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("ensoniq", ES5505, XTAL(30'476'100) / 2)
	MCFG_ES5505_REGION0("ensoniq.0")
	MCFG_ES5505_REGION1("ensoniq.0")
	MCFG_ES5505_CHANNELS(4)
	MCFG_SOUND_ROUTE(0, "pump", 1.0, 0)
	MCFG_SOUND_ROUTE(1, "pump", 1.0, 1)
	MCFG_SOUND_ROUTE(2, "pump", 1.0, 2)
	MCFG_SOUND_ROUTE(3, "pump", 1.0, 3)
	MCFG_SOUND_ROUTE(4, "pump", 1.0, 4)
	MCFG_SOUND_ROUTE(5, "pump", 1.0, 5)
	MCFG_SOUND_ROUTE(6, "pump", 1.0, 6)
	MCFG_SOUND_ROUTE(7, "pump", 1.0, 7)
MACHINE_CONFIG_END
