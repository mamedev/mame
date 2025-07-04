// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Creative Sound Blaster 16 for PC-9800 Series (board name: CT2720)

References:
- https://j02.nobody.jp/jto98/n_desk_sound/ct2720.htm
- http://retropc.net/yasuma/V2/PC/SOUND/ct2720.html

TODO:
- Optional YM2203 (+ ROM socket), PC-9801-26K equivalent fallback;
- Game port;
- MIDI;
- CT1741 (DSP);
- CT1745 (Mixer);
- CD-Rom interface/CD-In;
- Mic-In/Line-In;
- Configuration dips;

**************************************************************************************************/

#include "emu.h"
#include "bus/cbus/sb16_ct2720.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(SB16_CT2720, sb16_ct2720_device, "sb16_ct2720", "Creative Sound Blaster 16 CT-2720")

sb16_ct2720_device::sb16_ct2720_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SB16_CT2720, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_opl3(*this, "opl3")
	, m_mixer(*this, "mixer")
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
{
}

void sb16_ct2720_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	CT1745(config, m_mixer);
	m_mixer->set_fm_tag(m_opl3);
	m_mixer->set_ldac_tag(m_ldac);
	m_mixer->set_rdac_tag(m_rdac);
	m_mixer->add_route(0, "speaker", 1.0, 0);
	m_mixer->add_route(1, "speaker", 1.0, 1);
	m_mixer->irq_status_cb().set([this] () {
		(void)this;
		return 0;
		//return (m_irq8 << 0) | (m_irq16 << 1) | (m_irq_midi << 2) | (0x8 << 4);
	});

//  PC_JOY(config, m_joy);

//  MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(sb_device::midi_rx_w));
//  MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 1); // unknown DAC

	YMF262(config, m_opl3, XTAL(14'318'181));
	m_opl3->add_route(0, m_mixer, 1.00, 0);
	m_opl3->add_route(1, m_mixer, 1.00, 1);
	m_opl3->add_route(2, m_mixer, 1.00, 0);
	m_opl3->add_route(3, m_mixer, 1.00, 1);
}

void sb16_ct2720_device::device_start()
{
	m_bus->install_device(0x0000, 0xffff, *this, &sb16_ct2720_device::io_map);
}

void sb16_ct2720_device::device_reset()
{
}

void sb16_ct2720_device::io_map(address_map &map)
{
	const u16 base = 0xd2;
//  map(0x0400 | base, 0x0400 | base).select(0x300) PC Gameport
	map(0x2000 | base, 0x2000 | base).select(0x300).lrw8(
		NAME([this] (offs_t offset) {
			return m_opl3->read(offset >> 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_opl3->write(offset >> 8, data);
		})
	);
	// ($224-$225 on AT)
	map(0x2400 | base, 0x2400 | base).select(0x100).lrw8(
		NAME([this] (offs_t offset) {
			return m_mixer->read(offset >> 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_mixer->write(offset >> 8, data);
		})
	);
//  map(0x2600 | base, 0x2600 | base) CT1741 DSP reset
	map(0x2800 | base, 0x2800 | base).select(0x100).lrw8(
		NAME([this] (offs_t offset) {
			return m_opl3->read(offset >> 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_opl3->write(offset >> 8, data);
		})
	);
//  map(0x2a00 | base, 0x2a00 | base) CT1741 DSP read data port
//  map(0x2c00 | base, 0x2c00 | base).select(0x300) CT1741 DSP ($22c-$22f on AT)
//  map(0x8000 | base, 0x8000 | base).select(0x100) MIDI port/Wave Blaster
	map(0xc800 | base, 0xc800 | base).select(0x300).lrw8(
		NAME([this] (offs_t offset) {
			return m_opl3->read(offset >> 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_opl3->write(offset >> 8, data);
		})
	);
}
