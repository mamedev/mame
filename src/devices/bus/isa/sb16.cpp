// license:BSD-3-Clause
// copyright-holders:Carl

// Soundblaster 16 - LLE
//
// The mcu does host communication and control of the dma-dac unit
// TODO: UART is connected to MIDI port, mixer, adc

#include "emu.h"
#include "sb16.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA16_SB16, sb16_lle_device, "sb16", "SoundBlaster 16 Audio Adapter LLE")

uint8_t sb16_lle_device::dsp_data_r()
{
	if(!machine().side_effects_disabled())
		m_data_in = false;

	return m_in_byte;
}

void sb16_lle_device::dsp_data_w(uint8_t data)
{
	m_data_out = true;
	m_out_byte = data;
}

uint8_t sb16_lle_device::dac_ctrl_r()
{
	return 0;
}

void sb16_lle_device::dac_ctrl_w(uint8_t data)
{
	/* port 0x05
	 * bit0 -
	 * bit1 - ?
	 * bit2 -
	 * bit3 -
	 * bit4 -
	 * bit5 - ?
	 * bit6 - clear irq line?
	 * bit7 -
	*/
	if(data & 0x40)
	{
		m_cpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
		m_cpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	}
}

uint8_t sb16_lle_device::adc_data_r()
{
	return 0;
}

void sb16_lle_device::dac_data_w(uint8_t data)
{
	m_ldac->write(data << 8);
	m_rdac->write(data << 8);
}

uint8_t sb16_lle_device::p1_r()
{
	uint8_t ret = 0;
	ret |= m_data_out << 0;
	ret |= m_data_in << 1;
	return ret;
}

void sb16_lle_device::p1_w(uint8_t data)
{
	/* port P1
	 * bit0 - output byte ready
	 * bit1 - input byte ready
	 * bit2 - irq mask?
	 * bit3 -
	 * bit4 - ?
	 * bit5 - DRQ?
	 * bit6 - MIDI?
	 * bit7 - ?
	*/
}

uint8_t sb16_lle_device::p2_r()
{
	return 0;
}

void sb16_lle_device::p2_w(uint8_t data)
{
	/* port P2
	 * bit0 -
	 * bit1 -
	 * bit2 -
	 * bit3 -
	 * bit4 - clock running?
	 * bit5 - ?
	 * bit6 - ?
	 * bit7 - ?
	*/
}

void sb16_lle_device::control_timer(bool start)
{
	if(start && m_freq)
	{
		double rate = ((46.61512_MHz_XTAL).dvalue()/1024/256) * m_freq;
		m_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
	}
	else
		m_timer->adjust(attotime::never);
}

void sb16_lle_device::rate_w(uint8_t data)
{
	m_freq = data;
	if(!(m_ctrl8 & 2) || !(m_ctrl16 & 2))
		control_timer(true);
}

uint8_t sb16_lle_device::dma8_r()
{
	return m_dac_fifo[0].b[0];
}

void sb16_lle_device::dma8_w(uint8_t data)
{
	m_adc_fifo[0].b[0] = data;
	m_isa->drq1_w(0);
}

uint8_t sb16_lle_device::dma_stat_r()
{
	/* port 0x06
	 * bit0 - 8 bit complete
	 * bit1 - 16 bit complete
	 * bit2 -
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 -
	 * bit7 -
	*/
	uint8_t ret = (m_dma16_done << 1) | m_dma8_done;
	return ret;
}

uint8_t sb16_lle_device::ctrl8_r()
{
	return m_ctrl8;
}

void sb16_lle_device::ctrl8_w(uint8_t data)
{
	/* port 0x08
	 * bit0 - ?
	 * bit1 - stop transfer
	 * bit2 - load counter
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 - ? (wolf3d)
	 * bit7 - toggle for 8bit irq
	*/
	if(data & 4)
	{
		m_dma8_cnt = m_dma8_len;
		if (!(BIT(m_mode, 6)))
			m_dma8_cnt >>= 1;
		m_dma8_cnt ++;
		m_dma8_done = false;
	}
	if(!(data & 2) || !(m_ctrl16 & 2))
		control_timer(true);
	if(data & 2)
	{
		m_isa->drq1_w(0);
		if(m_ctrl16 & 2)
			control_timer(false);
	}
	else
		m_isa->drq1_w(1);

	if(data & 0x80)
	{
		m_irq8 = true;
		m_isa->irq5_w(ASSERT_LINE);
	}
	m_ctrl8 = data;
}

