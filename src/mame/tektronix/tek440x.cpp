// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

    Tektronix 440x "AI Workstations"

    skeleton by R. Belmont

    Hardware overview:
        * 68010 (4404) or 68020 (4405) with custom MMU
        * Intelligent floppy subsystem with 6502 driving a uPD765 controller
        * NS32081 FPU
        * 6551 debug console AICA
        * SN76496 PSG for sound
        * MC146818 RTC
        * MC68681 DUART / timer (3.6864 MHz clock) (serial channel A = keyboard, channel B = RS-232 port)
        * AM9513 timer (source of timer IRQ)
        * NCR5385 SCSI controller
		* 8255 Centronics printer interface
				
        Video is a 640x480 1bpp window on a 1024x1024 VRAM area; smooth panning around that area
        is possible as is flat-out changing the scanout address.

    IRQ levels:	(see Figure 2.1-8)
        7 = Debug (NMI)
        6 = VBL
        5 = UART
        4 = Spare (exp slots)
        3 = SCSI
        2 = DMA
        1 = Timer	(and Printer)
        0 = Unused

    MMU info:
        Map control register (location 0x780000): bit 5 = Wenable, bit 4 = VMenable, bits3-0 process ID

        Map entries:
            bit 15 = dirty
            bit 14 = write enable
            bit 13-11 = process ID
            bits 10-0 = address bits 22-12 in the final address

***************************************************************************/

#include "emu.h"

#include "tek410x_kbd.h"
#include "tek_msu_fdc.h"

#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "machine/am9513.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/mos6551.h"    // debug tty
#include "machine/ncr5385.h"
#include "machine/ns32081.h"
#include "machine/i8255.h"
#include "machine/nscsi_bus.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "tek4404.lh"

#define LOG_GENERAL (1U << 0)
#define LOG_MMU (1U << 1)
#define LOG_FPU (1U << 2)

#define VERBOSE 1
#include "logmacro.h"

// mapcntl bits
#define MAP_VM_ENABLE 4
#define MAP_SYS_WR_ENABLE 5
// mapcntrl result bits
#define MAP_BLOCK_ACCESS 6
#define MAP_CPU_WR 7

#define OFF8_TO_OFF16(A)	((A)>>1)
#define OFF16_TO_OFF8(A)	((A)<<1)

#define MAXRAM 0x200000	// +1MB
//#define MAXRAM 0x400000	// +3MB (which was never a Thing)

// have m_readXX / m_writeXX use MMU translation
// OR
// do MMU translation inside memory_r / memory_w
#define USE_MMUxx

// pagetable as internal
#define USE_INTERNAL_MAPxx

class m68010_tekmmu_device : public m68010_device
{
	using m68010_device::m68010_device;

	// HACK
	u8 *m_map_control_ptr = nullptr;
	u16 *m_map_ptr = nullptr;


#ifdef USE_INTERNAL_MAP
	u8 m_map_control;
	u8 m_latched_map_control;		// latched until user mode access (page 2.1-54)
	memory_share_creator<u16> m_map;

	u16 map_r(offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			if ((offset>>11) < 0x20)
				LOG("map_r 0x%08x => %04x\n",offset>>11, m_map[(offset >> 11)&0x7ff] );

			// selftest does a read and expects it to fail iff !MAP_SYS_WR_ENABLE; its not WR enable, its enable..
			// NB page 2.1-52 shows WrMapEn coming from latch
			if (!BIT(m_latched_map_control, MAP_SYS_WR_ENABLE))
			{
					LOG("map_r: bus error: PID(%d) %08x fc(%d) pc(%08x)\n", BIT(m_map[(offset >> 11) & 0x7ff], 11, 3), OFF16_TO_OFF8(offset), get_fc(), pc());
					set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
					set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
					set_buserror_details(offset, 0, get_fc());
					return 0;
			}
		}
		
		return m_map[(offset >> 11) & 0x7ff];
	}

	void map_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if ((offset>>11) < 0x20)
		{
			LOG("map_w: %08x  <= %04x paddr(%08x) PID(%d) dirty(%d) write_enable(%d)\n",
				(offset >> 11) & 0x7ff, data,
				OFF16_TO_OFF8(BIT(data, 0, 11)<<11),BIT(data, 11, 3), data & 0x8000 ? 1 : 0, data & 0x4000 ? 1 : 0,
				pc());
		}
		
		// NB page 2.1-52 shows WrMapEn coming from latch
		if (BIT(m_latched_map_control, MAP_SYS_WR_ENABLE))
		{
			COMBINE_DATA(&m_map[(offset >> 11) & 0x7ff]);
		}
	}

	u8 mapcntl_r()
	{
		if (!machine().side_effects_disabled())
		{
			// page 2.1-54 implies that this can only be read in user mode

			LOGMASKED(LOG_MMU, "mapcntl_r(%02x) cpuWr(%d) BlockAccess(%d) SysWrEn(%d) PID(%d) pc(%08x)\n",m_map_control,
				BIT(m_map_control, MAP_CPU_WR) ? 1 : 0,
				BIT(m_map_control, MAP_BLOCK_ACCESS) ? 1 : 0,
				BIT(m_map_control, MAP_SYS_WR_ENABLE) ? 1 : 0,
				m_map_control & 15,
				pc());
		}
		
		u8 ans = m_map_control;
		
		// mask out 'SysWrEn'
		// 0xc0 means 'not blocked write' aka successful write
		if ((ans & 0xc0) != 0xc0)
			ans &= ~(1<<MAP_SYS_WR_ENABLE);

		return ans;
	}

	void mapcntl_w(u8 data)
	{
		// copied on user mode read/write
		m_latched_map_control = data & 0x3f;
		
		if (m_map_control != m_latched_map_control)
		{
			LOGMASKED(LOG_MMU, "mapcntl_w mmu_enable   %2d pc(%8x)\n", BIT(data, MAP_VM_ENABLE),  pc());
			LOGMASKED(LOG_MMU, "mapcntl_w write_enable %2d\n", BIT(data, MAP_SYS_WR_ENABLE));
			LOGMASKED(LOG_MMU, "mapcntl_w pte PID      %2d\n", data & 15);

			if (BIT(data, MAP_VM_ENABLE) && (data & 15))
			for(uint32_t i=0; i<2048; i++)
			{
				if (m_map[i])
				LOGMASKED(LOG_MMU, "mapcntl_w: %08x -> paddr(%08x) PID(%d) dirty(%d) write_enable(%d)\n",
					OFF16_TO_OFF8(i << 11), OFF16_TO_OFF8(BIT(m_map[i], 0, 11)<<11),
					BIT(m_map[i], 11, 3), m_map[i] & 0x8000 ? 1 : 0, m_map[i] & 0x4000 ? 1 : 0);
			}

		}

		// NB bit 6 & 7 is not used
		
		// disable using latched state for now
		//m_map_control = data & 0x3f;
		
	}


	// FIXME Phil says use internal_map
	// need to have ctor that can call m68000_musashi_device() with our internal_map
	void cpu_internal_map(address_map &map)
	{
		map(0x780000, 0x780000).rw(FUNC(m68010_tekmmu_device::mapcntl_r), FUNC(m68010_tekmmu_device::mapcntl_w));
		map(0x800000, 0xffffff).rw(FUNC(m68010_tekmmu_device::map_r), FUNC(m68010_tekmmu_device::map_w));
	}

	m68010_tekmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		m68010_device::m68010_device(mconfig, tag, owner, clock, address_map_constructor(FUNC(m68010_tekmmu_device::cpu_internal_map),this)),
		m_map(*this, "map", 0x1000, ENDIANNESS_BIG)		// 2k 16-bit entries
	{
	}
