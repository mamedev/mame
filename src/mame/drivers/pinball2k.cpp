// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie
/*
    Pinball 2000

    Skeleton by R. Belmont, based on mediagx.c by Ville Linde
    Updated by E. van Son 

    TODO:
        - NIC driver

    Hardware:
        - Cyrix MediaGX processor/VGA (northbridge)
        - Cyrix CX5520 (southbridge)
        - PC97317 SuperI/O standard PC I/O chip
        - 1 ISA, 2 PCI slots, 2 IDE headers
        - "Prism" PCI card with: PLX PCI9052 PCI-to-random stuff bridge
                                 ADSP-2104 (DCS2)
                                 Intel DA28F320J5 (U9270541N) Flash
                                 A23290 rev 2 
                                 Midway 5410-14590-00 (9852LK005)
                                 2x P174FCT 162245TV Z9815COC
                                 3x CY7C199-15VC 9903 F04 700778
                                 CY74FCT162543ETPYC CYP 306083
                                 LH52256CN-70LL

    Note: The media GX has been down clocked to get a workable system together with the DCS CPU. It is clocked to 20Mhz instead of 233Mhz.
          You should try to find your best setting when running this driver for your own setup.
          If someone could program it so the DCS cpu uses another thread it could be clocked much higher.

    NVRAM: nrvam_updates contains update and cmos data. (cmos is stored at the end)
            save bootdata, im_flsh0, game and symbols in one file in binary format and make sure the length is 0x800000 bytes and the update should work.
            This is a mandatory file as the version in the ROM does not work properly. It is either the rom version that works or the update.

           nvram2 contains eeprom data for the PLX chip. This file will be filled by the driver with the correct values if it does not exist. However it will fill the values used by updates.

           nvram contains cmos data 

    Mapped keys in mame:
            F2: flip screen (does not work in RFM older versions)
            F5: enter key (coin door switches)
            F6: down key (coin door switches)
            F7: up key (coin door switches)
            F8: esc key (coin door switches)

            left shift: left flipper
            left control: left action button
            right shift: right flipper
            right control: right action button

            0: opens or closes the coin door (needed to operate the coin door switches)
            S: start button
*/

#include "emu.h"
#include "audio/dcs.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/am9517a.h"
#include "machine/idectrl.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/lpci.h"
#include "machine/8042kbdc.h"
#include "machine/pc_lpt.h"
#include "machine/nvram.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"


namespace {

class pinball2k_state : public driver_device
{
public:
	pinball2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dma8237_1(*this, "dma8237_1"),
		m_dma8237_2(*this, "dma8237_2"),
		m_main_ram(*this, "main_ram"),
		m_system_bios1(*this, "system_bios1"),
		m_cga_ram(*this, "cga_ram"),
		m_bios_ram(*this, "bios_ram"),
		m_vram(*this, "vram"),
		m_cmos(*this, "nvram"),
		m_eeprom(*this, "nvram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_ramdac(*this, "ramdac"),
		m_palette(*this, "palette"),
		m_pic1(*this, "pic1"),
		m_pic2(*this, "pic2"),
		m_uart1(*this, "uart1"),
		m_uart2(*this, "uart2"),
		m_rtc(*this, "rtc"),
		m_kbdc(*this, "kbdc"),
		dcs(*this, "dcs"),
		m_pit(*this, "pit8253"),
		m_irq_callback(*this) { }

	void mediagx(machine_config &config);
	void swe1pb(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);

	void init_mediagx();
	void init_pinball2k();
	void init_swe1pb();
	void init(machine_config &config);
	[[maybe_unused]] DECLARE_WRITE_LINE_MEMBER(ethernet_interrupt);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_page_select_r(offs_t offset);
	void dma_page_select_w(offs_t offset, uint8_t data);
	void set_dma_channel(int channel, int state);
	DECLARE_WRITE_LINE_MEMBER( pc_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack1_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack2_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack3_w );
	uint8_t get_slave_ack(offs_t offset);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<mediagx_device> m_maincpu;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint32_t> m_system_bios1;
	required_shared_ptr<uint32_t> m_cga_ram;
	required_shared_ptr<uint32_t> m_bios_ram;
	required_shared_ptr<uint32_t> m_vram;
	required_shared_ptr<uint32_t> m_cmos;
	required_shared_ptr<uint32_t> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<ramdac_device> m_ramdac;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;
	required_device<mc146818_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;
	required_device<dcs2_audio_2104_pin2k_device> dcs;
	required_device<pit8253_device> m_pit;
	uint8_t m_pal[768];

	std::unique_ptr<uint8_t[]> m_nvram_updates;
	void nvram_updates_w(offs_t offset, uint16_t data);
	uint8_t nvram_updates_r(offs_t offset);
	void scanline_cb();

	// DCS
	auto irq_handler() { return m_irq_callback.bind(); }

	cpu_device *m_dcs_cpu;

	uint32_t m_reg[16];

	uint16_t dcs_read(address_space &space, offs_t offset);

	uint8_t m_shuffle_type;
	uint8_t m_shuffle_default;
	uint8_t m_shuffle_active;
	const uint8_t * m_shuffle_map;
	devcb_write8 m_irq_callback;
	uint8_t m_irq_state;
	uint16_t m_sound_irq_state;
	uint8_t m_auto_ack;
	uint8_t m_force_fifo_full;

	int m_dcs_load_bootdata;
	int m_dcs_load_bootitems;
	int m_dcs_load_bootwordcounter;
	uint32_t m_dcsdata;

	enum
	{
		IOASIC_PORT0,       /* 0: input port 0 */
		IOASIC_PORT1,       /* 1: input port 1 */
		IOASIC_PORT2,       /* 2: input port 2 */
		IOASIC_PORT3,       /* 3: input port 3 */
		IOASIC_UARTCONTROL, /* 4: controls some UART behavior */
		IOASIC_UARTOUT,     /* 5: UART output */
		IOASIC_UARTIN,      /* 6: UART input */
		IOASIC_COIN,        /* 7: triggered on coin insertion */
		IOASIC_SOUNDCTL,    /* 8: sound communications control */
		IOASIC_SOUNDOUT,    /* 9: sound output port */
		IOASIC_SOUNDSTAT,   /* a: sound status port */
		IOASIC_SOUNDIN,     /* b: sound input port */
		IOASIC_PICOUT,      /* c: PIC output port */
		IOASIC_PICIN,       /* d: PIC input port */
		IOASIC_INTSTAT,     /* e: interrupt status */
		IOASIC_INTCTL       /* f: interrupt control */
	};
	//DECLARE_WRITE_LINE_MEMBER(timer1_changed);

	uint8_t m_pdb[10];
	int m_pdbcounter;

	int m_prism_eprom_clk;
	int m_prism_clock_enabled;
	int m_prism_eprom_counter;
	int m_prism_eprom_offset;
	int m_prism_eprom_wordtoggle;
	int m_prismbank;
	int m_dcsbank;
	int m_updatebank;
	int m_startIRQ;
	int m_1200_mode;
	int m_buffer_counter;

	uint32_t m_eeprom_regs[256/4];
	uint32_t m_disp_ctrl_reg[256/4];
	uint32_t m_gfx_pipeline_reg[512/4];
	uint32_t m_scratchpad_mem[0xc00/4];
	int m_frame_width;
	int m_frame_height;

	void do_gfx_pipeline(int mode);

	uint32_t m_memory_ctrl_reg[256/4];
	int m_pal_index;

	uint32_t m_biu_ctrl_reg[256/4];

	uint32_t m_parport;

	uint32_t m_scanline;

	uint8_t m_mediagx_config_reg_sel;
	uint8_t m_mediagx_config_regs[256];

	uint8_t m_superio_reg_sel;
	uint8_t m_superio_regs[256];

	//uint8_t m_controls_data;
	//uint8_t m_parallel_pointer;
	//uint8_t m_parallel_latched;
	//uint32_t m_parport;
	//int m_control_num;
	//int m_control_num2;
	//int m_control_read;

	uint32_t m_cx5520_regs[256/4];
	uint32_t m_prism_regs[256/4];
	uint32_t m_mediagx_regs[65];

	// mediaGX Control registers GX_BASE + 0x8000
	uint32_t disp_ctrl_r(offs_t offset);
	void disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t memory_ctrl_r(offs_t offset);
	void memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t biu_ctrl_r(offs_t offset);
	void biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gx_pipeline_r(offs_t offset);
	void gx_pipeline_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t scratchpad_r(offs_t offset);
	void scratchpad_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	int m_vblank;

	[[maybe_unused]] void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint8_t io22_r(offs_t offset);
	void io22_w(offs_t offset, uint8_t data);
	[[maybe_unused]]uint8_t io23_r(offs_t offset);
	[[maybe_unused]]void io23_w(offs_t offset, uint8_t data);
	uint8_t io61_r(offs_t offset);
	void io61_w(offs_t offset, uint8_t data);
	uint32_t port400_r();
	void port400_w(uint32_t data);
	uint32_t port800_r();
	void port800_w(uint32_t data);
	uint8_t port2e_r(offs_t offset);
	void port2e_w(offs_t offset, uint8_t data);
	uint8_t port278_r(offs_t offset);
	void port278_w(offs_t offset, uint8_t data);
	uint8_t port208_r(offs_t offset);
	void port208_w(offs_t offset, uint8_t data);
	uint8_t read_lpt(offs_t offset);
	void write_lpt(offs_t offset, uint8_t data);

	[[maybe_unused]] uint8_t port378_r(offs_t offset);
	[[maybe_unused]] void port378_w(offs_t offset, uint8_t data);

	void expansion_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t expansion_r(offs_t offset);
	void prism_1000_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t prism_1000_r(offs_t offset, uint32_t mem_mask);

	void prism_1300_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void prism_1400_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t prism_1400_r(offs_t offset);
	uint32_t prism_1500_r(offs_t offset);
	uint32_t prism_1600_r(offs_t offset);
	uint32_t prism_1700_r(offs_t offset);

	uint32_t screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y);
	void draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int scanline);
	void draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mediagx_io(address_map &map);
	void mediagx_map(address_map &map);
	void ramdac_map(address_map &map);

