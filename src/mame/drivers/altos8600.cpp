// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i8089/i8089.h"
#include "machine/ram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "bus/rs232/rs232.h"

class altos8600_state : public driver_device
{
public:
	altos8600_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dmac(*this, "dmac"),
		m_pic1(*this, "pic8259_1"),
		m_pic2(*this, "pic8259_2"),
		m_pic3(*this, "pic8259_3"),
		m_uart8274(*this, "uart8274"),
		m_ram(*this, RAM_TAG),
		m_bios(*this, "bios")
	{}
	DECLARE_READ16_MEMBER(cpuram_r);
	DECLARE_WRITE16_MEMBER(cpuram_w);
	DECLARE_READ16_MEMBER(stkram_r);
	DECLARE_WRITE16_MEMBER(stkram_w);
	DECLARE_READ16_MEMBER(coderam_r);
	DECLARE_WRITE16_MEMBER(coderam_w);
	DECLARE_READ16_MEMBER(xtraram_r);
	DECLARE_WRITE16_MEMBER(xtraram_w);
	DECLARE_READ16_MEMBER(cpuio_r);
	DECLARE_WRITE16_MEMBER(cpuio_w);
	DECLARE_READ16_MEMBER(nmi_r);
	DECLARE_WRITE16_MEMBER(nmi_w);
	DECLARE_READ16_MEMBER(dmacram_r);
	DECLARE_WRITE16_MEMBER(dmacram_w);
	DECLARE_READ16_MEMBER(mmuaddr_r);
	DECLARE_WRITE16_MEMBER(mmuaddr_w);
	DECLARE_READ16_MEMBER(mmuflags_r);
	DECLARE_WRITE16_MEMBER(mmuflags_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_READ16_MEMBER(fault_r);
	DECLARE_READ16_MEMBER(errlo_r);
	DECLARE_READ16_MEMBER(errhi_r);
	DECLARE_WRITE16_MEMBER(clear_w);
	DECLARE_WRITE8_MEMBER(cattn_w);
	DECLARE_WRITE16_MEMBER(mode_w);
	DECLARE_WRITE_LINE_MEMBER(cpuif_w);
	DECLARE_WRITE_LINE_MEMBER(tmr0_w);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u16 xlate_r(address_space &space, offs_t offset, u16 mem_mask, int permbit);
	void xlate_w(address_space &space, offs_t offset, u16 data, u16 mem_mask, int permbit);
	void seterr(offs_t offset, u16 mem_mask, u16 err_mask);
	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8089_device> m_dmac;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<pic8259_device> m_pic3;
	required_device<i8274_new_device> m_uart8274;
	required_device<ram_device> m_ram;
	required_memory_region m_bios;
	u8 m_mmuaddr[256];
	u16 m_mmuflags[256], m_mmuerr, m_mode, m_mmueaddr[2];
	bool m_cpuif, m_user;
};

void altos8600_state::machine_start()
{
	m_mode = 0;
}

void altos8600_state::machine_reset()
{
	m_mode = (m_mode & 0x10) | 2;
	m_cpuif = false;
	m_user = false;
}


WRITE_LINE_MEMBER(altos8600_state::tmr0_w)
{
	m_uart8274->rxca_w(state);
	m_uart8274->txca_w(state);
}

WRITE_LINE_MEMBER(altos8600_state::cpuif_w)
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

READ16_MEMBER(altos8600_state::fault_r)
{
	return m_mmuerr;
}

READ16_MEMBER(altos8600_state::errlo_r)
{
	return m_mmueaddr[0];
}

READ16_MEMBER(altos8600_state::errhi_r)
{
	return m_mmueaddr[1];
}

WRITE16_MEMBER(altos8600_state::clear_w)
{
	m_mmuerr = 0xff;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

READ16_MEMBER(altos8600_state::mmuaddr_r)
{
	return m_mmuaddr[offset & 0xff];
}

WRITE16_MEMBER(altos8600_state::mmuaddr_w)
{
	if(mem_mask & 0xff)
		m_mmuaddr[offset & 0xff] = data & 0xff;
}

READ16_MEMBER(altos8600_state::mmuflags_r)
{
	return m_mmuflags[offset & 0xff] | (m_user ? 1 : 0) | (m_mode & 2);
}

WRITE16_MEMBER(altos8600_state::mmuflags_w)
{
	data &= ~0x17;
	COMBINE_DATA(&m_mmuflags[offset & 0xff]);
}

WRITE8_MEMBER(altos8600_state::cattn_w)
{
	m_dmac->sel_w(offset & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_dmac->ca_w(ASSERT_LINE);
	m_dmac->ca_w(CLEAR_LINE);
}

WRITE16_MEMBER(altos8600_state::mode_w)
{
	m_mode = data;
	if(m_cpuif && BIT(m_mode, 0))
		m_user = true;
}

READ8_MEMBER(altos8600_state::get_slave_ack)
{
	m_user = false;
	if(offset == 2)
		return m_pic2->acknowledge();
	else if(offset == 3)
		return m_pic3->acknowledge();
	return 0x00;
}

void altos8600_state::seterr(offs_t offset, u16 mem_mask, u16 err_mask)
{
	if(machine().side_effect_disabled())
		return;
	logerror("Fault at %06x type %04x\n", offset << 1, err_mask);
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_mmuerr &= ~err_mask;
	m_mmueaddr[0] = (offset << 1) | (!(mem_mask & 0xff) ? 1 : 0);
	m_mmueaddr[1] = (m_user ? 0x100 : 0) | (m_mode & 2 ? 0x200 : 0) | ((offset >> 3) & 0xf000);
}

u16 altos8600_state::xlate_r(address_space &space, offs_t offset, u16 mem_mask, int permbit)
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

void altos8600_state::xlate_w(address_space &space, offs_t offset, u16 data, u16 mem_mask, int permbit)
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
}

READ16_MEMBER(altos8600_state::cpuram_r)
{
	return xlate_r(space, offset, mem_mask, 14);
}

WRITE16_MEMBER(altos8600_state::cpuram_w)
{
	xlate_w(space, offset, data, mem_mask, 14);
}

READ16_MEMBER(altos8600_state::stkram_r)
{
	return xlate_r(space, offset, mem_mask, 13);
}

WRITE16_MEMBER(altos8600_state::stkram_w)
{
	xlate_w(space, offset, data, mem_mask, 13);
}

READ16_MEMBER(altos8600_state::coderam_r)
{
	return xlate_r(space, offset, mem_mask, 15);
}

WRITE16_MEMBER(altos8600_state::coderam_w)
{
	xlate_w(space, offset, data, mem_mask, 15);
}

READ16_MEMBER(altos8600_state::xtraram_r)
{
	return xlate_r(space, offset, mem_mask, 12);
}

WRITE16_MEMBER(altos8600_state::xtraram_w)
{
	xlate_w(space, offset, data, mem_mask, 12);
}

READ16_MEMBER(altos8600_state::cpuio_r)
{
	if(m_user)
	{
		seterr(offset, mem_mask, 0x800);
		return 0;
	}
	return m_dmac->space(AS_IO).read_word_unaligned(offset << 1, mem_mask);
}

WRITE16_MEMBER(altos8600_state::cpuio_w)
{
	if(m_user)
	{
		seterr(offset, mem_mask, 0x800);
		return;
	}
	m_dmac->space(AS_IO).write_word_unaligned(offset << 1, data, mem_mask);
}

READ16_MEMBER(altos8600_state::dmacram_r)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if((offset >= 0x7f000) && BIT(m_mode, 1))
		return m_bios->as_u16(offset & 0xfff);
	if(!BIT(flags, 10))
		seterr(offset, mem_mask, 0x400);
	return ((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)];

}

WRITE16_MEMBER(altos8600_state::dmacram_w)
{
	u8 page = m_mmuaddr[offset >> 11];
	u16 flags = m_mmuflags[offset >> 11];
	if(!BIT(flags, 6))
	{
		seterr(offset, mem_mask, 0x40);
		return;
	}
	COMBINE_DATA(&((u16 *)(m_ram->pointer()))[(page << 11) | (offset & 0x7ff)]);
}

READ16_MEMBER(altos8600_state::nmi_r)
{
	seterr(offset, mem_mask, 0x100);
	return 0;
}

WRITE16_MEMBER(altos8600_state::nmi_w)
{
	seterr(offset, mem_mask, 0x100);
}

static ADDRESS_MAP_START(cpu_mem, AS_PROGRAM, 16, altos8600_state)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(cpuram_r, cpuram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(stack_mem, i8086_cpu_device::AS_STACK, 16, altos8600_state)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(stkram_r, stkram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(code_mem, i8086_cpu_device::AS_CODE, 16, altos8600_state)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(coderam_r, coderam_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(extra_mem, i8086_cpu_device::AS_EXTRA, 16, altos8600_state)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(xtraram_r, xtraram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(cpu_io, AS_IO, 16, altos8600_state)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(cpuio_r, cpuio_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dmac_mem, AS_PROGRAM, 16, altos8600_state)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(dmacram_r, dmacram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dmac_io, AS_IO, 16, altos8600_state)
	AM_RANGE(0x0000, 0x0007) AM_READ(fault_r)
	AM_RANGE(0x0008, 0x000f) AM_WRITE(clear_w)
	AM_RANGE(0x0010, 0x0017) AM_READ(errlo_r)
	AM_RANGE(0x0018, 0x001f) AM_READ(errhi_r)
	AM_RANGE(0x0030, 0x0037) AM_WRITE(mode_w)
	AM_RANGE(0x0038, 0x003f) AM_WRITE8(cattn_w, 0xffff)
	AM_RANGE(0x0040, 0x0047) AM_DEVREADWRITE8("ppi", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0040, 0x0047) AM_DEVREADWRITE8("fd1797", fd1797_device, read, write, 0xff00)
	AM_RANGE(0x0048, 0x004f) AM_DEVREADWRITE8("uart8274", i8274_new_device, cd_ba_r, cd_ba_w, 0x00ff)
	AM_RANGE(0x0048, 0x004f) AM_DEVREADWRITE8("pit", pit8253_device, read, write, 0xff00)
	AM_RANGE(0x0058, 0x005f) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0060, 0x0067) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0068, 0x006f) AM_DEVREADWRITE8("pic8259_3", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0070, 0x0077) AM_NOP
	AM_RANGE(0x0200, 0x03ff) AM_READWRITE(mmuflags_r, mmuflags_w)
	AM_RANGE(0x0400, 0x05ff) AM_READWRITE(mmuaddr_r, mmuaddr_w)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(nmi_r, nmi_w)
ADDRESS_MAP_END

static SLOT_INTERFACE_START(altos8600_floppies)
	SLOT_INTERFACE( "8dd", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

static DEVICE_INPUT_DEFAULTS_START(altos8600_terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START(altos8600)
	MCFG_CPU_ADD("maincpu", I8086, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(cpu_mem)
	MCFG_CPU_IO_MAP(cpu_io)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(code_mem)
	MCFG_I8086_STACK_MAP(stack_mem)
	MCFG_I8086_CODE_MAP(code_mem)
	MCFG_I8086_EXTRA_MAP(extra_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)
	MCFG_I8086_IF_HANDLER(WRITELINE(altos8600_state, cpuif_w))

	MCFG_CPU_ADD("dmac", I8089, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(dmac_mem)
	MCFG_CPU_IO_MAP(dmac_io)
	MCFG_I8089_DATABUS_WIDTH(16)
	MCFG_I8089_SINTR1(DEVWRITELINE("pic8259_2", pic8259_device, ir3_w))
	MCFG_I8089_SINTR2(DEVWRITELINE("pic8259_2", pic8259_device, ir4_w))

	MCFG_PIC8259_ADD("pic8259_1", INPUTLINE("maincpu", 0), VCC, READ8(altos8600_state, get_slave_ack))
	MCFG_PIC8259_ADD("pic8259_2", DEVWRITELINE("pic8259_1", pic8259_device, ir2_w), GND, NOOP)
	MCFG_PIC8259_ADD("pic8259_3", DEVWRITELINE("pic8259_1", pic8259_device, ir3_w), GND, NOOP)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("512K")

	MCFG_I8274_ADD("uart8274", XTAL_16MHz/4, 0, 0, 0, 0)
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir7_w))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, ctsa_w))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, ctsb_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", altos8600_terminal)

	MCFG_DEVICE_ADD("ppi", I8255A, 0)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(1228800)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(altos8600_state, tmr0_w))
	MCFG_PIT8253_CLK1(1228800)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("uart8274", i8274_new_device, rxtxcb_w))
	MCFG_PIT8253_CLK2(1228800)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259_1", pic8259_device, ir1_w))

	MCFG_FD1797_ADD("fd1797", 1000000)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic8259_2", pic8259_device, ir1_w))
	MCFG_FLOPPY_DRIVE_ADD("fd1797:0", altos8600_floppies, "8dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1797:1", altos8600_floppies, "8dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1797:2", altos8600_floppies, "8dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1797:3", altos8600_floppies, "8dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

ROM_START(altos8600)
	ROM_REGION(0x2000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios", "bios")
	ROMX_LOAD("11753_1.5_lo.bin", 0x0000, 0x1000, CRC(dfa7bf0e) SHA1(6628fd7c579423b51d2642aeaa7fc0405a989252), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("11753_1.5_hi.bin", 0x0001, 0x1000, CRC(9b5e812c) SHA1(c2ef24859edd48d2096db47e16855c9bc01dae75), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

COMP(1981, altos8600, 0, 0, altos8600, 0, altos8600_state, 0, "Altos", "8600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