#endif

public:
	// device_memory_interface overrides
	bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override
	{
	
	target_space = &space(spacenum);

		//if (m_emmu_enabled)
		if (spacenum == AS_PROGRAM)
		if (!supervisor_mode())		// only in User mode
		if (BIT(*m_map_control_ptr, MAP_VM_ENABLE))
		{
			if (intention == TR_WRITE)
			if (BIT(m_map_ptr[address >> 12], 14) == 0)	// read only
			{
				LOG("memory_translate: write fail\n");
				return false;
			}

			if (BIT(m_map_ptr[address >> 12], 11, 3) != (*m_map_control_ptr & 7))
			{
				return false;
			}
			
			// dont try and translate a null page
			if (BIT(m_map_ptr[address >> 12], 11, 3) != 0)
			{
				//LOG("memory_translate: map %08x => paddr(%08x)\n",(address), (BIT(address, 0, 12) | (BIT(m_map_ptr[address >> 12], 0, 11) << 12) ) );
			
				address = BIT(address, 0, 12) | (BIT(m_map_ptr[address >> 12], 0, 11) << 12);
			}
			else
			{
				return false;
			}
		}
		
		return true;
	}

	u32 mmu_translate_address(offs_t address, u8 rw, u8 fc, u8 sz)
	{
		m_mmu_tmp_rw = rw;
		m_mmu_tmp_fc = fc;
		m_mmu_tmp_sz = sz;

		if (!m_mmu_tmp_buserror_occurred && ((fc & 4) == 0) && (BIT(*m_map_control_ptr, MAP_VM_ENABLE)))
		{
			LOG("mmu_translate_address: map %08x => paddr(%08x) fc(%d) pc(%08x)\n",(address), BIT(address, 0, 12) | (BIT(m_map_ptr[address >> 12], 0, 11) << 12), fc, pc());

			if (rw)
			{
				// is !cpuWr
				*m_map_control_ptr &= ~(1 << MAP_CPU_WR);
			}
			else
			{
				// is cpuWr
				*m_map_control_ptr |= (1 << MAP_CPU_WR);
			}

			// matching pid
			if (BIT(m_map_ptr[address >> 12], 11, 3) != (*m_map_control_ptr & 7))
			{
				*m_map_control_ptr &= ~(1 << MAP_BLOCK_ACCESS);

				LOG("mmu_translate_address: bus error: PID(%d) != %d %08x fc(%d) pc(%08x)\n", BIT(m_map_ptr[address >> 12], 11, 3), (*m_map_control_ptr & 7), address, fc, pc());
				set_buserror_details(address, rw, fc, true);
				restart_this_instruction();
				set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
				set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
				return address;
			}
			else
			{
				*m_map_control_ptr |= (1 << MAP_BLOCK_ACCESS);
			}

			// write enabled?
			if ((rw==0) && BIT(m_map_ptr[address >> 12], 14) == 0)	// read only page
			{
				*m_map_control_ptr &= ~(1 << MAP_BLOCK_ACCESS);

				LOG("mmu_translate_address: bus error: READONLY %08x fc(%d) pc(%08x)\n",(address), fc, pc());
				set_buserror_details(address, rw, fc, false);
				set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
				set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
				return address;
			}

			// mark page dirty
			if (rw==0)
			{
				m_map_ptr[address >> 12] |= 0x8000;
				LOG("mmu_translate_address: DIRTY m_map_ptr(0x%04x) m_map_control(%02x)\n", m_map_ptr[address >> 12], m_map_control_ptr);
			}

			address = BIT(address, 0, 12) | (BIT(m_map_ptr[address >> 12], 0, 11) << 12);
		}

		// is there memory here?
		if (address >= MAXRAM && address < 0x600000)
		{
			LOG("mmu_translate_address: bus error: NOMEM %08x fc(%d)\n", address, fc);
			set_buserror_details(address, rw, fc,false);
			set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		}

		return address;
	}

	u16 PTEread(offs_t address)
	{
		// selftest does a read and expects it to fail iff !MAP_SYS_WR_ENABLE; its not WR enable, its enable..
		if (!BIT(*m_map_control_ptr, MAP_SYS_WR_ENABLE))
		{
				LOG("PTEread: bus error: PID(%d) %08x fc(%d) pc(%08x)\n", BIT(m_map_ptr[(address >> 12) & 0x7ff], 11, 3), (address), get_fc(), pc());
				set_buserror_details(address, 1, get_fc(), false);
				set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
				set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
				return 0;
		}

		return m_map_ptr[(address>>12) & 0x7ff];
	}

	void PTEwrite(offs_t address, u16 data)
	{
		if (((address>>12) & 0x7ff) < 20)
			LOG("PTEwrite: %08x  <= %04x paddr(%08x) PID(%d) dirty(%d) write_enable(%d)\n",
			(address>>12) & 0x7ff, data,
			(BIT(data, 0, 11)<<12), BIT(data, 11, 3), data & 0x8000 ? 1 : 0, data & 0x4000 ? 1 : 0);

		if (BIT(*m_map_control_ptr, MAP_SYS_WR_ENABLE))
		{
			m_map_ptr[(address>>12) & 0x7ff] = data;
		}
	}

#ifdef USE_MMU
	void init16(address_space &space, address_space &ospace)
#else
	void init16XXXXXX(address_space &space, address_space &ospace)
#endif
	{
		LOG("m68010_tekmmu_device::init16: \n");
		m_space = &space;
		m_ospace = &ospace;
		ospace.cache(m_oprogram16);
		space.specific(m_program16);

		m_readimm16 = [this](offs_t address) -> u16  {
			if (address & 0x800000)
				return PTEread(address);		// needed?
			else
			{
				u32 address0 = mmu_translate_address(address, 1, m_s_flag | FUNCTION_CODE_USER_PROGRAM, M68K_SZ_WORD);
				if (m_mmu_tmp_buserror_occurred)
					return ~0;
				return m_oprogram16.read_word(address0);
			}
		};
		m_read8   = [this](offs_t address) -> u8     {
			u32 address0 = mmu_translate_address(address, 1, m_s_flag, M68K_SZ_BYTE);
			if (m_mmu_tmp_buserror_occurred)
				return ~0;
			return m_program16.read_byte(address0);
		};
		m_read16  = [this](offs_t address) -> u16    {
			if (address & 0x800000)
				return PTEread(address);
			else
			{
				u32 address0 = mmu_translate_address(address, 1, m_s_flag, M68K_SZ_WORD);
				if (m_mmu_tmp_buserror_occurred)
					return ~0;
				return m_program16.read_word(address0);
			}
		};
		m_read32  = [this](offs_t address) -> u32    {
			u32 address0 = mmu_translate_address(address, 1, m_s_flag, M68K_SZ_LONG);
			if (m_mmu_tmp_buserror_occurred)
				return ~0;
			return m_program16.read_dword(address0);
		};
		
		m_write8  = [this](offs_t address, u8 data)  {
			u32 address0 = mmu_translate_address(address, 0, m_s_flag, M68K_SZ_BYTE);
			if (m_mmu_tmp_buserror_occurred)
				return;
			m_program16.write_word(address0 & ~1, data | (data << 8), address0 & 1 ? 0x00ff : 0xff00);
		};
		m_write16 = [this](offs_t address, u16 data) {
			if (address & 0x800000)
				PTEwrite(address, data);
			else
			{
				u32 address0 = mmu_translate_address(address, 0, m_s_flag, M68K_SZ_WORD);
				if (m_mmu_tmp_buserror_occurred)
					return;
				m_program16.write_word(address0, data);
			}
		};
		m_write32 = [this](offs_t address, u32 data) {
			if (address & 0x800000)
				LOG("m_write32 to PTE!!!!!\n");
			else
			{
				u32 address0 = mmu_translate_address(address, 0, m_s_flag, M68K_SZ_LONG);
				if (m_mmu_tmp_buserror_occurred)
					return;
				m_program16.write_dword(address0, data);
			}
		};
	}

	void init_cpu_m68010(void)
	{
		LOG("m68010_tekmmu_device::init_cpu_m68010\n");
		init_cpu_common();
		m_cpu_type         = CPU_TYPE_010;

		init16(*m_program, *m_oprogram);
		m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
		m_state_table      = m68ki_instruction_state_table[2];
		m_cyc_instruction  = m68ki_cycles[2];
		m_cyc_exception    = m68ki_exception_cycle_table[2];
		m_cyc_bcc_notake_b = -4;
		m_cyc_bcc_notake_w = 0;
		m_cyc_dbcc_f_noexp = 0;
		m_cyc_dbcc_f_exp   = 6;
		m_cyc_scc_r_true   = 0;
		m_cyc_movem_w      = 4;
		m_cyc_movem_l      = 8;
		m_cyc_shift        = 2;
		m_cyc_reset        = 130;
		m_has_pmmu         = 0;
		m_has_fpu          = 0;

		define_state();
	}

	void device_start() override
	{
		//we need to override init_cpu_m68010 so replacing call to m68010_device::device_start();
		m68000_musashi_device::device_start();
		init_cpu_m68010();
	}
	void device_reset() override
	{
		m68000_musashi_device::device_reset();
		LOG("m68010_tekmmu_device::device_reset: setting emmu\n");
		set_emmu_enable(true);		// sets m_instruction_restart=true
	}

