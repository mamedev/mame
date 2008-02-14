/***************************************************************************

    Atari Jaguar blitter

****************************************************************************

    ------------------------------------------------------------
    BLITTER REGISTERS
    ------------------------------------------------------------
    F02200-F022FF   R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Blitter registers
    F02200            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_BASE - A1 base register
    F02204            W   -------- ---xxxxx -xxxxxxx xxxxx-xx   A1_FLAGS - A1 flags register
                      W   -------- ---x---- -------- --------      (YSIGNSUB - invert sign of Y delta)
                      W   -------- ----x--- -------- --------      (XSIGNSUB - invert sign of X delta)
                      W   -------- -----x-- -------- --------      (Y add control)
                      W   -------- ------xx -------- --------      (X add control)
                      W   -------- -------- -xxxxxx- --------      (width in 6-bit floating point)
                      W   -------- -------- -------x xx------      (ZOFFS1-6 - Z data offset)
                      W   -------- -------- -------- --xxx---      (PIXEL - pixel size)
                      W   -------- -------- -------- ------xx      (PITCH1-4 - data phrase pitch)
    F02208            W   -xxxxxxx xxxxxxxx -xxxxxxx xxxxxxxx   A1_CLIP - A1 clipping size
                      W   -xxxxxxx xxxxxxxx -------- --------      (height)
                      W   -------- -------- -xxxxxxx xxxxxxxx      (width)
    F0220C          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_PIXEL - A1 pixel pointer
                    R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel value)
                    R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel value)
    F02210            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_STEP - A1 step value
                      W   xxxxxxxx xxxxxxxx -------- --------      (Y step value)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (X step value)
    F02214            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FSTEP - A1 step fraction value
                      W   xxxxxxxx xxxxxxxx -------- --------      (Y step fraction value)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (X step fraction value)
    F02218          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FPIXEL - A1 pixel pointer fraction
                    R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel fraction value)
                    R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel fraction value)
    F0221C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_INC - A1 increment
                      W   xxxxxxxx xxxxxxxx -------- --------      (Y increment)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (X increment)
    F02220            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FINC - A1 increment fraction
                      W   xxxxxxxx xxxxxxxx -------- --------      (Y increment fraction)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (X increment fraction)
    F02224            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_BASE - A2 base register
    F02228            W   -------- ---xxxxx -xxxxxxx xxxxx-xx   A2_FLAGS - A2 flags register
                      W   -------- ---x---- -------- --------      (YSIGNSUB - invert sign of Y delta)
                      W   -------- ----x--- -------- --------      (XSIGNSUB - invert sign of X delta)
                      W   -------- -----x-- -------- --------      (Y add control)
                      W   -------- ------xx -------- --------      (X add control)
                      W   -------- -------- -xxxxxx- --------      (width in 6-bit floating point)
                      W   -------- -------- -------x xx------      (ZOFFS1-6 - Z data offset)
                      W   -------- -------- -------- --xxx---      (PIXEL - pixel size)
                      W   -------- -------- -------- ------xx      (PITCH1-4 - data phrase pitch)
    F0222C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_MASK - A2 window mask
    F02230          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_PIXEL - A2 pixel pointer
                    R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel value)
                    R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel value)
    F02234            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_STEP - A2 step value
                      W   xxxxxxxx xxxxxxxx -------- --------      (Y step value)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (X step value)
    F02238            W   -xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_CMD - command register
                      W   -x------ -------- -------- --------      (SRCSHADE - modify source intensity)
                      W   --x----- -------- -------- --------      (BUSHI - hi priority bus)
                      W   ---x---- -------- -------- --------      (BKGWREN - writeback destination)
                      W   ----x--- -------- -------- --------      (DCOMPEN - write inhibit from data comparator)
                      W   -----x-- -------- -------- --------      (BCOMPEN - write inhibit from bit coparator)
                      W   ------x- -------- -------- --------      (CMPDST - compare dest instead of src)
                      W   -------x xxx----- -------- --------      (logical operation)
                      W   -------- ---xxx-- -------- --------      (ZMODE - Z comparator mode)
                      W   -------- ------x- -------- --------      (ADDDSEL - select sum of src & dst)
                      W   -------- -------x -------- --------      (PATDSEL - select pattern data)
                      W   -------- -------- x------- --------      (TOPNEN - enable carry into top intensity nibble)
                      W   -------- -------- -x------ --------      (TOPBEN - enable carry into top intensity byte)
                      W   -------- -------- --x----- --------      (ZBUFF - enable Z updates in inner loop)
                      W   -------- -------- ---x---- --------      (GOURD - enable gouraud shading in inner loop)
                      W   -------- -------- ----x--- --------      (DSTA2 - reverses A2/A1 roles)
                      W   -------- -------- -----x-- --------      (UPDA2 - add A2 step to A2 in outer loop)
                      W   -------- -------- ------x- --------      (UPDA1 - add A1 step to A1 in outer loop)
                      W   -------- -------- -------x --------      (UPDA1F - add A1 fraction step to A1 in outer loop)
                      W   -------- -------- -------- x-------      (diagnostic use)
                      W   -------- -------- -------- -x------      (CLIP_A1 - clip A1 to window)
                      W   -------- -------- -------- --x-----      (DSTWRZ - enable dest Z write in inner loop)
                      W   -------- -------- -------- ---x----      (DSTENZ - enable dest Z read in inner loop)
                      W   -------- -------- -------- ----x---      (DSTEN - enables dest data read in inner loop)
                      W   -------- -------- -------- -----x--      (SRCENX - enable extra src read at start of inner)
                      W   -------- -------- -------- ------x-      (SRCENZ - enables source Z read in inner loop)
                      W   -------- -------- -------- -------x      (SRCEN - enables source data read in inner loop)
    F02238          R     xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_CMD - status register
                    R     xxxxxxxx xxxxxxxx -------- --------      (inner count)
                    R     -------- -------- xxxxxxxx xxxxxx--      (diagnostics)
                    R     -------- -------- -------- ------x-      (STOPPED - when stopped in collision detect)
                    R     -------- -------- -------- -------x      (IDLE - when idle)
    F0223C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_COUNT - counters register
                      W   xxxxxxxx xxxxxxxx -------- --------      (outer loop count)
                      W   -------- -------- xxxxxxxx xxxxxxxx      (inner loop count)
    F02240-F02247     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCD - source data register
    F02248-F0224F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_DSTD - destination data register
    F02250-F02257     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_DSTZ - destination Z register
    F02258-F0225F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCZ1 - source Z register 1
    F02260-F02267     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCZ2 - source Z register 2
    F02268-F0226F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_PATD - pattern data register
    F02270            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_IINC - intensity increment
    F02274            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_ZINC - Z increment
    F02278            W   -------- -------- -------- -----xxx   B_STOP - collision control
                      W   -------- -------- -------- -----x--      (STOPEN - enable blitter collision stops)
                      W   -------- -------- -------- ------x-      (ABORT - abort after stop)
                      W   -------- -------- -------- -------x      (RESUME - resume after stop)
    F0227C            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I3 - intensity 3
    F02280            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I2 - intensity 2
    F02284            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I1 - intensity 1
    F02288            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I0 - intensity 0
    F0228C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z3 - Z3
    F02290            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z2 - Z2
    F02294            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z1 - Z1
    F02298            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z0 - Z0

****************************************************************************/

