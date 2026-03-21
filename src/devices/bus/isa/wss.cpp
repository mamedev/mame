// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Microsoft Windows Sound System based cards

TODO:
- Untested;
- Configuration jumpers;

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

	AD1848(config, m_soundport, 0);
	m_soundport->irq().set([this] (int state) { m_isa->irq7_w(state); });
	m_soundport->drq().set([this] (int state) { m_isa->drq0_w(state); });

}

void isa16_wss_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(0, this, false);
}

void isa16_wss_device::device_reset()
{
	remap(AS_IO, 0, 0xffff);
}

void isa16_wss_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x0220, 0x022f, *this, &isa16_wss_device::host_io);

		m_isa->install_device(0x0388, 0x038b, read8sm_delegate(*m_opl3, FUNC(ymf262_device::read)), write8sm_delegate(*m_opl3, FUNC(ymf262_device::write)));

		m_isa->install_device(0x0534, 0x0537, read8sm_delegate(*m_soundport, FUNC(ad1848_device::read)), write8sm_delegate(*m_soundport, FUNC(ad1848_device::write)));
	}
}

void isa16_wss_device::host_io(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
	map(0x04, 0x07).rw(m_soundport, FUNC(ad1848_device::read), FUNC(ad1848_device::write));
	map(0x08, 0x09).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
}
