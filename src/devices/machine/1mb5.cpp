// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    1mb5.cpp

    HP-8x I/O Translator chip (1MB5-0101)

    Reference for this chip:
    HP, aug 79, 1MB5 Detailed specification - Translator chip

*********************************************************************/

#include "emu.h"
#include "1mb5.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(HP_1MB5, hp_1mb5_device, "hp_1mb5", "HP 1MB5")

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

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}

hp_1mb5_device::hp_1mb5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig , HP_1MB5 , tag , owner , clock),
	m_irl_handler(*this),
	m_halt_handler(*this),
	m_reset_handler(*this),
	m_int_handler(*this)
{
}

READ8_MEMBER(hp_1mb5_device::cpu_r)
{
	uint8_t res = 0;

	switch (offset) {
	case 0:
		// Read SR
		res = m_sr & 0x7e;
		if (m_obf) {
			BIT_SET(res , 7);
		}
		if (m_ibf) {
			BIT_SET(res , 0);
		}
		break;

	case 1:
		// Read IB
		res = m_ib;
		m_ibf = false;
		update_halt();
		break;
	}

	//LOG("RD %u=%02x\n" , offset , res);
	return res;
}

WRITE8_MEMBER(hp_1mb5_device::cpu_w)
{
	//LOG("WR %u=%02x\n" , offset , data);
	bool need_resched = false;

	switch (offset) {
	case 0:
		// Write CR
		m_cr = data;
		need_resched |= set_reset(BIT(m_cr , 7));
		need_resched |= set_int(!BIT(m_cr , 0));
		break;

	case 1:
		// Write OB
		m_ob = data;
		m_obf = true;
		update_halt();
		break;
	}
	if (need_resched) {
		LOG("resched %s\n" , space.device().tag());
		space.device().execute().yield();
	}
}

READ8_MEMBER(hp_1mb5_device::uc_r)
{
	uint8_t res = 0;
	bool need_resched = false;

	switch (offset) {
	case 0:
		// Read CR
		res = m_cr & 0x7e;
		if (m_obf) {
			BIT_SET(res , 7);
		}
		if (m_ibf) {
			BIT_SET(res , 0);
		}
		break;

	case 1:
		// Read OB
		res = m_ob;
		m_obf = false;
		need_resched |= update_halt();
		break;
	}

	if (need_resched) {
		LOG("resched %s\n" , space.device().tag());
		space.device().execute().spin();
	}
	//LOG("RDU %u=%02x\n" , offset , res);
	return res;
}

WRITE8_MEMBER(hp_1mb5_device::uc_w)
{
	//LOG("WRU %u=%02x SR=%02x\n" , offset , data , m_sr);
	bool need_resched = false;

	switch (offset) {
	case 0:
		// Write SR
		if (!BIT(m_sr , 0) && BIT(data , 0)) {
			need_resched |= set_service(true);
		}
		m_sr = data;
		m_hlten = BIT(m_sr , 7);
		if (update_halt() && !m_halt) {
			need_resched = true;
		}
		break;

	case 1:
		// Write IB
		m_ib = data;
		m_ibf = true;
		need_resched |= update_halt();
		break;
	}
	if (need_resched) {
		LOG("resched %s\n" , space.device().tag());
		space.device().execute().spin();
	}
}

READ_LINE_MEMBER(hp_1mb5_device::irl_r)
{
	return m_service;
}

READ_LINE_MEMBER(hp_1mb5_device::halt_r)
{
	return m_halt;
}

READ_LINE_MEMBER(hp_1mb5_device::reset_r)
{
	return m_reset;
}

READ_LINE_MEMBER(hp_1mb5_device::int_r)
{
	return m_cint;
}

void hp_1mb5_device::inten()
{
	// Enabling interrupts (i.e. writing to 0xff40) removes uC reset
	set_reset(false);
}

void hp_1mb5_device::clear_service()
{
	set_service(false);
}

void hp_1mb5_device::device_start()
{
	m_irl_handler.resolve_safe();
	m_halt_handler.resolve_safe();
	m_reset_handler.resolve_safe();
	m_int_handler.resolve_safe();

	save_item(NAME(m_sr));
	save_item(NAME(m_cr));
	save_item(NAME(m_ib));
	save_item(NAME(m_ob));
	save_item(NAME(m_ibf));
	save_item(NAME(m_obf));
	save_item(NAME(m_hlten));
	save_item(NAME(m_service));
	save_item(NAME(m_cint));
	save_item(NAME(m_reset));
	save_item(NAME(m_halt));
}

void hp_1mb5_device::device_reset()
{
	m_sr = 0;
	m_cr = 0;
	m_ib = 0;
	m_ob = 0;
	m_ibf = false;
	m_obf = false;
	m_hlten = false;
	m_service = false;
	m_cint = true;
	m_reset = true;
	m_halt = false;

	m_irl_handler(false);
	m_halt_handler(false);
	m_reset_handler(true);
	m_int_handler(true);
}

bool hp_1mb5_device::set_service(bool new_service)
{
	if (new_service != m_service) {
		m_service = new_service;
		//LOG("irl=%d\n" , m_service);
		m_irl_handler(m_service);
		return true;
	} else {
		return false;
	}
}

bool hp_1mb5_device::update_halt()
{
	bool new_halt = m_hlten && m_obf && !m_ibf;
	if (new_halt != m_halt) {
		//LOG("HALT=%d\n" , new_halt);
		m_halt = new_halt;
		m_halt_handler(m_halt);
		return true;
	} else {
		return false;
	}
}

bool hp_1mb5_device::set_reset(bool new_reset)
{
	if (new_reset != m_reset) {
		m_reset = new_reset;
		m_reset_handler(m_reset);
		return true;
	} else {
		return false;
	}
}

bool hp_1mb5_device::set_int(bool new_int)
{
	if (new_int != m_cint) {
		m_cint = new_int;
		LOG("cint=%d\n" , m_cint);
		m_int_handler(m_cint);
		return true;
	} else {
		return false;
	}
}
