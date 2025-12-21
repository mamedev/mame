// license:BSD-3-Clause
// copyright-holders:Carl
// 20MB HDD image CHS 512,5,17

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i8089/i8089.h"
#include "machine/ram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "acs8600_ics.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "bus/rs232/rs232.h"


namespace {

class altos8600_state : public driver_device
{
public:
	altos8600_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dmac(*this, "dmac"),
		m_pic(*this, "pic8259%u", 1U),
		m_uart8274(*this, "uart8274"),
		m_fdc(*this, "fd1797"),
		m_ram(*this, RAM_TAG),
		m_ics(*this, "ics"),
		m_hdd(*this, "hdd"),
		m_bios(*this, "bios")
	{}

	void altos8600(machine_config &config);

private:
	uint16_t cpuram_r(offs_t offset, u16 mem_mask = ~0);
	void cpuram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t stkram_r(offs_t offset, u16 mem_mask = ~0);
	void stkram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t coderam_r(offs_t offset, u16 mem_mask = ~0);
	void coderam_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t xtraram_r(offs_t offset, u16 mem_mask = ~0);
	void xtraram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t cpuio_r(offs_t offset, u16 mem_mask = ~0);
	void cpuio_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t nmi_r(offs_t offset, u16 mem_mask = ~0);
	void nmi_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	uint16_t dmacram_r(offs_t offset, u16 mem_mask = ~0);
	void dmacram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 mmuaddr_r(offs_t offset);
	void mmuaddr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 mmuflags_r(offs_t offset);
	void mmuflags_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 get_slave_ack(offs_t offset);
	u16 fault_r();
	u16 errlo_r();
	u16 errhi_r();
	void clear_w(u16 data);
	void cattn_w(offs_t offset, u8 data);
	u8 hd_r(offs_t offset);
	void hd_w(offs_t offset, u8 data);
	u8 romport_r(offs_t offset);
	void romport_w(offs_t offset, u8 data);
	void clrsys_w(u8 data);
	void mode_w(u16 data);
	void cpuif_w(int state);
	void fddrq_w(int state);
	void sintr1_w(int state);
	void ics_attn_w(offs_t offset, u8 data);
	IRQ_CALLBACK_MEMBER(inta);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void code_mem(address_map &map) ATTR_COLD;
	void cpu_io(address_map &map) ATTR_COLD;
	void cpu_mem(address_map &map) ATTR_COLD;
	void dmac_io(address_map &map) ATTR_COLD;
	void dmac_mem(address_map &map) ATTR_COLD;
	void extra_mem(address_map &map) ATTR_COLD;
	void stack_mem(address_map &map) ATTR_COLD;

