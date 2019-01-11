// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli
/***************************************************************************

fdc37c93x.h

SMSC FDC37C93x Plug and Play Compatible Ultra I/O Controller

***************************************************************************/

#include "emu.h"
#include "bus/isa/isa.h"
#include "machine/ds128x.h"
#include "machine/fdc37c93x.h"

DEFINE_DEVICE_TYPE(FDC37C93X, fdc37c93x_device, "fdc37c93x", "SMSC FDC37C93X")

fdc37c93x_device::fdc37c93x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDC37C93X, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, mode(OperatingMode::Run)
	, config_key_step(0)
	, config_index(0)
	, logical_device(0)
	, last_dma_line(-1)
	, m_gp20_reset_callback(*this)
	, m_gp25_gatea20_callback(*this)
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, floppy_controller_fdcdev(*this, "fdc")
	, pc_lpt_lptdev(*this, "lpt")
	, ds12885_rtcdev(*this, "rtc")
	, m_kbdc(*this, "pc_kbdc")
	, sysopt_pin(0)
{
	memset(global_configuration_registers, 0, sizeof(global_configuration_registers));
	global_configuration_registers[0x03] = 3;
	global_configuration_registers[0x20] = 2;
	global_configuration_registers[0x21] = 1;
	global_configuration_registers[0x24] = 4;
	memset(configuration_registers, 0, sizeof(configuration_registers));
	configuration_registers[0][0x60] = 3;
	configuration_registers[0][0x61] = 0xf0;
	configuration_registers[0][0x70] = 6;
	configuration_registers[0][0x74] = 2;
	configuration_registers[0][0xf0] = 0xe;
	configuration_registers[0][0xf2] = 0xff;
	configuration_registers[1][0x60] = 1;
	configuration_registers[1][0x61] = 0xf0;
	configuration_registers[1][0x62] = 3;
	configuration_registers[1][0x63] = 0xf6;
	configuration_registers[1][0x70] = 0xe;
	configuration_registers[3][0x74] = 4;
	configuration_registers[3][0xf0] = 0x3c;
	configuration_registers[6][0xf4] = 3;
	for (int n = 0xe0; n <= 0xed; n++)
		configuration_registers[8][n] = 1;
	for (int n = 0; n <= 8; n++)
		enabled_logical[n] = false;
	for (int n = 0; n < 4; n++)
		dreq_mapping[n] = -1;
}

/*
0 FDC:
60,61 03f0 +(0-7)
1 IDE1:
60,61 01f0 task file +(0-7)
62,63 03f6 misc
2 IDE2:
60,61 ?    task file +(0-7)
62,63 ?    misc
3 LPT:
60,61 ?    +(0-7) +400 +401 +402
4 SCI1:
60,61 ?    +(0-7)
5 SCI2:
60,61 ?    +(0-7)
6 RTC:
fixed 0x60 +0 +1
7 KBD:
fixed 0x70 +0 +4
8 aux:
60,61 ?    gpa
62,63 ?    gpw

 8042 p20 gp20 reset
 8042 p21 gp25 gatea20
 */

void fdc37c93x_device::eop_w(int state)
{
	// dma transfer finished
	if (last_dma_line < 0)
		return;
	switch (dreq_mapping[last_dma_line])
	{
	case LogicalDevice::FDC:
		floppy_controller_fdcdev->tc_w(state == ASSERT_LINE);
		break;
	default:
		break;
	}
	//last_dma_line = -1;
}

uint8_t fdc37c93x_device::dack_r(int line)
{
	// transferring data from device to memory using dma
	// read one byte from device
	last_dma_line = line;
	switch (dreq_mapping[line])
	{
	case LogicalDevice::FDC:
		return floppy_controller_fdcdev->dma_r();
		break;
	default:
		break;
	}
	return 0;
}

void fdc37c93x_device::dack_w(int line, uint8_t data)
{
	// transferring data from memory to device using dma
	// write one byte to device
	last_dma_line = line;
	switch (dreq_mapping[line])
	{
	case LogicalDevice::FDC:
		floppy_controller_fdcdev->dma_w(data);
		break;
	default:
		break;
	}
}

