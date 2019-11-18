// license:BSD-3-Clause
// copyright-holders:Robbbert
#ifndef MAME_INCLUDES_AUSSIEBYTE_H
#define MAME_INCLUDES_AUSSIEBYTE_H

#pragma once

/***********************************************************

    Includes

************************************************************/
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"

#include "imagedev/floppy.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"

#include "sound/spkrdev.h"
#include "sound/votrax.h"

#include "video/mc6845.h"

#include "imagedev/snapquik.h"

#include "emupal.h"



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
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "vram")
		, m_p_attribram(*this, "aram")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_centronics(*this, "centronics")
		, m_rs232(*this, "rs232")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_crtc(*this, "crtc")
		, m_speaker(*this, "speaker")
		, m_votrax(*this, "votrax")
		, m_rtc(*this, "rtc")
	{ }

	void aussiebyte(machine_config &config);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
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
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(votrax_w);
	DECLARE_WRITE_LINE_MEMBER(sio1_rdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio1_rdyb_w);
	DECLARE_WRITE_LINE_MEMBER(sio2_rdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio2_rdyb_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_WRITE8_MEMBER(address_w);
	DECLARE_WRITE8_MEMBER(register_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void aussiebyte_io(address_map &map);
	void aussiebyte_map(address_map &map);

private:
	uint8_t crt8002(uint8_t ac_ra, uint8_t ac_chr, uint8_t ac_attr, uint16_t ac_cnt, bool ac_curs);
	bool m_port15; // rom switched in (0), out (1)
	uint8_t m_port17;
	uint8_t m_port17_rdy;
	uint8_t m_port19;
	uint8_t m_port1a; // bank to switch to when write to port 15 happens
	uint8_t m_port28;
	uint8_t m_port34;
	uint8_t m_port35; // byte to be written to vram or aram
	uint8_t m_video_index;
	uint16_t m_cnt;
	uint16_t m_alpha_address;
	uint16_t m_graph_address;
	int m_centronics_busy;
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_region_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_attribram;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<votrax_sc01_device> m_votrax;
	required_device<msm5832_device> m_rtc;
};

#endif // MAME_INCLUDES_AUSSIEBYTE_H
