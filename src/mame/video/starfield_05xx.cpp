// license:BSD-3-Clause
// copyright-holders:Robert Hildinger


/***************************************************************************

  Starfield generator documentation
  
     R. Hildinger, based on RE effort Aug. 2019
     
  -----
  
  * These notes are based on what is presumably an original Namco 05xx starfield 
  generator from an original 1981 Namco/Midway Galaga board. This particular 
  chip is marked only with the text "0521" on the chip package. The ROMS on 
  this board are all marked "0508-00803 Galaga (c) Midway 1981"
  
  INTRO:
  ------
  
  The starfields for Galaga and Bosconian are driven by a custom Namco 05XX chip that
  is contains an internal 16-bit Linear Feedback Shift Register (LFSR) and all the
  necessary support logic for generating a 6-bit RGB signal. The chip is fed all the 
  required signals from the video system to allow it to output a colored "star" at
  pseudo-random intervals and to scroll these stars in both horizontal and vertical
  directions.
  
  The chip can generate a total of 256 stars in 4 banks of 64 for each vertical
  frame. Two of these banks of stars will be active at any one time, controlled
  by two starfield selector pins. Some stars will be hidden by the vertical and 
  horizontal blanking, so there will always be less than 128 stars on screen at one
  time. 
  
  The 05xx is a typical seventies-era 5v logic chip in a 24 pin, .6" width package:

  Pin arrangement:


                |--------------------------|
          CLK  <| 1  ++            /-\  24 |>  Gnd
                |    ++            \-/     |
       _HSYNC  <| 2                     23 |>  SF1
                |                          |
     LFSR_RUN  <| 3                0    22 |>  SF0
                |                  5       |
       _VSYNC  <| 4                2    21 |>  LFSR_OUT
                |                  1       |
          _OE  <| 5                     20 |>  OE_CHAIN
                |                          |
      BLUE_HI  <| 6                     19 |>  _STARCLR
                |                          |
      BLUE_LO  <| 7                     18 |>  SCROLL_Y2
                |                          |
     GREEN_HI  <| 8                     17 |>  SCROLL_Y1
                |                          |
     GEEEN_LO  <| 9                     16 |>  SCROLL_Y0
                |                          |
       RED_HI  <| 10                    15 |>  SCROLL_X2
                |                          |
       RED_LO  <| 11                    14 |>  SCROLL_X1
                |                          |
    Vcc (+5v)  <| 12                    13 |>  SCROLL_X0
                |--------------------------|


  NOTE: Pin names are not official - just my best approximation of their function.


  INPUTS:
  -------

  Pin 1 (CLK): This signal drives the internal linear feedback shift register (LFSR), 
               as well as some internal state logic.
   
  Pin 2 (_HSYNC): Video system horizontal sync signal (pulse low). Rising edge marks 
                  the start of each horizontal line in monitor's natural orientation.
           
  Pin 3 (LFSR_RUN): Enable the internal LFSR to update with every CLK cycle. This 
                    signal is gated internally with the SCROLL and _STARCLR signals 
                    to implement horizontal and vertical starfield scrolling, and 
                    to clear the starfield altogether.

  Pin 4 (_VSYNC): Video system vertical sync signal (pulse low). Rising edge marks 
                  the start of each video frame.

  Pin 5 (_OE): Output Enable (Active low): When low, the RGB outputs are active. When 
              high, the RGB outputs are placed in a high impedance state to prevent 
              interference with sprite and tile map generators. This line is used to 
              effectively place stars in the background and prevent them from 
              overwriting sprites and tiles.

  Pins 13-15 (SCROLL_X): These 3 lines control the horizontal speed and direction of 
                         the starfield scrolling. SCROLL_X2, SCROLL_X1, and 
                         SCROLL_X0 map to scroll speed as follows:

	SCROLL_X2	SCROLL_X1	SCROLL_X0	speed and direction
	---------	---------	---------	-------------------
	low		low		low		1 pixels per frame reverse (-X)
	low		low		high		2 pixels per frame reverse (-X)
	low		high		low		3 pixels per frame reverse (-X)
	low		high		high		4 pixels per frame reverse (-X)
		
	high		low		low		3 pixels per frame forward (+X)
	high		low		high		2 pixels per frame forward (+X)
	high		high		low		1 pixels per frame forward (+X)
	high		high		high		0 pixels per frame stationary

  Pins 16-18 (SCROLL_Y): These 3 lines control the vertical speed and direction of the 
                         starfield scrolling. SCROLL_Y2, SCROLL_Y1, and SCROLL_Y0 map to 
                         scroll speed as follows:

	SCROLL_Y2	SCROLL_Y1	SCROLL_Y0	speed
	---------	---------	---------	-----
	low		low		low		0 lines per frame stationary
	low		low		high		1 lines per frame reverse (-Y)
	low		high		low		2 lines per frame reverse (-Y)
	low		high		high		3 lines per frame reverse (-Y)

	high		low		low		4 lines per frame forward (+Y)
	high		low		high		3 lines per frame forward (+Y)
	high		high		low		2 lines per frame forward (+Y)
	high		high		high		1 lines per frame forward (+Y)

  Pin 19 (_STARCLR): Gates the LFSR_RUN signal when active, stopping the LFSR from 
                     running which effectively stops the starfield from being drawn. 
                     It also resets the LFSR seed back to boot value.

  Pins 22-23 (SF): These two lines control which of the 4 star sets are currently being
                   displayed on screen. SF1 and SF0 map to the star sets as follows:

	SF1		SF0		Star sets
	---		---		---------
	low		low		0 and 2
	low		high		1 and 2
	high		high		1 and 3
	High		low		0 and 3



  OUTPUTS:
  --------

  Pins 6-11 (RGB Output): These are the red, green, and blue outputs designed to be 
                          fed in to a digital to analog converter and sent to the picture 
                          tube. These outputs are active every time the LFSR has a "hit"
                          that indicates a star should be generated at this point. 
                          Together they form a 6-bit RGB color value as follows:

	Bit5        Bit4        Bit3        Bit2        Bit1        Bit0
	----        ----        ----        ----        ----        ----
	BLUE_HI     BLUE_LO     GREEN_HI    GREEN_LO    RED_HI      RED_LO
		
  Pin 20 (OE_CHAIN): This signal (chained output enable) is high for 1 CLK cycle every 
                     time the LFSR has a "hit" that indicates a star should be generated
                     at this point. It is active at the same time as the RGB outputs.
 
  Pin 21 (LFSR_OUT): This is the output bit of the internal 16-bit LFSR.


  MACHINE NOTES:
  --------------
  ( G = Galaga, B = Bosconian, GB = both)
  
  GB: CLK: - driven at 1/3 of the Master clock signal ( 18.432 Mhz / 3 = 6.144 MHz ).
  
  GB: _HSYNC: - driven at 1/384 of the CLK signal ( 6.144 Mhz / 384 = 16 KHz )
              - pulse width = 32 CLK cycles low, 352 CLK cycles high (total = 384)
              - High pulse width limits horizontal resolution to maximum of 352 pixels.
              - 256 pulses per vertical frame during _VSYNC high
              
  GB: _VSYNC: - driven at 1/264 of the _HSYNC signal ( 16 KHz / 264 =  60.60606 Hz)
              - pulse width = 8 _HSYNC pulses low, 256 _HSYNC pulses high (total = 264)
              - High pulse width limits vertical resolution to maximum of 256 lines.

  GB: LFSR_RUN: - driven at 1/384 of the CLK signal ( 6.144 Mhz / 384 = 16 KHz )
                - pulse width = 128 CLK cycles low, 256 CLK cycles high (total = 384)
                - 256 pulses per vertical frame during _VSYNC high
                - High pulse position sits inside _HSYNC high between CLK 
                  cycles 56 and 311, which effectively limits starfield width 
                  to 256 horizontal pixels 

  GB: _OE: - Semi-periodic signal driven at 1/384 of the CLK signal ( 16 KHz )
           - Low portion of signal is driven high at any point where a tile or
             sprite is generated
           - Low pulse width = 288 CLK cycles, High pulse width = 96 CLK cycles
           - Low pulse position inside _HSYNC high between CLK cycles 40 and 327
           - Low pulse width further limits horizontal resolution to 288 pixels
           - Signal is only actively generated by video system hardware between _HSYNC 
             pulses 25 and 248 for a total of 224 pulses, which further limits vertical
              esolution to 224 lines.

  G: SCROLL_Y: - All SCROLL_Y signals are tied to ground
  
  GB: RGB OUTPUT: These outputs are sent to the simple resistance ladders that form the 
                  8-bit RGB DAC within the video system:
           
                  BLUE_HI ----> 220 ohm resistor --------> Blue video signal
                  BLUE_LO ----> 470 ohm resistor ----^
           
                  GREEN_HI ---> 220 ohm resistor --------> Green video signal
                  GREEN_LO ---> 470 ohm resistor ----^
               ......--------->  1K ohm resistor ----^
     
                  RED_HI -----> 220 ohm resistor --------> Red video signal
                  RED_LO -----> 470 ohm resistor ----^
               ......--------->  1K ohm resistor ----^

  GB: OE_CHAIN: - This signal is sent to the color LUT PROM that handles the output 
                  from the tile and sprite generators. It forces the PROM outputs to 
                  go high impedance when the signal is high.
                  

  THEORY OF OPERATION:
  --------------------
  
  The operation of the 05xx starfield generator is fairly simple, but subject to a
  fairly complex clocking scheme when scrolling is taken into account. 
  
  The 05xx is designed as a beam-following color generator much like other starfield
  generators in games like Galaxian, etc. It uses an internal LFSR to function 
  a pseudo-random number generator, and it runs one step for every pixel clock except 
  when gated by the LFSR_RUN, SCROLL and _STARCLR input signals. 
  
  The LFSR is a 16-bit fibonacci-style shift register with taps at 16,13, 11, and 6; 
  producing a maximal sequence of 65,535 steps before starting over. It is run 
  over a 256x256 pixel portion of the video frame, and does not reset with each new
  frame. Since the sequence period is 65,535 steps, and the pixel field is 65,536 steps,
  the generated starfield will scroll in the -X direction 1 pixel per frame. To make the
  starfield stationary, the LFSR must skip 1 pixel clock per frame. In fact all 
  scrolling is achieved by either running the LFSR extra steps during the blanking 
  interval, or skipping pixel clocks.
  
  The individual stars are generated by a logic function that looks at the state of the
  LFSR and triggers based on the value of certain bits within the state. When the bits 
  match pre-determined values (a "hit"), a logic function is applied to the remaining 
  bits to generate both a starfield set value and a color value. If the starfield value 
  matches an active set, the color value is applied the RGB outputs for the duration of 
  the pixel clock, and the OE_CHAIN signal is raised to block the output from the sprite 
  and tilemap video sections.
  
  An LFSR hit is detected when the following is true: 
  
    bits 14, 13, 12, and 11 = 1
    bits 15,  9,  4, and  2 = 0

        LFSR state:     Bf Be Bd Bc - Bb Ba B9 B8 - B7 B6 B5 B4 - B3 B2 B1 B0
                        |  |  |  |    |  |  |  |    |  |  |  |    |  |  |  | 
        and'ed with:    1  1  1  1    1  0  1  0    0  0  0  1    0  1  0  0  (FA14)
                        |  |  |  |    |  |  |  |    |  |  |  |    |  |  |  | 
        equals:         0  1  1  1    1  0  0  0    0  0  0  0    0  0  0  0  (7800)

    Therefore an LFSR hit looks like this:
                            
                        0  1  1  1  - 1  Ba 0  B8 - B7 B6 B5 0  - B4 0  B1 B0
       
  By limiting LFSR hits to only those values where 8 of the 16 bits match a specific 
  value, only 256 hits will be detected per LFSR period, and thus 256 possible stars.
  Because the hit detection values contain at least one positive bit, it avoids the
  issue where an LFSR never reaches the all-zeroes state.
  
  Once a hit is detected, set and color values are extracted from the remaining 8 
  bits as follows:
  
       star set value = (Ba B8)b                            [0..3]
       
       color value (BBGGRR) = (!B4 !B1 !B0 !B7 !B6 !B5)b    [0..63]


    NOTE: --------------------------------------------------------------------------

           The Fibonacci LFSR described above was found to produce the right star
           colors and positions with the least complex hit detection and color
           decoding logic. It is not, however, the only way to produce the same
           sequence.

           Wolfgang Scherr over at pin4.at reverse-engineered the 05xx with the
           aim of creating an FPGA replacement for the NAmco original 05xx. He
           produced a VHDL implementation that was later translated into a
           MAME-suitable C++ implementation by Jindřich Makovička.

           Both of these implementations used the Galois form of the Fibonacci
           LFSR described above (and thus the same taps). The Galois form is
           slightly more efficient to implement than the Fibonacci form, and
           produces the same output bit stream. However, the internal state
           sequence of the Galois form is different, and therefore requires
           a different set of hit and decode logic. It was found that the
           logic required for the Galois form was much more complicated than
           the Fibonacci form, which is why the latter is used in this 
           implementation.

           The LFSR used by Wolfgang and Jindřich had taps at 15, 12, 10 and 5.
           With the same seed values, the following holds true:

           Decode_W(lfsr[t-4]) = Decode_J(lfsr[t])

           The two decoding algorithm (Wolfgang, Jindřich) thus delivered the
           same results but with a 4 clock difference.

           Jindřich: Seed Value 0x70cc
           Wolfgang: Seed Value 0xe7bf
          ---------------------------------------------------------------------------


  NUMBER OF STARS ON SCREEN:
  --------------------------
  
  As stated before, during any frame only 2 of the 4 banks of 64 stars will be displayed,
  for a maximum of 128 stars. This maximum is reduced to 126 by the fact that 1 star in
  every bank has the color 0x00, or completely black and therefore invisible on screen.
  Additionally, some stars are generated outside the vertical boundaries of the 
  visible portion of the screen and are also invisible on screen, making the total
  number of stars on screen at any one time less than 126.
  
  The durations of the horizontal and vertical sync pulses for Galaga and Bosconian allow
  for a frame size of 352 x 256. The LFSR_RUN signal pulse sits inside _HSYNC signal pulse
  between pixel clock cycles 56 and 311, thus limiting the size of the frame where stars 
  are generated to 256 x 256:
  
            0         56                                              311    351
          0 +---------+===============================================+------+
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |   Starfield generation area:                  |      |
            |         |   (56,0) -> (311,255)  [256x256]              |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
            |         |                                               |      |
        255 +---------+===============================================+------+
        
  The _OE signal restricts the visible area of the video frame. The _OE signal pulse sits
  between pixel clock cycles 40 and 327, limiting the visible horizontal dimension to
  288 pixels. Also, the _OE signal is only generated between lines 24 and 247, limiting 
  the vertical dimension to 224 lines, hence the final output resolution of 288 x 224:
  
            0         56                                              311    351
          0 +---------+===============================================+------+
            |         |                                               |      |
            |      40 |                                               |  327 |
            |   24 +--+-----------------------------------------------+--+   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |   Starfield visible area:                     |  |   |
            |      |  |   (56,24) -> (311,247)  [256x224]             |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |      |  |                                               |  |   |
            |  247 +--+-----------------------------------------------+--+   |
            |         |                                               |      |
        255 +---------+===============================================+------+

  In this manner, all stars generated in the starfield generation area that fall outside
  the starfield visible area are not shown on screen.


  HORIZONTAL SCROLLING:
  ---------------------
  
  Horizontal and vertical scrolling of the starfield is accomplished by either advancing 
  the LFSR or delaying the advance for some number of pixel clocks per frame
  
  For horizontal scrolling, advancing the LFSR an additional Z number of clocks results 
  in a starfield shift in the negative X direction of Z+1 pixels, while delaying the LFSR
  advance results in shift in the positive X direction of Z-1 pixels. The scrolling
  amount and direction is controlled by the SCROLL_X input signals as follows:
  
    SCROLL_X     LFSR ACTION / FRAME     NUM LFSR ADVs / FRAME   X SHIFT RESULT
    --------------------------------------------------------------------------------
    000          nothing                 256*256 + 0 = 65536     -1 pixels 
    001          Advance 1 extra         256*256 + 1 = 65537     -2 pixels 
    010          Advance 2 extra         256*256 + 2 = 65538     -3 pixels 
    011          Advance 3 extra         256*256 + 3 = 65539     -4 pixels 
    100          Delay 4 clock           256*256 - 4 = 65532     +3 pixels
    101          Delay 3 clocks          256*256 - 3 = 65533     +2 pixels
    110          Delay 2 clocks          256*256 - 2 = 65534     +1 pixels
    111          Delay 1 clocks          256*256 - 1 = 65535     No X Shift
  
      NOTE: The advance or delay of the LFSR occurs at pixel clock 0x500 (1280) after
      the start of each frame. This is during the non-visible portion of starfield 
      generation.


  VERTICAL SCROLLING:
  -------------------
  
  For vertical scrolling, advancing the LFSR an additional (Z * the horizontal frame
  size) number of clocks results in a starfield shift in the negative Y direction of Z+1 
  lines, while delaying the LFSR advance results in shift in the positive Y direction of 
  Z-1 pixels. The scrolling amount and direction is controlled by the SCROLL_Y input 
  signals as follows:
  
    SCROLL_Y     LFSR ACTION / FRAME     NUM LFSR ADVs / FRAME    Y SHIFT RESULT
    --------------------------------------------------------------------------------
    000          nothing                 256*256 + 0 = 65536      No Y Shift
    001          Advance 1*256 extra     256*256 + 256 = 65792    -1 lines 
    010          Advance 2*256 extra     256*256 + 512 = 66048    -2 lines 
    011          Advance 3*256 extra     256*256 + 768 = 66304    -3 lines 
    100          Delay 4*256 clock       256*256 - 1024 = 64512   +4 lines
    101          Delay 3*256 clocks      256*256 - 768 = 64768    +3 lines
    110          Delay 2*256 clocks      256*256 - 512 = 65024    +2 lines
    111          Delay 1*256 clocks      256*256 - 256 = 65280    +1 lines
  
  Vertical scrolling is accomplished in much the same way as horizontal scrolling in that
  the primary difference between the two is that for vertical scrolling we have to 
  advance or delay the LFSR in multiples of the horizontal starfield frame size to 
  effectively  scroll a single vertical line. Also, the points at which these delays 
  or advances occur are not at the same point in every case. 
  
  For example, the default SCROLL_Y value of 000 equates to no Y scrolling. In this case 
  the LFSR is run for 256 pixel clocks per line starting at line 2 within the frame and 
  ending at line 257, for a total of 256*256 = 65,526 pixel clocks. The start and end 
  lines for other SCROLL_Y values are different. The following table lists the lines over
  which the LFSR is run (keeping in mind that running the LFSR over a line means 256 LFSR
  advances):
  
    SCROLL_Y   SKIP   PRE_VIS         VISIBLE            POST_VIS             TOTAL_LINES
    -------------------------------------------------------------------------------------
    000        2      [2..23] (22)    [24..247] (224)    [248..257] (10)      (256)
    001        1      [1..23] (23)    [24..247] (224)    [248..257] (10)      (257)
    010        2      [2..23] (22)    [24..247] (224)    [248..259] (12)      (258)
    011        1      [1..23] (23)    [24..247] (224)    [248..259] (12)      (259)
    100        5      [5..23] (19)    [24..247] (224)    [248..256] (9)       (252)
    101        4      [4..23] (20)    [24..247] (224)    [248..256] (9)       (253)
    110        4      [4..23] (20)    [24..247] (224)    [248..257] (10)      (254)
    111        2      [2..23] (22)    [24..247] (224)    [248..256] (9)       (255)

    NOTE: SKIP = number of lines that are skipped at the start of the frame before
                 running the LFSR
          PRE_VIS, VISIBLE, POST_VIS = line range of LFSR runs during pre-visible,
                 visible, and post-visible potions of frame
          TOTAL_LINES = Total number of lines over which the LFSR is run
                
    NOTE2: Lines are indexed from 0
           Lines > 255 are inside the vertical blanking interval
    


  _STARCLR AND THE LFSR SEED:
  ----------------------------------------
  
  The _STARCLR input signal is used to stop the LFSR from running (and therefore 
  generating any stars), and to reset its internal state. At no other time is the 
  internal state of the LFSR reset. When the _STARCLR signal is held low, the 
  LFSR_RUN signal is blocked, and the internal state of the LFSR is loaded with 
  it's seed value, which is 0x7FFF. 

  NOTE: Testing with a Galaga machine reveals that during normal operation, the
        _STARCLR signal is held low at power on and then raised when the initial
        attract screen displays. The low-to-high transition arrives approximately
        14,191 starfield pixel clocks in to the first frame of attract screen animation, 
        resulting in a starfield that is half-filled. There is no real way to simulate
        this in the MAME galaga video driver as it fills the starfield between frames
        rather than mid-frame.
  
***************************************************************************/





