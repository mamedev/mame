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
 *  - disk controller
 *  - printer
 *  - communications cards
 *  - expansion feature
 *  - feature ROS (K4)
 */

#include "emu.h"

#include "ibm5100_kbd.h"

#include "cpu/palm/palm.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

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
		, m_nxr(*this, { "common", "basic", "apl" })
		, m_cgr(*this, "cgr")
		, m_pgm(*this, "pgm")
		, m_ros(*this, "ros")
		, m_conf(*this, "CONF")
		, m_disp(*this, "DISP")
		, m_lang(*this, "LANG")
	{
	}

	void common(machine_config &config);
	void ibm5100(machine_config &config);

protected:
	// driver_device implementation
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void cpu_pgm_map(address_map &map) ATTR_COLD;
	virtual void cpu_rws_map(address_map &map) ATTR_COLD;
	virtual void cpu_ioc_map(address_map &map) ATTR_COLD;
	virtual void cpu_iod_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	virtual void da0_ctl_w(u8 data);

	// non-executable ROS control
	u8 nxr_sts_r();
	void nxr_ctl_w(u8 data);
	virtual u8 nxr_r();
	virtual void nxr_w(u8 data);

	// keyboard
	u8 kbd_sts_r();
	void kbd_ctl_w(u8 data);

	void daf_ctl_w(u8 data);

	required_device<palm_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<ibm5100_keyboard_device> m_kbd;

	required_region_ptr_array<u16, 3> m_nxr;
	required_region_ptr<u8> m_cgr; // character generator ROS

	memory_view m_pgm;
	memory_view m_ros;

	required_ioport m_conf;
	required_ioport m_disp;
	required_ioport m_lang;

	u8 m_getb_bus;

	u8 m_nxr_ff; // non-executable ROS control flip-flops
	u8 m_bio_ff; // base I/O card flip-flops
	u16 m_nxr_address;

	std::unique_ptr<u16[]> m_rws;
};

class ibm5110_state : public ibm5100_state
{
public:
	ibm5110_state(machine_config const &mconfig, device_type type, char const *tag)
		: ibm5100_state(mconfig, type, tag)
		, m_alarm(*this, "alarm")
		, m_jmp(*this, { "L2_1", "L2_2", "K4" })
	{
	}

	void ibm5110(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void cpu_pgm_map(address_map &map) override ATTR_COLD;
	virtual void cpu_ioc_map(address_map &map) override ATTR_COLD;

	virtual void da0_ctl_w(u8 data) override;

	// executable ROS control
	u8 exr_sts_r();
	void exr_ctl_w(u8 data);

	// non-executable ROS control
	virtual u8 nxr_r() override;
	virtual void nxr_w(u8 data) override;

private:
	required_device<beep_device> m_alarm;

	required_ioport_array<3> m_jmp;

	u8 m_exr_ff; // executable ROS flip-flops
};

enum nxr_ff_mask : u8
{
	NXR_RS     = 0x03, // ROS select
	NXR_PS     = 0x04, // put strobe
	NXR_B0     = 0x08, // data address bit 0

