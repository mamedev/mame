// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_NOKIA_MIKROMIKKO2_H
#define MAME_NOKIA_MIKROMIKKO2_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "bus/scsi/s1410.h"
#include "bus/scsi/scsihd.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "machine/nvram.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/z80sio.h"
#include "machine/x2212.h"
#include "sound/spkrdev.h"
#include "video/crt9007.h"
#include "video/crt9212.h"

#define I80186_TAG      "maincpu"
#define UPD765_TAG      "upd765"
#define SCREEN_TAG      "screen"

class mm2_state : public driver_device
{
public:
	mm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I80186_TAG),
		m_novram(*this, "x2212"),
		m_pic(*this, "pic8259"),
		m_pit(*this, "pit8253"),
		m_mpsc(*this, "i8274"),
		m_vpac(*this, "crt9007"),
		m_drb0(*this, "crt9212_0"),
		m_drb1(*this, "crt9212_1"),
		m_sio(*this, "i8251"),
		m_dmac(*this, "am9517a"),
		m_fdc(*this, UPD765_TAG),
		m_floppy(*this, UPD765_TAG ":%u:525qd", 0U),
		m_speaker(*this, "speaker"),
		m_sasi(*this, "sasi:7:scsicb"),
		m_rs232a(*this, "rs232a"),
		m_rs232b(*this, "rs232b"),
		m_palette(*this, "palette")
	{ }

	void mm2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<x2212_device> m_novram;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<i8274_device> m_mpsc;
	required_device<crt9007_device> m_vpac;
	required_device<crt9212_device> m_drb0;
	required_device<crt9212_device> m_drb1;
	required_device<i8251_device> m_sio;
	required_device<am9517a_device> m_dmac;
	required_device<upd765a_device> m_fdc;
	optional_device_array<floppy_image_device, 2> m_floppy;
	required_device<speaker_sound_device> m_speaker;
	required_device<nscsi_callback_device> m_sasi;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<palette_device> m_palette;

	void mm2_map(address_map &map);
	void mm2_io_map(address_map &map);
	void vpac_mem(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static void floppy_formats(format_registration &fr);

	void diag_w(offs_t offset, uint8_t data) { popmessage("%02x", data); }
	void novram_store(offs_t offset, uint8_t data);
	void novram_recall(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);

	bool m_vpac_int;
	bool m_sio_rxrdy;
	bool m_sio_txrdy;
	int m_cls;
	bool m_ir0;

	void update_pic_ir0() { m_pic->ir0_w(m_ir0); }
	void ir0_w(int state) { if (state) { m_ir0 = ASSERT_LINE; update_pic_ir0(); } }
	void tcl_w(offs_t offset, uint8_t data) { m_ir0 = CLEAR_LINE; update_pic_ir0(); }
	
	void update_pic_ir5() { m_pic->ir5_w(m_vpac_int || m_sio_rxrdy || m_sio_txrdy); }
	void vpac_int_w(int state) { m_vpac_int = state; update_pic_ir5(); }
	void sio_rxrdy_w(int state) { m_sio_rxrdy = state; update_pic_ir5(); }
	void sio_txrdy_w(int state) { m_sio_txrdy = state; update_pic_ir5(); }
	void dtra_w(offs_t offset, uint8_t data) { m_rs232a->write_txd(!BIT(data, 0)); }
	void dtrb_w(offs_t offset, uint8_t data) { m_rs232b->write_txd(!BIT(data, 0)); }
	uint8_t status_r(offs_t offset);
	void cls0_w(offs_t offset, uint8_t data) { m_cls = (m_cls & 0x2) | BIT(data, 0); }
	void cls1_w(offs_t offset, uint8_t data) { m_cls = (BIT(data, 0) << 1) | (m_cls & 0x1); }
	void tmrout0_w(int state) { if (m_cls == 0) { m_mpsc->rxca_w(state); m_mpsc->txca_w(state); } };
	void tmrout1_w(int state) { if (m_cls == 1) { m_mpsc->rxca_w(state); m_mpsc->txca_w(state); } };

	uint8_t m_dma_hi;
	uint8_t m_sasi_data;

	uint8_t dmac_mem_r(offs_t offset);
	void dmac_mem_w(offs_t offset, uint8_t data);

	void dma_hi_w(offs_t offset, uint8_t data) { m_dma_hi = data & 0x0f; }
	uint8_t sasi_status_r(offs_t offset);
	void sasi_cmd_w(offs_t offset, uint8_t data);
	uint8_t sasi_data_r(offs_t offset);
	void sasi_data_w(offs_t offset, uint8_t data);
	void sasi_bsy_w(int state);
	void sasi_req_w(int state);
	void sasi_io_w(int state);
	void fdc_reset_w(offs_t offset, uint8_t data) { m_fdc->reset_w(BIT(data, 0)); }
	void motor_on_w(offs_t offset, uint8_t data) { m_floppy[0]->mon_w(BIT(data, 0)); }

	bool m_cpl;
	bool m_blc;
	bool m_mode;
	bool m_modeg;
	bool m_c70_50;
	bool m_cru;
	bool m_crb;

	void cpl_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_cpl = BIT(data, 0); }
	void blc_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_blc = BIT(data, 0); }
	void mode_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_mode = BIT(data, 0); }
	void modeg_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_modeg = BIT(data, 0); }
	void c70_50_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_c70_50 = BIT(data, 0); }
	void cru_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_cru = BIT(data, 0); }
	void crb_w(offs_t offset, uint16_t data, uint16_t mem_mask) { m_crb = BIT(data, 0); }
};

#endif // MAME_NOKIA_MIKROMIKKO2_H