public:
	void linktoMMU(u8 *map_control, u16 *map)
	{
		LOG("m68010_tekmmu_device::linktoMMU: control(%p) map(%p) m_emmu_enabled(%d)\n",map_control, map, m_emmu_enabled);
		
		m_map_control_ptr = map_control;
		m_map_ptr = map;
	}

};
DECLARE_DEVICE_TYPE(M68010_TEKMMU, m68010_tekmmu_device)

DEFINE_DEVICE_TYPE(M68010_TEKMMU, m68010_tekmmu_device, "mc68010_tekmmu", "MC68010 with Tek4404 custom MMU")


namespace {


	enum {
		PHASE_STATIC = 0,
		PHASE_POSITIVE,
		PHASE_NEGATIVE
	};
	
class tek440x_state : public driver_device
{
public:
	tek440x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vm(*this, "vm"),
		m_duart(*this, "duart"),
		m_keyboard(*this, "keyboard"),
		m_snsnd(*this, "snsnd"),
		m_timer(*this, "timer"),
		m_rtc(*this, "rtc"),
		m_scsi(*this, "scsi:7:ncr5385"),
		m_vint(*this, "vint"),
		m_screen(*this, "screen"),
		m_acia(*this, "acia"),
		m_printer(*this, "printer"),
		m_prom(*this, "maincpu"),			// FIXME why is the bootrom called 'maincpu'?
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_map(*this, "map", 0x1000, ENDIANNESS_BIG),		// 2k 16-bit entries
		m_map_view(*this, "map"),
		m_mousex(*this, "mousex"),
		m_mousey(*this, "mousey"),
		m_mousebtn(*this, "mousebtn"),
		m_led_1(*this, "led_1"),
		m_led_2(*this, "led_2"),
		m_led_4(*this, "led_4"),
		m_led_8(*this, "led_8"),
		m_led_disk(*this, "led_disk"),
		m_boot(false),
		m_map_control(0),
		m_latched_map_control(0),
		m_kb_rdata(true),
		m_kb_tdata(true),
		m_kb_rclamp(false),
		m_kb_loop(false),
		m_mouse(0),
		m_mouse_bnts(0),
		m_mouse_x(0),
		m_mouse_y(0),
		m_mouse_px(PHASE_STATIC),
		m_mouse_py(PHASE_STATIC),
		m_mouse_pc(0)

	{ }

	void tek4404(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 memory_r(offs_t offset, u16 mem_mask);
	void memory_w(offs_t offset, u16 data, u16 mem_mask);
	u16 map_r(offs_t offset);
	void map_w(offs_t offset, u16 data, u16 mem_mask);
	u8 mapcntl_r();
	void mapcntl_w(u8 data);
	void sound_w(u8 data);
	u8 diag_r();
	void diag_w(u8 data);
	
	int mouseupdate();
	u8 mouse_r(offs_t offset);
	void mouse_w(u8 data);
	void led_w(u8 data);

	void fpu_latch32_result(void *);
	void fpu_latch64_result(void *);
	u16 fpu_r(offs_t offset);
	void fpu_w(offs_t offset, u16 data);
	u16 videoaddr_r(offs_t offset);
	void videoaddr_w(offs_t offset, u16 data);
	u8 videocntl_r();
	void videocntl_w(u8 data);

	u8 nvram_r(offs_t offset);

	// fake output of serial
	void write_txd(int state);

	void vblank(int state);

	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);

	// need to handle bit 8 reset
	void irq1_raise(int state);
	u16 timer_r(offs_t offset);
	void timer_w(offs_t offset, u16 data);

	// need to handle loopback mode
	u8 duart_r(offs_t offset);
	void duart_w(offs_t offset, u8 data);

	void printer_pc_w(u8 data);

	void kb_rdata_w(int state);
	void kb_tdata_w(int state);
	void kb_rclamp_w(int state);

	void logical_map(address_map &map) ATTR_COLD;
	void physical_map(address_map &map) ATTR_COLD;

	required_device<m68010_tekmmu_device> m_maincpu;
	required_device<address_map_bank_device> m_vm;
	required_device<mc68681_device> m_duart;
	required_device<tek410x_keyboard_device> m_keyboard;
	required_device<sn76496_device> m_snsnd;
	required_device<am9513_device> m_timer;
	required_device<mc146818_device> m_rtc;
	required_device<ncr5385_device> m_scsi;
	required_device<input_merger_all_high_device> m_vint;
	required_device<screen_device> m_screen;
	required_device<mos6551_device> m_acia;
	required_device<i8255_device> m_printer;

	required_region_ptr<u16> m_prom;
	required_shared_ptr<u16> m_mainram;
	required_shared_ptr<u16> m_vram;
	memory_share_creator<u16> m_map;
	memory_view m_map_view;

	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mousebtn;
	
	output_finder<> m_led_1;
	output_finder<> m_led_2;
	output_finder<> m_led_4;
	output_finder<> m_led_8;
	output_finder<> m_led_disk;

	int m_u244latch;
	
	bool m_boot;
	u8 m_map_control;
	u8 m_latched_map_control;		// latched until user mode access (page 2.1-54)
	
	u8 m_printer_pc;
	bool m_kb_rdata;
	bool m_kb_tdata;
	bool m_kb_rclamp;
	bool m_kb_loop;
			
	u16 m_fpuselect;
	u16 m_operand;
	u16 m_lswoperand[5];
	u16 m_mswoperand[5];
	uint64_t m_result;

