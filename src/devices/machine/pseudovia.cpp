// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple pseudo-VIA device
    Emulation by R. Belmont
    Hardware testing by Doug Brown
    Hardware tests run on LC II, LC III, LC 550, and IIci

    A "pseudo-VIA" is a 2-port GPIO interface and interrupt controller
    with a 6522 compatible-ish register layout.  The 6522's timers,
    shift register, and data direction registers are all missing.

    This first appeared in the Mac IIci's RBV chip, and also showed
    up in V8/Eagle/Spice/Tinker Bell, Sonora/Ardbeg, and VASP.

    The original pseudo-VIA in the MDU ASIC (IIci/IIsi) only decodes A0, A1, and A4
    for registers 0, 1, 2, 3, 0x10, 0x11, 0x12, and 0x13.  Bit 7 of 0x13 (IER) reads
    back as '1' instead of the expected '0'.

    V8 and derivatives are the same as MDU except bit 7 of IER reads as '0'.

    Sonora and derivatives are like V8 but A2 and A3 are also decoded to support
    the additional system configuration registers 4 and 5.

    Quadras used a quite different pseudo-VIA that acts
    much closer to a "real" VIA.  (And a real 6522 in the
    Quadra 700/900/950).
*/

#include "emu.h"
#include "pseudovia.h"

#define LOG_IRQ (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_PSEUDOVIA, pseudovia_device, "pseudovia", "Apple pseudo-VIA (RBV)")
DEFINE_DEVICE_TYPE(APPLE_V8_PSEUDOVIA, v8_pseudovia_device, "v8psvia", "Apple pseudo-VIA (V8/Eagle/Spice/Tinker Bell/VASP)")
DEFINE_DEVICE_TYPE(APPLE_SONORA_PSEUDOVIA, sonora_pseudovia_device, "snpsvia", "Apple pseudo-VIA (Sonora/Ardbeg)")
DEFINE_DEVICE_TYPE(APPLE_MSC_PSEUDOVIA, msc_pseudovia_device, "mspsvia", "Apple pseudo-VIA (MSC)")
DEFINE_DEVICE_TYPE(APPLE_PB030_PSEUDOVIA, pb030_pseudovia_device, "pbpsvia", "Apple pseudo-VIA (Misc. GLU)")
DEFINE_DEVICE_TYPE(APPLE_QUADRA_PSEUDOVIA, quadra_pseudovia_device, "qdpsvia", "Apple pseudo-VIA (IOSB/PrimeTime/PrimeTime II)")

enum
{
	PVIA_PB = 0,
	PVIA_PA = 1,
	PVIA_DDRB = 2,
	PVIA_DDRA = 3,
	PVIA_T1CL = 4,
	PVIA_T1CH = 5,
	PVIA_T1LL = 6,
	PVIA_T1LH = 7,
	PVIA_T2CL = 8,
	PVIA_T2CH = 9,
	PVIA_SR = 10,
	PVIA_ACR = 11,
	PVIA_PCR = 12,
	PVIA_IFR = 13,
	PVIA_IER = 14,
	PVIA_PANH = 15
};

pseudovia_device::pseudovia_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
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
	m_out_msc_handler(*this)
{
	std::fill_n(m_pseudovia_regs, sizeof(m_pseudovia_regs), 0);
}

pseudovia_device::pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_PSEUDOVIA, tag, owner, clock)
{
}

void pseudovia_device::device_start()
{
	save_item(NAME(m_pseudovia_regs));
}

void pseudovia_device::device_reset()
{
	m_pseudovia_regs[2] = 0x7f;
	m_pseudovia_regs[3] = 0x1b;
}

void pseudovia_device::vbl_irq_w(int state)
{
	if (!state)
	{
		m_pseudovia_regs[2] |= 0x40;    // clear vblank signal
	}
	else
	{
		m_pseudovia_regs[2] &= ~0x40;   // set vblank signal
	}

	pseudovia_recalc_irqs();
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
	if (state && !BIT(m_live_main_ints, 4))
	{
		m_pseudovia_regs[3] |= 0x10; // CB1 interrupt
		pseudovia_recalc_irqs();
	}

	m_live_main_ints &= ~0x10;
	m_live_main_ints |= (state << 4);
}

void pseudovia_device::scsi_irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pseudovia_regs[3] |= 0x08; // CB2 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x08;
		pseudovia_recalc_irqs();
	}
}

