// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Matsushita CR-504-K/L (AppleCD 600i), PCB LMJB0299A.

    IC6: MN1880023 control MCU (704 bytes internal RAM)
    IC7: 27C512 firmware EPROM
    IC8: MN664031A CD-ROM/SCSI controller
    IC2: MN662743CDB CD signal/servo processor
    IC10/11: 256K x 4 DRAM each (256 KiB total buffer)

    The firmware runs unmodified; the optical and servo front end is
    provisional. The oscillator frequency and MCU interrupt routing need
    verification.

***************************************************************************/

#include "emu.h"
#include "cr504.h"

#include "coreutil.h"
#include "multibyte.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#define LOG_SERVO (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CR504, cr504_device, "cr504", "Matsushita CR-504 (AppleCD 600i)")

cr504_device::cr504_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CR504, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "controller")
	, device_sound_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_controller(*this, "controller")
	, m_image(*this, "image")
	, m_stream(nullptr)
	, m_tray_timer(nullptr)
	, m_optical_timer(nullptr)
	, m_audio_sector{}
	, m_audio_sample(588)
	, m_audio_control(1)
	, m_audio_gain{0, 0}
	, m_audio_route(4)
	, m_port1(0)
	, m_port3(0)
	, m_port9(0)
	, m_serial_out{}
	, m_serial_count(0)
	, m_servo_registers{}
	, m_servo_read(0)
	, m_servo_sense(false)
	, m_servo_status(0x75)
	, m_servo_frame_reset(false)
	, m_subcode{}
	, m_subcode_index(0)
	, m_toc_index(0)
	, m_frame(-750)
	, m_traverse(0)
	, m_speed(1)
	, m_board_control(0)
	, m_sled_fraction(0)
	, m_tracking(false)
	, m_focus(false)
	, m_tray_open(true)
	, m_image_sequence(0)
{
}

void cr504_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44'100);
	m_tray_timer = timer_alloc(FUNC(cr504_device::tray_tick), this);
	m_optical_timer = timer_alloc(FUNC(cr504_device::optical_tick), this);

	save_item(NAME(m_audio_sector));
	save_item(NAME(m_audio_sample));
	save_item(NAME(m_audio_control));
	save_item(NAME(m_audio_gain));
	save_item(NAME(m_audio_route));
	save_item(NAME(m_port1));
	save_item(NAME(m_port3));
	save_item(NAME(m_port9));
	save_item(NAME(m_serial_out));
	save_item(NAME(m_serial_count));
	save_item(NAME(m_servo_registers));
	save_item(NAME(m_servo_read));
	save_item(NAME(m_servo_sense));
	save_item(NAME(m_servo_status));
	save_item(NAME(m_servo_frame_reset));
	save_item(NAME(m_subcode));
	save_item(NAME(m_subcode_index));
	save_item(NAME(m_toc_index));
	save_item(NAME(m_frame));
	save_item(NAME(m_traverse));
	save_item(NAME(m_speed));
	save_item(NAME(m_board_control));
	save_item(NAME(m_sled_fraction));
	save_item(NAME(m_tracking));
	save_item(NAME(m_focus));
	save_item(NAME(m_tray_open));
	save_item(NAME(m_image_sequence));
}

void cr504_device::device_reset()
{
	m_stream->update();
	std::fill(std::begin(m_audio_sector), std::end(m_audio_sector), 0);
	m_audio_sample = 588;
	m_audio_control = 1;
	m_audio_gain[0] = m_audio_gain[1] = 0;
	m_audio_route = 4;

	// the MCU clears its port latches on reset without driving the pins
	m_port1 = m_port3 = m_port9 = 0;

	m_serial_count = m_servo_read = 0;
	m_servo_sense = false;
	std::fill(std::begin(m_servo_registers), std::end(m_servo_registers), 0);
	m_tray_timer->adjust(attotime::never);
	m_servo_status = 0x75;
	m_servo_frame_reset = false;
	m_subcode_index = m_toc_index = 0;
	m_frame = -750;
	m_traverse = 0;
	m_speed = 1;
	m_board_control = 0;
	m_sled_fraction = 0;
	m_tracking = m_focus = false;
	m_tray_open = !m_image->exists();
	m_image_sequence = m_image->sequence_counter();
	m_optical_timer->adjust(attotime::from_hz(75));
}

