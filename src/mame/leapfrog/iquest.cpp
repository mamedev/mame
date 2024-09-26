// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

   Leapfrog IQuest

   has LCD display

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/bankdev.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "screen.h"


namespace {

class leapfrog_iquest_state : public driver_device
{
public:
	leapfrog_iquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_cart_region(nullptr)
	{ }

	void leapfrog_iquest(machine_config &config);

protected:
	void leapfrog_base(machine_config &config);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;

	void rx_line_hack(int state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;

	void rom_map(address_map &map) ATTR_COLD;

	required_device<address_map_bank_device> m_rombank;
	memory_region *m_cart_region;

	uint8_t m_lowerbank[2];
	uint8_t m_upperbank[2];

	uint8_t lowerbank_r(offs_t offset);
	uint8_t upperbank_r(offs_t offset);
	void lowerbank_w(offs_t offset, uint8_t data);
	void upperbank_w(offs_t offset, uint8_t data);

	uint8_t m_iobank[2];

	uint8_t iobank_r(offs_t offset);
	void iobank_w(offs_t offset, uint8_t data);


	uint8_t lowerwindow_r(offs_t offset);
	uint8_t upperwindow_r(offs_t offset);

	uint8_t iowindow_r(offs_t offset);
	void iowindow_w(offs_t offset, uint8_t data);

	uint8_t unk_ff80_r();
	void unk_ff80_w(uint8_t data);
	uint8_t m_ff80 = 0;

	uint8_t unk_fc00_r();
	uint8_t unk_fc01_r(offs_t offset);

	uint8_t unk_fc2f_r();
	uint8_t unk_fc3f_r();
	void unk_fc3f_w(uint8_t data);

	void unk_fc22_w(uint8_t data);

	uint8_t unk_fce5_r();
	void unk_fce5_w(uint8_t data);

	uint8_t m_fce5 = 0;

	uint8_t unk_ff00_01_r(offs_t offset);

	void unk_ff81_84_w(offs_t offset, uint8_t data);
	uint8_t m_ff81_84[4]{};

	uint8_t unk_ff91_93_r(offs_t offset);
	void unk_ff91_93_w(offs_t offset, uint8_t data);
	uint8_t m_ff91_93[3]{};

	uint8_t unk_ffa8_r();
	void unk_ffa8_w(uint8_t data);
	uint8_t m_ffa8 = 0;

	void unk_ffa9_w(uint8_t data);

	uint8_t port0_r();
	void port0_w(u8 data);
	uint8_t port1_r();
	void port1_w(u8 data);
	uint8_t port2_r();
	void port2_w(u8 data);
	uint8_t port3_r();
	void port3_w(u8 data);
};


class leapfrog_turboextreme_state : public leapfrog_iquest_state
{
public:
	leapfrog_turboextreme_state(const machine_config &mconfig, device_type type, const char *tag)
		: leapfrog_iquest_state(mconfig, type, tag)
	{ }

	void leapfrog_turboex(machine_config &config);
};

class leapfrog_turbotwistmath_state : public leapfrog_iquest_state
{
public:
	leapfrog_turbotwistmath_state(const machine_config &mconfig, device_type type, const char *tag)
		: leapfrog_iquest_state(mconfig, type, tag)
	{ }

	void leapfrog_turbotwistmath(machine_config &config);
};

class leapfrog_turbotwistvocabulator_state : public leapfrog_iquest_state
{
public:
	leapfrog_turbotwistvocabulator_state(const machine_config &mconfig, device_type type, const char *tag)
		: leapfrog_iquest_state(mconfig, type, tag)
	{ }

	void leapfrog_turbotwistvocabulator(machine_config &config);
};

class leapfrog_turbotwistspelling_state : public leapfrog_iquest_state
{
public:
	leapfrog_turbotwistspelling_state(const machine_config &mconfig, device_type type, const char *tag)
		: leapfrog_iquest_state(mconfig, type, tag)
	{ }

	void leapfrog_turbotwistspelling(machine_config &config);
};


class leapfrog_turbotwistbrainquest_state : public leapfrog_iquest_state
{
public:
	leapfrog_turbotwistbrainquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: leapfrog_iquest_state(mconfig, type, tag)
	{ }

