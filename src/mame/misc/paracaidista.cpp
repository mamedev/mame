// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

/********************************************************************************************

  Paracaidista
  Video Game / Electrogame, 1979.

  Driver by Roberto Fresca & Grull Osgo.


  Paracaidista was a Spanish arcade game designed in 1979 that never saw an official release.
  This prototype was lost to time until its creator, Javier Valero, recovered the original
  EPROM contents by reconstructing the assembly code he had written decades earlier.
  Now, the game has been preserved on original hardware and also available in MAME.

  One of the most fascinating aspects of Paracaidista is its extensive use of undocumented
  and enhanced Intel 8085 instructions for arithmetic, positional calculations, timing, and
  logic operations. Combined with the complex manipulation of DMA channels via an Intel 8257,
  the clever (and borderline excessive) use of a PPI 8155 with 14-bit counters and its own
  internal RAM, among other advanced techniques, this game was remarkably ahead of its time
  for 1979.

  The hardware is composed by six PCBs connected by a small custom backplane.

  More info, schematics, and source code:
   https://www.recreativas.org/el-paracaidista-404-videogame-electrogame
   https://www.recreativas.org/paracaidista-version-2023-08-15294-videogame-electrogame

  Emulation Work In Progress:
   https://robertofresca.com/


*********************************************************************************************

  Notes:

  Routine at $0182 --> call $08de, draws the title.
  Routine at $0185 --> call $09e1, clear the screen.

  bp 0185 to see the title.


  Updates:

  [20250401]

 - Added memory and port maps.
 - Correct VRAM, work RAM, and NVRAM.
 - Hooked the PPI 8155 and connected the ports.
 - Decoded graphics, added palette.
 - Video update routines to draw the screen.
 - Hooked input port and DIP switch.
 - Accurate machine config.
 - Added technical notes.
 - Moved from /skeleton to /misc.
 - New binaries directly from the game developer.

  [20250401]

 - Working RAM through PPI 8155 internal RAM and handlers.
 - Extended the PPI 8155 to support the 14bit timer + 2bit control.
 - Partially hooked the i8257 DMA controller. Need the handlers to work.

  [20250407]

 - Rewrote the enhanced no documented i8085 RDEL & DSUB
   instructions and their own flags.
 - Demuxed the digital inputs.
 - Adjusted screen visible area.
 - Worked the DMA support to get registers in the correct addressing.
 - Hooked the analogic inputs.
 - Added inputs for two players.
 - Added DIP switches for coinage, difficulty, and lives.
 - Added and demuxed spinner controls.
 - Added NVRAM support.
 - Sound support.
 - Adjusted the spinners parameters to general purpose.
 - Sound level control circuitry.
 - Wired players lamps.
 - Added technical notes.


  TODO:

  - Interrupts through the i8257 DMA device.


********************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8257.h"
#include "machine/nvram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class paracaidista_state : public driver_device
{
public:
	paracaidista_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8155(*this, "i8155"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_dma8257(*this, "dma8257"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN%u", 0U),
		m_lamps(*this, "lamp%u", 0U)

	{ }

	void paracaidista(machine_config &config);

private:
	virtual void machine_start() override { m_lamps.resolve(); }
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void paraca_map(address_map &map) ATTR_COLD;
	void paraca_portmap(address_map &map) ATTR_COLD;
	u8 input_r();
	u8 muxed_input_r();
	void outputb_w(u8 data);
	void outputc_w(u8 data);
	void sound_out(int state);
	u8 dma_read(offs_t offset);
	void dma_write(offs_t offset, u8 data);
	u8 dmac_mem_r(offs_t offset);
	void dmac_mem_w(offs_t offset, u8 data);
	void dmac_hrq_w(int state);

	INTERRUPT_GEN_MEMBER(interrupts);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<i8155_device> m_ppi8155;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;
	required_device<i8257_device> m_dma8257;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<4> m_inputs;
	output_finder<2> m_lamps;

	// logic gates
	u8 m_sndvol = 0;
	u8 m_inputsel = 0;
	u8 m_inputasel = 0;
};


/*********************************************
*               Video Hardware               *
*********************************************/

uint32_t paracaidista_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		auto const *const src = &m_vram[y << 5];
		auto *const dst = &bitmap.pix(y);
		for (int x = cliprect.left() / 8; x <= (cliprect.right() / 8); x++)
		{
			const u8 pixel_data = src[x];

			// unpack 8 pixels from the byte
			for (int bit = 0; bit < 8; bit++)
				dst[(x << 3) | bit] = BIT(pixel_data, bit ^ 7);
		}
	}
	return 0;
}

