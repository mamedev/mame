// license:BSD-3-Clause
// copyright-holders:Carl
// Currently only emulates Shugart SA1000 interface disks (aka ST506)
// despite the controller supporting many different disk types (hard sectored, onboard controllers, ANSI X3T9)
// Xenix supports the Quantum Q540 CHS 512,8,9 with 1024B sectors

// TODO: multibus

#include "isbc_215g.h"

const device_type ISBC_215G = &device_creator<isbc_215g_device>;

isbc_215g_device::isbc_215g_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISBC_215G, "ISBC 215G Winchester Disk Controller", tag, owner, clock, "isbc_215g", __FILE__),
	m_dmac(*this, "u84"),
	m_hdd0(*this, "drive0"),
	m_hdd1(*this, "drive1"),
	m_sbx1(*this, "sbx1"),
	m_sbx2(*this, "sbx2"),
	m_out_irq_func(*this)
{
}

void isbc_215g_device::find_sector()
{
	//sector id
	// 0/6-7: sector type - 0-data, 1-alternate, 2-bad, 3-invalid
	// 0/4-5: size - 128<<n
	// 0/0-3: cyl high
	// 1:     cyl low
	// 2:     head
	// 3:     sector
	UINT16 cyl = ((m_idcompare[0] & 0xf) << 8) | m_idcompare[1];
	UINT16 bps = 128 << ((m_idcompare[0] >> 4) & 3);

	if(!m_geom[m_drive])
		return;

	if(m_cyl[m_drive] != cyl)
		return;

	if((m_idcompare[3] != m_head) || (m_head > m_geom[m_drive]->heads))
		return;

	if(m_idcompare[2] > m_geom[m_drive]->sectors)
		return;

	if(bps != m_geom[m_drive]->sectorbytes)
		return;

	if(m_idcompare[0] >> 6)
		return;

	m_idfound = true;
	m_lba[m_drive] = (cyl * m_geom[m_drive]->heads + m_head) * m_geom[m_drive]->sectors + m_idcompare[2];
	m_secoffset = 0;
	return;
}

UINT16 isbc_215g_device::read_sector()
{
	UINT16 wps = 64 << ((m_idcompare[0] >> 4) & 3);
	harddisk_image_device *drive = (m_drive ? m_hdd1 : m_hdd0);
	if(!m_secoffset)
		hard_disk_read(drive->get_hard_disk_file(), m_lba[m_drive], m_sector);
	if(m_secoffset >= wps)
		return 0;
	return m_sector[m_secoffset++];
}

bool isbc_215g_device::write_sector(UINT16 data)
{
	UINT16 wps = 64 << ((m_idcompare[0] >> 4) & 3);
	harddisk_image_device *drive = (m_drive ? m_hdd1 : m_hdd0);
	if(m_secoffset >= wps)
		return true;
	m_sector[m_secoffset++] = data;
	if(m_secoffset == wps)
	{
		hard_disk_write(drive->get_hard_disk_file(), m_lba[m_drive], m_sector);
		return true;
	}
	return false;
}

READ16_MEMBER(isbc_215g_device::io_r)
{
	UINT16 data = 0;
	switch(offset)
	{
		case 0x00:
			//read status
			// 0: gnd
			// 1: drive ready
			// 2: seek done
			// 3: sector sync found
			// 4: fault
			// 5: bus ack
			// 6: sector id not found or bad ecc
			// 7: timeout
			// 8: sbx 1 present
			// 9: sbx 1 int 0
			// a: sbx 1 int 1
			// b: sbx 1 opt 0
			// c: sbx 1 opt 1
			// d: illegal req/vendor 2
			// e: drive req
			// f: index latch
			data |= (!m_idfound) << 6;
			data |= (m_sbx1->get_card_device() ? 0 : 1) << 8;
			data |= m_isbx_irq[0] << 9;
			data |= m_isbx_irq[1] << 10;
			data |= ((m_index--) <= 0) ? 0x8000 : 0; // fake an index pulse
			break;
		case 0x04:
			//read status 2
			// 0: sbx 2 present
			// 1: sbx 2 int 0
			// 2: sbx 2 int 1
			// 3: sbx 2 opt 0
			// 4: sbx 2 opt 1
			// 5: vendor bit 0
			// 6: track 0/busy
			// 7: wp
			data |= m_sbx2->get_card_device() ? 0 : 1;
			data |= m_isbx_irq[2] << 1;
			data |= m_isbx_irq[3] << 2;
			data |= (!m_cyl[m_drive]) ? 0 : 0x40;
			break;
		case 0x08:
			//cmd data bus
			break;
		case 0x0c:
			// reset channel 2
			if(space.debugger_access()) // reading this is bad
				break;
			m_dmac->sel_w(1);
			m_dmac->ca_w(1);
			m_dmac->ca_w(0);
			m_dmac->sel_w(0);
			break;
		case 0x10:
			//pit ch 0
			break;
		case 0x11:
			//pit ch 1
			break;
		case 0x12:
			//pit ch 2
			break;
		case 0x14:
			//read buffer
			if(m_rdgate && !m_amsrch && m_geom[m_drive])
				data = read_sector();
			break;
		case 0x18:
			//vendor bit 1,3-4
			data |= 3<<4 | 2;
			break;
		default:
			logerror("isbc_215g: invalid port read 0x80%02x\n", offset*2);
			break;
	}
	return data;
}

