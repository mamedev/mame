// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_NOKIA_MIKROMIK_H
#define MAME_NOKIA_MIKROMIK_H

#pragma once

#include "mm1kb.h"
#include "bus/nscsi/devices.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/s1410.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/i8212.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80sio.h"
#include "video/i8275.h"
#include "video/upd7220.h"

#include "emupal.h"

#include "formats/mm_dsk.h"

#define SCREEN_TAG      "screen"
#define I8085A_TAG      "ic40"
#define I8212_TAG       "ic12"
#define I8237_TAG       "ic45"
#define I8253_TAG       "ic6"
#define UPD765_TAG      "ic15"
#define I8275_TAG       "ic59"
#define UPD7201_TAG     "ic11"
#define UPD7220_TAG     "ic101"
#define LS249_TAG       "ic48"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define RS232_C_TAG     "rs232c"
#define KB_TAG          "kb"

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8085A_TAG),
		m_io(*this, "io"),
		m_iop(*this, I8212_TAG),
		m_dmac(*this, I8237_TAG),
		m_pit(*this, I8253_TAG),
		m_crtc(*this, I8275_TAG),
		m_fdc(*this, UPD765_TAG),
		m_mpsc(*this, UPD7201_TAG),
		m_hgdc(*this, UPD7220_TAG),
		m_outlatch(*this, LS249_TAG),
		m_palette(*this, "palette"),
		m_floppy(*this, UPD765_TAG ":%u:525", 0U),
		m_sasi(*this, "sasi:7:scsicb"),
		m_rs232a(*this, RS232_A_TAG),
		m_rs232b(*this, RS232_B_TAG),
		m_rs232c(*this, RS232_C_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, I8085A_TAG),
		m_mmu_rom(*this, "address"),
		m_char_rom(*this, "chargen"),
		m_video_ram(*this, "video_ram"),
		m_a8(0),
		m_recall(0),
		m_dack3(1),
		m_tc(CLEAR_LINE),
		m_fdc_tc(0)
	{ }

	void common(machine_config &config);
	void mm1(machine_config &config);
	void mm1g(machine_config &config);
	void mm1m4(machine_config &config);
	void mm1m4g(machine_config &config);
	void mm1m6(machine_config &config);
	void mm1m6g(machine_config &config);
	void mm1m7(machine_config &config);
	void mm1m7g(machine_config &config);
	void mm1_320k_dual(machine_config &config);
	void mm1_640k_winchester(machine_config &config);
	void mm1_640k_dual(machine_config &config);
	void mm1_video(machine_config &config);
	void mm1g_video(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_io;
	required_device<i8212_device> m_iop;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<i8275_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_device<upd7201_device> m_mpsc;
	optional_device<upd7220_device> m_hgdc;
	optional_device<ls259_device> m_outlatch;
	required_device<palette_device> m_palette;
	optional_device_array<floppy_image_device, 2> m_floppy;
	optional_device<nscsi_callback_device> m_sasi;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<rs232_port_device> m_rs232c;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_mmu_rom;
	required_memory_region m_char_rom;
	optional_shared_ptr<uint16_t> m_video_ram;

	bool m_a8;

	// video state
	bool m_leen = 0;

	// serial state
	bool m_intc = 0;
	bool m_rx21 = 0;
	bool m_tx21 = 0;
	bool m_rcl = 0;

	// floppy state
	bool m_recall;
	bool m_dack3;
	bool m_tc;
	bool m_fdc_tc;

	// SASI state
	bool m_switch;
	uint8_t m_sasi_data;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void a8_w(int state) { m_a8 = state; }
	void recall_w(int state);
	void rx21_w(int state) { m_rx21 = state; }
	void tx21_w(int state) { m_tx21 = state; }
	void rcl_w(int state) { m_rcl = state; }
	void intc_w(int state) { m_intc = state; }
	void leen_w(int state) { m_leen = state; }
	void motor_on_w(int state);
	void switch_w(int state);
	uint8_t sasi_status_r(offs_t offset);
	void sasi_cmd_w(offs_t offset, uint8_t data);
	uint8_t sasi_data_r(offs_t offset);
	void sasi_data_w(offs_t offset, uint8_t data);
	uint8_t sasi_ior3_r(offs_t offset);
	void sasi_iow3_w(offs_t offset, uint8_t data);
	void sasi_bsy_w(int state);
	void sasi_req_w(int state);
	void sasi_io_w(int state);
	void dma_hrq_w(int state);
	uint8_t mpsc_dack_r();
	void mpsc_dack_w(uint8_t data);
	void dma_eop_w(int state);
	void dack3_w(int state);
	void itxc_w(int state);
	void irxc_w(int state);
	void auxc_w(int state);
	void drq2_w(int state);
	void drq1_w(int state);
	int dsra_r();

	void update_tc();

	static void floppy_formats(format_registration &fr);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	void mm1_palette(palette_device &palette) const;
	void mm1_map(address_map &map) ATTR_COLD;
	void mmu_io_map(address_map &map) ATTR_COLD;
	void mm1g_mmu_io_map(address_map &map) ATTR_COLD;
	void mm1_upd7220_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NOKIA_MIKROMIK_H
