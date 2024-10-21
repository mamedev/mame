// license:BSD-3-Clause
// copyright-holders:David Haywood
/********************************************************************

 PCB is marked 'Ver 1.8 B/D-' on 2 of the edges

 Custom chip marked
 ORIENTAL SOFT
  SPR800F1
    0011E

 -- Test Mode Note --

 The test mode for this game is very buggy, this is not a MAME bug
 the same behavior has been observed on the original PCB.

 One example of this is the DIP switch viewer, which should show 3
 rows of 8 1/0 values, one for each switch. ie

 00000000
 00000000
 00000000

 However..

 Instead of properly representing each of the banks, the 1st switch in
 each bank ends up turning on/off the entire row display (for rows 2/3
 it shifts row 1 by one pixel)

 This then means the 2nd switch changes the digit in the 1st position
 so

 10000000

 the 8th switch changes the 7th digit so

 10000010

 and there's no way at all to change the last digit.

*********************************************************************/

#include "emu.h"

#include "cpu/e132xs/e132xs.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class mjsenpu_state : public driver_device
{
public:
	mjsenpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_palette(*this, "palette")
		, m_hopper(*this, "hopper")
		, m_mainram(*this, "mainram")
	//  , m_vram(*this, "vram")
		, m_io_key(*this, "KEY%u", 0U)
	{
	}

	void mjsenpu(machine_config &config) ATTR_COLD;

	void init_mjsenpu() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<e132xt_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;

	required_shared_ptr<uint32_t> m_mainram;
//  required_shared_ptr<uint32_t> m_vram;

	required_ioport_array<5> m_io_key;

	std::unique_ptr<uint16_t[]> m_vram[2];
	uint8_t m_control = 0;
	uint8_t m_key_matrix_select = 0;

	void control_w(uint8_t data);
	void key_matrix_w(uint8_t data);

	uint32_t key_matrix_r();

	uint32_t speedup_r();

	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	static rgb_t mjsenpu_B6G5R5(uint32_t raw);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
};


rgb_t mjsenpu_state::mjsenpu_B6G5R5(uint32_t raw)
{
	return rgb_t(pal5bit(raw >> 0), pal5bit(raw >> 5), pal6bit(raw >> 10));
}


uint16_t mjsenpu_state::vram_r(offs_t offset)
{
	return m_vram[BIT(m_control, 0)][offset];
}

void mjsenpu_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[BIT(m_control, 0)][offset]);
}

void mjsenpu_state::control_w(uint8_t data)
{
	// bit 0x80 is always set? (sometimes disabled during screen transitions briefly, could be display enable?)

	// bit 0x40 not used?
	// bit 0x20 not used?

	// bit 0x10 is the M6295 bank, samples <26 are the same in both banks and so bank switch isn't written for them, not even in sound test.
	m_oki->set_rom_bank(BIT(data, 4));

	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // credits out
	m_hopper->motor_w(BIT(data, 2));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1)); // coin or key-in

	// bit 0x01 alternates frequently, using as video buffer, but that's a complete guess
	m_control = data;

//  if (data &~0x9e)
//      logerror("control_w %02x\n", data);
}

void mjsenpu_state::key_matrix_w(uint8_t data)
{
	if ((data & 0xe0) != 0x80)
		logerror("key_matrix_w %02x\n", data);

	// bit 0 to 4: Key matrix select
	// Bit 5 to 6: unknown, always clear?
	// bit 7: unknown, always set?
	m_key_matrix_select = data;
}

uint32_t mjsenpu_state::key_matrix_r()
{
	uint32_t result = 0xffffffff;
	for (unsigned i = 0; i < m_io_key.size(); i++)
	{
		if (BIT(~m_key_matrix_select, i))
			result &= m_io_key[i]->read();
	}
	return result;
}