	NXR_COMMON = 0x00,
	NXR_BASIC  = 0x01,
	NXR_APL    = 0x02,
};

enum bio_ff_mask : u8
{
	BIO_KIE = 0x01, // keyboard interrupt enable
	BIO_KI  = 0x08, // keyboard interrupt active
	BIO_DO  = 0x10, // display off
};

enum exr_ff_mask : u8
{
	EXR_ROS2 = 0x01, // ROS2 enable
};

void ibm5100_state::machine_start()
{
	// for simplicity allocate maximum 64KiB
	m_rws = std::make_unique<u16[]>(0x8000);

	save_item(NAME(m_getb_bus));

	save_item(NAME(m_nxr_ff));
	save_item(NAME(m_bio_ff));
	save_item(NAME(m_nxr_address));

	save_pointer(NAME(m_rws), 0x8000);
}

void ibm5110_state::machine_start()
{
	ibm5100_state::machine_start();

	save_item(NAME(m_exr_ff));
}

void ibm5100_state::machine_reset()
{
	m_nxr_ff = 0;
	m_bio_ff = 0;

	m_nxr_address = 0;

	// install configured rws
	unsigned const rws_cards = (m_conf->read() & 3) + 1;
	m_cpu->space(palm_device::AS_RWS).install_ram(0x80, 0x4000 * rws_cards - 1, &m_rws[0x40]);
	m_ros[1].install_ram(0x80, 0x4000 * rws_cards - 1, &m_rws[0x40]);
}

void ibm5110_state::machine_reset()
{
	ibm5100_state::machine_reset();

	m_exr_ff = 0;
}

void ibm5100_state::cpu_pgm_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_pgm);

	// normal mode program memory
	m_pgm[0](0x0000, 0xffff).view(m_ros);
	m_ros[0](0x0000, 0xffff).rom().region("ros", 0);
	// m_ros[1] RWS (configurable size)

	// interrupt mode program memory
	m_pgm[1](0x0000, 0xffff).rom().region("ros", 0);
}

void ibm5110_state::cpu_pgm_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_pgm);

	// normal mode program memory
	m_pgm[0](0x0000, 0xffff).view(m_ros);
	m_ros[0](0x0000, 0x7fff).rom().region("ros1", 0);
	// m_ros[1] RWS (configurable size)
	m_ros[2](0x0000, 0x7fff).rom().region("ros2a", 0);
	m_ros[3](0x0000, 0x7fff).rom().region("ros2b", 0);

	// interrupt mode program memory
	m_pgm[1](0x0000, 0x7fff).rom().region("ros1", 0);
}

void ibm5100_state::cpu_rws_map(address_map &map)
{
	map.unmap_value_high();
}

void ibm5100_state::cpu_ioc_map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(ibm5100_state::da0_ctl_w));
	map(0x1, 0x1).rw(FUNC(ibm5100_state::nxr_sts_r), FUNC(ibm5100_state::nxr_ctl_w));
	map(0x2, 0x3).noprw();
	map(0x4, 0x4).rw(FUNC(ibm5100_state::kbd_sts_r), FUNC(ibm5100_state::kbd_ctl_w));
	// 5 printer r:not used w:control
	map(0x6, 0x7).noprw();
	// 8 expansion r:status w:control
	map(0x9, 0xd).noprw();
	// e tape r:status w:control
	map(0xf, 0xf).nopr().w(FUNC(ibm5100_state::daf_ctl_w));
}

void ibm5110_state::cpu_ioc_map(address_map &map)
{
	ibm5100_state::cpu_ioc_map(map);

	map(0x2, 0x2).rw(FUNC(ibm5110_state::exr_sts_r), FUNC(ibm5110_state::exr_ctl_w));
}

void ibm5100_state::cpu_iod_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0, 0x0).noprw();
	map(0x1, 0x1).rw(FUNC(ibm5100_state::nxr_r), FUNC(ibm5100_state::nxr_w));
	map(0x2, 0x3).noprw();
	map(0x4, 0x4).r(m_kbd, FUNC(ibm5100_keyboard_device::read));
	// 5 r:printer w:print data
	map(0x6, 0x7).noprw();
	// 8 expansion r:not used w:data
	map(0x9, 0xd).noprw();
	// e r:tape data w:tape
	map(0xf, 0xf).noprw();
}

