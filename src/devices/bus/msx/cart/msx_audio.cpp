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

See also https://www.msxcomputermagazine.nl/mccm/millennium/milc/gc/topic_26.htm (dutch)
and/or http://ngs.no.coocan.jp/doc/wiki.cgi/datapack?page=4.2+MSX-AUDIO+%B3%C8%C4%A5BASIC (japanese)


**********************************************************************************/

#include "emu.h"
#include "msx_audio.h"

#include "msx_audio_kb.h"

#include "bus/midi/midi.h"
#include "machine/6850acia.h"
#include "sound/ymopl.h"

#include "speaker.h"


namespace {

class msx_cart_msx_audio_hxmu900_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_hxmu900_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_MSX_AUDIO_HXMU900, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_y8950(*this, "y8950")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<y8950_device> m_y8950;
};

void msx_cart_msx_audio_hxmu900_device::device_add_mconfig(machine_config &config)
{
	Y8950(config, m_y8950, DERIVED_CLOCK(1, 1)); // Not verified
	if (parent_slot())
		m_y8950->add_route(ALL_OUTPUTS, soundin(), 0.8);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);
}

void msx_cart_msx_audio_hxmu900_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc1, emu::rw_delegate(*m_y8950, FUNC(y8950_device::write)));
	io_space().install_read_handler(0xc0, 0xc1, emu::rw_delegate(*m_y8950, FUNC(y8950_device::read)));
}

std::error_condition msx_cart_msx_audio_hxmu900_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_msx_audio_hxmu900_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() < 0x8000)
	{
		message = "msx_cart_msx_audio_hxmu900_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return std::error_condition();
}

ROM_START(msx_hxmu)
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *msx_cart_msx_audio_hxmu900_device::device_rom_region() const
{
	return ROM_NAME(msx_hxmu);
}





class msx_cart_msx_audio_nms1205_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_nms1205_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_MSX_AUDIO_NMS1205, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_y8950(*this, "y8950")
		, m_acia6850(*this, "acia6850")
		, m_mdout(*this, "mdout")
		, m_mdthru(*this, "mdthru")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void midi_in(int state);
	void irq_write(int state);

	required_device<y8950_device> m_y8950;
	required_device<acia6850_device> m_acia6850;
	required_device<midi_port_device> m_mdout;
	required_device<midi_port_device> m_mdthru;
};

void msx_cart_msx_audio_nms1205_device::device_add_mconfig(machine_config &config)
{
	// At the same time the sound is also output on two outputs on the nms1205 cartridge itself
	Y8950(config, m_y8950, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_y8950->add_route(ALL_OUTPUTS, soundin(), 0.8);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_y8950->irq_handler().set(FUNC(msx_cart_msx_audio_nms1205_device::irq_write));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);

	// There is a 2 MHz crystal on the PCB, the 6850 TX and RX clocks are derived from it
	ACIA6850(config, m_acia6850, 0);
	m_acia6850->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(msx_cart_msx_audio_nms1205_device::midi_in));
	MIDI_PORT(config, m_mdthru, midiout_slot, "midiout");
	MIDI_PORT(config, m_mdout, midiout_slot, "midiout");
}

ROM_START(msx_nms1205)
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *msx_cart_msx_audio_nms1205_device::device_rom_region() const
{
	return ROM_NAME(msx_nms1205);
}

void msx_cart_msx_audio_nms1205_device::irq_write(int state)
{
	// Trigger IRQ on the maincpu
	// The 8950 seems to trigger an irq on reset, this causes an infinite loop of continuously triggering
	// the MSX's interrupt handler. The 8950 irq will never be cleared the nms1205's irq handler hook hasn't
	// been installed yet.
//  m_out_irq_cb(state);
}

void msx_cart_msx_audio_nms1205_device::midi_in(int state)
{
	// MIDI in signals is sent to both the 6850 and the MIDI thru output port
	m_acia6850->write_rxd(state);
	m_mdthru->write_txd(state);
}

void msx_cart_msx_audio_nms1205_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc1, emu::rw_delegate(*m_y8950, FUNC(y8950_device::write)));
	io_space().install_read_handler(0xc0, 0xc1, emu::rw_delegate(*m_y8950, FUNC(y8950_device::read)));
	io_space().install_write_handler(0x00, 0x01, emu::rw_delegate(*m_acia6850, FUNC(acia6850_device::write)));
	io_space().install_read_handler(0x04, 0x05, emu::rw_delegate(*m_acia6850, FUNC(acia6850_device::read)));
}

std::error_condition msx_cart_msx_audio_nms1205_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_msx_audio_nms1205_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() < 0x8000)
	{
		message = "msx_cart_msx_audio_nms1205_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return std::error_condition();
}







class msx_cart_msx_audio_fsca1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_fsca1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_MSX_AUDIO_FSCA1, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_y8950(*this, "y8950")
		, m_io_config(*this, "CONFIG")
		, m_region_y8950(*this, "y8950")
		, m_rombank(*this, "rombank%u", 0U)
		, m_view{ {*this, "view0"}, {*this, "view1"} }
		, m_7fff(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void write_y8950(offs_t offset, u8 data);
	u8 read_y8950(offs_t offset);
	void y8950_io_w(u8 data);
	u8 y8950_io_r();
	void bank_w(u8 data);
	void write_7fff(u8 data);

	required_device<y8950_device> m_y8950;
	required_ioport m_io_config;
	required_memory_region m_region_y8950;
	memory_bank_array_creator<2> m_rombank;
	memory_view m_view[2];
	u8 m_7fff;
};