void mjsenpu_state::main_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share(m_mainram);
	map(0x40000000, 0x401fffff).rom().region("maindata", 0); // main game rom

	map(0x80000000, 0x8001ffff).rw(FUNC(mjsenpu_state::vram_r), FUNC(mjsenpu_state::vram_w));

	map(0xffc00000, 0xffc000ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xffd00000, 0xffd000ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");

	map(0xffe00000, 0xffe007ff).ram().share("nvram");

	map(0xfff80000, 0xffffffff).rom().region("mainboot", 0); // boot rom
}


void mjsenpu_state::main_portmap(address_map &map)
{
	map(0x4000, 0x4003).r(FUNC(mjsenpu_state::key_matrix_r));
	map(0x4010, 0x4013).portr("IN1");

	map(0x4023, 0x4023).w(FUNC(mjsenpu_state::control_w));

	map(0x4030, 0x4033).portr("DSW1");
	map(0x4040, 0x4043).portr("DSW2");
	map(0x4050, 0x4053).portr("DSW3");

	map(0x4063, 0x4063).w(FUNC(mjsenpu_state::key_matrix_w));

	map(0x4073, 0x4073).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

static INPUT_PORTS_START( mjsenpu )

	PORT_START("KEY0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_A )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_E )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_M )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 槓
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 開始
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_UNUSED )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_B )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_F )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_N )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )          PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 聽
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 押
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_UNUSED )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_C )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_G )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 吃
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 胡
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_UNUSED )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_D )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_H )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )              PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 碰
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_UNUSED )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE)     PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 海底
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)           PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 得分
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)       PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 比倍
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)             PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 大
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)           PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)    // 小
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 開始
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )            PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 上
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )          PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 下
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )          PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 左
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )         PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 右
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON1 )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 摸捨
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                  PORT_CONDITION("DSW3", 0x02, EQUALS, 0x02)    // 投幣
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )           PORT_CONDITION("DSW3", 0x02, EQUALS, 0x00)    // 投幣
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )          PORT_CONDITION("DSW3", 0x04, EQUALS, 0x04)    // 退幣
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )          PORT_CONDITION("DSW3", 0x04, EQUALS, 0x00)    // 退幣
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // 哈巴
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                          // 査悵
	PORT_SERVICE_NO_TOGGLE( 0x00000010, IP_ACTIVE_LOW )                                                             // 測試
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // clears stats in bookkeeping
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )                 PORT_CONDITION("DSW3", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON2 )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 押注
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON3 )                PORT_CONDITION("DSW3", 0x08, EQUALS, 0x00)    // 功能
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x00000003, 0x00000003, DEF_STR(Coinage) )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(          0x00000000, DEF_STR(2C_1C) )                                          // 2:1
	PORT_DIPSETTING(          0x00000003, DEF_STR(1C_1C) )                                          // 1:1
	PORT_DIPSETTING(          0x00000002, DEF_STR(1C_2C) )                                          // 1:2
	PORT_DIPSETTING(          0x00000001, DEF_STR(1C_3C) )                                          // 1:3
	PORT_DIPNAME( 0x0000000c, 0x0000000c, "Key-In Rate" )             PORT_DIPLOCATION("SW1:3,4")   // 開分比率
	PORT_DIPSETTING(          0x0000000c, "5" )
	PORT_DIPSETTING(          0x00000008, "10" )
	PORT_DIPSETTING(          0x00000004, "50" )
	PORT_DIPSETTING(          0x00000000, "100" )
	PORT_DIPNAME( 0x00000030, 0x00000030, "Coin Out Rate" )           PORT_DIPLOCATION("SW1:5,6")   // 退幣比率
	PORT_DIPSETTING(          0x00000030, DEF_STR(1C_1C) )                                          // 1:1
	PORT_DIPSETTING(          0x00000020, DEF_STR(2C_1C) )                                          // 1:2
	PORT_DIPSETTING(          0x00000010, DEF_STR(5C_1C) )                                          // 1:5
	PORT_DIPSETTING(          0x00000000, "10 Coins/1 Credit" )                                     // 1:10
	PORT_DIPNAME( 0x000000c0, 0x000000c0, "Jackpot Odds" )            PORT_DIPLOCATION("SW1:7,8")   // 大滿貫機率
	PORT_DIPSETTING(          0x000000c0, "84%" )
	PORT_DIPSETTING(          0x00000080, "88%" )
	PORT_DIPSETTING(          0x00000040, "92%" )
	PORT_DIPSETTING(          0x00000000, "96%" )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Minimum Bet" )             PORT_DIPLOCATION("SW2:1,2")   // 最小押注
	PORT_DIPSETTING(          0x00000003, "1" )
	PORT_DIPSETTING(          0x00000002, "2" )
	PORT_DIPSETTING(          0x00000001, "3" )
	PORT_DIPSETTING(          0x00000000, "5" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Maximum Bet" )             PORT_DIPLOCATION("SW2:3")     // 最大押注
	PORT_DIPSETTING(          0x00000004, "10" )
	PORT_DIPSETTING(          0x00000000, "20" )
	PORT_DIPNAME( 0x00000008, 0x00000000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:4")     // 示範音樂
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW2:5")     // 倒轉画面
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000000e0, 0x000000e0, "Game Odds" )               PORT_DIPLOCATION("SW2:6,7,8") // 遊戲機率
	PORT_DIPSETTING(          0x00000000, "60%" )
	PORT_DIPSETTING(          0x00000020, "65%" )
	PORT_DIPSETTING(          0x00000040, "70%" )
	PORT_DIPSETTING(          0x00000060, "75%" )
	PORT_DIPSETTING(          0x00000080, "80%" )
	PORT_DIPSETTING(          0x000000a0, "85%" )
	PORT_DIPSETTING(          0x000000c0, "90%" )
	PORT_DIPSETTING(          0x000000e0, "95%" )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Credit Limit" )            PORT_DIPLOCATION("SW3:1")     // 进分上限
	PORT_DIPSETTING(          0x00000001, "100" )
	PORT_DIPSETTING(          0x00000000, "500" )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Credit Mode" )             PORT_DIPLOCATION("SW3:2")     // 進分方式
	PORT_DIPSETTING(          0x00000002, "Coin Acceptor" )                                         // 投幣
	PORT_DIPSETTING(          0x00000000, "Key-In" )                                                // 開分
	PORT_DIPNAME( 0x00000004, 0x00000004, "Payout Mode" )             PORT_DIPLOCATION("SW3:3")     // 退分方式
	PORT_DIPSETTING(          0x00000004, "Return Coins" )                                          // 退幣
	PORT_DIPSETTING(          0x00000000, "Key-Out" )                                               // 洗分
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR(Controls) )         PORT_DIPLOCATION("SW3:4")     // 操作方法
	PORT_DIPSETTING(          0x00000008, "Mahjong" )                                               // 按鍵
	PORT_DIPSETTING(          0x00000000, DEF_STR(Joystick) )                                       // 搖桿
	PORT_DIPNAME( 0x00000010, 0x00000010, "Double Up Game" )          PORT_DIPLOCATION("SW3:5")     // 續玩遊戲
	PORT_DIPSETTING(          0x00000010, DEF_STR(Off) )                                            // 無
	PORT_DIPSETTING(          0x00000000, DEF_STR(On) )                                             // 有
	PORT_DIPNAME( 0x00000060, 0x00000060, "Double Up Odds" )          PORT_DIPLOCATION("SW3:6,7")   // 續玩機率
	PORT_DIPSETTING(          0x00000060, "80%" )
	PORT_DIPSETTING(          0x00000040, "84%" )
	PORT_DIPSETTING(          0x00000020, "88%" )
	PORT_DIPSETTING(          0x00000000, "92%" )
	PORT_DIPNAME( 0x00000080, 0x00000080, "Jackpot Win" )             PORT_DIPLOCATION("SW3:8")     // 大滿貫中奨
	PORT_DIPSETTING(          0x00000080, DEF_STR(Off) )                                            // 無
	PORT_DIPSETTING(          0x00000000, DEF_STR(On) )                                             // 有
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


