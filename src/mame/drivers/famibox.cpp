// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

  Nintendo FamicomBox
  Driver by Mariusz Wojcieszek
  Thanks for Kevin Horton for hardware docs

In 1986, Nintendo Co., Ltd. began distributing the FamicomBox (SSS-CDS) which
allowed gamers to test out and play up to 15 different Famicom games which
could be installed in the unit itself.  Like it's sequel, the Super Famicom Box,
it was found in hotels often set up to accept 100 yen coins giving you an
adjusted amount of gametime (10 or 20 minutes - DIP selectable).  Sharp also
produced a version of the FamicomBox called FamicomStation which was more
of a consumer (non-coin accept) unit.  Besides that, the equipment and
capabilities between the two are thought to be virtually identical - save the
case style and cartridge/case color:  (FamicomBox = Black, FamicomStation = Gray).



Specific Hardware information
-----------------------------
The unit had 3 NES controller ports - the first 2 identical to the standard NES.
The third, intended to be used with a NES Zapper light gun, has pin 4 (D0) disconnected
(meaning it can't be used with a regular joypad) and shares its D3 and D4 lines with
port 2 (meaning it will appear like a conventional zapper in port 2). What is normally
GND (pin 1) of port 3 is controllable, in order to enable/disable the zapper.

Cartridges are shaped and appear to be similar to NES 72-pin cartridges. They have their
own unique PCBs, use EPROMS, and often (always?) contain the same data as an existing
NES/FC cart. It was made to play only the games specifically released for it.

- There a special lockout chip, but the lockout chip connects to different pins on
  a FamicomBox cartridge's connector than a regular cart
- The lockout chips in the system and the games have to 'talk' before the system will
  load any games into its menu.

Here's a list of some of the games known to have come with the FamicomBox:
1943; Baseball; Bomber Man; Devil World; Donkey Kong; Donkey Kong Jr.; Duck Hunt;
Excitebike; F1 Race; Fighting Golf; Golf; Gradius; Hogan's Alley; Ice Climbers;
Ice Hockey; Knight Rider; Makaimura: Ghosts 'n Goblins; McKids; Mah-Jong; Mario Bros.;
Mike Tyson's Punch-Out!!; Ninja Ryukenden; Operation Wolf (?); Punch-Out!!; Rockman;
Rygar; Senjou no Ookami; Soccer League Winner's Cup; Super Chinese 2; Super Mario Bros;
Tag Team Pro Wrestling; Takahashi Meijin no Boukenjima; Tennis; Twin Bee;
Volleyball; Wild Gunman; Wrecking Crew.

Here's a list of some of the games known to have come with the FamicomStation:
1943; Baseball; Donkey Kong; Duck Hunt; F1 Race; Golf; Kame no Ongaeshi:
Urashima Densetsu; Mah-Jong; Mario Bros.; Night Raider; Senjou no Ookami;
Soccer League Winner's Cup; Super Chinese 2; Super Mario Bros; Tag Team Pro Wrestling;
Takahashi Meijin no Boukenjima; Tennis; Wild Gunman; Wrecking Crew.

FamicomBox menu code maintains internal database of games (rom checksums and game names
in ASCII). When checking game cartridges, it scans roms and tries to find matching game
in its internal database. Additionally, games having standard Nintendo header are accepted too.
Current selection of games in driver is based on menu internal database.

Notes/ToDo:
- coin insertion sound is not emulated
- coin beep (before time out) is not emulated
- screen modulation (before time out) is not emulated
***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "cpu/m6502/n2a03.h"
#include "bus/nes_ctrl/ctrl.h"
#include "debugger.h"
#include "screen.h"
#include "speaker.h"


namespace {

class famibox_state : public driver_device
{
public:
	famibox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppu(*this, "ppu")
		, m_screen(*this, "screen")
		, m_ctrl(*this, "ctrl%u", 1) { }

	void famibox(machine_config &config);

	void init_famibox();
	void init_famistat();

	DECLARE_READ_LINE_MEMBER(coin_r);
	DECLARE_INPUT_CHANGED_MEMBER(famibox_keyswitch_changed);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<n2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_device<screen_device> m_screen;
	optional_device_array<nes_control_port_device, 3> m_ctrl;

	std::unique_ptr<uint8_t[]> m_nt_ram;
	uint8_t* m_nt_page[4];
	uint8_t m_mirroring;

	uint8_t       m_exception_mask;
	uint8_t       m_exception_cause;

	emu_timer*    m_attract_timer;
	uint8_t       m_attract_timer_period;

	uint32_t      m_coins;
	uint8_t       m_zapper_enable;

	emu_timer*    m_gameplay_timer;
	uint8_t       m_money_reg;

	void famibox_nt_w(offs_t offset, uint8_t data);
	uint8_t famibox_nt_r(offs_t offset);
	void set_mirroring(int mirroring);
	void sprite_dma_w(address_space &space, uint8_t data);
	uint8_t famibox_IN0_r();
	uint8_t famibox_IN1_r();
	void famibox_IN0_w(uint8_t data);
	uint8_t famibox_system_r(offs_t offset);
	void famibox_system_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(famicombox_attract_timer_callback);
	TIMER_CALLBACK_MEMBER(famicombox_gameplay_timer_callback);
	void famicombox_bankswitch(uint8_t bank);
	void famicombox_reset();
	void famibox_map(address_map &map);
	void famibox_ppu_map(address_map &map);
};

/******************************************************

   PPU external bus interface

*******************************************************/

void famibox_state::set_mirroring(int mirroring)
{
	switch (mirroring)
	{
		case PPU_MIRROR_LOW:
			m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram.get();
			break;
		case PPU_MIRROR_HIGH:
			m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram.get() + 0x400;
			break;
		case PPU_MIRROR_HORZ:
			m_nt_page[0] = m_nt_ram.get();
			m_nt_page[1] = m_nt_ram.get();
			m_nt_page[2] = m_nt_ram.get() + 0x400;
			m_nt_page[3] = m_nt_ram.get() + 0x400;
			break;
		case PPU_MIRROR_VERT:
			m_nt_page[0] = m_nt_ram.get();
			m_nt_page[1] = m_nt_ram.get() + 0x400;
			m_nt_page[2] = m_nt_ram.get();
			m_nt_page[3] = m_nt_ram.get() + 0x400;
			break;
	}
}

void famibox_state::famibox_nt_w(offs_t offset, uint8_t data)
{
	int page = BIT(offset, 10, 2);
	m_nt_page[page][offset & 0x3ff] = data;
}

uint8_t famibox_state::famibox_nt_r(offs_t offset)
{
	int page = BIT(offset, 10, 2);
	return m_nt_page[page][offset & 0x3ff];
}

/******************************************************

   NES interface

*******************************************************/

void famibox_state::sprite_dma_w(address_space &space, uint8_t data)
{
	int source = data & 7;
	m_ppu->spriteram_dma(space, source);
}


/******************************************************

   Inputs

*******************************************************/

uint8_t famibox_state::famibox_IN0_r()
{
	uint8_t ret = 0x40;
	ret |= m_ctrl[0]->read_bit0();
	ret |= m_ctrl[0]->read_bit34();
	return ret;
}

uint8_t famibox_state::famibox_IN1_r()
{
	uint8_t ret = 0x40;

	ret |= m_ctrl[1]->read_bit0();
	ret |= m_ctrl[1]->read_bit34();

	// only read port 3 if its pin 1 (normally GND) is held low
	if (m_zapper_enable)
		// 3rd port is intended for a zapper and its D0 pin is NOT connected
		ret |= m_ctrl[2]->read_bit34();

	return ret;
}

void famibox_state::famibox_IN0_w(uint8_t data)
{
	for (int i = 0; i < 3; i++)
		m_ctrl[i]->write(data);
}

/******************************************************

   System

*******************************************************/

void famibox_state::famicombox_bankswitch(uint8_t bank)
{
	struct
	{
		uint8_t bank;
		const char* memory_region;
		offs_t bank1_offset;
		offs_t bank2_offset;
		offs_t ppubank_offset;
		uint8_t mirroring;
	} famicombox_banks[] =
	{
		{ 0x11, "baseball", 0, 0,      0x4000, PPU_MIRROR_HORZ },
		{ 0x12, "bombman",  0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x13, "dkong",    0, 0,      0x4000, PPU_MIRROR_HORZ },
		{ 0x14, "duckhunt", 0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x15, "excitebk", 0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x26, "f1race",   0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x27, "hogan",    0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x28, "golf",     0, 0,      0x4000, PPU_MIRROR_HORZ },
		{ 0x29, "icehocky", 0, 0x4000, 0x8000, PPU_MIRROR_VERT },
		{ 0x2a, "mahjong",  0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x3b, "mario",    0, 0,      0x4000, PPU_MIRROR_HORZ },
		{ 0x3c, "smb",      0, 0x4000, 0x8000, PPU_MIRROR_VERT },
		{ 0x3d, "tennis",   0, 0,      0x4000, PPU_MIRROR_HORZ },
		{ 0x3e, "wildgunm", 0, 0,      0x4000, PPU_MIRROR_VERT },
		{ 0x3f, "wrecking", 0, 0x4000, 0x8000, PPU_MIRROR_HORZ },
		{ 0x00, "menu",     0, 0x4000, 0x8000, 0 }
	};


	for (auto & famicombox_bank : famicombox_banks)
	{
		if ( bank == famicombox_bank.bank ||
				famicombox_bank.bank == 0 )
		{
			membank("cpubank1")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.bank1_offset);
			membank("cpubank2")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.bank2_offset);
			membank("ppubank1")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.ppubank_offset);
			set_mirroring(famicombox_bank.bank ? famicombox_bank.mirroring : m_mirroring);
			break;
		}
	}
}

