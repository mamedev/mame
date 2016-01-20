// license:BSD-3-Clause
// copyright-holders:Carl

// Soundblaster 16 - LLE
//
// The mcu does host communication and control of the dma-dac unit
// TODO: UART is connected to MIDI port, mixer, adc

#include "sb16.h"

const device_type ISA16_SB16 = &device_creator<sb16_lle_device>;

READ8_MEMBER( sb16_lle_device::dsp_data_r )
{
	if(!space.debugger_access())
		m_data_in = false;

	return m_in_byte;
}

WRITE8_MEMBER( sb16_lle_device::dsp_data_w )
{
	m_data_out = true;
	m_out_byte = data;
}

READ8_MEMBER( sb16_lle_device::dac_ctrl_r )
{
	return 0;
}

WRITE8_MEMBER( sb16_lle_device::dac_ctrl_w )
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

READ8_MEMBER( sb16_lle_device::adc_data_r )
{
	return 0;
}

WRITE8_MEMBER( sb16_lle_device::dac_data_w )
{
	m_dacl->write_unsigned8(data);
	m_dacr->write_unsigned8(data);
}

READ8_MEMBER( sb16_lle_device::p1_r )
{
	UINT8 ret = 0;
	ret |= m_data_out << 0;
	ret |= m_data_in << 1;
	return ret;
}

WRITE8_MEMBER( sb16_lle_device::p1_w )
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

READ8_MEMBER( sb16_lle_device::p2_r )
{
	return 0;
}

WRITE8_MEMBER( sb16_lle_device::p2_w )
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
		double rate = (46615120.0/1024/256) * m_freq;
		m_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
	}
	else
		m_timer->adjust(attotime::never);
}

WRITE8_MEMBER( sb16_lle_device::rate_w )
{
	m_freq = data;
	if(!(m_ctrl8 & 2) || !(m_ctrl16 & 2))
		control_timer(true);
}

READ8_MEMBER( sb16_lle_device::dma8_r )
{
	return m_dac_fifo[0].b[0];
}

WRITE8_MEMBER( sb16_lle_device::dma8_w )
{
	m_adc_fifo[0].b[0] = data;
	m_isa->drq1_w(0);
}

READ8_MEMBER( sb16_lle_device::dma_stat_r )
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
	UINT8 ret = (m_dma16_done << 1) | m_dma8_done;
	return ret;
}

READ8_MEMBER( sb16_lle_device::ctrl8_r )
{
	return m_ctrl8;
}