uint8_t sb16_lle_device::ctrl16_r()
{
	return m_ctrl16;
}

void sb16_lle_device::ctrl16_w(uint8_t data)
{
	/* port 0x10
	 * bit0 -
	 * bit1 - stop transfer
	 * bit2 - load counter
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 -
	 * bit7 - toggle for 16bit irq
	*/
	if(data & 4)
	{
		m_dma16_cnt = m_dma16_len;
		if (!(BIT(m_mode, 7)))
			m_dma16_cnt >>= 1;
		m_dma16_cnt ++;
		m_dma16_done = false;
	}
	if(!(data & 2) || !(m_ctrl8 & 2))
		control_timer(true);
	if(data & 2)
	{
		m_isa->drq5_w(0);
		if(m_ctrl8 & 2)
			control_timer(false);
	}
	else
		m_isa->drq5_w(1);

	if(data & 0x80)
	{
		m_irq16 = true;
		m_isa->irq5_w(ASSERT_LINE);
	}
	m_ctrl16 = data;
}

uint8_t sb16_lle_device::dac_fifo_ctrl_r()
{
	return m_dac_fifo_ctrl;
}

void sb16_lle_device::dac_fifo_ctrl_w(uint8_t data)
{
	/* port 0x0E
	 * bit0 - reset fifo
	 * bit1 - DAC output sw off?
	 * bit2 - disable fifo
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 -
	 * bit7 -
	*/
	if(((m_dac_fifo_ctrl & 1) && !(data & 1)) || (data & 4))
	{
		m_dac_fifo_head = 1;
		m_dac_fifo_tail = 0;
		m_dac_r = false;
		m_dac_h = false;
	}
	m_mixer->dac_speaker_off_cb(BIT(data, 1));
	m_dac_fifo_ctrl = data;
}

uint8_t sb16_lle_device::adc_fifo_ctrl_r()
{
	return m_adc_fifo_ctrl;
}

void sb16_lle_device::adc_fifo_ctrl_w(uint8_t data)
{
	/* port 0x16
	 * bit0 - reset fifo
	 * bit1 - ?
	 * bit2 - disable fifo
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 -
	 * bit7 -
	*/
	if(((m_adc_fifo_ctrl & 1) && !(data & 1)) || (data & 4))
	{
		m_adc_fifo_head = 1;
		m_adc_fifo_tail = 0;
		m_adc_r = false;
		m_adc_h = false;
	}
	m_adc_fifo_ctrl = data;
}

uint8_t sb16_lle_device::mode_r()
{
	return m_mode;
}

void sb16_lle_device::mode_w(uint8_t data)
{
	/* port 0x04
	 * bit0 - 1 -- dac 16, adc 8; 0 -- adc 16, dac 8
	 * bit1 - int every sample
	 * bit2 - int dma complete
	 * bit3 -
	 * bit4 - 8 bit signed
	 * bit5 - 16 bit signed
	 * bit6 - 8 bit mono
	 * bit7 - 16 bit mono
	*/
	m_mode = data;
}

uint8_t sb16_lle_device::dma8_ready_r()
{
	/* port 0x0F
	 * bit0 -
	 * bit1 -
	 * bit2 -
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 - byte ready in fifo
	 * bit7 -
	*/
	return ((m_dac_fifo_tail - m_dac_fifo_head) != 1) << 6;
}

uint8_t sb16_lle_device::adc_data_ready_r()
{
	/* port 0x17
	 * bit0 -
	 * bit1 -
	 * bit2 -
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 -
	 * bit7 - sample ready from adc
	*/
	return (m_mode & 1) ? 0x80 : 0;
}

