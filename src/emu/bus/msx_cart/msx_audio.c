/**********************************************************************************

Emulation of the different MSX-AUDIO devices:

- Panasonic FS-CA1
  - Y8950
  - 4KB ram
  - Connector for Toshiba HX-MU901 keyboard

- Toshiba HX-MU900
  - Y8950
  - No midi ports
  - No ram
  - Connector for Toshiba HX-MU901 keyboard

- Philips NMS-1205
  - Y8950
  - Midi ports
  - 32KB sample ram
  - Connector for Philips NMS-1160 keyboard


The keyboards:
- Toshiba HX-MU901
  - 49 keys: 4 full octaves + high C
  - ENTER key
  - SELECT key
  - multi sensor (?)

- Philips NMS-1160

**********************************************************************************/

#include "emu.h"
#include "msx_audio.h"
#include "bus/msx_cart/msx_audio_kb.h"


const device_type MSX_CART_MSX_AUDIO_HXMU900 = &device_creator<msx_cart_msx_audio_hxmu900>;
const device_type MSX_CART_MSX_AUDIO_NMS1205 = &device_creator<msx_cart_msx_audio_nms1205>;
const device_type MSX_CART_MSX_AUDIO_FSCA1 = &device_creator<msx_cart_msx_audio_fsca1>;


msx_cart_msx_audio_hxmu900::msx_cart_msx_audio_hxmu900(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_HXMU900, "MSX Cartridge - MSX-AUDIO HX-MU900", tag, owner, clock, "msx_audio_hxmu900", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
{
}


static MACHINE_CONFIG_FRAGMENT( msx_audio_hxmu900 )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("y8950", Y8950, XTAL_3_579545MHz)    // Not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
	MCFG_Y8950_KEYBOARD_WRITE_HANDLER(DEVWRITE8("kbdc", msx_audio_kbdc_port_device, write))
	MCFG_Y8950_KEYBOARD_READ_HANDLER(DEVREAD8("kbdc", msx_audio_kbdc_port_device, read))

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, NULL)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio_hxmu900::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio_hxmu900 );
}


void msx_cart_msx_audio_hxmu900::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xc0, 0xc1, write8_delegate(FUNC(y8950_device::write), m_y8950.target()));
	space.install_read_handler(0xc0, 0xc1, read8_delegate(FUNC(y8950_device::read), m_y8950.target()));
}


void msx_cart_msx_audio_hxmu900::initialize_cartridge()
{
	if (get_rom_size() < 0x8000)
	{
		fatalerror("msx_audio: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_msx_audio_hxmu900::read_cart)
{
	if (offset >= 0x4000 && offset < 0xC000)
	{
		return m_rom[offset - 0x4000];
	}
	return 0xff;
}





msx_cart_msx_audio_nms1205::msx_cart_msx_audio_nms1205(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_NMS1205, "MSX Cartridge - MSX-AUDIO NMS-1205", tag, owner, clock, "msx_audio_nms1205", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_acia6850(*this, "acia6850")
{
}


static MACHINE_CONFIG_FRAGMENT( msx_audio_nms1205 )
	// There is a 2 MHz crystal on the PCB, where does it go?

	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("y8950", Y8950, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
	MCFG_Y8950_KEYBOARD_WRITE_HANDLER(DEVWRITE8("kbdc", msx_audio_kbdc_port_device, write))
	MCFG_Y8950_KEYBOARD_READ_HANDLER(DEVREAD8("kbdc", msx_audio_kbdc_port_device, read))

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, NULL)

	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio_nms1205::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio_nms1205 );
}


void msx_cart_msx_audio_nms1205::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xc0, 0xc1, write8_delegate(FUNC(y8950_device::write), m_y8950.target()));
	space.install_read_handler(0xc0, 0xc1, read8_delegate(FUNC(y8950_device::read), m_y8950.target()));
}


void msx_cart_msx_audio_nms1205::initialize_cartridge()
{
	if (get_rom_size() < 0x8000)
	{
		fatalerror("msx_audio: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_msx_audio_nms1205::read_cart)
{
	if (offset >= 0x4000 && offset < 0xC000)
	{
		return m_rom[offset - 0x4000];
	}
	return 0xff;
}







msx_cart_msx_audio_fsca1::msx_cart_msx_audio_fsca1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_FSCA1, "MSX Cartridge - MSX-AUDIO FS-CA1", tag, owner, clock, "msx_audio_fsca1", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_7ffe(0)
	, m_7fff(0)
{
}


static MACHINE_CONFIG_FRAGMENT( msx_audio_fsca1 )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("y8950", Y8950, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
	MCFG_Y8950_KEYBOARD_WRITE_HANDLER(DEVWRITE8("kbdc", msx_audio_kbdc_port_device, write))
	MCFG_Y8950_KEYBOARD_READ_HANDLER(DEVREAD8("kbdc", msx_audio_kbdc_port_device, read))

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, NULL)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio_fsca1::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio_fsca1 );
}


void msx_cart_msx_audio_fsca1::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xc0, 0xc3, write8_delegate(FUNC(msx_cart_msx_audio_fsca1::write_y8950), this));
	space.install_read_handler(0xc0, 0xc3, read8_delegate(FUNC(msx_cart_msx_audio_fsca1::read_y8950), this));
}


void msx_cart_msx_audio_fsca1::initialize_cartridge()
{
	if (get_rom_size() != 0x20000)
	{
		fatalerror("msx_audio_fsca1: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_msx_audio_fsca1::read_cart)
{
	if (offset < 0x8000)
	{
		if ((offset & 0x3000) == 0x3000)
		{
			return m_ram[offset & 0xfff];
		}
		return m_rom[offset];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_msx_audio_fsca1::write_cart)
{
	if (offset == 0x7ffe)
	{
		m_7ffe = data;
		return;
	}

	if (offset == 0x7fff)
	{
		m_7fff = data;
		return;
	}

	if ((offset & 0xb000) == 0x3000)
	{
		m_ram[offset & 0xfff] = data;
		return;
	}

	printf("msx_cart_msx_audio_fsca1: Unhandled write %02x to %04x\n", data, offset);
}


WRITE8_MEMBER(msx_cart_msx_audio_fsca1::write_y8950)
{
	if (offset & 2)
	{
		if (m_7fff & 0x02)
		{
			m_y8950->write(space, offset, data);
		}
	}
	else
	{
		if (m_7fff & 0x01)
		{
			m_y8950->write(space, offset, data);
		}
	}
}


READ8_MEMBER(msx_cart_msx_audio_fsca1::read_y8950)
{
	if (offset & 2)
	{
		return (m_7fff & 0x02) ? m_y8950->read(space, offset) : 0xff;
	}
	else
	{
		return (m_7fff & 0x01) ? m_y8950->read(space, offset) : 0xff;
	}
}