void fdc37c93x_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

void fdc37c93x_device::request_dma(int dreq, int state)
{
	switch (dreq)
	{
	case 0:
		m_isa->drq0_w(state);
		break;
	case 1:
		m_isa->drq1_w(state);
		break;
	case 2:
		m_isa->drq2_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	}
}

uint16_t fdc37c93x_device::get_base_address(int logical, int index)
{
	int position = index * 2 + 0x60;

	return ((uint16_t)configuration_registers[logical][position] << 8) + (uint16_t)configuration_registers[logical][position + 1];
}

// internal device "logical" uses dma pin "dreq"
void fdc37c93x_device::update_dreq_mapping(int dreq, int logical)
{
	if ((dreq < 0) || (dreq >= 4))
		return;
	for (int n = 0; n < 4; n++)
		if (dreq_mapping[n] == logical)
			dreq_mapping[n] = -1;
	dreq_mapping[dreq] = logical;
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

FLOPPY_FORMATS_MEMBER(fdc37c93x_device::floppy_formats)
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

MACHINE_CONFIG_START(fdc37c93x_device::device_add_mconfig)
	// floppy disc controller
	MCFG_SMC37C78_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(*this, fdc37c93x_device, irq_floppy_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(*this, fdc37c93x_device, drq_floppy_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", fdc37c93x_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", fdc37c93x_device::floppy_formats)
	// parallel port
	MCFG_DEVICE_ADD("lpt", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(WRITELINE(*this, fdc37c93x_device, irq_parallel_w))
	// RTC
	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(*this, fdc37c93x_device, irq_rtc_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)
	// keyboard
	MCFG_DEVICE_ADD("pc_kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_PS2)
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(WRITELINE(*this, fdc37c93x_device, irq_keyboard_w))
	MCFG_KBDC8042_SYSTEM_RESET_CB(WRITELINE(*this, fdc37c93x_device, kbdp20_gp20_reset_w))
	MCFG_KBDC8042_GATE_A20_CB(WRITELINE(*this, fdc37c93x_device, kbdp21_gp25_gatea20_w))
MACHINE_CONFIG_END

WRITE_LINE_MEMBER(fdc37c93x_device::irq_floppy_w)
{
	if (enabled_logical[LogicalDevice::FDC] == false)
		return;
	request_irq(configuration_registers[LogicalDevice::FDC][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(fdc37c93x_device::drq_floppy_w)
{
	if (enabled_logical[LogicalDevice::FDC] == false)
		return;
	request_dma(configuration_registers[LogicalDevice::FDC][0x74], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(fdc37c93x_device::irq_parallel_w)
{
	if (enabled_logical[LogicalDevice::Parallel] == false)
		return;
	request_irq(configuration_registers[LogicalDevice::Parallel][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(fdc37c93x_device::irq_rtc_w)
{
	if (enabled_logical[LogicalDevice::RTC] == false)
		return;
	request_irq(configuration_registers[LogicalDevice::RTC][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(fdc37c93x_device::irq_keyboard_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	request_irq(configuration_registers[LogicalDevice::Keyboard][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(fdc37c93x_device::kbdp21_gp25_gatea20_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	m_gp25_gatea20_callback(state);
}

WRITE_LINE_MEMBER(fdc37c93x_device::kbdp20_gp20_reset_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	m_gp20_reset_callback(state);
}

READ8_MEMBER(fdc37c93x_device::read_fdc37c93x)
{
	if (offset == 0)
	{
		if (mode == OperatingMode::Run)
			return 0;
		return config_index;
	}
	else if (offset == 1)
	{
		if (mode == OperatingMode::Run)
			return 0;
		if (config_index < 0x30)
			return read_global_configuration_register(config_index);
		else
			return read_logical_configuration_register(config_index);
	}
	else
		return 0;
}

WRITE8_MEMBER(fdc37c93x_device::write_fdc37c93x)
{
	uint8_t byt;

	if (offset == 0)
	{
		byt = data & 0xff;
		if (mode == OperatingMode::Run)
		{
			if (byt != 0x55)
				return;
			config_key_step++;
			if (config_key_step > 1)
			{
				config_key_step = 0;
				mode = OperatingMode::Configuration;
			}
		}
		else
		{
			if (byt == 0xaa)
			{
				mode = OperatingMode::Run;
				return;
			}
			config_index = byt;
		}
	}
	else if (offset == 1)
	{
		if (mode == OperatingMode::Run)
			return;
		byt = data & 0xff;
		if (config_index < 0x30)
			write_global_configuration_register(config_index, byt);
		else
			write_logical_configuration_register(config_index, byt);;
	}
	else
		return;
}

/* Map/unmap internal devices */

#if 1
READ8_MEMBER(fdc37c93x_device::disabled_read)
{
	return 0xff;
}

WRITE8_MEMBER(fdc37c93x_device::disabled_write)
{
}

void fdc37c93x_device::unmap_fdc(address_map &map)
{
	map(0x0, 0x0).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x1, 0x1).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x2, 0x2).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x3, 0x3).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x4, 0x4).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x5, 0x5).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
	map(0x7, 0x7).rw(this, FUNC(fdc37c93x_device::disabled_read), FUNC(fdc37c93x_device::disabled_write));
}
#else
void fdc37c93x_device::unmap_fdc(address_map &map)
{
	map(0x0, 0x0).noprw();
	map(0x1, 0x1).noprw();
	map(0x2, 0x2).noprw();
	map(0x3, 0x3).noprw();
	map(0x4, 0x4).noprw();
	map(0x5, 0x5).noprw();
	map(0x7, 0x7).noprw();
}
#endif

void fdc37c93x_device::map_fdc_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::FDC, 0);

	m_isa->install_device(base, base + 7, *floppy_controller_fdcdev, &pc_fdc_interface::map);
}

void fdc37c93x_device::unmap_fdc_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::FDC, 0);

	m_isa->install_device(base, base + 7, *this, &fdc37c93x_device::unmap_fdc);
}

void fdc37c93x_device::map_lpt(address_map &map)
{
	map(0x0, 0x3).rw(this, FUNC(fdc37c93x_device::lpt_read), FUNC(fdc37c93x_device::lpt_write));
}

READ8_MEMBER(fdc37c93x_device::lpt_read)
{
	return pc_lpt_lptdev->read(space, offset, mem_mask);
}

WRITE8_MEMBER(fdc37c93x_device::lpt_write)
{
	pc_lpt_lptdev->write(space, offset, data, mem_mask);
}

void fdc37c93x_device::map_lpt_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Parallel, 0);

	m_isa->install_device(base, base + 3, *this, &fdc37c93x_device::map_lpt);
}

void fdc37c93x_device::unmap_lpt_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Parallel, 0);

	m_isa->unmap_device(base, base + 3);
}

void fdc37c93x_device::map_rtc(address_map &map)
{
	map(0x0, 0xf).rw(this, FUNC(fdc37c93x_device::rtc_read), FUNC(fdc37c93x_device::rtc_write));
}

READ8_MEMBER(fdc37c93x_device::rtc_read)
{
	return ds12885_rtcdev->read(space, offset, mem_mask);
}

WRITE8_MEMBER(fdc37c93x_device::rtc_write)
{
	ds12885_rtcdev->write(space, offset, data, mem_mask);
}

void fdc37c93x_device::map_rtc_addresses()
{
	uint16_t base = 0x70;

	m_isa->install_device(base, base + 0xf, *this, &fdc37c93x_device::map_rtc);
}

void fdc37c93x_device::unmap_rtc_addresses()
{
	uint16_t base = 0x70;

	m_isa->unmap_device(base, base + 0xf);
}

void fdc37c93x_device::map_keyboard(address_map &map)
{
	map(0x0, 0x0).rw(this, FUNC(fdc37c93x_device::at_keybc_r), FUNC(fdc37c93x_device::at_keybc_w));
	map(0x4, 0x4).rw(this, FUNC(fdc37c93x_device::keybc_status_r), FUNC(fdc37c93x_device::keybc_command_w));
}

void fdc37c93x_device::unmap_keyboard(address_map &map)
{
	map(0x0, 0x0).noprw();
	map(0x4, 0x4).noprw();
}

READ8_MEMBER(fdc37c93x_device::at_keybc_r)
{
	switch (offset) //m_kbdc
	{
	case 0:
		return m_kbdc->data_r(space, 0);
	}

	return 0xff;
}

WRITE8_MEMBER(fdc37c93x_device::at_keybc_w)
{
	switch (offset)
	{
	case 0:
		m_kbdc->data_w(space, 0, data);
	}
}

READ8_MEMBER(fdc37c93x_device::keybc_status_r)
{
	return (m_kbdc->data_r(space, 4) & 0xfb) | 0x10; // bios needs bit 2 to be 0 as powerup and bit 4 to be 1
}

WRITE8_MEMBER(fdc37c93x_device::keybc_command_w)
{
	m_kbdc->data_w(space, 4, data);
}

void fdc37c93x_device::map_keyboard_addresses()
{
	uint16_t base = 0x60;

	m_isa->install_device(base, base + 7, *this, &fdc37c93x_device::map_keyboard);
}

void fdc37c93x_device::unmap_keyboard_addresses()
{
	uint16_t base = 0x60;

	m_isa->install_device(base, base + 7, *this, &fdc37c93x_device::unmap_keyboard);
}

void fdc37c93x_device::remap(int space_id, offs_t start, offs_t end)
{
	//printf("remapping needed %d %d %d\n\r", space_id, start, end);
}

/* Register access */

void fdc37c93x_device::write_global_configuration_register(int index, int data)
{
	global_configuration_registers[index] = data;
	switch (index)
	{
	case 7:
		logical_device = data;
		break;
	}
	logerror("Write global configuration register %02X = %02X\n", index, data);
}

void fdc37c93x_device::write_logical_configuration_register(int index, int data)
{
	configuration_registers[logical_device][index] = data;
	switch (logical_device)
	{
	case 0:
		write_fdd_configuration_register(index, data);
		break;
	case 1:
		write_ide1_configuration_register(index, data);
		break;
	case 2:
		write_ide2_configuration_register(index, data);
		break;
	case 3:
		write_parallel_configuration_register(index, data);
		break;
	case 4:
		write_serial1_configuration_register(index, data);
		break;
	case 5:
		write_serial2_configuration_register(index, data);
		break;
	case 6:
		write_rtc_configuration_register(index, data);
		break;
	case 7:
		write_keyboard_configuration_register(index, data);
		break;
	case 8:
		write_auxio_configuration_register(index, data);
		break;
	}
	logerror("Write logical configuration register %02X: %02X = %02X\n", logical_device, index, data);
}

void fdc37c93x_device::write_fdd_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::FDC] == false)
				map_fdc_addresses();
			enabled_logical[LogicalDevice::FDC] = true;
			logerror("Enabled FDD at %04X\n", get_base_address(LogicalDevice::FDC, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::FDC] == true)
				unmap_fdc_addresses();
			enabled_logical[LogicalDevice::FDC] = false;
			logerror("Disabled FDD at %04X\n", get_base_address(LogicalDevice::FDC, 0));
		}
	}
	if (index == 0x74)
		update_dreq_mapping(configuration_registers[LogicalDevice::FDC][0x74], LogicalDevice::FDC);
}