void mjsenpu_state::video_start()
{
	for (int i = 0; i < 2; i++)
	{
		uint32_t const vram_size = 0x20000 / 2;
		m_vram[i] = make_unique_clear<uint16_t[]>(vram_size);

		save_pointer(NAME(m_vram[i]), vram_size, i);
	}
}


uint32_t mjsenpu_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const vram = m_vram[BIT(~m_control, 0)].get();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		offs_t count = (y << 8) | (cliprect.min_x >> 1);
		for (int x = (cliprect.min_x >> 1); x <= ((cliprect.max_x + 1) >> 1); x++)
		{
			uint8_t color = vram[count] & 0x00ff;
			bitmap.pix(y, x*2 + 0) = color;

			color = (vram[count] & 0xff00) >> 8;
			bitmap.pix(y, x*2 + 1) = color;

			count++;
		}
	}
	return 0;
}


void mjsenpu_state::machine_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_key_matrix_select));
}

void mjsenpu_state::machine_reset()
{
}

/*
following clocks are on the PCB

22.1184
27.000
1.0000000

*/

void mjsenpu_state::mjsenpu(machine_config &config)
{
	// basic machine hardware
	E132XT(config, m_maincpu, 27000000*2); // ?? Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &mjsenpu_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mjsenpu_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(mjsenpu_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// more likely coins out?
	HOPPER(config, m_hopper, attotime::from_msec(50));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-16-1);
	screen.set_screen_update(FUNC(mjsenpu_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(2, &mjsenpu_state::mjsenpu_B6G5R5, 0x100);
	m_palette->set_membits(8);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00); // 1 Mhz?
}


