#ifndef __SG1000__
#define __SG1000__

#include "machine/ram.h"
#include "video/tms9928a.h"
#include "imagedev/cassette.h"

#define SCREEN_TAG		"screen"
#define Z80_TAG			"z80"
#define SN76489A_TAG	"sn76489a"
#define UPD765_TAG		"upd765"
#define CASSETTE_TAG	"cassette"
#define UPD8251_TAG		"upd8251"
#define UPD9255_TAG		"upd9255"
#define UPD9255_0_TAG	"upd9255_0"
#define UPD9255_1_TAG	"upd9255_1"
#define CENTRONICS_TAG	"centronics"

#define IS_CARTRIDGE_TV_DRAW(ptr) \
	(!strncmp("annakmn", (const char *)&ptr[0x13b3], 7))

#define IS_CARTRIDGE_THE_CASTLE(ptr) \
	(!strncmp("ASCII 1986", (const char *)&ptr[0x1cc3], 10))

#define IS_CARTRIDGE_BASIC_LEVEL_III(ptr) \
	(!strncmp("SC-3000 BASIC Level 3 ver 1.0", (const char *)&ptr[0x6a20], 29))

#define IS_CARTRIDGE_MUSIC_EDITOR(ptr) \
	(!strncmp("PIANO", (const char *)&ptr[0x0841], 5))

extern const TMS9928a_interface tms9928a_interface;
INPUT_PORTS_EXTERN( sk1100 );
extern INTERRUPT_GEN( sg1000_int );
extern const i8255_interface ( sc3000_ppi_intf );
extern const cassette_config sc3000_cassette_config;

class sg1000_state : public driver_device
{
public:
	sg1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_ram;

	virtual void machine_start();

	void install_cartridge(UINT8 *ptr, int size);

	DECLARE_WRITE8_MEMBER( tvdraw_axis_w );
	DECLARE_READ8_MEMBER( tvdraw_status_r );
	DECLARE_READ8_MEMBER( tvdraw_data_r );
	DECLARE_READ8_MEMBER( joysel_r );

	/* keyboard state */
	UINT8 m_keylatch;

	/* TV Draw state */
	UINT8 m_tvdraw_data;
};

class sc3000_state : public sg1000_state
{
public:
	sc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: sg1000_state(mconfig, type, tag),
		  m_cassette(*this, CASSETTE_TAG)
	{ }

	required_device<device_t> m_cassette;

	virtual void machine_start();

	void install_cartridge(UINT8 *ptr, int size);

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
};

#endif
