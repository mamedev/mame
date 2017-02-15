// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "ibm6580_fdc.h"


#define VERBOSE_DBG 2       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
	if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-10s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


const device_type DW_FDC = &device_creator<dw_fdc_device>;

ROM_START( dw_fdc )
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("4430030_FLP_8041.BIN", 0x0000, 0x400, CRC(2bb96799) SHA1(e30b0f2d790197f290858eab74ad5e151ded78c3))
ROM_END


const tiny_rom_entry *dw_fdc_device::device_rom_region() const
{
	return ROM_NAME( dw_fdc );
}

static ADDRESS_MAP_START( dw_fdc_io, AS_IO, 8, dw_fdc_device )
//  AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READWRITE(bus_r, bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
//  AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( dw_fdc )
	MCFG_CPU_ADD("mcu", I8048, XTAL_24MHz/4)    // divisor is unverified
	MCFG_CPU_IO_MAP(dw_fdc_io)

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)

	MCFG_UPD765A_ADD("upd765", false, false)
//  MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
//  MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dma8257", dma8257_device, XXX))
//  MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", wangpc_floppies, "525dd", wangpc_state::floppy_formats)
//  MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", wangpc_floppies, "525dd", wangpc_state::floppy_formats)
MACHINE_CONFIG_END

machine_config_constructor dw_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dw_fdc );
}

dw_fdc_device::dw_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DW_FDC, "IBM Displaywriter Floppy", tag, owner, clock, "dw_kbd", __FILE__)
	, m_out_data(*this)
	, m_out_clock(*this)
	, m_out_strobe(*this)
	, m_mcu(*this, "mcu")
{
}

void dw_fdc_device::device_start()
{
	m_out_data.resolve_safe();
	m_out_clock.resolve_safe();
	m_out_strobe.resolve_safe();
	m_reset_timer = timer_alloc();
}

void dw_fdc_device::device_reset()
{
	m_p1 = m_p2 = m_t0 = m_t1 = 0;
}

void dw_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE8_MEMBER( dw_fdc_device::p1_w )
{
	m_p1 = data;

	DBG_LOG(2,"p1",( "<- %02x\n", data, m_p1));
}

WRITE8_MEMBER( dw_fdc_device::p2_w )
{
	m_p2 = data;

	DBG_LOG(2,"p2",( "<- %02x\n", data));
}

READ8_MEMBER( dw_fdc_device::p2_r )
{
	uint8_t data = m_p2;

	DBG_LOG(2,"p2",( "== %02x\n", data));

	return data;
}

READ8_MEMBER( dw_fdc_device::t0_r )
{
	DBG_LOG(2,"t0",( "== %d\n", m_t0));

	return m_t0;
}

READ8_MEMBER( dw_fdc_device::t1_r )
{
	DBG_LOG(2,"t1",( "== %d\n", m_t1));

	return m_t1;
}

WRITE8_MEMBER( dw_fdc_device::bus_w )
{
	m_bus = data;
}

READ8_MEMBER( dw_fdc_device::bus_r )
{
	return m_bus;
}

WRITE_LINE_MEMBER( dw_fdc_device::reset_w )
{
	if(!state)
		m_reset_timer->adjust(attotime::from_msec(50));
	else
	{
		m_reset_timer->adjust(attotime::never);
		m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( dw_fdc_device::ack_w )
{
	m_t0 = state;
}
