// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    DataEast/Sega Version 3
*/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/decopincpu.h"
#include "audio/decobsmt.h"
#include "video/decodmd1.h"
#include "video/decodmd2.h"
#include "machine/genpin.h"
#include "machine/nvram.h"

// Data East CPU board is similar to Williams System 11, but without the generic audio board.
// For now, we'll presume the timings are the same.

// 6808 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL_4MHz/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

extern const char layout_pinball[];
class de_3_state : public genpin_class
{
public:
	de_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
		m_dmdtype1(*this, "decodmd1"),
		m_dmdtype2(*this, "decodmd2"),
		m_decobsmt(*this, "decobsmt")
	{ }

	// devices
	optional_device<decodmd_type1_device> m_dmdtype1;
	optional_device<decodmd_type2_device> m_dmdtype2;

	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_READ8_MEMBER(pia2c_pb_r);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	//DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	//DECLARE_WRITE_LINE_MEMBER(msm5205_irq_w);
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(dac_w) { };
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_READ8_MEMBER(dmd_status_r);

//  DECLARE_READ8_MEMBER(sound_latch_r);
//  DECLARE_WRITE8_MEMBER(sample_bank_w);

	required_device<decobsmt_device> m_decobsmt;
	bool m_nmi_enable;

	// devcb callbacks
	DECLARE_READ8_MEMBER(display_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(lamps_w);

protected:

	// driver_device overrides
	virtual void machine_reset() override;
private:
//  UINT32 m_segment1;
//  UINT32 m_segment2;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT8 m_diag;
	bool m_ca1;
	bool m_irq_active;
	UINT8 m_sound_data;

public:
	DECLARE_DRIVER_INIT(de_3);
};


static INPUT_PORTS_START( de_3 )
	PORT_START("INP0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("INP4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("INP8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("INP10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("INP20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("INP40")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

// 6821 PIA at 0x2000
WRITE8_MEMBER( de_3_state::sol3_w )
{
}

WRITE8_MEMBER( de_3_state::sound_w )
{
	m_sound_data = data;
	if(m_sound_data != 0xfe)
		m_decobsmt->bsmt_comms_w(space,offset,m_sound_data);
}

WRITE_LINE_MEMBER( de_3_state::pia21_ca2_w )
{
// sound ns
	m_ca1 = state;
}

// 6821 PIA at 0x2400
WRITE8_MEMBER( de_3_state::lamp0_w )
{
}


// 6821 PIA at 0x2800
WRITE8_MEMBER( de_3_state::dig0_w )
{
//  static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
//  data &= 0x7f;
//  m_strobe = data & 15;
//  m_diag = (data & 0x70) >> 4;
//  output_set_digit_value(60, patterns[data>>4]); // diag digit
//  m_segment1 = 0;
//  m_segment2 = 0;
}

WRITE8_MEMBER( de_3_state::dig1_w )
{
//  m_segment2 |= data;
//  m_segment2 |= 0x30000;
//  if ((m_segment2 & 0x70000) == 0x30000)
//  {
//      if(m_is_alpha3)  // Alphanumeric type 2 uses 7 segment LEDs on the bottom row, type 3 uses 14 segment LEDs
//          output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
//      else
//          output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0));
//      m_segment2 |= 0x40000;
//  }
}

READ8_MEMBER( de_3_state::pia28_w7_r )
{
	UINT8 ret = 0x80;

	ret |= m_strobe;
	ret |= m_diag << 4;

	return ret;
}

// 6821 PIA at 0x2c00
WRITE8_MEMBER( de_3_state::pia2c_pa_w )
{
	/* DMD data */
	if(m_dmdtype2)
	{
		m_dmdtype2->data_w(space,offset,data);
		logerror("DMD: Data write %02x\n", data);
	}
	else if(m_dmdtype1)
	{
		m_dmdtype1->data_w(space,offset,data);
		logerror("DMD: Data write %02x\n", data);
	}
//  m_segment1 |= (data<<8);
//  m_segment1 |= 0x10000;
//  if ((m_segment1 & 0x70000) == 0x30000)
//  {
//      output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
//      m_segment1 |= 0x40000;
//  }
}

READ8_MEMBER( de_3_state::pia2c_pb_r )
{
	if(m_dmdtype1)
		return m_dmdtype1->busy_r(space,offset);
	if(m_dmdtype2)
		return m_dmdtype2->busy_r(space,offset);
	return 0;
}

WRITE8_MEMBER( de_3_state::pia2c_pb_w )
{
	/* DMD ctrl */
	if(m_dmdtype2)
	{
		m_dmdtype2->ctrl_w(space,offset,data);
		logerror("DMD: Control write %02x\n", data);
	}
	else if(m_dmdtype1)
	{
		m_dmdtype1->ctrl_w(space,offset,data);
		logerror("DMD: Control write %02x\n", data);
	}

//  m_segment1 |= data;
//  m_segment1 |= 0x20000;
//  if ((m_segment1 & 0x70000) == 0x30000)
//  {
//      output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
//      m_segment1 |= 0x40000;
//  }
}


// 6821 PIA at 0x3000
READ8_MEMBER( de_3_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( de_3_state::switch_w )
{
	int x;

	for(x=0;x<8;x++)
	{
		if(data & (1<<x))
			break;
	}
	m_kbdrow = data & (1<<x);
}

// 6821 PIA at 0x3400
WRITE8_MEMBER( de_3_state::pia34_pa_w )
{
	// Not connected?
//  m_segment2 |= (data<<8);
//  m_segment2 |= 0x10000;
//  if ((m_segment2 & 0x70000) == 0x30000)
//  {
//      output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
//      m_segment2 |= 0x40000;
//  }
}

READ8_MEMBER( de_3_state::dmd_status_r )
{
	if(m_dmdtype1)
	{
		return m_dmdtype1->status_r(space,offset);
	}
	else if(m_dmdtype2)
	{
		return m_dmdtype2->status_r(space,offset);
	}
	return 0;
}

READ8_MEMBER(de_3_state::display_r)
{
	UINT8 ret = 0x00;

	switch(offset)
	{
	case 0:
		ret = pia28_w7_r(space,0);
		break;
	case 3:
		ret = pia2c_pb_r(space,0);
		break;
	}

	return ret;
}

WRITE8_MEMBER(de_3_state::display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_3_state::lamps_w)
{
	switch(offset)
	{
	case 0:
		lamp0_w(space,0,data);
		break;
	case 1:
		lamp1_w(space,0,data);
		break;
	}
}


void de_3_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(de_3_state,de_3)
{
}

static MACHINE_CONFIG_START( de_3, de_3_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE3_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_3_state,display_r),WRITE8(de_3_state,display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_3_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_3_state,switch_r),WRITE8(de_3_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_3_state,lamps_w))
	MCFG_DECOCPU_DMDSTATUS(READ8(de_3_state,dmd_status_r))

	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_DECOBSMT_ADD(DECOBSMT_TAG)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( de_3_dmd2, de_3 )
	MCFG_DECODMD_TYPE2_ADD("decodmd2",":gfx3")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( de_3_dmd1, de_3 )
	MCFG_DECODMD_TYPE1_ADD("decodmd1",":gfx3")
MACHINE_CONFIG_END

/*-------------------------------------------------------------
/ Adventures of Rocky and Bullwinkle and Friends - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(rab_320)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpua.320", 0x0000, 0x10000, CRC(21a2d518) SHA1(42123dca519034ecb740e5cb493b1b0b6b44e3be))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("rbdspa.300", 0x00000, 0x80000, CRC(a5dc2f72) SHA1(60bbb4914ff56ad48c86c3550e094a3d9d70c700))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

ROM_START(rab_130)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpua.130", 0x0000, 0x10000, CRC(f59b1a53) SHA1(046cd0eaee6e646286f3dfa73eeacfd93c2be273))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("rbdspa.130", 0x00000, 0x80000, CRC(b6e2176e) SHA1(9ccbb30dc0f386fcf5e5255c9f80c720e601565f))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

ROM_START(rab_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpu.103", 0x0000, 0x10000, CRC(d5fe3184) SHA1(dc1ca938f15240d1c15ee5724d29a3538418f8de))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("rabdspsp.103", 0x00000, 0x80000, CRC(02624948) SHA1(069ef69d6ce193d73954935b378230c05b83b8fc))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

/*-------------------------------------------------------------
/ Aaron Spelling - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(aar_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("as512cpu.bin", 0x0000, 0x10000, CRC(03c70e67) SHA1(3093e217943ae80c842a1d893cff5330ac90bc30))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("asdspu12.bin", 0x00000, 0x40000, CRC(5dd81be6) SHA1(20e5ec19550e3795670c5ee4e8e92fae0499fdb8))
	ROM_LOAD("asdspu14.bin", 0x40000, 0x40000, CRC(3f2204ca) SHA1(69523d6c5555d391ab24912f4c4c78aa09a400c1))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("assndu7.bin", 0x0000, 0x10000, CRC(f0414a0d) SHA1(b1f940be05426a39f4e5ea0802fd03a7ce055ebc))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("assndu17.bin", 0x000000, 0x80000, CRC(e151b1fe) SHA1(d7d97499d93885a4f7ebd7bb302731bc5bc456ff))
	ROM_LOAD("assndu21.bin", 0x080000, 0x80000, CRC(7d69e917) SHA1(73e21e65bc194c063933288cb617127b41593466))
ROM_END

/*-------------------------------------------------------------
/ Batman - CPU Rev 3 /DMD Type 1 128K Rom 16/32K CPU Roms
/------------------------------------------------------------*/
ROM_START(btmn_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpub5.103", 0x4000, 0x4000, CRC(6f160581) SHA1(0f2d6c396324fbf116309a872cf95d9a05446cea))
	ROM_LOAD("batcpuc5.103", 0x8000, 0x8000, CRC(8588c5a8) SHA1(41b159c9e4ca523b37f0b893e57f166c85e812e9))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("batdsp.102", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x040000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpub5.101", 0x0000, 0x8000, CRC(a7f5754e) SHA1(2c24cab4cc5f1e05539d2843a49b4b1a8d507630))
	ROM_LOAD("batcpuc5.101", 0x8000, 0x8000, CRC(1fcb85ca) SHA1(daf1e1297975b9b577c796d50b973885f925508e))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("batdsp.102", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x040000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_g13)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batbcpug.103", 0x4000, 0x4000, CRC(6f160581) SHA1(0f2d6c396324fbf116309a872cf95d9a05446cea))
	ROM_LOAD("batccpug.103", 0x8000, 0x8000, CRC(a199ab0f) SHA1(729dab10fee708a18b7be5a2b9b904aa211b233a))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("bat_dspg.104", 0x00000, 0x20000, CRC(1581819f) SHA1(88facfad2e74dd44b71fd19df685a4c2378d26de))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x040000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_106)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("b5_a106.128", 0x4000, 0x4000, CRC(5aa7fbe3) SHA1(587be4fd18ad730e675e720923e00d1775a4560e))
	ROM_LOAD("c5_a106.256", 0x8000, 0x8000, CRC(79e86ccd) SHA1(430ac436bd1c8841950986af80747285a7d25942))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("batdsp.106", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x040000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

/*------------------------------------------------------------
/ Checkpoint - CPU Rev 3 /DMD Type 1 64K Rom 16/32K CPU Roms
/------------------------------------------------------------*/
ROM_START(ckpt_a17)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("chkpntb5.107", 0x4000, 0x4000, CRC(9fbae8e3) SHA1(a25b9dcba2a3f84394972bf36930c0f0344eccbd))
	ROM_LOAD("chkpntc5.107", 0x8000, 0x8000, CRC(082dc283) SHA1(cc3038e0999d2c403fe1863e649b8029376b0387))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("chkpntds.512", 0x00000, 0x10000, CRC(14d9c6d6) SHA1(5470a4ebe7bc4a056f75aa1fffe3a4e3e24457c6))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("chkpntf7.rom", 0x8000, 0x8000, CRC(e6f6d716) SHA1(a034eb94acb174f7dbe192a55cfd00715ca85a75))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("chkpntf6.rom", 0x00000, 0x20000, CRC(2d08043e) SHA1(476c9945354e733bfc9a854760ca8cfa3bc62294))
	ROM_LOAD("chkpntf5.rom", 0x20000, 0x20000, CRC(167daa2c) SHA1(458781726c73a09da2b8e8313e1d359cb795a744))
ROM_END

/*-------------------------------------------------------------
/ Guns N Roses - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(gnr_300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gnrcpua.300", 0x0000, 0x10000, CRC(faf0cc8c) SHA1(0e889ad6eed832d4ccdc6e379f9e4e58ae0e0b83))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("gnrdispa.300", 0x00000, 0x80000, CRC(4abf29e3) SHA1(595328e0f92a6e1972d71c56505a5dd07a373ef5))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("gnru7.snd", 0x0000, 0x10000, CRC(3b9de915) SHA1(a901a1f37bf5433c819393c4355f9d13164b32ce))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("gnru17.snd", 0x000000, 0x80000, CRC(3d3219d6) SHA1(ac4a6d3eff0cdd02b8c79dddcb8fec2e22faa9b9))
	ROM_LOAD("gnru21.snd", 0x080000, 0x80000, CRC(d2ca17ab) SHA1(db7c4f74a2e2c099fe14f38de922fdc851bd4a6b))
	ROM_LOAD("gnru36.snd", 0x100000, 0x80000, CRC(5b32396e) SHA1(66462a6a929c869d668968e057fac199d05df267))
	ROM_LOAD("gnru37.snd", 0x180000, 0x80000, CRC(4930e1f2) SHA1(1569d0c7fea1af008acbdc492c3677ace7d1897a))
ROM_END

/*-------------------------------------------------------------
/ Hook - CPU Rev 3 /DMD  Type 1 128K Rom - CPU Rom
/------------------------------------------------------------*/
ROM_START(hook_408)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.408", 0x0000, 0x10000, CRC(46477fc7) SHA1(ce6228fd9ab4b6c774e128d291f50695746da358))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_401)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.401", 0x0000, 0x10000, CRC(20223298) SHA1(a8063765db947b059eadaad6654ed0c5cad9198d))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_404)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.404", 0x0000, 0x10000, CRC(53357d8b) SHA1(4e8f5f4376418fbac782065c602da82acab06ef3))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

/*-------------------------------------------------------------
/ Jurassic Park - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(jupk_513)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.513", 0x0000, 0x10000, CRC(9f70a937) SHA1(cdea6c6e852982eb5e800db138f7660d51b6fdc8))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("jpdspa.510", 0x00000, 0x80000, CRC(9ca61e3c) SHA1(38ae472f38e6fc33671e9a276313208e5ccd8640))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_501)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.501", 0x0000, 0x10000, CRC(d25f09c4) SHA1(a12ace496352002685b0415515f5f5ce4fc95bdb))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("jpdspa.501", 0x00000, 0x80000, CRC(04a87d42) SHA1(e13df9a63ec77ec6f97b681ed99216ef3f3af691))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_g51)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.501", 0x0000, 0x10000, CRC(d25f09c4) SHA1(a12ace496352002685b0415515f5f5ce4fc95bdb))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("jpdspg.501", 0x00000, 0x80000, CRC(3b524bfe) SHA1(ea6ae6f8fc8379f311fd7ef456f0d6711c4e35c5))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

/*-------------------------------------------------------------
/ Last Action Hero - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(lah_112)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.112", 0x0000, 0x10000, CRC(e7422236) SHA1(c0422fa6d29fe615cb718056bea00eb9a80ce803))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lahdispa.106", 0x00000, 0x80000, CRC(ca6cfec5) SHA1(5e2081387d76bed17c14120cd347d6aaf435276b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_l104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.104", 0x0000, 0x10000, CRC(49b9e5e9) SHA1(cf6198e4c93ce839dc6e5231090d4ca56e9bdea2))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lahdispl.102", 0x00000, 0x80000, CRC(3482c349) SHA1(8f03ba28132ea5159d3193b3adb7b4a6a43046c6))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_l108)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.108", 0x0000, 0x10000, CRC(8942794b) SHA1(f023ca040d6d4c6da80b58a162f1d217e571ed81))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lahdispl.104", 0x00000, 0x80000, CRC(6b1e51a7) SHA1(ad17507b63f2da8aa0651401ccb8d449c15aa46c))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_110)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.110", 0x0000, 0x10000, CRC(d1861dc2) SHA1(288bd06b6ae346d1f6a17a642d5533f1a9a3bf5e))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lahdispa.106", 0x00000, 0x80000, CRC(ca6cfec5) SHA1(5e2081387d76bed17c14120cd347d6aaf435276b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

/*----------------------------------------------------------------
/ Lethal Weapon 3 - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/---------------------------------------------------------------*/
ROM_START(lw3_208)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3cpuu.208", 0x0000, 0x10000, CRC(a3041f8a) SHA1(3c5b8525b8e9b924590648429c56aaf97adee460))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lw3drom1.a26", 0x00000, 0x40000, CRC(44a4cf81) SHA1(c7f3e3d5fbe930650e48423c8ba0ac484ce0640c))
	ROM_LOAD("lw3drom0.a26", 0x40000, 0x40000, CRC(22932ed5) SHA1(395aa376cd8562de7956a6e34b8747e7cf81f935))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_207)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3gc5.207", 0x0000, 0x10000, CRC(27aeaea9) SHA1(f8c40cbc37edac20187ac880be281dd45d8ad614))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lw3drom1.a26", 0x00000, 0x40000, CRC(44a4cf81) SHA1(c7f3e3d5fbe930650e48423c8ba0ac484ce0640c))
	ROM_LOAD("lw3drom0.a26", 0x40000, 0x40000, CRC(22932ed5) SHA1(395aa376cd8562de7956a6e34b8747e7cf81f935))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_205)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3gc5.205", 0x0000, 0x10000, CRC(5ad8ff4a) SHA1(6a01a2195543c0c57ce4ce78703c91500835a2da))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lw3dsp1.205", 0x00000, 0x40000, CRC(9dfeffb4) SHA1(f62f2a884da68b4dbfe7da071058dc8cd1766c36))
	ROM_LOAD("lw3dsp0.205", 0x40000, 0x40000, CRC(bd8156f1) SHA1(b18214af1b79cca79bdc634c175c3bf7d0052843))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3cpu.200", 0x0000, 0x10000, CRC(ddb6e7a7) SHA1(d48309e1984ef9a7682dfde190cf457632044657))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("lw3dsp1.204", 0x00000, 0x40000, CRC(1ba79363) SHA1(46d489a1190533c73370acd8a48cef60d12f87ce))
	ROM_LOAD("lw3dsp0.204", 0x40000, 0x40000, CRC(c74d3cf2) SHA1(076ee9b2e3cad0b8058ac0c70f5ffe7e29f3eff5))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

