// license: BSD-3-Clause
// copyright-holders:Sergio Galiano
/***************************************************************************

  NMK IRQ Generator

-- Original docs from nmk/nmk16.cpp:

  This hardware relies on two counters and the contents of two PROMs for the timing signals generation.
  The counters are implemented inside NMK902 custom chip (except tharrier) for all "low-res" games and are used to
  address the entries on each PROM in a sequential way. On "mid-res" and "hi-res" games, one of the counters is
  implemented outside the custom chip, due to they decided to boost up the horizontal resolution, and the initial
  configuration is hardwired inside the chip.

  - "Horizontal" signals, such as HBlank, HSync... are generated using one of the counters and a 256x4bit PROM, and
    each step on the counter takes 2 pixel clock cycles:
    - For "low-res" games the counter starts on 0x40 and goes to 0xFF having 192 steps. As each step is 2 px, the total
      H-size is 384px wide. PROM entries from 0x00 to 0x39 address are never used.
    - For "mid-res" games the counter starts on 0x20 and goes to 0xFF having 224 steps. As each step is 2 px, the total
      H-size is 448px wide. PROM entries from 0x00 to 0x19 address are never used.
    - For  "hi-res" games the counter starts on 0x00 and goes to 0xFF having 256 steps. As each step is 2 px, the total
      H-size is 512px wide. All PROM entries are used.

  - "Vertical" signals, such as VBlank, Interrupt requests... are generated using the other counter and a 256x8bit PROM,
    and each step on the counter takes 2 scanlines.
    - In this case, for all games the counter starts on 0x75 and goes to 0xFF having 139 steps. As each step is 2 lines,
      the total V-size is 278 lines high. PROM entries from 0x00 address to 0x74 are never used.

  Going into more detail:

  The H-timing PROMs is a 82S129 (256x4bit) (first 64 entries are not used in "low-res", and first 32 entries are not
  used in "mid-res"). The format is:

    Offset  Bits     Description
            3210
    0       ---x     Line counter trigger (active low)
    1       --x-     HSYNC (active low)
    2       -x--     HBLANK (active low)
    3       x---     unused on almost all games (only used in gunnail and raphero, purpose unknown)

  Considering that and looking at the contents of the PROMs, the horizontal timings are below:

  - For "low-res" (6MHz pixel clock):
                           0.....31.........91.....................................................................348.....366.....383
  /LINE-END (2 px):        ----------------------------------------------------------------------------------------------------------X
  HSYNC  (32 px):          XXXXXXXX---------------------------------------------------------------------------------------------------
  HBLANK (92 + 36 px):     XXXXXXXXXXXXXXXXXXX------------------------------256-wide-------------------------------XXXXXXXXXXXXXXXXXXX  // HBlank ends 92 pixels after 'start of line'
                           ^
                           |
                   'start of line'  (pixel 0)
  Each line: ( 6MHz / 384 pixels per line ) = 15625Hz = 64 usec


  - For "mid-res" (7MHz pixel clock):
                           0........59..........................................................................380...400...416....447
  /LINE-END (2 px):        ----------------------------------------------------------------------------------------------------------X
  HSYNC  (32 px):          -------------------------------------------------------------------------------------------------XXXXXXXXXX
  HBLANK (60 + 68 px):     XXXXXXXXXXX--------------------------------------320-wide----------------------------XXXXXXXXXXXXXXXXXXXXXX  // HBlank ends 60 pixels after 'start of line'
                           ^
                           |
                   'start of line'  (pixel 0)
  Each line: ( 7MHz / 448 pixels per line ) = 15625Hz = 64 usec


  - For "hi-res" (8MHz pixel clock):
                           0...27............................................................................412...432...448.479...511
  /LINE-END (2 px):        ----------------------------------------------------------------------------------------------------------X
  HSYNC  (32 px):          ----------------------------------------------------------------------------------------------XXXXXXX------
  HBLANK (28 + 100 px):    XXXXXX-------------------------------------------384-wide-------------------------XXXXXXXXXXXXXXXXXXXXXXXXX  // HBlank ends 28 pixels after 'start of line'
                           ^
                           |
                   'start of line'  (pixel 0)
  Each line: ( 8MHz / 512 pixels per line ) = 15625Hz = 64 usec


  For V-timing PROM, it's tipically a 82S135 (256x8bit) (first 117 entries are not used). Some games (such as Comad
  ones) use a 82S147 (512x8bit) with A5 tied to GND, using only half of the total space.
  The format is:

    Offset  Bits         Description
            76543210
    0       -------x     Sprite DMA trigger (active low)
    1       ------x-     VSYNC (active low)
    2       -----x--     VBLANK (active low)
    3       ----x---     unused
    4       ---x----     IPL0 (active high)
    5       --x-----     IPL1 (active high)
    6       -x------     IPL2 (active high)
    7       x-------     Interrupt Trigger (active low and effective on the very next PROM entry (2 scanlines), the interrupt is triggered on 0 to 1 transition)

  Considering that and looking at the content of the PROM, the vertical timings are below:

                           0.8.13..22.37.....................................................................................262...277
  /SPR-DMA-START (2 lines):--------------------------------------------------------------------------------------------------X--------
  VSYNC  (6 lines):        --XXXX-----------------------------------------------------------------------------------------------------
  VBLANK (38 + 16 lines):  XXXXXXXXXXXXX------------------------------------224-high-----------------------------------------XXXXXXXXX  // VBlank ends 16 lines after 'start of frame'

  (Showing timings for tdragon and other games. Some others has the IRQs a bit shifted or not all of them actually used):
                           0.9...21.....38..53......90..103..................................................218.231.........262...277
  IPL0 (15 + 15 lines):    -------------------------XXXXXXX-----------------128-gap--------------------------XXXXXXX------------------  // some games like bioship and vandyke have this interrupt a bit shifted, will be fixed in the next PR
  IPL1 (16 lines):         -------------XXXXXX----------------------------------------------------------------------------------------  // sometimes not present, i.e. tdragon2, macross2...
  IPL2 (26 lines):         XXX-----------------------------------------------------------------------------------------------XXXXXXXXX
  /FRAME-END (22 lines):   XXXXXXXX---------------------------------------------------------------------------------------------------  // This signal is low on last 22 lines of each frame and goes high on the first line of the next one
                                   ^
                                   |
                           'start of frame'  (line 22)

  VBLANK time: ( 6MHz / 384 pixels per line ) / 54 lines per VBlank = 289.35185185Hz = 3456 usec
  Active video time: ( 6MHz / 384 pixels per line ) / 224 lines per Active = 69.75446428Hz = 14336 usec
  Time between IPL0: ( 6MHz / 384 pixels per line ) / 128 lines between IPL0s = 122.0703125Hz = 8192 usec

  While there are only 3 different H-timing PROMs (one for each screen resolution and ignoring the 4th bit used in
  some games), multiple V-timings PROMs exists on games using this hardware.

  The differences in these V-PROMs exists mainly because each game is configured to have own interrupt timing.
  VSYNC, VBLANK, SPRITE-DMA timings are the same for all variants, though.

  Summary of triggered IRQs on different games:

  - IRQ1:
    - At 68 and 196 scanlines on most games
    - 'tharrier' only at 146 scanline
    - 'ddealer', 'vandyke', 'bioship' and 'blkheart' only at 102 scanline.
      'vandyke' permanently inhibits it by software.
    - 'powerins' only at 16 scanline (looks like it's always inhibited by software)

  - IRQ2:
    - At 16 scanline on most games (VBIN = end of VBLANK = start of active video)
    - 'tdragon', 'macross2', 'tdragon2' and 'raphero' lack it
    - 'tharrier' at 54 scanline
    - 'powerins' at 128 scanline (looks like it's always inhibited by software)

  - IRQ3:
    - Only triggered by 'powerins' at 90 and 166 scanlines (looks like it's always inhibited by software)

  - IRQ4: (VBOUT)
    - At 240 scanline for all games (VBOUT = start of VBLANK = end of active video)

  TODO:
  - Horizontal timing generator emulation?
  - nmk/quizpani.cpp and atlus/patapata.cpp (used larger PROM for support
    higher screen resolution) seems to have similar IRQ generator,
    Can be convert to using this device?

***************************************************************************/