WRITE16_MEMBER(isbc_215g_device::io_w)
{
	switch(offset)
	{
		case 0x00:
			//control, 0x8002 doesn't set gates or search
			// 0: wr gate
			// 1: rd gate
			// 2: address mark search
			// 3: cmd bus enable
			// 4: drive sel/head sel 2
			// 5: safe/head sel 1
			// 6: b ack
			// 7: drive reg addr 0/step dir
			// 8: drive reg addr 1
			// 9: cmd/rd
			// a: para/head sel 0
			m_wrgate = data & 1;
			m_rdgate = (data >> 1) & 1;
			m_amsrch = (data >> 2) & 1;
			if(m_wrgate && (m_rdgate || m_amsrch))
				logerror("isbc_215g: both write gate and read gate and/or address search enabled\n");
			else if(m_rdgate && m_amsrch)
				find_sector();
			else if(m_amsrch)
				logerror("isbc_215g: address search without read gate\n");
		case 0x01:
			m_stepdir = (data & 0x80) ? 1 : 0;
			break;
		case 0x04:
			//clear index and id latch
			m_index = 10;
			m_idfound = false;
			break;
		case 0x08:
			//cmd data bus/head sel
			m_head = data & 7;
			m_out_irq_func((data & 0x100) ? 1 : 0);
			break;
		case 0x0c:
			//unit select
			// 0: step/wr
			// 1: sbx 1 opt 0/1 (sbx2?)
			// 2: sbx 2 opt 0/1
			// 3: unit select 0
			// 4: unit select 1
			// 5: extr 2
			// 6: format
			// 7: format wr gate
			if(!m_step && (data & 1) && m_geom[m_drive])
			{
				if(m_cyl[m_drive] && !m_stepdir)
					m_cyl[m_drive]--;
				else if((m_cyl[m_drive] < m_geom[m_drive]->cylinders) && m_stepdir)
					m_cyl[m_drive]++;
			}
			m_step = data & 1;
			m_drive = (data >> 3) & 1; // shugart interface, two drives only
			if(((data >> 1) & 1) != m_fdctc)
			{
				m_fdctc = !m_fdctc;
				m_sbx2->opt0_w(m_fdctc);
			}
			if((data & 0xc0) == 0xc0) // low-level format
			{
				if(!m_format)
					m_format_bytes = 5; // sync + sector id
				m_format = true;
			}
			else
				m_format = false;
			break;
		case 0x10:
			//pit ch 0
			break;
		case 0x11:
			//pit ch 1
			break;
		case 0x12:
			//pit ch 2
			break;
		case 0x13:
			//pit control
			break;
		case 0x14:
			//write buffer
			if(m_wrgate)
			{
				if(m_format)
				{
					m_format_bytes -= 2;
					if(m_format_bytes <= 0)
					{
						m_dmac->ext1_w(1);
						m_dmac->ext1_w(0);
						m_format_bytes = 5;
					}
				}
				else
				{
					if(m_geom[m_drive])
					{
						if(write_sector(data))
						{
							m_dmac->ext1_w(1);
							m_dmac->ext1_w(0);
						}
					}
				}
			}
			break;
		case 0x18:
			//sector id/format
			m_idcompare[1] = data & 0xff;
			m_idcompare[0] = data >> 8;
			break;
		case 0x1c:
			//sector id low
			m_idcompare[3] = data & 0xff;
			m_idcompare[2] = data >> 8;
			break;
		default:
			logerror("isbc_215g: invalid port write 0x80%02x\n", offset*2);
			break;
	}
}

READ16_MEMBER(isbc_215g_device::mem_r)
{
	// XXX: hack to permit debugger to disassemble rom
	if(space.debugger_access() && (offset < 0x1fff))
		return m_dmac->space(AS_IO).read_word_unaligned(offset*2);

	switch(offset)
	{
		case 0x7fffb:
			return 1;
		case 0x7fffc:
			return 0;
		case 0x7fffd:
			return m_wakeup;
		default:
			return m_maincpu_mem->read_word_unaligned(offset*2, mem_mask);
	}
}

WRITE16_MEMBER(isbc_215g_device::mem_w)
{
	m_maincpu_mem->write_word_unaligned(offset*2, data, mem_mask);
}

