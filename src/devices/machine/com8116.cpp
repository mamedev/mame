// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COM8116 Dual Baud Rate Generator (Programmable Divider) emulation

**********************************************************************/

#include "emu.h"
#include "com8116.h"

#define LOG_SELECTED (1 << 0)
#define LOG_TABLE (1 << 1)
//#define VERBOSE (LOG_TABLE)
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(COM8116, com8116_device, "com8116", "COM8116 Dual BRG")
DEFINE_DEVICE_TYPE(COM8116_003, com8116_003_device, "com8116_003", "COM8116-003 Dual BRG")
DEFINE_DEVICE_TYPE(COM5016_5, com5016_5_device, "com5016_5", "COM5016-5 Dual BRG")
DEFINE_DEVICE_TYPE(COM5016_013, com5016_013_device, "com5016_013", "COM5016-013 Dual BRG")
DEFINE_DEVICE_TYPE(COM8116_020, com8116_020_device, "com8116_020", "COM8116-020 Dual BRG")
DEFINE_DEVICE_TYPE(K1135AB, k1135ab_device, "k1135ab", "K1135A/B Dual BRG")

// Parts with T after the number do not have an internal oscillator and require an external clock source
// The SMC/COM 5xxx parts are all dual 5v/12v parts, while the 8xxx parts are 5v only

// SMC/COM5016(T) with no dash, aka 'STD' part on the datasheet
// Also COM8116(T)/8126(T)/8136(T)/8146(T) and Synertek sy2661-3 and GI AY-5-8116(T)-000 and GI AY-5-8136(T)-000 and WD WD-1943-00 (aka WD8136-00)
// and Motorola K1135A/B (K1135B outputs undivided 5.0688 MHz on pin 1)
// SMC/COM8156(T) is the same chip but clocked twice as fast and 32x clocks per baud instead of 16x
// baud rates are 50, 75, 110, 134.5, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19200
const int com8116_device::divisors_16X_5_0688MHz[16] =
	{ 6336, 4224, 2880, 2355, 2112, 1056, 528, 264, 176, 158, 132, 88, 66, 44, 33, 16 };

// SMC/COM8116-003 and WD WD-1943-03
// from http://www.vintagecomputer.net/fjkraan/comp/divcomp/doc/SMC_BaudGen.pdf page 283 (pdf page 20)
// baud rates are 50, 75, 110, 134.5, 150, 200, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 9600, 19200
const int com8116_device::divisors_16X_6_01835MHz[16] =
	{ 7523, 5015, 3420, 2797, 2508, 1881, 1254, 627, 313, 209, 188, 157, 104, 78, 39, 20 };

// SMC/COM5016(T)-5 and WD WD-1943-05; Synertek SY2661-1 and 2 are NOT the same as this despite using same clock speed
// SMC/COM8156(T)-5 is the same chip but clocked twice as fast and 32x clocks per baud instead of 16x
// Motorola K1135C is similar, with undivided 4.9152 MHz on pin 1
// baud rates are 50, 75, 110, 134.5, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19200
const int com8116_device::divisors_16X_4_9152MHz[16] =
	{ 6144, 4096, 2793, 2284, 2048, 1024, 512, 256, 171, 154, 128, 85, 64, 43, 32, 16 };

// SMC/COM5016(T)-6 and WD WD-1943-06
// baud rates are 50, 75, 110, 134.5, 150, 200, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 19200
const int com8116_device::divisors_32X_5_0688MHz[16] =
	{ 3168, 2112, 1440, 1177, 1056, 792, 528, 264, 132, 88, 66, 44, 33, 22, 17, 8 };

// SMC/COM5016(T)-013 and WD WD-1943-02 (from http://bitsavers.informatik.uni-stuttgart.de/pdf/dec/terminal/vt100/EK-VT100-TM-003_VT100_Technical_Manual_Jul82.pdf page 4-27 (pdf page 78))
// and from http://www.vintagecomputer.net/fjkraan/comp/divcomp/doc/SMC_BaudGen.pdf page 283 (pdf page 20)
// SMC/COM5106-013A and WD WD-1943-04 are the same chip clocked twice as fast, but with 32x clocks per baud instead of 16x
// baud rates are 50, 75, 110, 134.5, 150, 200, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 9600, 19200
const int com8116_device::divisors_16X_2_7648MHz[16] =
	{ 3456, 2304, 1571, 1285, 1152, 864, 576, 288, 144, 96, 86, 72, 48, 36, 18, 9 };

// SMC/COM8116T-020 (corrected from http://bitsavers.org/pdf/visualTechnology/Visual_100/MM-100-001-0A_Visual_100_and_Visual_400_Video_Display_Terminal_Maintenance_Manual_Sep1980.pdf (pdf page 99))
// (75, 134.5 and 2000 baud divisors given as 1,807, 856 and 57, which do not match stated actual frequencies)
// baud rates are 50, 75, 110, 134.5, 150, 200, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 9600, 19200
const int com8116_device::divisors_16X_1_8432MHz[16] =
	{ 2304, 1536, 1047, 857, 768, 576, 384, 192, 96, 64, 58, 48, 32, 24, 12, 6 };

