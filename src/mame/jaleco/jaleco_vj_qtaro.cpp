// license:BSD-3-Clause
// copyright-holders:windyfairy

/*
King Qtaro PCI card and Qtaro subboards for VJ

Main board ("King Qtaro"):
No markings
----------------------------------------------
|                                            |
|                                            |
|                                            |
|                             FLEX           |
|       CN3                              CN2 |
|                                            |
|                                            |
|                   LS6201                   |
|                                            |
|    |-------|           |-|    |------------|
------       ------------- ------

LS6201 - LSI LS6201 027 9850KX001 PCI Local Bus Interface
FLEX - Altera Flex EPF10K10QC208-4 DAB239813
CN2, CN3 - 68-pin connectors

Information about the LS6201:
https://web.archive.org/web/20070912033617/http://www.lsisys.co.jp/prod/ls6201/ls6201.htm
https://web.archive.org/web/20001015203836/http://www.lsisys.co.jp:80/prod/LS6201.pdf

The King Qtaro board appears to be a custom spec ordered from LSI Systems and shares some
layout similarities to the LS6201 evaluation card offered by LSI Systems.




JALECO VJ-98347
MADE IN JAPAN
EB-00-20125-0
(Front)
-----------------------------------
|    CN2        CN4        CN6    |
|                                 |
|                                 |
|                                 |
|                                 |
|                                 |
|    CN1        CN3        CN5    |
-----------------------------------

CN1/2/3/4/5/6 - 80-pin connectors

(Back)
-----------------------------------
|               CN9               |
|                                 |
|               U4                |
|  CN7          U1           CN8  |
|               U2                |
|               U3                |
|                                 |
-----------------------------------

U1 - ?
U2, U3 - (Unpopulated)
U4 - HM87AV LM3940IS
CN7, CN8 - 68-pin connectors. Connects to CN3, CN2 on main board




JALECO VJ-98341 ("Qtaro")
MADE IN JAPAN
EB-00-20124-0
----------------------------------------
|                                      |
|   |----------|                       |
|   |          |   D4516161            |
|   |   FLEX   |             U2  CN3   |
|   |          |                   CN4 |
|   |----------|             U1        |
|                                      |
|                                      |
----------------------------------------
FLEX     - Altera FLEX EPF10K30AQC240-3 DBA439849
D4516161 - NEC uPD4516161AG5-A80 512K x 16-bit x 2-banks (16MBit) SDRAM (SSOP50)
U1, U2   - LVX244

CN3 - 40 pin connector (connects to VJ-98342)
CN4 - 40 pin connector (connects to VJ-98342)


Hardware testing confirms that the Qtaro board is responsible for mixing the sprites from the subboard
with the movies from the PC side.
On real hardware, when the CN3 ribbon cables for two monitors going into the subboard are swapped
but CN4 is left in its proper ordering, the sprites will appear based on the placement of the ribbon
cable on the subboard. The movies are still in the correct ordering.


TODO: Timing of when the videos start and stop is not accurate
VJ needs all of the videos to start and end at the same time, and if one video finishes before the others then
it will try to stop *all* of the video data streams at the same time on the PC side.
Setting the buffer size <= 0x8000 fixes it to some degree because the DMAs can't run ahead of each other too far,
but there's an issue with Stepping Stage where it'll just not send data for a small period causing videos to pause
and then go out of sync (???).

The MPEG decoder is on the FPGA on each of the Qtaro boards. The implementation here doesn't try to handle
any potential quirks with the decoding. I think a more accurate to the FPGA version implementation would
be required to make things work exactly like the real hardware, because the FPGA version seems to be able
to start the video stream sooner, and it does not seem to reset any kind of state between videos.
To the last point, I have multiple video recordings of Stepping Stage showing garbage from the previous
video at the very start of a new video for a few frames. DMA timings are almost surely incorrect.

TODO: The last decoded video frame can show up at inappropriate times
Color bar check background in VJ for example enables the video stream so will show the last decoded frame sometimes.
*/

#include "emu.h"
#include "jaleco_vj_qtaro.h"