	int m_dma_channel;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10];

	// TIMER_CALLBACK_MEMBER(m_scanline_timer_callback);
	emu_timer *m_scanline_timer;
	enum
	{
		TIMER_SCANLINE
	};

	void reset_scanline_timer(void)
	{
		m_scanline_timer->adjust(m_screen->scan_period());
	}

	uint32_t cx5520_pci_r(int function, int reg, uint32_t mem_mask);
	void cx5520_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);

	uint32_t prism_pci_r(int function, int reg, uint32_t mem_mask);
	void prism_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);

	uint32_t mediagx_pci_r(int function, int reg, uint32_t mem_mask);
	void mediagx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);

	int m_pit_out2;
	struct
	{
		/* PCI */
		uint32_t command;
		uint32_t base_addr;

		uint32_t init_enable;
	} m_prism_pci_regs;

	// switches
	int m_coindoor = 1;
	int m_coininserted = 0;
	int m_diagswitch = 0;
	int m_cabinetswitch = 2;
	int m_startbutton = 0;
	int m_switchcolumn = 0;
	int m_pdb_1 = 0;
	int m_pdb_2 = 0;

	int m_dcs_read_0 = 0;
	int m_dcs_test = 0;
	uint16_t m_dcs_lastreceived;
};

// Display controller registers
#define DC_UNLOCK               0x00/4
#define DC_GENERAL_CFG          0x04/4
#define DC_TIMING_CFG           0x08/4
#define DC_OUTPUT_CFG           0x0c/4
#define DC_FB_ST_OFFSET         0x10/4
#define DC_CB_ST_OFFSET         0x14/4
#define DC_CUR_ST_OFFSET        0x18/4
#define DC_VID_ST_OFFSET        0x20/4
#define DC_LINE_DELTA           0x24/4
#define DC_BUF_SIZE             0x28/4
#define DC_H_TIMING_1           0x30/4
#define DC_H_TIMING_2           0x34/4
#define DC_H_TIMING_3           0x38/4
#define DC_FP_H_TIMING          0x3c/4
#define DC_V_TIMING_1           0x40/4
#define DC_V_TIMING_2           0x44/4
#define DC_V_TIMING_3           0x48/4
#define DC_FP_V_TIMING          0x4c/4
#define DC_CURSOR_X             0x50/4
#define DC_V_LINE_CNT           0x54/4
#define DC_CURSOR_Y             0x58/4
#define DC_SS_LINE_CMP          0x5c/4
#define DC_CURSOR_COLOR         0x60/4
#define DC_BORDER_COLOR         0x68/4
#define DC_PAL_ADDRESS          0x70/4
#define DC_PAL_DATA             0x74/4
#define DC_DFIFO_DIAG           0x78/4
#define DC_CFIFO_DIAG           0x7c/4

// Graphics pipeline registers
#define GP_DST                  0x00/4
#define GP_WIDTH                0x04/4
#define GP_SRC_X                0x08/4
#define GP_SRC_COLOR            0x0c/4
#define GP_PAT_COLOR_A          0x10/4
#define GP_PAT_COLOR_B          0x14/4
#define GP_PAT_DATA_0           0x20/4
#define GP_PAT_DATA_1           0x24/4
#define GP_PAT_DATA_2           0x28/4
#define GP_PAT_DATA_3           0x2c/4
#define GP_VGA_WRITE            0x40/4
#define GP_VGA_READ             0x44/4
#define GP_RASTER_MODE          0x100/4
#define GP_VECTOR_MODE          0x104/4
#define GP_BLT_MODE             0x108/4
#define GP_BLT_STATUS           0x10c/4
#define GP_VGA_BASE             0x110/4
#define GP_VGA_LATCH            0x114/4



// DCS Functions
uint16_t pinball2k_state::dcs_read(address_space &space, offs_t offset)
{
	uint16_t result;

	switch (offset)
	{
		case 0x1:
			/* status from sound CPU */
			result = dcs->pin2000_status_r();
			break;
		case 0x0:
			result = dcs->data_r_pin2k();
			break;
		default:
			result = 0;
			break;
	}
	return result;
}

void pinball2k_state::prism_1300_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 0:
			dcs->data_w_pin2k(data);
			break;
		case 1: // watch dog timer 0x40 means run 0x80 means reset
			if (data == 0x40)
				m_eeprom_regs[0x13] |= 0x4;
			else
				m_eeprom_regs[0x13] &= ~0x4;
			break;
	}
}

// End DCS
void pinball2k_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SCANLINE: scanline_cb(); break;
	}
}

void pinball2k_state::scanline_cb()
{
	const int scanline = m_screen->vpos();

	m_disp_ctrl_reg[0x54/4] = scanline;

	m_scanline_timer->adjust(m_screen->time_until_pos(scanline + 1));
}

INPUT_CHANGED_MEMBER(pinball2k_state::kb_irq)
{
	uint8_t key = (uint8_t)(param & 0xff);
	switch (key)
	{
		case 0x71:
			if (newval == 1)
			{
				m_kbdc->data_w(4, 0xd2);
				m_kbdc->data_w(0,0x3c);
				m_pic1->ir1_w(1);
			}
			else if(newval == 0)
				m_pic1->ir1_w(0);
			break;
		case 0x0: // coindoor
			//if (newval == 1) m_cabinetswitch |= 1UL << 1;
			//if (newval == 0) m_cabinetswitch &= ~(1 << 1);
			if (newval == 1) {
				if ((m_cabinetswitch >> 1 & 1) == 1)
					m_cabinetswitch &= ~(1 << 1);
				else
					m_cabinetswitch |= 1UL << 1;
			}
			break;
		case 0x1: // 1
			if (newval == 1) m_diagswitch |= 1 << 5;
			if (newval == 0) m_diagswitch &= ~(1 << 5);
			break;
		case 0x2: // 2 = coin
			if (newval == 1) m_coininserted |= 1 << 1;
			if (newval == 0) m_coininserted &= ~(1 << 1);
			break;
		case 0x3: // 3 = coin
			if (newval == 1) m_coininserted |= 1 << 2;
			if (newval == 0) m_coininserted &= ~(1 << 2);
			break;
		case 0x4: // 4 = coin 
			if (newval == 1) m_coininserted |= 1 << 7;
			if (newval == 0) m_coininserted &= ~(1 << 7);
			break;
		case 0x5: // 5
			if (newval == 1) m_diagswitch |= 1 << 4;
			if (newval == 0) m_diagswitch &= ~(1 << 4);
			break;
		case 0xa: // S
			if (newval == 1) m_startbutton |= 1 << 0;
			if (newval == 0) m_startbutton &= ~(1 << 0);
			break;
		case 0x73: // F5
			if (newval == 1) m_diagswitch |= 1UL << 3;
			if (newval == 0) m_diagswitch &= ~(1 << 3);
			break;
		case 0x74: // F6
			if (newval == 1) m_diagswitch |= 1UL << 2;
			if (newval == 0) m_diagswitch &= ~(1 << 2);
			break;
		case 0x75: // F7
			if (newval == 1) m_diagswitch |= 1UL << 1;
			if (newval == 0) m_diagswitch &= ~(1 << 1);
			break;
		case 0x76: // F8
			if (newval == 1) m_diagswitch |= 1UL << 0;
			if (newval == 0) m_diagswitch &= ~(1 << 0);
			break;
		case 0x78: // left shift (flipper left)
			if (newval == 1) m_cabinetswitch |= 1UL << 5;
			if (newval == 0) m_cabinetswitch &= ~(1 << 5);
			break;
		case 0x79: // right shift (flipper right)
			if (newval == 1) m_cabinetswitch |= 1UL << 4;
			if (newval == 0) m_cabinetswitch &= ~(1 << 4);
			break;
		case 0x7A: // left action button
			if (newval == 1) m_cabinetswitch |= 1UL << 7;
			if (newval == 0) m_cabinetswitch &= ~(1 << 7);
			break;
		case 0x7B: // right action button
			if (newval == 1) m_cabinetswitch |= 1UL << 6;
			if (newval == 0) m_cabinetswitch &= ~(1 << 6);
			break;
	}
}

/******************
DMA8237 Controller
******************/

WRITE_LINE_MEMBER( pinball2k_state::pc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_1->hack_w( state );
}

uint8_t pinball2k_state::pc_dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	return prog_space.read_byte(page_offset + offset);
}

void pinball2k_state::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t pinball2k_state::dma_page_select_r(offs_t offset)
{
	uint8_t data;

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	default:
		data = m_at_pages[offset % 0x10];
		break;
	}
	return data;
}

void pinball2k_state::dma_page_select_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}

}

void pinball2k_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dma_channel = channel;
}

WRITE_LINE_MEMBER( pinball2k_state::pc_dack0_w ) { set_dma_channel(0, state); }
WRITE_LINE_MEMBER( pinball2k_state::pc_dack1_w ) { set_dma_channel(1, state); }
WRITE_LINE_MEMBER( pinball2k_state::pc_dack2_w ) { set_dma_channel(2, state); }
WRITE_LINE_MEMBER( pinball2k_state::pc_dack3_w ) { set_dma_channel(3, state); }

static const rgb_t cga_palette[16] =
{
	rgb_t( 0x00, 0x00, 0x00 ), rgb_t( 0x00, 0x00, 0xaa ), rgb_t( 0x00, 0xaa, 0x00 ), rgb_t( 0x00, 0xaa, 0xaa ),
	rgb_t( 0xaa, 0x00, 0x00 ), rgb_t( 0xaa, 0x00, 0xaa ), rgb_t( 0xaa, 0x55, 0x00 ), rgb_t( 0xaa, 0xaa, 0xaa ),
	rgb_t( 0x55, 0x55, 0x55 ), rgb_t( 0x55, 0x55, 0xff ), rgb_t( 0x55, 0xff, 0x55 ), rgb_t( 0x55, 0xff, 0xff ),
	rgb_t( 0xff, 0x55, 0x55 ), rgb_t( 0xff, 0x55, 0xff ), rgb_t( 0xff, 0xff, 0x55 ), rgb_t( 0xff, 0xff, 0xff ),
};

void pinball2k_state::video_start()
{
	int i;
	for (i=0; i < 16; i++)
	{
		m_palette->set_pen_color(i, cga_palette[i]);
	}
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
}

void pinball2k_state::draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y)
{
	int index = 0;
	pen_t const *const pens = m_palette->pens();

	uint8_t const *const dp = gfx->get_data(ch);

	for (int j=y; j < y+8; j++)
	{
		uint32_t *const p = &bitmap.pix(j);
		for (int i=x; i < x+8; i++)
		{
			uint8_t const pen = dp[index++];
			if (pen)
				p[i] = pens[gfx->colorbase() + (att & 0xf)];
			else
			{
				if (((att >> 4) & 7) > 0)
					p[i] = pens[gfx->colorbase() + ((att >> 4) & 0x7)];
			}
		}
	}
}