static ADDRESS_MAP_START(isbc_215g_mem, AS_PROGRAM, 16, isbc_215g_device)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(mem_r, mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc_215g_io, AS_IO, 16, isbc_215g_device)
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("i8089", 0)
	AM_RANGE(0x4000, 0x47ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0x8000, 0x8039) AM_MIRROR(0x3fc0) AM_READWRITE(io_r, io_w)
	AM_RANGE(0xc070, 0xc08f) AM_DEVREADWRITE8("sbx1", isbx_slot_device, mcs0_r, mcs0_w, 0x00ff)
	AM_RANGE(0xc0b0, 0xc0bf) AM_DEVREADWRITE8("sbx1", isbx_slot_device, mcs1_r, mcs1_w, 0x00ff)
	AM_RANGE(0xc0d0, 0xc0df) AM_DEVREADWRITE8("sbx2", isbx_slot_device, mcs0_r, mcs0_w, 0x00ff)
	AM_RANGE(0xc0e0, 0xc0ef) AM_DEVREADWRITE8("sbx2", isbx_slot_device, mcs1_r, mcs1_w, 0x00ff)
ADDRESS_MAP_END

WRITE_LINE_MEMBER(isbc_215g_device::isbx_irq_00_w)
{
	m_isbx_irq[0] = state ? true : false;
}

WRITE_LINE_MEMBER(isbc_215g_device::isbx_irq_01_w)
{
	m_isbx_irq[1] = state ? true : false;
}

WRITE_LINE_MEMBER(isbc_215g_device::isbx_irq_10_w)
{
	m_isbx_irq[2] = state ? true : false;
}

WRITE_LINE_MEMBER(isbc_215g_device::isbx_irq_11_w)
{
	m_isbx_irq[3] = state ? true : false;
}

static MACHINE_CONFIG_FRAGMENT( isbc_215g )
	MCFG_CPU_ADD("u84", I8089, XTAL_15MHz / 3)
	MCFG_CPU_PROGRAM_MAP(isbc_215g_mem)
	MCFG_CPU_IO_MAP(isbc_215g_io)
	MCFG_I8089_DATABUS_WIDTH(16)

	MCFG_HARDDISK_ADD("drive0")
	MCFG_HARDDISK_ADD("drive1")

	MCFG_ISBX_SLOT_ADD("sbx1", 0, isbx_cards, nullptr)
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(WRITELINE(isbc_215g_device, isbx_irq_00_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(WRITELINE(isbc_215g_device, isbx_irq_01_w))
	MCFG_ISBX_SLOT_ADD("sbx2", 0, isbx_cards, "fdc_218a")
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(WRITELINE(isbc_215g_device, isbx_irq_10_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(WRITELINE(isbc_215g_device, isbx_irq_11_w))
MACHINE_CONFIG_END

machine_config_constructor isbc_215g_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( isbc_215g );
}

ROM_START( isbc_215g )
	ROM_REGION( 0x4000, "i8089", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "174581.001.bin", 0x0000, 0x2000, CRC(ccdbc7ab) SHA1(5c2ebdde1b0252124177221ba9cacdb6d925a24d))
	ROM_LOAD16_BYTE( "174581.002.bin", 0x0001, 0x2000, CRC(6190fa67) SHA1(295dd4e75f699aaf93227cc4876cee8accae383a))
ROM_END

const rom_entry *isbc_215g_device::device_rom_region() const
{
	return ROM_NAME( isbc_215g );
}

void isbc_215g_device::device_reset()
{
	if(m_hdd0->get_hard_disk_file())
		m_geom[0] = hard_disk_get_info(m_hdd0->get_hard_disk_file());
	else
		m_geom[0] = nullptr;
	if(m_hdd1->get_hard_disk_file())
		m_geom[1] = hard_disk_get_info(m_hdd1->get_hard_disk_file());
	else
		m_geom[1] = nullptr;

	m_reset = false;
	m_fdctc = false;
	m_isbx_irq[0] = m_isbx_irq[1] = m_isbx_irq[2] = m_isbx_irq[3] = false;
}

void isbc_215g_device::device_start()
{
	m_maincpu_mem = &machine().device<cpu_device>(m_maincpu_tag)->space(AS_PROGRAM);
	m_cyl[0] = m_cyl[1] = 0;
	m_lba[0] = m_lba[1] = 0;
	m_idcompare[0] = m_idcompare[1] = m_idcompare[2] = m_idcompare[3] = 0;
	m_index = 10;
	m_idfound = false;
	m_drive = 0;
	m_head = 0;
	m_stepdir = false;
	m_step = false;
	m_format = false;
	m_wrgate = false;
	m_rdgate = false;
	m_format_bytes = 0;
	m_out_irq_func.resolve_safe();

}

WRITE8_MEMBER(isbc_215g_device::write)
{
	if(!offset)
	{
		data &= 3;
		if(!data && (m_reset == 2))
			m_dmac->reset();
		m_out_irq_func(0);
		m_dmac->ca_w(data == 1);
		m_dmac->ca_w(0);
		m_reset = data;
	}
}
