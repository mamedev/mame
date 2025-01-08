// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Atari "Stella on Steroids" hardware

****************************************************************************

    Games supported:
        * BeatHead

    Known bugs:
        * none known

****************************************************************************

    Memory map

    ===================================================================================================
    MAIN CPU
    ===================================================================================================
    00000000-0001FFFFF  R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main RAM
    01800000-01BFFFFFF  R     xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main ROM
    40000000-4000007FF  R/W   -------- -------- -------- xxxxxxxx   EEPROM
    41000000            R     -------- -------- -------- xxxxxxxx   Data from sound board
    41000000              W   -------- -------- -------- xxxxxxxx   Data to sound board
    41000100            R     -------- -------- -------- -----xxx   Interrupt enables
                              -------- -------- -------- -----x--      (scanline int enable)
                              -------- -------- -------- ------x-      (unknown int enable)
                              -------- -------- -------- -------x      (unknown int enable)
    41000100              W   -------- -------- -------- --------   Interrupt acknowledge
    41000104              W   -------- -------- -------- --------   Unknown int disable
    41000108              W   -------- -------- -------- --------   Unknown int disable
    4100010c              W   -------- -------- -------- --------   Scanline int disable
    41000114              W   -------- -------- -------- --------   Unknown int enable
    41000118              W   -------- -------- -------- --------   Unknown int enable
    4100011c              W   -------- -------- -------- --------   Scanline int enable
    41000200            R     -------- -------- xxxx--xx xxxx--xx   Player 2/3 inputs
                        R     -------- -------- xxxx---- --------      (player 3 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 3 button 1)
                        R     -------- -------- -------x --------      (player 3 button 2)
                        R     -------- -------- -------- xxxx----      (player 2 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 2 button 1)
                        R     -------- -------- -------- -------x      (player 2 button 2)
    41000204            R     -------- -------- xxxx--xx xxxx--xx   Player 1/4 inputs
                        R     -------- -------- xxxx---- --------      (player 1 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 1 button 1)
                        R     -------- -------- -------x --------      (player 1 button 2)
                        R     -------- -------- -------- xxxx----      (player 4 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 4 button 1)
                        R     -------- -------- -------- -------x      (player 4 button 2)
    41000208              W   -------- -------- -------- --------   Sound /RESET assert
    4100020C              W   -------- -------- -------- --------   Sound /RESET deassert
    41000220              W   -------- -------- -------- --------   Coin counter assert
    41000224              W   -------- -------- -------- --------   Coin counter deassert
    41000300            R     -------- -------- xxxxxxxx -xxx----   DIP switches/additional inputs
                        R     -------- -------- xxxxxxxx --------      (debug DIP switches)
                        R     -------- -------- -------- -x------      (service switch)
                        R     -------- -------- -------- --x-----      (sound output buffer full)
                        R     -------- -------- -------- ---x----      (sound input buffer full)
    41000304            R     -------- -------- -------- xxxxxxxx   Coin/service inputs
                        R     -------- -------- -------- xxxx----      (service inputs: R,RC,LC,L)
                        R     -------- -------- -------- ----xxxx      (coin inputs: R,RC,LC,L)
    41000400              W   -------- -------- -------- -xxxxxxx   Palette select
    41000500              W   -------- -------- -------- --------   EEPROM write enable
    41000600              W   -------- -------- -------- ----xxxx   Finescroll, vertical SYNC flags
                          W   -------- -------- -------- ----x---      (VBLANK)
                          W   -------- -------- -------- -----x--      (VSYNC)
                          W   -------- -------- -------- ------xx      (fine scroll value)
    41000700              W   -------- -------- -------- --------   Watchdog reset
    42000000-4201FFFF   R/W   -------- -------- xxxxxxxx xxxxxxxx   Palette RAM
                        R/W   -------- -------- x------- --------      (LSB of all three components)
                        R/W   -------- -------- -xxxxx-- --------      (red component)
                        R/W   -------- -------- ------xx xxx-----      (green component)
                        R/W   -------- -------- -------- ---xxxxx      (blue component)
    43000000              W   -------- -------- ----xxxx xxxxxxxx   HSYNC RAM address latch
                          W   -------- -------- ----x--- --------      (counter enable)
                          W   -------- -------- -----xxx xxxxxxxx      (RAM address)
    43000004            R/W   -------- -------- -------- xxxxx---   HSYNC RAM data latch
                        R/W   -------- -------- -------- x-------      (generate IRQ)
                        R/W   -------- -------- -------- -x------      (VRAM shift enable)
                        R/W   -------- -------- -------- --x-----      (HBLANK)
                        R/W   -------- -------- -------- ---x----      (/HSYNC)
                        R/W   -------- -------- -------- ----x---      (release wait for sync)
    43000008              W   -------- -------- -------- ---x-xx-   HSYNC unknown control
    8DF80000            R     -------- -------- -------- --------   Unknown
    8F380000-8F3FFFFF     W   -------- -------- -------- --------   VRAM latch address
    8F900000-8F97FFFF     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM transparent write
    8F980000-8F9FFFFF   R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM standard read/write
    8FB80000-8FBFFFFF     W   ----xxxx ----xxxx ----xxxx ----xxxx   VRAM "bulk" write
                          W   ----xxxx -------- -------- --------      (enable byte lanes for word 3?)
                          W   -------- ----xxxx -------- --------      (enable byte lanes for word 2?)
                          W   -------- -------- ----xxxx --------      (enable byte lanes for word 1?)
                          W   -------- -------- -------- ----xxxx      (enable byte lanes for word 0?)
    8FFF8000              W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM "bulk" data latch
    9E280000-9E2FFFFF     W   -------- -------- -------- --------   VRAM copy destination address latch
    ===================================================================================================

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"

