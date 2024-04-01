// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM 5100.
 *
 * Sources:
 *  - IBM 5100 Maintenance Information Manual, SY31-0405-3, Fourth Edition (October 1979), International Business Machines Corporation
 *
 * TODO:
 *  - display registers
 *  - device address f
 *  - tape controller
 *  - printer
 *  - communications cards
 *  - expansion feature
 *  - later models (5110, 5120, 5130)
 */

#include "emu.h"

#include "ibm5100_kbd.h"

#include "cpu/palm/palm.h"

#include "screen.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class ibm5100_state : public driver_device
{
public:
	ibm5100_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_screen(*this, "screen")
		, m_kbd(*this, "kbd")
		, m_nxr(*this, { "apl", "basic" })
		, m_j2(*this, "j2")
		, m_conf(*this, "CONF")
		, m_disp(*this, "DISP")
		, m_lang(*this, "LANG")
		, m_ros(*this, "ros")
	{
	}

	void ibm5100(machine_config &config);

protected:
	// driver_device implementation
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cpu_ros_map(address_map &map);
	void cpu_rws_map(address_map &map);
	void cpu_ioc_map(address_map &map);
	void cpu_iod_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	// e2 - ros control card
	u8 e2_sts_r();
	void e2_ctl_w(u8 data);
	u8 e2_r();
	void e2_w(u8 data);

	// f2 - base i/o card
	u8 f2_kbd_sts_r();
	void f2_kbd_ctl_w(u8 data);
	void da0_ctl_w(u8 data);
	void daf_ctl_w(u8 data);

private:
	required_device<palm_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<ibm5100_keyboard_device> m_kbd;

	required_region_ptr_array<u16, 2> m_nxr;
	required_region_ptr<u8> m_j2;

	required_ioport m_conf;
	required_ioport m_disp;
	required_ioport m_lang;

	memory_view m_ros;

	u8 m_getb_bus;

	u8 m_e2_ff; // e2 card flip-flops
	u8 m_f2_ff; // f2 card flip-flops
	u16 m_e2_address;

	std::unique_ptr<u16[]> m_rws;
};

enum e2_ff_mask : u8
{
	E2_RS = 0x01, // ROS select (0=APL, 1=BASIC/common)
	E2_PS = 0x02, // put strobe
	E2_B0 = 0x04, // data address bit 0
};

enum f2_ff_mask : u8
{
	F2_KIE = 0x01, // keyboard interrupt enable
	F2_DO  = 0x10, // display off
};

void ibm5100_state::machine_start()
{
	// for simplicity allocate maximum 64KiB
	m_rws = std::make_unique<u16[]>(0x8000);

	save_item(NAME(m_getb_bus));

	save_item(NAME(m_e2_ff));
	save_item(NAME(m_f2_ff));
	save_item(NAME(m_e2_address));

	save_pointer(NAME(m_rws), 0x8000);
}

void ibm5100_state::machine_reset()
{
	m_e2_ff = 0;
	m_f2_ff = 0;

	m_e2_address = 0;

	// install configured rws
	unsigned const rws_cards = (m_conf->read() & 3) + 1;
	m_cpu->space(palm_device::AS_RWS).install_ram(0x80, 0x4000 * rws_cards - 1, &m_rws[0x40]);
	m_ros[1].install_ram(0x80, 0x4000 * rws_cards - 1, &m_rws[0x40]);
}

void ibm5100_state::cpu_ros_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_ros);
	m_ros[0](0x0000, 0xffff).rom().region("ros", 0);
}

void ibm5100_state::cpu_rws_map(address_map &map)
{
	map.unmap_value_high();
}

void ibm5100_state::cpu_ioc_map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(ibm5100_state::da0_ctl_w));
	map(0x1, 0x1).rw(FUNC(ibm5100_state::e2_sts_r), FUNC(ibm5100_state::e2_ctl_w));
	map(0x2, 0x3).noprw();
	map(0x4, 0x4).rw(FUNC(ibm5100_state::f2_kbd_sts_r), FUNC(ibm5100_state::f2_kbd_ctl_w));
	// 5 printer r:not used w:control
	map(0x6, 0x7).noprw();
	// 8 expansion r:status w:control
	map(0x9, 0xd).noprw();
	// e tape r:status w:control
	map(0xf, 0xf).nopr().w(FUNC(ibm5100_state::daf_ctl_w));
}

