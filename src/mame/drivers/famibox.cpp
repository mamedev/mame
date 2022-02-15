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

- The FamicomBox will not run mmc3 games and many other advanced mappers
- There a special lockout chip, but the lockout chip connects to different pins on
  a FamicomBox cartridge's connector than a regular cart
- The lockout chips in the system and the games have to 'talk' before the system will
  load any games into its menu.

Here's a list of some of the games known to have come with the FamicomBox:
1943; Baseball; Bomber Man; Devil World; Donkey Kong; Donkey Kong Jr.; Duck Hunt;
Excitebike; F1 Race; Fighting Golf; Golf; Gradius; Hogan's Alley; Ice Climbers;
Ice Hockey; Knight Rider; Makaimura: Ghosts 'n Goblins; McKids; Mah-Jong; Mario Bros.;
Mike Tyson's Punch-Out!!; Ninja Ryukenden; Operation Wolf (?); Punch-Out!!; Rock Man;
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
- nametable mirroring is incorrectly hardcoded (cart PCBs have H/V solder pads like their NES counterparts)
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
	switch(mirroring)
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
	int page = (offset & 0xc00) >> 10;
	m_nt_page[page][offset & 0x3ff] = data;
}


uint8_t famibox_state::famibox_nt_r(offs_t offset)
{
	int page = (offset & 0xc00) >> 10;
	return m_nt_page[page][offset & 0x3ff];
}

/******************************************************

   NES interface

*******************************************************/

void famibox_state::sprite_dma_w(address_space &space, uint8_t data)
{
	int source = (data & 7);
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
	} famicombox_banks[] =
	{
		{ 0x11, "donkeykong",   0, 0, 0x4000 },
		{ 0x12, "donkeykongjr", 0, 0, 0x4000 },
		{ 0x13, "popeye",       0, 0, 0x4000 },
		{ 0x14, "eigoasobi",    0, 0, 0x4000 },
		{ 0x15, "mahjong",      0, 0, 0x4000 },
		{ 0x26, "gomokunarabe", 0, 0, 0x4000 },
		{ 0x27, "baseball",     0, 0, 0x4000 },
		{ 0x28, "empty",        0, 0, 0x4000 },
		{ 0x29, "empty",        0, 0, 0x4000 },
		{ 0x2a, "empty",        0, 0, 0x4000 },
		{ 0x3b, "empty",        0, 0, 0x4000 },
		{ 0x3c, "empty",        0, 0, 0x4000 },
		{ 0x3d, "empty",        0, 0, 0x4000 },
		{ 0x3e, "empty",        0, 0, 0x4000 },
		{ 0x3f, "empty",        0, 0, 0x4000 },
		{ 0x00, "menu",         0, 0x4000, 0x8000 },
	};


	for (auto & famicombox_bank : famicombox_banks)
	{
		if ( bank == famicombox_bank.bank ||
				famicombox_bank.bank == 0 )
		{
			membank("cpubank1")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.bank1_offset);
			membank("cpubank2")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.bank2_offset);
			membank("ppubank1")->set_base(memregion(famicombox_bank.memory_region)->base() + famicombox_bank.ppubank_offset);
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
	set_mirroring(m_mirroring);

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

#define GAME_LIST \
	ROM_REGION(0x6000, "donkeykong", 0) \
	ROM_LOAD("0.prg", 0x0000, 0x4000, CRC(06d1a012) SHA1(a6f92ae0a991c532e6377db2b3ab7f5c13d27675) ) \
	ROM_LOAD("0.chr", 0x4000, 0x2000, CRC(a21d7c2e) SHA1(97c16cd6b1f3656428b682a23e6e4248c1ca3607) ) \
 \
	ROM_REGION(0x6000, "donkeykongjr", 0) \
	ROM_LOAD("hvc-jr-1 prg", 0x0000, 0x4000, CRC(cf6c88b6) SHA1(cefc276e7601d14c6a20e545f334281b7a9fe8db) ) \
	ROM_LOAD("hvc-jr-0 chr", 0x4000, 0x2000, CRC(852778ab) SHA1(307f2245cce164491012f75897eb984af0c3f456) ) \
 \
	ROM_REGION(0x6000, "popeye", 0) \
	ROM_LOAD("hvc-pp-1 prg", 0x0000, 0x4000, CRC(0fa63a45) SHA1(50c594a6d8dcbeee2d83bca8c54c42cf57093aba) ) \
	ROM_LOAD("hvc-pp-0 chr", 0x4000, 0x2000, CRC(a5fd8d98) SHA1(09d229404babb6c89b417ac541bab80fb06d2ba9) ) \
 \
	ROM_REGION(0x6000, "eigoasobi", 0) \
	ROM_LOAD("hvc-en-0 prg", 0x0000, 0x4000, CRC(2dbfa36a) SHA1(0f4301d78d3dfa163239e7b7b7c4dff8a7e21bac) ) \
	ROM_LOAD("hvc-en-0 chr", 0x4000, 0x2000, CRC(fccc0f36) SHA1(3566709c3c74960ce2ee1e60a85d026e70d7fd2c) ) \
 \
	ROM_REGION(0x6000, "mahjong", 0) \
	ROM_LOAD("mahjong.prg", 0x0000, 0x4000, CRC(f86d8d8a) SHA1(2904137a030ae2370a8cd3e068078a1d59a4f229) ) \
	ROM_LOAD("mahjong.chr", 0x4000, 0x2000, CRC(6bb45576) SHA1(5974787496dfa27a4b7fe6023473fae930ea41dc) ) \
 \
	ROM_REGION(0x6000, "gomokunarabe", 0) \
	ROM_LOAD("hvc-go-0 prg", 0x0000, 0x4000, CRC(5603f579) SHA1(f2b007e3b13a777f9f88ff58f87ead6ae8f26327) ) \
	ROM_LOAD("hvc-go-0 chr", 0x4000, 0x2000, CRC(97ea7144) SHA1(47d354c654285275d0a9420cc6eb3564f0453eb0) ) \
 \
	ROM_REGION(0x6000, "baseball", 0) \
	ROM_LOAD("hvc-ba-0 prg", 0x0000, 0x4000, CRC(d18a3dde) SHA1(91f7d3e4c9d18c1969ca1fffdc811b763508a0a2) ) \
	ROM_LOAD("hvc-ba-0 chr", 0x4000, 0x2000, CRC(c27eef20) SHA1(d5bd643b3ba98846e520b4d3f38aae45a29cf250) )

ROM_START(famibox)
	ROM_REGION(0xa000, "menu", 0)
	ROM_LOAD("sss-m prg v-3", 0x0000, 0x8000, CRC(da1eb8d2) SHA1(943e3b0edfbf9bd3ee87dc5f298621b9ddc98db8))
	ROM_LOAD("sss-m chr v-1", 0x8000, 0x2000, CRC(a43d4435) SHA1(ee56b4d2110aff394bf2c8cd3414ca175ace01bd))

	GAME_LIST

	ROM_REGION(0x6000, "empty", ROMREGION_ERASEFF)
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

	ROM_REGION(0x6000, "empty", ROMREGION_ERASEFF)
ROM_END
} // Anonymous namespace


GAME( 1986, famibox,  0,       famibox, famibox, famibox_state, init_famibox,  ROT0, "Nintendo", "FamicomBox",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
GAME( 1986, famistat, famibox, famibox, famibox, famibox_state, init_famistat, ROT0, "Nintendo", "FamicomStation", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
