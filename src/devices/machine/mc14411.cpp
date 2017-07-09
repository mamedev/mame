// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*********************************************************************

    mc14411.cpp

    Motorola Bit Rate Generator

 This device assumed a fixed rate clock/crystal and does not support 
 changing it through pin 21. All other features are implemented.

*********************************************************************/

#include "emu.h"
#include "mc14411.h"

/***************************************************************************
    MACROS
***************************************************************************/

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h 
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE  (LOG_GENERAL|LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h 
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64d"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/
//    0/0    0/1    1/0    1/1   RSB/RSA
//     X1     X8    X16    X64
const int mc14411_device::counter_divider[16][4] = {
  {   192,    24,    12,     3 }, // F1
  {   256,    32,    16,     4 }, // F2
  {   384,    48,    24,     6 }, // F3
  {   512,    64,    32,     8 }, // F4
  {   768,    96,    48,    12 }, // F5
  {  1024,   128,    64,    16 }, // F6
  {  1536,   192,    96,    24 }, // F7
  {  3072,   384,   192,    48 }, // F8
  {  6144,   768,   384,    96 }, // F9
  {  9216,  1152,   576,   144 }, // F10
  { 12288,  1536,   768,   192 }, // F11
  { 13696,  1712,   856,   214 }, // F12
  { 16768,  2096,  1048,   262 }, // F13
  { 24576,  3072,  1536,   384 }, // F14
  {     2,     2,     2,     2 }, // F15
  {     1,     1,     1,     1 }  // F16
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
DEFINE_DEVICE_TYPE(MC14411, mc14411_device, "mc14411", "MC14411 BRG")

/***************************************************************************
    LIVE DEVICE
***************************************************************************/
mc14411_device::mc14411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc14411_device(mconfig, MC14411, tag, owner, clock)
{
}

mc14411_device::mc14411_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_fx_cbs{*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this }
	, m_divider(0)
	, m_reset(CLEAR_LINE)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void mc14411_device::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);

	memset(m_fx_timer, '\0', sizeof(m_fx_timer));
	for (int i = F1; i <= F16; i++)
	{
		m_out_fx_cbs[i].resolve();
		if (!m_out_fx_cbs[i].isnull()) m_fx_timer[i] = timer_alloc(i);
	}
	
	save_item(NAME(m_divider));
	save_item(NAME(m_reset));

	m_reset_timer = timer_alloc(TIMER_ID_RESET);

}

//------------------------------------------------------------------------
// device_reset - is called by the mame framework or by the owning device 
//   driver or by ASSERTING the reset line through set_reset_line
//------------------------------------------------------------------------
void mc14411_device::device_reset()
{
	LOGSETUP("%s\n", FUNCNAME);

	for (int i = F1; i <= F16; i++)
	{
		if (!m_out_fx_cbs[i].isnull())
		{
			// Reset line according to datasheet and remember it for transitions to come 
			(m_out_fx_cbs[i])(m_fx_state[i] = (i < F15 ? 0 : 1));

			// Arm the timer based on the selected divider and the crystal value 
			double hz = clock()/counter_divider[i][m_divider] * 2; // 2 flanks per cycle
			m_fx_timer[i]->adjust(attotime::from_hz(hz), i, attotime::from_hz(hz));
			LOGSETUP(" - arming timer for F%d at %fHz\n", i + 1, hz);
		}
	}

	if (m_reset == ASSERT_LINE)
	{
		m_reset_timer->adjust(attotime::from_nsec((double)900), TIMER_ID_RESET, attotime::from_nsec((double)900));
	}
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void mc14411_device::device_timer (emu_timer &timer, device_timer_id id, int32_t param, void *ptr)
{
	if (id >= F1 && id <= F16)
	{
		(m_out_fx_cbs[id])(m_fx_state[id]++ & 1);
	}
	else if (id == TIMER_ID_RESET)
	{
		// NOTE: This check could be triggered by either faulty hardware design or non accurate emulation so is just informative if the reset line is handled
		//       explicitelly instead of relying on calling device_reset
		if (!(m_reset == ASSERT_LINE))
		{
			LOG("Reset pulse is too short, should be 900nS minimum");
			logerror("Reset pulse is too short, should be 900nS minimum");
		}
	}
	else
	{
		LOG("Unhandled Timer ID %d\n", id);
	}
}

//--------------------------------------------------------
// rate_select_w - implements the RSA and RSB input pins
// TODO: Needs to check real device behaviour how changing 
//       divider at run time affects wave forms
//--------------------------------------------------------
WRITE8_MEMBER( mc14411_device::rate_select_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, data);

	m_divider = data & 3;

	for (int i = F1; i <= F16; i++)
	{
		if (!m_out_fx_cbs[i].isnull())
		{
			// Re-arm the timer based on the new selected divider and the crystal value 
			double hz = clock()/counter_divider[i][m_divider] * 2; // 2 flanks per cycle
			m_fx_timer[i]->adjust(attotime::from_hz(hz), i, attotime::from_hz(hz));
			LOGSETUP(" - Re-arming timer for F%d at %fHz\n", i + 1, hz);
		}
	}
}

//------------------------------------------------
// reset_w - implements software controlled reset
//------------------------------------------------
WRITE_LINE_MEMBER( mc14411_device::reset_w)
{
	LOGSETUP("%s %02x\n", FUNCNAME, state);

	m_reset = state;
	
	if (m_reset == ASSERT_LINE)
	{
		LOGSETUP(" - Asserting reset\n");
		device_reset();
	}
	else
	{
		LOGSETUP(" - Clearing reset\n");
	}
}
