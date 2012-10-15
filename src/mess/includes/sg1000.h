#ifndef __SG1000__
#define __SG1000__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/printer.h"
#include "formats/sc3000_bit.h"
#include "machine/ctronics.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/upd765.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"
#include "crsshair.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"z80"
#define SN76489AN_TAG	"sn76489an"
#define UPD765_TAG		"upd765"
#define UPD8251_TAG		"upd8251"
#define UPD9255_TAG		"upd9255"
#define UPD9255_0_TAG	"upd9255_0"
#define UPD9255_1_TAG	"upd9255_1"
#define CENTRONICS_TAG	"centronics"
#define TMS9918A_TAG	"tms9918a"

#define IS_CARTRIDGE_TV_DRAW(ptr) \
	(!strncmp("annakmn", (const char *)&ptr[0x13b3], 7))

#define IS_CARTRIDGE_THE_CASTLE(ptr) \
	(!strncmp("ASCII 1986", (const char *)&ptr[0x1cc3], 10))

#define IS_CARTRIDGE_BASIC_LEVEL_III(ptr) \
	(!strncmp("SC-3000 BASIC Level 3 ver 1.0", (const char *)&ptr[0x6a20], 29))

#define IS_CARTRIDGE_MUSIC_EDITOR(ptr) \
	(!strncmp("PIANO", (const char *)&ptr[0x0841], 5))

INPUT_PORTS_EXTERN( sk1100 );
extern const i8255_interface ( sc3000_ppi_intf );
extern const cassette_interface sc3000_cassette_interface;

class sg1000_state : public driver_device
{
public:
	sg1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	void install_cartridge(UINT8 *ptr, int size);

	DECLARE_WRITE8_MEMBER( tvdraw_axis_w );
	DECLARE_READ8_MEMBER( tvdraw_status_r );
	DECLARE_READ8_MEMBER( tvdraw_data_r );
	DECLARE_READ8_MEMBER( joysel_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	/* keyboard state */
	UINT8 m_keylatch;

	/* TV Draw state */
	UINT8 m_tvdraw_data;
	TIMER_CALLBACK_MEMBER(lightgun_tick);
	DECLARE_WRITE_LINE_MEMBER(sg1000_vdp_interrupt);
};

class sc3000_state : public sg1000_state
{
public:
	sc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: sg1000_state(mconfig, type, tag),
		  m_cassette(*this, CASSETTE_TAG)
	{ }

	required_device<cassette_image_device> m_cassette;

	virtual void machine_start();

	void install_cartridge(UINT8 *ptr, int size);

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
};

class sf7000_state : public sc3000_state
{
public:
	sf7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: sc3000_state(mconfig, type, tag),
		  m_fdc(*this, UPD765_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_floppy0(*this, UPD765_TAG ":0:525hd")
	{ }

	required_device<upd765a_device> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<floppy_image_device> m_floppy0;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_WRITE_LINE_MEMBER(sf7000_fdc_index_callback);

	void fdc_intrq_w(bool state);

	/* floppy state */
	int m_fdc_irq;
	int m_fdc_index;
};

#endif
