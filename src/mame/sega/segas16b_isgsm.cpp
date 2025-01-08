// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

ISG 'Selection Master' Type 2006 hardware

This is a 'multi-game' cart system (only the operator can select the game, via a dipswitch exposed from the cartridge)

The system is designed to look like a PGM system (the ISG logo and fonts are ripped straight from original IGS material,
and the external casing of the unit is near identical)  The system does NOT however run PGM games, the cartridges won't
fit, and the hardware is basically a bootleg of Sega System 16 instead of PGM!  So far only 2 cartridges have been seen.

There are various levels of 'protection' on the system
 - Address XOR + 16-bit bitswap on the BIOS ROM and Cartridge ROMs
 - A device which performs a 32-bit bitswap, used to produce decryption keys for the data compressed on the cartridges
 - An alternate way of reading the cartridge data through a port, which causes an additional 8-bit data xor to be applied
 - A simple hardware RLE decompression device used to decrypt data for the BIOS (the games use a stronger software
   implementation instead)

The PCB is entirely custom SMT components, as you'd expect from a modern bootleg, all chips have had their surface details
removed.

*/

#include "emu.h"
#include "segas16b.h"
#include "segaipt.h"

#include "speaker.h"


namespace {

class isgsm_state : public segas16b_state
{
public:
	// construction/destruction
	isgsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas16b_state(mconfig, type, tag)
		, m_read_xor(0)
		, m_cart_addrlatch(0)
		, m_cart_addr(0)
		, m_data_type(0)
		, m_data_addr(0)
		, m_data_mode(0)
		, m_addr_latch(0)
		, m_security_value(0)
		, m_security_latch(0)
		, m_rle_control_position(8)
		, m_rle_control_byte(0)
		, m_rle_latched(false)
		, m_rle_byte(0)
	{ }

	void isgsm(machine_config &config);

	// driver init
	void init_isgsm();
	void init_shinfz();
	void init_tetrbx();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// read/write handlers
	void cart_addr_high_w(uint16_t data);
	void cart_addr_low_w(uint16_t data);
	uint16_t cart_data_r();
	void data_w(uint16_t data);
	void datatype_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void addr_high_w(uint16_t data);
	void addr_low_w(uint16_t data);
	void cart_security_high_w(uint16_t data);
	void cart_security_low_w(uint16_t data);
	uint16_t cart_security_low_r();
	uint16_t cart_security_high_r();
	void sound_reset_w(uint16_t data);
	void main_bank_change_w(uint16_t data);

	// security callbacks
	uint32_t shinfz_security(uint32_t input);
	uint32_t tetrbx_security(uint32_t input);

	// configuration
	uint8_t           m_read_xor;
	typedef delegate<uint32_t (uint32_t)> security_callback_delegate;
	security_callback_delegate m_security_callback;

