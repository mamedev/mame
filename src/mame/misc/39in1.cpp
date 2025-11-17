// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ryan Holtz, Andreas Naive
/**************************************************************************
 *
 * 39in1.cpp - bootleg MAME-based "39-in-1" arcade PCB
 * Skeleton by R. Belmont, thanks to the Guru
 * PXA255 Peripheral hookup by Ryan Holtz
 * Decrypt by Andreas Naive
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5TE instruction set without the FPU
 *    ARM standard MMU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 * International Amusement Machine (I.A.M.) slots from the second half of the
 * 2000s use very similar PCBs (almost same main components, very similar layout,
 * same encryption).
 *
 * TODO:
 *   - PXA255 peripherals
 *   - 4in1a and 4in1b are very similar to 39in1, currently boot but stuck at
 *     'Hardware Check' with an error
 *   - rodent should be correctly decrypted but expects something different
 *     from the CPLD (probably)
 *   - 19in1, 48in1, 48in1a, 48in1b, 48in1c, 60in1 have more conditional XORs,
 *     encryption isn't completely beaten yet
 *   - fruitwld, fruitwlda, jumanjia show 'HW_002 ERROR', probably missing CPLD
 *     emulation
 *
 *
 * 39in1 notes:
 * The actual PCB just normally boots up to the game, whereas in MAME it
 * defaults to test mode and checks the ROM, then jumps out to the game
 * after loading all 39 games. It is almost like it is defaulting to test
 * mode on at bootup. On the real PCB, it just loads the 39 games then
 * shows the game selection menu. Going into the test mode does the same
 * code check but then shows  a test screen with color bars. Pressing
 * next shows a high score clear screen (if the HS dip is on). Pressing
 * next shows the game dips screens and allows you to set up soft dip
 * switches for each of the 39 games.
 **************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/eepromser.h"
#include "machine/pxa255.h"


namespace {

class _39in1_state : public driver_device
{
public:
	_39in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pxa_periphs(*this, "pxa_periphs")
		, m_eeprom(*this, "eeprom")
		, m_ram(*this, "ram")
		, m_mcu_ipt(*this, "MCUIPT")
		, m_dsw(*this, "DSW")
	{ }

	void _39in1(machine_config &config) ATTR_COLD;
	void base(machine_config &config) ATTR_COLD;
	void iam(machine_config &config) ATTR_COLD;

	void init_4in1a() ATTR_COLD;
	void init_4in1b() ATTR_COLD;
	void init_19in1() ATTR_COLD;
	void init_39in1() ATTR_COLD;
	void init_48in1() ATTR_COLD;
	void init_48in1a() ATTR_COLD;
	void init_48in1c() ATTR_COLD;
	void init_60in1() ATTR_COLD;
	void init_rodent() ATTR_COLD;

	// I.A.M. slots
	void init_fruitwld();
	void init_jumanji();
	void init_jumanjia();
	void init_plutus();
	void init_pokrwild();

	DECLARE_INPUT_CHANGED_MEMBER(set_flip_dip);
	DECLARE_INPUT_CHANGED_MEMBER(set_res_dip);
	DECLARE_INPUT_CHANGED_MEMBER(set_hiscore_dip);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 m_seed;
	u32 m_magic;
	u32 m_state;
	u32 m_mcu_ipt_pc;

	required_device<cpu_device> m_maincpu;
	required_device<pxa255_periphs_device> m_pxa_periphs;
	required_device<eeprom_serial_93c66_16bit_device> m_eeprom;
	required_shared_ptr<u32> m_ram;
	required_ioport m_mcu_ipt;
	required_ioport m_dsw;

	u32 cpld_r(offs_t offset);
	void cpld_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 prot_cheater_r();

	void _39in1_map(address_map &map) ATTR_COLD;
	void base_map(address_map &map) ATTR_COLD;
	void iam_map(address_map &map) ATTR_COLD;

	void decrypt(u8 xor00, u8 xor02, u8 xor04, u8 xor08, u8 xor10, u8 xor20, u8 xor40, u8 xor80, u8 bit7, u8 bit6, u8 bit5, u8 bit4, u8 bit3, u8 bit2, u8 bit1, u8 bit0) ATTR_COLD;
	void further_decrypt(u8 xor400, u8 xor800, u8 xor1000, u8 xor2000, u8 xor4000, u8 xor8000) ATTR_COLD;
};

void _39in1_state::machine_reset()
{
	m_pxa_periphs->gpio_in<1>(1);

	const u32 dsw = m_dsw->read();
	m_pxa_periphs->gpio_in<53>(BIT(dsw, 0));
	m_pxa_periphs->gpio_in<54>(BIT(dsw, 1));
	m_pxa_periphs->gpio_in<56>(BIT(dsw, 2));

	m_eeprom->di_write(ASSERT_LINE);
}

u32 _39in1_state::cpld_r(offs_t offset)
{
	// if (m_maincpu->pc() != m_mcu_ipt_pc) printf("CPLD read @ %x (PC %x state %d)\n", offset, m_maincpu->pc(), m_state);

	if (m_maincpu->pc() == 0x3f04)
	{
		return 0xf0;      // any non-zero value works here
	}
	else if (m_maincpu->pc() == m_mcu_ipt_pc)
	{
		return m_mcu_ipt->read();
	}
	else
	{
		if (m_state == 0)
		{
			return 0;
		}
		else if (m_state == 1)
		{
			switch (offset & ~1)
			{
				case 0x40010: return 0x55;
				case 0x40012: return 0x93;
				case 0x40014: return 0x89;
				case 0x40016: return 0xa2;
				case 0x40018: return 0x31;
				case 0x4001a: return 0x75;
				case 0x4001c: return 0x97;
				case 0x4001e: return 0xb1;
				default: printf("State 1 unknown offset %x\n", offset); break;
			}
		}
		else if (m_state == 2)                      // 29c0: 53 ac 0c 2b a2 07 e6 be 31
		{
			u32 seed = m_seed;
			u32 magic = m_magic;

			magic = ( (((~(seed >> 16))       ^ (magic >> 1))        & 0x01) |
				(((~((seed >> 19) << 1))        ^ ((magic >> 5) << 1)) & 0x02) |
				(((~((seed >> 20) << 2))        ^ ((magic >> 3) << 2)) & 0x04) |
				(((~((seed >> 22) << 3))        ^ ((magic >> 6) << 3)) & 0x08) |
				(((~((seed >> 23) << 4))        ^   magic)             & 0x10) |
				(((~(((seed >> 16) >> 2) << 5)) ^ ((magic >> 2) << 5)) & 0x20) |
				(((~(((seed >> 16) >> 1) << 6)) ^ ((magic >> 7) << 6)) & 0x40) |
				(((~(((seed >> 16) >> 5) << 7)) ^  (magic << 7))       & 0x80));

			m_magic = magic;
			return magic;
		}
	}

	return 0;
}

void _39in1_state::cpld_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask == 0xffff)
	{
		m_seed = data<<16;
	}

	if (m_maincpu->pc() == 0x280c)
	{
		m_state = 1;
	}
	if (m_maincpu->pc() == 0x2874)
	{
		m_state = 2;
		m_magic = m_maincpu->space(AS_PROGRAM).read_byte(0xa02d4ff0);
	}
	else if (offset == 0xa)
	{
	}
#if 0
	else
	{
		printf("%08x: CPLD_W: %08x = %08x & %08x\n", m_maincpu->pc(), offset, data, mem_mask);
	}
#endif
}

u32 _39in1_state::prot_cheater_r()
{
	return 0x37;
}

void _39in1_state::base_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00400000, 0x007fffff).rom().region("data", 0);
	map(0x40000000, 0x47ffffff).m(m_pxa_periphs, FUNC(pxa255_periphs_device::map));
	map(0xa0000000, 0xa07fffff).ram().share("ram");
}

void _39in1_state::_39in1_map(address_map &map)
{
	base_map(map);

	map(0x04000000, 0x047fffff).rw(FUNC(_39in1_state::cpld_r), FUNC(_39in1_state::cpld_w));
	map(0xa0151648, 0xa015164b).r(FUNC(_39in1_state::prot_cheater_r));
}

void _39in1_state::iam_map(address_map &map)
{
	base_map(map);

	map(0x04800000, 0x04ffffff).ram(); // CPLD here?
	map(0xa0800000, 0xa3ffffff).ram(); // TODO: probably not really all RAM
}


static INPUT_PORTS_START( 39in1 )
	PORT_START("MCUIPT")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80000000, IP_ACTIVE_LOW )

//  The following dips apply to 39in1 and 48in1. 60in1 is the same but the last unused dipsw#4 is test mode off/on.

	PORT_START("DSW")      // 1x 4-position DIP switch labelled SW3
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW3:1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_flip_dip), 0)
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Display Mode" )            PORT_DIPLOCATION("SW3:2") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_res_dip), 0)
	PORT_DIPSETTING(    0x02, "VGA 31.5kHz" )
	PORT_DIPSETTING(    0x00, "CGA 15.75kHz" )
	PORT_DIPNAME( 0x04, 0x04, "High Score Saver" )        PORT_DIPLOCATION("SW3:3") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_hiscore_dip), 0)
	PORT_DIPSETTING(    0x04, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(_39in1_state::set_flip_dip)
{
	m_pxa_periphs->gpio_in<53>(BIT(m_dsw->read(), 0));
}

INPUT_CHANGED_MEMBER(_39in1_state::set_res_dip)
{
	m_pxa_periphs->gpio_in<54>(BIT(m_dsw->read(), 1));
}

INPUT_CHANGED_MEMBER(_39in1_state::set_hiscore_dip)
{
	m_pxa_periphs->gpio_in<56>(BIT(m_dsw->read(), 2));
}

void _39in1_state::decrypt(u8 xor00, u8 xor02, u8 xor04, u8 xor08, u8 xor10, u8 xor20, u8 xor40, u8 xor80, u8 bit7, u8 bit6, u8 bit5, u8 bit4, u8 bit3, u8 bit2, u8 bit1, u8 bit0)
{
	u8 *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x80000; i += 2)
	{
		if (i & 0x02) // only used by plutus
			rom[i] ^= xor02;
		if (i & 0x04) // only used by plutus
			rom[i] ^= xor04;
		if (i & 0x08)
			rom[i] ^= xor08;
		if (i & 0x10)
			rom[i] ^= xor10;
		if (i & 0x20)
			rom[i] ^= xor20;
		if (i & 0x40)
			rom[i] ^= xor40;
		if (i & 0x80) // only used by plutus
			rom[i] ^= xor80;

		rom[i] = bitswap<8>(rom[i] ^ xor00, bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0);
	}
}

void _39in1_state::further_decrypt(u8 xor400, u8 xor800, u8 xor1000, u8 xor2000, u8 xor4000, u8 xor8000) // later versions have more conditional XORs
{
	u8 *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x80000; i += 2)
	{
		if (i & 0x400)
			rom[i] ^= xor400; // always 0x00 in the available dumps
		if (i & 0x800)
			rom[i] ^= xor800;
		if (i & 0x1000)
			rom[i] ^= xor1000;
		if (i & 0x2000)
			rom[i] ^= xor2000;
		if (i & 0x4000)
			rom[i] ^= xor4000; // TODO: currently unverified if the games actually use this
		if (i & 0x8000)
			rom[i] ^= xor8000; // TODO: currently unverified if the games actually use this
		// TODO: 0x10000, 0x20000, 0x40000?
	}
}

void _39in1_state::init_39in1()  { decrypt(0xc0, 0x00, 0x00, 0x02, 0x40, 0x04, 0x80, 0x00, 7, 2, 5, 6, 0, 3, 1, 4); m_mcu_ipt_pc = 0xe3af4; } // good
void _39in1_state::init_4in1a()  { decrypt(0x25, 0x00, 0x00, 0x01, 0x80, 0x04, 0x40, 0x00, 6, 0, 2, 1, 7, 5, 4, 3); m_mcu_ipt_pc = 0x45814; } // good
void _39in1_state::init_4in1b()  { decrypt(0x43, 0x00, 0x00, 0x80, 0x04, 0x40, 0x08, 0x00, 2, 4, 0, 6, 7, 3, 1, 5); m_mcu_ipt_pc = 0x57628; } // good
void _39in1_state::init_19in1()  { decrypt(0x00, 0x00, 0x00, 0x04, 0x01, 0x80, 0x40, 0x00, 2, 1, 7, 4, 5, 0, 6, 3); further_decrypt(0x00, 0x01, 0x00, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // TODO: 0x4000, 0x8000, 0x10000, 0x20000, 0x40000 conditional XORs?
void _39in1_state::init_48in1()  { decrypt(0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x20, 0x00, 5, 3, 2, 1, 4, 6, 0, 7); further_decrypt(0x00, 0x01, 0x20, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // applies to both 48in1 and 48in1b, same main CPU ROM. TODO: see above
void _39in1_state::init_48in1a() { init_48in1(); m_mcu_ipt_pc = 0x00000; } // same encryption as 48in1
void _39in1_state::init_48in1c() { init_48in1(); m_mcu_ipt_pc = 0x00000; } // same encryption as 48in1
void _39in1_state::init_rodent() { init_4in1b(); /*m_mcu_ipt_pc = 0x?????;*/ } // same encryption as 4in1b, thus good, but doesn't boot because of different CPLD calls
void _39in1_state::init_60in1()  { decrypt(0x00, 0x00, 0x00, 0x40, 0x10, 0x80, 0x20, 0x00, 5, 1, 4, 2, 0, 7, 6, 3); further_decrypt(0x00, 0x01, 0x00, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // TODO: see 19in1

// I.A.M. slots
void _39in1_state::init_fruitwld()  { decrypt(0x0a, 0x00, 0x00, 0x20, 0x80, 0x00, 0x00, 0x00, 5, 1, 7, 4, 3, 2, 0, 6); /* further_decrypt(0x00, 0x00, 0x00, 0x00, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; */ } // TODO: >= 0x4000 XORs unverified
void _39in1_state::init_jumanji()   { decrypt(0x00, 0x00, 0x00, 0x02, 0x00, 0x40, 0x08, 0x00, 1, 0, 6, 2, 5, 3, 4, 7); further_decrypt(0x00, 0x08, 0x10, 0x40, 0x00, 0x00); /* m_mcu_ipt_pc = 0x00000; */ } // TODO: >= 0x4000 XORs unverified
void _39in1_state::init_jumanjia()  { decrypt(0x00, 0x00, 0x00, 0x40, 0x10, 0x08, 0x04, 0x00, 3, 5, 1, 4, 7, 6, 2, 0); /* further_decrypt(0x00, 0x00, 0x00, 0x00, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; */ } // TODO: >= 0x4000 XORs unverified
void _39in1_state::init_plutus()    { decrypt(0x00, 0x40, 0x08, 0x01, 0x00, 0x04, 0x80, 0x02, 6, 4, 0, 5, 7, 3, 2, 1); further_decrypt(0x00, 0x00, 0x10, 0x00, 0x00, 0x00); /* m_mcu_ipt_pc = 0x00000; */ } // TODO: >= 0x4000 XORs unverified
void _39in1_state::init_pokrwild()  { decrypt(0x20, 0x00, 0x00, 0x40, 0x08, 0x00, 0x00, 0x00, 6, 5, 3, 1, 0, 7, 2, 4); /* further_decrypt(0x00, 0x00, 0x00, 0x00, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; */ } // TODO: >= 0x4000 XORs unverified

void _39in1_state::base(machine_config &config)
{
	PXA255(config, m_maincpu, 200'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::base_map);

	EEPROM_93C66_16BIT(config, m_eeprom);
	m_eeprom->do_callback().set(m_pxa_periphs, FUNC(pxa255_periphs_device::gpio_in<5>));

	PXA255_PERIPHERALS(config, m_pxa_periphs, 200'000'000, m_maincpu);
	m_pxa_periphs->gpio_out<4>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::di_write));
	m_pxa_periphs->gpio_out<2>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::cs_write));
	m_pxa_periphs->gpio_out<3>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::clk_write));
}

