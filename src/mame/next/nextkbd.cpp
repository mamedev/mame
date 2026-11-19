// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "nextkbd.h"

#include <algorithm>
#include <cmath>

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_CMD  (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW|LOG_CMD)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEXTKBD, nextkbd_device, "nextkbd", "NeXT Keyboard/Mouse/Sound")

void nextkbd_device::amap(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(nextkbd_device::status_snd_r), FUNC(nextkbd_device::ctrl_snd_w));
	map(0x1, 0x1).rw(FUNC(nextkbd_device::status_kms_r), FUNC(nextkbd_device::ctrl_kms_w));
	map(0x2, 0x2).rw(FUNC(nextkbd_device::status_dma_r), FUNC(nextkbd_device::ctrl_dma_w));
	map(0x3, 0x3).rw(FUNC(nextkbd_device::status_cmd_r), FUNC(nextkbd_device::ctrl_cmd_w));
	map(0x4, 0x7).rw(FUNC(nextkbd_device::cdata_r), FUNC(nextkbd_device::cdata_w));
	map(0x8, 0xb).rw(FUNC(nextkbd_device::kmdata_r), FUNC(nextkbd_device::kmdata_w));
}

nextkbd_device::nextkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEXTKBD, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	int_change_cb(*this),
	int_power_cb(*this),
	int_nmi_cb(*this),
	sound_drq_cb(*this),
	sound_irq_cb(*this),
	dac(*this, "dac%u", 0U),
	mousex(*this, "mousex"),
	mousey(*this, "mousey"),
	mousebtn(*this, "mousebtn"),
	revision(0)
{
}

void nextkbd_device::device_add_mconfig(machine_config &config)
{
	for(int channel = 0; channel != 2; channel++)
		DAC_16BIT_R2R_TWOS_COMPLEMENT(config, dac[channel]).add_route(0, *this, 1.0, channel);
}

void nextkbd_device::device_start()
{
	poll_timer = timer_alloc(FUNC(nextkbd_device::update_tick), this);
	sound_timer = timer_alloc(FUNC(nextkbd_device::sound_tick), this);

	save_item(NAME(ctrl_snd));
	save_item(NAME(ctrl_kms));
	save_item(NAME(ctrl_dma));
	save_item(NAME(ctrl_cmd));
	save_item(NAME(cdata));
	save_item(NAME(kmdata));
	save_item(NAME(fifo_ir));
	save_item(NAME(fifo_iw));
	save_item(NAME(fifo_size));
	save_item(NAME(fifo));
	save_item(NAME(modifiers_state));
	save_item(NAME(nmi_active));
	save_item(NAME(km_address));
	save_item(NAME(prev_mousex));
	save_item(NAME(prev_mousey));
	save_item(NAME(prev_mousebtn));
	save_item(NAME(sound_data));
	save_item(NAME(sound_sample));
	save_item(NAME(dac_sample));
	save_item(NAME(sound_bytes));
	save_item(NAME(sound_mode));
	save_item(NAME(sound_active));
	save_item(NAME(sound_repeat));
	save_item(NAME(sound_drq));
	save_item(NAME(sound_irq));
	save_item(NAME(sound_gpo));
	save_item(NAME(volume_shift));
	save_item(NAME(volume_bits));
	save_item(NAME(volume));
}

void nextkbd_device::device_reset()
{
	ctrl_snd = 0x00;
	ctrl_kms = 0x00;
	ctrl_dma = 0x00;
	ctrl_cmd = 0x00;
	cdata = 0;
	kmdata = 0;
	fifo_ir = 0;
	fifo_iw = 0;
	fifo_size = 0;
	memset(fifo, 0, sizeof(fifo));
	modifiers_state = 0;
	nmi_active = false;
	prev_mousex = 0;
	prev_mousey = 0;
	prev_mousebtn = 0;
	km_address = 0;
	sound_data = sound_sample = dac_sample = 0;
	sound_bytes = sound_mode = sound_gpo = 0;
	volume_shift = volume_bits = 0;
	volume[0] = volume[1] = 0;
	update_gain();
	sound_active = sound_repeat = sound_drq = sound_irq = false;
	sound_timer->adjust(attotime::never);
	sound_drq_cb(false);
	sound_irq_cb(false);
	sound_output(0);
	poll_timer->adjust(attotime::from_hz(200), 0, attotime::from_hz(200));
}

