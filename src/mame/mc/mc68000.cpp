// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    mc-68000-Computer

    Hardware:
    - MC68000
    - 16 MHz XTAL
    - 128k or 512k RAM
    - 2x 6522 VIA
    - 6845 CRTC
    - 8 expansion slots

    TODO:
    - Cassette
    - Color/brightness levels
    - Sound
    - Joysticks

    Notes:
    - This computer was described in the "mc" magazine by Franzis Verlag.
      You could build it yourself or order it already assembled.
    - Press ESC at the boot prompt to enter the monitor
    - A later version is called "System II" and features a built-in floppy
      and SASI controller

****************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/mc68000/sysbus.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "video/mc6845.h"
#include "screen.h"


#define LOG_IO_READ  (1U << 1)
#define LOG_IO_WRITE (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_IO_WRITE | LOG_IO_READ)
#include "logmacro.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mc68000_state : public driver_device
{
public:
	mc68000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq3(*this, "irq3"),
		m_ram(*this, RAM_TAG),
		m_crtc(*this, "crtc"),
		m_via(*this, "via%u", 0U),
		m_sysbus(*this, "sysbus"),
		m_centronics_latch(*this, "centronics_latch"),
		m_centronics_error(*this, "centronics_error"),
		m_centronics(*this, "centronics"),
		m_serial(*this, "serial"),
		m_apm_view(*this, "apm"),
		m_eprom(*this, "eprom%u", 0U),
		m_switches(*this, "switches")
	{ }

	void mc68000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<input_merger_device> m_irq3;
	required_device<ram_device> m_ram;
	required_device<mc6845_device> m_crtc;
	required_device_array<via6522_device, 2> m_via;
	required_device<mc68000_sysbus_device> m_sysbus;
	required_device<output_latch_device> m_centronics_latch;
	required_device<input_merger_device> m_centronics_error;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_serial;
	memory_view m_apm_view;
	required_memory_region_array<2> m_eprom;
	required_ioport m_switches;

	uint16_t *m_ram_base;
	uint32_t m_ram_mask;

	std::unique_ptr<uint8_t[]> m_addr_decode;

	uint8_t m_uvia_porta;
	uint8_t m_key;
	bool m_ibmkbd_clock;
	bool m_ibmkbd_data;
	uint8_t m_ibmkbd_bits;

	void mem_map(address_map &map) ATTR_COLD;
	void vector_map(address_map &map) ATTR_COLD;
	uint16_t memory_r(offs_t offset, uint16_t mem_mask = ~0);
	void memory_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void ibmkbd_clock_w(int state);
	void ibmkbd_data_w(int state);

	MC6845_UPDATE_ROW(crtc_update_row);

	void lvia_porta_w(uint8_t data);
	void uvia_porta_w(uint8_t data);
	void addr_decode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mc68000_state::mem_map(address_map &map)
{
	map(0x000000, 0xffffff).view(m_apm_view);
	m_apm_view[0](0x000000, 0x007fff).mirror(0xff8000).rom().region("eprom0", 0);
	m_apm_view[0](0x000000, 0xffffff).w(FUNC(mc68000_state::addr_decode_w));
	m_apm_view[1](0x000000, 0xffffff).rw(FUNC(mc68000_state::memory_r), FUNC(mc68000_state::memory_w));
}

void mc68000_state::vector_map(address_map &map)
{
	// vector number fetched from memory at 0xfffff0 to 0xffffff
	map(0xfffff0, 0xffffff).lr16(NAME([this](offs_t offset) -> uint16_t { return memory_r((0xfffff0 >> 1) | offset); }));
}

uint16_t mc68000_state::memory_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t code = m_addr_decode[offset >> 13];
	uint16_t data = 0x0000;

	switch (code)
	{
	// ram
	case 0x0:
		data = m_ram_base[offset & m_ram_mask];
		break;

	// io
	case 0x1:
		if (machine().side_effects_disabled())
			break;

		LOGMASKED(LOG_IO_READ, "Read from IO: %06x = %04x & %04x\n", offset << 1, data, mem_mask);

		// ic45, 74ls139
		switch ((offset >> 11) & 0x03)
		{
		case 0:
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 1))
				data = m_crtc->register_r() << 8;
			break;

		case 1:
			if (ACCESSING_BITS_0_7)
				data |= m_via[0]->read(offset) << 0;
			if (ACCESSING_BITS_8_15)
				data |= m_via[1]->read(offset) << 8;
			break;

		case 2:
			data = m_sysbus->floppy_r(offset, mem_mask);
			break;

		case 3:
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 0))
			{
				data = m_key << 8;

				// a read here also selects switch 1 to be read from cb1
				m_via[1]->write_cb1(1);
				m_via[1]->write_cb1(BIT(m_switches->read(), 1));
			}
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 1))
				m_apm_view.select(0);
			break;
		}

		LOGMASKED(LOG_IO_READ, "IO data = %04x\n", data);

		break;

	// second eprom slot
	case 0x2:
		data = m_eprom[1]->as_u16(offset & 0x3fff);
		break;

	// monitor rom
	case 0x3:
		data = m_eprom[0]->as_u16(offset & 0x3fff);
		break;

	// bus error
	case 0x6:
		if (!machine().side_effects_disabled())
			m_maincpu->trigger_bus_error();
		break;

	// expansion slots
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		data = m_sysbus->slot_r(code - 0x8, offset, mem_mask);
		break;

	default:
		LOG("Unhandled code %x: %06x = %04x & %04x\n", code, offset, data, mem_mask);
	}

	return data & mem_mask;
}

