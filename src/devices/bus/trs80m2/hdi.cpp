// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II hard disk interface emulation

    8000143 host interface (5/12/15 MB redesign):
    - 16K of 4116 DRAM at 8000-BFFF, in the page pair picked by BS1-BS7
      from its own port FF latch (bits 1-3).
    - Write-only control latch at C1 (C1 reads come from the
      controller's own copy), Z80 CTC at C4-C7 (only while DEVEN is set).
    - Controller cable for C0-C3 and C8-CF.

    WD1000-TB1 controller (031095): WD1010, WD1100-11 and a 1K sector
    buffer, four ST506 drives with write-protect inputs.

    Port    Read                    Write
    C0      status, clears HDWPL
    C1      control                 control
    C2-C3
    C4-C7   CTC                     CTC
    C8      sector buffer           sector buffer
    C9      error                   write precompensation
    CA-CE   WD1010 task file        WD1010 task file
    CF      status                  command

    Status register (C0):

    bit     description

    0       WD1010 INTRQ
    1       HDWPL, write protect violation
    4-7     WPD1-WPD4, drive write protected

    Control register (C1):

    bit     description

    0       DMA enable
    1       interrupt enable
    3       DEVEN, CTC and controller access enable
    4       SFTRST, controller reset on the rising edge
    6       SFTINT, CTC channel 1 trigger

    TODO:

    - WD1000-TB1 C0 bit 0 INTRQ host-side polarity unverified.
    - TB1 busy gate input labelled "RD2" on sheet 1, uncertain.
    - 8X300 controller (8000122) and older 8 MB interface (8000125:
      WAIT enable, drive-ID switches, WP-change interrupt) not modelled.

**********************************************************************/

#include "emu.h"
#include "hdi.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_HDI, trs80m2_hdi_device, "trs80m2_hdi", "Tandy Model II hard disk interface")


ROM_START( trs80m2_hdi )
	ROM_REGION( 0x2c, "plds", 0 )
	ROM_LOAD( "pal10l8.u20", 0x00, 0x2c, NO_DUMP )
ROM_END

const tiny_rom_entry *trs80m2_hdi_device::device_rom_region() const
{
	return ROM_NAME( trs80m2_hdi );
}


static INPUT_PORTS_START( trs80m2_hdi )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Port base" )
	PORT_CONFSETTING( 0x00, "C0-CF" )
	PORT_CONFSETTING( 0x01, "70-7F" )
	PORT_CONFSETTING( 0x02, "60-6F" )
	PORT_CONFSETTING( 0x03, "50-5F" )
	PORT_CONFNAME( 0x70, 0x70, "RAM page pair" )
	PORT_CONFSETTING( 0x10, "2/3 (BS1)" )
	PORT_CONFSETTING( 0x20, "4/5 (BS2)" )
	PORT_CONFSETTING( 0x30, "6/7 (BS3)" )
	PORT_CONFSETTING( 0x40, "8/9 (BS4)" )
	PORT_CONFSETTING( 0x50, "A/B (BS5)" )
	PORT_CONFSETTING( 0x60, "C/D (BS6)" )
	PORT_CONFSETTING( 0x70, "E/F (BS7)" )
INPUT_PORTS_END

ioport_constructor trs80m2_hdi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_hdi );
}


trs80m2_hdi_device::trs80m2_hdi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_wd1010_card_device_base(mconfig, TRS80M2_HDI, tag, owner, clock),
	m_hdd(*this, "hdc:%u", 0U),
	m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE),
	m_intrq(false),
	m_hdwpl(false),
	m_ram_installed(false)
{
}

void trs80m2_hdi_device::device_add_mconfig(machine_config &config)
{
	trs80m2_wd1010_card_device_base::device_add_mconfig(config);

	m_hdc->out_wg_callback().set(FUNC(trs80m2_hdi_device::wg_w));

	HARDDISK(config, m_hdd[0], "st506_hdd");
	HARDDISK(config, m_hdd[1], "st506_hdd");
	HARDDISK(config, m_hdd[2], "st506_hdd");
	HARDDISK(config, m_hdd[3], "st506_hdd");
}

void trs80m2_hdi_device::map(address_map &map)
{
	trs80m2_wd1010_card_device_base::map(map);

	map(0x0, 0x0).r(FUNC(trs80m2_hdi_device::status_r));
	map(0x1, 0x1).rw(FUNC(trs80m2_hdi_device::control_r), FUNC(trs80m2_hdi_device::hdi_control_w));
}

void trs80m2_hdi_device::device_start()
{
	trs80m2_wd1010_card_device_base::device_start();

	std::fill_n(&m_ram[0], 0x4000, 0xff);

	save_item(NAME(m_intrq));
	save_item(NAME(m_hdwpl));
}

void trs80m2_hdi_device::device_reset()
{
	if (!m_ram_installed)
	{
		m_ram_installed = true;

		int const pair = BIT(m_config->read(), 4, 3);
		m_slot->bus().install_page_ram(pair << 1, 0xbfff, &m_ram[0]);
		m_slot->bus().install_page_ram((pair << 1) | 1, 0xbfff, &m_ram[0]);
	}

	m_hdwpl = false;

	trs80m2_wd1010_card_device_base::device_reset();

	m_ctc->trg1(0);
	update_intrq();
}

bool trs80m2_hdi_device::write_protected(int drive) const
{
	return m_hdd[drive]->exists() && m_hdd[drive]->is_readonly();
}

void trs80m2_hdi_device::update_drdy()
{
	m_hdc->drdy_w(m_hdd[selected_drive()]->exists() ? 1 : 0);
}

void trs80m2_hdi_device::update_intrq()
{
	m_ctc->trg0(m_intrq || m_hdwpl);
}

uint8_t trs80m2_hdi_device::status_r()
{
	uint8_t data = 0;

	for (int drive = 0; drive < 4; drive++)
		if (write_protected(drive))
			data |= 0x10 << drive;

	data |= m_hdwpl ? 0x02 : 0x00;
	data |= m_intrq ? 0x01 : 0x00;

	if (!machine().side_effects_disabled() && m_hdwpl)
	{
		m_hdwpl = false;
		update_intrq();
	}

	return data;
}

void trs80m2_hdi_device::hdi_control_w(uint8_t data)
{
	control_w(data);

	m_ctc->trg1(BIT(data, 6));
}

void trs80m2_hdi_device::intrq_w(int state)
{
	m_intrq = state;
	update_intrq();
}

void trs80m2_hdi_device::wg_w(int state)
{
	if (state && write_protected(selected_drive()))
	{
		m_hdwpl = true;
		update_intrq();
	}
}