void pinball2k_state::draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int scanline)
{
	int width, height;
	int line_delta = (m_disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) << 1;
	constexpr int _rgb_scale_5[32] =
	{
		0, 8, 16, 24, 32, 41, 49, 57,
		65, 74, 82, 90, 98, 106, 115, 123,
		131, 139, 148, 156, 164, 172, 180, 189,
		197, 205, 213, 222, 230, 238, 246, 255
	};
	width = (m_disp_ctrl_reg[DC_H_TIMING_1] & 0x7ff) + 1;
	if (m_disp_ctrl_reg[DC_TIMING_CFG] & 0x8000)     // pixel double
	{
		width >>= 1;
	}
	width += 4;

	height = (m_disp_ctrl_reg[DC_V_TIMING_1] & 0x7ff) + 1;

	if ( (width != m_frame_width || height != m_frame_height) &&
			(width > 1 && height > 1 && width <= 640 && height <= 480) )
	{
		rectangle visarea;

		m_frame_width = width;
		m_frame_height = height;

		visarea.set(0, width - 1, 0, height - 1);
		m_screen->configure(width, height * 260 / 240, visarea, m_screen->frame_period().attoseconds());
	}

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)        // 8-bit mode
	{
		uint8_t const *const framebuf = (uint8_t*)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];
		uint8_t const *const pal = m_pal;
		int lines = scanline * (line_delta << 1);

		uint32_t *const p = &bitmap.pix(scanline);
		uint8_t const *si = &framebuf[lines];
		for (int i=0; i < m_frame_width; i++)
		{
			int c = *si++;
			int r = pal[(c*3)+0] << 2;
			int g = pal[(c*3)+1] << 2;
			int b = pal[(c*3)+2] << 2;

			p[i] = r << 16 | g << 8 | b;
		}
	}
	else            // 16-bit
	{
		uint16_t const *const framebuf = (uint16_t*)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		// RGB 5-6-5 mode
		if ((m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x2) == 0)
		{
			uint32_t *const p = &bitmap.pix(scanline);
			uint16_t const *si = &framebuf[scanline * (line_delta << 1)];
			for (int i=0; i < m_frame_width; i++)
			{
				uint16_t c = *si++;
				int r = ((c >> 11) & 0x1f) << 3;
				int g = ((c >> 5) & 0x3f) << 2;
				int b = (c & 0x1f) << 3;

				p[i] = r << 16 | g << 8 | b;
			}
		}
		// RGB 5-5-5 mode
		else
		{
			// pinball 2000 uses 5-5-5 mode
			uint32_t *const p = &bitmap.pix(scanline);
			uint16_t const *si = &framebuf[scanline * line_delta];
			for (int i=0; i < m_frame_width; i++)
			{
				uint16_t c = *si++;
				int r = _rgb_scale_5[((c >> 10) & 0x1f)];	// ((c >> 10) & 0x1f) << 3;
				int g = _rgb_scale_5[((c >> 5) & 0x1f)];	// ((c >> 5) & 0x1f) << 3;
				int b = _rgb_scale_5[(c & 0x1f)];			// (c & 0x1f) << 3;

				p[i] = r << 16 | g << 8 | b;
			}
		}
	}
}

void pinball2k_state::draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint32_t const *const cga = m_cga_ram;
	int index = 0;

	for (int j=0; j < 25; j++)
	{
		for (int i=0; i < 80; i+=2)
		{
			int att0 = (cga[index] >> 8) & 0xff;
			int ch0 = (cga[index] >> 0) & 0xff;
			int att1 = (cga[index] >> 24) & 0xff;
			int ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
		}
	}
}

void pinball2k_state::do_gfx_pipeline(int mode)
{
	if (mode == GP_VECTOR_MODE)
	{
		logerror("vector not implemented!");
		return;
	}

	m_gfx_pipeline_reg[GP_BLT_STATUS] = m_gfx_pipeline_reg[GP_BLT_STATUS] | 0x7;

	int line_delta = (m_disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) << 1; // value is in dword need to convert to word

	uint8_t rastermode = m_gfx_pipeline_reg[GP_RASTER_MODE] & 0xff;
	int x = m_gfx_pipeline_reg[GP_DST] & 0xffff;
	int y = m_gfx_pipeline_reg[GP_DST] >> 16;
	int src_y = m_gfx_pipeline_reg[GP_SRC_X] >> 16;
	int src_x = m_gfx_pipeline_reg[GP_SRC_X] & 0xffff;
	int width = m_gfx_pipeline_reg[GP_WIDTH] &0xffff;

	uint16_t *srcdata = (uint16_t*)&m_vram[0];
	uint16_t *destdata = (uint16_t*)&m_vram[0];
	int16_t tr1 = 0;
	//int8_t tr2 = 0;

	int i = y * line_delta;
	int src_i = src_y * line_delta;

	for (int j=0; j < width; j++)
	{
		switch (rastermode)
		{
		case 0x00:		// BLACKNESS Fills the destination rectangle with black
			destdata[i + (j + x)] = 0x0000;
			//destdata[i + ((j + x) << 1)] = 0x00;
			//destdata[i + ((j + x) << 1) + 1] = 0x00;
			break;
		case 0x11:		// NOTSRCERASE Fills the destination area with (not (Dst or Src))
			logerror("NOTSRCERASE");
			break;
		case 0x33:		// NOTSRCCOPY Fills the destination area with (not Src)
			logerror("NOTSRCCOPY");
			break;
		case 0x44:		// SRCERASE  Fills the destination area with ((not Dst) and Src)
			logerror("SRCERASE");
			break;
		case 0x55:		// DSTINVERT Inverts the colors of the destination area
			logerror("DSTINVERT");
			break;
		case 0x5a:		// PATINVERT Fills the destination area with (Dst xor Pattern)
			logerror("PATINVERT");
			break;
		case 0x66:		// SRCINVERT Fills the destination area with (Dst xor Src)
			logerror("SRCINVERT");
			break;
		case 0x88:		// SRCAND Fills the destination area with (Dst and Src)
			logerror("SRCAND");
			break;
		case 0xbb:		// MERGEPAINT Fills the destination area with (Dst or not Src)
			logerror("MERGEPAINT");
			break;
		case 0xc0:		// MERGECOPY Fills the destination area with (Src and Pattern)
			logerror("MERGECOPY");
			break;
		case 0xc6: // transparant
			tr1 = srcdata[src_i + src_x + j];
			if (tr1 != 0x7c1f) // hack for transp check
				destdata[i + (j + x)] = tr1;
			break;
		case 0xcc:		// SRCCOPY Fills the destination area with Src
			destdata[i + (j + x)] = srcdata[src_i + src_x + j];
			break;
		case 0xee:		// SRCPAINT Combines the colors of the source and the destination using the operator OR on each pixel
			logerror("SRCPAINT");
			break;
		case 0xf0:		// PATCOPY Fills the destination area with (Pattern)
			logerror("PATCOPY");
			break;
		case 0xfb:		// PATPAINT Fills the destination area with (Dst or (not Src) or Pattern)
			logerror("PATPAINT");
			break;
		case 0xff:		// WHITENESS Fills the destination rectangle with white
			destdata[i + (j + x)] = 0xffff;
			break;
		default:
			logerror("unknown rastermode: %02x\n", rastermode);
			break;
		}
	}
	m_gfx_pipeline_reg[GP_BLT_STATUS] = m_gfx_pipeline_reg[GP_BLT_STATUS] & 0xfffffff8;
}

uint32_t pinball2k_state::screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_framebuffer( bitmap, cliprect, screen.vpos());

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)   // don't show MDA text screen on 16-bit mode. this is basically a hack
	{
		draw_cga(bitmap, cliprect);
	}
	return 0;
}

uint32_t pinball2k_state::scratchpad_r(offs_t offset)
{
	return m_scratchpad_mem[offset];
}

void pinball2k_state::scratchpad_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_scratchpad_mem + offset);
}

uint32_t pinball2k_state::disp_ctrl_r(offs_t offset)
{
	uint32_t r = m_disp_ctrl_reg[offset];

	switch (offset)
	{
		case DC_TIMING_CFG:
			r |= 0x40000000;

			if (m_screen->vpos() >= m_frame_height)
				r &= ~0x40000000;
			break;
	}

	return r;
}

void pinball2k_state::disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_disp_ctrl_reg + offset);
}


uint32_t pinball2k_state::memory_ctrl_r(offs_t offset)
{
	return m_memory_ctrl_reg[offset];
}

void pinball2k_state::gx_pipeline_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
		case GP_VECTOR_MODE:
			if (data > 0)
				do_gfx_pipeline(GP_VECTOR_MODE);
			break;
		case GP_BLT_MODE:
			if (data > 0)
				do_gfx_pipeline(GP_BLT_MODE);
			break;
	}
	COMBINE_DATA(m_gfx_pipeline_reg + offset);
}

void pinball2k_state::memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0x20/4)
	{
		if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00e00000) == 0x00400000)
		{
			// guess: crtc params?
			// ...
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00000000)
		{
			m_pal_index = data;
			m_ramdac->index_w(data);
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00100000)
		{
			m_pal[m_pal_index] = data & 0xff;
			m_pal_index++;
			if (m_pal_index >= 768)
			{
				m_pal_index = 0;
			}
			m_ramdac->pal_w(data);
		}
	}
	else
	{
		COMBINE_DATA(m_memory_ctrl_reg + offset);
	}
}



uint32_t pinball2k_state::biu_ctrl_r(offs_t offset)
{
	return m_biu_ctrl_reg[offset];
}

void pinball2k_state::biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_biu_ctrl_reg + offset);
}

uint32_t pinball2k_state::gx_pipeline_r(offs_t offset)
{
	return m_gfx_pipeline_reg[offset]; 
}

void pinball2k_state::bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

void pinball2k_state::expansion_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

uint32_t pinball2k_state::expansion_r(offs_t offset)
{
	return ((uint32_t *)memregion("prismdata1")->base())[offset];
}

uint32_t pinball2k_state::prism_1000_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == 0x14)
	{
		uint32_t t = 0x10000000;
		t |= m_eeprom_regs[offset]; // enable always bit 28 for eeprom OK

		if (m_prism_clock_enabled == 1 && m_prism_eprom_clk == 0x1 && m_prism_eprom_offset != -1)
		{
			if (m_prism_eprom_counter == 0) {
				if (m_prism_eprom_wordtoggle == 1) {
					m_prism_eprom_offset += 1;
					m_prism_eprom_wordtoggle = 0;
				}
				else if (m_prism_eprom_wordtoggle == 0)
					m_prism_eprom_wordtoggle += 1;
				m_prism_eprom_counter = 16;
			}
			m_prism_eprom_counter--;
			// start with offset + 1 then offset, shift
			uint16_t val;
			//val = memregion("eeprom_data")->base()[m_prism_eprom_offset + 1] << 8;
			if (m_prism_eprom_wordtoggle == 0) 
				val = ((m_eeprom[m_prism_eprom_offset] & 0xffff0000) >> 16);
			else
				val = m_eeprom[m_prism_eprom_offset] & 0xffff;

			//val |= memregion("eeprom_data")->base()[m_prism_eprom_offset];
			//val |= eeprom[m_prism_eprom_offset];

			uint8_t bit = (val >> m_prism_eprom_counter) & 0x1;
			t = (t & (~(1 << 27))) | (bit << 27);
		}
		else if (m_prism_clock_enabled == 1 && m_prism_eprom_clk == 0x1 && m_prism_eprom_offset == -1) // first send a 0
		{
			t = (t & (~(1 << 27))) | (0 << 27);
			m_prism_eprom_offset = 0;
		}
		return t;
	}

	if (offset == 0x13)
	{
		return m_eeprom_regs[offset] | 0x4;
		//return eeprom[offset];
	}
	else
		return m_eeprom_regs[offset];
}

