// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Interphase V/SCSI 4210 Jaguar High-Performance VMEbus Dual SCSI Host Adapter (aka Motorola MVME328 VMEbus Dual SCSI Host Adapter)
 *
 * Sources:
 *  - V/SCSI 4210 Jaguar High-Performance VMEbus Dual SCSI Host Adapter System Integration Guide, Interphase Corporation, March 13, 1991 (UG04210-000 REVD)
 *  - MVME328 VMEbus VMEbus Dual SCSI Host Adapter User's Manual (MVME328/D1)
 *
 * TODO:
 *  - scsi data transfer and dma
 *  - vme interrupts
 *  - leds
 *  - timer
 *  - smt variant
 */
/*
 * WIP notes
 * --
 * configuration block (+0x788 at 0x61ff88 = base 0x61f800)
 *  - product code = "077"
 *  - variation = "0"
 *  - firmware version = "01T"
 *  - firmware date "11 51992"
 *
 * LED1 - red/green, board OK (bit 1 in MSR, red indicates reset or failure)
 * LED2.0 - VMEbus busy (system)
 * LED2.1 - VMEbus busy (bus master)
 * LED2.2 - SCSI busy (port 0)
 * LED2.3 - not used
 */
#include "emu.h"
#include "mvme328.h"

#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_MVME328, vme_mvme328_device, "mvme328", "Motorola MVME328")

vme_mvme328_device::vme_mvme328_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_MVME328, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_scsi(*this, "scsi%u:7:mb87030", 0U)
	, m_bram(*this, "buffer_ram")
	, m_sw1(*this, "SW1")
	, m_led1(*this, "led1")
	, m_led2(*this, "led2.%u", 0U)
	, m_boot(*this, "boot")
{
}

ROM_START(mvme328)
	ROM_REGION16_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "077001t", "Interphase Jaguar 0770 firmware version 01T, 11 51992")
	ROMX_LOAD("0770k0801t.uk8",  0x0001, 0x8000, CRC(22f0440e) SHA1(37d8039778ec7ef0cb24940eabd45992920975a1), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("0770k1001t.uk10", 0x0000, 0x8000, CRC(d7d0d4c3) SHA1(5756719bf005e3e26be7bbc8cb1c2f771d260a02), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "077401s", "Interphase Jaguar 0774 firmware version 01S, 07311992")
	ROMX_LOAD("0774e1501s__9348.ue15", 0x0000, 0x8000, CRC(e614ef1f) SHA1(8bc56ad49989e78682a6d69351e8cf0f2557b311), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("0774f1501s__9348.uf15", 0x0001, 0x8000, CRC(3c6e8016) SHA1(0a39873b5a17f71db86afcd361141e5b9f6786e9), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(mvme328)
	PORT_START("SW1")
	PORT_DIPNAME(0x100, 0x000, "Permit User Access") PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(0x000, DEF_STR(No))
	PORT_DIPSETTING(0x100, DEF_STR(Yes))
	PORT_DIPNAME(0x0f8, 0x000, "Base Address") PORT_DIPLOCATION("SW1:4,5,6,7,8")
	PORT_DIPSETTING(0x000, "0x0000")
	PORT_DIPSETTING(0x008, "0x0800")
	PORT_DIPSETTING(0x010, "0x1000")
	PORT_DIPSETTING(0x018, "0x1800")
	PORT_DIPSETTING(0x020, "0x2000")
	PORT_DIPSETTING(0x028, "0x2800")
	PORT_DIPSETTING(0x030, "0x3000")
	PORT_DIPSETTING(0x038, "0x3800")
	PORT_DIPSETTING(0x040, "0x4000")
	PORT_DIPSETTING(0x048, "0x4800")
	PORT_DIPSETTING(0x050, "0x5000")
	PORT_DIPSETTING(0x058, "0x5800")
	PORT_DIPSETTING(0x060, "0x6000")
	PORT_DIPSETTING(0x068, "0x6800")
	PORT_DIPSETTING(0x070, "0x7000")
	PORT_DIPSETTING(0x078, "0x7800")
	PORT_DIPSETTING(0x080, "0x8000")
	PORT_DIPSETTING(0x088, "0x8800")
	PORT_DIPSETTING(0x090, "0x9000")
	PORT_DIPSETTING(0x098, "0x9800")
	PORT_DIPSETTING(0x0a0, "0xa000")
	PORT_DIPSETTING(0x0a8, "0xa800")
	PORT_DIPSETTING(0x0b0, "0xb000")
	PORT_DIPSETTING(0x0b8, "0xb800")
	PORT_DIPSETTING(0x0c0, "0xc000")
	PORT_DIPSETTING(0x0c8, "0xc800")
	PORT_DIPSETTING(0x0d0, "0xd000")
	PORT_DIPSETTING(0x0d8, "0xd800")
	PORT_DIPSETTING(0x0e0, "0xe000")
	PORT_DIPSETTING(0x0e8, "0xe800")
	PORT_DIPSETTING(0x0f0, "0xf000")
	PORT_DIPSETTING(0x0f8, "0xf800")
	PORT_DIPNAME(0x007, 0x007, "SCSI Port 0 ID") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x000, "0")
	PORT_DIPSETTING(0x001, "1")
	PORT_DIPSETTING(0x002, "2")
	PORT_DIPSETTING(0x003, "3")
	PORT_DIPSETTING(0x004, "4")
	PORT_DIPSETTING(0x005, "5")
	PORT_DIPSETTING(0x006, "6")
	PORT_DIPSETTING(0x007, "7")

	PORT_START("Priority")
	PORT_CONFNAME(0x03, 0x03, "VMEbus Request Priority")
	PORT_CONFSETTING(0x00, "0")
	PORT_CONFSETTING(0x01, "1")
	PORT_CONFSETTING(0x02, "2")
	PORT_CONFSETTING(0x03, "3")
INPUT_PORTS_END

const tiny_rom_entry *vme_mvme328_device::device_rom_region() const
{
	return ROM_NAME(mvme328);
}

ioport_constructor vme_mvme328_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme328);
}