void paracaidista_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));  // black
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));  // white
}


/*******************************************
*               R/W Handlers               *
*******************************************/

u8 paracaidista_state::input_r()
{
/*  PPI 8155 PortA IN.
    Analogic input port.

    7654 3210
    xxxx xxxx  spinner.

*/
	if(m_inputasel == 0)
		return m_inputs[2]->read();
	else
		return m_inputs[3]->read();
}


u8 paracaidista_state::muxed_input_r()
{
	if(m_inputsel == 0)
		return m_inputs[0]->read();
	else
		return m_inputs[1]->read();
}


void paracaidista_state::outputb_w(u8 data)
{
/*  PPI 8155 PortB OUT.
    Players lamps.

    7654 3210
    ---- ---x  P1 lamp.
    ---- --x-  P2 lamp.
    -xxx xx--  not connected.
    x--- ----  documented as coin counter, but it's not.
*/
	m_lamps[0] = BIT(data, 0);  // P1 lamp
	m_lamps[1] = BIT(data, 1);  // P2 lamp
}

void paracaidista_state::outputc_w(u8 data)
{
/*  PPI 8155 PortC OUT.
    Inputs selectors.

    7654 3210
    ---- ---x  PC0: not used.
    ---- --x-  PC1: sound volume control, bit 0
    ---- -x--  PC2: sound volume control, bit 1
    ---- x---  PC3: analogic input mux selector, bit 0.
    ---x ----  PC4: analogic input mux selector, bit 1.
    --x- ----  PC5: digital inputs mux selector.
    -x-- ----  PC6: not used.
    x--- ----  PC7: not used.
*/

	m_sndvol = (data & 0x06) >> 1 ;  // PC1 & PC2
	m_inputasel = BIT(data, 3);      // using only PC3 due to the existence of only two analogic devices.
	m_inputsel = BIT(data, 5);       // PC5 - input selector (2 muxed inputs sets)
	logerror("outc: sound vol: %02x\n", m_sndvol);
}

void paracaidista_state::sound_out(int state)
{
	logerror("sound level: %02x\n", (state*4) + m_sndvol);
	m_speaker->level_w( (state * 4 ) + m_sndvol );
}


/*******************************************
*               DMA Handlers               *
*******************************************/

u8 paracaidista_state::dma_read(offs_t offset)
{
//  DMA 8257 address lines 0-7 are going to CPU address lines 2-9
	return m_dma8257->read(offset >> 2);
}

void paracaidista_state::dma_write(offs_t offset, u8 data)
{
//  DMA 8257 address lines 0-7 are going to CPU address lines 2-9
	m_dma8257->write(offset >> 2, data);
}

void paracaidista_state::dmac_hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma8257->hlda_w(state);
}

u8 paracaidista_state::dmac_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void paracaidista_state::dmac_mem_w(offs_t offset, u8 data)
{
	return m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}


/*******************************************
*           Interrupts Handling            *
*******************************************/

INTERRUPT_GEN_MEMBER(paracaidista_state::interrupts)
{
	device.execute().set_input_line(I8085_RST65_LINE, HOLD_LINE);
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void paracaidista_state::paraca_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x23ff).rom();
	map(0x3400, 0x342f).rw(FUNC(paracaidista_state::dma_read), FUNC(paracaidista_state::dma_write));
	map(0x3800, 0x3800).portr("DSW");
	map(0x3c00, 0x3c00).r(FUNC(paracaidista_state::muxed_input_r));
	map(0x3e00, 0x3eff).ram().share("nvram"); // CMOS RAM 256x4
	map(0x4000, 0x5fff).ram().share("vram");  // video RAM (size 1A00h)
	map(0x6000, 0x60ff).rw(m_ppi8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));  // work RAM + stack
	map(0x7001, 0x7001).noprw();              // debugger LEDs to see the status
}