#include <algorithm>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg/pl_mpeg.h"

#define LOG_VIDEO              (1U << 1)
#define LOG_DMA                (1U << 2)
#define LOG_VERBOSE_VIDEO      (1U << 3)
#define LOG_VERBOSE_DMA        (1U << 4)
#define LOG_VERBOSE_VIDEO_DATA (1U << 5)

// #define VERBOSE (LOG_VIDEO | LOG_DMA | LOG_VERBOSE_VIDEO | LOG_VERBOSE_DMA)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"


DEFINE_DEVICE_TYPE(JALECO_VJ_QTARO,      jaleco_vj_qtaro_device,      "jaleco_vj_qtaro",      "Jaleco VJ Qtaro Subboard")
DEFINE_DEVICE_TYPE(JALECO_VJ_KING_QTARO, jaleco_vj_king_qtaro_device, "jaleco_vj_king_qtaro", "Jaleco VJ King Qtaro PCI Device")


static constexpr unsigned DMA_BURST_SIZE = 128U;
#define DMA_TIMER_PERIOD attotime::from_hz(33'000'000 / 64)


jaleco_vj_qtaro_device::jaleco_vj_qtaro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JALECO_VJ_QTARO, tag, owner, clock),
	m_buffer_size(0x100000)
{
}

void jaleco_vj_qtaro_device::device_start()
{
	m_plm_video = nullptr;
	m_plm_buffer = nullptr;

	save_item(NAME(m_int));
	save_item(NAME(m_mix_level));
}

void jaleco_vj_qtaro_device::device_reset()
{
	m_int = 0;
	m_mix_level = 0;

	reset_video();
}

void jaleco_vj_qtaro_device::reset_video()
{
	m_video_frame = bitmap_rgb32(352, 240);

	if (m_plm_video != nullptr)
		plm_video_destroy(m_plm_video);

	if (m_plm_buffer != nullptr)
		plm_buffer_destroy(m_plm_buffer);

	m_plm_buffer = plm_buffer_create_with_capacity(m_buffer_size);
	m_plm_video = plm_video_create_with_buffer(m_plm_buffer, false);
}

void jaleco_vj_qtaro_device::video_mix_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data != m_mix_level) {
		LOGMASKED(LOG_VIDEO, "[%s] video_mix_w %04x\n", tag(), data);
	}

	m_mix_level = data;
}

uint8_t jaleco_vj_qtaro_device::reg_r(offs_t offset)
{
	return m_int;
}

void jaleco_vj_qtaro_device::reg_w(offs_t offset, uint8_t data)
{
	// Bit 7 is set when starting DMA for an entirely new video, then unset when video is ended
	if (BIT(data, 7) && !BIT(m_int, 7))  {
		LOGMASKED(LOG_VIDEO, "[%s] DMA transfer thread started %02x\n", tag(), data);
		reset_video();
	} else if (!BIT(data, 7) && BIT(m_int, 7))  {
		LOGMASKED(LOG_VIDEO, "[%s] DMA transfer thread ended %02x\n", tag(), data);
	}

	m_int = data;
}

uint8_t jaleco_vj_qtaro_device::reg2_r(offs_t offset)
{
	// WriteStream cleanup function will loop until this returns 0.
	// Probably relates to DMA or video playback state.
	return 0;
}

uint32_t jaleco_vj_qtaro_device::reg3_r(offs_t offset)
{
	// 0x20 is some kind of default state. Relates to DMA or video playback.
	// If this value is 0x40 then the code sets it back to 0x20 during the WriteStream cleanup function.
	return 0x20;
}

void jaleco_vj_qtaro_device::reg3_w(offs_t offset, uint32_t data)
{
}

void jaleco_vj_qtaro_device::write(uint8_t *data, uint32_t len)
{
	LOGMASKED(LOG_VERBOSE_VIDEO_DATA, "[%s] video data write %02x\n", tag(), data);
	plm_buffer_write(m_plm_buffer, data, len);
}