void _39in1_state::_39in1(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::_39in1_map);
}

void _39in1_state::iam(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::iam_map);
}


ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66_eeprom.bin", 0x000, 0x200, CRC(a423a969) SHA1(4c68654c81e70367209b9f6c712564aae89a3122) )
ROM_END

ROM_START( 48in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "48_flash.u19", 0x000000, 0x400000, CRC(a975db44) SHA1(5be6520b2ba7728e9e2de3c62ae7c3b88b25172a) )  // CGC-NP205 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48_93c66.u32", 0x000, 0x200, CRC(cec06912) SHA1(2bc2e45602c5b1e8a3e031dd384e9f16be4e2ddb) )
ROM_END


ROM_START( 48in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ver302.u2",    0x000000, 0x080000, CRC(5ea25870) SHA1(66edc59a3d355bc3462e98d2062ada721c371af6) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1c )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "48in1_hph_ver308.u2", 0x000000, 0x080000, CRC(5d42beb0) SHA1(f21d1923b588cca1a6cd48a8ea6f3b5b996ebc1a) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 60in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver300.u8",   0x000000, 0x080000, CRC(6fba84c4) SHA1(28881e51227e94a80c8449d9c00a1a675f008d64) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, CRC(0cfed2a0) SHA1(9aac23f5267af56255e6f8aefade9f00bc106325) )  // CGC-NP206 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "60in1_eeprom.u32", 0x000, 0x200, CRC(54af5973) SHA1(30aca7790458f4be906f7fa7c74206e16d9fc36f) )
ROM_END