void pinball2k_state::prism_1000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0x14)
	{
		if (((data >> 24 ) & 0x2) == 0x2) // bit 25 = 1 need clock
		{
			if (m_prism_clock_enabled == 0)
				{m_prism_eprom_offset = -1;}
			m_prism_clock_enabled = 1;
			// bit 24 is clock
			m_prism_eprom_clk = ((data >> 24) & 0x1);
		}
		else
		{
			m_prism_clock_enabled = 0;
			m_prism_eprom_counter = 16;
			m_prism_eprom_wordtoggle = 0;
		}
	}

	COMBINE_DATA(m_eeprom_regs + offset);
}

void pinball2k_state::nvram_updates_w(offs_t offset, uint16_t data)
{
	// flash chip: 98h = read Query
				// 70h read status register
				// ffh = read array
				// 50h = clear status register
				// e8h = write to buffer
				// 40h or 10h = word / byte program
				// 20h = block erase
				// b0h = block erase suspend
				// d0h = block erase resume
				// b8h = configuration
				// 60h + 01h = set block lock-bit
				// 60h + d0h = clear block lock-bit
				// 60h + f1h = set master lock bit
	//logerror("%s: mode = %02x\n", machine().describe_context(), m_1200_mode);

	if ((data  == 0x0098) && (offset >= 0) && (offset <= 0x7fffff) && !(m_1200_mode == 6))
		m_1200_mode = 1; // read query
	else if ((data  == 0x0070) && (offset >= 0) && (offset <= 0x7fffff) && !(m_1200_mode == 6))
		m_1200_mode = 2; // read status register
	else if ((data  == 0x00ff) && (offset >= 0) && (offset <= 0x7fffff) && !(m_1200_mode == 6))
		m_1200_mode = 0; // read array
	else if ((data == 0x0020) && (offset % 0x2000) == 0 && !(m_1200_mode == 6))
		m_1200_mode = 3;
	else if ((m_1200_mode == 3) && (data  == 0x00d0) && !(m_1200_mode == 6))
	{ // block erase part2
		m_1200_mode = 4;
		for (u32 i = 0; i< 0x2000; i++)
			m_nvram_updates[offset * 2 + i] = 0xff;
		m_1200_mode = 2; // put in read status register
	} // block erase
	else if ((data == 0x00e8) && (offset >= 0) && (offset <= 0x7fffff) && !(m_1200_mode == 6))
	{ // write to buffer
		m_1200_mode = 5;
		m_buffer_counter = 0;
	}
	else if ((m_1200_mode == 5) && (m_buffer_counter == 0))
	{
		m_1200_mode = 6;
		m_buffer_counter = data;

		return;
	}
	else if (m_1200_mode == 6)
	{
		if (offset >= 0x3e0000) {
			m_nvram_updates[offset * 2] = data;
			m_nvram_updates[offset * 2 + 1] = data >> 8;
		}
		m_buffer_counter--;
		if (m_buffer_counter < 0)
			m_1200_mode = 2;
		return;
	}
}

uint8_t pinball2k_state::nvram_updates_r(offs_t offset)
{
	//return m_nvram_updates[offset];

	if (m_1200_mode == 0) {
		return m_nvram_updates[offset];
	}
	else if (m_1200_mode == 1)
	{
		switch (offset)
		{
			case 0x20:	return 0x51;
			case 0x21:	return 0x00;
			case 0x22:	return 0x52;
			case 0x23:	return 0x00;
			case 0x24:	return 0x59;
			case 0x25:	return 0x00;
			case 0x26:	return 0x01;
			case 0x27:	return 0x00;
			case 0x28:	return 0x00;
			case 0x29:	return 0x00;
			case 0x2a:	return 0x31;
			case 0x2b:	return 0x00;
			case 0x2c:	return 0x0;
			case 0x2d:	return 0x0;
			case 0x2e:	return 0x0;
			case 0x2f:	return 0x0;
			case 0x30:	return 0x0;
			case 0x31:	return 0x0;
			case 0x32:	return 0x0;
			case 0x33:	return 0x0;
			case 0x34:	return 0x00;
			case 0x35:	return 0x00;
			case 0x36:	return 0x45;
			case 0x37:	return 0x00;
			case 0x38:	return 0x55;
			case 0x39:	return 0x00;
			case 0x3a:	return 0x00;
			case 0x3b:	return 0x00;
			case 0x3c:	return 0x00;
			case 0x3d:	return 0x00;
			case 0x3e:	return 0x07;
			case 0x3f:	return 0x00;
			case 0x40:	return 0x07;
			case 0x41:	return 0x00;
			case 0x42:	return 0x0a;
			case 0x43:	return 0x00;
			case 0x44:	return 0x00;
			case 0x45:	return 0x00;
			case 0x46:	return 0x04;
			case 0x47:	return 0x00;
			case 0x48:	return 0x04;
			case 0x49:	return 0x00;
			case 0x4a:	return 0x04;
			case 0x4b:	return 0x00;
			case 0x4c:	return 0x00;
			case 0x4d:	return 0x00;
			case 0x4e:	return 0x17; // for 8mbit flash //return 0x16; for 4mbit
			case 0x4f:	return 0x00;
			case 0x50:	return 0x02;
			case 0x51:	return 0x00;
			case 0x52:	return 0x00;
			case 0x53:	return 0x00;
			case 0x54:	return 0x05;
			case 0x55:	return 0x00;
			case 0x56:	return 0x00;
			case 0x57:	return 0x00;
			case 0x58:	return 0x01;
			case 0x59:	return 0x00;
			case 0x5a:	return 0x3f; // for 8mbit // return 0x1f; for 4mbit
			case 0x5B:	return 0x00;
			case 0x5c:	return 0x00;
			case 0x5d:	return 0x00;
			case 0x5e:	return 0x00;
			case 0x5f:	return 0x00;
			case 0x60:	return 0x02;
			case 0x61:	return 0x00;
			case 0x62:	return 0x50;
			case 0x63:	return 0x00;
			case 0x64:	return 0x52;
			case 0x65:	return 0x00;
			case 0x66:	return 0x49;
			case 0x67:	return 0x00;
		}
		return 0;
	}
	else if ((m_1200_mode == 2) || (m_1200_mode == 5))
		return 0x80;
	else
		return m_nvram_updates[offset];
}

uint32_t pinball2k_state::prism_1400_r(offs_t offset)
{
	switch(m_prismbank)
	{
		case 0:
			return ((uint32_t *)memregion("prismdata1")->base())[offset];
		case 1:
			return ((uint32_t *)memregion("prismdata2")->base())[offset];
		case 2:
			return ((uint32_t *)memregion("prismdata3")->base())[offset];
		case 3:
			return ((uint32_t *)memregion("prismdata4")->base())[offset];
	}
	return 0;
}

uint32_t pinball2k_state::prism_1500_r(offs_t offset)
{
	return ((uint32_t *)memregion("prismdata2")->base())[offset];
}

uint32_t pinball2k_state::prism_1600_r(offs_t offset)
{
	return ((uint32_t *)memregion("prismdata3")->base())[offset];
}

uint32_t pinball2k_state::prism_1700_r(offs_t offset)
{
	return ((uint32_t *)memregion("prismdata4")->base())[offset];
}

void pinball2k_state::prism_1400_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (data == 0x00000098) // bank1
	{
		m_prismbank = 0;
	}
	else if (data == 0x00009800) // bank2
	{
		m_prismbank = 1;
	}
	else if (data == 0x00980000) // bank3
	{
		m_prismbank = 2;
	}
	else if (data == 0x98000000) // bank4
	{
		m_prismbank = 3;
	}
	else
	{
		m_prismbank = 0;
	}
}

uint8_t pinball2k_state::io22_r(offs_t offset)
{
	uint8_t r = 0;

	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
	}
	else if (offset == 0x01)
	{
		r = m_mediagx_config_regs[m_mediagx_config_reg_sel];
	}
	return r;
}

void pinball2k_state::io22_w(offs_t offset, uint8_t data)
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
		m_mediagx_config_reg_sel = data;
	}
	else if (offset == 0x01)
	{
		m_mediagx_config_regs[m_mediagx_config_reg_sel] = data;
	}
}

uint8_t pinball2k_state::io23_r(offs_t offset)
{
	// 0x22, 0x23, Cyrix configuration registers
	return m_mediagx_config_regs[m_mediagx_config_reg_sel];
}

void pinball2k_state::io23_w(offs_t offset, uint8_t data)
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
	}
	else if (offset == 0x01)
	{
		m_mediagx_config_regs[m_mediagx_config_reg_sel] = data;
	}
}

uint8_t pinball2k_state::io61_r(offs_t offset)
{
	return 0;
}

void pinball2k_state::io61_w(offs_t offset, uint8_t data)
{
}

uint8_t pinball2k_state::port2e_r(offs_t offset)
{
	uint8_t r = 0;

	if (offset == 0x00)
	{
	}
	else if (offset == 0x01)
	{
		r = m_superio_regs[m_superio_reg_sel];
	}
	return r;
}

void pinball2k_state::port2e_w(offs_t offset, uint8_t data)
{
	if (offset == 0x00)
	{
		m_superio_reg_sel = data;
	}
	else if (offset == 0x01)
	{
		m_superio_regs[m_superio_reg_sel] = data;
	}
}