void ibm5100_state::common(machine_config &config)
{
	PALM(config, m_cpu, 15'091'200);
	m_cpu->set_addrmap(palm_device::AS_PGM, &ibm5100_state::cpu_pgm_map);
	m_cpu->set_addrmap(palm_device::AS_RWS, &ibm5100_state::cpu_rws_map);
	m_cpu->set_addrmap(palm_device::AS_IOC, &ibm5100_state::cpu_ioc_map);
	m_cpu->set_addrmap(palm_device::AS_IOD, &ibm5100_state::cpu_iod_map);
	m_cpu->getb_bus().set([this](offs_t offset, u8 data) { m_getb_bus = data; });
	m_cpu->program_level().set([this](int state) { m_pgm.select(state); });

	/*
	 * Display output is 16 rows of 64 characters. Each character cell is 10x12
	 * pixels with 8x12 driven from character ROS data. The last two horizontal
	 * pixels of each character cell line and every other scan line are blank.
	 */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(15'091'200, 64*10+15, 0, 64*10, 16*12*2, 0, 16*12*2);
	m_screen->set_screen_update(FUNC(ibm5100_state::screen_update));
}

void ibm5100_state::ibm5100(machine_config &config)
{
	ibm5100_state::common(config);

	m_cpu->select_ros().set([this](int state) { m_ros.select(state); });

	IBM5100_KEYBOARD(config, m_kbd);
	m_kbd->strobe().set(
		[this](int state)
		{
			if ((m_bio_ff & BIO_KIE) && !state)
				m_cpu->set_input_line(palm_device::IRPT_REQ3, 0);
		});
}

void ibm5110_state::ibm5110(machine_config &config)
{
	ibm5100_state::common(config);

	m_cpu->select_ros().set(
		[this](int state)
		{
			if (!state && (m_exr_ff & EXR_ROS2))
				m_ros.select(BIT(m_lang->read(), 6) + 2);
			else
				m_ros.select(state);
		});

	IBM5110_KEYBOARD(config, m_kbd);
	m_kbd->strobe().set(
		[this](int state)
		{
			if (!state)
			{
				m_bio_ff |= BIO_KI;

				if (m_bio_ff & BIO_KIE)
					m_cpu->set_input_line(palm_device::IRPT_REQ3, 0);
			}
		});

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_alarm, 3'885); // FIXME: frequency
	m_alarm->add_route(ALL_OUTPUTS, "mono", 0.25);

	// gfxdecode is only used to show the font data in the tile viewer
	static const gfx_layout g2_layout =
	{
		8, 12, 256, 1,
		{ 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8 },
		8 * 16
	};

	static GFXDECODE_START(g2)
		GFXDECODE_ENTRY("cgr", 0, g2_layout, 0, 1)
	GFXDECODE_END

	PALETTE(config, "palette", palette_device::MONOCHROME);
	GFXDECODE(config, "gfx", "palette", g2);
}