ROM_START( 4in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plz-v014_ver300.bin", 0x000000, 0x080000, CRC(775f101d) SHA1(8a299a67b487518ba2e2cb5334347b93f8640190) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // confirmed same flash rom as 39 in 1,   CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "4in1_eeprom.bin", 0x000, 0x200, CRC(df1724f7) SHA1(07814aee3622f4bb8bada938f2a93fae791d6e31) )
ROM_END

ROM_START( 4in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pzv001-4.bin", 0x000000, 0x080000, CRC(7679a95f) SHA1(56c20fa7d086560b76477b42208cb43d42adba41) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66-4.bin", 0x000, 0x200, CRC(84d1c26a) SHA1(de823adddf949bf77d8478762720fe0b56fba8ea) )
ROM_END

// 19-in-1 is visibly different hardware, extent of differences unknown due to lack of quality pictures/scans
// also, there is a bootleg of the 19-in-1 which may have less or different protection
ROM_START( 19in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "19in1.u8",    0x000000, 0x080000, CRC(87b0506c) SHA1(c43ae4b403864a28e56370685572fa02e7572e66) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // assuming same flash rom, CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "19in1_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( rodent )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "exterminator.u2", 0x00000, 0x80000, CRC(23c1d21f) SHA1(349565b0f0a015196827707cabb8d9ce6560d2cc) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m29w160db.u19", 0x000000, 0x200000, CRC(665ee79c) SHA1(35896b97378e8cd78e99d4527b9dc7392e545e17) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66.u32", 0x000, 0x200, CRC(c311c7bc) SHA1(8328002b7f6a8b7a3ffca079b7960bc990211d7b) )
ROM_END


// The following are dumps from I.A.M. slot machines

ROM_START( fruitwld ) // PCB451 - FRUIT WORLD FP101 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fruit world v111.u2", 0x00000, 0x80000, CRC(44092be5) SHA1(a579455c4581fc2f6be37979d651f3f685353e8e) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(8cc9799a) SHA1(5bec178d11c722e26bf380c19d99118e7223bcd7) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF, fruit-FPv101 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(11245518) SHA1(6363568facbe12f7be95e994c551815e7c3682f4) )
ROM_END

ROM_START( fruitwlda ) // PCB383 - FRUIT WORLD FP101 sticker on PCB outside the lid, FRUIT WORLD V102 on another sticker on big FPGA
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fruit world v110.u2", 0x00000, 0x80000, CRC(d81bdd3c) SHA1(79ec9d12bb94537655778ac1138d3611bda9179e) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(8cc9799a) SHA1(5bec178d11c722e26bf380c19d99118e7223bcd7) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF, fruit-FPv101 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(9e2e676e) SHA1(c3bda9c63118e9efb41445d941be44d4499b694f) )
ROM_END

ROM_START( jumanji ) // PCB383 - CHZ FP100 sticker on RAM under the lid. Dump was presented as Jumanji but has CHZ both on stickers and in ROM strings.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u2", 0x00000, 0x80000, CRC(45bd43c7) SHA1(0da61fc1f5f17b2b9531ccfc69495a61aa272efd) ) // no label

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, NO_DUMP )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66.u32", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( jumanjia ) // PCB383 - Jumanji FP101 sticker on RAM under the lid.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jumanji_113_fp101.u2", 0x00000, 0x80000, CRC(40a66c50) SHA1(8909db087a8527af8ce229b03e2f7f16160db6f0) )

	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(4cd694fe) SHA1(723c0ed2af994cc584654dbf0d779e1c90827c7b) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66.u32", 0x000, 0x200, CRC(60930a27) SHA1(9222b23d64d85f664037ba180f79045c108fed9c) )