#include "emu.h"
#include "starfield_05xx.h"


DEFINE_DEVICE_TYPE(STARFIELD_05XX, starfield_05xx_device, "starfield_05xx_stars", "Galaga/Bosconian starfield")




#define STARS_COLOR_BASE (64*4+64*4)

#define VISIBLE_LINES                   224
#define STARFIELD_PIXEL_WIDTH           256

#define LFSR_CYCLES_PER_LINE            256
#define LFSR_HIT_MASK           0xFA14
#define LFSR_HIT_VALUE          0x7800
#define LFSR_SEED               0x7FFF


static const int speed_X_cycle_count_offset[] =
{
	0,  1,  2,  3, -4, -3, -2, -1
};

static const int pre_vis_cycle_count_values[] =
{
	22 * LFSR_CYCLES_PER_LINE,
	23 * LFSR_CYCLES_PER_LINE,
	22 * LFSR_CYCLES_PER_LINE,
	23 * LFSR_CYCLES_PER_LINE,
	19 * LFSR_CYCLES_PER_LINE,
	20 * LFSR_CYCLES_PER_LINE,
	20 * LFSR_CYCLES_PER_LINE,
	22 * LFSR_CYCLES_PER_LINE
};

static const int post_vis_cycle_count_values[] =
{
	10 * LFSR_CYCLES_PER_LINE,
	10 * LFSR_CYCLES_PER_LINE,
	12 * LFSR_CYCLES_PER_LINE,
	12 * LFSR_CYCLES_PER_LINE,
	9  * LFSR_CYCLES_PER_LINE,
	9  * LFSR_CYCLES_PER_LINE,
	10 * LFSR_CYCLES_PER_LINE,
	9  * LFSR_CYCLES_PER_LINE
};