	// internal state
	uint16_t          m_cart_addrlatch;
	uint32_t          m_cart_addr;
	uint8_t           m_data_type;
	uint32_t          m_data_addr;
	uint8_t           m_data_mode;
	uint16_t          m_addr_latch;
	uint32_t          m_security_value;
	uint16_t          m_security_latch;
	uint8_t           m_rle_control_position;
	uint8_t           m_rle_control_byte;
	bool              m_rle_latched;
	uint8_t           m_rle_byte;
	void isgsm_map(address_map &map) ATTR_COLD;
};



void isgsm_state::cart_addr_high_w(uint16_t data)
{
	m_cart_addrlatch = data;
}

void isgsm_state::cart_addr_low_w(uint16_t data)
{
	m_cart_addr = data | (m_cart_addrlatch << 16);
}

// the cart can be read here 8-bits at a time.
// when reading from this port the data is xored by a fixed value depending on the cart
uint16_t isgsm_state::cart_data_r()
{
	int size = memregion("gamecart_rgn")->bytes();
	uint8_t *rgn = memregion("gamecart_rgn")->base();
	return rgn[(++m_cart_addr & (size - 1)) ^ 1] ^ m_read_xor;
}

void isgsm_state::data_w(uint16_t data)
{
	// m_data_type
	// rrrp o?dd
	//
	// r = bit-rotation
	// p = apply rotation post-operation
	// dd = destination (0 = sprites, 1 = tiles, 2 = soundcpu, 3 = maincpu)
	// o = write opcodes? (not used by any dumped carts)

	uint8_t *dest = nullptr;

	switch (m_data_type & 0x0f)
	{
		case 0x0: dest = memregion("sprites")->base();
			break;

		case 0x1: dest = memregion("gfx1")->base();
			break;

		case 0x2: dest = memregion("soundcpu")->base();
			break;

		case 0x3: dest = memregion("maincpu")->base();
			break;

		default: // no other cases?
			break;
	}

	// pre-rotate
	if ((m_data_type & 0x10) == 0x00)
	{
		// 8-bit rotation - used by bloxeed
		uint8_t shift = ((m_data_type >> 5 & 7) - 1) & 7;
		data = (data << shift & 0xff) | ((data & 0xff) >> (8 - shift));
	}

	if (dest)
	{
		// mode register
		//  droo
		//  d = direction
		//  r = hardware rle (used by the bios only, games are using some kind of software compression/encryption
		//  oo = operator mode (0 = plain, 1 = xor, 2 = OR, 3 = AND)

		// address can auto-increment or decrement, happens *before* data is written
		int bytes_to_write = 1;

		if (m_data_mode & 0x4)
		{
			if (!m_rle_latched)
			{
				if (m_rle_control_position == 8)
				{
					m_rle_control_byte = data;
					m_rle_control_position = 0;
					bytes_to_write = 0;
				}
				else
				{
					if (((m_rle_control_byte << m_rle_control_position) & 0x80) == 0) // RLE
					{
						m_rle_byte = data;
						m_rle_latched = true;
					}
					else
						bytes_to_write = 1;

					m_rle_control_position++;
				}
			}
			else
			{
				m_rle_latched = false;
				bytes_to_write = data+2;
				data = m_rle_byte;
			}
		}

		for (int i = 0; i < bytes_to_write; i++)
		{
			uint8_t byte = 0;

			if (m_data_mode & 0x8)
			{
				m_data_addr++;
				m_data_addr &= 0xfffffff;
			}
			else
			{
				m_data_addr--;
				m_data_addr &= 0xfffffff;
			}

			switch (m_data_mode & 0x3)
			{
				case 0: byte = data; break;
				case 1: byte = dest[m_data_addr] ^ data; break;
				case 2: byte = dest[m_data_addr] | data; break;
				case 3: byte = dest[m_data_addr] & data; break;
			}

			// post-rotate
			if ((m_data_type & 0x10) == 0x10)
			{
				// 8-bit rotation - used by tetris
				uint8_t shift = ((m_data_type >> 5 & 7) - 1) & 7;
				byte = (byte << shift & 0xff) | ((byte & 0xff) >> (8 - shift));
			}

			dest[m_data_addr] = byte;

			if (dest == memregion("gfx1")->base())
			{
				// we need to re-decode the tiles if writing to this area to keep MAME happy
				m_gfxdecode->gfx(0)->mark_dirty((m_data_addr & 0x1ffff) / 8);
			}
		}
	}
}


void isgsm_state::datatype_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("type set to %04x %04x\n", data, mem_mask);
	m_data_type = data;
}

void isgsm_state::addr_high_w(uint16_t data)
{
	// this is latched, doesn't get applied until low part is written.
	m_addr_latch = data;
}

void isgsm_state::addr_low_w(uint16_t data)
{
	// update the address and mode
	m_data_mode = (m_addr_latch & 0xf000) >> 12;
	m_data_addr = data | ((m_addr_latch & 0x0fff) << 16);

	// also resets the RLE
	m_rle_control_position = 8;
	m_rle_control_byte = 0;
	m_rle_latched = false;
}

void isgsm_state::cart_security_high_w(uint16_t data)
{
	// this is latched, doesn't get applied until low part is written.
	m_security_latch = data;
}

uint32_t isgsm_state::shinfz_security(uint32_t input)
{
	return bitswap<32>(input, 19, 20, 25, 26, 15, 0, 16, 2, 8, 9, 13, 14, 31, 21, 7, 18, 11, 30, 22, 17, 3, 4, 12, 28, 29, 5, 27, 10, 23, 24, 1, 6);
}

uint32_t isgsm_state::tetrbx_security(uint32_t input)
{
	// no bitswap on this cart? just returns what was written
	return input;
}



