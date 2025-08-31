// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Micro and Master series common code

******************************************************************************/

#include "emu.h"
#include "bbc.h"

#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/uef_cas.h"
#include "formats/csw_cas.h"


INPUT_PORTS_START(bbc_config)
	PORT_START("BBCCONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Monitor") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_state::reset_palette), 0) PORT_CONDITION("BBCCONFIG", 0x08, EQUALS, 0x00)
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x01, "B&W")
	PORT_CONFSETTING(    0x02, "Green")
	PORT_CONFSETTING(    0x03, "Amber")
	PORT_CONFNAME( 0x04, 0x00, "Econet")
	PORT_CONFSETTING(    0x00, DEF_STR( No ))
	PORT_CONFSETTING(    0x04, DEF_STR( Yes ))
	PORT_CONFNAME( 0x08, 0x00, "VideoNuLA") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_state::reset_palette), 0)
	PORT_CONFSETTING(    0x00, DEF_STR( No ))
	PORT_CONFSETTING(    0x08, DEF_STR( Yes ))
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bbc_state::reset_palette)
{
	m_vnula.disable = !BIT(m_bbcconfig.read_safe(0), 3);
	if (m_vnula.disable)
		update_palette((monitor_type)(m_bbcconfig.read_safe(0) & 0x03));
	else
		update_palette(monitor_type::COLOUR);
}


void bbc_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_HFE_FORMAT);
	//fr.add(FLOPPY_HFE3_FORMAT);
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_ACORN_DOS_FORMAT);
	fr.add(FLOPPY_OPUS_DDOS_FORMAT);
	fr.add(FLOPPY_OPUS_DDCPM_FORMAT);
	fr.add(FLOPPY_FSD_FORMAT);
}


/****************************************
  BBC Model B memory handling functions
****************************************/

uint8_t bbc_state::mos_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_mos())
		return m_internal->mos_r(offset);
	else
		return m_region_mos->base()[offset];
}

void bbc_state::mos_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_mos())
		m_internal->mos_w(offset, data);
}


/**************************************
   Analogue Joystick
**************************************/

int bbc_state::get_analogue_input(int channel_number)
{
	return m_analog->ch_r(channel_number) << 8;
}


/***************************************
   Cassette Motor
****************************************/

