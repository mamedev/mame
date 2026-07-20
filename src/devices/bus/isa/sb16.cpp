// license:BSD-3-Clause
// copyright-holders:Carl

// Soundblaster 16 - ISA LLE variant
//
/*
 * TODO:
 * - UART is connected to MIDI port, mixer, adc
 * - jagdead: Gus Tarballs utterances randomly drifts (enables both DMA controls)
 * - tentacle: misses voice playback quite often (voice mixing?)
 * - guimo: acts weird with sound detail and frequency (really a PIT bug?)
 * - Check dack DMAs to be against the intended line;
 *
 */

#include "emu.h"
#include "sb16.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA16_SB16, sb16_lle_device, "sb16", "SoundBlaster 16 Audio Adapter LLE")

void sb16_lle_device::host_io(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
	map(0x04, 0x05).rw(m_mixer, FUNC(ct1745_mixer_device::read), FUNC(ct1745_mixer_device::write));
	map(0x06, 0x07).w(m_dsp, FUNC(ct1741_dsp_device::dsp_reset_w));
	map(0x08, 0x09).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
	map(0x0a, 0x0b).r(m_dsp, FUNC(ct1741_dsp_device::host_data_r));
	map(0x0c, 0x0d).rw(m_dsp, FUNC(ct1741_dsp_device::dsp_wbuf_status_r), FUNC(ct1741_dsp_device::host_cmd_w));
	map(0x0e, 0x0f).r(m_dsp, FUNC(ct1741_dsp_device::dsp_rbuf_status_r));
//  map(0x10, 0x13) CD-ROM interface
}

void sb16_lle_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	CT1745(config, m_mixer);
	m_mixer->set_fm_tag(m_opl3);
	m_mixer->set_ldac_tag(m_ldac);
	m_mixer->set_rdac_tag(m_rdac);
	m_mixer->add_route(0, "speaker", 1.0, 0);
	m_mixer->add_route(1, "speaker", 1.0, 1);
	m_mixer->irq_status_cb().set([this] () {
		return (m_irq8 << 0) | (m_irq16 << 1) | (m_irq_midi << 2) | (0x8 << 4);
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
		m_isa->drq1_w(state);
	});
	m_dsp->drq16_cb().set([this] (int state) {
		m_isa->drq5_w(state);
	});
	m_dsp->speaker_off_cb().set(m_mixer, FUNC(ct1745_mixer_device::dac_speaker_off_cb));


	// TODO: PnP line
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set([this](int state) {m_isa->irq5_w(state ? ASSERT_LINE : CLEAR_LINE); });

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 1); // unknown DAC

	YMF262(config, m_opl3, XTAL(14'318'181));
	m_opl3->add_route(0, m_mixer, 1.00, 0);
	m_opl3->add_route(1, m_mixer, 1.00, 1);
	m_opl3->add_route(2, m_mixer, 1.00, 0);
	m_opl3->add_route(3, m_mixer, 1.00, 1);

	PC_JOY(config, m_joy);
}

u8 sb16_lle_device::dack_r(int line)
{
	return m_dsp->dack_r();
}

void sb16_lle_device::dack_w(int line, u8 data)
{
	m_dsp->dack_w(data);
}

uint16_t sb16_lle_device::dack16_r(int line)
{
	return m_dsp->dack16_r();
}

void sb16_lle_device::dack16_w(int line, uint16_t data)
{
	m_dsp->dack16_w(data);
}

// just using the old dummy mpu401 for now
uint8_t sb16_lle_device::mpu401_r(offs_t offset)
{
	uint8_t res;

	if (!machine().side_effects_disabled())
	{
		m_irq_midi = false;
		m_irqs->in_w<2>(CLEAR_LINE);
	}
	if(offset == 0) // data
	{
		res = m_mpu_byte;
		if (!machine().side_effects_disabled())
			m_mpu_byte = 0xff;
	}
	else // status
	{
		// bit 7 queue empty (DSR), bit 6 DRR (Data Receive Ready?)
		res = ((m_mpu_byte != 0xff) ? 0 : 0x80) | 0x3f;
	}

	return res;
}

void sb16_lle_device::mpu401_w(offs_t offset, uint8_t data)
{
	if(offset == 0) // data
	{
		logerror("SB MPU401:%02x %02x\n",offset,data);
	}
	else // command
	{
		logerror("SB MPU401:%02x %02x\n",offset,data);

		switch(data)
		{
			case 0xff: // reset
				m_irq_midi = true;
				m_irqs->in_w<2>(ASSERT_LINE);
				m_mpu_byte = 0xfe;
				break;
		}
	}

}

sb16_lle_device::sb16_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SB16, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_opl3(*this, "opl3"),
	m_dsp(*this, "dsp"),
	m_mixer(*this, "mixer"),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac"),
	m_irqs(*this, "irqs"),
	m_joy(*this, "pc_joy"),
	m_mpu_byte(0),
	m_irq8(false),
	m_irq16(false),
	m_irq_midi(false)
{
}

void sb16_lle_device::device_start()
{
	set_isa_device();

	m_isa->set_dma_channel(1, this, false);
	m_isa->set_dma_channel(5, this, false);

	save_item(NAME(m_irq8));
	save_item(NAME(m_irq16));
	save_item(NAME(m_irq_midi));
	save_item(NAME(m_mpu_byte));
}


void sb16_lle_device::device_reset()
{
	m_isa->drq1_w(0);
	m_isa->drq5_w(0);
	m_isa->irq5_w(0);

	m_irq8 = m_irq16 = m_irq_midi = false;
	m_irqs->in_w<0>(0);
	m_irqs->in_w<1>(0);
	m_irqs->in_w<2>(0);
	remap(AS_IO, 0, 0xffff);
}

void sb16_lle_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
		m_isa->install_device(0x0220, 0x023f, *this, &sb16_lle_device::host_io);
		m_isa->install_device(0x0330, 0x0331, read8sm_delegate(*this, FUNC(sb16_lle_device::mpu401_r)), write8sm_delegate(*this, FUNC(sb16_lle_device::mpu401_w)));
		m_isa->install_device(0x0388, 0x038b, read8sm_delegate(*m_opl3, FUNC(ymf262_device::read)), write8sm_delegate(*m_opl3, FUNC(ymf262_device::write)));
	}
}
