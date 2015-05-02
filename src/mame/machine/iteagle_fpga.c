#include "iteagle_fpga.h"
#include "coreutil.h"

#define LOG_FPGA            (0)
#define LOG_RTC             (0)
#define LOG_EEPROM          (0)
#define LOG_IDE             (0)
#define LOG_IDE_CTRL        (0)


const device_type ITEAGLE_FPGA = &device_creator<iteagle_fpga_device>;

DEVICE_ADDRESS_MAP_START(fpga_map, 32, iteagle_fpga_device)
	AM_RANGE(0x000, 0x01f) AM_READWRITE(fpga_r, fpga_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(rtc_map, 32, iteagle_fpga_device)
	AM_RANGE(0x000, 0x7ff) AM_READWRITE(rtc_r, rtc_w)
ADDRESS_MAP_END

iteagle_fpga_device::iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ITEAGLE_FPGA, "ITEagle FPGA", tag, owner, clock, "iteagle_fpga", __FILE__),
		device_nvram_interface(mconfig, *this)
{
}

void iteagle_fpga_device::device_start()
{
	pci_device::device_start();
	status = 0x5555;
	command = 0x5555;

	add_map(sizeof(m_fpga_regs), M_IO, FUNC(iteagle_fpga_device::fpga_map));
	// fpga defaults to base address 0x00000300
	bank_infos[0].adr = 0x00000300 & (~(bank_infos[0].size - 1));

	add_map(sizeof(m_rtc_regs), M_MEM, FUNC(iteagle_fpga_device::rtc_map));
	// RTC defaults to base address 0x000c0000
	bank_infos[1].adr = 0x000c0000 & (~(bank_infos[1].size - 1));
}

void iteagle_fpga_device::device_reset()
{
	pci_device::device_reset();
	memset(m_fpga_regs, 0, sizeof(m_fpga_regs));
	//memset(m_rtc_regs, 0, sizeof(m_rtc_regs));
	//m_rtc_regs[0] = 0x11223344;
	switch ((machine().root_device().ioport("VERSION")->read()>>4)&0xF) {
		case 3:
			m_seq = 0x0a0b0a; // gt02
			break;
		case 4:
			m_seq = 0x0a020b; // gt04
			break;
		case 5:
			m_seq = 0x0b0a0c; // gt05
			break;
		default:
			m_seq = 0x0c0b0d; // gt06
			break;
	}

	m_seq_rem1 = 0;
	m_seq_rem2 = 0;

	// 0x00&0x2 == 1 for boot
	//m_fpga_regs[0x00/4] =  0xC1110002; // 0xCF000002;// byte 3 is voltage sensor? high = 0x40 good = 0xC0 0xF0 0xFF; //0x80 0x30 0x00FF = voltage low
	//m_fpga_regs[0x00/4] =  0xC010ffff;
	// byte 3 is voltage sensor? high = 0x40 good = 0xC0 0xF0 0xFF; //0x80 0x30 0x00FF = voltage low
	// Bit 20 seems to be sw51 (operator mode) 0 = Normal, 1 = Operator Mode
	//m_fpga_regs[0x04/4] =  0x06060044; // Nibble starting at bit 20 is resolution, byte 0 is atmel response
	m_fpga_regs[0x04/4] =  0x00000000; // Nibble starting at bit 20 is resolution, byte 0 is atmel response
	//m_fpga_regs[0x308/4]=0x0000ffff; // Low 16 bits gets read alot?
	//m_fpga_regs[0x08/4]=0x0000ffff; // Low 16 bits is trackball
	m_prev_reg = 0;
}

void iteagle_fpga_device::update_sequence(UINT32 data)
{
	UINT32 offset = 0x04/4;
	if (data & 0x80) {
		switch (data&0x3) {
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
	} else {
		UINT32 val1, feed;
		feed = ((m_seq<<4) ^ m_seq)>>7;
		if (data & 0x1) {
			val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
			m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
			m_seq_rem2 = ((m_seq & 0x80)>>1) | ((m_seq & 0x100)>>3) | ((m_seq & 0x200)>>5);
			m_seq = (m_seq>>9) | ((feed&0x1ff)<<15);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
		} else if (data & 0x2) {
			val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
			m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
			m_seq = (m_seq>>6) | ((feed&0x3f)<<18);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
		} else {
			val1 = ((m_seq & 0x2)<<6) | ((m_seq & 0x4)<<4) | ((m_seq & 0x8)<<2) | ((m_seq & 0x10)<<0)
					| ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4) | ((m_seq & 0x80)>>6) | ((m_seq & 0x100)>>8);
			m_seq = (m_seq>>8) | ((feed&0xff)<<16);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2) & 0xff);
		}
		if (0 && LOG_FPGA)
			logerror("%s:fpga update_sequence In: %02X Seq: %06X Out: %02X\n", machine().describe_context(), data, m_seq, m_fpga_regs[offset]&0xff);
	}
}