// SMC/COM5026(T)-030 (non-standard serial rates, from http://bitsavers.informatik.uni-stuttgart.de/pdf/standardMicrosystems/_dataBooks/1979_StandardMicrosystems.pdf page 135)
const int com8116_device::divisors_16X_5_0688MHz_030[16] =
	{ 731, 733, 735, 737, 741, 743, 745, 751, 6970, 5569, 5433, 4752, 4269, 1920, 1584, 301 };

// SMC/COM5036(T)-080 (from http://bitsavers.informatik.uni-stuttgart.de/pdf/standardMicrosystems/_dataBooks/1979_StandardMicrosystems.pdf page 135)
const int com8116_device::divisors_16X_4_6080MHz[16] =
	{ 5760, 3840, 2618, 2141, 1920, 960, 480, 240, 160, 144, 120, 80, 60, 40, 30, 15 };

// COM8046 combines the -6 and STD tables into one device as a 32-entry table

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  com8116_device - constructor
//-------------------------------------------------

com8116_device::com8116_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const int *divisors) :
	device_t(mconfig, type, tag, owner, clock),
	m_fx4_handler(*this),
	m_fr_handler(*this),
	m_ft_handler(*this),
	m_divisors(divisors)
{
}

com8116_device::com8116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, COM8116, tag, owner, clock, divisors_16X_5_0688MHz)
{
}

com8116_003_device::com8116_003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, COM8116_003, tag, owner, clock, divisors_16X_6_01835MHz)
{
}

com5016_5_device::com5016_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, COM5016_5, tag, owner, clock, divisors_16X_4_9152MHz)
{
}

com5016_013_device::com5016_013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, COM5016_013, tag, owner, clock, divisors_16X_2_7648MHz)
{
}

com8116_020_device::com8116_020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, COM8116_020, tag, owner, clock, divisors_16X_1_8432MHz)
{
}

k1135ab_device::k1135ab_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	com8116_device(mconfig, K1135AB, tag, owner, clock, divisors_16X_5_0688MHz)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void com8116_device::device_start()
{
	// resolve callbacks
	m_fx4_handler.resolve();
	m_fr_handler.resolve_safe();
	m_ft_handler.resolve_safe();

	// allocate timers
	if (!m_fx4_handler.isnull())
	{
		m_fx4_timer = timer_alloc(TIMER_FX4);
		m_fx4_timer->adjust(attotime::from_hz((clock() / 4) * 2), 0, attotime::from_hz((clock() / 4)) * 2);
	}
	m_fr_timer = timer_alloc(TIMER_FR);
	m_ft_timer = timer_alloc(TIMER_FT);

	for (int i = 0; i < 16; i++)
		LOGMASKED(LOG_TABLE, "Output Frequency %01X: 16X %f Hz\n", i, double(clock()) / m_divisors[i] / 16.0);

	// register for state saving
	save_item(NAME(m_fx4));
	save_item(NAME(m_fr));
	save_item(NAME(m_ft));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void com8116_device::device_reset()
{
	m_fx4 = 0;
	m_fr = 0;
	m_ft = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void com8116_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_FX4:
		m_fx4 = !m_fx4;
		m_fx4_handler(m_fx4);
		break;

	case TIMER_FR:
		m_fr = !m_fr;
		m_fr_handler(m_fr);
		break;

	case TIMER_FT:
		m_ft = !m_ft;
		m_ft_handler(m_ft);
		break;
	}
}


//-------------------------------------------------
//  str_w -
//-------------------------------------------------

void com8116_device::str_w(uint8_t data)
{
	int fr_divider = data & 0x0f;
	int fr_clock = clock() / m_divisors[fr_divider];

	LOGMASKED(LOG_SELECTED, "Receiver Divisor Select %01X: %u (%u Hz)\n", data & 0x0f, m_divisors[fr_divider], fr_clock);

	m_fr_timer->adjust(attotime::from_nsec(3500), 0, attotime::from_hz(fr_clock * 2));
}


//-------------------------------------------------
//  stt_w -
//-------------------------------------------------

void com8116_device::stt_w(uint8_t data)
{
	int ft_divider = data & 0x0f;
	int ft_clock = clock() / m_divisors[ft_divider];

	LOGMASKED(LOG_SELECTED, "Transmitter Divisor Select %01X: %u (%u Hz)\n", data & 0x0f, m_divisors[ft_divider], ft_clock);

	m_ft_timer->adjust(attotime::from_nsec(3500), 0, attotime::from_hz(ft_clock * 2));
}


//-------------------------------------------------
//  str_stt_w -
//-------------------------------------------------

void com8116_device::str_stt_w(uint8_t data)
{
	str_w(data >> 4);
	stt_w(data & 0x0f);
}


//-------------------------------------------------
//  stt_str_w -
//-------------------------------------------------

void com8116_device::stt_str_w(uint8_t data)
{
	stt_w(data >> 4);
	str_w(data & 0x0f);
}
