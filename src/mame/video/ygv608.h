#ifndef __YGV608_H__
#define __YGV608_H__

/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 */

// Fundamental constants
#define YGV608_SPRITE_ATTR_TABLE_SIZE 256

enum {
	p5_rwai		= 0x80,		// increment register select on reads
	p5_rrai		= 0x40,		// increment register select on writes
	p5_rn		= 0x3f,		// register select

//  p6_res6     = 0xe0,
	p6_fp		= 0x10,		// position detection flag
	p6_fv		= 0x08,		// vertical border interval flag
	p6_fc		= 0x04,
	p6_hb		= 0x02,		// horizontal blanking flag?
	p6_vb		= 0x01,		// vertical blanking flag?

//  p7_res7     = 0xc0,
	p7_tr		= 0x20,
	p7_tc		= 0x10,
	p7_tl		= 0x08,
	p7_ts		= 0x04,
	p7_tn		= 0x02,
	p7_sr		= 0x01,

	r0_pnya		= 0x80,		// if set, increment name table address after reads
	r0_b_a		= 0x40,		// if set, we're reading from the second plane of tile data
	r0_pny		= 0x3f,		// y-coordinate of current name table address

	r1_pnxa		= 0x80,		// if set, increment name table address after read
	r1_res1		= 0x40,
	r1_pnx		= 0x3f,		// x-coordinate of current name table address

	r2_cpaw		= 0x80,		// if set, increment color palette address after writes
	r2_cpar		= 0x40,		// if set, increment color palette address after reads
//  r2_res2     = 0x20,
	r2_b_a		= 0x10,
	r2_scaw		= 0x08,		// if set, increment scroll address after writes
	r2_scar		= 0x04,		// if set, increment scroll address after reads
	r2_saaw		= 0x02,		// if set, increment sprite address after writes
	r2_saar		= 0x01,		// if set, increment sprite address after reads

	r7_dckm 	= 0x80,
	r7_flip		= 0x40,
//  r7_res7     = 0x30,
	r7_zron		= 0x08,		// if set, roz plane is active
	r7_md		= 0x06,		// determines 1 of 4 possible video modes
	r7_dspe		= 0x01,		// if set, display is enabled

	r8_hds		= 0xc0,
	r8_vds		= 0x30,
	r8_rlrt		= 0x08,
	r8_rlsc		= 0x04,
//  r8_res8     = 0x02,
	r8_pgs		= 0x01,

	r9_pts		= 0xc0,
	r9_slh		= 0x38,
	r9_slv		= 0x07,

	r10_spa		= 0xc0,		// misc global sprite attributes (x/y flip or sprite size)
	r10_spas	= 0x20,		// if set, spa controls sprite flip, if clear, controls sprite size
	r10_sprd	= 0x10,		// if set, sprites are disabled
	r10_mcb		= 0x0c,
	r10_mca		= 0x03,

	r11_scm		= 0xc0,
	r11_yse		= 0x20,
	r11_cbdr	= 0x10,
	r11_prm		= 0x0c,		// determines one of 4 priority settings
	r11_ctpb	= 0x02,
	r11_ctpa	= 0x01,

	r12_spf		= 0xc0,
	r12_bpf		= 0x38,
	r12_apf		= 0x07,

//  r14_res14   = 0xfc,
	r14_iep		= 0x02,		// if set, generate IRQ on position detection (?)
	r14_iev		= 0x01,		// if set, generate IRQ on vertical border interval draw

	// these 4 are currently unimplemented by us
	r16_fpm		= 0x80,
	r16_il8		= 0x40,
	r16_res16	= 0x20,
	r16_ih		= 0x1f,