READ32_MEMBER( iteagle_fpga_device::fpga_r )
{
	UINT32 result = m_fpga_regs[offset];
	switch (offset) {
		case 0x00/4:
			result = ((machine().root_device().ioport("SYSTEM")->read()&0xffff)<<16) | (machine().root_device().ioport("IN1")->read()&0xffff);
			if (1 && LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x04/4:
			result =  (result & 0xFF0FFFFF) | ((machine().root_device().ioport("SW5")->read()&0xf)<<20);
			//if (0 && LOG_FPGA  && ACCESSING_BITS_0_7)
			if (1 && LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;

		case 0x08/4:
			result = (result & 0xffff0000) | ((machine().root_device().ioport("TRACKY1")->read()&0xff)<<8) | (machine().root_device().ioport("TRACKX1")->read()&0xff);
			if (1 && LOG_FPGA && m_prev_reg != offset)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x1c/4: // 1d = modem byte
			result =  (result & 0xFFFFFF00) | 0x04;
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	m_prev_reg = offset;
	return result;
}

WRITE32_MEMBER( iteagle_fpga_device::fpga_w )
{
	COMBINE_DATA(&m_fpga_regs[offset]);
	switch (offset) {
		case 0x04/4:
			if (ACCESSING_BITS_0_7) {
				// ATMEL Chip access.  Returns version id's when bit 7 is set.
				update_sequence(data & 0xff);
				if (0 && LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			} else {
				if (LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			if (LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}
//*************************************
//*  RTC M48T02
//*************************************

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void iteagle_fpga_device::nvram_default()
{
	memset(m_rtc_regs, 0, sizeof(m_rtc_regs));
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------
void iteagle_fpga_device::nvram_read(emu_file &file)
{
	file.read(m_rtc_regs, sizeof(m_rtc_regs));
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------
void iteagle_fpga_device::nvram_write(emu_file &file)
{
	file.write(m_rtc_regs, sizeof(m_rtc_regs));
}

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
				m_rtc_regs[0x7FC/4] = (raw[7]<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
				//m_rtc_regs[0x7FC/4] = (0x95<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
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

// When corrupt writes 0x3=2, 0x3e=2, 0xa=0, 0x30=0
// 0x4 = HW Version
// 0x5 = Serial Num + top byte of 0x4
// 0x6 = OperID
// 0xe = SW Version
// 0x7f = checksum
static const UINT16 iteagle_default_eeprom[0x40] =
{
	0x0011,0x0022,0x0033,0x0002,0x1206,0x1111,0x2222,0x1234,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x0000
};

static MACHINE_CONFIG_FRAGMENT( iteagle_eeprom )
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(iteagle_default_eeprom, 0x80)
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
	// Set software version and calc crc
	m_eeprom->write(0xe, (machine().root_device().ioport("VERSION")->read()&0xFF00) |
			(((machine().root_device().ioport("VERSION")->read()>>4)&0x0F)));
	UINT16 checkSum = 0;
	for (int i=0; i<0x3f; i++) {
		checkSum += m_eeprom->read(i);
		//logerror("eeprom init i: %x data: %04x\n", i, m_eeprom->read(i));
	}
	m_eeprom->write(0x3f, checkSum);
	pci_device::device_reset();
}

READ32_MEMBER( iteagle_eeprom_device::eeprom_r )
{
	UINT32 result = 0;

	switch (offset) {
		case 0xC/4: // I2C Handler
			if (ACCESSING_BITS_16_23) {
				result = m_eeprom->do_read()<<(16+3);
				if (LOG_EEPROM)
					logerror("%s:eeprom read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			}   else {
					logerror("%s:eeprom read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			}
			break;
		default:
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
				if (LOG_EEPROM)
					logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}   else {
				//if (LOG_EEPROM)
					logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			//if (LOG_EEPROM)
				logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

//************************************
// Attached IDE Controller
//************************************

const device_type ITEAGLE_IDE = &device_creator<iteagle_ide_device>;

DEVICE_ADDRESS_MAP_START(ctrl_map, 32, iteagle_ide_device)
	AM_RANGE(0x000, 0x02f) AM_READWRITE(ctrl_r, ctrl_w)
ADDRESS_MAP_END


DEVICE_ADDRESS_MAP_START(ide_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0x7) AM_READWRITE(ide_r, ide_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(ide_ctrl_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0x3) AM_READWRITE(ide_ctrl_r, ide_ctrl_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(ide2_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0x7) AM_READWRITE(ide2_r, ide2_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(ide2_ctrl_map, 32, iteagle_ide_device)
	AM_RANGE(0x0, 0x3) AM_READWRITE(ide2_ctrl_r, ide2_ctrl_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( iteagle_ide )
	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, NULL, "cdrom", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(iteagle_ide_device, ide_interrupt))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide2", ata_devices, "hdd", NULL, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(iteagle_ide_device, ide2_interrupt))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
MACHINE_CONFIG_END

machine_config_constructor iteagle_ide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( iteagle_ide );
}

iteagle_ide_device::iteagle_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ITEAGLE_IDE, "ITEagle IDE Controller", tag, owner, clock, "ide", __FILE__),
		m_ide(*this, "ide"),
		m_ide2(*this, "ide2"),
		m_irq_num(-1)
{
}

void iteagle_ide_device::set_irq_info(const char *tag, const int irq_num)
{
	m_cpu_tag = tag;
	m_irq_num = irq_num;
}

void iteagle_ide_device::device_start()
{
	m_cpu = machine().device<cpu_device>(m_cpu_tag);
	pci_device::device_start();
	add_map(sizeof(m_ctrl_regs), M_IO, FUNC(iteagle_ide_device::ctrl_map));
	// ctrl defaults to base address 0x00000000
	bank_infos[0].adr = 0x000;

	add_map(0x8, M_IO, FUNC(iteagle_ide_device::ide_map));
	bank_infos[1].adr = 0x170;
	add_map(0x4, M_IO, FUNC(iteagle_ide_device::ide_ctrl_map));
	bank_infos[2].adr = 0x374;
	add_map(0x8, M_IO, FUNC(iteagle_ide_device::ide2_map));
	bank_infos[3].adr = 0x1f0;
	add_map(0x4, M_IO, FUNC(iteagle_ide_device::ide2_ctrl_map));
	bank_infos[4].adr = 0x3f4;
}

void iteagle_ide_device::device_reset()
{
	pci_device::device_reset();
	memset(m_ctrl_regs, 0, sizeof(m_ctrl_regs));
	// 0x23 & 0x20 = IDE LED
	m_ctrl_regs[0x10/4] =  0x00000000; // 0x6=No SIMM, 0x2, 0x0 = SIMM
}

READ32_MEMBER( iteagle_ide_device::ctrl_r )
{
	UINT32 result = m_ctrl_regs[offset];
	switch (offset) {
		case 0x0/4:
			if (LOG_IDE_CTRL)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_IDE_CTRL)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

WRITE32_MEMBER( iteagle_ide_device::ctrl_w )
{
	COMBINE_DATA(&m_ctrl_regs[offset]);
	switch (offset) {
		case 0x20/4: // IDE LED and ??
			if (ACCESSING_BITS_16_23) {
				// Probably watchdog
				if (1 && LOG_IDE_CTRL)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			} else if (ACCESSING_BITS_24_31) {
				// Bit 25 is IDE LED
				if (1 && LOG_IDE_CTRL)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			} else {
				if (LOG_IDE_CTRL)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			if (LOG_IDE_CTRL)
					logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

READ32_MEMBER( iteagle_ide_device::ide_r )
{
	UINT32 result = m_ide->read_cs0(space, offset, mem_mask);
	if (offset==0x4/4 && ACCESSING_BITS_24_31) {
		//result = 0;
		if ((m_irq_num!=-1) && m_ctrl_regs[0x20/4]&0x80000000) {
			m_cpu->set_input_line(m_irq_num, CLEAR_LINE);
			if (LOG_IDE)
				logerror("%s:ide_interrupt Clearing interrupt\n", machine().describe_context());
		}
	}
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
WRITE_LINE_MEMBER(iteagle_ide_device::ide_interrupt)
{
	//cpu_device *m_cpu = machine().device<cpu_device>(":maincpu");
	if ((m_irq_num!=-1) && m_ctrl_regs[0x20/4]&0x80000000) {
		m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
		if (LOG_IDE_CTRL)
			logerror("%s:ide_interrupt Setting interrupt\n", machine().describe_context());
	}
}

READ32_MEMBER( iteagle_ide_device::ide2_r )
{
	UINT32 result = m_ide2->read_cs0(space, offset, mem_mask);
	if (offset==0x4/4 && ACCESSING_BITS_24_31) {
		if ((m_irq_num!=-1) && m_ctrl_regs[0x20/4]&0x40000000) {
			m_cpu->set_input_line(m_irq_num, CLEAR_LINE);
			if (LOG_IDE_CTRL)
				logerror("%s:ide2_interrupt Clearing interrupt\n", machine().describe_context());
		}
	}
	if (LOG_IDE)
		logerror("%s:ide2_r read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER( iteagle_ide_device::ide2_w )
{
	m_ide2->write_cs0(space, offset, data, mem_mask);
	if (LOG_IDE)
		logerror("%s:ide2_w write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
READ32_MEMBER( iteagle_ide_device::ide2_ctrl_r )
{
	UINT32 result = m_ide2->read_cs1(space, offset+1, mem_mask);
	if (LOG_IDE_CTRL)
		logerror("%s:ide2_ctrl_r read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER( iteagle_ide_device::ide2_ctrl_w )
{
	m_ide2->write_cs1(space, offset+1, data, mem_mask);
	if (LOG_IDE_CTRL)
		logerror("%s:ide2_ctrl_w write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}
WRITE_LINE_MEMBER(iteagle_ide_device::ide2_interrupt)
{
	if ((m_irq_num!=-1) &&m_ctrl_regs[0x20/4]&0x40000000) {
		m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
		if (LOG_IDE_CTRL)
			logerror("%s:ide2_interrupt Setting interrupt\n", machine().describe_context());
	}
}