u32 ibm5100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	static rgb_t const c[] = { rgb_t::white(), rgb_t::black() };

	// read display switches
	u8 const disp = m_disp->read();

	bool const reverse = BIT(disp, 0);
	bool const n64 = !BIT(disp, 2);
	unsigned const r32 = BIT(disp, 3) ? 32 : 0;

	// start with a blank screen
	bitmap.fill(c[reverse]);

	if (m_bio_ff & BIO_DO)
		return 0;

	// generate characters
	auto const rws = util::big_endian_cast<u8 const>(m_rws.get());
	for (unsigned char_y = 0; char_y < 16; char_y++)
	{
		// every alternate scan line is blank
		int const y = screen.visible_area().min_y + char_y * 12 * 2;

		// compute offset into rws for each row
		offs_t offset = 0x200 + char_y * 64 + r32;

		for (unsigned char_x = 0; char_x < 64; char_x++)
		{
			// skip alternate columns in l32/r32 mode
			if (!n64 && (char_x & 1))
				continue;

			int const x = screen.visible_area().min_x + char_x * 10;

			// read next character
			u8 const char_data = rws[offset++];

			// draw 8x12 character cell
			for (unsigned cell_y = 0; cell_y < 12; cell_y++)
			{
				u8 cell_data = 0;

				/*
				 * The J2 (5100) display adapter has 2KiB of character ROS used
				 * to produce 128 unique characters with normal and underlined
				 * variants. The G2 (5110) has 4KiB with 256 unique character
				 * patterns, with only selected characters having underlined
				 * variations.
				 */
				if (m_cgr.bytes() < 4096)
				{
					unsigned const underline = ((cell_y > 7) && BIT(char_data, 7)) ? 4 : 0;

					cell_data = m_cgr[(char_data & 0x7f) * 16 + cell_y + underline];
				}
				else
					cell_data = m_cgr[char_data * 16 + cell_y];

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
	// bit 3: 0=display on
	if (!BIT(data, 4))
		m_bio_ff |= BIO_DO;
	else if (!BIT(data, 3))
		m_bio_ff &= ~BIO_DO;
}

void ibm5110_state::da0_ctl_w(u8 data)
{
	ibm5100_state::da0_ctl_w(data);

	// bit 0: 0=alarm on
	// bit 1: 0=alarm off
	if (!BIT(data, 0))
		m_alarm->set_state(1);
	else if (!BIT(data, 1))
		m_alarm->set_state(0);
}

u8 ibm5110_state::exr_sts_r()
{
	LOG("exr_sts_r 0x%02x (%s)\n", m_getb_bus, machine().describe_context());

	switch (m_getb_bus)
	{
	case 0x80: return m_jmp[0]->read();
	case 0x40: return m_jmp[1]->read();
	case 0x20: return m_jmp[2]->read();
	default:
		return 0;
	}
}

void ibm5110_state::exr_ctl_w(u8 data)
{
	LOG("exr_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	if (BIT(data, 7))
	{
		m_exr_ff |= EXR_ROS2;

		if (m_ros.entry() != 1)
			m_ros.select(BIT(m_lang->read(), 6) + 2);
	}
	else if (BIT(data, 6))
	{
		m_exr_ff &= ~EXR_ROS2;

		if (m_ros.entry() != 1)
			m_ros.select(0);
	}
}

u8 ibm5100_state::nxr_sts_r()
{
	u8 data = 0xff;

	if (!machine().side_effects_disabled())
	{
		data = (m_nxr_ff & NXR_PS) ? u8(m_nxr_address) : (m_nxr_address >> 8);

		m_nxr_ff ^= NXR_PS;
	}

	return data;
}

void ibm5100_state::nxr_ctl_w(u8 data)
{
	LOG("nxr_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	m_nxr_ff &= ~(NXR_B0 | NXR_PS | NXR_RS);

	if (BIT(data, 3))
		m_nxr_ff |= NXR_BASIC;
	else if (BIT(data, 2))
		m_nxr_ff |= NXR_APL;

	m_nxr_address = 0;
}

u8 ibm5100_state::nxr_r()
{
	u8 data = 0xff;

	if (!machine().side_effects_disabled())
	{
		unsigned const rs = m_nxr_ff & NXR_RS;
		bool const basic = !(rs & NXR_APL);

		// check model has selected ROS (all models have common)
		if (BIT(m_conf->read(), 2 + basic) || (basic && m_nxr_address >= 0x9000))
		{
			/*
			 * APL non-executable ROS is addressed with an unshifted 16-bit
			 * address, giving a 128KiB range; BASIC uses a shifted address,
			 * giving 64KiB. Even and odd bytes are selected by a flip-flop
			 * which is toggled with each read.
			 */
			data = BIT(m_nxr[rs][m_nxr_address >> basic], (m_nxr_ff & NXR_B0) ? 0 : 8, 8);
		}

		// always increment address for BASIC, only on odd bytes for APL
		if (basic || (m_nxr_ff & NXR_B0))
			m_nxr_address++;

		// toggle even/odd byte flip-flop
		m_nxr_ff ^= NXR_B0;
	}

	return data;
}

u8 ibm5110_state::nxr_r()
{
	u8 data = 0xff;

	if (!machine().side_effects_disabled())
	{
		unsigned const rs = m_nxr_ff & NXR_RS;

		// check model has selected ROS (all models have common)
		if (!(rs & NXR_APL) || BIT(m_conf->read(), 2))
			data = BIT(m_nxr[rs][m_nxr_address], (m_nxr_ff & NXR_B0) ? 0 : 8, 8);

		// increment after odd bytes
		if (m_nxr_ff & NXR_B0)
			m_nxr_address++;

		// toggle even/odd byte flip-flop
		m_nxr_ff ^= NXR_B0;
	}

	return data;
}

void ibm5100_state::nxr_w(u8 data)
{
	m_nxr_address = (m_nxr_address << 8) | data;

	if (m_nxr_ff & NXR_PS)
	{
		LOG("nxr_address 0x%04x (%s)\n", m_nxr_address, machine().describe_context());

		// data byte even/odd flip-flop is cleared except when BASIC is
		// selected and the address is odd
		if (!(m_nxr_ff & NXR_APL) && (m_nxr_address & 1))
			m_nxr_ff |= NXR_B0;
		else
			m_nxr_ff &= ~NXR_B0;
	}

	// toggle put strobe flip-flop
	m_nxr_ff ^= NXR_PS;
}

void ibm5110_state::nxr_w(u8 data)
{
	m_nxr_address = (m_nxr_address << 8) | data;

	if (m_nxr_ff & NXR_PS)
	{
		LOG("nxr_address 0x%04x (%s)\n", m_nxr_address, machine().describe_context());

		m_nxr_ff &= ~NXR_B0;
	}

	// toggle put strobe flip-flop
	m_nxr_ff ^= NXR_PS;
}

u8 ibm5100_state::kbd_sts_r()
{
	if (!machine().side_effects_disabled())
	{
		switch (m_getb_bus)
		{
		case 0x40:
			// keyboard data gate
			m_bio_ff &= ~BIO_KI;
			return m_kbd->read();
		case 0x80:
			// keyboard status gate
			return m_lang->read() | (m_bio_ff & BIO_KI);
		default:
			LOG("kbd_sts_r: unknown 0x%02x (%s)\n", m_getb_bus, machine().describe_context());
			break;
		}
	}

	return 0xff;
}

void ibm5100_state::kbd_ctl_w(u8 data)
{
	LOG("kbd_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit 6: 0=reset and disable keyboard interrupts
	if (!BIT(data, 6))
	{
		m_bio_ff &= ~BIO_KIE;

		m_cpu->set_input_line(palm_device::IRPT_REQ3, 1);
	}

	// bit 4: 0=lock keyboard
	m_kbd->lock_w(BIT(data, 4));

	// bit 1: 0=enable typamatic
	// FIXME: inverted?
	m_kbd->typamatic_w(BIT(data, 1));

	// bit 0: 0=enable keyboard interrupt
	if (!BIT(data, 0))
		m_bio_ff |= BIO_KIE;
	else
		m_bio_ff &= ~BIO_KIE;
}

void ibm5100_state::daf_ctl_w(u8 data)
{
	LOG("daf_ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit  function
	//  7   expansion da=8
	//  6   tape da=e
	//  5   keyboard da=4
	if (BIT(data, 5))
	{
		m_bio_ff &= ~(BIO_KI | BIO_KIE);
		m_cpu->set_input_line(palm_device::IRPT_REQ3, 1);
	}
	//  4   printer da=5
	//  3   enable cycle steal
	if (BIT(data, 3))
		m_bio_ff &= ~BIO_DO;
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
	ROM_REGION16_BE(0x10000, "common", 0)
	ROM_LOAD("c4.ros", 0x0000, 0x9000, CRC(b1abeb4a) SHA1(e0151fefe63c43c8912599615ddfb7c06f111c72))
	ROM_LOAD("e2.ros", 0x9000, 0x4800, CRC(be4289c3) SHA1(008ea7bb25fda94540bf5e02eff5a59bb1c86aac))
	ROM_FILL(0xd800, 0x2800, 0xff)

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
	ROM_REGION(0x800, "cgr", 0)
	ROM_LOAD("j2.ros", 0x000, 0x800, CRC(428e5b66) SHA1(9def68eed9dc2b8f08581387f8b74b49b3faf7e7) BAD_DUMP)
ROM_END

ROM_START(ibm5110)
	// Executable ROS
	ROM_REGION16_BE(0x8000, "ros1", 0)
	ROM_LOAD("l2_1.ros", 0x0000, 0x8000, CRC(0355894f) SHA1(c76a91cbbec226feb942ccde93ecb1637c88a01b))

	// APL Executable ROS
	ROM_REGION16_BE(0x8000, "ros2a", 0)
	ROM_LOAD("l2_2a.ros", 0x0000, 0x5000, CRC(46918be9) SHA1(bf45a44f77104c55f2ccfa462af06944e6bffe1a))
	ROM_FILL(0x5000, 0x3000, 0xff)

	// BASIC Executable ROS
	ROM_REGION16_BE(0x8000, "ros2b", 0)
	ROM_LOAD("l2_2b.ros", 0x0000, 0x4000, CRC(a69dd0c1) SHA1(ecdc1363e25b72b695c517af145c50a069b6e8dc))
	ROM_FILL(0x4000, 0x3000, 0xff)

	// Common and Language non-executable ROS
	ROM_REGION16_BE(0x20000, "common", 0)
	ROM_LOAD("f2_c.ros", 0x0000, 0x4000, CRC(83beafb2) SHA1(80ad07a2a83ff395d918b69d5c81a6e94ac7af37))
	ROM_FILL(0x4000, 0x1c000, 0xff)

	// BASIC non-executable ROS
	ROM_REGION16_BE(0x20000, "basic", 0)
	ROM_LOAD("f2_b.ros", 0x0000, 0x12000, CRC(3ffb79b6) SHA1(254f5a7ae739b7bf6a800e8380cf3b6686ee9768))
	ROM_FILL(0x12000, 0x8000, 0xff)

	// APL non-executable ROS
	ROM_REGION16_BE(0x20000, "apl", 0)
	ROM_LOAD("e4.ros", 0x00000, 0x20000, CRC(1e28db20) SHA1(cf12893bb76c44756a9554828d42299b169e560a))

	// Display Adapter ROS
	/*
	 * This data was hand-made based on the character map in the documentation.
	 * It was assumed that the first 12 bytes of each character store the 8x12
	 * cell, followed by 4 empty bytes.
	 */
	ROM_REGION(0x1000, "cgr", 0)
	ROM_LOAD("g2.ros", 0x000, 0x1000, CRC(86e6a99c) SHA1(7168ba05fbac3f66bd98b8ef9fc135d0d08eb44b) BAD_DUMP)
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

static INPUT_PORTS_START(ibm5110)
	PORT_INCLUDE(ibm5100)

	PORT_START("L2_1")
	PORT_DIPNAME(0xf0, 0xf0, "Model") PORT_DIPLOCATION("L2_1:4,3,2,1")
	PORT_DIPSETTING(0xf0, "5110-X1X")
	PORT_DIPSETTING(0xe0, "5110-X2X")

	PORT_START("L2_2")
	PORT_DIPNAME(0xf0, 0xc0, "L2_2") PORT_DIPLOCATION("L2_2:4,3,2,1")
	PORT_DIPSETTING(0xc0, DEF_STR(Unknown))

	PORT_START("K4")
	PORT_DIPNAME(0x80, 0x80, "Feature ROS") PORT_DIPLOCATION("K4:1")
	PORT_DIPSETTING(0x80, "Absent")
	PORT_DIPSETTING(0x00, "Present")
INPUT_PORTS_END

} // anonymous namespace

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                            FULLNAME    FLAGS */
COMP(1975, ibm5100, 0,      0,      ibm5100, ibm5100, ibm5100_state, empty_init, "International Business Machines", "IBM 5100", MACHINE_NO_SOUND_HW)
COMP(1978, ibm5110, 0,      0,      ibm5110, ibm5110, ibm5110_state, empty_init, "International Business Machines", "IBM 5110", 0)
