// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert

#include "emu.h"
#include "k055555.h"
#include "k054338.h" // For the bitmap indexes in input

/*
K055555
-------

Priority encoder.  Always found in conjunction with k054338, but the
reverse isn't true.  The 55555 has 8 inputs: "A", "B", "C", and "D"
intended for a 156/157 type tilemap chip, "OBJ" intended for a '246
type sprite chip, and "SUB1-SUB3" which can be used for 3 additional
layers.  In addition a background layer with an optional gradient is
internally generated.  The disp register allows to enable/disable
layers independently.

For each pixel the circuit computes for each layer a 17-bits color, a
8-bits priority, 2 bits of shadow, 2 bits of mixing mode, and 2 bits
of brightness.  All 8 priorities are compared, with an implicit order
in case of ties.  The two front pixels are sent to the 054338.


  Layers A-D:

Layers A to D provide 10 color bits as input.  Transparency is
determined by checking if the <nl> low bits are 0, when nl is 4 to 8
depending on the colsel setting.

Color is built from the 10 color bits + 7 bits from clbk added at the
top, for 17 bits total.

Priority of the layer is set in the associated pri register.  For
layers A and B, default priority is in register pri0.  If the
associated ?p bit of colchg is set and the <nl> bottom bits of the
color have a higher value that the <nl> bottom bits of colpri, then
the priority is taken from pri1.

Mixing mode bits are taken either from bits 8-9 of the input or the v
inmix value depending on the v inmix on bits.

Brigthness is taken from v inbri.  For layers A-B, if the ?b bit of
colchg is set, brightness is forced to zero if the <nl> bottom bits of
the color have a lower or equal value that the <nl> bottom bits of
colpri.


  Object/Sub 1-3:

Object and Sub 1 layers provide 8 color bits, 8 priority bits, 2
mixing mode bits and 2 brightness bits.  Sub 2-3 is the same except
with 17 color bits.  Transparency is determined by checking if the
<nl> low bits are 0, when nl is 4 to 8 depending on the colsel
setting, same as for layers A-D.

For Object and Sub 1, color is built from the 8 color bits, plus the
bits from priority which are not selected by inpri.  For the bits
selected by inpri, clbk is used instead starting by bit 10 (same
alignement than for layers A-D).

For Sub 2 and 3, color is build from bits 0-9 of the input.  Bits
10-16 come either from the input or cblk depending on the cblk on
register.

Priority is built from the priority input for the bits selected by
inpri, from the pri register for the others.

Mixing mode is taken from either the mixing mode input of the os inmix
value depending on the os inmix on bits.

Brightness mode is similarly taken from either the input or os inbri
following os inbri on.


  Shadow bits:

There are two shadow bits not tied to any specific layer, but usually
provided by the sprite generator.  When both are zero, there is no
shadow and the result shadow bits are zero.  Otherwise, the two bits
select a test to do (never, <, = or >, shd pri sel register) and a
priority level to test against (shd pri register).  If the test passes
for the two top layers, the shadow bits are passed on.

Each layer says whether it can be shadowed by its shd on bit.  If
zero, shadow value will always be 0 for that layer, no matter the test result.


  Background layer:

A background layer is generated when all normal layers are disabled or
transparent.  Shadow/mixing mode/brightness are zero.  Color is built
from 9 bits of an internal counter with 4 bits from bgc cblk added as
msb.

If the cnton bit is zero, the counter stays at zero.  When cnton is
one and bgcmd is zero the counter is reset at start of screen and
incremented on hblank (horizontal lines).  When both cnton and bgcmd
are one the counter is reset at start of line and incremented every
pixel.  The exact position of the resets is unknown, probably the
syncs.


  Priority comparison:

For each pixel, each of the eight layers has a computed 8-bits
priority associated.  Priority resolution depends on prflp bit.

When prflp is 0, the lowest priority score wins.  In case of equality,
the order is A>B>C>D>O>S2>S3>S1.

When prflp is 1, the highest priority score wins.  In case of
equality, the order is S1>S3>S2>O>D>B>C>A.


The two to highest priority pixels are output with their associated
shadow, mixing mode and brightness bits.  If only one pixel is enabled
and non-transparent, the second layer uses the background layer color.
If no pixel is available, both outputs use the background layer color.
When an output uses the background, the "no pixel present" signal is
set.


Register map:
                    7   6   5   4   3   2   1   0
00  bgc cblk        .   .   .   .   --bgc cblk---
01  bgc set         .   .   .   . sds prf cnt bgc
02  colsel0         .   -vb col--   .   -va col--
03  colsel1         .   -vd col--   .   -vc col--
04  colsel2         .   -s1 col--   .   --o col--
05  colsel3         .   -s3 col--   .   -s2 col--
06  colchg on       .   .   .   .  ap  ab  bp  bb
07  a pri0          ------------a pri0-----------
08  a pri1          ------------a pri1-----------
09  a colpri        ----------a colpri-----------
0a  b pri0          ------------b pri0-----------
0b  b pri1          ------------b pri1-----------
0c  b colpri        ----------b colpri-----------
0d  c pri           ------------c pri------------
0e  d pri           ------------d pri------------
0f  o pri           ------------o pri------------
10  s1 pri          -----------s1 pri------------
11  s2 pri          -----------s2 pri------------
12  s3 pri          -----------s3 pri------------
13  o inpri on      ----------o inpri on---------
14  s1 inpri on     ---------s1 inpri on---------
15  s2 inpri on     ---------s2 inpri on---------
16  s3 inpri on     ---------s3 inpri on---------
17  a cblk          .   ---------a cblk----------
18  b cblk          .   ---------b cblk----------
19  c cblk          .   ---------c cblk----------
1a  d cblk          .   ---------d cblk----------
1b  o cblk          .   ---------o cblk----------
1c  s1 cblk         .   --------s1 cblk----------
1d  s2 cblk         .   --------s2 cblk----------
1e  s3 cblk         .   --------s3 cblk----------
1f  s2 cblk on      .   --------s2 cblk on-------
20  s3 cblk on      .   --------s3 cblk on-------
21  v inmix         -vd--   -vc--   -vb--   -va--
22  v inmix on      -vdm-   -vcm-   -vbm-   -vam-
23  os inmix        -s3--   -s2--   -s1--   --o--
24  os inmix on     -s3m-   -s2m-   -s1m-   --om-
25  shd pri1        -----------shd pri1----------
26  shd pri2        -----------shd pri2----------
27  shd pri3        -----------shd pri3----------
28  shd on        s3s s2s s1s  os vds vcs vbs vas
29  shd pri sel     .   .   -ps3-   -ps2-   -ps1-
2a  v inbri         -vdb-   -vcb-   -vbb-   -vab-
2b  os inbri        -s3b-   -s2b-   -s1b-   -s0b-
2c  os inbri on     -s3m-   -s2m-   -s1m-   -s0m-
2d  disp           s3  s2  s1   o  vd  vc  vb  va

*/


