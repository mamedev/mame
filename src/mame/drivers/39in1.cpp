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
 * TODO:
 *   - PXA255 peripherals
 *   - 4in1a and 4in1b are very similar to 39in1, currently boot but stuck at
 *     'Hardware Check' with an error
 *   - rodent should be correctly decrypted but expects something different
       from the CPLD (probably)
 *   - 19in1, 48in1, 48in1a, 48in1b, 48in1c,60in1 have more conditional XORs,
 *     encryption isn't completely beaten yet
 *
 * 39in1 notes:
 * The actual PCB just normally boots up to the game, whereas in MAME it
 * defaults to test mode and checks the rom, then jumps out to the game
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
#include "cpu/arm7/arm7core.h"
#include "machine/eepromser.h"
#include "machine/pxa255.h"

class _39in1_state : public driver_device
{
public:
	_39in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pxa_periphs(*this, "pxa_periphs")
		, m_ram(*this, "ram")
		, m_eeprom(*this, "eeprom")
		, m_maincpu(*this, "maincpu")
	{ }

	void _39in1(machine_config &config);

	void init_4in1a();
	void init_4in1b();
	void init_19in1();
	void init_39in1();
	void init_48in1();
	void init_48in1a();
	void init_48in1c();
	void init_60in1();
	void init_rodent();

private:
	uint32_t m_seed;
	uint32_t m_magic;
	uint32_t m_state;
	uint32_t m_mcu_ipt_pc;

	void driver_init() override;

	required_device<pxa255_periphs_device> m_pxa_periphs;
	required_shared_ptr<uint32_t> m_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	uint32_t eeprom_r();
	void eeprom_w(uint32_t data, uint32_t mem_mask = ~0);

	uint32_t cpld_r(offs_t offset);
	void cpld_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t prot_cheater_r();
	required_device<cpu_device> m_maincpu;
	void _39in1_map(address_map &map);

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );

	void decrypt(uint8_t xor00, uint8_t xor08, uint8_t xor10, uint8_t xor20, uint8_t xor40, uint8_t bit7, uint8_t bit6, uint8_t bit5, uint8_t bit4, uint8_t bit3, uint8_t bit2, uint8_t bit1, uint8_t bit0);
	void further_decrypt(uint8_t xor400, uint8_t xor800, uint8_t xor1000, uint8_t xor2000, uint8_t xor4000, uint8_t xor8000);
};


#define VERBOSE_LEVEL (0)

inline void ATTR_PRINTF(3,4) _39in1_state::verboselog(int n_level, const char *s_fmt, ... )
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%s: %s", machine().describe_context(), buf);
	}
}

uint32_t _39in1_state::eeprom_r()
{
	return (m_eeprom->do_read() << 5) | (1 << 1); // Must be on.  Probably a DIP switch.
}

void _39in1_state::eeprom_w(uint32_t data, uint32_t mem_mask)
{
	if (BIT(mem_mask, 2))
		m_eeprom->cs_write(ASSERT_LINE);
	if (BIT(mem_mask, 3))
		m_eeprom->clk_write(BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
	if (BIT(mem_mask, 4))
		m_eeprom->di_write(BIT(data, 4));
}

uint32_t _39in1_state::cpld_r(offs_t offset)
{
	// if (m_maincpu->pc() != m_mcu_ipt_pc) printf("CPLD read @ %x (PC %x state %d)\n", offset, m_maincpu->pc(), m_state);

	if (m_maincpu->pc() == 0x3f04)
	{
		return 0xf0;      // any non-zero value works here
	}
	else if (m_maincpu->pc() == m_mcu_ipt_pc)
	{
		return ioport("MCUIPT")->read();
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
			uint32_t seed = m_seed;
			uint32_t magic = m_magic;

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

void _39in1_state::cpld_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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

uint32_t _39in1_state::prot_cheater_r()
{
	return 0x37;
}

void _39in1_state::driver_init()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_read_handler (0xa0151648, 0xa015164b, read32smo_delegate(*this, FUNC(_39in1_state::prot_cheater_r)));
}

void _39in1_state::_39in1_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00400000, 0x005fffff).rom().region("data", 0);
	map(0x04000000, 0x047fffff).rw(FUNC(_39in1_state::cpld_r), FUNC(_39in1_state::cpld_w));
	map(0x40000000, 0x400002ff).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::dma_r), FUNC(pxa255_periphs_device::dma_w));
	map(0x40400000, 0x40400083).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::i2s_r), FUNC(pxa255_periphs_device::i2s_w));
	map(0x40a00000, 0x40a0001f).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::ostimer_r), FUNC(pxa255_periphs_device::ostimer_w));
	map(0x40d00000, 0x40d00017).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::intc_r), FUNC(pxa255_periphs_device::intc_w));
	map(0x40e00000, 0x40e0006b).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::gpio_r), FUNC(pxa255_periphs_device::gpio_w));
	map(0x44000000, 0x4400021f).rw(m_pxa_periphs, FUNC(pxa255_periphs_device::lcd_r), FUNC(pxa255_periphs_device::lcd_w));
	map(0xa0000000, 0xa07fffff).ram().share("ram");
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
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Display Mode" )            PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "VGA 31.5kHz" )
	PORT_DIPSETTING(    0x00, "CGA 15.75kHz" )
	PORT_DIPNAME( 0x04, 0x04, "High Score Saver" )        PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void _39in1_state::decrypt(uint8_t xor00, uint8_t xor08, uint8_t xor10, uint8_t xor20, uint8_t xor40, uint8_t bit7, uint8_t bit6, uint8_t bit5, uint8_t bit4, uint8_t bit3, uint8_t bit2, uint8_t bit1, uint8_t bit0)
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x80000; i += 2)
	{
		if (i & 0x08)
			rom[i] ^= xor08;
		if (i & 0x10)
			rom[i] ^= xor10;
		if (i & 0x20)
			rom[i] ^= xor20;
		if (i & 0x40)
			rom[i] ^= xor40;

		rom[i] = bitswap<8>(rom[i] ^ xor00, bit7, bit6, bit5, bit4, bit3, bit2, bit1, bit0);
	}
}