void jaleco_vj_qtaro_device::update_frame()
{
	plm_frame_t *frame = plm_video_decode(m_plm_video);

	if (frame == nullptr)
		return;

	LOGMASKED(LOG_VIDEO,
		"[%s] video frame generated %lf %d %08lx %08lx %08lx\n",
		tag(),
		plm_video_get_time(m_plm_video),
		frame != nullptr,
		plm_buffer_get_size(m_plm_buffer),
		plm_buffer_get_remaining(m_plm_buffer),
		get_remaining_memory_size()
	);

	if (m_video_frame.width() != frame->width || m_video_frame.height() != frame->height) {
		m_video_frame = bitmap_rgb32(frame->width, frame->height);
	}

	plm_frame_to_bgra(frame, reinterpret_cast<uint8_t *>(m_video_frame.raw_pixptr(0)), m_video_frame.rowbytes());
}

void jaleco_vj_qtaro_device::render_video_frame(bitmap_rgb32& base)
{
	// The mix level has been confirmed to only apply to the sprite layer.
	// Due to a presumed bug in the game's code, Stepping Stage will animate the
	// mix level from 0 to 15 whenever a video stops playing. This causes the credits
	// text to immediately disappear and then slowly fade back in at the end of songs
	// (visible in every gameplay video on Youtube).
	// Also received videos from a cab owner that shows the same fade issue on the attract
	// screen at certain times, where it'll clear out all sprites on the screen and then
	// slowly fade them back in right before ending the video it was playing.
	// VJ will set the mix level to 0 when no sprites are on screen, 8 when overlays appear
	// while playing a song, and 15 when showing UI elements during menus. And it will also do
	// the same 0-15 fade in at the end of a song.

	assert(base.width() >= m_video_frame.width());
	assert(base.height() >= m_video_frame.height());

	if (m_mix_level == 0) {
		// Copy full movie frame
		copybitmap(
			base,
			m_video_frame,
			0, 0, 0, 0,
			m_video_frame.cliprect()
		);
		return;
	}

	for (int y = 0; y < m_video_frame.height(); y++) {
		for (int x = 0; x < m_video_frame.width(); x++) {
			const uint32_t v = m_video_frame.pix(y, x);
			uint32_t *p = &base.pix(y, x);
			double a = (BIT(*p, 24, 8) / 255.0) * (m_mix_level / 15.0);
			const double movie_blend = 1.0 - a;
			uint8_t r = uint8_t(std::min(255.0, BIT(v, 16, 8) * movie_blend + BIT(*p, 16, 8) * a));
			uint8_t g = uint8_t(std::min(255.0, BIT(v, 8, 8) * movie_blend + BIT(*p, 8, 8) * a));
			uint8_t b = uint8_t(std::min(255.0, BIT(v, 0, 8) * movie_blend + BIT(*p, 0, 8) * a));
			*p = 0xff000000 | (r << 16) | (g << 8) | b;
		}
	}
}

uint32_t jaleco_vj_qtaro_device::get_remaining_memory_size()
{
	// Helper function for burst DMAs
	auto rem = plm_buffer_get_remaining(m_plm_buffer);
	return rem < m_buffer_size ? m_buffer_size - rem : 0;
}

/////////////////////////////////////////////

void jaleco_vj_king_qtaro_device::video_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Speed controlled by IRQ 4 on subboard CPU
	LOGMASKED(LOG_VIDEO, "video_control_w %04x\n", data);

	for (int i = 0; i < 3; i++) {
		// Bits 0, 2, 4 change when a video frame should or shouldn't be decoded
		// Bits 1, 3, 5 are always set?
		bool video_decode_frame = BIT(data, i * 2, 1) == 0;

		if (video_decode_frame)
			m_qtaro[i]->update_frame();
	}
}

uint32_t jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_status_r(offs_t offset)
{
	// Tested when uploading Qtaro firmware
	// 0x100 is set when busy and will keep looping until it's not 0x100
	return 0;
}

void jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_status_w(offs_t offset, uint32_t data)
{
	// Set to 0x80000020 when uploading Qtaro firmware
}

