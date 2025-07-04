// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***********************************************************************************************************

    'High Rate DVD' HW (c) 1998 Nichibutsu

    preliminary driver by Angelo Salese

    TODO:
    - Implement DVD routing and YUV decoding;
    - game timings seem busted, could be due of missing DVD hook-up
    - csplayh1: inputs doesn't work at all, slower than the others too.
      Probably not a DVD but CD rom game?


    DVD Notes:
    - h8 board components:
      H8/3002
      MN7100 8-bit channel data acquisition system
      Fujitsu MD0208
      Zoran ZR36110PQC (mpeg ps/video decoder)
      Nippon Steel Corp NN71003F (mpeg audio decoder)
      uPD6379A dual 16-bits DAC
      Toshiba TC9223 PLL
      IDE and RS232c ports
      xtal 27 MHz (dvd board), 12.288MHz (main board)

    H8 ports directions:
      8: fe /5 ---ooooi
      9: c3 /6 --iiiioo
      a: ff /8 oooooooo
      b: 3f /8 iioooooo

***********************************************************************************************************/

#include "emu.h"

#include "nichisnd.h"

#include "bus/ata/atadev.h"
#include "bus/ata/atapicdr.h"
#include "bus/ata/ataintf.h"
#include "cpu/h8/h83002.h"
#include "cpu/m68000/tmp68301.h"
#include "machine/nvram.h"
#include "machine/tc9223.h"
#include "machine/timer.h"
#include "sound/nn71003f.h"
#include "video/v9938.h"
#include "video/zr36110.h"

#include "speaker.h"


class hrdvd_ata_controller_device : public abstract_ata_interface_device
{
public:
	hrdvd_ata_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t dma_read();
	void dma_write(uint16_t data);

	uint16_t read(offs_t offset, uint16_t mem_mask);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask);
};

DEFINE_DEVICE_TYPE(HRDVD_ATA_CONTROLLER_DEVICE, hrdvd_ata_controller_device, "hrdvd_atactrl", "High Rate DVD ATA controller device")

class hrdvd_state : public driver_device
{
public:
	hrdvd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_ata(*this, "ata"),
		m_video(*this, "v9958"),
		m_mpeg(*this, "mpeg"),
		m_mpega(*this, "mpeg_audio"),
		m_pll(*this, "pll"),
		m_nichisnd(*this, "nichisnd"),
		m_speaker(*this, "speaker"),
		m_screen(*this, "screen"),
		m_key(*this, "KEY.%u", 0),
		m_region_maincpu(*this, "maincpu")
	{ }

	required_device<tmp68301_device> m_maincpu;
	required_device<h83002_device> m_subcpu;
	required_device<hrdvd_ata_controller_device> m_ata;
	required_device<v9958_device> m_video;
	required_device<zr36110_device> m_mpeg;
	required_device<nn71003f_device> m_mpega;
	required_device<tc9223_device> m_pll;
	required_device<nichisnd_device> m_nichisnd;
	required_device<speaker_device> m_speaker;
	required_device<screen_device> m_screen;
	required_ioport_array<5> m_key;
	required_memory_region m_region_maincpu;

	uint16_t m_mux_data;

	uint8_t m_p6, m_pa, m_pb;

	bool m_mpeg_dreq;

	void mpeg_dreq_w(int state);

