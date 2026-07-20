// license:BSD-3-Clause
// copyright-holders:Carl, Angelo Salese
/**************************************************************************************************

Creative Labs CT1741 DSP

The MCU does host communication and control of the dma-dac unit

**************************************************************************************************/

#include "emu.h"
#include "ct1741.h"


DEFINE_DEVICE_TYPE(CT1741, ct1741_dsp_device, "ct1741", "Creative Labs CT1741 SB16 DSP Sound Unit")

ct1741_dsp_device::ct1741_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CT1741, tag, owner, clock)
	, m_cpu(*this, "sb16_cpu")
	, m_ldac_w(*this)
	, m_rdac_w(*this)
	, m_irq8_w(*this)
	, m_irq16_w(*this)
	, m_drq8_w(*this)
	, m_drq16_w(*this)
	, m_speaker_off_w(*this)
	, m_data_in(false), m_in_byte(0), m_data_out(false), m_out_byte(0), m_freq(0), m_mode(0), m_dac_fifo_ctrl(0), m_adc_fifo_ctrl(0), m_ctrl8(0), m_ctrl16(0),
	m_dma8_len(0), m_dma16_len(0), m_dma8_cnt(0), m_dma16_cnt(0), m_adc_fifo_head(0), m_adc_fifo_tail(0), m_dac_fifo_head(0), m_dac_fifo_tail(0), m_adc_r(false), m_dac_r(false), m_adc_h(false),
	m_dac_h(false), m_dma8_done(false), m_dma16_done(false), m_timer(nullptr)
{
}

ROM_START( ct1741 )
	ROM_REGION( 0x2000, "sb16_cpu", 0 )
	ROM_LOAD("ct1741_v413@80c52.bin", 0x0000, 0x2000, CRC(5181892f) SHA1(5b42f1c34c4e9c8dbbdcffa0a36c178ca4f1aa77))

	ROM_REGION(0x40, "xor_table", 0)
	ROM_LOAD("ct1741_v413_xor.bin", 0x00, 0x40, CRC(5243d15a) SHA1(c7637c92828843f47e6e2f956af639b07aee4571))
ROM_END


const tiny_rom_entry *ct1741_dsp_device::device_rom_region() const
{
	return ROM_NAME( ct1741 );
}

void ct1741_dsp_device::device_add_mconfig(machine_config &config)
{
	I80C52(config, m_cpu, this->clock());
	m_cpu->set_addrmap(AS_DATA, &ct1741_dsp_device::data_map);
	m_cpu->port_in_cb<1>().set(FUNC(ct1741_dsp_device::p1_r));
	m_cpu->port_out_cb<1>().set(FUNC(ct1741_dsp_device::p1_w));
	m_cpu->port_in_cb<2>().set(FUNC(ct1741_dsp_device::p2_r));
	m_cpu->port_out_cb<2>().set(FUNC(ct1741_dsp_device::p2_w));
	m_cpu->port_in_cb<3>().set_constant(0xff); // spammy
}

void ct1741_dsp_device::device_start()
{
	//address_space &space = m_cpu->space(AS_PROGRAM);
	uint8_t *rom = memregion("sb16_cpu")->base();
	uint8_t *xor_table = memregion("xor_table")->base();

	for(int i = 0; i < 0x2000; i++)
		rom[i] = rom[i] ^ xor_table[i & 0x3f];

	m_timer = timer_alloc(FUNC(ct1741_dsp_device::timer_tick), this);

	save_item(NAME(m_data_in));
	save_item(NAME(m_in_byte));
	save_item(NAME(m_data_out));
	save_item(NAME(m_out_byte));

	save_item(NAME(m_freq));
	save_item(NAME(m_mode));
	save_item(NAME(m_dac_fifo_ctrl));
	save_item(NAME(m_adc_fifo_ctrl));
	save_item(NAME(m_ctrl8));
	save_item(NAME(m_ctrl16));
	save_item(NAME(m_dma8_len));
	save_item(NAME(m_dma16_len));
	save_item(NAME(m_dma8_cnt));
	save_item(NAME(m_dma16_cnt));
}

