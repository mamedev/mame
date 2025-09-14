// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ALA11 - Acorn Plus 1

    The Acorn Plus 1 added two ROM cartridge slots, an analogue interface
    (supporting two channels) and a Centronics parallel port. The
    analogue interface was normally used for joysticks, the parallel for
    a printer. The ROM slots could be booted from via the "Shift+BREAK"
    key-combination. (The slot at the front of the interface took priority
    if both were populated).

**********************************************************************/


#include "emu.h"
#include "plus1.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_PLUS1, electron_plus1_device, "electron_plus1", "Acorn Plus 1 Expansion")
DEFINE_DEVICE_TYPE(ELECTRON_AP1, electron_ap1_device, "electron_ap1", "P.R.E.S. Advanced Plus 1")
DEFINE_DEVICE_TYPE(ELECTRON_AP6, electron_ap6_device, "electron_ap6", "P.R.E.S. Advanced Plus 6")


//-------------------------------------------------
//  ROM( plus1 )
//-------------------------------------------------

ROM_START( plus1 )
	// Bank 12 Expansion module operating system
	ROM_REGION( 0x2000, "exp_rom", 0 )
	ROM_DEFAULT_BIOS("plus1")
	ROM_SYSTEM_BIOS(0, "plus1", "Expansion 1.00")
	ROMX_LOAD("plus1.rom", 0x0000, 0x1000, CRC(ac30b0ed) SHA1(2de04ab7c81414d6c9c967f965c53fc276392463), ROM_BIOS(0))
	ROM_RELOAD(            0x1000, 0x1000)
ROM_END

ROM_START( ap1 )
	// Bank 12 Expansion module operating system
	ROM_REGION( 0x2000, "exp_rom", 0 )
	ROM_DEFAULT_BIOS("ap2")
	ROM_SYSTEM_BIOS(0, "ap1", "PRES Expansion 1.1")
	ROMX_LOAD("presplus1.rom", 0x0000, 0x1000, CRC(8ef1e0e5) SHA1(080e1b788b3fe4fa272cd2cc792293eb7b874e82), ROM_BIOS(0))
	ROM_RELOAD(                0x1000, 0x1000)

	ROM_SYSTEM_BIOS(1, "ap2", "PRES AP2 Support 1.23")
	ROMX_LOAD("presap2_123.rom", 0x0000, 0x2000, CRC(f796689c) SHA1(bc40a79e6d2b4cb5e549d5d21f673c66a661850d), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "ap6", "RH Plus 1 1.33")
	ROMX_LOAD("ap6v133.rom", 0x0000, 0x2000, CRC(566c7bfe) SHA1(f1053109ea236dd3e4b91ec64859480e8c2ed549), ROM_BIOS(2))
ROM_END

ROM_START( ap6 )
	// Bank 12 Expansion module operating system
	ROM_REGION( 0x4000, "exp_rom", 0 )
	ROM_DEFAULT_BIOS("ap6")
	ROM_SYSTEM_BIOS(0, "ap2", "PRES AP2 Support 1.23")
	ROMX_LOAD("presap2_123.rom", 0x0000, 0x2000, CRC(f796689c) SHA1(bc40a79e6d2b4cb5e549d5d21f673c66a661850d), ROM_BIOS(0))
	ROM_RELOAD(                  0x2000, 0x2000)

	ROM_SYSTEM_BIOS(1, "ap6", "RH Plus 1 1.33")
	ROMX_LOAD("ap6v133t.rom", 0x0000, 0x4000, CRC(364591eb) SHA1(316a25aeeda0266dae510eea52324b087875740f), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( plus1 )
//-------------------------------------------------

static INPUT_PORTS_START( ap6 )
	PORT_START("LINKS")
	PORT_CONFNAME(0x01, 0x01, "J1 ROM 13")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "Enabled")
	PORT_CONFNAME(0x02, 0x02, "J2 ROM 4")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x02, "Enabled")
	PORT_CONFNAME(0x04, 0x04, "J3 ROM 4,5,6,7")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x04, "Enabled")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_ap6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ap6 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_plus1_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; });
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* adc */
	ADC0844(config, m_adc);
	m_adc->intr_callback().set([this](int state) { m_adc_ready = !state; });
	m_adc->ch1_callback().set([this]() { return m_analogue->ch_r(0) >> 8; });
	m_adc->ch2_callback().set([this]() { return m_analogue->ch_r(1) >> 8; });
	m_adc->ch3_callback().set([this]() { return m_analogue->ch_r(2) >> 8; });
	m_adc->ch4_callback().set([this]() { return m_analogue->ch_r(3) >> 8; });

	BBC_ANALOGUE_SLOT(config, m_analogue, bbc_analogue_devices_no_lightpen, "acornjoy");

	/* cartridges */
	ELECTRON_CARTSLOT(config, m_cart_sk1, DERIVED_CLOCK(1, 1), electron_cart, nullptr);
	m_cart_sk1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_cart_sk1->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
	ELECTRON_CARTSLOT(config, m_cart_sk2, DERIVED_CLOCK(1, 1), electron_cart, nullptr);
	m_cart_sk2->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_cart_sk2->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}

