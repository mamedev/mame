// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Mattel Intellivoice cart emulation

 TODO:
   - speech ROM shall be loaded from softlist

 ***********************************************************************************************************/


#include "emu.h"
#include "voice.h"


//-------------------------------------------------
//  intv_voice_device - constructor
//-------------------------------------------------

const device_type INTV_ROM_VOICE = &device_creator<intv_voice_device>;

intv_voice_device::intv_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: intv_rom_device(mconfig, INTV_ROM_VOICE, "Intellivision Intellivoice Expansion", tag, owner, clock, "intv_voice", __FILE__),
				m_speech(*this, "sp0256_speech"),
				m_subslot(*this, "subslot"),
				m_ramd0_enabled(false),
				m_ram88_enabled(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intv_voice_device::device_start()
{
}

void intv_voice_device::late_subslot_setup()
{
	switch (m_subslot->get_type())
	{
		case INTV_RAM:
			m_ramd0_enabled = true;
			break;
		case INTV_GFACT:
			m_ram88_enabled = true;
			break;
		case INTV_VOICE:
			printf("WARNING: You cannot connect serially multiple IntelliVoice units.\n");
			printf("WARNING: Emulation will likely misbehave.\n");
			break;
		case INTV_ECS:
			printf("WARNING: You cannot connect ECS to IntelliVoice in this manner.\n");
			printf("WARNING: Emulation will likely misbehave.\n");
			break;
		case INTV_KEYCOMP:
			printf("WARNING: You cannot connect the Keyboard component to the IntelliVoice unit.\n");
			printf("WARNING: Emulation will likely misbehave.\n");
			break;
	}
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( intellivoice )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( intellivoice )
	MCFG_SPEAKER_STANDARD_MONO("mono_voice")

	MCFG_SOUND_ADD("sp0256_speech", SP0256, 3120000)
	/* The Intellivoice uses a speaker with its own volume control so the relative volumes to use are subjective */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono_voice", 1.00)

	MCFG_INTV_CARTRIDGE_ADD("subslot", intv_cart, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor intv_voice_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( intellivoice );
}


ROM_START( intellivoice )
	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	/* SP0256-012 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256-012.bin",   0x1000, 0x0800, CRC(0de7579d) SHA1(618563e512ff5665183664f52270fa9606c9d289) )
ROM_END

const rom_entry *intv_voice_device::device_rom_region() const
{
	return ROM_NAME( intellivoice );
}

/*-------------------------------------------------
 read_audio
 -------------------------------------------------*/

READ16_MEMBER(intv_voice_device::read_speech)
{
	if (ACCESSING_BITS_0_7)
		return m_speech->spb640_r(space, offset, mem_mask);
	else
		return 0xff;
}

/*-------------------------------------------------
 write_audio
 -------------------------------------------------*/

WRITE16_MEMBER(intv_voice_device::write_speech)
{
	if (ACCESSING_BITS_0_7)
		return m_speech->spb640_w(space, offset, data, mem_mask);
}