void ct1741_dsp_device::device_reset()
{
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
	m_dma8_done = m_dma16_done = false;

//	m_timer->adjust(attotime::never);

	m_irq8_w(CLEAR_LINE);
	m_irq16_w(CLEAR_LINE);
	m_drq8_w(CLEAR_LINE);
	m_drq16_w(CLEAR_LINE);
}


void ct1741_dsp_device::data_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0xff00).rw(FUNC(ct1741_dsp_device::dsp_data_r), FUNC(ct1741_dsp_device::dsp_data_w));
//  map(0x0001, 0x0001) // MIDI related?
//  map(0x0002, 0x0002)
	map(0x0004, 0x0004).mirror(0xff00).rw(FUNC(ct1741_dsp_device::mode_r), FUNC(ct1741_dsp_device::mode_w));
	map(0x0005, 0x0005).mirror(0xff00).rw(FUNC(ct1741_dsp_device::dac_ctrl_r), FUNC(ct1741_dsp_device::dac_ctrl_w));
	map(0x0006, 0x0006).mirror(0xff00).r(FUNC(ct1741_dsp_device::dma_stat_r));
	// unknown, readback status of stereo f/f? (doom)
	map(0x0007, 0x0007).mirror(0xff00).lr8(NAME([] () { return 0; }));
	map(0x0008, 0x0008).mirror(0xff00).rw(FUNC(ct1741_dsp_device::ctrl8_r), FUNC(ct1741_dsp_device::ctrl8_w));
	map(0x0009, 0x0009).mirror(0xff00).w(FUNC(ct1741_dsp_device::rate_w));
	map(0x000A, 0x000A).mirror(0xff00).r(FUNC(ct1741_dsp_device::dma8_cnt_lo_r));
	map(0x000B, 0x000B).mirror(0xff00).w(FUNC(ct1741_dsp_device::dma8_len_lo_w));
	map(0x000C, 0x000C).mirror(0xff00).w(FUNC(ct1741_dsp_device::dma8_len_hi_w));
	map(0x000D, 0x000D).mirror(0xff00).r(FUNC(ct1741_dsp_device::dma8_cnt_hi_r));
	map(0x000E, 0x000E).mirror(0xff00).rw(FUNC(ct1741_dsp_device::dac_fifo_ctrl_r), FUNC(ct1741_dsp_device::dac_fifo_ctrl_w));
	map(0x000F, 0x000F).mirror(0xff00).r(FUNC(ct1741_dsp_device::dma8_ready_r));
	map(0x0010, 0x0010).mirror(0xff00).rw(FUNC(ct1741_dsp_device::ctrl16_r), FUNC(ct1741_dsp_device::ctrl16_w));
	map(0x0013, 0x0013).mirror(0xff00).w(FUNC(ct1741_dsp_device::dma16_len_lo_w));
	map(0x0014, 0x0014).mirror(0xff00).w(FUNC(ct1741_dsp_device::dma16_len_hi_w));
	map(0x0016, 0x0016).mirror(0xff00).rw(FUNC(ct1741_dsp_device::adc_fifo_ctrl_r), FUNC(ct1741_dsp_device::adc_fifo_ctrl_w));
	map(0x0017, 0x0017).mirror(0xff00).r(FUNC(ct1741_dsp_device::adc_data_ready_r));
	map(0x0019, 0x0019).mirror(0xff00).w(FUNC(ct1741_dsp_device::dac_data_w));
	map(0x001B, 0x001B).mirror(0xff00).r(FUNC(ct1741_dsp_device::adc_data_r));
	map(0x001D, 0x001D).mirror(0xff00).w(FUNC(ct1741_dsp_device::dma8_w));
	map(0x001F, 0x001F).mirror(0xff00).r(FUNC(ct1741_dsp_device::dma8_r));