WRITE8_MEMBER( sb16_lle_device::ctrl8_w )
{
	/* port 0x08
	 * bit0 - ?
	 * bit1 - stop transfer
	 * bit2 - load counter
	 * bit3 -
	 * bit4 -
	 * bit5 -
	 * bit6 - ?
	 * bit7 - toggle for 8bit irq
	*/
	if(data & 4)
	{
		m_dma8_cnt = m_dma8_len;
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

READ8_MEMBER( sb16_lle_device::ctrl16_r )
{
	return m_ctrl16;
}

WRITE8_MEMBER( sb16_lle_device::ctrl16_w )
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

READ8_MEMBER( sb16_lle_device::dac_fifo_ctrl_r )
{
	return m_dac_fifo_ctrl;
}

WRITE8_MEMBER( sb16_lle_device::dac_fifo_ctrl_w )
{
	/* port 0x0E
	 * bit0 - reset fifo
	 * bit1 - ?
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
	m_dac_fifo_ctrl = data;
}

READ8_MEMBER( sb16_lle_device::adc_fifo_ctrl_r )
{
	return m_adc_fifo_ctrl;
}

WRITE8_MEMBER( sb16_lle_device::adc_fifo_ctrl_w )
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

READ8_MEMBER( sb16_lle_device::mode_r )
{
	return m_mode;
}

WRITE8_MEMBER( sb16_lle_device::mode_w )
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

READ8_MEMBER( sb16_lle_device::dma8_ready_r )
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

READ8_MEMBER( sb16_lle_device::adc_data_ready_r )
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

READ8_MEMBER( sb16_lle_device::dma8_cnt_lo_r )
{
	return m_dma8_cnt & 0xff;
}

READ8_MEMBER( sb16_lle_device::dma8_cnt_hi_r )
{
	return m_dma8_cnt >> 8;
}

WRITE8_MEMBER( sb16_lle_device::dma8_len_lo_w )
{
	m_dma8_len = (m_dma8_len & 0xff00) | data;
}

WRITE8_MEMBER( sb16_lle_device::dma8_len_hi_w )
{
	m_dma8_len = (m_dma8_len & 0xff) | (data << 8);
}

WRITE8_MEMBER( sb16_lle_device::dma16_len_lo_w )
{
	m_dma16_len = (m_dma16_len & 0xff00) | data;
}

WRITE8_MEMBER( sb16_lle_device::dma16_len_hi_w )
{
	m_dma16_len = (m_dma16_len & 0xff) | (data << 8);
}

ROM_START( sb16 )
	ROM_REGION( 0x2000, "sb16_cpu", 0 )
	ROM_LOAD("ct1741_v413[80c52].bin", 0x0000, 0x2000, CRC(5181892f) SHA1(5b42f1c34c4e9c8dbbdcffa0a36c178ca4f1aa77))

	ROM_REGION(0x40, "xor_table", 0)
	ROM_LOAD("ct1741_v413_xor.bin", 0x00, 0x40, CRC(5243d15a) SHA1(c7637c92828843f47e6e2f956af639b07aee4571))
ROM_END

static ADDRESS_MAP_START(sb16_io, AS_IO, 8, sb16_lle_device)
	AM_RANGE(0x0000, 0x0000) AM_MIRROR(0xff00) AM_READWRITE(dsp_data_r, dsp_data_w)
//  AM_RANGE(0x0001, 0x0001) // MIDI related?
//  AM_RANGE(0x0002, 0x0002)
	AM_RANGE(0x0004, 0x0004) AM_MIRROR(0xff00) AM_READWRITE(mode_r, mode_w)
	AM_RANGE(0x0005, 0x0005) AM_MIRROR(0xff00) AM_READWRITE(dac_ctrl_r, dac_ctrl_w)
	AM_RANGE(0x0006, 0x0006) AM_MIRROR(0xff00) AM_READ(dma_stat_r)
//  AM_RANGE(0x0007, 0x0007) // unknown
	AM_RANGE(0x0008, 0x0008) AM_MIRROR(0xff00) AM_READWRITE(ctrl8_r, ctrl8_w)
	AM_RANGE(0x0009, 0x0009) AM_MIRROR(0xff00) AM_WRITE(rate_w)
	AM_RANGE(0x000A, 0x000A) AM_MIRROR(0xff00) AM_READ(dma8_cnt_lo_r)
	AM_RANGE(0x000B, 0x000B) AM_MIRROR(0xff00) AM_WRITE(dma8_len_lo_w)
	AM_RANGE(0x000C, 0x000C) AM_MIRROR(0xff00) AM_WRITE(dma8_len_hi_w)
	AM_RANGE(0x000D, 0x000D) AM_MIRROR(0xff00) AM_READ(dma8_cnt_hi_r)
	AM_RANGE(0x000E, 0x000E) AM_MIRROR(0xff00) AM_READWRITE(dac_fifo_ctrl_r, dac_fifo_ctrl_w)
	AM_RANGE(0x000F, 0x000F) AM_MIRROR(0xff00) AM_READ(dma8_ready_r)
	AM_RANGE(0x0010, 0x0010) AM_MIRROR(0xff00) AM_READWRITE(ctrl16_r, ctrl16_w)
	AM_RANGE(0x0013, 0x0013) AM_MIRROR(0xff00) AM_WRITE(dma16_len_lo_w)
	AM_RANGE(0x0014, 0x0014) AM_MIRROR(0xff00) AM_WRITE(dma16_len_hi_w)
	AM_RANGE(0x0016, 0x0016) AM_MIRROR(0xff00) AM_READWRITE(adc_fifo_ctrl_r, adc_fifo_ctrl_w)
	AM_RANGE(0x0017, 0x0017) AM_MIRROR(0xff00) AM_READ(adc_data_ready_r)
	AM_RANGE(0x0019, 0x0019) AM_MIRROR(0xff00) AM_WRITE(dac_data_w)
	AM_RANGE(0x001B, 0x001B) AM_MIRROR(0xff00) AM_READ(adc_data_r)
	AM_RANGE(0x001D, 0x001D) AM_MIRROR(0xff00) AM_WRITE(dma8_w)
	AM_RANGE(0x001F, 0x001F) AM_MIRROR(0xff00) AM_READ(dma8_r)
//  AM_RANGE(0x0080, 0x0080) // ASP comms
//  AM_RANGE(0x0081, 0x0081)
//  AM_RANGE(0x0082, 0x0082)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(p1_r, p1_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READWRITE(p2_r, p2_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( sb16 )
	MCFG_CPU_ADD("sb16_cpu", I80C52, XTAL_24MHz)
	MCFG_CPU_IO_MAP(sb16_io)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ymf262", YMF262, XTAL_14_31818MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "rspeaker", 1.00)

	MCFG_SOUND_ADD("dacl", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ADD("dacr", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MCFG_PC_JOY_ADD("pc_joy")
MACHINE_CONFIG_END

const rom_entry *sb16_lle_device::device_rom_region() const
{
	return ROM_NAME( sb16 );
}

machine_config_constructor sb16_lle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sb16 );
}

READ8_MEMBER( sb16_lle_device::host_data_r )
{
	m_data_out = false;
	return m_out_byte;
}

WRITE8_MEMBER( sb16_lle_device::host_cmd_w )
{
	m_data_in = true;
	m_in_byte = data;
}

UINT8 sb16_lle_device::dack_r(int line)
{
	UINT8 ret = m_adc_fifo[m_adc_fifo_tail].b[m_adc_h + (m_adc_r * 2)];

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

	++m_adc_fifo_tail %= 16;

	if(m_adc_fifo_ctrl & 4)
	{
		m_isa->drq1_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % 16))
		m_isa->drq1_w(0);
	return ret;
}

void sb16_lle_device::dack_w(int line, UINT8 data)
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

	++m_dac_fifo_head %= 16;

	if(m_dac_fifo_ctrl & 4)
	{
		m_isa->drq1_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_isa->drq1_w(0);
}

UINT16 sb16_lle_device::dack16_r(int line)
{
	UINT16 ret = m_adc_fifo[m_adc_fifo_tail].h[m_adc_r];

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
	++m_adc_fifo_tail %= 16;

	if(m_adc_fifo_ctrl & 4)
	{
		m_isa->drq5_w(0);
		return ret;
	}

	if(m_adc_fifo_head == ((m_adc_fifo_tail + 1) % 16))
		m_isa->drq5_w(0);
	return ret;
}

void sb16_lle_device::dack16_w(int line, UINT16 data)
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
	++m_dac_fifo_head %= 16;

	if(m_dac_fifo_ctrl & 4)
	{
		m_isa->drq5_w(0);
		return;
	}

	if(m_dac_fifo_head == m_dac_fifo_tail)
		m_isa->drq5_w(0);
}

WRITE8_MEMBER( sb16_lle_device::dsp_reset_w )
{
	if(data & 1)
	{
		device_reset();
		m_cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
}

READ8_MEMBER( sb16_lle_device::dsp_wbuf_status_r )
{
	if(offset)
		return 0xff;
	return m_data_in << 7;
}

READ8_MEMBER( sb16_lle_device::dsp_rbuf_status_r )
{
	if(offset)
	{
		m_irq16 = false;
		m_isa->irq5_w((m_irq8 || m_irq16 || m_irq_midi) ? ASSERT_LINE : CLEAR_LINE);
		return 0xff;
	}
	m_irq8 = false;
	m_isa->irq5_w((m_irq8 || m_irq16 || m_irq_midi) ? ASSERT_LINE : CLEAR_LINE);
	return m_data_out << 7;
}

WRITE8_MEMBER( sb16_lle_device::invalid_w )
{
	logerror("sb16: invalid port write\n");
}

READ8_MEMBER( sb16_lle_device::invalid_r )
{
	logerror("sb16: invalid port read\n");
	return 0xff;
}

// just using the old dummy mpu401 for now
READ8_MEMBER( sb16_lle_device::mpu401_r )
{
	UINT8 res;

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

WRITE8_MEMBER( sb16_lle_device::mpu401_w )
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

sb16_lle_device::sb16_lle_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISA16_SB16, "SoundBlaster 16 Audio Adapter LLE", tag, owner, clock, "sb16", __FILE__),
	device_isa16_card_interface(mconfig, *this),
	m_dacl(*this, "dacl"),
	m_dacr(*this, "dacr"),
	m_joy(*this, "pc_joy"),
	m_cpu(*this, "sb16_cpu"), m_data_in(false), m_in_byte(0), m_data_out(false), m_out_byte(0), m_freq(0), m_mode(0), m_dac_fifo_ctrl(0), m_adc_fifo_ctrl(0), m_ctrl8(0), m_ctrl16(0), m_mpu_byte(0),
	m_dma8_len(0), m_dma16_len(0), m_dma8_cnt(0), m_dma16_cnt(0), m_adc_fifo_head(0), m_adc_fifo_tail(0), m_dac_fifo_head(0), m_dac_fifo_tail(0), m_adc_r(false), m_dac_r(false), m_adc_h(false),
	m_dac_h(false), m_irq8(false), m_irq16(false), m_irq_midi(false), m_dma8_done(false), m_dma16_done(false), m_timer(nullptr)
{
}

void sb16_lle_device::device_start()
{
	//address_space &space = m_cpu->space(AS_PROGRAM);
	UINT8 *rom = memregion("sb16_cpu")->base();
	UINT8 *xor_table = memregion("xor_table")->base();

	for(int i = 0; i < 0x2000; i++)
		rom[i] = rom[i] ^ xor_table[i & 0x3f];


	ymf262_device *ymf262 = subdevice<ymf262_device>("ymf262");
	set_isa_device();

	m_isa->install_device(0x0200, 0x0207, 0, 0, read8_delegate(FUNC(pc_joy_device::joy_port_r), subdevice<pc_joy_device>("pc_joy")), write8_delegate(FUNC(pc_joy_device::joy_port_w), subdevice<pc_joy_device>("pc_joy")));
	m_isa->install_device(0x0226, 0x0227, 0, 0, read8_delegate(FUNC(sb16_lle_device::invalid_r), this), write8_delegate(FUNC(sb16_lle_device::dsp_reset_w), this));
	m_isa->install_device(0x022a, 0x022b, 0, 0, read8_delegate(FUNC(sb16_lle_device::host_data_r), this), write8_delegate(FUNC(sb16_lle_device::invalid_w), this) );
	m_isa->install_device(0x022c, 0x022d, 0, 0, read8_delegate(FUNC(sb16_lle_device::dsp_wbuf_status_r), this), write8_delegate(FUNC(sb16_lle_device::host_cmd_w), this) );
	m_isa->install_device(0x022e, 0x022f, 0, 0, read8_delegate(FUNC(sb16_lle_device::dsp_rbuf_status_r), this), write8_delegate(FUNC(sb16_lle_device::invalid_w), this) );
	m_isa->install_device(0x0330, 0x0331, 0, 0, read8_delegate(FUNC(sb16_lle_device::mpu401_r), this), write8_delegate(FUNC(sb16_lle_device::mpu401_w), this));
	m_isa->install_device(0x0388, 0x0389, 0, 0, read8_delegate(FUNC(ymf262_device::read), ymf262), write8_delegate(FUNC(ymf262_device::write), ymf262));
	m_isa->install_device(0x0220, 0x0223, 0, 0, read8_delegate(FUNC(ymf262_device::read), ymf262), write8_delegate(FUNC(ymf262_device::write), ymf262));
	m_isa->install_device(0x0228, 0x0229, 0, 0, read8_delegate(FUNC(ymf262_device::read), ymf262), write8_delegate(FUNC(ymf262_device::write), ymf262));
	m_isa->set_dma_channel(1, this, FALSE);
	m_isa->set_dma_channel(5, this, FALSE);
	m_timer = timer_alloc();
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

void sb16_lle_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	UINT16 dacl = 0, dacr = 0, adcl = 0, adcr = 0;
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
				dacl = (m_dac_fifo[m_dac_fifo_tail].h[0] - 0x8000);
				dacr = (m_dac_fifo[m_dac_fifo_tail].h[1] - 0x8000);
				break;
			case 0x20: // signed stereo
				dacl = m_dac_fifo[m_dac_fifo_tail].h[0];
				dacr = m_dac_fifo[m_dac_fifo_tail].h[1];
				break;
			case 0x80: // unsigned mono
				dacl = (m_dac_fifo[m_dac_fifo_tail].h[0] - 0x8000);
				dacr = (m_dac_fifo[m_dac_fifo_tail].h[0] - 0x8000);
				break;
			case 0xa0: // signed mono
				dacl = m_dac_fifo[m_dac_fifo_tail].h[0];
				dacr = m_dac_fifo[m_dac_fifo_tail].h[0];
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
				dacl = (m_dac_fifo[m_dac_fifo_tail].b[0] - 0x80) << 8;
				dacr = (m_dac_fifo[m_dac_fifo_tail].b[2] - 0x80) << 8;
				break;
			case 0x10: // signed stereo
				dacl = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
				dacr = m_dac_fifo[m_dac_fifo_tail].b[2] << 8;
				break;
			case 0x40: // unsigned mono
				dacl = (m_dac_fifo[m_dac_fifo_tail].b[0] - 0x80) << 8;
				dacr = (m_dac_fifo[m_dac_fifo_tail].b[0] - 0x80) << 8;
				break;
			case 0x50: // signed mono
				dacl = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
				dacr = m_dac_fifo[m_dac_fifo_tail].b[0] << 8;
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
	m_dacr->write(dacr);
	m_dacl->write(dacl);

	if(!(m_ctrl8 & 2))
		m_isa->drq1_w(1);

	if(!(m_ctrl16 & 2))
		m_isa->drq5_w(1);

	if((!(m_ctrl8 & 2) && !(m_mode & 1)) || (!(m_ctrl16 & 2) && (m_mode & 1)))
		++m_dac_fifo_tail %= 16;

	if((!(m_ctrl8 & 2) && (m_mode & 1)) || (!(m_ctrl16 & 2) && !(m_mode & 1)))
	{
		m_adc_fifo[m_adc_fifo_head].h[0] = adcl;
		m_adc_fifo[m_adc_fifo_head].h[1] = adcr;
		++m_adc_fifo_head %= 16;
	}
}