	u16 m_videoaddr[4];
	u8 m_videocntl;
	u8 m_diag;
	u8 m_mouse,m_mouse_bnts,m_mouse_x,m_mouse_y;
	u8 m_mouse_px,m_mouse_py,m_mouse_pc;
};

/*************************************
 *
 *  Machine start
 *
 *************************************/

void tek440x_state::machine_start()
{
	save_item(NAME(m_boot));
	save_item(NAME(m_map_control));
	save_item(NAME(m_latched_map_control));
	save_item(NAME(m_kb_rdata));
	save_item(NAME(m_kb_tdata));
	save_item(NAME(m_kb_rclamp));
	save_item(NAME(m_kb_loop));
	
	save_item(NAME(m_videocntl));
	save_item(NAME(m_diag));
	
	m_led_1.resolve();
	m_led_2.resolve();
	m_led_4.resolve();
	m_led_8.resolve();
	
	m_led_disk.resolve();
	m_maincpu->space(AS_PROGRAM).install_write_tap(0x7be002, 0x7be003, "led_tap", [this](offs_t offset, u16 &data, u16 mem_mask) { m_led_disk = !BIT(data, 3);});

	
	m_vint->in_w<0>(0);		// VBL enable
	m_vint->in_w<1>(0);		// VBL

	m_maincpu->linktoMMU(&m_map_control, &m_map[0]);

	// AB is this needed for external MMU?
	m_maincpu->set_emmu_enable(true);
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

void tek440x_state::machine_reset()
{

	m_boot = true;
	diag_w(0);
	m_u244latch = 0;
	m_led_disk = 1;
	m_keyboard->kdo_w(1);
	m_latched_map_control = 0;
	mapcntl_w(0);
	videocntl_w(0);
}


/*************************************
 *
 *  Video refresh
 *
 *************************************/

u32 tek440x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_videocntl, 5))
	{
		// screen off
		return 1;
	}

	u32 invert = BIT(m_videocntl, 4) ? 0 : -1;
	int pan = (m_videocntl & 15) ^ 15;

	int woffset = m_videoaddr[0] - 0xffe9;  // ??? why

	for (int y = 0; y < 480; y++)
	{
	
		u16 *const line = &bitmap.pix(y);
		u16 const *video_ram = &m_vram[y * 64 + woffset];

		for (int x = 0; x < 640; x += 16)
		{
			u16 const word = *(video_ram++);
			u16 const word2 = *(video_ram);
			u32 dword = ((word << 16) | word2) ^ invert;
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = BIT(dword, 31 - pan - b);
			}
		}
	}

	return 0;
}


static int inbuserr = 0;

/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/
u16 tek440x_state::memory_r(offs_t offset, u16 mem_mask)
{

	if (!machine().side_effects_disabled())
	{
		if ((m_maincpu->get_fc() & 4) == 0)				// User mode access updates map_control from write latch
		{
				if (m_latched_map_control != (m_map_control & 0x3f))
				{
					LOG("memory_r: m_map_control updated 0x%04x\n", m_latched_map_control);
					m_map_control &= ~0x3f;
					m_map_control |= m_latched_map_control;
				}
		}

		const offs_t offset0 = offset;

#ifndef USE_MMU
		if (!inbuserr)			// not in buserr interrupt
		if ((m_maincpu->get_fc() & 4) == 0)			// only in User mode
		if (BIT(m_map_control, MAP_VM_ENABLE))
		{
		
			// is !cpuWr
			m_map_control &= ~(1 << MAP_CPU_WR);

			// selftest expects fail if page.pid != map_control.pid
			if ( BIT(m_map[offset >> 11], 11, 3) != (m_map_control & 7))
			{
				m_map_control &= ~(1 << MAP_BLOCK_ACCESS);

				inbuserr = 1;

				LOG("memory_r: %06x: bus error: PTE PID %d != current PID %d fc(%d) pc(%08x) berr(%d)\n", offset<<1,
					BIT(m_map[offset >> 11], 11, 3), (m_map_control & 7), m_maincpu->get_fc(), m_maincpu->pc(),
					inbuserr);
				m_maincpu->set_buserror_details(OFF16_TO_OFF8(offset0), 1, m_maincpu->get_fc(), true);

				mem_mask = 0;
				return 0xffff;
		
			}
			else
			{
				m_map_control |= (1 << MAP_BLOCK_ACCESS);
			}
			
			//LOG("memory_r: map %08x => paddr(%08x) fc(%d) pc(%08x)\n",OFF16_TO_OFF8(offset), OFF16_TO_OFF8(BIT(offset, 0, 11) | BIT(m_map[offset >> 11], 0, 11) << 11), m_maincpu->get_fc(), m_maincpu->pc());
			
			offset = BIT(offset, 0, 11) | BIT(m_map[offset >> 11], 0, 11) << 11;
		}
#endif

		// NB byte memory limit, offset is *word* offset
		if (offset >= OFF8_TO_OFF16(MAXRAM) && offset < OFF8_TO_OFF16(0x600000))
		{
			LOG("memory_r: bus error: NOMEM %08x fc(%d) pc(%08x)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc(), m_maincpu->pc());
			m_maincpu->set_buserror_details(OFF16_TO_OFF8(offset0), 1, m_maincpu->get_fc(), true);
		}
	}

	if (inbuserr && (m_maincpu->get_fc() & 4))
	{
	LOG("berr reset(r) %06x\n", offset<<1);
	inbuserr = 0;
	}

	return (m_boot) ? m_prom[offset & 0x3fff] : m_vm->read16(offset, mem_mask);
}

void tek440x_state::memory_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((m_maincpu->get_fc() & 4) == 0)		// User mode access updates map_control from write latch
	{
			if (m_latched_map_control != (m_map_control & 0x3f))
			{
				LOG("memory_w: m_map_control updated 0x%04x\n", m_latched_map_control);
				m_map_control &= ~0x3f;
				m_map_control |= m_latched_map_control;
			}
	}
	
	const offs_t offset0 = offset;
#ifndef USE_MMU
	if ((m_maincpu->get_fc() & 4) == 0)		// only in User mode
	if (BIT(m_map_control, MAP_VM_ENABLE))
	{
		//LOG("memory_w: m_map(0x%04x)\n", m_map[offset >> 11]);
	
		// is cpuWr
		m_map_control |= (1 << MAP_CPU_WR);
				
		// matching pid?
		if ( (BIT(m_map[offset >> 11], 11, 3) != (m_map_control & 7)))
		{
			// NB active low
			m_map_control &= ~(1 << MAP_BLOCK_ACCESS);

			inbuserr = 1;

			LOG("memory_w: bus error: PID(%d) != %d %08x fc(%d)\n", BIT(m_map[offset >> 11], 11, 3), (m_map_control & 7), OFF16_TO_OFF8(offset), m_maincpu->get_fc());
			m_maincpu->set_buserror_details(OFF16_TO_OFF8(offset0), 0, m_maincpu->get_fc(), true);
			
			mem_mask = 0;	// disable write
		}
		else if (!inbuserr)
		{
			m_map_control |= (1 << MAP_BLOCK_ACCESS);
		}
		else mem_mask = 0;

		// write-enabled page?
		if (mem_mask)
		if (BIT(m_map[offset >> 11], 14) == 0)
		{
			m_map_control &= ~(1 << MAP_BLOCK_ACCESS);

			inbuserr = 1;

			LOG("memory_w: bus error: READONLY %08x fc(%d) pc(%08x)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc(),  m_maincpu->pc());
			m_maincpu->set_buserror_details(OFF16_TO_OFF8(offset0), 0, m_maincpu->get_fc(), true);
			
			mem_mask = 0;	// disable write
			return;
		}

		// mark page dirty (NB before we overwrite offset)
		if (mem_mask)
		{
			if (!(m_map[offset >> 11] & 0x8000))
				LOG("memory_w: DIRTY m_map(0x%04x) m_map_control(%02x) berr(%d)\n", m_map[offset >> 11], m_map_control, inbuserr);
			m_map[offset >> 11] |= 0x8000;
		}
		
		offset = BIT(offset, 0, 11) | (BIT(m_map[offset >> 11], 0, 11) << 11);
	}
#endif

	// NB byte memory limit, offset is *word* offset
	if (offset >= OFF8_TO_OFF16(MAXRAM) && offset < OFF8_TO_OFF16(0x600000) && !machine().side_effects_disabled())
	{
		LOG("memory_w: bus error: NOMEM %08x fc(%d)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc());
		m_maincpu->set_buserror_details(OFF16_TO_OFF8(offset0), 0, m_maincpu->get_fc(), true);
	}

	if (inbuserr && (m_maincpu->get_fc() & 4))
	{
	LOG("berr reset(w) %06x\n", offset<<1);
	inbuserr = 0;
	}

	m_vm->write16(offset, data, mem_mask);
}

u16 tek440x_state::map_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset>>11) < 0x20)
			LOGMASKED(LOG_MMU, "map_r 0x%08x => %04x\n",offset>>11, m_map[(offset >> 11)&0x7ff] );

		// selftest does a read and expects it to fail iff !MAP_SYS_WR_ENABLE; its not WR enable, its enable..
		if (!BIT(m_map_control, MAP_SYS_WR_ENABLE))
		{
				LOGMASKED(LOG_MMU, "map_r: bus error: PID(%d) %08x fc(%d) pc(%08x)\n", BIT(m_map[(offset >> 11) & 0x7ff], 11, 3), OFF16_TO_OFF8(offset), m_maincpu->get_fc(), m_maincpu->pc());
				m_maincpu->set_buserror_details(offset, 1, m_maincpu->get_fc(), true);
				return 0;
		}
	}
	
	return m_map[(offset >> 11) & 0x7ff];
}