	r17_ba1		= 0x70,
	r17_ba0		= 0x07,
	r18_ba3		= 0x70,
	r18_ba2		= 0x07,
	r19_ba5		= 0x70,
	r19_ba4		= 0x07,
	r20_ba7		= 0x70,
	r20_ba6		= 0x07,
	r21_bb1		= 0x70,
	r21_bb0		= 0x07,
	r22_bb3		= 0x70,
	r22_bb2		= 0x07,
	r23_bb5		= 0x70,
	r23_bb4		= 0x07,
	r24_bb7		= 0x70,
	r24_bb6		= 0x07,

	r39_hsw		= 0xe0,
	r39_hbw		= 0x1f,

	r40_htl89	= 0xc0,
	r40_hdw		= 0x3f,

	r43_vsw		= 0xe0,
	r43_vbw		= 0x1f,

	r44_vsls	= 0x40,
	r44_vdw		= 0x1f,

	r45_vtl8	= 0x80,
	r45_tres	= 0x40,
	r45_vdp		= 0x1f
};


typedef struct _ygv_ports {
	UINT8 na;			// P#0 - pattern name table data port (read/write)
	UINT8 p1;			// P#1 - sprite data port (read/write)
	UINT8 p2;			// P#2 - scroll data port (read/write)
	UINT8 p3;			// P#3 - colour palette data port (read/write)
	UINT8 p4;			// P#4 - register data port (read/write)
	UINT8 p5;			// P#5 - register select port (write only)
	UINT8 p6;			// P#6 - status port (read/write)
	UINT8 p7;			// P#7 - system control port (read/write)
} YGV_PORTS, *pYGV_PORTS;

typedef struct _ygv_regs {
	UINT8 r0;			// R#0 - pattern name table access ptr (r/w)
	UINT8 r1;			// R#1 - pattern name table access ptr (r/w)
	UINT8 r2;			// R#2 - built in ram access control
	UINT8 saa;			// R#3 - sprite attribute table access ptr (r/w)
	UINT8 sca;			// R#4 - scroll table access ptr (r/w)
	UINT8 cc;			// R#5 - color palette access ptr (r/w)
	UINT8 sba;			// R#6 - sprite generator base address (r/w)

	// R#7 - R#11 - screen control (r/w)
	UINT8 r7;			// misc screen control (r/w)
	UINT8 r8;			// misc screen control (r/w)
	UINT8 r9;			// misc screen control (r/w)
	UINT8 r10;			// misc screen control (r/w)
	UINT8 r11;			// misc screen control (r/w)

	UINT8 r12;			// R#12 - color palette selection (r/w)
	UINT8 bdc;			// R#13 - border colour (wo)

	// R#14 - R#16 - interrupt control
	UINT8 r14;
	UINT8 il;
	UINT8 r16;

	// R#17 - R#24 - base address (wo)
	UINT8 r17;
	UINT8 r18;
	UINT8 r19;
	UINT8 r20;
	UINT8 r21;
	UINT8 r22;
	UINT8 r23;
	UINT8 r24;

	// R#25 - R#38 - enlargement, contraction and rotation parameters (wo)
	UINT8 ax0;
	UINT8 ax8;
	UINT8 ax16;

	UINT8 dx0;
	UINT8 dx8;
	UINT8 dxy0;
	UINT8 dxy8;

	UINT8 ay0;
	UINT8 ay8;
	UINT8 ay16;

	UINT8 dy0;
	UINT8 dy8;
	UINT8 dyx0;
	UINT8 dyx8;

	// R#39 - R#46 - display scan control (wo)
	UINT8 r39;
	UINT8 r40;
	UINT8 hdsp;
	UINT8 htl;
	UINT8 r43;
	UINT8 r44;
	UINT8 r45;
	UINT8 vtl;

	// R#47 - R#49 - rom transfer control (wo)
	UINT8 tb5;
	UINT8 tb13;
	UINT8 tn4;

} YGV_REGS, *pYGV_REGS;

#define YGV608_MAX_SPRITES (YGV608_SPRITE_ATTR_TABLE_SIZE>>2)