void nextkbd_device::device_post_load()
{
	update_gain();
}

// The KMS serial link carries one big-endian 16-bit sample per channel.
// Sound commands and the serial volume-control protocol are documented in
// https://github.com/mihaip/previous/blob/master/src/kms.c and src/snd.c.
// TODO: the switchable analog low-pass filter (GPO bit 3).
void nextkbd_device::sound_output(uint32_t sample)
{
	dac_sample = sample;
	for(int channel = 0; channel != 2; channel++) {
		const int16_t value = int16_t(channel ? sample : (sample >> 16));
		const double gain = BIT(sound_gpo, 4) ? 0.0 : sound_gain[channel];
		dac[channel]->write(int16_t(value * gain));
	}
}

void nextkbd_device::sound_request()
{
	const bool request = sound_active && (ctrl_dma & C_KMS_ENABLE) && sound_bytes < 4;
	if(request)
		ctrl_snd |= C_SOUND_OUT_REQUEST;
	else
		ctrl_snd &= ~C_SOUND_OUT_REQUEST;
	const bool drq = request && (ctrl_snd & C_SOUND_OUT_ENABLE);
	if(drq)
		ctrl_dma |= C_SDMA_OUT_PENDING;
	else
		ctrl_dma &= ~(C_SDMA_OUT_PENDING | C_SDMA_OUT_RUNNING);
	if(sound_drq != drq) {
		sound_drq = drq;
		sound_drq_cb(drq);
	}
}

void nextkbd_device::update_sound_irq()
{
	const bool state = ctrl_snd & (C_SOUND_OUT_UNDERRUN | C_SOUND_IN_OVERRUN);
	if(sound_irq != state) {
		sound_irq = state;
		sound_irq_cb(state);
	}
}

void nextkbd_device::sound_dma_w(uint8_t data)
{
	ctrl_dma |= C_SDMA_OUT_RUNNING;
	sound_data = (sound_data << 8) | data;
	++sound_bytes;
	// Withdraw DRQ after a stereo frame. The sample clock requests the next
	// frame, rather than allowing the DMA controller to drain the whole buffer.
	sound_request();
}

TIMER_CALLBACK_MEMBER(nextkbd_device::sound_tick)
{
	if(sound_repeat) {
		sound_repeat = false;
		sound_output(BIT(sound_mode, 5) ? 0 : sound_sample);
		return;
	}
	if(sound_bytes == 4) {
		sound_sample = sound_data;
		sound_bytes = 0;
	} else {
		sound_sample = 0;
		ctrl_snd |= C_SOUND_OUT_UNDERRUN;
		update_sound_irq();
	}
	sound_output(sound_sample);
	sound_repeat = BIT(sound_mode, 4);
	sound_request();
}

void nextkbd_device::sound_enable(uint8_t command)
{
	sound_mode = command & 0x30;
	const bool enable = BIT(command, 3) && (ctrl_dma & C_KMS_ENABLE);
	if(enable && !sound_active) {
		sound_repeat = false;
		sound_timer->adjust(attotime::from_hz(44'100), 0, attotime::from_hz(44'100));
	} else if(!enable) {
		sound_timer->adjust(attotime::never);
		sound_bytes = 0;
		sound_sample = 0;
		ctrl_snd &= ~C_SOUND_OUT_UNDERRUN;
		update_sound_irq();
		sound_output(0);
	}
	sound_active = enable;
	sound_request();
}

