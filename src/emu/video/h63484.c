/***************************************************************************

    HD63484 ACRTC (rewrite in progress)

    TODO:
    - 8-bit support for FIFO, parameters and command values
    - execution cycles;

***************************************************************************/

#include "emu.h"
#include "h63484.h"

#define LOG 1
#define FIFO_LOG 0

// default address map
static ADDRESS_MAP_START( h63484_vram, AS_0, 8, h63484_device )
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0x80000, 0xfffff) AM_NOP
ADDRESS_MAP_END


//-------------------------------------------------
//  h63484_device - constructor
//-------------------------------------------------

h63484_device::h63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, H63484, "H63484", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	m_ar(0),
	m_sr(0),
	m_fifo_ptr(-1),
	m_fifo_r_ptr(-1),
	m_cr(0),
	m_param_ptr(0),
	m_rwp_dn(0),
	m_org_dpa(0),
	m_org_dn(0),
	m_org_dpd(0),
	m_cl0(0),
	m_cl1(0),
	m_dcr(0),
	  m_space_config("videoram", ENDIANNESS_LITTLE, 8, 20, 0, NULL, *ADDRESS_MAP_NAME(h63484_vram))
{
	m_shortname = "h63484";
}

enum
{
	FIFO_READ = 0,
	FIFO_WRITE
};

enum
{
	FIFO_EMPTY = -1,
	FIFO_PARAMETER,
	FIFO_COMMAND
};

#define H63484_SR_CER 0x80 // Command Error
#define H63484_SR_ARD 0x40 // Area Detect
#define H63484_SR_CED 0x20 // Command End
#define H63484_SR_LPD 0x10 // Light Pen Strobe Detect
#define H63484_SR_RFF 0x08 // Read FIFO Full
#define H63484_SR_RFR 0x04 // Read FIFO Ready
#define H63484_SR_WFR 0x02 // Write FIFO Ready
#define H63484_SR_WFE 0x01 // Write FIFO Empty