	uint8_t p6_r();
	void p6_w(uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	void pa_w(uint8_t data);

	uint8_t cs0_r(offs_t offset);
	void cs0_w(offs_t offset, uint8_t data);

	uint16_t hrdvd_mux_r();
	void hrdvd_mux_w(uint16_t data);
	void tmp68301_parallel_port_w(uint16_t data);

	void ata_irq(int state);
	void ata_drq(int state);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void general_init(int patchaddress, int patchvalue);
	void hrdvd(machine_config &config);
	void hrdvd_map(address_map &map) ATTR_COLD;
	void hrdvd_sub_map(address_map &map) ATTR_COLD;

	static void dvdrom_config(device_t *device);
};

void hrdvd_state::mpeg_dreq_w(int state)
{
	m_mpeg_dreq = state;
	m_subcpu->set_input_line(H8_INPUT_LINE_DREQ0, m_mpeg_dreq && !(m_p6 & 0x04));
}

uint8_t hrdvd_state::p6_r()
{
	return m_p6;
}

void hrdvd_state::p6_w(uint8_t data)
{
	u8 delta = data ^ m_p6;
	m_p6 = data;
	if(delta & 0x02)
		m_mpeg->reset();

	m_subcpu->set_input_line(H8_INPUT_LINE_DREQ0, m_mpeg_dreq && !(m_p6 & 0x04));

	logerror("p6 %02x\n", m_p6);
}

uint8_t hrdvd_state::pb_r()
{
	return m_pb;
}

void hrdvd_state::pb_w(uint8_t data)
{
	u8 delta = data ^ m_pb;
	m_pb = (m_pb & 0xc0) | (data & 0x3f);
	m_mpega->ss_w(BIT(m_pb, 0));
	m_mpega->sclk_w(BIT(m_pb, 1));
	m_mpega->mosi_w(BIT(m_pb, 2));
	if(delta & 0x38)
		logerror("pb %02x\n", data);
}

void hrdvd_state::pa_w(uint8_t data)
{
	u8 delta = data ^ m_pa;
	m_pa = data;
	m_pll->stb_w(BIT(m_pa, 5));
	m_pll->dat_w(BIT(m_pa, 6));
	m_pll->clk_w(BIT(m_pa, 7));
	if(delta & 0x1f)
		logerror("pa %02x\n", data);
}

uint16_t hrdvd_state::hrdvd_mux_r()
{
	for(int i=0;i<5;i++)
	{
		if(m_mux_data & 1 << i)
			return m_key[i]->read();
	}

	popmessage("Multiple bytes used for mux %02x",m_mux_data);

	return 0xffff;
}

void hrdvd_state::hrdvd_mux_w(uint16_t data)
{
	m_mux_data = (~data & 0x1f);
}

hrdvd_ata_controller_device::hrdvd_ata_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abstract_ata_interface_device(mconfig, HRDVD_ATA_CONTROLLER_DEVICE, tag, owner, clock)
{
}

uint16_t hrdvd_ata_controller_device::dma_read()
{
	write_dmack(1);
	uint16_t r = swapendian_int16(read_dma());
	write_dmack(0);
	return r;
}

void hrdvd_ata_controller_device::dma_write(uint16_t data)
{
	write_dmack(1);
	write_dma(swapendian_int16(data));
	write_dmack(0);
}

uint16_t hrdvd_ata_controller_device::read(offs_t offset, uint16_t mem_mask)
{
	if(mem_mask == 0xffff)
		return swapendian_int16(internal_read_cs0(offset * 2, 0xffff));
	else if(ACCESSING_BITS_0_7)
		return internal_read_cs0(offset * 2 + 1, 0xff);
	else
		return internal_read_cs0(offset * 2, 0xff) << 8;
}

void hrdvd_ata_controller_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(mem_mask == 0xffff)
		internal_write_cs0(offset * 2, swapendian_int16(data), 0xffff);
	else if(ACCESSING_BITS_0_7)
		internal_write_cs0(offset * 2 + 1, data, 0xff);
	else
		internal_write_cs0(offset * 2, data >> 8, 0xff);
}


void hrdvd_state::hrdvd_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x200000, 0x200001).portr("DSW");
	map(0x200000, 0x200000).w(m_nichisnd, FUNC(nichisnd_device::sound_host_command_w));
	map(0x200200, 0x200201).rw(FUNC(hrdvd_state::hrdvd_mux_r), FUNC(hrdvd_state::hrdvd_mux_w));
	map(0x200400, 0x200401).portr("SYSTEM");

	map(0x200600, 0x200607).rw("v9958", FUNC(v9958_device::read), FUNC(v9958_device::write)).umask16(0x00ff);

	map(0x800000, 0xbfffff).rom().region("blit_gfx", 0); // GFX ROM routes here

	map(0xc80000, 0xcbffff).ram(); // work RAM

	map(0xe80000, 0xefffff).ram(); // work RAM
}

void hrdvd_state::hrdvd_sub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	map(0x020008, 0x020009).rw(m_mpeg, FUNC(zr36110_device::stat0x_r), FUNC(zr36110_device::setupx_w));
	map(0x02000a, 0x02000b).rw(m_mpeg, FUNC(zr36110_device::stat1x_r), FUNC(zr36110_device::mc1x_w));
	map(0x02000c, 0x02000d).rw(m_mpeg, FUNC(zr36110_device::stat2x_r), FUNC(zr36110_device::cmdx_w));
	map(0x02000e, 0x02000f).rw(m_mpeg, FUNC(zr36110_device::userx_r),  FUNC(zr36110_device::mc23x_w));
	map(0x020010, 0x02001f). w(m_mpeg,                                 FUNC(zr36110_device::dmax_w));

	map(0x040018, 0x040019).rw(m_ata, FUNC(hrdvd_ata_controller_device::dma_read), FUNC(hrdvd_ata_controller_device::dma_write));
	map(0x040028, 0x04002f).rw(m_ata, FUNC(hrdvd_ata_controller_device::read), FUNC(hrdvd_ata_controller_device::write));

	map(0x060000, 0x07ffff).mirror(0xf80000).ram(); //.share("nvram");
}