void cr504_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples() && m_audio_sample < 588; ++i, ++m_audio_sample)
	{
		const u8 *const data = &m_audio_sector[4 * m_audio_sample];
		const s16 left = get_s16be(&data[0]);
		const s16 right = get_s16be(&data[2]);
		for (int channel = 0; channel < 2; ++channel)
		{
			if ((m_audio_control & 3) == 1 || !BIT(m_port9, 4 + channel))
			{
				continue;
			}

			const u8 route = BIT(m_audio_route, channel * 2, 2);
			const int sample = (route == 0) ? left : (route == 1) ? right : (route == 2) ? ((left + right) / 2) : 0;
			// TODO: MN662743 gain law, soft mute and de-emphasis (linear n/256 is from the MN662785)
			const int gain = ((m_audio_control & 3) == 0) ? 256 : m_audio_gain[channel];
			stream.put_int(channel, i, sample * gain, 32768 * 256);
		}
	}
}

u8 cr504_device::port0_r()
{
	// P0.1: servo SENSE, or focus OK in status modes 72 and 77
	const bool sense = (m_servo_status == 0x72 || m_servo_status == 0x77) ? (disc_present() && m_focus) : m_servo_sense;
	return sense ? 0x02 : 0;
}

u8 cr504_device::port1_r()
{
	// P1.0/P1.1: active-low focus status
	return (disc_present() && m_focus) ? 0 : 0x03;
}

u8 cr504_device::port2_r()
{
	// P2.5: active-low controller IRQ
	return m_controller->irq_r() ? 0 : 0x20;
}

u8 cr504_device::port8_r()
{
	// P8.1/P8.2: active-low SCSI ID jumpers (the third is on the controller)
	return ((~m_controller->scsi_id() & 3) << 1) | (m_tray_open ? 0x08 : 0x10) | ((m_frame <= -750) ? 0 : 0x20);
}

u8 cr504_device::port9_r()
{
	return (disc_present() && m_tracking) ? 0x40 : 0;
}

void cr504_device::port1_w(u8 data)
{
	// P1.4: active-low tray motor enable, P1.3: open; travel time is a guess
	if ((m_port1 ^ data) & 0x18)
	{
		m_tray_timer->adjust(BIT(data, 4) ? attotime::never : attotime::from_seconds(1));
	}
	m_port1 = data;
}

void cr504_device::port3_w(u8 data)
{
	// P3.2: servo /RESET, P3.0: latches the serial command
	const u8 changed = m_port3 ^ data;
	m_port3 = data;
	if (BIT(changed, 2))
	{
		m_servo_sense = BIT(data, 2);
		m_serial_count = 0;
	}

	if (BIT(changed & data, 0))
	{
		servo_command();
	}
}

void cr504_device::port9_w(u8 data)
{
	// P9.4/P9.5: audio output enables
	m_stream->update();
	m_port9 = data;
}

void cr504_device::servo_serial_w(u8 data)
{
	if (m_serial_count < std::size(m_serial_out))
	{
		m_serial_out[m_serial_count++] = ~data;
	}
}

u8 cr504_device::servo_serial_r()
{
	// status 76 reads spindle speed (74 is an inferred locked value), f4 a servo register
	const u8 data = (m_servo_status == 0x76)
			? ((disc_present() && m_tracking && !m_servo_frame_reset) ? 0x74 : 0)
			: m_servo_registers[m_servo_read];
	return ~data;
}

u8 cr504_device::subcode_serial_r()
{
	return (m_subcode_index < std::size(m_subcode)) ? ~m_subcode[m_subcode_index++] : 0xff;
}

u8 cr504_device::sled_level_r()
{
	// ideal filtered PWM level: firmware maps 13 PWM steps to 128 ADC steps around 2f3
	return std::clamp(128 + (int(m_mcu->pwm_compare()) - 0x2f3) * 128 / 13, 0, 255);
}

void cr504_device::sled_pwm_w(int state)
{
	if (!state)
	{
		return;
	}

	// board GPIO5 enables the coarse sled driver
	if (disc_present() && BIT(m_board_control, 5) && m_mcu->pwm_output_enabled())
	{
		const int reload = m_mcu->pwm_reload();
		const int compare = m_mcu->pwm_compare();
		const int neutral = (0x400 + reload) / 2;

		// provisional motor gain, in tracks/second per PWM step
		m_sled_fraction += (neutral - compare) * 200.0 * 4 * (0x400 - reload) / m_mcu->clock();
		const int tracks = int(m_sled_fraction);
		m_sled_fraction -= tracks;
		for (int i = 0; i < std::abs(tracks); ++i)
		{
			const s32 previous = m_frame;
			move_pickup((tracks > 0) ? 1 : -1);
			if (previous != m_frame)
			{
				m_mcu->timer4_count_w(1);
				m_mcu->timer4_count_w(0);
			}
		}
	}
	else
	{
		m_sled_fraction = 0;
	}
}

