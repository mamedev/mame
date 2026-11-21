// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Microsoft Windows Sound System based cards

TODO:
- How DMA selection works for 16-bit modes on original config?
- IRQ / DMA combinations aren't extensively tested under win311 (a PITA overhead to test)
- SoundBlaster compatibilty is untested (requires WSSXLAT.EXE and EMM386.EXE setup properly);

**************************************************************************************************/

#include "emu.h"
#include "wss.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(ISA16_WSS, isa16_wss_device, "wss", "Microsoft Windows Sound System (original design)")

isa16_wss_device::isa16_wss_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_WSS, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_opl3(*this, "opl3")
	, m_soundport(*this, "soundport")
	, m_jp1(*this, "JP1")
{
}


void isa16_wss_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	YMF262(config, m_opl3, XTAL(14'318'181));
	m_opl3->add_route(0, "speaker", 1.0, 0);
	m_opl3->add_route(1, "speaker", 1.0, 1);
	m_opl3->add_route(2, "speaker", 1.0, 0);
	m_opl3->add_route(3, "speaker", 1.0, 1);

	AD1848(config, m_soundport);
	m_soundport->irq().set([this] (int state) {
		switch(m_irq_line)
		{
			case 7:  m_isa->irq7_w(state);  break;
			case 9:  m_isa->irq2_w(state);  break;
			case 10: m_isa->irq10_w(state); break;
			case 11: m_isa->irq11_w(state); break;
		}
	});
	m_soundport->drq().set([this] (int state) {
		switch(m_drq_line)
		{
			case 0: m_isa->drq0_w(state); break;
			case 1: m_isa->drq1_w(state); break;
			case 3: m_isa->drq3_w(state); break;
		}
	});
}

static INPUT_PORTS_START( wss )
	// TODO: guesswork, requires an user manual
	PORT_START("JP1")
	PORT_CONFNAME( 0x03, 0x00, "I/O port base address")
	PORT_CONFSETTING(    0x00, "0x530" )
	PORT_CONFSETTING(    0x01, "0x604" )
	PORT_CONFSETTING(    0x02, "0xe80" )
	PORT_CONFSETTING(    0x03, "0xf40" )
INPUT_PORTS_END

ioport_constructor isa16_wss_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wss );
}


void isa16_wss_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_config));
	save_item(NAME(m_port_base));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_drq_line));
}

void isa16_wss_device::device_reset()
{
	static constexpr uint16_t port_bases[4] = { 0x0530, 0x0604, 0x0e80, 0x0f40 };

	const uint8_t port_setting = m_jp1->read() & 3;
	m_port_base = port_bases[port_setting];

	remap(AS_IO, 0, 0xffff);

	// TODO: default not actually known (IRQ 9 DMA 3?)
	m_config = 0x09;
	config_irq_lanes(m_config);
}

void isa16_wss_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x0220, 0x022f, *this, &isa16_wss_device::host_io);

		m_isa->install_device(0x0388, 0x038b, read8sm_delegate(*m_opl3, FUNC(ymf262_device::read)), write8sm_delegate(*m_opl3, FUNC(ymf262_device::write)));

		// 0x530
		m_isa->install_device(m_port_base, m_port_base, read8sm_delegate(*this, FUNC(isa16_wss_device::config_r)), write8sm_delegate(*this, FUNC(isa16_wss_device::config_w)));
		// 0x534 ~ 0x537
		m_isa->install_device(m_port_base + 4, m_port_base + 7, read8sm_delegate(*m_soundport, FUNC(ad1848_device::read)), write8sm_delegate(*m_soundport, FUNC(ad1848_device::write)));
	}
}

/*
 * win311 WSS driver v2.0 allows selecting only following combinations.
 * MIDAS sound driver will actively crash when selecting any other IRQ/DMA combination
 * (probably expecting a newer variant instead).
 *
 * IRQ  7 DMA 0: 0x09
 * IRQ  7 DMA 1: 0x0a
 * IRQ  7 DMA 3: 0x0b
 * IRQ  9 DMA 0: 0x11
 * IRQ  9 DMA 1: 0x12
 * IRQ  9 DMA 3: 0x13
 * IRQ 10 DMA 0: 0x19
 * IRQ 10 DMA 1: 0x1a
 * IRQ 10 DMA 3: 0x1b
 * IRQ 11 DMA 0: 0x21
 * IRQ 11 DMA 1: 0x22
 * IRQ 11 DMA 3: 0x23
 */
void isa16_wss_device::config_irq_lanes(uint8_t data)
{
	static constexpr int irq_lines[8] = { -1, 7, 9, 10, 11, -1, -1 };
	static constexpr int drq_lines[4] = { -1, 0, 1, 3 };

	const uint8_t irq_config = (data >> 3) & 7;
	const uint8_t drq_config = (data & 3);
	m_irq_line = irq_lines[irq_config];
	m_drq_line = drq_lines[drq_config];

	logerror("WSS config_irq_lanes: %02x IRQ %02x -> %d DRQ %02x -> %d\n", data, irq_config, m_irq_line, drq_config, m_drq_line);
	if (m_irq_line == -1)
		logerror("Warning: unexpected IRQ combination!\n");

	if (m_drq_line == -1)
		logerror("Warning: unexpected DRQ combination!\n");
	else
	{
		m_isa->set_dma_channel(m_drq_line, this, false);
	}
}

void isa16_wss_device::host_io(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
	map(0x04, 0x07).rw(m_soundport, FUNC(ad1848_device::read), FUNC(ad1848_device::write));
	map(0x08, 0x09).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
}

uint8_t isa16_wss_device::config_r(offs_t offset)
{
	return m_config;
}

void isa16_wss_device::config_w(offs_t offset, uint8_t data)
{
	if (m_config != data)
		config_irq_lanes(data);
	m_config = data;
}

u8 isa16_wss_device::dack_r(int line)
{
	return m_soundport->dack_r();
}

void isa16_wss_device::dack_w(int line, u8 data)
{
	m_soundport->dack_w(data);
}

// TODO: dack16 lines should be a thing for later Crystal/Analog variants
// MIDAS sound engine allows selecting DMA16 lines, which this hookup shouldn't be able to select.