void famibox_state::famicombox_reset()
{
	famicombox_bankswitch(0);
	m_maincpu->reset();
}

TIMER_CALLBACK_MEMBER(famibox_state::famicombox_attract_timer_callback)
{
	m_attract_timer->adjust(attotime::never, 0, attotime::never);
	if ( BIT(m_exception_mask,1) )
	{
		m_exception_cause &= ~0x02;
		famicombox_reset();
	}
}

TIMER_CALLBACK_MEMBER(famibox_state::famicombox_gameplay_timer_callback)
{
	if (m_coins > 0)
		m_coins--;

	if (m_coins == 0)
	{
		m_gameplay_timer->adjust(attotime::never, 0, attotime::never);
		if ( BIT(m_exception_mask,4) )
		{
			m_exception_cause &= ~0x10;
			famicombox_reset();
		}
	}
}

uint8_t famibox_state::famibox_system_r(offs_t offset)
{
	switch( offset & 0x07 )
	{
		case 0: // device which caused exception
			{
				uint8_t ret = m_exception_cause;
				m_exception_cause = 0xff;
				return ret;
			}
		case 2:
			return ioport("DSW")->read();
		case 3:
			return ioport("KEYSWITCH")->read();
		case 7:
			// this assumes zapper is connected? should be high if and only if zapper is connected and disabled?
			return 0x02 | (m_zapper_enable ^ 0x04);
		default:
			logerror("%s: Unhandled famibox_system_r(%x)\n", machine().describe_context(), offset );
			return 0;
	}
}