uint8_t sb16_lle_device::dma8_cnt_lo_r()
{
	return m_dma8_cnt & 0xff;
}

uint8_t sb16_lle_device::dma8_cnt_hi_r()
{
	return m_dma8_cnt >> 8;
}

void sb16_lle_device::dma8_len_lo_w(uint8_t data)
{
	m_dma8_len = (m_dma8_len & 0xff00) | data;
}

void sb16_lle_device::dma8_len_hi_w(uint8_t data)
{
	m_dma8_len = (m_dma8_len & 0xff) | (data << 8);
}

void sb16_lle_device::dma16_len_lo_w(uint8_t data)
{
	m_dma16_len = (m_dma16_len & 0xff00) | data;
}

void sb16_lle_device::dma16_len_hi_w(uint8_t data)
{
	m_dma16_len = (m_dma16_len & 0xff) | (data << 8);
}

ROM_START( sb16 )
	ROM_REGION( 0x2000, "sb16_cpu", 0 )
	ROM_LOAD("ct1741_v413@80c52.bin", 0x0000, 0x2000, CRC(5181892f) SHA1(5b42f1c34c4e9c8dbbdcffa0a36c178ca4f1aa77))

	ROM_REGION(0x40, "xor_table", 0)
	ROM_LOAD("ct1741_v413_xor.bin", 0x00, 0x40, CRC(5243d15a) SHA1(c7637c92828843f47e6e2f956af639b07aee4571))
ROM_END

void sb16_lle_device::sb16_io(address_map &map)
{
	map(0x0000, 0x0000).mirror(0xff00).rw(FUNC(sb16_lle_device::dsp_data_r), FUNC(sb16_lle_device::dsp_data_w));
//  map(0x0001, 0x0001) // MIDI related?
//  map(0x0002, 0x0002)
	map(0x0004, 0x0004).mirror(0xff00).rw(FUNC(sb16_lle_device::mode_r), FUNC(sb16_lle_device::mode_w));
	map(0x0005, 0x0005).mirror(0xff00).rw(FUNC(sb16_lle_device::dac_ctrl_r), FUNC(sb16_lle_device::dac_ctrl_w));
	map(0x0006, 0x0006).mirror(0xff00).r(FUNC(sb16_lle_device::dma_stat_r));
//  map(0x0007, 0x0007) // unknown, readback status of stereo f/f?
	map(0x0008, 0x0008).mirror(0xff00).rw(FUNC(sb16_lle_device::ctrl8_r), FUNC(sb16_lle_device::ctrl8_w));
	map(0x0009, 0x0009).mirror(0xff00).w(FUNC(sb16_lle_device::rate_w));
	map(0x000A, 0x000A).mirror(0xff00).r(FUNC(sb16_lle_device::dma8_cnt_lo_r));
	map(0x000B, 0x000B).mirror(0xff00).w(FUNC(sb16_lle_device::dma8_len_lo_w));
	map(0x000C, 0x000C).mirror(0xff00).w(FUNC(sb16_lle_device::dma8_len_hi_w));
	map(0x000D, 0x000D).mirror(0xff00).r(FUNC(sb16_lle_device::dma8_cnt_hi_r));
	map(0x000E, 0x000E).mirror(0xff00).rw(FUNC(sb16_lle_device::dac_fifo_ctrl_r), FUNC(sb16_lle_device::dac_fifo_ctrl_w));
	map(0x000F, 0x000F).mirror(0xff00).r(FUNC(sb16_lle_device::dma8_ready_r));
	map(0x0010, 0x0010).mirror(0xff00).rw(FUNC(sb16_lle_device::ctrl16_r), FUNC(sb16_lle_device::ctrl16_w));
	map(0x0013, 0x0013).mirror(0xff00).w(FUNC(sb16_lle_device::dma16_len_lo_w));
	map(0x0014, 0x0014).mirror(0xff00).w(FUNC(sb16_lle_device::dma16_len_hi_w));
	map(0x0016, 0x0016).mirror(0xff00).rw(FUNC(sb16_lle_device::adc_fifo_ctrl_r), FUNC(sb16_lle_device::adc_fifo_ctrl_w));
	map(0x0017, 0x0017).mirror(0xff00).r(FUNC(sb16_lle_device::adc_data_ready_r));
	map(0x0019, 0x0019).mirror(0xff00).w(FUNC(sb16_lle_device::dac_data_w));
	map(0x001B, 0x001B).mirror(0xff00).r(FUNC(sb16_lle_device::adc_data_r));
	map(0x001D, 0x001D).mirror(0xff00).w(FUNC(sb16_lle_device::dma8_w));
	map(0x001F, 0x001F).mirror(0xff00).r(FUNC(sb16_lle_device::dma8_r));
//  map(0x0080, 0x0080) // ASP comms
//  map(0x0081, 0x0081)
//  map(0x0082, 0x0082)
}