starfield_05xx_device::starfield_05xx_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, STARFIELD_05XX, tag, owner, clock)
	, m_enable(0)
	, m_lfsr(LFSR_SEED)
	, m_pre_vis_cycle_count(0)
	, m_post_vis_cycle_count(0)
	, m_set_a(0)
	, m_set_b(0)
	, m_offset_x(0)
	, m_offset_y(0)
	, m_limit_x(0)

{
}



void starfield_05xx_device::enable_starfield(uint8_t on)
{
	if (!on) m_lfsr = LFSR_SEED;

	m_enable = on ? 1 : 0;
}



void starfield_05xx_device::set_scroll_speed(uint8_t index_x, uint8_t index_y)
{
	// Set initial pre- and post- visible cycle counts based on vertical 
	// scroll registers
	m_pre_vis_cycle_count = pre_vis_cycle_count_values[index_y];
	m_post_vis_cycle_count = post_vis_cycle_count_values[index_y];
	
	// X scrolling occurs during pre-visible portion, so adjust 
	// pre-visible cycle count to based on horizontal scroll registers
	m_pre_vis_cycle_count += speed_X_cycle_count_offset[index_x];
}


void starfield_05xx_device::set_active_starfield_sets(uint8_t set_a, uint8_t set_b)
{
	// Set active starfield sets based on starfield select registers
	m_set_a = set_a;
	m_set_b = set_b;
}


