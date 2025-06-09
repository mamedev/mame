// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple pseudo-VIA device
    Emulation by R. Belmont

    A "pseudo-VIA" is a 2-port GPIO interface and interrupt controller
    with a 6522 compatible-ish register layout.  The 6522's timers,
    shift register, and data direction registers are all missing.

    This first appeared in the Mac IIci's RBV chip, and also showed
    up in V8/Eagle/Spice/Tinker Bell, Sonora/Ardbeg, and VASP.

    Quadras used a fairly different pseudo-VIA that acts
    much closer to a "real" VIA.  (And a real 6522 in the
    Quadra 700/900/950).
*/

#include "emu.h"
#include "pseudovia.h"

#define LOG_IRQ (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_PSEUDOVIA, pseudovia_device, "pseudovia", "Apple pseudo-VIA")

pseudovia_device::pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APPLE_PSEUDOVIA, tag, owner, clock),
	m_write_irq(*this),
	m_in_a_handler(*this, 0),
	m_in_b_handler(*this, 0),
	m_in_config_handler(*this, 0),
	m_in_video_handler(*this, 0),
	m_in_msc_handler(*this, 0),
	m_out_a_handler(*this),
	m_out_b_handler(*this),
	m_out_config_handler(*this),
	m_out_video_handler(*this),
	m_out_msc_handler(*this),
	m_pseudovia_ier(0),
	m_pseudovia_ifr(0)
{
	std::fill_n(m_pseudovia_regs, 256, 0);
}

void pseudovia_device::device_start()
{
	save_item(NAME(m_pseudovia_regs));
	save_item(NAME(m_pseudovia_ier));
	save_item(NAME(m_pseudovia_ifr));
}

void pseudovia_device::device_reset()
{
	m_pseudovia_regs[2] = 0x7f;
}

void pseudovia_device::vbl_irq_w(int state)
{
	if (!state)
	{
		return;
	}

	m_pseudovia_regs[2] &= ~0x40; // set vblank signal

	if (m_pseudovia_regs[0x12] & 0x40)
	{
		pseudovia_recalc_irqs();
	}
}

template <u8 mask>
void pseudovia_device::slot_irq_w(int state)
{
	if (state)
	{
		m_pseudovia_regs[2] &= ~mask;
	}
	else
	{
		m_pseudovia_regs[2] |= mask;
	}

	pseudovia_recalc_irqs();
}

template void pseudovia_device::slot_irq_w<0x40>(int state);
template void pseudovia_device::slot_irq_w<0x20>(int state);
template void pseudovia_device::slot_irq_w<0x10>(int state);
template void pseudovia_device::slot_irq_w<0x08>(int state);
template void pseudovia_device::slot_irq_w<0x04>(int state);
template void pseudovia_device::slot_irq_w<0x02>(int state);
template void pseudovia_device::slot_irq_w<0x01>(int state);

void pseudovia_device::asc_irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pseudovia_regs[3] |= 0x10; // any VIA 2 interrupt | CB1 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x10;
		pseudovia_recalc_irqs();
	}
}

void pseudovia_device::scsi_irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pseudovia_regs[3] |= 0x08; // any VIA 2 interrupt | CB2 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x08;
		pseudovia_recalc_irqs();
	}
}

void pseudovia_device::pseudovia_recalc_irqs()
{
	// check slot interrupts and bubble them down to IFR
	uint8_t slot_irqs = (~m_pseudovia_regs[2]) & 0x78;
	slot_irqs &= (m_pseudovia_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_pseudovia_regs[3] |= 2; // any slot
	}
	else // no slot irqs, clear the pending bit
	{
		m_pseudovia_regs[3] &= ~2; // any slot
	}

	uint8_t ifr = (m_pseudovia_regs[3] & m_pseudovia_ier) & 0x1b;

	LOGMASKED(LOG_IRQ, "%s: slot_irqs %02x IFR %02x\n", slot_irqs, ifr);

	if (ifr != 0)
	{
		m_pseudovia_regs[3] = ifr | 0x80;
		m_pseudovia_ifr = ifr | 0x80;

		m_write_irq(ASSERT_LINE);
	}
	else
	{
		m_write_irq(CLEAR_LINE);
	}
}

uint8_t pseudovia_device::read(offs_t offset)
{
	u8 data = 0;
	if (offset < 0x100)
	{
		data = m_pseudovia_regs[offset];

		if (offset == 0)
		{
			data = m_in_b_handler();
		}

		if (offset == 1)
		{
			data = m_in_config_handler();
		}

		if (offset == 0x10)
		{
			data &= ~0x38;
			data |= m_in_video_handler();
		}

		// bit 7 of these registers always reads as 0 on pseudo-VIAs
		if ((offset == 0x12) || (offset == 0x13))
		{
			data &= ~0x80;
		}

		if ((offset >= 0x20) && (offset <= 0x2f))
		{
			data = m_in_msc_handler(offset & 0xf);
		}
	}
	else
	{
		offset >>= 9;
		switch (offset)
		{
		case 1: // Port A
			data = m_in_a_handler();
			break;

		case 13: // IFR
			data = m_pseudovia_ifr;
			break;

		case 14: // IER
			data = m_pseudovia_ier;
			break;

		default:
			LOG("pseudovia_r: Unknown pseudo-VIA register %d access\n", offset);
			break;
		}
	}
	return data;
}

void pseudovia_device::write(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		switch (offset)
		{
		case 0x00:
			m_out_b_handler(data);
			break;

		case 0x01:
			m_out_config_handler(data);
			break;

		case 0x02:
			m_pseudovia_regs[offset] |= (data & 0x40);
			pseudovia_recalc_irqs();
			break;

		case 0x03:           // write here to ack
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
				m_pseudovia_ifr |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
				m_pseudovia_ifr &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x10:
			m_pseudovia_regs[offset] = data;
			m_out_video_handler(data);
			break;

		case 0x12:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x13:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;

				if (data == 0xff)
					m_pseudovia_regs[offset] = 0x1f; // I don't know why this is special, but the IIci ROM's POST demands it
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			break;

		default:
			if ((offset >= 0x20) && (offset <= 0x2f))
			{
				m_out_msc_handler(offset & 0xf, data);
			}
			m_pseudovia_regs[offset] = data;
			break;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 1:   // Port A
			m_out_a_handler(data);
			break;

		case 13: // IFR
			if (data & 0x80)
			{
				data = 0x7f;
			}
			pseudovia_recalc_irqs();
			break;

		case 14:             // IER
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_ier |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_ier &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		default:
			LOG("pseudovia_w: Unknown extended pseudo-VIA register %d access\n", offset);
			break;
		}
	}
}