/*
Write Data to the Driver Board Sequence.
Below describes the sequence of events on the parallel port to apply data to the port, clock it into the index register, then apply data to the port again, and clock it into the I/O register.

Write Parallel Port Data Register with Index Register Value. Value to select proper I/O register.
Write Parallel Port Control Register with a 4h (4d) to set Initialize bit High This clocks the index register high.
Write Parallel Port Control Register with a 0h (0d) to clear Initialize bit Low This clocks the index register LOW.
Write Parallel Port Data Register with I/O data Data that is required to be written to the selected I/O register.
Write Parallel Port Control Register with a 1h to set /strobe bit. This enables the index reg decode output to clock I/O register.
Write Parallel Port Control Register with a 0h to clear /strobe bit. This disables the index reg decode output to to clock I/O register.

Read Data from Board Sequence.
Below describes the sequence of events on the parallel port to apply data to the port, clock it into the index register, change direction of the parallel port & direction of buffer & enable index register decoded signal, read the port, change direction of parallel port & direction of buffer, and disable index register decoded signal.

Write Parallel Port Data Register with Index Register Data Value to select proper I/O register.
Write Parallel Port Control Register with a 4h (4d) to set Initialize bit High This clocks the index register high.
Write Parallel Port Control Register with a 0h (0d) to clear Initialize bit Low. This clocks the index register LOW.
Write Parallel Port Control Register with a 29h (41d) to change dir of parallel port and driver data buffer and decode enable sig. This changes the direction of the parallel port, changes the direction of Buffer and enables the index reg decode output.
Read Parallel Port for desired I/O data. Obtain the desired information.
Write Parallel Port Control Register with a 0h to change dir and enable sig. Reset the direction of the port and buffer as well as disable the index reg decode output.

Special Function Register and Controls.
Blanking.
Blanking is controlled through the switch column. By the action of strobing the switches the blanking circuit is held in the non-blanking condition allowing the I/O to function. If the switch matrix strobing of the columns is halted the blanking will be asserted in approximately 2.5 ms thus disabling all power I/O device drivers. The trigger to the blanking circuit is on any low transition of the index register #5 which is the switch column register clock.

The blanking circuit can be disabled during development by shorting the pins 1 & 3 on connector J1. This should be done under extreme care since any stalled operation of the I/O can cause electrical and/or game feature damage to occur.

This blanking signal is feed back to the parallel interface cable for use with future expansions on pin 26 of the parallel port cable connection.

Zero Cross.
Zero Cross occurs upon each transition of the AC line through 0 voltage. The initial detection circuit generates a pulse of approximate 1.5 ms. This pulse is feed to a synchronization circuit. This circuit latches the zero cross and is read through the index register #0F which is the Switch-System information register. The action of reading the zero cross will clear the latch and ready it for the next zero transition.

Lamp Test Mode.
There are two modes of testing the lamps. The first will allow the system to find any missing or burned out lamp positions. The second will allow the system to find any shorted lamp positions.

By setting the lamp test mode bit to a value of "0" will cause the lamp matrix to function in test mode for missing bulbs or burned out lamps. When the column is activated and the rows are activated any lamp that is missing within the activated column will be indicated by a value of a "1" in the corresponding lamptest register read. A required delay time from column and row activation to reading the lamp test register is 150uS.

By setting the lamp mode bit to a value of "1" will allow the lamp matrix to function normally as well as allow for indication of a shorted lamp position. This is done by reading the a value of "0" in the corresponding lamptest register read. A required delay time from column and row activation to reading the lamp test register is 300uS.

Fuse Test.
By reading the two register it can be determined if a fuse is blown out. Reading a high ("1") will indicate that a fuse is blown or missing. The relay must be energized to check fuses F100-F107 other wise these will appear to be blown.

Bit Description of I/O register.

00 Switch-Coin (Read)
D0-coin1 High indicates switch input closure to GND.
D1-coin2 "
D2-coin3 "
D3-coin4 "
D4-coin5 "
D5-coin6 "
D6-coin7 "
D7-coin8 "


01 Switch-Flipper (Read)
D0- Cabinet Flipper1 High indicates switch input closure to GND.
D1- Cabinet Flipper2 "
D2- Cabinet Flipper3 "
D3- Cabinet Flipper4 "
D4- Cabinet Flipper5 "
D5- Cabinet Flipper6 "
D6- Cabinet Flipper7 "
D7- Cabinet Flipper8 "

02 Switch-Dip (Read)
D0- Dip Switch1 High indicates switch input closure to GND.
D1- Dip Switch2 "
D2- Dip Switch3 "
D3- Dip Switch4 "
D4- Not Used Read back is high
D5- Not Used "
D6- Not Used "
D7- Not Used "


03 Switch-EOS/Diag. (Read)
D0- Diagnostic1 High indicates switch input closure to GND.
D1- Diagnostic2 "
D2- Diagnostic3 "
D3- Diagnostic4 "
D4- E.O.S.1 "
D5- E.O.S.2 "
D6- E.O.S.3 "
D7- E.O.S.4 "

04 Switch-Row (Read)
D0- ROW1 High indicates switch closure to active column
D1- ROW2 "
D2- ROW3 "
D3- ROW4 "
D4- ROW5 "
D5- ROW6 "
D6- ROW7 "
D7- ROW8 "


05 Switch-Col (Write) (Blanking trigger)
D0- COLUMN1 Low to active column
D1- COLUMN2 "
D2- COLUMN3 "
D3- COLUMN4 "
D4- COLUMN5 "
D5- COLUMN6 "
D6- COLUMN7 "
D7- COLUMN8 "


06 Lamp Row-A (Write)
D0- ROW1 High to activate row
D1- ROW2 "
D2- ROW3 "
D3- ROW4 "
D4- ROW5 "
D5- ROW6 "
D6- ROW7 "
D7- ROW8 "


07 Lamp Row-B (Write)
D0- ROW1 High to activate row
D1- ROW2 "
D2- ROW3 "
D3- ROW4 "
D4- ROW5 "
D5- ROW6 "
D6- ROW7 "
D7- ROW8 "


08 Lamp Col (Write)
D0- COLUMN1 High to activate column driver
D1- COLUMN2 "
D2- COLUMN3 "
D3- COLUMN4 "
D4- COLUMN5 "
D5- COLUMN6 "
D6- COLUMN7 "
D7- COLUMN8 "


09 Solenoid-C (Write)(no diode tie backs)
D0- Solenoid Opt 1 High to activate solenoid driver (output low)
D1- Solenoid Opt 2 "
D2- Solenoid Opt 3 "
D3- Solenoid Opt 4 "
D4- Solenoid Opt 5 "
D5- Solenoid Opt 6 "
D6- Solenoid Opt 7 "
D7- Solenoid Opt 8 "


0A Solenoid-B (Write)
D0- Solenoid B1 High to activate solenoid driver (output low)
D1- Solenoid B2 "
D2- Solenoid B3 "
D3- Solenoid B4 "
D4- Solenoid B5 "
D5- Solenoid B6 "
D6- Solenoid B7 "
D7- Solenoid B8 "


0B Solenoid-A (Write)
D0- Solenoid A1 High to activate solenoid driver (output low)
D1- Solenoid A2 "
D2- Solenoid A3 "
D3- Solenoid A4 "
D4- Solenoid A5 "
D5- Solenoid A6 "
D6- Solenoid A7 "
D7- Solenoid A8 "


0C Solenoid-Flipper (Write)
D0- Solenoid Flip1 High to activate solenoid driver (output low)
D1- Solenoid Flip2 "
D2- Solenoid Flip3 "
D3- Solenoid Flip4 "
D4- Solenoid Flip5 "
D5- Solenoid Flip6 "
D6- Solenoid Flip7 "
D7- Solenoid Flip8 "


0D Solenoid-D (Write)(no diode tie backs)
D0- Solenoid Flash 1 High to activate solenoid driver (output low)
D1- Solenoid Flash 2 "
D2- Solenoid Flash 3 "
D3- Solenoid Flash 4 "
D4- Health LED High turns LED on
D5- Power Relay Control High turns on relay
D6- Coin Counter High turns counter on
D7- Lamp Test Control High = use mode, Low = test mode


0E Solenoid-Logic (Write) (very low current sink)
D0- Solenoid Logic 1 High to activate logic driver (output low)
D1- Solenoid Logic 2 "
D2- Solenoid Logic 3 "
D3- Solenoid Logic 4 "
D4- Solenoid Logic 5 "
D5- Solenoid Logic 6 "
D6- Solenoid Logic 7 "
D7- Solenoid Logic 8 "


0F Switch-System (Read)
D0- Not Used Read back is high
D1- " "
D2- " "
D3- " "
D4- Ticket Notch Signal a notch in the ticket (For dispencer kit)
D5- Ticket Low Signal low level of tickets (For dispencer kit)
D6- Blanking High indicates outputs enabled
D7- Zero Cross LOW indicates Zero Cross has occurred.


10 Lamp matrix A diagnostic
D0- Row1
D1- Row2 In test mode a "1" will
D2- Row3 indicate a missing or burned
D3- Row4 out lamp.
D4- Row5
D5- Row6 In operation mode a "1" will
D6- Row7 indicate a shorted lamp position.
D7- Row8


11 Lamp matrix B diagnostic
D0- Row1
D1- Row2 In test mode a "1" will
D2- Row3 indicate a missing or burned
D3- Row4 out lamp.
D4- Row5
D5- Row6 In operation mode a "1" will
D6- Row7 indicate a shorted lamp position.
D7- Row8


12 Fuse A diagnostic
D0- Fuse 50V Flipper 1 (F104) High ("1") indicates blown fuse
D1- Fuse 50V Flipper 2 (F105) "
D2- Fuse 50V Flipper 3 (F106) "
D3- Fuse 50V Flipper 4 (F107) "
D4- Fuse 50V Solenoid1 (F103) "
D5- Fuse 50V Solenoid 2 (F102) "
D6- Fuse 50V Solenoid 3 (F101) "
D7- Fuse 50V Solenoid 4 (F100) "


13 Fuse B diagnostic
D0- Fuse 20V (F109) High ("1") indicates blown fuse
D1- Fuse 50V main (F110) "
D2- Fuse Lamp Matrix B (F112) "
D3- Fuse Lamp Matrix A (F111) "
D4- Not Used Read back is high
D5- " "
D6- " "
D7- " "
*/