uint32_t jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_r(offs_t offset)
{
	// Should only return 1 when the Qtaro subboard firmware is finished writing.
	// Returning 1 on the first byte will cause it to stop uploading the firmware,
	// then it'll write 3 0xffs and on the last 0xff if it sees 1 then it thinks it finished
	// uploading the firmware successfully.
	return 1;
}

void jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_w(offs_t offset, uint32_t data)
{
}

uint32_t jaleco_vj_king_qtaro_device::fpga_firmware_status_r(offs_t offset)
{
	// Tested when uploading King Qtaro firmware
	// 0x100 is set when busy and will keep looping until it's not 0x100
	return 0;
}

void jaleco_vj_king_qtaro_device::fpga_firmware_status_w(offs_t offset, uint32_t data)
{
	// Set to 0x80000020 when uploading King Qtaro firmware
}

uint32_t jaleco_vj_king_qtaro_device::fpga_firmware_r(offs_t offset)
{
	// Should only return 1 when the King Qtaro firmware is finished writing.
	// Returning 1 on the first byte will cause it to stop uploading the firmware,
	// then it'll write 3 0xffs and on the last 0xff if it sees 1 then it thinks it finished
	// uploading the firmware successfully.
	return 1;
}

void jaleco_vj_king_qtaro_device::fpga_firmware_w(offs_t offset, uint32_t data)
{
}

uint8_t jaleco_vj_king_qtaro_device::event_io_mask_r(offs_t offset)
{
	return m_event_io_mask[offset];
}

void jaleco_vj_king_qtaro_device::event_io_mask_w(offs_t offset, uint8_t data)
{
	m_event_io_mask[offset] = data;
}

uint8_t jaleco_vj_king_qtaro_device::event_unk_r(offs_t offset)
{
	return m_event_unk[offset];
}

void jaleco_vj_king_qtaro_device::event_unk_w(offs_t offset, uint8_t data)
{
	m_event_unk[offset] = data;
}

uint8_t jaleco_vj_king_qtaro_device::event_io_r(offs_t offset)
{
	uint8_t r = m_event_io[offset];
	if (offset == 0)
		r |= 0b111; // Some kind of status flag for each Qtaro board? Must be 1 after writing FPGA firmware
	return r;
}

void jaleco_vj_king_qtaro_device::event_io_w(offs_t offset, uint8_t data)
{
	m_event_io[offset] = data;
}

uint32_t jaleco_vj_king_qtaro_device::event_r(offs_t offset)
{
	// 0x200 = Read event, based on debug strings (What was read? DMA data?)
	return m_event;
}

void jaleco_vj_king_qtaro_device::event_w(offs_t offset, uint32_t data)
{
	m_event &= ~data;
}

uint32_t jaleco_vj_king_qtaro_device::event_mask_r(offs_t offset)
{
	return m_event_mask;
}

void jaleco_vj_king_qtaro_device::event_mask_w(offs_t offset, uint32_t data)
{
	m_event_mask = data;
}

uint32_t jaleco_vj_king_qtaro_device::int_r(offs_t offset)
{
	auto r = m_int & ~0x10;

	if (m_dma_running[0] || m_dma_running[1] || m_dma_running[2]) {
		// The only time 0x10 is referenced is when ending WriteStream for the individual Qtaro devices.
		// All 3 of the WriteStream cleanup functions start by writing 0 to dma_requested_w and dma_running_w
		// then loop until 0x10 is not set here.
		r |= 0x10;
	}

	return r;
}

void jaleco_vj_king_qtaro_device::int_w(offs_t offset, uint32_t data)
{
	// 0x1000000 is used to trigger an event interrupt in the Qtaro driver.
	// The interrupt will only be accepted and cleared when event_r, event2_r, event_io_r are non-zero.
	// It's set, read, and cleared all in the device driver on the PC so no need to handle it here.
	m_int = data;
}

uint32_t jaleco_vj_king_qtaro_device::int_fpga_r(offs_t offset)
{
	return m_int_fpga;
}

