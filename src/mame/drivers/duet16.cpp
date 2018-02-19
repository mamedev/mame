// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/msm58321.h"
#include "machine/6840ptm.h"
#include "machine/z80sio.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "video/mc6845.h"
#include "screen.h"

class duet16_state : public driver_device
{
public:
	duet16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void duet16(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	void duet16_io(address_map &map);
	void duet16_mem(address_map &map);
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(duet16_state::duet16_mem)
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xc0000, 0xc0fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xf80c0, 0xf80c1) AM_DEVWRITE8("crtc", h46505_device, address_w, 0x00ff)
	AM_RANGE(0xf80c2, 0xf80c3) AM_DEVWRITE8("crtc", h46505_device, register_w, 0x00ff)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(duet16_state::duet16_io)
ADDRESS_MAP_END

MC6845_UPDATE_ROW(duet16_state::crtc_update_row)
{

}

MACHINE_CONFIG_START(duet16_state::duet16)
	MCFG_CPU_ADD("maincpu", I8086, 24_MHz_XTAL/3)
	MCFG_CPU_PROGRAM_MAP(duet16_mem)
	MCFG_CPU_IO_MAP(duet16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic", pic8259_device, inta_cb)

	MCFG_CPU_ADD("i8741", I8741, 5_MHz_XTAL)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("pic", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("dmac", AM9517A, 5_MHz_XTAL)
	/*MCFG_AM9517A_OUT_HRQ_CB(WRITELINE(olyboss_state, hrq_w))
	MCFG_AM9517A_IN_MEMR_CB(READ8(olyboss_state, dma_mem_r))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(olyboss_state, dma_mem_w))
	MCFG_AM9517A_IN_IOR_0_CB(READ8(olyboss_state, fdcdma_r))
	MCFG_AM9517A_OUT_IOW_0_CB(WRITE8(olyboss_state, fdcdma_w))
	MCFG_AM9517A_OUT_IOW_2_CB(WRITE8(olyboss_state, crtcdma_w))
	MCFG_AM9517A_OUT_TC_CB(WRITELINE(olyboss_state, tc_w))*/

	MCFG_DEVICE_ADD("pit", PIT8253, 8_MHz_XTAL/8)

	MCFG_DEVICE_ADD("mpsc", UPD7201_NEW, 4_MHz_XTAL)

	MCFG_DEVICE_ADD("uart", I8251, 2_MHz_XTAL)

	MCFG_DEVICE_ADD("fdc", UPD765A, 0)

	MCFG_DEVICE_ADD("crtc", H46505, 2000000)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(duet16_state, crtc_update_row)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)

	MCFG_DEVICE_ADD("rtc", MSM58321, 32768_Hz_XTAL)
MACHINE_CONFIG_END

ROM_START(duet16)
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD16_BYTE("duet16_h516a_3.bin", 0x0001, 0x1000, CRC(936706aa) SHA1(412ff9c7bf4443d2ed29a8d792fc3c849c9393cc))
	ROM_LOAD16_BYTE("duet16_h517a_z.bin", 0x0000, 0x1000, CRC(1633cce8) SHA1(5145d04a48921cacfed17a94873e8988772fc8d4))

	ROM_REGION(0x2000, "char", 0)
	ROM_LOAD("duet16_char_j500a_4.bin", 0x0000, 0x2000, CRC(edf860f8) SHA1(0dcc584db701d21b7c3304cd2296562ebda6fb4c))

	ROM_REGION(0x400, "i8741", 0)
	ROM_LOAD("duet16_key_8741ak001b_z.bin", 0x000, 0x400, CRC(d23ee68d) SHA1(3b6a86fe2a304823c5385cd673f9580a35199dac))
ROM_END

COMP(1983, duet16, 0, 0, duet16, 0, duet16_state, 0, "Panafacom (Panasonic/Fujitsu)", "Duet-16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
