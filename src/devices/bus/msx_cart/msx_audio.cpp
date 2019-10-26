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

#include "speaker.h"


DEFINE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_HXMU900, msx_cart_msx_audio_hxmu900_device, "msx_audio_hxmu900", "MSX Cartridge - MSX-AUDIO HX-MU900")
DEFINE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_NMS1205, msx_cart_msx_audio_nms1205_device, "msx_audio_nms1205", "MSX Cartridge - MSX-AUDIO NMS-1205")
DEFINE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_FSCA1,   msx_cart_msx_audio_fsca1_device,   "msx_audio_fsca1",   "MSX Cartridge - MSX-AUDIO FS-CA1")


msx_cart_msx_audio_hxmu900_device::msx_cart_msx_audio_hxmu900_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_HXMU900, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
{
}


void msx_cart_msx_audio_hxmu900_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	Y8950(config, m_y8950, XTAL(3'579'545)); // Not verified
	m_y8950->add_route(ALL_OUTPUTS, "mono", 0.40);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);
}


void msx_cart_msx_audio_hxmu900_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc1, write8sm_delegate(*m_y8950, FUNC(y8950_device::write)));
	io_space().install_read_handler(0xc0, 0xc1, read8sm_delegate(*m_y8950, FUNC(y8950_device::read)));
}


void msx_cart_msx_audio_hxmu900_device::initialize_cartridge()
{
	if (get_rom_size() < 0x8000)
	{
		fatalerror("msx_audio: Invalid ROM size\n");
	}
}


uint8_t msx_cart_msx_audio_hxmu900_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xC000)
	{
		return m_rom[offset - 0x4000];
	}
	return 0xff;
}


ROM_START( msx_hxmu )
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END


const tiny_rom_entry *msx_cart_msx_audio_hxmu900_device::device_rom_region() const
{
	return ROM_NAME( msx_hxmu );
}





msx_cart_msx_audio_nms1205_device::msx_cart_msx_audio_nms1205_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_NMS1205, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_acia6850(*this, "acia6850")
	, m_mdout(*this, "mdout")
	, m_mdthru(*this, "mdthru")
{
}


void msx_cart_msx_audio_nms1205_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	// At the same time the sound is also output on two output on the nms1205 cartridge itself
	SPEAKER(config, "mono").front_center();
	Y8950(config, m_y8950, XTAL(3'579'545));
	m_y8950->add_route(ALL_OUTPUTS, "mono", 0.40);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_y8950->irq().set(FUNC(msx_cart_msx_audio_nms1205_device::irq_write));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);

	// There is a 2 MHz crystal on the PCB, the 6850 TX and RX clocks are derived from it
	ACIA6850(config, m_acia6850, 0);
	m_acia6850->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(msx_cart_msx_audio_nms1205_device::midi_in));
	MIDI_PORT(config, m_mdthru, midiout_slot, "midiout");
	MIDI_PORT(config, m_mdout, midiout_slot, "midiout");
}


ROM_START( msx_nms1205 )
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END


const tiny_rom_entry *msx_cart_msx_audio_nms1205_device::device_rom_region() const
{
	return ROM_NAME( msx_nms1205 );
}


WRITE_LINE_MEMBER(msx_cart_msx_audio_nms1205_device::irq_write)
{
	// Trigger IRQ on the maincpu
	// The 8950 seems to trigger an irq on reset, this causes an infinite loop of continuously triggering
	// the MSX's interrupt handler. The 8950 irq will never be cleared the nms1205's irq handler hook hasn't
	// been installed yet.
//  m_out_irq_cb(state);
}


WRITE_LINE_MEMBER(msx_cart_msx_audio_nms1205_device::midi_in)
{
	// MIDI in signals is sent to both the 6850 and the MIDI thru output port
	m_acia6850->write_rxd(state);
	m_mdthru->write_txd(state);
}