static INPUT_PORTS_START( hrdvd )
	PORT_START("KEY.0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY.1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY.2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY.3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY.4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	// comes from csplayh5 manual, other games might change slightly
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(      0x0007, "1" )
	PORT_DIPSETTING(      0x0006, "2" )
	PORT_DIPSETTING(      0x0005, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Show girls in attract mode" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Color Test" ) PORT_DIPLOCATION("SWA:7") // shows color bars during POST
	PORT_DIPSETTING(      0x0040, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SWA:8" )

	// A note indicates inoshikachou, shichigosan, hanami de ippai, tsukimi de ippai to be used, which are Koi Koi rulesets
	PORT_DIPNAME( 0x0100, 0x0100, "Use Koi Koi local ruleset" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0200, 0x0000, "Koi Koi input layout" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(      0x0000, "A Type" ) // Yes: Riichi No: Ron
	PORT_DIPSETTING(      0x0200, "B Type" ) // Yes: M No: N
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SWB:3" )
	// TODO: duplicate of SWA:5? Maybe applies to nudity, will see once the DVD part works.
	PORT_DIPNAME( 0x1800, 0x1800, "Background type" ) PORT_DIPLOCATION("SWB:4,5")
//  PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, "Color in-game and attract" )
	PORT_DIPSETTING(      0x1000, "Girls in-game, color in attract" )
	PORT_DIPSETTING(      0x1800, "Girls in-game and attract" )
	PORT_DIPNAME( 0x2000, 0x2000, "Analyzer" ) PORT_DIPLOCATION("SWB:6") //in some games
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( No ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Credit Clear")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) // labeled analyzer in self-test
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4) PORT_NAME("Out Coin")
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void hrdvd_state::machine_start()
{
	save_item(NAME(m_mux_data));
	save_item(NAME(m_p6));
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_mpeg_dreq));
}

void hrdvd_state::machine_reset()
{
	m_mux_data = 0;
	m_p6 = 0;
	m_pa = 0;
	m_pb = 0;
	m_mpeg_dreq = false;
}

void hrdvd_state::ata_irq(int state)
{
	//  logerror("ata irq %d\n", state);
}

void hrdvd_state::ata_drq(int state)
{
	//  logerror("ata drq %d\n", state);
	m_pb = (m_pb & 0x7f) | (state ? 0x00 : 0x80);
	m_subcpu->set_input_line(H8_INPUT_LINE_DREQ1, state);
}

void hrdvd_state::tmp68301_parallel_port_w(uint16_t data)
{
	/*
	    -x-- ---- used during ROM check, h8 reset assert?
	    ---- x--- enable DVD sound? Used by aimode at very least
	*/

	logerror("tmp: %02x\n", data);
}

static void atapi_devs(device_slot_interface &device)
{
	device.option_add("dvdrom", ATAPI_DVDROM);
}

void hrdvd_state::dvdrom_config(device_t *device)
{
	auto *drive = downcast<atapi_dvdrom_device *>(device);
	drive->set_model("PIONEER        DVD-A01  1.17"); // Wants firmware version between 1.14 and 1.19
}

void hrdvd_state::hrdvd(machine_config &config)
{
	/* basic machine hardware */
	TMP68301(config, m_maincpu, 12.288_MHz_XTAL); /* TMP68301-16 */
	m_maincpu->set_addrmap(AS_PROGRAM, &hrdvd_state::hrdvd_map);
	m_maincpu->parallel_w_cb().set(FUNC(hrdvd_state::tmp68301_parallel_port_w));

	H83002(config, m_subcpu, 27_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &hrdvd_state::hrdvd_sub_map);
	m_subcpu->read_port6().set(FUNC(hrdvd_state::p6_r));
	m_subcpu->write_port6().set(FUNC(hrdvd_state::p6_w));
	m_subcpu->write_porta().set(FUNC(hrdvd_state::pa_w));
	m_subcpu->read_portb().set(FUNC(hrdvd_state::pb_r));
	m_subcpu->write_portb().set(FUNC(hrdvd_state::pb_w));

	m_maincpu->tx0_handler().set(m_subcpu, FUNC(h83002_device::sci_rx_w<0>));
	m_subcpu->write_sci_tx<0>().set(m_maincpu, FUNC(tmp68301_device::rx0_w));

	HRDVD_ATA_CONTROLLER_DEVICE(config, m_ata).options(atapi_devs, "dvdrom", nullptr, true);
	m_ata->slot(0).set_option_machine_config("dvdrom", dvdrom_config);
	m_ata->irq_handler().set(FUNC(hrdvd_state::ata_irq));
	m_ata->dmarq_handler().set(FUNC(hrdvd_state::ata_drq));

	//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video & sound hardware */
	TC9223(config, m_pll);

	V9958(config, m_video, XTAL(21'477'272)); // typical 9958 clock, not verified
	m_video->set_screen_ntsc(m_screen);
	m_video->set_vram_size(0x20000);
	m_video->int_cb().set_inputline(m_maincpu, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	ZR36110(config, m_mpeg, 27_MHz_XTAL/2);
	m_mpeg->drq_w().set(FUNC(hrdvd_state::mpeg_dreq_w));

	NN71003F(config, m_mpega, 0);
	m_mpega->add_route(0, m_speaker, 1.0, 0);
	m_mpega->add_route(1, m_speaker, 1.0, 1);
	m_mpeg->sp2_frm_w().set(m_mpega, FUNC(nn71003f_device::frm_w));
	m_mpeg->sp2_clk_w().set(m_mpega, FUNC(nn71003f_device::clk_w));
	m_mpeg->sp2_dat_w().set(m_mpega, FUNC(nn71003f_device::dat_w));

	NICHISND(config, m_nichisnd, 0);

	SPEAKER(config, m_speaker, 2).front();
}


/*
 * Base BIOS root (DVD board is common for all DVD games)
 */

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

#define DVD_BIOS \
	ROM_REGION( 0x20000, "subcpu", 0 ) \
	ROM_SYSTEM_BIOS( 0,  "vb102",    "va1b102" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "va1b102.u2",   0x00000, 0x20000, CRC(67374715) SHA1(8767cbd81614c2686a1adb70159f909e8ffd634d) ) \
	ROM_SYSTEM_BIOS( 1,  "vb101",    "va1b101" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "va1b101.u2",   0x00000, 0x20000, CRC(b92a83c8) SHA1(bd6d9adaa74cf7512478838d1bd5a79dbd0c4aa1) ) \
	ROM_SYSTEM_BIOS( 2,  "va101",    "va1a101" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "va1a101.u2",   0x00000, 0x20000, CRC(36135792) SHA1(1b9c50bd02df8227b228b35cc485efd5a13ec639) )


// dummy ROM definition
ROM_START( nichidvd )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 ) // tmp68301 prg

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", ROMREGION_ERASE00 ) // z80

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs

	DISK_REGION( "ata:0:dvdrom" )
ROM_END

// TODO: this surely uses a different subboard
ROM_START( csplayh1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "3.bin", 0x000000, 0x020000, CRC(86ac0289) SHA1(7ae3047fc7ea22705cc5b04d0ec6c792c429e8ee) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x020000, CRC(1f056e64) SHA1(7c5fb318abcd87313ef739dec191af9bcf284f24) )

	ROM_REGION( 0x20000, "subcpu", 0 ) // h8, cd-rom player
	ROM_LOAD16_WORD_SWAP( "u2",   0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "1.bin", 0x000000, 0x020000, CRC(8296d67f) SHA1(20eb944a2bd27980e1aaf60ca544059e84129760) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "4.bin", 0x000001, 0x080000, CRC(2e63ee15) SHA1(78fefbc277234458212cded997d393bd8b82cf76) )
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x080000, CRC(a8567f1b) SHA1(2a854ef8b1988ad097bbcbeddc4b275ad738e1e1) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "csplayh1", 0, SHA1(d6514882c2626e62c5079df9ac68ecb70fc33209) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.ic8", 0x000000, 0x0008c1, NO_DUMP )
ROM_END

ROM_START( mjgalpri )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(e8427076) SHA1(9b449599ffac2b67a29fac11d1e85218668d805d) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(653fcc14) SHA1(6231ec5f45a9f5e587dcd00ff85f9bbfae7364ab) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(7b9b1887) SHA1(1393a1d79f3cc7ab68275791af4ec16e825056df) )

	DVD_BIOS

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(6497bc8f) SHA1(ce0ecfab8df87f7356aa42648e47ffda53840188) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(3ac982e8) SHA1(d889d45888cf7bcb5af808f63e9ad41204bd5992) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.020", 0x000000, 0x040000, CRC(4c92a523) SHA1(51da73fdfdfccdc070fa8a13163e031438b50876) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8001", 0, SHA1(7860688f8152ba7fdaeef5fec9eeb8e85c90b9a5) )
ROM_END