//  map(0x0080, 0x0080) // ASP comms
//  map(0x0081, 0x0081)
//  map(0x0082, 0x0082)
}

/*
 *
 * Internal P1/P2 connections
 *
 */

uint8_t ct1741_dsp_device::p1_r()
{
	uint8_t ret = 0;
	ret |= m_data_out << 0;
	ret |= m_data_in << 1;
	return ret;
}

void ct1741_dsp_device::p1_w(uint8_t data)
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

uint8_t ct1741_dsp_device::p2_r()
{
	return 0;
	// return (m_timer->period() == attotime::never) << 4;
}

void ct1741_dsp_device::p2_w(uint8_t data)
{
	/* port P2
	 * bit0 -
	 * bit1 -
	 * bit2 -
	 * bit3 -
	 * bit4 - clock running?
	 * bit5 - prescaler or clock input source (checked around PC=af1, guimo on low sound detail)
	 * bit6 - ?
	 * bit7 - ?
	*/
}

/*
 *
 * Internal Data map handlers
 *
 */

uint8_t ct1741_dsp_device::dsp_data_r()
{
	if(!machine().side_effects_disabled())
		m_data_in = false;

	return m_in_byte;
}

void ct1741_dsp_device::dsp_data_w(uint8_t data)
{
	m_data_out = true;
	m_out_byte = data;
}

uint8_t ct1741_dsp_device::dac_ctrl_r()
{
	return 0;
}

void ct1741_dsp_device::dac_ctrl_w(uint8_t data)
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

uint8_t ct1741_dsp_device::adc_data_r()
{
	return 0;
}

void ct1741_dsp_device::dac_data_w(uint8_t data)
{
	m_ldac_w(data << 8);
	m_rdac_w(data << 8);
}

void ct1741_dsp_device::control_timer()
{
	const bool start = !BIT(m_ctrl8, 1) || !BIT(m_ctrl16, 1) || BIT(m_mode, 1);

	if(start && m_freq)
	{
		double rate = ((46.61512_MHz_XTAL).dvalue()/1024/256) * m_freq;
		m_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
	}
	else
		m_timer->adjust(attotime::never);
}

void ct1741_dsp_device::rate_w(uint8_t data)
{
	m_freq = data;
	control_timer();
}

uint8_t ct1741_dsp_device::dma8_r()
{
	return m_dac_fifo[0].b[0];
}

void ct1741_dsp_device::dma8_w(uint8_t data)
{
	m_adc_fifo[0].b[0] = data;
	m_drq8_w(0);
}

uint8_t ct1741_dsp_device::dma_stat_r()
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

uint8_t ct1741_dsp_device::ctrl8_r()
{
	return m_ctrl8;
}