void mc68000_state::memory_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint8_t code = m_addr_decode[offset >> 13];

	switch (code)
	{
	// ram
	case 0x0:
		COMBINE_DATA(&m_ram_base[offset & m_ram_mask]);
		break;

	// io
	case 0x1:
		LOGMASKED(LOG_IO_WRITE, "Write to IO: %06x = %04x & %04x\n", offset << 1, data, mem_mask);

		// ic45, 74ls139
		switch ((offset >> 11) & 0x03)
		{
		case 0:
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 0))
				m_crtc->address_w(data >> 8);
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 1))
				m_crtc->register_w(data >> 8);
			break;

		case 1:
			if (ACCESSING_BITS_0_7)
				m_via[0]->write(offset, data >> 0);
			if (ACCESSING_BITS_8_15)
				m_via[1]->write(offset, data >> 8);
			break;

		case 2:
			m_sysbus->floppy_w(offset, data, mem_mask);
			break;

		case 3:
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 0))
			{
				m_centronics_latch->write(data >> 8);

				// a write here also selects switch 0 to be read from cb1
				m_via[1]->write_cb1(1);
				m_via[1]->write_cb1(BIT(m_switches->read(), 0));
			}
			if (ACCESSING_BITS_8_15 && (BIT(offset, 0) == 1))
				LOGMASKED(LOG_IO_WRITE, "Unhandled volume latch write\n");
			break;
		}

		break;

	// second eprom slot
	case 0x2:
		LOG("Write to second EPROM: %06x = %04x & %04x\n", offset, data, mem_mask);
		break;

	// monitor eprom
	case 0x3:
		LOG("Write to monitor EPROM: %06x = %04x & %04x\n", offset, data, mem_mask);
		break;

	// bus error
	case 0x6:
		m_maincpu->trigger_bus_error();
		break;

	// expansion slots
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		m_sysbus->slot_w(code - 0x8, offset, data, mem_mask);
		break;

	default:
		LOG("Unhandled code %x: %06x = %04x & %04x\n", code, offset, data, mem_mask);
	}
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( mc68000 )
	PORT_START("switches")
	PORT_DIPNAME(0x01, 0x01, "IO Mode")
	PORT_DIPLOCATION("DIL:1")
	PORT_DIPSETTING(   0x00, "Terminal")
	PORT_DIPSETTING(   0x01, "Internal")
	PORT_DIPNAME(0x02, 0x02, "Columns")
	PORT_DIPLOCATION("DIL:2")
	PORT_DIPSETTING(   0x00, "40")
	PORT_DIPSETTING(   0x02, "80")
	// DIL:3 and DIL:4 select parallel keyboard strobe polarity