ROM_START( mjsenpu )
	ROM_REGION32_BE( 0x80000, "mainboot", 0 ) // Hyperstone CPU Code
	ROM_LOAD( "u1", 0x000000, 0x080000, CRC(ebfb1079) SHA1(9d676c635d5ee464df5730518399e141ebc515ed) )

	ROM_REGION32_BE( 0x200000, "maindata", 0 ) // Hyperstone CPU Code
	ROM_LOAD16_WORD_SWAP( "u13", 0x000000, 0x200000, CRC(a803c5a5) SHA1(61c7386a1bb6224b788de01293697d0e896839a8) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "su2", 0x000000, 0x080000, CRC(848045d5) SHA1(4d32e1a5bd0937069dd8d50dfd8b63d4a45e40e6) )
ROM_END



uint32_t mjsenpu_state::speedup_r()
{
	offs_t const pc = m_maincpu->pc();

	if (pc == 0xadb8)
	{
		m_maincpu->spin_until_interrupt();
	}
	else
	{
	//  logerror("%08x\n", pc);
	}

	return m_mainram[0x23468/4];
}




void mjsenpu_state::init_mjsenpu()
{
/*
0000ADAE: LDHU.D L42, L38, $0
0000ADB2: LDW.A 0, L39, $23468
0000ADB8: ADDI L38, $1
0000ADBA: STHU.D L42, L38, $0
0000ADBE: CMPI L39, $0
0000ADC0: BNE $adae

   (loops for 744256 instructions)
*/
	// not especially effective, might be wrong.
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x23468, 0x2346b, read32smo_delegate(*this, FUNC(mjsenpu_state::speedup_r)));
}

} // anonymous namespace

// Japanese text (excluding test mode), Chinese voice and test mode texts
GAME( 2002, mjsenpu, 0, mjsenpu, mjsenpu, mjsenpu_state, init_mjsenpu, ROT0, "Oriental Soft", "Mahjong Senpu (Japan)", 0 )
