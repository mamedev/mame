// license:BSD-3-Clause
// copyright-holders:Robbbert

/***********************************************************

    Includes

************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "bus/centronics/ctronics.h"
#include "sound/speaker.h"
#include "sound/votrax.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "machine/wd_fdc.h"
#include "machine/msm5832.h"


/***********************************************************

    Class

************************************************************/
class aussiebyte_state : public driver_device
{
public:
	aussiebyte_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_sio1(*this, "sio1")
		, m_sio2(*this, "sio2")
		, m_centronics(*this, "centronics")
		, m_rs232(*this, "rs232")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_crtc(*this, "crtc")
		, m_speaker(*this, "speaker")
		, m_votrax(*this, "votrax")
	{}

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE8_MEMBER(port15_w);
	DECLARE_WRITE8_MEMBER(port16_w);
	DECLARE_WRITE8_MEMBER(port17_w);
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_READ8_MEMBER(port19_r);
	DECLARE_WRITE8_MEMBER(port1a_w);
	DECLARE_WRITE8_MEMBER(port1b_w);
	DECLARE_WRITE8_MEMBER(port1c_w);
	DECLARE_WRITE8_MEMBER(port20_w);
	DECLARE_READ8_MEMBER(port28_r);
	DECLARE_READ8_MEMBER(port33_r);
	DECLARE_WRITE8_MEMBER(port34_w);
	DECLARE_WRITE8_MEMBER(port35_w);
	DECLARE_READ8_MEMBER(port36_r);
	DECLARE_READ8_MEMBER(port37_r);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(votrax_w);
	DECLARE_WRITE_LINE_MEMBER(sio1_rdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio1_rdyb_w);
	DECLARE_WRITE_LINE_MEMBER(sio2_rdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio2_rdyb_w);
	DECLARE_MACHINE_RESET(aussiebyte);
	DECLARE_DRIVER_INIT(aussiebyte);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_WRITE8_MEMBER(address_w);
	DECLARE_WRITE8_MEMBER(register_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);
	int m_centronics_busy;
	required_device<palette_device> m_palette;

private:
	UINT8 crt8002(UINT8 ac_ra, UINT8 ac_chr, UINT8 ac_attr, UINT16 ac_cnt, bool ac_curs);
	bool m_port15; // rom switched in (0), out (1)
	UINT8 m_port17;
	UINT8 m_port17_rdy;
	UINT8 m_port19;
	UINT8 m_port1a; // bank to switch to when write to port 15 happens
	UINT8 m_port28;
	UINT8 m_port34;
	UINT8 m_port35; // byte to be written to vram or aram
	UINT8 m_video_index;
	UINT16 m_cnt;
	UINT8 *m_p_videoram;
	UINT8 *m_p_attribram;
	const UINT8 *m_p_chargen;
	UINT16 m_alpha_address;
	UINT16 m_graph_address;
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<z80sio0_device> m_sio1;
	required_device<z80sio0_device> m_sio2;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<votrax_sc01_device> m_votrax;
};