static const char *const acrtc_regnames[0x100/2] =
{
	"FIFO Entry", // 0x00
	"Command Control (CCR)", // 0x02
	"Operation Mode (OMR)", // 0x04
	"Display Control (DCR)", // 0x06
	"(Undefined)", // 0x08
	"(Undefined)", // 0x0a
	"(Undefined)", // 0x0c
	"(Undefined)", // 0x0e
	"(Undefined)", // 0x10
	"(Undefined)", // 0x12
	"(Undefined)", // 0x14
	"(Undefined)", // 0x16
	"(Undefined)", // 0x18
	"(Undefined)", // 0x1a
	"(Undefined)", // 0x1c
	"(Undefined)", // 0x1e
	"(Undefined)", // 0x20
	"(Undefined)", // 0x22
	"(Undefined)", // 0x24
	"(Undefined)", // 0x26
	"(Undefined)", // 0x28
	"(Undefined)", // 0x2a
	"(Undefined)", // 0x2c
	"(Undefined)", // 0x2e
	"(Undefined)", // 0x30
	"(Undefined)", // 0x32
	"(Undefined)", // 0x34
	"(Undefined)", // 0x36
	"(Undefined)", // 0x38
	"(Undefined)", // 0x3a
	"(Undefined)", // 0x3c
	"(Undefined)", // 0x3e
	"(Undefined)", // 0x40
	"(Undefined)", // 0x42
	"(Undefined)", // 0x44
	"(Undefined)", // 0x46
	"(Undefined)", // 0x48
	"(Undefined)", // 0x4a
	"(Undefined)", // 0x4c
	"(Undefined)", // 0x4e
	"(Undefined)", // 0x50
	"(Undefined)", // 0x52
	"(Undefined)", // 0x54
	"(Undefined)", // 0x56
	"(Undefined)", // 0x58
	"(Undefined)", // 0x5a
	"(Undefined)", // 0x5c
	"(Undefined)", // 0x5e
	"(Undefined)", // 0x60
	"(Undefined)", // 0x62
	"(Undefined)", // 0x64
	"(Undefined)", // 0x66
	"(Undefined)", // 0x68
	"(Undefined)", // 0x6a
	"(Undefined)", // 0x6c
	"(Undefined)", // 0x6e
	"(Undefined)", // 0x70
	"(Undefined)", // 0x72
	"(Undefined)", // 0x74
	"(Undefined)", // 0x76
	"(Undefined)", // 0x78
	"(Undefined)", // 0x7a
	"(Undefined)", // 0x7c
	"(Undefined)", // 0x7e
	"Raster Count (RCR)", // 0x80
	"Horizontal Sync (HSR)", // 0x82
	"Horizontal Display (HDR)", // 0x84
	"Vertical Sync (VSR)", // 0x86
	"Vertical Display (VDR)", // 0x88
	"Split Screen Width (SSW) 0x8a", // 0x8a
	"Split Screen Width (SSW) 0x8c", // 0x8c
	"Split Screen Width (SSW) 0x8e", // 0x8e
	"Blink Control (BCR)", // 0x90
	"Horizontal Window Display (HWR)", // 0x92
	"Vertical Window Display (VWR) 0x94", // 0x94
	"Vertical Window Display (VWR) 0x96", // 0x96
	"Graphic Cursor (GCR0)", // 0x98
	"Graphic Cursor (GCR1)", // 0x9a
	"Graphic Cursor (GCR2)", // 0x9c
	"(Undefined)", // 0x9e
	"(Undefined)", // 0xa0
	"(Undefined)", // 0xa2
	"(Undefined)", // 0xa4
	"(Undefined)", // 0xa6
	"(Undefined)", // 0xa8
	"(Undefined)", // 0xaa
	"(Undefined)", // 0xac
	"(Undefined)", // 0xae
	"(Undefined)", // 0xb0
	"(Undefined)", // 0xb2
	"(Undefined)", // 0xb4
	"(Undefined)", // 0xb6
	"(Undefined)", // 0xb8
	"(Undefined)", // 0xba
	"(Undefined)", // 0xbc
	"(Undefined)", // 0xbe
	// upper screen
	"Raster Address 0 (RAR0)", // 0xc0
	"Memory Width 0 (MWR0)", // 0xc2
	"Start Address 0 (SAR0) 0xc4", // 0xc4
	"Start Address 0 (SAR0) 0xc6", // 0xc6
	// base screen
	"Raster Address 1 (RAR1)", // 0xc8
	"Memory Width 1 (MWR1)", // 0xca
	"Start Address 1 (SAR1) 0xcc", // 0xcc
	"Start Address 1 (SAR1) 0xce", // 0xce
	// lower screen
	"Raster Address 2 (RAR2)", // 0xd0
	"Memory Width 2 (MWR2)", // 0xd2
	"Start Address 2 (SAR2) 0xd4", // 0xd4
	"Start Address 2 (SAR2) 0xd6", // 0xd6
	// window screen
	"Raster Address 3 (RAR3)", // 0xd8
	"Memory Width 3 (MWR3)", // 0xda
	"Start Address 3 (SAR3) 0xdc", // 0xdc
	"Start Address 3 (SAR3) 0xde", // 0xde
	// block cursor 1
	"Block Cursor 1 (BCUR1) 0xe0",
	"Block Cursor 1 (BCUR1) 0xe2",
	// block cursor 2
	"Block Cursor 2 (BCUR2) 0xe4",
	"Block Cursor 2 (BCUR2) 0xe6",
	"Cursor Definition (CDR)",
	"Zoom Factor (ZFR)",
	"Lightpen Address (LPAR) 0xec",
	"Lightpen Address (LPAR) 0xee",
	"(Undefined)", // 0xf0
	"(Undefined)", // 0xf2
	"(Undefined)", // 0xf4
	"(Undefined)", // 0xf6
	"(Undefined)", // 0xf8
	"(Undefined)", // 0xfa
	"(Undefined)", // 0xfc
	"(Undefined)"  // 0xfe
};

static const char *const wpr_regnames[0x20] =
{
	"Color 0 (CL0)",
	"Color 1 (CL1)",
	"Color Comparison (COMP)",
	"Edge Color (EDG)",
	"Mask (MASK)",
	"Pattern RAM Control (PRC) 0x05",
	"Pattern RAM Control (PRC) 0x06",
	"Pattern RAM Control (PRC) 0x07",
	"Area Definition (ADR) 0x08",
	"Area Definition (ADR) 0x09",
	"Area Definition (ADR) 0x0a",
	"Area Definition (ADR) 0x0b",
	"Read Write Pointer (RWP) H",
	"Read Write Pointer (RWP) L",
	"(Undefined)",
	"(Undefined)",
	"Drawing Pointer (DP) H",
	"Drawing Pointer (DP) L",
	"Current Pointer (CP) H",
	"Current Pointer (CP) L",
	"(Undefined)", // 0x14
	"(Undefined)",
	"(Undefined)", // 0x16
	"(Undefined)",
	"(Undefined)", // 0x18
	"(Undefined)",
	"(Undefined)", // 0x1a
	"(Undefined)",
	"(Undefined)", // 0x1c
	"(Undefined)",
	"(Undefined)", // 0x1e
	"(Undefined)"
};

