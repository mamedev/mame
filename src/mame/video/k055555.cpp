// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************/
/*                                                                         */
/*                                 055555                                  */
/*  Mixer Device / Priority encoder                                        */
/*                                                                         */
/***************************************************************************/

/*


K055555
-------

Priority encoder.  Always found in conjunction with k054338, but the reverse
isn't true.  The 55555 has 8 inputs: "A", "B", "C", and "D" intended for a 156/157
type tilemap chip, "OBJ" intended for a '246 type sprite chip, and "SUB1-SUB3"
which can be used for 3 additional layers.

When used in combintion with a k054338, each input can be chosen to participate
in shadow/highlight operations, R/G/B alpha blending, and R/G/B brightness control.
Per-tile priority is supported for tilemap planes A and B.

There are also 3 shadow priority registers.  When these are enabled, layers and
sprites with a priority greater than or equal to them become a shadow, and either
then gets drawn as a shadow/highlight or not at all (I'm not sure what selects
this yet.  Dadandarn relies on this mechanism to hide the 53936 plane when
it doesn't want it visible).

It also appears that brightness control and alpha blend can be decided per-tile
and per-sprite, although this is not certain.

Additionally the 55555 can provide a gradient background with one palette entry
per scanline.  This is fairly rarely used, but does turn up in Gokujou Parodius as
well as the Sexy Parodius title screen.

Lots of byte-wise registers.  A partial map:

0: Palette index(?) for the gradient background
1: related to tilemap brightness control
2-5: COLSEL for various inputs (?)
6: COLCHG ON
7-18: priority levels (VA1/VA2/VAC/VB1/VB2/VBC/VC/VD/OBJ/S1/S2/S3)
19-22: INPRI for OBJ/S1/S2/S3
23-32: palette bases (VA/VB/VC/VD/OBJ/S1/S2/S3)
37: shadow 1 priority
38: shadow 2 priority
39: shadow 3 priority
40: shadow/highlight master enable
41: master shadow/highlight priority
42: VBRI: enables brightness control for each VRAM layer (bits: x x x x D B C A)
43: OSBRI: enables brightness control for OBJ and SUB layers, depending for OBJ on attr bits
44: OSBRI_ON: not quite sure
45: input enables.  bits as follows: (MSB) S3 S2 S1 OB VD VC VB VA (LSB)


*/



#include "emu.h"
#include "k055555.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* K055555 5-bit-per-pixel priority encoder */
/* This device has 48 8-bit-wide registers */

void k055555_device::K055555_write_reg(UINT8 regnum, UINT8 regdat)
{
	static const char *const rnames[46] =
	{
		"BGC CBLK", "BGC SET", "COLSET0", "COLSET1", "COLSET2", "COLSET3", "COLCHG ON",
		"A PRI 0", "A PRI 1", "A COLPRI", "B PRI 0", "B PRI 1", "B COLPRI", "C PRI", "D PRI",
		"OBJ PRI", "SUB1 PRI", "SUB2 PRI", "SUB3 PRI", "OBJ INPRI ON", "S1 INPRI ON", "S2 INPRI ON",
		"S3 INPRI ON", "A PAL", "B PAL", "C PAL", "D PAL", "OBJ PAL", "SUB1 PAL", "SUB2 PAL", "SUB3 PAL",
		"SUB2 PAL ON", "SUB3 PAL ON", "V INMIX", "V INMIX ON", "OS INMIX", "OS INMIX ON", "SHD PRI 1",
		"SHD PRI 2", "SHD PRI 3", "SHD ON", "SHD PRI SEL", "V BRI", "OS INBRI", "OS INBRI ON", "ENABLE"
	};

	if (regdat != m_regs[regnum])
	{
		LOG(("5^5: %x to reg %x (%s)\n", regdat, regnum, rnames[regnum]));
	}

	m_regs[regnum] = regdat;
}

WRITE32_MEMBER( k055555_device::K055555_long_w )
{
	UINT8 regnum, regdat;

	// le2 accesses this area with a dword write ...
	if (ACCESSING_BITS_24_31)
	{
		regnum = offset<<1;
		regdat = data>>24;
		K055555_write_reg(regnum, regdat);
	}

	if (ACCESSING_BITS_8_15)
	{
		regnum = (offset<<1)+1;
		regdat = data>>8;
		K055555_write_reg(regnum, regdat);
	}

}

WRITE16_MEMBER( k055555_device::K055555_word_w )
{
	if (mem_mask == 0x00ff)
	{
		K055555_write_reg(offset, data&0xff);
	}
	else
	{
		K055555_write_reg(offset, data>>8);
	}
}

int k055555_device::K055555_read_register(int regnum)
{
	return(m_regs[regnum]);
}

int k055555_device::K055555_get_palette_index(int idx)
{
	return(m_regs[K55_PALBASE_A + idx]);
}



/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/



const device_type K055555 = &device_creator<k055555_device>;

k055555_device::k055555_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K055555, "K055555 Priority Encoder", tag, owner, clock, "k055555", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k055555_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k055555_device::device_start()
{
	save_item(NAME(m_regs));

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k055555_device::device_reset()
{
	memset(m_regs, 0, 64 * sizeof(UINT8));
}
