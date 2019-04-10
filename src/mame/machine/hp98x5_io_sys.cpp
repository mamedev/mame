// License:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp98x5_io_sys.cpp

    HP98x5 I/O sub-system

*********************************************************************/

#include "emu.h"
#include "hp98x5_io_sys.h"
#include "cpu/hphybrid/hphybrid.h"

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

// Device type definition
DEFINE_DEVICE_TYPE(HP98X5_IO_SYS, hp98x5_io_sys_device, "hp98x5_io_sys", "HP98x5 I/O sub-system")

hp98x5_io_sys_device::hp98x5_io_sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig , HP98X5_IO_SYS , tag , owner , clock)
	, m_irl_handler(*this)
	, m_irh_handler(*this)
	, m_flg_handler(*this)
	, m_sts_handler(*this)
	, m_dmar_handler(*this)
{
}

void hp98x5_io_sys_device::device_start()
{
	m_irl_handler.resolve_safe();
	m_irh_handler.resolve_safe();
	m_flg_handler.resolve_safe();
	m_sts_handler.resolve_safe();
	m_dmar_handler.resolve_safe();

	save_item(NAME(m_irq_pending));
	save_item(NAME(m_pa));
	save_item(NAME(m_flg_status));
	save_item(NAME(m_sts_status));
	save_item(NAME(m_dmar_status));
}

void hp98x5_io_sys_device::device_reset()
{
	m_irq_pending = 0;
	update_irq();
	m_pa = 0;
	m_flg_status = 0;
	m_sts_status = 0;
	update_flg_sts();
	m_dmar_status = 0;
	update_dmar();
}

IRQ_CALLBACK_MEMBER(hp98x5_io_sys_device::irq_callback)
{
	if (irqline == HPHYBRID_IRL) {
		return m_irq_pending & 0xff;
	} else {
		return m_irq_pending >> 8;
	}
}

WRITE8_MEMBER(hp98x5_io_sys_device::pa_w)
{
	m_pa = data;
	update_flg_sts();
}

void hp98x5_io_sys_device::set_irq(uint8_t sc , int state)
{
	if (state) {
		BIT_SET(m_irq_pending, sc);
	} else {
		BIT_CLR(m_irq_pending, sc);
	}
	update_irq();
}

void hp98x5_io_sys_device::set_sts(uint8_t sc , int state)
{
	if (state) {
		BIT_SET(m_sts_status, sc);
	} else {
		BIT_CLR(m_sts_status, sc);
	}
	if (sc == m_pa) {
		update_flg_sts();
	}
}

void hp98x5_io_sys_device::set_flg(uint8_t sc , int state)
{
	if (state) {
		BIT_SET(m_flg_status, sc);
	} else {
		BIT_CLR(m_flg_status, sc);
	}
	if (sc == m_pa) {
		update_flg_sts();
	}
}

void hp98x5_io_sys_device::set_dmar(uint8_t sc , int state)
{
	if (state) {
		BIT_SET(m_dmar_status, sc);
	} else {
		BIT_CLR(m_dmar_status, sc);
	}
	update_dmar();
}

bool hp98x5_io_sys_device::is_irq_pending(uint8_t sc) const
{
	return BIT(m_irq_pending , sc);
}

void hp98x5_io_sys_device::update_irq()
{
	m_irl_handler((m_irq_pending & 0x00ff) != 0);
	m_irh_handler((m_irq_pending & 0xff00) != 0);
}

void hp98x5_io_sys_device::update_flg_sts()
{
	bool sts = BIT(m_sts_status , m_pa);
	bool flg = BIT(m_flg_status , m_pa);
	m_sts_handler(sts);
	m_flg_handler(flg);
}

void hp98x5_io_sys_device::update_dmar()
{
	m_dmar_handler(m_dmar_status != 0);
}