void bbc_state::cassette_motor(int state)
{
	// cassette relay sound
	m_samples->start(0, state ? 1 : 0);

	m_cassette->change_state(state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	m_motor_led = state ? 0 : 1;
}


/**************************************
   NMI's
***************************************/

void bbc_state::update_nmi()
{
	if (m_fdc_irq || m_fdc_drq || (m_adlc_ie && m_adlc_irq) || m_bus_nmi)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void bbc_state::econet_int_enable(int enabled)
{
	if (!machine().side_effects_disabled())
	{
		m_adlc_ie = enabled;
		update_nmi();
	}
}

void bbc_state::adlc_irq_w(int state)
{
	m_adlc_irq = state;
	update_nmi();
}

void bbc_state::fdc_intrq_w(int state)
{
	m_fdc_irq = state;
	update_nmi();
}

void bbc_state::fdc_drq_w(int state)
{
	m_fdc_drq = state;
	update_nmi();
}

void bbc_state::bus_nmi_w(int state)
{
	m_bus_nmi = state;
	update_nmi();
}


/**************************************
   Machine Initialisation functions
***************************************/

void bbc_state::init_bbc()
{
	// vertical sync pulse from video circuit
	m_sysvia->write_ca1(1);

	// light pen strobe detect
	m_sysvia->write_cb2(1);

	update_palette(monitor_type::COLOUR);
}

void bbc_state::init_ltmp()
{
	init_bbc();

	// LTM machines used a 9" Hantarex MT3000 green monitor
	update_palette(monitor_type::GREEN);
}

void bbc_state::init_cfa()
{
	init_bbc();

	update_palette(monitor_type::GREEN);
}


/**************************************
   Helpers for ROM management
***************************************/

std::string bbc_state::get_rom_name(uint8_t* header)
{
	std::string title = "";
	// check for valid copyright
	uint8_t offset = header[7];
	if (header[offset + 0] == 0 && header[offset + 1] == '(' && header[offset + 2] == 'C' && header[offset + 3] == ')')
	{
		// extract ROM title from header
		for (int pos = 9; pos < header[7]; pos++)
			title.append(1, header[pos] == 0x00 ? 0x20 : header[pos]);
	}
	return title;
}

void bbc_state::insert_device_rom(memory_region *rom)
{
	std::string region_tag;

	if (rom == nullptr) return;

	// check whether ROM already present
	for (int bank = 15; bank >= 0; bank--)
	{
		// compare first 1K of bank with what we want to insert
		if (!memcmp(rom->base(), m_region_rom->base() + (bank * 0x4000), 0x400))
		{
			osd_printf_verbose("Found '%s' in romslot%d\n", get_rom_name(rom->base()), bank);
			return;
		}
	}

	// iterate over romslots until an empty socket is found
	for (int bank = 15; bank >= 0; bank--)
	{
		// if bank has socket and is empty
		if (m_rom[bank] && !memregion(region_tag.assign(m_rom[bank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			uint8_t *swr = m_region_rom->base() + (bank * 0x4000);
			switch (rom->bytes())
			{
			case 0x8000:
				// 32K (or 2x16K) ROM, check whether ROM exists in this and next bank
				if (swr[0x0006] == 0xff && swr[0x4006] == 0xff)
				{
					memcpy(m_region_rom->base() + (bank * 0x4000), rom->base(), rom->bytes());
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base() + 0x4000), bank + 1);
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()), bank);
					return;
				}
				break;
			case 0x4000:
			case 0x2000:
				// 16/8K ROM, check whether ROM exists in this bank
				if (swr[0x0006] == 0xff)
				{
					memcpy(m_region_rom->base() + (bank * 0x4000), rom->base(), rom->bytes());
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()), bank);
					return;
				}
				break;
			}
		}
	}

	// unable to insert ROM
	fatalerror("Unable to insert '%s'. Add a sideways ROM board for more sockets.\n", get_rom_name(rom->base()).c_str());
}

