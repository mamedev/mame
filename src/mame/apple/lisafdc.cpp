// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The LISA FDC subsystem

#include "emu.h"
#include "lisafdc.h"

// Read command
// lisa 1: 136a
//   87 88 00 00 00 00 00 00 (wait until boot)
//   86 08 00 00 00 00 00 00
//   85 ff 00 00 00 00 00 00
//   81 00 00 00 00 00 00 00
//   85 cc 00 00 00 00 00 00
//   81 00 00 00 00 00 00 00

// lisa 2: 149a
//   86 80 00 00 00 00 00 00
//   85 ff 00 00 00 00 00 00
//   81 00 00 00 00 00 00 00

// lisa2 floppy failure
//    1db3 = write 10 at +b (fail!)
// -> 1d8a =
//    bp 1d7a

// macxl: 149e
//   87 88 00 00 00 00 00 00 (wait until boot)
//   86 80 00 00 00 00 00 00
//   85 ff 00 00 00 00 00 00
//   81 00 80 00 00 00 00 00
//   85 cc 80 00 00 00 00 00

// 050-4008-L_IWM_floppy_pg4.pdf

// 54321 (0=value)
// 00000 ph0
// 00001 ph1
// 00010 ph2
// 00011 ph3
// 00100 !hds
// 00101 enable/disable lss clocking (4MHz exactly)
// 00110 prom a2
// 00111 prom a3 & wrq
// 01000 dr0
// 01001 dr1
// 01010 !mt0
// 01011 !mt1
// 01100 dis
// 01101 nc
// 01110 dskdiag
// 01111 fdir

// latch read, ma0=0 && ma45=00


DEFINE_DEVICE_TYPE(LISAFDC,  lisa_fdc_device,  "lisafdc",  "Lisa 1 FDC subsystem")
DEFINE_DEVICE_TYPE(LISA2FDC, lisa2_fdc_device, "lisa2fdc", "Lisa 2 FDC subsystem")
DEFINE_DEVICE_TYPE(MACXLFDC, macxl_fdc_device, "macxlfdc", "Mac XL/Lisa 210 FDC subsystem")

ROM_START( lisa_fdc )
	ROM_REGION(0x100, "state", 0)
	ROM_LOAD("341-0172-a", 0x000, 0x0100, CRC(223f3917) SHA1(c7d32f091c3a02a0f8159bf51f6c810d23bd7b9f)) /* Very similar to the diskii's 341-0028 */

	ROM_REGION(0x1000, "cpu", 0)
	// note: other ?prototype? revisions of this rom for the lisa probably exist as well
	ROM_LOAD("341-0138-f", 0x0000, 0x1000, CRC(edd8d560) SHA1(872211d21386cd9625b3735d7682e2b2ecff05b4) )
ROM_END

ROM_START( lisa2_fdc )
	ROM_REGION(0x100, "state", 0)
	ROM_LOAD("341-0172-a", 0x000, 0x0100, CRC(223f3917) SHA1(c7d32f091c3a02a0f8159bf51f6c810d23bd7b9f)) /* Very similar to the diskii's 341-0028 */

	ROM_REGION(0x1000, "cpu",0)
	ROM_LOAD("341-0290-b", 0x0000, 0x1000, CRC(bc6364f1) SHA1(f3164923330a51366a06d9d8a4a01ec7b0d3a8aa)) // 341-0290-B LISA 2/5 Disk Rom (ioa8), supports profile on external port
ROM_END