uint8_t pinball2k_state::read_lpt(offs_t offset)
{
	uint8_t data;
	if (m_pdb_1 == 1 && m_pdb_2 == 0) {
		m_pdb_2 = 1;
		data = m_pdb[0];
	}
	else
		return 0;

	switch (offset)
	{
	case 0:
		switch (data)
		{
			case 0x00: //00 Switch-Coin (Read)
				return m_coininserted;
				break;
			case 0x01: //01 Switch-Flipper (Read)
				return m_cabinetswitch;
				//return 0x2;		//	1 = slam tilt switch 2 = coin door switch, plomb tilt, not used, right flipper, left flipper, right action, left action
				break;
			case 0x02: //02 Switch-Dip (Read)
				return 1;
				break;
			case 0x03: //03 Switch-EOS/Diag. (Read)
				return m_diagswitch;
				break;
			case 0x04: //04 Switch-Row (Read)
				if (m_switchcolumn == 0x1 && m_startbutton != 0) 
					return 0x4;
				else
					return 0;
				break;
			case 0x05:	//05 Switch-Col (Write) (Blanking trigger)
				return 0;
				break;
			case 0x06: // 06 Lamp Row-A (Write)
				return 0;
				break;
			case 0x07: // 07 Lamp Row-B (Write)
				return 0;
				break;
			case 0x08: // 08 Lamp Col (Write)
				return 0;
				break;
			case 0x09: // 09 Solenoid-C (Write)(no diode tie backs)
				return 0;
				break;
			case 0x0a: // 0A Solenoid-B (Write)
				return 0;
				break;
			case 0x0b: // 0B Solenoid-A (Write)
				return 0;
				break;
			case 0x0c: // 0C Solenoid-Flipper (Write)
				return 0xd;
				break;
			case 0x0d: // 0D Solenoid-D (Write)(no diode tie backs)
				return 0xe;
				break;
			case 0x0e: // 0E Solenoid-Logic (Write) (very low current sink)
				return 0xf;
				break;
			case 0x0f: // 0F Switch-System (Read)
				return 0x10;
				break;
			case 0x10: //10 Lamp matrix A diagnostic
				return 0x0;
				break;
			case 0x11: //11 Lamp matrix B diagnostic
				return 0x0;
				break;
			case 0x12: //12 Fuse A diagnostic
				return 0x0;
				break;
			case 0x13: //13 Fuse B diagnostic
				return 0x0;
				break;
			default:
				return 0xff;
		}
	case 1:
		return 0xff;
	case 2:
		return 0;
	}
	return 0;
}

void pinball2k_state::write_lpt(offs_t offset, uint8_t data)
{
	if (offset == 0) {
		if (m_pdb_1 == 0) {
			m_pdb_1 = 1;
			m_pdb_2 = 0;
		} else if (m_pdb_1 == 1 && m_pdb_2 == 0)
			m_pdb_2 = 1;
		else if (m_pdb_1 == 1 && m_pdb_2 == 1)
		{
			m_pdb_1 = 1;
			m_pdb_2 = 0;
		}
		if (m_pdb_1 == 1 && m_pdb_2 == 0)
			m_pdb[0] = data; // set data register
		if (m_pdb_2 == 1 && m_pdb[0] == 5)
			m_switchcolumn = data;
	}
}

uint8_t pinball2k_state::port208_r(offs_t offset)
{
	return 0x6;
}

void pinball2k_state::port208_w(offs_t offset, uint8_t data)
{
	logerror("%s: port208_w %08x = %02x\n", machine().describe_context(), offset, data);
}

uint8_t pinball2k_state::port278_r(offs_t offset)
{
	return 0x0;
}

void pinball2k_state::port278_w(offs_t offset, uint8_t data)
{
	logerror("%s: port278_w %08x = %02x\n", machine().describe_context(), offset, data);
}

uint8_t pinball2k_state::port378_r(offs_t offset)
{
	return 0x0;
}

void pinball2k_state::port378_w(offs_t offset, uint8_t data)
{
	logerror("%s: port378_w %08x = %02x\n", machine().describe_context(), offset, data);
}

uint32_t pinball2k_state::port400_r()
{
	return 0x8000;
}

void pinball2k_state::port400_w(uint32_t data)
{
}

uint32_t pinball2k_state::port800_r()
{
	return 0x80;
}

void pinball2k_state::port800_w(uint32_t data)
{
}

uint32_t pinball2k_state::prism_pci_r(int function, int reg, uint32_t mem_mask)
{
	return m_prism_regs[reg/4] & mem_mask;
}

void pinball2k_state::prism_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x10:
			if (data == 0xffffffff)
				m_prism_pci_regs.base_addr = 0xff000000;
			break;
		case 0x40:
			m_prism_pci_regs.init_enable = data;
			break;
	}
	COMBINE_DATA(&m_prism_regs[reg/4]);
}

uint32_t pinball2k_state::mediagx_pci_r(int function, int reg, uint32_t mem_mask)
{
	return m_mediagx_regs[reg] & mem_mask;
}

void pinball2k_state::mediagx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_mediagx_regs + reg);
}

uint32_t pinball2k_state::cx5520_pci_r(int function, int reg, uint32_t mem_mask)
{
	return m_cx5520_regs[reg] & mem_mask;
}

void pinball2k_state::cx5520_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_cx5520_regs + reg);
}

uint8_t pinball2k_state::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return m_pic2->acknowledge();
	else
		return 0x00;
}

/*****************************************************************************/

void pinball2k_state::mediagx_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram().share("main_ram");
	map(0x000a0000, 0x000affff).ram();
	map(0x000b0000, 0x000bffff).ram().share("cga_ram");
	map(0x000c0000, 0x000c7fff).rw(FUNC(pinball2k_state::expansion_r), FUNC(pinball2k_state::expansion_w)).share("expansion_ram");
	map(0x000c8000, 0x000cffff).ram();
	map(0x000d0000, 0x000fffff).ram().share("bios_ram");
	map(0x00100000, 0x0fffffff).ram();
	map(0x10000000, 0x1000007f).rw(FUNC(pinball2k_state::prism_1000_r), FUNC(pinball2k_state::prism_1000_w)).share("nvram2");
	map(0x11000000, 0x1102ffff).ram().share("nvram");
	map(0x12000000, 0x12ffffff).rw(FUNC(pinball2k_state::nvram_updates_r), FUNC(pinball2k_state::nvram_updates_w));
	map(0x13000000, 0x137fffff).rw(FUNC(pinball2k_state::dcs_read), FUNC(pinball2k_state::prism_1300_w)).share("dcs_data");
	map(0x14000000, 0x14ffffff).rw(FUNC(pinball2k_state::prism_1400_r), FUNC(pinball2k_state::prism_1400_w)).share("prism_bank0");
	map(0x15000000, 0x15ffffff).r(FUNC(pinball2k_state::prism_1500_r)).share("prism_bank1");
	map(0x16000000, 0x16ffffff).r(FUNC(pinball2k_state::prism_1600_r)).share("prism_bank2");
	map(0x17000000, 0x17ffffff).r(FUNC(pinball2k_state::prism_1700_r)).share("prism_bank3");
	map(0x18000000, 0x18ffffff).ram().share("prism_bank9");
	map(0x40000400, 0x40000fff).rw(FUNC(pinball2k_state::scratchpad_r), FUNC(pinball2k_state::scratchpad_w));
	map(0x40008000, 0x400080ff).rw(FUNC(pinball2k_state::biu_ctrl_r), FUNC(pinball2k_state::biu_ctrl_w));
	map(0x40008100, 0x400082ff).rw(FUNC(pinball2k_state::gx_pipeline_r), FUNC(pinball2k_state::gx_pipeline_w));
	map(0x40008300, 0x400083ff).rw(FUNC(pinball2k_state::disp_ctrl_r), FUNC(pinball2k_state::disp_ctrl_w));
	map(0x40008400, 0x400084ff).rw(FUNC(pinball2k_state::memory_ctrl_r), FUNC(pinball2k_state::memory_ctrl_w));
	map(0x40400000, 0x4047ffff).ram().share("smm");
	map(0x40800000, 0x40bfffff).ram().share("vram");

	map(0xf00c0000, 0xf00c7fff).r(FUNC(pinball2k_state::expansion_r));

	map(0xfffd0000, 0xffffffff).ram().share("system_bios1");    /* System BIOS */
}

void pinball2k_state::mediagx_io(address_map &map)
{
	// all in pcat32_io_common
	map(0x0000, 0x001f).rw(m_dma8237_1, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic1, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x006f).rw(m_kbdc, FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	// RTC
	map(0x0070, 0x0071).rw(m_rtc, FUNC(mc146818_device::read), FUNC(mc146818_device::write));

	map(0x0080, 0x009f).rw(FUNC(pinball2k_state::dma_page_select_r), FUNC(pinball2k_state::dma_page_select_w));//TODO
	map(0x00a0, 0x00a1).rw(m_pic2, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(m_dma8237_2, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask32(0x00ff00ff);

	// mediaGX port
	map(0x0022, 0x0023).rw(FUNC(pinball2k_state::io22_r), FUNC(pinball2k_state::io22_w));
	//map(0x0023, 0x0023).rw(FUNC(pinball2k_state::io23_r), FUNC(pinball2k_state::io23_w));

	map(0x0161, 0x0161).rw(FUNC(pinball2k_state::io61_r), FUNC(pinball2k_state::io61_w));
	map(0x00e8, 0x00eb).noprw();     // I/O delay port
	// IDE mapping
	map(0x0170, 0x0177).rw("ide2", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x0370, 0x0377).rw("ide2", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));

	// serial port mapping
	map(0x03f8, 0x03ff).rw(m_uart1, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x02f8, 0x02ff).rw(m_uart2, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	// Mainboard I use has this version of superIO PC937..
	map(0x02e, 0x02f).rw(FUNC(pinball2k_state::port2e_r), FUNC(pinball2k_state::port2e_w));
	// port 400 mapping
	map(0x0400, 0x0403).rw(FUNC(pinball2k_state::port400_r), FUNC(pinball2k_state::port400_w));
	map(0x0800, 0x0803).rw(FUNC(pinball2k_state::port800_r), FUNC(pinball2k_state::port800_w));
	// pci bus mapping
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));

	map(0x0208, 0x0208).rw(FUNC(pinball2k_state::port208_r), FUNC(pinball2k_state::port208_w));
	map(0x0278, 0x0278).rw(FUNC(pinball2k_state::port278_r), FUNC(pinball2k_state::port278_w));

	map(0x0378, 0x037b).rw("lpt_0", FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
	map(0x03bc, 0x03bf).rw(FUNC(pinball2k_state::read_lpt), FUNC(pinball2k_state::write_lpt)); //.umask16(0x00ff);
}

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,                    /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,
		4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_cga )
	// Support up to four CGA fonts
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout, 0, 256 )   // Font 0
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout, 0, 256 )   // Font 1
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout, 0, 256 )   // Font 2
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout, 0, 256 )   // Font 3
GFXDECODE_END


// key inputs for controlling switches etc
static INPUT_PORTS_START(mediagx)
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 3) 
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 4) 
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 5) 
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 10)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 113)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 115)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 116)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 117)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 118)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 120)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 121)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 122)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_CHANGED_MEMBER(DEVICE_SELF, pinball2k_state, kb_irq, 123)

INPUT_PORTS_END

void pinball2k_state::machine_start()
{
	m_nvram_updates = std::make_unique<uint8_t[]>(0x800000);
	subdevice<nvram_device>("nvram_updates")->set_base(m_nvram_updates.get(), 0x800000);
	save_pointer(NAME(m_nvram_updates), 0x800000);

	std::fill(std::begin(m_disp_ctrl_reg), std::end(m_disp_ctrl_reg), 0);
	std::fill(std::begin(m_biu_ctrl_reg), std::end(m_biu_ctrl_reg), 0);
}