bool cr504_device::disc_present() const
{
	return m_image->exists() && !m_tray_open;
}

TIMER_CALLBACK_MEMBER(cr504_device::tray_tick)
{
	m_stream->update();
	m_tray_open = BIT(m_port1, 3);
	m_tracking = m_focus = false;
	m_audio_sample = 588;
	if (m_tray_open && m_image->exists())
	{
		m_image->unload();
		m_image_sequence = m_image->sequence_counter();
	}
}

void cr504_device::servo_command()
{
	if (!m_serial_count)
	{
		return;
	}

	m_stream->update();
	const u8 command = m_serial_out[m_serial_count - 1];
	LOGMASKED(LOG_SERVO, "Servo command %02x (%u bytes: %02x %02x)\n", command, m_serial_count, m_serial_out[0], m_serial_out[1]);
	if ((command & 0xf0) == 0x20 && m_serial_count == 1)
	{
		m_audio_control = command & 0x0f;
	}

	switch (command)
	{
	case 0x43:
		// selector bits 6/7 latch the left/right gain
		if (m_serial_count == 3)
		{
			m_audio_route = m_serial_out[1] & 0x0f;
			if (BIT(m_serial_out[1], 6))
			{
				m_audio_gain[0] = m_serial_out[0];
			}

			if (BIT(m_serial_out[1], 7))
			{
				m_audio_gain[1] = m_serial_out[0];
			}
		}
		break;
	case 0x45:
		if (m_serial_count == 3)
		{
			m_speed = BIT(m_serial_out[1], 3) ? 4 : BIT(m_serial_out[0], 6) ? 2 : 1;
		}
		break;
	case 0x72:
	case 0x77:
	case 0x75:
		m_servo_status = command;
		break;
	case 0x76:
		m_servo_status = command;
		if (m_serial_count == 3)
		{
			m_servo_frame_reset = BIT(m_serial_out[1], 7);
		}
		break;
	case 0xe2:
		m_servo_sense = disc_present();
		break;
	case 0xe0:
	case 0xe3:
		m_focus = command == 0xe3;
		m_tracking = false;
		m_audio_sample = 588;
		m_servo_sense = false;
		break;
	case 0xe4:
	case 0xe5:
		m_tracking = m_focus = disc_present();
		m_servo_sense = false;
		break;
	case 0xe8:
		m_traverse = 0;
		break;
	case 0xea:
	case 0xeb:
		m_traverse = (command == 0xea) ? 1 : -1;
		break;
	case 0xec: case 0xed: case 0xee: case 0xef:
		m_traverse = 0;
		break;
	case 0xf1:
		if (m_serial_count == 3)
		{
			move_pickup(((m_serial_out[0] & 0x7f) * 256 + m_serial_out[1]) * (BIT(m_serial_out[0], 7) ? 1 : -1));
		}
		m_servo_sense = false;
		break;
	case 0xf7: case 0xf9: case 0xfa: case 0xfb:
	case 0xfc: case 0xfd: case 0xfe: case 0xff:
		m_servo_sense = false;
		break;
	case 0xf2:
		if (m_serial_count == 3)
		{
			m_servo_registers[m_serial_out[1]] = m_serial_out[0];
		}
		m_servo_sense = false;
		break;
	case 0xf4:
		m_servo_status = command;
		if (m_serial_count == 3)
		{
			m_servo_read = m_serial_out[1];
		}
		m_servo_sense = false;
		break;
	case 0xf5:
		m_servo_sense = false;
		break;
	}
	m_serial_count = 0;
}

void cr504_device::move_pickup(int tracks)
{
	m_stream->update();
	m_audio_sample = 588;

	// ideal CLV spiral: 1.2 m/s, 1.6 um pitch, 26 mm program area radius
	constexpr double pitch = 0.0016;
	constexpr double radius = 26.0;
	constexpr double pi = 3.14159265358979323846;
	const double current_radius = std::sqrt(radius * radius + (m_frame - 150) * 16.0 * pitch / pi);
	const double target_radius = std::max(25.0, current_radius + tracks * pitch);
	m_frame = std::max(-750, int(std::lround((target_radius * target_radius - radius * radius) * pi / (16.0 * pitch))) + 150);
}

