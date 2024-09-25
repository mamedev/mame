// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************

Yamaha SFG01/SFG05 emulation

**************************************************************************/

#include "emu.h"
#include "sfg.h"

#include "bus/midi/midi.h"
#include "bus/msx/cart/msx_audio_kb.h"
#include "machine/ym2148.h"
#include "sound/ymopm.h"

#include "speaker.h"


namespace {

class msx_cart_sfg_device : public device_t, public msx_cart_interface
{
protected:
	msx_cart_sfg_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	IRQ_CALLBACK_MEMBER(irq_callback);

	void ym2151_irq_w(int state);
	void ym2148_irq_w(int state);

	void check_irq();

	required_memory_region m_region_sfg;
	required_device<ym_generic_device> m_ym2151;
	required_device<msx_audio_kbdc_port_device> m_kbdc;
	required_device<ym2148_device> m_ym2148;
	int m_ym2151_irq_state;
	int m_ym2148_irq_state;
	u32 m_rom_mask;
};

msx_cart_sfg_device::msx_cart_sfg_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock)
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

void msx_cart_sfg_device::device_add_mconfig(machine_config &config)
{
	// YM2151 (OPM)
	// YM3012 (DAC)
	// YM2148 (MKS)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ym2151_device &ym2151(YM2151(config, m_ym2151, DERIVED_CLOCK(1, 1)));  // The SFG01 uses a YM2151, the SFG05 uses a YM2164, input clock comes from the main cpu frequency
	ym2151.irq_handler().set(FUNC(msx_cart_sfg_device::ym2151_irq_w));
	ym2151.add_route(0, "lspeaker", 0.80);
	ym2151.add_route(1, "rspeaker", 0.80);

	YM2148(config, m_ym2148, XTAL(4'000'000));
	m_ym2148->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_ym2148->port_write_handler().set("kbdc", FUNC(msx_audio_kbdc_port_device::write));
	m_ym2148->port_read_handler().set("kbdc", FUNC(msx_audio_kbdc_port_device::read));
	m_ym2148->irq_handler().set(FUNC(msx_cart_sfg_device::ym2148_irq_w));

	MSX_AUDIO_KBDC_PORT(config, m_kbdc, msx_audio_keyboards, nullptr);

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set("ym2148", FUNC(ym2148_device::write_rxd));
}

void msx_cart_sfg_device::device_start()
{
	// Set rom mask
	m_rom_mask = m_region_sfg->bytes() - 1;

	maincpu().set_irq_acknowledge_callback(*this, FUNC(msx_cart_sfg_device::irq_callback));

	page(0)->install_rom(0x0000, 0x3fff, m_region_sfg->base());
	if (m_region_sfg->bytes() == 0x8000)
	{
		page(1)->install_rom(0x4000, 0x7fff, m_region_sfg->base() + 0x4000);
	}
	else
	{
		page(1)->install_rom(0x4000, 0x7fff, m_region_sfg->base());
	}

	for (int i = 0; i < 4; i++)
	{
		// These addresses deliberately overlap
		page(i)->install_read_handler(0x4000 * i + 0x3ff0, 0x4000 * i + 0x3ff6, emu::rw_delegate(*m_ym2148, FUNC(ym2148_device::read)));
		page(i)->install_read_handler(0x4000 * i + 0x3ff0, 0x4000 * i + 0x3ff1, emu::rw_delegate(*m_ym2151, FUNC(ym_generic_device::read)));
		page(i)->install_write_handler(0x4000 * i + 0x3ff0, 0x4000 * i + 0x3ff6, emu::rw_delegate(*m_ym2148, FUNC(ym2148_device::write)));
		page(i)->install_write_handler(0x4000 * i + 0x3ff0, 0x4000 * i + 0x3ff1, emu::rw_delegate(*m_ym2151, FUNC(ym_generic_device::write)));
	}
}

