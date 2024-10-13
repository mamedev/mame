// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
// thanks-to:James Wallace, Martin Buchholz, Juergen Oppermann, Volker Hann, Jan-Ole Christian
#ifndef MAME_DDR_POLYPLAY_H
#define MAME_DDR_POLYPLAY_H

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "emupal.h"

#define POLYPLAY_MAIN_CLOCK XTAL(9'830'400)

#define Z80CPU_TAG     "maincpu"
#define Z80CTC_TAG     "z80ctc"
#define Z80PIO_TAG     "z80pio"
#define Z80SIO_TAG     "z80sio"

class polyplay_state : public driver_device
{
public:
	polyplay_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_characterram(*this, "characterram"),
		m_maincpu(*this, Z80CPU_TAG),
		m_z80ctc(*this, Z80CTC_TAG),
		m_z80pio(*this, Z80PIO_TAG),
		m_z80sio(*this, Z80SIO_TAG),
		m_in0_port(*this, "IN0"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_speaker1(*this, "speaker1"),
		m_speaker2(*this, "speaker2"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void polyplay_zre(machine_config &config);
	void polyplay_zrepp(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	INTERRUPT_GEN_MEMBER(nmi_handler);

	/* devices */
	void ctc_zc0_w(int state);
	void ctc_zc1_w(int state);
	void ctc_zc2_w(int state);

	uint8_t pio_porta_r();
	void pio_porta_w(uint8_t data);
	uint8_t pio_portb_r();
	void pio_portb_w(uint8_t data);

	void polyplay_characterram_w(offs_t offset, uint8_t data);
	void polyplay_palette(palette_device &palette) const;
	uint32_t screen_update_polyplay(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void polyplay_io_zre(address_map &map) ATTR_COLD;
	void polyplay_io_zrepp(address_map &map) ATTR_COLD;
	void polyplay_mem_zre(address_map &map) ATTR_COLD;
	void polyplay_mem_zrepp(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_characterram;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_z80ctc;
	required_device<z80pio_device> m_z80pio;
	optional_device<z80sio_device> m_z80sio;
	required_ioport m_in0_port;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* audio */
	uint8_t m_flipflop1 = 0;
	uint8_t m_flipflop2 = 0;
	required_device<speaker_sound_device> m_speaker1;
	required_device<speaker_sound_device> m_speaker2;
	output_finder<4> m_lamps;
};

#endif // MAME_DDR_POLYPLAY_H