u32 cr504_device::optical_track() const
{
	// Q changes track at INDEX 0, the image at INDEX 1
	const s32 lba = m_frame - 150;
	const auto &toc = m_image->get_toc();
	u32 track = 0;
	while (track + 1 < toc.numtrks && lba >= s32(toc.tracks[track + 1].logframeofs - toc.tracks[track + 1].pregap))
	{
		++track;
	}
	return track;
}

bool cr504_device::read_sector(u8 *data, u32 track, u32 type)
{
	const auto &info = m_image->get_toc().tracks[track];
	const s32 offset = m_frame - 150 - info.logframeofs;
	if (offset < 0 && (!info.pgdatasize || offset < -s32(info.pregap)))
	{
		return true; // unstored pregap; caller supplies zero-filled data
	}

	// read physical frames so a stored INDEX 0 belongs to its own track
	return m_image->read_data(info.physframeofs + offset + (info.pgdatasize ? info.pregap : 0), data, type, true);
}

void cr504_device::make_subcode()
{
	std::fill(std::begin(m_subcode), std::end(m_subcode), 0);
	const auto msf = [this] (u32 offset, u32 frame) { put_u24be(&m_subcode[offset], cdrom_file::lba_to_msf(frame)); };
	if (m_frame < 0)
	{
		// lead-in Q is synthesized from the TOC
		const u32 entry = (m_toc_index++ / 3) % (m_image->get_last_track() + 3);
		const u8 control = m_image->get_adr_control((entry == 0) ? 0 : (m_image->get_last_track() - 1));
		m_subcode[0] = (control << 4) | (control >> 4);
		msf(3, 100 * 60 * 75 + m_frame);
		if (entry == 0)
		{
			m_subcode[2] = 0xa0;
			m_subcode[7] = 1;
		}
		else if (entry == 1)
		{
			m_subcode[2] = 0xa1;
			m_subcode[7] = dec_2_bcd(m_image->get_last_track());
		}
		else
		{
			const u32 track = (entry == 2) ? 0xaa : (entry - 3);
			m_subcode[2] = (entry == 2) ? 0xa2 : dec_2_bcd(track + 1);
			const u8 adr_control = m_image->get_adr_control(track);
			m_subcode[0] = (adr_control << 4) | (adr_control >> 4);
			msf(7, m_image->get_track_start(track) + 150);
		}
	}
	else
	{
		const u32 lba = std::max(0, m_frame - 150);
		const bool leadout = lba >= m_image->get_track_start(0xaa);
		const u32 track = leadout ? 0xaa : optical_track();
		const u8 adr_control = m_image->get_adr_control(track);
		m_subcode[0] = (adr_control << 4) | (adr_control >> 4);
		m_subcode[1] = leadout ? 0xaa : dec_2_bcd(track + 1);
		const s32 relative = m_frame - 150 - m_image->get_track_start(track);
		m_subcode[2] = (relative < 0) ? 0 : leadout ? 1 : dec_2_bcd(m_image->get_track_index(lba));
		msf(3, std::abs(relative));
		msf(7, m_frame);
	}
}

TIMER_CALLBACK_MEMBER(cr504_device::optical_tick)
{
	m_stream->update();
	m_audio_sample = 588;
	m_optical_timer->adjust(attotime::from_hz(75 * m_speed));
	if (m_image_sequence != m_image->sequence_counter())
	{
		// mounting an image inserts a disc and closes the tray
		m_image_sequence = m_image->sequence_counter();
		m_frame = -750;
		m_toc_index = 0;
		m_tracking = m_focus = false;
		m_tray_open = !m_image->exists();
		m_tray_timer->adjust(attotime::never);
	}

	if (!disc_present())
	{
		return;
	}

	if (m_traverse && !BIT(m_board_control, 5))
	{
		m_frame = std::max(-750, m_frame + m_traverse * 6);
	}

	if (!m_tracking)
	{
		return;
	}

	make_subcode();
	m_controller->subcode_w(m_subcode);
	m_subcode_index = 0;
	m_mcu->timer2_capture_w(1);
	m_mcu->timer2_capture_w(0);
	m_mcu->pulse_input_line(INPUT_LINE_IRQ2, attotime::zero);

	if (m_frame >= 0 && m_frame < s32(m_image->get_track_start(0xaa) + 150))
	{
		u8 sector[2352]{};
		const u32 track = optical_track();
		const auto &info = m_image->get_toc().tracks[track];
		const bool pregap = m_frame - 150 < s32(info.logframeofs) && !info.pgdatasize;
		const u32 type = info.trktype;

		// the first track's unstored pregap is reconstructed for header acquisition
		if (type == cdrom_file::CD_TRACK_MODE1 || (pregap && type == cdrom_file::CD_TRACK_MODE1_RAW))
		{
			const bool valid = read_sector(sector + 16, track, cdrom_file::CD_TRACK_MODE1);
			std::fill_n(sector + 1, 10, 0xff);
			put_u24be(&sector[12], cdrom_file::lba_to_msf(m_frame));
			sector[15] = 1;
			// TODO: cooked images have no EDC/ECC to check
			if (valid)
			{
				m_controller->sector_w(sector);
			}
		}
		else if (!pregap && (type == cdrom_file::CD_TRACK_MODE1_RAW || type == cdrom_file::CD_TRACK_MODE2_RAW)
				&& read_sector(sector, track, cdrom_file::CD_TRACK_RAW_DONTCARE))
		{
			m_controller->sector_w(sector);
		}
		else if (type == cdrom_file::CD_TRACK_AUDIO && m_speed == 1)
		{
			std::fill(std::begin(m_audio_sector), std::end(m_audio_sector), 0);
			if (read_sector(m_audio_sector, track, cdrom_file::CD_TRACK_AUDIO))
			{
				m_audio_sample = 0;
			}
		}
	}

	++m_frame;
}

