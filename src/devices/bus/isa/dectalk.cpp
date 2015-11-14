// license:BSD-3-Clause
// copyright-holders:Carl
#include "dectalk.h"

const device_type ISA8_DECTALK = &device_creator<dectalk_isa_device>;

dectalk_isa_device::dectalk_isa_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	device_t(mconfig, ISA8_DECTALK, "DECTalk-PC", tag, owner, clock, "dectalk_isa", __FILE__),
	device_isa8_card_interface(mconfig, *this), 
	m_cmd(0), 
	m_stat(0), 
	m_data(0), 
	m_dsp_dma(0),
	m_ctl(0), 
	m_dma(0), 
	m_vol(0), 
	m_bio(0),
	m_cpu(*this, "dectalk_cpu"),
	m_dac(*this, "dac"),
	m_dsp(*this, "dectalk_dsp")
{
}

WRITE16_MEMBER(dectalk_isa_device::status_w)
{
	m_stat = data;
}

READ16_MEMBER(dectalk_isa_device::cmd_r)
{
	return m_cmd;
}

WRITE16_MEMBER(dectalk_isa_device::data_w)
{
	m_data = data;
}

READ16_MEMBER(dectalk_isa_device::data_r)
{
	return m_data;
}

READ16_MEMBER(dectalk_isa_device::host_irq_r)
{
	//m_isa->ir?_w(1);
	return 0;
}

READ8_MEMBER(dectalk_isa_device::dma_r)
{
	m_cpu->drq1_w(0);
	return m_dma;
}

WRITE8_MEMBER(dectalk_isa_device::dma_w)
{
	m_cpu->drq1_w(0);
	m_dma = data;
}

WRITE16_MEMBER(dectalk_isa_device::dac_w)
{
	m_dac->write(data & 0xfff0);
}

WRITE16_MEMBER(dectalk_isa_device::output_ctl_w)
{
	// X9C503P potentiometer, 8-CS, 4-U/D, 2-INC
	if(!(data & 8) && !(m_ctl & 2) && (data & 2))
	{
		if((data & 4) && (m_vol < 64))
			m_vol++;
		else if(!(data & 4) && m_vol)
			m_vol--;

		m_dac->set_output_gain(ALL_OUTPUTS, m_vol / 63.0);
	}
	m_dsp->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
	m_ctl = data;
}

READ16_MEMBER(dectalk_isa_device::dsp_dma_r)
{
	m_bio = ASSERT_LINE;
	m_cpu->drq1_w(0);
	return m_dsp_dma;
}

WRITE16_MEMBER(dectalk_isa_device::dsp_dma_w)
{
	m_bio = CLEAR_LINE;
	m_dsp_dma = data;
}

READ16_MEMBER(dectalk_isa_device::bio_line_r)
{
	// TODO: reading the bio line doesn't cause any direct external effects so this is wrong
	if(m_bio == ASSERT_LINE)
		m_cpu->drq0_w(1);
	return m_bio;
}

WRITE16_MEMBER(dectalk_isa_device::irq_line_w)
{
	m_cpu->int1_w(0);
}

WRITE_LINE_MEMBER(dectalk_isa_device::clock_w)
{
	m_dsp->set_input_line(INPUT_LINE_IRQ0, (!(m_ctl & 0x20) || state) ? CLEAR_LINE : ASSERT_LINE);
}

static ADDRESS_MAP_START(dectalk_cpu_io, AS_IO, 16, dectalk_isa_device)
	AM_RANGE(0x0400, 0x0401) AM_READWRITE(cmd_r, status_w) //PCS0
	AM_RANGE(0x0480, 0x0481) AM_READWRITE(data_r, data_w) //PCS1
	AM_RANGE(0x0500, 0x0501) AM_WRITE(dsp_dma_w) //PCS2
	AM_RANGE(0x0580, 0x0581) AM_READ(host_irq_r) //PCS3
	AM_RANGE(0x0600, 0x0601) AM_WRITE(output_ctl_w) //PCS4
	AM_RANGE(0x0680, 0x0681) AM_READWRITE8(dma_r, dma_w, 0xff) //PCS5
	AM_RANGE(0x0700, 0x0701) AM_WRITE(irq_line_w) //PCS6