void sb16_lle_device::host_io(address_map &map)
{
	map(0x4, 0x5).rw(m_mixer, FUNC(ct1745_mixer_device::read), FUNC(ct1745_mixer_device::write));
	map(0x6, 0x7).w(FUNC(sb16_lle_device::dsp_reset_w));
	map(0xa, 0xb).r(FUNC(sb16_lle_device::host_data_r));
	map(0xc, 0xd).rw(FUNC(sb16_lle_device::dsp_wbuf_status_r), FUNC(sb16_lle_device::host_cmd_w));
	map(0xe, 0xf).r(FUNC(sb16_lle_device::dsp_rbuf_status_r));
}

const tiny_rom_entry *sb16_lle_device::device_rom_region() const
{
	return ROM_NAME( sb16 );
}

void sb16_lle_device::device_add_mconfig(machine_config &config)
{
	I80C52(config, m_cpu, XTAL(24'000'000));
	m_cpu->set_addrmap(AS_IO, &sb16_lle_device::sb16_io);
	m_cpu->port_in_cb<1>().set(FUNC(sb16_lle_device::p1_r));
	m_cpu->port_out_cb<1>().set(FUNC(sb16_lle_device::p1_w));
	m_cpu->port_in_cb<2>().set(FUNC(sb16_lle_device::p2_r));
	m_cpu->port_out_cb<2>().set(FUNC(sb16_lle_device::p2_w));

	SPEAKER(config, "speaker", 2).front();

	CT1745(config, m_mixer);
	m_mixer->set_fm_tag("ymf262");
	m_mixer->set_ldac_tag(m_ldac);
	m_mixer->set_rdac_tag(m_rdac);
	m_mixer->add_route(0, "speaker", 1.0, 0);
	m_mixer->add_route(1, "speaker", 1.0, 1);
	m_mixer->irq_status_cb().set([this] () {
		return (m_irq8 << 0) | (m_irq16 << 1) | (m_irq_midi << 2) | (0x8 << 4);
	});

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, m_mixer, 0.5, 1); // unknown DAC

	ymf262_device &ymf262(YMF262(config, "ymf262", XTAL(14'318'181)));
	ymf262.add_route(0, m_mixer, 1.00, 0);
	ymf262.add_route(1, m_mixer, 1.00, 1);
	ymf262.add_route(2, m_mixer, 1.00, 0);
	ymf262.add_route(3, m_mixer, 1.00, 1);

	PC_JOY(config, m_joy);
}

uint8_t sb16_lle_device::host_data_r()
{
	if (!machine().side_effects_disabled())
		m_data_out = false;
	return m_out_byte;
}

void sb16_lle_device::host_cmd_w(uint8_t data)
{
	m_data_in = true;
	m_in_byte = data;
}