void famibox_state::famibox_system_w(offs_t offset, uint8_t data)
{
	switch( offset & 0x07 )
	{
		case 0:
			logerror("%s: Interrupt enable\n", machine().describe_context());
			logerror("6.82Hz interrupt source (0 = enable): %d\n", BIT(data,0));
			logerror("8 bit timer expiration @ 5003W (1 = enable): %d\n", BIT(data,1));
			logerror("controller reads (1 = enable): %d\n", BIT(data,2));
			logerror("keyswitch rotation (1 = enable): %d\n", BIT(data,3));
			logerror("money insertion (1 = enable): %d\n", BIT(data,4));
			logerror("reset button (1 = enable): %d\n", BIT(data,5));
			logerror("\"CATV connector\" pin 4 detection (1 = enable): %d\n", BIT(data,7));
			m_exception_mask = data;
			if ( BIT(m_exception_mask,1) && ( m_attract_timer_period != 0 ) )
			{
				if (m_attract_timer->start() != attotime::never)
				{
					m_attract_timer->adjust(attotime::from_seconds((int32_t)((double)1.0/6.8274*m_attract_timer_period)), 0, attotime::never);
				}
			}
			break;
		case 1:
			m_money_reg = data;
			logerror("%s: Money handling register: %02x\n", machine().describe_context(), data);
			break;
		case 2:
			logerror("%s: LED & memory protect register: %02x\n", machine().describe_context(), data);
			break;
		case 3:
			logerror("%s: 8 bit down counter, for attract mode timing: %02x\n", machine().describe_context(), data);
			m_attract_timer_period = data;
			if ( BIT(m_exception_mask,1) && ( data != 0 ) )
			{
				m_attract_timer->adjust(attotime::from_hz(6.8274/m_attract_timer_period), 0, attotime::never);
			}
			break;
		case 4:
			logerror("%s: bankswitch %x\n", machine().describe_context(), data );
			famicombox_bankswitch(data & 0x3f);
			break;
		case 5:
			logerror("%s: misc control register: %02x\n", machine().describe_context(), data);
			m_zapper_enable = data & 0x04;
			break;
		default:
			logerror("%s: Unhandled famibox_system_w(%x,%02x)\n", machine().describe_context(), offset, data );
	}
}