void fdc37c93x_device::write_parallel_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Parallel] == false)
				map_lpt_addresses();
			enabled_logical[LogicalDevice::Parallel] = true;
			logerror("Enabled LPT at %04X\n", get_base_address(LogicalDevice::Parallel, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::Parallel] == true)
				unmap_lpt_addresses();
			enabled_logical[LogicalDevice::Parallel] = false;
		}
	}
}

void fdc37c93x_device::write_rtc_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::RTC] == false)
				map_rtc_addresses();
			enabled_logical[LogicalDevice::RTC] = true;
			logerror("Enabled RTC\n");
		}
		else
		{
			if (enabled_logical[LogicalDevice::RTC] == true)
				unmap_rtc_addresses();
			enabled_logical[LogicalDevice::RTC] = false;
		}
	}
}

void fdc37c93x_device::write_keyboard_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Keyboard] == false)
				map_keyboard_addresses();
			enabled_logical[LogicalDevice::Keyboard] = true;
			logerror("Enabled Keyboard\n");
		}
		else
		{
			if (enabled_logical[LogicalDevice::Keyboard] == true)
				unmap_keyboard_addresses();
			enabled_logical[LogicalDevice::Keyboard] = false;
		}
	}
}

void fdc37c93x_device::write_auxio_configuration_register(int index, int data)
{
	if (index == 0x30)
		if (data & 1)
			logerror("Enabled Aux I/O\n");
	if (index == 0xe8)
	{
		if (data & 16)
			logerror("GP20 used as 8042 P20 keyboard reset line\n");;
	}
	if (index == 0xed)
	{
		if (data & 8)
			logerror("GP25 used as GATEA20 line\n");;
	}
}

