// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert

/***************************************************************************/
/*                                                                         */
/*    054156 with either 054157 or 056832                                  */
/*    058143 with 056832                                                   */
/*                                                                         */
/*    Konami Tilemap Chips                                                 */
/*                                                                         */
/***************************************************************************/

/*

054156/054157
054156/056832
058143/056832
-------------

054156 = AVAC
058143 = AVAC2?
054157 = AVSC?
054832 = AVSC2



This chip combination generates tilemap layers in konami systems.  Up
to four independant, 10-bit layers are generated to be connected to
inputs of the mixer.  There is no concept of transparency, shadow, etc
at that point, that's entirely up to the mixer.

The 054156 manages the tilemap (vram) itself and from it generates
character rom addresses.  The 054156/053832 gets the result of reading
the character roms and buffers/shifts things around to generate the
pixels.



The 054156 is in charge of the tilemap vram.  It is connected to 2 or
3 8-bit rams, giving 16 to 24 bits per tile.  The ram is organized in
pages of 0x800 tiles, each page corresponding to a 512x256 pixel
surface, either as a 64x32 grid of 8x8 chars, or as a 1x256 grid of
512x1 chars.  The pages are setup as a 8x8 grid, but unconnected
address lines allow to reduce the size of the grid (and the amount of
ram required).  Actual seen sizes are 2x2, 2x4 and 4x4.  Each layer is
of course scrollable independently, with possibilities of line and
8-line block scrolling using data from a user-selected page. The
number of actually generated layers can be chosen as either 2 or 4.

The character roms are setup so that one address, ignoring the low
bit, corresponds to a 8x1 pixel span whatever the bpp.  So the first
chip function is to output one address per layer (e.g. 2 or 4) every 8
pixel clocks.  The addresses are up to 21 bits wide (to which bit 0
must be added, for a total 22-bits address space), with the four top
bits banked from the top two bits of the tile number.


The 054157 sports a 16-bit data bus from the character roms.  Two
accesses are required for each 8x1 pixel span, the first giving
bitplanes 0 and 2, the second bitplanes 1 and 3.  For that reason it
is the one providing the low bit address.  The '156 somehow provides
the global line shift and horizontal flip status (there are 4 lines
called Z1H, Z2H, Z4H and NRE linking the two chips that probably
provide the information somehow) and the palette information (8 color
lines, flip may be in there since these are two too many, and the flip
info is colocated with the palette information in the vram).  The chip
then can push out the colors for each layer with the dotclock.

It's interesting to note that the character roms are read 8 times
(four layers * two accesses) every 8 pixels, which makes one access
per dotclock.  Nice design.  If the rom is too slow for the dotclock,
switching to two layers reduces the speed to one access every two
dotclocks.

Another interesting point is that the design is such that with the
exception of the low address line and the layers themselves all the
pins on the 054157 are inputs.  So it's possible to connect two of
them to get a 32-bits crom data path and get 8bpp tiles.

The 056832 is the grown-up version of the pair-of-157s.  It sports a
32-bits data path, and can be programmed for any bpp between 4 and 8.
The two-cycles method is identically used, the first cycle providing
bitplanes 0,2,4 and 6, and the second bitplanes 1,3,5 and 7 as needed
by the configuration.  The roms must be correctly wired up to ensure
the one even/odd address = a full 8x1 pixel span though.  One result
is that on odd bpps one byte rom has to be connected ignoring the low
address line, making its contents appear duplicated when reading the
rom through the chip.  A second capability of the 056832 is to provide
a "fast dotclock" mode where, instead of dropping two layers to reduce
the number of accesses, the bpps are reduced to 4 while the whole
32-bits are used.  The whole 8x1x4bpp data is read in one cycle,
providing bitplanes 0,1,2 and 3, with the roms are all connected
ignoring the low address bit.  This reduces the access rate to one
every two clocks.  The final capability of the 056832 is a 3-to-6 bits
tile address expansion through eight banking registers.
Interestingly, the two top bits of the address given by the 054156,
which come from its banking tend to be unconnected when paired with
the 056832.  As a result only one address bit is added instead of
three compared to with the 054157, but the game gets four more banks
to control.


The 054156 manages access to the vram from the cpu.  One banked page
is visible in the address space, and can be setup to be accessed
efficiently as bytes, words or dwords.  Efficient in that case means
that each consecutive address accesses consecutive tiles.  E.g. in
byte mode adr and adr+1 refers to two consecutive tiles, in word mode
adr and adr+2 do, and in dword mode adr and adr+4 do.  When one access
is not enough for all the tile bits, the rest of the data is at a
fixed offset (0x800 or 0x1000).


The 058143 adds enough address lines to allow for linear access to the
whole vram.  It is probably 24-bits vram, dword access only.


VRAM layout:

          1 1 1 1 1 1 1 1
          7 6 5 4 3 2 1 0 f e d c b a 9 8 7 6 5 4 3 2 1 0
16-bits:                  X X X X c c c c c c c c c c c c
24-bits:  X X X X X X X X c c c c c c c c c c c c c c c c

c = character code (e.g. rom address, once shifted as needed)
X = palette/flip bits.  Which bits are what depends on other registers


Registers 054156/058143:

           f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
+00  reg1  .   .   . pba mod   ---cm---- vrd  8m vfl hfl  ov  ex  ez dms
+02  reg2  .   .   .   .   .   .   .   . dfv dfh cfv cfh bfv bfh afv afh
+04  reg3  -pd--   -pc--   -pb--   -pa--   .   -cr--   .   .   ---sb----
+06  reg4  .   .   .   .   .   .   .   .   -fb--  8b opt zrm   .   .   .
+08  reg5  .   .   .   .   .   .   .   . hds hcs hbs has mdd mdc mdb mda
+0a  rzs   .   .   .   .   .   .   .   .   -scd-   -scc-   -scb-   -sca-
+0c  ars   .   .   .   .   .   .   .   .   .   . drm crm brm arm   abit-
+0a  abv   .   .   .   .   .   .   .   .   .   .   ---avs---   ---avb---
+0c  bbv   .   .   .   .   .   .   .   .   .   .   ---bvs---   ---bvb---
+0e  cbv   .   .   .   .   .   .   .   .   .   .   ---cvs---   ---cvb---
+10  dbv   .   .   .   .   .   .   .   .   .   .   ---dvs---   ---dvb---
+12  abh   .   .   .   .   .   .   .   .   .   .   ---ahs---   ---ahb---
+14  bbh   .   .   .   .   .   .   .   .   .   .   ---bhs---   ---bhb---
+16  cbh   .   .   .   .   .   .   .   .   .   .   ---chs---   ---chb---
+18  dbh   .   .   .   .   .   .   .   .   .   .   ---dhs---   ---dhb---
+20  mav   .   .   .   .   .   ----------------vscroll a----------------
+22  mbv   .   .   .   .   .   ----------------vscroll b----------------
+24  mcv   .   .   .   .   .   ----------------vscroll c----------------
+26  mdv   .   .   .   .   .   ----------------vscroll d----------------
+28  mah   .   .   .   .   --------------------hscroll a----------------
+2a  mbh   .   .   .   .   --------------------hscroll b----------------
+2c  mch   .   .   .   .   --------------------hscroll c----------------
+2e  mdh   .   .   .   .   --------------------hscroll d----------------
+30  mpz   .   .   .   .   .   .   .   .   .   .   ---mzv---   ---mzh---
+32  mpa   .   .   .   .   .   .   .   .   .   .   ---mav---   ---mah---
+34  cadlm -------------cadm------------   -------------cadl------------
+36  cadh  .   .   .   .   .   .   .   .   -------------cadh------------
+38  vrc   -----vrc3----   -----vrc2----   -----vrc1----   -----vrc0----
+3a  offh  .   .   .   .   --------------------hflip offset-------------
+3c  offv  .   .   .   .   .   ----------------vflip offset-------------

Note: registers reg1 and reg3 high bytes only exist when a 056832 is
connected.  Consider them all-0 otherwise.

pba: palette banking, if one use the 2 bits p[a-d] as two top palette
     bits for the associated layer
mod: one activates the fast-dotclock 4bpp mode, overrides cm
cm:  0=4bpp, 1=5bpp, 2=6bpp, 3=7bpp, 4+=8bpp
vrd: 1=vrom system?  Never encountered
8m:  input dotclock frequency, 0=8Mhz, 1=6Mhz
vfl/hfl: global flip bits
ov:  ?
ex:  0=the 054156 generates the syncs, 1=it's externally done (by a CCU for instance)
ez:  external ram present for linescroll?
dms: 0=four layers, 1=two layers
[a-d]f[hv]: 1=bit for horizontal/vertical character flipping present in layer a-d
cr: 00=normal mode, 11=read character rom mode, other unknown (used for character ram access)
sb: 000 = banking disabled
    001 = banking keyed on bits b,a of the character code
    010 = banking keyed on bits 9,8 of the character code
    100 = banking keyed on bits f,e of the character code
fb: indicates which bits are used for character flipping in vram
    offset = 8 for 16-bits vram, 16 for 24-bits vram
    bit (3-val)*2+1+offset for vflip
    bit (3-val)*2+  offset for hflip
    bit is active if enabled in reg2
    active bits are removed from the palette value
8b:  1 = ignore bits 8-15 of the data bus, the cpu is 8-bits.
opt: 1 = vram is 24bits wide, 0 = vram is 16bits wide
zrm: 0 = external ram for raster scroll present? Use and/or interaction with ez unknown
h[a-d]s: scroll mode setting, 0=asynchronous, 1=syncronous, impact unknown
md[a-d]: 0 = characters for the layer are 512x1, 1 = characters are 8x8
sc[a-d]: scroll mode, 0=linescroll, 2=8-line block linescroll, 1/3=normal scroll
[a-d]rm: 0=layer is in vram, 1=layer is in vrom.  Never used afaict
abit: vram layout, 0 = dword, 1=word, 2=byte
[a-d][vh]s: vertical/horizontal size of the layer in pages, minus 1.  Non-power of two is probably unusable.
[a-d][vh]b: vertical/horizontal position of the layer in the page grid
mz[vh]: vertical/horizontal position of the page with the linescroll data
ma[vh]: vertical/horizontal position of the cpu-visible page
m[a-d][vh]: vertical/horizontal scroll position for the layer
cad[hml]: address for rom readback (only cadl is actually used)
vrc: 4-bits bank values, indexed on the bits chosen with sb
off[vh]: offset added to the layer scrolls when flipped


Scrolling is done independently per-layer.  Vertical scrolling is
controlled by the m?v registers.  Horizontal scolling uses one of
three modes: standard full-layer scrolling, linescroll and scroll by
8-line blocks according to the sc? register.  Full-layer scrolling
position is controller by m?h.  Linescroll and 8-line block scroll
uses either a page or external dedicated ram to store the positions.
There are 4x512 (0x800) positions, 512 for each layer, using exactly
one page.  The positions are indexed by the tilemap line numbers and
the layer number, the vertical scroll position has no influence.  Or not.
In 8-line mode, holes are left between each position (e.g. the IC zeroes
the three bottom address bits when reading the offset).


The 054156 can generate the sync signals for fixed video configurations,
and the associated interrupts.  It just has to be provided with a 6MHz
dotclock for 288x224 or a 8Mhz one for 384x224 (or maybe 384x256, not
sure).


Global layer positioning on the screen is a complex but critical
issue.  When connected to an external sync generator, like the 053252,
the 054156 only gets the horizontal and vertical sync signals.  For
each screen line the 054156 must decide which tilemap line to display,
then within that line which tiles to get, in which order (increasing
or decreasing memory order), and when to start (for pixel-level
horizontal scrolling).  Clipping is done aggressively by the external
video blanking circuit, which is controlled by the sync generator,
internal or external.  So positioning has to be precise, otherwise the
screen will be cut.  There's no overscan to be had.

But, when external, the 054156 has noaccess to the blanking signal, so
all positioning is relative to sync.  In addition the global flipping
bits have to be taken into account.  They invert the screen, but how
is not defined.  Offsets in registers offv/offh are automatically
included in the current scrolling position when the associated global
flip is active, but how is not documented, only some specific values
are given.  It is expected that every game sets them so that the
actual viewport doesn't change when flipping.

Vertical position model: we need to determine how the circuit goes
from a screen line number to a tilemap line number.  We set screen
line number 0 at the start of the back porch (end of vsync).

From a panel of games, we note whether sync is internal or external,
the visible screen height, visible screen start position, vsync line
count (just in case), vertical offset value, vertical scroll value,
tilemap line of the first visible pixel and height of the tilemap
layer.  All values in hex.

Orig | Height | Visible | Vsync | Voff | Scroll | TilePos | THeight | Game
ext  |     e0 |       f |     8 |  700 |   fff0 |       0 | 100/200 | gokuparo
ext  |    100 |       f |     6 |  720 |   7ff0 |       0 | 100/200 | xexex
int  |     e0?|       f?|     8?|  700 |      0 |      10 |     100 | gijoe


Observations:
  - Vsync duration doesn't seem to have any impact.

  - An increasing scroll value scrolls the screen up, which means
    increasing the tilemap line number, whatever the flip value.

  - Voff is there to ensure that the top visible screen line hits the
    bottom line of the visible zone of the tilemap when flipped.  So
    everything else being equal, an increase of the height requires a
    biggest increment through Voff.  We see that Voff goes from 700 to
    720 when the height goes from e0 to 100, so Voff is added.

  - When non-flipped, the tilemap line must increase with the screen
    line number.  When flipped, the tilemap line must decrease with
    the screen line number.

  - Tilemap line number wraps with the tilemap height

So the screen -> tile line formula takes the form:
   ty = ( sy + scroll +        offv1) & (th-1) (non-flipped)
      = (-sy + scroll + Voff + offv2) & (th-1) (flipped)

Now, in gokuparo's case, the first visible line (sy=f) must hit the
first tilemap line (ty=0) when non-flipped, and the last (ty=df) when
flipped.

  (  f + fff0 + offv1) & ff == 0
  ( -f + fff0 + 700 + offv2) & ff == df

  -> offv1 = 1, offv2 = -2

But we can do a little better.  For the flipped case, we get:
  ty = (-sy - 2 + scroll + Voff) & (th-1)
     = (~sy - 1 + scroll + Voff) & (th-1)
     = (~(sy + 1) + scroll + Voff) & (th-1)

which allows to factor in the same offset than for the non-flipped
case.  So we get for the final computation:

- Take the screen line number, add 1 (e.g. it's a 1-based count, not a
  0-base one)
- Invert the value if flipped
- Add the scroll register value
- If flipped, add Voff
- Mask with the tilemap size

That process has the advantage of requiring only an incrementer, an
inverter and two adders, all easy in hardware.

Verification:
   gokuparo (visible range f..ee, tilemap range 0..df):
     non-flipped, at visible:  ((f+1) + fff0) & (ff/1ff)         = 0
                  at bottom:   ((ee+1) + fff0) & (ff/1ff)        = df
     flipped,     at visible:  (~(f+1) + fff0 + 700) & (ff/1ff)  = df
                  at bottom:   (~(ee+1) + fff0 + 700) & (ff/1ff) = 0

   xexex (visible range f..10e, tilemap range 0..ff):
     non-flipped, at visible:  ((f+1) + 7ff0) & (ff/1ff)          = 0
                  at bottom:   ((10e+1) + 7ff0) & (ff/1ff)        = ff
     flipped,     at visible:  (~(f+1) + 7ff0 + 720) & (ff/1ff)   = ff
                  at bottom:   (~(10e+1) + 7ff0 + 720) & (ff/1ff) = 0

   gijoe (visible range f..ee, tilemap range 10..ef):
     non-flipped, at visible:  ((f+1) + 0) & (ff/1ff)         = 10
                  at bottom:   ((ee+1) + 0) & (ff/1ff)        = ef
     flipped,     at visible:  (~(f+1) + 0 + 700) & (ff/1ff)  = ef
                  at bottom:   (~(ee+1) + 0 + 700) & (ff/1ff) = 10

So, that works for the vertical position.
'
*/