void paracaidista_state::paraca_portmap(address_map &map)
{
/*  060H  ; Port status register
    061H  ; I/O PortA
    062H  ; I/O PortB
    063H  ; PortC I/O Control

    064H  ; Timer 8 lower bits
    065H  ; Timer 6 higher bits + 2 control bits
*/
	map.global_mask(0xff);
	map(0x60, 0x67).rw(m_ppi8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));  // PPI handles ports and timer
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START(paracaidista)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_PLAYER(2) PORT_CODE(KEYCODE_S) PORT_NAME("P2 Long Fire")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_PLAYER(1) PORT_CODE(KEYCODE_X) PORT_NAME("P1 Long Fire")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN1-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN1-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN1-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN1-80")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_PLAYER(2) PORT_CODE(KEYCODE_A) PORT_NAME("P2 Short Fire")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_PLAYER(1) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 Short Fire")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN0-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN0-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN0-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN0-80")

	PORT_START("IN2")  // P1 spinner
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(1) PORT_NAME("P1 Spinner")

	PORT_START("IN3")  // P2 spinner
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(2) PORT_NAME("P2 Spinner")


	PORT_START("DSW")  // 3800h
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )       PORT_DIPLOCATION("DSW:5,6")
	PORT_DIPSETTING(    0x00, "03" )
	PORT_DIPSETTING(    0x01, "04" )
	PORT_DIPSETTING(    0x02, "05" )
	PORT_DIPSETTING(    0x03, "06" )

	// Coin B always is set to 1c-1c
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A" )               PORT_DIPLOCATION("DSW:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )  // first coin = 1; second coin = 2
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0xa0, 0xa0, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0xa0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	// The last two routed seems unused
	PORT_DIPNAME( 0x10, 0x10, "DSW #7" )               PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW #8" )               PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*********************************************/

void paracaidista_state::paracaidista(machine_config &config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, 6'553'600);
	m_maincpu->set_addrmap(AS_PROGRAM, &paracaidista_state::paraca_map);
	m_maincpu->set_addrmap(AS_IO, &paracaidista_state::paraca_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(paracaidista_state::interrupts));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8155(config, m_ppi8155, 6'553'600 / 16);  // port A input, B output, C special mode 2
	m_ppi8155->in_pa_callback().set(FUNC(paracaidista_state::input_r));
	m_ppi8155->out_pb_callback().set(FUNC(paracaidista_state::outputb_w));
	m_ppi8155->out_pc_callback().set(FUNC(paracaidista_state::outputc_w));
	m_ppi8155->out_to_callback().set(FUNC(paracaidista_state::sound_out));

	I8257(config, m_dma8257, 6'553'600 / 2);
	m_dma8257->out_tc_cb().set_inputline(m_maincpu, I8085_RST65_LINE);   // under test
	m_dma8257->out_hrq_cb().set(FUNC(paracaidista_state::dmac_hrq_w));   // tied to HALT(HOLD) 8085 line
	m_dma8257->in_memr_cb().set(FUNC(paracaidista_state::dmac_mem_r));   // under test
	m_dma8257->out_memw_cb().set(FUNC(paracaidista_state::dmac_mem_w));  // under test

	// video hardware
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(paracaidista_state::screen_update));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 208-1);

	screen.set_palette("palette");
	PALETTE(config, m_palette, FUNC(paracaidista_state::palette), 2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[8] = { 0, 0, 0, 0, 1.0, 0.5, 0.4, 0.2 };
	m_speaker->set_levels(8, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( paraca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.a3", 0x0000, 0x0400, CRC(7068953a) SHA1(ff91d719ca2e05bf71d555c1f4da37debe35e679) )
	ROM_LOAD( "rom2.a2", 0x0400, 0x0400, CRC(99a58854) SHA1(ab223873cf451a4a42a747c801705b4084c09abd) )
	ROM_LOAD( "rom3.a1", 0x0800, 0x0400, CRC(a6e9939d) SHA1(8067db64f1479912bbffb56131a253c16b82d874) )
	ROM_LOAD( "rom4.b3", 0x0C00, 0x0400, CRC(f30df1bd) SHA1(24030a34f8557969f99c25a7876b49a176bf0bf3) )
	ROM_LOAD( "rom5.b2", 0x1000, 0x0400, CRC(298c863b) SHA1(7a079ae63e308641bf971004060dcd33e54e75cb) )
	ROM_LOAD( "rom6.b1", 0x1400, 0x0400, CRC(b314be9d) SHA1(264af4312ae4262a0105e556430070e299750712) )
	ROM_LOAD( "rom7.c3", 0x1800, 0x0400, CRC(f2ae0152) SHA1(719242732811b9b183469c619195b147828fa539) )
	ROM_LOAD( "rom8.c2", 0x1c00, 0x0400, CRC(a311057c) SHA1(5c2d43b3982d54653bb9170dd148d0eaf967bae6) )
	ROM_LOAD( "rom9.c1", 0x2000, 0x0400, CRC(64f44bb3) SHA1(11b70a1fd6457d0d04eb647bcedcf63d14212cde) )
ROM_END


} // Anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME     PARENT  MACHINE       INPUT         CLASS               INIT        ROT   COMPANY                     FULLNAME        FLAGS
GAME( 1979, paraca,  0,      paracaidista, paracaidista, paracaidista_state, empty_init, ROT0, "Video Game / Electrogame", "Paracaidista", 0 )