void jaleco_vj_king_qtaro_device::int_fpga_w(offs_t offset, uint32_t data)
{
	m_int_fpga = data;
}

template <int DeviceId>
void jaleco_vj_king_qtaro_device::dma_requested_w(offs_t offset, uint32_t data)
{
	m_dma_running[DeviceId] = data == 1;

	if (data == 1 && m_dma_descriptor_requested_addr[DeviceId] != 0) {
		m_dma_descriptor_addr[DeviceId] = m_dma_descriptor_requested_addr[DeviceId];
		m_dma_descriptor_length[DeviceId] = 0;
		m_dma_descriptor_requested_addr[DeviceId] = 0;
	}

	LOGMASKED(LOG_DMA, "%lf %s dma_requested_w<%d>: %08x\n", machine().time().as_double(), machine().describe_context().c_str(), DeviceId, data);
}

template <int DeviceId>
void jaleco_vj_king_qtaro_device::dma_descriptor_phys_addr_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_DMA, "dma_descriptor_phys_addr_w<%d>: %08x %d\n", DeviceId, data, m_dma_running[DeviceId]);
	m_dma_descriptor_requested_addr[DeviceId] = data;
}

template <int DeviceId>
uint32_t jaleco_vj_king_qtaro_device::dma_running_r(offs_t offset)
{
	return m_dma_running[DeviceId];
}

template <int DeviceId>
void jaleco_vj_king_qtaro_device::dma_running_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_DMA, "dma_running_w<%d>: %08x\n", DeviceId, data);

	if (data == 0)
		m_dma_running[DeviceId] = false;
}

TIMER_CALLBACK_MEMBER(jaleco_vj_king_qtaro_device::video_dma_callback)
{
	address_space &dma_space = *get_pci_busmaster_space();
	for (int device_id = 0; device_id < 3; device_id++) {
		if (!m_dma_running[device_id] || BIT(m_dma_descriptor_addr[device_id], 0)) {
			m_dma_running[device_id] = false;
			continue;
		}

		const uint32_t dmaLength = dma_space.read_dword(m_dma_descriptor_addr[device_id] + 4);
		const uint32_t bufferPhysAddr = dma_space.read_dword(m_dma_descriptor_addr[device_id] + 8);
		const uint32_t burstLength = std::min(
			m_qtaro[device_id]->get_remaining_memory_size(),
			std::min(dmaLength - m_dma_descriptor_length[device_id], DMA_BURST_SIZE)
		);

		if (burstLength == 0)
			continue;

		LOGMASKED(LOG_VERBOSE_DMA, "DMA %d copy %08x + %04x = %08x: %08x bytes\n", device_id, bufferPhysAddr, m_dma_descriptor_length[device_id], bufferPhysAddr + m_dma_descriptor_length[device_id], burstLength);

		uint8_t buf[DMA_BURST_SIZE];
		for (int i = 0; i < burstLength; i++) {
			buf[i] = dma_space.read_byte(bufferPhysAddr + m_dma_descriptor_length[device_id]);
			m_dma_descriptor_length[device_id]++;
		}

		m_qtaro[device_id]->write(buf, burstLength);

		if (m_dma_running[device_id] && m_dma_descriptor_length[device_id] >= dmaLength) {
			const uint32_t nextDescriptorPhysAddr = dma_space.read_dword(m_dma_descriptor_addr[device_id]);
			const uint32_t flags = dma_space.read_dword(m_dma_descriptor_addr[device_id] + 12); // Bit 24 is set to denote the last entry at the same time as bit 0 of the next descriptor addr is set

			LOGMASKED(LOG_DMA, "DMA %d: %08x -> %08x %08x (%d %d)\n", device_id, m_dma_descriptor_addr[device_id], nextDescriptorPhysAddr, m_dma_descriptor_length[device_id], BIT(nextDescriptorPhysAddr, 0), BIT(flags, 24));

			m_dma_descriptor_addr[device_id] = nextDescriptorPhysAddr;
			m_dma_descriptor_length[device_id] = 0;
			m_dma_running[device_id] = BIT(m_dma_descriptor_addr[device_id], 0) == 0 && BIT(flags, 24) == 0;
		}
	}
}