ROM_START( sengomjk )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(a202bf13) SHA1(01e15e7577f6ac6a90b7ab30f402def211360d4d) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(98d4979a) SHA1(477361ec183674220e282fed8bfce098b0f75873) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(c0bf69c6) SHA1(dd06ec9b3232f025de2c87765b88cb101eab47f5) )

	DVD_BIOS

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(20791a5a) SHA1(03c38e9b8e60b0dded7504b2725210df5405110c) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(1ed72387) SHA1(7e2b8ce49561d6fd79dcf0d427569e5f6ef8dc67) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8002", 0, SHA1(0fac6bbe52bc7ed56ce786ed0690f7e150fc4f3e) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.bin", 0x000000, 0x000117, CRC(9f0dec43) SHA1(d836e564da496c3049e16f025daf362cced413d4) )
ROM_END

ROM_START( junai )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(5923bf2e) SHA1(8fc7b95a44eb792ce03c1bffb9ad56f82d34b470) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(4ac649ee) SHA1(f5b5bccecb6eba5addcf6a57e54deff7f29f6381) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(a0472ea5) SHA1(0fd04941ff595cffe64357f3a1a9dc1170db8703) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(f17fa4c4) SHA1(fd8b69b18f9ac00f468d751bf1ea0715498ea742) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(4182dc30) SHA1(89601c62b74aff3d65b075d4b5cd1eb2ccf4e386) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8003", 0, SHA1(098925d09cfdd2c970edf40054b36423f6e71a42) )
ROM_END