void nextkbd_device::update_gain()
{
	for(int channel = 0; channel != 2; channel++)
		sound_gain[channel] = (volume[channel] >= 43) ? 0.0 : std::pow(10.0, -double(volume[channel]) / 10.0);
}

void nextkbd_device::sound_volume(uint8_t data)
{
	if(BIT(data, 6))
		volume[0] = data & 0x3f;
	if(BIT(data, 7))
		volume[1] = data & 0x3f;
	update_gain();
	sound_output(dac_sample);
}

void nextkbd_device::sound_control(uint8_t data)
{
	// Eleven bits, MSB first: 111, right/left selects, six-bit attenuation
	// in 2 dB steps. STROBE latches the word; CLOCK samples DATA on rising edges.
	if(BIT(data, 0) && !BIT(sound_gpo, 0)) {
		if(volume_bits == 11 && (volume_shift & 0x700) == 0x700)
			sound_volume(uint8_t(volume_shift));
	} else if(!BIT(data, 0) && BIT(sound_gpo, 0)) {
		volume_shift = 0;
		volume_bits = 0;
	} else if(!BIT(data, 0) && BIT(data, 2) && !BIT(sound_gpo, 2)) {
		volume_shift = (volume_shift << 1) | BIT(data, 1);
		volume_bits = std::min<uint8_t>(volume_bits + 1, 12);
	}
	sound_gpo = data;
	sound_output(dac_sample);
}

void nextkbd_device::send()
{
	if(ctrl_kms & C_KBD_DATA)
		return;

	kmdata = fifo_pop();
	ctrl_kms |= C_KBD_DATA;
	if(!(ctrl_kms & C_KBD_INTERRUPT)) {
		ctrl_kms |= C_KBD_INTERRUPT;
		int_change_cb(true);
	}
}

void nextkbd_device::update_mouse(bool force_update)
{
	uint32_t cur_mousex   = mousex->read();
	uint32_t cur_mousey   = mousey->read();
	uint32_t cur_mousebtn = mousebtn->read();

	if(!force_update && cur_mousex == prev_mousex && cur_mousey == prev_mousey && cur_mousebtn == prev_mousebtn)
		return;

	int32_t deltax = -(cur_mousex - prev_mousex);
	int32_t deltay = -(cur_mousey - prev_mousey);

	if(deltax >= 128)
		deltax -= 256;
	if(deltax < -128)
		deltax += 256;
	if(deltay >= 128)
		deltay -= 256;
	if(deltay < -128)
		deltay += 256;

	prev_mousex   = cur_mousex;
	prev_mousey   = cur_mousey;
	prev_mousebtn = cur_mousebtn;

	if(deltax < -0x40)
		deltax = -0x40;
	if(deltax > 0x3f)
		deltax = 0x3f;
	if(deltay < -0x40)
		deltay = -0x40;
	if(deltay > 0x3f)
		deltay = 0x3f;

	uint32_t base = ((deltax & 0x7f) << 1) | ((deltay & 0x7f) << 9) | cur_mousebtn;
	fifo_push(km_address | D_SECONDARY | base);
	send();
}

TIMER_CALLBACK_MEMBER(nextkbd_device::update_tick)
{
	if(!fifo_empty())
		send();

	update_mouse(false);
}

void nextkbd_device::fifo_push(uint32_t val)
{
	if(fifo_size == FIFO_SIZE)
		return;

	fifo_size++;
	fifo[fifo_iw++] = val;
	if(fifo_iw == FIFO_SIZE)
		fifo_iw = 0;
}

uint32_t nextkbd_device::fifo_pop()
{
	if(fifo_size == 0)
		return 0;

	fifo_size--;
	uint32_t res = fifo[fifo_ir++];
	if(fifo_ir == FIFO_SIZE)
		fifo_ir = 0;

	return res;
}

bool nextkbd_device::fifo_empty() const
{
	return !fifo_size;
}