void jaleco_vj_king_qtaro_device::map(address_map &map)
{
	map(0x10, 0x10).r(m_qtaro[0], FUNC(jaleco_vj_qtaro_device::reg2_r));
	map(0x18, 0x1b).rw(m_qtaro[0], FUNC(jaleco_vj_qtaro_device::reg3_r), FUNC(jaleco_vj_qtaro_device::reg3_w));
	map(0x20, 0x20).r(m_qtaro[1], FUNC(jaleco_vj_qtaro_device::reg2_r));
	map(0x28, 0x2b).rw(m_qtaro[1], FUNC(jaleco_vj_qtaro_device::reg3_r), FUNC(jaleco_vj_qtaro_device::reg3_w));
	map(0x30, 0x30).r(m_qtaro[2], FUNC(jaleco_vj_qtaro_device::reg2_r));
	map(0x38, 0x3b).rw(m_qtaro[2], FUNC(jaleco_vj_qtaro_device::reg3_r), FUNC(jaleco_vj_qtaro_device::reg3_w));

	map(0x50, 0x53).w(FUNC(jaleco_vj_king_qtaro_device::dma_requested_w<0>));
	map(0x54, 0x57).w(FUNC(jaleco_vj_king_qtaro_device::dma_descriptor_phys_addr_w<0>));
	map(0x58, 0x5b).rw(FUNC(jaleco_vj_king_qtaro_device::dma_running_r<0>), FUNC(jaleco_vj_king_qtaro_device::dma_running_w<0>));
	map(0x60, 0x63).w(FUNC(jaleco_vj_king_qtaro_device::dma_requested_w<1>));
	map(0x64, 0x67).w(FUNC(jaleco_vj_king_qtaro_device::dma_descriptor_phys_addr_w<1>));
	map(0x68, 0x6b).rw(FUNC(jaleco_vj_king_qtaro_device::dma_running_r<1>), FUNC(jaleco_vj_king_qtaro_device::dma_running_w<1>));
	map(0x70, 0x73).w(FUNC(jaleco_vj_king_qtaro_device::dma_requested_w<2>));
	map(0x74, 0x77).w(FUNC(jaleco_vj_king_qtaro_device::dma_descriptor_phys_addr_w<2>));
	map(0x78, 0x7b).rw(FUNC(jaleco_vj_king_qtaro_device::dma_running_r<2>), FUNC(jaleco_vj_king_qtaro_device::dma_running_w<2>));

	map(0x80, 0x83).rw(FUNC(jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_status_r), FUNC(jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_status_w));
	map(0x84, 0x87).rw(FUNC(jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_r), FUNC(jaleco_vj_king_qtaro_device::qtaro_fpga_firmware_w));
	map(0x88, 0x8b).rw(FUNC(jaleco_vj_king_qtaro_device::fpga_firmware_status_r), FUNC(jaleco_vj_king_qtaro_device::fpga_firmware_status_w));
	map(0x8c, 0x8f).rw(FUNC(jaleco_vj_king_qtaro_device::fpga_firmware_r), FUNC(jaleco_vj_king_qtaro_device::fpga_firmware_w));

	map(0x90, 0x94).rw(FUNC(jaleco_vj_king_qtaro_device::event_io_r), FUNC(jaleco_vj_king_qtaro_device::event_io_w));
	map(0x98, 0x9c).rw(FUNC(jaleco_vj_king_qtaro_device::event_unk_r), FUNC(jaleco_vj_king_qtaro_device::event_unk_w));
	map(0xa0, 0xa4).rw(FUNC(jaleco_vj_king_qtaro_device::event_io_mask_r), FUNC(jaleco_vj_king_qtaro_device::event_io_mask_w));
	map(0xa8, 0xab).rw(FUNC(jaleco_vj_king_qtaro_device::event_mask_r), FUNC(jaleco_vj_king_qtaro_device::event_mask_w));
	map(0xac, 0xaf).rw(FUNC(jaleco_vj_king_qtaro_device::event_r), FUNC(jaleco_vj_king_qtaro_device::event_w));

	map(0xb1, 0xb1).rw(m_qtaro[0], FUNC(jaleco_vj_qtaro_device::reg_r), FUNC(jaleco_vj_qtaro_device::reg_w));
	map(0xb2, 0xb2).rw(m_qtaro[1], FUNC(jaleco_vj_qtaro_device::reg_r), FUNC(jaleco_vj_qtaro_device::reg_w));
	map(0xb3, 0xb3).rw(m_qtaro[2], FUNC(jaleco_vj_qtaro_device::reg_r), FUNC(jaleco_vj_qtaro_device::reg_w));
	map(0xb4, 0xb7).rw(FUNC(jaleco_vj_king_qtaro_device::int_r), FUNC(jaleco_vj_king_qtaro_device::int_w));
	map(0xb8, 0xbb).rw(FUNC(jaleco_vj_king_qtaro_device::int_fpga_r), FUNC(jaleco_vj_king_qtaro_device::int_fpga_w));
}

