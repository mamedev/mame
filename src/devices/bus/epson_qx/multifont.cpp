// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Q10MF Multifont card
 *   Schematic: https://github.com/brijohn/qx10/raw/master/schematics/multifont/multifont.pdf
 *   Documentation: http://www.bitsavers.org/pdf/epson/QX-10/options/QX-10-multifont.pdf
 *
 *  I/O Ports:
 *     0xFC:
 *        Read:
 *          Reading from this will read a status byte followed by the character data for a requested character.
 *          Status byte:
 *          -------------------------------------------------
 *          | VALID | MODE | 0 0 | UP | RIGHT | LEFT | DOWN |
 *          -------------------------------------------------
 *          Valid: 1 = Character is valid, 0 = Character is invalid
 *          Mode:  1 = Pattern is for CRT, 0 = Pattern is for printer
 *          Up:    Extends upwards 0 = No, 1 = Yes
 *          Right: Extends to the right 0 = No, 1 = Yes
 *          Left:  Extends to the left 0 = No, 1 = Yes
 *          Down:  Extends downwards 0 = No, 1 = Yes
 *        Write:
 *          Data written to this consists of a command byte followed by two bytes representing
 *          the character code being requested, least significant byte first. (0x0000-0x0DFF)
 *          Command byte:
 *          ------------------------------------------
 *          |  1  |  MODE  |  0  0  0  0  0  |  BIT  |
 *          ------------------------------------------
 *          Mode: 0 = Pattern for printer, 1 = Pattern for CRT
 *          Bit:  0 = Non-DMA Pattern for CRT, 1 = DMA pattern for CRT
 *     0xFD:
 *        Read:
 *          This is the status register for the multifont card.
 *          ----------------------------------------------
 *          |  IBF  |  X  X  X  |  X  X  |  ERR  |  OBF  |
 *          ----------------------------------------------
 *          IBF: 1 = Data available to read, 0 = Data not available
 *          ERR: 1 = Error occured, 0 = No error
 *          OBF: 1 = Not ready for data to be written , 0 = Ready for data
 *        Write:
 *          Writing any value will trigger an INTL interrupt. This used by cpm to determine
 *          which slot the card is installed in. To reset the interrupt simply read from the
 *          status port.
 *
 *******************************************************************/
#include "emu.h"
#include "multifont.h"


//**************************************************************************
//  EPSON Q10MF MULTIFONT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_MULTIFONT, bus::epson_qx::multifont_device, "epson_qx_option_multifont", "Epson Multifont Card")


namespace bus::epson_qx {

ROM_START(multifont)
	/* This is the i8039 program ROM for the Q10MF Multifont card, and the actual font ROMs are missing (6 * HN43128) */
	ROM_REGION( 0x0800, "i8039", 0 )
	ROM_LOAD( "m12020a.3e", 0x0000, 0x0800, CRC(fa27f333) SHA1(73d27084ca7b002d5f370220d8da6623a6e82132))

