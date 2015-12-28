// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
 Project PROCONN (and PC92/PC98) Fruit Machine hardware
  skeleton driver!

  what's the difference between the platforms, sound hardware?

  Some early Proconn hardware was used for Maygay stuff, so this may be a bit of a mix.

  Error codes:

  ERROR 1   EPROM FAILED
  ERROR 3   RAM CORRUPTION DETECTED (common on PC90 boards)
  ERROR 10  RAM FAILURE
  ERROR 20  METERS DISCONNECTED
  ERROR 21  CASH IN METER FAIL
  ERROR 22  CASH OUT METER FAIL
  ERROR 25  REFILL METER FAIL
  ERROR 30  REEL 1 FAIL
  ERROR 31  REEL 2 FAIL
  ERROR 32  REEL 3 FAIL
  ERROR 99  SECURITY CARD MISSING OR FAILED, OR INCORRECT FOR PROGRAM
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "sound/ay8910.h"
#include "video/awpvid.h"
#include "machine/roc10937.h"
#include "machine/meters.h"

#include "proconn.lh"

class proconn_state : public driver_device
{
public:
	proconn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_vfd(*this, "vfd"),
			m_maincpu(*this, "maincpu"),
			m_z80pio_1(*this, "z80pio_1"),
			m_z80pio_2(*this, "z80pio_2"),
			m_z80pio_3(*this, "z80pio_3"),
			m_z80pio_4(*this, "z80pio_4"),
			m_z80pio_5(*this, "z80pio_5"),
			m_z80ctc(*this, "z80ctc"),
			m_z80sio(*this, "z80sio"),
			m_ay(*this, "aysnd")
	{ }

	optional_device<s16lf01_t> m_vfd;

	DECLARE_WRITE8_MEMBER( ay_w0 ) { m_ay->address_data_w(space, 0, data); }
	DECLARE_WRITE8_MEMBER( ay_w1 ) { m_ay->address_data_w(space, 1, data); }