ROM_START( macxl_fdc )
	ROM_REGION(0x1000, "cpu", 0)
	ROM_DEFAULT_BIOS("apple")

	ROM_SYSTEM_BIOS(0, "apple", "LISA 2/10 Disk Rom")
	ROMX_LOAD("341-0281-d", 0x0000, 0x1000, CRC(e343fe74) SHA1(a0e484ead2d2315fca261f39fff2f211ff61b0ef), ROM_BIOS(0)) // 341-0281-D LISA 2/10 Disk Rom (io88), supports widget on internal port

	ROM_SYSTEM_BIOS(1, "sun", "Sun Mac XL Disk rom for 800k drives (Rev C, from Goodwill XL)")
	ROMX_LOAD("341-8003-c", 0x0000, 0x1000, CRC(8c67959a) SHA1(aa446b0c4acb4cb6c9d0adfbbea900fb8c04c1e9), ROM_BIOS(1)) // 341-8003-C Sun Mac XL Disk rom for 800k drives (Rev C, from Goodwill XL) (io88800k)
	// Note: there are two earlier/alternate versions of this rom as well which are dumped */
ROM_END

const tiny_rom_entry *lisa_fdc_device::device_rom_region() const
{
	return ROM_NAME( lisa_fdc );
}

const tiny_rom_entry *lisa2_fdc_device::device_rom_region() const
{
	return ROM_NAME( lisa2_fdc );
}

const tiny_rom_entry *macxl_fdc_device::device_rom_region() const
{
	return ROM_NAME( macxl_fdc );
}

lisa_base_fdc_device::lisa_base_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, "cpu"),
	m_nvram(*this, "ram"),
	m_ram(*this, "ram"),
	m_floppy(*this, "%d", 0U),
	m_diag_cb(*this),
	m_fdir_cb(*this),
	m_cur_floppy(nullptr)
{
}

lisa_original_fdc_device::lisa_original_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	lisa_base_fdc_device(mconfig, type, tag, owner, clock)
{
}

lisa_fdc_device::lisa_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lisa_original_fdc_device(mconfig, LISAFDC, tag, owner, clock)
{
}

lisa2_fdc_device::lisa2_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lisa_original_fdc_device(mconfig, LISA2FDC, tag, owner, clock)
{
}

macxl_fdc_device::macxl_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lisa_base_fdc_device(mconfig, MACXLFDC, tag, owner, clock),
	m_iwm(*this, "iwm")
{
}

void lisa_original_fdc_device::device_add_mconfig(machine_config &config)
{
	M6504(config, m_cpu, 16_MHz_XTAL / 8);
	m_cpu->set_addrmap(AS_PROGRAM, &lisa_original_fdc_device::map);

	NVRAM(config, m_nvram);

	applefdintf_device::add_twiggy(config, m_floppy[0]);
	applefdintf_device::add_twiggy(config, m_floppy[1]);
}

void lisa_fdc_device::device_add_mconfig(machine_config &config)
{
	lisa_original_fdc_device::device_add_mconfig(config);
}

void lisa2_fdc_device::device_add_mconfig(machine_config &config)
{
	lisa_original_fdc_device::device_add_mconfig(config);
}

void macxl_fdc_device::device_add_mconfig(machine_config &config)
{
	M6504(config, m_cpu, 16_MHz_XTAL / 8);
	m_cpu->set_addrmap(AS_PROGRAM, &macxl_fdc_device::map);

	NVRAM(config, m_nvram);

	IWM(config, m_iwm, 16_MHz_XTAL / 2);
	m_iwm->devsel_cb().set(FUNC(macxl_fdc_device::devsel_w));
	m_iwm->phases_cb().set(FUNC(macxl_fdc_device::phases_w));

	applefdintf_device::add_35_nc(config, m_floppy[0]);
	applefdintf_device::add_35_sd(config, m_floppy[1]);
}

void lisa_base_fdc_device::device_start()
{
	save_item(NAME(m_hds));
	save_item(NAME(m_sel));
	save_item(NAME(m_phases));
}

void lisa_base_fdc_device::device_reset()
{
	m_sel = 0;
	m_hds = 0;
	m_phases = 0;
	update_sel();
}

u8 lisa_base_fdc_device::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void lisa_base_fdc_device::ram_w(offs_t offset, u8 data)
{
	m_ram[offset] = data;
}

