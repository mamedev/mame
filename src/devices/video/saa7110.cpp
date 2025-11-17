// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SAA7110/SAA7110A One Chip Front-end 1

TV decoder, can decode PAL B/G, NTSC M and SECAM

TODO:
- Stub, barely enough to make i2c tests happy in ZR36057 core;
- SA configuration pin;
- How to read transmitter regs? Should be slave addresses 0x9d (SA=0) or 0x9f (SA=1),
  ZR36057 doesn't seem to care for now, '60 eventually will?
- Needs an actual multimedia source to continue;

**************************************************************************************************/

#include "emu.h"
#include "saa7110.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SAA7110A, saa7110a_device, "saa7110a", "SAA7110A OCF1")

// TODO: pin address overridable with SA pin = 1 (0x9e >> 1)
saa7110a_device::saa7110a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA7110A, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0x9c >> 1)
	, device_memory_interface(mconfig, *this)
	//, m_out_vs_cb(*this)
{
	m_space_config = address_space_config("regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(saa7110a_device::regs_map), this));
}

void saa7110a_device::device_start()
{
	//m_href_timer = timer_alloc(FUNC(saa7110a_device::href_cb), this);
	save_item(NAME(m_secs));
	save_item(NAME(m_sstb));
	save_item(NAME(m_hrmv));
	save_item(NAME(m_rtse));
	save_item(NAME(m_vtrc));
}

void saa7110a_device::device_reset()
{
	m_secs = m_sstb = m_hrmv = m_rtse = m_vtrc = false;
	//m_current_href = 0;
	//m_href_timer->adjust(attotime::from_hz(15734), 0, attotime::from_hz(15734));
}

device_memory_interface::space_config_vector saa7110a_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

u8 saa7110a_device::read_data(u16 offset)
{
	// OCF1 transmitter regs
	// IC version number, not necessarily device ID?
	if (m_sstb == false)
	{
		LOG("OCF1 transmitter: read IC version number (check me)\n");
		machine().debug_break();
		return 0x01;
	}
	// TODO: status byte
	// x--- ---- STTC horizontal time constant (0) TV time constant (1) VCR time constant
	// -x-- ---- HLCK locked horizontal frequency (1) unlocked
	// --x- ---- FIDT Identification bit for detected field frequency (0) 50 Hz (1) 60 Hz
	// ---x ---- GLIM Gain value for active luminance limit
	// ---- -x-- WIPA White peak loop activation
	// ---- --x- ALTD line alternating colour burst detection (PAL/SECAM)
	// ---- ---x CODE color signal has been detected
	LOG("OCF1 transmitter: read status byte\n");
	return 0x00;
}

void saa7110a_device::write_data(u16 offset, u8 data)
{
	// OCF1 receiver regs
	space(0).write_byte(offset, data);
}

void saa7110a_device::regs_map(address_map &map)
{
	// BMSD-SQP + BSC slave receiver
	map(0x00, 0x00).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU00: Increment delay %02x\n", data); }));
	map(0x01, 0x01).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU01: HSY begin 50 Hz %02x\n", data); }));
	map(0x02, 0x02).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU02: HSY stop 50 Hz %02x\n", data); }));
	map(0x03, 0x03).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU03: HCL begin 50 Hz %02x\n", data); }));
	map(0x04, 0x04).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU04: HCL stop 50 Hz %02x\n", data); }));
	map(0x05, 0x05).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU05: HSY after PHI1 50 Hz %02x\n", data); }));
	map(0x06, 0x06).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU06: Luminance control %02x\n", data); }));
	map(0x07, 0x07).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU07: Hue control %02x\n", data); }));
	map(0x08, 0x08).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU08: Colour killer threshold QUAM (PAL/NTSC) %02x\n", data); }));
	map(0x09, 0x09).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU09: Colour killer threshold SECAM %02x\n", data); }));
	map(0x0a, 0x0a).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU0A: PAL switch sensitivity %02x\n", data); }));
	map(0x0b, 0x0b).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU0B: SECAM switch sensitivity %02x\n", data); }));
	map(0x0c, 0x0c).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU0C: Gain control chrominance %02x\n", data); }));
	map(0x0d, 0x0d).lw8(NAME([this] (offs_t offset, u8 data) {
		LOG("SU0D: Standard/mode control %02x\n", data);
		m_secs = BIT(data, 0);
		m_sstb = BIT(data, 1);
		m_hrmv = BIT(data, 2);
		m_rtse = BIT(data, 3);
		m_vtrc = BIT(data, 7);
		LOG("\tSECAM mode bit SECS = %d: %s\n", m_secs, m_secs ? "SECAM mode" : "Other standards");
		LOG("\tStatus byte select SSTB = %d\n", m_sstb);
		LOG("\tHREF position select HRMV = %d: %s\n", m_hrmv, m_hrmv ? "HREF position 8 LLC2 later (SAA7191 mode)" : "HREF normal position");
		LOG("\tReal time outputs mode select RTSE = %d: %s\n", m_rtse, m_rtse ? "HL pin 39, VL pin 40" : "PLIN pin 39, ODD pin 40");
		LOG("\tTV/VCR mode select VTRC = %d: %s\n", m_vtrc, m_vtrc ? "VTR mode" : "TV mode");
	}));
	map(0x0e, 0x0e).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU0E: I/O and clock control %02x\n", data); }));
	map(0x0f, 0x0f).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU0F: Control #1 %02x\n", data); }));
	map(0x10, 0x10).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU10: Control #2 %02x\n", data); }));
	map(0x11, 0x11).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU11: Chrominance gain reference %02x\n", data); }));
	map(0x12, 0x12).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU12: Chrominance saturation %02x\n", data); }));
	map(0x13, 0x13).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU13: Luminance contrast %02x\n", data); }));
	map(0x14, 0x14).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU14: HSY begin 60 Hz %02x\n", data); }));
	map(0x15, 0x15).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU15: HSY stop 60 Hz %02x\n", data); }));
	map(0x16, 0x16).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU16: HCL begin 60 Hz %02x\n", data); }));
	map(0x17, 0x17).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU17: HCL stop 60 Hz %02x\n", data); }));
	map(0x18, 0x18).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU18: HSY after PHI1 60 Hz %02x\n", data); }));
	map(0x19, 0x19).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU19: Luminance brightness %02x\n", data); }));

	// DUAD slave receiver
	map(0x20, 0x20).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU20: Analog control #1 %02x\n", data); }));
	map(0x21, 0x21).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU21: Analog control #2 %02x\n", data); }));
	map(0x22, 0x22).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU22: Mixer control #1 %02x\n", data); }));
	map(0x23, 0x23).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU23: Clamping level control 21 %02x\n", data); }));
	map(0x24, 0x24).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU24: Clamping level control 22 %02x\n", data); }));
	map(0x25, 0x25).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU25: Clamping level control 31 %02x\n", data); }));
	map(0x26, 0x26).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU26: Clamping level control 32 %02x\n", data); }));
	map(0x27, 0x27).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU27: Gain control analog #1 %02x\n", data); }));
	map(0x28, 0x28).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU28: White peak control %02x\n", data); }));
	map(0x29, 0x29).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU29: Sync bottom control %02x\n", data); }));
	map(0x2a, 0x2a).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU2A: Gain control analog #2 %02x\n", data); }));
	map(0x2b, 0x2b).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU2B: Gain control analog #3 %02x\n", data); }));
	map(0x2c, 0x2c).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU2C: Mixer control #2 %02x\n", data); }));
	map(0x2d, 0x2d).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU2D: Integration value gain %02x\n", data); }));
	map(0x2e, 0x2e).lw8(NAME([this] (offs_t offset, u8 data) {
		m_vbps = data;
		LOG("SU2E: Vertical blanking pulse set %02x\n", data);
	}));
	map(0x2f, 0x2f).lw8(NAME([this] (offs_t offset, u8 data) {
		m_vbpr = data;
		LOG("SU2F: Vertical blanking pulse reset %02x\n", data);
	}));
	map(0x30, 0x30).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU30: ADCs gain control %02x\n", data); }));
	map(0x31, 0x31).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU31: Mixer control #3 %02x\n", data); }));
	map(0x32, 0x32).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU32: Integration value white peak %02x\n", data); }));
	map(0x33, 0x33).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU33: Mixer control #4 %02x\n", data); }));
	map(0x34, 0x34).lw8(NAME([this] (offs_t offset, u8 data) { LOG("SU34: Gain update level %02x\n", data); }));
}

//TIMER_CALLBACK_MEMBER(saa7110a_device::href_cb)
//{
//  if (m_vbpr == m_vbps)
//      return;
//  m_current_href ++;
//  if (m_current_href == m_vbps * 2)
//  {
//      m_out_vs_cb(1);
//  }
//  if (m_current_href == m_vbpr * 2)
//  {
//      m_out_vs_cb(0);
//  }
//  m_current_href %= 262;
//}
//