	u16 xlate_r(offs_t offset, u16 mem_mask, int permbit);
	void xlate_w(offs_t offset, u16 data, u16 mem_mask, int permbit);
	void seterr(offs_t offset, u16 mem_mask, u16 err_mask);
	bool find_sector();
	bool write_sector(u8 data);
	u8 read_sector();
	void format_sector();
	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8089_device> m_dmac;
	required_device_array<pic8259_device, 3> m_pic;
	required_device<i8274_device> m_uart8274;
	required_device<fd1797_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<acs8600_ics_device> m_ics;
	required_device<harddisk_image_device> m_hdd;
	required_memory_region m_bios;
	u8 m_mmuaddr[256], m_romport[4], m_dmamplex;
	u16 m_mmuflags[256], m_mmuerr, m_mode, m_mmueaddr[2];
	bool m_cpuif, m_user, m_nmiinh, m_nmistat;
	u32 m_lba;
	u16 m_head, m_sect, m_cyl, m_curcyl;
	int m_secoff;
	u8 m_cmd, m_stat;
	bool m_cylhi, m_sechi;
	const struct hard_disk_file::info* m_geom;
	u8 m_sector[512];
};

void altos8600_state::machine_start()
{
	m_mode = 0;
	save_item(NAME(m_mmuaddr));
	save_item(NAME(m_romport));
	save_item(NAME(m_dmamplex));
	save_item(NAME(m_mmuflags));
	save_item(NAME(m_mmuerr));
	save_item(NAME(m_mode));
	save_item(NAME(m_mmueaddr));
	save_item(NAME(m_cpuif));
	save_item(NAME(m_user));
	save_item(NAME(m_nmiinh));
	save_item(NAME(m_nmistat));
	save_item(NAME(m_lba));
	save_item(NAME(m_head));
	save_item(NAME(m_sect));
	save_item(NAME(m_cyl));
	save_item(NAME(m_curcyl));
	save_item(NAME(m_secoff));
	save_item(NAME(m_cmd));
	save_item(NAME(m_stat));
	save_item(NAME(m_cylhi));
	save_item(NAME(m_sechi));
	save_item(NAME(m_sector));

	if(m_hdd->exists())
		m_geom = &m_hdd->get_info();
	else
		m_geom = nullptr;
}

void altos8600_state::machine_reset()
{
	m_mode = (m_mode & 0x10) | 2;
	m_romport[0] = 0x80;
	m_cpuif = false;
	m_user = false;
	m_nmiinh = true;
	m_nmistat = false;
	m_cylhi = m_sechi = false;
	m_stat = 0;
	if(m_hdd->exists())
		m_geom = &m_hdd->get_info();
	else
		m_geom = nullptr;
}

bool altos8600_state::find_sector()
{
	u8 head = m_head >> 4;

	if(!m_geom)
		return false;

	if(m_cyl != m_curcyl)
		return false;

	if(m_curcyl > m_geom->cylinders)
		return false;

	if(head > m_geom->heads)
		return false;

	if(m_sect > m_geom->sectors)
		return false;

	m_lba = (m_cyl * m_geom->heads + head) * m_geom->sectors + m_sect;
	return true;
}

u8 altos8600_state::read_sector()
{
	int secoff = m_secoff;
	m_secoff++;
	if(m_cmd == 1)
	{
		switch(secoff)
		{
			case 0:
				return m_curcyl;
			case 1:
				return (m_head & 0xf0) | (m_curcyl >> 8);
			case 2:
				return m_sect;
		}
		secoff -= 3;
	}
	if(!secoff)
		m_hdd->read(m_lba, m_sector);
	if(secoff >= 511)
	{
		m_dmac->drq1_w(CLEAR_LINE);
		m_pic[1]->ir0_w(ASSERT_LINE);
		m_stat &= ~1;
		m_stat |= 2;
	}
	if(secoff >= 512)
		return 0;
	return m_sector[secoff];
}

bool altos8600_state::write_sector(u8 data)
{
	if(m_secoff >= 512)
		return true;
	m_sector[m_secoff++] = data;
	if(m_secoff == 512)
	{
		m_stat &= ~1;
		m_stat |= 2;
		m_hdd->write(m_lba, m_sector);
		m_dmac->drq1_w(CLEAR_LINE);
		m_pic[1]->ir0_w(ASSERT_LINE);
		return true;
	}
	return false;
}

u8 altos8600_state::hd_r(offs_t offset)
{
	switch(offset)
	{
		case 1:
			if(BIT(m_stat, 0) && (m_cmd & 1))
				return read_sector();
			break;
		case 3:
			m_pic[1]->ir0_w(CLEAR_LINE);
			return m_stat;
	}
	return 0;
}

void altos8600_state::hd_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			m_head = data;
			if((m_head & 3) == 1)
				m_stat |= 0x80;
			else
				m_stat &= ~0x80;
			break;
		case 1:
			if(BIT(m_stat, 0))
			{
				switch(m_cmd)
				{
					case 2:
						write_sector(data);
						break;
					case 4:
						m_secoff++;
						if(m_secoff == 4)
						{
							m_dmac->drq1_w(CLEAR_LINE);
							m_stat &= ~1;
							m_stat |= 2;
						}
						break;
				}
				break;
			}
			if(m_sechi)
			{
				m_sechi = false;
				m_sect |= (data & 7) << 8;
			}
			else
			{
				m_sechi = true;
				m_sect = data;
			}
			break;
		case 2:
			if(m_cylhi)
			{
				m_cylhi = false;
				m_cyl |= (data & 7) << 8;
			}
			else
			{
				m_cylhi = true;
				m_cyl = data;
			}
			break;
		case 3:
			m_cmd = data;
			m_cylhi = false;
			m_sechi = false;
			m_dmac->drq1_w(CLEAR_LINE);
			if(!BIT(m_stat, 7))
				break;
			m_stat &= 0x80;
			switch(m_cmd)
			{
				case 0x10:
					m_curcyl = m_cyl;
					m_stat |= 2;
					break;
				case 0x20:
					m_curcyl = 0;
					m_stat |= 2;
					break;
				case 0x1:
				case 0x2:
				case 0x9:
					if(!find_sector())
					{
						m_stat |= 0xa;
						break;
					}
					[[fallthrough]];
				case 0x4:
					m_secoff = 0;
					m_stat |= 1;
					m_dmac->drq1_w(ASSERT_LINE);
					break;
			}
			m_pic[1]->ir0_w(ASSERT_LINE);
			break;
	}
}

