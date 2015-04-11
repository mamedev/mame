#include "iteagle_fpga.h"
#include "coreutil.h"

#define LOG_FPGA            (1)
#define LOG_RTC             (0)
#define LOG_EEPROM          (1)
#define LOG_IDE             (0)
#define LOG_IDE_CTRL        (0)


const device_type ITEAGLE_FPGA = &device_creator<iteagle_fpga_device>;

DEVICE_ADDRESS_MAP_START(ctrl_map, 32, iteagle_fpga_device)
	AM_RANGE(0x000, 0x02f) AM_READWRITE(ctrl_r, ctrl_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(fpga_map, 32, iteagle_fpga_device)
	AM_RANGE(0x000, 0x01f) AM_READWRITE(fpga_r, fpga_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(rtc_map, 32, iteagle_fpga_device)
	AM_RANGE(0x000, 0x800) AM_READWRITE(rtc_r, rtc_w)
ADDRESS_MAP_END

iteagle_fpga_device::iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ITEAGLE_FPGA, "ITEagle FPGA", tag, owner, clock, "iteagle_fpga", __FILE__)
{
}

void iteagle_fpga_device::device_start()
{
	pci_device::device_start();
	status = 0x5555;
	command = 0x5555;

	add_map(sizeof(m_ctrl_regs), M_IO, FUNC(iteagle_fpga_device::ctrl_map));
	// ctrl defaults to base address 0x00000000
	bank_infos[0].adr = 0x00000000 & (~(bank_infos[0].size - 1));

	add_map(sizeof(m_fpga_regs), M_IO, FUNC(iteagle_fpga_device::fpga_map));
	// fpga defaults to base address 0x00000300
	bank_infos[1].adr = 0x00000300 & (~(bank_infos[1].size - 1));

	add_map(sizeof(m_rtc_regs), M_MEM, FUNC(iteagle_fpga_device::rtc_map));
	// RTC defaults to base address 0x000c0000
	bank_infos[2].adr = 0x000c0000 & (~(bank_infos[2].size - 1));
}

void iteagle_fpga_device::device_reset()
{
	pci_device::device_reset();
	memset(m_ctrl_regs, 0, sizeof(m_ctrl_regs));
	memset(m_fpga_regs, 0, sizeof(m_fpga_regs));
	memset(m_rtc_regs, 0, sizeof(m_rtc_regs));
	// 0x23 & 0x20 = IDE LED
	m_ctrl_regs[0x10/4] =  0x00000000; // 0xFFFFFFFF causes a write of 0xFFFEFFFF then 0xFFFFFFFF  // Not sure
	// 0x00&0x2 == 1 for boot
	m_fpga_regs[0x00/4] =  0xC0000002; // 0xCF000002;// byte 3 is voltage sensor? high = 0x40 good = 0xC0 0xF0 0xFF; //0x80 0x30 0x00FF = voltage low
	//m_fpga_regs[0x308/4]=0x0000ffff; // Low 16 bits gets read a lot?
	m_fpga_regs[0x08/4]=0x00000000; // Low 16 bits gets read a lot?
	m_prev_reg = 0;
}

READ32_MEMBER( iteagle_fpga_device::ctrl_r )
{
	UINT32 result = m_fpga_regs[offset];
	switch (offset) {
		case 0x0/4:
			if (LOG_FPGA)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_FPGA)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

WRITE32_MEMBER( iteagle_fpga_device::ctrl_w )
{
	COMBINE_DATA(&m_fpga_regs[offset]);
	switch (offset) {
		case 0x20/4: // IDE LED and ??
			if (ACCESSING_BITS_16_23) {
				// Probably watchdog
			} else if (ACCESSING_BITS_24_31) {
				// Bit 1 is IDE LED
			} else {
				if (LOG_FPGA)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			if (LOG_FPGA)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

READ32_MEMBER( iteagle_fpga_device::fpga_r )
{
	UINT32 result = m_fpga_regs[offset];
	switch (offset) {
		case 0x00/4:
			if (LOG_FPGA && (m_prev_reg != offset && m_prev_reg != (0x08/4)))
				logerror("%s:fpga read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x04/4:
			result =  (result & 0xFF0FFFFF) | (machine().root_device().ioport("SW5")->read()<<20); // Resolution
			if (LOG_FPGA)
				logerror("%s:fpga read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;

		case 0x08/4:
			if (LOG_FPGA && (m_prev_reg != offset && m_prev_reg != (0x00/4)))
				logerror("%s:fpga read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_FPGA)
				logerror("%s:fpga read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	m_prev_reg = offset;
	return result;
}

WRITE32_MEMBER( iteagle_fpga_device::fpga_w )
{
	UINT8 byte;

	COMBINE_DATA(&m_fpga_regs[offset]);
	switch (offset) {
		case 0x04/4:
			if (ACCESSING_BITS_0_7) {
				// ATMEL Chip access.  Returns version id's when bit 7 is set.
				byte = data & 0xFF;
				if (byte & 0x80) {
					switch (byte&0x3) {
						case 0:
							m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((machine().root_device().ioport("VERSION")->read()>>4)&0xF);
							break;
						case 1:
							m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((machine().root_device().ioport("VERSION")->read()>>8)&0xF);
							break;
						case 2:
							m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((machine().root_device().ioport("VERSION")->read()>>12)&0xF);
							break;
						case 3:
							m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((machine().root_device().ioport("VERSION")->read()>>0)&0xF);
							break;
					}
				} // Else???
			}
			if (LOG_FPGA)
					logerror("%s:fpga write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
		default:
			if (LOG_FPGA)
					logerror("%s:fpga write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}
//*************************************
//*  RTC M48T02
//*************************************
READ32_MEMBER( iteagle_fpga_device::rtc_r )
{
	UINT32 result = m_rtc_regs[offset];

	switch (offset) {
		default:
			if (LOG_RTC)
				logerror("%s:RTC read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}
WRITE32_MEMBER( iteagle_fpga_device::rtc_w )
{
	system_time systime;
	int raw[8];

	COMBINE_DATA(&m_rtc_regs[offset]);
	switch (offset) {
		case 0x7F8/4: // M48T02 time
			if (data & mem_mask & 0x40) {
				// get the current date/time from the core
				machine().current_datetime(systime);
				raw[0] = 0x40;
				raw[1] = dec_2_bcd(systime.local_time.second);
				raw[2] = dec_2_bcd(systime.local_time.minute);
				raw[3] = dec_2_bcd(systime.local_time.hour);

				raw[4] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
				raw[5] = dec_2_bcd(systime.local_time.mday);
				raw[6] = dec_2_bcd(systime.local_time.month + 1);
				raw[7] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
				m_rtc_regs[0x7F8/4] = (raw[3]<<24) | (raw[2]<<16) | (raw[1]<<8) | (raw[0] <<0);
				//m_rtc_regs[0x7FC/4] = (raw[7]<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
				m_rtc_regs[0x7FC/4] = (0x95<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
			}
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);

			break;
		default:
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}

}

//************************************
// Attached serial EEPROM
//************************************

const device_type ITEAGLE_EEPROM = &device_creator<iteagle_eeprom_device>;

DEVICE_ADDRESS_MAP_START(eeprom_map, 32, iteagle_eeprom_device)
	AM_RANGE(0x0000, 0x000F) AM_READWRITE(eeprom_r, eeprom_w) AM_SHARE("eeprom")
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( iteagle_eeprom )
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
MACHINE_CONFIG_END

machine_config_constructor iteagle_eeprom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( iteagle_eeprom );
}

iteagle_eeprom_device::iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ITEAGLE_EEPROM, "ITEagle EEPROM AT93C46", tag, owner, clock, "eeprom", __FILE__),
		m_eeprom(*this, "eeprom")
{
}

void iteagle_eeprom_device::device_start()
{
	pci_device::device_start();
	skip_map_regs(1);
	add_map(0x10, M_IO, FUNC(iteagle_eeprom_device::eeprom_map));
}

void iteagle_eeprom_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER( iteagle_eeprom_device::eeprom_r )
{
	UINT32 result = 0;

	switch (offset) {
		case 0xC/4: // I2C Handler
			if (ACCESSING_BITS_16_23) {
				result = m_eeprom->do_read()<<(16+3);
			}   else {
				if (LOG_EEPROM)
					logerror("%s:eeprom read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			}
			break;
		default:
			if (LOG_EEPROM)
				logerror("%s:eeprom read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

WRITE32_MEMBER( iteagle_eeprom_device::eeprom_w )
{
	switch (offset) {
		case 0xC/4: // I2C Handler
			if (ACCESSING_BITS_16_23) {
				m_eeprom->di_write((data  & 0x040000) >> (16+2));
				m_eeprom->cs_write((data  & 0x020000) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->clk_write((data & 0x010000) ? ASSERT_LINE : CLEAR_LINE);
			}   else {
				if (LOG_EEPROM)
					logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			if (LOG_EEPROM)
				logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

//************************************
// Attached IDE Controller
//************************************

const device_type ITEAGLE_IDE = &device_creator<iteagle_ide_device>;

DEVICE_ADDRESS_MAP_START(ide_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0xf) AM_READWRITE(ide_r, ide_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(ide_ctrl_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0x3) AM_READWRITE(ide_ctrl_r, ide_ctrl_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( iteagle_ide )
	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", NULL, true)
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
MACHINE_CONFIG_END

machine_config_constructor iteagle_ide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( iteagle_ide );
}
iteagle_ide_device::iteagle_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ITEAGLE_IDE, "ITEagle IDE Controller", tag, owner, clock, "ide", __FILE__),
		m_ide(*this, "ide")
{
}

void iteagle_ide_device::device_start()
{
	pci_device::device_start();
	add_map(0x10, M_IO, FUNC(iteagle_ide_device::ide_map));
	bank_infos[0].adr = 0x1f0;
	add_map(0x4, M_IO, FUNC(iteagle_ide_device::ide_ctrl_map));
	bank_infos[1].adr = 0x3f4;
}

void iteagle_ide_device::device_reset()
{
	pci_device::device_reset();
}
//*************************************
//*  IDE
//*************************************
READ32_MEMBER( iteagle_ide_device::ide_r )
{
	UINT32 result = m_ide->read_cs0(space, offset, mem_mask);
	if (LOG_IDE)
		logerror("%s:ide_r read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER( iteagle_ide_device::ide_w )
{
	m_ide->write_cs0(space, offset, data, mem_mask);
	if (LOG_IDE)
		logerror("%s:ide_w write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
READ32_MEMBER( iteagle_ide_device::ide_ctrl_r )
{
	UINT32 result = m_ide->read_cs1(space, offset+1, mem_mask);
	if (LOG_IDE_CTRL)
		logerror("%s:ide_ctrl_r read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER( iteagle_ide_device::ide_ctrl_w )
{
	m_ide->write_cs1(space, offset+1, data, mem_mask);
	if (LOG_IDE_CTRL)
		logerror("%s:ide_ctrl_w write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