void vme_mvme328_device::device_start()
{
	m_led1.resolve();
	m_led2.resolve();

	m_timer = timer_alloc(FUNC(vme_mvme328_device::timer), this);

	save_item(NAME(m_ctl));
	save_item(NAME(m_sts));

	m_ctl = 0;
}

void vme_mvme328_device::device_reset()
{
	m_timer->adjust(attotime::never);

	m_boot.select(0);

	// HACK: 0x0700 indicates an original board type?
	m_sts = (BIT(m_sw1->read(), 0, 3) << 12) | 0x0700;
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
}

void vme_mvme328_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 24_MHz_XTAL / 2); // HD68HC000P12
	m_cpu->set_addrmap(AS_PROGRAM, &vme_mvme328_device::cpu_mem);

	NSCSI_BUS(config, "scsi0");
	NSCSI_CONNECTOR(config, "scsi0:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("mb87030", MB87030).machine_config(
		[this](device_t *device)
		{
			mb87030_device &mb87030(downcast<mb87030_device &>(*device));

			mb87030.set_clock(8'000'000);
			mb87030.out_irq_callback().set(
				[this](int state)
				{
					if (state)
						m_sts |= 1U << 6;
					else
						m_sts &= ~(1U << 6);

					m_cpu->set_input_line(INPUT_LINE_IRQ6, state);
				});
		});

	NSCSI_BUS(config, "scsi1");
	NSCSI_CONNECTOR(config, "scsi1:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("mb87030", MB87030).machine_config(
		[this](device_t *device)
		{
			mb87030_device &mb87030(downcast<mb87030_device &>(*device));

			mb87030.set_clock(8'000'000);
			mb87030.out_irq_callback().set(
				[this](int state)
				{
					if (state)
						m_sts |= 1U << 5;
					else
						m_sts &= ~(1U << 5);

					m_cpu->set_input_line(INPUT_LINE_IRQ5, state);
				});
		});

	// TODO: VLSI VGC7224-0420 II-DMA-50M
}

void vme_mvme328_device::cpu_mem(address_map &map)
{
	map(0x00'0000, 0x00'ffff).view(m_boot);
	m_boot[0](0x00'0000, 0x00'ffff).rom().region("eprom", 0);
	m_boot[1](0x00'0000, 0x00'3fff).ram();

	map(0x20'0000, 0x20'ffff).rom().region("eprom", 0);
	map(0x60'0000, 0x61'ffff).ram().share("buffer_ram");
	map(0x68'0000, 0x69'ffff).noprw(); // event ram?

	map(0x80'0000, 0x80'0001).r(FUNC(vme_mvme328_device::sts_r));
	map(0xa0'0000, 0xa0'0001).rw(FUNC(vme_mvme328_device::ctl_r), FUNC(vme_mvme328_device::ctl_w));

	map(0xc0'0000, 0xc0'001f).m(m_scsi[0], FUNC(mb87030_device::map)).umask16(0x00ff);
	map(0xc0'8000, 0xc0'801f).m(m_scsi[1], FUNC(mb87030_device::map)).umask16(0x00ff);

	// 0xc0'c000 = smt version switches?

	map(0xc1'0000, 0xc1'0001).lw16([this](u16 data) { m_timer->adjust(attotime::from_msec(10)); }, "timer");
}

void vme_mvme328_device::timer(int param)
{
	m_cpu->set_input_line(INPUT_LINE_IRQ1, HOLD_LINE);
}

u16 vme_mvme328_device::sts_r()
{
	LOG("sts_r 0x%04x (%s)\n", m_sts, machine().describe_context());

	// .sss .??? iiii iiii
	// sss scsi id
	// ??? xor with same bits from 90'0000?, board type?
	// iii irq state?

	return m_sts;
}

u16 vme_mvme328_device::ctl_r()
{
	return m_ctl;
}

void vme_mvme328_device::ctl_w(u16 data)
{
	LOG("ctl_w 0x%04x (%s)\n", data, machine().describe_context());

	// bit 10: enable scratchpad ram?
	if (BIT(data, 10))
		m_boot.select(1);

	// bit 14: enable VME access?
	if (BIT(data, 14) && !BIT(m_ctl, 14))
	{
		unsigned const sw1 = m_sw1->read();

		// share last 2k of buffer RAM in VME supervisor short space (a16, privileged)
		offs_t const base = BIT(sw1, 3, 5) * 0x800U;
		vme_space(vme::AM_2d).install_ram(base, base + 0x7ffU, &m_bram[0xfc00]);

		// optionally share in user short space (a16, non-privileged)
		if (BIT(sw1, 8))
			vme_space(vme::AM_29).install_ram(base, base + 0x7ffU, &m_bram[0xfc00]);
	}

	// bit 15: led?

	m_ctl = data;
}