const device_type K055555 = device_creator<k055555_device>;

DEVICE_ADDRESS_MAP_START(map, 8, k055555_device)
	AM_RANGE(0x00, 0x00) AM_WRITE(bgc_cblk_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(bgc_set_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(colset0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(colset1_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(colset2_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(colset3_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(colchg_on_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(a_pri0_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(a_pri1_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(a_colpri_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(b_pri0_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(b_pri1_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(b_colpri_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(c_pri_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(d_pri_w)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(o_pri_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(s1_pri_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(s2_pri_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(s3_pri_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(o_inpri_on_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(s1_inpri_on_w)
	AM_RANGE(0x15, 0x15) AM_WRITE(s2_inpri_on_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(s3_inpri_on_w)
	AM_RANGE(0x17, 0x17) AM_WRITE(a_cblk_w)
	AM_RANGE(0x18, 0x18) AM_WRITE(b_cblk_w)
	AM_RANGE(0x19, 0x19) AM_WRITE(c_cblk_w)
	AM_RANGE(0x1a, 0x1a) AM_WRITE(d_cblk_w)
	AM_RANGE(0x1b, 0x1b) AM_WRITE(o_cblk_w)
	AM_RANGE(0x1c, 0x1c) AM_WRITE(s1_cblk_w)
	AM_RANGE(0x1d, 0x1d) AM_WRITE(s2_cblk_w)
	AM_RANGE(0x1e, 0x1e) AM_WRITE(s3_cblk_w)
	AM_RANGE(0x1f, 0x1f) AM_WRITE(s2_cblk_on_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(s3_cblk_on_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(v_inmix_w)
	AM_RANGE(0x22, 0x22) AM_WRITE(v_inmix_on_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(os_inmix_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(os_inmix_on_w)
	AM_RANGE(0x25, 0x25) AM_WRITE(shd_pri1_w)
	AM_RANGE(0x26, 0x26) AM_WRITE(shd_pri2_w)
	AM_RANGE(0x27, 0x27) AM_WRITE(shd_pri3_w)
	AM_RANGE(0x28, 0x28) AM_WRITE(shd_on_w)
	AM_RANGE(0x29, 0x29) AM_WRITE(shd_pri_sel_w)
	AM_RANGE(0x2a, 0x2a) AM_WRITE(v_inbri_w)
	AM_RANGE(0x2b, 0x2b) AM_WRITE(os_inbri_w)
	AM_RANGE(0x2c, 0x2c) AM_WRITE(os_inbri_on_w)
	AM_RANGE(0x2d, 0x2d) AM_WRITE(disp_w)
ADDRESS_MAP_END

WRITE8_MEMBER( k055555_device::bgc_cblk_w )
{
	if(m_bgc_cblk != data) {
		m_bgc_cblk = data;
		logerror("bgc_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::bgc_set_w )
{
	if(m_bgc_set != data) {
		m_bgc_set = data;
		logerror("bgc_set_w sdsel=%c pri_front=%s bg_gradient=%s gradient_orientation=%s\n",
				 data & 8 ? '1' : '0',
				 data & 4 ? "high" : "low",
				 data & 2 ? "on" : "off",
				 data & 1 ? "horizontal" : "vertical");
	}
}

WRITE8_MEMBER( k055555_device::colset0_w )
{
	if(m_colset[0] != data) {
		m_colset[0] = data;
		compute_color_mask(0);
		logerror("colset0_w a=%cbpp b=%cbpp\n",
				 data & 0x04 ? '8' : '4' + (data & 3),
				 data & 0x40 ? '8' : '4' + ((data >> 4) & 3));
	}
}

WRITE8_MEMBER( k055555_device::colset1_w )
{
	if(m_colset[1] != data) {
		m_colset[1] = data;
		compute_color_mask(1);
		logerror("colset1_w c=%cbpp d=%cbpp\n",
				 data & 0x04 ? '8' : '4' + (data & 3),
				 data & 0x40 ? '8' : '4' + ((data >> 4) & 3));
	}
}

WRITE8_MEMBER( k055555_device::colset2_w )
{
	if(m_colset[2] != data) {
		m_colset[2] = data;
		compute_color_mask(2);
		logerror("colset2_w o=%cbpp s1=%cbpp\n",
				 data & 0x04 ? '8' : '4' + (data & 3),
				 data & 0x40 ? '8' : '4' + ((data >> 4) & 3));
	}
}

WRITE8_MEMBER( k055555_device::colset3_w )
{
	if(m_colset[3] != data) {
		m_colset[3] = data;
		compute_color_mask(3);
		logerror("colset3_w s2=%cbpp s3=%cbpp\n",
				 data & 0x04 ? '8' : '4' + (data & 3),
				 data & 0x40 ? '8' : '4' + ((data >> 4) & 3));
	}
}

WRITE8_MEMBER( k055555_device::colchg_on_w )
{
	if(m_colchg_on != data) {
		m_colchg_on = data;
		logerror("colchg_on_w apri=%s abri=%s bpri=%s bbri=%s\n",
				 m_colchg_on & 8 ? "on" : "off",
				 m_colchg_on & 4 ? "on" : "off",
				 m_colchg_on & 2 ? "on" : "off",
				 m_colchg_on & 1 ? "on" : "off");
	}
}

WRITE8_MEMBER( k055555_device::a_pri0_w )
{
	if(m_pri[0] != data) {
		m_pri[0] = data;
		logerror("a_pri0_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::a_pri1_w )
{
	if(m_pri[8] != data) {
		m_pri[8] = data;
		logerror("a_pri1_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::a_colpri_w )
{
	if(m_colpri[0] != data) {
		m_colpri[0] = data;
		logerror("a_colpri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::b_pri0_w )
{
	if(m_pri[1] != data) {
		m_pri[1] = data;
		logerror("b_pri0_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::b_pri1_w )
{
	if(m_pri[9] != data) {
		m_pri[9] = data;
		logerror("b_pri1_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::b_colpri_w )
{
	if(m_colpri[1] != data) {
		m_colpri[1] = data;
		logerror("b_colpri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::c_pri_w )
{
	if(m_pri[2] != data) {
		m_pri[2] = data;
		logerror("c_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::d_pri_w )
{
	if(m_pri[3] != data) {
		m_pri[3] = data;
		logerror("d_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::o_pri_w )
{
	if(m_pri[4] != data) {
		m_pri[4] = data;
		logerror("o_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s1_pri_w )
{
	if(m_pri[5] != data) {
		m_pri[5] = data;
		logerror("s1_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s2_pri_w )
{
	if(m_pri[6] != data) {
		m_pri[6] = data;
		logerror("s2_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s3_pri_w )
{
	if(m_pri[7] != data) {
		m_pri[7] = data;
		logerror("s3_pri_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::o_inpri_on_w )
{
	if(m_inpri_on[0] != data) {
		m_inpri_on[0] = data;
		logerror("o_inpri_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s1_inpri_on_w )
{
	if(m_inpri_on[1] != data) {
		m_inpri_on[1] = data;
		logerror("s1_inpri_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s2_inpri_on_w )
{
	if(m_inpri_on[2] != data) {
		m_inpri_on[2] = data;
		logerror("s2_inpri_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s3_inpri_on_w )
{
	if(m_inpri_on[3] != data) {
		m_inpri_on[3] = data;
		logerror("s3_inpri_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::a_cblk_w )
{
	if(m_cblk[0] != data) {
		m_cblk[0] = data;
		logerror("a_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::b_cblk_w )
{
	if(m_cblk[1] != data) {
		m_cblk[1] = data;
		logerror("b_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::c_cblk_w )
{
	if(m_cblk[2] != data) {
		m_cblk[2] = data;
		logerror("c_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::d_cblk_w )
{
	if(m_cblk[3] != data) {
		m_cblk[3] = data;
		logerror("d_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::o_cblk_w )
{
	if(m_cblk[4] != data) {
		m_cblk[4] = data;
		logerror("o_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s1_cblk_w )
{
	if(m_cblk[5] != data) {
		m_cblk[5] = data;
		logerror("s1_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s2_cblk_w )
{
	if(m_cblk[6] != data) {
		m_cblk[6] = data;
		logerror("s2_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s3_cblk_w )
{
	if(m_cblk[7] != data) {
		m_cblk[7] = data;
		logerror("s3_cblk_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s2_cblk_on_w )
{
	if(m_cblk_on[0] != data) {
		m_cblk_on[0] = data;
		logerror("s2_cblk_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::s3_cblk_on_w )
{
	if(m_cblk_on[1] != data) {
		m_cblk_on[1] = data;
		logerror("s3_cblk_on_w %02x\n", data);
	}
}

WRITE8_MEMBER( k055555_device::v_inmix_w )
{
	if(m_v_inmix != data) {
		m_v_inmix = data;
		logerror("v_inmix_w a=%d b=%d c=%d d=%d\n",
				 (m_v_inmix >> 0) & 3,
				 (m_v_inmix >> 2) & 3,
				 (m_v_inmix >> 4) & 3,
				 (m_v_inmix >> 6) & 3);
	}
}

WRITE8_MEMBER( k055555_device::v_inmix_on_w )
{
	if(m_v_inmix_on != data) {
		m_v_inmix_on = data;
		logerror("v_inmix_on_w a=%c%c b=%c%c c=%c%c d=%c%c\n",
				 m_v_inmix_on & 0x02 ? 'i' : 'e',
				 m_v_inmix_on & 0x01 ? 'i' : 'e',
				 m_v_inmix_on & 0x08 ? 'i' : 'e',
				 m_v_inmix_on & 0x04 ? 'i' : 'e',
				 m_v_inmix_on & 0x20 ? 'i' : 'e',
				 m_v_inmix_on & 0x10 ? 'i' : 'e',
				 m_v_inmix_on & 0x80 ? 'i' : 'e',
				 m_v_inmix_on & 0x40 ? 'i' : 'e');
	}
}

WRITE8_MEMBER( k055555_device::os_inmix_w )
{
	if(m_os_inmix != data) {
		m_os_inmix = data;
		logerror("os_inmix_w o=%d s1=%d s2=%d s3=%d\n",
				 (m_os_inmix >> 0) & 3,
				 (m_os_inmix >> 2) & 3,
				 (m_os_inmix >> 4) & 3,
				 (m_os_inmix >> 6) & 3);
	}
}

WRITE8_MEMBER( k055555_device::os_inmix_on_w )
{
	if(m_os_inmix_on != data) {
		m_os_inmix_on = data;
		logerror("os_inmix_on_w o=%c%c s1=%c%c s2=%c%c s3=%c%c\n",
				 m_os_inmix_on & 0x02 ? 'i' : 'e',
				 m_os_inmix_on & 0x01 ? 'i' : 'e',
				 m_os_inmix_on & 0x08 ? 'i' : 'e',
				 m_os_inmix_on & 0x04 ? 'i' : 'e',
				 m_os_inmix_on & 0x20 ? 'i' : 'e',
				 m_os_inmix_on & 0x10 ? 'i' : 'e',
				 m_os_inmix_on & 0x80 ? 'i' : 'e',
				 m_os_inmix_on & 0x40 ? 'i' : 'e');
	}
}

WRITE8_MEMBER( k055555_device::shd_pri1_w )
{
	if(m_shd_pri[0] != data) {
		m_shd_pri[0] = data;
		logerror("shd_pri1_w %02x\n", data);
		update_shadow_value_array(1);
	}
}

WRITE8_MEMBER( k055555_device::shd_pri2_w )
{
	if(m_shd_pri[1] != data) {
		m_shd_pri[1] = data;
		logerror("shd_pri2_w %02x\n", data);
		update_shadow_value_array(2);
	}
}

WRITE8_MEMBER( k055555_device::shd_pri3_w )
{
	if(m_shd_pri[2] != data) {
		m_shd_pri[2] = data;
		logerror("shd_pri3_w %02x\n", data);
		update_shadow_value_array(3);
	}
}

WRITE8_MEMBER( k055555_device::shd_on_w )
{
	if(m_shd_on != data) {
		m_shd_on = data;
		logerror("shd_on_w a=%s b=%s c=%s d=%s o=%s s1=%s s2=%s s3=%s\n",
				 m_shd_on & 0x01 ? "on" : "off",
				 m_shd_on & 0x02 ? "on" : "off",
				 m_shd_on & 0x04 ? "on" : "off",
				 m_shd_on & 0x08 ? "on" : "off",
				 m_shd_on & 0x10 ? "on" : "off",
				 m_shd_on & 0x20 ? "on" : "off",
				 m_shd_on & 0x40 ? "on" : "off",
				 m_shd_on & 0x80 ? "on" : "off");
	}
}

WRITE8_MEMBER( k055555_device::shd_pri_sel_w )
{
	if(m_shd_pri_sel != data) {
		static const char *const shdmode[4] = { "off", ">", "=", "<" };
		m_shd_pri_sel = data;
		logerror("shd_pri_sel_w 1=%s 2=%s 3=%s\n",
				 shdmode[(m_shd_pri_sel >> 0) & 3],
				 shdmode[(m_shd_pri_sel >> 2) & 3],
				 shdmode[(m_shd_pri_sel >> 4) & 3]);
		update_shadow_value_array(1);
		update_shadow_value_array(2);
		update_shadow_value_array(3);
	}
}

WRITE8_MEMBER( k055555_device::v_inbri_w )
{
	if(m_v_inbri != data) {
		m_v_inbri = data;
		logerror("v_inbri_w a=%d b=%d c=%d d=%d\n",
				 (m_v_inbri >> 0) & 3,
				 (m_v_inbri >> 2) & 3,
				 (m_v_inbri >> 4) & 3,
				 (m_v_inbri >> 6) & 3);
	}
}

WRITE8_MEMBER( k055555_device::os_inbri_w )
{
	if(m_os_inbri != data) {
		m_os_inbri = data;
		logerror("os_inbri_w o=%d s1=%d s2=%d s3=%d\n",
				 (m_os_inbri >> 0) & 3,
				 (m_os_inbri >> 2) & 3,
				 (m_os_inbri >> 4) & 3,
				 (m_os_inbri >> 6) & 3);
	}
}

WRITE8_MEMBER( k055555_device::os_inbri_on_w )
{
	if(m_os_inbri_on != data) {
		m_os_inbri_on = data;
		logerror("os_inbri_on_w o=%c%c s1=%c%c s2=%c%c s3=%c%c\n",
				 m_os_inbri_on & 0x02 ? 'i' : 'e',
				 m_os_inbri_on & 0x01 ? 'i' : 'e',
				 m_os_inbri_on & 0x08 ? 'i' : 'e',
				 m_os_inbri_on & 0x04 ? 'i' : 'e',
				 m_os_inbri_on & 0x20 ? 'i' : 'e',
				 m_os_inbri_on & 0x10 ? 'i' : 'e',
				 m_os_inbri_on & 0x80 ? 'i' : 'e',
				 m_os_inbri_on & 0x40 ? 'i' : 'e');
	}
}

WRITE8_MEMBER( k055555_device::disp_w )
{
	if(m_disp != data) {
		m_disp = data;
		logerror("disp_w a=%s b=%s c=%s d=%s o=%s s1=%s s2=%s s3=%s\n",
				 m_disp & 0x01 ? "on" : "off",
				 m_disp & 0x02 ? "on" : "off",
				 m_disp & 0x04 ? "on" : "off",
				 m_disp & 0x08 ? "on" : "off",
				 m_disp & 0x10 ? "on" : "off",
				 m_disp & 0x20 ? "on" : "off",
				 m_disp & 0x40 ? "on" : "off",
				 m_disp & 0x80 ? "on" : "off");
	}
}
k055555_device::k055555_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K055555, "K055555 Priority Encoder", tag, owner, clock, "k055555", __FILE__)
{
}

k055555_device::~k055555_device()
{
}

void k055555_device::device_start()
{
	m_init_cb.bind_relative_to(*owner());
	m_update_cb.bind_relative_to(*owner());
	memset(m_bitmaps, 0, sizeof(m_bitmaps));

	save_item(NAME(m_bgc_cblk));
	save_item(NAME(m_bgc_set));
	save_item(NAME(m_colchg_on));
	save_item(NAME(m_shd_on));
	save_item(NAME(m_shd_pri_sel));
	save_item(NAME(m_v_inmix));
	save_item(NAME(m_v_inmix_on));
	save_item(NAME(m_os_inmix));
	save_item(NAME(m_os_inmix_on));
	save_item(NAME(m_v_inbri));
	save_item(NAME(m_os_inbri));
	save_item(NAME(m_os_inbri_on));
	save_item(NAME(m_disp));

	save_item(NAME(m_colset));
	save_item(NAME(m_cblk));
	save_item(NAME(m_cblk_on));
	save_item(NAME(m_pri));
	save_item(NAME(m_colpri));
	save_item(NAME(m_shd_pri));
	save_item(NAME(m_inpri_on));
}

void k055555_device::device_reset()
{
	m_bgc_cblk = 0x00;
	m_bgc_set = 0x00;
	m_colchg_on = 0x00;
	m_shd_on = 0x00;
	m_shd_pri_sel = 0x00;
	m_v_inmix = 0x00;
	m_v_inmix_on = 0x00;
	m_os_inmix = 0x00;
	m_os_inmix_on = 0x00;
	m_v_inbri = 0x00;
	m_os_inbri = 0x00;
	m_os_inbri_on = 0x00;
	m_disp = 0x00;

	memset(m_colset,   0, sizeof(m_colset));
	memset(m_cblk,     0, sizeof(m_cblk));
	memset(m_cblk_on,  0, sizeof(m_cblk_on));
	memset(m_pri,      0, sizeof(m_pri));
	memset(m_colpri,   0, sizeof(m_colpri));
	memset(m_shd_pri,  0, sizeof(m_shd_pri));
	memset(m_inpri_on, 0, sizeof(m_inpri_on));

	memset(m_shadow_value, 0^1, sizeof(m_shadow_value));

	for(int i=0; i<4; i++)
		compute_color_mask(i);

	for(int i=1; i<4; i++)
		update_shadow_value_array(i);
}

void k055555_device::device_post_load()
{
	for(int i=0; i<4; i++)
		compute_color_mask(i);

	for(int i=1; i<4; i++)
		update_shadow_value_array(i);
}

void k055555_device::compute_color_mask(int i)
{
	m_color_mask[i << 1] = m_colset[i] & 0x04 ? 0xff : 0xff >> (4 - (m_colset[i] & 3));
	m_color_mask[(i << 1)|1] = m_colset[i] & 0x40 ? 0xff : 0xff >> (4 - ((m_colset[i] >> 4) & 3));
}

void k055555_device::update_shadow_value_array(int i)
{
	uint8_t *sarray = m_shadow_value[i];
	uint8_t pri = m_shd_pri[i-1];
	switch((m_shd_pri_sel >> (2*(i-1))) & 3) {
	case 0:
		memset(sarray, 0^1, 256);
		break;
	case 1:
		if(pri)
			memset(sarray, i^1, pri);
		memset(sarray+pri, 0^1, 256-pri);
		break;
	case 2:
		memset(sarray, 0^1, 256);
		sarray[pri] = i^1;
		break;
	case 3:
		memset(sarray, 0^1, pri+1);
		if(pri != 255)
			memset(sarray+pri+1, i^1, 255-pri);
		break;
	}
}

void k055555_device::bitmap_update(bitmap_ind16 **bitmaps, const rectangle &cliprect)
{
	if(!m_bitmaps[0] || m_bitmaps[0]->width() != bitmaps[0]->width() || m_bitmaps[0]->height() != bitmaps[0]->height()) {
		if(m_bitmaps[0])
			for(int i=0; i<BITMAP_COUNT; i++)
				delete m_bitmaps[i];
		for(int i=0; i<BITMAP_COUNT; i++)
			m_bitmaps[i] = new bitmap_ind16(bitmaps[0]->width(), bitmaps[0]->height());
		if(!m_init_cb.isnull())
			m_init_cb(m_bitmaps);
	}

	m_update_cb(m_bitmaps, cliprect);
	uint16_t prixor = m_bgc_set & 8 ? 0x7ff : 0;
	uint8_t os_inbri_masked = m_os_inbri & m_os_inbri_on;
	uint8_t v_inmix_masked  = m_v_inmix  & m_v_inmix_on;
	uint8_t os_inmix_masked = m_os_inmix & m_os_inmix_on;

	uint8_t disp = m_disp;

	if(machine().input().code_pressed(KEYCODE_Q)) disp &= ~0x01;
	if(machine().input().code_pressed(KEYCODE_W)) disp &= ~0x02;
	if(machine().input().code_pressed(KEYCODE_E)) disp &= ~0x04;
	if(machine().input().code_pressed(KEYCODE_R)) disp &= ~0x08;
	if(machine().input().code_pressed(KEYCODE_T)) disp &= ~0x10;
	if(machine().input().code_pressed(KEYCODE_Y)) disp &= ~0x20;
	if(machine().input().code_pressed(KEYCODE_U)) disp &= ~0x40;
	if(machine().input().code_pressed(KEYCODE_I)) disp &= ~0x80;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		const uint16_t *acolor  = &m_bitmaps[LAYERA_COLOR ]->pix16(y, cliprect.min_x);
		const uint16_t *bcolor  = &m_bitmaps[LAYERB_COLOR ]->pix16(y, cliprect.min_x);
		const uint16_t *ccolor  = &m_bitmaps[LAYERC_COLOR ]->pix16(y, cliprect.min_x);
		const uint16_t *dcolor  = &m_bitmaps[LAYERD_COLOR ]->pix16(y, cliprect.min_x);
		const uint16_t *ocolor  = &m_bitmaps[LAYERO_COLOR ]->pix16(y, cliprect.min_x);
		const uint16_t *oattr   = &m_bitmaps[LAYERO_ATTR  ]->pix16(y, cliprect.min_x);
		const uint16_t *s1color = &m_bitmaps[LAYERS1_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *s1attr  = &m_bitmaps[LAYERS1_ATTR ]->pix16(y, cliprect.min_x);
		const uint16_t *s2color = &m_bitmaps[LAYERS2_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *s2attr  = &m_bitmaps[LAYERS2_ATTR ]->pix16(y, cliprect.min_x);
		const uint16_t *s3color = &m_bitmaps[LAYERS3_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *s3attr  = &m_bitmaps[LAYERS3_ATTR ]->pix16(y, cliprect.min_x);
		uint16_t *fc = &bitmaps[k054338_device::BITMAP_FRONT_COLOR]->pix16(y, cliprect.min_x);
		uint16_t *fa = &bitmaps[k054338_device::BITMAP_FRONT_ATTRIBUTES]->pix16(y, cliprect.min_x);
		uint16_t *bc = &bitmaps[k054338_device::BITMAP_LAYER2_COLOR]->pix16(y, cliprect.min_x);
		uint16_t *ba = &bitmaps[k054338_device::BITMAP_LAYER2_ATTRIBUTES]->pix16(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			uint16_t prif = 0x7ff, prib = 0x7ff;
			uint16_t colorf = 0x0000, colorb = 0x0000, attrf = 0x0000, attrb = 0x0000;
			uint16_t oa = *oattr++;

			if(disp & 0x01) {
				uint16_t col = *acolor++ & 0x3ff;
				if(col & m_color_mask[0]) {
					uint16_t lpri = (m_colchg_on & 8) && ((col & m_color_mask[0]) > (m_colpri[0] & m_color_mask[0])) ? m_pri[8] : m_pri[0];
					lpri = ((lpri << 3) | 0) ^ prixor;
					prif = lpri;
					colorf = (col & (0x0ff | (m_v_inmix_on << 8))) | (m_cblk[0] << 10);
					attrf = m_shd_on & 0x01 ? 0x8001 : 0x8000;
					attrf |= (m_v_inbri << 2) & 0xc;
					attrf |= ((v_inmix_masked << 4) | ((~m_v_inmix_on << 4) & (col >> 4))) & 0x30;
				}
			}
				
			if(disp & 0x02) {
				uint16_t col = *bcolor++ & 0x3ff;
				if(col & m_color_mask[1]) {
					uint16_t lpri = (m_colchg_on & 2) && ((col & m_color_mask[1]) > (m_colpri[1] & m_color_mask[1])) ? m_pri[9] : m_pri[1];
					lpri = ((lpri << 3) | 1) ^ prixor;

					uint16_t ncolor = (col & (0x0ff | (m_v_inmix_on << 6))) | (m_cblk[1] << 10);
					uint16_t nattr = m_shd_on & 0x02 ? 0x8001 : 0x8000;
					nattr |= (m_v_inbri << 0) & 0xc;
					nattr |= ((v_inmix_masked << 2) | ((~m_v_inmix_on << 2) & (col >> 4))) & 0x30;
					ncolor = ncolor ;
					if(!attrf || lpri < prif) {
						prib = prif; colorb = colorf; attrb = attrf;
						prif = lpri;
						colorf = ncolor;
						attrf = nattr;
					} else {
						prib = lpri;
						colorb = ncolor;
						attrb = nattr;
					}
				}
			}

			if(disp & 0x04) {
				uint16_t col = *ccolor++ & 0x3ff;
				if(col & m_color_mask[2]) {
					uint16_t lpri = m_pri[2];
					lpri = ((lpri << 3) | 2) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = (col & (0x0ff | (m_v_inmix_on << 4))) | (m_cblk[2] << 10);
						uint16_t nattr = m_shd_on & 0x04 ? 0x8001 : 0x8000;
						nattr |= (m_v_inbri >> 2) & 0xc;
						nattr |= ((v_inmix_masked << 0) | ((~m_v_inmix_on << 0) & (col >> 4))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(disp & 0x08) {
				uint16_t col = *dcolor++ & 0x3ff;
				if(col & m_color_mask[3]) {
					uint16_t lpri = m_pri[3];
					lpri = ((lpri << 3) | 3) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = (col & (0x0ff | (m_v_inmix_on << 2))) | (m_cblk[3] << 10);
						uint16_t nattr = m_shd_on & 0x08 ? 0x8001 : 0x8000;
						nattr |= (m_v_inbri >> 4) & 0xc;
						nattr |= ((v_inmix_masked >> 2) | ((~m_v_inmix_on >> 2) & (col >> 4))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(disp & 0x10) {
				uint16_t col = *ocolor++ & 0xff;
				if(col & m_color_mask[4]) {
					uint16_t lpri = ((oa & ~m_inpri_on[0]) | (m_pri[4] & m_inpri_on[0])) & 0xff;
					lpri = ((lpri << 3) | 4) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = col | (((oa & m_inpri_on[0]) | ((m_cblk[4] << 2) & ~m_inpri_on[0])) << 8);
						uint16_t nattr = m_shd_on & 0x10 ? 0x8001 : 0x8000;
						nattr |= ((os_inbri_masked << 2) | ((oa >> 8) & ~(m_os_inbri_on << 2))) & 0xc;
						nattr |= ((os_inmix_masked << 4) | ((~m_os_inmix_on << 4) & (oa >> 8))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(disp & 0x20) {
				uint16_t col  = *s1color++;
				uint16_t attr = *s1attr++;
				if(col & m_color_mask[5]) {
					// Note that the order is O-S2-S3-S1, so the 7 is correct
					uint16_t lpri = ((attr & ~m_inpri_on[1]) | (m_pri[5] & m_inpri_on[1])) & 0xff;
					lpri = ((lpri << 3) | 7) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = col | (((attr & m_inpri_on[1]) | ((m_cblk[5] << 2) & ~m_inpri_on[1])) << 8);
						uint16_t nattr = m_shd_on & 0x20 ? 0x8001 : 0x8000;
						nattr |= ((os_inbri_masked) | ((attr >> 8) & ~(m_os_inbri_on))) & 0xc;
						nattr |= ((os_inmix_masked << 2) | ((~m_os_inmix_on << 2) & (attr >> 8))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(disp & 0x40) {
				uint16_t col  = *s2color++;
				uint16_t attr = *s2attr++;
				if(col & m_color_mask[6]) {
					uint16_t lpri = ((attr & ~m_inpri_on[2]) | (m_pri[6] & m_inpri_on[2])) & 0xff;
					lpri = ((lpri << 3) | 5) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = (col & (0x3ff | (m_cblk_on[0] << 10))) | ((m_cblk[6] & ~m_cblk_on[0]) << 10);
						uint16_t nattr = m_shd_on & 0x40 ? 0x8001 : 0x8000;
						nattr |= ((os_inbri_masked >> 2) | ((attr >> 8) & ~(m_os_inbri_on >> 2))) & 0xc;
						nattr |= ((os_inmix_masked) | ((~m_os_inmix_on) & (attr >> 8))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(disp & 0x80) {
				uint16_t col  = *s3color++;
				uint16_t attr = *s3attr++;
				if(col & m_color_mask[7]) {
					uint16_t lpri = ((attr & ~m_inpri_on[3]) | (m_pri[7] & m_inpri_on[3])) & 0xff;
					lpri = ((lpri << 3) | 6) ^ prixor;

					if(!attrb || lpri < prib) {
						uint16_t ncolor = (col & (0x3ff | (m_cblk_on[1] << 10))) | ((m_cblk[7] & ~m_cblk_on[1]) << 10);
						uint16_t nattr = m_shd_on & 0x80 ? 0x8001 : 0x8000;
						nattr |= ((os_inbri_masked >> 4) | ((attr >> 8) & ~(m_os_inbri_on >> 4))) & 0xc;
						nattr |= ((os_inmix_masked >> 2) | ((~m_os_inmix_on >> 2) & (attr >> 8))) & 0x30;
						if(!attrf || lpri < prif) {
							prib = prif; colorb = colorf; attrb = attrf;
							prif = lpri;
							colorf = ncolor;
							attrf = nattr;
						} else {
							prib = lpri;
							colorb = ncolor;
							attrb = nattr;
						}
					}
				}
			}

			if(attrf & 1)
				attrf ^= m_shadow_value[(oa >> 8) & 3][(prif ^ prixor) >> 3];
			if(attrb & 1)
				attrb ^= m_shadow_value[(oa >> 8) & 3][(prib ^ prixor) >> 3];

			if(!(attrf & attrb & 0x8000)) {
				uint16_t col = (m_bgc_cblk & 0xf) << 9;
				if(m_bgc_set & 0x0002) {
					if(m_bgc_set & 0x0001)
						col |= x & 0x1ff;
					else
						col |= y & 0x1ff;
				}

				if(!(attrf & 0x8000))
					colorf = col;
				if(!(attrb & 0x8000))
					colorb = col;
			}

			*fc++ = colorf;
			*fa++ = attrf;
			*bc++ = colorb;
			*ba++ = attrb;
		}
	}
}