uint16_t fdc37c93x_device::read_global_configuration_register(int index)
{
	uint16_t ret = 0;

	ret = global_configuration_registers[index];
	switch (index)
	{
	case 7:
		ret = logical_device;
		break;
	case 0x20:
		ret = 2;
		break;
	case 0x21:
		ret = 1;
		break;
	}
	logerror("Read global configuration register %02X = %02X\n", index, ret);
	return ret;
}

uint16_t fdc37c93x_device::read_logical_configuration_register(int index)
{
	uint16_t ret = 0;

	switch (logical_device)
	{
	case 0:
		ret = read_fdd_configuration_register(index);
		break;
	case 1:
		ret = read_ide1_configuration_register(index);
		break;
	case 2:
		ret = read_ide2_configuration_register(index);
		break;
	case 3:
		ret = read_parallel_configuration_register(index);
		break;
	case 4:
		ret = read_serial1_configuration_register(index);
		break;
	case 5:
		ret = read_serial2_configuration_register(index);
		break;
	case 6:
		ret = read_rtc_configuration_register(index);
		break;
	case 7:
		ret = read_keyboard_configuration_register(index);
		break;
	case 8:
		ret = read_auxio_configuration_register(index);
		break;
	}
	logerror("Read logical configuration register %02X: %02X = %02X\n", logical_device, index, ret);
	return ret;
}

