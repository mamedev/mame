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

MACHINE_CONFIG_START(human_interface_device::device_add_mconfig)

	i8042_device &iocpu(I8042(config, "iocpu", XTAL(5'000'000)));
	iocpu.set_addrmap(AS_PROGRAM, &human_interface_device::iocpu_map);
	iocpu.p1_out_cb().set(FUNC(human_interface_device::iocpu_port1_w));
	iocpu.p2_out_cb().set(FUNC(human_interface_device::iocpu_port2_w));
	iocpu.p1_in_cb().set(FUNC(human_interface_device::iocpu_port1_r));
	iocpu.t0_in_cb().set(FUNC(human_interface_device::iocpu_test0_r));
	iocpu.p2_in_cb().set_constant(0xdf);
	iocpu.t1_in_cb().set_constant(1);

	MCFG_DEVICE_ADD(m_mlc, HP_HIL_MLC, XTAL(8'000'000))
	MCFG_HP_HIL_SLOT_ADD("mlc", "hil1", hp_hil_devices, "hp_46021a")
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("sn76494", SN76494, 333333)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_DEVICE_ADD("rtc", MSM58321, 32.768_kHz_XTAL)
	MCFG_MSM58321_DEFAULT_24H(false)
	MCFG_MSM58321_D0_HANDLER(WRITELINE(*this, human_interface_device, rtc_d0_w))
	MCFG_MSM58321_D1_HANDLER(WRITELINE(*this, human_interface_device, rtc_d1_w))
	MCFG_MSM58321_D2_HANDLER(WRITELINE(*this, human_interface_device, rtc_d2_w))
	MCFG_MSM58321_D3_HANDLER(WRITELINE(*this, human_interface_device, rtc_d3_w))

	MCFG_DEVICE_ADD(m_tms9914, TMS9914, XTAL(5000000))
	MCFG_TMS9914_INT_WRITE_CB(WRITELINE(*this, human_interface_device, gpib_irq));
	MCFG_TMS9914_ACCRQ_WRITE_CB(WRITELINE(*this, human_interface_device, gpib_dreq));
	MCFG_TMS9914_DIO_READWRITE_CB(READ8(IEEE488_TAG, ieee488_device, dio_r), WRITE8(IEEE488_TAG, ieee488_device, host_dio_w))
	MCFG_TMS9914_EOI_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_eoi_w))
	MCFG_TMS9914_DAV_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_dav_w))
	MCFG_TMS9914_NRFD_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_nrfd_w))
	MCFG_TMS9914_NDAC_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_ndac_w))
	MCFG_TMS9914_IFC_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_ifc_w))
	MCFG_TMS9914_SRQ_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_srq_w))
	MCFG_TMS9914_ATN_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_atn_w))
	MCFG_TMS9914_REN_WRITE_CB(WRITELINE(IEEE488_TAG, ieee488_device, host_ren_w))

	MCFG_IEEE488_SLOT_ADD("ieee0", 0, hp_ieee488_devices, "hp9122c")
	MCFG_IEEE488_SLOT_ADD("ieee_rem", 0, remote488_devices, nullptr)
	MCFG_IEEE488_BUS_ADD()

	MCFG_IEEE488_EOI_CALLBACK(WRITELINE(m_tms9914, tms9914_device, eoi_w))
	MCFG_IEEE488_DAV_CALLBACK(WRITELINE(m_tms9914, tms9914_device, dav_w))
	MCFG_IEEE488_NRFD_CALLBACK(WRITELINE(m_tms9914, tms9914_device, nrfd_w))
	MCFG_IEEE488_NDAC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ndac_w))
	MCFG_IEEE488_IFC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ifc_w))
	MCFG_IEEE488_SRQ_CALLBACK(WRITELINE(m_tms9914, tms9914_device, srq_w))
	MCFG_IEEE488_ATN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, atn_w))
	MCFG_IEEE488_REN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ren_w))
MACHINE_CONFIG_END

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
void human_interface_device::map(address_map& map)
{
	map(0x00420000, 0x00420003).mirror(0x0000fffc).rw(m_iocpu, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w)).umask32(0x00ff00ff);
	map(0x00470000, 0x0047001f).mirror(0x0000ffe0).rw(FUNC(human_interface_device::gpib_r), FUNC(human_interface_device::gpib_w)).umask16(0x00ff);

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
	m_rtc(*this, "rtc")
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
	save_item(NAME(gpib_irq_line));
	save_item(NAME(m_old_latch_enable));
	save_item(NAME(m_hil_data));
	save_item(NAME(m_latch_data));
	save_item(NAME(m_rtc_data));
}

void human_interface_device::device_reset()
{
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
WRITE_LINE_MEMBER(human_interface_device::gpib_irq)
{
	gpib_irq_line = state;
	irq3_out(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(human_interface_device::gpib_dreq)
{
	dmar0_out(state);
}

WRITE8_MEMBER(human_interface_device::gpib_w)
{
	if (offset & 0x08) {
		m_tms9914->reg8_w(space, offset & 0x07, data);
		return;
	}
	LOG("gpib_w: %s %02X = %02X\n", machine().describe_context().c_str(), offset, data);
}

READ8_MEMBER(human_interface_device::gpib_r)
{
	uint8_t data = 0xff;

	if (offset & 0x8) {
		data = m_tms9914->reg8_r(space, offset & 0x07);
		return data;
	}

	switch(offset) {
	case 0: /* ID */
		data = 0x80;
		break;
	case 1:
		/* Int control */
		data = 0x80 | (gpib_irq_line ? 0x40 : 0);
		break;
	case 2:
		/* Address */
		data = (m_tms9914->cont_r() ? 0x0 : 0x40) | 0x81;
		if (m_kbd_nmi)
			data |= 0x04;
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
//	FIXME
//	m_tms9914->reg8_w(memory_space(), 7, data);
}

uint8_t human_interface_device::dmack_r_in(int channel)
{
	if (channel)
		return 0xff;
	return m_tms9914->reg8_r(machine().dummy_space(), 7);
}

} // namespace bus::hp_dio
} // namespace bus