	void leapfrog_turbotwistbrainquest(machine_config &config);
};


void leapfrog_iquest_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void leapfrog_iquest_state::machine_reset()
{
	m_lowerbank[0] = m_lowerbank[1] = 0x00;
	m_upperbank[0] = m_upperbank[1] = 0x00;
	m_iobank[0] = m_iobank[1] = 0x00;

	m_rombank->set_bank(0);
	m_ff80 = 0x00;

	m_fce5 = 0x00;

	m_ff91_93[0] = m_ff91_93[1] = m_ff91_93[2] = 0x00;
}


uint8_t leapfrog_iquest_state::lowerbank_r(offs_t offset)
{
	return m_lowerbank[offset];
}

uint8_t leapfrog_iquest_state::upperbank_r(offs_t offset)
{
	return m_upperbank[offset];
}


void leapfrog_iquest_state::lowerbank_w(offs_t offset, uint8_t data)
{
	m_lowerbank[offset] = data;
}

void leapfrog_iquest_state::upperbank_w(offs_t offset, uint8_t data)
{
	m_upperbank[offset] = data;
}

uint8_t leapfrog_iquest_state::iobank_r(offs_t offset)
{
	return m_iobank[offset];
}


void leapfrog_iquest_state::iobank_w(offs_t offset, uint8_t data)
{
	//printf("iobank_w %d, %02x\n", offset, data);
	m_iobank[offset] = data;
}


void leapfrog_iquest_state::rom_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0);
	map(0x00400000, 0x007fffff).ram();
}