void isgsm_state::cart_security_low_w(uint16_t data)
{
	m_security_value = data | m_security_latch << 16;
	// come up with security answer
	// -- this probably depends on the cart.
	m_security_value = m_security_callback(m_security_value);
}

uint16_t isgsm_state::cart_security_low_r()
{
	return m_security_value & 0xffff;
}

uint16_t isgsm_state::cart_security_high_r()
{
	return (m_security_value >> 16) & 0xffff;
}

void isgsm_state::sound_reset_w(uint16_t data)
{
	if (data == 0)
	{
		m_soundcpu->reset();
		m_soundcpu->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		m_soundcpu->reset();
		m_soundcpu->suspend(SUSPEND_REASON_HALT, 1);
	}
}

void isgsm_state::main_bank_change_w(uint16_t data)
{
	// other values on real hw have strange results, change memory mapping etc??
	if (data != 0)
		membank("mainbank")->set_base(memregion("maincpu")->base());
}

void isgsm_state::isgsm_map(address_map &map)
{
	map(0x000000, 0x0fffff).bankr("mainbank"); // this area is ALWAYS read-only, even when the game is banked in
	map(0x200000, 0x23ffff).ram(); // used during startup for decompression
	map(0x3f0000, 0x3fffff).w(FUNC(isgsm_state::rom_5704_bank_w));
	map(0x400000, 0x40ffff).rw(m_segaic16vid, FUNC(segaic16_video_device::tileram_r), FUNC(segaic16_video_device::tileram_w)).share("tileram");
	map(0x410000, 0x410fff).rw(m_segaic16vid, FUNC(segaic16_video_device::textram_r), FUNC(segaic16_video_device::textram_w)).share("textram");
	map(0x440000, 0x4407ff).ram().share("sprites");
	map(0x840000, 0x840fff).ram().w(FUNC(isgsm_state::paletteram_w)).share("paletteram");
	map(0xc40000, 0xc43fff).rw(FUNC(isgsm_state::standard_io_r), FUNC(isgsm_state::standard_io_w));

	map(0xe00000, 0xe00001).w(FUNC(isgsm_state::data_w)); // writes decompressed data here (copied from RAM..)
	map(0xe00002, 0xe00003).w(FUNC(isgsm_state::datatype_w)); // selects which 'type' of data we're writing
	map(0xe00004, 0xe00005).w(FUNC(isgsm_state::addr_high_w)); // high address, and some mode bits
	map(0xe00006, 0xe00007).w(FUNC(isgsm_state::addr_low_w));  // low address

	map(0xe80000, 0xe80001).r(FUNC(isgsm_state::cart_data_r)); // 8-bit port that the entire cart can be read from
	map(0xe80002, 0xe80003).portr("CARDDSW");
	map(0xe80004, 0xe80005).w(FUNC(isgsm_state::cart_addr_high_w));
	map(0xe80006, 0xe80007).w(FUNC(isgsm_state::cart_addr_low_w));
	map(0xe80008, 0xe80009).rw(FUNC(isgsm_state::cart_security_high_r), FUNC(isgsm_state::cart_security_high_w)); // 32-bit bitswap device..
	map(0xe8000a, 0xe8000b).rw(FUNC(isgsm_state::cart_security_low_r), FUNC(isgsm_state::cart_security_low_w));

	map(0xee0000, 0xefffff).rom().region("gamecart_rgn", 0); // only the first 0x20000 bytes of the cart are visible here..

	map(0xfe0006, 0xfe0007).w(FUNC(isgsm_state::sound_w16));
	map(0xfe0008, 0xfe0009).w(FUNC(isgsm_state::sound_reset_w));
	map(0xfe000a, 0xfe000b).w(FUNC(isgsm_state::main_bank_change_w));
	map(0xffc000, 0xffffff).ram().share("workram");
}