void cr504_device::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("firmware", 0);
}

void cr504_device::data_map(address_map &map)
{
	map(0x8000, 0x803f).m(m_controller, FUNC(mn664031_device::map));
}

void cr504_device::device_add_mconfig(machine_config &config)
{
	MN1880023(config, m_mcu, 20'000'000); // inferred from timer reloads and Q-frame capture thresholds
	m_mcu->set_addrmap(AS_PROGRAM, &cr504_device::program_map);
	m_mcu->set_addrmap(AS_DATA, &cr504_device::data_map);
	m_mcu->set_port_input_mask(0, 0x02);
	m_mcu->port_in_cb<0>().set(FUNC(cr504_device::port0_r));
	m_mcu->set_port_input_mask(1, 0x03);
	m_mcu->port_in_cb<1>().set(FUNC(cr504_device::port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(cr504_device::port1_w));
	m_mcu->set_port_input_mask(2, 0x20);
	m_mcu->port_in_cb<2>().set(FUNC(cr504_device::port2_r));
	m_mcu->port_out_cb<3>().set(FUNC(cr504_device::port3_w));
	m_mcu->set_port_input_mask(4, 0x08);
	m_mcu->port_in_cb<4>().set_constant(0x08); // active-low eject switch, released
	m_mcu->set_port_input_mask(8, 0x3e);
	m_mcu->port_in_cb<8>().set(FUNC(cr504_device::port8_r));
	m_mcu->set_port_input_mask(9, 0x40);
	m_mcu->port_in_cb<9>().set(FUNC(cr504_device::port9_r));
	m_mcu->port_out_cb<9>().set(FUNC(cr504_device::port9_w));
	m_mcu->sio_out_cb<0>().set(FUNC(cr504_device::servo_serial_w));
	m_mcu->sio_in_cb<0>().set(FUNC(cr504_device::servo_serial_r));
	m_mcu->sio_in_cb<1>().set(FUNC(cr504_device::subcode_serial_r));
	m_mcu->pwm_out_cb().set(FUNC(cr504_device::sled_pwm_w));

	// ADC6 measures the filtered sled PWM; the other inputs read mid-scale
	m_mcu->ad_in_cb<0>().set_constant(0x80);
	m_mcu->ad_in_cb<1>().set_constant(0x80);
	m_mcu->ad_in_cb<2>().set_constant(0x80);
	m_mcu->ad_in_cb<3>().set_constant(0x80);
	m_mcu->ad_in_cb<4>().set_constant(0x80);
	m_mcu->ad_in_cb<5>().set_constant(0x80);
	m_mcu->ad_in_cb<6>().set(FUNC(cr504_device::sled_level_r));
	m_mcu->ad_in_cb<7>().set_constant(0x80);

	MN664031(config, m_controller);
	m_controller->gpio_callback().set([this] (u8 data) { m_board_control = data; });
	m_controller->irq_callback().set_inputline(m_mcu, INPUT_LINE_IRQ1);
	m_controller->decoder_irq_callback().set_inputline(m_mcu, INPUT_LINE_IRQ0);

	CDROM(config, m_image).set_interface("cdrom");
}

ROM_START(cr504)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("504par4.0i.ic7", 0x0000, 0x10000, CRC(25f7dd46) SHA1(ec3b3031742807924c6259af865e701827208fec))
ROM_END

const tiny_rom_entry *cr504_device::device_rom_region() const
{
	return ROM_NAME(cr504);
}