ROM_END

ROM_START( plutus ) // PCB451 - PLUTUS FP100 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plutus v100.u2", 0x00000, 0x80000, CRC(3ac49895) SHA1(6de3dcac42afc4d9f927c9c9accf592b3d974fd3) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(352387e7) SHA1(24e2d98681791f42033a58721b5af9cc6a04ebe4) ) // PLUTUS-FP100 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(625eb014) SHA1(f1fb0777ce8a12ee09d882cd843c07daea14145a) )
ROM_END

ROM_START( pokrwild ) // PCB451 - POKER'S WILD FP102 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pokers wild v117.u2", 0x00000, 0x80000, CRC(96e18540) SHA1(b8fbf0a78a496e4ebea3e4603f4e3a52823c1f31) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(824fd188) SHA1(38517a78e853a600abcb6256ff77482a250c6ee6) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF, PKWILD-FP103 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(27c7a209) SHA1(4d8e0ab18adb882362d800e2c247b3e27e6949e1) )
ROM_END

} // anonymous namespace


GAME(2004, 4in1a,     39in1,    base,   39in1, _39in1_state, init_4in1a,    ROT90, "bootleg", "4 in 1 MAME bootleg (ver 3.00, PLZ-V014)",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 4in1b,     39in1,    base,   39in1, _39in1_state, init_4in1b,    ROT90, "bootleg", "4 in 1 MAME bootleg (PLZ-V001)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 19in1,     39in1,    base,   39in1, _39in1_state, init_19in1,    ROT90, "bootleg", "19 in 1 MAME bootleg (BAR-V000)",                      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 39in1,     0,        _39in1, 39in1, _39in1_state, init_39in1,    ROT90, "bootleg", "39 in 1 MAME bootleg (GNO-V000)",                      MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1,     39in1,    base,   39in1, _39in1_state, init_48in1,    ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1b,    39in1,    base,   39in1, _39in1_state, init_48in1,    ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000, alt flash)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1a,    39in1,    base,   39in1, _39in1_state, init_48in1a,   ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.02, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1c,    39in1,    base,   39in1, _39in1_state, init_48in1c,   ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.08, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 60in1,     39in1,    base,   39in1, _39in1_state, init_60in1,    ROT90, "bootleg", "60 in 1 MAME bootleg (ver 3.00, ICD-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2005, rodent,    0,        base,   39in1, _39in1_state, init_rodent,   ROT0,  "The Game Room", "Rodent Exterminator",                            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// I.A.M. slots. Versions are taken from program ROM stickers or ROM strings, where available
GAME(2008, fruitwld,  0,        iam,    39in1, _39in1_state, init_fruitwld, ROT0,  "I.A.M.",  "Fruit World (V111)",                                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // FRUIT_V111.BIN 2008-04-30 15:59:21
GAME(2007, fruitwlda, fruitwld, iam,    39in1, _39in1_state, init_fruitwld, ROT0,  "I.A.M.",  "Fruit World (V110)",                                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // FRUIT_V110.BIN 2007-07-26 13:46:30
GAME(2007, jumanji,   0,        iam,    39in1, _39in1_state, init_jumanji,  ROT0,  "I.A.M.",  "Jumanji (V502)",                                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // CHZ_V502.BIN 2007-07-26 13:49:35 in clear text at the end of the main CPU ROM
GAME(2007, jumanjia,  jumanji,  iam,    39in1, _39in1_state, init_jumanjia, ROT0,  "I.A.M.",  "Jumanji (V113)",                                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // JUMANJI_V113.BIN 2007-07-25 10:54:33
GAME(200?, plutus,    0,        iam,    39in1, _39in1_state, init_plutus,   ROT0,  "I.A.M.",  "Plutus (V100)",                                        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // no string
GAME(200?, pokrwild,  0,        iam,    39in1, _39in1_state, init_pokrwild, ROT0,  "I.A.M.",  "Poker's Wild (V117)",                                  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // no string
