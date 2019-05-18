// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************

Yamaha SFG01/SFG05 emulation

TODO:
- Use a real YM2164 implementation for SFG05

**************************************************************************/

#include "emu.h"
#include "yamaha.h"

#include "bus/midi/midi.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(MSX_CART_SFG01, msx_cart_sfg01_device, "msx_cart_sfg01", "MSX Cartridge - SFG01")
DEFINE_DEVICE_TYPE(MSX_CART_SFG05, msx_cart_sfg05_device, "msx_cart_sfg05", "MSX Cartridge - SFG05")


msx_cart_sfg_device::msx_cart_sfg_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_region_sfg(*this, "sfg")
	, m_ym2151(*this, "ym2151")
	, m_kbdc(*this, "kbdc")
	, m_ym2148(*this, "ym2148")
	, m_ym2151_irq_state(CLEAR_LINE)
	, m_ym2148_irq_state(CLEAR_LINE)
	, m_rom_mask(0)
{
}


msx_cart_sfg01_device::msx_cart_sfg01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_cart_sfg_device(mconfig, MSX_CART_SFG01, tag, owner, clock)
{
}


msx_cart_sfg05_device::msx_cart_sfg05_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_cart_sfg_device(mconfig, MSX_CART_SFG05, tag, owner, clock)
{
}


void msx_cart_sfg_device::device_add_mconfig(machine_config &config)
{
	// YM2151 (OPM)
	// YM3012 (DAC)
	// YM2148 (MKS)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	YM2151(config, m_ym2151, XTAL(3'579'545));  // The SFG01 uses a YM2151, the SFG05 uses a YM2164, input clock comes from the main cpu frequency
	m_ym2151->irq_handler().set(FUNC(msx_cart_sfg_device::ym2151_irq_w));
	m_ym2151->add_route(0, "lspeaker", 0.80);
	m_ym2151->add_route(1, "rspeaker", 0.80);

	YM2148(config, m_ym2148, XTAL(4'000'000));
	m_ym2148->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_ym2148->port_write_handler().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_ym2148->port_read_handler().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_ym2148->irq_handler().set(FUNC(msx_cart_sfg_device::ym2148_irq_w));

	MSX_AUDIO_KBDC_PORT(config, m_kbdc, msx_audio_keyboards, nullptr);

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set("ym2148", FUNC(ym2148_device::write_rxd));
}


ROM_START( msx_sfg01 )
	ROM_REGION(0x4000, "sfg", 0)
	ROM_LOAD("sfg01.rom", 0x0, 0x4000, CRC(0995fb36) SHA1(434651305f92aa770a89e40b81125fb22d91603d))
ROM_END


const tiny_rom_entry *msx_cart_sfg01_device::device_rom_region() const
{
	return ROM_NAME( msx_sfg01 );
}


ROM_START( msx_sfg05 )
	ROM_REGION(0x8000, "sfg", 0)
	ROM_SYSTEM_BIOS( 0, "sfg05", "SFG05 (original)" )
	ROMX_LOAD( "sfg05.rom", 0x0, 0x8000, CRC(2425c279) SHA1(d956167e234f60ad916120437120f86fc8c3c321), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "sfg05a", "SFG05 (SFG01 upgrade)" ) // SFG01 PCB, Yamaha official upgrade, has YM2151 instead of YM2164
	ROMX_LOAD( "sfg05a.rom", 0x0, 0x8000, CRC(5bc237f8) SHA1(930338f45c08228108c0831cc4a26014c2674718), ROM_BIOS(1) )
ROM_END


const tiny_rom_entry *msx_cart_sfg05_device::device_rom_region() const
{
	return ROM_NAME( msx_sfg05 );
}


void msx_cart_sfg_device::device_start()
{
	// Set rom mask
	m_rom_mask = m_region_sfg->bytes() - 1;

	// This should probably moved up in the bus/slot hierarchy for the msx driver
	cpu_device *maincpu = machine().device<cpu_device>("maincpu");
	maincpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(msx_cart_sfg_device::irq_callback),this));
}


IRQ_CALLBACK_MEMBER(msx_cart_sfg_device::irq_callback)
{
	return m_ym2148->get_irq_vector();
}


WRITE_LINE_MEMBER(msx_cart_sfg_device::ym2151_irq_w)
{
	m_ym2151_irq_state = state ? ASSERT_LINE : CLEAR_LINE;
	check_irq();
}


WRITE_LINE_MEMBER(msx_cart_sfg_device::ym2148_irq_w)
{
	m_ym2148_irq_state = state ? ASSERT_LINE : CLEAR_LINE;
	check_irq();
}


void msx_cart_sfg_device::check_irq()
{
	if (m_ym2151_irq_state != CLEAR_LINE || m_ym2148_irq_state != CLEAR_LINE)
	{
		irq_out(ASSERT_LINE);
	}
	else
	{
		irq_out(CLEAR_LINE);
	}
}


uint8_t msx_cart_sfg_device::read_cart(offs_t offset)
{
	switch (offset & 0x3fff)
	{
		case 0x3ff0:     // YM-2151 status read
		case 0x3ff1:     // YM-2151 status read mirror?
			return m_ym2151->status_r();

		case 0x3ff2:     // YM-2148 keyboard column read
		case 0x3ff3:     // YM-2148 --
		case 0x3ff4:     // YM-2148 --
		case 0x3ff5:     // YM-2148 MIDI UART data read register
		case 0x3ff6:     // YM-2148 MIDI UART status register
							// ------x- - 1 = received a byte/receive buffer full?
							// -------x - 1 = ready to send next byte/send buffer empty?
			return m_ym2148->read(offset & 7);
	}

	if (offset < 0x8000)
	{
		return m_region_sfg->as_u8(offset & m_rom_mask);
	}

	return 0xff;
}


void msx_cart_sfg_device::write_cart(offs_t offset, uint8_t data)
{
	switch (offset & 0x3fff)
	{
		case 0x3ff0:     // YM-2151 register
			m_ym2151->register_w(data);
			break;

		case 0x3ff1:    // YM-2151 data
			m_ym2151->data_w(data);
			break;

		case 0x3ff2:   // YM-2148 write keyboard row
		case 0x3ff3:   // YM-2148 MIDI IRQ vector
		case 0x3ff4:   // YM-2148 External IRQ vector
		case 0x3ff5:   // YM-2148 MIDI UART data write register
		case 0x3ff6:   // YM-2148 MIDI UART command register
						// On startup the sfg01 writes 0x80
						// followed by 0x05.
						// Other write seen in the code: 0x15
						//
						// x------- - 1 = reset
						// -----x-- - 1 = enable receiving / sending midi data
						// -------x - 1 = enable receiving / sending midi data
			m_ym2148->write(offset & 7, data);
			break;

		default:
			logerror("msx_cart_sfg_device::write_cart: write %02x to %04x\n", data, offset);
			break;
	}
}