void _39in1_state::further_decrypt(uint8_t xor400, uint8_t xor800, uint8_t xor1000, uint8_t xor2000, uint8_t xor4000, uint8_t xor8000) // later versions have more conditional XORs
{
	uint8_t *rom = memregion("maincpu")->base();

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

	/*{
	    char filename[256];
	    sprintf(filename,"p_decrypted_%s", machine().system().name);
	    FILE *fp = fopen(filename, "w+b");
	    if (fp)
	    {
	    fwrite(rom, 0x80000, 1, fp);
	    fclose(fp);
	    }
	}*/
}

void _39in1_state::init_39in1()  { driver_init(); decrypt(0xc0, 0x02, 0x40, 0x04, 0x80, 7, 2, 5, 6, 0, 3, 1, 4); m_mcu_ipt_pc = 0xe3af4; } // good
void _39in1_state::init_4in1a()  { driver_init(); decrypt(0x25, 0x01, 0x80, 0x04, 0x40, 6, 0, 2, 1, 7, 5, 4, 3); m_mcu_ipt_pc = 0x45814; } // good
void _39in1_state::init_4in1b()  { driver_init(); decrypt(0x43, 0x80, 0x04, 0x40, 0x08, 2, 4, 0, 6, 7, 3, 1, 5); m_mcu_ipt_pc = 0x57628; } // good
void _39in1_state::init_19in1()  { driver_init(); decrypt(0x00, 0x04, 0x01, 0x80, 0x40, 2, 1, 7, 4, 5, 0, 6, 3); further_decrypt(0x00, 0x01, 0x00, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // TODO: 0x4000, 0x8000, 0x10000, 0x20000, 0x40000 conditional XORs?
void _39in1_state::init_48in1()  { driver_init(); decrypt(0x00, 0x01, 0x40, 0x00, 0x20, 5, 3, 2, 1, 4, 6, 0, 7); further_decrypt(0x00, 0x01, 0x20, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // applies to both 48in1 and 48in1b, same main CPU ROM. TODO: see above
void _39in1_state::init_48in1a() { init_48in1(); m_mcu_ipt_pc = 0x00000; } // same encryption as 48in1
void _39in1_state::init_48in1c() { init_48in1(); m_mcu_ipt_pc = 0x00000; } // same encryption as 48in1
void _39in1_state::init_rodent() { init_4in1b(); /*m_mcu_ipt_pc = 0x?????;*/ } // same encryption as 4in1b, thus good, but doesn't boot because of different CPLD calls
void _39in1_state::init_60in1()  { driver_init(); decrypt(0x00, 0x40, 0x10, 0x80, 0x20, 5, 1, 4, 2, 0, 7, 6, 3); further_decrypt(0x00, 0x01, 0x00, 0x10, 0x00, 0x00); m_mcu_ipt_pc = 0x00000; } // TODO: see 19in1

void _39in1_state::_39in1(machine_config &config)
{
	PXA255(config, m_maincpu, 200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::_39in1_map);

	EEPROM_93C66_16BIT(config, "eeprom");

	PXA255_PERIPHERALS(config, m_pxa_periphs, 200000000, m_maincpu);
	m_pxa_periphs->gpio0_write().set(FUNC(_39in1_state::eeprom_w));
	m_pxa_periphs->gpio0_read().set(FUNC(_39in1_state::eeprom_r));
	m_pxa_periphs->gpio1_read().set_ioport("DSW").lshift(21);
}

ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66_eeprom.bin", 0x000, 0x200, CRC(a423a969) SHA1(4c68654c81e70367209b9f6c712564aae89a3122) )
ROM_END

ROM_START( 48in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "48_flash.u19", 0x000000, 0x400000, CRC(a975db44) SHA1(5be6520b2ba7728e9e2de3c62ae7c3b88b25172a) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48_93c66.u32", 0x000, 0x200, CRC(cec06912) SHA1(2bc2e45602c5b1e8a3e031dd384e9f16be4e2ddb) )
ROM_END


ROM_START( 48in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ver302.u2",    0x000000, 0x080000, CRC(5ea25870) SHA1(66edc59a3d355bc3462e98d2062ada721c371af6) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1c )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "48in1_hph_ver308.u2", 0x000000, 0x080000, CRC(5d42beb0) SHA1(f21d1923b588cca1a6cd48a8ea6f3b5b996ebc1a) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 60in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver300.u8",   0x000000, 0x080000, CRC(6fba84c4) SHA1(28881e51227e94a80c8449d9c00a1a675f008d64) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, CRC(0cfed2a0) SHA1(9aac23f5267af56255e6f8aefade9f00bc106325) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "60in1_eeprom.u32", 0x000, 0x200, CRC(54af5973) SHA1(30aca7790458f4be906f7fa7c74206e16d9fc36f) )
ROM_END

ROM_START( 4in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plz-v014_ver300.bin", 0x000000, 0x080000, CRC(775f101d) SHA1(8a299a67b487518ba2e2cb5334347b93f8640190) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // confirmed same flash rom as 39 in 1

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "4in1_eeprom.bin", 0x000, 0x200, CRC(df1724f7) SHA1(07814aee3622f4bb8bada938f2a93fae791d6e31) )
ROM_END

ROM_START( 4in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pzv001-4.bin", 0x000000, 0x080000, CRC(7679a95f) SHA1(56c20fa7d086560b76477b42208cb43d42adba41) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

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
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // assuming same flash rom

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "19in1_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( rodent )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "exterminator.u2", 0x00000, 0x80000, CRC(23c1d21f) SHA1(349565b0f0a015196827707cabb8d9ce6560d2cc) )

	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "m29w160db.u19", 0x000000, 0x200000, CRC(665ee79c) SHA1(35896b97378e8cd78e99d4527b9dc7392e545e17) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66.u32", 0x000, 0x200, CRC(c311c7bc) SHA1(8328002b7f6a8b7a3ffca079b7960bc990211d7b) )
ROM_END

GAME(2004, 4in1a,  39in1, _39in1, 39in1, _39in1_state, init_4in1a,  ROT90, "bootleg", "4 in 1 MAME bootleg (ver 3.00, PLZ-V014)",             MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 4in1b,  39in1, _39in1, 39in1, _39in1_state, init_4in1b,  ROT90, "bootleg", "4 in 1 MAME bootleg (PLZ-V001)",                       MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 19in1,  39in1, _39in1, 39in1, _39in1_state, init_19in1,  ROT90, "bootleg", "19 in 1 MAME bootleg (BAR-V000)",                      MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 39in1,  0,     _39in1, 39in1, _39in1_state, init_39in1,  ROT90, "bootleg", "39 in 1 MAME bootleg (GNO-V000)",                      MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1,  39in1, _39in1, 39in1, _39in1_state, init_48in1,  ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1b, 39in1, _39in1, 39in1, _39in1_state, init_48in1,  ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000, alt flash)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1a, 39in1, _39in1, 39in1, _39in1_state, init_48in1a, ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.02, HPH-V000)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1c, 39in1, _39in1, 39in1, _39in1_state, init_48in1c, ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.08, HPH-V000)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 60in1,  39in1, _39in1, 39in1, _39in1_state, init_60in1,  ROT90, "bootleg", "60 in 1 MAME bootleg (ver 3.00, ICD-V000)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2005, rodent, 0,     _39in1, 39in1, _39in1_state, init_rodent, ROT0,  "The Game Room", "Rodent Exterminator",                            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