/******************************************************

   Memory map

*******************************************************/

void famibox_state::famibox_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(FUNC(famibox_state::sprite_dma_w));
	map(0x4016, 0x4016).rw(FUNC(famibox_state::famibox_IN0_r), FUNC(famibox_state::famibox_IN0_w)); // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(famibox_state::famibox_IN1_r));     // IN1 - input port 2 / PSG second control register
	map(0x5000, 0x5fff).rw(FUNC(famibox_state::famibox_system_r), FUNC(famibox_state::famibox_system_w));
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xbfff).bankr("cpubank1");
	map(0xc000, 0xffff).bankr("cpubank2");
}

void famibox_state::famibox_ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr("ppubank1");
	map(0x2000, 0x3eff).rw(FUNC(famibox_state::famibox_nt_r), FUNC(famibox_state::famibox_nt_w));
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

/******************************************************

   Inputs

*******************************************************/

INPUT_CHANGED_MEMBER(famibox_state::famibox_keyswitch_changed)
{
	if ( BIT(m_exception_mask, 3) )
	{
		m_exception_cause &= ~0x08;
		famicombox_reset();
	}
}

INPUT_CHANGED_MEMBER(famibox_state::coin_inserted)
{
	if ( newval )
	{
		m_coins++;
		if (m_attract_timer->start() != attotime::never)
		{
			m_gameplay_timer->adjust(attotime::from_seconds(60*(m_money_reg == 0x22 ? 20 : 10)), 0, attotime::never);
		}

		if ( BIT(m_exception_mask,4) && (m_coins == 1) )
		{
			m_exception_cause &= ~0x10;
			famicombox_reset();
		}
	}
}

READ_LINE_MEMBER(famibox_state::coin_r)
{
	return m_coins > 0;
}

static INPUT_PORTS_START( famibox )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Self Test" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin timeout period" )
	PORT_DIPSETTING(    0x00, "10 min" )
	PORT_DIPSETTING(    0x02, "20 min" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Famicombox menu time" )
	PORT_DIPSETTING(    0x00, "5 sec" )
	PORT_DIPSETTING(    0x08, "10 sec" )
	PORT_DIPNAME( 0x30, 0x00, "Attract time" )
	PORT_DIPSETTING(    0x30, "5 sec" )
	PORT_DIPSETTING(    0x00, "10 sec" )
	PORT_DIPSETTING(    0x10, "15 sec" )
	PORT_DIPSETTING(    0x20, "20 sec" )
	PORT_DIPNAME( 0xc0, 0x80, "Operational mode" )
	PORT_DIPSETTING(    0x00, "KEY MODE" )
	PORT_DIPSETTING(    0x40, "CATV MODE" )
	PORT_DIPSETTING(    0x80, "COIN MODE" )
	PORT_DIPSETTING(    0xc0, "FREEPLAY" )

	PORT_START("KEYSWITCH")
	PORT_DIPNAME( 0x3f, 0x01, "Key switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, famibox_state, famibox_keyswitch_changed, 0)
	PORT_DIPSETTING(    0x20, "Game Count" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x08, "Self Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Unused ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(famibox_state, coin_r)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, famibox_state, coin_inserted, 0)
INPUT_PORTS_END

/******************************************************

   PPU/Video interface

*******************************************************/

void famibox_state::machine_reset()
{
	famicombox_bankswitch(0);
}

void famibox_state::machine_start()
{
	m_nt_ram = std::make_unique<uint8_t[]>(0x800);

	famicombox_bankswitch(0);

	m_attract_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(famibox_state::famicombox_attract_timer_callback),this));
	m_gameplay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(famibox_state::famicombox_gameplay_timer_callback),this));
	m_exception_cause = 0xff;
	m_exception_mask = 0;
	m_attract_timer_period = 0;
	m_money_reg = 0;
	m_coins = 0;
	m_zapper_enable = 0;
}

