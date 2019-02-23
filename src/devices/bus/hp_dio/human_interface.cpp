// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP Human interface board

***************************************************************************/

#include "emu.h"
#include "human_interface.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/tms9914.h"
#include "machine/msm58321.h"
#include "sound/sn76496.h"
#include "bus/hp_hil/hp_hil.h"
#include "bus/hp_hil/hil_devices.h"
#include "bus/ieee488/ieee488.h"
#include "speaker.h"
//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(HPDIO_HUMAN_INTERFACE, bus::hp_dio, human_interface_device, "human_interface", "HP human interface card")

namespace bus {
	namespace hp_dio {

void human_interface_device::device_add_mconfig(machine_config &config)
{
	i8042_device &iocpu(I8042(config, "iocpu", XTAL(5'000'000)));
	iocpu.set_addrmap(AS_PROGRAM, &human_interface_device::iocpu_map);
	iocpu.p1_out_cb().set(FUNC(human_interface_device::iocpu_port1_w));
	iocpu.p2_out_cb().set(FUNC(human_interface_device::iocpu_port2_w));
	iocpu.p1_in_cb().set(FUNC(human_interface_device::iocpu_port1_r));
	iocpu.t0_in_cb().set(FUNC(human_interface_device::iocpu_test0_r));
	iocpu.p2_in_cb().set_constant(0xdf);
	iocpu.t1_in_cb().set_constant(1);

	HP_HIL_MLC(config, m_mlc, XTAL(8'000'000));
	HP_HIL_SLOT(config, "hil1", m_mlc, hp_hil_devices, "hp_46021a");
	HP_HIL_SLOT(config, "hil2", m_mlc, hp_hil_devices, "hp_46060b");

	SPEAKER(config, "mono").front_center();
	sn76494_device &sound(SN76494(config, "sn76494", 333333));
	sound.add_route(ALL_OUTPUTS, "mono", 0.75);

	msm58321_device &rtc(MSM58321(config, "rtc", 32.768_kHz_XTAL));
	rtc.set_default_24h(false);
	rtc.d0_handler().set(FUNC(human_interface_device::rtc_d0_w));
	rtc.d1_handler().set(FUNC(human_interface_device::rtc_d1_w));
	rtc.d2_handler().set(FUNC(human_interface_device::rtc_d2_w));
	rtc.d3_handler().set(FUNC(human_interface_device::rtc_d3_w));

	tms9914_device &gpib(TMS9914(config, "tms9914", XTAL(5'000'000)));
	gpib.eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	gpib.dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	gpib.nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	gpib.ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	gpib.ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	gpib.srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	gpib.atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	gpib.ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	gpib.dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	gpib.dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	gpib.int_write_cb().set(FUNC(human_interface_device::gpib_irq));
	gpib.accrq_write_cb().set(FUNC(human_interface_device::gpib_dreq));

	ieee488_device &ieee488(IEEE488(config, IEEE488_TAG, 0));
	ieee488.eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	ieee488.dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	ieee488.nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	ieee488.ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	ieee488.ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	ieee488.srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	ieee488.atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	ieee488.ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));
	ieee488.dio_callback().set(FUNC(human_interface_device::ieee488_dio_w));

	ieee488_slot_device &slot0(IEEE488_SLOT(config, "ieee0", 0));
	hp_ieee488_devices(slot0);
	slot0.set_default_option("hp9122c");
}

ROM_START(human_interface)
	ROM_REGION(0x800 , "iocpu" , 0)
	ROM_LOAD( "1820-4784.bin", 0x000000, 0x000800, CRC(e929044a) SHA1(90849a10bdb8c6e38e73ce027c9c0ad8b3956b1b))
ROM_END

const tiny_rom_entry *human_interface_device::device_rom_region() const
{
	return ROM_NAME(human_interface);
}

void human_interface_device::iocpu_map(address_map& map)
{
	map(0x0000, 0x07ff).rom().region("iocpu", 0);
}

human_interface_device::human_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	human_interface_device(mconfig, HPDIO_HUMAN_INTERFACE, tag, owner, clock)
{
}