void altos8600_state::cpuif_w(int state)
{
	if(m_user)
	{
		seterr(0, 0, 1);
		return;
	}
	m_cpuif = state ? true : false;
	if(state && BIT(m_mode, 0))
		m_user = true;
}

void altos8600_state::fddrq_w(int state)
{
	if(!m_dmamplex)
		m_dmac->drq2_w(state);
}

void altos8600_state::sintr1_w(int state)
{
	if(state)
	{
		m_dmac->drq1_w(CLEAR_LINE);
		m_pic[1]->ir0_w(ASSERT_LINE);
		m_pic[1]->ir3_w(ASSERT_LINE);
		m_stat &= ~1;
		m_stat |= 2;
	}
	else
		m_pic[1]->ir3_w(CLEAR_LINE);
}

u16 altos8600_state::fault_r()
{
	return m_mmuerr;
}

u16 altos8600_state::errlo_r()
{
	return m_mmueaddr[0];
}

u16 altos8600_state::errhi_r()
{
	return m_mmueaddr[1];
}

void altos8600_state::clear_w(u16 data)
{
	m_mmuerr = 0xffff;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmistat = false;
}

u16 altos8600_state::mmuaddr_r(offs_t offset)
{
	return m_mmuaddr[offset & 0xff];
}

void altos8600_state::mmuaddr_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(mem_mask & 0xff)
		m_mmuaddr[offset & 0xff] = data & 0xff;
}

u16 altos8600_state::mmuflags_r(offs_t offset)
{
	return m_mmuflags[offset & 0xff] | (m_user ? 1 : 0) | (m_mode & 2);
}

void altos8600_state::mmuflags_w(offs_t offset, u16 data, u16 mem_mask)
{
	data &= ~0x17;
	if(mem_mask == 0xff)
		data |= 0xff00;
	else if(mem_mask == 0xff00)
		return;
	m_mmuflags[offset & 0xff] = data;
}