void ibm5100_state::cpu_iod_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0, 0x0).noprw();
	map(0x1, 0x1).rw(FUNC(ibm5100_state::e2_r), FUNC(ibm5100_state::e2_w));
	map(0x2, 0x3).noprw();
	map(0x4, 0x4).r(m_kbd, FUNC(ibm5100_keyboard_device::read)).nopw();
	// 5 r:printer w:print data
	map(0x6, 0x7).noprw();
	// 8 expansion r:not used w:data
	map(0x9, 0xd).noprw();
	// e r:tape data w:tape
	map(0xf, 0xf).noprw();
}

void ibm5100_state::ibm5100(machine_config &config)
{
	PALM(config, m_cpu, 15'091'200);
	m_cpu->set_addrmap(palm_device::AS_ROS, &ibm5100_state::cpu_ros_map);
	m_cpu->set_addrmap(palm_device::AS_RWS, &ibm5100_state::cpu_rws_map);
	m_cpu->set_addrmap(palm_device::AS_IOC, &ibm5100_state::cpu_ioc_map);
	m_cpu->set_addrmap(palm_device::AS_IOD, &ibm5100_state::cpu_iod_map);
	m_cpu->getb_bus().set([this](offs_t offset, u8 data) { m_getb_bus = data; });
	m_cpu->select_ros().set([this](int state) { m_ros.select(state); });

	/*
	 * Display output is 16 rows of 64 characters. Each character cell is 10x12
	 * pixels with 8x12 driven from character ROS data. The last two horizontal
	 * pixels of each character cell line and every other scan line are blank.
	 */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(15'091'200, 64*10, 0, 64*10, 16*12*2, 0, 16*12*2);
	m_screen->set_screen_update(FUNC(ibm5100_state::screen_update));

	IBM5100_KEYBOARD(config, m_kbd);
	m_kbd->strobe().set(
		[this](int state)
		{
			if ((m_f2_ff & F2_KIE) && !state)
				m_cpu->set_input_line(palm_device::IRPT_REQ3, 0);
		});
}

u32 ibm5100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	static rgb_t const c[] = { rgb_t::white(), rgb_t::black() };

	// read display switches
	u8 const disp = m_disp->read();

	bool const reverse = BIT(disp, 0);
	bool const n64 = !BIT(disp, 2);
	bool const r32 = BIT(disp, 3);

	// start with a blank screen
	bitmap.fill(c[reverse]);

	// then generate characters
	auto const rws = util::big_endian_cast<u8 const>(m_rws.get());
	for (unsigned char_y = 0; char_y < 16; char_y++)
	{
		// every other scan line is blank
		int const y = screen.visible_area().min_y + char_y * 12 * 2;

		// compute offset into rws for each row
		offs_t offset = 0x200 + char_y * 64 + (r32 ? 32 : 0);

		for (unsigned char_x = 0; char_x < 64; char_x++)
		{
			int const x = screen.visible_area().min_x + char_x * 10;

			// read next character if display is on and normal mode or even column
			u8 char_data = 0;
			if (!(m_f2_ff & F2_DO) && (n64 || !(char_x & 1)))
				char_data = rws[offset++];

			// draw 8x12 character cell
			for (unsigned cell_y = 0; cell_y < 12; cell_y++)
			{
				// index into character font data
				unsigned const underline = ((cell_y > 7) && BIT(char_data, 7)) ? 4 : 0;
				u8 const cell_data = m_j2[(char_data & 0x7f) * 16 + cell_y + underline];

				bitmap.pix(y + cell_y * 2, x + 0) = c[BIT(cell_data, 7) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 1) = c[BIT(cell_data, 6) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 2) = c[BIT(cell_data, 5) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 3) = c[BIT(cell_data, 4) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 4) = c[BIT(cell_data, 3) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 5) = c[BIT(cell_data, 2) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 6) = c[BIT(cell_data, 1) ^ reverse];
				bitmap.pix(y + cell_y * 2, x + 7) = c[BIT(cell_data, 0) ^ reverse];
			}
		}
	}

	return 0;
}

