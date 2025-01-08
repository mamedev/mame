// license:BSD-3-Clause
// copyright-holders:

/*
    Raiden (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 8 main boards and 1 sub board.

    8086/1 red board - 2 V30s, M68000 socket for connecting to board 6/1 + 6 ROMs + RAMs.
    MOD-6/1 - RAMs, 20 MHz XTAL and  M68000 socket used for connecting to board 8086/1.
    MOD 21/1 - 24 MHz XTAL.
    MOD 1/5 - Sound board (Z80, 2xYM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/3 - Sprite board, has logic + 4 sprite ROMs.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1) for terminating the connector (it's not used).
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1) for terminating the connector (it's not used).
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1) for terminating the connector (it's not used).

    PCBs pictures and dip listing are available at: http://www.recreativas.org/modular-system-raiden-4315-gaelco-sa

Board 6/1                                           Board 21/1
 _____________________________________________       ______________________________________________
 |             _______________              __|_     |                                             |
 |             |              |             |   |    |                                             |
 |             | HM62256LP-12 |             |   |    |   ________   _______  ________              |
 |             |______________|             |   |    |   |_EMPTY_| |_EMPTY_| |_EMPTY_|             |
 |  _______   ________________    ________  |   |    |                                             |
 | 74LS367AN  |               |  T74LS138B1 |   |    |                                             |
 |            | EMPTY         |             |   |    |                                             |
 |            |_______________|             |   |    |   ________   _______  ________              |
 |  _______   ________________    _______   |   |    |   |74F74N_| 74LS393PC 74LS393PC             |
 | 74LS367AN  |               |   74LS245N  |   |    |                                             |
 |            | EMPTY         |             |   |    |                                             |
 |            |_______________|             |   |    |                                             |
 |            ________________              |   |    |   ________   ________  ________             |
 |  _______   |               |   _______   |   |    |   74LS7273N |82S129_| |82S129_|             |
 | 74LS138N   | EMPTY         |   74LS374N  |   |    |               P0202      211                |
 |            |_______________|             |   |    |                                             |
 |             _______________              |   |    |                                             |
 |  _______    |              |   _______   |___|    |   ________   _______  ________              |
 |  GAL16V8    | HM62256LP-12 |   74LS245N    |      |   74LS74AN    XTAL    SN74LS367AN           |
 |    686      |______________|               |      |             24.000 MHz                      |
 |            ________________                |      |                                             |
 |  _______   |               |   _______     |      |   ________   _______  ________              |
 | 74LS174N   | EMPTY         |   74LS374N    |   __ |_  74LS32PC  74LS368AB UM2148-1              |
 |            |_______________|               |   |   |                                            |
 |            ________________                |   |   |                                            |
 |            |               |               |   |   |  ________   _______  ________              |
 |  _______   | EMPTY         |   _______     |   |   |  74LS157N  74F112PC  UM2148-1              |
 |  GAL16V8   |_______________|  T74LS138B1   |   |   |                                            |
 |   645B*    ________________                |   |   |                                            |
 |            |               |               |   |   |  ________   _______  ________              |
 |            | EMPTY         |               |   |   |  74LS148N  74LS368AB UM2148-1              |
 |            |_______________|   _______     |   |   |                                            |
 |  __________________________    74LS32B1    |   |   |                                            |
 |  | 68000 SOCKET BUT        |               |   |   |  ________   _______  ________   ________   |
 |  | CONNECTED TO CPU PCB    |   _______     |   |   |  74LS298N  74LS298N  74LS298N   74LS174PC  |
 |  |_________________________|   74LS20N     |   |   |                                            |
 |                                            |   |   |                                            |
 |  _______  ______  _______      _______     |   |   |  ________   _______  ________   ________   |
 |  |74S74N   XTAL  74LS368AB1    74LS132N    |   |   |  74LS245N  74LS245N  |74LS08B   74LS174PC  |
 |          20.000MHz                         |   |___|                                            |
 |____________________________________________|      |_____________________________________________|

  Sound Board (1/5)
    ______________________________________________
    |                                             |
    |  ________  ________  ________               |
    |  74LS107N  |74LS32N| |_EMPTY_|              |
    |                                             |
    |  ________  __________________               |
    | 74LS368AB1 |                 |              |
    |            | EMPTY           |              |
    |  ________  |_________________|              |
    |  74LS74AN  __________________               |
    |            |  Z8400BB1       |              |
    |  ________  |  Z80 B CPU      |  ________    |
    | 74HCT157E  |_________________|  |LM324AN|   |
    |            ________________                 |
    |  ________  |               |             __ |
  110->N82S123N  | RD 01         |             |D||
    |  ________  |_______________|             |I||
    | 74HCT157E  ________________              |P||     SOUND SUB
    | __________ |               | _____ _____ |S|| __________________
    | |         || LH5164D-10L   |Y3014B Y3014B|-|| | ______          |
    | | SOUND   ||_______________|             |D|| | LM358N     ___  |
    | |  SUB    |                              |I|| | .......... |  | |
    | |         |                              |P|| |            |OKI |
  __|_|_________|__________________            |S|| | .......... |M5205
  |   |          | YAMAHA          |           |_|| | ________   |__| |
  |   |          | YM2203C         |              | | 74LS377P        |
  |   |          |_________________|              | |_________________|
  |   |                                           |
  |   | _______  ________                         |
  |   | |EMPTY_| 74LS74AN                         |
  |   |                                           |
  |   |                                           |
  |   |          __________________    _____      |
  |   | _______  | YAMAHA          |  TDA2003     |
  |   | |EMPTY|  | YM2203C         |              |
  |   |          |_________________|              |
  |   |                                           |
  |   |          _________  _________             |
  |   |          74LS244B1  74LS244B1             |
  |   |                                           |
  |___|        __________________________         |
    |__________| |_|_|_|_|_|_|_|_|_|_|_| |________|
                        PRE-JAMMA

  CPU PCB (8086/1)                                       Board 51/3
  ______________________________________________         _____________________________________________
 | ____________                    ____________ |       |                                            |
 ||RD601B      | ________         |NEC D70116C-10       | __________  ________ ________ ________     |
 ||____________|SN74LS373N        |_V30________||       | |RD504    | 74LS299N 74LS169BN CY7C149|    |
 | ____________                    ____________ |       | |_________| ________ ________ ________     |
 ||RD601B      | ________         |NEC D70116C-10       |            74LS169BN 74LS169BN CY7C149|    |
 ||____________|SN74LS373N        |_V30________||       |                                            |
 |  ___________                     ___________ |       | __________  ________ ________ ________     |
 | |RD603      | ________          |RD605B     ||       | |RD503    | 74LS158A 74LS169BN 82S129N <- 502
 | |___________|SN74LS373N         |___________||       | |_________| ________ ________ ________     |
 |  ___________  ________           ___________ |       |             74LS299N 74LS169BN 74LS244N    |
 | |RD604      |SN74LS373N         |RD606B     ||       |                                            |
 | |___________|                   |___________||       | __________  ________ ________ ________     |
 |  ___________  ________ ________   __________ |       | |RD502    | 74LS299N 74LS169BN 74LS244N    |
 | |SRM20256LC |SN74LS373N 74LS00N  |D4464-15L ||       | |_________| ________ ________ ________     |
 | |___________| ________ ________  |__________||       |             74LS20B1 |CY7C149 74LS298B1    |
 |  ___________ SN74LS373N 74LS368AN __________ |       |                                            |
 | |SRM20256LC | ________ ________  |D4464-15L ||       | __________  ________ ________ ________     |
 | |___________| |74LS32N 74LS139N  |__________||       | |RD501    | 74LS299N |CY7C149 74LS298B1    |
 |                                              |       | |_________|                                |
 |   ________   ________  ________   ________   |       |   ________  ________ ________ ________  __ |
 |   74LS245N   74LS157N  74LS157N   74LS245N   |       |   74LS273B1 |74LS00N 74LS86B1 74LS244N  | ||
 |   ________   ________  ________   ________   |       |                                         | ||
 |   74LS245N   74LS157N  74LS157N   74LS245N   |      _|_  ________  ________ ________ ________  |V||
 |   ______________________          ________   |      |  | |74LS08N  74F158APC 74LS74B1 74LS20B1 |O||
 |  | 68000 SOCKET BUT     |         GAL16V8 <- 645C   |  |                                       |I||
 |  | CONNECTED TO PCB 6/1 |         ________   |      |  | ________  ________ ________ ________  |D||
 |  |______________________|         PALCE16V8 <- 645D 503-> GAL16V8 74LS393B1 74LS368AB1 74LS377N|E||
 |______________________________________________|      |  |                                       |D||
                                                       |  | ________  ________ ________ ________  | ||
                                                       |  | 74LS138N  74LS283N UM2148-2 |_EMPTY_| | ||
                                                       |  |                                       | ||
                                                       |  | ________  ________ ________ ________  | ||
                                                       |  | 74LS175B1 74LS283N UM2148-2 74LS273B1 | ||
                                                       |  |                                       | ||
                                                       |  | ________  ________ ________ ________  |_||
                                                       |  | 74LS298B1 74LS157N 74LS157N 74LS273B1    |
                                                       |  |                                          |
                                                       |  | ________  ________ ________ ________     |
                                                       |  | 74LS158P 74LS169BN 74LS169BN 74LS245B1   |
                                                       |__|                                          |
                                                        |____________________________________________|

  Board 4/3 (B)                                     Board 4/3 (A)                                    Board 4/3
 _____________________________________________    _____________________________________________    _____________________________________________
 |                                            |   |                                            |   |                                            |
 |   ________  ________ ________ ________     |   |   ________  ________ ________ ________     |   |   ________  ________ ________ ________     |
 |   74LS00PC  |74S20N| 74LS244N 74LS163A  __ |   |   74LS00PC  |74S20N| 74LS244N 74LS163A  __ |   |   74LS00PC  |74S20N| 74LS244N 74LS163A  __ |
 |                                         | ||   |                                         | ||   |                                         | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||
 |   PAL16R8A 74LS163A 74LS373PC 74LS163A  | ||   |   PAL16R8A 74LS163A 74LS373PC 74LS163A  | ||   |   PAL16R8A 74LS163A 74LS373PC 74LS163A  | ||
 |    PC403                                | ||   |     403                                 | ||   |    P0403                                | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||
 |   74LS138PC 74LS283N 74LS273PC 74LS163A | ||   |   74LS138PC 74LS283N 74LS273PC 74LS163A | ||   |   74LS138B1 74LS283N 74LS273PC 74LS163A | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||
 |   74LS393N  74LS283N 74LS273N 74LS153PC | ||   |   74LS393N  74LS283N 74LS273N 74LS153   | ||   |   74LS393N  74LS283N 74LS273N 74LS153PC | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||
 |   74LS86PC  74LS299N |RD4B4  | 74LS153PC| ||   |   74LS86PC  74LS299N |RD4A4  | 74LS153  | ||   |   74LS86PC  74LS299  |RD401  | 74LS153N | ||
 |                      |_______|          | ||   |                      |_______|          | ||   |                      |_______|          | ||
 |                                         | ||   |                                         | ||   |                                         | ||
 |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||   |   ________  ________ ________ ________  | ||
_|_  74LS86PC  74LS299N |RD4B3  | 74LS153PC|V||  _|_  74LS86PC  74LS299N |RD4A3  | 74LS153  |V||  _|_  74LS86PC  74LS299  |RD402  | 74LS153N |V||
|  |                    |_______|          |O||  |  |                    |_______|          |O||  |  |                    |_______|          |O||
|  |                                       |I||  |  |                                       |I||  |  |                                       |I||
|  | ________  ________ ________ ________  |D||  |  | ________  ________ ________ ________  |D||  |  | ________  ________ ________ ________  |D||
|  |  74LS04   74LS299N |RD4B2  | 74LS153PC|E||  |  |  74LS04   74LS299N |RD4A2  | 74LS153  |E||  |  |  74LS04   74LS299  |RD403  | 74LS153N |E||
|  |                    |_______|          |D||  |  |                    |_______|          |D||  |  |                    |_______|          |D||
|  |                                       | ||  |  |                                       | ||  |  |                                       | ||
|  | ________  ________ ________ ________  | ||  |  | ________  ________ ________ ________  | ||  |  | ________  ________ ________ ________  | ||
|  | 74LS245P  74LS299N |RD4B1  | 74LS153PC| ||  |  | 74LS245P  74LS299N |RD4A1  | 74LS153  | ||  |  | 74LS245P  74LS299  |RD4A4  | 74LS153N | ||
|  |                    |_______|          | ||  |  |                    |_______|          | ||  |  |                    |_______|          | ||
|  |                                       | ||  |  |                                       | ||  |  |                                       | ||
|  | ________  ________ ________ ________  | ||  |  | ________  ________ ________ ________  | ||  |  | ________  ________ ________ ________  | ||
|  | 74LS32PC  74LS158P |D4364C | 74LS153PC| ||  |  | 74LS32PC  74LS158P |HM32064| 74LS153  | ||  |  | 74LS32B1  74LS158N |SRM2064| 74LS153N | ||
|  |                    |15L____|          | ||  |  |                    |5______|          | ||  |  |                    |C15____|          | ||
|  |                                       |_||  |  |                                       |_||  |  |                                       |_||
|__|                                          |  |__|                                          |  |__|                                          |
 |____________________________________________|   |____________________________________________|   |____________________________________________|

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class raiden_ms_state : public driver_device
{
public:
	raiden_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ym1(*this, "ym1"),
		m_ym2(*this, "ym2"),
		m_msm(*this, "msm5205"),
		m_spriteram(*this, "spriteram"),
		m_scrollram1(*this, "scrollram1"),
		m_scrollram2(*this, "scrollram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_soundlatch(*this, "soundlatch%u", 1),
		m_soundbank(*this, "sound_bank")
	{ }

	void raidenm(machine_config &config);
	void init_raidenm();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ym2203_device> m_ym1;
	required_device<ym2203_device> m_ym2;
	required_device<msm5205_device> m_msm;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scrollram1;
	required_shared_ptr<uint16_t> m_scrollram2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_videoram3;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_memory_bank m_soundbank;

	uint16_t pal_read16(offs_t offset, u16 mem_mask = ~0) { uint16_t data = m_palette->read16(offset); return swapendian_int16(data); };
	uint16_t pal_read16_ext(offs_t offset, u16 mem_mask = ~0) { uint16_t data = m_palette->read16_ext(offset); return swapendian_int16(data);  };
	void pal_write16(offs_t offset, u16 data, u16 mem_mask = ~0) { m_palette->write16(offset, swapendian_int16(data), swapendian_int16(mem_mask)); };
	void pal_write16_ext(offs_t offset, u16 data, u16 mem_mask = ~0) { m_palette->write16_ext(offset, swapendian_int16(data), swapendian_int16(mem_mask)); };

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void raidenm_map(address_map &map) ATTR_COLD;
	void raidenm_sub_map(address_map &map) ATTR_COLD;

	u8 sound_status_r();
	void adpcm_w(u8 data);
	void ym_w(offs_t offset, u8 data);
	void audio_map(address_map &map) ATTR_COLD;
	void adpcm_int(int state);
	bool m_audio_select;
	u8 m_adpcm_data;

	void unk_snd_dffx_w(offs_t offset, u8 data);
	void soundlatch_w(u8 data);

	void vblank_irq(int state);

	void descramble_16x16tiles(uint8_t* src, int len);

	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap2);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap3);

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap2;
	tilemap_t *m_bg_tilemap3;
};


void raiden_ms_state::raidenm_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram();
	map(0x0a000, 0x0afff).ram().share("shared_ram");

	map(0x0b000, 0x0b001).portr("P1");
	map(0x0b002, 0x0b003).portr("P2");
	map(0x0b004, 0x0b005).portr("P3");
	map(0x0b006, 0x0b007).nopw();
	map(0x0b008, 0x0b008).w(m_soundlatch[0], FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0x0b009, 0x0b009).r(FUNC(raiden_ms_state::sound_status_r)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write)).umask16(0xff00);

	map(0x0c000, 0x0cfff).ram().w(FUNC(raiden_ms_state::vram_w)).share("videoram");
	map(0x0d800, 0x0dfff).ram().share("spriteram");

	map(0x0f100, 0x0f103).ram().share("scrollram1");
	map(0x0f900, 0x0f903).ram().share("scrollram2");

	map(0xa0000, 0xfffff).rom();
}

void raiden_ms_state::raidenm_sub_map(address_map &map)
{
	map(0x00000, 0x01fff).ram();
	map(0x02000, 0x027ff).ram().w(FUNC(raiden_ms_state::vram2_w)).share("videoram2");
	map(0x02800, 0x02fff).ram().w(FUNC(raiden_ms_state::vram3_w)).share("videoram3");
	map(0x03000, 0x033ff).ram().rw(FUNC(raiden_ms_state::pal_read16), FUNC(raiden_ms_state::pal_write16)).share("palette");
	map(0x03400, 0x037ff).ram().rw(FUNC(raiden_ms_state::pal_read16_ext), FUNC(raiden_ms_state::pal_write16_ext)).share("palette_ext");
	map(0x03800, 0x03fff).ram();

	map(0x04000, 0x04fff).ram().share("shared_ram");

	map(0x07ffe, 0x07fff).ram();

	map(0x08000, 0x08001).nopw();
	map(0x0a000, 0x0a001).nopw();
	map(0x0e800, 0x0e801).nopw();

	map(0xc0000, 0xfffff).rom();
}


u8 raiden_ms_state::sound_status_r()
{
	return 0;
}


void raiden_ms_state::adpcm_w(u8 data)
{
	m_audio_select = BIT(data, 7);
	m_soundbank->set_entry(m_audio_select ? 1 : 0);
	m_msm->reset_w(BIT(data, 4));

	m_adpcm_data = data & 0xf;
}

void raiden_ms_state::ym_w(offs_t offset, u8 data)
{
	if (!m_audio_select)
	{
		if (!BIT(offset, 1))
			m_ym1->write(offset & 1, data);
		else
			m_ym2->write(offset & 1, data);
	}
	else
	{
		// ??? (written during ADPCM IRQ)
	}
}

void raiden_ms_state::unk_snd_dffx_w(offs_t offset, u8 data)
{

}

void raiden_ms_state::soundlatch_w(u8 data)
{
	m_soundlatch[0]->clear_w(data);
	m_soundlatch[1]->clear_w(data);
}

void raiden_ms_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).w(FUNC(raiden_ms_state::adpcm_w));
	map(0x8000, 0xbfff).bankr("sound_bank");
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd7ff).ram();
	// area 0xdff0-5 is never ever readback, applying a RAM mirror causes sound to go significantly worse,
	// what they are even for?  (offset select bankswitch rather than data select?)
	map(0xdff0, 0xdff7).w(FUNC(raiden_ms_state::unk_snd_dffx_w));
	map(0xdff8, 0xdff8).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(FUNC(raiden_ms_state::soundlatch_w));
	map(0xdff9, 0xdff9).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe003).w(FUNC(raiden_ms_state::ym_w));
	map(0xe008, 0xe009).r(m_ym1, FUNC(ym2203_device::read));
	map(0xe00a, 0xe00b).r(m_ym2, FUNC(ym2203_device::read));
}

void raiden_ms_state::adpcm_int(int state)
{
	m_msm->data_w(m_adpcm_data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


void raiden_ms_state::machine_start()
{
	m_soundbank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);
}


void raiden_ms_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void raiden_ms_state::vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset);
}

void raiden_ms_state::vram3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap3->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(raiden_ms_state::get_tile_info_tilemap1)
{
	int tiledata = m_videoram[tile_index];
	int tile = (tiledata & 0xff) | ((tiledata >> 6) & 0x300);
	int color = (tiledata >> 8) & 0x0f;

	tileinfo.set(3, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_ms_state::get_tile_info_tilemap2)
{
	int tile = m_videoram2[tile_index];
	tileinfo.set(1, tile & 0xfff, tile >> 12, 0);
}

TILE_GET_INFO_MEMBER(raiden_ms_state::get_tile_info_tilemap3)
{
	int tile = m_videoram3[tile_index];
	tileinfo.set(2, tile & 0xfff, tile >> 12, 0);
}


void raiden_ms_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_ms_state::get_tile_info_tilemap1)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(15);

	m_bg_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_ms_state::get_tile_info_tilemap2)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	//m_bg_tilemap2->set_transparent_pen(15);

	m_bg_tilemap3 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_ms_state::get_tile_info_tilemap3)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_bg_tilemap3->set_transparent_pen(15);

}

uint32_t raiden_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_bg_tilemap3->set_scrollx(0, 64 + (((m_scrollram1[0] & 0xff00) >> 8) | ((m_scrollram1[0] & 0x00ff) << 8)));
	m_bg_tilemap3->set_scrolly(0, 17 + (((m_scrollram1[1] & 0xff00) >> 8) | ((m_scrollram1[1] & 0x00ff) << 8)));

	m_bg_tilemap2->set_scrollx(0, 64 + (((m_scrollram2[0] & 0xff00) >> 8) | ((m_scrollram2[0] & 0x00ff) << 8)));
	m_bg_tilemap2->set_scrolly(0, 17 + (((m_scrollram2[1] & 0xff00) >> 8) | ((m_scrollram2[1] & 0x00ff) << 8)));

	m_bg_tilemap2->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap3->draw(screen, bitmap, cliprect, 0, 0);


	// TODO, convert to device, share between Modualar System games
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = 256-16;
	const int Y_EXTRA_OFFSET = 30;

	for (int i = NUM_SPRITES - 2; i >= 0; i -= 2)
	{
		gfx_element* gfx = m_gfxdecode->gfx(0);

		uint16_t attr0 = m_spriteram[i + 0];
		uint16_t attr1 = m_spriteram[i + 1];

		uint16_t attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t attr3 = m_spriteram[i + NUM_SPRITES + 1]; // unused?

		int ypos = attr0 & 0x00ff;
		int xpos = (attr1 & 0xff00) >> 8;
		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x003f) << 8;

		int flipy = (attr2 & 0x4000);
		int flipx = (attr2 & 0x2000);

		gfx->transpen(bitmap, cliprect, tile, (attr2 & 0x0f00) >> 8, flipx, flipy, xpos - 16 - X_EXTRA_OFFSET, ypos - Y_EXTRA_OFFSET, 15);
	}

	m_bg_tilemap->set_scrolly(0, 16);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

static INPUT_PORTS_START( raidenm )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	// Coin Mode A
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_CONDITION("P2", 0x0001, EQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	// Coin Mode B
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_CONDITION("P2", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0000, "5C/1C or Free if Coin B too" )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ) ) PORT_CONDITION("P2", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1C/6C or Free if Coin A too" )

	PORT_DIPNAME( 0x0020, 0x0020, "Credits to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "80000 300000" )
	PORT_DIPSETTING(      0x0c00, "150000 400000" )
	PORT_DIPSETTING(      0x0400, "300000 1000000" )
	PORT_DIPSETTING(      0x0000, "1000000 5000000" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};


static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	16 * 16
};


static GFXDECODE_START( gfx_raiden_ms )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x200, 16 )

	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16x4_layout, 0x300, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8x4_layout, 0x000, 16 )
GFXDECODE_END

void raiden_ms_state::vblank_irq(int state)
{
	if (state)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8/4); // V30
		m_subcpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8/4); // V30
	}
}


void raiden_ms_state::raidenm(machine_config &config)
{
	// Basic machine hardware
	V30(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden_ms_state::raidenm_map);

	V30(config, m_subcpu, 20_MHz_XTAL / 2); // divisor unknown
	m_subcpu->set_addrmap(AS_PROGRAM, &raiden_ms_state::raidenm_sub_map);

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &raiden_ms_state::audio_map);

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(raiden_ms_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(raiden_ms_state::vblank_irq));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x400);

	GFXDECODE(config, "gfxdecode", "palette", gfx_raiden_ms);

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);

	// Sound hardware
	SPEAKER(config, "mono").front_center();
	YM2203(config, m_ym1, XTAL(4'000'000)/4); // unknown clock
	m_ym1->add_route(0, "mono", 0.15);
	m_ym1->add_route(1, "mono", 0.15);
	m_ym1->add_route(2, "mono", 0.15);
	m_ym1->add_route(3, "mono", 0.40);

	YM2203(config, m_ym2, XTAL(4'000'000)/4); // unknown clock
	m_ym2->add_route(0, "mono", 0.15);
	m_ym2->add_route(1, "mono", 0.15);
	m_ym2->add_route(2, "mono", 0.15);
	m_ym2->add_route(3, "mono", 0.40);

	MSM5205(config, m_msm, XTAL(384'000)); // unknown clock
	m_msm->vck_legacy_callback().set(FUNC(raiden_ms_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S96_4B); // unverified
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.00);

}

// reorganize graphics into something we can decode with a single pass
void raiden_ms_state::descramble_16x16tiles(uint8_t* src, int len)
{
	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void raiden_ms_state::init_raidenm()
{
	descramble_16x16tiles(memregion("gfx1")->base(), memregion("gfx1")->bytes());
	descramble_16x16tiles(memregion("gfx2")->base(), memregion("gfx2")->bytes());
	// gfx3 is 8x8 tiles
}



ROM_START( raidenm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on red board
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd604.u7",   0x0a0000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd603.u8",   0x0a0001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd602b.u5",  0x0c0000, 0x20000, CRC(2af64e22) SHA1(4aef347ef6396c3e12fe27b82046ee0efb274602) )
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd601b.u6",  0x0c0001, 0x20000, CRC(bf6245e1) SHA1(8eccd42f1eef012a5c937d77d2f877de14dcf39c) )

	ROM_REGION( 0x100000, "subcpu", 0 ) // on red board
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd606b.u19", 0x0c0001, 0x20000, CRC(58eac0b6) SHA1(618813c7593d13271d2826739f24e08dda400b0d) )
	ROM_LOAD16_BYTE( "msraid_6-1-8086-1_rd605b.u18", 0x0c0000, 0x20000, CRC(251fef93) SHA1(818095a77cb94fd4acc9eb26954615ee93e8432c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/5 board
	ROM_LOAD( "msraid_1-5_rd101.ic12",  0x00000, 0x10000, CRC(2b76e371) SHA1(4c9732950f576e498d02fde485ba92fb293d5594) )

	// dumper's note: ROMs [rd4b1, rd4b2, rb4b3, rd4b4] and [rd4a1, rd4a2, rb4a3, rd4a4] have a strange setup
	// with pins 32, 31 and 30 soldered together and pin 2 connected between all four chips, while the sockets are for 28 pin chips
	// (with 27C512 silkscreened on the PCB behind the chips)
	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) // on one of the MOD 4/3 boards
	ROM_LOAD32_BYTE( "msraid_4-3-1_rd4b1.ic17",  0x00003, 0x20000, CRC(ff35b830) SHA1(fb552b2aa50aed12c3adb6ef9032a438adf6f37f) )
	ROM_LOAD32_BYTE( "msraid_4-3-1_rd4b2.ic16",  0x00002, 0x20000, CRC(da0b2fca) SHA1(4ffe177587759ea03e73bcc5c36bedc869653ce5) )
	ROM_LOAD32_BYTE( "msraid_4-3-1_rd4b3.ic15",  0x00001, 0x20000, CRC(00e5953f) SHA1(488ac889a587bf4108be424faa9123abe73b5246) )
	ROM_LOAD32_BYTE( "msraid_4-3-1_rd4b4.ic14",  0x00000, 0x20000, CRC(932f8407) SHA1(0c7a0b18bbc3ac1f30bdb0ba4466c1e253130305) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT ) // on another MOD 4/3 board
	ROM_LOAD32_BYTE( "msraid_4-3-2_rd4a1.ic17",  0x00003, 0x20000, CRC(32892554) SHA1(8136626bf7073cdf19a8a38d19f6d0c52405df16) )
	ROM_LOAD32_BYTE( "msraid_4-3-2_rd4a2.ic16",  0x00002, 0x20000, CRC(cb325246) SHA1(f0a7bf8b1b5145541af78eba0375392a3030e2d9) )
	ROM_LOAD32_BYTE( "msraid_4-3-2_rd4a3.ic15",  0x00001, 0x20000, CRC(7554acef) SHA1(ec418ef2246c889d0e308925e11b1d3328bd83f9) )
	ROM_LOAD32_BYTE( "msraid_4-3-2_rd4a4.ic14",  0x00000, 0x20000, CRC(1e1537a5) SHA1(3f17a85c185dd8be7f7b3a5adb28d508f9f059a5) )

	ROM_REGION( 0x20000, "gfx3", ROMREGION_INVERT ) // on a third MOD 4/3 board, all 1st and 2nd half identical
	ROM_LOAD32_BYTE( "msraid_4-3-3_rd404.ic17",  0x00000, 0x08000, CRC(caec39f5) SHA1(c61bd1f02515c6597d276dfcb21ed0969b556b8b) )
	ROM_LOAD32_BYTE( "msraid_4-3-3_rd403.ic16",  0x00001, 0x08000, CRC(0e817530) SHA1(efbc05d31ab38d0213387f4e61977f5203e19ace) )
	ROM_LOAD32_BYTE( "msraid_4-3-3_rd402.ic15",  0x00002, 0x08000, CRC(55dd887b) SHA1(efa687afe0b19fc741626b8fe5cd6ab541874396) )
	ROM_LOAD32_BYTE( "msraid_4-3-3_rd401.ic14",  0x00003, 0x08000, CRC(da82ab5d) SHA1(462db31a3cc1494fdc163d5abdf6d74a182a1421) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_INVERT ) // on MOD 51/3 board
	ROM_LOAD32_BYTE( "msraid_51-3_rd501.ic43",   0x00003, 0x20000, CRC(fcd1fc21) SHA1(465036348f19dd3ff999c12e6591b172a7fb621e) )
	ROM_LOAD32_BYTE( "msraid_51-3_rd502.ic42",   0x00002, 0x20000, CRC(faba2544) SHA1(1d32b370f43bf6f7cb3bbe052775e52403a7ccb6) )
	ROM_LOAD32_BYTE( "msraid_51-3_rd503.ic41",   0x00001, 0x20000, CRC(ae4001e9) SHA1(5f21a042cad1807d2ef5e7f4f2cfd86cadc0503b) )
	ROM_LOAD32_BYTE( "msraid_51-3_rd504.ic40",   0x00000, 0x20000, CRC(0452eb10) SHA1(3b998da404bd7133d12aadcadd57ee21a0cfc226) )

	ROM_REGION( 0x0700, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "msraid_1-5_110_82s123.ic20",      0x0000, 0x0020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "msraid_21-1_211_82s129.ic4",      0x0100, 0x0100, CRC(4f8c3e63) SHA1(0aa68fa1de6ca945027366a06752e834bbbc8d09) )
	ROM_LOAD( "msraid_21-1_p0202_82s129.ic12",   0x0200, 0x0100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "msraid_51-3_502_82129.ic10",      0x0600, 0x0100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "msraid_4-3-1_pc403_pal16r8.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "msraid_4-3-2_403_pal16r8.ic29",      0x000, 0x104, CRC(16379b0d) SHA1(5379560b0ec7c67cbe131a581a347b86395f34ac) )
	ROM_LOAD( "msraid_4-3-3_p0403_pal16r8.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // yes, same as the first one
	ROM_LOAD( "msraid_51-3_503_gal16v8.ic46",       0x000, 0x117, CRC(11470ea1) SHA1(cfcafbcc7e55be717348f895df61e144fdd0cc9b) )
	ROM_LOAD( "msraid_6-1_645b_gal16v8a.ic7",       0x000, 0x117, NO_DUMP )
	ROM_LOAD( "msraid_6-1_686_ga16v8.ic13",         0x000, 0x117, NO_DUMP )
	ROM_LOAD( "msraid_6-1-8086-1_645c_gal16v8.u33", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "msraid_6-1-8086-1_645d_gal16v8.u27", 0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1990, raidenm,  raiden,  raidenm,  raidenm,  raiden_ms_state, init_raidenm, ROT270, "bootleg (Gaelco / Ervisa)", "Raiden (Modular System)", MACHINE_NOT_WORKING )
