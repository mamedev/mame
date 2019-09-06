// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics RE2 device.
 *
 * TODO:
 *   - skeleton only
 *
 */

#include "emu.h"
#include "debugger.h"
#include "sgi_re2.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_REG       (1U << 1)

//#define VERBOSE       (LOG_GENERAL|LOG_REG)
#include "logmacro.h"

static char const *const regnames[] =
{
	nullptr,     nullptr,     nullptr,     nullptr,     "ENABRGB",   "BIGENDIAN", "FUNC",      "HADDR",
	"NOPUP",     "XYFRAC",    "RGB",       "YX",        "PUPDATA",   "PATL",      "PATH",      "DZI",
	"DZF",       "DR",        "DG",        "DB",        "Z",         "R",         "G",         "B",
	"STIP",      "STIPCOUNT", "DX",        "DY",        "NUMPIX",    "X",         "Y",         "IR",

	"RWDATA",    "PIXMASK",   "AUXMASK",   "WIDDATA",   "UAUXDATA",  "RWMODE",    "READBUF",   "PIXTYPE",
	"ASELECT",   "ALIGNPAT",  "ENABPAT",   "ENABSTIP",  "ENABDITH",  "ENABWID",   "CURWID",    "DEPTHFN",
	"REPSTIP",   "ENABLWID",  "FBOPTION",  "TOPSCAN",   nullptr,     nullptr,     "ZBOPTION",  "XZOOM",
	"UPACMODE",  "YMIN",      "YMAX",      "XMIN",      "XMAX",      "COLORCMP",  "MEGOPTION", nullptr,
};

DEFINE_DEVICE_TYPE(SGI_RE2, sgi_re2_device, "sgi_re2", "SGI Raster Engine 2")

sgi_re2_device::sgi_re2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_RE2, tag, owner, clock)
{
}

void sgi_re2_device::device_start()
{
	// TODO: save state
}

void sgi_re2_device::device_reset()
{
}

u32 sgi_re2_device::reg_r(offs_t offset)
{
	u32 data = 0;

	switch (offset)
	{
	case RE2_STIP: data = m_stip; break;
	case RE2_STIPCOUNT: data = m_stipcount; break;
	case RE2_RWDATA: data = m_rwdata; break;
	default:
		logerror("reg_r unhandled register 0x%x\n", offset);
		break;
	}

	return data;
}

void sgi_re2_device::reg_w(offs_t offset, u32 data)
{
	if ((offset < 64) && regnames[offset])
		LOGMASKED(LOG_REG, "reg_w register 0x%x (%s) data 0x%x\n", offset, regnames[offset], data);
	else
		logerror("reg_w unhandled register 0x%x data 0x%x\n", offset, data);

	switch (offset)
	{
	case RE2_STIP: m_stip = data; break;
	case RE2_STIPCOUNT: m_stipcount = data; break;
	}
}