	ROM_REGION( 0x4000, "font0", 0 )
	ROM_LOAD( "hn43128.1a", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font1", 0 )
	ROM_LOAD( "hn43128.2a", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font2", 0 )
	ROM_LOAD( "hn43128.1b", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font3", 0 )
	ROM_LOAD( "hn43128.2b", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font4", 0 )
	ROM_LOAD( "hn43128.3a", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font5", 0 )
	ROM_LOAD( "hn43128.3b", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font6", 0 )
	ROM_LOAD( "hn43128.1c", 0x0000, 0x4000, NO_DUMP)
	ROM_REGION( 0x4000, "font7", 0 )
	ROM_LOAD( "hn43128.2c", 0x0000, 0x4000, NO_DUMP)
ROM_END

INPUT_PORTS_START(multifont)
	PORT_START("IOBASE")
	PORT_CONFNAME(0x06, 0x04, "IO Base Address Selection")
	PORT_CONFSETTING(0x00, "&F8")
	PORT_CONFSETTING(0x02, "&FA")
	PORT_CONFSETTING(0x04, "&FC")
	PORT_CONFSETTING(0x06, "&FE")
INPUT_PORTS_END

//-------------------------------------------------
//  multifont_device - constructor
//-------------------------------------------------
multifont_device::multifont_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_MULTIFONT, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_mcu(*this, "mcu"),
	m_i8243(*this, "i8243"),
	m_fonts(*this, "font%u",0),
	m_ioport(*this, "IOBASE")
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------
ioport_constructor multifont_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(multifont);
}

//-------------------------------------------------
//  device_rom_region - device-specific roms
//-------------------------------------------------
const tiny_rom_entry *multifont_device::device_rom_region() const
{
	return ROM_NAME(multifont);
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void multifont_device::device_add_mconfig(machine_config &config)
{
	I8039(config, m_mcu, 9_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &multifont_device::rom_map);
	m_mcu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));
	m_mcu->p1_out_cb().set(FUNC(multifont_device::p1_w));
	m_mcu->p2_out_cb().set(FUNC(multifont_device::p2_w));
	m_mcu->bus_out_cb().set(FUNC(multifont_device::write_bus));
	m_mcu->bus_in_cb().set(FUNC(multifont_device::read_bus));
	m_mcu->t1_in_cb().set(FUNC(multifont_device::t1_r));
	m_mcu->t0_in_cb().set(FUNC(multifont_device::t0_r));
	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(multifont_device::rom_addr_w<0>));
	m_i8243->p5_out_cb().set(FUNC(multifont_device::rom_addr_w<4>));
	m_i8243->p6_out_cb().set(FUNC(multifont_device::rom_addr_w<8>));
	m_i8243->p7_out_cb().set(FUNC(multifont_device::rom_select_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void multifont_device::device_start()
{
	m_hard_reset = true;

	m_data_out = 0x00;
	m_data_in = 0x00;
	m_p1 = 0xff;
	m_p2 = 0xff;

	save_item(NAME(m_status));
	save_item(NAME(m_data_out));
	save_item(NAME(m_data_in));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_rom_sel));
	save_item(NAME(m_address));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_bus_reset));
	save_item(NAME(m_hard_reset));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void multifont_device::device_reset()
{
	uint8_t iobase;

	m_status = 0x00;

	if (m_hard_reset) {
		address_space &space = m_bus->iospace();
		iobase = (m_ioport->read() & 0x06) + 0xf8;
		space.install_device(iobase, iobase+1, *this, &multifont_device::map);
		m_hard_reset = false;
	}

	m_bus_reset = true;
}

uint8_t multifont_device::t0_r()
{
	return BIT(m_status, 0);
}

uint8_t multifont_device::t1_r()
{
	return BIT(m_status, 7);
}

void multifont_device::write_bus(uint8_t data)
{
	if (!m_bus_reset) {
		m_status = m_status | 0x80;
		m_data_out = data;
	} else {
		m_bus_reset = false;
	}
}

uint8_t multifont_device::read_bus()
{
	if (BIT(m_p1, 0)) {
		m_status = (m_status & 0xfe);
		return m_data_in;
	} else if (BIT(m_p1, 1)) {
		uint8_t *rom = m_fonts[m_rom_bank]->base();
		return rom[m_address];
	}
	return 0xff;
}

void multifont_device::p1_w(uint8_t data)
{
	m_p1 = data;
	if (BIT(m_p1, 7) == 0) {
		get_slot()->intl_w(ASSERT_LINE);
	}
}

void multifont_device::p2_w(uint8_t data)
{
	m_p2 = data;
	m_i8243->p2_w(m_p2 & 0x0f);
	update_bank_select();
	m_status = (m_status & 0xfd) | ((m_p2 & 0x20) >> 4);
}

template<int Shift>
void multifont_device::rom_addr_w(uint8_t data)
{
	m_address = (m_address & ~(0x00f << Shift)) | ((data & 0x0f) << Shift);
}

void multifont_device::rom_select_w(uint8_t data)
{
	m_rom_sel = (data & 0x0f) >> 2;
	m_address = (m_address & 0xfff) | ((data & 0x03) << 12);
	update_bank_select();
}

void multifont_device::update_bank_select()
{
	m_rom_bank = ((m_p2 & 0x80) >> 5) | (m_rom_sel & 0x03);
}

uint8_t multifont_device::read(offs_t offset)
{
	if (offset) {
		if (!machine().side_effects_disabled()) {
			if (BIT(m_p1, 7)) {
				get_slot()->intl_w(CLEAR_LINE);
			}
		}
		return m_status;
	}
	if (!machine().side_effects_disabled()) {
		m_status = (m_status & 0x7f);
	}
	return m_data_out;
}

void multifont_device::write(offs_t offset, uint8_t data)
{
	if (offset) {
		get_slot()->intl_w(ASSERT_LINE);
	} else {
		m_status = m_status | 0x01;
		m_data_in = data;
	}
}

void multifont_device::rom_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).rom().region("i8039", 0);
}

void multifont_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(multifont_device::read), FUNC(multifont_device::write));
}

} // namespace bus::epson_qx
