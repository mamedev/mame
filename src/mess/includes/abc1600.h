#pragma once

#ifndef __ABC1600__
#define __ABC1600__


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/8530scc.h"
#include "machine/abc99.h"
#include "machine/abc1600_bus.h"
#include "machine/e0516.h"
#include "machine/lux4105.h"
#include "machine/nmc9306.h"
#include "machine/wd_fdc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z8536.h"
#include "video/mc6845.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MC68008P8_TAG       "3f"
#define Z8410AB1_0_TAG      "5g"
#define Z8410AB1_1_TAG      "7g"
#define Z8410AB1_2_TAG      "9g"
#define Z8470AB1_TAG        "17b"
#define Z8530B1_TAG         "2a"
#define Z8536B1_TAG         "15b"
#define SAB1797_02P_TAG     "5a"
#define FDC9229BT_TAG       "7a"
#define E050_C16PC_TAG      "13b"
#define NMC9306_TAG         "14c"
#define SY6845E_TAG         "sy6845e"
#define SCREEN_TAG          "screen"
#define BUS0I_TAG           "bus0i"
#define BUS0X_TAG           "bus0x"
#define BUS1_TAG            "bus1"
#define BUS2_TAG            "bus2"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc1600_state
class abc1600_state : public driver_device
{
public:
	abc1600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, MC68008P8_TAG),
			m_dma0(*this, Z8410AB1_0_TAG),
			m_dma1(*this, Z8410AB1_1_TAG),
			m_dma2(*this, Z8410AB1_2_TAG),
			m_dart(*this, Z8470AB1_TAG),
			m_scc(*this, Z8530B1_TAG),
			m_cio(*this, Z8536B1_TAG),
			m_fdc(*this, SAB1797_02P_TAG),
			m_rtc(*this, E050_C16PC_TAG),
			m_nvram(*this, NMC9306_TAG),
			m_crtc(*this, SY6845E_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, SAB1797_02P_TAG":0"),
			m_floppy1(*this, SAB1797_02P_TAG":1"),
			m_floppy2(*this, SAB1797_02P_TAG":2"),
			m_bus0i(*this, BUS0I_TAG),
			m_bus0x(*this, BUS0X_TAG),
			m_bus1(*this, BUS1_TAG),
			m_bus2(*this, BUS2_TAG),
			m_rom(*this, MC68008P8_TAG),
			m_wrmsk_rom(*this, "wrmsk"),
			m_shinf_rom(*this, "shinf"),
			m_drmsk_rom(*this, "drmsk"),
			m_segment_ram(*this, "segment_ram"),
			m_page_ram(*this, "page_ram"),
			m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80dma_device> m_dma0;
	required_device<z80dma_device> m_dma1;
	required_device<z80dma_device> m_dma2;
	required_device<z80dart_device> m_dart;
	required_device<scc8530_t> m_scc;
	required_device<z8536_device> m_cio;
	required_device<fd1797_t> m_fdc;
	required_device<e0516_device> m_rtc;
	required_device<nmc9306_device> m_nvram;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<abc1600bus_slot_device> m_bus0i;
	required_device<abc1600bus_slot_device> m_bus0x;
	required_device<abc1600bus_slot_device> m_bus1;
	required_device<abc1600bus_slot_device> m_bus2;
	required_memory_region m_rom;
	required_memory_region m_wrmsk_rom;
	required_memory_region m_shinf_rom;
	required_memory_region m_drmsk_rom;
	optional_shared_ptr<UINT8> m_segment_ram;
	optional_shared_ptr<UINT16> m_page_ram;
	optional_shared_ptr<UINT16> m_video_ram;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( mac_r );
	DECLARE_WRITE8_MEMBER( mac_w );
	DECLARE_READ8_MEMBER( cause_r );
	DECLARE_WRITE8_MEMBER( task_w );
	DECLARE_READ8_MEMBER( segment_r );
	DECLARE_WRITE8_MEMBER( segment_w );
	DECLARE_READ8_MEMBER( page_r );
	DECLARE_WRITE8_MEMBER( page_w );
	DECLARE_WRITE8_MEMBER( fw0_w );
	DECLARE_WRITE8_MEMBER( fw1_w );
	DECLARE_WRITE8_MEMBER( spec_contr_reg_w );

	DECLARE_WRITE8_MEMBER( dmamap_w );
	DECLARE_WRITE_LINE_MEMBER( dbrq_w );
	DECLARE_READ8_MEMBER( dma0_mreq_r );
	DECLARE_WRITE8_MEMBER( dma0_mreq_w );
	DECLARE_READ8_MEMBER( dma0_iorq_r );
	DECLARE_WRITE8_MEMBER( dma0_iorq_w );
	DECLARE_READ8_MEMBER( dma1_mreq_r );
	DECLARE_WRITE8_MEMBER( dma1_mreq_w );
	DECLARE_READ8_MEMBER( dma1_iorq_r );
	DECLARE_WRITE8_MEMBER( dma1_iorq_w );
	DECLARE_READ8_MEMBER( dma2_mreq_r );
	DECLARE_WRITE8_MEMBER( dma2_mreq_w );
	DECLARE_READ8_MEMBER( dma2_iorq_r );
	DECLARE_WRITE8_MEMBER( dma2_iorq_w );

	DECLARE_READ8_MEMBER( video_ram_r );
	DECLARE_WRITE8_MEMBER( video_ram_w );
	DECLARE_READ8_MEMBER( iord0_r );
	DECLARE_WRITE8_MEMBER( iowr0_w );
	DECLARE_WRITE8_MEMBER( iowr1_w );
	DECLARE_WRITE8_MEMBER( iowr2_w );

	DECLARE_READ8_MEMBER( cio_pa_r );
	DECLARE_READ8_MEMBER( cio_pb_r );
	DECLARE_WRITE8_MEMBER( cio_pb_w );
	DECLARE_READ8_MEMBER( cio_pc_r );
	DECLARE_WRITE8_MEMBER( cio_pc_w );

	DECLARE_WRITE_LINE_MEMBER( nmi_w );

	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

	int get_current_task(offs_t offset);
	offs_t get_segment_address(offs_t offset);
	offs_t get_page_address(offs_t offset, UINT8 segd);
	offs_t translate_address(offs_t offset, int *nonx, int *wp);
	UINT8 read_ram(offs_t offset);
	void write_ram(offs_t offset, UINT8 data);
	UINT8 read_io(offs_t offset);
	void write_io(offs_t offset, UINT8 data);
	UINT8 read_internal_io(offs_t offset);
	void write_internal_io(offs_t offset, UINT8 data);
	UINT8 read_external_io(offs_t offset);
	void write_external_io(offs_t offset, UINT8 data);
	UINT8 read_user_memory(offs_t offset);
	void write_user_memory(offs_t offset, UINT8 data);
	int get_fc();
	UINT8 read_supervisor_memory(offs_t offset);
	void write_supervisor_memory(offs_t offset, UINT8 data);

	inline void update_drdy0();
	inline void update_drdy1();
	inline void update_drdy2();
	inline offs_t get_dma_address(int index, UINT16 offset);
	inline UINT8 dma_mreq_r(int index, UINT16 offset);
	inline void dma_mreq_w(int index, UINT16 offset, UINT8 data);
	inline UINT8 dma_iorq_r(int index, UINT16 offset);
	inline void dma_iorq_w(int index, UINT16 offset, UINT8 data);

	inline UINT16 get_drmsk();
	inline void get_shinf();
	inline UINT16 get_wrmsk();
	inline UINT16 barrel_shift(UINT16 gmdr);
	inline UINT16 word_mixer(UINT16 rot);
	inline void clock_mfa_x();
	inline void clock_mfa_y();
	inline void clock_mta_x();
	inline void clock_mta_y();
	inline void load_mfa_x();
	inline void load_mta_x();
	inline void compare_mta_x();
	inline void compare_mta_y();
	inline void load_xy_reg();
	void mover();

	inline UINT16 read_videoram(UINT32 offset);
	inline void write_videoram(UINT32 offset, UINT16 data, UINT16 mask);
	inline UINT16 get_crtca(UINT16 ma, UINT8 ra, UINT8 column);
	void crtc_update_row(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);

	void scc_irq(bool status);

	// memory access controller
	int m_ifc2;
	UINT8 m_task;

	// DMA
	UINT8 m_dmamap[8];
	int m_dmadis;
	int m_sysscc;
	int m_sysfs;
	UINT8 m_cause;
	int m_partst;               // parity test

	// peripherals
	int m_cs7;                  // card select address bit 7
	int m_bus0;                 // BUS 0 selected
	UINT8 m_csb;                // card select
	int m_atce;                 // V.24 channel A external clock enable
	int m_btce;                 // V.24 channel B external clock enable

	// video
	int m_endisp;               // enable display
	int m_clocks_disabled;      // clocks disabled
	UINT16 m_gmdi;              // video RAM data latch
	UINT16 m_wrm;               // write mask latch
	UINT8 m_ms[16];             // mover sequence control
	UINT8 m_ds[16];             // display sequence control
	UINT8 m_flag;               // flags
	UINT16 m_xsize;             // X size
	UINT16 m_ysize;             // Y size
	int m_udx;                  // up/down X
	int m_udy;                  // up/down Y
	UINT16 m_xfrom;             // X from
	UINT16 m_xto;               // X to
	UINT16 m_yto;               // Y to
	UINT16 m_ty;                // to Y
	UINT32 m_mfa;               // mover from address
	UINT32 m_mta;               // mover to address
	UINT8 m_sh;                 //
	UINT16 m_mdor;              //
	int m_hold_1w_cyk;          //
	int m_wrms0;                //
	int m_wrms1;                //
	int m_rmc;                  // row match count
	int m_cmc;                  // column match count
	int m_amm;                  // active mover mask
};



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

/*----------- defined in video/abc1600.c -----------*/

MACHINE_CONFIG_EXTERN( abc1600_video );


#endif