void famibox_state::famibox(machine_config &config)
{
	// basic machine hardware
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &famibox_state::famibox_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update("ppu", FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_addrmap(0, &famibox_state::famibox_ppu_map);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);

	NES_CONTROL_PORT(config, m_ctrl[0], famibox_control_port12_devices, "joypad");
	NES_CONTROL_PORT(config, m_ctrl[1], famibox_control_port12_devices, "joypad");
	NES_CONTROL_PORT(config, m_ctrl[2], famibox_control_port3_devices, "zapper");
	for (int i = 0; i < 3; i++)
		m_ctrl[i]->set_screen_tag(m_screen);
}

void famibox_state::init_famibox()
{
	m_mirroring = PPU_MIRROR_HORZ;
}

void famibox_state::init_famistat()
{
	m_mirroring = PPU_MIRROR_VERT;
}

// These have all been confirmed against FamicomBox carts, except Excitebike and Hogan's Alley
#define GAME_LIST \
	ROM_REGION(0x6000, "baseball", 0) \
	ROM_LOAD("hvc-ba-0 prg", 0x0000, 0x4000, CRC(d18a3dde) SHA1(91f7d3e4c9d18c1969ca1fffdc811b763508a0a2) ) \
	ROM_LOAD("hvc-ba-0 chr", 0x4000, 0x2000, CRC(c27eef20) SHA1(d5bd643b3ba98846e520b4d3f38aae45a29cf250) ) \
 \
	ROM_REGION(0x6000, "bombman", 0) \
	ROM_LOAD("hvc-bm-0 prg", 0x0000, 0x4000, CRC(9684657f) SHA1(055db2dc8cec0448f3845da1626e108c7692cfc6) ) \
	ROM_LOAD("hvc-bm-0 chr", 0x4000, 0x2000, CRC(a775822e) SHA1(b0584f9f4172b9e111ae275d8de6644b76372b32) ) \
 \
	ROM_REGION(0x6000, "dkong", 0) \
	ROM_LOAD("hvc-dk-1 prg", 0x0000, 0x4000, CRC(f56a5b10) SHA1(2c4b1d653194df0996d54d9de9188b270d0337d9) ) \
	ROM_LOAD("hvc-dk-0 chr", 0x4000, 0x2000, CRC(a21d7c2e) SHA1(97c16cd6b1f3656428b682a23e6e4248c1ca3607) ) \
 \
	ROM_REGION(0x6000, "duckhunt", 0) \
	ROM_LOAD("hvc-dh-0 prg", 0x0000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) ) \
	ROM_LOAD("hvc-dh-0 chr", 0x4000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) ) \
 \
	ROM_REGION(0x6000, "excitebk", 0) \
	ROM_LOAD("hvc-eb-0 prg", 0x0000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) ) \
	ROM_LOAD("hvc-eb-0 chr", 0x4000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) ) \
 \
	ROM_REGION(0x6000, "f1race", 0) \
	ROM_LOAD("sss-fr prg", 0x0000, 0x4000, CRC(57970078) SHA1(c212294be2a3b8f89ff440df821324fa0d522a55) ) \
	ROM_LOAD("sss-fr chr", 0x4000, 0x2000, CRC(e653dbcb) SHA1(f9758fcc8e07890bd733af127defc86bb70f179e) ) \
 \
	ROM_REGION(0x6000, "golf", 0) \
	ROM_LOAD("hvc-gf-0 prg", 0x0000, 0x4000, CRC(9c7e6421) SHA1(e67e9ff5ee81fbd1af8d7439b86a9ad98499b9dc) ) \
	ROM_LOAD("hvc-gf-0 chr", 0x4000, 0x2000, CRC(7dfa75a8) SHA1(ee016d37f4c54bea8cbbb9ae125bff4c7e14bfb3) ) \
 \
	ROM_REGION(0x6000, "hogan", 0) \
	ROM_LOAD("hvc-ha-0 prg", 0x0000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) ) \
	ROM_LOAD("hvc-ha-0 chr", 0x4000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) ) \
 \
	ROM_REGION(0xa000, "icehocky", 0) \
	ROM_LOAD("sss hy-0 prg", 0x0000, 0x8000, CRC(82dff13d) SHA1(4edbf555d319dfe1c2a08dc28f484d4344a228ba) ) \
	ROM_LOAD("sss hy-0 chr", 0x8000, 0x2000, CRC(f10fc90a) SHA1(1a2a657267de1f5bdf284d1b69ed7d4895dfb281) ) \
 \
	ROM_REGION(0x6000, "mahjong", 0) \
	ROM_LOAD("hvc mj-1 prg", 0x0000, 0x4000, CRC(f86d8d8a) SHA1(2904137a030ae2370a8cd3e068078a1d59a4f229) ) \
	ROM_LOAD("hvc mj-1 chr", 0x4000, 0x2000, CRC(6bb45576) SHA1(5974787496dfa27a4b7fe6023473fae930ea41dc) ) \
 \
	ROM_REGION(0x6000, "mario", 0) \
	ROM_LOAD("hvc-ma-0 prg", 0x0000, 0x4000, CRC(75f6a9f3) SHA1(b6f88f7a2f9a49cc9182a244571730198f1edc4b) ) \
	ROM_LOAD("hvc-ma-0 chr", 0x4000, 0x2000, CRC(10f77435) SHA1(a646c3443832ada84d31a3a8a4b34aebc17cecd5) ) \
 \
	ROM_REGION(0xa000, "smb", 0) \
	ROM_LOAD("hvc sm-0 prg", 0x0000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) ) \
	ROM_LOAD("hvc sm-0 chr", 0x8000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) ) \
 \
	ROM_REGION(0x6000, "tennis", 0) \
	ROM_LOAD("hvc-te-0 prg", 0x0000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) ) \
	ROM_LOAD("hvc-te-0 chr", 0x4000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) ) \
 \
	ROM_REGION(0x6000, "wildgunm", 0) \
	ROM_LOAD("hvc-wg-1 prg", 0x0000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) ) \
	ROM_LOAD("hvc-wg-0 chr", 0x4000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) ) \
 \
	ROM_REGION(0xa000, "wrecking", 0) \
	ROM_LOAD("hvc-wr-0 prg", 0x0000, 0x8000, CRC(4328b273) SHA1(764d68f05f4a6e43fb26d7e654e237d2b0258fe4) ) \
	ROM_LOAD("hvc-wr-0 chr", 0x8000, 0x2000, CRC(23f0b9fd) SHA1(c7f2d4f5f555490847654b8458687f94fba3bd12) )