void pinball2k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void pinball2k_state::init(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram_updates", nvram_device::DEFAULT_ALL_0);

	/* basic machine hardware */
	MEDIAGX(config, m_maincpu, 20000000); // should be 233MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &pinball2k_state::mediagx_map);
	m_maincpu->set_addrmap(AS_IO, &pinball2k_state::mediagx_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic1, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(pinball2k_state::get_slave_ack));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir2_w));
	m_pic2->in_sp_callback().set_constant(0);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device(0, FUNC(pinball2k_state::mediagx_pci_r), FUNC(pinball2k_state::mediagx_pci_w));
	pcibus.set_device(8, FUNC(pinball2k_state::prism_pci_r), FUNC(pinball2k_state::prism_pci_w));
	pcibus.set_device(18, FUNC(pinball2k_state::cx5520_pci_r), FUNC(pinball2k_state::cx5520_pci_w));

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set(m_pic2, FUNC(pic8259_device::ir6_w));
	ide_controller_32_device &ide2(IDE_CONTROLLER_32(config, "ide2").options(ata_devices, nullptr, nullptr, true));
	ide2.irq_handler().set(m_pic2, FUNC(pic8259_device::ir7_w));

	NS16550(config, m_uart1, XTAL(1'843'200));
	m_uart1->out_tx_callback().set("com1", FUNC(rs232_port_device::write_txd));
	m_uart1->out_dtr_callback().set("com1", FUNC(rs232_port_device::write_dtr));
	m_uart1->out_rts_callback().set("com1", FUNC(rs232_port_device::write_rts));
	m_uart1->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir4_w));

	rs232_port_device &com1(RS232_PORT(config, "com1", default_rs232_devices, nullptr));
	com1.rxd_handler().set(m_uart1, FUNC(ins8250_uart_device::rx_w));
	com1.dcd_handler().set(m_uart1, FUNC(ins8250_uart_device::dcd_w));
	com1.dsr_handler().set(m_uart1, FUNC(ins8250_uart_device::dsr_w));
	com1.ri_handler().set(m_uart1, FUNC(ins8250_uart_device::ri_w));
	com1.cts_handler().set(m_uart1, FUNC(ins8250_uart_device::cts_w));

	NS16550(config, m_uart2, XTAL(1'843'200));
	m_uart2->out_tx_callback().set("com2", FUNC(rs232_port_device::write_txd));
	m_uart2->out_dtr_callback().set("com2", FUNC(rs232_port_device::write_dtr));
	m_uart2->out_rts_callback().set("com2", FUNC(rs232_port_device::write_rts));
	m_uart2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir3_w));

	rs232_port_device &com2(RS232_PORT(config, "com2", default_rs232_devices, nullptr));
	com2.rxd_handler().set(m_uart2, FUNC(ins8250_uart_device::rx_w));
	com2.dcd_handler().set(m_uart2, FUNC(ins8250_uart_device::dcd_w));
	com2.dsr_handler().set(m_uart2, FUNC(ins8250_uart_device::dsr_w));
	com2.ri_handler().set(m_uart2, FUNC(ins8250_uart_device::ri_w));
	com2.cts_handler().set(m_uart2, FUNC(ins8250_uart_device::cts_w));

	/* printer */
	pc_lpt_device &lpt0(PC_LPT(config, "lpt_0"));
	lpt0.irq_handler().set(m_pic1, FUNC(pic8259_device::ir7_w));

	/* sound hardware */
	DCS2_AUDIO_2104_PIN2K(config, "dcs", 0);

	RAMDAC(config, m_ramdac, 0, m_palette);
	m_ramdac->set_addrmap(0, &pinball2k_state::ramdac_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(9830400, 640, 0, 640, 256, 0, 240);
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->set_screen_update(FUNC(pinball2k_state::screen_update_mediagx));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cga);
	PALETTE(config, m_palette).set_entries(256);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic2, FUNC(pic8259_device::ir0_w)); //using irq8
	m_rtc->set_binary(true);
	m_rtc->set_binary_year(true);
	m_rtc->set_epoch(1900);
	m_rtc->set_24hrs(true);

	// kbdc
	KBDC8042(config, m_kbdc, 0);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	// keyboard not enabled keys are captured using mame. When keyboard is enabled annoying screen is shown by pin 2000 software that keyboard is attached.
	//m_kbdc->system_reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	//m_kbdc->gate_a20_callback().set_inputline(m_maincpu, INPUT_LINE_A20);
	//m_kbdc->input_buffer_full_callback().set(m_pic1, FUNC(pic8259_device::ir1_w));

	PIT8253(config, m_pit, 0);
	//m_pit->set_clk<0>(4772720/4); // heartbeat IRQ
	m_pit->set_clk<0>(925000);
	m_pit->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(925000);
	m_pit->set_clk<2>(925000);
}