void altos8600_state::cattn_w(offs_t offset, u8 data)
{
	m_dmac->sel_w(offset & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_dmac->ca_w(ASSERT_LINE);
	m_dmac->ca_w(CLEAR_LINE);
}

uint8_t altos8600_state::romport_r(offs_t offset)
{
	if(offset & 1)
		return m_romport[offset >> 1];
	return 0;
}

void altos8600_state::romport_w(offs_t offset, u8 data)
{
	const char *fdc = nullptr;
	switch(offset)
	{
		case 1:
			//m_romport[0] = data;
			break;
		case 3:
			m_romport[1] = data;
			if(!BIT(data, 0))
				fdc = "0";
			else if(!BIT(data, 1))
				fdc = "1";
			else if(!BIT(data, 2))
				fdc = "2";
			else if(!BIT(data, 3))
				fdc = "3";

			if(fdc != nullptr)
				m_fdc->set_floppy(m_fdc->subdevice<floppy_connector>(fdc)->get_device());
			else
				m_fdc->set_floppy(nullptr);

			if(m_nmistat && m_nmiinh && !BIT(data, 4))
			{
				m_mode &= ~1;
				m_user = false;
				m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			}
			else if(BIT(data, 4) && m_nmistat)
				m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			m_nmiinh = BIT(data, 4) ? true : false;
			break;
		case 5:
			m_romport[2] = data;
			m_fdc->mr_w(!BIT(data, 3));
			m_fdc->dden_w(!BIT(data, 2));
			m_dmamplex = (data >> 4) & 3;
			break;
		case 7:
			m_romport[3] = data;
			break;
	}
}

void altos8600_state::clrsys_w(u8 data)
{
	m_pic[0]->ir0_w(CLEAR_LINE);
}

void altos8600_state::ics_attn_w(offs_t offset, u8 data)
{
	if(!offset)
	{
		m_ics->attn_w(ASSERT_LINE);
		m_ics->attn_w(CLEAR_LINE);
	}
}

void altos8600_state::mode_w(u16 data)
{
	m_mode = data;
	if(m_cpuif && BIT(m_mode, 0))
		m_user = true;
}

u8 altos8600_state::get_slave_ack(offs_t offset)
{
	if(offset == 2)
		return m_pic[1]->acknowledge();
	else if(offset == 3)
		return m_pic[2]->acknowledge();
	return 0x00;
}

void altos8600_state::seterr(offs_t offset, u16 mem_mask, u16 err_mask)
{
	if(machine().side_effects_disabled())
		return;
	logerror("Fault at %05x type %04x\n", offset << 1, err_mask);
	if(!m_nmiinh)
	{
		m_mode &= ~1;
		m_user = false;
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	m_nmistat = true;
	m_mmuerr &= ~err_mask;
	m_mmueaddr[0] = (offset << 1) | (!(mem_mask & 0xff) ? 1 : 0);
	m_mmueaddr[1] = (m_user ? 0x100 : 0) | (m_mode & 2 ? 0x200 : 0) | ((offset >> 3) & 0xf000);
}

u16 altos8600_state::xlate_r(offs_t offset, u16 mem_mask, int permbit)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if((offset >= 0x7f000) && BIT(m_mode, 1))
		return m_bios->as_u16(offset & 0xfff);
	if(m_user && !BIT(flags, 11))
		seterr(offset, mem_mask, 0x800);
	else if(m_user && !BIT(flags, permbit))
		seterr(offset, mem_mask, 1 << permbit);
	return ((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)];

}

void altos8600_state::xlate_w(offs_t offset, u16 data, u16 mem_mask, int permbit)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if(m_user && !BIT(flags, 7))
	{
		seterr(offset, mem_mask, 0x80);
		return;
	}
	else if(!m_user && !BIT(flags, 8))
	{
		seterr(offset, mem_mask, 0x10);
		return;
	}
	else if(m_user && !BIT(flags, permbit))
		seterr(offset, mem_mask, 1 << permbit);
	else if(m_user && BIT(flags, 3) && ((offset & 0x7ff) < 64))
		seterr(offset, mem_mask, 8);
	COMBINE_DATA(&((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)]);
	m_mmuflags[offset >> 11] |= 4;
}

u16 altos8600_state::cpuram_r(offs_t offset, u16 mem_mask)
{
	return xlate_r(offset, mem_mask, 14);
}

void altos8600_state::cpuram_w(offs_t offset, u16 data, u16 mem_mask)
{
	xlate_w(offset, data, mem_mask, 14);
}

u16 altos8600_state::stkram_r(offs_t offset, u16 mem_mask)
{
	return xlate_r(offset, mem_mask, 13);
}

void altos8600_state::stkram_w(offs_t offset, u16 data, u16 mem_mask)
{
	xlate_w(offset, data, mem_mask, 13);
}

u16 altos8600_state::coderam_r(offs_t offset, u16 mem_mask)
{
	return xlate_r(offset, mem_mask, 15);
}

void altos8600_state::coderam_w(offs_t offset, u16 data, u16 mem_mask)
{
	xlate_w(offset, data, mem_mask, 15);
}

u16 altos8600_state::xtraram_r(offs_t offset, u16 mem_mask)
{
	return xlate_r(offset, mem_mask, 12);
}

void altos8600_state::xtraram_w(offs_t offset, u16 data, u16 mem_mask)
{
	xlate_w(offset, data, mem_mask, 12);
}

u16 altos8600_state::cpuio_r(offs_t offset, u16 mem_mask)
{
	if(m_user && !machine().side_effects_disabled())
	{
		m_pic[0]->ir0_w(ASSERT_LINE);
		return 0;
	}
	return m_dmac->space(AS_IO).read_word_unaligned(offset << 1, mem_mask);
}

