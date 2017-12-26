// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"
#include "cpu/z180/z180.h"
#include "debug/debugcpu.h"
#include "sound/beep.h"
#include "video/mc6845.h"

// fixed quite a few DMA related bugs in z180 core!

// if updating project, c:\msys64\win32env.bat
// cd \schreibmaschine\mame_src
// make SUBTARGET=schreibmaschine NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 vs2017

// command line parameters:
// -log -debug -window -intscalex 2 -intscaley 2 lw350 -resolution 960x256 -flop roms\lw350\Brother_LW-200-300_GW-24-45_Ver1.0_SpreadsheetProgramAndDataStorageDisk.img

#pragma region(LW-350)

/***************************************************************************

Brother LW-350
1995

Hardware:

#4 
Hitachi HG62F33R63FH
US0021-A
CMOS Gate Array
3,297 gates, QFPS-136
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#1
Hitachi HD63266F
CMOS Floppy Disk Controller
QFP-64
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#2
Hitachi HM658128ALP-10
01105330
131072-word x 8-bit High Speed CMOS Pseudo Static RAM
DP-32
100 ns

#3
Hitachi HN62334BP
UC6273-A-LWB6
524288-word x 8-bit CMOS Mask Programmable ROM
DP-32
150 ns

#5
Hitachi HD64180ZP8
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
8 MHz, DP-64S, Address Space 512 K Byte
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

1.44MB Floppy Drive

Hidden Keys during "DECKEL OFFEN!" ("Case Open!")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Adjustment Printer Menu

Hidden Keys during "SCHREIBMASCHINE" ("Typewriter")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Self Test Menu
- Ctrl+Shift+Enter: Self Print Menu

Emulation Status:
- Printer not working

***************************************************************************/

#pragma region(Floppy)

class lw350_floppy_image_device;

class lw350_floppy_connector : public device_t, public device_slot_interface
{
public:
	lw350_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~lw350_floppy_connector();

	lw350_floppy_image_device *get_device();

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;
};

class lw350_floppy_image_device : public device_t, public device_image_interface, public device_slot_card_interface
{
public:
	lw350_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~lw350_floppy_image_device();

public:
	virtual iodevice_t image_type() const override { return IO_FLOPPY; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "img"; }
	virtual const char *image_interface() const override { return "floppy_35"; }
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	bool loaded = false;
	bool dirty = false;
	std::unique_ptr<uint8_t[]> image;
	uint32_t image_length{};

protected:
	virtual void device_start() override { }
	//virtual void device_config_complete() override { update_names(); } // needed; otherwise infinite loop during startup
};

DEFINE_DEVICE_TYPE(LW350_FLOPPY_CONNECTOR, lw350_floppy_connector, "floppy_connector", "Floppy drive connector abstraction")
DEFINE_DEVICE_TYPE(LW350_FLOPPY, lw350_floppy_image_device, "floppy_35", "3.5\" floppy drive")

lw350_floppy_connector::lw350_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LW350_FLOPPY_CONNECTOR, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}

lw350_floppy_connector::~lw350_floppy_connector()
{
}

void lw350_floppy_connector::device_start()
{
}

void lw350_floppy_connector::device_config_complete()
{
}

lw350_floppy_image_device *lw350_floppy_connector::get_device()
{
	return dynamic_cast<lw350_floppy_image_device *>(get_card_device());
}

lw350_floppy_image_device::lw350_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LW350_FLOPPY, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_card_interface(mconfig, *this)
{
}

lw350_floppy_image_device::~lw350_floppy_image_device()
{
}

image_init_result lw350_floppy_image_device::call_load()
{
	// copy image into our buffer
	auto p = ptr();
	auto l = length();
	image.reset(new uint8_t[l]);
	memcpy(image.get(), p, l);
	image_length = l;

	loaded = true;
	return image_init_result::PASS;
}

void lw350_floppy_image_device::call_unload()
{
	if(dirty && !is_readonly()) {
		if(reopen_for_write(filename()) == IMAGE_ERROR_SUCCESS)
			fwrite(image.get(), image_length);
	}

	loaded = false;
}

#pragma endregion

#pragma region(HD63266F)

// FDC high-level emulation (not timing accurate) based on "Hitachi 8-Bit Microcomputer HD63265 FDC Floppy Disk Controller User's Manual"
// https://archive.org/details/bitsavers_hitachidatDiskControllerUsersManual2edMar89_3858532

#define MCFG_HD63266F_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, HD63266F, _clock)

