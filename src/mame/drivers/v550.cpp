// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    The Visual 500 and 550 are the graphical members of the third series of display terminals released by Visual Technology.
    While they appear to use the same character generators as the text-mode-only Visual 300 and 330 and have similar detachable
    keyboards, they uniquely feature high-definition green-screen (P39 phosphor) monitors and support Tektronix 4010/4014-
    compatible graphics at a resolution of 768 x 555 pixels.

    The VT100-compatible Visual 550 was released first, and conforms to the ANSI X3.64 standard (like the Visual 300).
    The Visual 500 instead emulates the ADM3A, D200, Hazeltine 1500 and VT52 terminals (like the Visual 330).

***********************************************************************************************************************************/

#include "emu.h"
//include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/z80sio.h"
#include "video/scn2674.h"
#include "video/upd7220.h"
#include "screen.h"

class v550_state : public driver_device
{
public:
	v550_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_chargen(*this, "chargen")
	{ }

	void v550(machine_config &config);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }
private:
	void mem_map(address_map &map);
	void io_map(address_map &map);
	void pvtc_map(address_map &map);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_chargen;
};


void v550_state::mem_map(address_map &map)
{
	map(0x0000, 0x7bff).rom().region("maincpu", 0);
	map(0x7c00, 0x7fff).ram().share("nvram"); // actually 4 bits wide
	map(0x8000, 0x87ff).ram();
}

void v550_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("gdc", FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0x10, 0x10).w("brg1", FUNC(com8116_device::stt_str_w));
	map(0x20, 0x23).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x30).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x31, 0x31).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x40, 0x40).rw("mpsc", FUNC(upd7201_new_device::da_r), FUNC(upd7201_new_device::da_w));
	map(0x41, 0x41).rw("mpsc", FUNC(upd7201_new_device::ca_r), FUNC(upd7201_new_device::ca_w));
	map(0x48, 0x48).rw("mpsc", FUNC(upd7201_new_device::db_r), FUNC(upd7201_new_device::db_w));
	map(0x49, 0x49).rw("mpsc", FUNC(upd7201_new_device::cb_r), FUNC(upd7201_new_device::cb_w));
	map(0x50, 0x50).w("brg2", FUNC(com8116_device::stt_str_w));
	map(0x60, 0x67).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x70, 0x70).rw("pvtc", FUNC(scn2672_device::buffer_r), FUNC(scn2672_device::buffer_w));
	map(0x71, 0x71).noprw(); // TODO: attribute buffer
}

void v550_state::pvtc_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
}


INPUT_PORTS_START(v550)
INPUT_PORTS_END


void v550_state::machine_start()
{
	subdevice<i8251_device>("usart")->write_cts(0);
}

MACHINE_CONFIG_START(v550_state::v550)
	MCFG_DEVICE_ADD("maincpu", Z80, 4'000'000) // NEC D780C
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // NEC D446-2 + battery

	MCFG_DEVICE_ADD("gdc", UPD7220, 4'000'000) // NEC D7220D
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("ppi", I8255, 0) // NEC D8255AC-5

	MCFG_DEVICE_ADD("usart", I8251, 4'000'000) // NEC D8251AC
	MCFG_I8251_RXRDY_HANDLER(WRITELINE("mainint", input_merger_device, in_w<1>))

	MCFG_DEVICE_ADD("mpsc", UPD7201_NEW, 4'000'000) // NEC D7201C
	MCFG_Z80SIO_OUT_INT_CB(WRITELINE("mainint", input_merger_device, in_w<0>))

	MCFG_INPUT_MERGER_ANY_HIGH("mainint")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("brg1", COM8116, 5068800) // actually SMC COM8116T-020 (unknown clock)
	MCFG_COM8116_FT_HANDLER(WRITELINE("mpsc", upd7201_new_device, txca_w))
	MCFG_COM8116_FR_HANDLER(WRITELINE("mpsc", upd7201_new_device, rxca_w))

	MCFG_DEVICE_ADD("brg2", COM8116, 5068800) // actually SMC COM8116T-020
	MCFG_COM8116_FT_HANDLER(WRITELINE("mpsc", upd7201_new_device, txcb_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("mpsc", upd7201_new_device, rxcb_w))
	MCFG_COM8116_FR_HANDLER(WRITELINE("usart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("usart", i8251_device, write_rxc))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(16'248'600, 918, 0, 720, 295, 0, 272)
	MCFG_SCREEN_UPDATE_DRIVER(v550_state, screen_update)

	MCFG_DEVICE_ADD("pvtc", SCN2672, 1'805'400)
	MCFG_DEVICE_ADDRESS_MAP(0, pvtc_map)
	MCFG_SCN2672_CHARACTER_WIDTH(9)
	MCFG_SCN2672_INTR_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_VIDEO_SET_SCREEN("screen")
MACHINE_CONFIG_END


ROM_START( v550 )
	// Silkscreened on bottom left of PCB: "PA016-A REV B"

	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("e244-001_r07_u42.bin", 0x0000, 0x2000, CRC(d18a8b62) SHA1(7faf8a9f1ae3148adacbff960a2663793710cef2))
	ROM_LOAD("e244-002_r07_u43.bin", 0x2000, 0x2000, CRC(1b62db47) SHA1(7ad69aea6088545d843c2e95737895f81c269ccc))
	ROM_LOAD("e244-003_r07_u44.bin", 0x4000, 0x2000, CRC(f6d6f734) SHA1(a9efa8ebe86addb77872dbe9863ea7f75b33a2b9))
	ROM_LOAD("e244-017_r07_u45.bin", 0x6000, 0x2000, CRC(b0dcd535) SHA1(9237723d01a720217f50f756bb55c7e5ed05a594))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("e242-085_r03_u97.bin", 0x0000, 0x1000, CRC(8a491cee) SHA1(d8a9546a7dd2ffc0a5e54524ee16068dde56975c))
ROM_END

COMP( 1982, v550, 0, 0, v550, v550, v550_state, empty_init, "Visual Technology", "Visual 550", MACHINE_IS_SKELETON )