void tek440x_state::map_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((offset>>11) < 0x20)
	{
		LOGMASKED(LOG_MMU, "map_w: %08x  <= %04x paddr(%08x) PID(%d) dirty(%d) write_enable(%d)\n",
			(offset >> 11) & 0x7ff, data,
			OFF16_TO_OFF8(BIT(data, 0, 11)<<11),BIT(data, 11, 3), data & 0x8000 ? 1 : 0, data & 0x4000 ? 1 : 0,
			m_maincpu->pc());
	}
	
	// NB page 2.1-52 shows WrMapEn coming from latch
	if (BIT(m_latched_map_control, MAP_SYS_WR_ENABLE))
	{
		COMBINE_DATA(&m_map[(offset >> 11) & 0x7ff]);
	}
}

u8 tek440x_state::mapcntl_r()
{
	if (!machine().side_effects_disabled())
	{
		// page 2.1-54 implies that this can only be read in user mode

		LOG("mapcntl_r(%02x) cpuWr(%d) BlockAccess(%d) SysWrEn(%d) PID(%d) pc(%08x)\n",m_map_control,
			BIT(m_map_control, MAP_CPU_WR) ? 1 : 0,
			BIT(m_map_control, MAP_BLOCK_ACCESS) ? 1 : 0,
			BIT(m_map_control, MAP_SYS_WR_ENABLE) ? 1 : 0,
			m_map_control & 15,
			m_maincpu->pc());

	}
	
	u8 ans = m_map_control;
	
	// mask out 'SysWrEn'
	// 0xc0 means 'not blocked write' aka successful write
	if ((ans & 0xc0) != 0xc0)
		ans &= ~(1<<MAP_SYS_WR_ENABLE);

	return ans;
}

void tek440x_state::mapcntl_w(u8 data)
{
	// copied on user mode read/write
	m_latched_map_control = data & 0x3f;
	
	if (m_map_control != m_latched_map_control)
	{
		LOGMASKED(LOG_MMU, "mapcntl_w(%02x) cpuWr(%d) BlockAccess(%d) SysWrEn(%d) PID(%d) pc(%08x)\n",m_latched_map_control,
			BIT(m_latched_map_control, MAP_CPU_WR) ? 1 : 0,
			BIT(m_latched_map_control, MAP_BLOCK_ACCESS) ? 1 : 0,
			BIT(m_latched_map_control, MAP_SYS_WR_ENABLE) ? 1 : 0,
			m_latched_map_control & 15,
			m_maincpu->pc());
		
		if (BIT(data, MAP_VM_ENABLE) && (data & 15))
		for(uint32_t i=0; i<2048; i++)
		{
			if (m_map[i])
				LOGMASKED(LOG_MMU, "XXXmapcntl_w: %08x -> paddr(%08x) PID(%d) dirty(%d) write_enable(%d)\n",
				OFF16_TO_OFF8(i << 11), OFF16_TO_OFF8(BIT(m_map[i], 0, 11)<<11),
				BIT(m_map[i], 11, 3), m_map[i] & 0x8000 ? 1 : 0, m_map[i] & 0x4000 ? 1 : 0);
		}

	}

	// NB bit 6 & 7 is not used
	
	// disable using latched state below
	//m_map_control = data & 0x3f;
	
}

enum {FLOATRESULT=0, DOUBLERESULT=1, INTRESULT=2};

enum {
	FPCALC = 0xbe,
	FPCONV = 0x3e,
	
	SETS = 0x0b42,
	GETS = 0x3342,
	
	FItoF = 0x0742,
	FFtoIr = 0x2742,
	FFtoIt = 0x2f42,
	FFtoIf = 0x3f42,
	FFtoD = 0x1b42,
	
	FItoD = 0x0342,
	FDtoIr = 0x2342,
	FDtoIt = 0x2b42,
	FDtoIf = 0x3b42,
	FDtoF = 0x1642,
	
	FADD = 0x0142,
	FSUB = 0x1142,
	FMUL = 0x3142,
	FDIV = 0x2142,
	FCMP = 0x0942,
	FNEG = 0xff00,	// unused
	FABS = 0xff01,	// unused

	DADD = 0x0042,
	DSUB = 0x1042,
	DMUL = 0x3042,
	DDIV = 0x2042,
	DCMP = 0x0842,
	DNEG = 0xff80,	// unused
	DABS = 0xff81,	// unused

};

void tek440x_state::fpu_w(offs_t offset, u16 data)
{
	LOG("fpu_w:  %08x <= 0x%04x\n", offset, data);

	switch(offset)
	{
		default:
			break;
		
		// latches opcode.w, arg1.l, arg2.l
		case 2:
			m_lswoperand[m_operand] = data;
			if (!m_operand) m_operand++;
			break;
		case 3:
			m_mswoperand[m_operand] = data;
			m_operand++;
			break;

		// start op,  can be FPCALC or FPCONV...
		case 6:
			m_result = 0;
			m_fpuselect = data;
			m_operand = 0;
			break;
		case 7:
			break;
	}
}

void tek440x_state::fpu_latch32_result(void *v)
{
	m_result <<= 32;
	m_result |= *(uint32_t *)v;
}

void tek440x_state::fpu_latch64_result(void *v)
{
	m_result = *(uint64_t *)v;
}

#define CASE_ENUM(C) case C: opname = #C; break