/*-------------------------------------------------------------
/ Star Trek - CPU Rev 3 /DMD Type 1 128K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(trek_201)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpuu.201", 0x0000, 0x10000, CRC(ea0681fe) SHA1(282c8181e60da6358ef320358575a538aa4abe8c))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("trekdspa.109", 0x00000, 0x20000, CRC(a7e7d44d) SHA1(d26126310b8b316ca161d4202645de8fb6359822))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpuu.200", 0x0000, 0x10000, CRC(4528e803) SHA1(0ebb16ab8b95f04a19fa4510e58c01493393d48c))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("trekdspa.109", 0x00000, 0x20000, CRC(a7e7d44d) SHA1(d26126310b8b316ca161d4202645de8fb6359822))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_120)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.120", 0x0000, 0x10000, CRC(2cac0731) SHA1(abf68c358c50bdeb36714cca0a9848e398a6f9fc))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("trekdsp.106", 0x00000, 0x20000, CRC(dc3bf312) SHA1(3262d6604d1dcd1dc738bc3f919a3319b783fd73))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_110)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.110", 0x0000, 0x10000, CRC(06e0f87b) SHA1(989d70e067cd322351768550549a4e2c8923132c))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("trekdsp.106", 0x00000, 0x20000, CRC(dc3bf312) SHA1(3262d6604d1dcd1dc738bc3f919a3319b783fd73))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_11a)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.110", 0x0000, 0x10000, CRC(06e0f87b) SHA1(989d70e067cd322351768550549a4e2c8923132c))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("trekadsp.bin", 0x00000, 0x20000, CRC(54681627) SHA1(4251fa0568d2e869b44358471a3d4a4e88443954))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

/*-------------------------------------------------------------
/ Star Wars - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(stwr_104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.104", 0x0000, 0x10000, CRC(12b87cfa) SHA1(12e0ab52f6784beefce8291d29b8aff01b2f2818))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.103", 0x0000, 0x10000, CRC(318085ca) SHA1(7c35bdee52e8093fe05f0624615baabe559a1917))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_g11)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpug.101", 0x0000, 0x10000, CRC(c74b4576) SHA1(67db9294cd802be8d62102fe756648f750821960))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("swdsp_g.102", 0x00000, 0x80000, CRC(afdfbfc4) SHA1(1c3cd90b9cd4f88ee2b556abef863a0ae9a10056))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_a14)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.103", 0x0000, 0x10000, CRC(318085ca) SHA1(7c35bdee52e8093fe05f0624615baabe559a1917))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("swrom1.a14", 0x00000, 0x40000, CRC(4d577828) SHA1(8b1f302621fe2ee13a067b9c97e3dc33f4519cea))
	ROM_LOAD("swrom0.a14", 0x40000, 0x40000, CRC(104e5a6b) SHA1(b6a9e32f8aec078665faf2ba9ba4f9f51f68cea8))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_102)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.102", 0x0000, 0x10000, CRC(8b9d90d6) SHA1(2fb7594e6f4aae1dc3a07192546fabd2901acbed))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_e12)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpue.102", 0x0000, 0x10000, CRC(b441abd3) SHA1(42cab6e16be8e25a68b2db30f53ba516bbb8741d))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

/*-------------------------------------------------------------
/ Tales From the Crypt - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(tftc_303)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.303", 0x0000, 0x10000, CRC(e9bec98e) SHA1(02643805d596017c88d9a534b94b2075bb2ab101))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tftcdspa.301", 0x00000, 0x80000, CRC(3888d06f) SHA1(3d276df436a76c6e9bed6629114204dacd88245b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END
ROM_START(tftc_302)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.302", 0x0000, 0x10000, CRC(a194fe0f) SHA1(b83e048300f7e072f76672d72cdf43e43fab2e9e))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tftcdspa.301", 0x00000, 0x80000, CRC(3888d06f) SHA1(3d276df436a76c6e9bed6629114204dacd88245b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END
ROM_START(tftc_300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.300", 0x0000, 0x10000, CRC(3d275152) SHA1(0aa6df629c27d9265cf35ca0724e241d9820e56b))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tftcdspa.300", 0x00000, 0x80000, CRC(bf5c812b) SHA1(c10390b6cad0ad457fb83241c7ee1d6b109cf5be))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

ROM_START(tftc_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftcgc5.a20", 0x0000, 0x10000, CRC(94b61f83) SHA1(9f36353a06cacb8ad67f70cd8d9d8ac698905ba3))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tftcdot.a20", 0x00000, 0x80000, CRC(16b3968a) SHA1(6ce91774fc60187e4b0d8874a14ef64e2805eb3f))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

ROM_START(tftc_104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.104", 0x0000, 0x10000, CRC(efb3c0d0) SHA1(df1505947732704171e31dbace4c263723c8342b))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tftcdspl.103", 0x00000, 0x80000, CRC(98f3b13e) SHA1(909c373b1a27b5aeebad2535ae4fb9bba71e9b5c))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

/*-----------------------------------------------------------------------------
/ Teenage Mutant Ninja Turtles - CPU Rev 3 /DMD Type 1 64K Rom 16/32K CPU Roms
/-----------------------------------------------------------------------------*/
ROM_START(tmnt_104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5a.104", 0x4000, 0x4000, CRC(f508eeee) SHA1(5e67fde49f6e7d5d563645df9036d5691be076cf))
	ROM_LOAD("tmntc5a.104", 0x8000, 0x8000, CRC(a33d18d4) SHA1(41cf815c1f3d117efe0ddd14ad84076dcb80318a))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("tmntdsp.104", 0x00000, 0x10000, CRC(545686b7) SHA1(713df7820d024db3406f5e171f62a53e34474f70))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

ROM_START(tmnt_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5.103", 0x4000, 0x4000, CRC(fcc6c5b0) SHA1(062bbc93de0f8bb1921da4d756a13923f23cf5d9))
	ROM_LOAD("tmntc5.103", 0x8000, 0x8000, CRC(46b68ecc) SHA1(cb94041017c0856f1e15de05c70369cb4f8756cd))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x400, "user3", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD("tmntdsp.104", 0x00000, 0x10000, CRC(545686b7) SHA1(713df7820d024db3406f5e171f62a53e34474f70))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

/*-------------------------------------------------------------
/ The Who's Tommy Pinball Wizard - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(tomy_400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpua.400", 0x0000, 0x10000, CRC(d0310a1a) SHA1(5b14f5d6e271676b4ec93b64f1cde9607844b677))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tommydva.400", 0x00000, 0x80000, CRC(9e640d09) SHA1(d921fadeb728cf929c6bae2e79bd4d140192a4d2))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_h30)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpuh.300", 0x0000, 0x10000, CRC(121b5932) SHA1(e7d7bf8a78baf1c00c8bac908d4646586b8cf1f5))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("tommydva.300", 0x00000, 0x80000, CRC(1f2d0896) SHA1(50c617e30bb843c69a6ca8afeeb751c886f5e6bd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

/*-------------------------------------------------------------
/ WWF Royal Rumble - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(wwfr_106)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wwfcpua.106", 0x0000, 0x10000, CRC(5f1c7da2) SHA1(9188e0b9c26e4b6c92c63a58b52ee42bd3b77ca0))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("wwfdispa.102", 0x00000, 0x80000, CRC(4b629a4f) SHA1(c301d0c785f7bc4d3c23cbda76ff955c742eaeef))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("wfsndu7.512", 0x0000, 0x10000, CRC(eb01745c) SHA1(7222e39c52ed298b737aadaa5b57d2068d39287e))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("wfsndu17.400", 0x000000, 0x80000, CRC(7d9c2ca8) SHA1(5d84559455fe7e27634b28bcab81d54f2676390e))
	ROM_LOAD("wfsndu21.400", 0x080000, 0x80000, CRC(242dcdcb) SHA1(af7220e14b0956ef40f75b2749eb1b9d715a1af0))
	ROM_LOAD("wfsndu36.400", 0x100000, 0x80000, CRC(39db8d85) SHA1(a55dd88fd4d9154b523dca9160bf96119af1f94d))
ROM_END
ROM_START(wwfr_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wfcpuc5.512", 0x0000, 0x10000, CRC(7e9ead89) SHA1(6cfd64899128b5f9b4ccc37b7bfdbb0a2a75a3a5))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "gfx3", 0)
	ROM_LOAD("wfdisp0.400", 0x00000, 0x80000, CRC(e190b90f) SHA1(a0e73ce0b241a81e935e6790e04ea5e1fccf3742))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("wfsndu7.512", 0x0000, 0x10000, CRC(eb01745c) SHA1(7222e39c52ed298b737aadaa5b57d2068d39287e))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("wfsndu17.400", 0x000000, 0x80000, CRC(7d9c2ca8) SHA1(5d84559455fe7e27634b28bcab81d54f2676390e))
	ROM_LOAD("wfsndu21.400", 0x080000, 0x80000, CRC(242dcdcb) SHA1(af7220e14b0956ef40f75b2749eb1b9d715a1af0))
	ROM_LOAD("wfsndu36.400", 0x100000, 0x80000, CRC(39db8d85) SHA1(a55dd88fd4d9154b523dca9160bf96119af1f94d))
ROM_END


GAME(1993,  rab_320,        0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Adventures of Rocky and Bullwinkle and Friends (3.20)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  rab_130,        rab_320,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Adventures of Rocky and Bullwinkle and Friends (1.30)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  rab_103,        rab_320,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Adventures of Rocky and Bullwinkle and Friends (1.03 Spain)",  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  aar_101,        0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Aaron Spelling (1.01)",                                        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  btmn_103,       0,          de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Batman (1.03)",                                                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  btmn_101,       btmn_103,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Batman (1.01)",                                                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  btmn_g13,       btmn_103,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Batman (1.03 Germany)",                                        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  btmn_106,       btmn_103,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Batman (1.06)",                                                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  ckpt_a17,       0,          de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Checkpoint (1.7)",                                             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  gnr_300,        0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Guns N Roses (3.00)",                                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  hook_408,       0,          de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Hook (4.08)",                                                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  hook_401,       hook_408,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Hook (4.01)",                                                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  hook_404,       hook_408,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Hook (4.04)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  jupk_513,       0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Jurassic Park (5.13)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  jupk_501,       jupk_513,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Jurassic Park (5.01)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  jupk_g51,       jupk_513,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Jurassic Park (5.01 Germany)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  lah_112,        0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Last Action Hero (1.12)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  lah_l104,       lah_112,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Last Action Hero (1.04 Spain)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  lah_l108,       lah_112,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Last Action Hero (1.08 Spain)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  lah_110,        lah_112,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Last Action Hero (1.10)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  lw3_208,        0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Lethal Weapon 3 (2.08)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  lw3_207,        lw3_208,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Lethal Weapon 3 (2.07 Canada)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  lw3_205,        lw3_208,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Lethal Weapon 3 (2.05)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  lw3_200,        lw3_208,    de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Lethal Weapon 3 (2.00)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  trek_201,       0,          de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Trek 25th Anniversary (2.01)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  trek_200,       trek_201,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Trek 25th Anniversary (2.00)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  trek_120,       trek_201,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Trek 25th Anniversary (1.20)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  trek_110,       trek_201,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Trek 25th Anniversary (1.10)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  trek_11a,       trek_201,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Trek 25th Anniversary (1.10 Alpha Display)",  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_104,       0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (1.04)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_103,       stwr_104,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (1.03)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_g11,       stwr_104,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (1.01 Germany)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_a14,       stwr_104,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (Display Rev.1.04)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_102,       stwr_104,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (1.02)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  stwr_e12,       stwr_104,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Star Wars (1.02 England)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  tftc_303,       0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Tales From the Crypt (3.03)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  tftc_302,       tftc_303,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Tales From the Crypt (3.02 Dutch)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  tftc_300,       tftc_303,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Tales From the Crypt (3.00)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  tftc_200,       tftc_303,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Tales From the Crypt (2.00)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  tftc_104,       tftc_303,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Tales From the Crypt (1.04 Spain)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  tmnt_104,       0,          de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Teenage Mutant Ninja Turtles (1.04)",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  tmnt_103,       tmnt_104,   de_3_dmd1,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "Teenage Mutant Ninja Turtles (1.03)",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  tomy_400,       0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "The Who's Tommy Pinball Wizard (4.00)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  tomy_h30,       tomy_400,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "The Who's Tommy Pinball Wizard (3.00, The Netherlands)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  wwfr_106,       0,          de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "WWF Royal Rumble (1.06)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  wwfr_103,       wwfr_106,   de_3_dmd2,   de_3, de_3_state,   de_3,   ROT0,   "Data East",    "WWF Royal Rumble (1.03)",              MACHINE_IS_SKELETON_MECHANICAL)