void pseudovia_device::scsi_drq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pseudovia_regs[3] |= 0x01;
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x01;
		pseudovia_recalc_irqs();
	}
}

void pseudovia_device::slot_irq_w(int state)
{
	if (state == CLEAR_LINE)
	{
		m_pseudovia_regs[3] |= 0x02;
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x02;
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

	const uint8_t ifr = m_pseudovia_regs[3] & m_pseudovia_regs[0x13] & 0x1b;
	LOGMASKED(LOG_IRQ, "%s: slot_irqs %02x IFR %02x 0x02 %02x 0x03 %02x 0x12 %02x 0x13 %02x\n", tag(), slot_irqs, ifr, m_pseudovia_regs[2], m_pseudovia_regs[3], m_pseudovia_regs[0x12], m_pseudovia_regs[0x13]);

	if (ifr != 0)
	{
		m_pseudovia_regs[3] = ifr | 0x80;
		m_write_irq(ASSERT_LINE);
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x80;
		m_write_irq(CLEAR_LINE);
	}
}

uint8_t pseudovia_device::read(offs_t offset)
{
	offset &= 0x13;
	u8 data = m_pseudovia_regs[offset];

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

	return data;
}

void pseudovia_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x13;
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
		m_pseudovia_regs[offset] &= ~(data & 0x7f);
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
			{
				m_pseudovia_regs[offset] = 0x1f; // I don't know why this is special, but the IIci ROM's POST demands it
			}
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[offset] &= ~(data & 0x7f);
		}
		pseudovia_recalc_irqs();
		break;
	}
}

// V8: like base RBV but ASC IRQs are level triggered
v8_pseudovia_device::v8_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_V8_PSEUDOVIA, tag, owner, clock)
{
}

void v8_pseudovia_device::asc_irq_w(int state)
{
	if (state)
	{
		m_pseudovia_regs[3] |= 0x10; // CB1 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x10;
		pseudovia_recalc_irqs();
	}
}

void v8_pseudovia_device::write(offs_t offset, uint8_t data)
{
	if ((offset >> 9) == 1)
	{
		m_out_a_handler(data);
		return;
	}

	offset &= 0x1f;
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

	case 0x03:         // write here to ack
		data &= ~0x10; // ASC IRQ is level triggered, so writing a 1 here should be a NOP.
		m_pseudovia_regs[offset] &= ~(data & 0x7f);
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
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[offset] &= ~(data & 0x7f);
		}
		pseudovia_recalc_irqs();
		break;
	}
}

// Sonora: same as base RBV but decodes A4-A0 and has 2 additional registers
sonora_pseudovia_device::sonora_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_SONORA_PSEUDOVIA, tag, owner, clock)
{
}

void sonora_pseudovia_device::asc_irq_w(int state)
{
	// Sonora ASC IRQs are level triggered
	if (state)
	{
		m_pseudovia_regs[3] |= 0x10; // CB1 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x10;
		pseudovia_recalc_irqs();
	}
}

uint8_t sonora_pseudovia_device::read(offs_t offset)
{
	offset &= 0x1f;
	u8 data = m_pseudovia_regs[offset];

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

	return data;
}

void sonora_pseudovia_device::write(offs_t offset, uint8_t data)
{
	if ((offset >> 9) == 1)
	{
		m_out_a_handler(data);
		return;
	}

	offset &= 0x1f;
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
		data &= ~0x10; // ASC IRQ is level triggered, so writing a 1 here should be a NOP.
		m_pseudovia_regs[offset] &= ~(data & 0x7f);
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
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[offset] &= ~(data & 0x7f);
		}
		pseudovia_recalc_irqs();
		break;
	}
}

// MSC version decodes a full 256 bytes and mirrors every 256 bytes
msc_pseudovia_device::msc_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_MSC_PSEUDOVIA, tag, owner, clock)
{
}

void msc_pseudovia_device::asc_irq_w(int state)
{
	// MSC ASC IRQs are level triggered
	if (state)
	{
		m_pseudovia_regs[3] |= 0x10; // CB1 interrupt
		pseudovia_recalc_irqs();
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x10;
		pseudovia_recalc_irqs();
	}
}

uint8_t msc_pseudovia_device::read(offs_t offset)
{
	offset &= 0xff;
	u8 data = m_pseudovia_regs[offset];

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
		return m_in_msc_handler(offset & 0xf);
	}

	return data;
}