uint8_t nextkbd_device::status_snd_r()
{
	LOGMASKED(LOG_REGR, "status_snd_r %02x %s\n", ctrl_snd, machine().describe_context());
	return ctrl_snd;
}

uint8_t nextkbd_device::status_kms_r()
{
	LOGMASKED(LOG_REGR, "status_kms_r %02x %s\n", ctrl_kms, machine().describe_context());
	return ctrl_kms;
}

uint8_t nextkbd_device::status_dma_r()
{
	LOGMASKED(LOG_REGR, "status_dma_r %02x %s\n", ctrl_dma, machine().describe_context());
	return ctrl_dma;
}

uint8_t nextkbd_device::status_cmd_r()
{
	LOGMASKED(LOG_REGR, "status_cmd_r %02x %s\n", ctrl_cmd, machine().describe_context());
	return ctrl_cmd;
}

uint32_t nextkbd_device::cdata_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_REGR, "cdata_r %08x @ %08x %s\n", cdata, mem_mask, machine().describe_context());
	return cdata;
}

uint32_t nextkbd_device::kmdata_r(offs_t offset, uint32_t mem_mask)
{
	if(!machine().side_effects_disabled()) {
		uint8_t old = ctrl_kms;
		ctrl_kms &= ~(C_KBD_INTERRUPT|C_KBD_DATA);
		if(old & C_KBD_INTERRUPT)
			int_change_cb(false);
	}
	LOGMASKED(LOG_REGR, "kmdata_r %08x @ %08x %s\n", kmdata, mem_mask, machine().describe_context());
	return kmdata;
}

void nextkbd_device::ctrl_snd_w(uint8_t data)
{
	LOGMASKED(LOG_REGW, "ctrl_snd_w %02x %s\n", data, machine().describe_context());
	ctrl_snd = (ctrl_snd & ~(C_SOUND_OUT_ENABLE | C_SOUND_IN_ENABLE)) | (data & (C_SOUND_OUT_ENABLE | C_SOUND_IN_ENABLE));
	if(!sound_active && (data & C_SOUND_OUT_UNDERRUN))
		ctrl_snd &= ~C_SOUND_OUT_UNDERRUN;
	if(data & C_SOUND_IN_OVERRUN)
		ctrl_snd &= ~C_SOUND_IN_OVERRUN;
	update_sound_irq();
	sound_request();
}

void nextkbd_device::ctrl_kms_w(uint8_t data)
{
	ctrl_kms = (ctrl_kms & ~C_KMS_WMASK) | (data & C_KMS_WMASK);
	LOGMASKED(LOG_REGW, "ctrl_kms_w %02x %s\n", ctrl_kms, machine().describe_context());
}

void nextkbd_device::ctrl_dma_w(uint8_t data)
{
	ctrl_dma = (ctrl_dma & ~C_WMASK) | (data & C_WMASK);
	LOGMASKED(LOG_REGW, "ctrl_dma_w %02x %s\n", ctrl_dma, machine().describe_context());
	if(!(ctrl_dma & C_KMS_ENABLE))
		sound_enable(0);
}

void nextkbd_device::ctrl_cmd_w(uint8_t data)
{
	ctrl_cmd = data;
	LOGMASKED(LOG_REGW, "ctrl_cmd_w %02x %s\n", ctrl_cmd, machine().describe_context());
}

void nextkbd_device::cdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&cdata);
	LOGMASKED(LOG_REGW, "cdata_w %08x @ %08x %s\n", data, mem_mask, machine().describe_context());
	handle_command();
}

void nextkbd_device::kmdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_REGW, "kmdata_w %08x @ %08x %s\n", data, mem_mask, machine().describe_context());
}