#define MCFG_HD63266F_IRQ_CALLBACK(_write) \
	devcb = &hd63266f_t::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_HD63266F_DREQ_CALLBACK(_write) \
	devcb = &hd63266f_t::set_dreq_wr_callback(*device, DEVCB_##_write);

#define MCFG_HD63266F_DEND_CALLBACK(_read) \
	devcb = &hd63266f_t::set_dend_rd_callback(*device, DEVCB_##_read);

class hd63266f_t : public device_t {
public:
	hd63266f_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<hd63266f_t&>(device).irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dreq_wr_callback(device_t &device, _Object object) { return downcast<hd63266f_t&>(device).dreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dend_rd_callback(device_t &device, _Object object) { return downcast<hd63266f_t&>(device).dend_cb.set_callback(object); }

	void set_floppy(lw350_floppy_image_device* floppy) { this->floppy = floppy; }

	void reset();
	void write(uint8_t data);
	uint8_t read();
	void execute();
	void abort();

	DECLARE_READ8_MEMBER(status_r) { return STR; }
	DECLARE_WRITE8_MEMBER(abort_w) { if(data == 0xff) abort(); else logerror("fdc:abort_w %02x is ignored %s\n", data, callstack()); }
	DECLARE_WRITE8_MEMBER(data_w) { write(data); }
	DECLARE_READ8_MEMBER(data_r) { return read(); }

protected:
	uint8_t STR; // status register
	uint8_t DTR; // data register
	uint8_t CMD; // current command
	uint8_t parameter_cnt;
	uint16_t DMA_cnt; // active DMA transfer; number of bytes left
	off_t DMA_src; // current offset in floppy image for DMA transfer

	// command parameters
	uint8_t HSL_US; // Head Select-Unit Select
	uint8_t CA; // Cylinder Address (0-255)
	uint8_t HA; // Head Address (0-1)
	uint8_t SA; // Sector Address (1-255)
	uint8_t RL; // Record Length (0-6) (p. 20)
	uint8_t ESN; // End Sector Number (1-255; compare: 1-253)
	uint8_t GSL; // Gap Skip Length
	uint8_t MNL; // Meaning Length

	// WRITE FORMAT
	uint8_t SCNT; // Sector Count (1-255)
	uint8_t GP3L; // Gap 3 Length (1-255)
	uint8_t DUD; // Dummy Data (0-255)

	// SEEK
	uint8_t NCN; // New Cylinder Number (0-255)

	// COMPARE
	uint8_t STEP; // Step (p. 23)

	// SPECIFY 1
	uint8_t STR_HDUT; // Stepping Rate-Head Unload Time (p. 23)
	uint8_t HDLT_NDM; // Head Load-Time-Non-DMA Mode (p. 25)

	// SPECIFY 2
	uint8_t LCTK; // Low Current Track
	uint8_t PC1_PC0; // Precompensation Delay 1, 0
	uint8_t PCDCT; // Precompensation Delay Change Track

	// result parameters
	uint8_t SSB0, SSB1, SSB2, SSB3; // (p. 28ff)
	uint8_t PCN; // physical cylinder number

	devcb_write_line irq_cb, dreq_cb;
	devcb_read_line dend_cb;

	virtual void device_start() override;

	lw350_floppy_image_device *floppy;

	// TODO
	std::string pc() { return ""; }
	std::string callstack() { return ""; }
};

DEFINE_DEVICE_TYPE(HD63266F, hd63266f_t, "hd63266f", "HD63266F")

enum FDC_STATUS_MASK : uint8_t
{
	FDC_STATUSM_D0S = 0x1,
	FDC_STATUSM_D1S = 0x2,
	FDC_STATUSM_D2S = 0x4,
	FDC_STATUSM_D3S = 0x8,
	FDC_STATUSM_BSY = 0x10,
	FDC_STATUSM_NDM = 0x20,
	FDC_STATUSM_DIR = 0x40,
	FDC_STATUSM_TXR = 0x80,
};

enum FDC_STATUS_BITS : uint8_t
{
	FDC_STATUSB_D0S = 0x0,
	FDC_STATUSB_D1S = 0x1,
	FDC_STATUSB_D2S = 0x2,
	FDC_STATUSB_D3S = 0x3,
	FDC_STATUSB_BSY = 0x4,
	FDC_STATUSB_NDM = 0x5,
	FDC_STATUSB_DIR = 0x6,
	FDC_STATUSB_TXR = 0x7,
};

enum FDC_COMMAND : uint8_t
{
	FDC_COMMAND_READ_ERRONEOUS_DATA = 0x2,
	FDC_COMMAND_SPECIFY_1 = 0x3,
	FDC_COMMAND_CHECK_DEVICE_STATUS = 0x4,
	FDC_COMMAND_WRITE_DATA = 0x5,
	FDC_COMMAND_READ_DATA = 0x6,
	FDC_COMMAND_RECALIBRATE = 0x7,
	FDC_COMMAND_CHECK_INTERRUPT_STATUS = 0x8,
	FDC_COMMAND_WRITE_DELETED_DATA = 0x9,
	FDC_COMMAND_READ_ID = 0xA,
	FDC_COMMAND_SPECIFY_2 = 0xB,
	FDC_COMMAND_READ_DELETED_DATA = 0xC,
	FDC_COMMAND_WRITE_FORMAT = 0xD,
	FDC_COMMAND_SLEEP = 0xE,
	FDC_COMMAND_SEEK = 0xF,
	FDC_COMMAND_COMPARE_EQUAL = 0x11,
	FDC_COMMAND_READ_LONG = 0x12,
	FDC_COMMAND_WRITE_LONG = 0x16,
	FDC_COMMAND_COMPARE_LOW_OR_EQUAL = 0x19,
	FDC_COMMAND_COMPARE_HIGH_OR_EQUAL = 0x1D,
	FDC_COMMAND_ABORT = 0x1F,
	FDC_COMMAND_SKIP_DDAM = 0x20,
	FDC_COMMAND_MODE_FM = 0x0,
	FDC_COMMAND_MODE_MFM = 0x40,
	FDC_COMMAND_MULTI_TRACK = 0x80,
};

enum FDC_SSB3_MASK : uint8_t
{
	FDC_SSB3M_US0 = 0x1,
	FDC_SSB3M_US1 = 0x2,
	FDC_SSB3M_HSL = 0x4,
	FDC_SSB3M_DSD = 0x8,
	FDC_SSB3M_TRZ = 0x10,
	FDC_SSB3M_RDY = 0x20,
	FDC_SSB3M_WPT = 0x40,
	FDC_SSB3M_FLT = 0x80,
};

enum FDC_SSB3_BITS : uint8_t
{
	FDC_SSB3B_US0 = 0x0,
	FDC_SSB3B_US1 = 0x1,
	FDC_SSB3B_HSL = 0x2,
	FDC_SSB3B_DSD = 0x3,
	FDC_SSB3B_TRZ = 0x4,
	FDC_SSB3B_RDY = 0x5,
	FDC_SSB3B_WPT = 0x6,
	FDC_SSB3B_FLT = 0x7,
};

enum FDC_SSB0_BITS
{
	FDC_SSB0B_US0 = 0x0,
	FDC_SSB0B_US1 = 0x1,
	FDC_SSB0B_HSL = 0x2,
	FDC_SSB0B_DNR = 0x3,
	FDC_SSB0B_DER = 0x4,
	FDC_SSB0B_SED = 0x5,
};

enum FDC_SSB0_MASK : uint8_t
{
	FDC_SSB0M_US0 = 0x1,
	FDC_SSB0M_US1 = 0x2,
	FDC_SSB0M_HSL = 0x4,
	FDC_SSB0M_DNR = 0x8, // drive not ready
	FDC_SSB0M_DER = 0x10,
	FDC_SSB0M_SED = 0x20, // seek end
	FDC_SSB0M_INC_NOR = 0x0,
	FDC_SSB0M_INC_ABE = 0x40,
	FDC_SSB0M_INC_IVC = 0x80,
	FDC_SSB0M_INC_CDS = 0xC0,
};

enum FDC_SSB1_BITS
{
	FDC_SSB1B_ANF = 0x0,
	FDC_SSB1B_WPM = 0x1,
	FDC_SSB1B_INF = 0x2,
	FDC_SSB1B_DOR = 0x4,
	FDC_SSB1B_CER = 0x5,
	FDC_SSB1B_NDE = 0x7,
};

enum FDC_SSB1_MASK : uint8_t
{
	FDC_SSB1M_ANF = 0x1,
	FDC_SSB1M_WPM = 0x2,
	FDC_SSB1M_INF = 0x4,
	FDC_SSB1M_DOR = 0x10,
	FDC_SSB1M_CER = 0x20,
	FDC_SSB1M_NDE = 0x80,
};

hd63266f_t::hd63266f_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HD63266F, tag, owner, clock),
	irq_cb(*this),
	dreq_cb(*this),
	dend_cb(*this)
{
}

void hd63266f_t::device_start()
{
	irq_cb.resolve();
	dreq_cb.resolve();
	dend_cb.resolve();
	assert(!dreq_cb.isnull() && "only DMA mode supported; supply a DREQ_W callback!");
	assert(!dend_cb.isnull() && "only DMA mode supported; supply a DEND_R callback!");
}

void hd63266f_t::reset()
{
	STR = FDC_STATUSM_TXR;
	CMD = 0x00;
	parameter_cnt = 0;
	DMA_cnt = 0;
	HSL_US = CA = HA = SA = RL = ESN = GSL = MNL = 0;
	SCNT = GP3L = DUD = 0;
	NCN = 0;
	STEP = 0;
	STR_HDUT = 0; HDLT_NDM = 0;
	LCTK = PC1_PC0 = PCDCT = 0;
	SSB0 = SSB1 = SSB2 = SSB3 = 0;
	PCN = 0;
}

void hd63266f_t::write(uint8_t data)
{
	bool input_done = false;
	auto oldSTR = STR;

	if(DMA_cnt) {
		auto length = floppy && floppy->is_open() ? floppy->length() : 0;
		auto buffer = static_cast<uint8_t*>(floppy && floppy->is_open() ? floppy->image.get() : nullptr);

		if(buffer && DMA_src < length) {
			switch(CMD & 0x1f) {
			case FDC_COMMAND_WRITE_DATA:
			case FDC_COMMAND_WRITE_DELETED_DATA:
				DMA_cnt--;
				if(dend_cb()) {
					logerror("fdc: ~TEND0 asserted; stopping DMA; DMA_cnt=%04x %s\n", DMA_cnt, callstack());
					DMA_cnt = 0;
				}
				logerror("fdc: read DMA; DMA_cnt=%04x DMA_src=%08x %s\n", DMA_cnt, DMA_src, callstack());
				if(DMA_cnt == 0) {
					// indicate result is ready
					STR = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
					if(!irq_cb.isnull()) irq_cb(true);
				} else {
					// notify DMA to write the next byte
					dreq_cb(true);
				}
				buffer[DMA_src++] = data;
				floppy->dirty = true;
				break;
			default:
				logerror("fdc: DMA active for unknown command %s\n", callstack());
				break;
			}
		} else {
			logerror("fdc: DMA out of range of floppy image %s\n", callstack());
			return;
		}
	} else {
		if(!(STR & FDC_STATUSM_BSY)) {
			// write command code
			CMD = data;
			STR |= FDC_STATUSM_BSY;
			STR &= ~FDC_STATUSM_TXR;

			switch(CMD & 0x1f) {
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
				input_done = true;
				break;
			default:
				// ready to receive command parameters
				parameter_cnt = 0;
				STR |= FDC_STATUSM_TXR;
				break;
			}
			logerror("fdc:write_cmd(%02x) STR=%02x => %02x %s\n", data, oldSTR, STR, callstack());
		} else {
			// write command parameter
			switch(CMD & 0x1f) {
			case FDC_COMMAND_READ_DATA:
			case FDC_COMMAND_READ_DELETED_DATA:
			case FDC_COMMAND_READ_ERRONEOUS_DATA:
			case FDC_COMMAND_WRITE_DATA:
			case FDC_COMMAND_WRITE_DELETED_DATA:
			case FDC_COMMAND_READ_LONG:
			case FDC_COMMAND_WRITE_LONG:
				switch(parameter_cnt) {
				case 0: HSL_US = data; break;
				case 1: CA = data; break;
				case 2: HA = data; break;
				case 3: SA = data; break;
				case 4: RL = data; break;
				case 5: ESN = data; break;
				case 6: GSL = data; break;
				case 7: MNL = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_READ_ID:
			case FDC_COMMAND_RECALIBRATE:
			case FDC_COMMAND_CHECK_DEVICE_STATUS:
				switch(parameter_cnt) {
				case 0: HSL_US = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_WRITE_FORMAT:
				switch(parameter_cnt) {
				case 0: HSL_US = data; break;
				case 1: RL = data; break;
				case 2: SCNT = data; break;
				case 3: GP3L = data; break;
				case 4: DUD = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_SEEK:
				switch(parameter_cnt) {
				case 0: HSL_US = data; break;
				case 1: NCN = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_COMPARE_EQUAL:
			case FDC_COMMAND_COMPARE_LOW_OR_EQUAL:
			case FDC_COMMAND_COMPARE_HIGH_OR_EQUAL:
				switch(parameter_cnt) {
				case 0: HSL_US = data; break;
				case 1: CA = data; break;
				case 2: HA = data; break;
				case 3: SA = data; break;
				case 4: RL = data; break;
				case 5: ESN = data; break;
				case 6: GSL = data; break;
				case 7: STEP = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
			default: // INVALID
				logerror("%s: no more parameters\n", callstack());
				break;
			case FDC_COMMAND_SPECIFY_1:
				switch(parameter_cnt) {
				case 0: STR_HDUT = data; break;
				case 1: HDLT_NDM = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_SPECIFY_2:
				switch(parameter_cnt) {
				case 0: STR_HDUT = data; break;
				case 1: HDLT_NDM = data; break;
				case 2: LCTK = data; break;
				case 3: PC1_PC0 = data; break;
				case 4: PCDCT = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			}

			// ready to receive command parameters
			parameter_cnt++;
			if(!input_done)
				STR |= FDC_STATUSM_TXR;

			logerror("fdc:write_param(%02x) STR=%02x => %02x %s\n", data, oldSTR, STR, callstack());
		}
		if(input_done)
			execute();
	}
}

uint8_t hd63266f_t::read()
{
	bool output_done = false;

	auto oldSTR = STR;

	if(DMA_cnt) {
		auto length = floppy && floppy->is_open() ? floppy->length() : 0;
		auto buffer = static_cast<uint8_t*>(floppy && floppy->is_open() ? floppy->image.get() : nullptr);

		if(buffer && DMA_src < length) {
			switch(CMD & 0x1f) {
			case FDC_COMMAND_READ_DATA:
			case FDC_COMMAND_READ_DELETED_DATA:
			case FDC_COMMAND_READ_ERRONEOUS_DATA:
				DMA_cnt--;
				if(dend_cb()) {
					logerror("fdc: ~TEND0 asserted; stopping DMA; DMA_cnt=%04x %s\n", DMA_cnt, callstack());
					DMA_cnt = 0;
				}
				//logerror("fdc: read DMA; DMA_cnt=%04x DMA_src=%08x %s\n", DMA_cnt, DMA_src, callstack());
				if(DMA_cnt == 0) {
					// indicate result is ready
					STR = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
					if(!irq_cb.isnull()) irq_cb(true);
				} else {
					// notify DMA that next byte is ready
					dreq_cb(true);
				}
				return buffer[DMA_src++];
				break;
			default:
				logerror("fdc: DMA active for unknown command %s\n", callstack());
				break;
			}
		} else {
			logerror("fdc: DMA out of range of floppy image %s\n", callstack());
			return 0;
		}
	} else {
		// read result parameters
		if((STR & (FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY)) == (FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY)) {
			switch(CMD & 0x1f) {
			case FDC_COMMAND_READ_DATA:
			case FDC_COMMAND_READ_DELETED_DATA:
			case FDC_COMMAND_READ_ERRONEOUS_DATA:
			case FDC_COMMAND_READ_ID:
			case FDC_COMMAND_WRITE_DATA:
			case FDC_COMMAND_WRITE_DELETED_DATA:
			case FDC_COMMAND_WRITE_FORMAT:
			case FDC_COMMAND_COMPARE_EQUAL:
			case FDC_COMMAND_COMPARE_HIGH_OR_EQUAL:
			case FDC_COMMAND_COMPARE_LOW_OR_EQUAL:
			case FDC_COMMAND_READ_LONG:
			case FDC_COMMAND_WRITE_LONG:
				switch(parameter_cnt) {
				case 0: DTR = SSB0; break;
				case 1: DTR = SSB1; break;
				case 2: DTR = SSB2; break;
				case 3: DTR = CA; break;
				case 4: DTR = HA; break;
				case 5: DTR = SA; break;
				case 6: DTR = RL; output_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_SEEK:
			case FDC_COMMAND_RECALIBRATE:
			case FDC_COMMAND_SPECIFY_1:
			case FDC_COMMAND_SPECIFY_2:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
				logerror("%s: no more parameters\n", callstack());
				break;
			case FDC_COMMAND_CHECK_DEVICE_STATUS:
			default: // INVALID
				switch(parameter_cnt) {
				case 0: DTR = SSB3; output_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
				switch(parameter_cnt) {
				case 0: DTR = SSB0; break;
				case 1: DTR = PCN; output_done = true; break;
				default: logerror("%s: no more parameters\n", callstack());
				}
				break;
			}
			parameter_cnt++;

			STR &= ~FDC_STATUSM_TXR;
			STR &= ~FDC_STATUSM_DIR;
			if(!irq_cb.isnull()) irq_cb(false);

			if(output_done) {
				// enter command waiting state
				STR |= FDC_STATUSM_TXR;
				STR &= ~FDC_STATUSM_BSY;
			} else {
				// signal next result parameter is ready
				STR |= FDC_STATUSM_TXR;
				STR |= FDC_STATUSM_DIR;
			}
		}
	}

	logerror("fdc:read STR=%02x => %02x %s\n", oldSTR, STR, callstack());

	return DTR;
}

void hd63266f_t::execute()
{
	SSB0 = SSB1 = SSB2 = SSB3 = 0;

	auto oldSTR = STR;

	auto readwrite_params = [this] {
		return string_format("HSL_US=%02x CA=%02x HA=%02x SA=%02x RL=%02x ESN=%02x GSL=%02x MNL=%02x", HSL_US, CA, HA, SA, RL, ESN, GSL, MNL);
	};

	auto length = floppy && floppy->is_open() ? floppy->length() : 0;
	auto buffer = floppy && floppy->is_open() ? floppy->image.get() : nullptr;

	// do something
	switch(CMD & 0x1f) {
	case FDC_COMMAND_READ_DATA: {
		// 720kb  floppy: 80 cylinders, 2 heads,  9 each 512b sectors per track
		// 1.44mb floppy: 80 cylinders, 2 heads, 18 each 512b sectors per track
		auto sector_length = 128 << (RL & 0b111);
		auto sectors_per_track = (STR_HDUT >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		DMA_src = (((CA << 1) | (HA & 1)) * sectors_per_track + SA - 1) * sector_length;
		DMA_cnt = (ESN - SA - 1 + 1) * sector_length;
		STR = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
		dreq_cb(true);
		logerror("%s: fdc: execute: READ DATA; %s DMA_cnt=%04x DMC_src=%08x\n", pc(), readwrite_params(), DMA_cnt, DMA_src);
		break;
	}
	case FDC_COMMAND_READ_DELETED_DATA:
		logerror("%s: fdc: execute: READ DELETED DATA; %s\n", pc(), readwrite_params());
		break;
	case FDC_COMMAND_READ_ERRONEOUS_DATA:
		logerror("%s: fdc: execute: READ ERRONEOUS DATA; %s\n", pc(), readwrite_params());
		break;
	case FDC_COMMAND_READ_ID:
		logerror("%s: fdc: execute: READ ID\n", pc());
		break;
	case FDC_COMMAND_WRITE_DATA: {
		auto sector_length = 128 << (RL & 0b111);
		auto sectors_per_track = (STR_HDUT >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		DMA_src = (((CA << 1) | (HA & 1)) * sectors_per_track + SA - 1) * sector_length;
		DMA_cnt = (ESN - SA - 1 + 1) * sector_length;
		STR = FDC_STATUSM_TXR | FDC_STATUSM_BSY;
		dreq_cb(true);
		logerror("%s: fdc: execute: WRITE DATA; %s DMA_cnt=%04x DMC_src=%08x\n", pc(), readwrite_params(), DMA_cnt, DMA_src);
		break;
	}
	case FDC_COMMAND_WRITE_DELETED_DATA:
		logerror("%s: fdc: execute: WRITE DELETED DATA; %s\n", pc(), readwrite_params());
		break;
	case FDC_COMMAND_WRITE_FORMAT: {
		auto sector_length = 128 << (RL & 0b111);
		auto sectors_per_track = (STR_HDUT >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		auto dst = ((PCN << 1) | ((HSL_US & 0b100) >> 2)) * sectors_per_track * sector_length;
		auto cnt = SCNT * sector_length >> 1; // not sure why >> 1, but matches how it's called
		logerror("%s: fdc: execute: WRITE FORMAT; HSL_US=%02x PCN=%02x SCNT=%02x RL=%02x DUD=%02x cnt=%04x ofs=%08x\n", pc(), HSL_US, PCN, SCNT, RL, DUD, cnt, dst);
		while(cnt--) {
			if(dst < length)
				buffer[dst++] = DUD;
		}
		break;
	}
	case FDC_COMMAND_SEEK:
		logerror("%s: fdc: execute: SEEK NCN=%d\n", pc(), NCN);
		SSB0 |= FDC_SSB0M_SED;
		PCN = NCN;
		if(NCN == 0)
			SSB3 |= FDC_SSB3M_TRZ;
		else
			SSB3 &= ~FDC_SSB3M_TRZ;
		break;
	case FDC_COMMAND_RECALIBRATE:
		logerror("%s: fdc: execute: RECALIBRATE\n", pc());
		SSB0 |= FDC_SSB0M_SED;
		SSB3 |= FDC_SSB3M_TRZ;
		PCN = 0;
		break;
	case FDC_COMMAND_COMPARE_EQUAL:
		logerror("%s: fdc: execute: COMPARE EQUAL\n", pc());
		break;
	case FDC_COMMAND_COMPARE_LOW_OR_EQUAL:
		logerror("%s: fdc: execute: COMPARE LOW OR EQUAL\n", pc());
		break;
	case FDC_COMMAND_COMPARE_HIGH_OR_EQUAL:
		logerror("%s: fdc: execute: COMPARE HIGH OR EQUAL\n", pc());
		break;
	case FDC_COMMAND_CHECK_DEVICE_STATUS:
		logerror("%s: fdc: execute: CHECK DEVICE STATUS\n", pc());
		break;
	case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
		logerror("%s: fdc: execute: CHECK INTERRUPT STATUS\n", pc());
		SSB0 &= ~FDC_SSB0M_HSL;
		break;
	case FDC_COMMAND_SPECIFY_1:
		logerror("%s: fdc: execute: SPECIFY 1\n", pc());
		break;
	case FDC_COMMAND_SPECIFY_2:
		logerror("%s: fdc: execute: SPECIFY 2\n", pc());
		break;
	case FDC_COMMAND_SLEEP:
		logerror("%s: fdc: execute: SLEEP\n", pc());
		break;
	case FDC_COMMAND_ABORT:
		logerror("%s: fdc: execute: ABORT\n", pc());
		break;
	case FDC_COMMAND_READ_LONG:
		logerror("%s: fdc: execute: READ LONG\n", pc());
		break;
	case FDC_COMMAND_WRITE_LONG:
		logerror("%s: fdc: execute: WRITE LONG\n", pc());
		break;
	}

	SSB0 = FDC_SSB0M_INC_NOR;
	SSB0 |= HSL_US & 0b11; // mirror US1, US0 from command parameters
	SSB3 |= HSL_US & 0b111; // mirror HSL, US1, US0 from command parameters
	if(floppy && floppy->loaded)
		SSB3 |= FDC_SSB3M_RDY;
	else {
		SSB0 |= FDC_SSB0M_DNR;
		SSB3 &= ~FDC_SSB3M_RDY;
	}

	parameter_cnt = 0;

	if(!DMA_cnt) {
		switch(CMD & 0x1f) {
		case FDC_COMMAND_SEEK:
		case FDC_COMMAND_RECALIBRATE:
		case FDC_COMMAND_SPECIFY_1:
		case FDC_COMMAND_SPECIFY_2:
		case FDC_COMMAND_SLEEP:
		case FDC_COMMAND_ABORT:
			// no result
			STR = FDC_STATUSM_TXR;
			break;
			//[[fallthrough]]
			// no IRQ
		case FDC_COMMAND_CHECK_DEVICE_STATUS:
		case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
		default: // INVALID
			// indicate result is ready
			STR = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
			break;
		}

		// set IRQ to indicate result is ready
		switch(CMD & 0x1f) {
		case FDC_COMMAND_CHECK_DEVICE_STATUS:
		case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			break;
		default:
			if(!irq_cb.isnull()) irq_cb(true); // (p.34)
		   // TODO: not for INVALID
			break;
		}
	}

	logerror("fdc:execute STR=%02x => %02x %s\n", oldSTR, STR, callstack());
}

void hd63266f_t::abort()
{
	auto oldSTR = STR;
	reset();

	logerror("fdc:abort STR=%02x => %02x %s\n", oldSTR, STR, callstack());
}

#pragma endregion

class lw350_state : public driver_device
{
public:
	lw350_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		palette(*this, "palette"),
		io_kbrow(*this, "kbrow.%u", 0),
		fdc(*this, "fdc"),
		floppy(*this, "floppy"),
		beeper(*this, "beeper")
	{ }

	// helpers
	std::string pc();
	std::string symbolize(uint32_t adr);
	std::string callstack();

	// devices
	required_device<z180_device> maincpu;
	required_device<palette_device> palette;
	required_device<lw350_floppy_connector> floppy;
	required_device<hd63266f_t> fdc;
	required_device<beep_device> beeper;
	optional_ioport_array<9> io_kbrow;

	// screen updates
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	DECLARE_READ8_MEMBER(illegal_r); DECLARE_WRITE8_MEMBER(illegal_w);

	// ROM
	DECLARE_READ8_MEMBER(rom40000_r);
	DECLARE_READ8_MEMBER(rom72000_r);

	// IO
	DECLARE_READ8_MEMBER(illegal_io_r); DECLARE_WRITE8_MEMBER(illegal_io_w);
	DECLARE_WRITE8_MEMBER(io_70_w);
	DECLARE_READ8_MEMBER(io_74_r);
	DECLARE_READ8_MEMBER(io_a8_r);
	DECLARE_READ8_MEMBER(io_b8_r); DECLARE_WRITE8_MEMBER(io_b8_w);
	DECLARE_WRITE8_MEMBER(rombank_w); // E0
	DECLARE_WRITE8_MEMBER(beeper_w); // F0
	DECLARE_WRITE8_MEMBER(irqack_w); // F8

	// Floppy
	DECLARE_WRITE8_MEMBER(fdc_dtr_w); // 79
	DECLARE_READ8_MEMBER(io_7a_r); // 7a
	DECLARE_READ8_MEMBER(io_7e_r); DECLARE_WRITE8_MEMBER(io_7e_w); // 7e
	DECLARE_READ8_MEMBER(io_90_r); // 90
	TIMER_DEVICE_CALLBACK_MEMBER(io_90_timer_callback);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w) { io_7a &= ~0x40; if(state) io_7a |= 0x40; }
	DECLARE_WRITE_LINE_MEMBER(fdc_dreq_w) { maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE); }
	DECLARE_READ_LINE_MEMBER(fdc_dend_r) { return maincpu->get_tend0(); }

	// VRAM
	DECLARE_READ8_MEMBER(vram_r); DECLARE_WRITE8_MEMBER(vram_w);

	uint8_t vram[80*128];
	uint8_t io_70, io_7a, io_b8, io_90;
	uint8_t rombank;

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	uint8_t* rom{};
	std::map<uint32_t, std::string> symbols;
};

static ADDRESS_MAP_START( lw350_map, AS_PROGRAM, 8, lw350_state )
	AM_RANGE(0x00000, 0x01fff) AM_ROM
	AM_RANGE(0x02000, 0x05fff) AM_RAM
	AM_RANGE(0x06000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x5ffff) AM_READ(rom40000_r) // => ROM 0x40000 or 0x60000 or ??? (bank switching IO E0)
	AM_RANGE(0x60000, 0x617ff) AM_RAM
	AM_RANGE(0x61800, 0x63fff) AM_READWRITE(vram_r, vram_w)
	AM_RANGE(0x64000, 0x71fff) AM_RAM
	AM_RANGE(0x72000, 0x75fff) AM_READ(rom72000_r) // => ROM 0x02000-0x05fff
	AM_RANGE(0x76000, 0x7ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( lw350_io, AS_IO, 8, lw350_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP // Z180 internal registers
	AM_RANGE(0x70, 0x70) AM_WRITE(io_70_w)
	AM_RANGE(0x74, 0x74) AM_READ(io_74_r)

	// floppy
	AM_RANGE(0x78, 0x78) AM_DEVREADWRITE("fdc", hd63266f_t, status_r, abort_w)
	AM_RANGE(0x79, 0x79) AM_DEVREAD("fdc", hd63266f_t, data_r) AM_WRITE(fdc_dtr_w)
	AM_RANGE(0x7a, 0x7a) AM_READ(io_7a_r)
	AM_RANGE(0x7e, 0x7e) AM_READWRITE(io_7e_r, io_7e_w)
	AM_RANGE(0x90, 0x90) AM_READ(io_90_r)

	// printer
	AM_RANGE(0xa8, 0xa8) AM_READ(io_a8_r)

	AM_RANGE(0xb8, 0xb8) AM_READWRITE(io_b8_r, io_b8_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(rombank_w)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(beeper_w)
	AM_RANGE(0xf8, 0xf8) AM_WRITE(irqack_w)

	AM_RANGE(0x40, 0xff) AM_READWRITE(illegal_io_r, illegal_io_w)
ADDRESS_MAP_END

void lw350_state::video_start()
{
}

std::string lw350_state::pc()
{
	class z180_friend : z180_device { friend class lw350_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t phys = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, phys);

	return symbolize(phys);
}

std::string lw350_state::symbolize(uint32_t adr)
{
	if(symbols.empty())
		return string_format("%06x", adr);

	auto floor_it = symbols.lower_bound(adr);
	if((floor_it == symbols.end() && !symbols.empty()) || floor_it->first != adr)
		--floor_it;
	if(floor_it != symbols.end())
		return string_format("%s+%x (%06x)", floor_it->second, adr - floor_it->first, adr);
	else
		return string_format("%06x", adr);
}

std::string lw350_state::callstack()
{
	class z180_friend : z180_device { friend class lw350_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t pc = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, pc);

	int depth = 0;
	std::string output;
	output += symbolize(pc) + " >> ";

//	if(output.find("abort") != std::string::npos)
//		__debugbreak();

	// floppy routines
	if(pc >= 0x73000 && pc <= 0x75000) {
		offs_t sp = cpu->sp();
		for(int i = 0; i < 4; i++) {
			offs_t back = cpu->space(AS_PROGRAM).read_word_unaligned(sp);
			cpu->memory_translate(AS_PROGRAM, 0, back);
			if(back >= 0x73000 && back <= 0x75000) {
				output += symbolize(back) + " >> ";
				depth++;
				if(depth < 8)
					i = 0;
			}
			sp += 2;
		}
	}

	return output;
}

uint32_t lw350_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	// video on?
	if(!BIT(io_70, 0))
		return 0;

	// backlight on?
	//if(BIT(io_70, 7))
		//...


	const rgb_t *palette = this->palette->palette()->entry_list_raw();

	for(auto y = 0; y < 128; y++) {
		uint32_t* p = &bitmap.pix32(y);
		for(auto x = 0; x < 640; x += 8) {
			auto gfx = vram[y * 80 + x / 8];
			//*p++ = palette[BIT(gfx, 7)];
			//*p++ = palette[BIT(gfx, 6)];
			*p++ = palette[BIT(gfx, 5)];
			*p++ = palette[BIT(gfx, 4)];
			*p++ = palette[BIT(gfx, 3)];
			*p++ = palette[BIT(gfx, 2)];
			*p++ = palette[BIT(gfx, 1)];
			*p++ = palette[BIT(gfx, 0)];
		}
	}
	return 0;
}

READ8_MEMBER(lw350_state::rom40000_r)
{
	if(rombank == 0x02)
		return rom[0x40000 + offset] & mem_mask;
	else if(rombank == 0x03)
		return rom[0x60000 + offset] & mem_mask;
	else {
		space.device().logerror("%s: illegal rombank (IO E0=%02x) read offset %06x \n", pc(), rombank, space.byte_to_address(offset));
		return 0x00;
	}
}

READ8_MEMBER(lw350_state::rom72000_r)
{
	return rom[0x02000 + offset] & mem_mask;
}

READ8_MEMBER(lw350_state::vram_r)
{
	return vram[offset] & mem_mask;
}

WRITE8_MEMBER(lw350_state::vram_w)
{
	vram[offset] = (vram[offset] & ~mem_mask) | data;
}

#pragma region(LW-350 Keyboard)

static INPUT_PORTS_START(lw350)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)      PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)      PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)      PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)      PORT_CHAR('x')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)    PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)      PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)      PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)      PORT_CHAR('c')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)      PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)      PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)      PORT_CHAR('h')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)      PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)      PORT_CHAR('v')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G.S.END")               PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(L'ß') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Inhalt")                PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(L'ö') PORT_CHAR(L'Ö')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(L'ü') PORT_CHAR(L'Ü')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SM/Layout")             PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORNO")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Horz/Vert")             //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(L'´') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)	     PORT_CHAR(L'ä') PORT_CHAR(L'Ä')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

