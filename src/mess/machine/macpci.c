/****************************************************************************

    machine/pcimac.c

    PCI-based Power Macintosh hardware

    R. Belmont

****************************************************************************/

#include "emu.h"
#include "machine/6522via.h"
#include "machine/8530scc.h"
#include "cpu/m68000/m68000.h"
#include "machine/applefdc.h"
#include "devices/sonydriv.h"
#include "includes/macpci.h"
#include "debug/debugcpu.h"
#include "machine/ram.h"
#include "debugger.h"

#define LOG_ADB			0
#define LOG_VIA			0

static TIMER_CALLBACK(mac_6015_tick);

/* VIA1 Handlers */

static READ8_DEVICE_HANDLER(mac_via_in_a);
static READ8_DEVICE_HANDLER(mac_via_in_b);
static READ8_DEVICE_HANDLER(mac_adb_via_in_cb2);
static WRITE8_DEVICE_HANDLER(mac_via_out_a);
static WRITE8_DEVICE_HANDLER(mac_via_out_b);
static WRITE8_DEVICE_HANDLER(mac_adb_via_out_cb2);
static void mac_via_irq(device_t *device, int state);

const via6522_interface pcimac_via6522_intf =
{
	DEVCB_HANDLER(mac_via_in_a), DEVCB_HANDLER(mac_via_in_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_NULL, DEVCB_HANDLER(mac_adb_via_in_cb2),
	DEVCB_HANDLER(mac_via_out_a), DEVCB_HANDLER(mac_via_out_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_NULL, DEVCB_HANDLER(mac_adb_via_out_cb2),
	DEVCB_LINE(mac_via_irq)
};

static void mac_via_irq(device_t *device, int state)
{
}

static READ8_DEVICE_HANDLER(mac_via_in_a)
{
//  macpci_state *mac = device->machine().driver_data<macpci_state>();

//    printf("VIA1 IN_A (PC %x)\n", mac->m_maincpu->safe_pc());

	return 0x80;
}

static READ8_DEVICE_HANDLER(mac_via_in_b)
{
	int val = 0;
	macpci_state *mac = device->machine().driver_data<macpci_state>();

    val |= mac->m_cuda->get_treq()<<3;

//    printf("VIA1 IN B = %02x (PC %x)\n", val, mac->m_maincpu->safe_pc());

    return val;
}

static WRITE8_DEVICE_HANDLER(mac_via_out_a)
{
//  macpci_state *mac = device->machine().driver_data<macpci_state>();

//    printf("VIA1 OUT A: %02x (PC %x)\n", data, mac->m_maincpu->safe_pc());
}

static WRITE8_DEVICE_HANDLER(mac_via_out_b)
{
	macpci_state *mac = device->machine().driver_data<macpci_state>();

//    printf("VIA1 OUT B: %02x (PC %x)\n", data, mac->m_maincpu->safe_pc());

    #if LOG_ADB
    printf("PPC: New Cuda state: TIP %d BYTEACK %d (PC %x)\n", (data>>5)&1, (data>>4)&1, mac->m_maincpu->safe_pc());
    #endif
    mac->m_cuda->set_byteack((data&0x10) ? 1 : 0);
    mac->m_cuda->set_tip((data&0x20) ? 1 : 0);
}

READ16_MEMBER ( macpci_state::mac_via_r )
{
    UINT16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		printf("mac_via_r: offset=0x%02x (PC=%x)\n", offset, m_maincpu->safe_pc());
	data = m_via1->read(space, offset);

	device_adjust_icount(m_maincpu, m_via_cycles);

    return data | (data<<8);
}

WRITE16_MEMBER ( macpci_state::mac_via_w )
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		printf("mac_via_w: offset=0x%02x data=0x%08x (PC=%x)\n", offset, data, m_maincpu->safe_pc());

	if (ACCESSING_BITS_0_7)
		m_via1->write(space, offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(space, offset, (data >> 8) & 0xff);

	device_adjust_icount(m_maincpu, m_via_cycles);
}

static READ8_DEVICE_HANDLER(mac_adb_via_in_cb2)
{
	UINT8 ret;
	macpci_state *mac = device->machine().driver_data<macpci_state>();

    ret = mac->m_cuda->get_via_data();
    #if LOG_ADB
    printf("PPC: Read VIA_DATA %x\n", ret);
    #endif

    return ret;
}

static WRITE8_DEVICE_HANDLER(mac_adb_via_out_cb2)
{
	macpci_state *mac = device->machine().driver_data<macpci_state>();

    mac->m_cuda->set_via_data(data & 1);
}

void macpci_state::machine_start()
{
	m_6015_timer = machine().scheduler().timer_alloc(FUNC(mac_6015_tick));
	m_6015_timer->adjust(attotime::never);
}

void macpci_state::machine_reset()
{
    m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

    m_via_cycles = -256;    // for a 66 MHz PowerMac

    cputag_set_input_line(machine(), "maincpu", INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE_LINE_MEMBER(macpci_state::cuda_reset_w)
{
    cputag_set_input_line(machine(), "maincpu", INPUT_LINE_RESET, state);
}

static void mac_driver_init(running_machine &machine, model_t model)
{
	macpci_state *mac = machine.driver_data<macpci_state>();

	mac->m_model = model;

    memset(mac->m_ram->pointer(), 0, mac->m_ram->size());
}

#define MAC_DRIVER_INIT(label, model)	\
DRIVER_INIT_MEMBER(macpci_state,label)  \
{	\
	mac_driver_init(machine(), model );	\
}

MAC_DRIVER_INIT(pippin, PCIMODEL_MAC_PIPPIN)

READ32_MEMBER(macpci_state::mac_read_id)
{
    printf("Mac read ID reg @ PC=%x\n", m_maincpu->safe_pc());

	switch (m_model)
	{
		case PCIMODEL_MAC_PIPPIN:
			return 0xa55a7001;

		default:
			return 0;
	}
}

/* 8530 SCC interface */

READ16_MEMBER ( macpci_state::mac_scc_r )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	UINT16 result;

	result = scc->reg_r(space, offset);
	return (result << 8) | result;
}

WRITE16_MEMBER ( macpci_state::mac_scc_w )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	scc->reg_w(space, offset, data);
}

WRITE16_MEMBER ( macpci_state::mac_scc_2_w )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	scc->reg_w(space, offset, data >> 8);
}

READ8_MEMBER(macpci_state::mac_5396_r)
{
	if (offset < 0x100)
	{
		return m_539x_1->read(space, offset>>4);
	}
	else	// pseudo-DMA: read from the FIFO
	{
		return m_539x_1->read(space, 2);
	}

	return 0;
}

WRITE8_MEMBER(macpci_state::mac_5396_w)
{
	if (offset < 0x100)
	{
		m_539x_1->write(space, offset>>4, data);
	}
	else	// pseudo-DMA: write to the FIFO
	{
		m_539x_1->write(space, 2, data);
	}
}

WRITE_LINE_MEMBER(macpci_state::irq_539x_1_w)
{
}

WRITE_LINE_MEMBER(macpci_state::drq_539x_1_w)
{
}

static TIMER_CALLBACK(mac_6015_tick)
{
//  macpci_state *mac = machine.driver_data<macpci_state>();
}