u16 tek440x_state::fpu_r(offs_t offset)
{
	// status check; should pretend to be not-ready for a bit
	if (offset == 4)
	{
		float af,bf;
		double ad,bd;
		int ai;
		
		uint32_t v32;
		uint64_t v64;

#ifdef DEBUG
		const char *opname;
		switch(m_lswoperand[0])
		{
			CASE_ENUM(FADD);
			CASE_ENUM(FSUB);
			CASE_ENUM(FMUL);
			CASE_ENUM(FDIV);
			CASE_ENUM(FCMP);

			CASE_ENUM(FItoF);
			CASE_ENUM(FFtoIr);
			CASE_ENUM(FFtoIt);
			CASE_ENUM(FFtoIf);
			CASE_ENUM(FFtoD);
			CASE_ENUM(FDtoF);

			CASE_ENUM(DADD);
			CASE_ENUM(DSUB);
			CASE_ENUM(DMUL);
			CASE_ENUM(DDIV);
			CASE_ENUM(DCMP);

			CASE_ENUM(FItoD);
			CASE_ENUM(FDtoIr);
			CASE_ENUM(FDtoIt);
			CASE_ENUM(FDtoIf);
			
			default:
				opname = "unknown";
				break;
		}
		LOGMASKED(LOG_FPU,"fpu_r: %s  ************** %s \n", m_fpuselect == FPCALC ? "FPCALC" : "FPCONV",  opname);
#endif

		// assembling operands
		if (m_operand >= 2)		// has at least 1 32-bit operands
		{
			v32 = (m_mswoperand[1] << 16) | (m_lswoperand[1]);
			ai = v32;
			af = *(float *)&v32;
			LOGMASKED(LOG_FPU,"fpu_r:  m_operand(%d) float(%f) int(%d)\n", m_operand, af, ai);
			
			if (m_operand >= 3)		// has at least 2 32-bit operands or 1 64-bit operand
			{
				v32 = (m_mswoperand[2] << 16) | (m_lswoperand[2]);
				bf = *(float *)&v32;

				v64 = (((uint64_t)m_mswoperand[2]) << 48) | (((uint64_t)m_lswoperand[2])<<32) | (((uint64_t)m_mswoperand[1]) << 16) | (m_lswoperand[1]);
				ad = *(double *)&v64;

				LOGMASKED(LOG_FPU,"fpu_r:  m_operand(%d) float(%f) float(%f) double(%lf) 0x%lx\n", m_operand, af,bf, ad, v64);
			}
		
			if (m_operand == 5)		// has 2 64-bit operands
			{
				v64 = (((uint64_t)m_mswoperand[4]) << 48) | (((uint64_t)m_lswoperand[4])<<32) | (((uint64_t)m_mswoperand[3]) << 16) | (m_lswoperand[3]);
				bd = *(double *)&v64;

				LOGMASKED(LOG_FPU,"fpu_r:  m_operand(%d) double(%lf) double(%lf)\n", m_operand,  ad,bd);
			}
		}
		
		// execute the functionality
		int resulttype = FLOATRESULT;
		int ci;
		float cf;
		double cd;
		switch(m_lswoperand[0])
		{
			case FADD:
				cf = bf + af;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FSUB:
				cf = bf - af;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FMUL:
				cf = bf * af;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FDIV:
				cf = bf / af;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FCMP:
				ci = af > bf ? 1 : (af < bf ? -1 : 0);
				fpu_latch32_result(&ci);
				resulttype = FLOATRESULT;
				break;
				
			case FNEG:
				cf = -af;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FABS:
				cf = fabs(af);
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;

			case FItoF:
				cf = (float)ai;
				fpu_latch32_result(&cf);
				resulttype = FLOATRESULT;
				break;
			case FFtoIr:
				ci = (int)(af + 0.5f);
				fpu_latch32_result(&ci);
				resulttype = INTRESULT;
				break;
			case FFtoIt:
				ci = (int)(af);
				fpu_latch32_result(&ci);
				resulttype = INTRESULT;
				break;
			case FFtoIf:
				ci = (int)floorf(af);
				fpu_latch32_result(&ci);
				resulttype = INTRESULT;
				break;

			case FFtoD:
				cd = (double)af;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case FDtoF:
				cf = (float)ad;
				fpu_latch32_result(&cf);
				break;
				
			case DADD:
				cd = bd + ad;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case DSUB:
				cd = bd - ad;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case DMUL:
				cd = bd * ad;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case DDIV:
				cd = bd / ad;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case DCMP:
				ci = ad > bd ? 1 : (ad < bd ? -1 : 0);
				fpu_latch32_result(&ci);
				resulttype = DOUBLERESULT;
				break;

			case DNEG:
				cd = -ad;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case DABS:
				cd = fabs(ad);
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;

			case FItoD:
				cd = (double)ai;
				fpu_latch64_result(&cd);
				resulttype = DOUBLERESULT;
				break;
			case FDtoIr:
				ci = (int)(ad + 0.5);
				fpu_latch32_result(&ci);
				resulttype = DOUBLERESULT;
				break;
			case FDtoIt:
				ci = (int)(ad);
				fpu_latch32_result(&ci);
				resulttype = DOUBLERESULT;
				break;
			case FDtoIf:
				ci = (int)floor(ad);
				fpu_latch32_result(&ci);
				resulttype = DOUBLERESULT;
				break;

			default:
				LOGMASKED(LOG_FPU,"fpu_r: unknown opcode 0x%04x\n", m_lswoperand[0]);
				m_result = 0x4000;
				break;
		}
	
		switch(resulttype)
		{
			default:
			case FLOATRESULT:
				LOGMASKED(LOG_FPU,"fpu_r: m_result = 0x%lx float(%f)\n", m_result, cf);
				break;
			case DOUBLERESULT:
				LOGMASKED(LOG_FPU,"fpu_r: m_result = 0x%lx double(%lf)\n", m_result, cd);
				break;
			case INTRESULT:
				LOGMASKED(LOG_FPU,"fpu_r: m_result = 0x%lx int(%d)\n", m_result, ci);
				break;

		}

		// always ready  (bit15 clear)
		return 0;
	}

	u16 result = 0;
	switch(offset)
	{
		default:
			break;

		// shift out result word at a time
		case 2:
		case 3:
			result = m_result & 0xffff;
			m_result >>= 16;
			break;
	}
	
	return result;
}

u16 tek440x_state::videoaddr_r(offs_t offset)
{
	LOG("videoaddr_r %08x\n", offset);

	return m_videoaddr[offset];
}

void tek440x_state::videoaddr_w(offs_t offset, u16 data)
{
	LOG("videoaddr_w %08x %04x\n", offset, data);
	m_videoaddr[offset] = data;
}

u8 tek440x_state::videocntl_r()
{
	int ans = m_videocntl;

	// page 2.1-92
	if (m_screen->vblank())
		ans |= 0x20;
	else
		ans |= 0x10;

	if (m_screen->hblank())
		ans |= 0x40;
		
	return ans;
}

void tek440x_state::videocntl_w(u8 data)
{
	if (0)
	if (m_videocntl != data)
	{
		LOG("m_videocntl %02x\n", data);
		LOG("m_videocntl VBenable   %2d\n", BIT(data, 6));
		LOG("m_videocntl ScreenOn   %2d\n", BIT(data, 5));
		LOG("m_videocntl ScreenInv  %2d\n", BIT(data, 4));
		LOG("m_videocntl ScreenPan  %2d\n", data & 15);
	}

	m_vint->in_w<0>(BIT(data, 6));
	
  if (BIT(m_videocntl ^ data, 6) && !BIT(data, 6))
			m_vint->in_w<2>(0);

	m_videocntl = data;
}


void tek440x_state::sound_w(u8 data)
{
	if (m_boot)
		LOG("BOOT PROM disabled\n");
	
	m_snsnd->write(data);
	m_boot = false;
}

void tek440x_state::led_w(u8 data)
{

	LOG("LED %c%c%c%c\n",data & 8 ? '*' : '-',data & 4 ? '*' : '-',data & 2 ? '*' : '-',data & 1 ? '*' : '-');

	m_led_1 = BIT(data, 0);
	m_led_2 = BIT(data, 1);
	m_led_4 = BIT(data, 2);
	m_led_8 = BIT(data, 3);

}