#include "emu.h"
#include "nmk_irq.h"

#define VERBOSE     0
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NMK_IRQ, nmk_irq_device, "nmk_irq", "NMK IRQ Generator")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nmk_irq_device - constructor
//-------------------------------------------------

nmk_irq_device::nmk_irq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NMK_IRQ, tag, owner, clock),
	device_video_interface(mconfig, *this),
	//m_htiming_prom(*this, "htiming"),
	m_vtiming_prom(*this, "vtiming"),
	m_irq_cb(*this),
	m_sprite_dma_cb(*this),
	m_prom_start_offset(0),
	m_prom_frame_offset(0),
	m_vtiming_prom_usage(1),
	m_scanline_timer(nullptr),
	m_vtiming_val(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmk_irq_device::device_start()
{
	// allocate scanline timer and start it
	m_scanline_timer = timer_alloc(FUNC(nmk_irq_device::scanline_callback), this);

	// register for save states
	save_item(NAME(m_vtiming_val));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nmk_irq_device::device_reset()
{
	m_scanline_timer->adjust(screen().time_until_pos(0));

	m_vtiming_val = 0xff;
}

//-------------------------------------------------
//  scanline_callback - called on each scanline
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( nmk_irq_device::scanline_callback )
{
	constexpr int SPRDMA_INDEX = 0;
//  constexpr int VSYNC_INDEX  = 1; // not used in emulation
//  constexpr int VBLANK_INDEX = 2; // not used in emulation
//  constexpr int NOT_USED     = 3; // not used in emulation
	constexpr int IPL0_INDEX   = 4;
	constexpr int IPL1_INDEX   = 5;
	constexpr int IPL2_INDEX   = 6;
	constexpr int TRIGG_INDEX  = 7;

	const size_t len = m_vtiming_prom.length();

	const int y = screen().vpos();

	// every PROM entry is addressed each 2 scanlines, so only even lines are actually addressing it:
	if ((y & 0x1) == 0x0)
	{
		const u32 address = ((((y / 2) + m_prom_start_offset) % (m_vtiming_prom_usage - m_prom_start_offset)) + m_prom_start_offset) % len;

		LOG("scanline_callback: Scanline: %03d - Current PROM entry: %03d\n", y, address);

		const u8 val = m_vtiming_prom[address];

		// Interrupt requests are triggered at rising edge of bit 7:
		if (BIT(val & ~m_vtiming_val, TRIGG_INDEX))
		{
			const u8 int_level = bitswap<3>(val, IPL2_INDEX, IPL1_INDEX, IPL0_INDEX);
			LOG("scanline_callback: Triggered interrupt: IRQ%d at scanline: %03d\n", int_level, y);
			m_irq_cb(int_level);
		}

		// Sprite DMA, as per UPL docs, 256 usec after VBOUT = 4 lines, but index on the PROM says otherwise:
		if (BIT(val & ~m_vtiming_val, SPRDMA_INDEX) && !m_sprite_dma_cb.isunset())
			m_sprite_dma_cb(ASSERT_LINE);

		m_vtiming_val = val;
	}

	// wait for next line
	m_scanline_timer->adjust(screen().time_until_pos(y + 1));
}