#include "cpu/asap/asap.h"
#include "machine/eeprompar.h"
#include "machine/timer.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class beathead_state : public driver_device
{
public:
	beathead_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_scan_timer(*this, "scan_timer"),
		m_videoram(*this, "videoram"),
		m_vram_bulk_latch(*this, "vram_bulk_latch"),
		m_palette_select(*this, "palette_select"),
		m_ram_base(*this, "ram_base"),
		m_rom_base(*this, "maincpu")
	{ }

	void beathead(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<timer_device> m_scan_timer;

	required_shared_ptr<uint32_t> m_videoram;
	required_shared_ptr<uint32_t> m_vram_bulk_latch;
	required_shared_ptr<uint32_t> m_palette_select;
	required_shared_ptr<uint32_t> m_ram_base;
	required_region_ptr<uint32_t> m_rom_base;

	uint32_t m_finescroll = 0U;
	offs_t m_vram_latch_offset = 0U;

	offs_t m_hsyncram_offset = 0U;
	offs_t m_hsyncram_start = 0U;
	uint8_t m_hsyncram[0x800]{};

	attotime m_hblank_offset;

	uint8_t m_irq_line_state = 0U;
	uint8_t m_irq_enable[3]{};
	uint8_t m_irq_state[3]{};

	static constexpr int MAX_SCANLINES = 262;

	void update_interrupts();
	void interrupt_control_w(offs_t offset, uint32_t data);
	uint32_t interrupt_control_r();
	void sound_reset_w(offs_t offset, uint32_t data);
	void coin_count_w(offs_t offset, uint32_t data);
	void vram_transparent_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vram_bulk_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vram_latch_w(offs_t offset, uint32_t data);
	void vram_copy_w(offs_t offset, uint32_t data);
	void finescroll_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hsync_ram_r(offs_t offset);
	void hsync_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);


	void main_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  Video start/stop
 *
 *************************************/

void beathead_state::video_start()
{
	save_item(NAME(m_finescroll));
	save_item(NAME(m_vram_latch_offset));
	save_item(NAME(m_hsyncram_offset));
	save_item(NAME(m_hsyncram_start));
	save_item(NAME(m_hsyncram));

	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_state));

	m_hsyncram_offset = 0;
}



/*************************************
 *
 *  VRAM handling
 *
 *************************************/

void beathead_state::vram_transparent_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// writes to this area appear to handle transparency
	if (!(data & 0x000000ff)) mem_mask &= ~0x000000ff;
	if (!(data & 0x0000ff00)) mem_mask &= ~0x0000ff00;
	if (!(data & 0x00ff0000)) mem_mask &= ~0x00ff0000;
	if (!(data & 0xff000000)) mem_mask &= ~0xff000000;
	COMBINE_DATA(&m_videoram[offset]);
}