ROM_START(famibox)
	ROM_REGION(0xa000, "menu", 0)
	ROM_LOAD("sss-m prg v-3", 0x0000, 0x8000, CRC(da1eb8d2) SHA1(943e3b0edfbf9bd3ee87dc5f298621b9ddc98db8))
	ROM_LOAD("sss-m chr v-1", 0x8000, 0x2000, CRC(a43d4435) SHA1(ee56b4d2110aff394bf2c8cd3414ca175ace01bd))

	GAME_LIST
ROM_END

ROM_START(famistat)
	ROM_REGION(0xa000, "menu", 0)
// These should be verified. Dumps come from a board believed to have been damaged by overvoltage.
// In particular PRG has had zeroed bytes at $0000 and $0005 replaced with bytes from the Famicombox
// menu's PRG. Code at this location uses the attract mode DSW which was not looping between game
// and Sharp logo screen. Hardware footage confirms it should work. Furthermore, correcting these
// bytes makes the Self Check PRG checksum pass, further evidence this is correct.
	ROM_LOAD("sss-m prg", 0x0000, 0x8000, BAD_DUMP CRC(c637d2fd) SHA1(1acb6a7afb1674fa77a6ebadb21b0a8aa9249001))
	ROM_LOAD("sss-m chr", 0x8000, 0x2000, CRC(85561c8a) SHA1(35ab7e72512831a2f4cfaa689551fe7b5fa6d673))

	GAME_LIST
ROM_END
} // Anonymous namespace


GAME( 1986, famibox,  0,       famibox, famibox, famibox_state, init_famibox,  ROT0, "Nintendo", "FamicomBox",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
GAME( 1986, famistat, famibox, famibox, famibox, famibox_state, init_famistat, ROT0, "Nintendo", "FamicomStation", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
