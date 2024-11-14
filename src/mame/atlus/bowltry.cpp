// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/************************************************************************************************************

    Bowling Try!

    (c)200? Atlus

    TODO:
    - Decompress YGV631 GFXs (RGB555/RGB565 format?)
    - Add base YGV631 display parsing;
    - TT5665 sound interface doesn't quite work, also definitely missing bankswitching behaviour.
      Notice that TT5665 works in 0x100000 data ROM banks, but only the first part seems to
      have a valid table header?
    - serial comms, very verbose;
    - I/O, we currently add aggressive hookups for debugging reasons;
    - Understand how exactly the mechanical part comes into play.
      The only thing available from the net is a cabinet picture, depicting a long bowling lane.
      There's no video of this in action, the speculation is that there's a physical ball and pins
      that are drawn OSD, and touchscreen sensors that determines hitting point.
    - Are we missing an irq event here?
      Whatever is happening with rand hookups it doesn't seem to have much impact on the game logic
      (i.e. serial always sends a P1 0000, never a P2 or an actual positive score), or even acknowledge the
      likely coin insertions, definitely needs some video display to verify.

    ATLUS PCB  BT-208001
    ------------------------

    At U12 the chip is Toshiba TA8428FG

    At U1 the chip is H8/3008

    At X1 on the crystal it is printed S753

    big gfx chip marked

    YAMAHA JAPAN
    YGV631-B
    0806LU004

************************************************************************************************************/


#include "emu.h"
#include "cpu/h8/h83008.h"
#include "sound/tt5665.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bowltry_state : public driver_device
{
public:
	bowltry_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_adpcm(*this, "tt5665")
		, m_vregs(*this, "vregs")
	{ }

	void bowltry(machine_config &config);

protected:
	void bowltry_map(address_map &map) ATTR_COLD;

	uint32_t screen_update_bowltry(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 fake_io_r(offs_t offset);
	u16 vregs_r(offs_t offset, u16 mem_mask = ~0);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	required_device<h83008_device> m_maincpu;
	required_device<tt5665_device> m_adpcm;
	required_shared_ptr<u16> m_vregs;
};

uint32_t bowltry_state::screen_update_bowltry(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// YGV display list starts off at top of the VRAM stack,
	// and executes in 0x14 byte chunks
	// [0x0/2] control word
	// 0x8001 always on first entry, jump link (to next entry anyway)?
	// 0x0000 on first list POST, used in tandem with entry [2] bit 15 high may be ignore or fill
	// 0x4000 on first list POST, same as above plus end of list marker
	// 0x2000 on normal execution, draw this object
	// 0x6000 on normal execution, draw plus end of list marker (which is either tail or a cut short of a few objects earlier)
	// [0xa/2] object number?
	// [0xc/2] Y coordinate?
	// [0xe/2] X coordinate?
	// everything else is always filled with the same data,
	// best guess is that SW fills to whatever necessary to "draw a normal sprite with no zoom/transparency etc. etc."

	return 0;
}

u16 bowltry_state::vregs_r(offs_t offset, u16 mem_mask)
{
	// ---- ---x: vblank
	if (offset == 0x090/2)
		return (m_vregs[offset] & ~1) | (machine().rand() & 1);

	// --x- ---- ---- ----: DMA enable?
	// ---- ---- --x- ----: display list trigger, writes 1 then tight loops expecting to clear
	// ---- ---- ---- x---: unknown, may be DMA mode
	if (offset == 0x092/2)
		return (m_vregs[offset] & ~0x20) | (machine().rand() & 0x20);

	// other registers sets up stuff like cliprect coords (no CRTC?)
	// hard to say without decoded GFXs.

	return m_vregs[offset];
}

void bowltry_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vregs[offset]);
}

u16 bowltry_state::fake_io_r(offs_t offset)
{
	return machine().rand();
}

void bowltry_state::bowltry_map(address_map &map)
{
//  map.unmap_value_high();
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x080000, 0x083fff).ram();
	// These two are clearly 8-bit inputs
	map(0x200000, 0x200001).r(FUNC(bowltry_state::fake_io_r));
	map(0x200002, 0x200003).r(FUNC(bowltry_state::fake_io_r));
	map(0x200004, 0x200005).nopw();
	map(0x240000, 0x240001).nopw();
	// routine at PC=0x8db8 does some intricate checks with the status and work RAM buffers
	// if successful it eventually writing at PC=8e28 and onward, including OKI style writings for the other chip as well
	// current hookup never gives a valid write except for a 0x78 at POST on both channels ...
	// TODO: we also currently hook this up as mirror but the specs definitely mentions having separate L & R channel routings unlike 6295
	map(0x400003, 0x400003).rw(m_adpcm, FUNC(tt5665_device::read), FUNC(tt5665_device::write));
	map(0x400005, 0x400005).rw(m_adpcm, FUNC(tt5665_device::read), FUNC(tt5665_device::write));
	// ygv VRAM, TBD boundaries, may not be all of it?
	// SW just aimlessly clears everything in the range at PC=5f28
	map(0x600000, 0x60dfff).ram();
	map(0x60e000, 0x60ffff).rw(FUNC(bowltry_state::vregs_r), FUNC(bowltry_state::vregs_w)).share("vregs");
}

static INPUT_PORTS_START( bowltry )
INPUT_PORTS_END

void bowltry_state::bowltry(machine_config &config)
{
	H83008(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bowltry_state::bowltry_map);
	// uses vector $64, IMIAB according to the manual (timer/compare B, internal to the CPU)
	// no vblank, most likely handled by YGV video register bit at 0x090/2
	// TODO: serial hookup, comms with a LED type ring display?
	// it sometimes feeds with "P1" and four zeroes

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(bowltry_state::screen_update_bowltry));
	//screen.set_palette("palette");

	//PALETTE(config, "palette").set_entries(65536);

	/* tt5665 sound */
	SPEAKER(config, "speaker").front_center();

	TT5665(config, "tt5665", 16000000/4, tt5665_device::ss_state::SS_HIGH, 0).add_route(1, "speaker", 1.0); // clock and SS pin unverified
}

ROM_START( bowltry )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30_v1.00.u30", 0x000000, 0x080000, CRC(2bd47419) SHA1(8fc975340e47ddeedf96e454a6c5372328f28b72) )

	ROM_REGION( 0x800000, "gfx", 0 )
	// TODO: confirm arrangement
	ROM_LOAD16_BYTE( "u27_v1.00.u27", 0x000000, 0x400000, CRC(80f51c25) SHA1(53c21325e7796197c26ca0cf4f8e51bf1e0bdcd3) )
	ROM_LOAD16_BYTE( "u28_v1.00.u28", 0x000001, 0x400000, CRC(9cc8b577) SHA1(6ef5cbb83860f88c9c83d4410034c5b528b2138b) )

	ROM_REGION( 0x400000, "tt5665", 0 ) // sound
	ROM_LOAD( "u24_v1.00.u24", 0x000000, 0x400000, CRC(4e082d58) SHA1(d2eb58bc3d8ade2ea556960013d580f0fb952090) )
ROM_END

} // anonymous namespace


GAME( 200?, bowltry, 0, bowltry, bowltry, bowltry_state, empty_init, ROT0, "Atlus", "Bowling Try!", MACHINE_IS_SKELETON_MECHANICAL )