/*
Orig | Width | Visible | Hsync | Hoff | Scroll | TilePos | TWidth  | Game
ext  |   120 |      30 |    20 |  d55 |   ffe6 |       0 |    200  | gokuparo
ext  |       |         |       |      |        |         |         | xexex
int  |       |         |       |      |        |         |         | gijoe



*/
#include "k054156_k054157_k056832.h"
#include "screen.h"

const device_type K054156_054157 = device_creator<k054156_054157_device>;
const device_type K054156_056832 = device_creator<k054156_056832_device>;
const device_type K058143_056832 = device_creator<k058143_056832_device>;


void k054156_056832_device::set_info(int _sizex, int _sizey, int _vramwidth, const char *_palette)
{
	m_sizex = _sizex;
	m_sizey = _sizey;
	m_vramwidth = _vramwidth;
	static_set_palette(*this, _palette);
	if(m_sizex != 1 && m_sizex != 2 && m_sizex != 4 && m_sizex != 8)
		throw emu_fatalerror("%s: requested width (%d) must be 1, 2, 4 or 8", tag(), m_sizex);
	if(m_sizey != 1 && m_sizey != 2 && m_sizey != 4 && m_sizey != 8)
		throw emu_fatalerror("%s: requested height (%d) must be 1, 2, 4 or 8", tag(), m_sizey);
	if(m_vramwidth != 16 && m_vramwidth != 24)
		throw emu_fatalerror("%s: requested vram width (%d) must be 16 or 24", tag(), m_vramwidth);
}


