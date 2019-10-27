// license:BSD-3-Clause
// copyright-holders:Robbbert, Dirk Best
/****************************************************************************

    Qume QVT-102/QVT-102A video terminal

    Hardware:
    - Motorola 6800 CPU
    - Hitachi HD46505SP (Motorola 6845-compatible) CRTC
    - Hitachi HD46850 (Motorola 6850-compatible) ACIA
    - M58725P-15 (6116-compatible) (2k x 8bit RAM)
    - Zilog Z8430 CTC
    - 16.6698MHz Crystal
    - 2x TC5514-APL + 3V battery, functioning as NVRAM

    Keyboard: D8748D, 6.000MHz Crystal, Beeper

    TODO:
    - Support QVT-102A differences (bidirectional aux, different keyboard)
    - Key click sounds weird

****************************************************************************/

#ifndef MAME_MACHINE_QVT102_H
#define MAME_MACHINE_QVT102_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/6850acia.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "sound/spkrdev.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"

INPUT_PORTS_EXTERN(qvt102);

class qvt102_device : public device_t
{
public:
	qvt102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Interface to a RS232 connection.
	auto rs232_conn_txd_handler() { return m_rs232_conn_txd_handler.bind(); }
	auto rs232_conn_dtr_handler() { return m_rs232_conn_dtr_handler.bind(); }
	auto rs232_conn_rts_handler() { return m_rs232_conn_rts_handler.bind(); }

	// Interface to a RS232 connection.
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_dcd_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_dsr_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_ri_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_cts_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_rxd_w);

protected:
	qvt102_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<m6800_cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<i8748_device> m_kbdmcu;
	required_ioport_array<8> m_keys_p1;
	required_ioport_array<8> m_keys_p2;
	required_ioport m_keys_special;
	required_ioport m_jumper;
	required_device<acia6850_device> m_acia;
	required_device<rs232_port_device> m_aux;
	required_device<z80ctc_device> m_ctc;
	required_device<hd6845s_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<u8> m_char_rom;

	void mem_map(address_map &map);

	MC6845_UPDATE_ROW(crtc_update_row);

	DECLARE_READ8_MEMBER(vsync_ack_r);
	DECLARE_WRITE_LINE_MEMBER(vsync_w);
	DECLARE_READ8_MEMBER(kbd_r);
	DECLARE_WRITE8_MEMBER(latch_w);

	DECLARE_READ8_MEMBER(ctc_r);
	DECLARE_WRITE8_MEMBER(ctc_w);

	DECLARE_WRITE_LINE_MEMBER(acia_txd_w);
	DECLARE_WRITE_LINE_MEMBER(acia_rts_w);

	uint8_t m_latch;
	int m_kbd_data;

	// keyboard mcu
	DECLARE_READ8_MEMBER(mcu_bus_r);
	DECLARE_WRITE8_MEMBER(mcu_bus_w);
	DECLARE_READ_LINE_MEMBER(mcu_t0_r);
	DECLARE_READ_LINE_MEMBER(mcu_t1_r);
	DECLARE_WRITE8_MEMBER(mcu_p1_w);
	DECLARE_WRITE8_MEMBER(mcu_p2_w);

	uint8_t m_kbd_bus, m_kbd_p1, m_kbd_p2;

	devcb_write_line m_rs232_conn_txd_handler;
	devcb_write_line m_rs232_conn_dtr_handler;
	devcb_write_line m_rs232_conn_rts_handler;
};

DECLARE_DEVICE_TYPE(QVT102, qvt102_device)


class qvt102a_device : public qvt102_device
{
public:
	qvt102a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	qvt102a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override;
};

DECLARE_DEVICE_TYPE(QVT102A, qvt102a_device)

#endif // MAME_MACHINE_QVT102_H