/* mask for flags that are always handled dynamically or ignored */
#define DYNAMIC_COMMAND_MASK	0x00000704
#define DYNAMIC_FLAGS_MASK		0xffe0fe04

#define STATIC_COMMAND_MASK		(~DYNAMIC_COMMAND_MASK)
#define STATIC_FLAGS_MASK		(~DYNAMIC_FLAGS_MASK)


/* blitter registers */
enum
{
	A1_BASE,	A1_FLAGS,	A1_CLIP,	A1_PIXEL,
	A1_STEP,	A1_FSTEP,	A1_FPIXEL,	A1_INC,
	A1_FINC,	A2_BASE,	A2_FLAGS,	A2_MASK,
	A2_PIXEL,	A2_STEP,	B_CMD,		B_COUNT,
	B_SRCD_H,	B_SRCD_L,	B_DSTD_H,	B_DSTD_L,
	B_DSTZ_H,	B_DSTZ_L,	B_SRCZ1_H,	B_SRCZ1_L,
	B_SRCZ2_H,	B_SRCZ2_L,	B_PATD_H,	B_PATD_L,
	B_IINC,		B_ZINC,		B_STOP,		B_I3,
	B_I2,		B_I1,		B_I0,		B_Z3,
	B_Z2,		B_Z1,		B_Z0,
	BLITTER_REGS
};