enum
{
	COMMAND_INVALID = -1,
	COMMAND_ORG,
	COMMAND_WPR,
	COMMAND_RPR,
	COMMAND_WPTN,
	COMMAND_RPTN,
	COMMAND_DRD,
	COMMAND_DWT,
	COMMAND_DMOD,
	COMMAND_RD,
	COMMAND_WT,
	COMMAND_MOD,
	COMMAND_CLR,
	COMMAND_SCLR,
	COMMAND_CPY,
	COMMAND_SCPY,
	COMMAND_AMOVE,
	COMMAND_RMOVE,
	COMMAND_ALINE,
	COMMAND_RLINE,
	COMMAND_ARCT,
	COMMAND_RRCT,
	COMMAND_APLL,
	COMMAND_RPLL,
	COMMAND_APLG,
	COMMAND_RPLG,
	COMMAND_CRCL,
	COMMAND_ELPS,
	COMMAND_AARC,
	COMMAND_RARC,
	COMMAND_AEARC,
	COMMAND_REARC,
	COMMAND_AFRCT,
	COMMAND_RFRCT,
	COMMAND_PAINT,
	COMMAND_DOT,
	COMMAND_PTN,
	COMMAND_AGCPY,
	COMMAND_RGCPY
};

#define H63484_COMMAND_ORG		0x0400	//              p: 2
#define H63484_COMMAND_WPR		0x0800	// & ~0x1f      p: 1
#define H63484_COMMAND_RPR		0x0c00	// & ~0x1f      p: 0
#define H63484_COMMAND_WPTN		0x1800	// & ~0xf       p: 1 + n
#define H63484_COMMAND_RPTN		0x1c00	// & ~0xf       p: 1
#define H63484_COMMAND_DRD		0x2400	//              p: 2
#define H63484_COMMAND_DWT		0x2800	//              p: 2
#define H63484_COMMAND_DMOD		0x2c00	// & ~3         p: 2
#define H63484_COMMAND_RD		0x4400	//              p: 0
#define H63484_COMMAND_WT		0x4800	//              p: 1
#define H63484_COMMAND_MOD		0x4c00	// & ~3         p: 1
#define H63484_COMMAND_CLR		0x5800	//              p: 3
#define H63484_COMMAND_SCLR		0x5c00	// & ~3         p: 3
#define H63484_COMMAND_CPY		0x6000	// & ~0x0f03    p: 4
#define H63484_COMMAND_SCPY		0x7000	// & ~0x0f03    p: 4
#define H63484_COMMAND_AMOVE	0x8000	//              p: 2
#define H63484_COMMAND_RMOVE	0x8400	//              p: 2
#define H63484_COMMAND_ALINE	0x8800	// & ~0x00ff    p: 2
#define H63484_COMMAND_RLINE	0x8c00	// & ~0x00ff    p: 2
#define H63484_COMMAND_ARCT		0x9000	// & ~0x00ff    p: 2
#define H63484_COMMAND_RRCT		0x9400	// & ~0x00ff    p: 2
#define H63484_COMMAND_APLL		0x9800	// & ~0x00ff    p: 1 + n
#define H63484_COMMAND_RPLL		0x9c00	// & ~0x00ff    p: 1 + n
#define H63484_COMMAND_APLG		0xa000	// & ~0x00ff    p: 1 + n
#define H63484_COMMAND_RPLG		0xa400	// & ~0x00ff    p: 1 + n
#define H63484_COMMAND_CRCL		0xa800	// & ~0x01ff    p: 1
#define H63484_COMMAND_ELPS		0xac00	// & ~0x01ff    p: 3
#define H63484_COMMAND_AARC		0xb000	// & ~0x01ff    p: 4
#define H63484_COMMAND_RARC		0xb400	// & ~0x01ff    p: 4
#define H63484_COMMAND_AEARC	0xb800	// & ~0x01ff    p: 6
#define H63484_COMMAND_REARC	0xbc00	// & ~0x01ff    p: 6
#define H63484_COMMAND_AFRCT	0xc000	// & ~0x00ff    p: 2
#define H63484_COMMAND_RFRCT	0xc400	// & ~0x00ff    p: 2
#define H63484_COMMAND_PAINT	0xc800	// & ~0x01ff    p: 0
#define H63484_COMMAND_DOT		0xcc00	// & ~0x00ff    p: 0
#define H63484_COMMAND_PTN		0xd000	// & ~0x0fff    p: 1
#define H63484_COMMAND_AGCPY	0xe000	// & ~0x0fff    p: 4
#define H63484_COMMAND_RGCPY	0xf000	// & ~0x0fff    p: 4


/*-------------------------------------------------
    ROM( h63484 )
-------------------------------------------------*/

// devices
const device_type H63484 = &device_creator<h63484_device>;