INPUT_PORTS_END

void mc68000_state::ibmkbd_clock_w(int state)
{
	if (state && !m_ibmkbd_clock)
	{
		if (m_ibmkbd_bits >= 1 && m_ibmkbd_bits <= 8)
		{
			m_key <<= 1;
			m_key |= m_ibmkbd_data ? 0x01 : 0x00;
		}

		if (m_ibmkbd_bits == 9)
		{
			m_ibmkbd_bits = 0;

			m_via[0]->write_ca2(1);
			m_via[0]->write_ca2(0);
		}
		else
		{
			m_ibmkbd_bits++;
		}
	}

	m_ibmkbd_clock = bool(state);
}

void mc68000_state::ibmkbd_data_w(int state)
{
	m_ibmkbd_data = bool(state);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_UPDATE_ROW( mc68000_state::crtc_update_row )
{
	// page select
	offs_t offset = (m_uvia_porta & 0x07 & (m_ram_mask >> 15)) << 16;

	// offset into page
	offset |= ((ma & 0x3000) << 2) | ((ra & 0x07) << 11) | (ma & 0x7fe);

	rgb_t fg = rgb_t::white();
	rgb_t bg = rgb_t::black();

	for (int i = 0; i < x_count / 2; i++)
	{
		uint16_t data = m_ram_base[(offset >> 1) + i];

		// cursor
		if (cursor_x == (i * 2) + 0)
			data |= 0xff00;
		else if  (cursor_x == (i * 2) + 1)
			data |= 0x00ff;

		// lines 8 and 9 are blank
		if (ra > 7)
			data = 0x0000;

		// draw 16 pixels of the cell
		for (int x = 0; x < 16; x++)
			bitmap.pix(y, x + i * 16) = BIT(data, 15 - x) ? fg : bg;
	}
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void mc68000_state::lvia_porta_w(uint8_t data)
{
	// 7-------  serial cts (read)
	// -6------  40/80 char mode
	// --5-----  serial tx
	// ---4----  serial dtr
	// ----3---  ibm/pc keyboard (read)
	// -----2--  serial rx (read)
	// ------1-  cassette write
	// -------0  cassette read (read)

	m_serial->write_dtr(BIT(data, 4));
	m_serial->write_txd(BIT(~data, 5));
}

void mc68000_state::uvia_porta_w(uint8_t data)
{
	// 7-------  centronics error (read)
	// -6------  centronics strobe
	// --5-----  joystick control
	// ---43---  color helper bits
	// -----210  video base address

	m_uvia_porta = data;
	m_centronics->write_strobe(BIT(data, 6));
}

void mc68000_state::addr_decode_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_addr_decode[offset >> 13] = data & 0x0f;

	// a write here deselects the address decoder
	if (ACCESSING_BITS_8_15)
		m_apm_view.select(1);
}

void mc68000_state::machine_start()
{
	// allocate space for address decoder ram
	m_addr_decode = std::make_unique<uint8_t[]>(0x400);

	// base ram
	m_ram_base = reinterpret_cast<uint16_t *>(m_ram->pointer());
	m_ram_mask = m_ram->mask() >> 1;

	// register for save states
	save_pointer(NAME(m_addr_decode), 0x400);
	save_item(NAME(m_uvia_porta));
	save_item(NAME(m_key));
	save_item(NAME(m_ibmkbd_clock));
	save_item(NAME(m_ibmkbd_data));
	save_item(NAME(m_ibmkbd_bits));
}

void mc68000_state::machine_reset()
{
	m_apm_view.select(0);

	m_key = 0x00;
	m_ibmkbd_bits = 0;

	// enable ibm pc keyboard mode
	m_via[0]->write_pa3(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void mc68000_state::mc68000(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mc68000_state::mem_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &mc68000_state::vector_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq3);
	m_irq3->output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ3);

	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("512K");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 250);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mc68000_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(m_via[1], FUNC(via6522_device::write_ca1));

	MOS6522(config, m_via[0], 16_MHz_XTAL / 2 / 10); // ic55
	m_via[0]->irq_handler().set(m_irq3, FUNC(input_merger_device::in_w<0>));
	m_via[0]->writepa_handler().set(FUNC(mc68000_state::lvia_porta_w));

	MOS6522(config, m_via[1], 16_MHz_XTAL / 2 / 10); // ic56
	m_via[1]->irq_handler().set(m_irq3, FUNC(input_merger_device::in_w<1>));
	m_via[1]->writepa_handler().set(FUNC(mc68000_state::uvia_porta_w));

	MC68000_SYSBUS(config, m_sysbus, 16_MHz_XTAL);
	m_sysbus->irq1_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_sysbus->irq2_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	m_sysbus->irq3_cb().set(m_irq3, FUNC(input_merger_device::in_w<2>));
	m_sysbus->irq4_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	m_sysbus->irq5_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ5);
	m_sysbus->irq6_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	m_sysbus->irq7_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ7);
	MC68000_SYSBUS_SLOT(config, "sysbus:0", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:1", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:2", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:3", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:4", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:5", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:6", mc68000_sysbus_cards, nullptr);
	MC68000_SYSBUS_SLOT(config, "sysbus:7", mc68000_sysbus_cards, nullptr);

	OUTPUT_LATCH(config, m_centronics_latch); // ic85

	INPUT_MERGER_ANY_LOW(config, m_centronics_error);
	m_centronics_error->output_handler().set(m_via[1], FUNC(via6522_device::write_pa7)).invert();

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(*m_centronics_latch);
	m_centronics->ack_handler().set(m_via[1], FUNC(via6522_device::write_ca2)); // alternatively connected to busy
	m_centronics->fault_handler().set(m_centronics_error, FUNC(input_merger_device::in_w<0>));
	m_centronics->perror_handler().set(m_centronics_error, FUNC(input_merger_device::in_w<1>)).invert();

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_via[0], FUNC(via6522_device::write_ca1));
	m_serial->rxd_handler().append(m_via[0], FUNC(via6522_device::write_pa2));
	m_serial->cts_handler().set(m_via[0], FUNC(via6522_device::write_pa7));

	pc_kbdc_device &ibmkbd(PC_KBDC(config, "ibmkbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
	ibmkbd.out_clock_cb().set(FUNC(mc68000_state::ibmkbd_clock_w));
	ibmkbd.out_data_cb().set(FUNC(mc68000_state::ibmkbd_data_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mc68000 )
	ROM_REGION16_BE(0x8000, "eprom0", 0)
	ROM_DEFAULT_BIOS("v143")
	ROM_SYSTEM_BIOS(0, "v141",  "V1.41")
	ROMX_LOAD("mc68000_system_1.41_upper.bin", 0x00000, 0x4000, CRC(e57f246f) SHA1(89e59e307ced22f243c4f6619dc07948e714fe71), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("mc68000_system_1.41_lower.bin", 0x00001, 0x4000, CRC(9183494d) SHA1(3be47d956d03e4f89c7ff17e027cd2fe8334d64a), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v143",  "V1.43")
	ROMX_LOAD("mc68000_system_1.43_upper.bin", 0x00000, 0x4000, CRC(5d73ae54) SHA1(38a594de605d63ba13ee963b004d3175a98e669a), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("mc68000_system_1.43_lower.bin", 0x00001, 0x4000, CRC(296434e5) SHA1(74416aebf3ccfa9b57d8d778b3689971f354d87c), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION16_BE(0x8000, "eprom1", 0)
	ROM_LOAD16_BYTE("mc68000_cpm_2.0_upper.bin", 0x00000, 0x4000, CRC(9dbe197a) SHA1(a26dcbbf567cdccf026ce5cebb248e52b5a8b24a))
	ROM_LOAD16_BYTE("mc68000_cpm_2.0_lower.bin", 0x00001, 0x4000, CRC(064dcc3b) SHA1(2fa3d1ddf1485bc46ddc90e2a68d9cc654157c27))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                FULLNAME             FLAGS
COMP( 1984, mc68000, 0,      0,      mc68000, mc68000, mc68000_state, empty_init, "mc / Franzis Verlag", "mc-68000-Computer", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