ADDRESS_MAP_END

static ADDRESS_MAP_START(dectalk_cpu_map, AS_PROGRAM, 16, dectalk_isa_device)
	AM_RANGE(0x00000, 0xFBFFF) AM_RAM
	AM_RANGE(0xFC000, 0xFFFFF) AM_ROM AM_REGION("dectalk_cpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dectalk_dsp_io, AS_IO, 16, dectalk_isa_device)
	AM_RANGE(0x0, 0x0) AM_READ(dsp_dma_r)
	AM_RANGE(0x1, 0x1) AM_READWRITE(dsp_dma_r, dac_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(bio_line_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dectalk_dsp_map, AS_PROGRAM, 16, dectalk_isa_device)
	AM_RANGE(0x0000, 0x0FFF) AM_ROM AM_REGION("dectalk_dsp", 0)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( dectalk_isa )
	MCFG_CPU_ADD("dectalk_cpu", I80186, XTAL_20MHz)
	MCFG_CPU_IO_MAP(dectalk_cpu_io)
	MCFG_CPU_PROGRAM_MAP(dectalk_cpu_map)
	MCFG_80186_TMROUT0_HANDLER(WRITELINE(dectalk_isa_device, clock_w));

	MCFG_CPU_ADD("dectalk_dsp", TMS32015, XTAL_20MHz)
	MCFG_CPU_IO_MAP(dectalk_dsp_io)
	MCFG_CPU_PROGRAM_MAP(dectalk_dsp_map)

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.00)
MACHINE_CONFIG_END

ROM_START( dectalk_isa )
	ROM_REGION( 0x4000, "dectalk_cpu", 0 )
	ROM_LOAD16_BYTE("pc_boot_hxl.am27c64.d6.e26", 0x0000, 0x2000, CRC(7492f1e3) SHA1(fe6946a227f01c94f2b99220320a616445c96ee0)) // Some cards have a different label on the chip which lists the sum16: 31AC (matches contents)
	ROM_LOAD16_BYTE("pc_boot_hxh.am27c64.d8.e27", 0x0001, 0x2000, CRC(1fe7fe40) SHA1(6e89c237f01aa22e0d21ff4d6fdf8137c6ace374)) // Some cards have a different label on the chip which lists the sum16: 1A25 (matches contents)
	ROM_REGION( 0x2000, "dectalk_dsp", 0 )
	ROM_LOAD("spc_034c__2-1-92.tms320p15nl.d3.bin", 0x0000, 0x2000, CRC(d8b1201e) SHA1(4b873a5e882205fcac79a27562054b5c4d1a117c))
ROM_END

const rom_entry* dectalk_isa_device::device_rom_region() const
{
	return ROM_NAME( dectalk_isa );
}

machine_config_constructor dectalk_isa_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dectalk_isa );
}

WRITE8_MEMBER(dectalk_isa_device::write)
{
	switch(offset)
	{
		case 0:
			m_cmd = (m_cmd & 0xff00) | data;
			break;
		case 1:
			m_cmd = (m_cmd & 0xff) | (data << 8);
			break;
		case 2:
			m_data = (m_data & 0xff00) | data;
			break;
		case 3:
			m_data = (m_data & 0xff) | (data << 8);
			break;
		case 4:
			m_dma = data;
			m_cpu->drq1_w(1);
			break;
		case 6:
			m_cpu->int1_w(1);
			break;
	}
}

READ8_MEMBER(dectalk_isa_device::read)
{
	switch(offset)
	{
		case 0:
			return m_stat & 0xff;
		case 1:
			return m_stat >> 8;
		case 2:
			return m_data & 0xff;
		case 3:
			return m_data >> 8;
		case 4:
			m_cpu->drq1_w(1);
			return m_dma;
	}
	return 0;
}

void dectalk_isa_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0250, 0x0257, 0, 0, read8_delegate(FUNC(dectalk_isa_device::read), this), write8_delegate(FUNC(dectalk_isa_device::write), this));
}

void dectalk_isa_device::device_reset()
{
	m_ctl = 0;
	m_vol = 63;
	m_bio = ASSERT_LINE;
}