static INPUT_PORTS_START( isgsm )
	PORT_INCLUDE( system16b_generic )

	//PORT_MODIFY("DSW2")
	//"SW2:1" unused
	//"SW2:2" unused
	//"SW2:3" unused
	//"SW2:4" unused
	//"SW2:5" unused
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("CARDDSW") // on the gamecard..
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( shinfz )
	PORT_INCLUDE( isgsm )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	//"SW2:2" unused
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, "Extra Ship Cost" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	//PORT_MODIFY("DSW1")
	//"SW1:1" unused
	//"SW1:2" unused
	//"SW1:3" unused
	//"SW1:4" unused
	//"SW1:5" unused
	//"SW1:6" unused
	//"SW1:7" unused
	//"SW1:8" unused

	PORT_MODIFY("CARDDSW") // on the gamecard..
	PORT_DIPNAME( 0x0003, 0x0000, "Game Type" )
	PORT_DIPSETTING(      0x0000, "Shinobi Ninja Game" )
	PORT_DIPSETTING(      0x0001, "FZ-2006 Game I" )
	PORT_DIPSETTING(      0x0002, "FZ-2006 Game II" )
	PORT_DIPSETTING(      0x0003, "Invalid" ) // this (or higher) gives 'BAD SELECT - Check Switch' message.
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tetrbx )
	PORT_INCLUDE( isgsm )

	//PORT_MODIFY("DSW2")
	//"SW2:1" unused
	//"SW2:2" unused
	//"SW2:3" unused
	//"SW2:4" unused
	//"SW2:5" unused
	//"SW2:6" unused
	//"SW2:7" unused
	//"SW2:8" unused

	//PORT_MODIFY("DSW1")
	//"SW1:1" unused
	//"SW1:2" unused
	//"SW1:3" unused
	//"SW1:4" unused
	//"SW1:5" unused
	//"SW1:6" unused
	//"SW1:7" unused
	//"SW1:8" unused

	PORT_MODIFY("CARDDSW") // on the gamecard..
	PORT_DIPNAME( 0x0003, 0x0000, "Game Type" )
	PORT_DIPSETTING(      0x0000, "Tetris" )
	PORT_DIPSETTING(      0x0001, "Tetris II (Blox)" )
	PORT_DIPSETTING(      0x0002, "Tetris Turbo" )
	PORT_DIPSETTING(      0x0003, "Invalid" ) // this (or higher) gives 'BAD SELECT - Check Switch' message.
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void isgsm_state::machine_reset()
{
	m_cart_addrlatch = 0;
	m_cart_addr = 0;
	m_data_type = 0;
	m_data_addr = 0;
	m_data_mode = 0;
	m_addr_latch = 0;
	m_security_value = 0;
	m_security_latch = 0;
	m_rle_control_position = 0;
	m_rle_control_byte = 0;
	m_rle_latched = 0;
	m_rle_byte = 0;

	m_segaic16vid->tilemap_reset(*m_screen);

	// configure sprite banks
	if (m_sprites.found())
		for (int i = 0; i < 16; i++)
			m_sprites->set_bank(i, i);

	membank("mainbank")->set_base(memregion("bios")->base());
}

void isgsm_state::machine_start()
{
	segas16b_state::machine_start();

	save_item(NAME(m_cart_addrlatch));
	save_item(NAME(m_cart_addr));
	save_item(NAME(m_data_type));
	save_item(NAME(m_data_addr));
	save_item(NAME(m_data_mode));
	save_item(NAME(m_addr_latch));
	save_item(NAME(m_security_value));
	save_item(NAME(m_security_latch));
	save_item(NAME(m_rle_control_position));
	save_item(NAME(m_rle_control_byte));
	save_item(NAME(m_rle_latched));
	save_item(NAME(m_rle_byte));
}


void isgsm_state::isgsm(machine_config &config)
{
	system16b(config);

	// basic machine hardware
	config.device_remove("maincpu");
	config.device_remove("mapper");

	M68000(config, m_maincpu, 16000000); // no obvious CPU, but seems to be clocked faster than an original system16 based on the boot times
	m_maincpu->set_addrmap(AS_PROGRAM, &isgsm_state::isgsm_map);
	m_maincpu->set_vblank_int("screen", FUNC(isgsm_state::irq4_line_hold));

	m_soundcpu->set_addrmap(AS_PROGRAM, &isgsm_state::bootleg_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &isgsm_state::bootleg_sound_portmap);

	GENERIC_LATCH_8(config, m_soundlatch);
}

