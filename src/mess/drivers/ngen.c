/*

	Convergent NGen series

	10-11-14 - Skeleton driver

*/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/i386/i386.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/z80dart.h"

class ngen_state : public driver_device
{
public:
	ngen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_crtc(*this,"crtc"),
		m_viduart(*this,"videouart"),
		m_iouart(*this,"iouart"),
		m_dmac(*this,"dmac"),
		m_pic(*this,"pic"),
		m_pit(*this,"pit"),
		m_vram(*this,"vram"),
		m_fontram(*this,"fontram")
	{}

	DECLARE_WRITE_LINE_MEMBER(pit_out0_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out1_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_w);
	DECLARE_WRITE16_MEMBER(cpu_peripheral_cb);
	DECLARE_WRITE16_MEMBER(peripheral_w);
	DECLARE_READ16_MEMBER(peripheral_r);
	DECLARE_WRITE16_MEMBER(port00_w);
	DECLARE_READ16_MEMBER(port00_r);
	MC6845_UPDATE_ROW(crtc_update_row);

protected:
private:
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_viduart;
	required_device<upd7201_device> m_iouart;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;
	required_shared_ptr<UINT16> m_vram;
	required_shared_ptr<UINT16> m_fontram;

	UINT16 m_peripheral;
	UINT16 m_upper;
	UINT16 m_middle;
	UINT16 m_port00;
	UINT16 m_periph141;
};

WRITE_LINE_MEMBER(ngen_state::pit_out0_w)
{
	//m_pic->ir0_w(state);
	logerror("80186 Timer 1 state %i\n",state);
}

WRITE_LINE_MEMBER(ngen_state::pit_out1_w)
{
	logerror("PIT Timer 1 state %i\n",state);
}

WRITE_LINE_MEMBER(ngen_state::pit_out2_w)
{
	logerror("PIT Timer 2 state %i\n",state);
}

WRITE16_MEMBER(ngen_state::cpu_peripheral_cb)
{
	UINT32 addr;

	switch(offset)
	{
	case 0:  // upper memory
		m_upper = data;
		break;
	case 2:  // peripheral
		m_peripheral = data;
		addr = (m_peripheral & 0xffc0) << 4;
		if(m_middle & 0x0040)
		{
			m_maincpu->device_t::memory().space(AS_PROGRAM).install_readwrite_handler(addr, addr + 0x3ff, read16_delegate(FUNC(ngen_state::peripheral_r), this), write16_delegate(FUNC(ngen_state::peripheral_w), this));
			logerror("Mapped peripherals to memory 0x%08x\n",addr);
		}
		else
		{
			addr &= 0xffff;
			m_maincpu->device_t::memory().space(AS_IO).install_readwrite_handler(addr, addr + 0x3ff, read16_delegate(FUNC(ngen_state::peripheral_r), this), write16_delegate(FUNC(ngen_state::peripheral_w), this));
			logerror("Mapped peripherals to I/O 0x%04x\n",addr);
		}
		break;	
	case 4:
		m_middle = data;
		break;
	}
}

// 80186 peripheral space
// Largely guesswork at this stage
WRITE16_MEMBER(ngen_state::peripheral_w)
{
	switch(offset)
	{
	case 0x141:
		// bit 1 enables speaker?
		COMBINE_DATA(&m_periph141);
		break;
	case 0x144:
		if(mem_mask & 0x00ff)
			m_crtc->address_w(space,0,data & 0xff);
		break;
	case 0x145:
		if(mem_mask & 0x00ff)
			m_crtc->register_w(space,0,data & 0xff);
		break;
	case 0x146:
		if(mem_mask & 0x00ff)
			m_iouart->ba_cd_w(space,0,data & 0xff);
		logerror("Video write offset 0x146 data %04x mask %04x\n",data,mem_mask);
		break;
	case 0x147:
		if(mem_mask & 0x00ff)
			m_iouart->ba_cd_w(space,1,data & 0xff);
		logerror("Video write offset 0x147 data %04x mask %04x\n",data,mem_mask);
		break;
	default:
		logerror("(PC=%06x) Unknown 80186 peripheral write offset %04x data %04x mask %04x\n",m_maincpu->device_t::safe_pc(),offset,data,mem_mask);
	}
}

READ16_MEMBER(ngen_state::peripheral_r)
{
	UINT16 ret = 0xffff;
	switch(offset)
	{
	case 0x141:
		ret = m_periph141;
		break;
	case 0x144:
		if(mem_mask & 0x00ff)
			ret = m_crtc->status_r(space,0);
		break;
	case 0x145:
		if(mem_mask & 0x00ff)
			ret = m_crtc->register_r(space,0);
		break;
	case 0x146:
		if(mem_mask & 0x00ff)
			ret = m_iouart->ba_cd_r(space,0);
		break;
	case 0x147:  // definitely video related, likely UART sending data to the video board
		if(mem_mask & 0x00ff)
			ret = m_iouart->ba_cd_r(space,1);
		// expects bit 0 to be set (Video ready signal?)
		ret |= 1;
		break;
	default:
		logerror("(PC=%06x) Unknown 80186 peripheral read offset %04x mask %04x returning %04x\n",m_maincpu->device_t::safe_pc(),offset,mem_mask,ret);
	}
	return ret;
}