human_interface_device::human_interface_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	m_iocpu(*this, "iocpu"),
	m_mlc(*this, "mlc"),
	m_sound(*this, "sn76494"),
	m_tms9914(*this, "tms9914"),
	m_rtc(*this, "rtc"),
	m_ieee488(*this, IEEE488_TAG)
{
}

void human_interface_device::device_start()
{
	program_space()->install_readwrite_handler(0x420000, 0x420003, 0x0003, 0xfffc, 0,
		read8_delegate(FUNC(upi41_cpu_device::upi41_master_r), &(*m_iocpu)),
		write8_delegate(FUNC(upi41_cpu_device::upi41_master_w), &(*m_iocpu)), 0x00ff00ff);

	program_space()->install_readwrite_handler(0x470000, 0x47001f, 0x1f, 0xffe0, 0,
		read8_delegate(FUNC(human_interface_device::gpib_r), this),
		write8_delegate(FUNC(human_interface_device::gpib_w), this), 0x00ff00ff);

	save_item(NAME(m_hil_read));
	save_item(NAME(m_kbd_nmi));
	save_item(NAME(m_gpib_irq_line));
	save_item(NAME(m_gpib_dma_line));
	save_item(NAME(m_old_latch_enable));
	save_item(NAME(m_hil_data));
	save_item(NAME(m_latch_data));
	save_item(NAME(m_rtc_data));
	save_item(NAME(m_ppoll_mask));
	save_item(NAME(m_ppoll_sc));
	save_item(NAME(m_gpib_dma_enable));
}

void human_interface_device::device_reset()
{
	m_ppoll_sc = 0;
	m_gpib_irq_line = false;
	m_kbd_nmi = false;
	m_old_latch_enable = true;
	m_rtc->cs1_w(ASSERT_LINE);
	m_rtc->cs2_w(CLEAR_LINE);
	m_rtc->write_w(CLEAR_LINE);
	m_rtc->read_w(CLEAR_LINE);
	m_rtc->cs2_w(CLEAR_LINE);
	m_iocpu->reset();
}

WRITE_LINE_MEMBER(human_interface_device::reset_in)
{
	if (state)
		device_reset();
}