IRQ_CALLBACK_MEMBER(msx_cart_sfg_device::irq_callback)
{
	return m_ym2148->get_irq_vector();
}

void msx_cart_sfg_device::ym2151_irq_w(int state)
{
	m_ym2151_irq_state = state ? ASSERT_LINE : CLEAR_LINE;
	check_irq();
}

void msx_cart_sfg_device::ym2148_irq_w(int state)
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


class msx_cart_sfg01_device : public msx_cart_sfg_device
{
public:
	msx_cart_sfg01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

msx_cart_sfg01_device::msx_cart_sfg01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_cart_sfg_device(mconfig, MSX_CART_SFG01, tag, owner, clock)
{
}

ROM_START(msx_sfg01)
	ROM_REGION(0x4000, "sfg", 0)
	ROM_LOAD("sfg01.rom", 0x0, 0x4000, CRC(0995fb36) SHA1(434651305f92aa770a89e40b81125fb22d91603d)) // correct label is almost certainly "yamaha__ym2211-22702__48_18_89_b.ic104" though the datecode portion may vary between late 1983 and mid 1984
ROM_END

const tiny_rom_entry *msx_cart_sfg01_device::device_rom_region() const
{
	return ROM_NAME(msx_sfg01);
}



class msx_cart_sfg05_device : public msx_cart_sfg_device
{
public:
	msx_cart_sfg05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

msx_cart_sfg05_device::msx_cart_sfg05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_cart_sfg_device(mconfig, MSX_CART_SFG05, tag, owner, clock)
{
}

void msx_cart_sfg05_device::device_add_mconfig(machine_config &config)
{
	msx_cart_sfg_device::device_add_mconfig(config);

	ym2164_device &ym2164(YM2164(config.replace(), m_ym2151, DERIVED_CLOCK(1, 1)));
	ym2164.irq_handler().set(FUNC(msx_cart_sfg05_device::ym2151_irq_w));
	ym2164.add_route(0, "lspeaker", 0.80);
	ym2164.add_route(1, "rspeaker", 0.80);
}

ROM_START(msx_sfg05)
	ROM_REGION(0x8000, "sfg", 0)
	// Version string starts at $02BD
	ROM_SYSTEM_BIOS(0, "m5.01.011", "SFG05 (original) M5.01.011")
	ROMX_LOAD( "sfg05.rom", 0x0, 0x8000, CRC(2425c279) SHA1(d956167e234f60ad916120437120f86fc8c3c321), ROM_BIOS(0)) // correct label MIGHT be "yamaha__ym2301-23959.ic104" but this needs redump/verification
	ROM_SYSTEM_BIOS(1, "m5.00.013", "SFG05 (SFG01 upgrade) M5.00.013") // SFG01 PCB, Yamaha official upgrade, has YM2151 instead of YM2164
	ROMX_LOAD( "sfg05a.rom", 0x0, 0x8000, CRC(5bc237f8) SHA1(930338f45c08228108c0831cc4a26014c2674718), ROM_BIOS(1)) // this came on a single eprom on a daughterboard on an SFG01 board which had been factory upgraded to SFG05
	ROM_SYSTEM_BIOS(2, "m5.00.011", "SFG05 M5.00.011") // Found in Sakhr AX-200M
	ROMX_LOAD("sfg05_m5_00_011.rom", 0x0, 0x8000, BAD_DUMP CRC(9d5e20c9) SHA1(fcc385b90c65575e29fc009aa00b5120fc4c251a), ROM_BIOS(2)) // Still seems to have I/O reads at $7ff0 - $7fff
ROM_END

const tiny_rom_entry *msx_cart_sfg05_device::device_rom_region() const
{
	return ROM_NAME(msx_sfg05);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SFG01, msx_cart_interface, msx_cart_sfg01_device, "msx_cart_sfg01", "MSX Cartridge - SFG01")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SFG05, msx_cart_interface, msx_cart_sfg05_device, "msx_cart_sfg05", "MSX Cartridge - SFG05")