void altos8600_state::cpuio_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(m_user && !machine().side_effects_disabled())
	{
		m_pic[0]->ir0_w(ASSERT_LINE);
		return;
	}
	m_dmac->space(AS_IO).write_word_unaligned(offset << 1, data, mem_mask);
}

u16 altos8600_state::dmacram_r(offs_t offset, u16 mem_mask)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if((offset >= 0x7f000) && BIT(m_mode, 1))
		return m_bios->as_u16(offset & 0xfff);
	if(!BIT(flags, 10))
		seterr(offset, mem_mask, 0x400);
	return ((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)];

}

void altos8600_state::dmacram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if(!BIT(flags, 6))
	{
		seterr(offset, mem_mask, 0x40);
		return;
	}
	COMBINE_DATA(&((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)]);
	m_mmuflags[offset >> 11] |= 4;
}

u16 altos8600_state::nmi_r(offs_t offset, u16 mem_mask)
{
	seterr(offset, mem_mask, 0x100);
	return 0;
}

void altos8600_state::nmi_w(offs_t offset, u16 data, u16 mem_mask)
{
	seterr(offset, mem_mask, 0x100);
}

IRQ_CALLBACK_MEMBER(altos8600_state::inta)
{
	m_user = false;
	m_mode &= ~1;
	return m_pic[0]->acknowledge();
}

void altos8600_state::cpu_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos8600_state::cpuram_r), FUNC(altos8600_state::cpuram_w));
}

void altos8600_state::stack_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos8600_state::stkram_r), FUNC(altos8600_state::stkram_w));
}

void altos8600_state::code_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos8600_state::coderam_r), FUNC(altos8600_state::coderam_w));
}

void altos8600_state::extra_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos8600_state::xtraram_r), FUNC(altos8600_state::xtraram_w));
}

void altos8600_state::cpu_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(altos8600_state::cpuio_r), FUNC(altos8600_state::cpuio_w));
}

void altos8600_state::dmac_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos8600_state::dmacram_r), FUNC(altos8600_state::dmacram_w));
}

void altos8600_state::dmac_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(altos8600_state::nmi_r), FUNC(altos8600_state::nmi_w));
	map(0x0000, 0x0007).r(FUNC(altos8600_state::fault_r));
	map(0x0008, 0x000f).w(FUNC(altos8600_state::clear_w));
	map(0x0010, 0x0017).r(FUNC(altos8600_state::errlo_r));
	map(0x0018, 0x001f).r(FUNC(altos8600_state::errhi_r));
	map(0x0020, 0x0027).rw(FUNC(altos8600_state::hd_r), FUNC(altos8600_state::hd_w)).umask16(0x00ff);
	map(0x0030, 0x0037).w(FUNC(altos8600_state::mode_w));
	map(0x0038, 0x003f).w(FUNC(altos8600_state::cattn_w));
	map(0x0040, 0x0047).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0040, 0x0047).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write)).umask16(0xff00);
	map(0x0048, 0x004f).rw(m_uart8274, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0x00ff);
	map(0x0048, 0x004f).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
	map(0x0050, 0x0057).rw(FUNC(altos8600_state::romport_r), FUNC(altos8600_state::romport_w));
	map(0x0058, 0x005f).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0058, 0x005f).w(FUNC(altos8600_state::clrsys_w)).umask16(0xff00);
	map(0x0060, 0x0067).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0068, 0x006f).rw(m_pic[2], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0070, 0x0077).noprw();
	map(0x0078, 0x0079).w(FUNC(altos8600_state::ics_attn_w));
	map(0x0200, 0x03ff).rw(FUNC(altos8600_state::mmuflags_r), FUNC(altos8600_state::mmuflags_w));
	map(0x0400, 0x05ff).rw(FUNC(altos8600_state::mmuaddr_r), FUNC(altos8600_state::mmuaddr_w));
}

static void altos8600_floppies(device_slot_interface &device)
{
	device.option_add("8dd", FLOPPY_8_DSDD);
}