jaleco_vj_king_qtaro_device::jaleco_vj_king_qtaro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jaleco_vj_king_qtaro_device(mconfig, JALECO_VJ_KING_QTARO, tag, owner, clock)
{
}

jaleco_vj_king_qtaro_device::jaleco_vj_king_qtaro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, type, tag, owner, clock),
	m_qtaro(*this, "qtaro%u", 1)
{
}

void jaleco_vj_king_qtaro_device::device_start()
{
	pci_device::device_start();
	set_ids(0x11ca0007, 0x01, 0x068000, 0x00000000);

	intr_pin = 1; // TODO: Verify with real hardware
	intr_line = 10; // TODO: No idea what this should be on real hardware, but a valid IRQ is required to work

	add_map(256, M_MEM, FUNC(jaleco_vj_king_qtaro_device::map));

	m_dma_timer = timer_alloc(FUNC(jaleco_vj_king_qtaro_device::video_dma_callback), this);
	m_dma_timer->adjust(DMA_TIMER_PERIOD, 0, DMA_TIMER_PERIOD);

	save_item(NAME(m_int));
	save_item(NAME(m_int_fpga));
	save_item(NAME(m_event));
	save_item(NAME(m_event_mask));
	save_item(NAME(m_event_io));
	save_item(NAME(m_event_io_mask));
	save_item(NAME(m_event_unk));
	save_item(NAME(m_event_unk_mask));
	save_item(NAME(m_dma_running));
	save_item(NAME(m_dma_descriptor_requested_addr));
	save_item(NAME(m_dma_descriptor_addr));
	save_item(NAME(m_dma_descriptor_length));
}

void jaleco_vj_king_qtaro_device::device_reset()
{
	m_int = 0;
	m_int_fpga = 0;

	m_event = m_event_mask = 0;
	std::fill(std::begin(m_event_io), std::end(m_event_io), 0);
	std::fill(std::begin(m_event_io_mask), std::end(m_event_io_mask), 0);
	std::fill(std::begin(m_event_unk), std::end(m_event_unk), 0);
	std::fill(std::begin(m_event_unk_mask), std::end(m_event_unk_mask), 0);

	std::fill(std::begin(m_dma_running), std::end(m_dma_running), false);
	std::fill(std::begin(m_dma_descriptor_requested_addr), std::end(m_dma_descriptor_requested_addr), 0);
	std::fill(std::begin(m_dma_descriptor_addr), std::end(m_dma_descriptor_addr), 0);
	std::fill(std::begin(m_dma_descriptor_length), std::end(m_dma_descriptor_length), 0);
}

void jaleco_vj_king_qtaro_device::device_add_mconfig(machine_config &config)
{
	JALECO_VJ_QTARO(config, m_qtaro[0], 0);
	JALECO_VJ_QTARO(config, m_qtaro[1], 0);
	JALECO_VJ_QTARO(config, m_qtaro[2], 0);
}