// A sequencial number is written to this port, and keeps on going until an NMI is triggered.
// Maybe this is a RAM test of some kind, and this would be a bank switch register?
// Even though the system supports 1MB at the most, which would fit in the CPU's whole address space...
WRITE16_MEMBER(ngen_state::port00_w)
{
	m_port00 = data;
	if(data > 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
	logerror("SYS: Port 0 write %04x\n",data);
}

READ16_MEMBER(ngen_state::port00_r)
{
	return m_port00;
}

MC6845_UPDATE_ROW( ngen_state::crtc_update_row )
{
	UINT16 addr = ma;

	for(int x=0;x<bitmap.width();x+=9)
	{
		UINT8 ch = m_vram[addr++];
		for(int z=0;z<9;z++)
		{
			if(BIT(m_fontram[ch*16+ra],8-z))
				bitmap.pix32(y,x+z) = rgb_t(0,0xff,0);
			else
				bitmap.pix32(y,x+z) = rgb_t(0,0,0);
		}
	}
}

static ADDRESS_MAP_START( ngen_mem, AS_PROGRAM, 16, ngen_state )
	AM_RANGE(0x00000, 0xf7fff) AM_RAM
	AM_RANGE(0xf8000, 0xf9fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xfa000, 0xfbfff) AM_RAM AM_SHARE("fontram")
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen_io, AS_IO, 16, ngen_state )
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(port00_r,port00_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386_mem, AS_PROGRAM, 32, ngen_state )
	AM_RANGE(0x00000000, 0x000fdfff) AM_RAM
	AM_RANGE(0x000fe000, 0x000fffff) AM_ROM AM_REGION("bios",0)
	AM_RANGE(0xffffe000, 0xffffffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386i_mem, AS_PROGRAM, 32, ngen_state )
	AM_RANGE(0x00000000, 0x000fbfff) AM_RAM
	AM_RANGE(0x000fc000, 0x000fffff) AM_ROM AM_REGION("bios",0)
	AM_RANGE(0xffffc000, 0xffffffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386_io, AS_IO, 32, ngen_state )
	AM_RANGE(0xfd0c, 0xfd0f) AM_DEVREADWRITE8("pic",pic8259_device,read,write,0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( ngen )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ngen, ngen_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ngen_mem)
	MCFG_CPU_IO_MAP(ngen_io)
	MCFG_80186_CHIP_SELECT_CB(WRITE16(ngen_state, cpu_peripheral_cb))
	MCFG_80186_TMROUT1_HANDLER(WRITELINE(ngen_state, pit_out0_w))

	MCFG_PIC8259_ADD( "pic", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4)  // correct?
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ngen_state, pit_out0_w))
	MCFG_PIT8253_CLK0(4772720/4)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ngen_state, pit_out1_w))
	MCFG_PIT8253_CLK0(4772720/4)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ngen_state, pit_out2_w))

	MCFG_DEVICE_ADD("dmac", AM9517A, XTAL_14_7456MHz / 3)  // NEC D8237A, divisor unknown

	// I/O board
	MCFG_UPD7201_ADD("iouart",XTAL_14_7456MHz / 3, 0,0,0,0) // no clock visible on I/O board, guessing for now

	// video board
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(720,348)
	MCFG_SCREEN_VISIBLE_AREA(0,719,0,347)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("crtc",mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, NULL, 19980000 / 9)  // divisor unknown -- /9 gives 60Hz output, so likely correct
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(ngen_state, crtc_update_row)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("videouart", I8251, 19980000 / 9)  // divisor unknown

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ngen386, ngen )
	MCFG_CPU_REPLACE("maincpu", I386, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ngen386_mem)
	MCFG_CPU_IO_MAP(ngen386_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( 386i, ngen386 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ngen386i_mem)
MACHINE_CONFIG_END

ROM_START( ngen )
	ROM_REGION( 0x2000, "bios", 0)
	ROM_LOAD16_BYTE( "72-00414_80186_cpu.bin",  0x000000, 0x001000, CRC(e1387a03) SHA1(ddca4eba67fbf8b731a8009c14f6b40edcbc3279) )
	ROM_LOAD16_BYTE( "72-00415_80186_cpu.bin",  0x000001, 0x001000, CRC(a6dde7d9) SHA1(b4d15c1bce31460ab5b92ff43a68c15ac5485816) )
ROM_END

// not sure just how similar these systems are to the 80186 model, but are here at the moment to document the dumps
ROM_START( ngenb38 )
	ROM_REGION( 0x2000, "bios", 0)
	ROM_LOAD16_BYTE( "72-168_fpc_386_cpu.bin",  0x000000, 0x001000, CRC(250a3b68) SHA1(49c070514bac264fa4892f284f7d2c852ae6605d) )
	ROM_LOAD16_BYTE( "72-167_fpc_386_cpu.bin",  0x000001, 0x001000, CRC(4010cc4e) SHA1(74a3024d605569056484d08b63f19fbf8eaf31c6) )
ROM_END

ROM_START( 386i )
	ROM_REGION( 0x4000, "bios", 0)
	ROM_LOAD16_BYTE( "72-1561o_386i_cpu.bin",  0x000000, 0x002000, CRC(b5efd768) SHA1(8b250d47d9c6eb82e1afaeb2244d8c4134ecbc47) )
	ROM_LOAD16_BYTE( "72-1562e_386i_cpu.bin",  0x000001, 0x002000, CRC(002d0d3a) SHA1(31de8592999377db9251acbeff348390a2d2602a) )

	ROM_REGION( 0x2000, "video", 0)
	ROM_LOAD( "72-1630_gc-104_vga.bin",  0x000000, 0x002000, CRC(4e4d8ebe) SHA1(50c96ccb4d0bd1beb2d1aee0d18b2c462d25fc8f) )
ROM_END


COMP( 1983, ngen,    0,      0,      ngen,           ngen, driver_device, 0,      "Convergent Technologies",  "NGEN CP-001", GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1991, ngenb38, ngen,   0,      ngen386,        ngen, driver_device, 0,      "Financial Products Corp.", "B28/38",      GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1990, 386i,    ngen,   0,      386i,           ngen, driver_device, 0,      "Convergent Technologies",  "386i",        GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
