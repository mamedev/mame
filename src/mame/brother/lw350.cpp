// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "debug/debugcpu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "util/utf8.h"

// command line parameters:
// -log -debug -window -intscalex 2 -intscaley 2 lw350 -resolution 960x256 -flop roms\lw350\Brother_LW-200-300_GW-24-45_Ver1.0_SpreadsheetProgramAndDataStorageDisk.img

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
MS-DOS compatible FAT12 disk format

Hidden Keys during "DECKEL OFFEN!" ("Case Open!")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Adjustment Printer Menu

Hidden Keys during "SCHREIBMASCHINE" ("Typewriter")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Self Test Menu
- Ctrl+Shift+Enter: Self Print Menu

Emulation Status:
- Printer not working

see https://github.com/BartmanAbyss/brother-hardware/tree/master/2G%20-%20Brother%20LW-350 for datasheets, photos

***************************************************************************/

// UPD765 interface not working
//#define UPD765

#ifndef UPD765

class lw350_floppy_image_device;

class lw350_floppy_connector : public device_t, public device_slot_interface
{
public:
	lw350_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	lw350_floppy_image_device *get_device();

protected:
	void device_start() override {}
	void device_config_complete() override {}
};

class lw350_floppy_image_device : public device_t, public device_image_interface
{
public:
	lw350_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

public:
	bool is_readable()  const noexcept override { return true; }
	bool is_writeable() const noexcept override { return true; }
	bool is_creatable() const noexcept override { return true; }
	bool is_reset_on_load() const noexcept override { return false; }
	const char* image_type_name() const noexcept override { return "floppydisk"; }
	const char* image_brief_type_name() const noexcept override { return "flop"; }
	const char *file_extensions() const noexcept override { return "img"; }
	const char *image_interface() const noexcept override { return "lw350_floppy_35"; }
	image_init_result call_load() override;
	void call_unload() override;

	bool loaded = false;
	bool dirty = false;
	std::unique_ptr<uint8_t[]> image;
	uint32_t image_length{};

protected:
	void device_start() override {}
};

DEFINE_DEVICE_TYPE(LW350_FLOPPY_CONNECTOR, lw350_floppy_connector, "lw350_floppy_connector", "LW350 Floppy drive connector abstraction")
DEFINE_DEVICE_TYPE(LW350_FLOPPY, lw350_floppy_image_device, "lw350_floppy_35", "LW350 3.5\" floppy drive")

lw350_floppy_connector::lw350_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LW350_FLOPPY_CONNECTOR, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}

lw350_floppy_image_device *lw350_floppy_connector::get_device()
{
	return dynamic_cast<lw350_floppy_image_device *>(get_card_device());
}

lw350_floppy_image_device::lw350_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LW350_FLOPPY, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

image_init_result lw350_floppy_image_device::call_load()
{
	// copy image into our buffer
	auto l = length();
	fread(image, l);
	image_length = l;

	loaded = true;
	return image_init_result::PASS;
}

void lw350_floppy_image_device::call_unload()
{
	if(dirty && !is_readonly()) {
		if(reopen_for_write(filename()) == std::error_condition())
			fwrite(image.get(), image_length);
	}

	loaded = false;
}

// FDC high-level emulation (not timing accurate) based on "Hitachi 8-Bit Microcomputer HD63265 FDC Floppy Disk Controller User's Manual"
// https://archive.org/details/bitsavers_hitachidatDiskControllerUsersManual2edMar89_3858532

class hd63266f_device_t : public device_t {
public:
	hd63266f_device_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_cb() { return irq_cb.bind(); }
	auto dreq_wr_cb() { return dreq_cb.bind(); }
	auto dend_rd_cb() { return dend_cb.bind(); }

	void set_floppy(lw350_floppy_image_device* floppy) { this->floppy = floppy; }

	void reset();
	void write(uint8_t data);
	uint8_t read();
	void execute();
	void abort();

	uint8_t status_r() { return str; }
	void abort_w(uint8_t data) { if(data == 0xff) abort(); else logerror("fdc:abort_w %02x is ignored %s\n", data, machine().describe_context()); }
	void data_w(uint8_t data) { write(data); }
	uint8_t data_r() { return read(); }

protected:
	uint8_t str; // status register
	uint8_t dtr; // data register
	uint8_t cmd; // current command
	uint8_t parameter_cnt;
	uint16_t dma_cnt; // active DMA transfer; number of bytes left
	off_t dma_src; // current offset in floppy image for DMA transfer

	// command parameters
	uint8_t hsl_us; // Head Select-Unit Select
	uint8_t ca; // Cylinder Address (0-255)
	uint8_t ha; // Head Address (0-1)
	uint8_t sa; // Sector Address (1-255)
	uint8_t rl; // Record Length (0-6) (p. 20)
	uint8_t esn; // End Sector Number (1-255; compare: 1-253)
	uint8_t gsl; // Gap Skip Length
	uint8_t mnl; // Meaning Length

	// WRITE FORMAT
	uint8_t scnt; // Sector Count (1-255)
	uint8_t gp3l; // Gap 3 Length (1-255)
	uint8_t dud; // Dummy Data (0-255)

	// SEEK
	uint8_t ncn; // New Cylinder Number (0-255)

	// COMPARE
	uint8_t step; // Step (p. 23)