void altos8600_state::altos8600(machine_config &config)
{
	I8086(config, m_maincpu, 5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &altos8600_state::cpu_mem);
	m_maincpu->set_addrmap(AS_IO, &altos8600_state::cpu_io);
	m_maincpu->set_addrmap(AS_OPCODES, &altos8600_state::code_mem);
	m_maincpu->set_addrmap(i8086_cpu_device::AS_STACK, &altos8600_state::stack_mem);
	m_maincpu->set_addrmap(i8086_cpu_device::AS_CODE, &altos8600_state::code_mem);
	m_maincpu->set_addrmap(i8086_cpu_device::AS_EXTRA, &altos8600_state::extra_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(altos8600_state::inta));
	m_maincpu->if_handler().set(FUNC(altos8600_state::cpuif_w));

	I8089(config, m_dmac, 5_MHz_XTAL);
	m_dmac->set_addrmap(AS_PROGRAM, &altos8600_state::dmac_mem);
	m_dmac->set_addrmap(AS_IO, &altos8600_state::dmac_io);
	m_dmac->set_data_width(16);
	m_dmac->sintr1().set(FUNC(altos8600_state::sintr1_w));
	m_dmac->sintr2().set(m_pic[1], FUNC(pic8259_device::ir4_w));

	PIC8259(config, m_pic[0], 0);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(altos8600_state::get_slave_ack));

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	PIC8259(config, m_pic[2], 0);
	m_pic[2]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir3_w));
	m_pic[2]->in_sp_callback().set_constant(0);

	RAM(config, RAM_TAG).set_default_size("1M");//.set_extra_options("512K");

	I8274(config, m_uart8274, 16_MHz_XTAL/4);
	m_uart8274->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_uart8274->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_uart8274->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir7_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_uart8274, FUNC(i8274_device::rxa_w));
	rs232a.dcd_handler().set(m_uart8274, FUNC(i8274_device::dcda_w));
	rs232a.cts_handler().set(m_uart8274, FUNC(i8274_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_uart8274, FUNC(i8274_device::rxb_w));
	rs232b.dcd_handler().set(m_uart8274, FUNC(i8274_device::dcdb_w));
	rs232b.cts_handler().set(m_uart8274, FUNC(i8274_device::ctsb_w));

	I8255A(config, "ppi", 0);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(1228800);
	pit.out_handler<0>().set(m_uart8274, FUNC(i8274_device::rxca_w));
	pit.out_handler<0>().append(m_uart8274, FUNC(i8274_device::txca_w));
	pit.set_clk<1>(1228800);
	pit.out_handler<1>().set(m_uart8274, FUNC(i8274_device::rxtxcb_w));
	pit.set_clk<2>(1228800);
	pit.out_handler<1>().set(m_pic[0], FUNC(pic8259_device::ir1_w));

	FD1797(config, m_fdc, 2000000);
	m_fdc->intrq_wr_callback().set(m_pic[1], FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set(FUNC(altos8600_state::fddrq_w));
	FLOPPY_CONNECTOR(config, "fd1797:0", altos8600_floppies, "8dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fd1797:1", altos8600_floppies, "8dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fd1797:2", altos8600_floppies, "8dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fd1797:3", altos8600_floppies, "8dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	ACS8600_ICS(config, m_ics, 0);
	m_ics->set_host_space(m_dmac, AS_PROGRAM); // TODO: fixme
	m_ics->irq1_callback().set(m_pic[0], FUNC(pic8259_device::ir5_w));
	m_ics->irq2_callback().set(m_pic[0], FUNC(pic8259_device::ir6_w));

	HARDDISK(config, "hdd", 0);

	SOFTWARE_LIST(config, "flop_list").set_original("altos8600");
}

ROM_START(altos8600)
	ROM_REGION(0x2000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios", "bios")
	ROMX_LOAD("11753_1.5_lo.bin", 0x0000, 0x1000, CRC(dfa7bf0e) SHA1(6628fd7c579423b51d2642aeaa7fc0405a989252), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("11753_1.5_hi.bin", 0x0001, 0x1000, CRC(9b5e812c) SHA1(c2ef24859edd48d2096db47e16855c9bc01dae75), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

} // anonymous namespace


COMP( 1981, altos8600, 0, 0, altos8600, 0, altos8600_state, empty_init, "Altos Computer Systems", "ACS8600", MACHINE_NO_SOUND_HW)