void lisa_base_fdc_device::update_sel()
{
	if(m_sel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if(m_sel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	if(m_cur_floppy) {
		m_cur_floppy->ss_w(m_hds);
		m_cur_floppy->seek_phase_w(m_phases);
	}
}

void lisa_base_fdc_device::update_phases()
{
	if(m_cur_floppy)
		m_cur_floppy->seek_phase_w(m_phases);
}

void lisa_original_fdc_device::map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share(m_ram);

	map(0x0400, 0x0400).lr8(NAME([this]() -> u8 { ph0_0();  return 0; })).lw8(NAME([this](u8) { ph0_0();  }));
	map(0x0401, 0x0401).lr8(NAME([this]() -> u8 { ph0_1();  return 0; })).lw8(NAME([this](u8) { ph0_1();  }));
	map(0x0402, 0x0402).lr8(NAME([this]() -> u8 { ph1_0();  return 0; })).lw8(NAME([this](u8) { ph1_0();  }));
	map(0x0403, 0x0403).lr8(NAME([this]() -> u8 { ph1_1();  return 0; })).lw8(NAME([this](u8) { ph1_1();  }));
	map(0x0404, 0x0404).lr8(NAME([this]() -> u8 { ph2_0();  return 0; })).lw8(NAME([this](u8) { ph2_0();  }));
	map(0x0405, 0x0405).lr8(NAME([this]() -> u8 { ph2_1();  return 0; })).lw8(NAME([this](u8) { ph2_1();  }));
	map(0x0406, 0x0406).lr8(NAME([this]() -> u8 { ph3_0();  return 0; })).lw8(NAME([this](u8) { ph3_0();  }));
	map(0x0407, 0x0407).lr8(NAME([this]() -> u8 { ph3_1();  return 0; })).lw8(NAME([this](u8) { ph3_1();  }));
	map(0x0408, 0x0408).lr8(NAME([this]() -> u8 { hds_0();  return 0; })).lw8(NAME([this](u8) { hds_0();  }));
	map(0x0409, 0x0409).lr8(NAME([this]() -> u8 { hds_1();  return 0; })).lw8(NAME([this](u8) { hds_1();  }));

	map(0x0410, 0x0410).lr8(NAME([this]() -> u8 { dr1_0();  return 0; })).lw8(NAME([this](u8) { dr1_0();  }));
	map(0x0411, 0x0411).lr8(NAME([this]() -> u8 { dr1_1();  return 0; })).lw8(NAME([this](u8) { dr1_1();  }));
	map(0x0412, 0x0412).lr8(NAME([this]() -> u8 { dr2_0();  return 0; })).lw8(NAME([this](u8) { dr2_0();  }));
	map(0x0413, 0x0413).lr8(NAME([this]() -> u8 { dr2_1();  return 0; })).lw8(NAME([this](u8) { dr2_1();  }));
	map(0x0414, 0x0414).lr8(NAME([this]() -> u8 { mt0_0();  return 0; })).lw8(NAME([this](u8) { mt0_0();  }));
	map(0x0415, 0x0415).lr8(NAME([this]() -> u8 { mt0_1();  return 0; })).lw8(NAME([this](u8) { mt0_1();  }));
	map(0x0416, 0x0416).lr8(NAME([this]() -> u8 { mt1_0();  return 0; })).lw8(NAME([this](u8) { mt1_0();  }));
	map(0x0417, 0x0417).lr8(NAME([this]() -> u8 { mt1_1();  return 0; })).lw8(NAME([this](u8) { mt1_1();  }));
	map(0x0418, 0x0418).lr8(NAME([this]() -> u8 { dis_0();  return 0; })).lw8(NAME([this](u8) { dis_0();  }));
	map(0x0419, 0x0419).lr8(NAME([this]() -> u8 { dis_1();  return 0; })).lw8(NAME([this](u8) { dis_1();  }));
	map(0x041c, 0x041c).lr8(NAME([this]() -> u8 { diag_0(); return 0; })).lw8(NAME([this](u8) { diag_0(); }));
	map(0x041d, 0x041d).lr8(NAME([this]() -> u8 { diag_1(); return 0; })).lw8(NAME([this](u8) { diag_1(); }));
	map(0x041e, 0x041e).lr8(NAME([this]() -> u8 { fdir_0(); return 0; })).lw8(NAME([this](u8) { fdir_0(); }));
	map(0x041f, 0x041f).lr8(NAME([this]() -> u8 { fdir_1(); return 0; })).lw8(NAME([this](u8) { fdir_1(); }));

	map(0x1000, 0x1fff).rom().region("cpu", 0);
}

void macxl_fdc_device::map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share(m_ram);

	map(0x0800, 0x080f).rw(m_iwm, FUNC(iwm_device::read), FUNC(iwm_device::write));
	map(0x0810, 0x0810).lr8(NAME([this]() -> u8 { stop_0(); return 0; })).lw8(NAME([this](u8) { stop_0(); }));
	map(0x0811, 0x0811).lr8(NAME([this]() -> u8 { stop_1(); return 0; })).lw8(NAME([this](u8) { stop_1(); }));
	map(0x0814, 0x0814).lr8(NAME([this]() -> u8 { mt0_0();  return 0; })).lw8(NAME([this](u8) { mt0_0();  }));
	map(0x0815, 0x0815).lr8(NAME([this]() -> u8 { mt0_1();  return 0; })).lw8(NAME([this](u8) { mt0_1();  }));
	map(0x0816, 0x0816).lr8(NAME([this]() -> u8 { mt1_0();  return 0; })).lw8(NAME([this](u8) { mt1_0();  }));
	map(0x0817, 0x0817).lr8(NAME([this]() -> u8 { mt1_1();  return 0; })).lw8(NAME([this](u8) { mt1_1();  }));
	map(0x0818, 0x0818).lr8(NAME([this]() -> u8 { dis_0();  return 0; })).lw8(NAME([this](u8) { dis_0();  }));
	map(0x0819, 0x0819).lr8(NAME([this]() -> u8 { dis_1();  return 0; })).lw8(NAME([this](u8) { dis_1();  }));
	map(0x081a, 0x081a).lr8(NAME([this]() -> u8 { hds_0();  return 0; })).lw8(NAME([this](u8) { hds_0();  }));
	map(0x081b, 0x081b).lr8(NAME([this]() -> u8 { hds_1();  return 0; })).lw8(NAME([this](u8) { hds_1();  }));
	map(0x081c, 0x081c).lr8(NAME([this]() -> u8 { diag_0(); return 0; })).lw8(NAME([this](u8) { diag_0(); }));
	map(0x081d, 0x081d).lr8(NAME([this]() -> u8 { diag_1(); return 0; })).lw8(NAME([this](u8) { diag_1(); }));
	map(0x081e, 0x081e).lr8(NAME([this]() -> u8 { fdir_0(); return 0; })).lw8(NAME([this](u8) { fdir_0(); }));
	map(0x081f, 0x081f).lr8(NAME([this]() -> u8 { fdir_1(); return 0; })).lw8(NAME([this](u8) { fdir_1(); }));
	map(0x0820, 0x0820).w(FUNC(macxl_fdc_device::pwm_w));

	map(0x1000, 0x1fff).rom().region("cpu", 0);
}