void msx_cart_msx_audio_nms1205_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc1, write8sm_delegate(*m_y8950, FUNC(y8950_device::write)));
	io_space().install_read_handler(0xc0, 0xc1, read8sm_delegate(*m_y8950, FUNC(y8950_device::read)));
	io_space().install_write_handler(0x00, 0x01, write8sm_delegate(*m_acia6850, FUNC(acia6850_device::write)));
	io_space().install_read_handler(0x04, 0x05, read8sm_delegate(*m_acia6850, FUNC(acia6850_device::read)));
}


void msx_cart_msx_audio_nms1205_device::initialize_cartridge()
{
	if (get_rom_size() < 0x8000)
	{
		fatalerror("msx_audio: Invalid ROM size\n");
	}
}


uint8_t msx_cart_msx_audio_nms1205_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xC000)
	{
		return m_rom[offset - 0x4000];
	}
	return 0xff;
}







msx_cart_msx_audio_fsca1_device::msx_cart_msx_audio_fsca1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MSX_AUDIO_FSCA1, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_y8950(*this, "y8950")
	, m_io_config(*this, "CONFIG")
	, m_region_y8950(*this, "y8950")
	, m_7ffe(0)
	, m_7fff(0)
{
}


void msx_cart_msx_audio_fsca1_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	Y8950(config, m_y8950, XTAL(3'579'545));
	m_y8950->add_route(ALL_OUTPUTS, "mono", 0.40);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_y8950->io_read().set(FUNC(msx_cart_msx_audio_fsca1_device::y8950_io_r));
	m_y8950->io_write().set(FUNC(msx_cart_msx_audio_fsca1_device::y8950_io_w));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);
}


static INPUT_PORTS_START( msx_audio_fsca1 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x04, 0x04, "FS-CA1 Firmware switch")
	PORT_CONFSETTING( 0x04, "On" )
	PORT_CONFSETTING( 0x00, "Off" )
	PORT_BIT(0xFB, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END


ioport_constructor msx_cart_msx_audio_fsca1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( msx_audio_fsca1 );
}


ROM_START( msx_fsca1 )
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END


const tiny_rom_entry *msx_cart_msx_audio_fsca1_device::device_rom_region() const
{
	return ROM_NAME( msx_fsca1 );
}


void msx_cart_msx_audio_fsca1_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc3, write8sm_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::write_y8950)));
	io_space().install_read_handler(0xc0, 0xc3, read8sm_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::read_y8950)));
}


void msx_cart_msx_audio_fsca1_device::initialize_cartridge()
{
	if (get_rom_size() < 0x20000)
	{
		fatalerror("msx_audio_fsca1: Invalid ROM size\n");
	}
}


uint8_t msx_cart_msx_audio_fsca1_device::read_cart(offs_t offset)
{
	if (m_7ffe == 0 && (offset & 0xB000) == 0x3000)
	{
		return m_sram[offset & 0xfff];
	}
	return m_rom[((m_7ffe & 0x03) << 15) | (offset & 0x7fff)];
}


void msx_cart_msx_audio_fsca1_device::write_cart(offs_t offset, uint8_t data)
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

	logerror("msx_cart_msx_audio_fsca1_device: Unhandled write %02x to %04x\n", data, offset);
}


void msx_cart_msx_audio_fsca1_device::write_y8950(offs_t offset, uint8_t data)
{
	if (offset & 2)
	{
		if (m_7fff & 0x02)
		{
			m_y8950->write(offset, data);
		}
	}
	else
	{
		if (m_7fff & 0x01)
		{
			m_y8950->write(offset, data);
		}
	}
}


uint8_t msx_cart_msx_audio_fsca1_device::read_y8950(offs_t offset)
{
	if (offset & 2)
	{
		return (m_7fff & 0x02) ? m_y8950->read(offset) : 0xff;
	}
	else
	{
		return (m_7fff & 0x01) ? m_y8950->read(offset) : 0xff;
	}
}


void msx_cart_msx_audio_fsca1_device::y8950_io_w(uint8_t data)
{
	logerror("msx_fsca1::y8950_io_w: %02x\n", data);
}


uint8_t msx_cart_msx_audio_fsca1_device::y8950_io_r()
{
	return m_io_config->read();
}