void isgsm_state::init_isgsm()
{
	init_generic_5521();

	// decrypt the bios...
	std::vector<uint16_t> temp(0x20000/2);
	uint16_t *rom = (uint16_t *)memregion("bios")->base();
	for (int addr = 0; addr < 0x20000/2; addr++)
		temp[addr ^ 0x4127] = bitswap<16>(rom[addr], 6, 14, 4, 2, 12, 10, 8, 0, 1, 9, 11, 13, 3, 5, 7, 15);
	memcpy(rom, &temp[0], 0x20000);
}

void isgsm_state::init_shinfz()
{
	init_isgsm();

	std::vector<uint16_t> temp(0x200000/2);
	uint16_t *rom = (uint16_t *)memregion("gamecart_rgn")->base();
	for (int addr = 0; addr < 0x200000/2; addr++)
		temp[addr ^ 0x68956] = bitswap<16>(rom[addr], 8, 4, 12, 3, 6, 7, 1, 0, 15, 11, 5, 14, 10, 2, 9, 13);
	memcpy(rom, &temp[0], 0x200000);

	m_read_xor = 0x66;
	m_security_callback = security_callback_delegate(&isgsm_state::shinfz_security, this);
}

void isgsm_state::init_tetrbx()
{
	init_isgsm();

	std::vector<uint16_t> temp(0x80000/2);
	uint16_t *rom = (uint16_t *)memregion("gamecart_rgn")->base();
	for (int addr = 0; addr < 0x80000/2; addr++)
		temp[addr ^ 0x2a6e6] = bitswap<16>(rom[addr], 4, 0, 12, 5, 7, 3, 1, 14, 10, 11, 9, 6, 15, 2, 13, 8);
	memcpy(rom, &temp[0], 0x80000);

	m_read_xor = 0x73;
	m_security_callback = security_callback_delegate(&isgsm_state::tetrbx_security, this);
}


// other regions are filled with data from the game cartridge at run-time via port accesses
#define ISGSM_BIOS \
	ROM_REGION16_BE( 0x100000, "bios", 0 ) \
	ROM_LOAD16_WORD_SWAP("ism2006v00.u1",0x00000,0x20000, CRC(2292585c) SHA1(97ba0e0f0be832a5114d95151e519bc027f6675b) ) \
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x60000, "gfx1", ROMREGION_ERASE00 ) \
	ROM_REGION16_BE( 0x200000, "sprites", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x40000, "soundcpu", ROMREGION_ERASE00 )

ROM_START( isgsm )
	ISGSM_BIOS
	ROM_REGION16_BE( 0x200000, "gamecart_rgn", ROMREGION_ERASE00 )
ROM_END

ROM_START( tetrbx )
	ISGSM_BIOS

	ROM_REGION16_BE( 0x400000, "gamecart_rgn", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP("tetr06.u13", 0x00000, 0x080000, CRC(884dd693) SHA1(33549613844be16f7903c9b0cf4e028f0bceaff2) )
ROM_END

ROM_START( shinfz )
	ISGSM_BIOS

	ROM_REGION16_BE( 0x200000, "gamecart_rgn", 0 )
	ROM_LOAD16_WORD_SWAP("shin06.u13", 0x00000, 0x200000, CRC(39d773e9) SHA1(5284f90cb5190128a17ebee8b539a39c8914c364) )
ROM_END

} // anonymous namespace


//             YEAR, NAME,   PARENT  MACHINE  INPUT   CLASS        INIT         MONITOR  COMPANY          FULLNAME, FLAGS
GAME(          2006, isgsm,  0,      isgsm,   isgsm,  isgsm_state, init_isgsm,  ROT0,    "bootleg (ISG)", "ISG Selection Master Type 2006 BIOS", MACHINE_IS_BIOS_ROOT )

/* 01 */ // ?? unknown
/* 02 */ GAME( 2006, tetrbx, isgsm,  isgsm,   tetrbx, isgsm_state, init_tetrbx, ROT0,    "bootleg (ISG)", "Tetris / Bloxeed (Korean System 16 bootleg) (ISG Selection Master Type 2006)", 0 )
/* 03 */ GAME( 2008, shinfz, isgsm,  isgsm,   shinfz, isgsm_state, init_shinfz, ROT0,    "bootleg (ISG)", "Shinobi / FZ-2006 (Korean System 16 bootleg) (ISG Selection Master Type 2006)", 0 ) // claims it's released in 2006, but set includes the PS2/S16 remake of Fantasy Zone II which is clearly from 2008