INPUT_CHANGED_MEMBER( nextkbd_device::update )
{
	int bank = param;
	switch(bank) {
	case 0: case 1: case 2: {
		int index;
		for(index=0; index < 32; index++)
			if(field.mask() & (1 << index))
				break;
		assert(index != 32);
		index += bank*32;
		uint16_t val = index | modifiers_state | D_KBD_VALID;
		if(!newval)
			val |= D_KBD_KEYDOWN;
		if(val == 0x8826 || val == 0x884a) {
			nmi_active = true;
			int_nmi_cb(true);
		} else if(nmi_active) {
			nmi_active = false;
			int_nmi_cb(false);
		}
		fifo_push(val | D_MASTER | km_address);
		send();
		break;
	}

	case 3:
		if(newval)
			modifiers_state |= field.mask();
		else
			modifiers_state &= ~field.mask();
		fifo_push(modifiers_state | D_MASTER | km_address);

		send();
		break;

	case 4:
		if(field.mask() & 1)
			int_power_cb(newval & 1);
		break;
	}
}

void nextkbd_device::handle_fifo_command()
{
	LOGMASKED(LOG_CMD, "Fifo command %08x\n", cdata);
	fifo_ir = 0;
	fifo_iw = 0;
	fifo_size = 0;
	memset(fifo, 0, sizeof(fifo));
	if(ctrl_kms & C_KBD_INTERRUPT)
		int_change_cb(false);
	ctrl_kms &= ~(C_KBD_INTERRUPT|C_KBD_DATA);
}

void nextkbd_device::handle_kbd_command()
{
	switch(cdata >> 24) {
	case 0x00:
		logerror("Keyboard LED control %06x?\n", cdata & 0xffffff);
		ctrl_kms |= C_KBD_DATA; // Hmmmm.  The rom wants it, but I'm not sure if data is actually expected
		break;

	case 0xef:
		logerror("Set keyboard/mouse address to %d\n", (cdata >> 17) & 7);
		km_address = ((cdata >> 17) & 7) << 25;
		ctrl_kms |= C_KBD_DATA; // Hmmmm.  The rom wants it, but I'm not sure if data is actually expected
		break;

	default:
		logerror("Unhandled keyboard command %02x.%06x\n", cdata >> 24, cdata & 0xffffff);
		break;
	}
}

void nextkbd_device::handle_command()
{
	if((ctrl_cmd & 0xc7) == 0x07) {
		sound_enable(ctrl_cmd);
		return;
	}
	switch(ctrl_cmd) {
	case 0xc2:
		// only the later KMS takes the volume directly
		if(!revision)
			logerror("Volume command on the original KMS %08x\n", cdata);
		else if(ctrl_dma & C_KMS_ENABLE)
			sound_volume(cdata >> 24);
		break;

	case 0xc4:
		if(ctrl_dma & C_KMS_ENABLE)
			sound_control(cdata >> 24);
		break;

	case 0xc5:
		handle_kbd_command();
		break;

	case 0xc6:
		handle_fifo_command();
		break;

	case 0xc7:
		if(sound_active) {
			sound_data = cdata;
			sound_bytes = 4;
			sound_request();
		}
		break;

	default:
		logerror("Unhandled command %02x.%08x\n", ctrl_cmd, cdata);
		break;
	}
}

static INPUT_PORTS_START(nextkbd_keymap)
	PORT_START("0")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 0) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')

	PORT_START("1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) // Keypad = ?
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 1) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')

	PORT_START("2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0xfffe0000, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 2)

	PORT_START("modifiers")
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Control")
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Left)")
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Right)")
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_LWIN)       PORT_NAME("Command (Left)")
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_RWIN)       PORT_NAME("Command (Right)")
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_NAME("Alt (Left)")
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3) PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Alt (Right)")
	PORT_BIT(0xffff80ff, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 3)

	PORT_START("special")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 4) PORT_CODE(KEYCODE_HOME)       PORT_NAME("Power")
	PORT_BIT(0xfffffffe, IP_ACTIVE_HIGH, IPT_UNUSED)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nextkbd_device::update), 4)

	PORT_START("mousex")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("mousey")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("mousebtn")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
INPUT_PORTS_END

ioport_constructor nextkbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nextkbd_keymap);
}
