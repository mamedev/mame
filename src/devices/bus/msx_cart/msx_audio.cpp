// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
  - 61 keys: 5 full octaves + high C
  - Different wiring, so incompatible with the other keyboards

- Panasonic YK-20
  - 49 keys: 4 full octaves + high C


TODO:
- Test MIDI in/out/through
- Sample RAM, doesn't seem to get written to
- Test NMS-1160 keyboard
- HX-MU901: ENTER/SELECT keys and multi sensors
- NMS1160: Test the keyboard
- NMS1205: Add DAC
- NMS1205/FSCA1: Add muting of dac and y8950 based on io config writes.


For testing the sample ram (playback of a cuckoo sound, the volume of the sound is quite low):
- Disable firmware on the fs-ca1
- Execute the following basic commands:
  - CALL AUDIO
  - CALL COPY PCM(#1,0)
  - CALL PLAY PCM(0)

See also http://www.mccm.hetlab.tk/millennium/milc/gc/topic_26.htm (dutch)
and/or http://ngs.no.coocan.jp/doc/wiki.cgi/datapack?page=4.2+MSX-AUDIO+%B3%C8%C4%A5BASIC (japanese)


**********************************************************************************/

#include "emu.h"
#include "msx_audio.h"
#include "bus/msx_cart/msx_audio_kb.h"


const device_type MSX_CART_MSX_AUDIO_HXMU900 = &device_creator<msx_cart_msx_audio_hxmu900>;
const device_type MSX_CART_MSX_AUDIO_NMS1205 = &device_creator<msx_cart_msx_audio_nms1205>;
const device_type MSX_CART_MSX_AUDIO_FSCA1 = &device_creator<msx_cart_msx_audio_fsca1>;


msx_cart_msx_audio_hxmu900::msx_cart_msx_audio_hxmu900(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
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

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, nullptr)
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





msx_cart_msx_audio_nms1205::msx_cart_msx_audio_nms1205(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_NMS1205, "MSX Cartridge - MSX-AUDIO NMS-1205", tag, owner, clock, "msx_audio_nms1205", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_acia6850(*this, "acia6850")
	, m_mdout(*this, "mdout")
	, m_mdthru(*this, "mdthru")
{
}


static MACHINE_CONFIG_FRAGMENT( msx_audio_nms1205 )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	// At the same time the sound is also output on two output on the nms1205 cartridge itself
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("y8950", Y8950, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
	MCFG_Y8950_KEYBOARD_WRITE_HANDLER(DEVWRITE8("kbdc", msx_audio_kbdc_port_device, write))
	MCFG_Y8950_KEYBOARD_READ_HANDLER(DEVREAD8("kbdc", msx_audio_kbdc_port_device, read))
	MCFG_Y8950_IRQ_HANDLER(WRITELINE(msx_cart_msx_audio_nms1205, irq_write))

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, nullptr)

	// There is a 2 MHz crystal on the PCB, the 6850 TX and RX clocks are derived from it
	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("mdout", midi_port_device, write_txd))

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(WRITELINE(msx_cart_msx_audio_nms1205, midi_in))

	MCFG_MIDI_PORT_ADD("mdthru", midiout_slot, "midiout")

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio_nms1205::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio_nms1205 );
}


ROM_START( msx_nms1205 )
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END


const rom_entry *msx_cart_msx_audio_nms1205::device_rom_region() const
{
	return ROM_NAME( msx_nms1205 );
}


WRITE_LINE_MEMBER(msx_cart_msx_audio_nms1205::irq_write)
{
	// Trigger IRQ on the maincpu
	// The 8950 seems to trigger an irq on reset, this causes an infinite loop of continuously triggering
	// the MSX's interrupt handler. The 8950 irq will never be cleared the nms1205's irq handler hook hasn't
	// been installed yet.
//  m_out_irq_cb(state);
}


WRITE_LINE_MEMBER(msx_cart_msx_audio_nms1205::midi_in)
{
	// MIDI in signals is sent to both the 6850 and the MIDI thru output port
	m_acia6850->write_rxd(state);
	m_mdthru->write_txd(state);
}


void msx_cart_msx_audio_nms1205::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xc0, 0xc1, write8_delegate(FUNC(y8950_device::write), m_y8950.target()));
	space.install_read_handler(0xc0, 0xc1, read8_delegate(FUNC(y8950_device::read), m_y8950.target()));
	space.install_write_handler(0x00, 0x00, write8_delegate(FUNC(acia6850_device::control_w), m_acia6850.target()));
	space.install_write_handler(0x01, 0x01, write8_delegate(FUNC(acia6850_device::data_w), m_acia6850.target()));
	space.install_read_handler(0x04,0x04, read8_delegate(FUNC(acia6850_device::status_r), m_acia6850.target()));
	space.install_read_handler(0x05,0x05, read8_delegate(FUNC(acia6850_device::data_r), m_acia6850.target()));
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







msx_cart_msx_audio_fsca1::msx_cart_msx_audio_fsca1(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_FSCA1, "MSX Cartridge - MSX-AUDIO FS-CA1", tag, owner, clock, "msx_audio_fsca1", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_io_config(*this, "CONFIG")
	, m_region_y8950(*this, "y8950")
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
	MCFG_Y8950_IO_READ_HANDLER(READ8(msx_cart_msx_audio_fsca1, y8950_io_r))
	MCFG_Y8950_IO_WRITE_HANDLER(WRITE8(msx_cart_msx_audio_fsca1, y8950_io_w))

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, nullptr)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_msx_audio_fsca1::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_audio_fsca1 );
}


static INPUT_PORTS_START( msx_audio_fsca1 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x04, 0x04, "FS-CA1 Firmware switch")
	PORT_CONFSETTING( 0x04, "On" )
	PORT_CONFSETTING( 0x00, "Off" )
	PORT_BIT(0xFB, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END


ioport_constructor msx_cart_msx_audio_fsca1::device_input_ports() const
{
	return INPUT_PORTS_NAME( msx_audio_fsca1 );
}


ROM_START( msx_fsca1 )
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END


const rom_entry *msx_cart_msx_audio_fsca1::device_rom_region() const
{
	return ROM_NAME( msx_fsca1 );
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
	if (get_rom_size() < 0x20000)
	{
		fatalerror("msx_audio_fsca1: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_msx_audio_fsca1::read_cart)
{
	if (m_7ffe == 0 && (offset & 0xB000) == 0x3000)
	{
		return m_sram[offset & 0xfff];
	}
	return m_rom[((m_7ffe & 0x03) << 15) | (offset & 0x7fff)];
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

	if (m_7ffe == 0 && (offset & 0xb000) == 0x3000)
	{
		m_sram[offset & 0xfff] = data;
		return;
	}

	logerror("msx_cart_msx_audio_fsca1: Unhandled write %02x to %04x\n", data, offset);
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


WRITE8_MEMBER(msx_cart_msx_audio_fsca1::y8950_io_w)
{
	logerror("msx_fsca1::y8950_io_w: %02x\n", data);
}


READ8_MEMBER(msx_cart_msx_audio_fsca1::y8950_io_r)
{
	return m_io_config->read();
}