void msc_pseudovia_device::write(offs_t offset, uint8_t data)
{
	offset &= 0xff;

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
		data &= ~0x10;   // ASC IRQ is level triggered, so writing a 1 here should be a NOP.
		m_pseudovia_regs[offset] &= ~(data & 0x7f);
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
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[offset] &= ~(data & 0x7f);
		}
		pseudovia_recalc_irqs();
		break;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		m_out_msc_handler(offset & 0xf, data);
		break;
	}
}

// More VIA-like pseudo-VIA found in PowerBook 140/160/170/180
pb030_pseudovia_device::pb030_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_PB030_PSEUDOVIA, tag, owner, clock)
{
}

uint8_t pb030_pseudovia_device::read(offs_t offset)
{
	u8 data = 0;
	offset >>= 9;
	switch (offset)
	{
	case 0:
		data = m_in_b_handler();
		break;

	case 1: // Port A
		data = m_in_a_handler();
		break;

	case 13: // IFR
		data = m_pseudovia_regs[0x03];
		break;

	case 14: // IER
		data = m_pseudovia_regs[0x13];
		break;

	default:
		LOG("pseudovia_r: Unknown pseudo-VIA register %d access\n", offset);
		break;
	}
	return data;
}

void pb030_pseudovia_device::write(offs_t offset, uint8_t data)
{
	offset >>= 9;

	switch (offset)
	{
	case 0:
		m_out_b_handler(data);
		break;

	case 1:   // Port A
		m_out_a_handler(data);
		break;

	case 13: // IFR
		m_pseudovia_regs[0x03] &= ~(data & 0x7f);
		pseudovia_recalc_irqs();
		break;

	case 14:             // IER
		if (data & 0x80) // 1 bits write 1s
		{
			m_pseudovia_regs[0x13] |= data & 0x7f;
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[0x13] &= ~(data & 0x7f);
		}
		pseudovia_recalc_irqs();
		break;

	default:
		LOG("pseudovia_w: Unknown extended pseudo-VIA register %d access\n", offset);
		break;
	}
}

// Most VIA-like pseudo-VIA found in PowerBook 140/160/170/180
quadra_pseudovia_device::quadra_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pseudovia_device(mconfig, APPLE_QUADRA_PSEUDOVIA, tag, owner, clock)
{
}

uint8_t quadra_pseudovia_device::read(offs_t offset)
{
	u8 data = 0;
	offset >>= 9;
	switch (offset)
	{
	case 0:
		data = m_in_b_handler();
		break;

	case 1: // Port A
	case 15: // also Port A
		data = m_in_a_handler();
		break;

	case 13: // IFR
		data = m_pseudovia_regs[0x03];
		break;

	case 14: // IER
		data = m_pseudovia_regs[0x13];
		break;

	default:
		LOG("pseudovia_r: Unknown pseudo-VIA register %d access\n", offset);
		break;
	}
	return data;
}

void quadra_pseudovia_device::write(offs_t offset, uint8_t data)
{
	offset >>= 9;

	switch (offset)
	{
	case 0:
		m_out_b_handler(data);
		break;

	case 1: // Port A
		m_out_a_handler(data);
		break;

	case 13: // IFR
		{
			u8 mask = ~(data & 0x1b);
			m_pseudovia_regs[0x03] &= mask;
			pseudovia_recalc_irqs();
		}
		break;

	case 14:             // IER
		if (data & 0x80) // 1 bits write 1s
		{
			m_pseudovia_regs[0x13] |= data & 0x1b;
		}
		else // 1 bits write 0s
		{
			m_pseudovia_regs[0x13] &= ~(data & 0x1b);
		}
		pseudovia_recalc_irqs();
		break;

	default:
		LOG("pseudovia_w: Unknown extended pseudo-VIA register %d access\n", offset);
		break;
	}
}

void quadra_pseudovia_device::pseudovia_recalc_irqs()
{
	const uint8_t ifr = m_pseudovia_regs[3] & m_pseudovia_regs[0x13] & 0x1b;
	if (ifr != 0)
	{
		m_pseudovia_regs[3] |= 0x80;
		m_write_irq(ASSERT_LINE);
	}
	else
	{
		m_pseudovia_regs[3] &= ~0x80;
		m_write_irq(CLEAR_LINE);
	}
}