void electron_ap6_device::device_add_mconfig(machine_config &config)
{
	electron_plus1_device::device_add_mconfig(config);

	/* rom sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[0]->set_device_load(FUNC(electron_ap6_device::rom1_load));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[1]->set_device_load(FUNC(electron_ap6_device::rom2_load));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[2]->set_device_load(FUNC(electron_ap6_device::rom3_load));
	GENERIC_SOCKET(config, m_rom[3], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[3]->set_device_load(FUNC(electron_ap6_device::rom4_load));
	GENERIC_SOCKET(config, m_rom[4], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[4]->set_device_load(FUNC(electron_ap6_device::rom5_load));
	GENERIC_SOCKET(config, m_rom[5], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[5]->set_device_load(FUNC(electron_ap6_device::rom6_load));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *electron_plus1_device::device_rom_region() const
{
	return ROM_NAME( plus1 );
}

const tiny_rom_entry *electron_ap1_device::device_rom_region() const
{
	return ROM_NAME( ap1 );
}

const tiny_rom_entry *electron_ap6_device::device_rom_region() const
{
	return ROM_NAME( ap6 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_plus1_device - constructor
//-------------------------------------------------

electron_plus1_device::electron_plus1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_irqs(*this, "irqs")
	, m_exp_rom(*this, "exp_rom")
	, m_cart_sk1(*this, "cart_sk1")
	, m_cart_sk2(*this, "cart_sk2")
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
	, m_analogue(*this, "analogue")
	, m_adc(*this, "adc")
	, m_romsel(0)
	, m_centronics_busy(0)
	, m_adc_ready(0)
{
}

electron_plus1_device::electron_plus1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: electron_plus1_device(mconfig, ELECTRON_PLUS1, tag, owner, clock)
{
}

electron_ap1_device::electron_ap1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: electron_plus1_device(mconfig, ELECTRON_AP1, tag, owner, clock)
{
}

electron_ap6_device::electron_ap6_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock)
	: electron_plus1_device(mconfig, ELECTRON_AP6, tag, owner, clock)
	, m_rom(*this, "rom%u", 1)
	, m_links(*this, "LINKS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_plus1_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
}

void electron_ap6_device::device_start()
{
	electron_plus1_device::device_start();

	m_ram = make_unique_clear<uint8_t[]>(0x18000);
	memset(m_ram.get(), 0xff, 0x18000);

	m_bank_locked[0] = false;
	m_bank_locked[1] = false;

	/* register for save states */
	save_pointer(NAME(m_ram), 0x18000);
	save_item(NAME(m_bank_locked));
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_plus1_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 0:
		case 1:
			data = m_cart_sk2->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			data = m_cart_sk1->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 12:
			data = m_exp_rom->base()[offset & 0x1fff];
			break;
		case 13:
			data &= m_cart_sk1->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			data &= m_cart_sk2->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			data &= m_cart_sk1->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);
			data &= m_cart_sk2->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc70)
			{
				data &= m_adc->read();
			}
			else if (offset == 0xfc72)
			{
				// Status: b7: printer Busy
				//         b6: ADC conversion end
				//         b5: Fire Button 1
				//         b4: Fire Button 2
				data &= (m_centronics_busy << 7) | (m_adc_ready << 6) | m_analogue->pb_r() | 0x0f;
			}
			break;

		case 0xfd:
			data &= m_cart_sk1->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			data &= m_cart_sk2->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			break;
		}
	}

	return data;
}