void macxl_fdc_device::stop_0()
{
	logerror("stop 0\n");
}

void macxl_fdc_device::stop_1()
{
	logerror("stop 1\n");
}

void lisa_base_fdc_device::mt0_0()
{
	logerror("mt0 0\n");
}

void lisa_base_fdc_device::mt0_1()
{
	logerror("mt0 1\n");
}

void lisa_base_fdc_device::mt1_0()
{
	logerror("mt1 0\n");
}

void lisa_base_fdc_device::mt1_1()
{
	logerror("mt1 1\n");
}

void lisa_base_fdc_device::dis_0()
{
	logerror("dis 0\n");
}

void lisa_base_fdc_device::dis_1()
{
	logerror("dis 1\n");
}

void lisa_base_fdc_device::hds_0()
{
	m_hds = 0;
	if(m_cur_floppy)
		m_cur_floppy->ss_w(m_hds);
}

void lisa_base_fdc_device::hds_1()
{
	m_hds = 1;
	if(m_cur_floppy)
		m_cur_floppy->ss_w(m_hds);
}

void lisa_base_fdc_device::diag_0()
{
	m_diag_cb(0);
}

void lisa_base_fdc_device::diag_1()
{
	m_diag_cb(1);
}

void lisa_base_fdc_device::fdir_0()
{
	m_fdir_cb(0);
}

