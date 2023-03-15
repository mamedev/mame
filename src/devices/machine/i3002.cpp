// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/**********************************************************************

    i3002.cpp

    Intel 3002 Central Processing Element

**********************************************************************/

#include "emu.h"
#include "i3002.h"

// Device type definition
DEFINE_DEVICE_TYPE(I3002, i3002_device, "i3002", "Intel i3002 CPE")

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Table to decode bits 0..3 of FC into RG and register index
struct fc_fields {
	uint8_t rg; // Register group [0..2] ("I" , "II" , "III" in Intel docs)
	unsigned reg;   // Register index
};

constexpr std::array<struct fc_fields , 16> fc_field_tab =
	{{
	  { 0 , i3002_device::REG_R0 }, // 0000
	  { 0 , i3002_device::REG_R1 }, // 0001
	  { 0 , i3002_device::REG_R2 }, // 0010
	  { 0 , i3002_device::REG_R3 }, // 0011
	  { 0 , i3002_device::REG_R4 }, // 0100
	  { 0 , i3002_device::REG_R5 }, // 0101
	  { 0 , i3002_device::REG_R6 }, // 0110
	  { 0 , i3002_device::REG_R7 }, // 0111
	  { 0 , i3002_device::REG_R8 }, // 1000
	  { 0 , i3002_device::REG_R9 }, // 1001
	  { 1 , i3002_device::REG_T },  // 1010
	  { 1 , i3002_device::REG_AC }, // 1011
	  { 0 , i3002_device::REG_T },  // 1100
	  { 0 , i3002_device::REG_AC }, // 1101
	  { 2 , i3002_device::REG_T },  // 1110
	  { 2 , i3002_device::REG_AC }  // 1111
	}};

// Register names
static const std::array<const char* , i3002_device::REG_COUNT> reg_names =
	{{
	  "R0",
	  "R1",
	  "R2",
	  "R3",
	  "R4",
	  "R5",
	  "R6",
	  "R7",
	  "R8",
	  "R9",
	  "T",
	  "AC",
	  "MAR"
	}};

i3002_device::i3002_device(const machine_config &mconfig , const char *tag , device_t *owner , uint32_t clock)
	: device_t(mconfig , I3002 , tag , owner , clock)
	, m_co_handler(*this)
	, m_ro_handler(*this)
	, m_ibus_handler(*this)
	, m_mbus_handler(*this)
{
}

const char *i3002_device::reg_name(unsigned reg_idx)
{
	return reg_names[ reg_idx ];
}

void i3002_device::decode_fc(uint8_t fc , uint8_t& fg , uint8_t& rg , unsigned& reg)
{
	// Function group
	fg = (fc >> 4) & 7;

	uint8_t lsn = fc & 0xf;

	// Register & register group
	rg = fc_field_tab[ lsn ].rg;
	reg = fc_field_tab[ lsn ].reg;
}

void i3002_device::fc_kbus_w(uint8_t fc , uint8_t k)
{
	m_fc = fc & 0x7f;
	m_kbus = k & WORD_MASK;
}

WRITE_LINE_MEMBER(i3002_device::clk_w)
{
	if (state) {
		update();
	}
}

bool i3002_device::update_ro()
{
	// Update RO output if doing a right-shift only
	uint8_t fg;
	uint8_t rg;
	unsigned reg;

	decode_fc(m_fc , fg , rg , reg);

	if (fg == 0 && rg == 2) {
		uint8_t ik = !m_ibus_handler.isnull() ? m_ibus_handler() & m_kbus : 0;
		uint8_t at = m_reg[ reg ];
		set_ro(BIT(at , 0) && !BIT(ik , 0));
		return true;
	} else {
		return false;
	}
}

void i3002_device::device_start()
{
	m_co_handler.resolve();
	m_ro_handler.resolve();
	m_ibus_handler.resolve();
	m_mbus_handler.resolve();

	save_item(NAME(m_reg));
	save_item(NAME(m_fc));
	save_item(NAME(m_kbus));
	save_item(NAME(m_ci));
	save_item(NAME(m_li));
	save_item(NAME(m_co));
	save_item(NAME(m_ro));
}

