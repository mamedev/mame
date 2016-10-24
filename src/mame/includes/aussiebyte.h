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
#include "machine/clock.h"


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

	uint8_t memory_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void memory_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t io_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_centronics_busy(int state);
	void port15_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port16_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port17_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port18_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port19_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port1a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port1b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port1c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port20_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port28_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port33_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port34_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port35_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port36_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port37_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void busreq_w(int state);
	void votrax_w(int state);
	void sio1_rdya_w(int state);
	void sio1_rdyb_w(int state);
	void sio2_rdya_w(int state);
	void sio2_rdyb_w(int state);
	void clock_w(int state);
	void machine_reset_aussiebyte();
	void init_aussiebyte();
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	void address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void register_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);
	int m_centronics_busy;
	required_device<palette_device> m_palette;

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
	uint8_t *m_p_videoram;
	uint8_t *m_p_attribram;
	const uint8_t *m_p_chargen;
	uint16_t m_alpha_address;
	uint16_t m_graph_address;
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