void beathead_state::vram_bulk_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// it appears that writes to this area pass in a mask for 4 words in VRAM
	// allowing them to be filled from a preset latch
	offset &= ~3;
	data = data & mem_mask & 0x0f0f0f0f;

	// for now, just handle the bulk fill case; the others we'll catch later
	if (data == 0x0f0f0f0f)
		m_videoram[offset + 0] = m_videoram[offset + 1] = m_videoram[offset + 2] = m_videoram[offset + 3] = *m_vram_bulk_latch;
	else
		logerror("Detected bulk VRAM write with mask %08x\n", data);
}


void beathead_state::vram_latch_w(offs_t offset, uint32_t data)
{
	// latch the address
	m_vram_latch_offset = (4 * offset) & 0x7ffff;
}


void beathead_state::vram_copy_w(offs_t offset, uint32_t data)
{
	// copy from VRAM to VRAM, for 1024 bytes
	offs_t const dest_offset = (4 * offset) & 0x7ffff;
	memcpy(&m_videoram[dest_offset / 4], &m_videoram[m_vram_latch_offset / 4], 0x400);
}



/*************************************
 *
 *  Scroll offset handling
 *
 *************************************/

void beathead_state::finescroll_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const oldword = m_finescroll;
	uint32_t const newword = COMBINE_DATA(&m_finescroll);

	// if VBLANK is going off on a scanline other than the last, suspend time
	if ((oldword & 8) && !(newword & 8) && m_screen->vpos() != 261)
	{
		logerror("Suspending time! (scanline = %d)\n", m_screen->vpos());
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}



/*************************************
 *
 *  HSYNC RAM handling
 *
 *************************************/

uint32_t beathead_state::hsync_ram_r(offs_t offset)
{
	// offset 0 is probably write-only
	if (offset == 0)
		logerror("%08X:Unexpected HSYNC RAM read at offset 0\n", m_maincpu->pcbase());

	// offset 1 reads the data
	else
		return m_hsyncram[m_hsyncram_offset];

	return 0;
}

void beathead_state::hsync_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// offset 0 selects the address, and can specify the start address
	if (offset == 0)
	{
		COMBINE_DATA(&m_hsyncram_offset);
		if (m_hsyncram_offset & 0x800)
			m_hsyncram_start = m_hsyncram_offset & 0x7ff;
	}

	// offset 1 writes the data
	else
		COMBINE_DATA(&m_hsyncram[m_hsyncram_offset]);
}



/*************************************
 *
 *  Main screen refresher
 *
 *************************************/

uint32_t beathead_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const auto videoram = util::little_endian_cast<const uint8_t>(m_videoram.target());

	// generate the final screen
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const pen_t pen_base = (*m_palette_select & 0x7f) * 256;
		uint16_t scanline[336];

		if (m_finescroll & 8)
		{
			// blanking
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				scanline[x] = pen_base;
		}
		else
		{
			// non-blanking
			const offs_t scanline_offset = m_vram_latch_offset + (m_finescroll & 3);
			offs_t src = scanline_offset + cliprect.left();

			// unswizzle the scanline first
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				scanline[x] = pen_base | videoram[src++];
		}

		// then draw it
		draw_scanline16(bitmap, cliprect.left(), y, cliprect.width(), &scanline[cliprect.left()], nullptr);
	}
	return 0;
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(beathead_state::scanline_callback)
{
	int scanline = param;

	// update the video
	m_screen->update_partial(m_screen->vpos());

	// on scanline zero, clear any halt condition
	if (scanline == 0)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// wrap around at 262
	scanline++;
	if (scanline >= MAX_SCANLINES)
		scanline = 0;

	// set the scanline IRQ
	m_irq_state[2] = 1;
	update_interrupts();

	// set the timer for the next one
	m_scan_timer->adjust(m_screen->time_until_pos(scanline) - m_hblank_offset, scanline);
}

