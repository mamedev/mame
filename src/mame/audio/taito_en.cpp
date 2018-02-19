// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, hap, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

    TODO:

    * ES5510 ESP emulation is not perfect, Verify ES5505 Output Channels
    * Where does the MB8421 go? Taito F3 (and Super Chase) have 2 of them on
      the sound area, Taito JC has one.

****************************************************************************/

#include "emu.h"
#include "taito_en.h"
#include "speaker.h"
#include <algorithm>


DEFINE_DEVICE_TYPE(TAITO_EN, taito_en_device, "taito_en", "Taito Ensoniq Sound System")

taito_en_device::taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITO_EN, tag, owner, clock),
	m_audiocpu(*this, "audiocpu"),
	m_ensoniq(*this, "ensoniq"),
	m_esp(*this, "esp"),
	m_pump(*this, "pump"),
	m_duart68681(*this, "duart68681"),
	m_mb87078(*this, "mb87078"),
	m_osram(*this, "osram"),
	m_osrom(*this, "audiocpu"),
	m_cpubank(*this, "cpubank%u", 1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito_en_device::device_start()
{
	m_pump->set_otis(m_ensoniq);
	m_pump->set_esp(m_esp);
	uint8_t *ROM = m_osrom->base();
	uint32_t max = (m_osrom->bytes() - 0x100000) / 0x20000;
	for (int i = 0; i < 3; i++)
		m_cpubank[i]->configure_entries(0, max, &ROM[0x100000], 0x20000);
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
	uint32_t max_banks_this_game = (memregion(":ensoniq.0")->bytes()/0x200000)-1;

	/* mask out unused bits */
	data &= max_banks_this_game;
	m_ensoniq->voice_bank_w(offset,data<<20);
}

WRITE8_MEMBER( taito_en_device::en_volume_w )
{
	m_mb87078->data_w(data, offset ^ 1);
}


/*************************************
 *
 *  68000 memory map
 *
 *************************************/

ADDRESS_MAP_START(taito_en_device::en_sound_map)
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_MIRROR(0x30000) AM_SHARE("osram")
	AM_RANGE(0x140000, 0x140fff) AM_DEVREADWRITE8("dpram", mb8421_device, right_r, right_w, 0xff00)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE("ensoniq", es5505_device, read, write)
	AM_RANGE(0x260000, 0x2601ff) AM_DEVREADWRITE8("esp", es5510_device, host_r, host_w, 0x00ff)
	AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart68681", mc68681_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x30003f) AM_WRITE(en_es5505_bank_w)
	AM_RANGE(0x340000, 0x340003) AM_WRITE8(en_volume_w, 0xff00)
	AM_RANGE(0xc00000, 0xc1ffff) AM_ROMBANK("cpubank1")
	AM_RANGE(0xc20000, 0xc3ffff) AM_ROMBANK("cpubank2")
	AM_RANGE(0xc40000, 0xc7ffff) AM_ROMBANK("cpubank3")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")  // mirror
ADDRESS_MAP_END


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
	MCFG_CPU_ADD("audiocpu", M68000, XTAL(30'476'100) / 2)
	MCFG_CPU_PROGRAM_MAP(en_sound_map)

	MCFG_CPU_ADD("esp", ES5510, XTAL(10'000'000)) // ES5510 Clock is unverified
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("duart68681", MC68681, XTAL(16'000'000) / 4)
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(XTAL(16'000'000)/2/8, XTAL(16'000'000)/2/16, XTAL(16'000'000)/2/16, XTAL(16'000'000)/2/8)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(taito_en_device, duart_irq_handler))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(taito_en_device, duart_output))

	MCFG_DEVICE_ADD("mb87078", MB87078, 0)
	MCFG_MB87078_GAIN_CHANGED_CB(WRITE8(taito_en_device, mb87078_gain_changed))

	MCFG_DEVICE_ADD("dpram", MB8421, 0) // host accesses this from the other side

	/* sound hardware */
	MCFG_SOUND_ADD("pump", ESQ_5505_5510_PUMP, XTAL(30'476'100) / (2 * 16 * 32))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, XTAL(30'476'100) / 2)
	MCFG_ES5505_REGION0("ensoniq.0")
	MCFG_ES5505_REGION1("ensoniq.0")
	MCFG_ES5506_CHANNELS(4) // TODO : Verify output channels
	MCFG_SOUND_ROUTE_EX(0, "pump", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "pump", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "pump", 1.0, 2)
	MCFG_SOUND_ROUTE_EX(3, "pump", 1.0, 3)
	MCFG_SOUND_ROUTE_EX(4, "pump", 1.0, 4)
	MCFG_SOUND_ROUTE_EX(5, "pump", 1.0, 5)
	MCFG_SOUND_ROUTE_EX(6, "pump", 1.0, 6)
	MCFG_SOUND_ROUTE_EX(7, "pump", 1.0, 7)
MACHINE_CONFIG_END