uint8_t sb16_lle_device::dack_r(int line)
{
	uint8_t ret = m_adc_fifo[m_adc_fifo_tail].b[m_adc_h + (m_adc_r * 2)];

	if(m_ctrl8 & 2)
		return 0;

	if(!(m_mode & 1))
	{
		m_adc_h = !m_adc_h;
		if(m_adc_h)
			return ret;
	}
	if((!(m_mode & 0x40) && (m_mode & 1)) || (!(m_mode & 0x80) && !(m_mode & 1)))
	{
		m_adc_r = !m_adc_r;
		if(m_adc_r)
			return ret;
	}

	m_dma8_cnt--;
	if(!m_dma8_cnt)
	{
		m_dma8_done = true;
		if(m_mode & 4)
			m_cpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		m_isa->drq1_w(0);
		return ret;
	}

	++m_adc_fifo_tail %= FIFO_SIZE;

	if(m_adc_fifo_ctrl & 4)
	{
		m_isa->drq1_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % FIFO_SIZE))
		m_isa->drq1_w(0);
	return ret;
}

void sb16_lle_device::dack_w(int line, uint8_t data)
{
	if(m_ctrl8 & 2)
		return;

	m_dac_fifo[m_dac_fifo_head].b[m_dac_h + (m_dac_r * 2)] = data;

	if(m_mode & 1)
	{
		m_dac_h = !m_dac_h;
		if(m_dac_h)
			return;
	}
	if((!(m_mode & 0x40) && !(m_mode & 1)) || (!(m_mode & 0x80) && (m_mode & 1)))
	{
		m_dac_r = !m_dac_r;
		if(m_dac_r)
			return;
	}

	m_dma8_cnt--;
	if(!m_dma8_cnt)
	{
		m_dma8_done = true;
		if(m_mode & 4)
			m_cpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		m_isa->drq1_w(0);
		return;
	}

	++m_dac_fifo_head %= FIFO_SIZE;

	if(m_dac_fifo_ctrl & 4)
	{
		m_isa->drq1_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_isa->drq1_w(0);
}

uint16_t sb16_lle_device::dack16_r(int line)
{
	uint16_t ret = m_adc_fifo[m_adc_fifo_tail].h[m_adc_r];

	if(m_ctrl16 & 2)
		return 0;

	if(!(m_mode & 0x80))
	{
		m_adc_r = !m_adc_r;
		if(m_adc_r)
			return ret;
	}
	m_dma16_cnt--;
	if(!m_dma16_cnt)
	{
		m_dma16_done = true;
		if(m_mode & 4)
			m_cpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		m_isa->drq5_w(0);
		return ret;
	}
	++m_adc_fifo_tail %= FIFO_SIZE;

	if(m_adc_fifo_ctrl & 4)
	{
		m_isa->drq5_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % FIFO_SIZE))
		m_isa->drq5_w(0);
	return ret;
}

