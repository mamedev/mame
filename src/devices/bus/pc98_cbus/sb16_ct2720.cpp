// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Creative Sound Blaster 16 for PC-9800 Series (board name: CT2720)

References:
- https://j02.nobody.jp/jto98/n_desk_sound/ct2720.htm
- http://retropc.net/yasuma/V2/PC/SOUND/ct2720.html

TODO:
- Optional YM2203 (+ ROM socket), PC-9801-26K equivalent fallback;
- MIDI;
- CT1745 (Mixer);
- CD-Rom interface/CD-In;
- Mic-In/Line-In;
- Configuration dips;
- Doesn't play well with hardcoded FDC DRQ3 in base driver (use DRQ0 for now);
- wolf3d: fails identification purely from OPL3
\- Writes 0x60 -> 0x80 to reg $04 (mask timer A and B, send a reset to FM)
\- loads OPL3 timer A with a 0xff in reg $02 (max time?)
\- sets OPL3 timer reg $04 -> 0x21 (enable timer A, mask timer B)
\- bp 13e51b pretends to read it back as res & 0xe0 == 0xc0
   (irq and timer A statuses high, timer B low)
\- does plenty of non-SB16 accesses at $28d1, for delay?

Notes:
- For debugging DSP from debugger (and convert to AT style):
wpiset 0x2c00,0x400,w,(wpaddr & 0xff) == 0xd2,{printf "%04x %02x",((wpaddr >> 8) & 0xf) + 0x220,wpdata;g}

wpiset 0x2c00,0x400,r,(wpaddr & 0xff) == 0xd2,{printf "%04x R",((wpaddr >> 8) & 0x0f) + 0x220;g}


**************************************************************************************************/

#include "emu.h"
#include "sb16_ct2720.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(SB16_CT2720, sb16_ct2720_device, "sb16_ct2720", "Creative Sound Blaster 16 CT-2720")

sb16_ct2720_device::sb16_ct2720_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SB16_CT2720, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_opl3(*this, "opl3")
	, m_dsp(*this, "dsp")
	, m_mixer(*this, "mixer")
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
	, m_irqs(*this, "irqs")
	, m_joy(*this, "pc_joy")
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
		//return (m_irq8 << 0) | (m_irq16 << 1) | (m_irq_midi << 2) | (0x8 << 4);
		return (m_irq8 << 0) | (m_irq16 << 1) | (0x8 << 4);
	});

	CT1741(config, m_dsp, XTAL(24'000'000));
	m_dsp->ldac_write_cb().set(m_ldac, FUNC(dac_16bit_r2r_device::write));
	m_dsp->rdac_write_cb().set(m_rdac, FUNC(dac_16bit_r2r_device::write));
	m_dsp->irq8_cb().set([this] (int state) {
		m_irq8 = state;
		m_irqs->in_w<0>(state);
	});
	m_dsp->irq16_cb().set([this] (int state) {
		m_irq16 = state;
		m_irqs->in_w<1>(state);
	});
	m_dsp->drq8_cb().set([this] (int state) {
		m_bus->drq_w(0, state);
	});
	m_dsp->drq16_cb().set([this] (int state) {
		if (state)
			popmessage("sb16_ct2720.cpp: DMA16 high on C-Bus?\n");
	});
	m_dsp->speaker_off_cb().set(m_mixer, FUNC(ct1745_mixer_device::dac_speaker_off_cb));

	// TODO: PnP line
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set([this](int state) { m_bus->int_w(1, state); });


	PC_JOY(config, m_joy);
	// doom can set this up, but generally for PC-98 we want it opt-in really
	m_joy->set_default_option(nullptr);

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
	m_bus->set_dma_channel(0, this, false);

	save_item(NAME(m_irq8));
	save_item(NAME(m_irq16));
}

void sb16_ct2720_device::device_reset()
{
}

void sb16_ct2720_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &sb16_ct2720_device::io_map);
	}
}



void sb16_ct2720_device::io_map(address_map &map)
{
	const u16 base = 0xd2;
	// ($200-$207 on AT)
	map(0x0400 | base, 0x0400 | base).select(0x300).rw(m_joy, FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
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
	map(0x2600 | base, 0x2600 | base).w(m_dsp, FUNC(ct1741_dsp_device::dsp_reset_w));
	map(0x2800 | base, 0x2800 | base).select(0x100).lrw8(
		NAME([this] (offs_t offset) {
			return m_opl3->read(offset >> 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_opl3->write(offset >> 8, data);
		})
	);
	map(0x2a00 | base, 0x2a00 | base).r(m_dsp, FUNC(ct1741_dsp_device::host_data_r));
	// $22c-$22f on AT
	map(0x2c00 | base, 0x2c00 | base).select(0x100).rw(m_dsp, FUNC(ct1741_dsp_device::dsp_wbuf_status_r), FUNC(ct1741_dsp_device::host_cmd_w));
	map(0x2e00 | base, 0x2e00 | base).select(0x100).r(m_dsp, FUNC(ct1741_dsp_device::dsp_rbuf_status_r));

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

u8 sb16_ct2720_device::dack_r(int line)
{
	return m_dsp->dack_r();
}

void sb16_ct2720_device::dack_w(int line, u8 data)
{
	m_dsp->dack_w(data);
}