uint8_t leapfrog_iquest_state::lowerwindow_r(offs_t offset)
{
	uint32_t bank = ((m_lowerbank[0] << 8) | (m_lowerbank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

uint8_t leapfrog_iquest_state::upperwindow_r(offs_t offset)
{
	uint32_t bank = ((m_upperbank[0] << 8) | (m_upperbank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

uint8_t leapfrog_iquest_state::iowindow_r(offs_t offset)
{
	uint32_t bank = ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

void leapfrog_iquest_state::iowindow_w(offs_t offset, uint8_t data)
{
	uint32_t bank = ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000;
	m_rombank->write8(bank + offset, data);
}


void leapfrog_iquest_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).r(FUNC(leapfrog_iquest_state::lowerwindow_r));
	map(0x8000, 0xffff).r(FUNC(leapfrog_iquest_state::upperwindow_r));
}

uint8_t leapfrog_iquest_state::unk_ff80_r()
{
	logerror("%s: unk_ff80_r\n", machine().describe_context());
	return m_ff80;
}

void leapfrog_iquest_state::unk_ff80_w(uint8_t data)
{
	// must return what is written for some startup tests
	logerror("%s: m_ff80 %02x\n", machine().describe_context(), data);
	m_ff80 = data;
}

uint8_t leapfrog_iquest_state::unk_fc00_r()
{
	logerror("%s: unk_fc00_r\n", machine().describe_context());
	return 0x00;// machine().rand();
}

uint8_t leapfrog_iquest_state::unk_fc01_r(offs_t offset)
{
	logerror("%s: unk_fc01_r %d\n", machine().describe_context(), offset);
	return 0xff;// machine().rand();
}

uint8_t leapfrog_iquest_state::unk_fc2f_r()
{
	logerror("%s: unk_fc2f_r\n", machine().describe_context());
	return 0x00;// machine().rand();
}

uint8_t leapfrog_iquest_state::unk_fc3f_r()
{
	logerror("%s: unk_fc3f_r\n", machine().describe_context());
	return 0x00;// machine().rand();
}

void leapfrog_iquest_state::unk_fc3f_w(uint8_t data)
{
	logerror("%s: unk_fc3f_w %02x\n", machine().describe_context(), data);
}

void leapfrog_iquest_state::unk_fc22_w(uint8_t data)
{
	logerror("%s: unk_fc22_w %02x\n", machine().describe_context(), data);
}

uint8_t leapfrog_iquest_state::unk_ff00_01_r(offs_t offset)
{
	// read around the time of fc22 writes
	logerror("%s: unk_ff00_01_r %d\n", machine().describe_context(), offset);
	return 0x00;
}

uint8_t leapfrog_iquest_state::unk_fce5_r()
{
	logerror("%s: unk_fce5_r\n", machine().describe_context());
	return 0x00;// m_fce5;// machine().rand();
}

void leapfrog_iquest_state::unk_fce5_w(uint8_t data)
{
	// repeated read/write pattern on this address
	logerror("%s: unk_fce5_w %02x\n", machine().describe_context(), data);
	m_fce5 = data;
}

/*

in all cases ff91-ff93 writes appear to be an address in the current main space
which is 0x10000-0x17fff, 0x10000-0x17fff in ROM at the time of writing
or for the later writes  0x10000-0x17fff, 0x20000-0x27fff  (the b448 is from 0x23448 for example)

each of the blocks pointed to is preceded by a 0x00 byte? (maybe 0x00 is a terminator for previous block?)

the blocks being pointed at during startup are debug text, c style strings for 'printf' functions, complete with
formatting characters while the ff81 to ff84 area contains the parameters.

it seems likely this is just a RAM area and these are temporary storage for the debug print function,
either for futher processing and output to an actual debug console, or shared with another device (ff80, which is
possibly the start of this RAM area, is written after putting these pointers in RAM)

*/

uint8_t leapfrog_iquest_state::unk_ff91_93_r(offs_t offset)
{
	logerror("%s: unk_ff91_93_r %d\n", machine().describe_context(), offset);
	return 0x00;// m_ff91_93[offset];
}

void leapfrog_iquest_state::unk_ff91_93_w(offs_t offset, uint8_t data)
{
	// these 3 values are written together
	m_ff91_93[offset] = data;

	// form is ff then 2 other values, these are pointers into the main space it seems

	if (offset == 2)
	{
		logerror("%s: write to ff91 to ff93 region %02x %02x %02x (current banks are %08x %08x %08x)\n", machine().describe_context(), m_ff91_93[0], m_ff91_93[1], m_ff91_93[2], ((m_lowerbank[0] << 8) | (m_lowerbank[1])) * 0x8000, ((m_upperbank[0] << 8) | (m_upperbank[1])) * 0x8000, ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000);

		uint16_t pointer = (m_ff91_93[1] << 8) | (m_ff91_93[2]);

		address_space& spc = m_maincpu->space(AS_PROGRAM);
		char readdat = 0x00;

		std::string textout;
		do
		{
			readdat = spc.read_byte(pointer++);
			textout.append(1, readdat);
		} while (readdat != 0x00);
		logerror("%s: DEBUG MESSAGE: %s", machine().describe_context(), textout);
		logerror("\n");
	}
}

void leapfrog_iquest_state::unk_ff81_84_w(offs_t offset, uint8_t data)
{
	// these 4 values are written together, with FFA8 before and FFA9 after
	// form is usually 00 00 then 2 used values, maybe coordinates?
	// used in conjunction with unk_ff91_93_w writes above
	m_ff81_84[offset] = data;

	if (offset == 3)
	{
		logerror("%s: write to ff81 to ff84 region %02x %02x %02x %02x\n", machine().describe_context(), m_ff81_84[0], m_ff81_84[1], m_ff81_84[2], m_ff81_84[3]);

		uint16_t pointer = (m_ff81_84[2] << 8) | (m_ff81_84[3]);

		if (pointer != 0x00)
		{
			address_space& spc = m_maincpu->space(AS_PROGRAM);
			char readdat = 0x00;

			std::string textout;
			do
			{
				readdat = spc.read_byte(pointer++);
				textout.append(1, readdat);
			} while (readdat != 0x00);

			//logerror("%s: %s", machine().describe_context(), textout);
			//logerror("\n");
		}
	}
}

uint8_t leapfrog_iquest_state::unk_ffa8_r()
{
	logerror("%s: read from ffa8 ----------- POSSIBLE END OF DEBUG TEXT OPERATION?\n", machine().describe_context());
	return 0x00;
}

void leapfrog_iquest_state::unk_ffa8_w(uint8_t data)
{
	logerror("%s: write to ffa8 %02x ----------- POSSIBLE START OF DEBUG TEXT OPERATION?\n", machine().describe_context(), data);
	m_ffa8 = data;
}

void leapfrog_iquest_state::unk_ffa9_w(uint8_t data)
{
	logerror("%s: write to ffa9 %02x ----------- POSSIBLE TRIGGER DEBUG TEXT OPERATION?? (current banks are %08x %08x %08x)\n", machine().describe_context(), data, ((m_lowerbank[0] << 8) | (m_lowerbank[1])) * 0x8000, ((m_upperbank[0] << 8) | (m_upperbank[1])) * 0x8000, ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000);
}




void leapfrog_iquest_state::ext_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(leapfrog_iquest_state::iowindow_r), FUNC(leapfrog_iquest_state::iowindow_w)); // assume this accesses the same space, with different bank register

	map(0xc260, 0xc52f).ram(); // = clears 0x2d0 bytes (90*64 / 8) display buffer?
	map(0xc530, 0xc7ff).ram(); // = clears 0x2d0 bytes (90*64 / 8) display buffer?

	//map(0xf001, 0xf056).ram(); // written as a block
	map(0xf000, 0xf5ff).ram(); // ? 0xf400 - 0xf427 written as a block, other areas uncertain, might be more registers in here as there are reads too

	map(0xfc00, 0xfc00).r(FUNC(leapfrog_iquest_state::unk_fc00_r));
	map(0xfc01, 0xfc04).r(FUNC(leapfrog_iquest_state::unk_fc01_r));

	map(0xfc06, 0xfc07).rw(FUNC(leapfrog_iquest_state::lowerbank_r), FUNC(leapfrog_iquest_state::lowerbank_w)); // ROM / RAM window in main space at 0000-7fff
	map(0xfc08, 0xfc09).rw(FUNC(leapfrog_iquest_state::upperbank_r), FUNC(leapfrog_iquest_state::upperbank_w)); // ROM / RAM window in main space at 8000-ffff
	map(0xfc0a, 0xfc0b).rw(FUNC(leapfrog_iquest_state::iobank_r), FUNC(leapfrog_iquest_state::iobank_w)); // ROM / RAM window in ext space at 0000-7fff

	map(0xfc22, 0xfc22).w(FUNC(leapfrog_iquest_state::unk_fc22_w));

	map(0xfc2f, 0xfc2f).r(FUNC(leapfrog_iquest_state::unk_fc2f_r));
	map(0xfc3f, 0xfc3f).rw(FUNC(leapfrog_iquest_state::unk_fc3f_r), FUNC(leapfrog_iquest_state::unk_fc3f_w));

	map(0xfce5, 0xfce5).rw(FUNC(leapfrog_iquest_state::unk_fce5_r), FUNC(leapfrog_iquest_state::unk_fce5_w));

	map(0xff00, 0xff01).r(FUNC(leapfrog_iquest_state::unk_ff00_01_r));

	// it seems more likely that this is just RAM, as that after setting pointers in RAM, they're used to construct
	// strings to transmit over the serial.
	// however, mapping this area as RAM instead results in the program stalling much earlier, waiting for $24.3 to
	// be cleared.
	// 017658: 20 23 fd  jb    $24.3,$17658
	//
	//The only realistic place for this to be cleared is deep in the interrupt handler for
	// Serial Receive/Transmit
	// 010023: 02 76 9e  ljmp  $769E
	// (which eventually can reach)
	// 016991: c2 23     clr   $24.3
	// however I'm uncertain how to get the driver to trigger this at all.
	if (0)
	{
		map(0xff80, 0xffff).ram();
	}
	else
	{
		map(0xff80, 0xff80).rw(FUNC(leapfrog_iquest_state::unk_ff80_r), FUNC(leapfrog_iquest_state::unk_ff80_w));

		map(0xff81, 0xff84).w(FUNC(leapfrog_iquest_state::unk_ff81_84_w));

		map(0xff91, 0xff93).rw(FUNC(leapfrog_iquest_state::unk_ff91_93_r), FUNC(leapfrog_iquest_state::unk_ff91_93_w));

		map(0xffa8, 0xffa8).rw(FUNC(leapfrog_iquest_state::unk_ffa8_r), FUNC(leapfrog_iquest_state::unk_ffa8_w));
		map(0xffa9, 0xffa9).w(FUNC(leapfrog_iquest_state::unk_ffa9_w));
	}
}

DEVICE_IMAGE_LOAD_MEMBER(leapfrog_iquest_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( leapfrog_iquest )
INPUT_PORTS_END

uint32_t leapfrog_iquest_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

// doesn't help?
void leapfrog_iquest_state::rx_line_hack(int state)
{
	if (0)
	{
		// HACK: force past the wait loop if we're treating ff80 - ffff as RAM
		address_space& spc = m_maincpu->space(AS_DATA);
		uint8_t readdat = spc.read_byte(0x24);
		readdat &= ~0x08;
		spc.write_byte(0x24, readdat);
	}
}

uint8_t leapfrog_iquest_state::port0_r()
{
	logerror("%s: port0_r\n", machine().describe_context());
	return 0x00;
}

uint8_t leapfrog_iquest_state::port1_r()
{
	logerror("%s: port1_r\n", machine().describe_context());
	return 0x00;
}

uint8_t leapfrog_iquest_state::port2_r()
{
	logerror("%s: port2_r\n", machine().describe_context());
	return 0x00;
}

uint8_t leapfrog_iquest_state::port3_r()
{
	logerror("%s: port3_r\n", machine().describe_context());
	return 0x00;
}

void leapfrog_iquest_state::port0_w(u8 data)
{
	logerror("%s: port0_w %02x\n", machine().describe_context(), data);
}

void leapfrog_iquest_state::port1_w(u8 data)
{
	logerror("%s: port1_w %02x\n", machine().describe_context(), data);
}

void leapfrog_iquest_state::port2_w(u8 data)
{
	logerror("%s: port2_w %02x\n", machine().describe_context(), data);
}

void leapfrog_iquest_state::port3_w(u8 data)
{
	logerror("%s: port3_w %02x\n", machine().describe_context(), data);
}



void leapfrog_iquest_state::leapfrog_base(machine_config &config)
{
	// seems to have an IRQ vector at 002b, which would suggest it's an 8052 or similar, rather than plain 8031?
	//I8052(config, m_maincpu, 96000000/10); // unknown clock
	I8032(config, m_maincpu, 96000000/10); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_iquest_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_iquest_state::ext_map);
	m_maincpu->port_in_cb<0>().set(FUNC(leapfrog_iquest_state::port0_r));
	m_maincpu->port_out_cb<0>().set(FUNC(leapfrog_iquest_state::port0_w));
	m_maincpu->port_in_cb<1>().set(FUNC(leapfrog_iquest_state::port1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(leapfrog_iquest_state::port1_w));
	m_maincpu->port_in_cb<2>().set(FUNC(leapfrog_iquest_state::port2_r));
	m_maincpu->port_out_cb<2>().set(FUNC(leapfrog_iquest_state::port2_w));
	m_maincpu->port_in_cb<3>().set(FUNC(leapfrog_iquest_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(leapfrog_iquest_state::port3_w));

	ADDRESS_MAP_BANK(config, "rombank").set_map(&leapfrog_iquest_state::rom_map).set_options(ENDIANNESS_LITTLE, 8, 31, 0x80000000);
}

void leapfrog_iquest_state::leapfrog_iquest(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(90, 64);
	m_screen->set_visarea(0, 90-1, 0, 64-1);
	m_screen->set_screen_update(FUNC(leapfrog_iquest_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_iquest_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_iquest_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_iquest_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_iquest_cart");
}

void leapfrog_turboextreme_state::leapfrog_turboex(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64, 32); // unknown resolution, lower than iquest
	m_screen->set_visarea(0, 64-1, 0, 32-1);
	m_screen->set_screen_update(FUNC(leapfrog_turboextreme_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_turboextreme_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_turboextreme_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_turboextreme_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("turboextreme_cart");
}

void leapfrog_turbotwistmath_state::leapfrog_turbotwistmath(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64, 8); // unknown resolution, single row display
	m_screen->set_visarea(0, 64-1, 0, 8-1);
	m_screen->set_screen_update(FUNC(leapfrog_turbotwistmath_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_turbotwistmath_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_turbotwistmath_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_turbotwistmath_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("ttwist_math_cart");
}

void leapfrog_turbotwistspelling_state::leapfrog_turbotwistspelling(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64, 8); // unknown resolution, single row display
	m_screen->set_visarea(0, 64-1, 0, 8-1);
	m_screen->set_screen_update(FUNC(leapfrog_turbotwistspelling_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_turbotwistspelling_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_turbotwistspelling_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_turbotwistspelling_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("ttwist_spelling_cart");
}

void leapfrog_turbotwistvocabulator_state::leapfrog_turbotwistvocabulator(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64, 8); // unknown resolution, single row display
	m_screen->set_visarea(0, 64-1, 0, 8-1);
	m_screen->set_screen_update(FUNC(leapfrog_turbotwistvocabulator_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_turbotwistvocabulator_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_turbotwistvocabulator_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_turbotwistvocabulator_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("ttwist_vocabulator_cart");
}


void leapfrog_turbotwistbrainquest_state::leapfrog_turbotwistbrainquest(machine_config &config)
{
	leapfrog_iquest_state::leapfrog_base(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(64, 8); // unknown resolution, single row display
	m_screen->set_visarea(0, 64-1, 0, 8-1);
	m_screen->set_screen_update(FUNC(leapfrog_turbotwistbrainquest_state::screen_update));
	m_screen->screen_vblank().set(FUNC(leapfrog_turbotwistbrainquest_state::rx_line_hack));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_turbotwistbrainquest_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_turbotwistbrainquest_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("ttwist_brainquest_cart");
}

ROM_START( iquest )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "iquest.bin", 0x000000, 0x400000, CRC(f785dc4e) SHA1(ec002c18df536737334fe6b7db0e7342bad7b66b))

	// there is also a 39vf512 flash containing user data
ROM_END

ROM_START( turboex )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "turbotwistextreme.bin", 0x000000, 0x400000, CRC(d7cabcff) SHA1(7813811c0518aba017f7a79fd477be5ed9fa7529))
ROM_END

ROM_START( ttwistm )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "turbotwistmath.bin", 0x000000, 0x200000, CRC(a21d3723) SHA1(d0ae245621d7bc92bdf9fd683908690db6e25133))
ROM_END

ROM_START( ttwistsp ) // PCB marks ROM glob as 8M
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ttspelling.bin", 0x000000, 0x100000, CRC(09715a8e) SHA1(5d7eb7b714b95012aeb03c37dd0b71a1c025bdda))
ROM_END

ROM_START( ttwistvc ) // PCB marks ROM glob as 8M
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vocabulator.bin", 0x000000, 0x100000, CRC(71f9c4c9) SHA1(8cafb42bd56c7db99949781e42b6ad4991ee2246))
ROM_END

ROM_START( ttwistbq )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "turbotwistbrainquest.bin", 0x000000, 0x200000, CRC(b184a517) SHA1(181975da58b3d117a389c54ac025b6a9b24342b2))
ROM_END

ROM_START( ttwistfb ) // PCB marks ROM glob as 16M
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "factblaster.bin", 0x000000, 0x200000, CRC(8efd63f5) SHA1(2cbd1299006ad9743d846e774afe7a134ba0fce3))
ROM_END

} // anonymous namespace


//    year, name,        parent,    compat, machine,                         input,            class,                                init,       company,    fullname,                         flags
// it is unknown if the versions of IQuest without 4.0 on the case have different system ROM
CONS( 2004, iquest,      0,         0,      leapfrog_iquest,                 leapfrog_iquest,  leapfrog_iquest_state,                empty_init, "LeapFrog", "IQuest 4.0 (US)",                MACHINE_IS_SKELETON )

CONS( 2004, turboex,     0,         0,      leapfrog_turboex,                leapfrog_iquest,  leapfrog_turboextreme_state,          empty_init, "LeapFrog", "Turbo Extreme (US)",             MACHINE_IS_SKELETON )

// from a silver unit with orange lettering (there are different case styles, it is unknown if the software changed)
CONS( 2002, ttwistm,     0,         0,      leapfrog_turbotwistmath,         leapfrog_iquest,  leapfrog_turbotwistmath_state,        empty_init, "LeapFrog", "Turbo Twist Math (US)",          MACHINE_IS_SKELETON )

// Brain Quest / Fact Blaster are compatible with the same cartridges
CONS( 200?, ttwistfb,    0,         0,      leapfrog_turbotwistbrainquest,   leapfrog_iquest,  leapfrog_turbotwistbrainquest_state,  empty_init, "LeapFrog", "Turbo Twist Fact Blaster (US)",  MACHINE_IS_SKELETON )
CONS( 2002, ttwistbq,    0,         0,      leapfrog_turbotwistbrainquest,   leapfrog_iquest,  leapfrog_turbotwistbrainquest_state,  empty_init, "LeapFrog", "Turbo Twist Brain Quest (US)",   MACHINE_IS_SKELETON )

// from a green unit with blue edges
CONS( 2000, ttwistsp,    0,         0,      leapfrog_turbotwistspelling,     leapfrog_iquest,  leapfrog_turbotwistspelling_state,    empty_init, "LeapFrog", "Turbo Twist Spelling (US)",      MACHINE_IS_SKELETON )

CONS( 2001, ttwistvc,    0,         0,      leapfrog_turbotwistvocabulator,  leapfrog_iquest,  leapfrog_turbotwistvocabulator_state, empty_init, "LeapFrog", "Turbo Twist Vocabulator (US)",   MACHINE_IS_SKELETON )

// Undumped units

// These have 2 digit 7-seg style displays, it is unknown if they are on related hardware
// Twist & Shout Addition
// Twist & Shout Subtraction
// Twist & Shout Multiplication
// Twist & Shout Division

// This appears to have a 4 digit 7-seg style display, unknown if it is on related hardware
// Twist & Shout Phonics