#pragma endregion

READ8_MEMBER(lw350_state::illegal_r)
{
	space.device().logerror("%s: unmapped %s memory read from %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, mem_mask);
	return 0;
}

WRITE8_MEMBER(lw350_state::illegal_w)
{
	space.device().logerror("%s: unmapped %s memory write to %0*X = %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, data, 2, mem_mask);
}

READ8_MEMBER(lw350_state::illegal_io_r)
{
	space.device().logerror("%s: unmapped %s memory read from %0*X & %0*X\n", callstack(), space.name(), space.addrchars(), space.byte_to_address(offset + 0x40), 2, mem_mask);
	return 0;
}

WRITE8_MEMBER(lw350_state::illegal_io_w)
{
	space.device().logerror("%s: unmapped %s memory write to %0*X = %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset + 0x40), 2, data, 2, mem_mask);
}

TIMER_DEVICE_CALLBACK_MEMBER(lw350_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

WRITE8_MEMBER(lw350_state::rombank_w)
{
	rombank = data;
}

WRITE8_MEMBER(lw350_state::beeper_w)
{
	beeper->set_state(data == 0);
}

WRITE8_MEMBER(lw350_state::irqack_w)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

WRITE8_MEMBER(lw350_state::io_70_w)
{
	io_70 = data;
}

READ8_MEMBER(lw350_state::io_74_r)
{
	// 0x00: 7 lines display (64 pixels height)
	// 0x80: 14 lines display (128 pixels height)
	return 0x80;
}

READ8_MEMBER(lw350_state::io_a8_r)
{
	// bit 0: case open
	// bit 2: carriage return indicator
	//return 0x01; // case open
	return 0x00;
}

// Floppy
//////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(lw350_state::fdc_dtr_w)
{
	fdc->set_floppy(floppy->get_device());
	fdc->write(data);
}

READ8_MEMBER(lw350_state::io_7a_r)
{
	return io_7a;
}

READ8_MEMBER(lw350_state::io_7e_r)
{
	return 0x80; // 1.44mb floppy
}

WRITE8_MEMBER(lw350_state::io_7e_w)
{
}

READ8_MEMBER(lw350_state::io_90_r)
{
	return io_90;
}

TIMER_DEVICE_CALLBACK_MEMBER(lw350_state::io_90_timer_callback)
{
	if(floppy && floppy->get_device() && floppy->get_device()->loaded)
		io_90 ^= 1 << 6; // int1_process_floppy wants this to toggle, 150 ms, HACK
}

// Keyboard
//////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(lw350_state::io_b8_w)
{
	io_b8 = data;
}

READ8_MEMBER(lw350_state::io_b8_r)
{
	// keyboard matrix
 	if(io_b8 <= 8) {
		if(io_kbrow[io_b8].found()) {
			return io_kbrow[io_b8].read_safe(0);
		}
 	}

	switch(io_b8) {
	// get language
	case 0x09:  // valid results (see get_index_from_language)
		//return ~0x20; // french
		//return ~0x10; // french
		//return ~0x08; // german
		//return ~0x04; // french
		//return ~0x02; // french
		//return ~0x01; // german
		return ~0x00; // german
	default:   return 0x00;
	}
}

//uint8_t char_attribute = 0x00;

void lw350_state::machine_start()
{
	// try to load map file
	FILE* f;
	if(fopen_s(&f, "lw350.map", "rt") == 0) {
		char line[512];
		do {
			if(fgets(line, sizeof(line), f)) {
				int segment, offset;
				char symbol[512];
				if(sscanf(line, "%x:%x %512s", &segment, &offset, symbol) == 3) {
					uint32_t phys = (segment << 4) + offset;
					//TRACE(_T("%04x:%04x => %02x:%04x\n"), segment, offset, bank, offset);
					symbols[phys] = symbol;
				}
			}
		} while(!feof(f));
		fclose(f);
	}

	rom = memregion("maincpu")->base();

	// force jump to self-test menu
//	rom[0x280f2] = 0x20;
//	rom[0x280f2+1] = 0x05;

	// force jump to lcd-test menu
//	rom[0x280f2] = 0x2c;
//	rom[0x280f2+1] = 0x05;

	// force jump to self-print menu
//	rom[0x280f2] = 0x32;
//	rom[0x280f2 + 1] = 0x05;

	// set initial mode
	//rom[0x29a12] = 0x0a;

	// patch out printer init
	rom[0x280df] = 0x00;

	// force RAM DOWN
	//rom[0x280c3] = rom[0x280be];
	//rom[0x280c3 + 1] = rom[0x280be + 1];

	// patch out self test bit 5 check; causes reboot loop
	//rom[0x280c0] = 0x00;
	//rom[0x280c1] = 0x00;
	//rom[0x280c2] = 0x00;

	// force bold font
	//rom[0x6b29] = 0x00;
	//rom[0x6b29+1] = 0x00;

	// char attributes
	//rom[0x6b47] = 0x3e;
	//rom[0x6b48] = char_attribute;
	//rom[0x6b49] = 0x00;
	//rom[0x6b4a] = 0x00;
	//rom[0x6b4b] = 0x00;
	//rom[0x6b4c] = 0x00;
}

void lw350_state::machine_reset()
{
	io_90 = 0x00;
	io_7a = 0x01; // int1_process_floppy waits for bit 0 set
	fdc->reset();
}

static SLOT_INTERFACE_START(lw350_floppies)
	SLOT_INTERFACE("35hd", LW350_FLOPPY)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( lw350 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z180, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(lw350_map)
	MCFG_CPU_IO_MAP(lw350_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("1khz", lw350_state, int1_timer_callback, attotime::from_hz(1000))

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t(6, 245, 206))
	MCFG_SCREEN_REFRESH_RATE(78.1)
	MCFG_SCREEN_UPDATE_DRIVER(lw350_state, screen_update)
	MCFG_SCREEN_SIZE(480, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 128-1)
	MCFG_PALETTE_ADD_MONOCHROME_INVERTED("palette")

	// floppy disk
	MCFG_TIMER_DRIVER_ADD_PERIODIC("io_90", lw350_state, io_90_timer_callback, attotime::from_msec(160))
	MCFG_DEVICE_ADD("floppy", LW350_FLOPPY_CONNECTOR, 0);
	MCFG_DEVICE_SLOT_INTERFACE(lw350_floppies, "35hd", false);
	MCFG_HD63266F_ADD("fdc", XTAL_16MHz);
	MCFG_HD63266F_IRQ_CALLBACK(WRITELINE(lw350_state, fdc_irq_w))
	MCFG_HD63266F_DREQ_CALLBACK(WRITELINE(lw350_state, fdc_dreq_w))
	MCFG_HD63266F_DEND_CALLBACK(READLINE(lw350_state, fdc_dend_r))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 4000) // 4.0 kHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

#pragma endregion

#pragma region(LW-450)

//////////////////////////////////////////////////////////////////////////
// LW-450
//////////////////////////////////////////////////////////////////////////

/***************************************************************************

Brother LW-450
1992

Hardware:

#13
Hitachi
HD64180RF6X
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
6 MHz, FP-80B, Address Space 1 M Byte
MuRata CST12MTW 12.00 MHz Ceramic Resonator

#12
Mitsubishi
M65020FP
UA7777-A

#15
Hitachi
HD74LS368AP

#10, #11
Hitachi
HD74LS157P

#7, #8
NEC
D41464C-10
65,536 x 4-Bit
Dynamic NMOS RAM
100 ns
18-pin plastic DIP

#1
NEC
D23C4001EC-172
UA2849-A
4MBit Mask ROM

#2
AMD
AM27C020
2BC04
2MBit (256K x 8-Bit) CMOS EPROM

#3
Hitachi
HM65256BSP-10
32,768 word x 8-bit High Speed Pseudo Static RAM
100 ns
DP-28N

#4
Hitachi HD63266F
CMOS Floppy Disk Controller
QFP-64
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#5
Hitachi
HD6445P4
CRTC-II (CRT Controller)
DP-40
80 system Bus Interface
4.0 MHz Bus Timing

#9
Mitsubishi
M65133FP
UA7550-A

#14
Hitachi
HM62256AF-8
32,768 word x 8-bit High Speed CMOS Static RAM
85 ns
FP-28

720kb Floppy Drive

D-SUB9 CRT Connector
HSYNC 18.5 kHz
VSYNC 50 Hz
PC MDA standard
  DB9 connector 1+2 GND, 3+4+5 nc, 6 Intensity, 7 Brightness, 8 HSync, 9 VSync
  HSync positive, VSync negative active

Emulation Status:
Printer not working
Floppy Read has some problems (directory working, but LW-450 reports illegal file format when trying to load a .wpt file (content verified with LW-350), but writing seems fine)
Dictionary ROM probably not correctly mapped

***************************************************************************/

constexpr int MDA_CLOCK = 16257000;

class lw450_state : public driver_device
{
public:
	lw450_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		palette(*this, "palette"),
		io_kbrow(*this, "kbrow.%u", 0),
		vram(*this, "vram"),
		m_crtc(*this, "hd6445"),
		fdc(*this, "fdc"),
		floppy(*this, "floppy"),
		speaker(*this, "beeper")
	{ }

	// helpers
	std::string pc();
	std::string symbolize(uint32_t adr);
	std::string callstack();

	// devices
	required_device<z180_device> maincpu;
	required_device<palette_device> palette;
	required_device<lw350_floppy_connector> floppy;
	required_device<hd63266f_t> fdc;
	required_device<hd6345_device> m_crtc;
	required_shared_ptr<uint8_t> vram;
	required_device<beep_device> speaker;
	optional_ioport_array<9> io_kbrow;

	DECLARE_READ8_MEMBER(illegal_r); DECLARE_WRITE8_MEMBER(illegal_w);

	// ROM
	DECLARE_READ8_MEMBER(rom40000_r);
	DECLARE_READ8_MEMBER(rom72000_r);

	// IO
	DECLARE_READ8_MEMBER(illegal_io_r); DECLARE_WRITE8_MEMBER(illegal_io_w);
	DECLARE_READ8_MEMBER(io_b0_r);
	DECLARE_READ8_MEMBER(io_b8_r); DECLARE_WRITE8_MEMBER(io_b8_w);
	DECLARE_WRITE8_MEMBER(rombank_w); // E0
	DECLARE_WRITE8_MEMBER(beeper_w); // F0
	DECLARE_WRITE8_MEMBER(irqack_w); // F8

	// Floppy
	DECLARE_WRITE8_MEMBER(fdc_dtr_w); // 79
	DECLARE_READ8_MEMBER(io_7a_r); // 7a
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w) { io_7a &= ~0x40; if(state) io_7a |= 0x40; }
	DECLARE_WRITE_LINE_MEMBER(fdc_dreq_w) { maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE); }
	DECLARE_READ_LINE_MEMBER(fdc_dend_r) { return maincpu->get_tend0(); }

	// CRTC
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
	DECLARE_WRITE8_MEMBER(io_72_w) { io_72 = data; }
	DECLARE_WRITE8_MEMBER(io_73_w) { io_73 = data; }
	DECLARE_WRITE8_MEMBER(io_74_w);
	DECLARE_WRITE8_MEMBER(io_75_w) { io_75 = data; }

	uint8_t io_72, io_73, io_74, io_75; // gfx
	uint8_t io_7a, io_b8, rombank;
	uint32_t framecnt;

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	uint8_t* rom{};
	uint8_t* dict_rom{};
	uint8_t framebuffer[720 * 350]; // pixel data
	std::map<uint32_t, std::string> symbols;
};

static ADDRESS_MAP_START( lw450_map, AS_PROGRAM, 8, lw450_state )
	AM_RANGE(0x00000, 0x01fff) AM_ROM
	AM_RANGE(0x02000, 0x05fff) AM_RAM
	AM_RANGE(0x06000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x5ffff) AM_READ(rom40000_r) // dictionary ROM, banked according to I/O E0
	AM_RANGE(0x62000, 0x71fff) AM_RAM // D-RAM UPPER/LOWER
	AM_RANGE(0x72000, 0x75fff) AM_READ(rom72000_r) // => ROM 0x02000-0x05fff
	AM_RANGE(0x78000, 0x7ffff) AM_RAM // PS-RAM
	AM_RANGE(0xf8000, 0xfffff) AM_RAM AM_SHARE("vram") // VRAM
	// text vram @ F8000-F8C80 (2*80 bytes/line)
	// font @ FC000-FD000 pitch 16
	ADDRESS_MAP_END

static ADDRESS_MAP_START( lw450_io, AS_IO, 8, lw450_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP // Z180 internal registers

	AM_RANGE(0x70, 0x70) AM_DEVWRITE("hd6445", hd6345_device, address_w)
	AM_RANGE(0x71, 0x71) AM_DEVWRITE("hd6445", hd6345_device, register_w)
	AM_RANGE(0x72, 0x72) AM_WRITE(io_72_w)
	AM_RANGE(0x73, 0x73) AM_WRITE(io_73_w)
	AM_RANGE(0x74, 0x74) AM_WRITE(io_74_w)
	AM_RANGE(0x75, 0x75) AM_WRITE(io_75_w)

	// floppy
	AM_RANGE(0x78, 0x78) AM_DEVREADWRITE("fdc", hd63266f_t, status_r, abort_w)
	AM_RANGE(0x79, 0x79) AM_DEVREAD("fdc", hd63266f_t, data_r) AM_WRITE(fdc_dtr_w)
	AM_RANGE(0x7a, 0x7a) AM_READ(io_7a_r)

	AM_RANGE(0xb0, 0xb0) AM_READ(io_b0_r)
	AM_RANGE(0xb8, 0xb8) AM_READWRITE(io_b8_r, io_b8_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(rombank_w)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(beeper_w)
	AM_RANGE(0xf8, 0xf8) AM_WRITE(irqack_w)

	AM_RANGE(0x40, 0xff) AM_READWRITE(illegal_io_r, illegal_io_w)
ADDRESS_MAP_END

void lw450_state::video_start()
{
}

std::string lw450_state::pc()
{
	class z180_friend : z180_device { friend class lw450_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t phys = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, phys);

	return symbolize(phys);
}

std::string lw450_state::symbolize(uint32_t adr)
{
	if(symbols.empty())
		return string_format("%06x", adr);

	auto floor_it = symbols.lower_bound(adr);
	if((floor_it == symbols.end() && !symbols.empty()) || floor_it->first != adr)
		--floor_it;
	if(floor_it != symbols.end())
		return string_format("%s+%x (%06x)", floor_it->second, adr - floor_it->first, adr);
	else
		return string_format("%06x", adr);
}

std::string lw450_state::callstack()
{
	class z180_friend : z180_device { friend class lw450_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t pc = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, pc);

	int depth = 0;
	std::string output;
	output += symbolize(pc) + " >> ";

//	if(output.find("abort") != std::string::npos)
//		__debugbreak();
/*
	// floppy routines
	if(pc >= 0x73000 && pc <= 0x75000) {
		offs_t sp = cpu->sp();
		for(int i = 0; i < 4; i++) {
			offs_t back = cpu->space(AS_PROGRAM).read_word_unaligned(sp);
			cpu->memory_translate(AS_PROGRAM, 0, back);
			if(back >= 0x73000 && back <= 0x75000) {
				output += symbolize(back) + " >> ";
				depth++;
				if(depth < 8)
					i = 0;
			}
			sp += 2;
		}
	}
*/
	return output;
}

READ8_MEMBER(lw450_state::rom40000_r)
{
	if(rombank == 0x04)
		return dict_rom[offset] & mem_mask;
	else if(rombank == 0x05)
		return dict_rom[0x20000 + offset] & mem_mask;
	else if(rombank == 0x06)
		return dict_rom[0x40000 + offset] & mem_mask;
	else if(rombank == 0x07)
		return dict_rom[0x60000 + offset] & mem_mask;
	else {
		space.device().logerror("%s: illegal rombank (IO E0=%02x) read offset %06x \n", pc(), rombank, space.byte_to_address(offset));
		return 0x00;
	}
}

READ8_MEMBER(lw450_state::rom72000_r)
{
	return rom[0x02000 + offset] & mem_mask;
}

READ8_MEMBER(lw450_state::illegal_r)
{
	space.device().logerror("%s: unmapped %s memory read from %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, mem_mask);
	return 0;
}

WRITE8_MEMBER(lw450_state::illegal_w)
{
	space.device().logerror("%s: unmapped %s memory write to %0*X = %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, data, 2, mem_mask);
}

READ8_MEMBER(lw450_state::illegal_io_r)
{
	space.device().logerror("%s: unmapped %s memory read from %0*X & %0*X\n", callstack(), space.name(), space.addrchars(), space.byte_to_address(offset + 0x40), 2, mem_mask);
	return 0;
}

WRITE8_MEMBER(lw450_state::illegal_io_w)
{
	space.device().logerror("%s: unmapped %s memory write to %0*X = %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset + 0x40), 2, data, 2, mem_mask);
}

TIMER_DEVICE_CALLBACK_MEMBER(lw450_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

READ8_MEMBER(lw450_state::io_b0_r)
{
	return ~0x00;
}

WRITE8_MEMBER(lw450_state::rombank_w)
{
	rombank = data;
}

WRITE8_MEMBER(lw450_state::beeper_w)
{
	speaker->set_state(data == 0);
}

WRITE8_MEMBER(lw450_state::irqack_w)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

// Keyboard
//////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(lw450_state::io_b8_w)
{
	io_b8 = data;
}

READ8_MEMBER(lw450_state::io_b8_r)
{
	// keyboard matrix
	if(io_b8 <= 8) {
		if(io_kbrow[io_b8].found()) {
			return io_kbrow[io_b8].read_safe(0);
		}
	}
	return 0x00;
}

// Floppy
//////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(lw450_state::fdc_dtr_w)
{
	fdc->set_floppy(floppy->get_device());
	fdc->write(data);
}

READ8_MEMBER(lw450_state::io_7a_r)
{
	return io_7a;
}

// CRTC
//////////////////////////////////////////////////////////////////////////

WRITE_LINE_MEMBER(lw450_state::crtc_vsync) 
{ 
	if(state) {
		framecnt++;
	}
}

WRITE8_MEMBER(lw450_state::io_74_w)
{
	io_74 = data;
	if(io_74 & 0x04) {
		// graphics mode
		hd6345_device::set_char_width(*m_crtc, 8);
		m_crtc->set_clock(MDA_CLOCK / 8);
	} else {
		// text mode
		hd6345_device::set_char_width(*m_crtc, 9);
		m_crtc->set_clock(MDA_CLOCK / 9);
	}
}

//(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x, int de, int hbp, int vbp)
MC6845_UPDATE_ROW(lw450_state::crtc_update_row)
{
	// IO             72 73 74 75
	// ============================
	// typewriter:    05 02 08 f8 text     inverted
	// crt test menu: 07 02 00 f8 text
	// main menu:     05 02 0c f8 graphics inverted

	// based on LW-450 CRT Test Menu
	enum attrs {
		underline			= 0b00000001,
		extended_charset	= 0b00000100,
		bold				= 0b00001000,
		reverse				= 0b00010000,
		blink               = 0b10000000,
	};

	const rgb_t *palette = this->palette->palette()->entry_list_raw();
	uint32_t* p = &bitmap.pix32(y);
	uint16_t chr_base = ra;

	// video off
	if(!(io_72 & 0b1)) {
		for(int x = 0; x < cliprect.width(); x++)
			*p++ = palette[0];
		return;
	}

	// graphics mode
	if(io_74 & 0x04) {
		uint8_t bg = 0, fg = 1;
		// inverse display
		if(!(io_72 & 0x02))
			std::swap(bg, fg);

		for(int x = 0; x < x_count; x++) {
			auto data = vram[(ma + x + (ra << 14)) & 0x7fff];
			*p++ = palette[(data & 0x80) ? fg : bg];
			*p++ = palette[(data & 0x40) ? fg : bg];
			*p++ = palette[(data & 0x20) ? fg : bg];
			*p++ = palette[(data & 0x10) ? fg : bg];
			*p++ = palette[(data & 0x08) ? fg : bg];
			*p++ = palette[(data & 0x04) ? fg : bg];
			*p++ = palette[(data & 0x02) ? fg : bg];
			*p++ = palette[(data & 0x01) ? fg : bg];
		}
	} else {
		// text mode
		auto charram = &vram[0];
		auto fontram = &vram[0x4000];

		for(int x = 0; x < x_count; x++) {
			uint16_t offset = ((ma + x) << 1) & 0x0FFF;
			auto atr = charram[offset];
			uint32_t chr = charram[offset + 1];
			if(atr & extended_charset) chr += 0x100;
			auto data = fontram[chr_base + chr * 16];
			uint8_t bit9 = 0x00;
			if(atr & extended_charset) bit9 = BIT(data, 0) ? 0xff : 0x00;
			uint8_t bg = 0, fg = 1;

			if(atr & bold)
				fg = 2;

			// inverse display
			if(!(io_72 & 0x02))
				std::swap(bg, fg);

			if((atr & underline) && ra == 13) {
				data = bit9 = 0xff;
			}
			if(atr & reverse)
				std::swap(bg, fg);

			if(x == cursor_x) {
				data = 0xff;
				bit9 = 0x00;
			} else {
				if((atr & blink) && (framecnt & 0x10)) // TODO: check blinking frequency
					data = 0x00;
			}

			for(int b = 0; b < 8; b++)
				*p++ = BIT(data, 7 - b) ? palette[fg] : palette[bg];
			*p++ = BIT(bit9, 7) ? palette[fg] : palette[bg];
		}
	}

	// TODO: cursor
}

MC6845_ON_UPDATE_ADDR_CHANGED(lw450_state::crtc_addr)
{
}

// PIN 21 (Character Clock) of CRTC-II: menu: 2.0 MHz; schreibmaschine: 1.8 MHz

// these timings are all at MDA clock
// bootup:          [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps
// menu:            [:hd6445] M6845 config screen: HTOTAL: 990  VTOTAL: 370  MAX_X: 809  MAX_Y: 319  HSYNC: 819-953  VSYNC: 334-349  Freq: 44.381646fps
// schreibmaschine: [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps

void lw450_state::machine_start()
{
	/*
		// try to load map file
		FILE* f;
		if(fopen_s(&f, "rom.map", "rt") == 0) {
			char line[512];
			do {
				if(fgets(line, sizeof(line), f)) {
					int segment, offset;
					char symbol[512];
					if(sscanf(line, "%x:%x %512s", &segment, &offset, symbol) == 3) {
						uint32_t phys = (segment << 4) + offset;
						//TRACE(_T("%04x:%04x => %02x:%04x\n"), segment, offset, bank, offset);
						symbols[phys] = symbol;
					}
				}
			} while(!feof(f));
			fclose(f);
		}
	*/
	rom = memregion("maincpu")->base();
	dict_rom = memregion("dictionary")->base();

	palette->set_pen_color(0, rgb_t(0, 0, 0));
	palette->set_pen_color(1, rgb_t(0xaa, 0xaa, 0xaa));
	palette->set_pen_color(2, rgb_t(0xff, 0xff, 0xff));

	// patch out printer init
	rom[0x280db] = 0x00;
}

void lw450_state::machine_reset()
{
	framecnt = 0;
}

static const gfx_layout pc_16_charlayout {
	8, 16,                  // 8 x 16 characters
	256,                    // 256 characters
	1,						// 1 bits per pixel
	{ 0 },                  // no bitplanes
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, // x offsets
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8, 12 * 8, 13 * 8, 14 * 8, 15 * 8 }, 	// y offsets
	16*8                 // every char takes 2 x 8 bytes
};

static GFXDECODE_START( lw450 )
	GFXDECODE_RAM("vram", 0x4000, pc_16_charlayout, 0, 1)
GFXDECODE_END

static SLOT_INTERFACE_START(lw450_floppies)
	SLOT_INTERFACE("35dd", LW350_FLOPPY)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( lw450 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz/2)
	MCFG_CPU_PROGRAM_MAP(lw450_map)
	MCFG_CPU_IO_MAP(lw450_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("1khz", lw450_state, int1_timer_callback, attotime::from_hz(1000))

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::white())
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("hd6445", hd6345_device, screen_update)
	MCFG_SCREEN_SIZE(720, 320)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 320-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lw450)
	MCFG_PALETTE_ADD("palette", 3)

	// CRTC
	MCFG_MC6845_ADD("hd6445", HD6345, "screen", MDA_CLOCK / 9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_ADDR_CHANGED_CB(lw450_state, crtc_addr)
	MCFG_MC6845_UPDATE_ROW_CB(lw450_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(lw450_state, crtc_vsync))

	MCFG_DEVICE_ADD("floppy", LW350_FLOPPY_CONNECTOR, 0);
	MCFG_DEVICE_SLOT_INTERFACE(lw450_floppies, "35dd", false);
	MCFG_HD63266F_ADD("fdc", XTAL_16MHz);
	MCFG_HD63266F_IRQ_CALLBACK(WRITELINE(lw450_state, fdc_irq_w))
	MCFG_HD63266F_DREQ_CALLBACK(WRITELINE(lw450_state, fdc_dreq_w))
	MCFG_HD63266F_DEND_CALLBACK(READLINE(lw450_state, fdc_dend_r))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 4000) // 4.0 kHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

#pragma endregion

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( lw350 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD("uc6273-a-lwb6", 0x00000, 0x80000, CRC(5E85D1EC) SHA1(4ca68186fc70f30ccac95429604c88db4f0c34d2))
//	ROM_LOAD("patched", 0x00000, 0x80000, CRC(5E85D1EC) SHA1(4ca68186fc70f30ccac95429604c88db4f0c34d2))
ROM_END

ROM_START( lw450 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("2bc04", 0x00000, 0x40000, CRC(96C2A6F1) SHA1(eb47e37ea46e3becc1b4453286f120682a0a1ddc))
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(FA8712EB) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe))
ROM_END

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT     COMPANY         FULLNAME          FLAGS
COMP( 1995, lw350,  0,   0,       lw350,  lw350, lw350_state,    0,       "Brother",      "Brother LW-350", MACHINE_NODEVICE_PRINTER )
COMP( 1992, lw450,  0,   0,       lw450,  lw350, lw450_state,    0,       "Brother",      "Brother LW-450", MACHINE_NODEVICE_PRINTER )