DEVICE_ADDRESS_MAP_START(vacset, 16, k054156_056832_device)
	AM_RANGE(0x00, 0x01) AM_WRITE(reg1_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(reg2_w)
	AM_RANGE(0x04, 0x05) AM_WRITE(reg3_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(reg4_w)
	AM_RANGE(0x08, 0x09) AM_WRITE(reg5_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(rzs_w)
	AM_RANGE(0x0c, 0x0d) AM_WRITE(ars_w)
	AM_RANGE(0x10, 0x17) AM_WRITE(bv_w)
	AM_RANGE(0x18, 0x1f) AM_WRITE(bh_w)
	AM_RANGE(0x20, 0x27) AM_WRITE(mv_w)
	AM_RANGE(0x28, 0x2f) AM_WRITE(mh_w)
	AM_RANGE(0x30, 0x31) AM_WRITE(mpz_w)
	AM_RANGE(0x32, 0x33) AM_WRITE(mpa_w)
	AM_RANGE(0x34, 0x35) AM_WRITE(cadlm_w)
	AM_RANGE(0x36, 0x37) AM_WRITE(cadh_w)
	AM_RANGE(0x38, 0x39) AM_WRITE(vrc_w)
	AM_RANGE(0x3a, 0x3b) AM_WRITE(offh_w)
	AM_RANGE(0x3c, 0x3d) AM_WRITE(offv_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(vacset8, 8, k054156_056832_device)
	AM_RANGE(0x01, 0x01) AM_WRITE(reg1_8w)
	AM_RANGE(0x03, 0x03) AM_WRITE(reg2_8w)
	AM_RANGE(0x05, 0x05) AM_WRITE(reg3_8w)
	AM_RANGE(0x07, 0x07) AM_WRITE(reg4_8w)
	AM_RANGE(0x09, 0x09) AM_WRITE(reg5_8w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(rzs_8w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(ars_8w)
	AM_RANGE(0x10, 0x17) AM_WRITE(bv_8w)
	AM_RANGE(0x18, 0x1f) AM_WRITE(bh_8w)
	AM_RANGE(0x20, 0x27) AM_WRITE(mv_8w)
	AM_RANGE(0x28, 0x2f) AM_WRITE(mh_8w)
	AM_RANGE(0x30, 0x31) AM_WRITE(mpz_8w)
	AM_RANGE(0x32, 0x33) AM_WRITE(mpa_8w)
	AM_RANGE(0x34, 0x35) AM_WRITE(cadlm_8w)
	AM_RANGE(0x37, 0x37) AM_WRITE(cadh_8w)
	AM_RANGE(0x38, 0x39) AM_WRITE(vrc_8w)
	AM_RANGE(0x3a, 0x3b) AM_WRITE(offh_8w)
	AM_RANGE(0x3c, 0x3d) AM_WRITE(offv_8w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(vsccs, 16, k054156_056832_device)
	AM_RANGE(0x00, 0x07) AM_WRITE(vrc2_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(vsccs8, 8, k054156_056832_device)
	AM_RANGE(0x00, 0x07) AM_WRITE(vrc2_8w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(vsccs, 16, k054156_054157_device)
	AM_RANGE(0x00, 0x01) AM_WRITE8(reg1b_w, 0x00ff)
	AM_RANGE(0x02, 0x03) AM_WRITE8(reg2b_w, 0x00ff)
	AM_RANGE(0x04, 0x05) AM_WRITE8(reg3b_w, 0x00ff)
	AM_RANGE(0x06, 0x07) AM_WRITE8(reg4b_w, 0x00ff)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(vsccs8, 8, k054156_054157_device)
	AM_RANGE(0x01, 0x01) AM_WRITE(reg1b_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(reg2b_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(reg3b_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(reg4b_w)
ADDRESS_MAP_END

WRITE_LINE_MEMBER(k054156_056832_device::vsync_w)
{
}

WRITE16_MEMBER(k054156_056832_device::reg1_w)
{
	static const char *depths[8] = { "4bpp", "5bpp", "6bpp", "7bpp", "8bpp", "8bpp", "8bpp", "8bpp" };
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;

	if(ACCESSING_BITS_8_15 && m_reg1h != data >> 8) {
		m_reg1h = data >> 8;
		decode_character_roms();
		logerror("reg1_w %02x pb=%s fast=%s depth=%s\n", m_reg1h,
				 data & 0x1000 ? "on" : "off",
				 data & 0x0800 ? "on" : "off",
				 depths[(data >> 8) & 7]);
	}

	if(ACCESSING_BITS_0_7 && m_reg1l != (0x100|(data & 0xff))) {
		m_reg1l = data;
		logerror("reg1_w type=%s dot=%s fli=%c%c ov=%s ex=%s ext_z=%s layers=%c\n",
				 data & 0x80 ? "vrom" : "vram",
				 data & 0x40 ? "8MHz" : "6MHz",
				 data & 0x20 ? 'y' : '-',
				 data & 0x10 ? 'x' : '-',
				 data & 0x08 ? "on" : "off",
				 data & 0x04 ? "ccu" : "internal",
				 data & 0x02 ? "present" : "none",
				 data & 0x01 ? '2' : '4');
	}
}

WRITE16_MEMBER(k054156_056832_device::reg2_w)
{
	if(ACCESSING_BITS_0_7 && m_reg2 != data) {
		m_reg2 = data;
		logerror("reg2_w flips enable a=%c%c b=%c%c c=%c%c d=%c%c\n",
				 data & 0x02 ? 'y' : '-',
				 data & 0x01 ? 'x' : '-',
				 data & 0x08 ? 'y' : '-',
				 data & 0x04 ? 'x' : '-',
				 data & 0x20 ? 'y' : '-',
				 data & 0x10 ? 'x' : '-',
				 data & 0x80 ? 'y' : '-',
				 data & 0x40 ? 'x' : '-');
	}
}

WRITE16_MEMBER(k054156_056832_device::reg3_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;

	if(ACCESSING_BITS_8_15 && m_reg3h != data >> 8) {
		m_reg3h = data >> 8;
		logerror("reg3_w palettes a=%d b=%d c=%d d=%d\n",
				 (data >>  8) & 3,
				 (data >> 10) & 3,
				 (data >> 12) & 3,
				 (data >> 14) & 3);
	}

	if(ACCESSING_BITS_0_7 && m_reg3l != (data & 0xff)) {
		m_reg3l = data;
		logerror("reg3_w v=%02x cr=%c%c vrc=%s\n", m_reg3l,
				 data & 0x40 ? '1' : '0',
				 data & 0x20 ? '1' : '0',
				 data & 1 ? "3-2" : data & 2 ? "1-0" : data & 4 ? "7-6" : "off");
	}
}

WRITE16_MEMBER(k054156_056832_device::reg4_w)
{
	if(ACCESSING_BITS_0_7 && m_reg4 != (data & 0xff)) {
		if((m_reg4 & 0xf8) != (data & 0xf8))
			logerror("reg4_w flipbits=%d-%d mode=%d vram=%d ext=%s int=%x\n", 7^((data & 0xc0)>>5), 6^((data & 0xc0)>>5), data & 0x20 ? 8 : 16, data & 0x10 ? 24 : 16, data & 0x10 ? "off" : "on", data & 7);
		m_reg4 = data;
		if(m_screen && !(data & 7))
			m_int1_cb(CLEAR_LINE);
	}
}

WRITE16_MEMBER(k054156_056832_device::reg5_w)
{
	if(ACCESSING_BITS_0_7 && m_reg5 != (data & 0xff)) {
		m_reg5 = data;
		logerror("reg5_w a=%s%c b=%s%c c=%s%c d=%s%c\n",
				 data & 0x01 ? "8x8" : "512x1", data & 0x10 ? 's' : 'a',
				 data & 0x02 ? "8x8" : "512x1", data & 0x20 ? 's' : 'a',
				 data & 0x04 ? "8x8" : "512x1", data & 0x40 ? 's' : 'a',
				 data & 0x08 ? "8x8" : "512x1", data & 0x80 ? 's' : 'a');
	}
}

WRITE16_MEMBER(k054156_056832_device::rzs_w)
{
	static const char *const scroll_type[4] = { "line", "normal", "block", "normal" };

	if(ACCESSING_BITS_0_7 && m_rzs != (data & 0xff)) {
		m_rzs = data;
		logerror("rzs_w scroll type a=%s b=%s c=%s d=%s\n",
				 scroll_type[(data >> 0) & 3],
				 scroll_type[(data >> 2) & 3],
				 scroll_type[(data >> 4) & 3],
				 scroll_type[(data >> 6) & 3]);
	}
}

WRITE16_MEMBER(k054156_056832_device::ars_w)
{
	static const int access_width[4] = { 32, 16, 8, 0 };
	if(ACCESSING_BITS_0_7 && m_ars != (data & 0xff)) {
		m_ars = data;
		select_vram_access();
		logerror("ars_w layers a=%s b=%s c=%s d=%s access=%d\n",
				 data & 0x04 ? "rom" : "ram",
				 data & 0x08 ? "rom" : "ram",
				 data & 0x10 ? "rom" : "ram",
				 data & 0x20 ? "rom" : "ram",
				 access_width[data & 3]);
	}
}

WRITE16_MEMBER(k054156_056832_device::bv_w)
{
	if(ACCESSING_BITS_0_7 && m_bv[offset] != (data & 0xff)) {
		m_bv[offset] = data;
		setup_tilemap(offset);
		logerror("bv_w layer %c y=%d h=%d\n",
				 'a' + offset,
				 (data >> 3) & 7,
				 1+(data & 7));
	}
}

WRITE16_MEMBER(k054156_056832_device::bh_w)
{
	if(ACCESSING_BITS_0_7 && m_bh[offset] != (data & 0xff)) {
		m_bh[offset] = data;
		setup_tilemap(offset);
		logerror("bh_w layer %c x=%d w=%d\n",
				 'a' + offset,
				 (data >> 3) & 7,
				 1+(data & 7));
	}
}

WRITE16_MEMBER(k054156_056832_device::mv_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t omv = m_mv[offset];
	COMBINE_DATA(m_mv + offset);

	if(0)	if(space.device().safe_pc() == 0xc08476 || space.device().safe_pc() == 0xc08442) {
		uint32_t delta = (omv^m_mv[offset]) & 0x1ff;
		if((delta & 0x180) && (delta & 0x7f) < 0x10)
			m_mv[offset] = omv ^ (delta & 0x7f);
	}
	if(0)
	if((omv & 0x1ff) != (m_mv[offset] & 0x1ff))
		logerror("layer %d delta %03x pc %06x\n", offset, (omv^m_mv[offset]) & 0x1ff, space.device().safe_pc());
	if(false && omv != m_mv[offset]) {
		logerror("XTX scroll %d %4x\n", offset, m_mv[offset]);
		//		uint16_t vv = space.read_word(0xc02878);
		logerror("mv_w %d, %04x @ %04x\n", offset, data, mem_mask);
	}
	
}

WRITE16_MEMBER(k054156_056832_device::mh_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t omh = m_mh[offset];
	COMBINE_DATA(m_mh + offset);
	if(false && omh != m_mh[offset]) {
		logerror("YTY scroll %d %4x\n", offset, m_mh[offset]);
		logerror("mh_w %d, %04x @ %04x\n", offset, data, mem_mask);
	}
}

WRITE16_MEMBER(k054156_056832_device::mpz_w)
{
	if(ACCESSING_BITS_0_7 && m_mpz != (data & 0xff)) {
		m_mpz = data;
		select_linescroll_page();
		logerror("mpz_w scroll bank (%d, %d)\n",
				 data & 7,
				 (data >> 3) & 7);
	}
}

WRITE16_MEMBER(k054156_056832_device::mpa_w)
{
	if(ACCESSING_BITS_0_7 && m_mpa != (data & 0xff)) {
		m_mpa = data;
		select_cpu_page();
		if(false)
			logerror("mpa_w cpu bank (%d, %d)\n",
					 data & 7,
					 (data >> 3) & 7);
	}
}

WRITE16_MEMBER(k054156_056832_device::cadlm_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t old = m_cadlm;
	COMBINE_DATA(&m_cadlm);
	if(old != m_cadlm)
		logerror("cadlm_w %04x @ %04x\n", data, mem_mask);
}

WRITE16_MEMBER(k054156_056832_device::cadh_w)
{
	if(ACCESSING_BITS_0_7 && m_cadh != data) {
		m_cadh = data;
		logerror("cadh_w %04x @ %04x\n", data, mem_mask);
	}
}

WRITE16_MEMBER(k054156_056832_device::vrc_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t old = m_vrc;
	COMBINE_DATA(&m_vrc);
	if(old != m_vrc)
		logerror("vrc_w %04x @ %04x\n", data, mem_mask);
}

WRITE16_MEMBER(k054156_056832_device::offh_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t old = m_offh;
	COMBINE_DATA(&m_offh);
	if(old != m_offh) {
		logerror("offh_w %04x %4d @ %04x\n", m_offh, m_offh, mem_mask);
		logerror("TIMINGS TH %4d\n", m_offh & 0xfff);
		logerror("YTY Hoff %4x\n", m_offh);
	}
}

WRITE16_MEMBER(k054156_056832_device::offv_w)
{
	if(mem_mask == 0xffff && (m_reg4 & 0x20))
		mem_mask = 0x00ff;
	uint16_t old = m_offv;
	COMBINE_DATA(&m_offv);
	if(old != m_offv) {
		logerror("offv_w %04x %4d @ %04x\n", m_offv, m_offv, mem_mask);
		logerror("TIMINGS TV %4d\n", m_offv & 0x7ff);
		logerror("XTX Voff %4x\n", m_offv);
	}
}

WRITE16_MEMBER(k054156_056832_device::vrc2_w)
{
	if(ACCESSING_BITS_8_15 && m_vrc2[offset*2] != ((data >> 8) & 0x3f)) {
		m_vrc2[offset*2] = (data >> 8) & 0x3f;
		logerror("vrc2_w %d, %02x\n", offset*2, m_vrc2[offset*2]);
	}
	if(ACCESSING_BITS_0_7 && m_vrc2[offset*2+1] != (data & 0x3f)) {
		m_vrc2[offset*2+1] = data & 0x3f;
		logerror("vrc2_w %d, %02x\n", offset*2+1, m_vrc2[offset*2+1]);
	}
}

WRITE8_MEMBER(k054156_056832_device::reg1b_w)
{
	if(m_reg1b != (data & 0xff)) {
		m_reg1b = data;
		logerror("reg1b_w %02x\n", data);
	}
}

WRITE8_MEMBER(k054156_056832_device::reg2b_w)
{
	if(m_reg2b != (data & 0xff)) {
		m_reg2b = data;
		logerror("reg2b_w %02x\n", data);
	}
}

WRITE8_MEMBER(k054156_056832_device::reg3b_w)
{
	if(m_reg3b != (data & 0xff)) {
		m_reg3b = data;
		logerror("reg3b_w %02x\n", data);
	}
}

WRITE8_MEMBER(k054156_056832_device::reg4b_w)
{
	if(m_reg4b != (data & 0xff)) {
		m_reg4b = data;
		//		logerror("reg4b_w %02x\n", data);
	}
}

WRITE8_MEMBER(k054156_056832_device::reg1_8w)
{
	reg1_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::reg2_8w)
{
	reg2_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::reg3_8w)
{
	reg3_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::reg4_8w)
{
	reg4_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::reg5_8w)
{
	reg5_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::rzs_8w)
{
	rzs_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::ars_8w)
{
	ars_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::bv_8w)
{
	bv_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::bh_8w)
{
	bh_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::mv_8w)
{
	mv_w(space, offset >> 1, offset & 1 ? data << 8 : data, offset & 1 ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::mh_8w)
{
	mh_w(space, offset >> 1, offset & 1 ? data << 8 : data, offset & 1 ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::mpz_8w)
{
	mpz_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::mpa_8w)
{
	mpa_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::cadlm_8w)
{
	cadlm_w(space, 0, offset ? data << 8 : data, offset ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::cadh_8w)
{
	cadh_w(space, 0, data, 0xff);
}

WRITE8_MEMBER(k054156_056832_device::vrc_8w)
{
	vrc_w(space, 0, offset ? data << 8 : data, offset ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::offh_8w)
{
	offh_w(space, 0, offset ? data << 8 : data, offset ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::offv_8w)
{
	offv_w(space, 0, offset ? data << 8 : data, offset ? 0xff00 : 0x00ff);
}

WRITE8_MEMBER(k054156_056832_device::vrc2_8w)
{
	vrc2_w(space, 0, offset ? data << 8 : data, offset ? 0xff00 : 0x00ff);
}

READ8_MEMBER  (k054156_056832_device::vram8_r)
{
	switch(m_cur_vram_access) {
	case vram_access::l32w16:
	case vram_access::l32w24:
		return m_cur_cpu_page[offset >> 2] >> (24 - 8*(offset & 3));
	default:
		abort();
	}
	return 0;
}

WRITE8_MEMBER (k054156_056832_device::vram8_w)
{
	switch(m_cur_vram_access) {
	case vram_access::l32w16: {
		if(offset & 2) {
			int shift = 24 - 8*(offset & 3);
			m_cur_cpu_page[offset >> 2] = (m_cur_cpu_page[offset >> 2] & ~(0xff << shift)) | (data << shift);
		}
		break;
	}
	case vram_access::l32w24: {
		if(offset & 3) {
			int shift = 24 - 8*(offset & 3);
			m_cur_cpu_page[offset >> 2] = (m_cur_cpu_page[offset >> 2] & ~(0xff << shift)) | (data << shift);
		}
		break;
	}
	default:
		abort();
	}
}

READ16_MEMBER (k054156_056832_device::vram16_r)
{
	switch(m_cur_vram_access) {
	case vram_access::l16w16:
	case vram_access::l16w24:
		if(!(offset & 0x800))
			return m_cur_cpu_page[offset];
		else
			return m_cur_cpu_page[offset & 0x7ff] >> 16;

	case vram_access::l32w16:
	case vram_access::l32w24:
		return (offset & 1) ? m_cur_cpu_page[offset >> 1] : m_cur_cpu_page[offset >> 1] >> 16;

	default:
		abort();
	}
	return 0;
}

WRITE16_MEMBER(k054156_056832_device::vram16_w)
{
	if(m_reg4 & 0x20)
		mem_mask &= 0x00ff;
	switch(m_cur_vram_access) {
	case vram_access::l16w16:
		if(!(offset & 0x800))
			COMBINE_DATA(m_cur_cpu_page + offset);
		break;

	case vram_access::l16w24:
		if(!(offset & 0x800))
			COMBINE_DATA(m_cur_cpu_page + offset);
		else
			m_cur_cpu_page[offset & 0x7ff] = (m_cur_cpu_page[offset & 0x7ff] & 0xffff) | ((data & 0xff) << 16);
		break;

	case vram_access::l32w16:
		if(offset & 1)
			COMBINE_DATA(m_cur_cpu_page + (offset >> 1));
		break;

	case vram_access::l32w24:
		if(offset & 1) {
			COMBINE_DATA(m_cur_cpu_page + (offset >> 1));
		} else if(ACCESSING_BITS_0_7) {
			m_cur_cpu_page[offset >> 1] = (m_cur_cpu_page[offset >> 1] & 0xffff) | ((data & 0xff) << 16);
		}
		break;

	default:
		abort();
	}
}

READ32_MEMBER (k054156_056832_device::vram32_r)
{
	m_cur_a0 = 0;

	switch(m_cur_vram_access) {
	case vram_access::l32w16:
	case vram_access::l32w24:
		return m_cur_cpu_page[offset];
	default:
		abort();
	}
	return 0;
}

WRITE32_MEMBER(k054156_056832_device::vram32_w)
{
	if(m_reg4 & 0x20)
		mem_mask &= 0x00ff00ff;
	switch(m_cur_vram_access) {
	case vram_access::l32w16:
		mem_mask &= 0x0000ffff;
		COMBINE_DATA(m_cur_cpu_page + offset);
		break;
	case vram_access::l32w24:
		mem_mask &= 0x00ffffff;
		COMBINE_DATA(m_cur_cpu_page + offset);
		break;
	default:
		abort();
	}
}


READ8_MEMBER  (k054156_056832_device::rom8_r)
{
	uint32_t off;
	if(!m_is_054157) {
		off = (m_vrc2[(m_cadlm >> 5) & 7] << 17) | ((m_cadlm & 0x1f) << 12) | m_cur_a0;
		off |= (offset >> 1) & ~1;
		off = (off << 2) | (offset & 3);
	} else {
		off = (m_cadlm << 13) | offset;
	}
	const uint8_t *rom = m_region->base() + (off & (m_region->bytes() - 1));
	uint8_t res = rom[0];
	m_cur_a0 = m_cur_a0 ^ 1;
	return res;
}

READ16_MEMBER (k054156_056832_device::rom16_r)
{
	uint32_t off;
	if(!m_is_054157) {
		off = (m_vrc2[(m_cadlm >> 5) & 7] << 17) | ((m_cadlm & 0x1f) << 12) | m_cur_a0;
		off |= offset & ~1;
		off = (off << 2) | ((offset & 1) << 1);
	} else {
		off = (m_cadlm << 12) | offset;
		off = off << 1;
	}
	const uint8_t *rom = m_region->base() + (off & (m_region->bytes() - 1));
	uint16_t res = (rom[0] << 8) | rom[1];
	m_cur_a0 = m_cur_a0 ^ 1;
	return res;
}

READ32_MEMBER (k054156_056832_device::rom32_r)
{
	assert(!m_is_054157);
	uint32_t off = (m_vrc2[(m_cadlm >> 5) & 7] << 17) | ((m_cadlm & 0x1f) << 12) | m_cur_a0;
	off |= offset << 1;
	off = off << 2;
	const uint8_t *rom = m_region->base() + (off & (m_region->bytes() - 1));
	uint32_t res = (rom[0] << 24) | (rom[1] << 16) | (rom[2] << 8) | rom[3];
	m_cur_a0 = m_cur_a0 ^ 1;
	return res;
}

k054156_056832_device::k054156_056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054156_056832, "054156/056832 Tilemap Generator Combo", tag, owner, clock, "k054156_056832", __FILE__),
	  device_gfx_interface(mconfig, *this),
	  device_video_interface(mconfig, *this, false),
	  m_int1_cb(*this),
	  m_vblank_cb(*this),
	  m_region(*this, DEVICE_SELF)
{
	m_is_054157 = false;
	m_is_5bpp = false;
	m_is_dual = false;
	memset(m_page_pointers, 0, sizeof(m_page_pointers));
	memset(m_tilemap_page, 0, sizeof(m_tilemap_page));
	m_cur_a0 = 0;
	m_screen_tag = nullptr;
}

k054156_056832_device::k054156_056832_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	  device_gfx_interface(mconfig, *this),
	  device_video_interface(mconfig, *this, false),
	  m_int1_cb(*this),
	  m_vblank_cb(*this),
	  m_region(*this, DEVICE_SELF)
{
	m_is_054157 = false;
	m_is_5bpp = false;
	m_is_dual = false;
	memset(m_page_pointers, 0, sizeof(m_page_pointers));
	memset(m_tilemap_page, 0, sizeof(m_tilemap_page));
	m_cur_a0 = 0;
	m_screen_tag = nullptr;
}

k054156_054157_device::k054156_054157_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: k054156_056832_device(mconfig, K054156_054157, "054156/054157 Tilemap Generator Combo", tag, owner, clock, "k054156_054157", __FILE__)
{
	m_is_054157 = true;
}

void k054156_054157_device::set_5bpp()
{
	m_is_5bpp = true;
}

void k054156_054157_device::set_dual()
{
	m_is_dual = true;
}

k058143_056832_device::k058143_056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: k054156_056832_device(mconfig, K058143_056832, "058143/056832 Tilemap Generator Combo", tag, owner, clock, "k058143_056832", __FILE__)
{
}


void k054156_056832_device::device_start()
{
	m_int1_cb.resolve_safe();
	m_vblank_cb.resolve_safe();

	save_item(NAME(m_mv));
	save_item(NAME(m_mh));
	save_item(NAME(m_cadlm));
	save_item(NAME(m_vrc));
	save_item(NAME(m_offh));
	save_item(NAME(m_offv));
	save_item(NAME(m_bv));
	save_item(NAME(m_bh));
	save_item(NAME(m_reg1h));
	save_item(NAME(m_reg1l));
	save_item(NAME(m_reg2));
	save_item(NAME(m_reg3h));
	save_item(NAME(m_reg3l));
	save_item(NAME(m_reg4));
	save_item(NAME(m_reg5));
	save_item(NAME(m_rzs));
	save_item(NAME(m_ars));
	save_item(NAME(m_mpz));
	save_item(NAME(m_mpa));
	save_item(NAME(m_cadh));
	save_item(NAME(m_reg1b));
	save_item(NAME(m_reg2b));
	save_item(NAME(m_reg3b));
	save_item(NAME(m_reg4b));
	save_item(NAME(m_vrc2));

	memset(m_mv, 0, sizeof(m_mv));
	memset(m_mh, 0, sizeof(m_mh));
	m_cadlm = 0x0000;
	m_vrc   = 0x0000;
	m_offh  = 0x0000;
	m_offv  = 0x0000;

	memset(m_bv, 0, sizeof(m_bv));
	memset(m_bh, 0, sizeof(m_bh));

	m_reg1h = 0x00;
	m_reg1l = 0x00;
	m_reg2  = 0x00;
	m_reg3h = 0x00;
	m_reg3l = 0x00;
	m_reg4  = 0x00;
	m_reg5  = 0x00;
	m_rzs   = 0x00;
	m_ars   = 0x00;
	m_mpz   = 0x00;
	m_mpa   = 0x00;
	m_cadh  = 0x00;
	m_reg1b = 0x00;
	m_reg2b = 0x00;
	m_reg3b = 0x00;
	m_reg4b = 0x00;

	memset(m_vrc2, 0, sizeof(m_vrc2));

	decode_character_roms();

	m_videoram.resize(m_sizex * m_sizey * 0x800);
	memset(&m_videoram[0], 0, m_videoram.size()*sizeof(m_videoram[0]));
	save_pointer(NAME(&m_videoram[0]), m_sizex * m_sizey * 0x800);

	for(int y=0; y<8; y++)
		for(int x=0; x<8; x++) {
			int xx = x & (m_sizex - 1);
			int yy = y & (m_sizey - 1);
			m_page_pointers[y][x] = &m_videoram[0x800 * (xx + yy*m_sizex)];
		}

	select_cpu_page();
	select_linescroll_page();
	select_vram_access();
	for(int i=0; i<4; i++)
		setup_tilemap(i);

	if(m_screen)
		m_screen->register_vblank_callback(vblank_state_delegate(&k054156_054157_device::screen_vblank, this));
}

void k054156_056832_device::device_post_load()
{
	select_cpu_page();
	select_linescroll_page();
	select_vram_access();
	for(int i=0; i<4; i++)
		setup_tilemap(i);	

	decode_character_roms();
}

void k054156_056832_device::screen_vblank(screen_device &src, bool state)
{
	m_vblank_cb(state);
	m_int1_cb(state && (m_reg4 & 1));
}

void k054156_056832_device::select_cpu_page()
{
	m_cur_cpu_page = m_page_pointers[(m_mpa >> 3) & 7][m_mpa & 7];
}

void k054156_056832_device::select_linescroll_page()
{
	m_cur_linescroll_page = m_page_pointers[(m_mpz >> 3) & 7][m_mpz & 7];
}

void k054156_056832_device::select_vram_access()
{
	int mode = 4-2*(m_ars & 3);
	if(mode < 0)
		mode = 0;
	if(m_vramwidth == 24)
		mode++;
	m_cur_vram_access = vram_access(mode);
}

void k054156_056832_device::setup_tilemap(int layer)
{
	m_x[layer] = (m_bh[layer] >> 3) & (m_sizex - 1);
	m_y[layer] = (m_bv[layer] >> 3) & (m_sizey - 1);
	m_sx[layer] = 1+(m_bh[layer] & (m_sizex - 1));
	m_sy[layer] = 1+(m_bv[layer] & (m_sizey - 1));

	int x0 = m_x[layer];
	int y0 = m_y[layer];
	int mx = m_sx[layer] - 1;
	int my = m_sy[layer] - 1;

	logerror("layout %d (%d,%d)-(%d,%d)\n", layer, x0, y0, x0+mx, y0+my);
	for(int y=0; y<8; y++)
		for(int x=0; x<8; x++)
			m_tilemap_page[layer][y][x] = m_page_pointers[(y0 + (y & my)) & (m_sizey - 1)][(x0 + (x & mx)) & (m_sizex - 1)];
}

template<bool gflipx, bool gflipy> void k054156_056832_device::draw_page_512x1(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, const uint32_t *page, gfx_element *g, uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y, int32_t basex, int32_t basey)
{
	int bpp = m_reg1h & 0x08 ? 4 : m_reg1h & 0x04 ? 8 : 4 + (m_reg1h & 3);
	int width = m_reg4 & 0x10 ? 24 : 16;
	int flipbits = width - 2 - ((m_reg4 & 0xc0) >> 5);
	int flipmask = (m_reg2 >> (2*layer)) & 3;

	int vrcb = 0;
	uint32_t vrcm = 0;
	if(m_reg3l & 7) {
		vrcb = m_reg3l & 1 ? 10-3 : m_reg3l & 2 ? 8-3 : 14-3;
		vrcm = 0xffff ^ (3 << vrcb);
	}

	for(uint32_t y = min_y; y <= max_y; y++) {
		uint32_t info = page[y];
		uint32_t code = info & 0xffff;
		if(vrcb) {
			uint16_t vrc = m_vrc >> (4*((code >> vrcb) & 3));
			code = (code & vrcm) | ((vrc & 3) << vrcb) | ((vrc & 0xc) << 14);
		}

		if(!m_is_054157)
			code = (code & 0x1fff) | (m_vrc2[(code >> 13) & 7] << 13);

		int flipx, flipy;
		uint32_t color;

		if(width == 24) {
			flipx = (info >> flipbits) & flipmask & 1;
			if(gflipx)
				flipx ^= 1;
			flipy = (((info >> flipbits) & flipmask) >> 1) & 1;
			if(gflipy)
				flipy ^= 1;
			if(flipbits == 22)
				color = (info & 0x3f0000) >> (bpp+12);
			else if(flipbits <= bpp+12)
				color = (info & 0xfc0000) >> (bpp+14);
			else if(flipbits == 20)
				color = ((info & 0xc00000) >> (bpp+14)) | ((info & 0x0f00) >> (bpp+12));
			else
				color = ((info & 0xf00000) >> (bpp+14)) | ((info & 0x0300) >> (bpp+12));
			//			color >>= 2; // gijoe?
		} else {
			color = 0;
			flipx = 0;
			flipy = 0;
		}
		if(code)
			logerror("layer %d %04x %d %d\n", layer, code, basex, basey+y);
		g->opaque(*bitmap, cliprect, code, color, flipx, false, basex, basey + (gflipy ? y^0xff : y));
	}
}

template<bool gflipx, bool gflipy> void k054156_056832_device::draw_page_8x8(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, const uint32_t *page, gfx_element *g, uint32_t min_x, uint32_t max_x, uint32_t min_y, uint32_t max_y, int32_t basex, int32_t basey)
{
	int bpp = m_reg1h & 0x08 ? 4 : m_reg1h & 0x04 ? 8 : 4 + (m_reg1h & 3);
	int width = m_reg4 & 0x10 ? 24 : 16;
	int flipbits = width - 2 - ((m_reg4 & 0xc0) >> 5);
	int flipmask = (m_reg2 >> (2*layer)) & 3;

	uint32_t tile_min_x = min_x >> 3;
	uint32_t tile_max_x = max_x >> 3;
	uint32_t tile_min_y = min_y >> 3;
	uint32_t tile_max_y = max_y >> 3;

	int vrcb = 0;
	uint32_t vrcm = 0;
	if(m_reg3l & 7) {
		vrcb = m_reg3l & 1 ? 10 : m_reg3l & 2 ? 8 : 14;
		vrcb = 14;
		vrcm = 0xffff ^ (3 << vrcb);
	}
	vrcb = 0;

	for(uint32_t y = tile_min_y; y <= tile_max_y; y++) {
		const uint32_t *tiles = page + (y << 6) + tile_min_x;
		for(uint32_t x = tile_min_x; x <= tile_max_x; x++) {
			uint32_t info = *tiles;
			uint32_t code = info & 0xffff;

			if(vrcb) {
				uint16_t vrc = m_vrc >> (4*((code >> vrcb) & 3));
				code = (code & vrcm) | ((vrc & 3) << vrcb) | ((vrc & 0xc) << 14);
			}

			if((code ^ info) & 0xffff && (info & 0xffff))
				logerror("vrc %04x -> %05x\n", info & 0xffff, code);

			if(!m_is_054157)
				code = (code & 0x1fff) | (m_vrc2[code >> 13] << 13);

			int flipx, flipy;
			uint32_t color;

			if(width == 24) {
				flipx = (info >> flipbits) & flipmask & 1;
				if(gflipx)
					flipx ^= 1;
				flipy = (((info >> flipbits) & flipmask) >> 1) & 1;
				if(gflipy)
					flipy ^= 1;
				if(flipbits == 22)
					color = (info & 0x3f0000) >> (bpp+12);
				else if(flipbits <= bpp+12)
					color = (info & 0xfc0000) >> (bpp+14);
				else if(flipbits == 20)
					color = ((info & 0xc00000) >> (bpp+14)) | ((info & 0x0f00) >> (bpp+12));
				else
					color = ((info & 0xf00000) >> (bpp+14)) | ((info & 0x0300) >> (bpp+12));
				//				color >>= 2; // gijoe?
			} else {
				color = 0;
				flipx = 0;
				flipy = 0;
			}

			g->opaque(*bitmap, cliprect, code, color, flipx, flipy, basex + ((gflipx ? x^0x3f : x) << 3), basey + ((gflipy ? y^0x1f : y) << 3));

			tiles++;
		}
	}
}

template<bool gflipy> uint32_t k054156_056832_device::screen_to_tile_y(int32_t y, uint32_t delta)
{
	if(gflipy)
		return (~(y + 1) + delta) & 0x7ff;
	else
		return (y + 1 + delta) & 0x7ff;
}

template<bool gflipy> int32_t k054156_056832_device::tile_to_screen_y(uint32_t ty, uint32_t delta)
{
	int32_t y;
	if(gflipy)
		y = ~(ty - delta) - 1;
	else
		y = ty - delta - 1;
	y = y & 0x7ff;
	if(y & 0x400)
		y -= 0x800;
	return y;
}


template<bool gflipx> uint32_t k054156_056832_device::screen_to_tile_x(int32_t x, uint32_t delta)
{
	if(gflipx)
		return (~(x - 22) + delta) & 0xfff;
	else
		return (x - 22 + delta) & 0xfff;
}

template<bool gflipx> int32_t k054156_056832_device::tile_to_screen_x(uint32_t tx, uint32_t delta)
{
	int32_t x;
	if(gflipx)
		x = ~(tx - delta) + 22;
	else
		x = tx - delta + 22;
	x = x & 0xfff;
	if(x & 0x800)
		x -= 0x1000;
	return x;
}

template<bool gflipx, bool gflipy> void k054156_056832_device::draw_line_block(bitmap_ind16 *bitmap, int layer, const rectangle &cliprect, uint32_t deltay, uint32_t deltax)
{
	uint32_t base_min_x = screen_to_tile_x<gflipx>(gflipx ? cliprect.max_x : cliprect.min_x, deltax);
	uint32_t base_max_x = screen_to_tile_x<gflipx>(gflipx ? cliprect.min_x : cliprect.max_x, deltax);
	uint32_t base_min_y = screen_to_tile_y<gflipy>(gflipy ? cliprect.max_y : cliprect.min_y, deltay);
	uint32_t base_max_y = screen_to_tile_y<gflipy>(gflipy ? cliprect.min_y : cliprect.max_y, deltay);
	uint32_t pxmin = base_min_x >> 9;
	uint32_t pxmax = base_max_x >> 9;
	uint32_t pymin = base_min_y >> 8;
	uint32_t pymax = base_max_y >> 8;

	uint32_t py = pymin;
	for(;;) {
		int32_t min_y = (base_min_y - (py << 8)) & 0x7ff;
		int32_t max_y = (base_max_y - (py << 8)) & 0x7ff;

		if(min_y > max_y)
			min_y = 0;
		if(max_y > 0xff)
			max_y = 0xff;

		int32_t basey = tile_to_screen_y<gflipy>((py << 8) | (gflipy ? 0xff : 0x00), deltay);

		uint32_t px = pxmin;
		for(;;) {	
			int32_t min_x = (base_min_x - (px << 9)) & 0xfff;
			int32_t max_x = (base_max_x - (px << 9)) & 0xfff;
			if(min_x > max_x)
				min_x = 0;
			if(max_x > 0x1ff)
				max_x = 0x1ff;

			int32_t basex = tile_to_screen_x<gflipx>((px << 9) | (gflipx ? 0x1ff : 0x00), deltax);;

			if(0)
			logerror("draw page layer %d pos=%d.%d (%d, %d)-(%d, %d) to (%d, %d)\n",
					 layer, px, py, min_x, min_y, max_x, max_y, basex, basey);
			if(m_reg5 & (1 << layer))
				draw_page_8x8<gflipx, gflipy>(bitmap, layer, cliprect, m_tilemap_page[layer][py][px], gfx(0), min_x, max_x, min_y, max_y, basex, basey);
			else
				draw_page_512x1<gflipx, gflipy>(bitmap, layer, cliprect, m_tilemap_page[layer][py][px], gfx(1), min_x, max_x, min_y, max_y, basex, basey);

			if(px == pxmax)
				break;
			px = (px+1) & 7;
		}
		if(py == pymax)
			break;
		py = (py+1) & 7;
	}
}

void k054156_056832_device::bitmap_update(bitmap_ind16 *bitmap, const rectangle &cliprect, int layer)
{
	static const int xdelta[4] = { 0, 2, 4, 5 };
	//	static const int xdelta[4] = { 22+0, 22+2, 22+4, 22+5 };
	//	static const int xdelta[4] = { 24+0, 24+4, 24+6, 24+8 };
	if(0)
	logerror("draw layer %d scroll = %03x %03x\n", layer, m_mh[layer] & 0xfff, m_mv[layer] & 0x7ff);

	switch((m_rzs >> (2*layer)) & 3) {
	case 0: {
		const uint32_t *sbase = m_cur_linescroll_page + 512 * layer;
		switch(m_reg1l & 0x30) {
		case 0x00:
			for(int y = cliprect.top(); y <= cliprect.bottom(); y++)
				draw_line_block<false, false>(bitmap, layer, rectangle(cliprect.left(), cliprect.right(), y, y), m_mv[layer],          sbase[(y + m_mv[layer]+1) & 511] - xdelta[layer]);
			break;
		case 0x10:
			for(int y = cliprect.top(); y <= cliprect.bottom(); y++)
				draw_line_block<true , false>(bitmap, layer, rectangle(cliprect.left(), cliprect.right(), y, y), m_mv[layer],          sbase[y & 511] - xdelta[layer] + m_offh);
			break;
		case 0x20:
			for(int y = cliprect.top(); y <= cliprect.bottom(); y++)
				draw_line_block<false, true >(bitmap, layer, rectangle(cliprect.left(), cliprect.right(), y, y), m_mv[layer] + m_offv, sbase[y & 511] - xdelta[layer]);
			break;
		case 0x30:
			for(int y = cliprect.top(); y <= cliprect.bottom(); y++)
				draw_line_block<true , true >(bitmap, layer, rectangle(cliprect.left(), cliprect.right(), y, y), m_mv[layer] + m_offv, sbase[y & 511] - xdelta[layer] + m_offh);
			break;
		}
		break;
	}
	case 2:
		logerror("blockscroll on %d\n", layer);
		break;
	case 1:
	case 3:
		switch(m_reg1l & 0x30) {
		case 0x00: draw_line_block<false, false>(bitmap, layer, cliprect, m_mv[layer],          m_mh[layer] - xdelta[layer]); break;
		case 0x10: draw_line_block<true , false>(bitmap, layer, cliprect, m_mv[layer],          m_mh[layer] - xdelta[layer] + m_offh); break;
		case 0x20: draw_line_block<false, true >(bitmap, layer, cliprect, m_mv[layer] + m_offv, m_mh[layer] - xdelta[layer]); break;
		case 0x30: draw_line_block<true , true >(bitmap, layer, cliprect, m_mv[layer] + m_offv, m_mh[layer] - xdelta[layer] + m_offh); break;
		}
		break;
	}
}

void k054156_056832_device::decode_character_roms()
{
	gfx_layout gfx_layouts[2];
	gfx_decode_entry gfx_entries[3];
	uint32_t extxoffs[512];

	int bpp;
	bool fastdotclock;
	if(m_is_054157) {
		bpp = m_is_5bpp ? 5 : m_is_dual ? 8 : 4;
		fastdotclock = false;
	} else if(m_reg1h & 0x08) {
		bpp = 4;
		fastdotclock = true;
	} else {
		fastdotclock = false;
		if(m_reg1h & 0x04)
			bpp = 8;
		else
			bpp = 4 | (m_reg1h & 0x03);
	}

	logerror("Decoding character roms as %d bpp, %s dotclock, %s-bits wide character rom bus\n", bpp, fastdotclock ? "fast" : "normal", m_is_054157 ? m_is_5bpp ? "16+1" : m_is_dual ? "2x16" : "16" : "32");

	if(m_is_5bpp)
		convert_chunky_planar();

	int bits_per_line = m_is_054157 && !m_is_5bpp & !m_is_dual ? 32 : 64;

	for(int i=0; i<2; i++) {
		gfx_layouts[i].width = i ? 512 : 8;
		gfx_layouts[i].height = i ? 1 : 8;
		gfx_layouts[i].total = m_region->bytes() / (i ? 64 : 8) / (bits_per_line/8);
		gfx_layouts[i].planes = bpp;
		if(m_is_054157 && !m_is_5bpp) {
			// Chunky format, 32 or 64 bits per line (64 when dual)
			for(int j=0; j<bpp; j++)
				gfx_layouts[i].planeoffset[j] = (j & 3) + (j & 4 ? 16 : 0);
			if(i) {
				for(int j=0; j<512; j++)
					extxoffs[j] = j*4 + (m_is_dual ? 2*(j & ~3) : 0);;
				gfx_layouts[i].extxoffs = extxoffs;
				gfx_layouts[i].yoffset[0] = 0;
			} else {
				for(int j=0; j<8; j++) {
					gfx_layouts[i].xoffset[j] = j*4 + (m_is_dual ? 2*(j & ~3) : 0);;
					gfx_layouts[i].yoffset[j] = j*bits_per_line;
				}
				gfx_layouts[i].extxoffs = nullptr;
			}
			
		} else {
			// Planar format, 64 bits per line (32 to 64 actually used)
			if(fastdotclock)
				for(int j=0; j<bpp; j++)
					gfx_layouts[i].planeoffset[bpp-1-j] = 8*j;
			else
				for(int j=0; j<bpp; j++)
					gfx_layouts[i].planeoffset[bpp-1-j] = 8*(j >> 1) + (j & 1 ? 32 : 0);
			if(i) {
				for(int j=0; j<512; j++)
					extxoffs[j] = (7-(j & 7)) + 64*(j >> 3);
				gfx_layouts[i].extxoffs = extxoffs;
				gfx_layouts[i].yoffset[0] = 0;
			} else {
				for(int j=0; j<8; j++) {
					gfx_layouts[i].xoffset[j] = j;
					gfx_layouts[i].yoffset[j] = j*64;
				}
				gfx_layouts[i].extxoffs = nullptr;
			}
		}

		gfx_layouts[i].extyoffs = nullptr;
		gfx_layouts[i].charincrement = bits_per_line * (i ? 64 : 8);

		gfx_entries[i].memory_region = tag();
		gfx_entries[i].start = 0;
		gfx_entries[i].gfxlayout = gfx_layouts + i;
		gfx_entries[i].color_codes_start = 0;
		gfx_entries[i].total_color_codes = palette().entries() >> bpp;
		gfx_entries[i].flags = 0;
	}
	gfx_entries[2].gfxlayout = nullptr;

	decode_gfx(gfx_entries);

	if(m_is_5bpp) {
		for(int i=0; i<2; i++)
			for(int j=0; j<gfx_layouts[i].total; j++)
				gfx(i)->get_data(j);
		convert_planar_chunky();
	}
}

void k054156_056832_device::convert_chunky_planar()
{
	// Convert bitplanes 0-3 (out of 5) from chunky to planar
	uint32_t size = m_region->bytes();
	uint8_t *data = m_region->base();
	for(uint32_t pos = 0; pos < size; pos += 8) {
		uint32_t bits = (data[pos] << 24) | (data[pos+1] << 16) | (data[pos+4] << 8) | data[pos+5];
		uint32_t ov = bits;
		bits = BITSWAP32(bits,
						 31, 27, 23, 19, 15, 11, 7, 3,
						 30, 26, 22, 18, 14, 10, 6, 2,
						 29, 25, 21, 17, 13,  9, 5, 1,
						 28, 24, 20, 16, 12,  8, 4, 0);
		data[pos  ] = bits >> 24;
		data[pos+1] = bits >> 16;
		data[pos+4] = bits >> 8;
		data[pos+5] = bits;
		if(pos < 32)
			logerror("%x %08x -> %08x\n", pos, ov, bits);
	}
}

void k054156_056832_device::convert_planar_chunky()
{
	// Convert bitplanes 0-3 (out of 5) from planar back to chunky
	uint32_t size = m_region->bytes();
	uint8_t *data = m_region->base();
	for(uint32_t pos = 0; pos < size; pos += 8) {
		uint32_t bits = (data[pos] << 24) | (data[pos+1] << 16) | (data[pos+4] << 8) | data[pos+5];
		bits = BITSWAP32(bits,
						 0, 4,  8, 12, 16, 20, 24, 28,
						 1, 5,  9, 13, 17, 21, 25, 29,
						 2, 6, 10, 14, 18, 22, 26, 30,
						 3, 7, 11, 15, 19, 23, 27, 31);
		data[pos  ] = bits >> 24;
		data[pos+1] = bits >> 16;
		data[pos+4] = bits >> 8;
		data[pos+5] = bits;
	}
}
