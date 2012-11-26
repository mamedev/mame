#pragma once

#ifndef __PLUS4__
#define __PLUS4__

#include "emu.h"
#include "cpu/m6502/m7501.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "audio/t6721.h"
#include "audio/mos7360.h"
#include "machine/6551acia.h"
#include "machine/plus4exp.h"
#include "machine/plus4user.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/mos6529.h"
#include "machine/petcass.h"
#include "machine/pla.h"
#include "machine/ram.h"

#define MOS7501_TAG			"u2"
#define MOS7360_TAG			"u1"
#define MOS6551_TAG			"u3"
#define MOS6529_USER_TAG	"u5"
#define MOS6529_KB_TAG		"u27"
#define T6721_TAG			"t6721"
#define PLA_TAG				"u19"
#define SCREEN_TAG			"screen"

class plus4_state : public driver_device
{
public:
	plus4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, MOS7501_TAG),
		  m_pla(*this, PLA_TAG),
		  m_ted(*this, MOS7360_TAG),
		  m_acia(*this, MOS6551_TAG),
		  m_spi_user(*this, MOS6529_USER_TAG),
		  m_spi_kb(*this, MOS6529_KB_TAG),
		  m_t6721(*this, T6721_TAG),
		  m_iec(*this, CBM_IEC_TAG),
		  m_exp(*this, PLUS4_EXPANSION_SLOT_TAG),
		  m_user(*this, PLUS4_USER_PORT_TAG),
		  m_ram(*this, RAM_TAG),
		  m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		  m_function(NULL),
		  m_c2(NULL),
		  m_addr(0),
		  m_ted_irq(CLEAR_LINE),
		  m_acia_irq(CLEAR_LINE),
		  m_exp_irq(CLEAR_LINE)
	{ }

	required_device<m7501_device> m_maincpu;
	required_device<pls100_device> m_pla;
	required_device<mos7360_device> m_ted;
	optional_device<acia6551_device> m_acia;
	optional_device<mos6529_device> m_spi_user;
	required_device<mos6529_device> m_spi_kb;
	optional_device<t6721_device> m_t6721;
	required_device<cbm_iec_device> m_iec;
	required_device<plus4_expansion_slot_device> m_exp;
	optional_device<plus4_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	void check_interrupts();
	void bankswitch(offs_t offset, int phi0, int mux, int ras, int *scs, int *phi2, int *user, int *_6551, int *addr_clk, int *keyport, int *kernal, int *cs0, int *cs1);
	UINT8 read_memory(address_space &space, offs_t offset, int ba, int scs, int phi2, int user, int _6551, int addr_clk, int keyport, int kernal, int cs0, int cs1);
	UINT8 read_keyboard(UINT8 databus);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( ted_videoram_r );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_READ8_MEMBER( c16_cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( ted_irq_w );
	DECLARE_READ8_MEMBER( ted_ram_r );
	DECLARE_READ8_MEMBER( ted_rom_r );
	DECLARE_READ8_MEMBER( ted_k_r );

	DECLARE_READ8_MEMBER( spi_kb_r );
	DECLARE_WRITE8_MEMBER( spi_kb_w );

	DECLARE_READ8_MEMBER( exp_dma_r );
	DECLARE_WRITE8_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );

	// memory state
	const UINT8 *m_kernal;
	const UINT8 *m_function;
	const UINT8 *m_c2;
	UINT8 m_addr;

	// interrupt state
	int m_ted_irq;
	int m_acia_irq;
	int m_exp_irq;

	// keyboard state
	UINT8 m_port6529;
	UINT8 m_keyline[10];
	INTERRUPT_GEN_MEMBER(c16_raster_interrupt);
	INTERRUPT_GEN_MEMBER(c16_frame_interrupt);
};



#endif
