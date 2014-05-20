/**********************************************************************************

**********************************************************************************/

#include "emu.h"
#include "msx_audio.h"

const device_type MSX_CART_MSX_AUDIO = &device_creator<msx_cart_msx_audio>;


msx_cart_msx_audio::msx_cart_msx_audio(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO, "MSX Cartridge - MSX-AUDIO", tag, owner, clock, "msx_cart_msx_audio", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_acia6850(*this, "acia6850")
{
}


static MACHINE_CONFIG_FRAGMENT( msx_audio )
	// There is a 2 MHz crystal on the PCB, where does it go?

	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("y8950", Y8950, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio );
}


void msx_cart_msx_audio::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xc0, 0xc1, write8_delegate(FUNC(msx_cart_msx_audio::write_y8950), this));
	space.install_read_handler(0xc0, 0xc1, read8_delegate(FUNC(msx_cart_msx_audio::read_y8950), this));
}


void msx_cart_msx_audio::initialize_cartridge()
{
	if ( get_rom_size() != 0x8000 )
	{
		fatalerror("msx_audio: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_msx_audio::read_cart)
{
	if (offset >= 0x4000 && offset < 0xC000)
	{
		return m_rom[offset - 0x4000];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_msx_audio::write_y8950)
{
	m_y8950->write(space, offset, data);
}

READ8_MEMBER(msx_cart_msx_audio::read_y8950)
{
	return m_y8950->read(space, offset);
}