void sb16_lle_device::dack16_w(int line, uint16_t data)
{
	if(m_ctrl16 & 2)
		return;

	m_dac_fifo[m_dac_fifo_head].h[m_dac_r] = data;

	if(!(m_mode & 0x80))
	{
		m_dac_r = !m_dac_r;
		if(m_dac_r)
			return;
	}
	m_dma16_cnt--;
	if(!m_dma16_cnt)
	{
		m_dma16_done = true;
		if(m_mode & 4)
			m_cpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		m_isa->drq5_w(0);
		return;
	}
	++m_dac_fifo_head %= FIFO_SIZE;

	if(m_dac_fifo_ctrl & 4)
	{
		m_isa->drq5_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_isa->drq5_w(0);
}

void sb16_lle_device::dsp_reset_w(uint8_t data)
{
	if(data & 1)
	{
		device_reset();
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

uint8_t sb16_lle_device::dsp_wbuf_status_r(offs_t offset)
{
	if(offset)
		return 0xff;
	return m_data_in << 7;
}

uint8_t sb16_lle_device::dsp_rbuf_status_r(offs_t offset)
{
	if(offset)
	{
		if(!machine().side_effects_disabled())
		{
			m_irq16 = false;
			m_isa->irq5_w((m_irq8 || m_irq16 || m_irq_midi) ? ASSERT_LINE : CLEAR_LINE);
		}
		return 0xff;
	}
	if(!machine().side_effects_disabled())
	{
		m_irq8 = false;
		m_isa->irq5_w((m_irq8 || m_irq16 || m_irq_midi) ? ASSERT_LINE : CLEAR_LINE);
	}
	return m_data_out << 7;
}

// just using the old dummy mpu401 for now
uint8_t sb16_lle_device::mpu401_r(offs_t offset)
{
	uint8_t res;

	m_irq_midi = false;
	m_isa->irq5_w((m_irq8 || m_irq16 || m_irq_midi) ? ASSERT_LINE : CLEAR_LINE);
	if(offset == 0) // data
	{
		res = m_mpu_byte;
		m_mpu_byte = 0xff;
	}
	else // status
	{
		res = ((m_mpu_byte != 0xff)?0:0x80) | 0x3f; // bit 7 queue empty (DSR), bit 6 DRR (Data Receive Ready?)
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
				m_isa->irq5_w(ASSERT_LINE);
				m_irq_midi = true;
				m_mpu_byte = 0xfe;
				break;
		}
	}

}

sb16_lle_device::sb16_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SB16, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_mixer(*this, "mixer"),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac"),
	m_joy(*this, "pc_joy"),
	m_cpu(*this, "sb16_cpu"), m_data_in(false), m_in_byte(0), m_data_out(false), m_out_byte(0), m_freq(0), m_mode(0), m_dac_fifo_ctrl(0), m_adc_fifo_ctrl(0), m_ctrl8(0), m_ctrl16(0), m_mpu_byte(0),
	m_dma8_len(0), m_dma16_len(0), m_dma8_cnt(0), m_dma16_cnt(0), m_adc_fifo_head(0), m_adc_fifo_tail(0), m_dac_fifo_head(0), m_dac_fifo_tail(0), m_adc_r(false), m_dac_r(false), m_adc_h(false),
	m_dac_h(false), m_irq8(false), m_irq16(false), m_irq_midi(false), m_dma8_done(false), m_dma16_done(false), m_timer(nullptr)
{
}

void sb16_lle_device::device_start()
{
	//address_space &space = m_cpu->space(AS_PROGRAM);
	uint8_t *rom = memregion("sb16_cpu")->base();
	uint8_t *xor_table = memregion("xor_table")->base();

	for(int i = 0; i < 0x2000; i++)
		rom[i] = rom[i] ^ xor_table[i & 0x3f];


	ymf262_device &ymf262 = *subdevice<ymf262_device>("ymf262");
	set_isa_device();

	m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_isa->install_device(0x0220, 0x022f, *this, &sb16_lle_device::host_io);
	m_isa->install_device(0x0330, 0x0331, read8sm_delegate(*this, FUNC(sb16_lle_device::mpu401_r)), write8sm_delegate(*this, FUNC(sb16_lle_device::mpu401_w)));
	m_isa->install_device(0x0388, 0x038b, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	m_isa->install_device(0x0220, 0x0223, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	m_isa->install_device(0x0228, 0x0229, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	m_isa->set_dma_channel(1, this, false);
	m_isa->set_dma_channel(5, this, false);
	m_timer = timer_alloc(FUNC(sb16_lle_device::timer_tick), this);
}


void sb16_lle_device::device_reset()
{
	m_isa->drq1_w(0);
	m_isa->drq5_w(0);
	m_isa->irq5_w(0);
	m_data_out = false;
	m_data_in = false;
	m_freq = 0;
	m_mode = 0;
	m_dma8_len = m_dma16_len = 0;
	m_dma8_cnt = m_dma16_cnt = 0;
	m_ctrl8 = m_ctrl16 = 0;
	m_dac_fifo_ctrl = m_adc_fifo_ctrl = 0;
	m_adc_fifo_head = m_dac_fifo_head = 1;
	m_adc_fifo_tail = m_dac_fifo_tail = 0;
	m_dac_r = m_adc_r = false;
	m_dac_h = m_adc_h = false;
	m_irq8 = m_irq16 = m_irq_midi = false;
	m_dma8_done = m_dma16_done = false;
}

TIMER_CALLBACK_MEMBER(sb16_lle_device::timer_tick)
{
	uint16_t dacl = 0, dacr = 0, adcl = 0, adcr = 0;
	if(m_mode & 2)
	{
		// it might be possible to run the adc though dma simultaneously but the rom doesn't appear to permit it
		if(!(m_ctrl8 & 2))
			m_cpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
		return;
	}

	if(m_mode & 1)
	{
		switch(m_mode & 0xa0) // dac 16
		{
			case 0x00: // unsigned stereo
				dacl = m_dac_fifo[m_dac_fifo_tail].h[0];
				dacr = m_dac_fifo[m_dac_fifo_tail].h[1];
				break;
			case 0x20: // signed stereo
				dacl = (m_dac_fifo[m_dac_fifo_tail].h[0] ^ 0x8000);
				dacr = (m_dac_fifo[m_dac_fifo_tail].h[1] ^ 0x8000);
				break;
			case 0x80: // unsigned mono
				dacl = m_dac_fifo[m_dac_fifo_tail].h[0];
				dacr = m_dac_fifo[m_dac_fifo_tail].h[0];
				break;
			case 0xa0: // signed mono
				dacl = (m_dac_fifo[m_dac_fifo_tail].h[0] ^ 0x8000);
				dacr = (m_dac_fifo[m_dac_fifo_tail].h[0] ^ 0x8000);
				break;
		}
		switch(m_mode & 0x50) // adc 8; placeholder
		{
			case 0x00: // unsigned stereo
				adcl = 0;
				adcr = 0;
				break;
			case 0x10: // signed stereo
				adcl = 0;
				adcr = 0;
				break;
			case 0x40: // unsigned mono
				adcl = 0;
				adcr = 0;
				break;
			case 0x50: // signed mono
				adcl = 0;
				adcr = 0;
				break;
		}
	}
	else
	{
		switch(m_mode & 0x50) // dac 8
		{
			case 0x00: // unsigned stereo
				dacl = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
				dacr = m_dac_fifo[m_dac_fifo_tail].b[2] << 8;
				break;
			case 0x10: // signed stereo
				dacl = (m_dac_fifo[m_dac_fifo_tail].b[0] ^ 0x80) << 8;
				dacr = (m_dac_fifo[m_dac_fifo_tail].b[2] ^ 0x80) << 8;
				break;
			case 0x40: // unsigned mono
				dacl = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
				dacr = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
				break;
			case 0x50: // signed mono
				dacl = (m_dac_fifo[m_dac_fifo_tail].b[0] ^ 0x80) << 8;
				dacr = (m_dac_fifo[m_dac_fifo_tail].b[0] ^ 0x80) << 8;
				break;
		}
		switch(m_mode & 0xa0) // adc 16; placeholder
		{
			case 0x00: // unsigned stereo
				adcl = 0;
				adcr = 0;
				break;
			case 0x20: // signed stereo
				adcl = 0;
				adcr = 0;
				break;
			case 0x80: // unsigned mono
				adcl = 0;
				adcr = 0;
				break;
			case 0xa0: // signed mono
				adcl = 0;
				adcr = 0;
				break;
		}
	}
	m_rdac->write(dacr);
	m_ldac->write(dacl);

	if(!(m_ctrl8 & 2))
		m_isa->drq1_w(1);

	if(!(m_ctrl16 & 2))
		m_isa->drq5_w(1);

	if((!(m_ctrl8 & 2) && !(m_mode & 1)) || (!(m_ctrl16 & 2) && (m_mode & 1)))
		++m_dac_fifo_tail %= FIFO_SIZE;

	if((!(m_ctrl8 & 2) && (m_mode & 1)) || (!(m_ctrl16 & 2) && !(m_mode & 1)))
	{
		m_adc_fifo[m_adc_fifo_head].h[0] = adcl;
		m_adc_fifo[m_adc_fifo_head].h[1] = adcr;
		++m_adc_fifo_head %= FIFO_SIZE;
	}
}