uint8_t electron_ap6_device::expbus_r(offs_t offset)
{
	uint8_t data = electron_plus1_device::expbus_r(offset);

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 4:
			if (BIT(m_links->read(), 1) && BIT(m_links->read(), 2)) // ROM 4 enabled
			{
				if (m_rom[m_romsel - 4]->exists())
					data = m_rom[m_romsel - 4]->read_rom(offset & 0x3fff);
				else
					data = m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)];
			}
			break;

		case 5: case 6: case 7:
			if (BIT(m_links->read(), 2)) // ROM 4,5,6,7 enabled
			{
				if (m_rom[m_romsel - 4]->exists())
					data = m_rom[m_romsel - 4]->read_rom(offset & 0x3fff);
				else
					data = m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)];
			}
			break;

		case 12:
			data = m_exp_rom->base()[offset & 0x3fff];
			break;

		case 13:
			if (BIT(m_links->read(), 0)) // ROM 13 enabled
			{
				if (m_rom[m_romsel - 9]->exists())
					data &= m_rom[m_romsel - 9]->read_rom(offset & 0x3fff);
				else
					data &= m_ram[(m_romsel - 9) << 14 | (offset & 0x3fff)];
			}
			break;

		case 14:
			if (m_rom[m_romsel - 9]->exists())
				data &= m_rom[m_romsel - 9]->read_rom(offset & 0x3fff);
			else
				data &= m_ram[(m_romsel - 9) << 14 | (offset & 0x3fff)];
			break;
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_plus1_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 0:
		case 1:
			m_cart_sk2->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			m_cart_sk1->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 13:
			m_cart_sk1->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 0, 1);
			m_cart_sk2->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 0, 1);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			m_cart_sk1->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);
			m_cart_sk2->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc70)
			{
				m_adc->write(data);
			}
			else if (offset == 0xfc71)
			{
				m_cent_data_out->write(data);
				m_centronics->write_strobe(0);
				m_centronics->write_strobe(1);
			}
			break;

		case 0xfd:
			m_cart_sk1->write(offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
			m_cart_sk2->write(offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
			break;

		case 0xfe:
			if ((offset == 0xfe05) && !(data & 0xf0))
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}


void electron_ap6_device::expbus_w(offs_t offset, uint8_t data)
{
	electron_plus1_device::expbus_w(offset, data);

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 4:
			if (BIT(m_links->read(), 1) && BIT(m_links->read(), 2)) // ROM 4 enabled
			{
				if (!m_rom[m_romsel - 4]->exists())
					m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)] = data;
			}
			break;

		case 7:
			if (BIT(m_links->read(), 2)) // ROM 4,5,6,7 enabled
			{
				if (!m_rom[m_romsel - 4]->exists())
					m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)] = data;
			}
			break;

		case 5: case 6:
			if (BIT(m_links->read(), 2)) // ROM 4,5,6,7 enabled
			{
				if (!m_rom[m_romsel - 4]->exists() && !m_bank_locked[0])
					m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)] = data;
			}
			break;

		case 13:
			if (BIT(m_links->read(), 0)) // ROM 13 enabled
			{
				if (!m_rom[m_romsel - 9]->exists() && !m_bank_locked[1])
					m_ram[(m_romsel - 9) << 14 | (offset & 0x3fff)] = data;
			}
			break;

		case 14:
			if (!m_rom[m_romsel - 9]->exists())
				m_ram[(m_romsel - 9) << 14 | (offset & 0x3fff)] = data;
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			switch (offset & 0xff)
			{
			case 0xdc:
				m_bank_locked[0] = false;
				break;
			case 0xdd:
				m_bank_locked[0] = true;
				break;
			case 0xde:
				m_bank_locked[1] = false;
				break;
			case 0xdf:
				m_bank_locked[1] = true;
				break;
			}
			break;
		}
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

std::pair<std::error_condition, std::string> electron_ap6_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t const size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size: Only 8K/16K is supported");

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *const crt = slot->get_rom_base();
	if (size <= 0x2000)
		memcpy(crt + 0x2000, crt, 0x2000);

	return std::make_pair(std::error_condition(), std::string());
}