void bbc_state::setup_device_roms()
{
	std::string region_tag;
	memory_region *rom_region;
	device_t *exp_device;
	device_t *ext_device;

	// insert ROM(s) for internal expansion boards
	if (m_internal && (exp_device = dynamic_cast<device_t*>(m_internal->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	// insert ROM for FDC devices (BBC Model B only), always place into romslot 0
	if (m_fdc && m_fdc->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_fdc->get_card_device())))
	{
		if (exp_device->memregion("dfs_rom"))
		{
			memcpy(m_region_rom->base(), exp_device->memregion("dfs_rom")->base(), exp_device->memregion("dfs_rom")->bytes());
		}
	}

	// configure romslots
	for (int i = 0; i < 16; i++)
	{
		if (m_rom[i] && (rom_region = memregion(region_tag.assign(m_rom[i]->tag()).append(BBC_ROM_REGION_TAG).c_str())))
		{
			if (m_rom[i]->get_rom_size())
				memcpy(m_region_rom->base() + (i * 0x4000), rom_region->base(), std::min((int32_t)m_rom[i]->get_slot_size(), (int32_t)m_rom[i]->get_rom_size()));
			else
				memset(m_region_rom->base() + (i * 0x4000), 0, 0x4000);
		}
	}

	// configure cartslots
	for (int i = 0; i < 2; i++)
	{
		if (m_cart[i] && (rom_region = memregion(region_tag.assign(m_cart[i]->tag()).append(ELECTRON_CART_ROM_REGION_TAG).c_str())))
		{
			memcpy(m_region_rom->base() + (i * 0x8000), rom_region->base(), rom_region->bytes());
		}
	}

	// insert ROM(s) for Expansion port devices (Compact only), with additional Mertec slots
	if (m_exp && (ext_device = dynamic_cast<device_t*>(m_exp->get_card_device())))
	{
		// only the Mertec device has ext_rom region and should be placed in romslots 0,1
		if (ext_device->memregion("ext_rom"))
		{
			memcpy(m_region_rom->base(), ext_device->memregion("ext_rom")->base(), ext_device->memregion("ext_rom")->bytes());
		}

		bbc_analogue_slot_device* analogue_port = ext_device->subdevice<bbc_analogue_slot_device>("analogue");
		if (analogue_port && (exp_device = dynamic_cast<device_t*>(analogue_port->get_card_device())))
		{
			insert_device_rom(exp_device->memregion("exp_rom"));
		}

		bbc_userport_slot_device* user_port = ext_device->subdevice<bbc_userport_slot_device>("userport");
		if (user_port && (exp_device = dynamic_cast<device_t*>(user_port->get_card_device())))
		{
			insert_device_rom(exp_device->memregion("exp_rom"));
		}

		bbc_1mhzbus_slot_device* exp_port = ext_device->subdevice<bbc_1mhzbus_slot_device>("2mhzbus");
		while (exp_port != nullptr)
		{
			if ((exp_device = dynamic_cast<device_t*>(exp_port->get_card_device())))
			{
				insert_device_rom(exp_device->memregion("exp_rom"));
				exp_port = exp_device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus");
			}
			else
			{
				exp_port = nullptr;
			}
		}
	}

	// insert ROM(s) for 1MHz bus devices, with pass-through
	if (m_1mhzbus)
	{
		bbc_1mhzbus_slot_device* exp_port = m_1mhzbus;
		while (exp_port != nullptr)
		{
			if ((exp_device = dynamic_cast<device_t*>(exp_port->get_card_device())))
			{
				insert_device_rom(exp_device->memregion("exp_rom"));
				exp_port = exp_device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus");
			}
			else
			{
				exp_port = nullptr;
			}
		}
	}

	// insert ROM(s) for Tube devices
	if (m_tube && m_tube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_tube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_intube && m_intube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_intube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_extube && m_extube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_extube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	// insert ROM(s) for Userport devices
	if (m_userport && (exp_device = dynamic_cast<device_t*>(m_userport->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	// insert ROM(s) for Analogue port devices
	if (m_analog && (exp_device = dynamic_cast<device_t*>(m_analog->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	// list all inserted ROMs
	for (int i = 15; i >= 0; i--)
	{
		osd_printf_verbose("ROM %X : %s\n", i, get_rom_name(m_region_rom->base() + (i * 0x4000)));
	}
}


/**************************************
   Machine start
***************************************/

void bbc_state::machine_start()
{
	m_motor_led.resolve();

	setup_device_roms();

	m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x0000, 0xffff, "cpu_clock_tap",
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); },
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); });

	m_romsel = 0;
	m_adlc_irq = 0;
	m_adlc_ie = 0;
	m_bus_nmi = 0;
	m_fdc_irq = 0;
	m_fdc_drq = 0;
}

void bbc_state::set_cpu_clock(offs_t offset)
{
	// CPU mostly runs at 2MHz, including RAM and ROM accesses
	double clock_scale = 1.0;

	// CPU slows to 1MHz when addressing slow devices
	switch (offset & 0xff00)
	{
	case 0xfc00: // FRED
	case 0xfd00: // JIM
		clock_scale = 0.5;
		break;
	case 0xfe00: // SHEILA
		switch (offset & 0xe0)
		{
		case 0x00: // CRTC, ACIA, SERPROC
		case 0x40: // VIA A
		case 0x60: // VIA B
		case 0xc0: // ADC
			clock_scale = 0.5;
			break;
		}
		break;
	}

	m_maincpu->set_clock_scale(clock_scale);
}