void ibm5100_state::da0_ctl_w(u8 data)
{
	LOG("da0_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit 4: 0=display off
	if (!BIT(data, 4))
		m_f2_ff |= F2_DO;
	else
		m_f2_ff &= ~F2_DO;
}

u8 ibm5100_state::e2_sts_r()
{
	u8 data = 0xff;

	if (!machine().side_effects_disabled())
	{
		data = (m_e2_ff & E2_PS) ? u8(m_e2_address) : (m_e2_address >> 8);

		m_e2_ff ^= E2_PS;
	}

	return data;
}

void ibm5100_state::e2_ctl_w(u8 data)
{
	LOG("e2_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!BIT(data, 3))
		m_e2_ff &= ~E2_RS;
	else if (!BIT(data, 2))
		m_e2_ff |= E2_RS;

	m_e2_ff &= ~(E2_B0 | E2_PS);
	m_e2_address = 0;
}

u8 ibm5100_state::e2_r()
{
	u8 data = 0xff;

	if (!machine().side_effects_disabled())
	{
		bool const basic = m_e2_ff & E2_RS;

		// check model has selected ROS (all models have common)
		if (BIT(m_conf->read(), 2 + basic) || (basic && m_e2_address >= 0x9000))
		{
			/*
			 * APL non-executable ROS is addressed with an unshifted 16-bit
			 * address, giving a 128KiB range; BASIC uses a shifted address,
			 * giving 64KiB. Even and odd bytes are selected by a flip-flop
			 * which is toggled with each read.
			 */
			data = BIT(m_nxr[basic][m_e2_address >> basic], (m_e2_ff & E2_B0) ? 0 : 8, 8);
		}

		// always increment address for BASIC, only on odd bytes for APL
		if (basic || (m_e2_ff & E2_B0))
			m_e2_address++;

		// toggle even/odd byte flip-flop
		m_e2_ff ^= E2_B0;
	}

	return data;
}

void ibm5100_state::e2_w(u8 data)
{
	m_e2_address = (m_e2_address << 8) | data;

	if (m_e2_ff & E2_PS)
	{
		LOG("e2_address 0x%04x (%s)\n", m_e2_address, machine().describe_context());

		// data byte even/odd flip-flop is cleared except when BASIC is
		// selected and the address is odd
		if ((m_e2_ff & E2_RS) && (m_e2_address & 1))
			m_e2_ff |= E2_B0;
		else
			m_e2_ff &= ~E2_B0;
	}

	// toggle put strobe flip-flop
	m_e2_ff ^= E2_PS;
}

u8 ibm5100_state::f2_kbd_sts_r()
{
	if (!machine().side_effects_disabled())
	{
		switch (m_getb_bus)
		{
		case 0x40:
			// keyboard data gate
			return m_kbd->read();
		case 0x80:
			// keyboard status gate
			return m_lang->read();
		default:
			LOG("f2_kbd_sts_r: unknown 0x%02x (%s)\n", m_getb_bus, machine().describe_context());
			break;
		}
	}

	return 0xff;
}

void ibm5100_state::f2_kbd_ctl_w(u8 data)
{
	LOG("f2_kbd_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit 6: 0=reset and disable keyboard interrupts
	if (!BIT(data, 6))
	{
		m_f2_ff &= ~F2_KIE;

		m_cpu->set_input_line(palm_device::IRPT_REQ3, 1);
	}

	// bit 4: 0=lock keyboard
	m_kbd->lock_w(BIT(data, 4));

	// bit 1: 0=enable typamatic
	// FIXME: inverted?
	m_kbd->typamatic_w(BIT(data, 1));

	// bit 0: 0=enable keyboard interrupt
	if (!BIT(data, 0))
		m_f2_ff |= F2_KIE;
	else
		m_f2_ff &= ~F2_KIE;
}

void ibm5100_state::daf_ctl_w(u8 data)
{
	LOG("daf_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit  function
	//  7   expansion da=8
	//  6   tape da=e
	//  5   keyboard da=4
	//  4   printer da=5
	//  3   enable cycle steal
	//  2   reset da=b
	//  1   reset da=c
	//  0   reset da=d
}

ROM_START(ibm5100)
	// Executable ROS
	ROM_REGION16_BE(0x10000, "ros", 0)
	ROM_LOAD("h2.ros", 0x0000, 0x8000, CRC(36d11e3d) SHA1(6be2c0728b88debcd557879c25781a9c7afc5224))
	ROM_LOAD("h4.ros", 0x8000, 0x8000, CRC(5d3eceb7) SHA1(e5412914d74e8149ea8a250a6560d1738555ec7e))

	// BASIC + Common and Language non-executable ROS
	/*
	 * The common ROS is physically located on the E2 card, and not on the C4
	 * BASIC card, however it is selected together with the BASIC ROS and
	 * logically appended to the address space.
	 */
	ROM_REGION16_BE(0x10000, "basic", 0)
	ROM_LOAD("c4.ros", 0x0000, 0x9000, CRC(b1abeb4a) SHA1(e0151fefe63c43c8912599615ddfb7c06f111c72))
	ROM_LOAD("e2.ros", 0x9000, 0x4800, CRC(be4289c3) SHA1(008ea7bb25fda94540bf5e02eff5a59bb1c86aac))
	ROM_FILL(0xd800, 0x2800, 0xff)

	// APL non-executable ROS
	ROM_REGION16_BE(0x20000, "apl", 0)
	ROM_LOAD("c2.ros", 0x00000, 0x8000, CRC(fba01c70) SHA1(a7dd9b60ba33021d830751df2de5513e0de452f2))
	ROM_LOAD("d2.ros", 0x08000, 0x8000, CRC(afb3ba33) SHA1(15292d1082a2d6211fbdbbb0781466506d310954))
	ROM_LOAD("d4.ros", 0x10000, 0x8000, CRC(a03570c9) SHA1(5a6e7a5b38b96c8ff47be2572272f8ed0cd31efd))
	ROM_FILL(0x18000, 0x8000, 0xff)

	// Display Adapter ROS
	/*
	 * This data was hand-made based on the character map in the documentation.
	 * It was assumed that the first 8 bytes of each character store the upper
	 * 8x8 cell, then 2x4 byte entries contain the normal and underlined lower
	 * 4 rows of the total 8x12 cell respectively.
	 */
	ROM_REGION(0x800, "j2", 0)
	ROM_LOAD("j2.ros", 0x000, 0x800, CRC(428e5b66) SHA1(9def68eed9dc2b8f08581387f8b74b49b3faf7e7) BAD_DUMP)
ROM_END

static INPUT_PORTS_START(ibm5100)
	PORT_START("CONF")
	PORT_CONFNAME(0x0f, 0x0f, "Model")
	PORT_CONFSETTING(0x04, "A1 - APL 16K")
	PORT_CONFSETTING(0x05, "A2 - APL 32K")
	PORT_CONFSETTING(0x06, "A3 - APL 48K")
	PORT_CONFSETTING(0x07, "A4 - APL 64K")

	PORT_CONFSETTING(0x08, "B1 - BASIC 16K")
	PORT_CONFSETTING(0x09, "B2 - BASIC 32K")
	PORT_CONFSETTING(0x0a, "B3 - BASIC 48K")
	PORT_CONFSETTING(0x0b, "B4 - BASIC 64K")

	PORT_CONFSETTING(0x0c, "C1 - APL/BASIC 16K")
	PORT_CONFSETTING(0x0d, "C2 - APL/BASIC 32K")
	PORT_CONFSETTING(0x0e, "C3 - APL/BASIC 48K")
	PORT_CONFSETTING(0x0f, "C4 - APL/BASIC 64K")

	PORT_START("DISP")
	PORT_CONFNAME(0x01, 0x00, "Reverse")
	PORT_CONFSETTING(0x00, "Black on White")
	PORT_CONFSETTING(0x01, "White on Black")
	PORT_CONFNAME(0x02, 0x00, "Display")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x02, "Registers")
	PORT_CONFNAME(0x0c, 0x00, "Columns")
	PORT_CONFSETTING(0x04, "L32")
	PORT_CONFSETTING(0x00, "64")
	PORT_CONFSETTING(0x0c, "R32")

	PORT_START("LANG")
	PORT_CONFNAME(0x40, 0x40, "Language")
	PORT_CONFSETTING(0x00, "APL")   PORT_CONDITION("CONF", 0x04, EQUALS, 0x04)
	PORT_CONFSETTING(0x40, "BASIC") PORT_CONDITION("CONF", 0x08, EQUALS, 0x08)
INPUT_PORTS_END

} // anonymous namespace

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                            FULLNAME    FLAGS */
COMP(1975, ibm5100, 0,      0,      ibm5100, ibm5100, ibm5100_state, empty_init, "International Business Machines", "IBM 5100", MACHINE_NO_SOUND_HW)