u8 tek440x_state::diag_r()
{
	return m_diag;
}

void tek440x_state::diag_w(u8 data)
{
	if (!m_kb_rclamp && m_kb_loop != BIT(data, 7))
		m_keyboard->kdo_w(!BIT(data, 7) || m_kb_tdata);

	m_kb_loop = BIT(data, 7);
	m_diag = data;
}


u8 tek440x_state::nvram_r(offs_t offset)
{
	static u8  nvram[32] = {0xbf,0x6f,0x4f,0x0f,0x41,0x44,0x41,0x4d,0x42,0x2f,0x6f,0x4f,0x0f,0x41,0x5f,0x8f};

	return nvram[offset>>1];
}


// copied from stkbd.cpp
int tek440x_state::mouseupdate()
{
const int mouse_xya[3][4] = { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 } };
const int mouse_xyb[3][4] = { { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 0, 0 } };

	uint8_t x = m_mousex->read();
	uint8_t y = m_mousey->read();
	
	if(m_mouse_pc == 0)
	{
		if(x == m_mouse_x)
			m_mouse_px = PHASE_STATIC;

		else if((x > m_mouse_x) || (x == 0 && m_mouse_x == 0xff))
			m_mouse_px = PHASE_POSITIVE;

		else if((x < m_mouse_x) || (x == 0xff && m_mouse_x == 0))
			m_mouse_px = PHASE_NEGATIVE;

		if(y == m_mouse_y)
			m_mouse_py = PHASE_STATIC;

		else if((y > m_mouse_y) || (y == 0 && m_mouse_y == 0xff))
			m_mouse_py = PHASE_POSITIVE;

		else if((y < m_mouse_y) || (y == 0xff && m_mouse_y == 0))
			m_mouse_py = PHASE_NEGATIVE;

		m_mouse_x = x;
		m_mouse_y = y;
	}

	m_mouse <<= 4;
	m_mouse &= ~15;

	m_mouse |= mouse_xyb[m_mouse_px][m_mouse_pc];      // XB
	m_mouse |= mouse_xya[m_mouse_px][m_mouse_pc] << 1; // XA
	m_mouse |= mouse_xyb[m_mouse_py][m_mouse_pc] << 2; // YA
	m_mouse |= mouse_xya[m_mouse_py][m_mouse_pc] << 3; // YB

	m_mouse_pc++;
	m_mouse_pc &= 0x03;
	
	return m_mouse;
}

u8 tek440x_state::mouse_r(offs_t offset)
{
	u8 ans = 0;
	static u8 oldmx,oldmy;
	
	switch(offset)
	{
		case 0:
			ans = ~(oldmx - m_mouse_x);
			oldmx = m_mouse_x;
			break;
		case 2:
			ans = ~(oldmy - m_mouse_y);
			oldmy = m_mouse_y;
			ans ^= (m_diag & 15);
			
			break;
		case 4:
			m_mouse_bnts <<= 3;
			m_mouse_bnts |=  m_mousebtn->read() & 7;		// selftest xor diag register with it
			ans = m_mouse_bnts & 0x1f;
			ans ^= (m_diag & 15) << 3;
			ans |= 0x80;										// VCC from calender(?)
			
			break;
		case 6:
			mouseupdate();
			break;

		default:
			oldmx = oldmy = 0;
			break;
	}

	return ans;
}

void tek440x_state::mouse_w(u8 data)
{
	m_mouse = data;
	LOG("mouse select(%x)\n", m_mouse);
}

void tek440x_state::write_txd(int state)
{
	LOG("acia write_txd(%x)\n", state);
}

void tek440x_state::printer_pc_w(u8 data)
{

	LOG("printer_pc_w(%02x)\n", data);

	if (!m_kb_rclamp && BIT(m_printer_pc, 2) != BIT(data, 2))
		m_keyboard->kdo_w(!BIT(data, 2) || m_kb_tdata);

	m_printer_pc = data;
}


void tek440x_state::kb_rdata_w(int state)
{
	m_kb_rdata = state;
	if (!m_kb_rclamp)
		m_duart->rx_a_w(state);
}

void tek440x_state::kb_rclamp_w(int state)
{
	if (m_kb_rclamp != !state)
	{
		m_kb_rclamp = !state;

		// Clamp RXDA to 1 and KBRDATA to 0 when DUART asserts RxRDYA
		if (m_kb_tdata || !m_kb_loop)
			m_keyboard->kdo_w(state);
		m_duart->rx_a_w(state ? m_kb_rdata : 1);
	}
}

void tek440x_state::kb_tdata_w(int state)
{
	if (m_kb_tdata != state)
	{
		m_kb_tdata = state;

		m_duart->ip4_w(!state);
		if (m_kb_loop && m_kb_rdata && !m_kb_rclamp)
			m_keyboard->kdo_w(state);
	}
}