uint16_t fdc37c93x_device::read_rtc_configuration_register(int index)
{
	return configuration_registers[6][index];
}

uint16_t fdc37c93x_device::read_keyboard_configuration_register(int index)
{
	return configuration_registers[7][index];
}

uint16_t fdc37c93x_device::read_auxio_configuration_register(int index)
{
	return configuration_registers[8][index];
}

/* Device management */

void fdc37c93x_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(0, this, true);
	m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	m_isa->set_dma_channel(3, this, true);
	if (sysopt_pin == 0)
		m_isa->install_device(0x03f0, 0x03f3, read8_delegate(FUNC(fdc37c93x_device::read_fdc37c93x), this), write8_delegate(FUNC(fdc37c93x_device::write_fdc37c93x), this));
	else
		m_isa->install_device(0x0370, 0x0373, read8_delegate(FUNC(fdc37c93x_device::read_fdc37c93x), this), write8_delegate(FUNC(fdc37c93x_device::write_fdc37c93x), this));
	m_gp20_reset_callback.resolve_safe();
	m_gp25_gatea20_callback.resolve_safe();
	m_irq1_callback.resolve_safe();
	m_irq8_callback.resolve_safe();
	m_irq9_callback.resolve_safe();
}

void fdc37c93x_device::device_reset()
{
}
