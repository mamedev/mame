// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "nextkbd.h"

DEFINE_DEVICE_TYPE(NEXTKBD, nextkbd_device, "nextkbd", "NeXT Keyboard")

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
	int_change_cb(*this),
	int_power_cb(*this),
	int_nmi_cb(*this),
	mousex(*this, "mousex"),
	mousey(*this, "mousey"),
	mousebtn(*this, "mousebtn")
{
}

void nextkbd_device::device_start()
{
	poll_timer = timer_alloc(FUNC(nextkbd_device::update_tick), this);

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
	save_item(NAME(prev_mousex));
	save_item(NAME(prev_mousey));
	save_item(NAME(prev_mousebtn));
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
	poll_timer->adjust(attotime::from_hz(200), 0, attotime::from_hz(200));
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
	logerror("status_snd_r %02x %s\n", ctrl_snd, machine().describe_context());
	return ctrl_snd;
}

uint8_t nextkbd_device::status_kms_r()
{
	logerror("status_kms_r %02x %s\n", ctrl_kms, machine().describe_context());
	return ctrl_kms;
}

uint8_t nextkbd_device::status_dma_r()
{
	logerror("status_dma_r %02x %s\n", ctrl_dma, machine().describe_context());
	return ctrl_dma;
}

uint8_t nextkbd_device::status_cmd_r()
{
	logerror("status_cmd_r %02x %s\n", ctrl_cmd, machine().describe_context());
	return ctrl_cmd;
}

uint32_t nextkbd_device::cdata_r(offs_t offset, uint32_t mem_mask)
{
	logerror("cdata_r %08x @ %08x %s\n", cdata, mem_mask, machine().describe_context());
	return cdata;
}

uint32_t nextkbd_device::kmdata_r(offs_t offset, uint32_t mem_mask)
{
	uint8_t old = ctrl_kms;
	ctrl_kms &= ~(C_KBD_INTERRUPT|C_KBD_DATA);
	if(old & C_KBD_INTERRUPT)
		int_change_cb(false);
	logerror("kmdata_r %08x @ %08x %s\n", kmdata, mem_mask, machine().describe_context());
	return kmdata;
}

void nextkbd_device::ctrl_snd_w(uint8_t data)
{
	uint8_t old = ctrl_snd;
	ctrl_snd = (ctrl_snd & ~C_SOUND_WMASK) | (data & C_SOUND_WMASK);
	uint8_t diff = old ^ ctrl_snd;

	logerror("ctrl_snd_w %02x | %02x %s\n", ctrl_snd, diff, machine().describe_context());
}

void nextkbd_device::ctrl_kms_w(uint8_t data)
{
	uint8_t old = ctrl_kms;
	ctrl_kms = (ctrl_kms & ~C_KMS_WMASK) | (data & C_KMS_WMASK);
	uint8_t diff = old ^ ctrl_kms;

	logerror("ctrl_kms_w %02x | %02x %s\n", ctrl_kms, diff, machine().describe_context());
}

void nextkbd_device::ctrl_dma_w(uint8_t data)
{
	uint8_t old = ctrl_dma;
	ctrl_dma = (ctrl_dma & ~C_WMASK) | (data & C_WMASK);
	uint8_t diff = old ^ ctrl_dma;

	logerror("ctrl_dma_w %02x | %02x %s\n", ctrl_dma, diff, machine().describe_context());
}

void nextkbd_device::ctrl_cmd_w(uint8_t data)
{
	ctrl_cmd = data;
	logerror("ctrl_cmd_w %02x %s\n", ctrl_cmd, machine().describe_context());
}

void nextkbd_device::cdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&cdata);
	logerror("cdata_w %08x @ %08x %s\n", data, mem_mask, machine().describe_context());
	handle_command();
}

void nextkbd_device::kmdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("kmdata_w %08x @ %08x %s\n", data, mem_mask, machine().describe_context());
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
	logerror("Fifo command %08x?\n", cdata);
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
	switch(ctrl_cmd) {
	case 0xc5:
		handle_kbd_command();
		break;

	case 0xc6:
		handle_fifo_command();
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
