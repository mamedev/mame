// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    AMD Am79C30A Digital Subscriber Controller (DSC)

    TODO: everything (this is a glorified skeleton device)

****************************************************************************/

#include "emu.h"
#include "am79c30.h"

#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(AM79C30A, am79c30a_device, "am79c30a", "Am79C30A DSC")
//DEFINE_DEVICE_TYPE(AM79C32A, am79c32a_device, "am79c32a", "Am79C32A IDC")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  am79c30a_device - constructor
//-------------------------------------------------

am79c30a_device::am79c30a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AM79C30A, tag, owner, clock)
	, m_int_callback(*this)
	, m_init(0)
	, m_init2(0)
	, m_ir(0)
	, m_lsr(0)
	, m_lpr(0)
	, m_lmr1(0)
	, m_lmr2(0)
	, m_mf(0)
	, m_mfsb(0)
	, m_mfqb(0)
	, m_mcr{0, 0, 0}
	, m_mcr4(0)
	, m_x_coeff{0, 0, 0, 0, 0, 0, 0, 0}
	, m_r_coeff{0, 0, 0, 0, 0, 0, 0, 0}
	, m_gx_coeff(0)
	, m_gr_coeff(0)
	, m_ger_coeff(0)
	, m_stgr(0)
	, m_ftgr{0, 0}
	, m_atgr{0, 0}
	, m_mmr1(0)
	, m_mmr2(0)
	, m_mmr3(0)
	, m_stra(0)
	, m_strf(0)
	, m_peakx(0)
	, m_peakr(0)
	, m_tar(0)
	, m_frar{0, 0, 0, 0}
	, m_srar{0, 0, 0, 0}
	, m_drlr(0)
	, m_dtcr(0)
	, m_drcr(0)
	, m_rngr(0)
	, m_dmr1(0)
	, m_dmr2(0)
	, m_dmr3(0)
	, m_dmr4(0)
	, m_asr(0)
	, m_dsr1(0)
	, m_dsr2(0)
	, m_der(0)
	, m_efcr(0)
	, m_ppcr1(0)
	, m_ppsr(0)
	, m_ppier(0)
	, m_mtdr(0)
	, m_mrdr(0)
	, m_citdr0(0)
	, m_cirdr0(0)
	, m_citdr1(0)
	, m_cirdr1(0)
	, m_ppcr2(0)
	, m_ppcr3(0)
	, m_cr(0)
	, m_byte_seq(0)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void am79c30a_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am79c30a_device::device_start()
{
	// Save state
	save_item(NAME(m_init));
	save_item(NAME(m_init2));
	save_item(NAME(m_ir));
	save_item(NAME(m_lsr));
	save_item(NAME(m_lpr));
	save_item(NAME(m_lmr1));
	save_item(NAME(m_lmr2));
	save_item(NAME(m_mf));
	save_item(NAME(m_mfsb));
	save_item(NAME(m_mfqb));
	save_item(NAME(m_mcr));
	save_item(NAME(m_mcr4));
	save_item(NAME(m_x_coeff));
	save_item(NAME(m_r_coeff));
	save_item(NAME(m_gx_coeff));
	save_item(NAME(m_gr_coeff));
	save_item(NAME(m_ger_coeff));
	save_item(NAME(m_stgr));
	save_item(NAME(m_ftgr));
	save_item(NAME(m_atgr));
	save_item(NAME(m_mmr1));
	save_item(NAME(m_mmr2));
	save_item(NAME(m_mmr3));
	save_item(NAME(m_stra));
	save_item(NAME(m_strf));
	save_item(NAME(m_peakx));
	save_item(NAME(m_peakr));
	save_item(NAME(m_tar));
	save_item(NAME(m_frar));
	save_item(NAME(m_srar));
	save_item(NAME(m_drlr));
	save_item(NAME(m_dtcr));
	save_item(NAME(m_drcr));
	save_item(NAME(m_rngr));
	save_item(NAME(m_dmr1));
	save_item(NAME(m_dmr2));
	save_item(NAME(m_dmr3));
	save_item(NAME(m_dmr4));
	save_item(NAME(m_asr));
	save_item(NAME(m_dsr1));
	save_item(NAME(m_dsr2));
	save_item(NAME(m_der));
	save_item(NAME(m_efcr));
	save_item(NAME(m_ppcr1));
	save_item(NAME(m_ppsr));
	save_item(NAME(m_ppier));
	save_item(NAME(m_mtdr));
	save_item(NAME(m_mrdr));
	save_item(NAME(m_citdr0));
	save_item(NAME(m_cirdr0));
	save_item(NAME(m_citdr1));
	save_item(NAME(m_cirdr1));
	save_item(NAME(m_ppcr2));
	save_item(NAME(m_ppcr3));
	save_item(NAME(m_cr));
	save_item(NAME(m_byte_seq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void am79c30a_device::device_reset()
{
	// Clear init registers
	m_init = 0;
	m_init2 = 0;

	// Clear interrupt register and deactivate INT
	m_ir = 0;
	m_int_callback(CLEAR_LINE);

	// Clear LIU registers (except HSW bit)
	m_lsr &= 0x40;
	m_lpr = 0;
	m_lmr1 = 0;
	m_lmr2 = 0;
	m_mf = 0;
	m_mfsb = 0x40;
	m_mfqb = 0x0f;

	// Clear MUX registers
	m_mcr[0] = m_mcr[1] = m_mcr[2] = 0;
	m_mcr4 = 0;

	// Clear MAP registers
	m_ftgr[0] = m_ftgr[1] = 0;
	m_mmr1 = 0;
	m_mmr2 = 0;
	m_mmr3 = 0;
	m_stra = 0;
	m_strf = 0;

	// Clear DLC registers
	m_dmr1 = 0;
	m_dmr2 = 0;
	m_dmr3 = 0;
	m_dmr4 = 0;
	m_asr = 0;
	m_dsr1 = 0;
	m_dsr2 = 0;
	m_der = 0;

	// Set PP register defaults
	m_ppcr1 = 0x01;
	m_ppsr = 0x02;
	m_ppier = 0x80;
	m_mtdr = 0xff;
	m_mrdr = 0;
	m_citdr0 = 0x0f;
	m_cirdr0 = 0x0f;
	m_citdr1 = 0x3f;
	m_cirdr1 = 0x3f;
	m_ppcr2 = 0xc0; // revision-level dependent
	m_ppcr3 = 0x17;
}


//**************************************************************************
//  GLOBAL INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  set_init - indirect write to INIT register
//-------------------------------------------------

void am79c30a_device::set_init(u8 data)
{
	if ((m_init & 0x03) != (data & 0x03))
		LOG("%s: %s mode\n", machine().describe_context(),
					(data & 0x03) == 0x01 ? "Active voice and data" :
					(data & 0x03) == 0x02 ? "Active data only" :
					(data & 0x03) == 0x03 ? "Power-down" : "Idle");
	if (BIT(m_init, 2) != BIT(data, 2))
		LOG("%s: INT output %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if ((m_init & 0x38) != (data & 0x38))
	{
		unsigned div;
		if ((data & 0x38) == 0x08)
			div = 1;
		else if ((data & 0x38) == 0x10)
			div = 4;
		else if ((data & 0x38) == 0x20)
			div = 3;
		else
			div = 2;
		LOG("%s: MCLK frequency = %.3f MHz\n", machine().describe_context(), clocks_to_attotime(div).as_mhz());
	}
	if (BIT(m_init, 6) != BIT(data, 6))
		LOG("%s: DLC receiver abort %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(m_init, 7) != BIT(data, 7))
		LOG("%s: DLC transmitter abort %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");

	m_init = data;
}


//-------------------------------------------------
//  set_init2 - indirect write to INIT2 register
//-------------------------------------------------

void am79c30a_device::set_init2(u8 data)
{
	if (BIT(m_init2, 5) != BIT(data, 5))
		LOG("%s: Power-down %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(m_init2, 4) != BIT(data, 4))
		LOG("%s: Multiframe interrupt filter %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_init2, 3) != BIT(data, 3))
		LOG("%s: Clock speed-up option %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if ((m_init2 & 0x07) != (data & 0x07))
	{
		if ((data & 0x07) == 0x00)
			LOG("%s: MCLK frequency determined by INIT\n", machine().describe_context());
		else if ((data & 0x07) < 0x04)
			LOG("%s: MCLK frequency = %.1f kHz\n", machine().describe_context(), clocks_to_attotime(4 << (data & 0x07)).as_khz());
		else if ((data & 0x07) == 0x04)
			LOG("%s: MCLK stopped in high state\n", machine().describe_context());
		else
			logerror("%s: MCLK reserved setting\n", machine().describe_context());
	}
	if ((data & 0xc0) != 0 && !machine().side_effects_disabled())
		logerror("%s: Writing %02XH to INIT2 reserved bits\n", machine().describe_context(), data & 0xc0);

	m_init2 = data;
}


//**************************************************************************
//  INTERRUPT HANDLING
//**************************************************************************

//-------------------------------------------------
//  ir_r - direct read from interrupt register
//-------------------------------------------------

u8 am79c30a_device::ir_r()
{
	u8 ir = m_ir;

	if (ir != 0 && !machine().side_effects_disabled())
	{
		// Clear register and deactivate INT output
		m_ir = 0;
		m_int_callback(CLEAR_LINE);
	}

	return ir;
}


//**************************************************************************
//  LINE INTERFACE UNIT (LIU)
//**************************************************************************

//-------------------------------------------------
//  get_lsr - indirect read from LIU status
//  register
//-------------------------------------------------

u8 am79c30a_device::get_lsr()
{
	u8 lsr = m_lsr;

	// Bits 3, 4, 5 and 7 are cleared when read by microprocessor
	if (!machine().side_effects_disabled())
		m_lsr &= 0x47;

	return lsr;
}


//-------------------------------------------------
//  set_lpr - indirect write to LIU D-channel
//  priority register
//-------------------------------------------------

void am79c30a_device::set_lpr(u8 data)
{
	if ((data & 0xf0) != 0)
	{
		logerror("%s: Writing %X to LPR reserved upper bits\n", machine().describe_context(), data);
		data &= 0x0f;
	}

	LOG("%s: LIU D-channel access priority level set to %d\n", machine().describe_context(), data);

	m_lpr = data;
}


//-------------------------------------------------
//  set_lmr1 - indirect write to LIU mode register
//  1
//-------------------------------------------------

void am79c30a_device::set_lmr1(u8 data)
{
	if (BIT(data, 0) != BIT(m_lmr1, 0))
		LOG("%s: B1 transmit %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(data, 1) != BIT(m_lmr1, 1))
		LOG("%s: B2 transmit %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(data, 2) != BIT(m_lmr1, 2))
		LOG("%s: F transmit %sabled\n", machine().describe_context(), BIT(data, 2) ? "dis" : "en");
	if (BIT(data, 3) != BIT(m_lmr1, 3))
		LOG("%s: Fa transmit %sabled\n", machine().describe_context(), BIT(data, 3) ? "dis" : "en");
	if (BIT(data, 4) != BIT(m_lmr1, 4))
		LOG("%s: %sctivation request\n", machine().describe_context(), BIT(data, 4) ? "A" : "No a");
	if (BIT(data, 5) != BIT(m_lmr1, 5))
		LOG("%s: %sF8 to F3 transition %s\n", machine().describe_context(), BIT(data, 5) ? "" : "No ");
	if (BIT(data, 6) != BIT(m_lmr1, 6))
		LOG("%s: LIU receiver/transmitter %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(data, 7))
		logerror("%s: LMR1 reserved bit 7 set\n");

	m_lmr1 = data;
}


//-------------------------------------------------
//  set_lmr2 - indirect write to LIU mode register
//  2
//-------------------------------------------------

void am79c30a_device::set_lmr2(u8 data)
{
	if (BIT(data, 0) != BIT(m_lmr2, 0))
		LOG("%s: D-channel loopback at DSC %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(data, 1) != BIT(m_lmr2, 1))
		LOG("%s: D-channel loopback at LIU %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(data, 2) != BIT(m_lmr2, 2))
		LOG("%s: D-channel back-off %sabled\n", machine().describe_context(), BIT(data, 2) ? "dis" : "en");
	if (BIT(data, 3) != BIT(m_lmr2, 3))
		LOG("%s: F3 change of state interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(data, 4) != BIT(m_lmr2, 4))
		LOG("%s: F8 change of state interrupt %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(data, 5) != BIT(m_lmr2, 5))
		LOG("%s: HSW interrupt %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(data, 6) != BIT(m_lmr2, 6))
		LOG("%s: F7 change of state interrupt %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(data, 7))
		logerror("%s: LMR2 reserved bit 7 set\n");

	m_lmr2 = data;
}


//-------------------------------------------------
//  set_mf - indirect write to multiframe register
//-------------------------------------------------

void am79c30a_device::set_mf(u8 data)
{
	if (BIT(data, 0) != BIT(m_mf, 0))
		LOG("%s: Multiframe sync %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(data, 1) != BIT(m_mf, 1))
		LOG("%s: S-data available interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(data, 2) != BIT(m_mf, 2))
		LOG("%s: Q-bit buffer available interrupt %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(data, 3) != BIT(m_mf, 3))
		LOG("%s: Multiframe change of state interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(data, 4) != BIT(m_mf, 4))
		LOG("%s: %sirst subframe\n", machine().describe_context(), BIT(data, 4) ? "F" : "Not f");

	// Bit 7 is read-only
	m_mf = (data & 0x1f) | (m_mf & 0x80);
}


//-------------------------------------------------
//  set_mfqb - indirect write to multiframe Q-bit
//  buffer
//-------------------------------------------------

void am79c30a_device::set_mfqb(u8 data)
{
	LOG("%s: Multiframe Q-bit buffer = %02XH\n", machine().describe_context(), data);

	// Bits 5, 6, 7 not used
	m_mfqb = data & 0x1f;
}


//**************************************************************************
//  MULTIPLEXER (MUX)
//**************************************************************************

const char *const am79c30a_device::s_mcr_channels[16] = {
	"No connection",
	"B1 (LIU)",
	"B2 (LIU)",
	"Ba (MAP)",
	"Bb (MPI)",
	"Bc (MPI)",
	"Bd (PP channel 1)",
	"Be (PP channel 2)",
	"Bf (PP channel 3)",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};


//-------------------------------------------------
//  set_mcr - indirect write to MUX control
//  register 1, 2 or 3 (bidirectional paths)
//-------------------------------------------------

void am79c30a_device::set_mcr(unsigned n, u8 data)
{
	if (m_mcr[n] != data)
	{
		if (data == 0)
			LOG("%s: No connect (MCR%d)\n", machine().describe_context(), n + 1);
		else if ((data & 0xf0) >> 4 == (data & 0x0f))
			LOG("%s: %s loopback (MCR%d)\n", machine().describe_context(), data & 0x0f, n + 1);
		else
			LOG("%s: %s <-> %s (MCR%d)\n", machine().describe_context(), (data & 0xf0) >> 4, data & 0x0f, n + 1);
	}

	m_mcr[n] = data;
}


//-------------------------------------------------
//  set_mcr4 - indirect write to MUX control
//  register 4
//-------------------------------------------------

void am79c30a_device::set_mcr4(u8 data)
{
	if (BIT(data, 3) != BIT(m_mcr4, 3))
		LOG("%s: Bb-/Bc-channel byte available interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(data, 4) != BIT(m_mcr4, 4))
		LOG("%s: Bb bit order %sreversed (%cSB first)\n", machine().describe_context(), BIT(data, 4) ? "" : "not ", BIT(data, 4) ? 'L' : 'M');
	if (BIT(data, 5) != BIT(m_mcr4, 5))
		LOG("%s: Bc bit order %sreversed (%cSB first)\n", machine().describe_context(), BIT(data, 5) ? "" : "not ", BIT(data, 5) ? 'L' : 'M');
	if ((data & 0xc7) != 0)
		logerror("%s: Writing %02XH to MCR4 reserved bits\n", machine().describe_context(), data & 0xc7);

	m_mcr4 = data;
}


//**************************************************************************
//  MAIN AUDIO PROCESSOR (MAP)
//**************************************************************************

//-------------------------------------------------
//  set_x_coeff - indirect write to X-filter
//  coefficient register (16 bytes)
//-------------------------------------------------

void am79c30a_device::set_x_coeff(unsigned n, u8 data, bool msb)
{
	LOG("%s: Writing %02XH to X h%u %cSB\n", machine().describe_context(), data, n, msb ? 'M' : 'L');

	if (msb)
		m_x_coeff[n] = (m_x_coeff[n] & 0x00ff) | u16(data) << 8;
	else
		m_x_coeff[n] = (m_x_coeff[n] & 0xff00) | data;
}


//-------------------------------------------------
//  set_r_coeff - indirect write to R-filter
//  coefficient register (16 bytes)
//-------------------------------------------------

void am79c30a_device::set_r_coeff(unsigned n, u8 data, bool msb)
{
	LOG("%s: Writing %02XH to R h%u %cSB\n", machine().describe_context(), data, n, msb ? 'M' : 'L');

	if (msb)
		m_r_coeff[n] = (m_r_coeff[n] & 0x00ff) | u16(data) << 8;
	else
		m_r_coeff[n] = (m_r_coeff[n] & 0xff00) | data;

}


//-------------------------------------------------
//  set_gx_coeff - indirect write to GX-gain
//  coefficient register (2 bytes)
//-------------------------------------------------

void am79c30a_device::set_gx_coeff(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to GX %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_gx_coeff = (m_gx_coeff & 0x00ff) | u16(data) << 8;
	else
		m_gx_coeff = (m_gx_coeff & 0xff00) | data;
}


//-------------------------------------------------
//  set_gr_coeff - indirect write to GR-gain
//  coefficient register (2 bytes)
//-------------------------------------------------

void am79c30a_device::set_gr_coeff(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to GR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_gr_coeff = (m_gr_coeff & 0x00ff) | u16(data) << 8;
	else
		m_gr_coeff = (m_gr_coeff & 0xff00) | data;
}


//-------------------------------------------------
//  set_ger_coeff - indirect write to GER-gain
//  coefficient register (2 bytes)
//-------------------------------------------------

void am79c30a_device::set_ger_coeff(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to GER %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_ger_coeff = (m_ger_coeff & 0x00ff) | u16(data) << 8;
	else
		m_ger_coeff = (m_ger_coeff & 0xff00) | data;
}


//-------------------------------------------------
//  set_stgr - indirect write to sidetone-gain
//  coefficient register (2 bytes)
//-------------------------------------------------

void am79c30a_device::set_stgr(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to STGR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_stgr = (m_stgr & 0x00ff) | u16(data) << 8;
	else
		m_stgr = (m_stgr & 0xff00) | data;
}


//-------------------------------------------------
//  set_ftgr - indirect write to frequency
//  tone generator register 1 or 2
//-------------------------------------------------

void am79c30a_device::set_ftgr(unsigned n, u8 data)
{
	LOG("%s: Writing %02XH to FTGR%u\n", machine().describe_context(), data, n + 1);

	m_ftgr[n] = data;
}


//-------------------------------------------------
//  set_atgr - indirect write to amplitude
//  tone generator register 1 or 2
//-------------------------------------------------

void am79c30a_device::set_atgr(unsigned n, u8 data)
{
	LOG("%s: Writing %02XH to ATGR%u\n", machine().describe_context(), data, n + 1);

	m_atgr[n] = data;
}


//-------------------------------------------------
//  set_mmr1 - indirect write to MAP mode register
//  1
//-------------------------------------------------

void am79c30a_device::set_mmr1(u8 data)
{
	if (BIT(m_mmr1, 0) != BIT(data, 0))
		LOG("%s: %s-law\n", machine().describe_context(), BIT(data, 0) ? "A" : "u");
	if (BIT(m_mmr1, 1) != BIT(data, 1))
		LOG("%s: GX %s\n", machine().describe_context(), BIT(data, 1) ? "coefficient loaded" : "bypassed");
	if (BIT(m_mmr1, 2) != BIT(data, 2))
		LOG("%s: GR %s\n", machine().describe_context(), BIT(data, 2) ? "coefficient loaded" : "bypassed");
	if (BIT(m_mmr1, 3) != BIT(data, 3))
		LOG("%s: GER %s\n", machine().describe_context(), BIT(data, 3) ? "coefficient loaded" : "bypassed");
	if (BIT(m_mmr1, 4) != BIT(data, 4))
		LOG("%s: X %s\n", machine().describe_context(), BIT(data, 4) ? "coefficient loaded" : "bypassed");
	if (BIT(m_mmr1, 5) != BIT(data, 5))
		LOG("%s: R %s\n", machine().describe_context(), BIT(data, 5) ? "coefficient loaded" : "bypassed");
	if (BIT(m_mmr1, 6) != BIT(data, 6))
		LOG("%s: Sidetone gain %s\n", machine().describe_context(), BIT(data, 6) ? "coefficient loaded" : "= -18 dB");
	if (BIT(m_mmr1, 7) != BIT(data, 7))
		LOG("%s: Digital loopback at MAP #1 %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");

	m_mmr1 = data;
}


//-------------------------------------------------
//  set_mmr2 - indirect write to MAP mode register
//  2
//-------------------------------------------------

void am79c30a_device::set_mmr2(u8 data)
{
	if (BIT(m_mmr2, 0) != BIT(data, 0))
		LOG("%s: AIN%c selected\n", machine().describe_context(), BIT(data, 0) ? 'B' : 'A');
	if (BIT(m_mmr2, 1) != BIT(data, 1))
		LOG("%s: %s selected\n", machine().describe_context(), BIT(data, 1) ? "LS1/LS2" : "EAR1/EAR2");
	if (BIT(m_mmr2, 2) != BIT(data, 2))
		LOG("%s: DTMF %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_mmr2, 3) != BIT(data, 3))
		LOG("%s: Tone generator %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(m_mmr2, 4) != BIT(data, 4))
		LOG("%s: Tone ringer %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_mmr2, 5) != BIT(data, 5))
		LOG("%s: High pass filter %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(m_mmr2, 6) != BIT(data, 6))
		LOG("%s: ADC auto-zero function %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(data, 7))
		logerror("%s: MMR2 reserved bit 7 set\n", machine().describe_context());

	m_mmr2 = data;
}


//-------------------------------------------------
//  set_mmr3 - indirect write to MAP mode register
//  3
//-------------------------------------------------

void am79c30a_device::set_mmr3(u8 data)
{
	if (BIT(data, 7))
		logerror("%s: MMR3 reserved bit 7 set\n", machine().describe_context());
	if ((m_mmr3 & 0x70) != (data & 0x70))
	{
		if ((data & 0x70) > 0x40)
			logerror("%s: Reserved pre-amplifier setting\n", machine().describe_context());
		else
			LOG("%s: +%d-dB pre-amplifier gain (%.3f-V maximum peak input voltage)\n", machine().describe_context(),
				((data & 0x70) >> 4) * 6,
				1.250 / (1 << ((data & 0x70) >> 4)));
	}
	if (BIT(m_mmr3, 3) != BIT(data, 3))
		LOG("%s: Mute %s\n", machine().describe_context(), BIT(data, 3) ? "on" : "off");
	if (BIT(m_mmr3, 2) != BIT(data, 2))
		LOG("%s: Digital loopback 2 %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_mmr3, 1) != BIT(data, 1))
		LOG("%s: EAR %s LS %sly enabled\n", machine().describe_context(), BIT(data, 1) ? "and" : "or", BIT(data, 1) ? "simultaneous" : "exclusive");
	if (BIT(m_mmr3, 0) != BIT(data, 0))
		LOG("%s: Secondary tone ringer %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");

	m_mmr3 = data;
}


//-------------------------------------------------
//  set_stra - indirect write to secondary tone
//  ringer amplitude register
//-------------------------------------------------

void am79c30a_device::set_stra(u8 data)
{
	if ((data & 0x0f) != 0)
		logerror("%s: STRA reserved bits %02XH set\n", machine().describe_context(), data & 0x0f);
	if ((m_stra & 0xf0) != (data & 0xf0))
	{
		if ((data & 0xf0) == 0)
			LOG("%s: Secondary tone ringer silent\n", machine().describe_context());
		else if ((data & 0xf0) < 0x80)
			logerror("%s: Secondary tone ringer reserved setting\n", machine().describe_context());
		else
		{
			u8 a = 15 - ((data & 0xf0) >> 4);
			LOG("%s: Secondary tone ringer %.2f V peak-to-peak, %d dB relative\n", BIT(a, 0) ? 3.53553390593274 : 5.0 / (1 << (a / 2)), a * -3);
		}
	}

	m_stra = data;
}


//-------------------------------------------------
//  set_strf - indirect write to secondary tone
//  ringer frequency register
//-------------------------------------------------

void am79c30a_device::set_strf(u8 data)
{
	if (m_strf != data)
		LOG("%s: Secondary tone ringer frequency control = %02XH\n", machine().describe_context(), data);

	m_strf = data;
}


//**************************************************************************
//  DATA LINK CONTROLLER (DLC)
//**************************************************************************

//-------------------------------------------------
//  set_tar - indirect write to transmit address
//  register
//-------------------------------------------------

void am79c30a_device::set_tar(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to TAR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_tar = (m_tar & 0x00ff) | u16(data) << 8;
	else
		m_tar = (m_tar & 0xff00) | data;
}


//-------------------------------------------------
//  set_frar - indirect write to first received
//  byte address register
//-------------------------------------------------

void am79c30a_device::set_frar(unsigned n, u8 data)
{
	LOG("%s: Writing %02XH to FRAR%u\n", machine().describe_context(), data, n + 1);

	m_frar[n] = data;
}


//-------------------------------------------------
//  set_srar - indirect write to second received
//  byte address register
//-------------------------------------------------

void am79c30a_device::set_srar(unsigned n, u8 data)
{
	LOG("%s: Writing %02XH to SRAR%u\n", machine().describe_context(), data, n + 1);

	m_srar[n] = data;
}


//-------------------------------------------------
//  set_drlr - indirect write to D-channel receive
//  byte limit register
//-------------------------------------------------

void am79c30a_device::set_drlr(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to DRLR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_drlr = (m_drlr & 0x00ff) | u16(data) << 8;
	else
		m_drlr = (m_drlr & 0xff00) | data;
}


//-------------------------------------------------
//  set_dtcr - indirect write to D-channel transmit
//  byte count register
//-------------------------------------------------

void am79c30a_device::set_dtcr(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to DTCR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_dtcr = (m_dtcr & 0x00ff) | u16(data) << 8;
	else
		m_dtcr = (m_dtcr & 0xff00) | data;

	m_dsr1 &= 0x3f;
	m_der &= 0x7b;
}


//-------------------------------------------------
//  set_rngr - indirect write to random number
//  generator register
//-------------------------------------------------

void am79c30a_device::set_rngr(u8 data, bool msb)
{
	LOG("%s: Writing %02XH to RNGR %cSB\n", machine().describe_context(), data, msb ? 'M' : 'L');

	if (msb)
		m_rngr = (m_rngr & 0x00ff) | u16(data) << 8;
	else
		m_rngr = (m_rngr & 0xff00) | data;
}


//-------------------------------------------------
//  dctb_w - direct write to D-channel transmit
//  buffer register (FIFO)
//-------------------------------------------------

void am79c30a_device::dctb_w(u8 data)
{
	LOG("%s: Writing %02XH to DCTB\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  dcrb_r - direct read from D-channel receive
//  buffer register (FIFO)
//-------------------------------------------------

u8 am79c30a_device::dcrb_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: Reading from DCRB\n", machine().describe_context());

	return 0;
}


//-------------------------------------------------
//  set_dmr1 - indirect write to D-channel mode
//  register 1
//-------------------------------------------------

void am79c30a_device::set_dmr1(u8 data)
{
	if (BIT(m_dmr1, 0) != BIT(data, 0))
		LOG("%s: D-channel transmit threshold interrupt %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(m_dmr1, 1) != BIT(data, 1))
		LOG("%s: D-channel receive threshold interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(m_dmr1, 2) != BIT(data, 2))
		LOG("%s: Transmit Address Register %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_dmr1, 3) != BIT(data, 3))
		LOG("%s: End of receive packet interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	for (int n = 0; n < 4; n++)
		if (BIT(m_dmr1, n + 4) != BIT(data, n + 4))
			LOG("%s: FRAR%d/SRAR%d %sabled\n", machine().describe_context(), n + 1, n + 1, BIT(data, n + 4) ? "en" : "dis");

	m_dmr1 = data;
}


//-------------------------------------------------
//  set_dmr2 - indirect write to D-channel mode
//  register 2
//-------------------------------------------------

void am79c30a_device::set_dmr2(u8 data)
{
	if (BIT(m_dmr2, 0) != BIT(data, 0))
		LOG("%s: Receiver abort interrupt %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(m_dmr2, 1) != BIT(data, 1))
		LOG("%s: Non-integer number of bytes receive interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(m_dmr2, 2) != BIT(data, 2))
		LOG("%s: Collision abort detected interrupt %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_dmr2, 3) != BIT(data, 3))
		LOG("%s: FCS error interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(m_dmr2, 4) != BIT(data, 4))
		LOG("%s: Overflow error interrupt %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_dmr2, 5) != BIT(data, 5))
		LOG("%s: Underflow error interrupt %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(m_dmr2, 6) != BIT(data, 6))
		LOG("%s: Overrun error interrupt %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(m_dmr2, 7) != BIT(data, 7))
		LOG("%s: Underrun error interrupt %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");

	m_dmr2 = data;
}


//-------------------------------------------------
//  set_dmr3 - indirect write to D-channel mode
//  register 3
//-------------------------------------------------

void am79c30a_device::set_dmr3(u8 data)
{
	if (BIT(m_dmr3, 0) != BIT(data, 0))
		LOG("%s: Valid address/end of address interrupt %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(m_dmr3, 1) != BIT(data, 1))
		LOG("%s: End of valid transmit packet interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(m_dmr3, 2) != BIT(data, 2))
		LOG("%s: Last byte of received packet interrupt %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_dmr3, 3) != BIT(data, 3))
		LOG("%s: Receive byte available interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(m_dmr3, 4) != BIT(data, 4))
		LOG("%s: Last byte transmitted interrupt %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_dmr3, 5) != BIT(data, 5))
		LOG("%s: Transmit buffer available interrupt %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(m_dmr3, 6) != BIT(data, 6))
		LOG("%s: Received packet lost interrupt %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(m_dmr3, 7) != BIT(data, 7))
		LOG("%s: FCS transfer to FIFO %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");

	m_dmr3 = data;
}


//-------------------------------------------------
//  set_dmr4 - indirect write to D-channel mode
//  register 4
//-------------------------------------------------

void am79c30a_device::set_dmr4(u8 data)
{
	if ((m_dmr4 & 0x03) != (data & 0x03))
	{
		if ((data & 0x03) == 0)
			LOG("%s: Receiver threshold = 1 byte\n", machine().describe_context());
		else
			LOG("%s: Receiver threshold = %d/%d bytes\n", machine().describe_context(), 1 << (data & 0x03), std::min((data & 0x03) * 8 + 8, 30);
	}
	if ((m_dmr4 & 0x0c) != (data & 0x0c))
	{
		if ((data & 0x0c) == 0)
			LOG("%s: Transmitter threshold = 1 byte\n", machine().describe_context());
		else
			LOG("%s: Transmitter threshold = %d/%d bytes\n", machine().describe_context(), 1 << ((data & 0x0c) >> 2), (data & 0x0c) + 2);
	}
	if (BIT(m_dmr4, 4) != BIT(data, 4))
		LOG("%s: Interframe fill = %s idle\n", machine().describe_context(), BIT(data, 4) ? "flag" : "mark");
	if ((m_dmr4 & 0xa0) != (data & 0xa0))
	{
		if (BIT(data, 5))
			LOG("%s: Address recognition on %s received byte only\n", machine().describe_context(), BIT(data, 7) ? "second" : "first");
		else
			LOG("%s: 2-byte address recognition\n", machine().describe_context());
	}
	if (BIT(m_dmr4, 6) != BIT(data, 6))
		LOG("%s: FRAR bit 1 compare %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");

	m_dmr4 = data;
}


//-------------------------------------------------
//  dsr1_r - direct read from D-channel status
//  register 1
//-------------------------------------------------

u8 am79c30a_device::dsr1_r()
{
	u8 dsr1 = m_dsr1;

	// Bits 0, 1, 6, 7 cleared when read by microprocessor
	if (!machine().side_effects_disabled())
		m_dsr1 &= 0x3c;

	return dsr1;
}


//-------------------------------------------------
//  dsr2_r - direct read from D-channel status
//  register 2
//-------------------------------------------------

u8 am79c30a_device::dsr2_r()
{
	u8 dsr2 = m_dsr2;

	// Bits 0, 1, 3 cleared when read by microprocessor
	if (!machine().side_effects_disabled())
		m_dsr2 &= 0xf2;

	return dsr2;
}


//-------------------------------------------------
//  der_r - direct read from D-channel error
//  register
//-------------------------------------------------

u8 am79c30a_device::der_r()
{
	u8 der = m_der;

	// All bits cleared when read by microprocessor
	if (!machine().side_effects_disabled())
		m_der = 0;

	return der;
}


//-------------------------------------------------
//  set_efcr - indirect write to extended FIFO
//  control register
//-------------------------------------------------

void am79c30a_device::set_efcr(u8 data)
{
	if ((m_efcr & 0x78) != (data & 0x78))
	{
		if ((data & 0x78) == 0)
			LOG("%s: Analog sidetone = <inf>\n", machine().describe_context());
		else
			LOG("%s: Analog sidetone = %.1f dB\n", machine().describe_context(), (19 - ((data & 0x78) >> 3)) * 1.5);
	}
	if (BIT(m_efcr, 1) != BIT(data, 1))
		LOG("%s: Start of second received packet in FIFO interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(m_efcr, 0) != BIT(data, 0))
		LOG("%s: %s mode of FIFO operation\n", machine().describe_context(), BIT(data, 0) ? "Extended" : "Normal");
	if ((data & 0x84) != 0)
		logerror("%s: Writing %02XH to EFCR reserved bits\n", machine().describe_context(), data & 0x84);

	m_efcr = data;
}


//**************************************************************************
//  PERIPHERAL PORT (PP)
//**************************************************************************

//-------------------------------------------------
//  set_ppcr1 - indirect write to peripheral port
//  control register 1
//-------------------------------------------------

void am79c30a_device::set_ppcr1(u8 data)
{
	if (BIT(m_ppcr1, 7) != BIT(data, 7))
		LOG("%s: Monitor channel abort request %s\n", machine().describe_context(), BIT(data, 7) ? "on" : "off");
	if ((m_ppcr1 & 0x60) != (data & 0x60))
	{
		LOG("%s: Monitor channel %d %sabled\n", machine().describe_context(), (data & 0x20) >> 5, BIT(data, 6) ? "en" : "dis");
		if (!BIT(data, 6))
			m_ppsr &= 0xf0;
	}
	if (BIT(m_ppcr1, 4) != BIT(data, 4))
		LOG("%s: Monitor end-of-message request %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_ppcr1, 2) != BIT(data, 2))
		LOG("%s: IOM-2 activation/deactivation bit %sset\n", machine().describe_context(), BIT(data, 2) ? "" : "re");
	if ((m_ppcr1 & 0x03) != (data & 0x03))
	{
		if ((data & 0x03) == 0x00)
			LOG("%s: Peripheral port disabled\n", machine().describe_context());
		else if ((data & 0x03) == 0x01)
			LOG("%s: Serial Bus Port mode enabled\n", machine().describe_context());
		else
			LOG("%s: IOM-2 %s mode enabled\n", machine().describe_context(), BIT(data, 1) ? "slave" : "master");
	}

	m_ppcr1 = data;
}


//-------------------------------------------------
//  get_ppsr - indirect read from peripheral port
//  status register
//-------------------------------------------------

u8 am79c30a_device::get_ppsr()
{
	u8 ppsr = m_ppsr;

	// Bits 6, 3 and 2 are cleared when read by microprocessor
	if (!machine().side_effects_disabled())
		m_ppsr &= 0xb3;

	return ppsr;
}


//-------------------------------------------------
//  set_ppier - indirect write to peripheral port
//  interrupt enable register
//-------------------------------------------------

void am79c30a_device::set_ppier(u8 data)
{
	if (BIT(m_ppier, 0) != BIT(data, 0))
		LOG("%s: Monitor receive data available interrupt %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");
	if (BIT(m_ppier, 1) != BIT(data, 1))
		LOG("%s: Monitor transmit buffer available interrupt %sabled\n", machine().describe_context(), BIT(data, 1) ? "en" : "dis");
	if (BIT(m_ppier, 2) != BIT(data, 2))
		LOG("%s: Monitor EOM received interrupt %sabled\n", machine().describe_context(), BIT(data, 2) ? "en" : "dis");
	if (BIT(m_ppier, 3) != BIT(data, 3))
		LOG("%s: Monitor abort received interrupt %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if (BIT(m_ppier, 4) != BIT(data, 4))
		LOG("%s: C/I channel 0 data change interrupt %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_ppier, 5) != BIT(data, 5))
		LOG("%s: C/I channel 1 data change interrupt %sabled\n", machine().describe_context(), BIT(data, 5) ? "en" : "dis");
	if (BIT(m_ppier, 6) != BIT(data, 6))
		LOG("%s: IOM-2 timing request interrupt %sabled\n", machine().describe_context(), BIT(data, 6) ? "en" : "dis");
	if (BIT(m_ppier, 7) != BIT(data, 7))
		LOG("%s: Peripheral port/multiframing interrupt %sabled\n", machine().describe_context(), BIT(data, 7) ? "en" : "dis");

	m_ppier = data;
}


//-------------------------------------------------
//  set_mtdr - indirect write to monitor transmit
//  data register
//-------------------------------------------------

void am79c30a_device::set_mtdr(u8 data)
{
	// Clear Monitor Channel Transmit Buffer Available bit
	if (!machine().side_effects_disabled())
		m_ppsr &= 0xfd;

	m_mtdr = data;
}


//-------------------------------------------------
//  get_mrdr - indirect read from monitor receive
//  data register
//-------------------------------------------------

u8 am79c30a_device::get_mrdr()
{
	// Clear Monitor Channel Receive Buffer Available bit
	if (!machine().side_effects_disabled())
		m_ppsr &= 0xfe;

	return m_mrdr;
}


//-------------------------------------------------
//  set_citdr0 - indirect write to C/I transmit
//  data register 0
//-------------------------------------------------

void am79c30a_device::set_citdr0(u8 data)
{
	LOG("%s: C/I Transmit Data Register 0 = %02XH\n", machine().describe_context(), data);

	m_citdr0 = data;
}


//-------------------------------------------------
//  set_citdr1 - indirect write to C/I transmit
//  data register 1
//-------------------------------------------------

void am79c30a_device::set_citdr1(u8 data)
{
	LOG("%s: C/I Transmit Data Register 1 = %02XH\n", machine().describe_context(), data);

	m_citdr1 = data;
}


//-------------------------------------------------
//  set_ppcr2 - indirect write to peripheral port
//  control register 2
//-------------------------------------------------

void am79c30a_device::set_ppcr2(u8 data)
{
	if (BIT(m_ppcr2, 0) != BIT(data, 0))
		LOG("%s: SCLK inversion %sabled\n", machine().describe_context(), BIT(data, 0) ? "en" : "dis");

	// Hardware revision code is read-only
	m_ppcr2 = (data & 0x1f) | (m_ppcr2 & 0xe0);
}


//-------------------------------------------------
//  set_ppcr3 - indirect write to peripheral port
//  control register 3
//-------------------------------------------------

void am79c30a_device::set_ppcr3(u8 data)
{
	if (BIT(m_ppcr3, 4) != BIT(data, 4))
		LOG("%s: IOM-2 slave mode bus reversal %sabled\n", machine().describe_context(), BIT(data, 4) ? "en" : "dis");
	if (BIT(m_ppcr3, 3) != BIT(data, 3))
		LOG("%s: TIC bus %sabled\n", machine().describe_context(), BIT(data, 3) ? "en" : "dis");
	if ((m_ppcr3 & 0x07) != (data & 0x07))
		LOG("%s: TIC bus address = %d\n", machine().describe_context(), data & 0x07);

	m_ppcr3 = data;
}


//**************************************************************************
//  MICROPROCESSOR INTERFACE (MPI)
//**************************************************************************

//-------------------------------------------------
//  cr_w - write to command register
//-------------------------------------------------

void am79c30a_device::cr_w(u8 data)
{
	m_cr = data;
	m_byte_seq = 0;
}


//-------------------------------------------------
//  dr_r - read from data register
//-------------------------------------------------

u8 am79c30a_device::dr_r()
{
	u8 data = 0;

	switch (m_cr)
	{
	case 0x20:
		data = m_init2;
		break;

	case 0x21:
		data = m_init;
		break;

	case 0x41: case 0x42: case 0x43:
		data = m_mcr[m_cr - 0x41];
		break;

	case 0x44:
		data = m_mcr4;
		break;

	case 0x45:
		data = (m_byte_seq & 0x03) == 0x03 ? m_mcr4 : m_mcr[m_byte_seq & 0x03];
		break;

	case 0x61:
		data = (m_x_coeff[(m_byte_seq & 0x0e) >> 1] >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x62:
		data = (m_r_coeff[(m_byte_seq & 0x0e) >> 1] >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x63:
		data = (m_gx_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x64:
		data = (m_gr_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x65:
		data = (m_ger_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x66:
		data = (m_stgr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x67:
		data = m_ftgr[m_byte_seq & 1];
		break;

	case 0x68:
		data = m_atgr[m_byte_seq & 1];
		break;

	case 0x69:
		data = m_mmr1;
		break;

	case 0x6a:
		data = m_mmr2;
		break;

	case 0x6b:
		switch (m_byte_seq % 46)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
			data = (m_x_coeff[(m_byte_seq % 46) >> 1] >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
			data = (m_r_coeff[((m_byte_seq % 46) - 16) >> 1] >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 32: case 33:
			data = (m_gx_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 34: case 35:
			data = (m_gr_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 36: case 37:
			data = (m_ger_coeff >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 38: case 39:
			data = (m_stgr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 40: case 41:
			data = m_ftgr[m_byte_seq & 1];
			break;

		case 42: case 43:
			data = m_atgr[m_byte_seq & 1];
			break;

		case 44:
			data = m_mmr1;
			break;

		case 45:
			data = m_mmr2;
			break;
		}
		break;

	case 0x6c:
		data = m_mmr3;
		break;

	case 0x6d:
		data = m_stra;
		break;

	case 0x6e:
		data = m_strf;
		break;

	case 0x70:
		data = m_peakx;
		break;

	case 0x71:
		data = m_peakr;
		break;

	case 0x72:
		data = BIT(m_byte_seq, 0) ? m_peakr : m_peakx;
		break;

	case 0x81:
		data = m_frar[m_byte_seq % 3];
		break;

	case 0x82:
		data = m_srar[m_byte_seq % 3];
		break;

	case 0x83:
		data = (m_tar >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x84:
		data = (m_drlr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x85:
		data = (m_dtcr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x86:
		data = m_dmr1;
		break;

	case 0x87:
		data = m_dmr2;
		break;

	case 0x88:
		switch (m_byte_seq % 14)
		{
		case 0: case 1: case 2:
			data = m_frar[m_byte_seq % 14];
			break;

		case 3: case 4: case 5:
			data = m_srar[(m_byte_seq % 14) - 3];
			break;

		case 6: case 7:
			data = (m_tar >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 8: case 9:
			data = (m_drlr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 10: case 11:
			data = (m_dtcr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
			break;

		case 12:
			data = m_dmr1;
			break;

		case 13:
			data = m_dmr2;
			break;
		}
		break;

	case 0x89:
		data = (m_drcr >> (BIT(m_byte_seq, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x8a: case 0x8b:
		data = (m_rngr >> (BIT(m_cr, 0) ? 8 : 0)) & 0xff;
		break;

	case 0x8c:
		data = m_frar[3];
		break;

	case 0x8d:
		data = m_srar[3];
		break;

	case 0x8e:
		data = m_dmr3;
		break;

	case 0x8f:
		data = m_dmr4;
		break;

	case 0x90:
		switch (m_byte_seq & 0x03)
		{
		case 0:
			data = m_frar[3];
			break;

		case 1:
			data = m_srar[3];
			break;

		case 2:
			data = m_dmr3;
			break;

		case 3:
			data = m_dmr4;
			break;
		}
		break;

	case 0x91:
		data = m_asr;
		break;

	case 0x92:
		data = m_efcr;
		break;

	case 0xa1:
		data = m_lsr;
		break;

	case 0xa2:
		data = m_lpr;
		break;

	case 0xa3:
		data = m_lmr1;
		break;

	case 0xa4:
		data = m_lmr2;
		break;

	case 0xa5:
		switch (m_byte_seq % 3)
		{
		case 0:
			data = m_lpr;
			break;

		case 1:
			data = m_lmr1;
			break;

		case 2:
			data = m_lmr2;
			break;
		}
		break;

	case 0xa6:
		data = m_mf;
		break;

	case 0xa7:
		data = m_mfsb;
		break;

	case 0xc0:
		data = m_ppcr1;
		break;

	case 0xc1:
		data = get_ppsr();
		break;

	case 0xc2:
		data = m_ppier;
		break;

	case 0xc3:
		data = get_mrdr();
		break;

	case 0xc4:
		data = m_cirdr0;
		break;

	case 0xc5:
		data = m_cirdr1;
		break;

	case 0xc8:
		data = m_ppcr2;
		break;

	case 0xc9:
		data = m_ppcr3;
		break;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Indirect read from unimplemented/write-only register %02XH\n", machine().describe_context(), m_cr);
		break;
	}

	if (!machine().side_effects_disabled())
		m_byte_seq++;
	return data;
}


//-------------------------------------------------
//  dr_w - write to data register
//-------------------------------------------------

void am79c30a_device::dr_w(u8 data)
{
	switch (m_cr)
	{
	case 0x20:
		set_init2(data);
		break;

	case 0x21:
		set_init(data);
		break;

	case 0x41: case 0x42: case 0x43:
		set_mcr(m_cr - 0x41, data);
		break;

	case 0x44:
		set_mcr4(data);
		break;

	case 0x45:
		if ((m_byte_seq & 0x03) == 0x03)
			set_mcr4(data);
		else
			set_mcr(m_byte_seq & 0x03, data);
		break;

	case 0x61:
		set_x_coeff((m_byte_seq & 0x0e) >> 1, data, BIT(m_byte_seq, 0));
		break;

	case 0x62:
		set_r_coeff((m_byte_seq & 0x0e) >> 1, data, BIT(m_byte_seq, 0));
		break;

	case 0x63:
		set_gx_coeff(data, BIT(m_byte_seq, 0));
		break;

	case 0x64:
		set_gr_coeff(data, BIT(m_byte_seq, 0));
		break;

	case 0x65:
		set_ger_coeff(data, BIT(m_byte_seq, 0));
		break;

	case 0x66:
		set_stgr(data, BIT(m_byte_seq, 0));
		break;

	case 0x67:
		set_ftgr(m_byte_seq & 1, data);
		break;

	case 0x68:
		set_atgr(m_byte_seq & 1, data);
		break;

	case 0x69:
		set_mmr1(data);
		break;

	case 0x6a:
		set_mmr2(data);
		break;

	case 0x6b:
		switch (m_byte_seq % 46)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
			set_x_coeff((m_byte_seq % 46) >> 1, data, BIT(m_byte_seq, 0));
			break;

		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
			set_r_coeff(((m_byte_seq % 46) - 16) >> 1, data, BIT(m_byte_seq, 0));
			break;

		case 32: case 33:
			set_gx_coeff(data, BIT(m_byte_seq, 0));
			break;

		case 34: case 35:
			set_gr_coeff(data, BIT(m_byte_seq, 0));
			break;

		case 36: case 37:
			set_ger_coeff(data, BIT(m_byte_seq, 0));
			break;

		case 38: case 39:
			set_stgr(data, BIT(m_byte_seq, 0));
			break;

		case 40: case 41:
			set_ftgr(m_byte_seq & 1, data);
			break;

		case 42: case 43:
			set_atgr(m_byte_seq & 1, data);
			break;

		case 44:
			set_mmr1(data);
			break;

		case 45:
			set_mmr2(data);
			break;
		}
		break;

	case 0x6c:
		set_mmr3(data);
		break;

	case 0x6d:
		set_stra(data);
		break;

	case 0x6e:
		set_strf(data);
		break;

	case 0x81:
		set_frar(m_byte_seq % 3, data);
		break;

	case 0x82:
		set_srar(m_byte_seq % 3, data);
		break;

	case 0x83:
		set_tar(data, BIT(m_byte_seq, 0));
		break;

	case 0x84:
		set_drlr(data, BIT(m_byte_seq, 0));
		break;

	case 0x85:
		set_dtcr(data, BIT(m_byte_seq, 0));
		break;

	case 0x86:
		set_dmr1(data);
		break;

	case 0x87:
		set_dmr2(data);
		break;

	case 0x88:
		switch (m_byte_seq % 14)
		{
		case 0: case 1: case 2:
			set_frar(m_byte_seq % 14, data);
			break;

		case 3: case 4: case 5:
			set_srar((m_byte_seq % 14) - 3, data);
			break;

		case 6: case 7:
			set_tar(data, BIT(m_byte_seq, 0));
			break;

		case 8: case 9:
			set_drlr(data, BIT(m_byte_seq, 0));
			break;

		case 10: case 11:
			set_dtcr(data, BIT(m_byte_seq, 0));
			break;

		case 12:
			set_dmr1(data);
			break;

		case 13:
			set_dmr2(data);
			break;
		}
		break;

	case 0x8a: case 0x8b:
		set_rngr(data, BIT(m_cr, 0));
		break;

	case 0x8c:
		set_frar(3, data);
		break;

	case 0x8d:
		set_srar(3, data);
		break;

	case 0x8e:
		set_dmr3(data);
		break;

	case 0x8f:
		set_dmr4(data);
		break;

	case 0x90:
		switch (m_byte_seq & 0x03)
		{
		case 0:
			set_frar(3, data);
			break;

		case 1:
			set_srar(3, data);
			break;

		case 2:
			set_dmr3(data);
			break;

		case 3:
			set_dmr4(data);
			break;
		}
		break;

	case 0x92:
		set_efcr(data);
		break;

	case 0xa2:
		set_lpr(data);
		break;

	case 0xa3:
		set_lmr1(data);
		break;

	case 0xa4:
		set_lmr2(data);
		break;

	case 0xa5:
		switch (m_byte_seq % 3)
		{
		case 0:
			set_lpr(data);
			break;

		case 1:
			set_lmr1(data);
			break;

		case 2:
			set_lmr2(data);
			break;
		}
		break;

	case 0xa6:
		set_mf(data);
		break;

	case 0xa8:
		set_mfqb(data);
		break;

	case 0xc0:
		set_ppcr1(data);
		break;

	case 0xc2:
		set_ppier(data);
		break;

	case 0xc3:
		set_mtdr(data);
		break;

	case 0xc4:
		set_citdr0(data);
		break;

	case 0xc5:
		set_citdr1(data);
		break;

	case 0xc8:
		set_ppcr2(data);
		break;

	case 0xc9:
		set_ppcr3(data);
		break;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Indirect write of %02XH to unimplemented/read-only register %02XH\n", machine().describe_context(), data, m_cr);
		break;
	}

	if (!machine().side_effects_disabled())
		m_byte_seq++;
}


//-------------------------------------------------
//  bbrb_r - direct read from Bb-channel receive
//  buffer
//-------------------------------------------------

u8 am79c30a_device::bbrb_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading from BBRB\n", machine().describe_context());

	return 0;
}


//-------------------------------------------------
//  bbtb_w - direct write to Bb-channel transmit
//  buffer
//-------------------------------------------------

void am79c30a_device::bbtb_w(u8 data)
{
	logerror("%s: Writing %02XH to BBTB\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  bcrb_r - direct read from Bc-channel receive
//  buffer
//-------------------------------------------------

u8 am79c30a_device::bcrb_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading from BCRB\n", machine().describe_context());

	return 0;
}


//-------------------------------------------------
//  bctb_w - direct write to Bc-channel transmit
//  buffer
//-------------------------------------------------

void am79c30a_device::bctb_w(u8 data)
{
	logerror("%s: Writing %02XH to BCTB\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  read - microprocessor direct read
//-------------------------------------------------

u8 am79c30a_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case 0:
		return ir_r();

	case 1:
		return dr_r();

	case 2:
		return dsr1_r();

	case 3:
		return der_r();

	case 4:
		return dcrb_r();

	case 5:
		return bbrb_r();

	case 6:
		return bcrb_r();

	case 7:
		return dsr2_r();

	default: // can't happen despite what compilers believe
		return 0;
	}
}


//-------------------------------------------------
//  write - microprocessor direct write
//-------------------------------------------------

void am79c30a_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case 0:
		cr_w(data);
		break;

	case 1:
		dr_w(data);
		break;

	case 4:
		dctb_w(data);
		break;

	case 5:
		bbtb_w(data);
		break;

	case 6:
		bctb_w(data);
		break;

	default:
		logerror("%s: Writing %02XH to unknown/reserved address %d\n", machine().describe_context(), data, offset & 7);
		break;
	}
}