ROM_START( csplayh5 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(980bf3b0) SHA1(89da7354552f30aaa9d46442972c060b4b0f8979) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(81ca49a4) SHA1(601b6802ab85be61f45a64f5b4c7e1f1ae5ee887) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(0b920806) SHA1(95f50ebfb296ba29aaa8079a41f5362cb9e879cc) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(895b5e1f) SHA1(9398ee95d391f74d62fe641cb75311f31d4d1c8d) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(113d7e96) SHA1(f3fb9c719544417a6a018b82f07c65bf73de21ff) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8004", 0, SHA1(51520ef06125b9ae45c2fb0ce0e9d554dd98bfd9) )
ROM_END

ROM_START( junai2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(b0ce71d6) SHA1(35cff8f8b18312808e2f6b96f02d952b0d1f04a1) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(5a428e91) SHA1(dffce6f0a48cc4110970f124684dcaa267fe1b7f) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(a4b07757) SHA1(5010f28d7a80af0cc3f4fd135f777950fb2cf679) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(95ecb29d) SHA1(e07bb0ff15aaee9fb26d8ef7f4644b47045c81a8) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(5b37c8dd) SHA1(8de5e2f92721c6679c6506850a442cafff89653f) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8005", 0, SHA1(ec11caa96833b80324e68f0345b000d702eaf6cb) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.ic8", 0x000000, 0x0008c1, BAD_DUMP CRC(01c2895a) SHA1(782166a60fa14d5faa5a92629f7ca65a878ad7fe) )
ROM_END


ROM_START( mogitate )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(f71546c6) SHA1(546b0d12e7b1627c96d5a17c015bdbbca1e93232) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(42ec6c2e) SHA1(a0279502e1f7e62f072ec6612caf198aa0ae3af7) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(7927c1d6) SHA1(15f0c0051124e7b7667eb721dd12938333b31899) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(ea655990) SHA1(7f59cfab21e8858625e82a9501acc943b07f799c) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(4c910b86) SHA1(48007f03f4e445b9de15531afe821c1b18fccae1) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8006", 0, SHA1(334c37710affc852bec750721280488530b3d1e0) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.020", 0x000000, 0x040000, CRC(ac5c9495) SHA1(1c54ecf6dedbf8c3a29207c1c91b52e2ff394d9d) )
ROM_END

ROM_START( mjmania )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(7b0f79c5) SHA1(73f23f68db4426b32583a7922abf773d67c76862) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(19192ae7) SHA1(4e9fca04b567c8ef9136a3ab87b21207a44a24c4) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(f0c3bb11) SHA1(691a0ff53a9417e69051e9e2bdee7500bc6a746b) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(37dde764) SHA1(0530b63d8e682cdf01128057fdc3a8c23262afc9) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(dea4a2d2) SHA1(0118eb1330c9da8fead99f64fc015fd343fed79b) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8007", 0, SHA1(777668854058b1586f599d266d1cea55c35aa30d) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.ic8", 0x000000, 0x0008c1, BAD_DUMP CRC(6a92b563) SHA1(a6c4305cf021f37845f99713427daa9394b6ec7d) )
ROM_END

