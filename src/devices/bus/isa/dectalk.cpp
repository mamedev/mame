// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "dectalk.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA8_DECTALK, dectalk_isa_device, "dectalk_isa", "DECTalk-PC")

dectalk_isa_device::dectalk_isa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	device_t(mconfig, ISA8_DECTALK, tag, owner, clock),
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

void dectalk_isa_device::status_w(uint16_t data)
{
	m_stat = data;
}

uint16_t dectalk_isa_device::cmd_r()
{
	return m_cmd;
}

void dectalk_isa_device::data_w(uint16_t data)
{
	m_data = data;
}

uint16_t dectalk_isa_device::data_r()
{
	return m_data;
}

uint16_t dectalk_isa_device::host_irq_r()
{
	//m_isa->ir?_w(1);
	return 0;
}

uint8_t dectalk_isa_device::dma_r()
{
	return m_dma;
}

void dectalk_isa_device::dma_w(uint8_t data)
{
	m_dma = data;
}

void dectalk_isa_device::dac_w(uint16_t data)
{
	m_dac->write(data >> 4);
}

void dectalk_isa_device::output_ctl_w(uint16_t data)
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

uint16_t dectalk_isa_device::dsp_dma_r()
{
	m_bio = ASSERT_LINE;
	return m_dsp_dma;
}

void dectalk_isa_device::dsp_dma_w(uint16_t data)
{
	m_bio = CLEAR_LINE;
	m_dsp_dma = data;
}

int dectalk_isa_device::bio_line_r()
{
	// TODO: reading the bio line doesn't cause any direct external effects so this is wrong
	if(m_bio == ASSERT_LINE)
		m_cpu->dma_sync_req(0);
	return m_bio;
}

void dectalk_isa_device::irq_line_w(uint16_t data)
{
	m_cpu->int1_w(0);
}

void dectalk_isa_device::clock_w(int state)
{
	m_dsp->set_input_line(INPUT_LINE_IRQ0, (!(m_ctl & 0x20) || state) ? CLEAR_LINE : ASSERT_LINE);
}

void dectalk_isa_device::dectalk_cpu_io(address_map &map)
{
	map(0x0400, 0x0401).rw(FUNC(dectalk_isa_device::cmd_r), FUNC(dectalk_isa_device::status_w)); //PCS0
	map(0x0480, 0x0481).rw(FUNC(dectalk_isa_device::data_r), FUNC(dectalk_isa_device::data_w)); //PCS1
	map(0x0500, 0x0501).w(FUNC(dectalk_isa_device::dsp_dma_w)); //PCS2
	map(0x0580, 0x0581).r(FUNC(dectalk_isa_device::host_irq_r)); //PCS3
	map(0x0600, 0x0601).w(FUNC(dectalk_isa_device::output_ctl_w)); //PCS4
	map(0x0680, 0x0680).rw(FUNC(dectalk_isa_device::dma_r), FUNC(dectalk_isa_device::dma_w)); //PCS5
	map(0x0700, 0x0701).w(FUNC(dectalk_isa_device::irq_line_w)); //PCS6
}

void dectalk_isa_device::dectalk_cpu_map(address_map &map)
{
	map(0x00000, 0xFBFFF).ram();
	map(0xFC000, 0xFFFFF).rom().region("dectalk_cpu", 0);
}

void dectalk_isa_device::dectalk_dsp_io(address_map &map)
{
	map(0x0, 0x0).r(FUNC(dectalk_isa_device::dsp_dma_r));
	map(0x1, 0x1).rw(FUNC(dectalk_isa_device::dsp_dma_r), FUNC(dectalk_isa_device::dac_w));
}

void dectalk_isa_device::dectalk_dsp_map(address_map &map)
{
	map(0x0000, 0x0FFF).rom().region("dectalk_dsp", 0);
}

ROM_START( dectalk_isa )
	ROM_REGION( 0x4000, "dectalk_cpu", 0 )
	ROM_LOAD16_BYTE("pc_boot_hxl.am27c64.d6.e26", 0x0000, 0x2000, CRC(7492f1e3) SHA1(fe6946a227f01c94f2b99220320a616445c96ee0)) // Some cards have a different label on the chip which lists the sum16: 31AC (matches contents)
	ROM_LOAD16_BYTE("pc_boot_hxh.am27c64.d8.e27", 0x0001, 0x2000, CRC(1fe7fe40) SHA1(6e89c237f01aa22e0d21ff4d6fdf8137c6ace374)) // Some cards have a different label on the chip which lists the sum16: 1A25 (matches contents)
	ROM_REGION( 0x2000, "dectalk_dsp", 0 )
	ROM_LOAD("spc_034c__2-1-92.tms320p15nl.d3.bin", 0x0000, 0x2000, CRC(d8b1201e) SHA1(4b873a5e882205fcac79a27562054b5c4d1a117c))
ROM_END

const tiny_rom_entry* dectalk_isa_device::device_rom_region() const
{
	return ROM_NAME( dectalk_isa );
}

void dectalk_isa_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_cpu, XTAL(20'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &dectalk_isa_device::dectalk_cpu_map);
	m_cpu->set_addrmap(AS_IO, &dectalk_isa_device::dectalk_cpu_io);
	m_cpu->tmrout0_handler().set(FUNC(dectalk_isa_device::clock_w));

	TMS32015(config, m_dsp, XTAL(20'000'000));
	m_dsp->set_addrmap(AS_PROGRAM, &dectalk_isa_device::dectalk_dsp_map);
	m_dsp->set_addrmap(AS_IO, &dectalk_isa_device::dectalk_dsp_io);
	m_dsp->bio().set(FUNC(dectalk_isa_device::bio_line_r));

	SPEAKER(config, "speaker").front_center();
	DAC_12BIT_R2R(config, m_dac, 0).add_route(0, "speaker", 1.0); // unknown DAC
}

void dectalk_isa_device::write(offs_t offset, uint8_t data)
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
			m_cpu->dma_sync_req(1);
			break;
		case 6:
			m_cpu->int1_w(1);
			break;
	}
}

uint8_t dectalk_isa_device::read(offs_t offset)
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
			if (!machine().side_effects_disabled())
				m_cpu->dma_sync_req(1);
			return m_dma;
	}
	return 0;
}

void dectalk_isa_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0250, 0x0257, read8sm_delegate(*this, FUNC(dectalk_isa_device::read)), write8sm_delegate(*this, FUNC(dectalk_isa_device::write)));
}

void dectalk_isa_device::device_reset()
{
	m_ctl = 0;
	m_vol = 63;
	m_bio = ASSERT_LINE;
}