// R#7(md)
#define MD_2PLANE_8BIT      0x00
#define MD_2PLANE_16BIT     0x02
#define MD_1PLANE_16COLOUR  0x04
#define MD_1PLANE_256COLOUR 0x06
#define MD_1PLANE           (MD_1PLANE_16COLOUR & MD_1PLANE_256COLOUR)
#define MD_SHIFT            0
#define MD_MASK             0x06

// R#8
#define PGS_64X32         0x0
#define PGS_32X64         0x1
#define PGS_SHIFT         0
#define PGS_MASK          0x01

// R#9
#define SLV_SCREEN        0x00
#define SLV_8             0x04
#define SLV_16            0x05
#define SLV_32            0x06
#define SLV_64            0x07
#define SLH_SCREEN        0x00
#define SLH_8             0x04
#define SLH_16            0x05
#define SLH_32            0x06
#define SLH_64            0x07
#define PTS_8X8           0x00
#define PTS_16X16         0x40
#define PTS_32X32         0x80
#define PTS_64X64         0xc0
#define PTS_SHIFT         0
#define PTS_MASK          0xc0

// R#10
#define SPAS_SPRITESIZE    0
#define SPAS_SPRITEREVERSE 1

// R#10(spas)=1
#define SZ_8X8            0x00
#define SZ_16X16          0x01
#define SZ_32X32          0x02
#define SZ_64X64          0x03

// R#10(spas)=0
#define SZ_NOREVERSE      0x00
#define SZ_VERTREVERSE    0x01
#define SZ_HORIZREVERSE   0x02
#define SZ_BOTHREVERSE    0x03

// R#11(prm)
#define PRM_SABDEX        0x00
#define PRM_ASBDEX        0x04
#define PRM_SEABDX        0x08
#define PRM_ASEBDX        0x0c

// R#40
#define HDW_SHIFT         0
#define HDW_MASK          0x3f

// R#44
#define VDW_SHIFT         0
#define VDW_MASK          0x3f

typedef struct {
	UINT8 sy;		// y dot position 7:0
	UINT8 sx;		// x dot position 7:0
	UINT8 attr;		// 0xf0 = color, 0x0c = size, reverse, 0x02 = x hi bit, 0x01 = y hi bit
	UINT8 sn;    // pattern name (0-255)
} SPRITE_ATTR, *PSPRITE_ATTR;


typedef struct _ygv608 {

  union {
    UINT8		b[8];
    YGV_PORTS	s;
  } ports;

  union {
    UINT8		b[50];
    YGV_REGS	s;
  } regs;

  /*
   *  Built in ram
   */

  UINT8 pattern_name_table[4096];

  union {
    UINT8			b[YGV608_SPRITE_ATTR_TABLE_SIZE];
    SPRITE_ATTR		s[YGV608_MAX_SPRITES];
  } sprite_attribute_table;

  UINT8 scroll_data_table[2][256];
  UINT8 colour_palette[256][3];

  /*
   *  Shortcut variables
   */

  UINT32 bits16;          // bits per pattern (8/16)
  UINT32 page_x, page_y;  // pattern page size
  UINT32 pny_shift;       // y coord multiplier
  UINT8 na8_mask;       // mask on/off na11/9:8
  int col_shift;                // shift in scroll table column index

  // rotation, zoom shortcuts
  UINT32 ax, dx, dxy, ay, dy, dyx;

  // base address shortcuts
  UINT32 base_addr[2][8];
  UINT32 base_y_shift;    // for extracting pattern y coord 'base'

  UINT8 screen_resize;  // screen requires resize
  UINT8 tilemap_resize; // tilemap requires resize

} YGV608, *pYGV608;


INTERRUPT_GEN( ygv608_timed_interrupt );
VIDEO_START( ygv608 );
VIDEO_UPDATE( ygv608 );

READ16_HANDLER( ygv608_r );
WRITE16_HANDLER( ygv608_w );

// to be removed
extern READ16_HANDLER( ygv608_debug_trigger );

#endif