ROM_START( h63484 )
	ROM_REGION( 0x100, "h63484", 0 )
	ROM_LOAD( "h63484.bin", 0x000, 0x100, NO_DUMP ) /* internal control ROM */
ROM_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *h63484_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *h63484_device::device_rom_region() const
{
	return ROM_NAME( h63484 );
}

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 h63484_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void h63484_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void h63484_device::device_config_complete()
{
	// inherit a copy of the static data
	const h63484_interface *intf = reinterpret_cast<const h63484_interface *>(static_config());
	if (intf != NULL)
		*static_cast<h63484_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		// ...
	}
}

inline void h63484_device::fifo_w_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo[i] = 0;

	m_fifo_ptr = -1;

	m_sr |= H63484_SR_WFR;
	m_sr |= H63484_SR_WFE;
}

inline void h63484_device::queue_w(UINT8 data)
{
	if (m_fifo_ptr < 15)
	{
		m_fifo_ptr++;

		m_fifo[m_fifo_ptr] = data;

		if (m_fifo_ptr == 16)
			m_sr &= ~H63484_SR_WFR;

		m_sr &= ~H63484_SR_WFE;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

inline void h63484_device::dequeue_w(UINT8 *data)
{
	int i;

	*data = m_fifo[0];

	if (m_fifo_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo[i] = m_fifo[i + 1];

		m_fifo[15] = 0;

		m_fifo_ptr--;

		m_sr |= H63484_SR_WFR;

		if (m_fifo_ptr == -1)
			m_sr |= H63484_SR_WFE;

	}
}

inline void h63484_device::fifo_r_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo_r[i] = 0;

	m_fifo_r_ptr = -1;

	m_sr &= ~H63484_SR_RFR;
	m_sr &= ~H63484_SR_RFF;
}

inline void h63484_device::queue_r(UINT8 data)
{
	if (m_fifo_r_ptr < 15)
	{
		m_fifo_r_ptr++;

		m_fifo_r[m_fifo_r_ptr] = data;

		if (m_fifo_r_ptr == 16)
			m_sr |= H63484_SR_RFF;

		m_sr |= H63484_SR_RFR;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

inline void h63484_device::dequeue_r(UINT8 *data)
{
	int i;

	*data = m_fifo_r[0];

	if (m_fifo_r_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo_r[i] = m_fifo_r[i + 1];

		m_fifo_r[15] = 0;

		m_fifo_r_ptr--;

		m_sr &= ~H63484_SR_RFF;

		if (m_fifo_r_ptr == -1)
			m_sr &= ~H63484_SR_RFR;
	}
}

//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

inline void h63484_device::recompute_parameters()
{
	if(m_hc == 0 || m_vc == 0) //bail out if screen params aren't valid
		return;

	printf("HC %d HSW %d HDS %d HDW %d HWS %d HWW %d\n",m_hc,m_hsw,m_hds,m_hdw,m_hws,m_hww);
	printf("VC %d VDS %d VSW %d VWS %d VWW %d\n",m_vc,m_vds,m_vsw,m_vws,m_vww);
}

/*-------------------------------------------------
    ADDRESS_MAP( h63484 )
-------------------------------------------------*/

READ8_MEMBER( h63484_device::vram_r )
{
	return m_vram[offset];
}

WRITE8_MEMBER( h63484_device::vram_w )
{
	m_vram[offset] = data;
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

int h63484_device::translate_command(UINT16 data)
{
	/* annoying switch-case sequence, but it's the only way to get invalid commands ... */
	switch (data)
	{
		case H63484_COMMAND_ORG:	return COMMAND_ORG;
		case H63484_COMMAND_DRD:	return COMMAND_DRD;
		case H63484_COMMAND_DWT:	return COMMAND_DWT;
		case H63484_COMMAND_RD: 	return COMMAND_RD;
		case H63484_COMMAND_WT: 	return COMMAND_WT;
		case H63484_COMMAND_CLR:	return COMMAND_CLR;
		case H63484_COMMAND_AMOVE:	return COMMAND_AMOVE;
		case H63484_COMMAND_RMOVE:	return COMMAND_RMOVE;
	}

	switch(data & ~0x3)
	{
		case H63484_COMMAND_DMOD:	return COMMAND_DMOD;
		case H63484_COMMAND_MOD:	return COMMAND_MOD;
		case H63484_COMMAND_SCLR:	return COMMAND_SCLR;
	}

	switch(data & ~0xf)
	{
		case H63484_COMMAND_WPTN:	return COMMAND_WPTN;
		case H63484_COMMAND_RPTN:	return COMMAND_RPTN;
	}

	switch(data & ~0x1f)
	{
		case H63484_COMMAND_WPR:	return COMMAND_WPR;
		case H63484_COMMAND_RPR:	return COMMAND_RPR;
	}

	switch(data & ~0x0f03)
	{
		case H63484_COMMAND_CPY:	return COMMAND_CPY;
		case H63484_COMMAND_SCPY:	return COMMAND_SCPY;
	}

	switch(data & ~0x00ff)
	{
		case H63484_COMMAND_ALINE:	return COMMAND_ALINE;
		case H63484_COMMAND_RLINE:	return COMMAND_RLINE;
		case H63484_COMMAND_ARCT:	return COMMAND_ARCT;
		case H63484_COMMAND_RRCT:	return COMMAND_RRCT;
		case H63484_COMMAND_APLL:	return COMMAND_APLL;
		case H63484_COMMAND_RPLL:	return COMMAND_RPLL;
		case H63484_COMMAND_APLG:	return COMMAND_APLG;
		case H63484_COMMAND_RPLG:	return COMMAND_RPLG;
		case H63484_COMMAND_AFRCT:	return COMMAND_AFRCT;
		case H63484_COMMAND_RFRCT:	return COMMAND_RFRCT;
		case H63484_COMMAND_DOT:	return COMMAND_DOT;
	}

	switch(data & ~0x01ff)
	{
		case H63484_COMMAND_CRCL:	return COMMAND_CRCL;
		case H63484_COMMAND_ELPS:	return COMMAND_ELPS;
		case H63484_COMMAND_AARC:	return COMMAND_AARC;
		case H63484_COMMAND_RARC:	return COMMAND_RARC;
		case H63484_COMMAND_AEARC:	return COMMAND_AEARC;
		case H63484_COMMAND_REARC:	return COMMAND_REARC;
		case H63484_COMMAND_PAINT:	return COMMAND_PAINT;
	}

	switch(data & ~0x0fff)
	{
		case H63484_COMMAND_PTN:	return COMMAND_PTN;
		case H63484_COMMAND_AGCPY:	return COMMAND_AGCPY;
		case H63484_COMMAND_RGCPY:	return COMMAND_RGCPY;
	}

	return COMMAND_INVALID;
}

inline void h63484_device::command_end_seq()
{
	//h63484->param_ptr = 0;
	m_sr |= H63484_SR_CED;

	/* TODO: we might need to be more aggressive and clear the params in there */
}

void h63484_device::command_wpr_exec()
{
	switch(m_cr & 0x1f)
	{
		case 0x00: // color 0
			m_cl0 = m_pr[0];
			break;
		case 0x01: // color 1
			m_cl1 = m_pr[0];
			break;
		case 0x0c: // Read Write Pointer H
			m_rwp_dn = (m_pr[0] & 0xc000) >> 14;
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0x00fff) | ((m_pr[0] & 0x00ff) << 12);
			break;
		case 0x0d: // Read Write Pointer L
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0xff000) | ((m_pr[0] & 0xfff0) >> 4);
			break;
		default:
			if(LOG) printf("%s -> %02x\n",wpr_regnames[m_cr & 0x1f],m_pr[0]);
			break;
	}
}

void h63484_device::command_clr_exec()
{
	INT16 ax, ay;
	UINT16 d;
	INT16 x,y;
	int inc_x,inc_y;
	UINT32 offset;
	UINT8 data_r;
	UINT8 res;

	d = m_pr[0];
	ax = m_pr[1];
	ay = m_pr[2];

	inc_x = (ax < 0) ? -1 : 1;
	inc_y = (ay < 0) ? -1 : 1;

	offset = m_rwp[m_rwp_dn] & 0xfffff;

	for(y=0;y!=ay;y+=inc_y)
	{
		for(x=0;x!=ax;x+=inc_x)
		{
			offset = (m_rwp[m_rwp_dn] + (x >> 1) + y * m_mwr[m_rwp_dn]) & 0xfffff; /* TODO: address if bpp != 4 */
			data_r = readbyte(offset);
			res = (d >> (((x & 2) << 2) ^ 8)) & 0xff;

			switch(m_cr & 3)
			{
				case 0: // replace
					if(x & 1)
						res = (res & 0x0f) | (data_r & 0xf0);
					else
						res = (res & 0xf0) | (data_r & 0x0f);
					break;
				case 1: // OR
					if(x & 1)
						res = (res & 0xff) | (data_r & 0xf0);
					else
						res = (res & 0xff) | (data_r & 0x0f);
					break;
				case 2: // AND
					if(x & 1)
						res = (res & 0x0f) | ((res & 0xf0) & (data_r & 0xf0));
					else
						res = (res & 0xf0) | ((res & 0x0f) & (data_r & 0x0f));
					break;
				case 3: // EOR
					if(x & 1)
						res = (res & 0x0f) | ((res & 0xf0) ^ (data_r & 0xf0));
					else
						res = (res & 0xf0) | ((res & 0x0f) ^ (data_r & 0x0f));
					break;
			}

			writebyte(offset,res);
		}
	}

	m_rwp[m_rwp_dn] = offset;
}

void h63484_device::command_cpy_exec()
{
	INT16 ax, ay;
	UINT32 src_offset,dst_offset;
	//int inc_x,inc_y;

	src_offset = ((m_pr[0] & 0xff) << 12) | ((m_pr[1]&0xfff0) >> 4);
	ax = m_pr[2];
	ay = m_pr[3];

	//inc_x = (ax < 0) ? -1 : 1;
	//inc_y = (ay < 0) ? -1 : 1;

	dst_offset = m_rwp[m_rwp_dn] & 0xfffff;

	printf("%08x %08x %d %d\n",src_offset,dst_offset,ax,ay);

	// ...
}

void h63484_device::command_rct_exec()
{
	INT16 dX, dY;
	UINT8 col; //, area, opm;
	UINT32 offset;
	int inc_x,inc_y;
	int i;
	UINT8 res,data_r;

//  area = (m_cr & 0xe0) >> 5;
	col = (m_cr & 0x18) >> 3;
//  opm = (m_cr & 0x07);
	dX = m_pr[0];
	dY = m_pr[1];

	offset = m_rwp[m_rwp_dn] & 0xfffff;

	inc_x = (dX < 0) ? -1 : 1;
	inc_y = (dY < 0) ? -1 : 1;

	printf("%d %d\n",dX,dY);

	/*
    3<-2
    |  ^
    v  |
    0->1
    */

	/* 0 -> 1 */
	for(i=0;i<dX;i+=inc_x)
	{
		offset = (((i + m_cpx) >> 1) + (0 + m_cpy) * m_mwr[m_rwp_dn]) & 0xfffff; /* TODO: address if bpp != 4 */
		data_r = readbyte(offset);

		if((i + m_cpx) & 1)
			res = (col & 0x0f) | (data_r & 0xf0);
		else
			res = (col & 0xf0) | (data_r & 0x0f);

		writebyte(offset,res);
	}

	/* 1 -> 2 */
	for(i=0;i<dY;i+=inc_y)
	{
		offset = (((dX + m_cpx) >> 1) + (i + m_cpy) * m_mwr[m_rwp_dn]) & 0xfffff; /* TODO: address if bpp != 4 */
		data_r = readbyte(offset);

		if((dX + m_cpx) & 1)
			res = (col & 0x0f) | (data_r & 0xf0);
		else
			res = (col & 0xf0) | (data_r & 0x0f);

		writebyte(offset,res);
	}

	/* 2 -> 3 */
	for(i=0;i<dX;i+=inc_x)
	{
		offset = (((i + m_cpx) >> 1) + (dY + m_cpy) * m_mwr[m_rwp_dn]) & 0xfffff; /* TODO: address if bpp != 4 */
		data_r = readbyte(offset);

		if((i + m_cpx) & 1)
			res = (col & 0x0f) | (data_r & 0xf0);
		else
			res = (col & 0xf0) | (data_r & 0x0f);

		writebyte(offset,res);
	}

	/* 3 -> 4 */
	for(i=0;i<dY;i+=inc_y)
	{
		offset = (((0 + m_cpx) >> 1) + (i + m_cpy) * m_mwr[m_rwp_dn]) & 0xfffff; /* TODO: address if bpp != 4 */
		data_r = readbyte(offset);

		if((0 + m_cpx) & 1)
			res = (col & 0x0f) | (data_r & 0xf0);
		else
			res = (col & 0xf0) | (data_r & 0x0f);

		writebyte(offset,res);
	}


	m_cpx += dX;
	m_cpy += dY;
}

void h63484_device::process_fifo()
{
	UINT8 data;

	dequeue_w(&data);

	if (m_sr & H63484_SR_CED)
	{
		m_cr = (data & 0xff) << 8;
		dequeue_w(&data);
		m_cr |= data & 0xff;
		m_param_ptr = 0;
		m_sr &= ~H63484_SR_CED;
	}
	else
	{
		m_pr[m_param_ptr] = (data & 0xff) << 8;
		dequeue_w(&data);
		m_pr[m_param_ptr] |= (data & 0xff);
		m_param_ptr++;
	}

	switch (translate_command(m_cr))
	{
		case COMMAND_INVALID:
			printf("H63484 '%s' Invalid Command Byte %02x\n", tag(), m_cr);
			m_sr |= H63484_SR_CER; // command error
			break;

		case COMMAND_ORG:
			if (m_param_ptr == 2)
			{
				m_org_dn = (m_pr[0] & 0xc000) >> 14;
				m_org_dpa = ((m_pr[0] & 0xff) << 12) | ((m_pr[1] & 0xfff0) >> 4);
				m_org_dpd = (m_pr[1] & 0xf);
				// TODO: CP = 0
				command_end_seq();
			}
			break;

		case COMMAND_WPR: // 0x0800 & ~0x1f
			if (m_param_ptr == 1)
			{
				command_wpr_exec();
				command_end_seq();
			}
			break;

		case COMMAND_WPTN:
			if(m_param_ptr == 1)
			{
				m_dn = m_pr[0]; // number of param words

				//if(m_dn > 0x10 || m_dn == 0)
				//  fatalerror("stop!\n");
			}

			if(m_param_ptr == (1 + m_dn))
			{
				int pra = m_cr & 0xf;
				int i;

				for(i=0;i<m_dn;i++)
					m_pram[(i + pra) & 0xf] = m_pr[1 + i];

				command_end_seq();
			}

			break;

		case COMMAND_RD:
			if (m_param_ptr == 0)
			{
				queue_r(readbyte((m_rwp[m_rwp_dn]+0) & 0xfffff));
				queue_r(readbyte((m_rwp[m_rwp_dn]+1) & 0xfffff));
				m_rwp[m_rwp_dn]+=2;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq();
			}
			break;

		case COMMAND_WT:
			if (m_param_ptr == 1)
			{
				writebyte(((m_rwp[m_rwp_dn]+0) & 0xfffff),(m_pr[0] & 0xff00) >> 8);
				writebyte(((m_rwp[m_rwp_dn]+1) & 0xfffff),(m_pr[0] & 0x00ff) >> 0);
				m_rwp[m_rwp_dn]+=2;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq();
			}

		case COMMAND_CLR:
			if (m_param_ptr == 3)
			{
				command_clr_exec();
				command_end_seq();
			}
			break;

		case COMMAND_CPY:
			if (m_param_ptr == 4)
			{
				command_cpy_exec();
				command_end_seq();
			}
			break;

		case COMMAND_AMOVE:
			if (m_param_ptr == 2)
			{
				m_cpx = m_pr[0];
				m_cpy = m_pr[1];
				command_end_seq();
			}
			break;

		case COMMAND_RRCT:
			if (m_param_ptr == 2)
			{
				command_rct_exec();
				command_end_seq();
			}
			break;

		case COMMAND_RMOVE:
			if (m_param_ptr == 2)
			{
				m_cpx += (INT16)m_pr[0];
				m_cpy += (INT16)m_pr[1];
				command_end_seq();
			}
			break;

		default:
			printf("%04x\n",m_cr);
			fatalerror("stop!\n");
			break;
	}
}

void h63484_device::exec_abort_sequence()
{
	fifo_w_clear();
	fifo_r_clear();
	m_sr = H63484_SR_WFR | H63484_SR_WFE | H63484_SR_CED; // hard-set to 0x23
}

UINT16 h63484_device::video_registers_r(int offset)
{
	UINT16 res;

	res = 0;

	switch(offset)
	{
		case 0x06:
			res = m_dcr;
			break;

		case 0x80:
			res = m_screen->vpos() & 0xfff; // Raster Count
			break;

		default:
			if(LOG) printf("%s R\n",acrtc_regnames[m_ar/2]);
			break;
	}

	return res;
}

void h63484_device::video_registers_w(int offset)
{
	UINT16 vreg_data;

	vreg_data = (m_vreg[offset]<<8)|(m_vreg[offset+1]&0xff);

	switch(offset)
	{
		case 0x00: // FIFO entry
			queue_w((vreg_data & 0xff00) >> 8);
			queue_w((vreg_data & 0x00ff) >> 0);
			if(FIFO_LOG) printf("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			process_fifo();
			break;

		case 0x02: // Command Entry

			if(vreg_data & 0x8000) // abort sequence (ABT)
				exec_abort_sequence();

			/*
            x--- ---- ---- ---- ABorT
            -x-- ---- ---- ---- PauSE
            ...
            ---- -xxx ---- ---- Graphic Bit Mode (bpp)
            ---- ---- xxxx xxxx irq mask, directly correlated to sr
            */
			m_ccr = vreg_data;
			break;

		case 0x06:
			m_dcr = vreg_data;
			break;

		case 0x82: // Horizontal Sync Register
			m_hc = ((vreg_data & 0xff00) >> 8) + 1;
			m_hsw = vreg_data & 0x1f;
			recompute_parameters();
			break;
		case 0x84: // Horizontal Display Register
			m_hds = ((vreg_data & 0xff00) >> 8) + 1;
			m_hdw = ((vreg_data & 0x00ff) >> 8) + 1;
			recompute_parameters();
			break;
		case 0x92: // Horizontal Window Register
			m_hws = ((vreg_data & 0xff00) >> 8) + 1;
			m_hww = ((vreg_data & 0x00ff) >> 8) + 1;
			recompute_parameters();
			break;

		case 0x86: // Vertical Sync Register
			m_vc = (vreg_data & 0xfff);
			recompute_parameters();
			break;
		case 0x88: // Vertical Display Register
			m_vds = ((vreg_data & 0xff00) >> 8) + 1;
			m_vsw = (vreg_data & 0x1f);
			recompute_parameters();
			break;
		case 0x94: // Vertical Window Register A
			m_vws = (vreg_data & 0xfff) + 1;
			recompute_parameters();
			break;
		case 0x96: // Vertical Window Register B
			m_vww = (vreg_data & 0xfff);
			recompute_parameters();
			break;

		case 0xc2: // Memory Width Register
		case 0xca:
		case 0xd2:
		case 0xda:
			m_mwr[(offset & 0x18) >> 3] = vreg_data & 0xfff; // pitch
			m_mwr_chr[(offset & 0x18) >> 3] = (vreg_data & 0x8000) >> 15;
			break;

		case 0xc4: // Start Address Register
		case 0xcc:
		case 0xd4:
		case 0xdc:
			m_sar[(offset & 0x18) >> 3] = ((vreg_data & 0xf) << 16) | (m_sar[(offset & 0x18) >> 3] & 0xffff);
			m_sda[(offset & 0x18) >> 3] = (vreg_data & 0x0f00) >> 8;
			break;

		case 0xc6: // Start Address Register
		case 0xce:
		case 0xd6:
		case 0xde:
			m_sar[(offset & 0x18) >> 3] = (vreg_data & 0xffff) | (m_sar[(offset & 0x18) >> 3] & 0xf0000);
			break;

		default:
			if(LOG) printf("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			break;
	}
}

READ16_MEMBER( h63484_device::status_r )
{
	return m_sr;
}

READ16_MEMBER( h63484_device::data_r )
{
	int res;

	res = 0xffff;

	if(m_ar == 0) // FIFO read
	{
		UINT8 data;

		dequeue_r(&data);
		res = (data & 0xff) << 8;
		dequeue_r(&data);
		res |= data & 0xff;
	}
	else
		res = video_registers_r(m_ar);

	return res;
}

WRITE16_MEMBER( h63484_device::address_w )
{
	if(ACCESSING_BITS_0_7)
		m_ar = data & 0xff;
}

WRITE16_MEMBER( h63484_device::data_w )
{
	if(ACCESSING_BITS_8_15)
		m_vreg[m_ar] = (data & 0xff00) >> 8;

	if(ACCESSING_BITS_0_7)
		m_vreg[m_ar+1] = (data & 0xff);

	video_registers_w(m_ar);

	if(m_ar & 0x80)
	{
		m_ar+=2;
		m_ar &= 0xff; // TODO: what happens if it overflows?
	}
}

void h63484_device::device_start()
{
	m_screen = machine().device<screen_device>(m_screen_tag);

	//h63484->space = device->memory().space(AS_0);
	m_vram = auto_alloc_array_clear(machine(), UINT8, 1 << 20);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void h63484_device::device_reset()
{
	// ...
}

//-------------------------------------------------
//  draw_graphics_line -
//-------------------------------------------------

void h63484_device::draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int layer_n)
{
	int x;
	int pitch;
	int base_offs;

	pitch = m_mwr[layer_n];
	base_offs = m_sar[layer_n];

	for(x=0;x<pitch * 4;x+=4)
	{
		UINT16 data;

		data = readbyte(base_offs + (x >> 1) + y * pitch * 2);

		m_display_cb(this, bitmap, y, x+3, (data >> 4) & 0xf);
		m_display_cb(this, bitmap, y, x+2, data & 0xf);

		data = readbyte(base_offs + ((x + 2) >> 1) + y * pitch * 2);

		m_display_cb(this, bitmap, y, x+1, (data >> 4) & 0xf);
		m_display_cb(this, bitmap, y, x+0, data & 0xf);
	}
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 h63484_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if(m_dcr & 0x8000) // correct?
	{
		int y;

		for(y=cliprect.min_y;y<cliprect.max_y;y++)
		{
			if (m_display_cb)
				draw_graphics_line(bitmap,cliprect, y, 1);
		}
	}
	return 0;
}