void ct1741_dsp_device::ctrl8_w(uint8_t data)
{
	/* port 0x08
	 * bit0 - DMA resume? (resuming from pause in tentacle)
	 * bit1 - stop transfer
	 * bit2 - load counter
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 - DMA pause? (wolf3d, entering pause in tentacle)
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
	if(data & 2)
	{
		m_drq8_w(0);
		//if(m_ctrl16 & 2)
		//  control_timer(false);
	}
	// wolf3d disagrees with drq high here (will be short by 1 sample, cfr. MT09316)
	//else
	//  m_drq8_w(1);

//  if (data & 0x40)
//  {
//      printf("DMA pause? %02x\n", data);
//      //m_drq8_w(0);
//      control_timer(false);
//  }
//  else if (data & 1)
//  {
//      printf("DMA resume? %02x\n", data);
//      //m_drq8_w(1);
//      control_timer(true);
//  }

	if(data & 0x80)
	{
		m_irq8_w(ASSERT_LINE);
	}
	m_ctrl8 = data;
	control_timer();
}

uint8_t ct1741_dsp_device::ctrl16_r()
{
	return m_ctrl16;
}

void ct1741_dsp_device::ctrl16_w(uint8_t data)
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
	//if(!(data & 2) || !(m_ctrl8 & 2))
	//  control_timer(true);
	if(data & 2)
	{
		m_drq16_w(0);
		//if(m_ctrl8 & 2)
		//  control_timer(false);
	}
	//else
	//  m_drq16_w(1);

	if(data & 0x80)
	{
		m_irq16_w(ASSERT_LINE);
	}
	m_ctrl16 = data;
	control_timer();
}

uint8_t ct1741_dsp_device::dac_fifo_ctrl_r()
{
	return m_dac_fifo_ctrl;
}

void ct1741_dsp_device::dac_fifo_ctrl_w(uint8_t data)
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
	// TODO: does this connects to mixer or just clamps DAC to 0?
	m_speaker_off_w(BIT(data, 1));
	m_dac_fifo_ctrl = data;
}

uint8_t ct1741_dsp_device::adc_fifo_ctrl_r()
{
	return m_adc_fifo_ctrl;
}

void ct1741_dsp_device::adc_fifo_ctrl_w(uint8_t data)
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

uint8_t ct1741_dsp_device::mode_r()
{
	return m_mode;
}

void ct1741_dsp_device::mode_w(uint8_t data)
{
	/* port 0x04
	 * bit0 - 1 -- dac 16, adc 8; 0 -- adc 16, dac 8
	 * bit1 - output silence (required for fwmigolf for card detection)
	 * bit2 - int dma complete
	 * bit3 -
	 * bit4 - 8 bit signed
	 * bit5 - 16 bit signed
	 * bit6 - 8 bit mono
	 * bit7 - 16 bit mono
	*/
	m_mode = data;
	control_timer();
}

uint8_t ct1741_dsp_device::dma8_ready_r()
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

uint8_t ct1741_dsp_device::adc_data_ready_r()
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

uint8_t ct1741_dsp_device::dma8_cnt_lo_r()
{
	u16 res = m_dma8_cnt;
	if (!(BIT(m_mode, 6)))
		res <<= 1;
	res --;

	return res & 0xff;
}

uint8_t ct1741_dsp_device::dma8_cnt_hi_r()
{
	u16 res = m_dma8_cnt;
	if (!(BIT(m_mode, 6)))
		res <<= 1;
	res --;

	return res >> 8;
}

void ct1741_dsp_device::dma8_len_lo_w(uint8_t data)
{
	m_dma8_len = (m_dma8_len & 0xff00) | data;
}

void ct1741_dsp_device::dma8_len_hi_w(uint8_t data)
{
	m_dma8_len = (m_dma8_len & 0xff) | (data << 8);
}

void ct1741_dsp_device::dma16_len_lo_w(uint8_t data)
{
	m_dma16_len = (m_dma16_len & 0xff00) | data;
}

void ct1741_dsp_device::dma16_len_hi_w(uint8_t data)
{
	m_dma16_len = (m_dma16_len & 0xff) | (data << 8);
}

/*
 *
 * Host interface
 *
 */