void human_interface_device::update_gpib_irq()
{
	irq3_out((m_gpib_irq_line ||
		((m_ppoll_sc & (PPOLL_IR|PPOLL_IE)) == (PPOLL_IR|PPOLL_IE))) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(human_interface_device::gpib_irq)
{
	m_gpib_irq_line = state;
	update_gpib_irq();
}

void human_interface_device::update_gpib_dma()
{
	dmar0_out(m_gpib_dma_enable && m_gpib_dma_line);
}

WRITE_LINE_MEMBER(human_interface_device::gpib_dreq)
{
	m_gpib_dma_line = state;
	update_gpib_dma();
}

WRITE8_MEMBER(human_interface_device::ieee488_dio_w)
{
	if (m_ieee488->atn_r() || m_ieee488->eoi_r())
		return;

	if ((m_ppoll_mask & ~data) && (m_ppoll_sc & PPOLL_IE)) {
		LOG("%s: parallel poll triggered\n", __func__);
		if (m_ppoll_sc & PPOLL_IE)
			m_ppoll_sc |= PPOLL_IR;
		update_gpib_irq();
	}
}

WRITE8_MEMBER(human_interface_device::gpib_w)
{
	if (offset & 0x08) {
		m_tms9914->write(offset & 0x07, data);
		return;
	}

	switch (offset) {
	case 0:
		device_reset();
		break;

	case 1:
		m_gpib_dma_enable = data & 0x01;
		update_gpib_dma();
		break;
	case 3:
		m_ppoll_sc = data & PPOLL_IE;

		if (!(data & PPOLL_IR)) {
			m_ppoll_sc &= ~PPOLL_IR;
			update_gpib_irq();
		}

		if (m_ppoll_sc & PPOLL_IE) {
			LOG("%s: start parallel poll\n", __func__);
			ieee488_dio_w(space, 0, m_ieee488->dio_r(space, 0));
		}
		break;
	case 4:
		m_ppoll_mask = data;
		break;
	default:
		break;
	}
	LOG("gpib_w: %s %02X = %02X\n", machine().describe_context().c_str(), offset, data);
}

READ8_MEMBER(human_interface_device::gpib_r)
{
	uint8_t data = 0xff;

	if (offset & 0x8) {
		data = m_tms9914->read(offset & 0x07);
		return data;
	}

	switch(offset) {
	case 0: /* ID */
		data = 0x80;
		break;
	case 1:
		/* Int control */
		data = 0x80 | (m_gpib_irq_line ? 0x40 : 0) | (m_gpib_dma_enable ? 0x01 : 0);
		break;
	case 2:
		/* Address */
		data = (m_tms9914->cont_r() ? 0x0 : 0x40) | 0x81;
		if (m_kbd_nmi)
			data |= 0x04;
		break;
	case 3:
		/* Parallel poll status/control */
		data = m_ppoll_sc;
		break;
	case 4:
		/* Parallel poll mask */
		data = m_ppoll_mask;
		break;
	}
	LOG("gpib_r: %s %02X = %02X\n", machine().describe_context().c_str(), offset, data);
	return data;
}


WRITE8_MEMBER(human_interface_device::iocpu_port1_w)
{
	m_hil_data = data;
	m_rtc->d0_w(data & 0x01 ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d1_w(data & 0x02 ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d2_w(data & 0x04 ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->d3_w(data & 0x08 ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(human_interface_device::iocpu_port2_w)
{
	bool latch_enable = data & LATCH_EN;

	if ((data & (HIL_CS|HIL_WE)) == 0)
		m_mlc->write(space, (m_latch_data & 0xc0) >> 6, m_hil_data, 0xff);

	if ((data & SN76494_EN) == 0)
		m_sound->write(m_hil_data);

	m_hil_read = ((data & (HIL_CS|HIL_OE)) == 0);

	m_rtc->address_write_w(data & 0x02 ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->write_w(data & 0x04 ? ASSERT_LINE : CLEAR_LINE);

	if (!m_old_latch_enable && latch_enable) {
		m_latch_data = m_hil_data;
		m_rtc->read_w(m_latch_data & 0x20 ? ASSERT_LINE : CLEAR_LINE);
		m_rtc->cs2_w(m_latch_data & 0x10 ? ASSERT_LINE : CLEAR_LINE);
	}
	irq1_out((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	if (!(data & KBD_RESET)) {
		irq7_out(ASSERT_LINE);
		m_kbd_nmi = true;
	} else {
		irq7_out(CLEAR_LINE);
		m_kbd_nmi = false;
	}
	m_old_latch_enable = latch_enable;
}

READ8_MEMBER(human_interface_device::iocpu_port1_r)
{
	if (m_hil_read)
		return m_mlc->read(space, (m_latch_data & 0xc0) >> 6, 0xff);
	if (m_latch_data & 0x20)
		return m_rtc_data;
	return 0xff;
}

READ8_MEMBER(human_interface_device::iocpu_test0_r)
{
	return !m_mlc->get_int();
}

WRITE_LINE_MEMBER(human_interface_device::rtc_d0_w)
{
	if (state)
		m_rtc_data |= 1;
	else
		m_rtc_data &= ~1;

}

WRITE_LINE_MEMBER(human_interface_device::rtc_d1_w)
{
	if (state)
		m_rtc_data |= 2;
	else
		m_rtc_data &= ~2;
}

WRITE_LINE_MEMBER(human_interface_device::rtc_d2_w)
{
	if (state)
		m_rtc_data |= 4;
	else
		m_rtc_data &= ~4;

}

WRITE_LINE_MEMBER(human_interface_device::rtc_d3_w)
{
	if (state)
		m_rtc_data |= 8;
	else
		m_rtc_data &= ~8;

}

void human_interface_device::dmack_w_in(int channel, uint8_t data)
{
	if (channel)
		return;
	m_tms9914->write(7, data);
}

uint8_t human_interface_device::dmack_r_in(int channel)
{
	if (channel || !m_gpib_dma_enable)
		return 0xff;
	return m_tms9914->read(7);
}

} // namespace bus::hp_dio
} // namespace bus