	// SPECIFY 1
	uint8_t str_hdut; // Stepping Rate-Head Unload Time (p. 23)
	uint8_t hdlt_ndm; // Head Load-Time-Non-DMA Mode (p. 25)

	// SPECIFY 2
	uint8_t lctk; // Low Current Track
	uint8_t pc1_pc0; // Precompensation Delay 1, 0
	uint8_t pcdct; // Precompensation Delay Change Track

	// result parameters
	uint8_t ssb0, ssb1, ssb2, ssb3; // (p. 28ff)
	uint8_t pcn; // physical cylinder number

	devcb_write_line irq_cb, dreq_cb;
	devcb_read_line dend_cb;

	void device_start() override;

	lw350_floppy_image_device *floppy;
};

DEFINE_DEVICE_TYPE(HD63266F, hd63266f_device_t, "hd63266f", "HD63266F")

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

hd63266f_device_t::hd63266f_device_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HD63266F, tag, owner, clock),
	irq_cb(*this),
	dreq_cb(*this),
	dend_cb(*this)
{
}

void hd63266f_device_t::device_start()
{
	irq_cb.resolve();
	dreq_cb.resolve();
	dend_cb.resolve();
	assert(!dreq_cb.isnull() && "only DMA mode supported; supply a DREQ_W callback!");
	assert(!dend_cb.isnull() && "only DMA mode supported; supply a DEND_R callback!");
}

void hd63266f_device_t::reset()
{
	str = FDC_STATUSM_TXR;
	cmd = 0x00;
	parameter_cnt = 0;
	dma_cnt = 0;
	hsl_us = ca = ha = sa = rl = esn = gsl = mnl = 0;
	scnt = gp3l = dud = 0;
	ncn = 0;
	step = 0;
	str_hdut = 0; hdlt_ndm = 0;
	lctk = pc1_pc0 = pcdct = 0;
	ssb0 = ssb1 = ssb2 = ssb3 = 0;
	pcn = 0;
}