void lisa_base_fdc_device::fdir_1()
{
	m_fdir_cb(1);
}

void lisa_original_fdc_device::ph0_0()
{
	m_phases &= ~1;
	update_phases();
}

void lisa_original_fdc_device::ph0_1()
{
	m_phases |= 1;
	update_phases();
}

void lisa_original_fdc_device::ph1_0()
{
	m_phases &= ~2;
	update_phases();
}

void lisa_original_fdc_device::ph1_1()
{
	m_phases |= 2;
	update_phases();
}

void lisa_original_fdc_device::ph2_0()
{
	m_phases &= ~4;
	update_phases();
}

void lisa_original_fdc_device::ph2_1()
{
	m_phases |= 4;
	update_phases();
}

void lisa_original_fdc_device::ph3_0()
{
	m_phases &= ~8;
	update_phases();
}

void lisa_original_fdc_device::ph3_1()
{
	m_phases |= 8;
	update_phases();
}

void lisa_original_fdc_device::dr1_0()
{
	m_sel &= ~1;
	update_sel();
}

void lisa_original_fdc_device::dr1_1()
{
	m_sel |= 1;
	update_sel();
}

void lisa_original_fdc_device::dr2_0()
{
	m_sel &= ~2;
	update_sel();
}

void lisa_original_fdc_device::dr2_1()
{
	m_sel |= 2;
	update_sel();
}

void macxl_fdc_device::phases_w(uint8_t phases)
{
	m_phases = phases;
	update_phases();
}

void macxl_fdc_device::devsel_w(uint8_t devsel)
{
	m_sel = devsel;
	update_sel();
}

void macxl_fdc_device::update_sel()
{
	lisa_base_fdc_device::update_sel();
	m_iwm->set_floppy(m_cur_floppy);
	update_pwm();
}

void macxl_fdc_device::device_start()
{
	save_item(NAME(m_pwm));
}

void macxl_fdc_device::device_reset()
{
	m_pwm = 0;
}

void macxl_fdc_device::update_pwm()
{
	if(!m_cur_floppy)
		return;
	// Use a ramp somewhat similar to the macs
	// The documentation requires:
	// - duty cycle of 9.4%, 305 < rpm < 380 (middle 342.5, rom expectation 361)
	// - duty cycle of 91%,  625 < rpm < 780 (middle 702.5, rom expectation 723.5)
	// - linear between these two points

	// Assume 0 = duty cycle of 100%, ff = duty cycle of 0%

	float duty_cycle = (255 - m_pwm) / 255.0;
	float rpm = (duty_cycle - 0.094) * (723.5 - 361) / (0.91 - 0.094) + 361;

	logerror("rpm %02x -> %f (%s)\n", m_pwm, rpm, machine().describe_context());
	m_cur_floppy->set_rpm(rpm);
}

void macxl_fdc_device::pwm_w(u8 data)
{
	m_pwm = data;
	update_pwm();
}