void i3002_device::update()
{
	uint8_t fg;
	uint8_t rg;
	unsigned reg;

	decode_fc(m_fc , fg , rg , reg);

	switch (fg) {
	case 0:
		switch (rg) {
		case 0:
			// FC 0 RG I
			{
				uint8_t tmp = (m_reg[ REG_AC ] & m_kbus) + m_reg[ reg ] + m_ci;
				m_reg[ REG_AC ] = m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 1:
			// FC 0 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = (m_reg[ REG_AC ] & m_kbus) + m + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 2:
			// FC 0 RG III
			{
				// Who designed this mess???
				uint8_t ik = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK & m_kbus : 0;
				uint8_t at = m_reg[ reg ];
				m_reg[ reg ] = 0;
				set_ro(BIT(at , 0) && !BIT(ik , 0));
				if (m_li || (BIT(ik , 1) && BIT(at , 1))) {
					BIT_SET(m_reg[ reg ] , 1);
				}
				if ((BIT(at , 0) && BIT(ik , 0)) || (BIT(at , 1) || BIT(ik , 1))) {
					BIT_SET(m_reg[ reg ] , 0);
				}
			}
			break;
		}
		break;

	case 1:
		switch (rg) {
		case 0:
			// FC 1 RG I
			{
				m_reg[ REG_MAR ] = m_kbus | m_reg[ reg ];
				uint8_t tmp = m_reg[ reg ] + m_kbus + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 1:
			// FC 1 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				m_reg[ REG_MAR ] = m_kbus | m;
				uint8_t tmp = m + m_kbus + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 2:
			// FC 1 RG III
			{
				uint8_t tmp = ((m_reg[ reg ] ^ WORD_MASK) | m_kbus) + (m_reg[ reg ] & m_kbus) + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;
		}
		break;

	case 2:
		switch (rg) {
		case 0:
		case 1:
			// FC 2 RG I
			// FC 2 RG II
			{
				uint8_t tmp = (m_reg[ REG_AC ] & m_kbus) + WORD_MASK + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 2:
			// FC 2 RG III
			{
				uint8_t i = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK : 0;
				uint8_t tmp = (i & m_kbus) + WORD_MASK + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;
		}
		break;

	case 3:
		switch (rg) {
		case 0:
			// FC 3 RG I
			{
				uint8_t tmp = (m_reg[ REG_AC ] & m_kbus) + m_reg[ reg ] + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 1:
			// FC 3 RG II (== FC 0 RG II)
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = (m_reg[ REG_AC ] & m_kbus) + m + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;

		case 2:
			// FC 3 RG III
			{
				uint8_t i = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK : 0;
				uint8_t tmp = (i & m_kbus) + m_reg[ reg ] + m_ci;
				m_reg[ reg ] = tmp & WORD_MASK;
				set_co(BIT(tmp , SLICE_SIZE));
			}
			break;
		}
		break;

	case 4:
		switch (rg) {
		case 0:
			// FC 4 RG I
			{
				uint8_t tmp = m_reg[ reg ] & m_reg[ REG_AC ] & m_kbus;
				m_reg[ reg ] = tmp;
				set_co(m_ci || tmp != 0);
			}
			break;

		case 1:
			// FC 4 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = m & m_reg[ REG_AC ] & m_kbus;
				m_reg[ reg ] = tmp;
				set_co(m_ci || tmp != 0);
			}
			break;

		case 2:
			// FC 4 RG III
			{
				uint8_t i = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK : 0;
				uint8_t tmp = i & m_reg[ reg ] & m_kbus;
				m_reg[ reg ] = tmp;
				set_co(m_ci || tmp != 0);
			}
			break;
		}
		break;

	case 5:
		switch (rg) {
		case 0:
		case 2:
			// FC 5 RG I
			// FC 5 RG III
			{
				uint8_t tmp = m_reg[ reg ] & m_kbus;
				m_reg[ reg ] = tmp;
				set_co(m_ci || tmp != 0);
			}
			break;

		case 1:
			// FC 5 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = m & m_kbus;
				m_reg[ reg ] = tmp;
				set_co(m_ci || tmp != 0);
			}
			break;
		}
		break;

	case 6:
		switch (rg) {
		case 0:
			// FC 6 RG I
			{
				uint8_t tmp = m_reg[ REG_AC ] & m_kbus;
				m_reg[ reg ] |= tmp;
				set_co(m_ci || tmp != 0);
			}
			break;

		case 1:
			// FC 6 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = m_reg[ REG_AC ] & m_kbus;
				m_reg[ reg ] = m | tmp;
				set_co(m_ci || tmp != 0);
			}
			break;

		case 2:
			// FC 6 RG III
			{
				uint8_t ik = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK & m_kbus : 0;
				m_reg[ reg ] |= ik;
				set_co(m_ci || ik != 0);
			}
			break;
		}
		break;

	case 7:
		switch (rg) {
		case 0:
			// FC 7 RG I
			{
				uint8_t tmp = m_reg[ REG_AC ] & m_kbus;
				set_co(m_ci || (m_reg[ reg ] & tmp) != 0);
				m_reg[ reg ] = m_reg[ reg ] ^ tmp ^ WORD_MASK;
			}
			break;

		case 1:
			// FC 7 RG II
			{
				uint8_t m = !m_mbus_handler.isnull() ? m_mbus_handler() & WORD_MASK : 0;
				uint8_t tmp = m_reg[ REG_AC ] & m_kbus;
				set_co(m_ci || (m & tmp) != 0);
				m_reg[ reg ] = m ^ tmp ^ WORD_MASK;
			}
			break;

		case 2:
			// FC 7 RG III
			{
				uint8_t ik = !m_ibus_handler.isnull() ? m_ibus_handler() & WORD_MASK & m_kbus : 0;
				set_co(m_ci || (m_reg[ reg ] & ik) != 0);
				m_reg[ reg ] = m_reg[ reg ] ^ ik ^ WORD_MASK;
			}
			break;
		}
		break;
	}
}

void i3002_device::set_co(bool co)
{
	m_co = co;
	if (!m_co_handler.isnull()) {
		m_co_handler(m_co);
	}
}

void i3002_device::set_ro(bool ro)
{
	m_ro = ro;
	if (!m_ro_handler.isnull()) {
		m_ro_handler(m_ro);
	}
}