	DECLARE_WRITE8_MEMBER( ctc_w0 ) { m_z80ctc->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( ctc_w1 ) { m_z80ctc->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( ctc_w2 ) { m_z80ctc->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( ctc_w3 ) { m_z80ctc->write(space, 3, data); }

	DECLARE_WRITE8_MEMBER( sio_w0 ) { m_z80sio->cd_ba_w(space, 0, data); }
	DECLARE_WRITE8_MEMBER( sio_w1 ) { m_z80sio->cd_ba_w(space, 1, data); }
	DECLARE_WRITE8_MEMBER( sio_w2 ) { m_z80sio->cd_ba_w(space, 2, data); }
	DECLARE_WRITE8_MEMBER( sio_w3 ) { m_z80sio->cd_ba_w(space, 3, data); }

	DECLARE_WRITE8_MEMBER( pio1_w0 ) { m_z80pio_1->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( pio1_w1 ) { m_z80pio_1->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( pio1_w2 ) { m_z80pio_1->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( pio1_w3 ) { m_z80pio_1->write(space, 3, data); }

	DECLARE_WRITE8_MEMBER( pio2_w0 ) { m_z80pio_2->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( pio2_w1 ) { m_z80pio_2->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( pio2_w2 ) { m_z80pio_2->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( pio2_w3 ) { m_z80pio_2->write(space, 3, data); }

	DECLARE_WRITE8_MEMBER( pio3_w0 ) { m_z80pio_3->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( pio3_w1 ) { m_z80pio_3->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( pio3_w2 ) { m_z80pio_3->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( pio3_w3 ) { m_z80pio_3->write(space, 3, data); }

	DECLARE_WRITE8_MEMBER( pio4_w0 ) { m_z80pio_4->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( pio4_w1 ) { m_z80pio_4->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( pio4_w2 ) { m_z80pio_4->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( pio4_w3 ) { m_z80pio_4->write(space, 3, data); }

	DECLARE_WRITE8_MEMBER( pio5_w0 ) { m_z80pio_5->write(space, 0, data); }
	DECLARE_WRITE8_MEMBER( pio5_w1 ) { m_z80pio_5->write(space, 1, data); }
	DECLARE_WRITE8_MEMBER( pio5_w2 ) { m_z80pio_5->write(space, 2, data); }
	DECLARE_WRITE8_MEMBER( pio5_w3 ) { m_z80pio_5->write(space, 3, data); }

	DECLARE_READ8_MEMBER( ay_r0 ) { return m_ay->data_r(space, 0); }

	DECLARE_READ8_MEMBER( ctc_r0 ) { return m_z80ctc->read(space, 0); }
	DECLARE_READ8_MEMBER( ctc_r1 ) { return m_z80ctc->read(space, 1); }
	DECLARE_READ8_MEMBER( ctc_r2 ) { return m_z80ctc->read(space, 2); }
	DECLARE_READ8_MEMBER( ctc_r3 ) { return m_z80ctc->read(space, 3); }

	DECLARE_READ8_MEMBER( sio_r0 ) { return m_z80sio->cd_ba_r(space, 0); }
	DECLARE_READ8_MEMBER( sio_r1 ) { return m_z80sio->cd_ba_r(space, 1); }
	DECLARE_READ8_MEMBER( sio_r2 ) { return m_z80sio->cd_ba_r(space, 2); }
	DECLARE_READ8_MEMBER( sio_r3 ) { return m_z80sio->cd_ba_r(space, 3); }

	DECLARE_READ8_MEMBER( pio1_r0 ) { return m_z80pio_1->read(space, 0); }
	DECLARE_READ8_MEMBER( pio1_r1 ) { return m_z80pio_1->read(space, 1); }
	DECLARE_READ8_MEMBER( pio1_r2 ) { return m_z80pio_1->read(space, 2); }
	DECLARE_READ8_MEMBER( pio1_r3 ) { return m_z80pio_1->read(space, 3); }

	DECLARE_READ8_MEMBER( pio2_r0 ) { return m_z80pio_2->read(space, 0); }
	DECLARE_READ8_MEMBER( pio2_r1 ) { return m_z80pio_2->read(space, 1); }
	DECLARE_READ8_MEMBER( pio2_r2 ) { return m_z80pio_2->read(space, 2); }
	DECLARE_READ8_MEMBER( pio2_r3 ) { return m_z80pio_2->read(space, 3); }

	DECLARE_READ8_MEMBER( pio3_r0 ) { return m_z80pio_3->read(space, 0); }
	DECLARE_READ8_MEMBER( pio3_r1 ) { return m_z80pio_3->read(space, 1); }
	DECLARE_READ8_MEMBER( pio3_r2 ) { return m_z80pio_3->read(space, 2); }
	DECLARE_READ8_MEMBER( pio3_r3 ) { return m_z80pio_3->read(space, 3); }

	DECLARE_READ8_MEMBER( pio4_r0 ) { return m_z80pio_4->read(space, 0); }
	DECLARE_READ8_MEMBER( pio4_r1 ) { return m_z80pio_4->read(space, 1); }
	DECLARE_READ8_MEMBER( pio4_r2 ) { return m_z80pio_4->read(space, 2); }
	DECLARE_READ8_MEMBER( pio4_r3 ) { return m_z80pio_4->read(space, 3); }

	DECLARE_READ8_MEMBER( pio5_r0 ) { return m_z80pio_5->read(space, 0); }
	DECLARE_READ8_MEMBER( pio5_r1 ) { return m_z80pio_5->read(space, 1); }
	DECLARE_READ8_MEMBER( pio5_r2 ) { return m_z80pio_5->read(space, 2); }
	DECLARE_READ8_MEMBER( pio5_r3 ) { return m_z80pio_5->read(space, 3); }

	/* PIO 1 */

	DECLARE_WRITE_LINE_MEMBER(pio_1_m_out_int_w)    { /* logerror("pio_1_m_out_int_w %02x\n", state); */ }
	DECLARE_READ8_MEMBER(pio_1_m_in_pa_r)           { logerror("pio_1_m_in_pa_r (INPUT MATRIX)\n"); return space.machine().rand(); }
	DECLARE_WRITE8_MEMBER(pio_1_m_out_pa_w)         { logerror("pio_1_m_out_pa_w %02x\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_1_m_out_ardy_w)   { logerror("pio_1_m_out_ardy_w %02x\n", state); }
	DECLARE_READ8_MEMBER(pio_1_m_in_pb_r)           { logerror("pio_1_m_in_pb_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_1_m_out_pb_w)         { logerror("pio_1_m_out_pb_w %02x (REELS)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_1_m_out_brdy_w)   { logerror("pio_1_m_out_brdy_w %02x\n", state); }

	/* PIO 2 */
	DECLARE_WRITE_LINE_MEMBER(pio_2_m_out_int_w)    { /* logerror("pio_2_m_out_int_w %02x\n", state); */ }
	DECLARE_READ8_MEMBER(pio_2_m_in_pa_r)           { logerror("pio_2_m_in_pa_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_2_m_out_pa_w)         { logerror("pio_2_m_out_pa_w %02x\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_2_m_out_ardy_w)   { logerror("pio_2_m_out_ardy_w %02x\n", state); }
	DECLARE_READ8_MEMBER(pio_2_m_in_pb_r)           { logerror("pio_2_m_in_pb_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_2_m_out_pb_w)         { logerror("pio_2_m_out_pb_w %02x (ALPHA)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_2_m_out_brdy_w)   { logerror("pio_2_m_out_brdy_w %02x\n", state); }

	/* PIO 3 */
	DECLARE_WRITE_LINE_MEMBER(pio_3_m_out_int_w)    { /* logerror("pio_3_m_out_int_w %02x\n", state); */ }
	DECLARE_READ8_MEMBER(pio_3_m_in_pa_r)           { logerror("pio_3_m_in_pa_r (REEL OPTICS)\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_3_m_out_pa_w)         { logerror("pio_3_m_out_pa_w %02x (STROBE)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_3_m_out_ardy_w)   { logerror("pio_3_m_out_ardy_w %02x\n", state); }
	DECLARE_READ8_MEMBER(pio_3_m_in_pb_r)           { logerror("pio_3_m_in_pb_r (COIN INPUT)\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_3_m_out_pb_w)         { logerror("pio_3_m_out_pb_w %02x\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_3_m_out_brdy_w)   { logerror("pio_3_m_out_brdy_w %02x\n", state); }

	/* PIO 4 */
	DECLARE_WRITE_LINE_MEMBER(pio_4_m_out_int_w)    { /* logerror("pio_4_m_out_int_w %02x\n", state); */ }
	DECLARE_READ8_MEMBER(pio_4_m_in_pa_r)           { logerror("pio_4_m_in_pa_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_4_m_out_pa_w)         { logerror("pio_4_m_out_pa_w %02x (TRIAC)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_4_m_out_ardy_w)   { logerror("pio_4_m_out_ardy_w %02x\n", state); }
	DECLARE_READ8_MEMBER(pio_4_m_in_pb_r)           { logerror("pio_4_m_in_pb_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_4_m_out_pb_w)         { logerror("pio_4_m_out_pb_w %02x (7SEG)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_4_m_out_brdy_w)   { logerror("pio_4_m_out_brdy_w %02x\n", state); }

	/* PIO 5 */
	DECLARE_WRITE_LINE_MEMBER(pio_5_m_out_int_w)    { /* logerror("pio_5_m_out_int_w %02x\n", state); */ }
	DECLARE_READ8_MEMBER(pio_5_m_in_pa_r)           { logerror("pio_5_m_in_pa_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_5_m_out_pa_w)         { logerror("pio_5_m_out_pa_w %02x (LAMPS0)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_5_m_out_ardy_w)   { logerror("pio_5_m_out_ardy_w %02x\n", state); }
	DECLARE_READ8_MEMBER(pio_5_m_in_pb_r)           { logerror("pio_5_m_in_pb_r\n"); return 0x00; }
	DECLARE_WRITE8_MEMBER(pio_5_m_out_pb_w)         { logerror("pio_5_m_out_pb_w %02x (LAMPS1)\n", data); }
	DECLARE_WRITE_LINE_MEMBER(pio_5_m_out_brdy_w)   { logerror("pio_5_m_out_brdy_w %02x\n", state); }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_z80pio_1;
	required_device<z80pio_device> m_z80pio_2;
	required_device<z80pio_device> m_z80pio_3;
	required_device<z80pio_device> m_z80pio_4;
	required_device<z80pio_device> m_z80pio_5;
	required_device<z80ctc_device> m_z80ctc;
	required_device<z80dart_device> m_z80sio;
	required_device<ay8910_device> m_ay;
public:
	int m_meter;
	DECLARE_DRIVER_INIT(proconn);
	virtual void machine_reset() override;
	DECLARE_WRITE8_MEMBER(meter_w);
	DECLARE_WRITE16_MEMBER(serial_transmit);
	DECLARE_READ16_MEMBER(serial_receive);
};

static ADDRESS_MAP_START( proconn_map, AS_PROGRAM, 8, proconn_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END




// the mapping of the devices is rather ugly with address bits 8-9 providing the usual address bits 0-1 or 'offset'
// r0/r1/r2/r3 and w0/w1/w2/w3 might still be in the wrong order at the moment.
static ADDRESS_MAP_START( proconn_portmap, AS_IO, 8, proconn_state )
//  ADDRESS_MAP_GLOBAL_MASK(0x3ff)

	// sio (vfd should be connected to it?)
	AM_RANGE(0x00ff, 0x00ff) AM_READWRITE(sio_r0, sio_w0)
	AM_RANGE(0x01ff, 0x01ff) AM_READWRITE(sio_r2, sio_w2)
	AM_RANGE(0x02ff, 0x02ff) AM_READWRITE(sio_r1, sio_w1)
	AM_RANGE(0x03ff, 0x03ff) AM_READWRITE(sio_r3, sio_w3)

	// ctc
	AM_RANGE(0x00fe, 0x00fe) AM_READWRITE(ctc_r0, ctc_w0)
	AM_RANGE(0x01fe, 0x01fe) AM_READWRITE(ctc_r2, ctc_w2)
	AM_RANGE(0x02fe, 0x02fe) AM_READWRITE(ctc_r1, ctc_w1)
	AM_RANGE(0x03fe, 0x03fe) AM_READWRITE(ctc_r3, ctc_w3)

	// ay (meters connected to it?)
	AM_RANGE(0x00fd, 0x00fd) AM_READWRITE(ay_r0, ay_w0)
	AM_RANGE(0x00fc, 0x00fc) AM_WRITE(ay_w1)

	// ??
	AM_RANGE(0xfbf9, 0xfbf9) AM_WRITENOP
	AM_RANGE(0xfff9, 0xfff9) AM_WRITENOP

	// pio5 (lamps?)
	AM_RANGE(0x00f0, 0x00f0) AM_READWRITE(pio5_r0, pio5_w0)
	AM_RANGE(0x01f0, 0x01f0) AM_READWRITE(pio5_r1, pio5_w1)
	AM_RANGE(0x02f0, 0x02f0) AM_READWRITE(pio5_r2, pio5_w2)
	AM_RANGE(0x03f0, 0x03f0) AM_READWRITE(pio5_r3, pio5_w3)

	// pio4 (triacs + 7segs)
	AM_RANGE(0x00e8, 0x00e8) AM_READWRITE(pio4_r0, pio4_w0)
	AM_RANGE(0x01e8, 0x01e8) AM_READWRITE(pio4_r1, pio4_w1)
	AM_RANGE(0x02e8, 0x02e8) AM_READWRITE(pio4_r2, pio4_w2)
	AM_RANGE(0x03e8, 0x03e8) AM_READWRITE(pio4_r3, pio4_w3)

	// pio3 (lamps? + opto in?)
	AM_RANGE(0x00d8, 0x00d8) AM_READWRITE(pio3_r0, pio3_w0)
	AM_RANGE(0x01d8, 0x01d8) AM_READWRITE(pio3_r1, pio3_w1)
	AM_RANGE(0x02d8, 0x02d8) AM_READWRITE(pio3_r2, pio3_w2)
	AM_RANGE(0x03d8, 0x03d8) AM_READWRITE(pio3_r3, pio3_w3)

	// pio2 (reels?)
	AM_RANGE(0x00b8, 0x00b8) AM_READWRITE(pio2_r0, pio2_w0)
	AM_RANGE(0x01b8, 0x01b8) AM_READWRITE(pio2_r1, pio2_w1)
	AM_RANGE(0x02b8, 0x02b8) AM_READWRITE(pio2_r2, pio2_w2)
	AM_RANGE(0x03b8, 0x03b8) AM_READWRITE(pio2_r3, pio2_w3)

	// pio1 (reels? + inputs?)
	AM_RANGE(0x0078, 0x0078) AM_READWRITE(pio1_r0, pio1_w0)
	AM_RANGE(0x0178, 0x0178) AM_READWRITE(pio1_r1, pio1_w1)
	AM_RANGE(0x0278, 0x0278) AM_READWRITE(pio1_r2, pio1_w2)
	AM_RANGE(0x0378, 0x0378) AM_READWRITE(pio1_r3, pio1_w3)
ADDRESS_MAP_END


static INPUT_PORTS_START( proconn )
INPUT_PORTS_END

WRITE16_MEMBER(proconn_state::serial_transmit)
{
//Don't like the look of this, should be a clock somewhere

	// should probably be in the pios above

	if (offset == 0)
	{
		for (int i=0; i<8;i++)
		{
			m_vfd->data(data & (1<<i));
			m_vfd->sclk(1);
			m_vfd->sclk(0);
		}
	}
}

READ16_MEMBER(proconn_state::serial_receive)
{
	logerror("proconn serial receive read %x",offset);
	return -1;
}

WRITE8_MEMBER(proconn_state::meter_w)
{
	int i;
	for (i=0; i<8; i++)
	{
		if ( data & (1 << i) )
		{
			MechMtr_update(i, data & (1 << i) );
			m_meter = data;
		}
	}
}

static const z80_daisy_config z80_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80sio" },
	{ nullptr }
};

void proconn_state::machine_reset()
{
	m_vfd->reset(); // reset display1
}

static MACHINE_CONFIG_START( proconn, proconn_state )
	MCFG_CPU_ADD("maincpu", Z80, 4000000) /* ?? Mhz */
	MCFG_CPU_CONFIG(z80_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(proconn_map)
	MCFG_CPU_IO_MAP(proconn_portmap)
	MCFG_S16LF01_ADD("vfd",0)

	MCFG_DEVICE_ADD("z80pio_1", Z80PIO, 4000000) /* ?? Mhz */
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(proconn_state,pio_1_m_out_int_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(proconn_state, pio_1_m_in_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(proconn_state, pio_1_m_out_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(proconn_state, pio_1_m_out_ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(proconn_state, pio_1_m_in_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(proconn_state, pio_1_m_out_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(proconn_state, pio_1_m_out_brdy_w))

	MCFG_DEVICE_ADD("z80pio_2", Z80PIO, 4000000) /* ?? Mhz */
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(proconn_state,pio_2_m_out_int_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(proconn_state, pio_2_m_in_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(proconn_state, pio_2_m_out_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(proconn_state, pio_2_m_out_ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(proconn_state, pio_2_m_in_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(proconn_state, pio_2_m_out_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(proconn_state, pio_2_m_out_brdy_w))

	MCFG_DEVICE_ADD("z80pio_3", Z80PIO, 4000000) /* ?? Mhz */
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(proconn_state,pio_3_m_out_int_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(proconn_state, pio_3_m_in_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(proconn_state, pio_3_m_out_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(proconn_state, pio_3_m_out_ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(proconn_state, pio_3_m_in_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(proconn_state, pio_3_m_out_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(proconn_state, pio_3_m_out_brdy_w))

	MCFG_DEVICE_ADD("z80pio_4", Z80PIO, 4000000) /* ?? Mhz */
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(proconn_state,pio_4_m_out_int_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(proconn_state, pio_4_m_in_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(proconn_state, pio_4_m_out_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(proconn_state, pio_4_m_out_ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(proconn_state, pio_4_m_in_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(proconn_state, pio_4_m_out_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(proconn_state, pio_4_m_out_brdy_w))

	MCFG_DEVICE_ADD("z80pio_5", Z80PIO, 4000000) /* ?? Mhz */
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(proconn_state,pio_5_m_out_int_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(proconn_state, pio_5_m_in_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(proconn_state, pio_5_m_out_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(proconn_state, pio_5_m_out_ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(proconn_state, pio_5_m_in_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(proconn_state, pio_5_m_out_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(proconn_state, pio_5_m_out_brdy_w))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, 4000000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD( "z80sio",   4000000, 0, 0, 0, 0 ) /* ?? Mhz */

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")


	MCFG_DEFAULT_LAYOUT(layout_proconn)

	MCFG_SOUND_ADD("aysnd", AY8910, 1000000) /* ?? Mhz */ // YM2149F on PC92?
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(proconn_state, meter_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)
MACHINE_CONFIG_END




ROM_START( pr_bears )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bs8pv26.rom", 0x0000, 0x008000, CRC(d6f7c02c) SHA1(d6549a5d47d644366c1c21ca2455ece3f282f5a5) )
ROM_END

ROM_START( pr_bearsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_bs47.bin", 0x0000, 0x008000, CRC(fa6fded4) SHA1(3dac6afcc03fce9f0c33d9776eb3b96e34d50ce5) )
ROM_END

ROM_START( pr_bearsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_bs37.bin", 0x0000, 0x008000, CRC(03f7f043) SHA1(437085466d8f1efa0359593e7f9c2616ed4033cb) )
ROM_END

ROM_START( pr_bearx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bear x 2.3 8.bin", 0x0000, 0x008000, CRC(db781396) SHA1(5b1755b481c1ccb02508d17fc2738878a5fe5f44) )
ROM_END

ROM_START( pr_bearxa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bear x v2-2 (27256)", 0x0000, 0x008000, CRC(86ec85b1) SHA1(49304d0c6cffc34a28a4205c3ed9893dabb95246) )
ROM_END

ROM_START( pr_bearxb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bear x v1-3 (27256)", 0x0000, 0x008000, CRC(c81fd801) SHA1(4833fdeeb8fc4d5e5a087a1077482ddb811cded8) )
ROM_END

ROM_START( pr_bearxc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx20psec.rom", 0x0000, 0x008000, CRC(4004e75c) SHA1(d1a73869df937519682824095eddcbe5439a3764) )
ROM_END

ROM_START( pr_bearxd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx20pse.rom", 0x0000, 0x008000, CRC(23b14cfc) SHA1(46534d21aa507de0677c5419aa281ec3184f9738) )
ROM_END

ROM_START( pr_bearxe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx_v31~1.bin", 0x0000, 0x008000, CRC(0be7cf73) SHA1(ff7e4eb1b985a9085fba2a342e138a6045db6bde) )//bx10s10p.rom
ROM_END

ROM_START( pr_bearxf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx10psec.rom", 0x0000, 0x008000, CRC(485fc104) SHA1(89bbb5411da9cfb91ab2f58285829948dba64776) )
ROM_END

ROM_START( pr_bearxg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx10pse.rom", 0x0000, 0x008000, CRC(2bea6aa4) SHA1(fc8cebe6be44c2806dd851ef52d4209a51ceb805) )
ROM_END

ROM_START( pr_bearxh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx10s20p.rom", 0x0000, 0x008000, CRC(03bce92b) SHA1(09ea35ba2dc41e96c94892532b5162796482a485) )
ROM_END

ROM_START( pr_bearxi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx3110p.rom", 0x0000, 0x008000, CRC(0fca5c5f) SHA1(8dad9995c8c46bd82afe38bb1474b766fd6b2bc8) )
ROM_END

ROM_START( pr_bearxj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx813c10.bin", 0x0000, 0x008000, CRC(abaa73a1) SHA1(b83a9bb0ad83c148cb58a8ec66fce076f8335736) )
ROM_END

ROM_START( pr_bearxk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx8t10n6.bin", 0x0000, 0x008000, CRC(829dcc99) SHA1(014b53999becb241a3fcd34e802be8b58bfeb3c4) )
ROM_END

ROM_START( pr_bearxl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_bx41.bin", 0x0000, 0x008000, CRC(e9212049) SHA1(215038324baa928b3c41495e78a2be68c8a1a767) )//bx1c20n7.bin
ROM_END

ROM_START( pr_bearxlp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bx_v4_1p.bin", 0x0000, 0x010000, CRC(6adb5508) SHA1(9d2276d13c82f9d82bdd5103e8043c463845e514) )
ROM_END

ROM_START( pr_bearxm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_bx31.bin", 0x0000, 0x008000, CRC(07917a07) SHA1(4fc51550a3e16073762f92dd2b81987dd4285570) )
ROM_END

ROM_START( pr_fspot )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_fs41.bin", 0x0000, 0x008000, CRC(fa93c9b5) SHA1(e7a33963730dc14c1d1e513d064e1a341f104339) )
ROM_END

ROM_START( pr_fspota )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p_fs31.bin", 0x0000, 0x008000, CRC(c9ae0199) SHA1(a67c99eb37e5e534d0fd64f1d8f43af3cfe2beb5) )
ROM_END

ROM_START( pr_fspotb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fspot_v2.1", 0x0000, 0x008000, CRC(8ed27c1b) SHA1(31ff8dd2fbcac2b19b64196256de7de2f6cf8d66) )
ROM_END

ROM_START( pr_fspotc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "funspot v2.1 20p po 82%.bin", 0x0000, 0x008000, CRC(caa45a7f) SHA1(7b04be4157750817e77217ebad2c4d2092496347) )
ROM_END

ROM_START( pr_fspotd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "funspot v2.1 7 button.bin", 0x0000, 0x008000, CRC(b8c90956) SHA1(dbf51ffee21b2a2017dd713b75029110d8cace33) )
ROM_END

ROM_START( pr_fspote )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fspot_v1.1", 0x0000, 0x008000, CRC(d826951e) SHA1(9476936987555aad4cf5432072c1841702b507cf) )
ROM_END

ROM_START( pr_fspotf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "funspot v1.1 20p po 82%.bin", 0x0000, 0x008000, CRC(9c50b37a) SHA1(cffd52ba9c452e51df0cab0e4766534b6b8a1386) )
ROM_END

ROM_START( pr_fspotg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "funspot v1.1 6 button.bin", 0x0000, 0x008000, CRC(ee3de053) SHA1(e4f6901aadf918ec19f475c1df440a3f09142c01) )
ROM_END

ROM_START( pr_gnuc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nugget22.bin", 0x0000, 0x008000, CRC(b289a4d0) SHA1(d7b52602ce126f4b6ae2d6f842a7bef26400f96f) )
ROM_END

ROM_START( pr_gnuca )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nugget12.bin", 0x0000, 0x008000, CRC(dc7411b2) SHA1(fb0d2ab936965b66aad8401c79ccadec225a60b2) )
ROM_END


ROM_START( pr_magln )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "magic lines v2-1.bin", 0x0000, 0x008000, CRC(b2619fac) SHA1(7a6603f538fde3ebda8025589bb562ee35a8a097) )
ROM_END

ROM_START( pr_maglna )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "magic lines v1-1.bin", 0x0000, 0x008000, CRC(ff7103d0) SHA1(9c926d8f97d3cd95bd721cfb751f583ef610c6e2) )
ROM_END

ROM_START( pr_swop )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "280a30pn.090", 0x0000, 0x010000, CRC(39ebee65) SHA1(45d97ac8f4404f43be8ed8f070121434f402a5be) )
ROM_END


ROM_START( pr_lday )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "lday72c", 0x0000, 0x010000, CRC(eec97b47) SHA1(ff9bb493146779883587457c9cf1f36f7f6a734a) )
ROM_END

ROM_START( pr_ldaya )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "lday72t", 0x0000, 0x010000, CRC(ac12e1ed) SHA1(2be3fff875958b9699fc07c830b7e76f0aeea0e7) )
ROM_END

ROM_START( pr_5xcsh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "5 x_cash_v24a_15bingo", 0x0000, 0x010000, CRC(65949855) SHA1(50ea765a45ae6e28ccc71e46b9ee8d9e81614420) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "5x_cash_235sound1", 0x0000, 0x020000, CRC(fad44418) SHA1(af41e6eb07f1b0665c7cd6d1a31b168532f5dbff) )
ROM_END

ROM_START( pr_7hvn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "7 heaven v170 (27256)", 0x0000, 0x008000, CRC(6605c956) SHA1(ffa663dc9a5e6d9138aeef9f65c8d5ee4bd3a308) )
ROM_END


ROM_START( pr_7hvna )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777heaven10pd", 0x0000, 0x008000, CRC(9532dc9a) SHA1(6e8f763e523a5e1ec72665c11518350d533bec39) )//10GBP
ROM_END

ROM_START( pr_7hvnb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777h20p6_72.bin", 0x0000, 0x008000, CRC(c8b3f54d) SHA1(5e6f734970009131fc4ed0623854993baec08555) )//20p 6GBP 72%
ROM_END

ROM_START( pr_7hvnc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "7h10p3c_380.bin", 0x0000, 0x010000, CRC(9fafd2a1) SHA1(7da1fa02005e0e232b7bb01088b180c32f271b00) )//10p 3 pound cash rom 380
ROM_END

ROM_START( pr_7hvnd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "7775prb", 0x0000, 0x010000, CRC(cc6cb41e) SHA1(7b9e4d9cfdb4071c83081a1306ffa7e0be3938aa) )//5p 3 pound cash 105
ROM_END

ROM_START( pr_7hvne )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven 892a20pn-390 (27256)", 0x0000, 0x008000, CRC(ffd1a8e0) SHA1(4b3a72c855d5b28d8be5db13166c58a44a1cbb59) )
ROM_END

ROM_START( pr_7hvnf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven 892a10pn-380 (27256)", 0x0000, 0x008000, CRC(ca9d2108) SHA1(ebbb54c6640c67ce905a2dbf83f9f2fde31d342f) )
ROM_END

ROM_START( pr_7hvng )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven 892a10pn-370", 0x0000, 0x008000, CRC(26c875df) SHA1(fd5fbadd1d8455f855c60213c54e9976d4763227) )
ROM_END

ROM_START( pr_7hvnh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven 892a10po-340", 0x0000, 0x008000, CRC(ddd7c3e4) SHA1(853a3d41492392470d619701987bdb95ae01acdc) )
ROM_END

ROM_START( pr_7hvni )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 v23-0 (27256)", 0x0000, 0x008000, CRC(68c01ea5) SHA1(744346bedc54cda397f3974b93f932f1ffec4411) )
ROM_END

ROM_START( pr_7hvnj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 v110 (27256)", 0x0000, 0x008000, CRC(2c7966a4) SHA1(67b10adf1440fd31e94c88b61f341734b381ca3f) )
ROM_END

ROM_START( pr_7hvnk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777heaven5p(27256)", 0x0000, 0x008000, CRC(d5a0a06d) SHA1(e6209d406319617d7b1462f788a4e68fa7142cde) )
ROM_END

ROM_START( pr_7hvnl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777620p", 0x0000, 0x008000, CRC(83f3f72e) SHA1(ffa8a63bd81b5d316d21b3834939318a4079e024) )
ROM_END

ROM_START( pr_7hvnm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777hea5p", 0x0000, 0x008000, CRC(cbbccb11) SHA1(3ed9bc244bafdb059c2d7d0303cc3483a9f12d62) )
ROM_END

ROM_START( pr_7hvnn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777heav1020p.bin", 0x0000, 0x010000, CRC(a885298e) SHA1(eb378af28562a028d388cba5ead98644d5c9532f) )
ROM_END

ROM_START( pr_7hvno )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven 8t 5c 10p (27512)", 0x0000, 0x010000, CRC(dadbb559) SHA1(f8bb6579446548f1d0519555eabfa0c076885832) )
ROM_END

ROM_START( pr_7hvnp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 10 (27512)", 0x0000, 0x010000, CRC(91d67978) SHA1(de57a4095c814d6c396b3fa80f66fc3eb912d42f) )
ROM_END

ROM_START( pr_7hvnq )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 8t 4c 20p (27512)", 0x0000, 0x010000, CRC(7cadbd6c) SHA1(fb0ec1b8e43e772b2fd2b71b82fafc6c06d4d3a4) )
ROM_END

ROM_START( pr_7hvnr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 8t 4c 5p (27512)", 0x0000, 0x010000, CRC(b813bec1) SHA1(3926b9cd9f452f3291ee26c14809b0a717c794b9) )
ROM_END

ROM_START( pr_7hvns )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "777 heaven pc90 8t 5c 5p (27512)", 0x0000, 0x010000, CRC(47214e06) SHA1(318f7d9891e7d37e2956c462bd04137af5bf972b) )
ROM_END

ROM_START( pr_7hvnt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "77h5pcbs", 0x0000, 0x010000, CRC(15a61bbe) SHA1(3c98f43c6f229da9a3fc334568cdce4d39bd6563) )
ROM_END

ROM_START( pr_7hvnu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cl7h1020", 0x0000, 0x010000, CRC(b12e3219) SHA1(634f6984d9ef6ef9964841b5586143d82d8b52f7) )
ROM_END


ROM_START( pr_alwy9 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "always 9 10p.bin", 0x0000, 0x010000, CRC(aff337c0) SHA1(a335a857c1bba696150d51cabfd39a2996c1092f) )
ROM_END

ROM_START( pr_alwy9a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "always9.bin", 0x0000, 0x010000, CRC(6372d4d6) SHA1(7e767fe531c50e6d8f810193d7cdb55703114914) )
ROM_END

ROM_START( pr_barbl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bars and bells.bin", 0x0000, 0x010000, CRC(a5d65329) SHA1(e28954a4555b8d7f0781cb26f2a10390e142af34) )
ROM_END

#define pr_batls_sound \
	ROM_REGION( 0x80000, "snd", 0 )\
	ROM_LOAD( "080snd1.bin", 0x0000, 0x020000, CRC(011170ab) SHA1(60a174c09261c2ee230c4194d918173b41f267de) )\
	ROM_LOAD( "080snd2.bin", 0x020000, 0x020000, CRC(9189793b) SHA1(b47a3c214eb01595581f1e9d18c154560ee02ca1) )
ROM_START( pr_batls )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "batt72c", 0x0000, 0x010000, CRC(6c6ab1f1) SHA1(6e0663fcdfa1948d9d74b6df388d09fed73f0ed2) )

	pr_batls_sound
ROM_END

ROM_START( pr_batlsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "batt72t", 0x0000, 0x010000, CRC(e823e506) SHA1(6de98a585546b4005ef547a37dc27542df0fef0c) )

	pr_batls_sound
ROM_END

ROM_START( pr_batlsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "battleships5p10p20p8.bin", 0x0000, 0x010000, CRC(3a1491a9) SHA1(c3fbffb66ac8d576edf17027dfa360089d8eb14c) )

	pr_batls_sound
ROM_END

ROM_START( pr_btwar )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "beatthewarden.bin", 0x0000, 0x010000, CRC(d2bcb356) SHA1(d2c3230395ca5c7e713bb7122db33b66ad83eb2c) )
ROM_END

ROM_START( pr_btwara )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btw72c", 0x0000, 0x010000, CRC(12e4eceb) SHA1(9be20b54f3d1edba3a21a89dbb22c33f52362870) )
ROM_END

ROM_START( pr_btwarb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btw72t", 0x0000, 0x010000, CRC(96adb81c) SHA1(d43925b6cc01ddd9050bd023902f7ec37e1fbf52) )
ROM_END


ROM_START( pr_bigdp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "962a206n.368", 0x0000, 0x010000, CRC(a59a22c9) SHA1(d1500dfab69ec3e19dbb709122a55227cfef05a1) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "962snd1.050", 0x000000, 0x020000, CRC(618003a0) SHA1(01d5ab57dbe481729ef5dccca53c458eadaf042e) )
	ROM_LOAD( "962snd2.050", 0x020000, 0x020000, CRC(510dd26d) SHA1(02f1c9b10d08da838120457cac02f552e162c056) )
ROM_END

ROM_START( pr_bigdpa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "big dipper ndp.bin", 0x0000, 0x010000, CRC(23f483c6) SHA1(acc45b30aaa22c35f6cfe44bf92997c8ac1a3cff) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "962snd1.050", 0x000000, 0x020000, CRC(618003a0) SHA1(01d5ab57dbe481729ef5dccca53c458eadaf042e) )
	ROM_LOAD( "962snd2.050", 0x020000, 0x020000, CRC(510dd26d) SHA1(02f1c9b10d08da838120457cac02f552e162c056) )
ROM_END

//first rom == bullseyesnd
#define pr_bulls_sound \
	ROM_REGION( 0x80000, "snd", 0 )\
	ROM_LOAD( "010snd1 316e.bin" , 0x0000, 0x020000, CRC(5e3cfdc6) SHA1(32db10e7bacc6a4728d8821e77789cf146e2a277) )\
	ROM_LOAD( "010snd2 9e9a.bin" , 0x0000, 0x020000, CRC(496a9d51) SHA1(703631d3cb2e3c7fa676cb5a31903bf39ee6c44f) )

ROM_START( pr_bulls )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "beye58c", 0x0000, 0x010000, CRC(5fc52311) SHA1(dd4d3f29ec608c37d3d644bfbfa3ba06cc134b59) )

	ROM_REGION( 0x80000, "unknown", 0 ) // don't know what this is, doesn't look the same
	ROM_LOAD( "bb10p1", 0x0000, 0x020000, CRC(afe7b7bb) SHA1(dcf65e2113354171dc8858c6bfd8b97955b470a0) )

	pr_bulls_sound
ROM_END

ROM_START( pr_bullsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bullseye std 6 tok 6de7.bin", 0x0000, 0x010000, CRC(c3b6e153) SHA1(03a3a4e781f1b9029bfbe976a8223df1bacb0f67) )

	pr_bulls_sound
ROM_END

ROM_START( pr_bullsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bullseye10", 0x0000, 0x010000, CRC(de43085c) SHA1(bdd1e22545680c9a2cc3cb4244a4887cb0f5519e) )

	pr_bulls_sound
ROM_END

ROM_START( pr_bulbn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bbn72c", 0x0000, 0x010000, CRC(488d8f4f) SHA1(1168763072b6a89d9c4e315474fb74be3018c530) )
ROM_END

ROM_START( pr_bulbna )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bbn72t", 0x0000, 0x010000, CRC(38659c4c) SHA1(e27f7ae532cc4a0cee3c40b8c9bae6c10e52e45d) )
ROM_END

ROM_START( pr_bulbnb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bully5sw", 0x0000, 0x010000, CRC(b85c2f32) SHA1(c600e81930dba642f226a2a0e06a017629745abf) )
ROM_END

ROM_START( pr_buljp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "121a20pn.038", 0x0000, 0x010000, CRC(0cbce459) SHA1(2e37528991ec465f6fcb6540d76fe422f74b2973) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "076snd1.010", 0x0000, 0x020000, CRC(a33f9778) SHA1(a3961e1058037971d4f0ddea23be0a6715834e46) )
	ROM_LOAD( "076snd2.010", 0x0000, 0x020000, CRC(702f3977) SHA1(9612f25abb51693f8e4ff52bc193d1d18e7d774a) )
ROM_END

ROM_START( pr_buljpa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bjp58c", 0x0000, 0x010000, CRC(d999d635) SHA1(875244e44ecbe8618d676ecaae9f260073b326f0) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "076snd1.010", 0x0000, 0x020000, CRC(a33f9778) SHA1(a3961e1058037971d4f0ddea23be0a6715834e46) )
	ROM_LOAD( "076snd2.010", 0x0000, 0x020000, CRC(702f3977) SHA1(9612f25abb51693f8e4ff52bc193d1d18e7d774a) )
ROM_END

ROM_START( pr_cashb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "311a30pn.120", 0x0000, 0x010000, CRC(b2572ea7) SHA1(ea3d650752715420d2fcac56e4bcea911d632cf5) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "235snd.003", 0x0000, 0x020000, CRC(71c32280) SHA1(3f6e44b9b43515e08db7266c52b94c5b0a2a7d17) )
ROM_END

ROM_START( pr_cas7 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "244a30pn-140_15", 0x0000, 0x010000, CRC(21f26490) SHA1(4b616fce289892264aa9e7c5521e481b2ae288d7) )
ROM_END

#define pr_chico_sound\
	ROM_REGION( 0x80000, "snd", 0 )\
	ROM_LOAD( "006snd1.000", 0x0000, 0x020000, CRC(f906857a) SHA1(cfa47a3e887e6788c577d31bb567f7fbaaf0bbf3) )\
	ROM_LOAD( "006snd2.000", 0x0000, 0x020000, CRC(8445d0ef) SHA1(fd2ecc96f74e99e8f5ea7c7772fbd451ce52e889) )
ROM_START( pr_chico )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006a20pn.810", 0x0000, 0x010000, CRC(c4b491b7) SHA1(7629857cf6f1f69fb9ccf82a290a491cf695d373) )

	pr_chico_sound
ROM_END

ROM_START( pr_chicoa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006a25pn.810", 0x0000, 0x010000, CRC(d765215c) SHA1(22fe8d4b387e9fbc34a72ea1583b1c040f7761bc) )

	pr_chico_sound
ROM_END

ROM_START( pr_chicob )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chic8c", 0x0000, 0x010000, CRC(2d5628be) SHA1(0c3f60f72a4d4eb458cac8ca4fe7d5584cbe4f18) )

	pr_chico_sound
ROM_END


ROM_START( pr_coolm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cm5p5ro", 0x0000, 0x010000, CRC(5a784345) SHA1(fbfd550e61ca3a683a66f372e37c9d21a43a59e2) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "cmsnd.bin", 0x0000, 0x080000, CRC(db79a326) SHA1(afe19eaa5d63c871ed80ffdfc8fb991a8c16cb36) )
ROM_END

ROM_START( pr_coolma )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cm5pndan", 0x0000, 0x010000, CRC(e0697fe6) SHA1(d4b66272030dd64c86834f0546376dd02cfb0cc4) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "cmsnd.bin", 0x0000, 0x080000, CRC(db79a326) SHA1(afe19eaa5d63c871ed80ffdfc8fb991a8c16cb36) )
ROM_END

ROM_START( pr_coolmb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cmil5.5r", 0x0000, 0x010000, CRC(c9bfec79) SHA1(a2856caf6052b1e05dd1de3af5bf0da0b4b7098f) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "cmsnd.bin", 0x0000, 0x080000, CRC(db79a326) SHA1(afe19eaa5d63c871ed80ffdfc8fb991a8c16cb36) )
ROM_END

ROM_START( pr_crz77 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crazy 777", 0x0000, 0x010000, CRC(c77e0b92) SHA1(7558fda85cb68ef0e1b183ce9e4824f6968a931e) )
ROM_END

ROM_START( pr_crzbr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crazybars.bin", 0x0000, 0x010000, CRC(896ad5d3) SHA1(6df42dc8016d50239f9467af016e3a4224065599) )

ROM_END

// 'PCP' Super Bars seems to be the same thing
ROM_START( pr_supbr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "s bars 052a20p0.050 procon.bin", 0x0000, 0x010000, CRC(037a6a82) SHA1(6f804e80e529293f9de540623cd4e4c5a4fd1022) )
ROM_END

ROM_START( pr_coyot )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crazycoyotev810ptube.bin", 0x0000, 0x010000, CRC(9903a0b8) SHA1(124dc1d8de8c0384a42fbf347aefd193706dcc1a) )

ROM_END

ROM_START( pr_coyota )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crazycoyotev820ptube.bin", 0x0000, 0x010000, CRC(fc251903) SHA1(93d6e49c40420f7d645d392bcc48839f977bd87c) )
ROM_END

ROM_START( pr_crzpy )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cpay10", 0x0000, 0x010000, CRC(bfab234c) SHA1(fb20e3270dfd9572aec0d7dec4e8c3d8d1c18a10) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "cpay20p6", 0x0000, 0x010000, CRC(a73e7c0d) SHA1(c652311836312f15ff6f4763a8522d190d38ff09) )
	ROM_LOAD( "cpays(27512)", 0x0000, 0x010000, CRC(e86015b0) SHA1(b773010cc022fb76874a32362b4d46a53e9e37cb) )
	ROM_LOAD( "cpays4-80(27512)", 0x0000, 0x010000, CRC(bff4f0c6) SHA1(5fececef33b9b21323a511cec65233874141a315) )
	ROM_LOAD( "cpaysgala(27512)", 0x0000, 0x010000, CRC(e667b50a) SHA1(26106df5d61079f285f2c4ea224f120551ae00dd) )
	ROM_LOAD( "crazypays20p4-80ac.bin", 0x0000, 0x010000, CRC(0aeea436) SHA1(505c52b0ed04392cbf4fce694cffb72bbb88046a) )
ROM_END

ROM_START( pr_dblup )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "896a10po.050", 0x0000, 0x008000, CRC(b562bbc5) SHA1(439007dfb4153d3ed635738e82647c0b6ee13cb9) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "double up 5p.bin", 0x0000, 0x008000, CRC(bd7802ad) SHA1(0d4a2f91a687bd38dcae9ffaf07f2bbd4c2690a7) )
ROM_END

ROM_START( pr_fire )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "firecracker20p-8token.bin", 0x0000, 0x010000, CRC(ccda1584) SHA1(89dc06d7811adb1d9b16442078e57ec8991ee5ab) )
ROM_END

ROM_START( pr_flshc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fcash58c", 0x0000, 0x010000, CRC(3a748e76) SHA1(fd85bff7841f4990f5aedcbad03476b15939db45) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "163a25pn.930", 0x0000, 0x010000, CRC(36c6f14c) SHA1(7608bc179a5505f73782eb87a389bff691f56ad4) ) // 'The Joker'

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "163snd1", 0x000000, 0x020000, CRC(b80f6395) SHA1(48ea3714fb456df8cf20afe29363a4d4c2108079) )
	ROM_LOAD( "163snd2", 0x020000, 0x020000, CRC(c8486cd2) SHA1(39f90e2fb5fcd84d6455d65fbc305432e55c0e5b) )
ROM_END

ROM_START( pr_ftwhl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "2p-5pfortunewheel.bin", 0x0000, 0x010000, CRC(f47960bd) SHA1(de9e13f9607789deaff37d4d74ec924b1273e682) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "943a20pn.250", 0x0000, 0x010000, CRC(a6f86f67) SHA1(43b96e2866099af6693aaf03313e119474180934) )
	ROM_LOAD( "f-wheel.bin", 0x0000, 0x010000, CRC(ecce8953) SHA1(d90a203e3009be73d456a1f028ffe88754175514) )
	ROM_LOAD( "fortunewheel6.bin", 0x0000, 0x010000, CRC(fa95bf1b) SHA1(e22b6979f01ff545c47fdc58600a42b78ecea731) )
	ROM_LOAD( "943a206n.258", 0x0000, 0x010000, CRC(3ef23263) SHA1(40fb4399e4ac34fcb52aaca6ec19a38723bc8031) )//6GBP

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "943snd1.000", 0x000000, 0x020000, CRC(6b4223a7) SHA1(7af5779ef0309fef40b930f522e962708bd25930) )
	ROM_LOAD( "943snd2.000", 0x020000, 0x020000, CRC(4e4488a7) SHA1(d29e02d430b0afffea39b6c23afc292551151001) )
ROM_END

ROM_START( pr_funrn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "012a20pn.810", 0x0000, 0x010000, CRC(c9c3c6f1) SHA1(4cf41f172e785c861359dc25e6d500d9be63aa9f) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "fun10", 0x0000, 0x010000, CRC(9426c031) SHA1(6ee731db298e05f56bed20a2cd7807b8a00abfe4) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "012snd1.000", 0x000000, 0x020000, CRC(d7424290) SHA1(5eb908a8903405868064fc9f898260a1a7db3e0f) )
	ROM_LOAD( "012snd2.000", 0x020000, 0x020000, CRC(5f9355f2) SHA1(75d8452106b17ad9c4bce75b07246e10f6e06f55) )
ROM_END

ROM_START( pr_gogld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "079a20pn.150", 0x0000, 0x010000, CRC(4b4330d9) SHA1(3932a080ecfae1a28540efbc533b393cad0da7c6) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "goforgold.bin", 0x0000, 0x010000, CRC(873aa85b) SHA1(68854aea536fb818da0f0eeb4ca0f5ec493f885d) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "079snd1.000", 0x000000, 0x020000, CRC(d4b07715) SHA1(65b04cbc797a6878eb807d8e602724d6824dbaa7) )
	ROM_LOAD( "079snd2.000", 0x020000, 0x020000, CRC(a4c73a67) SHA1(961ae3b24a5201abf4e427c753eb23dcea13780f) )
ROM_END

ROM_START( pr_gldnl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "golden_nile_v10.bin", 0x0000, 0x010000, CRC(bf77d3ff) SHA1(e5006d2e29f64926165d31a5e91ae996df586425) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "golden_nile_v14.bin", 0x0000, 0x010000, CRC(63162079) SHA1(625f1f117724f031d8bc9188d5474c68fb7acde8) )
ROM_END

ROM_START( pr_gldng )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "311a30pn.120", 0x0000, 0x010000, CRC(13b2d335) SHA1(5bb26516f527a2b7a5ae9d2b90ae1a8551f09925) )

	ROM_REGION( 0x80000, "altrevs", 0 ) // not sure these are the same game..
	ROM_LOAD( "golden n v1.1 4-8.bin", 0x0000, 0x008000, CRC(9448ecaf) SHA1(d43f529a94b09f13aa544279b6036f40f620c51f) )
	ROM_LOAD( "golden n v2.2 4-8.bin", 0x0000, 0x008000, CRC(fab559cd) SHA1(884e5f2a8ae386eee5d5e5c18a6e8d2da1449c8f) )
	ROM_LOAD( "golden n v3-1.bin", 0x0000, 0x008000, CRC(da302ec7) SHA1(c1dd565f288f7ed55f88e0460739811ce95204dd) )
	ROM_LOAD( "golden n v4-1.bin", 0x0000, 0x008000, CRC(57346840) SHA1(8eb014968d65326e78a5fc3a6379936db78daf92) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "235snd.003", 0x0000, 0x020000, CRC(71c32280) SHA1(3f6e44b9b43515e08db7266c52b94c5b0a2a7d17) )
ROM_END

ROM_START( pr_gdft )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "249a05pn-010 (27512)", 0x0000, 0x010000, CRC(99c99d02) SHA1(50e4be53e07de1ace47593a112628f1f76576313) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "249 sound 1-000 (27c040)", 0x0000, 0x080000, CRC(d4b5390f) SHA1(9efee17e0c343e286d3c6eddef85d641664d039f) )
ROM_END

ROM_START( pr_happy )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "058a20pn.130", 0x0000, 0x010000, CRC(6d628f38) SHA1(e1a3af6147bc37a97486f10a013140c2fabbb7a7) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "058snd1.000", 0x000000, 0x020000, CRC(87d78979) SHA1(1aa0d8dc6b8defa34fca09765128aeb993337859) )
	ROM_LOAD( "058snd2.000", 0x020000, 0x020000, CRC(f8ef3014) SHA1(d1d49b39054721615f2f9ebc1cfb4318ce31440c) )
ROM_END

ROM_START( pr_heato )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032a20pn.500", 0x0000, 0x010000, CRC(87e1d114) SHA1(0f863d9c21be2907b70f6490f8272a75e92c4a44) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "heat10", 0x0000, 0x010000, CRC(e9478967) SHA1(3bfcdb66947682a86394e39cbaff2d9971da278b) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "032snd1.000", 0x000000, 0x020000, CRC(2424dadf) SHA1(f98e6972c63c8211948810312c6fba94e7fc64af) )
	ROM_LOAD( "032snd2.000", 0x020000, 0x020000, CRC(f91e13c5) SHA1(595b1807d4d33f7fa40bb0021848275f014d6ddc) )
ROM_END

ROM_START( pr_hiclm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hi-climber 7 button v2.2.bin", 0x0000, 0x008000, CRC(589fdc5f) SHA1(bb6b4921eb034fcd4ec26aafcdb2366b35ef83f7) )
ROM_END

ROM_START( pr_hit6 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hit the six (27512)", 0x0000, 0x010000, CRC(6d4c2139) SHA1(8ee1316e644590d679646f52b7816a9634265737) )
ROM_END

ROM_START( pr_hit6a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hitthesix.bin", 0x0000, 0x010000, CRC(993990ff) SHA1(9ac16aaf52599a310498eb0ba6f40a763b01dc59) )
ROM_END

ROM_START( pr_hit6b )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ht6902a10pn_300_ds_8515.bin", 0x0000, 0x010000, CRC(c212be54) SHA1(da28eea13fb04ae9fe93639df7566aaa44edcb86) )
ROM_END

ROM_START( pr_hotcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "htcash55", 0x0000, 0x010000, CRC(d422ac0e) SHA1(57135b606eb0a3fc67385f5759e7a21f7182705a) )
ROM_END

ROM_START( pr_hotsp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hot spots 5p.bin", 0x0000, 0x008000, CRC(456ba6bf) SHA1(b28d6fe63650e6ce17c279fdb217ce4e9cdd4f2f) )
ROM_END

ROM_START( pr_jkpt7 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "j77754.bin", 0x0000, 0x008000, CRC(c8b72a0e) SHA1(b5a93afbf881a98d9b605a5d1b4c5b34735f01d2) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "jackpot 7 10 (27512)", 0x0000, 0x010000, CRC(4230fa35) SHA1(af4bcebf934d2674f9c364a93cb6142cb3d75856) )
	ROM_LOAD( "jackpot 7 5p.bin", 0x0000, 0x010000, CRC(84e5803c) SHA1(4c4226a18b9fb60ccf9a6fda92e86983eb5bd95c) )
	ROM_LOAD( "jackpot7s4-80.bin", 0x0000, 0x010000, CRC(8bb8b893) SHA1(ab02aa826b4741196a355299995545bf3617120c) )
	ROM_LOAD( "jp7.bin", 0x0000, 0x008000, CRC(8b54a58d) SHA1(1663463eb0d3837eb3e2c2baf71c93c776892e11) )
	ROM_LOAD( "jpot7-10-0a-b.bin", 0x0000, 0x008000, CRC(bd9f6cea) SHA1(4b7e2a12efac325124bc5d3ce76bf68b77d5b9c5) )
ROM_END

ROM_START( pr_jkrwd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jokers wild 5p 256.bin", 0x0000, 0x008000, CRC(10dc7dc7) SHA1(a0cc4a880a828742d5bce3c4dfb89797f47f095b) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "jokerswild.bin", 0x0000, 0x008000, CRC(35c241e5) SHA1(23a639a36711127bae8b0ce68faf396938071006) )
	ROM_LOAD( "jokerswild220.bin", 0x0000, 0x008000, CRC(24e1ce4d) SHA1(cc606f9abfbaeb4aeeebc2b5acbe66a5e229318e) )
	ROM_LOAD( "jokerswildprocon10p(27256)", 0x0000, 0x008000, CRC(3c2d9c46) SHA1(6d21c3dfa5df21dc991d3117a516dc4255fffab4) )
	ROM_LOAD( "jw 5p mk2.bin", 0x0000, 0x008000, CRC(344150e9) SHA1(a92313956536ff5772deee25209d2e580be64214) )
	ROM_LOAD( "jwild 10m.bin", 0x0000, 0x008000, CRC(bde648bd) SHA1(523ad5887cf61affac0773df4a1d912a2b38f34c) )
	ROM_LOAD( "jwild54.bin", 0x0000, 0x008000, CRC(085c1d29) SHA1(393f7c5f78314bd2dadef0d7f1e5bce3f69188af) )
ROM_END

ROM_START( pr_jumpj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "j jacks 994a20pn-040 (27512)", 0x0000, 0x010000, CRC(e6ffe171) SHA1(6fdb413621dd6e3f4185ff1f33af2f52b530a0a8) )

ROM_END

ROM_START( pr_jumpja )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jumping.bin", 0x0000, 0x010000, CRC(3d11a584) SHA1(580450b8bf3adc62ff71e2c3b5b086e702e3345f) )
ROM_END

ROM_START( pr_medl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "medalist_269", 0x0000, 0x010000, CRC(a887f0f1) SHA1(032af1e9cb624bf4ab9def9cd17a2ad70b10ead7) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "medalist_sound", 0x0000, 0x020000, CRC(71c32280) SHA1(3f6e44b9b43515e08db7266c52b94c5b0a2a7d17) )
ROM_END

ROM_START( pr_megmn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mega mon 5p 3.bin", 0x0000, 0x010000, CRC(a26e041c) SHA1(a0f31e7aff8beb19c8dd8712183a60f08e55cf6b) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "mega money.bin", 0x0000, 0x010000, CRC(16b42417) SHA1(dfa49f973dfeda40c6d2a26a26f7e588832c23c3) )
	ROM_LOAD( "mm05.bin", 0x0000, 0x008000, CRC(93b9b992) SHA1(7a93444d004f01b9205f3105e4ea06e48e6f5ec4) )
	ROM_LOAD( "954p206n.066 (labelled - mm pro).bin", 0x0000, 0x008000, CRC(9162f89b) SHA1(8134eaff3211a2423794875ebdb25c5412b54e96) )
ROM_END

ROM_START( pr_nudxs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "923a206n.110", 0x0000, 0x010000, CRC(7b21f7ee) SHA1(e7970363de3c954f53556d027382955e1fcc3cd1) )
ROM_END

ROM_START( pr_rags )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "937a206n.130", 0x0000, 0x010000, CRC(2fba55ef) SHA1(f529b9475fcd94adca9f93adaefe661fd1495d1c) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "937snd1.000", 0x000000, 0x020000, CRC(7dce6c5b) SHA1(2c52296a0fec3275dfa9b86014a23d7427f78fb8) )
	ROM_LOAD( "937snd2.000", 0x020000, 0x020000, CRC(17019d23) SHA1(a8b307a550ff0cd49cc7a04fc834c6b259cc56da) )
ROM_END

ROM_START( pr_reflx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "reflex 5p.bin", 0x0000, 0x010000, CRC(172d00b8) SHA1(b0eb70d273664428994c4bfff4ccdfb023a842ee) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "reflex pc90 v0-36 (27256)", 0x0000, 0x008000, CRC(25eca9b0) SHA1(32ee614a2c82d872346633104d221977fec390a5) )//reflex_4_80_20p.bin
	ROM_LOAD( "reflex_5p (27512)", 0x0000, 0x010000, CRC(0788e2cc) SHA1(1ff6f517536e6394a92efdea0a90f8013871dab1) )
	ROM_LOAD( "reflexprocon5p(27256)", 0x0000, 0x008000, CRC(da5db154) SHA1(f549169bdcce0a556e3d65fbbe4f69b175a82a9a) )
	ROM_LOAD( "reflx54", 0x0000, 0x010000, CRC(f6668f0e) SHA1(8d033de303d5474588c8e2e55a066e79bb68668e) )
ROM_END

ROM_START( pr_roadr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "road riot sp v0.0 6.bin", 0x0000, 0x010000, CRC(39e0dcef) SHA1(ea41225529fee5a36ab13592bfaa00b780e0c279) )
ROM_END

ROM_START( pr_roll )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "roll.bin", 0x0000, 0x010000, CRC(7e092f85) SHA1(3c90d6a7189aae034fe23f3f916fbd575e0f10ab) )//5p5rd

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "roll10.bin", 0x0000, 0x010000, CRC(ae92129c) SHA1(438245533bb05e86a051d1fbd59802cb109f6a0a) )
	ROM_LOAD( "roll20p8t.bin", 0x0000, 0x010000, CRC(cbfa85f0) SHA1(d15dbb8c7334f3a2758a1fe4e4a83247cfd282b5) )
	ROM_LOAD( "roll5p5.bin", 0x0000, 0x010000, CRC(4ab5991b) SHA1(10894908be7ae0fa05bafbff031348dd2acd9cb8) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "rollsnd1.bin", 0x000000, 0x020000, CRC(53dea2e5) SHA1(cbc275f4cd6f415b090298772188bca4ba4c2b32) )
	ROM_LOAD( "rollsnd2.bin", 0x020000, 0x020000, CRC(f6722f64) SHA1(df40547deb7bb307724345db8d4dbe14e33f5dad) )
ROM_END

ROM_START( pr_sevab )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "7's above s_s.bin", 0x0000, 0x010000, CRC(f88bdceb) SHA1(9111616c162990cb5b89e073f9791b15645f5d91) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "7sab.bin", 0x0000, 0x010000, CRC(f7381284) SHA1(244c8981ce5080168eba117abc905f0a5339711b) )
	ROM_LOAD( "7sabv.bin", 0x0000, 0x010000, CRC(2cd65671) SHA1(26051b37189d30997f2022686bd4dad562500ee7) )
	ROM_LOAD( "7sabove.bin", 0x0000, 0x010000, CRC(b16f278f) SHA1(786c1ea1b701489c655c35c0bc83f35e70a9fe39) )
	ROM_LOAD( "sevens_above", 0x0000, 0x010000, CRC(9cdfaa94) SHA1(a7063b4fc30a56f8b162ca8ef2651ada05758771) )
ROM_END

ROM_START( pr_sevml )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "903a20pn.128", 0x0000, 0x010000, CRC(a941fdbd) SHA1(500229a40e089b948b45cc6b419675b199610594) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "sevenmelonsprocon5p(27256)", 0x0000, 0x008000, CRC(ebee9be6) SHA1(efd713852213a8dcf2bc302195acffee3f60da10) ) // looks like a bad dump
	ROM_LOAD( "sevens&melons4-80(20ptube).bin", 0x0000, 0x010000, CRC(2d08a94a) SHA1(0e0bb7b830cfbf6696059f8af384880a8917dd9c) )
	ROM_LOAD( "sevensandmelons.bin", 0x0000, 0x010000, CRC(e9942539) SHA1(4e782a0506c734e87871bfee815da84dbc7f6edb) )
	ROM_LOAD( "7andmel.bin", 0x0000, 0x008000, CRC(109e6dff) SHA1(34f5b5d70d2607ef10698cee87fdd8c8267a0d5c) )
ROM_END

ROM_START( pr_theme )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "150p20pn.057", 0x0000, 0x010000, CRC(19bbace0) SHA1(8811947fa9885e336a905e13ab595209878a1a65) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "150p25pn", 0x0000, 0x010000, CRC(2e599cbb) SHA1(fafb31d7b2667608097c30762a3d4ea6c65217c3) )
	ROM_LOAD( "thp55v2", 0x0000, 0x010000, CRC(f616e701) SHA1(466b22a8620a41e343785b9005d9837332322e20) )
	ROM_LOAD( "thp5p5ro", 0x0000, 0x010000, CRC(75441605) SHA1(bd36c193a8c8d46805e2f18a4b652331ee652c7f) )
	ROM_LOAD( "tprkd", 0x0000, 0x010000, CRC(b5b98439) SHA1(d532f1ea3e847ec641519c88046abd4c5b6189c6) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "150snd.000", 0x0000, 0x080000, CRC(a8f0a2f5) SHA1(2b957f464bcd9f3016ea73e7f8ac7e357e7144ac) )
ROM_END

ROM_START( pr_ttrai )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "t-trail.bin", 0x0000, 0x010000, CRC(a509f698) SHA1(2c1e03231f795e1e8b28f3de151b070cb07e4734) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "treasuretrail.bin", 0x0000, 0x010000, CRC(2be4e149) SHA1(d8cd8f9196b7f2131c78e3e00384872d3ecc0f5f) )
	ROM_LOAD( "ttrail 10_93.bin", 0x0000, 0x010000, CRC(88f97fff) SHA1(e6954030e13cc069e561f3a4729eca077ca8df20) )
	ROM_LOAD( "bifi-projectcoin.bin", 0x0000, 0x010000, CRC(463dd269) SHA1(da647f5942491f574aec17e6fdd99f75641ff332) )
ROM_END

ROM_START( pr_trpx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "triplex", 0x0000, 0x010000, CRC(5d80421d) SHA1(90b3336d185d7af67cca0ea30e574b1abd1db385) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "triplexsnd", 0x0000, 0x020000, CRC(fad44418) SHA1(af41e6eb07f1b0665c7cd6d1a31b168532f5dbff) )
ROM_END

ROM_START( pr_walls )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "050p20pn.005", 0x0000, 0x010000, CRC(17ea0895) SHA1(0f146c018421d8ef2b346fb219f92fb2df935391) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "wall72c", 0x0000, 0x010000, CRC(cf12cf28) SHA1(a6e76b39f3db4ec125480bf43f4cb4f8c2c07f2b) )
	ROM_LOAD( "wall72t", 0x0000, 0x010000, CRC(d8c03fb2) SHA1(586308780d75b2942eb757e3814f6be067d363d7) )
	ROM_LOAD( "wallstreet.bin", 0x0000, 0x010000, CRC(32e77493) SHA1(80237fe57051695a6a0503136b995c9e7983db4c) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "050snd1.000", 0x0000, 0x020000, CRC(58b58f6d) SHA1(b71bc925e9dd073bd034cece43e90061605824c8) )
	ROM_LOAD( "050snd2.000", 0x0000, 0x020000, CRC(84392f6c) SHA1(2bdadc5a37bce502b2b6de387f8f260213849bd4) )
ROM_END

ROM_START( pr_whlft )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wof.bin", 0x0000, 0x010000, CRC(67f0e4ff) SHA1(dbabb3ffd057de8dce24e5b88dc399ac4a1564f7) )
ROM_END

ROM_START( pr_wnstk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cwrldwnstrk_v1_1.bin", 0x0000, 0x008000, CRC(5d701d88) SHA1(3e57faba4d549c00593736158840b80aed732451) )
ROM_END

ROM_START( pr_wldkn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wild_kings_421", 0x0000, 0x010000, CRC(7c9b373e) SHA1(d73fa15189b3dd79ea1df4de56d0df134d9c80ee) )

	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "wk_sound", 0x0000, 0x020000, CRC(71c32280) SHA1(3f6e44b9b43515e08db7266c52b94c5b0a2a7d17) )
ROM_END

ROM_START( pr_nifty )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "391a30pn.130", 0x0000, 0x010000, CRC(76253ec1) SHA1(56bc49fe7e4e33dda25533e31586cbf0653ee91a) )
	ROM_REGION( 0x80000, "snd", 0 )
	ROM_LOAD( "nf50snd.000", 0x0000, 0x020000, CRC(4ed90aab) SHA1(6f8a4afe3e4717563272ce8575e64d1b0144a1c8) )
ROM_END

ROM_START( pr_upnun )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "up&underprojectcoin.bin", 0x0000, 0x010000, CRC(053a394f) SHA1(8d7e55092dfba2ce49ee009ed388be027be2ff28) )
ROM_END

ROM_START( pr_qksht )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "quickshot27256.bin", 0x0000, 0x008000, CRC(44188ee9) SHA1(a48807252a3fe3aeacbf2e6d2691bdeafc90e249) ) // Should be 10000?
ROM_END


ROM_START( pr_sptb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "simplythebest091a20pn.bin", 0x0000, 0x010000, CRC(8402d11f) SHA1(bc10f29c546fda03e18238811956c56546fa8bef) ) // wrong hardware
ROM_END

ROM_START( pr_trktr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "305a30pn.121", 0x00000, 0x010000, CRC(8ed1467c) SHA1(572a84bfaa5a2cef49404425058b96e1e0102cca) )
ROM_END

ROM_START( pr_trktp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "305a30pn.990", 0x00000, 0x010000, CRC(5448e7d5) SHA1(81414083341364c011ab814a3f57d0831edb3036) )
ROM_END

DRIVER_INIT_MEMBER(proconn_state,proconn)
{
}

GAME( 199?, pr_lday         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "'L' Of A Day (Project) (Cash set) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_ldaya        ,pr_lday    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "'L' Of A Day (Project) (Token set) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_5xcsh        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "5x Cash (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvn         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvna        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (10GBP Jackpot) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnb        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (20p 6GBP Jackpot Version 114) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnc        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (10p 3GBP Jackpot Version 380) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnd        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (5p 3GBP Jackpot Version 105) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvne        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 6) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnf        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 7) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvng        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 8) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnh        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 9) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvni        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 10) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnj        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 11) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnk        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 12) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnl        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 13) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnm        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 14) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnn        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 15) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvno        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 16) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnp        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 17) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnq        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 18) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnr        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 19) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvns        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 20) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnt        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 21) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_7hvnu        ,pr_7hvn    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "777 Heaven (Project) (set 22) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, pr_alwy9        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"PCP"    , "Always Nine (Pcp) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_alwy9a       ,pr_alwy9   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"PCP"    , "Always Nine (Pcp) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_barbl        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bars & Bells (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_batls        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Battleships (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_batlsa       ,pr_batls   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Battleships (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_batlsb       ,pr_batls   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Battleships (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_btwar        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Beat The Warden (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_btwara       ,pr_btwar   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Beat The Warden (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_btwarb       ,pr_btwar   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Beat The Warden (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bigdp        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Big Dipper (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bigdpa       ,pr_bigdp   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Big Dipper (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bulls        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bullseye (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bullsa       ,pr_bulls   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bullseye (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bullsb       ,pr_bulls   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bullseye (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bulbn        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bully's Big Night (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bulbna       ,pr_bulbn   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bully's Big Night (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bulbnb       ,pr_bulbn   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bully's Big Night (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_buljp        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bully's Jackpot (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_buljpa       ,pr_buljp   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Bully's Jackpot (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_cashb        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Cash Back (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_cas7         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Casino Jackpot 7s (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_chico        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Chico the Bandit (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_chicoa       ,pr_chico   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Chico the Bandit (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_chicob       ,pr_chico   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Chico the Bandit (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_coolm        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Cool Million (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_coolma       ,pr_coolm   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Cool Million (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_coolmb       ,pr_coolm   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Cool Million (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_crz77        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Crazy 777s (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_crzbr        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Crazy Bars (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_supbr        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"PCP",     "Super Bars (PCP) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, pr_coyot        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"PCP"    , "Crazy Coyote (Pcp) (10p) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_coyota       ,pr_coyot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"PCP"    , "Crazy Coyote (Pcp) (20p) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, pr_crzpy        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Crazy Pays (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_dblup        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Double Up (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fire         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Fircecracker (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_flshc        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Flash The Cash (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_ftwhl        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Fortune Wheel (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_funrn        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Fun On The Run (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_gogld        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Go For Gold (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_gldnl        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Golden Nile (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_gldng        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Golden Nugget (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_gdft         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Good Fortune (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_happy        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Happy Days (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_heato        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "The Heat Is On (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hiclm        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hi Climber (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hit6         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hit The Six (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hit6a        ,pr_hit6    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hit The Six (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hit6b        ,pr_hit6    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hit The Six (Project) (set 3) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hotcs        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hot Cash (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_hotsp        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Hot Spots (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_jkpt7        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Jackpot 7's (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_jkrwd        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Jokers Wild (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_jumpj        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Jumping Jacks (Project) (set 1) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_jumpja       ,pr_jumpj   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Jumping Jacks (Project) (set 2) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_medl         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Medalist (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_megmn        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Mega Money (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_nudxs        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Nudge XS (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_qksht        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Maygay", "Quickshot (Maygay) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_rags         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Rags To Riches (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_reflx        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Reflex (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_roadr        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Road Riot (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_roll         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "The Roll (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_sevab        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Seven's Above (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_sevml        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Sevens & Melons (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_theme        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Theme Park (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_ttrai        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Treasure Trail (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_trpx         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Triple X (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1999, pr_trktr        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Trick or Treat (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1999, pr_trktp        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Trick or Treat (Protocol?) (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_walls        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Wall Street (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_whlft        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Wheel Of Fortune (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_wldkn        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Wild Kings (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_nifty        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Nifty Fifty (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_upnun        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Project", "Up & Under (Project) (PROCONN)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_sptb         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Pcp", "Simply the Best (Pcp) (PROCONN?)",MACHINE_IS_SKELETON_MECHANICAL ) // not 100% sure this belongs here

// Some of these are PC98 hardware.. I don't know how / if that differs
GAME( 199?, pr_bears        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear Streak (set 1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearsa       ,pr_bears   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear Streak (set 2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearsb       ,pr_bears   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear Streak (set 3) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearx        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 2.3) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxa       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 2.2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxb       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 1.3) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxc       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (20p set 1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxd       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (20p set 2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxe       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (10p set 1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxf       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (10p set 2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxg       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (10p set 3) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxh       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (10p set 4?) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxi       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (10p set 5) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxj       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (code 813) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxk       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (8GBP Token?) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxl       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 41) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxlp      ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 41, Protocol) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_bearxm       ,pr_bearx   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Bear X (Version 31) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, pr_fspot        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 4.1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspota       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 3.1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspotb       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 2.1, set 1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspotc       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 2.1, 20p stake, 82%) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspotd       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 2.1, 7 button) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspote       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 1.1, set 1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspotf       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 1.1, 20p stake, 82%) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_fspotg       ,pr_fspot   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Fun Spot (Version 1.1, 6 button) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, pr_gnuc         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Golden Nugget (Version 2.2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_gnuca        ,pr_gnuc    ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Golden Nugget (Version 1.2) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_magln        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Magic Lines (Version 2.1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_maglna       ,pr_magln   ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Magic Lines (Version 1.1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_wnstk        ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Coinworld", "Winning Streak (Version 1.1) (Coinworld)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, pr_swop         ,0          ,proconn    ,proconn    , proconn_state,proconn ,ROT0   ,"Ace", "Swop It (Ace)",MACHINE_IS_SKELETON_MECHANICAL )