void tek440x_state::vblank(int state)
{

	if (state == 1)
	if (BIT(m_videocntl, 6))
	{
		LOG("vblank %04x\n", state);
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

u8 tek440x_state::rtc_r(offs_t offset)
{
	LOG("rtc_r %08x\n", offset);
	
	return 0;
}

void tek440x_state::rtc_w(offs_t offset, u8 data)
{
	LOG("rtc_w %08x\n", offset);
}


void tek440x_state::irq1_raise(int state)
{
	LOG("irq1_raise %04x\n", state);
	
//	if (m_u244latch == 1 && state == 0)
	{
		LOG("M68K_IRQ_1 assert\n");
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
	m_u244latch = state;
}

// to handle offset 0x1xx writes resetting TPInt...
u16 tek440x_state::timer_r(offs_t offset)
{
	LOG("timer_r %08x\n", offset);

//	if (m_u244latch)
	{
		LOG("M68K_IRQ_1 clear\n");
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_u244latch = 0;
	}

	return m_timer->read16(offset);
}

// to handle offset 0x1xx writes resetting TPInt...
void tek440x_state::timer_w(offs_t offset, u16 data)
{
	LOG("timer_w %08x %04x\n", OFF16_TO_OFF8(offset), data);
	m_timer->write16(offset, data);

//	if (m_u244latch)
	{
		LOG("M68K_IRQ_1 clear\n");
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_u244latch = 0;
	}
}

u8 tek440x_state::duart_r(offs_t offset)
{
	return m_duart->read(offset);
}

void tek440x_state::duart_w(offs_t offset, u8 data)
{
	// Transmit Buffer?
	if (offset == 3)
	{
		if (m_kb_loop)
		{
			m_duart->write(0x0, 0x80);
		}
	}

	m_duart->write(offset, data);
}

void tek440x_state::logical_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(tek440x_state::memory_r), FUNC(tek440x_state::memory_w));

#ifndef USE_MMU
	// NB we mask in handlers because I do not understand .mirror()!
	map(0x800000, 0xffffff).rw(FUNC(tek440x_state::map_r), FUNC(tek440x_state::map_w));
#endif
}

void tek440x_state::physical_map(address_map &map)
{
	map(0x000000, MAXRAM-1).ram().share("mainram");						// +1MB RAM option;
	map(0x600000, 0x61ffff).ram().share("vram");

	// 700000-71ffff spare 0
	// 720000-720fff spare 1 (ethernet)
	// 721000-721fff nvram nybbles
	map(0x721000, 0x721fff).r(FUNC(tek440x_state::nvram_r));
	map(0x740000, 0x747fff).rom().mirror(0x8000).region("maincpu", 0).w(FUNC(tek440x_state::led_w));
	map(0x760000, 0x760fff).ram().mirror(0xf000); // debug RAM

	// 780000-79ffff processor board I/O
	map(0x780000, 0x780000).rw(FUNC(tek440x_state::mapcntl_r), FUNC(tek440x_state::mapcntl_w));
	// 782000-783fff: video address registers
	map(0x782000, 0x782003).rw(FUNC(tek440x_state::videoaddr_r),FUNC(tek440x_state::videoaddr_w));
	// 784000-785fff: video control registers
	map(0x784000, 0x784000).rw(FUNC(tek440x_state::videocntl_r),FUNC(tek440x_state::videocntl_w));
	// 786000-787fff: spare
	map(0x788000, 0x788000).w(FUNC(tek440x_state::sound_w));
	map(0x78a000, 0x78bfff).rw(FUNC(tek440x_state::fpu_r),FUNC(tek440x_state::fpu_w));
	map(0x78c000, 0x78c007).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write)).umask16(0xff00);
	// 78e000-78ffff: spare

	// 7a0000-7bffff peripheral board I/O
	// 7a0000-7affff: reserved
	map(0x7b0000, 0x7b0000).rw(FUNC(tek440x_state::diag_r),FUNC(tek440x_state::diag_w));
	// 7b1000-7b1fff: diagnostic registers
	// 7b2000-7b3fff: Centronics printer data
	map(0x7b2000, 0x7b3fff).rw(m_printer, FUNC(i8255_device::read), FUNC(i8255_device::write));
	
	map(0x7b4000, 0x7b401f).rw(FUNC(tek440x_state::duart_r), FUNC(tek440x_state::duart_w)).umask16(0xff00);
	// 7b6000-7b7fff: Mouse
	map(0x7b6000, 0x7b6fff).rw(FUNC(tek440x_state::mouse_r),FUNC(tek440x_state::mouse_w));

	map(0x7b8000, 0x7b8003).rw(m_timer, FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0x7b8100, 0x7b8103).rw(FUNC(tek440x_state::timer_r), FUNC(tek440x_state::timer_w));
	
	// 7ba000-7bbfff: MC146818 RTC
	map(0x7ba000, 0x7ba000).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x7ba100, 0x7ba103).rw(FUNC(tek440x_state::rtc_r), FUNC(tek440x_state::rtc_w));

	
	map(0x7bc000, 0x7bc000).lw8(
		[this](u8 data)
		{
			m_scsi->set_own_id(data & 7);

			// TODO: bit 7 -> SCSI bus reset
			LOG("scsi bus reset %d\n", BIT(data, 7));
			if (BIT(data, 7))
			{
				//m_scsi->cmd_w(0);
			}
			
		}, "scsi_addr"); // 7bc000-7bdfff: SCSI bus address registers
	map(0x7be000, 0x7be01f).m(m_scsi, FUNC(ncr5385_device::map)).umask16(0xff00); //.mirror(0x1fe0) .cswidth(16);

	// 7c0000-7fffff EPROM application space
	map(0x7c0000, 0x7fffff).nopr();
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tek4404 )
	PORT_START("mousex")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("mousey")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("mousebtn")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tek_msu_fdc", TEK_MSU_FDC);
}

// interrupts
// 7 debug
// 6 vsync
// 5 uart
// 4 spare
// 3 scsi
// 2 dma (network?)
// 1 timer/printer

void tek440x_state::tek4404(machine_config &config)
{
	/* basic machine hardware */
	M68010_TEKMMU(config, m_maincpu, 40_MHz_XTAL / 4); // MC68010L10
	m_maincpu->set_addrmap(AS_PROGRAM, &tek440x_state::logical_map);

	ADDRESS_MAP_BANK(config, m_vm);
	m_vm->set_addrmap(0, &tek440x_state::physical_map);
	m_vm->set_data_width(16);
	m_vm->set_addr_width(23);
	m_vm->set_endianness(ENDIANNESS_BIG);

	INPUT_MERGER_ALL_HIGH(config, m_vint);
	m_vint->output_handler().set_inputline(m_maincpu, M68K_IRQ_6);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(25.2_MHz_XTAL, 800, 0, 640, 525, 0, 480); // 31.5 kHz horizontal (guessed), 60 Hz vertical
	m_screen->set_screen_update(FUNC(tek440x_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(m_vint, FUNC(input_merger_all_high_device::in_w<1>));
	m_screen->screen_vblank().append(m_vint, FUNC(input_merger_all_high_device::in_w<2>));
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	MOS6551(config, m_acia, 40_MHz_XTAL / 4 / 10);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->irq_handler().set_inputline(m_maincpu, M68K_IRQ_7);

	I8255A(config, m_printer);
	m_printer->in_pb_callback().set_constant(0x30);
m_printer->in_pb_callback().set_constant(0xbf);		// HACK:  vblank always checks if printer status < 0
	m_printer->out_pc_callback().set(FUNC(tek440x_state::printer_pc_w));

	MC68681(config, m_duart, 14.7456_MHz_XTAL / 4);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_5); // auto-vectored
	m_duart->outport_cb().set(FUNC(tek440x_state::kb_rclamp_w)).bit(4);
	m_duart->outport_cb().append(m_keyboard, FUNC(tek410x_keyboard_device::reset_w)).bit(3);
	m_duart->a_tx_cb().set(m_keyboard, FUNC(tek410x_keyboard_device::kdi_w));

	TEK410X_KEYBOARD(config, m_keyboard);
	m_keyboard->tdata_callback().set(FUNC(tek440x_state::kb_tdata_w));
	m_keyboard->rdata_callback().set(FUNC(tek440x_state::kb_rdata_w));

	AM9513(config, m_timer, 40_MHz_XTAL / 4 / 10 ); // from CPU E output

	// see diagram page 2.2-6
	INPUT_MERGER_ALL_HIGH(config, "irq1").output_handler().set(FUNC(tek440x_state::irq1_raise));
	m_timer->out1_cb().set("irq1", FUNC(input_merger_device::in_w<0>));
	m_timer->out2_cb().set("irq1", FUNC(input_merger_device::in_w<1>));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);

	NSCSI_BUS(config, "scsi");
	// hard disk is a Micropolis 1304 (https://www.micropolis.com/support/hard-drives/1304)
	// AB: Not sure this is correct:  with a Xebec 1401 SASI adapter inside the Mass Storage Unit
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "tek_msu_fdc");
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5385", NCR5385).clock(40_MHz_XTAL / 4).machine_config(
		[this](device_t *device)
		{
			ncr5385_device &adapter = downcast<ncr5385_device &>(*device);

			adapter.irq().set_inputline(m_maincpu, M68K_IRQ_3);
		});

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	SPEAKER(config, "mono").front_center();

	SN76496(config, m_snsnd, 25.2_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.80);
	
	config.set_default_layout(layout_tek4404);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tek4404 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "boot.bin", 0x000000, 0x008000, CRC(bceb9462) SHA1(01960a90eab482957469ad4e7e3dc74f33588779) )
	//ROM_LOAD16_BYTE( "tek_u158.bin", 0x000000, 0x004000, CRC(9939e660) SHA1(66b4309e93e4ff20c1295dc2ec2a8d6389b2578c) )
	//ROM_LOAD16_BYTE( "tek_u163.bin", 0x000001, 0x004000, CRC(a82dcbb1) SHA1(a7e4545e9ea57619faacc1556fa346b18f870084) )

	ROM_REGION( 0x2000, "scsimfm", 0 )
	ROM_LOAD( "scsi_mfm.bin", 0x000000, 0x002000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                               FLAGS
COMP( 1984, tek4404, 0,      0,      tek4404, tek4404, tek440x_state, empty_init, "Tektronix", "4404 Artificial Intelligence System", MACHINE_NOT_WORKING  )