// $226
void ct1741_dsp_device::dsp_reset_w(uint8_t data)
{
	if(data & 1)
	{
		device_reset();
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

// $228
uint8_t ct1741_dsp_device::host_data_r()
{
	if (!machine().side_effects_disabled())
		m_data_out = false;
	return m_out_byte;
}

// $22c write
void ct1741_dsp_device::host_cmd_w(uint8_t data)
{
	m_data_in = true;
	m_in_byte = data;
}

// $22c read
uint8_t ct1741_dsp_device::dsp_wbuf_status_r(offs_t offset)
{
	if(offset)
		return 0xff;
	return m_data_in << 7;
}

// $22e read
uint8_t ct1741_dsp_device::dsp_rbuf_status_r(offs_t offset)
{
	if(offset)
	{
		if(!machine().side_effects_disabled())
		{
			m_irq16_w(CLEAR_LINE);
		}
		return 0xff;
	}
	// reading here clears both irqs
	// ibm5170_cdrom:sideline boot init with mode = 0x65 (16-bit) then just uses mode = 0x64 (8-bit) in-game.
	if(!machine().side_effects_disabled())
	{
		m_irq8_w(CLEAR_LINE);
		m_irq16_w(CLEAR_LINE);
	}
	return m_data_out << 7;
}

/*
 *
 * Timer and DMA logic
 *
 */

TIMER_CALLBACK_MEMBER(ct1741_dsp_device::timer_tick)
{
	uint16_t dacl = 0, dacr = 0, adcl = 0, adcr = 0;
	//printf("mode %02x ctrl8 %02x ctrl16 %02x\n", m_mode, m_ctrl8, m_ctrl16);

	if(BIT(m_mode, 1))
	{
		// it might be possible to run the adc though dma simultaneously but the rom doesn't appear to permit it
		// Update: ibm5170_cdrom:fwmigolf uses this for card detection
		//if(!(m_ctrl8 & 2))
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
	m_rdac_w(dacr);
	m_ldac_w(dacl);

	if(!(m_ctrl8 & 2))
		m_drq8_w(1);

	if(!(m_ctrl16 & 2))
		m_drq16_w(1);

	if((!(m_ctrl8 & 2) && !(m_mode & 1)) || (!(m_ctrl16 & 2) && (m_mode & 1)))
		++m_dac_fifo_tail %= FIFO_SIZE;

	if((!(m_ctrl8 & 2) && (m_mode & 1)) || (!(m_ctrl16 & 2) && !(m_mode & 1)))
	{
		m_adc_fifo[m_adc_fifo_head].h[0] = adcl;
		m_adc_fifo[m_adc_fifo_head].h[1] = adcr;
		++m_adc_fifo_head %= FIFO_SIZE;
	}

//	m_cpu->set_input_line(MCS51_T2_LINE, ASSERT_LINE);
}

uint8_t ct1741_dsp_device::dack_r()
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
		m_drq8_w(0);
		return ret;
	}

	++m_adc_fifo_tail %= FIFO_SIZE;

	if(m_adc_fifo_ctrl & 4)
	{
		m_drq8_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % FIFO_SIZE))
		m_drq8_w(0);
	return ret;
}

void ct1741_dsp_device::dack_w(uint8_t data)
{
//  printf("dack_w %02x -> [%02x] FIFO (%02x ~ %02x) ctrl %02x cnt %04x mode %02x\n", m_ctrl8, data, m_dac_fifo_head, m_dac_fifo_tail, m_dac_fifo_ctrl, m_dma8_cnt, m_mode);
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
		m_drq8_w(0);
		return;
	}

	++m_dac_fifo_head %= FIFO_SIZE;

	if(m_dac_fifo_ctrl & 4)
	{
		m_drq8_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_drq8_w(0);
}

uint16_t ct1741_dsp_device::dack16_r()
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
		m_drq16_w(0);
		return ret;
	}
	++m_adc_fifo_tail %= FIFO_SIZE;

	if(m_adc_fifo_ctrl & 4)
	{
		m_drq16_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % FIFO_SIZE))
		m_drq16_w(0);
	return ret;
}

void ct1741_dsp_device::dack16_w(uint16_t data)
{
//  printf("dack16_w %02x -> [%02x] FIFO (%02x ~ %02x) ctrl %02x cnt %04x mode %02x\n", m_ctrl16, data, m_dac_fifo_head, m_dac_fifo_tail, m_dac_fifo_ctrl, m_dma16_cnt, m_mode);

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
		m_drq16_w(0);
		return;
	}
	++m_dac_fifo_head %= FIFO_SIZE;

	if(m_dac_fifo_ctrl & 4)
	{
		m_drq16_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_drq16_w(0);
}