void beathead_state::machine_reset()
{
	// the code is temporarily mapped at 0 at startup
	// just copying the first 0x40 bytes is sufficient
	memcpy(m_ram_base, m_rom_base, 0x40);

	// compute the timing of the HBLANK interrupt and set the first timer
	m_hblank_offset = m_screen->scan_period() * (455 - 336 - 25) / 455;

	m_scan_timer->adjust(m_screen->time_until_pos(0) - m_hblank_offset);

	// reset IRQs
	m_irq_line_state = CLEAR_LINE;
	m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;
	m_irq_enable[0] = m_irq_enable[1] = m_irq_enable[2] = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void beathead_state::update_interrupts()
{
	// compute the combined interrupt signal
	int gen_int  = m_irq_state[0] & m_irq_enable[0];
	gen_int |= m_irq_state[1] & m_irq_enable[1];
	gen_int |= m_irq_state[2] & m_irq_enable[2];
	gen_int  = gen_int ? ASSERT_LINE : CLEAR_LINE;

	// if it's changed since the last time, call through
	if (m_irq_line_state != gen_int)
	{
		m_irq_line_state = gen_int;
		//if (m_irq_line_state != CLEAR_LINE)
			m_maincpu->set_input_line(ASAP_IRQ0, m_irq_line_state);
		//else
			//asap_set_irq_line(ASAP_IRQ0, m_irq_line_state);
	}
}


void beathead_state::interrupt_control_w(offs_t offset, uint32_t data)
{
	int const irq = offset & 3;
	int const control = (offset >> 2) & 1;

	// offsets 1-3 seem to be the enable latches for the IRQs
	if (irq != 0)
		m_irq_enable[irq - 1] = control;

	// offset 0 seems to be the interrupt ack
	else
		m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;

	// update the current state
	update_interrupts();
}


uint32_t beathead_state::interrupt_control_r()
{
	// return the enables as a bitfield
	return (m_irq_enable[0]) | (m_irq_enable[1] << 1) | (m_irq_enable[2] << 2);
}



/*************************************
 *
 *  Sound communication
 *
 *************************************/

void beathead_state::sound_reset_w(offs_t offset, uint32_t data)
{
	logerror("Sound reset = %d\n", !offset);
	m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, offset ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Misc other I/O
 *
 *************************************/

void beathead_state::coin_count_w(offs_t offset, uint32_t data)
{
	machine().bookkeeping().coin_counter_w(0, !offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void beathead_state::main_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram().share(m_ram_base);
	map(0x01800000, 0x01bfffff).rom().region("maincpu", 0);
	map(0x40000000, 0x400007ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask32(0x000000ff);
	map(0x41000000, 0x41000000).rw(m_jsa, FUNC(atari_jsa_iii_device::main_response_r), FUNC(atari_jsa_iii_device::main_command_w));
	map(0x41000100, 0x41000103).r(FUNC(beathead_state::interrupt_control_r));
	map(0x41000100, 0x4100011f).w(FUNC(beathead_state::interrupt_control_w));
	map(0x41000200, 0x41000203).portr("IN1");
	map(0x41000204, 0x41000207).portr("IN0");
	map(0x41000208, 0x4100020f).w(FUNC(beathead_state::sound_reset_w));
	map(0x41000220, 0x41000227).w(FUNC(beathead_state::coin_count_w));
	map(0x41000300, 0x41000303).portr("IN2");
	map(0x41000304, 0x41000307).portr("IN3");
	map(0x41000400, 0x41000403).writeonly().share(m_palette_select);
	map(0x41000500, 0x41000500).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write8));
	map(0x41000600, 0x41000603).w(FUNC(beathead_state::finescroll_w));
	map(0x41000700, 0x41000703).w("watchdog", FUNC(watchdog_timer_device::reset32_w));
	map(0x42000000, 0x4201ffff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).umask32(0x0000ffff).share("palette");
	map(0x43000000, 0x43000007).rw(FUNC(beathead_state::hsync_ram_r), FUNC(beathead_state::hsync_ram_w));
	map(0x8df80000, 0x8df80003).nopr(); // noisy x4 during scanline int
	map(0x8f380000, 0x8f3fffff).w(FUNC(beathead_state::vram_latch_w));
	map(0x8f900000, 0x8f97ffff).w(FUNC(beathead_state::vram_transparent_w));
	map(0x8f980000, 0x8f9fffff).ram().share(m_videoram);
	map(0x8fb80000, 0x8fbfffff).w(FUNC(beathead_state::vram_bulk_w));
	map(0x8fff8000, 0x8fff8003).writeonly().share(m_vram_bulk_latch);
	map(0x9e280000, 0x9e2fffff).w(FUNC(beathead_state::vram_copy_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( beathead )
	PORT_START("IN0")       // Player 1
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")       // Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

// to do
//  PORT_MODIFY("jsa:JSAIII")
// coin 1+2 import from JSAIII not used - set to unused
//  PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void beathead_state::beathead(machine_config &config)
{
	// basic machine hardware
	ASAP(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &beathead_state::main_map);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	TIMER(config, m_scan_timer).configure_generic(FUNC(beathead_state::scanline_callback));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(beathead_state::screen_update));
	m_screen->set_size(42*8, 262);
	m_screen->set_visarea(0*8, 42*8-1, 0*8, 30*8-1);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette);
	m_palette->set_format(palette_device::IRGB_1555, 32768);
	m_palette->set_membits(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_III(config, m_jsa, 0);
	m_jsa->test_read_cb().set_ioport("IN2").bit(6);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 0.6);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( beathead )
	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "bhsnd.bin",  0x00000, 0x10000, CRC(dfd33f02) SHA1(479a4838c89691d5a4654a4cd84b6433a9e86109) )

	ROM_REGION32_LE( 0x400000, "maincpu", 0 ) // ASAP code
	ROM_LOAD32_BYTE( "bhprog0.bin", 0x000000, 0x80000, CRC(87975721) SHA1(862cb3a290c829aedea26ee7100c50a12e9517e7) )
	ROM_LOAD32_BYTE( "bhprog1.bin", 0x000001, 0x80000, CRC(25d89743) SHA1(9ff9a41355aa6914efc4a44909026e648a3c40f3) )
	ROM_LOAD32_BYTE( "bhprog2.bin", 0x000002, 0x80000, CRC(87722609) SHA1(dbd766fa57f4528702a98db28ae48fb5d2a7f7df) )
	ROM_LOAD32_BYTE( "bhprog3.bin", 0x000003, 0x80000, CRC(a795d616) SHA1(d3b201be62486f3b12e1b20c4694eeff0b4e3fca) )
	ROM_LOAD32_BYTE( "bhpics0.bin", 0x200000, 0x80000, CRC(926bf65d) SHA1(49f25a2844ca1cd940d17fc56c0d2698e95e0e1d) )
	ROM_LOAD32_BYTE( "bhpics1.bin", 0x200001, 0x80000, CRC(a8f12e41) SHA1(693cb7a2510f34af5442870a6ae4d19445d991f9) )
	ROM_LOAD32_BYTE( "bhpics2.bin", 0x200002, 0x80000, CRC(00b96481) SHA1(39daa46321c1d4f8bce8c25d0450b97f1f19dedb) )
	ROM_LOAD32_BYTE( "bhpics3.bin", 0x200003, 0x80000, CRC(99c4f1db) SHA1(aba4440c5cdf413f970a0c65457e2d1b37caf2d6) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 ) // ADPCM
	ROM_LOAD( "bhpcm0.bin",  0x00000, 0x20000, CRC(609ca626) SHA1(9bfc913fc4c3453b132595f8553245376bce3a51) )
	ROM_LOAD( "bhpcm1.bin",  0x20000, 0x20000, CRC(35511509) SHA1(41294b81e253db5d2f30f8589dd59729a31bb2bb) )
	ROM_LOAD( "bhpcm2.bin",  0x40000, 0x20000, CRC(f71a840a) SHA1(09d045552704cd1434307f9a36ce03c5c06a8ff6) )
	ROM_LOAD( "bhpcm3.bin",  0x60000, 0x20000, CRC(fedd4936) SHA1(430ed894fa4bfcd56ee5a8a8ef5e161246530e2d) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, beathead, 0, beathead, beathead, beathead_state, empty_init, ROT0, "Atari Games", "BeatHead (prototype)", MACHINE_SUPPORTS_SAVE )