void msx_cart_msx_audio_fsca1_device::device_add_mconfig(machine_config &config)
{
	Y8950(config, m_y8950, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_y8950->add_route(ALL_OUTPUTS, soundin(), 0.8);
	m_y8950->keyboard_write().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_y8950->keyboard_read().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_y8950->io_read().set(FUNC(msx_cart_msx_audio_fsca1_device::y8950_io_r));
	m_y8950->io_write().set(FUNC(msx_cart_msx_audio_fsca1_device::y8950_io_w));

	MSX_AUDIO_KBDC_PORT(config, "kbdc", msx_audio_keyboards, nullptr);
}

static INPUT_PORTS_START(msx_audio_fsca1)
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x04, 0x04, "FS-CA1 Firmware switch")
	PORT_CONFSETTING( 0x04, "On" )
	PORT_CONFSETTING( 0x00, "Off" )
	PORT_BIT(0xFB, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END

ioport_constructor msx_cart_msx_audio_fsca1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_audio_fsca1);
}

ROM_START(msx_fsca1)
	ROM_REGION(0x8000, "y8950", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *msx_cart_msx_audio_fsca1_device::device_rom_region() const
{
	return ROM_NAME(msx_fsca1);
}

void msx_cart_msx_audio_fsca1_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0xc0, 0xc3, emu::rw_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::write_y8950)));
	io_space().install_read_handler(0xc0, 0xc3, emu::rw_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::read_y8950)));
	save_item(NAME(m_7fff));
}

void msx_cart_msx_audio_fsca1_device::device_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_view[i].select(0);
		m_rombank[i]->set_entry(0);
	}
}

std::error_condition msx_cart_msx_audio_fsca1_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_msx_audio_fsca1_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() < 0x20000)
	{
		message = "msx_cart_msx_audio_fsca1_device: Region 'rom' has unsupported size";
		return image_error::INVALIDLENGTH;
	}

	m_rombank[0]->configure_entries(0, 4, cart_rom_region()->base(), 0x8000);
	m_rombank[1]->configure_entries(0, 4, cart_rom_region()->base() + 0x4000, 0x8000);

	page(0)->install_view(0x0000, 0x3fff, m_view[0]);
	m_view[0][0].install_read_bank(0x0000, 0x3fff, m_rombank[0]);
	m_view[0][0].install_ram(0x3000, 0x3fff, cart_sram_region()->base());
	m_view[0][1].install_read_bank(0x0000, 0x3fff, m_rombank[0]);

	page(1)->install_view(0x4000, 0x7fff, m_view[1]);
	m_view[1][0].install_read_bank(0x4000, 0x7ffd, m_rombank[1]);
	m_view[1][0].install_ram(0x7000, 0x7ffd, cart_sram_region()->base());
	m_view[1][1].install_read_bank(0x4000, 0x7ffd, m_rombank[1]);
	page(1)->install_write_handler(0x7ffe, 0x7ffe, emu::rw_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::bank_w)));
	page(1)->install_write_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_msx_audio_fsca1_device::write_7fff)));

	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[0]);
	page(3)->install_read_bank(0xc000, 0xffff, m_rombank[1]);

	return std::error_condition();
}

void msx_cart_msx_audio_fsca1_device::bank_w(u8 data)
{
	for (int i = 0; i < 2; i++)
	{
		m_rombank[i]->set_entry(data & 0x03);
		m_view[i].select((data != 0) ? 1 : 0);
	}
}

void msx_cart_msx_audio_fsca1_device::write_7fff(u8 data)
{
	m_7fff = data;
}

void msx_cart_msx_audio_fsca1_device::write_y8950(offs_t offset, u8 data)
{
	if (BIT(offset, 1))
	{
		if (BIT(m_7fff, 1))
			m_y8950->write(offset, data);
	}
	else
	{
		if (BIT(m_7fff, 0))
			m_y8950->write(offset, data);
	}
}

u8 msx_cart_msx_audio_fsca1_device::read_y8950(offs_t offset)
{
	if (BIT(offset, 1))
		return BIT(m_7fff, 1) ? m_y8950->read(offset) : 0xff;
	else
		return BIT(m_7fff, 0) ? m_y8950->read(offset) : 0xff;
}

void msx_cart_msx_audio_fsca1_device::y8950_io_w(u8 data)
{
	logerror("msx_fsca1::y8950_io_w: %02x\n", data);
}

u8 msx_cart_msx_audio_fsca1_device::y8950_io_r()
{
	return m_io_config->read();
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSX_AUDIO_HXMU900, msx_cart_interface, msx_cart_msx_audio_hxmu900_device, "msx_audio_hxmu900", "MSX Cartridge - MSX-AUDIO HX-MU900")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSX_AUDIO_NMS1205, msx_cart_interface, msx_cart_msx_audio_nms1205_device, "msx_audio_nms1205", "MSX Cartridge - MSX-AUDIO NMS-1205")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSX_AUDIO_FSCA1,   msx_cart_interface, msx_cart_msx_audio_fsca1_device,   "msx_audio_fsca1",   "MSX Cartridge - MSX-AUDIO FS-CA1")