void hd63266f_device_t::write(uint8_t data)
{
	bool input_done = false;
	auto oldSTR = str;

	if(dma_cnt) {
		auto length = floppy && floppy->is_open() ? floppy->length() : 0;
		auto buffer = static_cast<uint8_t*>(floppy && floppy->is_open() ? floppy->image.get() : nullptr);

		if(buffer && dma_src < length) {
			switch(cmd & 0x1f) {
			case FDC_COMMAND_WRITE_DATA:
			case FDC_COMMAND_WRITE_DELETED_DATA:
				dma_cnt--;
				if(dend_cb()) {
					logerror("fdc: ~TEND0 asserted; stopping DMA; DMA_cnt=%04x %s\n", dma_cnt, machine().describe_context());
					dma_cnt = 0;
				}
				logerror("fdc: read DMA; DMA_cnt=%04x DMA_src=%08x %s\n", dma_cnt, dma_src, machine().describe_context());
				if(dma_cnt == 0) {
					// indicate result is ready
					str = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
					if(!irq_cb.isnull()) irq_cb(true);
				} else {
					// notify DMA to write the next byte
					dreq_cb(true);
				}
				buffer[dma_src++] = data;
				floppy->dirty = true;
				break;
			default:
				logerror("fdc: DMA active for unknown command %s\n", machine().describe_context());
				break;
			}
		} else {
			logerror("fdc: DMA out of range of floppy image %s\n", machine().describe_context());
			return;
		}
	} else {
		if(!(str & FDC_STATUSM_BSY)) {
			// write command code
			cmd = data;
			str |= FDC_STATUSM_BSY;
			str &= ~FDC_STATUSM_TXR;

			switch(cmd & 0x1f) {
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
				input_done = true;
				break;
			default:
				// ready to receive command parameters
				parameter_cnt = 0;
				str |= FDC_STATUSM_TXR;
				break;
			}
			logerror("fdc:write_cmd(%02x) STR=%02x => %02x %s\n", data, oldSTR, str, machine().describe_context());
		} else {
			// write command parameter
			switch(cmd & 0x1f) {
			case FDC_COMMAND_READ_DATA:
			case FDC_COMMAND_READ_DELETED_DATA:
			case FDC_COMMAND_READ_ERRONEOUS_DATA:
			case FDC_COMMAND_WRITE_DATA:
			case FDC_COMMAND_WRITE_DELETED_DATA:
			case FDC_COMMAND_READ_LONG:
			case FDC_COMMAND_WRITE_LONG:
				switch(parameter_cnt) {
				case 0: hsl_us = data; break;
				case 1: ca = data; break;
				case 2: ha = data; break;
				case 3: sa = data; break;
				case 4: rl = data; break;
				case 5: esn = data; break;
				case 6: gsl = data; break;
				case 7: mnl = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_READ_ID:
			case FDC_COMMAND_RECALIBRATE:
			case FDC_COMMAND_CHECK_DEVICE_STATUS:
				switch(parameter_cnt) {
				case 0: hsl_us = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_WRITE_FORMAT:
				switch(parameter_cnt) {
				case 0: hsl_us = data; break;
				case 1: rl = data; break;
				case 2: scnt = data; break;
				case 3: gp3l = data; break;
				case 4: dud = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_SEEK:
				switch(parameter_cnt) {
				case 0: hsl_us = data; break;
				case 1: ncn = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_COMPARE_EQUAL:
			case FDC_COMMAND_COMPARE_LOW_OR_EQUAL:
			case FDC_COMMAND_COMPARE_HIGH_OR_EQUAL:
				switch(parameter_cnt) {
				case 0: hsl_us = data; break;
				case 1: ca = data; break;
				case 2: ha = data; break;
				case 3: sa = data; break;
				case 4: rl = data; break;
				case 5: esn = data; break;
				case 6: gsl = data; break;
				case 7: step = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
			default: // INVALID
				logerror("%s: no more parameters\n", machine().describe_context());
				break;
			case FDC_COMMAND_SPECIFY_1:
				switch(parameter_cnt) {
				case 0: str_hdut = data; break;
				case 1: hdlt_ndm = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_SPECIFY_2:
				switch(parameter_cnt) {
				case 0: str_hdut = data; break;
				case 1: hdlt_ndm = data; break;
				case 2: lctk = data; break;
				case 3: pc1_pc0 = data; break;
				case 4: pcdct = data; input_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			}

			// ready to receive command parameters
			parameter_cnt++;
			if(!input_done)
				str |= FDC_STATUSM_TXR;

			logerror("fdc:write_param(%02x) STR=%02x => %02x %s\n", data, oldSTR, str, machine().describe_context());
		}
		if(input_done)
			execute();
	}
}

uint8_t hd63266f_device_t::read()
{
	bool output_done = false;

	auto oldSTR = str;

	if(dma_cnt) {
		auto length = floppy && floppy->is_open() ? floppy->length() : 0;
		auto buffer = static_cast<uint8_t*>(floppy && floppy->is_open() ? floppy->image.get() : nullptr);

		if(buffer && dma_src < length) {
			switch(cmd & 0x1f) {
			case FDC_COMMAND_READ_DATA:
			case FDC_COMMAND_READ_DELETED_DATA:
			case FDC_COMMAND_READ_ERRONEOUS_DATA:
				dma_cnt--;
				if(dend_cb()) {
					logerror("fdc: ~TEND0 asserted; stopping DMA; DMA_cnt=%04x %s\n", dma_cnt, machine().describe_context());
					dma_cnt = 0;
				}
				//logerror("fdc: read DMA; DMA_cnt=%04x DMA_src=%08x %s\n", DMA_cnt, DMA_src, machine().describe_context());
				if(dma_cnt == 0) {
					// indicate result is ready
					str = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
					if(!irq_cb.isnull()) irq_cb(true);
				} else {
					// notify DMA that next byte is ready
					dreq_cb(true);
				}
				return buffer[dma_src++];
				break;
			default:
				logerror("fdc: DMA active for unknown command %s\n", machine().describe_context());
				break;
			}
		} else {
			logerror("fdc: DMA out of range of floppy image %s\n", machine().describe_context());
			return 0;
		}
	} else {
		// read result parameters
		if((str & (FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY)) == (FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY)) {
			switch(cmd & 0x1f) {
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
				case 0: dtr = ssb0; break;
				case 1: dtr = ssb1; break;
				case 2: dtr = ssb2; break;
				case 3: dtr = ca; break;
				case 4: dtr = ha; break;
				case 5: dtr = sa; break;
				case 6: dtr = rl; output_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_SEEK:
			case FDC_COMMAND_RECALIBRATE:
			case FDC_COMMAND_SPECIFY_1:
			case FDC_COMMAND_SPECIFY_2:
			case FDC_COMMAND_SLEEP:
			case FDC_COMMAND_ABORT:
				logerror("%s: no more parameters\n", machine().describe_context());
				break;
			case FDC_COMMAND_CHECK_DEVICE_STATUS:
			default: // INVALID
				switch(parameter_cnt) {
				case 0: dtr = ssb3; output_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
				switch(parameter_cnt) {
				case 0: dtr = ssb0; break;
				case 1: dtr = pcn; output_done = true; break;
				default: logerror("%s: no more parameters\n", machine().describe_context());
				}
				break;
			}
			parameter_cnt++;

			str &= ~FDC_STATUSM_TXR;
			str &= ~FDC_STATUSM_DIR;
			if(!irq_cb.isnull()) irq_cb(false);

			if(output_done) {
				// enter command waiting state
				str |= FDC_STATUSM_TXR;
				str &= ~FDC_STATUSM_BSY;
			} else {
				// signal next result parameter is ready
				str |= FDC_STATUSM_TXR;
				str |= FDC_STATUSM_DIR;
			}
		}
	}

	logerror("fdc:read STR=%02x => %02x %s\n", oldSTR, str, machine().describe_context());

	return dtr;
}

void hd63266f_device_t::execute()
{
	ssb0 = ssb1 = ssb2 = ssb3 = 0;

	auto oldSTR = str;

	auto readwrite_params = [this] {
		return string_format("HSL_US=%02x CA=%02x HA=%02x SA=%02x RL=%02x ESN=%02x GSL=%02x MNL=%02x", hsl_us, ca, ha, sa, rl, esn, gsl, mnl);
	};

	auto length = floppy && floppy->is_open() ? floppy->length() : 0;
	auto buffer = floppy && floppy->is_open() ? floppy->image.get() : nullptr;

	// do something
	switch(cmd & 0x1f) {
	case FDC_COMMAND_READ_DATA: {
		// 720kb  floppy: 80 cylinders, 2 heads,  9 each 512b sectors per track
		// 1.44mb floppy: 80 cylinders, 2 heads, 18 each 512b sectors per track
		auto sector_length = 128 << (rl & 0b111);
		auto sectors_per_track = (str_hdut >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		dma_src = (((ca << 1) | (ha & 1)) * sectors_per_track + sa - 1) * sector_length;
		dma_cnt = (esn - sa - 1 + 1) * sector_length;
		str = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
		dreq_cb(true);
		logerror("%s: fdc: execute: READ DATA; %s DMA_cnt=%04x DMC_src=%08x\n", machine().describe_context(), readwrite_params(), dma_cnt, dma_src);
		break;
	}
	case FDC_COMMAND_READ_DELETED_DATA:
		logerror("%s: fdc: execute: READ DELETED DATA; %s\n", machine().describe_context(), readwrite_params());
		break;
	case FDC_COMMAND_READ_ERRONEOUS_DATA:
		logerror("%s: fdc: execute: READ ERRONEOUS DATA; %s\n", machine().describe_context(), readwrite_params());
		break;
	case FDC_COMMAND_READ_ID:
		logerror("%s: fdc: execute: READ ID\n", machine().describe_context());
		break;
	case FDC_COMMAND_WRITE_DATA: {
		auto sector_length = 128 << (rl & 0b111);
		auto sectors_per_track = (str_hdut >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		dma_src = (((ca << 1) | (ha & 1)) * sectors_per_track + sa - 1) * sector_length;
		dma_cnt = (esn - sa - 1 + 1) * sector_length;
		str = FDC_STATUSM_TXR | FDC_STATUSM_BSY;
		dreq_cb(true);
		logerror("%s: fdc: execute: WRITE DATA; %s DMA_cnt=%04x DMC_src=%08x\n", machine().describe_context(), readwrite_params(), dma_cnt, dma_src);
		break;
	}
	case FDC_COMMAND_WRITE_DELETED_DATA:
		logerror("%s: fdc: execute: WRITE DELETED DATA; %s\n", machine().describe_context(), readwrite_params());
		break;
	case FDC_COMMAND_WRITE_FORMAT: {
		auto sector_length = 128 << (rl & 0b111);
		auto sectors_per_track = (str_hdut >> 4) == 0xa ? 18 : 9; // detect 1.44mb/720kb mode; based on LW-350 code
		auto dst = ((pcn << 1) | ((hsl_us & 0b100) >> 2)) * sectors_per_track * sector_length;
		auto cnt = scnt * sector_length >> 1; // not sure why >> 1, but matches how it's called
		logerror("%s: fdc: execute: WRITE FORMAT; HSL_US=%02x PCN=%02x SCNT=%02x RL=%02x DUD=%02x cnt=%04x ofs=%08x\n", machine().describe_context(), hsl_us, pcn, scnt, rl, dud, cnt, dst);
		while(cnt--) {
			if(dst < length)
				buffer[dst++] = dud;
		}
		break;
	}
	case FDC_COMMAND_SEEK:
		logerror("%s: fdc: execute: SEEK NCN=%d\n", machine().describe_context(), ncn);
		ssb0 |= FDC_SSB0M_SED;
		pcn = ncn;
		if(ncn == 0)
			ssb3 |= FDC_SSB3M_TRZ;
		else
			ssb3 &= ~FDC_SSB3M_TRZ;
		break;
	case FDC_COMMAND_RECALIBRATE:
		logerror("%s: fdc: execute: RECALIBRATE\n", machine().describe_context());
		ssb0 |= FDC_SSB0M_SED;
		ssb3 |= FDC_SSB3M_TRZ;
		pcn = 0;
		break;
	case FDC_COMMAND_COMPARE_EQUAL:
		logerror("%s: fdc: execute: COMPARE EQUAL\n", machine().describe_context());
		break;
	case FDC_COMMAND_COMPARE_LOW_OR_EQUAL:
		logerror("%s: fdc: execute: COMPARE LOW OR EQUAL\n", machine().describe_context());
		break;
	case FDC_COMMAND_COMPARE_HIGH_OR_EQUAL:
		logerror("%s: fdc: execute: COMPARE HIGH OR EQUAL\n", machine().describe_context());
		break;
	case FDC_COMMAND_CHECK_DEVICE_STATUS:
		logerror("%s: fdc: execute: CHECK DEVICE STATUS\n", machine().describe_context());
		break;
	case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
		logerror("%s: fdc: execute: CHECK INTERRUPT STATUS\n", machine().describe_context());
		ssb0 &= ~FDC_SSB0M_HSL;
		break;
	case FDC_COMMAND_SPECIFY_1:
		logerror("%s: fdc: execute: SPECIFY 1\n", machine().describe_context());
		break;
	case FDC_COMMAND_SPECIFY_2:
		logerror("%s: fdc: execute: SPECIFY 2\n", machine().describe_context());
		break;
	case FDC_COMMAND_SLEEP:
		logerror("%s: fdc: execute: SLEEP\n", machine().describe_context());
		break;
	case FDC_COMMAND_ABORT:
		logerror("%s: fdc: execute: ABORT\n", machine().describe_context());
		break;
	case FDC_COMMAND_READ_LONG:
		logerror("%s: fdc: execute: READ LONG\n", machine().describe_context());
		break;
	case FDC_COMMAND_WRITE_LONG:
		logerror("%s: fdc: execute: WRITE LONG\n", machine().describe_context());
		break;
	}

	ssb0 = FDC_SSB0M_INC_NOR;
	ssb0 |= hsl_us & 0b11; // mirror US1, US0 from command parameters
	ssb3 |= hsl_us & 0b111; // mirror HSL, US1, US0 from command parameters
	if(floppy && floppy->loaded)
		ssb3 |= FDC_SSB3M_RDY;
	else {
		ssb0 |= FDC_SSB0M_DNR;
		ssb3 &= ~FDC_SSB3M_RDY;
	}

	parameter_cnt = 0;

	if(!dma_cnt) {
		switch(cmd & 0x1f) {
		case FDC_COMMAND_SEEK:
		case FDC_COMMAND_RECALIBRATE:
		case FDC_COMMAND_SPECIFY_1:
		case FDC_COMMAND_SPECIFY_2:
		case FDC_COMMAND_SLEEP:
		case FDC_COMMAND_ABORT:
			// no result
			str = FDC_STATUSM_TXR;
			break;
			//[[fallthrough]]
			// no IRQ
		case FDC_COMMAND_CHECK_DEVICE_STATUS:
		case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
		default: // INVALID
			// indicate result is ready
			str = FDC_STATUSM_TXR | FDC_STATUSM_DIR | FDC_STATUSM_BSY;
			break;
		}

		// set IRQ to indicate result is ready
		switch(cmd & 0x1f) {
		case FDC_COMMAND_CHECK_DEVICE_STATUS:
		case FDC_COMMAND_CHECK_INTERRUPT_STATUS:
			break;
		default:
			if(!irq_cb.isnull()) irq_cb(true); // (p.34)
		   // TODO: not for INVALID
			break;
		}
	}

	logerror("fdc:execute STR=%02x => %02x %s\n", oldSTR, str, machine().describe_context());
}

void hd63266f_device_t::abort()
{
	auto oldSTR = str;
	reset();

	logerror("fdc:abort STR=%02x => %02x %s\n", oldSTR, str, machine().describe_context());
}

#else

#include "machine/upd765.h"


class hd63266f_device_t : public upd765_family_device {
public:
	hd63266f_device_t(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
	void map(address_map& map) override {
		map(0x0, 0x0).r(FUNC(upd765a_device::msr_r));
		map(0x1, 0x1).rw(FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	}

	auto irq_wr_cb() { return intrq_wr_callback(); }
	auto dreq_wr_cb() { return drq_wr_callback(); }
	//auto dend_rd_cb() { return dend_cb.bind(); }
};

DEFINE_DEVICE_TYPE(HD63266F, hd63266f_device_t, "hd63266f", "Hitachi HD63266F FDC")

hd63266f_device_t::hd63266f_device_t(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
: upd765_family_device(mconfig, HD63266F, tag, owner, clock) {
	set_ready_line_connected(false);
	set_select_lines_connected(false);
	has_dor = false;
}

#include "imagedev/floppy.h"

using lw350_floppy_connector = floppy_connector;
#define LW350_FLOPPY_CONNECTOR FLOPPY_CONNECTOR
#define LW350_FLOPPY FLOPPY_35_HD

#endif

class lw350_state : public driver_device
{
public:
	lw350_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		floppy(*this, "floppy"),
		fdc(*this, "fdc"),
		beeper(*this, "beeper"),
		io_kbrow(*this, "kbrow.%u", 0),
		rombank(*this, "rom")
	{ }

	void lw350(machine_config& config);

protected:
	// driver_device overrides
	void machine_start() override;
	void machine_reset() override;
	void video_start() override;

private:
	// devices
	required_device<hd64180rp_device> maincpu;
	required_device<screen_device> screen;
	required_device<lw350_floppy_connector> floppy;
	required_device<hd63266f_device_t> fdc;
	required_device<beep_device> beeper;
	required_ioport_array<9> io_kbrow;
	required_memory_bank rombank;

	uint8_t vram[80 * 128];
	uint8_t io_70, io_7a, io_b8, io_90;

	// screen updates
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	uint8_t illegal_r(offs_t offset, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory read from %0*X & %0*X\n", machine().describe_context(), 6, offset, 2, mem_mask);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory write to %0*X = %0*X & %0*X\n", machine().describe_context(), 6, offset, 2, data, 2, mem_mask);
	}

	// IO
	void io_70_w(uint8_t data) {
		io_70 = data;
	}
	uint8_t io_74_r() {
		// 0x00: 7 lines display (64 pixels height)
		// 0x80: 14 lines display (128 pixels height)
		return 0x80;
	}
	uint8_t io_a8_r() {
		// bit 0: case open
		// bit 2: carriage return indicator
		//return 0x01; // case open
		return 0x00;
	}
	uint8_t io_b8_r() {
		// keyboard matrix
 		if(io_b8 <= 8)
			return io_kbrow[io_b8]->read();

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
	void io_b8_w(uint8_t data) {
		io_b8 = data;
	}
	void rombank_w(uint8_t data) { // E0
		rombank->set_entry(data & 0x03);
	}
	void beeper_w(uint8_t data) { // F0
		beeper->set_state(data == 0);
	}
	void irqack_w(uint8_t) { // F8
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	// Floppy
	#ifndef UPD765
	void fdc_dtr_w(uint8_t data) { // 79
		fdc->set_floppy(floppy->get_device());
		fdc->write(data);
	}
	#endif
	uint8_t io_7a_r() { // 7a
		return io_7a;
	}
	uint8_t io_7e_r() {
		return 0x80; // 1.44mb floppy
	}
	void io_7e_w(uint8_t) { // 7e
	}
	uint8_t io_90_r() { // 90
		return io_90;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(io_90_timer_callback);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w) { io_7a &= ~0x40; if(state) io_7a |= 0x40; }
	DECLARE_WRITE_LINE_MEMBER(fdc_dreq_w) { maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE); }
	DECLARE_READ_LINE_MEMBER(fdc_dend_r) { return maincpu->get_tend0(); }

	// VRAM
	uint8_t vram_r(offs_t offset, uint8_t mem_mask = ~0) {
		return vram[offset] & mem_mask;
	}
	void vram_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) {
		vram[offset] = (vram[offset] & ~mem_mask) | data;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

	void map_program(address_map& map) {
		map(0x00000, 0x01fff).rom();
		map(0x02000, 0x05fff).ram();
		map(0x06000, 0x3ffff).rom();
		map(0x40000, 0x5ffff).bankr("rom");
		map(0x60000, 0x617ff).ram();
		map(0x61800, 0x63fff).rw(FUNC(lw350_state::vram_r), FUNC(lw350_state::vram_w));
		map(0x64000, 0x71fff).ram();
		map(0x72000, 0x75fff).rom().region("maincpu", 0x2000); // => ROM 0x02000-0x05fff
		map(0x76000, 0x7ffff).ram();
	}

	void map_io(address_map& map) {
		map.global_mask(0xff);
		map(0x00, 0x3f).noprw(); // Z180 internal registers
		map(0x70, 0x70).w(FUNC(lw350_state::io_70_w));
		map(0x74, 0x74).r(FUNC(lw350_state::io_74_r));

		// floppy
		#ifndef UPD765
			map(0x78, 0x78).rw("fdc", FUNC(hd63266f_device_t::status_r), FUNC(hd63266f_device_t::abort_w));
			map(0x79, 0x79).r("fdc", FUNC(hd63266f_device_t::data_r)).w(FUNC(lw350_state::fdc_dtr_w));
		#else
			map(0x78, 0x79).m(fdc, FUNC(hd63266f_device_t::map));
		#endif
		map(0x7a, 0x7a).r(FUNC(lw350_state::io_7a_r));
		map(0x7e, 0x7e).rw(FUNC(lw350_state::io_7e_r), FUNC(lw350_state::io_7e_w));
		map(0x90, 0x90).r(FUNC(lw350_state::io_90_r));

		// printer
		map(0xa8, 0xa8).r(FUNC(lw350_state::io_a8_r));

		map(0xb8, 0xb8).rw(FUNC(lw350_state::io_b8_r), FUNC(lw350_state::io_b8_w));
		map(0xe0, 0xe0).w(FUNC(lw350_state::rombank_w));
		map(0xf0, 0xf0).w(FUNC(lw350_state::beeper_w));
		map(0xf8, 0xf8).w(FUNC(lw350_state::irqack_w));

		//map(0x40, 0xff).rw(FUNC(lw350_state::illegal_io_r), FUNC(lw350_state::illegal_io_w));
	}
};

void lw350_state::video_start()
{
}

uint32_t lw350_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	// video on?
	if(!BIT(io_70, 0))
		return 0;

	// backlight on?
	//if(BIT(io_70, 7))
		//...

	const rgb_t palette[]{
		0xffffffff,
		0xff000000,
	};

	for(auto y = 0; y < 128; y++) {
		uint32_t* p = &bitmap.pix(y);
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

static INPUT_PORTS_START(lw350)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR(U'§')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)      PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)      PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)      PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)      PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)    PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)      PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)      PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)      PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)      PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)      PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)      PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G.S.END")               PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(U'ß') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Inhalt")                PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
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
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'´')  PORT_CHAR(U'`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\'')  PORT_CHAR(U'°')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä')  PORT_CHAR(U'Ä')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(lw350_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(lw350_state::io_90_timer_callback)
{
	#ifndef UPD765
	if(floppy && floppy->get_device() && floppy->get_device()->loaded)
	#endif
		io_90 ^= 1 << 6; // int1_process_floppy wants this to toggle, 150 ms, HACK
}

//uint8_t char_attribute = 0x00;

void lw350_state::machine_start()
{
	auto rom = memregion("maincpu")->base();
	rombank->configure_entries(0, 4, rom, 0x20000);
	screen->set_visible_area(0, 480 - 1, 0, 128 - 1);

	fdc->set_floppy(floppy->get_device());

	// ROM patches

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

#ifdef UPD765
	static void lw350_floppies(device_slot_interface& device) {
		device.option_add("35dd", FLOPPY_35_DD);
		device.option_add("35hd", FLOPPY_35_HD);
	}
#endif

void lw350_state::lw350(machine_config& config) {
	// basic machine hardware
	HD64180RP(config, maincpu, 16'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw350_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw350_state::map_io);
	TIMER(config, "1khz").configure_periodic(FUNC(lw350_state::int1_timer_callback), attotime::from_hz(1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t(6, 245, 206));
	screen->set_physical_aspect(480, 128);
	screen->set_refresh_hz(78.1);
	screen->set_screen_update(FUNC(lw350_state::screen_update));
	screen->set_size(480, 128);

	// floppy disk
	TIMER(config, "io_90").configure_periodic(FUNC(lw350_state::io_90_timer_callback), attotime::from_msec(160));
	#ifndef UPD765
		LW350_FLOPPY_CONNECTOR(config, floppy, 0);
		floppy->option_add("35hd", LW350_FLOPPY);
		floppy->set_default_option("35hd");
	#else
		FLOPPY_CONNECTOR(config, floppy, lw350_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
	#endif

	HD63266F(config, fdc, XTAL(16'000'000));
	fdc->irq_wr_cb().set(FUNC(lw350_state::fdc_irq_w));
	fdc->dreq_wr_cb().set(FUNC(lw350_state::fdc_dreq_w));
	#ifndef UPD765
		fdc->dend_rd_cb().set(FUNC(lw350_state::fdc_dend_r));
	#endif

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

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

see https://github.com/BartmanAbyss/brother-hardware/tree/master/1G%20-%20Brother%20LW-450 for datasheets, photos

Emulation Status:
Printer not working
Floppy Read has some problems (directory working, but LW-450 reports illegal file format when trying to load a .wpt file (content verified with LW-350), but writing seems fine)
Dictionary ROM probably not correctly mapped

***************************************************************************/

constexpr int MDA_CLOCK = 16'257'000;

class lw450_state : public driver_device
{
public:
	lw450_state(const machine_config& mconfig, device_type type, const char* tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		palette(*this, "palette"),
		floppy(*this, "floppy"),
		fdc(*this, "fdc"),
		m_crtc(*this, "hd6445"),
		vram(*this, "vram"),
		speaker(*this, "beeper"),
		io_kbrow(*this, "kbrow.%u", 0),
		rom(*this, "maincpu"),
		rombank(*this, "rom")
	{ }

	void lw450(machine_config& config);

protected:
	// driver_device overrides
	void machine_start() override;
	void machine_reset() override;
	void video_start() override;

private:
	// devices
	required_device<z80180_device> maincpu; // use z80180 instead of hd64180rp, because hd64180rf doesn't have hd64180rp's 19-bit address width
	required_device<screen_device> screen;
	required_device<palette_device> palette;
	required_device<lw350_floppy_connector> floppy;
	required_device<hd63266f_device_t> fdc;
	required_device<hd6345_device> m_crtc;
	required_shared_ptr<uint8_t> vram;
	required_device<beep_device> speaker;
	required_ioport_array<9> io_kbrow;
	required_region_ptr<uint8_t> rom;
	required_memory_bank rombank;

	uint8_t io_72, io_73, io_74, io_75; // gfx
	uint8_t io_7a, io_b8;
	uint32_t framecnt;

	uint8_t illegal_r(offs_t offset) {
		if(!machine().side_effects_disabled())
			logerror("%s: unmapped memory read from %0*X\n", machine().describe_context(), 6, offset);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data) {
		if(!machine().side_effects_disabled())
			logerror("%s: unmapped memory write to %0*X = %0*X\n", machine().describe_context(), 6, offset, 2, data);
	}

	uint8_t rom72000_r(offs_t offset) {
		return rom[0x02000 + offset];
	}

	// IO
	uint8_t io_b0_r() { return ~0x00; }
	uint8_t io_b8_r() {
		// keyboard matrix
		if(io_b8 <= 8)
			return io_kbrow[io_b8]->read();
		return 0x00;
	}
	void io_b8_w(uint8_t data) {
		io_b8 = data;
	}
	void rombank_w(uint8_t data) { // E0
		if(data >= 4 && data < 8)
			rombank->set_entry(data - 4);
	}
	void beeper_w(uint8_t data) { // F0
		speaker->set_state(data == 0);
	}
	void irqack_w(uint8_t) { // F8
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	// Floppy
	#ifndef UPD765
	void fdc_dtr_w(uint8_t data) { // 79
		fdc->set_floppy(floppy->get_device());
		fdc->write(data);
	}
	#endif
	uint8_t io_7a_r() { // 7a
		return io_7a;
	}
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w) { io_7a &= ~0x40; if(state) io_7a |= 0x40; }
	DECLARE_WRITE_LINE_MEMBER(fdc_dreq_w) { maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE); }
	DECLARE_READ_LINE_MEMBER(fdc_dend_r) { return maincpu->get_tend0(); }

	// CRTC
	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
	void io_72_w(uint8_t data) { io_72 = data; }
	void io_73_w(uint8_t data) { io_73 = data; }
	void io_74_w(uint8_t data);
	void io_75_w(uint8_t data) { io_75 = data; }

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

	void map_program(address_map& map);
	void map_io(address_map& map);
};

void lw450_state::video_start()
{
}

void lw450_state::map_program(address_map& map)
{
	map(0x00000, 0x01fff).rom();
	map(0x02000, 0x05fff).ram();
	map(0x06000, 0x3ffff).rom();
	map(0x40000, 0x5ffff).bankr("dictionary");
	map(0x62000, 0x71fff).ram(); // D-RAM UPPER/LOWER
	map(0x72000, 0x75fff).r(FUNC(lw450_state::rom72000_r)); // => ROM 0x02000-0x05fff
	map(0x78000, 0x7ffff).ram(); // PS-RAM
	map(0xf8000, 0xfffff).ram().share("vram"); // VRAM
	// text vram @ F8000-F8C80 (2*80 bytes/line)
	// font @ FC000-FD000 pitch 16
}

void lw450_state::map_io(address_map& map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // Z180 internal registers

	map(0x70, 0x70).w("hd6445", FUNC(hd6345_device::address_w));
	map(0x71, 0x71).w("hd6445", FUNC(hd6345_device::register_w));
	map(0x72, 0x72).w(FUNC(lw450_state::io_72_w));
	map(0x73, 0x73).w(FUNC(lw450_state::io_73_w));
	map(0x74, 0x74).w(FUNC(lw450_state::io_74_w));
	map(0x75, 0x75).w(FUNC(lw450_state::io_75_w));

	// floppy
	#ifndef UPD765
		map(0x78, 0x78).rw("fdc", FUNC(hd63266f_device_t::status_r), FUNC(hd63266f_device_t::abort_w));
		map(0x79, 0x79).r("fdc", FUNC(hd63266f_device_t::data_r)).w(FUNC(lw450_state::fdc_dtr_w));
	#else
		map(0x78, 0x79).m(fdc, FUNC(hd63266f_device_t::map));
	#endif
	map(0x7a, 0x7a).r(FUNC(lw450_state::io_7a_r));

	map(0xb0, 0xb0).r(FUNC(lw450_state::io_b0_r));
	map(0xb8, 0xb8).rw(FUNC(lw450_state::io_b8_r), FUNC(lw450_state::io_b8_w));
	map(0xe0, 0xe0).w(FUNC(lw450_state::rombank_w));
	map(0xf0, 0xf0).w(FUNC(lw450_state::beeper_w));
	map(0xf8, 0xf8).w(FUNC(lw450_state::irqack_w));

	//map(0x40, 0xff) AM_READWRITE(illegal_io_r, illegal_io_w)
}

// CRTC
//////////////////////////////////////////////////////////////////////////

TIMER_DEVICE_CALLBACK_MEMBER(lw450_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

WRITE_LINE_MEMBER(lw450_state::crtc_vsync) 
{ 
	if(state) {
		framecnt++;
	}
}

void lw450_state::io_74_w(uint8_t data)
{
	io_74 = data;
	if(io_74 & 0x04) {
		// graphics mode
		m_crtc->set_char_width(8);
		m_crtc->set_clock(MDA_CLOCK / 8);
	} else {
		// text mode
		m_crtc->set_char_width(9);
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
	uint32_t* p = &bitmap.pix(y);
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
}

// PIN 21 (Character Clock) of CRTC-II: menu: 2.0 MHz; schreibmaschine: 1.8 MHz

// these timings are all at MDA clock
// bootup:          [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps
// menu:            [:hd6445] M6845 config screen: HTOTAL: 990  VTOTAL: 370  MAX_X: 809  MAX_Y: 319  HSYNC: 819-953  VSYNC: 334-349  Freq: 44.381646fps
// schreibmaschine: [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps

void lw450_state::machine_start()
{
	rombank->configure_entries(0, 4, memregion("dictionary")->base(), 0x20000);

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

static GFXDECODE_START( gfx_lw450 )
	GFXDECODE_RAM("vram", 0x4000, pc_16_charlayout, 0, 1)
GFXDECODE_END

void lw450_state::lw450(machine_config& config) {
	// basic machine hardware
	Z80180(config, maincpu, 12'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw450_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw450_state::map_io);
	TIMER(config, "1khz").configure_periodic(FUNC(lw450_state::int1_timer_callback), attotime::from_hz(1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t::white());
	screen->set_physical_aspect(720, 320);
	screen->set_screen_update("hd6445", FUNC(hd6345_device::screen_update));
	screen->set_raw(MDA_CLOCK, 882, 0, 729, 370, 0, 320); // based on bootup crtc values

	GFXDECODE(config, "gfxdecode", palette, gfx_lw450);
	PALETTE(config, palette).set_entries(4);

	// CRTC
	HD6345(config, m_crtc, MDA_CLOCK / 9);
	m_crtc->set_screen(screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(lw450_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(lw450_state::crtc_vsync));

	LW350_FLOPPY_CONNECTOR(config, floppy, 0);
	floppy->option_add("35dd", LW350_FLOPPY);
	floppy->set_default_option("35dd");

	HD63266F(config, fdc, XTAL(16'000'000));
	fdc->irq_wr_cb().set(FUNC(lw450_state::fdc_irq_w));
	fdc->dreq_wr_cb().set(FUNC(lw450_state::fdc_dreq_w));
	#ifndef UPD765
	fdc->dend_rd_cb().set(FUNC(lw450_state::fdc_dend_r));
	#endif

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

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

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT              COMPANY         FULLNAME          FLAGS
COMP( 1995, lw350,  0,   0,       lw350,  lw350, lw350_state,    empty_init,       "Brother",      "Brother LW-350", MACHINE_NODEVICE_PRINTER )
COMP( 1992, lw450,  0,   0,       lw450,  lw350, lw450_state,    empty_init,       "Brother",      "Brother LW-450", MACHINE_NODEVICE_PRINTER )