void pinball2k_state::mediagx(machine_config &config)
{
	init(config);
	AM9517A(config, m_dma8237_1, 14.318181_MHz_XTAL / 3);
	m_dma8237_1->out_hreq_callback().set(FUNC(pinball2k_state::pc_dma_hrq_changed));
	m_dma8237_1->in_memr_callback().set(FUNC(pinball2k_state::pc_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(pinball2k_state::pc_dma_write_byte));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(pinball2k_state::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(pinball2k_state::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(pinball2k_state::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(pinball2k_state::pc_dack3_w));

	AM9517A(config, m_dma8237_2, 14.318181_MHz_XTAL / 3);
}

void pinball2k_state::init_mediagx()
{
	m_frame_width = m_frame_height = 1;
}

void pinball2k_state::init_pinball2k()
{
	init_mediagx();
}

void pinball2k_state::init_swe1pb()
{
	#define bootnormal 1
	init_mediagx();
}

void pinball2k_state::machine_reset()
{
	m_system_bios1[0xbffc] = 0x03ea; // set the jump
	m_system_bios1[0xbffd] = 0xc0;   // to the boot loader

	m_scanline_timer->adjust(m_screen->scan_period());

	if (m_nvram_updates[0] == 0x00 && m_nvram_updates[1] == 0x00)
	{
		int c = 0;
		for (u32 i=0; i < 0x20000; i+=4,++c)
		{
			m_nvram_updates[0x7e0000 + i] = char(m_cmos[0x900 + c]);
			m_nvram_updates[0x7e0000 + i + 1] = char(m_cmos[0x900 + c] >> 8);
			m_nvram_updates[0x7e0000 + i + 2] = char(m_cmos[0x900 + c] >> 16);
			m_nvram_updates[0x7e0000 + i + 3] = char(m_cmos[0x900 + c] >> 24);
		}
	}
	// plx eeprom has data?
	if (m_eeprom[0] == 0x00 && m_eeprom[1] == 0x00)
	{
		m_eeprom[0x0] = 0x0001146e;
		m_eeprom[0x1] = 0x03000000;
		m_eeprom[0x2] = 0x0;
		m_eeprom[0x3] = 0x0;
		m_eeprom[0x4] = 0xFFE0000;
		m_eeprom[0x5] = 0xF800000;
		m_eeprom[0x6] = 0x0FFF8000;
		m_eeprom[0x7] = 0x0C000008;
		m_eeprom[0x8] = 0x0FFF8001;
		m_eeprom[0x9] = 0x00100001;
		m_eeprom[0xa] = 0x01000001;
		m_eeprom[0xb] = 0x00000001;
		m_eeprom[0xc] = 0x08000001;
		m_eeprom[0xd] = 0x08000000;
		m_eeprom[0xe] = 0x5403A1E0;
		m_eeprom[0xf] = 0x5473B940;
		m_eeprom[0x10] = 0x4041A060;
		m_eeprom[0x11] = 0x54B2B8C0;
		m_eeprom[0x12] = 0x54B2B8C0;
		m_eeprom[0x13] = 0x08800001;
		m_eeprom[0x14] = 0x09800001;
		m_eeprom[0x15] = 0x0A800001;
		m_eeprom[0x16] = 0x0B800001;
		m_eeprom[0x17] = 0;
		m_eeprom[0x18] = 0x00789242;
	}

	for (int i = 0; i< 32; i++)
		m_eeprom_regs[i] = m_eeprom[i + 4];

	m_prism_regs[0] = 0x0001146E;
	m_prism_regs[4] = 0x02800002;
	m_prism_regs[8] = 0x03000002;

	m_mediagx_regs[0] = 0x00011078;
	m_mediagx_regs[4] = 0x02800002;
	m_mediagx_regs[8] = 0x06000000;
	m_mediagx_regs[0x40] = 0x80009600;

	m_cx5520_regs[0] = 0x00021078;
	m_cx5520_regs[4] = 0x02800002;
	m_cx5520_regs[8] = 0x06010000;

	m_1200_mode = 0;

	m_startIRQ = 0;
	reset_scanline_timer();

	m_superio_regs[0x20] = 0xDF;
	m_superio_regs[0x21] = 0x1;
	m_mediagx_config_regs[0xc1] = 0x2;		// CCR1
	m_mediagx_config_regs[0xc2] = 0x56;		// CCR2
	m_mediagx_config_regs[0xc3] = 0xf5;		// CCR3
	m_mediagx_config_regs[0xe8] = 0xd8;		// CCR4
	m_mediagx_config_regs[0xeB] = 0x0;		// CCR7
	m_mediagx_config_regs[0x20] = 0x0;		// PCR
	m_mediagx_config_regs[0xb0] = 0x0;		// SMHR0
	m_mediagx_config_regs[0xb1] = 0x0;		// SMHR1
	m_mediagx_config_regs[0xb2] = 0x41;		// SMHR2
	m_mediagx_config_regs[0xb3] = 0x40;		// SMHR3
	m_mediagx_config_regs[0xb8] = 0x9;		// gcr
	m_mediagx_config_regs[0xb9] = 0x0;		// VGACTL
	m_mediagx_config_regs[0xba] = 0x0;		// VGAM0
	m_mediagx_config_regs[0xbb] = 0x0;		// VGAM1
	m_mediagx_config_regs[0xbc] = 0x0;		// VGAM2
	m_mediagx_config_regs[0xbd] = 0x0;		// VGAM3
	m_mediagx_config_regs[0xcd] = 0x40;		// smar0
	m_mediagx_config_regs[0xce] = 0x40;		// smar1
	m_mediagx_config_regs[0xcf] = 0x6;		// smar2
	m_mediagx_config_regs[0xfe] = 0x46;		// DIR0
	m_mediagx_config_regs[0xff] = 0x63;		// DIR1
	// memory control registers init
	m_memory_ctrl_reg[0] = 0xb696390c;
	m_memory_ctrl_reg[1] = 0x21;
	m_memory_ctrl_reg[2] = 0x700110;
	m_memory_ctrl_reg[3] = 0x37533110;
	m_memory_ctrl_reg[4] = 0x37533110;
	m_memory_ctrl_reg[5] = 0x8;
	m_memory_ctrl_reg[9] = 0x8;
	//
	m_disp_ctrl_reg[DC_V_TIMING_1] = 0x190400ef;
	m_disp_ctrl_reg[DC_V_TIMING_2] = 0x190400ef;
	m_disp_ctrl_reg[DC_V_TIMING_3] = 0x18f300f0;
	m_disp_ctrl_reg[DC_H_TIMING_1] = 0x1e4004f8;
	m_disp_ctrl_reg[DC_H_TIMING_2] = 0x1e4004f8;
	m_disp_ctrl_reg[DC_H_TIMING_3] = 0x1dc00540;
	m_gfx_pipeline_reg[GP_BLT_STATUS] = 0x0;
	// internal bus interface unit registers
	m_biu_ctrl_reg[0] = 0x3fffff;
	m_biu_ctrl_reg[1] = 0x60;
	m_biu_ctrl_reg[2] = 0x11;
	m_biu_ctrl_reg[3] = 0x11111111;
	m_maincpu->reset();
}

/*****************************************************************************/

ROM_START( swe1pb )
	ROM_REGION32_LE(0x1000000, "prismdata1", 0)
	ROM_LOAD32_WORD( "swe1_u100.rom", 0x0000000, 0x800000, CRC(db2c9709) SHA1(14e8db2c0b09c4da6306a4a1f7fe54b2a334c5ed) )
	ROM_LOAD32_WORD( "swe1_u101.rom", 0x0000002, 0x800000, CRC(a039e80d) SHA1(8f63e8ab83e043232fc17ed3dff1f251396a178a) )
	ROM_FILL(0x191,1, 0x90)

	ROM_FILL(0x3b33,1,0x1)
	ROM_FILL(0x3b34,1,0x0)
	ROM_FILL(0x3b35,1,0x0)
	ROM_FILL(0x3b36,1,0x0)

	ROM_REGION32_LE(0x1000000, "prismdata2", 0)
	ROM_LOAD32_WORD( "swe1_u102.rom", 0x00000, 0x800000, CRC(c9feb7bc) SHA1(a34acd34c3f91f082b67e385b1f4da2e5b6e5087) )
	ROM_LOAD32_WORD( "swe1_u103.rom", 0x00002, 0x800000, CRC(7a692466) SHA1(9adf5ae9c12bd5b6314913f6c01d4566ee453fe1) )
	ROM_REGION32_LE(0x1000000, "prismdata3", 0)
	ROM_LOAD32_WORD( "swe1_u104.rom", 0x00000, 0x800000, CRC(76e2dd7e) SHA1(9bc20a1423b11c46eb2f5a514e985151defb5651) )
	ROM_LOAD32_WORD( "swe1_u105.rom", 0x00002, 0x800000, CRC(87f2460c) SHA1(cdc05e017367f61280e3d5682096e67e4c200150) )
	ROM_REGION32_LE(0x1000000, "prismdata4", 0)
	ROM_LOAD32_WORD( "swe1_u106.rom", 0x00000, 0x800000, CRC(84877e2f) SHA1(6dd8c761b2e26313ae9e159690b3a4a170cb3bd8) )
	ROM_LOAD32_WORD( "swe1_u107.rom", 0x00002, 0x800000, CRC(dc433c89) SHA1(9f1273debc9168c04202078503cfc4f1ca8cb30b) )

	ROM_REGION(0xC00000, "dcs", ROMREGION_ERASEFF) 
	ROM_LOAD( "28f800.bin", 0, 0x100000, CRC(5FC1FD2C) SHA1 (0967DB9B6E82D386D3A8415BBEF40BCAB5A06654) )
	ROM_LOAD( "swe1_u109.rom", 0x400000, 0x400000, CRC(cc08936b) SHA1(fc428393e8a0cf37b800dd475fd293a1a98c4bcf) )
	ROM_LOAD( "swe1_u110.rom", 0x800000, 0x400000, CRC(6011ecd9) SHA1(8575958c8942a6cbcb2ac18f291fcada6f8cbc09) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD( "cga.chr",       0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END

ROM_START( rfmpb )
	ROM_REGION32_LE(0x1000000, "prismdata1", 0)
	ROM_LOAD32_WORD( "rfm_u100.rom", 0x0000000, 0x800000, CRC(b3548b1b) SHA1(874a16282bb778886cea2567d68ec7024dc5ed22) )
	ROM_LOAD32_WORD( "rfm_u101.rom", 0x0000002, 0x800000, CRC(8bef301d) SHA1(2eade00b1a4cd3f5e98ebe8ed8f549e328188e77) )
	ROM_FILL(0x191,1, 0x90)

	ROM_FILL(0x419a,1,0x1)
	ROM_FILL(0x419b,1,0x0)
	ROM_FILL(0x419c,1,0x0)
	ROM_FILL(0x419d,1,0x0)
	ROM_REGION32_LE(0x1000000, "prismdata2", 0)
	ROM_LOAD32_WORD( "rfm_u102.rom", 0x00000, 0x800000, CRC(749f5c59) SHA1(2d8850e7f8ea3e07e8b444d7dd4dc4195a547ae7) )
	ROM_LOAD32_WORD( "rfm_u103.rom", 0x00002, 0x800000, CRC(a9ec5e97) SHA1(ce7c38dcbf34ce10d6e204a3176cd2c7a83b525a) )
	ROM_REGION32_LE(0x1000000, "prismdata3", 0)
	ROM_LOAD32_WORD( "rfm_u104.rom", 0x00000, 0x800000, CRC(0a1acd70) SHA1(dcca4de92eadeb82ac776953326410a9687838cb) )
	ROM_LOAD32_WORD( "rfm_u105.rom", 0x00002, 0x800000, CRC(1ef31684) SHA1(141900a7426ad483384606cddb018d186952f439) )
	ROM_REGION32_LE(0x1000000, "prismdata4", 0)
	ROM_LOAD32_WORD( "rfm_u106.rom", 0x00000, 0x800000, CRC(daf4e1dc) SHA1(0612495468fb962b833057e50f620c5f69cd5840) )
	ROM_LOAD32_WORD( "rfm_u107.rom", 0x00002, 0x800000, CRC(e737ab39) SHA1(0e978923db19e2893fdb4aae69d6ed3c3f664a31) )

	ROM_REGION(0xC00000, "dcs", ROMREGION_ERASEFF) 
	ROM_LOAD( "28f800.bin", 0, 0x100000, CRC(A57C55AD) SHA1 (60EE230B8978B7C5F1482B1B587D1C6DB5FDD20E) )
	ROM_LOAD( "rfm_u109.rom",  0x400000, 0x400000, CRC(385F1255) SHA1(0A3BE261CD35CD153EFF95335597BCA46B760568) )
	ROM_LOAD( "rfm_u110.rom",  0x800000, 0x400000, CRC(2258DBDE) SHA1(0C9E62E45FA7CC03AEDD43A6E06FEE28B2F288A5) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD( "cga.chr",       0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END

ROM_START( rfmpbr2 )
	ROM_REGION32_LE(0x1000000, "prismdata1", 0)
	ROM_LOAD32_WORD( "rfm_u100r2.rom", 0x0000000, 0x800000, CRC(d4278a9b) SHA1(ec07b97190acb6b34b9ed6cda505ee8fefd66fec) )
	ROM_LOAD32_WORD( "rfm_u101r2.rom", 0x0000002, 0x800000, CRC(e5d4c0ed) SHA1(cfc7d9d2324cc02c9eaf53fd674f7db24736699c) )
	ROM_FILL(0x191,1, 0x90)

	ROM_FILL(0x419a,1,0x1)
	ROM_FILL(0x419b,1,0x0)
	ROM_FILL(0x419c,1,0x0)
	ROM_FILL(0x419d,1,0x0)
	ROM_REGION32_LE(0x1000000, "prismdata2", 0)
	ROM_LOAD32_WORD( "rfm_u102.rom", 0x00000, 0x800000, CRC(749f5c59) SHA1(2d8850e7f8ea3e07e8b444d7dd4dc4195a547ae7) )
	ROM_LOAD32_WORD( "rfm_u103.rom", 0x00002, 0x800000, CRC(a9ec5e97) SHA1(ce7c38dcbf34ce10d6e204a3176cd2c7a83b525a) )
	ROM_REGION32_LE(0x1000000, "prismdata3", 0)
	ROM_LOAD32_WORD( "rfm_u104.rom", 0x00000, 0x800000, CRC(0a1acd70) SHA1(dcca4de92eadeb82ac776953326410a9687838cb) )
	ROM_LOAD32_WORD( "rfm_u105.rom", 0x00002, 0x800000, CRC(1ef31684) SHA1(141900a7426ad483384606cddb018d186952f439) )
	ROM_REGION32_LE(0x1000000, "prismdata4", 0)
	ROM_LOAD32_WORD( "rfm_u106.rom", 0x00000, 0x800000, CRC(daf4e1dc) SHA1(0612495468fb962b833057e50f620c5f69cd5840) )
	ROM_LOAD32_WORD( "rfm_u107.rom", 0x00002, 0x800000, CRC(e737ab39) SHA1(0e978923db19e2893fdb4aae69d6ed3c3f664a31) )

	ROM_REGION(0xc00000, "dcs", ROMREGION_ERASEFF) 
	ROM_LOAD( "28f800.bin", 0, 0x100000, CRC(5FC1FD2C) SHA1 (0967DB9B6E82D386D3A8415BBEF40BCAB5A06654) )
	ROM_LOAD( "rfm_u109.rom",  0x400000, 0x400000, CRC(385F1255) SHA1(0A3BE261CD35CD153EFF95335597BCA46B760568) )
	ROM_LOAD( "rfm_u110.rom",  0x800000, 0x400000, CRC(2258DBDE) SHA1(0C9E62E45FA7CC03AEDD43A6E06FEE28B2F288A5) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD( "cga.chr",       0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END

} // Anonymous namespace

/*****************************************************************************/

GAME( 1999, swe1pb,   0       , mediagx, mediagx, pinball2k_state, init_swe1pb, ROT0,   "Midway",  "Pinball 2000: Star Wars Episode 1", MACHINE_MECHANICAL )
GAME( 1999, rfmpb,    0       , mediagx, mediagx, pinball2k_state, init_pinball2k, ROT0,   "Midway",  "Pinball 2000: Revenge From Mars (rev. 1)", MACHINE_MECHANICAL )
GAME( 1999, rfmpbr2,  rfmpb   , mediagx, mediagx, pinball2k_state, init_pinball2k, ROT0,   "Midway",  "Pinball 2000: Revenge From Mars (rev. 2)", MACHINE_MECHANICAL )