ROM_START( renaimj )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(5455e94c) SHA1(97257ed020848611bf9f9637f1eb9ee3433a6e20) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(285a5651) SHA1(c572a7c82759600e29e31518c69b17ae173c2263) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(614d17b9) SHA1(d6fb4441f55902c2b89b4bec53aae5311d81f07b) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(790aa63d) SHA1(d94b88084311f317d584a33ad5b483403f2bf226) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(6d1c9efd) SHA1(c9ea9d6e6d34db5635fc55d41e7bb54a41948d27) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8008", 0, SHA1(8dd445030e287f9e4380277d2163c4163182ab51) )

	ROM_REGION( 0x40000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal18v8b.020", 0x000000, 0x040000, CRC(0a32a144) SHA1(f3b4a1174adbb2f7b7500adeafa20142f6a16d08) )
ROM_END

ROM_START( bikiniko )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(b80b5484) SHA1(35769d9502cbe587dad6380c35e535cea1578227) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(13a885af) SHA1(ba8221fab1a37f1937e4399eabe3eaa9093884d3) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(4a2142d6) SHA1(3a762f7b7cccdb6715b5f59524b04b12694fc130) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(12914d3b) SHA1(de0cdb47ee5cbf8bd19ab19b1b8d8afe103dcedf) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(1e2e1cf3) SHA1(f71b5dedf4f897644d519e412651152d0d81edb8) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8009", 0, SHA1(cac9a6c7fe86751c968cd15f7b779edbc14d5f0b) )
ROM_END

ROM_START( csplayh6 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(12d896cc) SHA1(7d602b44cb781dbc52b112c9f1a5d88a332dfbe0) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(1e4679ca) SHA1(f5df03c07f749906bbcef26a4a5d433564d4aeb8) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(3ce03f2d) SHA1(5ccdcac8bad25b4f680ed7a2074575711c25af41) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(a09e7575) SHA1(76f4d7562a3fd479c1c6de22f704a0953a39bb0c) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(858e0604) SHA1(64c23bc06898188798937770129697b3c4b547d6) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8010", 0, SHA1(5c3fab40d0cce94e80f2bdd533569d5507496ad4) )

	ROM_REGION( 0x40000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h.020_bad", 0x000000, 0x040000, BAD_DUMP CRC(2aec4e37) SHA1(79d64394c0f6f2c5e17ae9fc62eaa279da466ccd) )
ROM_END

ROM_START( thenanpa )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(ab0b686f) SHA1(a5681dbacbc60f3eb40e079779967cf69d9cb292) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(48b65f9a) SHA1(ce35475d3b0e9e8dc69892428f3957d8d3d5f22c) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(f44c4095) SHA1(d43e464bd6d614c34791445f8fd4af2f62a4dfc2) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(ee6b88c4) SHA1(64ae66a24f1639801c7bdda7faa0d604bb97ceb1) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(ce987845) SHA1(2f7dca32a79ad6afbc55ca1d492b582f952688ff) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8011", 0,  SHA1(730848295802b8596929ffd22b81712ec4ea30a6) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.ic8", 0x000000, 0x0008c1, BAD_DUMP CRC(daffd0ac)SHA1(cbeff914163d425a9cb30fe8d62f91fca281b11f) )
ROM_END

ROM_START( pokoachu )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(db63c2c3) SHA1(528b0eead52e54af0c5accb5f96a382b1f9b7123) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(789ffbc8) SHA1(44f3846414682e19465b485ffb89c7b78920cb0a)  )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(9d344bad) SHA1(276c8066a2b5090edf6ba00843b7a9496c90f99f) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(843c288e) SHA1(2741b9da83fd35c7472b8c67bc02313a1c5e4e25) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(6920a9b8) SHA1(0a4cb9e2a0d871aed60c1293b7cac4bf79a9446c) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8012", 0, SHA1(be6450351d6da4c96cbff353d587980a2728e306) )

	ROM_REGION( 0x40000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.020", 0x000000, 0x040000, CRC(ac5c9495) SHA1(1c54ecf6dedbf8c3a29207c1c91b52e2ff394d9d) )
ROM_END

ROM_START( csplayh7 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(c5ce76a6) SHA1(f8878285d2318c1ec50ba98607eb3f15a7f69913) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(162f8cff) SHA1(8aa185fd1daa943d0b21fdf6e692f7782bc6dac4) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(5905b199) SHA1(9155455bc21d23d439c4732549ff1143ee17b9d3) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(1d67ca95) SHA1(9b45045b6fa67308bade324f91c21010aa8d121e) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(b4f5f990) SHA1(88cccae04f89fef43d88f4e82b65de3de946e9af) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8013", 0, SHA1(e559c0667e5c55518442b874f6538b937e2e84b9) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "mjdvd12.gal16v8b.ic8.bin", 0x000000, 0x0008c1, BAD_DUMP CRC(6a92b563)SHA1(a6c4305cf021f37845f99713427daa9394b6ec7d) )
ROM_END

ROM_START( aimode )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(fd7fda98) SHA1(d938391cc99d9ffdb427ec491403f81d14e09f5a) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(c86765a8) SHA1(924831c07191e046beec79dd1da30c1944cfe57c) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(e6404950) SHA1(bb179c27ce65f7dc58d2aeed4710347e7953e11c) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(4a9863cf) SHA1(ccf08befe773fb94fa78423ed19b6b8d255ca3a7) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(893aac1a) SHA1(14dd3f07363858c2be3a9400793f720b1f5baf1a) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8014", 0, SHA1(f2aea46961630328d82ffc221c4b817b20fa9f5c) )

	ROM_REGION( 0x40000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.020", 0x000000, 0x040000, CRC(0a32a144) SHA1(f3b4a1174adbb2f7b7500adeafa20142f6a16d08) )
ROM_END

ROM_START( fuudol )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(b1fa335e) SHA1(8a881c9c511fb63b00a3a7e433bae12aa9c2c262) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(0cab2a72) SHA1(32d098bdd693a11f3cea6bbed3515c4217f40e23) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(f6442026) SHA1(f49ddeeeaf6fffdccea9ba73bce3ca60c07a7647) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(5c9e8665) SHA1(2a1b040e5c72d4400d4b5c467c75ae99e9bb01e2) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(fdd79d8f) SHA1(f8bb82afaa28affb04b83270eb407129f1c7e611) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8015", 0, SHA1(d6b5af48775304caa1d98878dc8dc154c975720e) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.ic8", 0x000000, 0x0008c1, CRC(30719630) SHA1(a8c7b6d0304c38691775c5af6c32fbeeefd9f9fa) )
ROM_END

ROM_START( nuretemi )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3", 0x000000, 0x020000, CRC(da303352) SHA1(91c8752c93ca4022e978744bf42261d1a0e899a1) )
	ROM_LOAD16_BYTE( "1.ic2", 0x000001, 0x020000, CRC(53ef6360) SHA1(ec90f01e4e78821511a6dba885c0d38f594a3a86) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51", 0x000000, 0x020000, CRC(655ec499) SHA1(5cea38e998edc7833b9a644930daecd99933c277) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40", 0x000001, 0x080000, CRC(5c7af7f6) SHA1(78e58e3a81a6585c2c61f0026b7dc73a72c0d862) )
	ROM_LOAD16_BYTE( "4.ic41", 0x000000, 0x080000, CRC(335b6388) SHA1(c5427b42af011b5a5026d905b1740684b9f6f953) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8016", 0, SHA1(76c7257e6c98e9dd8c594489c3104b2d032a9099) )

	ROM_REGION( 0x1000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.bin", 0x000000, 0x000117, CRC(865de327) SHA1(a97374ee765443d9da6919d0e226108c062f7942) )
ROM_END

ROM_START( tsuwaku )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(4577bf7b) SHA1(fed88157ded8ac72cc28cdd3b2ee36c293a6ee93) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(a9890007) SHA1(3cd36c653d387842289f74c3cf35435f9d2a3aca) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(8451b9a9) SHA1(4e61c4b5ea7e91b53c97bd060b41466ba5005fd0) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(00657ca3) SHA1(a02bb8a177f3915ddf0bf97fd69426a3a28061a5) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(edf56c94) SHA1(76d95a45aced3ad8bfe8a561f355731f4f99603e) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8017", 0, SHA1(0db86e6157b4bd402f0c41d0d69c1a80d2156b27) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8h.020", 0x000000, 0x040000, CRC(ac5c9495) SHA1(1c54ecf6dedbf8c3a29207c1c91b52e2ff394d9d) )
ROM_END

ROM_START( torarech )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(a7fda49b) SHA1(a7891e574b4d2ae3bcdc40f76b6e67e67d6e72bd) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(887c1a0d) SHA1(a594e3ef6514ed48f097e742633c19e51c10b730) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(bd785d10) SHA1(ceb91c0f13eafabb8d48384857af6fc555d48951) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(cbbbab5c) SHA1(ab8ae64b1f2acfab55ba7cbb173f3036a46001e6) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(18412fd8) SHA1(6907ce2739549519e1f3dcee2186f6add219a3c2) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8018", 0, SHA1(b6cf54735bf6fded2ffb17e6fac84ae9a86ccaf2) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h.020", 0x000000, 0xbb2, BAD_DUMP CRC(c8e8605a) SHA1(02e43d9de73256e5c73d6f99834a23cef321d56b) )
ROM_END

ROM_START( nichisel )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(fb84fc3e) SHA1(6b87c3516ceec59ec96012ea6a3d2fa9670a1cb3) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(95fb8e74) SHA1(79aa45ed1c3bd3e1a83b02afb64268efb386100e) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(f94981fd) SHA1(84dae027f10717a084016310cd245bb4c2ee6a56) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(5ab63481) SHA1(fc81fbdd1df496813fc0d80bcab6d0434b75d311) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(50085861) SHA1(b8f99a66a743c9bf66ef307fe4b581586e293fe5) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb80sp", 0, SHA1(3bf53fef4b002bfd8515e4133f636e1c9b031931) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h.020", 0x000000, 0x040000, CRC(228b98fb) SHA1(53b57a09610425a5bb9d0ffe0f68dce2d9ab3bf6) )
ROM_END

ROM_START( konhaji )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",            0x000000, 0x020000, CRC(f16c88db) SHA1(bd8b4619817bd95fffe3e7e6ca57cc2223f372fa) )
	ROM_LOAD16_BYTE( "1.ic2",            0x000001, 0x020000, CRC(9360eabe) SHA1(ecae0c8090f5cadd87fb544190112b53193f54ee) )

	DVD_BIOS

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) // z80
	ROM_LOAD( "11.ic51",           0x000000, 0x020000, CRC(d1ba05d6) SHA1(8d29cdbf00946e06e92225eb260a694d17d7b8d4) )

	ROM_REGION16_BE( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",            0x000001, 0x080000, CRC(88f31da7) SHA1(dc76532fa3261b3b238a42e2ca8f270f2b2ea1fa) )
	ROM_LOAD16_BYTE( "4.ic41",            0x000000, 0x080000, CRC(35893109) SHA1(6a55bd147a75913af59bc355abf010e1b75063bf) )

	DISK_REGION( "ata:0:dvdrom" )
	DISK_IMAGE_READONLY( "nb8019", 0, SHA1(a7b6dc0374b374b02e4a4eaf706330f961ebb563) )

	ROM_REGION( 0x040000, "gal", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.bin", 0x000000, 0x000117, CRC(9f0dec43) SHA1(d836e564da496c3049e16f025daf362cced413d4) )
ROM_END


/***************************************************************************

    Game metadata

***************************************************************************/

// 1995
GAME( 1995, csplayh1, 0,                 hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Sphinx/AV Japan/Astro System Japan", "Super CD Dai8dan Mahjong Hanafuda Cosplay Tengoku (Japan)", MACHINE_NOT_WORKING )

GAME( 1998, nichidvd, 0,                 hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu",                         "Nichibutsu High Rate DVD BIOS", MACHINE_IS_BIOS_ROOT )

// 1998
/* 01 */ GAME( 1998, mjgalpri, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Just&Just",                           "Mahjong Gal-pri - World Gal-con Grandprix (Japan)", MACHINE_NOT_WORKING )
/* 02 */ GAME( 1998, sengomjk, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Just&Just",                           "Sengoku Mahjong Kurenai Otome-tai (Japan)", MACHINE_NOT_WORKING )
/* 03 */ GAME( 1998, junai,    nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",                                 "Junai - Manatsu no First Kiss (Japan)", MACHINE_NOT_WORKING )
/* 04 */ GAME( 1998, csplayh5, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu",                                     "Mahjong Hanafuda Cosplay Tengoku 5 (Japan)", MACHINE_NOT_WORKING )
/* 05 */ GAME( 1998, junai2,   nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",                                 "Junai 2 - White Love Story (Japan)", MACHINE_NOT_WORKING )
/* 06 */ GAME( 1998, mogitate, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Just&Just/NVS/Astro System/AV Japan", "Mahjong Mogitate (Japan)", MACHINE_NOT_WORKING )

// 1999
/* 07 */ GAME( 1999, mjmania,  nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Sphinx/Just&Just",            "Mahjong Mania - Kairakukan e Youkoso (Japan)", MACHINE_NOT_WORKING )
/* 08 */ GAME( 1999, renaimj,  nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "Renai Mahjong Idol Gakuen (Japan)", MACHINE_NOT_WORKING )
/* 09 */ GAME( 1999, bikiniko, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "BiKiNikko - Okinawa de Ippai Shichaimashita (Japan)", MACHINE_NOT_WORKING )
/* 10 */ GAME( 1999, csplayh6, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "Mahjong Hanafuda Cosplay Tengoku 6 - Junai-hen (Japan)", MACHINE_NOT_WORKING )
/* 11 */ GAME( 1999, thenanpa, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Love Factory/eic", "The Nanpa (Japan)", MACHINE_NOT_WORKING )
/* 12 */ GAME( 1999, pokoachu, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "PokoaPoka Onsen de CHU - Bijin 3 Shimai ni Kiotsukete! (Japan)", MACHINE_NOT_WORKING )
/* 13 */ GAME( 1999, csplayh7, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "Cosplay Tengoku 7 - Super Kogal Ranking (Japan)", MACHINE_NOT_WORKING )
/* 14 */ GAME( 1999, aimode,   nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",              "Ai-mode - Pet Shiiku (Japan)", MACHINE_NOT_WORKING )

// 2000
/* 15 */ GAME( 2000, fuudol,   nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/eic",                    "Fuudol (Japan)", MACHINE_NOT_WORKING )
/* 16 */ GAME( 2000, nuretemi, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Love Factory",           "Nurete Mitaino... - Net Idol Hen (Japan)", MACHINE_NOT_WORKING )
/* 17 */ GAME( 2000, tsuwaku,  nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Love Factory/Just&Just", "Tsuugakuro no Yuuwaku (Japan)", MACHINE_NOT_WORKING )
/* 18 */ GAME( 2000, torarech, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Love Factory/M Friend",  "Torarechattano - AV Kantoku Hen (Japan)", MACHINE_NOT_WORKING )
/* sp */ GAME( 2000, nichisel, nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu",                        "DVD Select (Japan)", MACHINE_NOT_WORKING )

// 2001
/* 19 */ GAME( 2001, konhaji,  nichidvd, hrdvd, hrdvd, hrdvd_state, empty_init, ROT0, "Nichibutsu/Love Factory", "Konnano Hajimete! (Japan)", MACHINE_NOT_WORKING )
/* 20 */ // Uwasa no Deaikei Site : Nichibutsu/Love Factory/eic