void starfield_05xx_device::set_starfield_config(uint16_t off_x, uint16_t off_y, uint16_t lim_x)
{
	// Set X and Y starfield position offsets
	m_offset_x = off_x;
	m_offset_y = off_y;
	
	// Set X range limit
	m_limit_x = lim_x;
}


uint16_t starfield_05xx_device::get_next_lfsr_state(uint16_t lfsr)
{
    uint16_t bit;

    // 16-bit FIBONACCI-style LFSR with taps at 16,13,11, and 6
    // These taps produce a maximal sequence of 65,535 steps.

    bit = ((lfsr >> 0) ^ (lfsr >> 3) ^ (lfsr >> 5) ^ (lfsr >> 10));
    lfsr = (lfsr >> 1) | (bit << 15);

    return lfsr;
}


void starfield_05xx_device::draw_starfield(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	uint16_t pre_vis_cycle_count = m_pre_vis_cycle_count;
	uint16_t post_vis_cycle_count = m_post_vis_cycle_count;

	if (m_enable)
	{
		int x,y;

		// Advance the LFSR during the pre-visible portion of the frame
		do { m_lfsr = get_next_lfsr_state(m_lfsr); } while (--pre_vis_cycle_count);

		// Now we are in visible portion of the frame - Output all LFSR hits here
		for (y = m_offset_y; y < VISIBLE_LINES + m_offset_y; y++)
		{
			for (x = m_offset_x; x < STARFIELD_PIXEL_WIDTH + m_offset_x; x++)
			{
				// Check lfsr for hit
				if ((m_lfsr&LFSR_HIT_MASK) == LFSR_HIT_VALUE)
				{
					uint8_t star_set = bitswap<2>(m_lfsr, 10, 8);

					if ((m_set_a == star_set) || (m_set_b == star_set))
					{
						// don't draw the stars that are beyond the X limit
						if (x < m_limit_x)
						{
							int dx = x;

							if (flip) dx += 64;

							if (cliprect.contains(dx, y))
							{
								uint8_t color;

								color  = (m_lfsr>>5)&0x7;
								color |= (m_lfsr<<3)&0x18;
								color |= (m_lfsr<<2)&0x20;
								color = (~color)&0x3F;

								bitmap.pix16(y, dx) = STARS_COLOR_BASE + color;
							}
						}
					}
				}

				// Advance LFSR
				m_lfsr = get_next_lfsr_state(m_lfsr);
			}
		}

		// Advance the LFSR during the post-visible portion of the frame
		do { m_lfsr = get_next_lfsr_state(m_lfsr); } while (--post_vis_cycle_count);
	}
}



void starfield_05xx_device::device_start()
{
	save_item(NAME(m_enable));
	save_item(NAME(m_lfsr));
	save_item(NAME(m_pre_vis_cycle_count));
	save_item(NAME(m_post_vis_cycle_count));
	save_item(NAME(m_set_a));
	save_item(NAME(m_set_b));

	save_item(NAME(m_offset_x));
	save_item(NAME(m_offset_y));
	save_item(NAME(m_limit_x));
}



void starfield_05xx_device::device_reset()
{
	m_enable = 0;
	m_lfsr = LFSR_SEED;
	m_pre_vis_cycle_count = 0;
	m_post_vis_cycle_count = 0;
	m_set_a = 0;
	m_set_b = 0;
}
