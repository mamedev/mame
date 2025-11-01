// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generated file, do not edit, run dsp563xx-make.py instead

#include "emu.h"
#include "dsp563xx.h"

void dsp563xx_device::execute_ipar(u16 kipar)
{
	switch(kipar) {
	case 0: { // -
		break;
		}
	case 1: { // 
		break;
		}
	case 2: { // abs a
		unhandled("abs a");
		break;
		}
	case 3: { // abs b
		unhandled("abs b");
		break;
		}
	case 4: { // adc x,a
		unhandled("adc x,a");
		break;
		}
	case 5: { // adc x,b
		unhandled("adc x,b");
		break;
		}
	case 6: { // adc y,a
		unhandled("adc y,a");
		break;
		}
	case 7: { // adc y,b
		unhandled("adc y,b");
		break;
		}
	case 8: { // add b,a
		unhandled("add b,a");
		break;
		}
	case 9: { // add b,b
		unhandled("add b,b");
		break;
		}
	case 10: { // add x,a
		unhandled("add x,a");
		break;
		}
	case 11: { // add x,b
		unhandled("add x,b");
		break;
		}
	case 12: { // add y,a
		unhandled("add y,a");
		break;
		}
	case 13: { // add y,b
		unhandled("add y,b");
		break;
		}
	case 14: { // add x0,a
		unhandled("add x0,a");
		break;
		}
	case 15: { // add x0,b
		unhandled("add x0,b");
		break;
		}
	case 16: { // add y0,a
		unhandled("add y0,a");
		break;
		}
	case 17: { // add y0,b
		unhandled("add y0,b");
		break;
		}
	case 18: { // add x1,a
		unhandled("add x1,a");
		break;
		}
	case 19: { // add x1,b
		unhandled("add x1,b");
		break;
		}
	case 20: { // add y1,a
		unhandled("add y1,a");
		break;
		}
	case 21: { // add y1,b
		unhandled("add y1,b");
		break;
		}
	case 22: { // addl b,a
		unhandled("addl b,a");
		break;
		}
	case 23: { // addl a,b
		unhandled("addl a,b");
		break;
		}
	case 24: { // addr b,a
		unhandled("addr b,a");
		break;
		}
	case 25: { // addr a,b
		unhandled("addr a,b");
		break;
		}
	case 26: { // and x0,a
		unhandled("and x0,a");
		break;
		}
	case 27: { // and x0,b
		unhandled("and x0,b");
		break;
		}
	case 28: { // and y0,a
		unhandled("and y0,a");
		break;
		}
	case 29: { // and y0,b
		unhandled("and y0,b");
		break;
		}
	case 30: { // and x1,a
		unhandled("and x1,a");
		break;
		}
	case 31: { // and x1,b
		unhandled("and x1,b");
		break;
		}
	case 32: { // and y1,a
		unhandled("and y1,a");
		break;
		}
	case 33: { // and y1,b
		unhandled("and y1,b");
		break;
		}
	case 34: { // asl a
		unhandled("asl a");
		break;
		}
	case 35: { // asl b
		unhandled("asl b");
		break;
		}
	case 36: { // asr a
		unhandled("asr a");
		break;
		}
	case 37: { // asr b
		unhandled("asr b");
		break;
		}
	case 38: { // clr a
		set_a(0);
		m_ccr = (m_ccr & ~(CCR_E|CCR_N|CCR_V)) | (CCR_U|CCR_Z);
		break;
		}
	case 39: { // clr b
		set_b(0);
		m_ccr = (m_ccr & ~(CCR_E|CCR_N|CCR_V)) | (CCR_U|CCR_Z);
		break;
		}
	case 40: { // cmp b,a
		unhandled("cmp b,a");
		break;
		}
	case 41: { // cmp b,b
		unhandled("cmp b,b");
		break;
		}
	case 42: { // cmp x0,a
		unhandled("cmp x0,a");
		break;
		}
	case 43: { // cmp x0,b
		unhandled("cmp x0,b");
		break;
		}
	case 44: { // cmp y0,a
		unhandled("cmp y0,a");
		break;
		}
	case 45: { // cmp y0,b
		unhandled("cmp y0,b");
		break;
		}
	case 46: { // cmp x1,a
		unhandled("cmp x1,a");
		break;
		}
	case 47: { // cmp x1,b
		unhandled("cmp x1,b");
		break;
		}
	case 48: { // cmp y1,a
		unhandled("cmp y1,a");
		break;
		}
	case 49: { // cmp y1,b
		unhandled("cmp y1,b");
		break;
		}
	case 50: { // cmpm b,a
		unhandled("cmpm b,a");
		break;
		}
	case 51: { // cmpm b,b
		unhandled("cmpm b,b");
		break;
		}
	case 52: { // cmpm x0,a
		unhandled("cmpm x0,a");
		break;
		}
	case 53: { // cmpm x0,b
		unhandled("cmpm x0,b");
		break;
		}
	case 54: { // cmpm y0,a
		unhandled("cmpm y0,a");
		break;
		}
	case 55: { // cmpm y0,b
		unhandled("cmpm y0,b");
		break;
		}
	case 56: { // cmpm x1,a
		unhandled("cmpm x1,a");
		break;
		}
	case 57: { // cmpm x1,b
		unhandled("cmpm x1,b");
		break;
		}
	case 58: { // cmpm y1,a
		unhandled("cmpm y1,a");
		break;
		}
	case 59: { // cmpm y1,b
		unhandled("cmpm y1,b");
		break;
		}
	case 60: { // eor x0,a
		unhandled("eor x0,a");
		break;
		}
	case 61: { // eor x0,b
		unhandled("eor x0,b");
		break;
		}
	case 62: { // eor y0,a
		unhandled("eor y0,a");
		break;
		}
	case 63: { // eor y0,b
		unhandled("eor y0,b");
		break;
		}
	case 64: { // eor x1,a
		unhandled("eor x1,a");
		break;
		}
	case 65: { // eor x1,b
		unhandled("eor x1,b");
		break;
		}
	case 66: { // eor y1,a
		unhandled("eor y1,a");
		break;
		}
	case 67: { // eor y1,b
		unhandled("eor y1,b");
		break;
		}
	case 68: { // lsl a
		unhandled("lsl a");
		break;
		}
	case 69: { // lsl b
		unhandled("lsl b");
		break;
		}
	case 70: { // lsr a
		unhandled("lsr a");
		break;
		}
	case 71: { // lsr b
		unhandled("lsr b");
		break;
		}
	case 72: { // mac +x0,x0,a
		unhandled("mac +x0,x0,a");
		break;
		}
	case 73: { // mac -x0,x0,a
		unhandled("mac -x0,x0,a");
		break;
		}
	case 74: { // mac +x0,x0,b
		unhandled("mac +x0,x0,b");
		break;
		}
	case 75: { // mac -x0,x0,b
		unhandled("mac -x0,x0,b");
		break;
		}
	case 76: { // mac +y0,y0,a
		unhandled("mac +y0,y0,a");
		break;
		}
	case 77: { // mac -y0,y0,a
		unhandled("mac -y0,y0,a");
		break;
		}
	case 78: { // mac +y0,y0,b
		unhandled("mac +y0,y0,b");
		break;
		}
	case 79: { // mac -y0,y0,b
		unhandled("mac -y0,y0,b");
		break;
		}
	case 80: { // mac +x1,x0,a
		unhandled("mac +x1,x0,a");
		break;
		}
	case 81: { // mac -x1,x0,a
		unhandled("mac -x1,x0,a");
		break;
		}
	case 82: { // mac +x1,x0,b
		unhandled("mac +x1,x0,b");
		break;
		}
	case 83: { // mac -x1,x0,b
		unhandled("mac -x1,x0,b");
		break;
		}
	case 84: { // mac +y1,y0,a
		unhandled("mac +y1,y0,a");
		break;
		}
	case 85: { // mac -y1,y0,a
		unhandled("mac -y1,y0,a");
		break;
		}
	case 86: { // mac +y1,y0,b
		unhandled("mac +y1,y0,b");
		break;
		}
	case 87: { // mac -y1,y0,b
		unhandled("mac -y1,y0,b");
		break;
		}
	case 88: { // mac +x0,y1,a
		unhandled("mac +x0,y1,a");
		break;
		}
	case 89: { // mac -x0,y1,a
		unhandled("mac -x0,y1,a");
		break;
		}
	case 90: { // mac +x0,y1,b
		unhandled("mac +x0,y1,b");
		break;
		}
	case 91: { // mac -x0,y1,b
		unhandled("mac -x0,y1,b");
		break;
		}
	case 92: { // mac +y0,x0,a
		unhandled("mac +y0,x0,a");
		break;
		}
	case 93: { // mac -y0,x0,a
		unhandled("mac -y0,x0,a");
		break;
		}
	case 94: { // mac +y0,x0,b
		unhandled("mac +y0,x0,b");
		break;
		}
	case 95: { // mac -y0,x0,b
		unhandled("mac -y0,x0,b");
		break;
		}
	case 96: { // mac +x1,y0,a
		unhandled("mac +x1,y0,a");
		break;
		}
	case 97: { // mac -x1,y0,a
		unhandled("mac -x1,y0,a");
		break;
		}
	case 98: { // mac +x1,y0,b
		unhandled("mac +x1,y0,b");
		break;
		}
	case 99: { // mac -x1,y0,b
		unhandled("mac -x1,y0,b");
		break;
		}
	case 100: { // mac +y1,x1,a
		unhandled("mac +y1,x1,a");
		break;
		}
	case 101: { // mac -y1,x1,a
		unhandled("mac -y1,x1,a");
		break;
		}
	case 102: { // mac +y1,x1,b
		unhandled("mac +y1,x1,b");
		break;
		}
	case 103: { // mac -y1,x1,b
		unhandled("mac -y1,x1,b");
		break;
		}
	case 104: { // maccr +x0,x0,a
		unhandled("maccr +x0,x0,a");
		break;
		}
	case 105: { // maccr -x0,x0,a
		unhandled("maccr -x0,x0,a");
		break;
		}
	case 106: { // maccr +x0,x0,b
		unhandled("maccr +x0,x0,b");
		break;
		}
	case 107: { // maccr -x0,x0,b
		unhandled("maccr -x0,x0,b");
		break;
		}
	case 108: { // maccr +y0,y0,a
		unhandled("maccr +y0,y0,a");
		break;
		}
	case 109: { // maccr -y0,y0,a
		unhandled("maccr -y0,y0,a");
		break;
		}
	case 110: { // maccr +y0,y0,b
		unhandled("maccr +y0,y0,b");
		break;
		}
	case 111: { // maccr -y0,y0,b
		unhandled("maccr -y0,y0,b");
		break;
		}
	case 112: { // maccr +x1,x0,a
		unhandled("maccr +x1,x0,a");
		break;
		}
	case 113: { // maccr -x1,x0,a
		unhandled("maccr -x1,x0,a");
		break;
		}
	case 114: { // maccr +x1,x0,b
		unhandled("maccr +x1,x0,b");
		break;
		}
	case 115: { // maccr -x1,x0,b
		unhandled("maccr -x1,x0,b");
		break;
		}
	case 116: { // maccr +y1,y0,a
		unhandled("maccr +y1,y0,a");
		break;
		}
	case 117: { // maccr -y1,y0,a
		unhandled("maccr -y1,y0,a");
		break;
		}
	case 118: { // maccr +y1,y0,b
		unhandled("maccr +y1,y0,b");
		break;
		}
	case 119: { // maccr -y1,y0,b
		unhandled("maccr -y1,y0,b");
		break;
		}
	case 120: { // maccr +x0,y1,a
		unhandled("maccr +x0,y1,a");
		break;
		}
	case 121: { // maccr -x0,y1,a
		unhandled("maccr -x0,y1,a");
		break;
		}
	case 122: { // maccr +x0,y1,b
		unhandled("maccr +x0,y1,b");
		break;
		}
	case 123: { // maccr -x0,y1,b
		unhandled("maccr -x0,y1,b");
		break;
		}
	case 124: { // maccr +y0,x0,a
		unhandled("maccr +y0,x0,a");
		break;
		}
	case 125: { // maccr -y0,x0,a
		unhandled("maccr -y0,x0,a");
		break;
		}
	case 126: { // maccr +y0,x0,b
		unhandled("maccr +y0,x0,b");
		break;
		}
	case 127: { // maccr -y0,x0,b
		unhandled("maccr -y0,x0,b");
		break;
		}
	case 128: { // maccr +x1,y0,a
		unhandled("maccr +x1,y0,a");
		break;
		}
	case 129: { // maccr -x1,y0,a
		unhandled("maccr -x1,y0,a");
		break;
		}
	case 130: { // maccr +x1,y0,b
		unhandled("maccr +x1,y0,b");
		break;
		}
	case 131: { // maccr -x1,y0,b
		unhandled("maccr -x1,y0,b");
		break;
		}
	case 132: { // maccr +y1,x1,a
		unhandled("maccr +y1,x1,a");
		break;
		}
	case 133: { // maccr -y1,x1,a
		unhandled("maccr -y1,x1,a");
		break;
		}
	case 134: { // maccr +y1,x1,b
		unhandled("maccr +y1,x1,b");
		break;
		}
	case 135: { // maccr -y1,x1,b
		unhandled("maccr -y1,x1,b");
		break;
		}
	case 136: { // max a,b
		unhandled("max a,b");
		break;
		}
	case 137: { // maxm a,b
		unhandled("maxm a,b");
		break;
		}
	case 138: { // mpy +x0,x0,a
		unhandled("mpy +x0,x0,a");
		break;
		}
	case 139: { // mpy -x0,x0,a
		unhandled("mpy -x0,x0,a");
		break;
		}
	case 140: { // mpy +x0,x0,b
		unhandled("mpy +x0,x0,b");
		break;
		}
	case 141: { // mpy -x0,x0,b
		unhandled("mpy -x0,x0,b");
		break;
		}
	case 142: { // mpy +y0,y0,a
		unhandled("mpy +y0,y0,a");
		break;
		}
	case 143: { // mpy -y0,y0,a
		unhandled("mpy -y0,y0,a");
		break;
		}
	case 144: { // mpy +y0,y0,b
		unhandled("mpy +y0,y0,b");
		break;
		}
	case 145: { // mpy -y0,y0,b
		unhandled("mpy -y0,y0,b");
		break;
		}
	case 146: { // mpy +x1,x0,a
		unhandled("mpy +x1,x0,a");
		break;
		}
	case 147: { // mpy -x1,x0,a
		unhandled("mpy -x1,x0,a");
		break;
		}
	case 148: { // mpy +x1,x0,b
		unhandled("mpy +x1,x0,b");
		break;
		}
	case 149: { // mpy -x1,x0,b
		unhandled("mpy -x1,x0,b");
		break;
		}
	case 150: { // mpy +y1,y0,a
		unhandled("mpy +y1,y0,a");
		break;
		}
	case 151: { // mpy -y1,y0,a
		unhandled("mpy -y1,y0,a");
		break;
		}
	case 152: { // mpy +y1,y0,b
		unhandled("mpy +y1,y0,b");
		break;
		}
	case 153: { // mpy -y1,y0,b
		unhandled("mpy -y1,y0,b");
		break;
		}
	case 154: { // mpy +x0,y1,a
		unhandled("mpy +x0,y1,a");
		break;
		}
	case 155: { // mpy -x0,y1,a
		unhandled("mpy -x0,y1,a");
		break;
		}
	case 156: { // mpy +x0,y1,b
		unhandled("mpy +x0,y1,b");
		break;
		}
	case 157: { // mpy -x0,y1,b
		unhandled("mpy -x0,y1,b");
		break;
		}
	case 158: { // mpy +y0,x0,a
		unhandled("mpy +y0,x0,a");
		break;
		}
	case 159: { // mpy -y0,x0,a
		unhandled("mpy -y0,x0,a");
		break;
		}
	case 160: { // mpy +y0,x0,b
		unhandled("mpy +y0,x0,b");
		break;
		}
	case 161: { // mpy -y0,x0,b
		unhandled("mpy -y0,x0,b");
		break;
		}
	case 162: { // mpy +x1,y0,a
		unhandled("mpy +x1,y0,a");
		break;
		}
	case 163: { // mpy -x1,y0,a
		unhandled("mpy -x1,y0,a");
		break;
		}
	case 164: { // mpy +x1,y0,b
		unhandled("mpy +x1,y0,b");
		break;
		}
	case 165: { // mpy -x1,y0,b
		unhandled("mpy -x1,y0,b");
		break;
		}
	case 166: { // mpy +y1,x1,a
		unhandled("mpy +y1,x1,a");
		break;
		}
	case 167: { // mpy -y1,x1,a
		unhandled("mpy -y1,x1,a");
		break;
		}
	case 168: { // mpy +y1,x1,b
		unhandled("mpy +y1,x1,b");
		break;
		}
	case 169: { // mpy -y1,x1,b
		unhandled("mpy -y1,x1,b");
		break;
		}
	case 170: { // mpyr +x0,x0,a
		unhandled("mpyr +x0,x0,a");
		break;
		}
	case 171: { // mpyr -x0,x0,a
		unhandled("mpyr -x0,x0,a");
		break;
		}
	case 172: { // mpyr +x0,x0,b
		unhandled("mpyr +x0,x0,b");
		break;
		}
	case 173: { // mpyr -x0,x0,b
		unhandled("mpyr -x0,x0,b");
		break;
		}
	case 174: { // mpyr +y0,y0,a
		unhandled("mpyr +y0,y0,a");
		break;
		}
	case 175: { // mpyr -y0,y0,a
		unhandled("mpyr -y0,y0,a");
		break;
		}
	case 176: { // mpyr +y0,y0,b
		unhandled("mpyr +y0,y0,b");
		break;
		}
	case 177: { // mpyr -y0,y0,b
		unhandled("mpyr -y0,y0,b");
		break;
		}
	case 178: { // mpyr +x1,x0,a
		unhandled("mpyr +x1,x0,a");
		break;
		}
	case 179: { // mpyr -x1,x0,a
		unhandled("mpyr -x1,x0,a");
		break;
		}
	case 180: { // mpyr +x1,x0,b
		unhandled("mpyr +x1,x0,b");
		break;
		}
	case 181: { // mpyr -x1,x0,b
		unhandled("mpyr -x1,x0,b");
		break;
		}
	case 182: { // mpyr +y1,y0,a
		unhandled("mpyr +y1,y0,a");
		break;
		}
	case 183: { // mpyr -y1,y0,a
		unhandled("mpyr -y1,y0,a");
		break;
		}
	case 184: { // mpyr +y1,y0,b
		unhandled("mpyr +y1,y0,b");
		break;
		}
	case 185: { // mpyr -y1,y0,b
		unhandled("mpyr -y1,y0,b");
		break;
		}
	case 186: { // mpyr +x0,y1,a
		unhandled("mpyr +x0,y1,a");
		break;
		}
	case 187: { // mpyr -x0,y1,a
		unhandled("mpyr -x0,y1,a");
		break;
		}
	case 188: { // mpyr +x0,y1,b
		unhandled("mpyr +x0,y1,b");
		break;
		}
	case 189: { // mpyr -x0,y1,b
		unhandled("mpyr -x0,y1,b");
		break;
		}
	case 190: { // mpyr +y0,x0,a
		unhandled("mpyr +y0,x0,a");
		break;
		}
	case 191: { // mpyr -y0,x0,a
		unhandled("mpyr -y0,x0,a");
		break;
		}
	case 192: { // mpyr +y0,x0,b
		unhandled("mpyr +y0,x0,b");
		break;
		}
	case 193: { // mpyr -y0,x0,b
		unhandled("mpyr -y0,x0,b");
		break;
		}
	case 194: { // mpyr +x1,y0,a
		unhandled("mpyr +x1,y0,a");
		break;
		}
	case 195: { // mpyr -x1,y0,a
		unhandled("mpyr -x1,y0,a");
		break;
		}
	case 196: { // mpyr +x1,y0,b
		unhandled("mpyr +x1,y0,b");
		break;
		}
	case 197: { // mpyr -x1,y0,b
		unhandled("mpyr -x1,y0,b");
		break;
		}
	case 198: { // mpyr +y1,x1,a
		unhandled("mpyr +y1,x1,a");
		break;
		}
	case 199: { // mpyr -y1,x1,a
		unhandled("mpyr -y1,x1,a");
		break;
		}
	case 200: { // mpyr +y1,x1,b
		unhandled("mpyr +y1,x1,b");
		break;
		}
	case 201: { // mpyr -y1,x1,b
		unhandled("mpyr -y1,x1,b");
		break;
		}
	case 202: { // neg a
		unhandled("neg a");
		break;
		}
	case 203: { // neg b
		unhandled("neg b");
		break;
		}
	case 204: { // not a
		unhandled("not a");
		break;
		}
	case 205: { // not b
		unhandled("not b");
		break;
		}
	case 206: { // or x0,a
		unhandled("or x0,a");
		break;
		}
	case 207: { // or x0,b
		unhandled("or x0,b");
		break;
		}
	case 208: { // or y0,a
		unhandled("or y0,a");
		break;
		}
	case 209: { // or y0,b
		unhandled("or y0,b");
		break;
		}
	case 210: { // or x1,a
		unhandled("or x1,a");
		break;
		}
	case 211: { // or x1,b
		unhandled("or x1,b");
		break;
		}
	case 212: { // or y1,a
		unhandled("or y1,a");
		break;
		}
	case 213: { // or y1,b
		unhandled("or y1,b");
		break;
		}
	case 214: { // rnd a
		unhandled("rnd a");
		break;
		}
	case 215: { // rnd b
		unhandled("rnd b");
		break;
		}
	case 216: { // rol a
		unhandled("rol a");
		break;
		}
	case 217: { // rol b
		unhandled("rol b");
		break;
		}
	case 218: { // ror a
		unhandled("ror a");
		break;
		}
	case 219: { // ror b
		unhandled("ror b");
		break;
		}
	case 220: { // sbc x,a
		unhandled("sbc x,a");
		break;
		}
	case 221: { // sbc x,b
		unhandled("sbc x,b");
		break;
		}
	case 222: { // sbc y,a
		unhandled("sbc y,a");
		break;
		}
	case 223: { // sbc y,b
		unhandled("sbc y,b");
		break;
		}
	case 224: { // sub b,a
		unhandled("sub b,a");
		break;
		}
	case 225: { // sub b,b
		unhandled("sub b,b");
		break;
		}
	case 226: { // sub x,a
		unhandled("sub x,a");
		break;
		}
	case 227: { // sub x,b
		unhandled("sub x,b");
		break;
		}
	case 228: { // sub y,a
		unhandled("sub y,a");
		break;
		}
	case 229: { // sub y,b
		unhandled("sub y,b");
		break;
		}
	case 230: { // sub x0,a
		unhandled("sub x0,a");
		break;
		}
	case 231: { // sub x0,b
		unhandled("sub x0,b");
		break;
		}
	case 232: { // sub y0,a
		unhandled("sub y0,a");
		break;
		}
	case 233: { // sub y0,b
		unhandled("sub y0,b");
		break;
		}
	case 234: { // sub x1,a
		unhandled("sub x1,a");
		break;
		}
	case 235: { // sub x1,b
		unhandled("sub x1,b");
		break;
		}
	case 236: { // sub y1,a
		unhandled("sub y1,a");
		break;
		}
	case 237: { // sub y1,b
		unhandled("sub y1,b");
		break;
		}
	case 238: { // subl b,a
		unhandled("subl b,a");
		break;
		}
	case 239: { // subl a,b
		unhandled("subl a,b");
		break;
		}
	case 240: { // subr b,a
		unhandled("subr b,a");
		break;
		}
	case 241: { // subr a,b
		unhandled("subr a,b");
		break;
		}
	case 242: { // tfr b,a
		unhandled("tfr b,a");
		break;
		}
	case 243: { // tfr b,b
		unhandled("tfr b,b");
		break;
		}
	case 244: { // tfr x0,a
		unhandled("tfr x0,a");
		break;
		}
	case 245: { // tfr x0,b
		unhandled("tfr x0,b");
		break;
		}
	case 246: { // tfr y0,a
		unhandled("tfr y0,a");
		break;
		}
	case 247: { // tfr y0,b
		unhandled("tfr y0,b");
		break;
		}
	case 248: { // tfr x1,a
		unhandled("tfr x1,a");
		break;
		}
	case 249: { // tfr x1,b
		unhandled("tfr x1,b");
		break;
		}
	case 250: { // tfr y1,a
		unhandled("tfr y1,a");
		break;
		}
	case 251: { // tfr y1,b
		unhandled("tfr y1,b");
		break;
		}
	case 252: { // tst a
		unhandled("tst a");
		break;
		}
	case 253: { // tst b
		unhandled("tst b");
		break;
		}
	}
}

void dsp563xx_device::execute_pre_move(u16 kmove, u32 opcode, u32 exv)
{
	switch(kmove) {
	case 0: { // -
		break;
		}
	case 1: { // 
		break;
		}
	case 2: { // #[i],x0
		break;
		}
	case 3: { // #[i],x1
		break;
		}
	case 4: { // #[i],y0
		break;
		}
	case 5: { // #[i],y1
		break;
		}
	case 6: { // #[i],a0
		break;
		}
	case 7: { // #[i],b0
		break;
		}
	case 8: { // #[i],a2
		break;
		}
	case 9: { // #[i],b2
		break;
		}
	case 10: { // #[i],a1
		break;
		}
	case 11: { // #[i],b1
		break;
		}
	case 12: { // #[i],a
		break;
		}
	case 13: { // #[i],b
		break;
		}
	case 14: { // #[i],r
		break;
		}
	case 15: { // #[i],n
		break;
		}
	case 16: { // x0,x0
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 17: { // x0,x1
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 18: { // x0,y0
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 19: { // x0,y1
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 20: { // x0,a0
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 21: { // x0,b0
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 22: { // x0,a2
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 23: { // x0,b2
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 24: { // x0,a1
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 25: { // x0,b1
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 26: { // x0,a
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 27: { // x0,b
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 28: { // x0,r
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 29: { // x0,n
		u32 s = get_x0();
		m_tmp1 = s;
		break;
		}
	case 30: { // x1,x0
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 31: { // x1,x1
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 32: { // x1,y0
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 33: { // x1,y1
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 34: { // x1,a0
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 35: { // x1,b0
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 36: { // x1,a2
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 37: { // x1,b2
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 38: { // x1,a1
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 39: { // x1,b1
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 40: { // x1,a
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 41: { // x1,b
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 42: { // x1,r
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 43: { // x1,n
		u32 s = get_x1();
		m_tmp1 = s;
		break;
		}
	case 44: { // y0,x0
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 45: { // y0,x1
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 46: { // y0,y0
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 47: { // y0,y1
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 48: { // y0,a0
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 49: { // y0,b0
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 50: { // y0,a2
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 51: { // y0,b2
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 52: { // y0,a1
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 53: { // y0,b1
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 54: { // y0,a
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 55: { // y0,b
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 56: { // y0,r
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 57: { // y0,n
		u32 s = get_y0();
		m_tmp1 = s;
		break;
		}
	case 58: { // y1,x0
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 59: { // y1,x1
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 60: { // y1,y0
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 61: { // y1,y1
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 62: { // y1,a0
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 63: { // y1,b0
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 64: { // y1,a2
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 65: { // y1,b2
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 66: { // y1,a1
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 67: { // y1,b1
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 68: { // y1,a
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 69: { // y1,b
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 70: { // y1,r
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 71: { // y1,n
		u32 s = get_y1();
		m_tmp1 = s;
		break;
		}
	case 72: { // a0,x0
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 73: { // a0,x1
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 74: { // a0,y0
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 75: { // a0,y1
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 76: { // a0,a0
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 77: { // a0,b0
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 78: { // a0,a2
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 79: { // a0,b2
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 80: { // a0,a1
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 81: { // a0,b1
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 82: { // a0,a
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 83: { // a0,b
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 84: { // a0,r
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 85: { // a0,n
		u32 s = get_a0();
		m_tmp1 = s;
		break;
		}
	case 86: { // b0,x0
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 87: { // b0,x1
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 88: { // b0,y0
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 89: { // b0,y1
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 90: { // b0,a0
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 91: { // b0,b0
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 92: { // b0,a2
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 93: { // b0,b2
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 94: { // b0,a1
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 95: { // b0,b1
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 96: { // b0,a
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 97: { // b0,b
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 98: { // b0,r
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 99: { // b0,n
		u32 s = get_b0();
		m_tmp1 = s;
		break;
		}
	case 100: { // a2,x0
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 101: { // a2,x1
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 102: { // a2,y0
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 103: { // a2,y1
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 104: { // a2,a0
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 105: { // a2,b0
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 106: { // a2,a2
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 107: { // a2,b2
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 108: { // a2,a1
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 109: { // a2,b1
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 110: { // a2,a
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 111: { // a2,b
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 112: { // a2,r
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 113: { // a2,n
		u32 s = get_a2();
		m_tmp1 = s;
		break;
		}
	case 114: { // b2,x0
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 115: { // b2,x1
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 116: { // b2,y0
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 117: { // b2,y1
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 118: { // b2,a0
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 119: { // b2,b0
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 120: { // b2,a2
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 121: { // b2,b2
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 122: { // b2,a1
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 123: { // b2,b1
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 124: { // b2,a
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 125: { // b2,b
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 126: { // b2,r
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 127: { // b2,n
		u32 s = get_b2();
		m_tmp1 = s;
		break;
		}
	case 128: { // a1,x0
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 129: { // a1,x1
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 130: { // a1,y0
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 131: { // a1,y1
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 132: { // a1,a0
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 133: { // a1,b0
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 134: { // a1,a2
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 135: { // a1,b2
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 136: { // a1,a1
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 137: { // a1,b1
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 138: { // a1,a
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 139: { // a1,b
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 140: { // a1,r
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 141: { // a1,n
		u32 s = get_a1();
		m_tmp1 = s;
		break;
		}
	case 142: { // b1,x0
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 143: { // b1,x1
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 144: { // b1,y0
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 145: { // b1,y1
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 146: { // b1,a0
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 147: { // b1,b0
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 148: { // b1,a2
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 149: { // b1,b2
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 150: { // b1,a1
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 151: { // b1,b1
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 152: { // b1,a
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 153: { // b1,b
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 154: { // b1,r
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 155: { // b1,n
		u32 s = get_b1();
		m_tmp1 = s;
		break;
		}
	case 156: { // a,x0
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 157: { // a,x1
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 158: { // a,y0
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 159: { // a,y1
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 160: { // a,a0
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 161: { // a,b0
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 162: { // a,a2
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 163: { // a,b2
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 164: { // a,a1
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 165: { // a,b1
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 166: { // a,a
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 167: { // a,b
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 168: { // a,r
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 169: { // a,n
		u64 s = get_a();
		m_tmp1 = s;
		break;
		}
	case 170: { // b,x0
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 171: { // b,x1
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 172: { // b,y0
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 173: { // b,y1
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 174: { // b,a0
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 175: { // b,b0
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 176: { // b,a2
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 177: { // b,b2
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 178: { // b,a1
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 179: { // b,b1
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 180: { // b,a
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 181: { // b,b
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 182: { // b,r
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 183: { // b,n
		u64 s = get_b();
		m_tmp1 = s;
		break;
		}
	case 184: { // r,x0
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 185: { // r,x1
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 186: { // r,y0
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 187: { // r,y1
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 188: { // r,a0
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 189: { // r,b0
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 190: { // r,a2
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 191: { // r,b2
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 192: { // r,a1
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 193: { // r,b1
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 194: { // r,a
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 195: { // r,b
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 196: { // r,r
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 197: { // r,n
		u32 s = get_r(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 198: { // n,x0
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 199: { // n,x1
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 200: { // n,y0
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 201: { // n,y1
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 202: { // n,a0
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 203: { // n,b0
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 204: { // n,a2
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 205: { // n,b2
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 206: { // n,a1
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 207: { // n,b1
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 208: { // n,a
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 209: { // n,b
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 210: { // n,r
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 211: { // n,n
		u32 s = get_n(BIT(opcode, 13, 5) & 7);
		m_tmp1 = s;
		break;
		}
	case 212: { // (r)-n
		unhandled("(r)-n");
		break;
		}
	case 213: { // (r)+n
		unhandled("(r)+n");
		break;
		}
	case 214: { // (r)-
		unhandled("(r)-");
		break;
		}
	case 215: { // (r)+
		unhandled("(r)+");
		break;
		}
	case 216: { // x:(r)-n,x0
		unhandled("x:(r)-n,x0");
		break;
		}
	case 217: { // x:(r)+n,x0
		unhandled("x:(r)+n,x0");
		break;
		}
	case 218: { // x:(r)-,x0
		unhandled("x:(r)-,x0");
		break;
		}
	case 219: { // x:(r)+,x0
		unhandled("x:(r)+,x0");
		break;
		}
	case 220: { // x:(r),x0
		unhandled("x:(r),x0");
		break;
		}
	case 221: { // x:(r+n),x0
		unhandled("x:(r+n),x0");
		break;
		}
	case 222: { // x:-(r),x0
		unhandled("x:-(r),x0");
		break;
		}
	case 223: { // x:(r)-n,x1
		unhandled("x:(r)-n,x1");
		break;
		}
	case 224: { // x:(r)+n,x1
		unhandled("x:(r)+n,x1");
		break;
		}
	case 225: { // x:(r)-,x1
		unhandled("x:(r)-,x1");
		break;
		}
	case 226: { // x:(r)+,x1
		unhandled("x:(r)+,x1");
		break;
		}
	case 227: { // x:(r),x1
		unhandled("x:(r),x1");
		break;
		}
	case 228: { // x:(r+n),x1
		unhandled("x:(r+n),x1");
		break;
		}
	case 229: { // x:-(r),x1
		unhandled("x:-(r),x1");
		break;
		}
	case 230: { // x:(r)-n,y0
		unhandled("x:(r)-n,y0");
		break;
		}
	case 231: { // x:(r)+n,y0
		unhandled("x:(r)+n,y0");
		break;
		}
	case 232: { // x:(r)-,y0
		unhandled("x:(r)-,y0");
		break;
		}
	case 233: { // x:(r)+,y0
		unhandled("x:(r)+,y0");
		break;
		}
	case 234: { // x:(r),y0
		unhandled("x:(r),y0");
		break;
		}
	case 235: { // x:(r+n),y0
		unhandled("x:(r+n),y0");
		break;
		}
	case 236: { // x:-(r),y0
		unhandled("x:-(r),y0");
		break;
		}
	case 237: { // x:(r)-n,y1
		unhandled("x:(r)-n,y1");
		break;
		}
	case 238: { // x:(r)+n,y1
		unhandled("x:(r)+n,y1");
		break;
		}
	case 239: { // x:(r)-,y1
		unhandled("x:(r)-,y1");
		break;
		}
	case 240: { // x:(r)+,y1
		unhandled("x:(r)+,y1");
		break;
		}
	case 241: { // x:(r),y1
		unhandled("x:(r),y1");
		break;
		}
	case 242: { // x:(r+n),y1
		unhandled("x:(r+n),y1");
		break;
		}
	case 243: { // x:-(r),y1
		unhandled("x:-(r),y1");
		break;
		}
	case 244: { // x:(r)-n,a0
		unhandled("x:(r)-n,a0");
		break;
		}
	case 245: { // x:(r)+n,a0
		unhandled("x:(r)+n,a0");
		break;
		}
	case 246: { // x:(r)-,a0
		unhandled("x:(r)-,a0");
		break;
		}
	case 247: { // x:(r)+,a0
		unhandled("x:(r)+,a0");
		break;
		}
	case 248: { // x:(r),a0
		unhandled("x:(r),a0");
		break;
		}
	case 249: { // x:(r+n),a0
		unhandled("x:(r+n),a0");
		break;
		}
	case 250: { // x:-(r),a0
		unhandled("x:-(r),a0");
		break;
		}
	case 251: { // x:(r)-n,b0
		unhandled("x:(r)-n,b0");
		break;
		}
	case 252: { // x:(r)+n,b0
		unhandled("x:(r)+n,b0");
		break;
		}
	case 253: { // x:(r)-,b0
		unhandled("x:(r)-,b0");
		break;
		}
	case 254: { // x:(r)+,b0
		unhandled("x:(r)+,b0");
		break;
		}
	case 255: { // x:(r),b0
		unhandled("x:(r),b0");
		break;
		}
	case 256: { // x:(r+n),b0
		unhandled("x:(r+n),b0");
		break;
		}
	case 257: { // x:-(r),b0
		unhandled("x:-(r),b0");
		break;
		}
	case 258: { // x:(r)-n,a2
		unhandled("x:(r)-n,a2");
		break;
		}
	case 259: { // x:(r)+n,a2
		unhandled("x:(r)+n,a2");
		break;
		}
	case 260: { // x:(r)-,a2
		unhandled("x:(r)-,a2");
		break;
		}
	case 261: { // x:(r)+,a2
		unhandled("x:(r)+,a2");
		break;
		}
	case 262: { // x:(r),a2
		unhandled("x:(r),a2");
		break;
		}
	case 263: { // x:(r+n),a2
		unhandled("x:(r+n),a2");
		break;
		}
	case 264: { // x:-(r),a2
		unhandled("x:-(r),a2");
		break;
		}
	case 265: { // x:(r)-n,b2
		unhandled("x:(r)-n,b2");
		break;
		}
	case 266: { // x:(r)+n,b2
		unhandled("x:(r)+n,b2");
		break;
		}
	case 267: { // x:(r)-,b2
		unhandled("x:(r)-,b2");
		break;
		}
	case 268: { // x:(r)+,b2
		unhandled("x:(r)+,b2");
		break;
		}
	case 269: { // x:(r),b2
		unhandled("x:(r),b2");
		break;
		}
	case 270: { // x:(r+n),b2
		unhandled("x:(r+n),b2");
		break;
		}
	case 271: { // x:-(r),b2
		unhandled("x:-(r),b2");
		break;
		}
	case 272: { // x:(r)-n,a1
		unhandled("x:(r)-n,a1");
		break;
		}
	case 273: { // x:(r)+n,a1
		unhandled("x:(r)+n,a1");
		break;
		}
	case 274: { // x:(r)-,a1
		unhandled("x:(r)-,a1");
		break;
		}
	case 275: { // x:(r)+,a1
		unhandled("x:(r)+,a1");
		break;
		}
	case 276: { // x:(r),a1
		unhandled("x:(r),a1");
		break;
		}
	case 277: { // x:(r+n),a1
		unhandled("x:(r+n),a1");
		break;
		}
	case 278: { // x:-(r),a1
		unhandled("x:-(r),a1");
		break;
		}
	case 279: { // x:(r)-n,b1
		unhandled("x:(r)-n,b1");
		break;
		}
	case 280: { // x:(r)+n,b1
		unhandled("x:(r)+n,b1");
		break;
		}
	case 281: { // x:(r)-,b1
		unhandled("x:(r)-,b1");
		break;
		}
	case 282: { // x:(r)+,b1
		unhandled("x:(r)+,b1");
		break;
		}
	case 283: { // x:(r),b1
		unhandled("x:(r),b1");
		break;
		}
	case 284: { // x:(r+n),b1
		unhandled("x:(r+n),b1");
		break;
		}
	case 285: { // x:-(r),b1
		unhandled("x:-(r),b1");
		break;
		}
	case 286: { // x:(r)-n,a
		unhandled("x:(r)-n,a");
		break;
		}
	case 287: { // x:(r)+n,a
		unhandled("x:(r)+n,a");
		break;
		}
	case 288: { // x:(r)-,a
		unhandled("x:(r)-,a");
		break;
		}
	case 289: { // x:(r)+,a
		unhandled("x:(r)+,a");
		break;
		}
	case 290: { // x:(r),a
		unhandled("x:(r),a");
		break;
		}
	case 291: { // x:(r+n),a
		unhandled("x:(r+n),a");
		break;
		}
	case 292: { // x:-(r),a
		unhandled("x:-(r),a");
		break;
		}
	case 293: { // x:(r)-n,b
		unhandled("x:(r)-n,b");
		break;
		}
	case 294: { // x:(r)+n,b
		unhandled("x:(r)+n,b");
		break;
		}
	case 295: { // x:(r)-,b
		unhandled("x:(r)-,b");
		break;
		}
	case 296: { // x:(r)+,b
		unhandled("x:(r)+,b");
		break;
		}
	case 297: { // x:(r),b
		unhandled("x:(r),b");
		break;
		}
	case 298: { // x:(r+n),b
		unhandled("x:(r+n),b");
		break;
		}
	case 299: { // x:-(r),b
		unhandled("x:-(r),b");
		break;
		}
	case 300: { // x:(r)-n,r
		unhandled("x:(r)-n,r");
		break;
		}
	case 301: { // x:(r)+n,r
		unhandled("x:(r)+n,r");
		break;
		}
	case 302: { // x:(r)-,r
		unhandled("x:(r)-,r");
		break;
		}
	case 303: { // x:(r)+,r
		unhandled("x:(r)+,r");
		break;
		}
	case 304: { // x:(r),r
		unhandled("x:(r),r");
		break;
		}
	case 305: { // x:(r+n),r
		unhandled("x:(r+n),r");
		break;
		}
	case 306: { // x:-(r),r
		unhandled("x:-(r),r");
		break;
		}
	case 307: { // x:(r)-n,n
		unhandled("x:(r)-n,n");
		break;
		}
	case 308: { // x:(r)+n,n
		unhandled("x:(r)+n,n");
		break;
		}
	case 309: { // x:(r)-,n
		unhandled("x:(r)-,n");
		break;
		}
	case 310: { // x:(r)+,n
		unhandled("x:(r)+,n");
		break;
		}
	case 311: { // x:(r),n
		unhandled("x:(r),n");
		break;
		}
	case 312: { // x:(r+n),n
		unhandled("x:(r+n),n");
		break;
		}
	case 313: { // x:-(r),n
		unhandled("x:-(r),n");
		break;
		}
	case 314: { // x0,x:(r)-n
		unhandled("x0,x:(r)-n");
		break;
		}
	case 315: { // x0,x:(r)+n
		unhandled("x0,x:(r)+n");
		break;
		}
	case 316: { // x0,x:(r)-
		unhandled("x0,x:(r)-");
		break;
		}
	case 317: { // x0,x:(r)+
		unhandled("x0,x:(r)+");
		break;
		}
	case 318: { // x0,x:(r)
		unhandled("x0,x:(r)");
		break;
		}
	case 319: { // x0,x:(r+n)
		unhandled("x0,x:(r+n)");
		break;
		}
	case 320: { // x0,x:-(r)
		unhandled("x0,x:-(r)");
		break;
		}
	case 321: { // x1,x:(r)-n
		unhandled("x1,x:(r)-n");
		break;
		}
	case 322: { // x1,x:(r)+n
		unhandled("x1,x:(r)+n");
		break;
		}
	case 323: { // x1,x:(r)-
		unhandled("x1,x:(r)-");
		break;
		}
	case 324: { // x1,x:(r)+
		unhandled("x1,x:(r)+");
		break;
		}
	case 325: { // x1,x:(r)
		unhandled("x1,x:(r)");
		break;
		}
	case 326: { // x1,x:(r+n)
		unhandled("x1,x:(r+n)");
		break;
		}
	case 327: { // x1,x:-(r)
		unhandled("x1,x:-(r)");
		break;
		}
	case 328: { // y0,x:(r)-n
		unhandled("y0,x:(r)-n");
		break;
		}
	case 329: { // y0,x:(r)+n
		unhandled("y0,x:(r)+n");
		break;
		}
	case 330: { // y0,x:(r)-
		unhandled("y0,x:(r)-");
		break;
		}
	case 331: { // y0,x:(r)+
		unhandled("y0,x:(r)+");
		break;
		}
	case 332: { // y0,x:(r)
		unhandled("y0,x:(r)");
		break;
		}
	case 333: { // y0,x:(r+n)
		unhandled("y0,x:(r+n)");
		break;
		}
	case 334: { // y0,x:-(r)
		unhandled("y0,x:-(r)");
		break;
		}
	case 335: { // y1,x:(r)-n
		unhandled("y1,x:(r)-n");
		break;
		}
	case 336: { // y1,x:(r)+n
		unhandled("y1,x:(r)+n");
		break;
		}
	case 337: { // y1,x:(r)-
		unhandled("y1,x:(r)-");
		break;
		}
	case 338: { // y1,x:(r)+
		unhandled("y1,x:(r)+");
		break;
		}
	case 339: { // y1,x:(r)
		unhandled("y1,x:(r)");
		break;
		}
	case 340: { // y1,x:(r+n)
		unhandled("y1,x:(r+n)");
		break;
		}
	case 341: { // y1,x:-(r)
		unhandled("y1,x:-(r)");
		break;
		}
	case 342: { // a0,x:(r)-n
		unhandled("a0,x:(r)-n");
		break;
		}
	case 343: { // a0,x:(r)+n
		unhandled("a0,x:(r)+n");
		break;
		}
	case 344: { // a0,x:(r)-
		unhandled("a0,x:(r)-");
		break;
		}
	case 345: { // a0,x:(r)+
		unhandled("a0,x:(r)+");
		break;
		}
	case 346: { // a0,x:(r)
		unhandled("a0,x:(r)");
		break;
		}
	case 347: { // a0,x:(r+n)
		unhandled("a0,x:(r+n)");
		break;
		}
	case 348: { // a0,x:-(r)
		unhandled("a0,x:-(r)");
		break;
		}
	case 349: { // b0,x:(r)-n
		unhandled("b0,x:(r)-n");
		break;
		}
	case 350: { // b0,x:(r)+n
		unhandled("b0,x:(r)+n");
		break;
		}
	case 351: { // b0,x:(r)-
		unhandled("b0,x:(r)-");
		break;
		}
	case 352: { // b0,x:(r)+
		unhandled("b0,x:(r)+");
		break;
		}
	case 353: { // b0,x:(r)
		unhandled("b0,x:(r)");
		break;
		}
	case 354: { // b0,x:(r+n)
		unhandled("b0,x:(r+n)");
		break;
		}
	case 355: { // b0,x:-(r)
		unhandled("b0,x:-(r)");
		break;
		}
	case 356: { // a2,x:(r)-n
		unhandled("a2,x:(r)-n");
		break;
		}
	case 357: { // a2,x:(r)+n
		unhandled("a2,x:(r)+n");
		break;
		}
	case 358: { // a2,x:(r)-
		unhandled("a2,x:(r)-");
		break;
		}
	case 359: { // a2,x:(r)+
		unhandled("a2,x:(r)+");
		break;
		}
	case 360: { // a2,x:(r)
		unhandled("a2,x:(r)");
		break;
		}
	case 361: { // a2,x:(r+n)
		unhandled("a2,x:(r+n)");
		break;
		}
	case 362: { // a2,x:-(r)
		unhandled("a2,x:-(r)");
		break;
		}
	case 363: { // b2,x:(r)-n
		unhandled("b2,x:(r)-n");
		break;
		}
	case 364: { // b2,x:(r)+n
		unhandled("b2,x:(r)+n");
		break;
		}
	case 365: { // b2,x:(r)-
		unhandled("b2,x:(r)-");
		break;
		}
	case 366: { // b2,x:(r)+
		unhandled("b2,x:(r)+");
		break;
		}
	case 367: { // b2,x:(r)
		unhandled("b2,x:(r)");
		break;
		}
	case 368: { // b2,x:(r+n)
		unhandled("b2,x:(r+n)");
		break;
		}
	case 369: { // b2,x:-(r)
		unhandled("b2,x:-(r)");
		break;
		}
	case 370: { // a1,x:(r)-n
		unhandled("a1,x:(r)-n");
		break;
		}
	case 371: { // a1,x:(r)+n
		unhandled("a1,x:(r)+n");
		break;
		}
	case 372: { // a1,x:(r)-
		unhandled("a1,x:(r)-");
		break;
		}
	case 373: { // a1,x:(r)+
		unhandled("a1,x:(r)+");
		break;
		}
	case 374: { // a1,x:(r)
		unhandled("a1,x:(r)");
		break;
		}
	case 375: { // a1,x:(r+n)
		unhandled("a1,x:(r+n)");
		break;
		}
	case 376: { // a1,x:-(r)
		unhandled("a1,x:-(r)");
		break;
		}
	case 377: { // b1,x:(r)-n
		unhandled("b1,x:(r)-n");
		break;
		}
	case 378: { // b1,x:(r)+n
		unhandled("b1,x:(r)+n");
		break;
		}
	case 379: { // b1,x:(r)-
		unhandled("b1,x:(r)-");
		break;
		}
	case 380: { // b1,x:(r)+
		unhandled("b1,x:(r)+");
		break;
		}
	case 381: { // b1,x:(r)
		unhandled("b1,x:(r)");
		break;
		}
	case 382: { // b1,x:(r+n)
		unhandled("b1,x:(r+n)");
		break;
		}
	case 383: { // b1,x:-(r)
		unhandled("b1,x:-(r)");
		break;
		}
	case 384: { // a,x:(r)-n
		unhandled("a,x:(r)-n");
		break;
		}
	case 385: { // a,x:(r)+n
		unhandled("a,x:(r)+n");
		break;
		}
	case 386: { // a,x:(r)-
		unhandled("a,x:(r)-");
		break;
		}
	case 387: { // a,x:(r)+
		unhandled("a,x:(r)+");
		break;
		}
	case 388: { // a,x:(r)
		unhandled("a,x:(r)");
		break;
		}
	case 389: { // a,x:(r+n)
		unhandled("a,x:(r+n)");
		break;
		}
	case 390: { // a,x:-(r)
		unhandled("a,x:-(r)");
		break;
		}
	case 391: { // b,x:(r)-n
		unhandled("b,x:(r)-n");
		break;
		}
	case 392: { // b,x:(r)+n
		unhandled("b,x:(r)+n");
		break;
		}
	case 393: { // b,x:(r)-
		unhandled("b,x:(r)-");
		break;
		}
	case 394: { // b,x:(r)+
		unhandled("b,x:(r)+");
		break;
		}
	case 395: { // b,x:(r)
		unhandled("b,x:(r)");
		break;
		}
	case 396: { // b,x:(r+n)
		unhandled("b,x:(r+n)");
		break;
		}
	case 397: { // b,x:-(r)
		unhandled("b,x:-(r)");
		break;
		}
	case 398: { // r,x:(r)-n
		unhandled("r,x:(r)-n");
		break;
		}
	case 399: { // r,x:(r)+n
		unhandled("r,x:(r)+n");
		break;
		}
	case 400: { // r,x:(r)-
		unhandled("r,x:(r)-");
		break;
		}
	case 401: { // r,x:(r)+
		unhandled("r,x:(r)+");
		break;
		}
	case 402: { // r,x:(r)
		unhandled("r,x:(r)");
		break;
		}
	case 403: { // r,x:(r+n)
		unhandled("r,x:(r+n)");
		break;
		}
	case 404: { // r,x:-(r)
		unhandled("r,x:-(r)");
		break;
		}
	case 405: { // n,x:(r)-n
		unhandled("n,x:(r)-n");
		break;
		}
	case 406: { // n,x:(r)+n
		unhandled("n,x:(r)+n");
		break;
		}
	case 407: { // n,x:(r)-
		unhandled("n,x:(r)-");
		break;
		}
	case 408: { // n,x:(r)+
		unhandled("n,x:(r)+");
		break;
		}
	case 409: { // n,x:(r)
		unhandled("n,x:(r)");
		break;
		}
	case 410: { // n,x:(r+n)
		unhandled("n,x:(r+n)");
		break;
		}
	case 411: { // n,x:-(r)
		unhandled("n,x:-(r)");
		break;
		}
	case 412: { // [abs],x0
		unhandled("[abs],x0");
		break;
		}
	case 413: { // [abs],x1
		unhandled("[abs],x1");
		break;
		}
	case 414: { // [abs],y0
		unhandled("[abs],y0");
		break;
		}
	case 415: { // [abs],y1
		unhandled("[abs],y1");
		break;
		}
	case 416: { // [abs],a0
		unhandled("[abs],a0");
		break;
		}
	case 417: { // [abs],b0
		unhandled("[abs],b0");
		break;
		}
	case 418: { // [abs],a2
		unhandled("[abs],a2");
		break;
		}
	case 419: { // [abs],b2
		unhandled("[abs],b2");
		break;
		}
	case 420: { // [abs],a1
		unhandled("[abs],a1");
		break;
		}
	case 421: { // [abs],b1
		unhandled("[abs],b1");
		break;
		}
	case 422: { // [abs],a
		unhandled("[abs],a");
		break;
		}
	case 423: { // [abs],b
		unhandled("[abs],b");
		break;
		}
	case 424: { // [abs],r
		unhandled("[abs],r");
		break;
		}
	case 425: { // [abs],n
		unhandled("[abs],n");
		break;
		}
	case 426: { // #[i],x0
		unhandled("#[i],x0");
		break;
		}
	case 427: { // #[i],x1
		unhandled("#[i],x1");
		break;
		}
	case 428: { // #[i],y0
		unhandled("#[i],y0");
		break;
		}
	case 429: { // #[i],y1
		unhandled("#[i],y1");
		break;
		}
	case 430: { // #[i],a0
		unhandled("#[i],a0");
		break;
		}
	case 431: { // #[i],b0
		unhandled("#[i],b0");
		break;
		}
	case 432: { // #[i],a2
		unhandled("#[i],a2");
		break;
		}
	case 433: { // #[i],b2
		unhandled("#[i],b2");
		break;
		}
	case 434: { // #[i],a1
		unhandled("#[i],a1");
		break;
		}
	case 435: { // #[i],b1
		unhandled("#[i],b1");
		break;
		}
	case 436: { // #[i],a
		unhandled("#[i],a");
		break;
		}
	case 437: { // #[i],b
		unhandled("#[i],b");
		break;
		}
	case 438: { // #[i],r
		unhandled("#[i],r");
		break;
		}
	case 439: { // #[i],n
		unhandled("#[i],n");
		break;
		}
	case 440: { // x:[aa],x0
		unhandled("x:[aa],x0");
		break;
		}
	case 441: { // x:[aa],x1
		unhandled("x:[aa],x1");
		break;
		}
	case 442: { // x:[aa],y0
		unhandled("x:[aa],y0");
		break;
		}
	case 443: { // x:[aa],y1
		unhandled("x:[aa],y1");
		break;
		}
	case 444: { // x:[aa],a0
		unhandled("x:[aa],a0");
		break;
		}
	case 445: { // x:[aa],b0
		unhandled("x:[aa],b0");
		break;
		}
	case 446: { // x:[aa],a2
		unhandled("x:[aa],a2");
		break;
		}
	case 447: { // x:[aa],b2
		unhandled("x:[aa],b2");
		break;
		}
	case 448: { // x:[aa],a1
		unhandled("x:[aa],a1");
		break;
		}
	case 449: { // x:[aa],b1
		unhandled("x:[aa],b1");
		break;
		}
	case 450: { // x:[aa],a
		unhandled("x:[aa],a");
		break;
		}
	case 451: { // x:[aa],b
		unhandled("x:[aa],b");
		break;
		}
	case 452: { // x:[aa],r
		unhandled("x:[aa],r");
		break;
		}
	case 453: { // x:[aa],n
		unhandled("x:[aa],n");
		break;
		}
	case 454: { // x0,x:[aa]
		unhandled("x0,x:[aa]");
		break;
		}
	case 455: { // x1,x:[aa]
		unhandled("x1,x:[aa]");
		break;
		}
	case 456: { // y0,x:[aa]
		unhandled("y0,x:[aa]");
		break;
		}
	case 457: { // y1,x:[aa]
		unhandled("y1,x:[aa]");
		break;
		}
	case 458: { // a0,x:[aa]
		unhandled("a0,x:[aa]");
		break;
		}
	case 459: { // b0,x:[aa]
		unhandled("b0,x:[aa]");
		break;
		}
	case 460: { // a2,x:[aa]
		unhandled("a2,x:[aa]");
		break;
		}
	case 461: { // b2,x:[aa]
		unhandled("b2,x:[aa]");
		break;
		}
	case 462: { // a1,x:[aa]
		unhandled("a1,x:[aa]");
		break;
		}
	case 463: { // b1,x:[aa]
		unhandled("b1,x:[aa]");
		break;
		}
	case 464: { // a,x:[aa]
		unhandled("a,x:[aa]");
		break;
		}
	case 465: { // b,x:[aa]
		unhandled("b,x:[aa]");
		break;
		}
	case 466: { // r,x:[aa]
		unhandled("r,x:[aa]");
		break;
		}
	case 467: { // n,x:[aa]
		unhandled("n,x:[aa]");
		break;
		}
	case 468: { // x:(r)-n,x0 a,y0
		unhandled("x:(r)-n,x0 a,y0");
		break;
		}
	case 469: { // x:(r)+n,x0 a,y0
		unhandled("x:(r)+n,x0 a,y0");
		break;
		}
	case 470: { // x:(r)-,x0 a,y0
		unhandled("x:(r)-,x0 a,y0");
		break;
		}
	case 471: { // x:(r)+,x0 a,y0
		unhandled("x:(r)+,x0 a,y0");
		break;
		}
	case 472: { // x:(r),x0 a,y0
		unhandled("x:(r),x0 a,y0");
		break;
		}
	case 473: { // x:(r+n),x0 a,y0
		unhandled("x:(r+n),x0 a,y0");
		break;
		}
	case 474: { // x:-(r),x0 a,y0
		unhandled("x:-(r),x0 a,y0");
		break;
		}
	case 475: { // x:(r)-n,x0 a,y1
		unhandled("x:(r)-n,x0 a,y1");
		break;
		}
	case 476: { // x:(r)+n,x0 a,y1
		unhandled("x:(r)+n,x0 a,y1");
		break;
		}
	case 477: { // x:(r)-,x0 a,y1
		unhandled("x:(r)-,x0 a,y1");
		break;
		}
	case 478: { // x:(r)+,x0 a,y1
		unhandled("x:(r)+,x0 a,y1");
		break;
		}
	case 479: { // x:(r),x0 a,y1
		unhandled("x:(r),x0 a,y1");
		break;
		}
	case 480: { // x:(r+n),x0 a,y1
		unhandled("x:(r+n),x0 a,y1");
		break;
		}
	case 481: { // x:-(r),x0 a,y1
		unhandled("x:-(r),x0 a,y1");
		break;
		}
	case 482: { // x:(r)-n,x0 b,y0
		unhandled("x:(r)-n,x0 b,y0");
		break;
		}
	case 483: { // x:(r)+n,x0 b,y0
		unhandled("x:(r)+n,x0 b,y0");
		break;
		}
	case 484: { // x:(r)-,x0 b,y0
		unhandled("x:(r)-,x0 b,y0");
		break;
		}
	case 485: { // x:(r)+,x0 b,y0
		unhandled("x:(r)+,x0 b,y0");
		break;
		}
	case 486: { // x:(r),x0 b,y0
		unhandled("x:(r),x0 b,y0");
		break;
		}
	case 487: { // x:(r+n),x0 b,y0
		unhandled("x:(r+n),x0 b,y0");
		break;
		}
	case 488: { // x:-(r),x0 b,y0
		unhandled("x:-(r),x0 b,y0");
		break;
		}
	case 489: { // x:(r)-n,x0 b,y1
		unhandled("x:(r)-n,x0 b,y1");
		break;
		}
	case 490: { // x:(r)+n,x0 b,y1
		unhandled("x:(r)+n,x0 b,y1");
		break;
		}
	case 491: { // x:(r)-,x0 b,y1
		unhandled("x:(r)-,x0 b,y1");
		break;
		}
	case 492: { // x:(r)+,x0 b,y1
		unhandled("x:(r)+,x0 b,y1");
		break;
		}
	case 493: { // x:(r),x0 b,y1
		unhandled("x:(r),x0 b,y1");
		break;
		}
	case 494: { // x:(r+n),x0 b,y1
		unhandled("x:(r+n),x0 b,y1");
		break;
		}
	case 495: { // x:-(r),x0 b,y1
		unhandled("x:-(r),x0 b,y1");
		break;
		}
	case 496: { // x:(r)-n,x1 a,y0
		unhandled("x:(r)-n,x1 a,y0");
		break;
		}
	case 497: { // x:(r)+n,x1 a,y0
		unhandled("x:(r)+n,x1 a,y0");
		break;
		}
	case 498: { // x:(r)-,x1 a,y0
		unhandled("x:(r)-,x1 a,y0");
		break;
		}
	case 499: { // x:(r)+,x1 a,y0
		unhandled("x:(r)+,x1 a,y0");
		break;
		}
	case 500: { // x:(r),x1 a,y0
		unhandled("x:(r),x1 a,y0");
		break;
		}
	case 501: { // x:(r+n),x1 a,y0
		unhandled("x:(r+n),x1 a,y0");
		break;
		}
	case 502: { // x:-(r),x1 a,y0
		unhandled("x:-(r),x1 a,y0");
		break;
		}
	case 503: { // x:(r)-n,x1 a,y1
		unhandled("x:(r)-n,x1 a,y1");
		break;
		}
	case 504: { // x:(r)+n,x1 a,y1
		unhandled("x:(r)+n,x1 a,y1");
		break;
		}
	case 505: { // x:(r)-,x1 a,y1
		unhandled("x:(r)-,x1 a,y1");
		break;
		}
	case 506: { // x:(r)+,x1 a,y1
		unhandled("x:(r)+,x1 a,y1");
		break;
		}
	case 507: { // x:(r),x1 a,y1
		unhandled("x:(r),x1 a,y1");
		break;
		}
	case 508: { // x:(r+n),x1 a,y1
		unhandled("x:(r+n),x1 a,y1");
		break;
		}
	case 509: { // x:-(r),x1 a,y1
		unhandled("x:-(r),x1 a,y1");
		break;
		}
	case 510: { // x:(r)-n,x1 b,y0
		unhandled("x:(r)-n,x1 b,y0");
		break;
		}
	case 511: { // x:(r)+n,x1 b,y0
		unhandled("x:(r)+n,x1 b,y0");
		break;
		}
	case 512: { // x:(r)-,x1 b,y0
		unhandled("x:(r)-,x1 b,y0");
		break;
		}
	case 513: { // x:(r)+,x1 b,y0
		unhandled("x:(r)+,x1 b,y0");
		break;
		}
	case 514: { // x:(r),x1 b,y0
		unhandled("x:(r),x1 b,y0");
		break;
		}
	case 515: { // x:(r+n),x1 b,y0
		unhandled("x:(r+n),x1 b,y0");
		break;
		}
	case 516: { // x:-(r),x1 b,y0
		unhandled("x:-(r),x1 b,y0");
		break;
		}
	case 517: { // x:(r)-n,x1 b,y1
		unhandled("x:(r)-n,x1 b,y1");
		break;
		}
	case 518: { // x:(r)+n,x1 b,y1
		unhandled("x:(r)+n,x1 b,y1");
		break;
		}
	case 519: { // x:(r)-,x1 b,y1
		unhandled("x:(r)-,x1 b,y1");
		break;
		}
	case 520: { // x:(r)+,x1 b,y1
		unhandled("x:(r)+,x1 b,y1");
		break;
		}
	case 521: { // x:(r),x1 b,y1
		unhandled("x:(r),x1 b,y1");
		break;
		}
	case 522: { // x:(r+n),x1 b,y1
		unhandled("x:(r+n),x1 b,y1");
		break;
		}
	case 523: { // x:-(r),x1 b,y1
		unhandled("x:-(r),x1 b,y1");
		break;
		}
	case 524: { // x:(r)-n,a a,y0
		unhandled("x:(r)-n,a a,y0");
		break;
		}
	case 525: { // x:(r)+n,a a,y0
		unhandled("x:(r)+n,a a,y0");
		break;
		}
	case 526: { // x:(r)-,a a,y0
		unhandled("x:(r)-,a a,y0");
		break;
		}
	case 527: { // x:(r)+,a a,y0
		unhandled("x:(r)+,a a,y0");
		break;
		}
	case 528: { // x:(r),a a,y0
		unhandled("x:(r),a a,y0");
		break;
		}
	case 529: { // x:(r+n),a a,y0
		unhandled("x:(r+n),a a,y0");
		break;
		}
	case 530: { // x:-(r),a a,y0
		unhandled("x:-(r),a a,y0");
		break;
		}
	case 531: { // x:(r)-n,a a,y1
		unhandled("x:(r)-n,a a,y1");
		break;
		}
	case 532: { // x:(r)+n,a a,y1
		unhandled("x:(r)+n,a a,y1");
		break;
		}
	case 533: { // x:(r)-,a a,y1
		unhandled("x:(r)-,a a,y1");
		break;
		}
	case 534: { // x:(r)+,a a,y1
		unhandled("x:(r)+,a a,y1");
		break;
		}
	case 535: { // x:(r),a a,y1
		unhandled("x:(r),a a,y1");
		break;
		}
	case 536: { // x:(r+n),a a,y1
		unhandled("x:(r+n),a a,y1");
		break;
		}
	case 537: { // x:-(r),a a,y1
		unhandled("x:-(r),a a,y1");
		break;
		}
	case 538: { // x:(r)-n,a b,y0
		unhandled("x:(r)-n,a b,y0");
		break;
		}
	case 539: { // x:(r)+n,a b,y0
		unhandled("x:(r)+n,a b,y0");
		break;
		}
	case 540: { // x:(r)-,a b,y0
		unhandled("x:(r)-,a b,y0");
		break;
		}
	case 541: { // x:(r)+,a b,y0
		unhandled("x:(r)+,a b,y0");
		break;
		}
	case 542: { // x:(r),a b,y0
		unhandled("x:(r),a b,y0");
		break;
		}
	case 543: { // x:(r+n),a b,y0
		unhandled("x:(r+n),a b,y0");
		break;
		}
	case 544: { // x:-(r),a b,y0
		unhandled("x:-(r),a b,y0");
		break;
		}
	case 545: { // x:(r)-n,a b,y1
		unhandled("x:(r)-n,a b,y1");
		break;
		}
	case 546: { // x:(r)+n,a b,y1
		unhandled("x:(r)+n,a b,y1");
		break;
		}
	case 547: { // x:(r)-,a b,y1
		unhandled("x:(r)-,a b,y1");
		break;
		}
	case 548: { // x:(r)+,a b,y1
		unhandled("x:(r)+,a b,y1");
		break;
		}
	case 549: { // x:(r),a b,y1
		unhandled("x:(r),a b,y1");
		break;
		}
	case 550: { // x:(r+n),a b,y1
		unhandled("x:(r+n),a b,y1");
		break;
		}
	case 551: { // x:-(r),a b,y1
		unhandled("x:-(r),a b,y1");
		break;
		}
	case 552: { // x:(r)-n,b a,y0
		unhandled("x:(r)-n,b a,y0");
		break;
		}
	case 553: { // x:(r)+n,b a,y0
		unhandled("x:(r)+n,b a,y0");
		break;
		}
	case 554: { // x:(r)-,b a,y0
		unhandled("x:(r)-,b a,y0");
		break;
		}
	case 555: { // x:(r)+,b a,y0
		unhandled("x:(r)+,b a,y0");
		break;
		}
	case 556: { // x:(r),b a,y0
		unhandled("x:(r),b a,y0");
		break;
		}
	case 557: { // x:(r+n),b a,y0
		unhandled("x:(r+n),b a,y0");
		break;
		}
	case 558: { // x:-(r),b a,y0
		unhandled("x:-(r),b a,y0");
		break;
		}
	case 559: { // x:(r)-n,b a,y1
		unhandled("x:(r)-n,b a,y1");
		break;
		}
	case 560: { // x:(r)+n,b a,y1
		unhandled("x:(r)+n,b a,y1");
		break;
		}
	case 561: { // x:(r)-,b a,y1
		unhandled("x:(r)-,b a,y1");
		break;
		}
	case 562: { // x:(r)+,b a,y1
		unhandled("x:(r)+,b a,y1");
		break;
		}
	case 563: { // x:(r),b a,y1
		unhandled("x:(r),b a,y1");
		break;
		}
	case 564: { // x:(r+n),b a,y1
		unhandled("x:(r+n),b a,y1");
		break;
		}
	case 565: { // x:-(r),b a,y1
		unhandled("x:-(r),b a,y1");
		break;
		}
	case 566: { // x:(r)-n,b b,y0
		unhandled("x:(r)-n,b b,y0");
		break;
		}
	case 567: { // x:(r)+n,b b,y0
		unhandled("x:(r)+n,b b,y0");
		break;
		}
	case 568: { // x:(r)-,b b,y0
		unhandled("x:(r)-,b b,y0");
		break;
		}
	case 569: { // x:(r)+,b b,y0
		unhandled("x:(r)+,b b,y0");
		break;
		}
	case 570: { // x:(r),b b,y0
		unhandled("x:(r),b b,y0");
		break;
		}
	case 571: { // x:(r+n),b b,y0
		unhandled("x:(r+n),b b,y0");
		break;
		}
	case 572: { // x:-(r),b b,y0
		unhandled("x:-(r),b b,y0");
		break;
		}
	case 573: { // x:(r)-n,b b,y1
		unhandled("x:(r)-n,b b,y1");
		break;
		}
	case 574: { // x:(r)+n,b b,y1
		unhandled("x:(r)+n,b b,y1");
		break;
		}
	case 575: { // x:(r)-,b b,y1
		unhandled("x:(r)-,b b,y1");
		break;
		}
	case 576: { // x:(r)+,b b,y1
		unhandled("x:(r)+,b b,y1");
		break;
		}
	case 577: { // x:(r),b b,y1
		unhandled("x:(r),b b,y1");
		break;
		}
	case 578: { // x:(r+n),b b,y1
		unhandled("x:(r+n),b b,y1");
		break;
		}
	case 579: { // x:-(r),b b,y1
		unhandled("x:-(r),b b,y1");
		break;
		}
	case 580: { // x0,x:(r)-n a,y0
		unhandled("x0,x:(r)-n a,y0");
		break;
		}
	case 581: { // x0,x:(r)+n a,y0
		unhandled("x0,x:(r)+n a,y0");
		break;
		}
	case 582: { // x0,x:(r)- a,y0
		unhandled("x0,x:(r)- a,y0");
		break;
		}
	case 583: { // x0,x:(r)+ a,y0
		unhandled("x0,x:(r)+ a,y0");
		break;
		}
	case 584: { // x0,x:(r) a,y0
		unhandled("x0,x:(r) a,y0");
		break;
		}
	case 585: { // x0,x:(r+n) a,y0
		unhandled("x0,x:(r+n) a,y0");
		break;
		}
	case 586: { // x0,x:-(r) a,y0
		unhandled("x0,x:-(r) a,y0");
		break;
		}
	case 587: { // x0,x:(r)-n a,y1
		unhandled("x0,x:(r)-n a,y1");
		break;
		}
	case 588: { // x0,x:(r)+n a,y1
		unhandled("x0,x:(r)+n a,y1");
		break;
		}
	case 589: { // x0,x:(r)- a,y1
		unhandled("x0,x:(r)- a,y1");
		break;
		}
	case 590: { // x0,x:(r)+ a,y1
		unhandled("x0,x:(r)+ a,y1");
		break;
		}
	case 591: { // x0,x:(r) a,y1
		unhandled("x0,x:(r) a,y1");
		break;
		}
	case 592: { // x0,x:(r+n) a,y1
		unhandled("x0,x:(r+n) a,y1");
		break;
		}
	case 593: { // x0,x:-(r) a,y1
		unhandled("x0,x:-(r) a,y1");
		break;
		}
	case 594: { // x0,x:(r)-n b,y0
		unhandled("x0,x:(r)-n b,y0");
		break;
		}
	case 595: { // x0,x:(r)+n b,y0
		unhandled("x0,x:(r)+n b,y0");
		break;
		}
	case 596: { // x0,x:(r)- b,y0
		unhandled("x0,x:(r)- b,y0");
		break;
		}
	case 597: { // x0,x:(r)+ b,y0
		unhandled("x0,x:(r)+ b,y0");
		break;
		}
	case 598: { // x0,x:(r) b,y0
		unhandled("x0,x:(r) b,y0");
		break;
		}
	case 599: { // x0,x:(r+n) b,y0
		unhandled("x0,x:(r+n) b,y0");
		break;
		}
	case 600: { // x0,x:-(r) b,y0
		unhandled("x0,x:-(r) b,y0");
		break;
		}
	case 601: { // x0,x:(r)-n b,y1
		unhandled("x0,x:(r)-n b,y1");
		break;
		}
	case 602: { // x0,x:(r)+n b,y1
		unhandled("x0,x:(r)+n b,y1");
		break;
		}
	case 603: { // x0,x:(r)- b,y1
		unhandled("x0,x:(r)- b,y1");
		break;
		}
	case 604: { // x0,x:(r)+ b,y1
		unhandled("x0,x:(r)+ b,y1");
		break;
		}
	case 605: { // x0,x:(r) b,y1
		unhandled("x0,x:(r) b,y1");
		break;
		}
	case 606: { // x0,x:(r+n) b,y1
		unhandled("x0,x:(r+n) b,y1");
		break;
		}
	case 607: { // x0,x:-(r) b,y1
		unhandled("x0,x:-(r) b,y1");
		break;
		}
	case 608: { // x1,x:(r)-n a,y0
		unhandled("x1,x:(r)-n a,y0");
		break;
		}
	case 609: { // x1,x:(r)+n a,y0
		unhandled("x1,x:(r)+n a,y0");
		break;
		}
	case 610: { // x1,x:(r)- a,y0
		unhandled("x1,x:(r)- a,y0");
		break;
		}
	case 611: { // x1,x:(r)+ a,y0
		unhandled("x1,x:(r)+ a,y0");
		break;
		}
	case 612: { // x1,x:(r) a,y0
		unhandled("x1,x:(r) a,y0");
		break;
		}
	case 613: { // x1,x:(r+n) a,y0
		unhandled("x1,x:(r+n) a,y0");
		break;
		}
	case 614: { // x1,x:-(r) a,y0
		unhandled("x1,x:-(r) a,y0");
		break;
		}
	case 615: { // x1,x:(r)-n a,y1
		unhandled("x1,x:(r)-n a,y1");
		break;
		}
	case 616: { // x1,x:(r)+n a,y1
		unhandled("x1,x:(r)+n a,y1");
		break;
		}
	case 617: { // x1,x:(r)- a,y1
		unhandled("x1,x:(r)- a,y1");
		break;
		}
	case 618: { // x1,x:(r)+ a,y1
		unhandled("x1,x:(r)+ a,y1");
		break;
		}
	case 619: { // x1,x:(r) a,y1
		unhandled("x1,x:(r) a,y1");
		break;
		}
	case 620: { // x1,x:(r+n) a,y1
		unhandled("x1,x:(r+n) a,y1");
		break;
		}
	case 621: { // x1,x:-(r) a,y1
		unhandled("x1,x:-(r) a,y1");
		break;
		}
	case 622: { // x1,x:(r)-n b,y0
		unhandled("x1,x:(r)-n b,y0");
		break;
		}
	case 623: { // x1,x:(r)+n b,y0
		unhandled("x1,x:(r)+n b,y0");
		break;
		}
	case 624: { // x1,x:(r)- b,y0
		unhandled("x1,x:(r)- b,y0");
		break;
		}
	case 625: { // x1,x:(r)+ b,y0
		unhandled("x1,x:(r)+ b,y0");
		break;
		}
	case 626: { // x1,x:(r) b,y0
		unhandled("x1,x:(r) b,y0");
		break;
		}
	case 627: { // x1,x:(r+n) b,y0
		unhandled("x1,x:(r+n) b,y0");
		break;
		}
	case 628: { // x1,x:-(r) b,y0
		unhandled("x1,x:-(r) b,y0");
		break;
		}
	case 629: { // x1,x:(r)-n b,y1
		unhandled("x1,x:(r)-n b,y1");
		break;
		}
	case 630: { // x1,x:(r)+n b,y1
		unhandled("x1,x:(r)+n b,y1");
		break;
		}
	case 631: { // x1,x:(r)- b,y1
		unhandled("x1,x:(r)- b,y1");
		break;
		}
	case 632: { // x1,x:(r)+ b,y1
		unhandled("x1,x:(r)+ b,y1");
		break;
		}
	case 633: { // x1,x:(r) b,y1
		unhandled("x1,x:(r) b,y1");
		break;
		}
	case 634: { // x1,x:(r+n) b,y1
		unhandled("x1,x:(r+n) b,y1");
		break;
		}
	case 635: { // x1,x:-(r) b,y1
		unhandled("x1,x:-(r) b,y1");
		break;
		}
	case 636: { // a,x:(r)-n a,y0
		unhandled("a,x:(r)-n a,y0");
		break;
		}
	case 637: { // a,x:(r)+n a,y0
		unhandled("a,x:(r)+n a,y0");
		break;
		}
	case 638: { // a,x:(r)- a,y0
		unhandled("a,x:(r)- a,y0");
		break;
		}
	case 639: { // a,x:(r)+ a,y0
		unhandled("a,x:(r)+ a,y0");
		break;
		}
	case 640: { // a,x:(r) a,y0
		unhandled("a,x:(r) a,y0");
		break;
		}
	case 641: { // a,x:(r+n) a,y0
		unhandled("a,x:(r+n) a,y0");
		break;
		}
	case 642: { // a,x:-(r) a,y0
		unhandled("a,x:-(r) a,y0");
		break;
		}
	case 643: { // a,x:(r)-n a,y1
		unhandled("a,x:(r)-n a,y1");
		break;
		}
	case 644: { // a,x:(r)+n a,y1
		unhandled("a,x:(r)+n a,y1");
		break;
		}
	case 645: { // a,x:(r)- a,y1
		unhandled("a,x:(r)- a,y1");
		break;
		}
	case 646: { // a,x:(r)+ a,y1
		unhandled("a,x:(r)+ a,y1");
		break;
		}
	case 647: { // a,x:(r) a,y1
		unhandled("a,x:(r) a,y1");
		break;
		}
	case 648: { // a,x:(r+n) a,y1
		unhandled("a,x:(r+n) a,y1");
		break;
		}
	case 649: { // a,x:-(r) a,y1
		unhandled("a,x:-(r) a,y1");
		break;
		}
	case 650: { // a,x:(r)-n b,y0
		unhandled("a,x:(r)-n b,y0");
		break;
		}
	case 651: { // a,x:(r)+n b,y0
		unhandled("a,x:(r)+n b,y0");
		break;
		}
	case 652: { // a,x:(r)- b,y0
		unhandled("a,x:(r)- b,y0");
		break;
		}
	case 653: { // a,x:(r)+ b,y0
		unhandled("a,x:(r)+ b,y0");
		break;
		}
	case 654: { // a,x:(r) b,y0
		unhandled("a,x:(r) b,y0");
		break;
		}
	case 655: { // a,x:(r+n) b,y0
		unhandled("a,x:(r+n) b,y0");
		break;
		}
	case 656: { // a,x:-(r) b,y0
		unhandled("a,x:-(r) b,y0");
		break;
		}
	case 657: { // a,x:(r)-n b,y1
		unhandled("a,x:(r)-n b,y1");
		break;
		}
	case 658: { // a,x:(r)+n b,y1
		unhandled("a,x:(r)+n b,y1");
		break;
		}
	case 659: { // a,x:(r)- b,y1
		unhandled("a,x:(r)- b,y1");
		break;
		}
	case 660: { // a,x:(r)+ b,y1
		unhandled("a,x:(r)+ b,y1");
		break;
		}
	case 661: { // a,x:(r) b,y1
		unhandled("a,x:(r) b,y1");
		break;
		}
	case 662: { // a,x:(r+n) b,y1
		unhandled("a,x:(r+n) b,y1");
		break;
		}
	case 663: { // a,x:-(r) b,y1
		unhandled("a,x:-(r) b,y1");
		break;
		}
	case 664: { // b,x:(r)-n a,y0
		unhandled("b,x:(r)-n a,y0");
		break;
		}
	case 665: { // b,x:(r)+n a,y0
		unhandled("b,x:(r)+n a,y0");
		break;
		}
	case 666: { // b,x:(r)- a,y0
		unhandled("b,x:(r)- a,y0");
		break;
		}
	case 667: { // b,x:(r)+ a,y0
		unhandled("b,x:(r)+ a,y0");
		break;
		}
	case 668: { // b,x:(r) a,y0
		unhandled("b,x:(r) a,y0");
		break;
		}
	case 669: { // b,x:(r+n) a,y0
		unhandled("b,x:(r+n) a,y0");
		break;
		}
	case 670: { // b,x:-(r) a,y0
		unhandled("b,x:-(r) a,y0");
		break;
		}
	case 671: { // b,x:(r)-n a,y1
		unhandled("b,x:(r)-n a,y1");
		break;
		}
	case 672: { // b,x:(r)+n a,y1
		unhandled("b,x:(r)+n a,y1");
		break;
		}
	case 673: { // b,x:(r)- a,y1
		unhandled("b,x:(r)- a,y1");
		break;
		}
	case 674: { // b,x:(r)+ a,y1
		unhandled("b,x:(r)+ a,y1");
		break;
		}
	case 675: { // b,x:(r) a,y1
		unhandled("b,x:(r) a,y1");
		break;
		}
	case 676: { // b,x:(r+n) a,y1
		unhandled("b,x:(r+n) a,y1");
		break;
		}
	case 677: { // b,x:-(r) a,y1
		unhandled("b,x:-(r) a,y1");
		break;
		}
	case 678: { // b,x:(r)-n b,y0
		unhandled("b,x:(r)-n b,y0");
		break;
		}
	case 679: { // b,x:(r)+n b,y0
		unhandled("b,x:(r)+n b,y0");
		break;
		}
	case 680: { // b,x:(r)- b,y0
		unhandled("b,x:(r)- b,y0");
		break;
		}
	case 681: { // b,x:(r)+ b,y0
		unhandled("b,x:(r)+ b,y0");
		break;
		}
	case 682: { // b,x:(r) b,y0
		unhandled("b,x:(r) b,y0");
		break;
		}
	case 683: { // b,x:(r+n) b,y0
		unhandled("b,x:(r+n) b,y0");
		break;
		}
	case 684: { // b,x:-(r) b,y0
		unhandled("b,x:-(r) b,y0");
		break;
		}
	case 685: { // b,x:(r)-n b,y1
		unhandled("b,x:(r)-n b,y1");
		break;
		}
	case 686: { // b,x:(r)+n b,y1
		unhandled("b,x:(r)+n b,y1");
		break;
		}
	case 687: { // b,x:(r)- b,y1
		unhandled("b,x:(r)- b,y1");
		break;
		}
	case 688: { // b,x:(r)+ b,y1
		unhandled("b,x:(r)+ b,y1");
		break;
		}
	case 689: { // b,x:(r) b,y1
		unhandled("b,x:(r) b,y1");
		break;
		}
	case 690: { // b,x:(r+n) b,y1
		unhandled("b,x:(r+n) b,y1");
		break;
		}
	case 691: { // b,x:-(r) b,y1
		unhandled("b,x:-(r) b,y1");
		break;
		}
	case 692: { // x:[abs],x0 a,y0
		unhandled("x:[abs],x0 a,y0");
		break;
		}
	case 693: { // x:[abs],x0 a,y1
		unhandled("x:[abs],x0 a,y1");
		break;
		}
	case 694: { // x:[abs],x0 b,y0
		unhandled("x:[abs],x0 b,y0");
		break;
		}
	case 695: { // x:[abs],x0 b,y1
		unhandled("x:[abs],x0 b,y1");
		break;
		}
	case 696: { // x:[abs],x1 a,y0
		unhandled("x:[abs],x1 a,y0");
		break;
		}
	case 697: { // x:[abs],x1 a,y1
		unhandled("x:[abs],x1 a,y1");
		break;
		}
	case 698: { // x:[abs],x1 b,y0
		unhandled("x:[abs],x1 b,y0");
		break;
		}
	case 699: { // x:[abs],x1 b,y1
		unhandled("x:[abs],x1 b,y1");
		break;
		}
	case 700: { // x:[abs],a a,y0
		unhandled("x:[abs],a a,y0");
		break;
		}
	case 701: { // x:[abs],a a,y1
		unhandled("x:[abs],a a,y1");
		break;
		}
	case 702: { // x:[abs],a b,y0
		unhandled("x:[abs],a b,y0");
		break;
		}
	case 703: { // x:[abs],a b,y1
		unhandled("x:[abs],a b,y1");
		break;
		}
	case 704: { // x:[abs],b a,y0
		unhandled("x:[abs],b a,y0");
		break;
		}
	case 705: { // x:[abs],b a,y1
		unhandled("x:[abs],b a,y1");
		break;
		}
	case 706: { // x:[abs],b b,y0
		unhandled("x:[abs],b b,y0");
		break;
		}
	case 707: { // x:[abs],b b,y1
		unhandled("x:[abs],b b,y1");
		break;
		}
	case 708: { // x:#[i],x0 a,y0
		unhandled("x:#[i],x0 a,y0");
		break;
		}
	case 709: { // x:#[i],x0 a,y1
		unhandled("x:#[i],x0 a,y1");
		break;
		}
	case 710: { // x:#[i],x0 b,y0
		unhandled("x:#[i],x0 b,y0");
		break;
		}
	case 711: { // x:#[i],x0 b,y1
		unhandled("x:#[i],x0 b,y1");
		break;
		}
	case 712: { // x:#[i],x1 a,y0
		unhandled("x:#[i],x1 a,y0");
		break;
		}
	case 713: { // x:#[i],x1 a,y1
		unhandled("x:#[i],x1 a,y1");
		break;
		}
	case 714: { // x:#[i],x1 b,y0
		unhandled("x:#[i],x1 b,y0");
		break;
		}
	case 715: { // x:#[i],x1 b,y1
		unhandled("x:#[i],x1 b,y1");
		break;
		}
	case 716: { // x:#[i],a a,y0
		unhandled("x:#[i],a a,y0");
		break;
		}
	case 717: { // x:#[i],a a,y1
		unhandled("x:#[i],a a,y1");
		break;
		}
	case 718: { // x:#[i],a b,y0
		unhandled("x:#[i],a b,y0");
		break;
		}
	case 719: { // x:#[i],a b,y1
		unhandled("x:#[i],a b,y1");
		break;
		}
	case 720: { // x:#[i],b a,y0
		unhandled("x:#[i],b a,y0");
		break;
		}
	case 721: { // x:#[i],b a,y1
		unhandled("x:#[i],b a,y1");
		break;
		}
	case 722: { // x:#[i],b b,y0
		unhandled("x:#[i],b b,y0");
		break;
		}
	case 723: { // x:#[i],b b,y1
		unhandled("x:#[i],b b,y1");
		break;
		}
	case 724: { // a,x:(r)-n x0,a
		unhandled("a,x:(r)-n x0,a");
		break;
		}
	case 725: { // a,x:(r)+n x0,a
		unhandled("a,x:(r)+n x0,a");
		break;
		}
	case 726: { // a,x:(r)- x0,a
		unhandled("a,x:(r)- x0,a");
		break;
		}
	case 727: { // a,x:(r)+ x0,a
		unhandled("a,x:(r)+ x0,a");
		break;
		}
	case 728: { // a,x:(r) x0,a
		unhandled("a,x:(r) x0,a");
		break;
		}
	case 729: { // a,x:(r+n) x0,a
		unhandled("a,x:(r+n) x0,a");
		break;
		}
	case 730: { // a,x:-(r) x0,a
		unhandled("a,x:-(r) x0,a");
		break;
		}
	case 731: { // b,x:(r)-n x0,b
		unhandled("b,x:(r)-n x0,b");
		break;
		}
	case 732: { // b,x:(r)+n x0,b
		unhandled("b,x:(r)+n x0,b");
		break;
		}
	case 733: { // b,x:(r)- x0,b
		unhandled("b,x:(r)- x0,b");
		break;
		}
	case 734: { // b,x:(r)+ x0,b
		unhandled("b,x:(r)+ x0,b");
		break;
		}
	case 735: { // b,x:(r) x0,b
		unhandled("b,x:(r) x0,b");
		break;
		}
	case 736: { // b,x:(r+n) x0,b
		unhandled("b,x:(r+n) x0,b");
		break;
		}
	case 737: { // b,x:-(r) x0,b
		unhandled("b,x:-(r) x0,b");
		break;
		}
	case 738: { // y:(r)-n,x0
		unhandled("y:(r)-n,x0");
		break;
		}
	case 739: { // y:(r)+n,x0
		unhandled("y:(r)+n,x0");
		break;
		}
	case 740: { // y:(r)-,x0
		unhandled("y:(r)-,x0");
		break;
		}
	case 741: { // y:(r)+,x0
		unhandled("y:(r)+,x0");
		break;
		}
	case 742: { // y:(r),x0
		unhandled("y:(r),x0");
		break;
		}
	case 743: { // y:(r+n),x0
		unhandled("y:(r+n),x0");
		break;
		}
	case 744: { // y:-(r),x0
		unhandled("y:-(r),x0");
		break;
		}
	case 745: { // y:(r)-n,x1
		unhandled("y:(r)-n,x1");
		break;
		}
	case 746: { // y:(r)+n,x1
		unhandled("y:(r)+n,x1");
		break;
		}
	case 747: { // y:(r)-,x1
		unhandled("y:(r)-,x1");
		break;
		}
	case 748: { // y:(r)+,x1
		unhandled("y:(r)+,x1");
		break;
		}
	case 749: { // y:(r),x1
		unhandled("y:(r),x1");
		break;
		}
	case 750: { // y:(r+n),x1
		unhandled("y:(r+n),x1");
		break;
		}
	case 751: { // y:-(r),x1
		unhandled("y:-(r),x1");
		break;
		}
	case 752: { // y:(r)-n,y0
		unhandled("y:(r)-n,y0");
		break;
		}
	case 753: { // y:(r)+n,y0
		unhandled("y:(r)+n,y0");
		break;
		}
	case 754: { // y:(r)-,y0
		unhandled("y:(r)-,y0");
		break;
		}
	case 755: { // y:(r)+,y0
		unhandled("y:(r)+,y0");
		break;
		}
	case 756: { // y:(r),y0
		unhandled("y:(r),y0");
		break;
		}
	case 757: { // y:(r+n),y0
		unhandled("y:(r+n),y0");
		break;
		}
	case 758: { // y:-(r),y0
		unhandled("y:-(r),y0");
		break;
		}
	case 759: { // y:(r)-n,y1
		unhandled("y:(r)-n,y1");
		break;
		}
	case 760: { // y:(r)+n,y1
		unhandled("y:(r)+n,y1");
		break;
		}
	case 761: { // y:(r)-,y1
		unhandled("y:(r)-,y1");
		break;
		}
	case 762: { // y:(r)+,y1
		unhandled("y:(r)+,y1");
		break;
		}
	case 763: { // y:(r),y1
		unhandled("y:(r),y1");
		break;
		}
	case 764: { // y:(r+n),y1
		unhandled("y:(r+n),y1");
		break;
		}
	case 765: { // y:-(r),y1
		unhandled("y:-(r),y1");
		break;
		}
	case 766: { // y:(r)-n,a0
		unhandled("y:(r)-n,a0");
		break;
		}
	case 767: { // y:(r)+n,a0
		unhandled("y:(r)+n,a0");
		break;
		}
	case 768: { // y:(r)-,a0
		unhandled("y:(r)-,a0");
		break;
		}
	case 769: { // y:(r)+,a0
		unhandled("y:(r)+,a0");
		break;
		}
	case 770: { // y:(r),a0
		unhandled("y:(r),a0");
		break;
		}
	case 771: { // y:(r+n),a0
		unhandled("y:(r+n),a0");
		break;
		}
	case 772: { // y:-(r),a0
		unhandled("y:-(r),a0");
		break;
		}
	case 773: { // y:(r)-n,b0
		unhandled("y:(r)-n,b0");
		break;
		}
	case 774: { // y:(r)+n,b0
		unhandled("y:(r)+n,b0");
		break;
		}
	case 775: { // y:(r)-,b0
		unhandled("y:(r)-,b0");
		break;
		}
	case 776: { // y:(r)+,b0
		unhandled("y:(r)+,b0");
		break;
		}
	case 777: { // y:(r),b0
		unhandled("y:(r),b0");
		break;
		}
	case 778: { // y:(r+n),b0
		unhandled("y:(r+n),b0");
		break;
		}
	case 779: { // y:-(r),b0
		unhandled("y:-(r),b0");
		break;
		}
	case 780: { // y:(r)-n,a2
		unhandled("y:(r)-n,a2");
		break;
		}
	case 781: { // y:(r)+n,a2
		unhandled("y:(r)+n,a2");
		break;
		}
	case 782: { // y:(r)-,a2
		unhandled("y:(r)-,a2");
		break;
		}
	case 783: { // y:(r)+,a2
		unhandled("y:(r)+,a2");
		break;
		}
	case 784: { // y:(r),a2
		unhandled("y:(r),a2");
		break;
		}
	case 785: { // y:(r+n),a2
		unhandled("y:(r+n),a2");
		break;
		}
	case 786: { // y:-(r),a2
		unhandled("y:-(r),a2");
		break;
		}
	case 787: { // y:(r)-n,b2
		unhandled("y:(r)-n,b2");
		break;
		}
	case 788: { // y:(r)+n,b2
		unhandled("y:(r)+n,b2");
		break;
		}
	case 789: { // y:(r)-,b2
		unhandled("y:(r)-,b2");
		break;
		}
	case 790: { // y:(r)+,b2
		unhandled("y:(r)+,b2");
		break;
		}
	case 791: { // y:(r),b2
		unhandled("y:(r),b2");
		break;
		}
	case 792: { // y:(r+n),b2
		unhandled("y:(r+n),b2");
		break;
		}
	case 793: { // y:-(r),b2
		unhandled("y:-(r),b2");
		break;
		}
	case 794: { // y:(r)-n,a1
		unhandled("y:(r)-n,a1");
		break;
		}
	case 795: { // y:(r)+n,a1
		unhandled("y:(r)+n,a1");
		break;
		}
	case 796: { // y:(r)-,a1
		unhandled("y:(r)-,a1");
		break;
		}
	case 797: { // y:(r)+,a1
		unhandled("y:(r)+,a1");
		break;
		}
	case 798: { // y:(r),a1
		unhandled("y:(r),a1");
		break;
		}
	case 799: { // y:(r+n),a1
		unhandled("y:(r+n),a1");
		break;
		}
	case 800: { // y:-(r),a1
		unhandled("y:-(r),a1");
		break;
		}
	case 801: { // y:(r)-n,b1
		unhandled("y:(r)-n,b1");
		break;
		}
	case 802: { // y:(r)+n,b1
		unhandled("y:(r)+n,b1");
		break;
		}
	case 803: { // y:(r)-,b1
		unhandled("y:(r)-,b1");
		break;
		}
	case 804: { // y:(r)+,b1
		unhandled("y:(r)+,b1");
		break;
		}
	case 805: { // y:(r),b1
		unhandled("y:(r),b1");
		break;
		}
	case 806: { // y:(r+n),b1
		unhandled("y:(r+n),b1");
		break;
		}
	case 807: { // y:-(r),b1
		unhandled("y:-(r),b1");
		break;
		}
	case 808: { // y:(r)-n,a
		unhandled("y:(r)-n,a");
		break;
		}
	case 809: { // y:(r)+n,a
		unhandled("y:(r)+n,a");
		break;
		}
	case 810: { // y:(r)-,a
		unhandled("y:(r)-,a");
		break;
		}
	case 811: { // y:(r)+,a
		unhandled("y:(r)+,a");
		break;
		}
	case 812: { // y:(r),a
		unhandled("y:(r),a");
		break;
		}
	case 813: { // y:(r+n),a
		unhandled("y:(r+n),a");
		break;
		}
	case 814: { // y:-(r),a
		unhandled("y:-(r),a");
		break;
		}
	case 815: { // y:(r)-n,b
		unhandled("y:(r)-n,b");
		break;
		}
	case 816: { // y:(r)+n,b
		unhandled("y:(r)+n,b");
		break;
		}
	case 817: { // y:(r)-,b
		unhandled("y:(r)-,b");
		break;
		}
	case 818: { // y:(r)+,b
		unhandled("y:(r)+,b");
		break;
		}
	case 819: { // y:(r),b
		unhandled("y:(r),b");
		break;
		}
	case 820: { // y:(r+n),b
		unhandled("y:(r+n),b");
		break;
		}
	case 821: { // y:-(r),b
		unhandled("y:-(r),b");
		break;
		}
	case 822: { // y:(r)-n,r
		unhandled("y:(r)-n,r");
		break;
		}
	case 823: { // y:(r)+n,r
		unhandled("y:(r)+n,r");
		break;
		}
	case 824: { // y:(r)-,r
		unhandled("y:(r)-,r");
		break;
		}
	case 825: { // y:(r)+,r
		unhandled("y:(r)+,r");
		break;
		}
	case 826: { // y:(r),r
		unhandled("y:(r),r");
		break;
		}
	case 827: { // y:(r+n),r
		unhandled("y:(r+n),r");
		break;
		}
	case 828: { // y:-(r),r
		unhandled("y:-(r),r");
		break;
		}
	case 829: { // y:(r)-n,n
		unhandled("y:(r)-n,n");
		break;
		}
	case 830: { // y:(r)+n,n
		unhandled("y:(r)+n,n");
		break;
		}
	case 831: { // y:(r)-,n
		unhandled("y:(r)-,n");
		break;
		}
	case 832: { // y:(r)+,n
		unhandled("y:(r)+,n");
		break;
		}
	case 833: { // y:(r),n
		unhandled("y:(r),n");
		break;
		}
	case 834: { // y:(r+n),n
		unhandled("y:(r+n),n");
		break;
		}
	case 835: { // y:-(r),n
		unhandled("y:-(r),n");
		break;
		}
	case 836: { // x0,y:(r)-n
		unhandled("x0,y:(r)-n");
		break;
		}
	case 837: { // x0,y:(r)+n
		unhandled("x0,y:(r)+n");
		break;
		}
	case 838: { // x0,y:(r)-
		unhandled("x0,y:(r)-");
		break;
		}
	case 839: { // x0,y:(r)+
		unhandled("x0,y:(r)+");
		break;
		}
	case 840: { // x0,y:(r)
		unhandled("x0,y:(r)");
		break;
		}
	case 841: { // x0,y:(r+n)
		unhandled("x0,y:(r+n)");
		break;
		}
	case 842: { // x0,y:-(r)
		unhandled("x0,y:-(r)");
		break;
		}
	case 843: { // x1,y:(r)-n
		unhandled("x1,y:(r)-n");
		break;
		}
	case 844: { // x1,y:(r)+n
		unhandled("x1,y:(r)+n");
		break;
		}
	case 845: { // x1,y:(r)-
		unhandled("x1,y:(r)-");
		break;
		}
	case 846: { // x1,y:(r)+
		unhandled("x1,y:(r)+");
		break;
		}
	case 847: { // x1,y:(r)
		unhandled("x1,y:(r)");
		break;
		}
	case 848: { // x1,y:(r+n)
		unhandled("x1,y:(r+n)");
		break;
		}
	case 849: { // x1,y:-(r)
		unhandled("x1,y:-(r)");
		break;
		}
	case 850: { // y0,y:(r)-n
		unhandled("y0,y:(r)-n");
		break;
		}
	case 851: { // y0,y:(r)+n
		unhandled("y0,y:(r)+n");
		break;
		}
	case 852: { // y0,y:(r)-
		unhandled("y0,y:(r)-");
		break;
		}
	case 853: { // y0,y:(r)+
		unhandled("y0,y:(r)+");
		break;
		}
	case 854: { // y0,y:(r)
		unhandled("y0,y:(r)");
		break;
		}
	case 855: { // y0,y:(r+n)
		unhandled("y0,y:(r+n)");
		break;
		}
	case 856: { // y0,y:-(r)
		unhandled("y0,y:-(r)");
		break;
		}
	case 857: { // y1,y:(r)-n
		unhandled("y1,y:(r)-n");
		break;
		}
	case 858: { // y1,y:(r)+n
		unhandled("y1,y:(r)+n");
		break;
		}
	case 859: { // y1,y:(r)-
		unhandled("y1,y:(r)-");
		break;
		}
	case 860: { // y1,y:(r)+
		unhandled("y1,y:(r)+");
		break;
		}
	case 861: { // y1,y:(r)
		unhandled("y1,y:(r)");
		break;
		}
	case 862: { // y1,y:(r+n)
		unhandled("y1,y:(r+n)");
		break;
		}
	case 863: { // y1,y:-(r)
		unhandled("y1,y:-(r)");
		break;
		}
	case 864: { // a0,y:(r)-n
		unhandled("a0,y:(r)-n");
		break;
		}
	case 865: { // a0,y:(r)+n
		unhandled("a0,y:(r)+n");
		break;
		}
	case 866: { // a0,y:(r)-
		unhandled("a0,y:(r)-");
		break;
		}
	case 867: { // a0,y:(r)+
		unhandled("a0,y:(r)+");
		break;
		}
	case 868: { // a0,y:(r)
		unhandled("a0,y:(r)");
		break;
		}
	case 869: { // a0,y:(r+n)
		unhandled("a0,y:(r+n)");
		break;
		}
	case 870: { // a0,y:-(r)
		unhandled("a0,y:-(r)");
		break;
		}
	case 871: { // b0,y:(r)-n
		unhandled("b0,y:(r)-n");
		break;
		}
	case 872: { // b0,y:(r)+n
		unhandled("b0,y:(r)+n");
		break;
		}
	case 873: { // b0,y:(r)-
		unhandled("b0,y:(r)-");
		break;
		}
	case 874: { // b0,y:(r)+
		unhandled("b0,y:(r)+");
		break;
		}
	case 875: { // b0,y:(r)
		unhandled("b0,y:(r)");
		break;
		}
	case 876: { // b0,y:(r+n)
		unhandled("b0,y:(r+n)");
		break;
		}
	case 877: { // b0,y:-(r)
		unhandled("b0,y:-(r)");
		break;
		}
	case 878: { // a2,y:(r)-n
		unhandled("a2,y:(r)-n");
		break;
		}
	case 879: { // a2,y:(r)+n
		unhandled("a2,y:(r)+n");
		break;
		}
	case 880: { // a2,y:(r)-
		unhandled("a2,y:(r)-");
		break;
		}
	case 881: { // a2,y:(r)+
		unhandled("a2,y:(r)+");
		break;
		}
	case 882: { // a2,y:(r)
		unhandled("a2,y:(r)");
		break;
		}
	case 883: { // a2,y:(r+n)
		unhandled("a2,y:(r+n)");
		break;
		}
	case 884: { // a2,y:-(r)
		unhandled("a2,y:-(r)");
		break;
		}
	case 885: { // b2,y:(r)-n
		unhandled("b2,y:(r)-n");
		break;
		}
	case 886: { // b2,y:(r)+n
		unhandled("b2,y:(r)+n");
		break;
		}
	case 887: { // b2,y:(r)-
		unhandled("b2,y:(r)-");
		break;
		}
	case 888: { // b2,y:(r)+
		unhandled("b2,y:(r)+");
		break;
		}
	case 889: { // b2,y:(r)
		unhandled("b2,y:(r)");
		break;
		}
	case 890: { // b2,y:(r+n)
		unhandled("b2,y:(r+n)");
		break;
		}
	case 891: { // b2,y:-(r)
		unhandled("b2,y:-(r)");
		break;
		}
	case 892: { // a1,y:(r)-n
		unhandled("a1,y:(r)-n");
		break;
		}
	case 893: { // a1,y:(r)+n
		unhandled("a1,y:(r)+n");
		break;
		}
	case 894: { // a1,y:(r)-
		unhandled("a1,y:(r)-");
		break;
		}
	case 895: { // a1,y:(r)+
		unhandled("a1,y:(r)+");
		break;
		}
	case 896: { // a1,y:(r)
		unhandled("a1,y:(r)");
		break;
		}
	case 897: { // a1,y:(r+n)
		unhandled("a1,y:(r+n)");
		break;
		}
	case 898: { // a1,y:-(r)
		unhandled("a1,y:-(r)");
		break;
		}
	case 899: { // b1,y:(r)-n
		unhandled("b1,y:(r)-n");
		break;
		}
	case 900: { // b1,y:(r)+n
		unhandled("b1,y:(r)+n");
		break;
		}
	case 901: { // b1,y:(r)-
		unhandled("b1,y:(r)-");
		break;
		}
	case 902: { // b1,y:(r)+
		unhandled("b1,y:(r)+");
		break;
		}
	case 903: { // b1,y:(r)
		unhandled("b1,y:(r)");
		break;
		}
	case 904: { // b1,y:(r+n)
		unhandled("b1,y:(r+n)");
		break;
		}
	case 905: { // b1,y:-(r)
		unhandled("b1,y:-(r)");
		break;
		}
	case 906: { // a,y:(r)-n
		unhandled("a,y:(r)-n");
		break;
		}
	case 907: { // a,y:(r)+n
		unhandled("a,y:(r)+n");
		break;
		}
	case 908: { // a,y:(r)-
		unhandled("a,y:(r)-");
		break;
		}
	case 909: { // a,y:(r)+
		unhandled("a,y:(r)+");
		break;
		}
	case 910: { // a,y:(r)
		unhandled("a,y:(r)");
		break;
		}
	case 911: { // a,y:(r+n)
		unhandled("a,y:(r+n)");
		break;
		}
	case 912: { // a,y:-(r)
		unhandled("a,y:-(r)");
		break;
		}
	case 913: { // b,y:(r)-n
		unhandled("b,y:(r)-n");
		break;
		}
	case 914: { // b,y:(r)+n
		unhandled("b,y:(r)+n");
		break;
		}
	case 915: { // b,y:(r)-
		unhandled("b,y:(r)-");
		break;
		}
	case 916: { // b,y:(r)+
		unhandled("b,y:(r)+");
		break;
		}
	case 917: { // b,y:(r)
		unhandled("b,y:(r)");
		break;
		}
	case 918: { // b,y:(r+n)
		unhandled("b,y:(r+n)");
		break;
		}
	case 919: { // b,y:-(r)
		unhandled("b,y:-(r)");
		break;
		}
	case 920: { // r,y:(r)-n
		unhandled("r,y:(r)-n");
		break;
		}
	case 921: { // r,y:(r)+n
		unhandled("r,y:(r)+n");
		break;
		}
	case 922: { // r,y:(r)-
		unhandled("r,y:(r)-");
		break;
		}
	case 923: { // r,y:(r)+
		unhandled("r,y:(r)+");
		break;
		}
	case 924: { // r,y:(r)
		unhandled("r,y:(r)");
		break;
		}
	case 925: { // r,y:(r+n)
		unhandled("r,y:(r+n)");
		break;
		}
	case 926: { // r,y:-(r)
		unhandled("r,y:-(r)");
		break;
		}
	case 927: { // n,y:(r)-n
		unhandled("n,y:(r)-n");
		break;
		}
	case 928: { // n,y:(r)+n
		unhandled("n,y:(r)+n");
		break;
		}
	case 929: { // n,y:(r)-
		unhandled("n,y:(r)-");
		break;
		}
	case 930: { // n,y:(r)+
		unhandled("n,y:(r)+");
		break;
		}
	case 931: { // n,y:(r)
		unhandled("n,y:(r)");
		break;
		}
	case 932: { // n,y:(r+n)
		unhandled("n,y:(r+n)");
		break;
		}
	case 933: { // n,y:-(r)
		unhandled("n,y:-(r)");
		break;
		}
	case 934: { // [abs],x0
		unhandled("[abs],x0");
		break;
		}
	case 935: { // [abs],x1
		unhandled("[abs],x1");
		break;
		}
	case 936: { // [abs],y0
		unhandled("[abs],y0");
		break;
		}
	case 937: { // [abs],y1
		unhandled("[abs],y1");
		break;
		}
	case 938: { // [abs],a0
		unhandled("[abs],a0");
		break;
		}
	case 939: { // [abs],b0
		unhandled("[abs],b0");
		break;
		}
	case 940: { // [abs],a2
		unhandled("[abs],a2");
		break;
		}
	case 941: { // [abs],b2
		unhandled("[abs],b2");
		break;
		}
	case 942: { // [abs],a1
		unhandled("[abs],a1");
		break;
		}
	case 943: { // [abs],b1
		unhandled("[abs],b1");
		break;
		}
	case 944: { // [abs],a
		unhandled("[abs],a");
		break;
		}
	case 945: { // [abs],b
		unhandled("[abs],b");
		break;
		}
	case 946: { // [abs],r
		unhandled("[abs],r");
		break;
		}
	case 947: { // [abs],n
		unhandled("[abs],n");
		break;
		}
	case 948: { // #[i],x0
		unhandled("#[i],x0");
		break;
		}
	case 949: { // #[i],x1
		unhandled("#[i],x1");
		break;
		}
	case 950: { // #[i],y0
		unhandled("#[i],y0");
		break;
		}
	case 951: { // #[i],y1
		unhandled("#[i],y1");
		break;
		}
	case 952: { // #[i],a0
		unhandled("#[i],a0");
		break;
		}
	case 953: { // #[i],b0
		unhandled("#[i],b0");
		break;
		}
	case 954: { // #[i],a2
		unhandled("#[i],a2");
		break;
		}
	case 955: { // #[i],b2
		unhandled("#[i],b2");
		break;
		}
	case 956: { // #[i],a1
		unhandled("#[i],a1");
		break;
		}
	case 957: { // #[i],b1
		unhandled("#[i],b1");
		break;
		}
	case 958: { // #[i],a
		unhandled("#[i],a");
		break;
		}
	case 959: { // #[i],b
		unhandled("#[i],b");
		break;
		}
	case 960: { // #[i],r
		unhandled("#[i],r");
		break;
		}
	case 961: { // #[i],n
		unhandled("#[i],n");
		break;
		}
	case 962: { // y:[aa],x0
		unhandled("y:[aa],x0");
		break;
		}
	case 963: { // y:[aa],x1
		unhandled("y:[aa],x1");
		break;
		}
	case 964: { // y:[aa],y0
		unhandled("y:[aa],y0");
		break;
		}
	case 965: { // y:[aa],y1
		unhandled("y:[aa],y1");
		break;
		}
	case 966: { // y:[aa],a0
		unhandled("y:[aa],a0");
		break;
		}
	case 967: { // y:[aa],b0
		unhandled("y:[aa],b0");
		break;
		}
	case 968: { // y:[aa],a2
		unhandled("y:[aa],a2");
		break;
		}
	case 969: { // y:[aa],b2
		unhandled("y:[aa],b2");
		break;
		}
	case 970: { // y:[aa],a1
		unhandled("y:[aa],a1");
		break;
		}
	case 971: { // y:[aa],b1
		unhandled("y:[aa],b1");
		break;
		}
	case 972: { // y:[aa],a
		unhandled("y:[aa],a");
		break;
		}
	case 973: { // y:[aa],b
		unhandled("y:[aa],b");
		break;
		}
	case 974: { // y:[aa],r
		unhandled("y:[aa],r");
		break;
		}
	case 975: { // y:[aa],n
		unhandled("y:[aa],n");
		break;
		}
	case 976: { // x0,y:[aa]
		unhandled("x0,y:[aa]");
		break;
		}
	case 977: { // x1,y:[aa]
		unhandled("x1,y:[aa]");
		break;
		}
	case 978: { // y0,y:[aa]
		unhandled("y0,y:[aa]");
		break;
		}
	case 979: { // y1,y:[aa]
		unhandled("y1,y:[aa]");
		break;
		}
	case 980: { // a0,y:[aa]
		unhandled("a0,y:[aa]");
		break;
		}
	case 981: { // b0,y:[aa]
		unhandled("b0,y:[aa]");
		break;
		}
	case 982: { // a2,y:[aa]
		unhandled("a2,y:[aa]");
		break;
		}
	case 983: { // b2,y:[aa]
		unhandled("b2,y:[aa]");
		break;
		}
	case 984: { // a1,y:[aa]
		unhandled("a1,y:[aa]");
		break;
		}
	case 985: { // b1,y:[aa]
		unhandled("b1,y:[aa]");
		break;
		}
	case 986: { // a,y:[aa]
		unhandled("a,y:[aa]");
		break;
		}
	case 987: { // b,y:[aa]
		unhandled("b,y:[aa]");
		break;
		}
	case 988: { // r,y:[aa]
		unhandled("r,y:[aa]");
		break;
		}
	case 989: { // n,y:[aa]
		unhandled("n,y:[aa]");
		break;
		}
	case 990: { // a,x0 y:(r)-n,y0
		unhandled("a,x0 y:(r)-n,y0");
		break;
		}
	case 991: { // a,x0 y:(r)+n,y0
		unhandled("a,x0 y:(r)+n,y0");
		break;
		}
	case 992: { // a,x0 y:(r)-,y0
		unhandled("a,x0 y:(r)-,y0");
		break;
		}
	case 993: { // a,x0 y:(r)+,y0
		unhandled("a,x0 y:(r)+,y0");
		break;
		}
	case 994: { // a,x0 y:(r),y0
		unhandled("a,x0 y:(r),y0");
		break;
		}
	case 995: { // a,x0 y:(r+n),y0
		unhandled("a,x0 y:(r+n),y0");
		break;
		}
	case 996: { // a,x0 y:-(r),y0
		unhandled("a,x0 y:-(r),y0");
		break;
		}
	case 997: { // a,x1 y:(r)-n,y0
		unhandled("a,x1 y:(r)-n,y0");
		break;
		}
	case 998: { // a,x1 y:(r)+n,y0
		unhandled("a,x1 y:(r)+n,y0");
		break;
		}
	case 999: { // a,x1 y:(r)-,y0
		unhandled("a,x1 y:(r)-,y0");
		break;
		}
	case 1000: { // a,x1 y:(r)+,y0
		unhandled("a,x1 y:(r)+,y0");
		break;
		}
	case 1001: { // a,x1 y:(r),y0
		unhandled("a,x1 y:(r),y0");
		break;
		}
	case 1002: { // a,x1 y:(r+n),y0
		unhandled("a,x1 y:(r+n),y0");
		break;
		}
	case 1003: { // a,x1 y:-(r),y0
		unhandled("a,x1 y:-(r),y0");
		break;
		}
	case 1004: { // b,x0 y:(r)-n,y0
		unhandled("b,x0 y:(r)-n,y0");
		break;
		}
	case 1005: { // b,x0 y:(r)+n,y0
		unhandled("b,x0 y:(r)+n,y0");
		break;
		}
	case 1006: { // b,x0 y:(r)-,y0
		unhandled("b,x0 y:(r)-,y0");
		break;
		}
	case 1007: { // b,x0 y:(r)+,y0
		unhandled("b,x0 y:(r)+,y0");
		break;
		}
	case 1008: { // b,x0 y:(r),y0
		unhandled("b,x0 y:(r),y0");
		break;
		}
	case 1009: { // b,x0 y:(r+n),y0
		unhandled("b,x0 y:(r+n),y0");
		break;
		}
	case 1010: { // b,x0 y:-(r),y0
		unhandled("b,x0 y:-(r),y0");
		break;
		}
	case 1011: { // b,x1 y:(r)-n,y0
		unhandled("b,x1 y:(r)-n,y0");
		break;
		}
	case 1012: { // b,x1 y:(r)+n,y0
		unhandled("b,x1 y:(r)+n,y0");
		break;
		}
	case 1013: { // b,x1 y:(r)-,y0
		unhandled("b,x1 y:(r)-,y0");
		break;
		}
	case 1014: { // b,x1 y:(r)+,y0
		unhandled("b,x1 y:(r)+,y0");
		break;
		}
	case 1015: { // b,x1 y:(r),y0
		unhandled("b,x1 y:(r),y0");
		break;
		}
	case 1016: { // b,x1 y:(r+n),y0
		unhandled("b,x1 y:(r+n),y0");
		break;
		}
	case 1017: { // b,x1 y:-(r),y0
		unhandled("b,x1 y:-(r),y0");
		break;
		}
	case 1018: { // a,x0 y:(r)-n,y1
		unhandled("a,x0 y:(r)-n,y1");
		break;
		}
	case 1019: { // a,x0 y:(r)+n,y1
		unhandled("a,x0 y:(r)+n,y1");
		break;
		}
	case 1020: { // a,x0 y:(r)-,y1
		unhandled("a,x0 y:(r)-,y1");
		break;
		}
	case 1021: { // a,x0 y:(r)+,y1
		unhandled("a,x0 y:(r)+,y1");
		break;
		}
	case 1022: { // a,x0 y:(r),y1
		unhandled("a,x0 y:(r),y1");
		break;
		}
	case 1023: { // a,x0 y:(r+n),y1
		unhandled("a,x0 y:(r+n),y1");
		break;
		}
	case 1024: { // a,x0 y:-(r),y1
		unhandled("a,x0 y:-(r),y1");
		break;
		}
	case 1025: { // a,x1 y:(r)-n,y1
		unhandled("a,x1 y:(r)-n,y1");
		break;
		}
	case 1026: { // a,x1 y:(r)+n,y1
		unhandled("a,x1 y:(r)+n,y1");
		break;
		}
	case 1027: { // a,x1 y:(r)-,y1
		unhandled("a,x1 y:(r)-,y1");
		break;
		}
	case 1028: { // a,x1 y:(r)+,y1
		unhandled("a,x1 y:(r)+,y1");
		break;
		}
	case 1029: { // a,x1 y:(r),y1
		unhandled("a,x1 y:(r),y1");
		break;
		}
	case 1030: { // a,x1 y:(r+n),y1
		unhandled("a,x1 y:(r+n),y1");
		break;
		}
	case 1031: { // a,x1 y:-(r),y1
		unhandled("a,x1 y:-(r),y1");
		break;
		}
	case 1032: { // b,x0 y:(r)-n,y1
		unhandled("b,x0 y:(r)-n,y1");
		break;
		}
	case 1033: { // b,x0 y:(r)+n,y1
		unhandled("b,x0 y:(r)+n,y1");
		break;
		}
	case 1034: { // b,x0 y:(r)-,y1
		unhandled("b,x0 y:(r)-,y1");
		break;
		}
	case 1035: { // b,x0 y:(r)+,y1
		unhandled("b,x0 y:(r)+,y1");
		break;
		}
	case 1036: { // b,x0 y:(r),y1
		unhandled("b,x0 y:(r),y1");
		break;
		}
	case 1037: { // b,x0 y:(r+n),y1
		unhandled("b,x0 y:(r+n),y1");
		break;
		}
	case 1038: { // b,x0 y:-(r),y1
		unhandled("b,x0 y:-(r),y1");
		break;
		}
	case 1039: { // b,x1 y:(r)-n,y1
		unhandled("b,x1 y:(r)-n,y1");
		break;
		}
	case 1040: { // b,x1 y:(r)+n,y1
		unhandled("b,x1 y:(r)+n,y1");
		break;
		}
	case 1041: { // b,x1 y:(r)-,y1
		unhandled("b,x1 y:(r)-,y1");
		break;
		}
	case 1042: { // b,x1 y:(r)+,y1
		unhandled("b,x1 y:(r)+,y1");
		break;
		}
	case 1043: { // b,x1 y:(r),y1
		unhandled("b,x1 y:(r),y1");
		break;
		}
	case 1044: { // b,x1 y:(r+n),y1
		unhandled("b,x1 y:(r+n),y1");
		break;
		}
	case 1045: { // b,x1 y:-(r),y1
		unhandled("b,x1 y:-(r),y1");
		break;
		}
	case 1046: { // a,x0 y:(r)-n,a
		unhandled("a,x0 y:(r)-n,a");
		break;
		}
	case 1047: { // a,x0 y:(r)+n,a
		unhandled("a,x0 y:(r)+n,a");
		break;
		}
	case 1048: { // a,x0 y:(r)-,a
		unhandled("a,x0 y:(r)-,a");
		break;
		}
	case 1049: { // a,x0 y:(r)+,a
		unhandled("a,x0 y:(r)+,a");
		break;
		}
	case 1050: { // a,x0 y:(r),a
		unhandled("a,x0 y:(r),a");
		break;
		}
	case 1051: { // a,x0 y:(r+n),a
		unhandled("a,x0 y:(r+n),a");
		break;
		}
	case 1052: { // a,x0 y:-(r),a
		unhandled("a,x0 y:-(r),a");
		break;
		}
	case 1053: { // a,x1 y:(r)-n,a
		unhandled("a,x1 y:(r)-n,a");
		break;
		}
	case 1054: { // a,x1 y:(r)+n,a
		unhandled("a,x1 y:(r)+n,a");
		break;
		}
	case 1055: { // a,x1 y:(r)-,a
		unhandled("a,x1 y:(r)-,a");
		break;
		}
	case 1056: { // a,x1 y:(r)+,a
		unhandled("a,x1 y:(r)+,a");
		break;
		}
	case 1057: { // a,x1 y:(r),a
		unhandled("a,x1 y:(r),a");
		break;
		}
	case 1058: { // a,x1 y:(r+n),a
		unhandled("a,x1 y:(r+n),a");
		break;
		}
	case 1059: { // a,x1 y:-(r),a
		unhandled("a,x1 y:-(r),a");
		break;
		}
	case 1060: { // b,x0 y:(r)-n,a
		unhandled("b,x0 y:(r)-n,a");
		break;
		}
	case 1061: { // b,x0 y:(r)+n,a
		unhandled("b,x0 y:(r)+n,a");
		break;
		}
	case 1062: { // b,x0 y:(r)-,a
		unhandled("b,x0 y:(r)-,a");
		break;
		}
	case 1063: { // b,x0 y:(r)+,a
		unhandled("b,x0 y:(r)+,a");
		break;
		}
	case 1064: { // b,x0 y:(r),a
		unhandled("b,x0 y:(r),a");
		break;
		}
	case 1065: { // b,x0 y:(r+n),a
		unhandled("b,x0 y:(r+n),a");
		break;
		}
	case 1066: { // b,x0 y:-(r),a
		unhandled("b,x0 y:-(r),a");
		break;
		}
	case 1067: { // b,x1 y:(r)-n,a
		unhandled("b,x1 y:(r)-n,a");
		break;
		}
	case 1068: { // b,x1 y:(r)+n,a
		unhandled("b,x1 y:(r)+n,a");
		break;
		}
	case 1069: { // b,x1 y:(r)-,a
		unhandled("b,x1 y:(r)-,a");
		break;
		}
	case 1070: { // b,x1 y:(r)+,a
		unhandled("b,x1 y:(r)+,a");
		break;
		}
	case 1071: { // b,x1 y:(r),a
		unhandled("b,x1 y:(r),a");
		break;
		}
	case 1072: { // b,x1 y:(r+n),a
		unhandled("b,x1 y:(r+n),a");
		break;
		}
	case 1073: { // b,x1 y:-(r),a
		unhandled("b,x1 y:-(r),a");
		break;
		}
	case 1074: { // a,x0 y:(r)-n,b
		unhandled("a,x0 y:(r)-n,b");
		break;
		}
	case 1075: { // a,x0 y:(r)+n,b
		unhandled("a,x0 y:(r)+n,b");
		break;
		}
	case 1076: { // a,x0 y:(r)-,b
		unhandled("a,x0 y:(r)-,b");
		break;
		}
	case 1077: { // a,x0 y:(r)+,b
		unhandled("a,x0 y:(r)+,b");
		break;
		}
	case 1078: { // a,x0 y:(r),b
		unhandled("a,x0 y:(r),b");
		break;
		}
	case 1079: { // a,x0 y:(r+n),b
		unhandled("a,x0 y:(r+n),b");
		break;
		}
	case 1080: { // a,x0 y:-(r),b
		unhandled("a,x0 y:-(r),b");
		break;
		}
	case 1081: { // a,x1 y:(r)-n,b
		unhandled("a,x1 y:(r)-n,b");
		break;
		}
	case 1082: { // a,x1 y:(r)+n,b
		unhandled("a,x1 y:(r)+n,b");
		break;
		}
	case 1083: { // a,x1 y:(r)-,b
		unhandled("a,x1 y:(r)-,b");
		break;
		}
	case 1084: { // a,x1 y:(r)+,b
		unhandled("a,x1 y:(r)+,b");
		break;
		}
	case 1085: { // a,x1 y:(r),b
		unhandled("a,x1 y:(r),b");
		break;
		}
	case 1086: { // a,x1 y:(r+n),b
		unhandled("a,x1 y:(r+n),b");
		break;
		}
	case 1087: { // a,x1 y:-(r),b
		unhandled("a,x1 y:-(r),b");
		break;
		}
	case 1088: { // b,x0 y:(r)-n,b
		unhandled("b,x0 y:(r)-n,b");
		break;
		}
	case 1089: { // b,x0 y:(r)+n,b
		unhandled("b,x0 y:(r)+n,b");
		break;
		}
	case 1090: { // b,x0 y:(r)-,b
		unhandled("b,x0 y:(r)-,b");
		break;
		}
	case 1091: { // b,x0 y:(r)+,b
		unhandled("b,x0 y:(r)+,b");
		break;
		}
	case 1092: { // b,x0 y:(r),b
		unhandled("b,x0 y:(r),b");
		break;
		}
	case 1093: { // b,x0 y:(r+n),b
		unhandled("b,x0 y:(r+n),b");
		break;
		}
	case 1094: { // b,x0 y:-(r),b
		unhandled("b,x0 y:-(r),b");
		break;
		}
	case 1095: { // b,x1 y:(r)-n,b
		unhandled("b,x1 y:(r)-n,b");
		break;
		}
	case 1096: { // b,x1 y:(r)+n,b
		unhandled("b,x1 y:(r)+n,b");
		break;
		}
	case 1097: { // b,x1 y:(r)-,b
		unhandled("b,x1 y:(r)-,b");
		break;
		}
	case 1098: { // b,x1 y:(r)+,b
		unhandled("b,x1 y:(r)+,b");
		break;
		}
	case 1099: { // b,x1 y:(r),b
		unhandled("b,x1 y:(r),b");
		break;
		}
	case 1100: { // b,x1 y:(r+n),b
		unhandled("b,x1 y:(r+n),b");
		break;
		}
	case 1101: { // b,x1 y:-(r),b
		unhandled("b,x1 y:-(r),b");
		break;
		}
	case 1102: { // a,x0 y0,y:(r)-n
		unhandled("a,x0 y0,y:(r)-n");
		break;
		}
	case 1103: { // a,x0 y0,y:(r)+n
		unhandled("a,x0 y0,y:(r)+n");
		break;
		}
	case 1104: { // a,x0 y0,y:(r)-
		unhandled("a,x0 y0,y:(r)-");
		break;
		}
	case 1105: { // a,x0 y0,y:(r)+
		unhandled("a,x0 y0,y:(r)+");
		break;
		}
	case 1106: { // a,x0 y0,y:(r)
		unhandled("a,x0 y0,y:(r)");
		break;
		}
	case 1107: { // a,x0 y0,y:(r+n)
		unhandled("a,x0 y0,y:(r+n)");
		break;
		}
	case 1108: { // a,x0 y0,y:-(r)
		unhandled("a,x0 y0,y:-(r)");
		break;
		}
	case 1109: { // a,x1 y0,y:(r)-n
		unhandled("a,x1 y0,y:(r)-n");
		break;
		}
	case 1110: { // a,x1 y0,y:(r)+n
		unhandled("a,x1 y0,y:(r)+n");
		break;
		}
	case 1111: { // a,x1 y0,y:(r)-
		unhandled("a,x1 y0,y:(r)-");
		break;
		}
	case 1112: { // a,x1 y0,y:(r)+
		unhandled("a,x1 y0,y:(r)+");
		break;
		}
	case 1113: { // a,x1 y0,y:(r)
		unhandled("a,x1 y0,y:(r)");
		break;
		}
	case 1114: { // a,x1 y0,y:(r+n)
		unhandled("a,x1 y0,y:(r+n)");
		break;
		}
	case 1115: { // a,x1 y0,y:-(r)
		unhandled("a,x1 y0,y:-(r)");
		break;
		}
	case 1116: { // b,x0 y0,y:(r)-n
		unhandled("b,x0 y0,y:(r)-n");
		break;
		}
	case 1117: { // b,x0 y0,y:(r)+n
		unhandled("b,x0 y0,y:(r)+n");
		break;
		}
	case 1118: { // b,x0 y0,y:(r)-
		unhandled("b,x0 y0,y:(r)-");
		break;
		}
	case 1119: { // b,x0 y0,y:(r)+
		unhandled("b,x0 y0,y:(r)+");
		break;
		}
	case 1120: { // b,x0 y0,y:(r)
		unhandled("b,x0 y0,y:(r)");
		break;
		}
	case 1121: { // b,x0 y0,y:(r+n)
		unhandled("b,x0 y0,y:(r+n)");
		break;
		}
	case 1122: { // b,x0 y0,y:-(r)
		unhandled("b,x0 y0,y:-(r)");
		break;
		}
	case 1123: { // b,x1 y0,y:(r)-n
		unhandled("b,x1 y0,y:(r)-n");
		break;
		}
	case 1124: { // b,x1 y0,y:(r)+n
		unhandled("b,x1 y0,y:(r)+n");
		break;
		}
	case 1125: { // b,x1 y0,y:(r)-
		unhandled("b,x1 y0,y:(r)-");
		break;
		}
	case 1126: { // b,x1 y0,y:(r)+
		unhandled("b,x1 y0,y:(r)+");
		break;
		}
	case 1127: { // b,x1 y0,y:(r)
		unhandled("b,x1 y0,y:(r)");
		break;
		}
	case 1128: { // b,x1 y0,y:(r+n)
		unhandled("b,x1 y0,y:(r+n)");
		break;
		}
	case 1129: { // b,x1 y0,y:-(r)
		unhandled("b,x1 y0,y:-(r)");
		break;
		}
	case 1130: { // a,x0 y1,y:(r)-n
		unhandled("a,x0 y1,y:(r)-n");
		break;
		}
	case 1131: { // a,x0 y1,y:(r)+n
		unhandled("a,x0 y1,y:(r)+n");
		break;
		}
	case 1132: { // a,x0 y1,y:(r)-
		unhandled("a,x0 y1,y:(r)-");
		break;
		}
	case 1133: { // a,x0 y1,y:(r)+
		unhandled("a,x0 y1,y:(r)+");
		break;
		}
	case 1134: { // a,x0 y1,y:(r)
		unhandled("a,x0 y1,y:(r)");
		break;
		}
	case 1135: { // a,x0 y1,y:(r+n)
		unhandled("a,x0 y1,y:(r+n)");
		break;
		}
	case 1136: { // a,x0 y1,y:-(r)
		unhandled("a,x0 y1,y:-(r)");
		break;
		}
	case 1137: { // a,x1 y1,y:(r)-n
		unhandled("a,x1 y1,y:(r)-n");
		break;
		}
	case 1138: { // a,x1 y1,y:(r)+n
		unhandled("a,x1 y1,y:(r)+n");
		break;
		}
	case 1139: { // a,x1 y1,y:(r)-
		unhandled("a,x1 y1,y:(r)-");
		break;
		}
	case 1140: { // a,x1 y1,y:(r)+
		unhandled("a,x1 y1,y:(r)+");
		break;
		}
	case 1141: { // a,x1 y1,y:(r)
		unhandled("a,x1 y1,y:(r)");
		break;
		}
	case 1142: { // a,x1 y1,y:(r+n)
		unhandled("a,x1 y1,y:(r+n)");
		break;
		}
	case 1143: { // a,x1 y1,y:-(r)
		unhandled("a,x1 y1,y:-(r)");
		break;
		}
	case 1144: { // b,x0 y1,y:(r)-n
		unhandled("b,x0 y1,y:(r)-n");
		break;
		}
	case 1145: { // b,x0 y1,y:(r)+n
		unhandled("b,x0 y1,y:(r)+n");
		break;
		}
	case 1146: { // b,x0 y1,y:(r)-
		unhandled("b,x0 y1,y:(r)-");
		break;
		}
	case 1147: { // b,x0 y1,y:(r)+
		unhandled("b,x0 y1,y:(r)+");
		break;
		}
	case 1148: { // b,x0 y1,y:(r)
		unhandled("b,x0 y1,y:(r)");
		break;
		}
	case 1149: { // b,x0 y1,y:(r+n)
		unhandled("b,x0 y1,y:(r+n)");
		break;
		}
	case 1150: { // b,x0 y1,y:-(r)
		unhandled("b,x0 y1,y:-(r)");
		break;
		}
	case 1151: { // b,x1 y1,y:(r)-n
		unhandled("b,x1 y1,y:(r)-n");
		break;
		}
	case 1152: { // b,x1 y1,y:(r)+n
		unhandled("b,x1 y1,y:(r)+n");
		break;
		}
	case 1153: { // b,x1 y1,y:(r)-
		unhandled("b,x1 y1,y:(r)-");
		break;
		}
	case 1154: { // b,x1 y1,y:(r)+
		unhandled("b,x1 y1,y:(r)+");
		break;
		}
	case 1155: { // b,x1 y1,y:(r)
		unhandled("b,x1 y1,y:(r)");
		break;
		}
	case 1156: { // b,x1 y1,y:(r+n)
		unhandled("b,x1 y1,y:(r+n)");
		break;
		}
	case 1157: { // b,x1 y1,y:-(r)
		unhandled("b,x1 y1,y:-(r)");
		break;
		}
	case 1158: { // a,x0 a,y:(r)-n
		unhandled("a,x0 a,y:(r)-n");
		break;
		}
	case 1159: { // a,x0 a,y:(r)+n
		unhandled("a,x0 a,y:(r)+n");
		break;
		}
	case 1160: { // a,x0 a,y:(r)-
		unhandled("a,x0 a,y:(r)-");
		break;
		}
	case 1161: { // a,x0 a,y:(r)+
		unhandled("a,x0 a,y:(r)+");
		break;
		}
	case 1162: { // a,x0 a,y:(r)
		unhandled("a,x0 a,y:(r)");
		break;
		}
	case 1163: { // a,x0 a,y:(r+n)
		unhandled("a,x0 a,y:(r+n)");
		break;
		}
	case 1164: { // a,x0 a,y:-(r)
		unhandled("a,x0 a,y:-(r)");
		break;
		}
	case 1165: { // a,x1 a,y:(r)-n
		unhandled("a,x1 a,y:(r)-n");
		break;
		}
	case 1166: { // a,x1 a,y:(r)+n
		unhandled("a,x1 a,y:(r)+n");
		break;
		}
	case 1167: { // a,x1 a,y:(r)-
		unhandled("a,x1 a,y:(r)-");
		break;
		}
	case 1168: { // a,x1 a,y:(r)+
		unhandled("a,x1 a,y:(r)+");
		break;
		}
	case 1169: { // a,x1 a,y:(r)
		unhandled("a,x1 a,y:(r)");
		break;
		}
	case 1170: { // a,x1 a,y:(r+n)
		unhandled("a,x1 a,y:(r+n)");
		break;
		}
	case 1171: { // a,x1 a,y:-(r)
		unhandled("a,x1 a,y:-(r)");
		break;
		}
	case 1172: { // b,x0 a,y:(r)-n
		unhandled("b,x0 a,y:(r)-n");
		break;
		}
	case 1173: { // b,x0 a,y:(r)+n
		unhandled("b,x0 a,y:(r)+n");
		break;
		}
	case 1174: { // b,x0 a,y:(r)-
		unhandled("b,x0 a,y:(r)-");
		break;
		}
	case 1175: { // b,x0 a,y:(r)+
		unhandled("b,x0 a,y:(r)+");
		break;
		}
	case 1176: { // b,x0 a,y:(r)
		unhandled("b,x0 a,y:(r)");
		break;
		}
	case 1177: { // b,x0 a,y:(r+n)
		unhandled("b,x0 a,y:(r+n)");
		break;
		}
	case 1178: { // b,x0 a,y:-(r)
		unhandled("b,x0 a,y:-(r)");
		break;
		}
	case 1179: { // b,x1 a,y:(r)-n
		unhandled("b,x1 a,y:(r)-n");
		break;
		}
	case 1180: { // b,x1 a,y:(r)+n
		unhandled("b,x1 a,y:(r)+n");
		break;
		}
	case 1181: { // b,x1 a,y:(r)-
		unhandled("b,x1 a,y:(r)-");
		break;
		}
	case 1182: { // b,x1 a,y:(r)+
		unhandled("b,x1 a,y:(r)+");
		break;
		}
	case 1183: { // b,x1 a,y:(r)
		unhandled("b,x1 a,y:(r)");
		break;
		}
	case 1184: { // b,x1 a,y:(r+n)
		unhandled("b,x1 a,y:(r+n)");
		break;
		}
	case 1185: { // b,x1 a,y:-(r)
		unhandled("b,x1 a,y:-(r)");
		break;
		}
	case 1186: { // a,x0 b,y:(r)-n
		unhandled("a,x0 b,y:(r)-n");
		break;
		}
	case 1187: { // a,x0 b,y:(r)+n
		unhandled("a,x0 b,y:(r)+n");
		break;
		}
	case 1188: { // a,x0 b,y:(r)-
		unhandled("a,x0 b,y:(r)-");
		break;
		}
	case 1189: { // a,x0 b,y:(r)+
		unhandled("a,x0 b,y:(r)+");
		break;
		}
	case 1190: { // a,x0 b,y:(r)
		unhandled("a,x0 b,y:(r)");
		break;
		}
	case 1191: { // a,x0 b,y:(r+n)
		unhandled("a,x0 b,y:(r+n)");
		break;
		}
	case 1192: { // a,x0 b,y:-(r)
		unhandled("a,x0 b,y:-(r)");
		break;
		}
	case 1193: { // a,x1 b,y:(r)-n
		unhandled("a,x1 b,y:(r)-n");
		break;
		}
	case 1194: { // a,x1 b,y:(r)+n
		unhandled("a,x1 b,y:(r)+n");
		break;
		}
	case 1195: { // a,x1 b,y:(r)-
		unhandled("a,x1 b,y:(r)-");
		break;
		}
	case 1196: { // a,x1 b,y:(r)+
		unhandled("a,x1 b,y:(r)+");
		break;
		}
	case 1197: { // a,x1 b,y:(r)
		unhandled("a,x1 b,y:(r)");
		break;
		}
	case 1198: { // a,x1 b,y:(r+n)
		unhandled("a,x1 b,y:(r+n)");
		break;
		}
	case 1199: { // a,x1 b,y:-(r)
		unhandled("a,x1 b,y:-(r)");
		break;
		}
	case 1200: { // b,x0 b,y:(r)-n
		unhandled("b,x0 b,y:(r)-n");
		break;
		}
	case 1201: { // b,x0 b,y:(r)+n
		unhandled("b,x0 b,y:(r)+n");
		break;
		}
	case 1202: { // b,x0 b,y:(r)-
		unhandled("b,x0 b,y:(r)-");
		break;
		}
	case 1203: { // b,x0 b,y:(r)+
		unhandled("b,x0 b,y:(r)+");
		break;
		}
	case 1204: { // b,x0 b,y:(r)
		unhandled("b,x0 b,y:(r)");
		break;
		}
	case 1205: { // b,x0 b,y:(r+n)
		unhandled("b,x0 b,y:(r+n)");
		break;
		}
	case 1206: { // b,x0 b,y:-(r)
		unhandled("b,x0 b,y:-(r)");
		break;
		}
	case 1207: { // b,x1 b,y:(r)-n
		unhandled("b,x1 b,y:(r)-n");
		break;
		}
	case 1208: { // b,x1 b,y:(r)+n
		unhandled("b,x1 b,y:(r)+n");
		break;
		}
	case 1209: { // b,x1 b,y:(r)-
		unhandled("b,x1 b,y:(r)-");
		break;
		}
	case 1210: { // b,x1 b,y:(r)+
		unhandled("b,x1 b,y:(r)+");
		break;
		}
	case 1211: { // b,x1 b,y:(r)
		unhandled("b,x1 b,y:(r)");
		break;
		}
	case 1212: { // b,x1 b,y:(r+n)
		unhandled("b,x1 b,y:(r+n)");
		break;
		}
	case 1213: { // b,x1 b,y:-(r)
		unhandled("b,x1 b,y:-(r)");
		break;
		}
	case 1214: { // a,x0 x:[abs],y0
		unhandled("a,x0 x:[abs],y0");
		break;
		}
	case 1215: { // a,x1 x:[abs],y0
		unhandled("a,x1 x:[abs],y0");
		break;
		}
	case 1216: { // b,x0 x:[abs],y0
		unhandled("b,x0 x:[abs],y0");
		break;
		}
	case 1217: { // b,x1 x:[abs],y0
		unhandled("b,x1 x:[abs],y0");
		break;
		}
	case 1218: { // a,x0 x:[abs],y1
		unhandled("a,x0 x:[abs],y1");
		break;
		}
	case 1219: { // a,x1 x:[abs],y1
		unhandled("a,x1 x:[abs],y1");
		break;
		}
	case 1220: { // b,x0 x:[abs],y1
		unhandled("b,x0 x:[abs],y1");
		break;
		}
	case 1221: { // b,x1 x:[abs],y1
		unhandled("b,x1 x:[abs],y1");
		break;
		}
	case 1222: { // a,x0 x:[abs],a
		unhandled("a,x0 x:[abs],a");
		break;
		}
	case 1223: { // a,x1 x:[abs],a
		unhandled("a,x1 x:[abs],a");
		break;
		}
	case 1224: { // b,x0 x:[abs],a
		unhandled("b,x0 x:[abs],a");
		break;
		}
	case 1225: { // b,x1 x:[abs],a
		unhandled("b,x1 x:[abs],a");
		break;
		}
	case 1226: { // a,x0 x:[abs],b
		unhandled("a,x0 x:[abs],b");
		break;
		}
	case 1227: { // a,x1 x:[abs],b
		unhandled("a,x1 x:[abs],b");
		break;
		}
	case 1228: { // b,x0 x:[abs],b
		unhandled("b,x0 x:[abs],b");
		break;
		}
	case 1229: { // b,x1 x:[abs],b
		unhandled("b,x1 x:[abs],b");
		break;
		}
	case 1230: { // a,x0 x:#[i],y0
		unhandled("a,x0 x:#[i],y0");
		break;
		}
	case 1231: { // a,x1 x:#[i],y0
		unhandled("a,x1 x:#[i],y0");
		break;
		}
	case 1232: { // b,x0 x:#[i],y0
		unhandled("b,x0 x:#[i],y0");
		break;
		}
	case 1233: { // b,x1 x:#[i],y0
		unhandled("b,x1 x:#[i],y0");
		break;
		}
	case 1234: { // a,x0 x:#[i],y1
		unhandled("a,x0 x:#[i],y1");
		break;
		}
	case 1235: { // a,x1 x:#[i],y1
		unhandled("a,x1 x:#[i],y1");
		break;
		}
	case 1236: { // b,x0 x:#[i],y1
		unhandled("b,x0 x:#[i],y1");
		break;
		}
	case 1237: { // b,x1 x:#[i],y1
		unhandled("b,x1 x:#[i],y1");
		break;
		}
	case 1238: { // a,x0 x:#[i],a
		unhandled("a,x0 x:#[i],a");
		break;
		}
	case 1239: { // a,x1 x:#[i],a
		unhandled("a,x1 x:#[i],a");
		break;
		}
	case 1240: { // b,x0 x:#[i],a
		unhandled("b,x0 x:#[i],a");
		break;
		}
	case 1241: { // b,x1 x:#[i],a
		unhandled("b,x1 x:#[i],a");
		break;
		}
	case 1242: { // a,x0 x:#[i],b
		unhandled("a,x0 x:#[i],b");
		break;
		}
	case 1243: { // a,x1 x:#[i],b
		unhandled("a,x1 x:#[i],b");
		break;
		}
	case 1244: { // b,x0 x:#[i],b
		unhandled("b,x0 x:#[i],b");
		break;
		}
	case 1245: { // b,x1 x:#[i],b
		unhandled("b,x1 x:#[i],b");
		break;
		}
	case 1246: { // a,y:(r)-n y0,a
		unhandled("a,y:(r)-n y0,a");
		break;
		}
	case 1247: { // a,y:(r)+n y0,a
		unhandled("a,y:(r)+n y0,a");
		break;
		}
	case 1248: { // a,y:(r)- y0,a
		unhandled("a,y:(r)- y0,a");
		break;
		}
	case 1249: { // a,y:(r)+ y0,a
		unhandled("a,y:(r)+ y0,a");
		break;
		}
	case 1250: { // a,y:(r) y0,a
		unhandled("a,y:(r) y0,a");
		break;
		}
	case 1251: { // a,y:(r+n) y0,a
		unhandled("a,y:(r+n) y0,a");
		break;
		}
	case 1252: { // a,y:-(r) y0,a
		unhandled("a,y:-(r) y0,a");
		break;
		}
	case 1253: { // b,y:(r)-n y0,b
		unhandled("b,y:(r)-n y0,b");
		break;
		}
	case 1254: { // b,y:(r)+n y0,b
		unhandled("b,y:(r)+n y0,b");
		break;
		}
	case 1255: { // b,y:(r)- y0,b
		unhandled("b,y:(r)- y0,b");
		break;
		}
	case 1256: { // b,y:(r)+ y0,b
		unhandled("b,y:(r)+ y0,b");
		break;
		}
	case 1257: { // b,y:(r) y0,b
		unhandled("b,y:(r) y0,b");
		break;
		}
	case 1258: { // b,y:(r+n) y0,b
		unhandled("b,y:(r+n) y0,b");
		break;
		}
	case 1259: { // b,y:-(r) y0,b
		unhandled("b,y:-(r) y0,b");
		break;
		}
	case 1260: { // l:(r)-n,a10
		unhandled("l:(r)-n,a10");
		break;
		}
	case 1261: { // l:(r)+n,a10
		unhandled("l:(r)+n,a10");
		break;
		}
	case 1262: { // l:(r)-,a10
		unhandled("l:(r)-,a10");
		break;
		}
	case 1263: { // l:(r)+,a10
		unhandled("l:(r)+,a10");
		break;
		}
	case 1264: { // l:(r),a10
		unhandled("l:(r),a10");
		break;
		}
	case 1265: { // l:(r+n),a10
		unhandled("l:(r+n),a10");
		break;
		}
	case 1266: { // l:-(r),a10
		unhandled("l:-(r),a10");
		break;
		}
	case 1267: { // l:(r)-n,b10
		unhandled("l:(r)-n,b10");
		break;
		}
	case 1268: { // l:(r)+n,b10
		unhandled("l:(r)+n,b10");
		break;
		}
	case 1269: { // l:(r)-,b10
		unhandled("l:(r)-,b10");
		break;
		}
	case 1270: { // l:(r)+,b10
		unhandled("l:(r)+,b10");
		break;
		}
	case 1271: { // l:(r),b10
		unhandled("l:(r),b10");
		break;
		}
	case 1272: { // l:(r+n),b10
		unhandled("l:(r+n),b10");
		break;
		}
	case 1273: { // l:-(r),b10
		unhandled("l:-(r),b10");
		break;
		}
	case 1274: { // l:(r)-n,x
		unhandled("l:(r)-n,x");
		break;
		}
	case 1275: { // l:(r)+n,x
		unhandled("l:(r)+n,x");
		break;
		}
	case 1276: { // l:(r)-,x
		unhandled("l:(r)-,x");
		break;
		}
	case 1277: { // l:(r)+,x
		unhandled("l:(r)+,x");
		break;
		}
	case 1278: { // l:(r),x
		unhandled("l:(r),x");
		break;
		}
	case 1279: { // l:(r+n),x
		unhandled("l:(r+n),x");
		break;
		}
	case 1280: { // l:-(r),x
		unhandled("l:-(r),x");
		break;
		}
	case 1281: { // l:(r)-n,y
		unhandled("l:(r)-n,y");
		break;
		}
	case 1282: { // l:(r)+n,y
		unhandled("l:(r)+n,y");
		break;
		}
	case 1283: { // l:(r)-,y
		unhandled("l:(r)-,y");
		break;
		}
	case 1284: { // l:(r)+,y
		unhandled("l:(r)+,y");
		break;
		}
	case 1285: { // l:(r),y
		unhandled("l:(r),y");
		break;
		}
	case 1286: { // l:(r+n),y
		unhandled("l:(r+n),y");
		break;
		}
	case 1287: { // l:-(r),y
		unhandled("l:-(r),y");
		break;
		}
	case 1288: { // l:(r)-n,a
		unhandled("l:(r)-n,a");
		break;
		}
	case 1289: { // l:(r)+n,a
		unhandled("l:(r)+n,a");
		break;
		}
	case 1290: { // l:(r)-,a
		unhandled("l:(r)-,a");
		break;
		}
	case 1291: { // l:(r)+,a
		unhandled("l:(r)+,a");
		break;
		}
	case 1292: { // l:(r),a
		unhandled("l:(r),a");
		break;
		}
	case 1293: { // l:(r+n),a
		unhandled("l:(r+n),a");
		break;
		}
	case 1294: { // l:-(r),a
		unhandled("l:-(r),a");
		break;
		}
	case 1295: { // l:(r)-n,b
		unhandled("l:(r)-n,b");
		break;
		}
	case 1296: { // l:(r)+n,b
		unhandled("l:(r)+n,b");
		break;
		}
	case 1297: { // l:(r)-,b
		unhandled("l:(r)-,b");
		break;
		}
	case 1298: { // l:(r)+,b
		unhandled("l:(r)+,b");
		break;
		}
	case 1299: { // l:(r),b
		unhandled("l:(r),b");
		break;
		}
	case 1300: { // l:(r+n),b
		unhandled("l:(r+n),b");
		break;
		}
	case 1301: { // l:-(r),b
		unhandled("l:-(r),b");
		break;
		}
	case 1302: { // l:(r)-n,ab
		unhandled("l:(r)-n,ab");
		break;
		}
	case 1303: { // l:(r)+n,ab
		unhandled("l:(r)+n,ab");
		break;
		}
	case 1304: { // l:(r)-,ab
		unhandled("l:(r)-,ab");
		break;
		}
	case 1305: { // l:(r)+,ab
		unhandled("l:(r)+,ab");
		break;
		}
	case 1306: { // l:(r),ab
		unhandled("l:(r),ab");
		break;
		}
	case 1307: { // l:(r+n),ab
		unhandled("l:(r+n),ab");
		break;
		}
	case 1308: { // l:-(r),ab
		unhandled("l:-(r),ab");
		break;
		}
	case 1309: { // l:(r)-n,ba
		unhandled("l:(r)-n,ba");
		break;
		}
	case 1310: { // l:(r)+n,ba
		unhandled("l:(r)+n,ba");
		break;
		}
	case 1311: { // l:(r)-,ba
		unhandled("l:(r)-,ba");
		break;
		}
	case 1312: { // l:(r)+,ba
		unhandled("l:(r)+,ba");
		break;
		}
	case 1313: { // l:(r),ba
		unhandled("l:(r),ba");
		break;
		}
	case 1314: { // l:(r+n),ba
		unhandled("l:(r+n),ba");
		break;
		}
	case 1315: { // l:-(r),ba
		unhandled("l:-(r),ba");
		break;
		}
	case 1316: { // l:[abs],a10
		unhandled("l:[abs],a10");
		break;
		}
	case 1317: { // l:[abs],b10
		unhandled("l:[abs],b10");
		break;
		}
	case 1318: { // l:[abs],x
		unhandled("l:[abs],x");
		break;
		}
	case 1319: { // l:[abs],y
		unhandled("l:[abs],y");
		break;
		}
	case 1320: { // l:[abs],a
		unhandled("l:[abs],a");
		break;
		}
	case 1321: { // l:[abs],b
		unhandled("l:[abs],b");
		break;
		}
	case 1322: { // l:[abs],ab
		unhandled("l:[abs],ab");
		break;
		}
	case 1323: { // l:[abs],ba
		unhandled("l:[abs],ba");
		break;
		}
	case 1324: { // l:[aa],a10
		unhandled("l:[aa],a10");
		break;
		}
	case 1325: { // l:[aa],b10
		unhandled("l:[aa],b10");
		break;
		}
	case 1326: { // l:[aa],x
		unhandled("l:[aa],x");
		break;
		}
	case 1327: { // l:[aa],y
		unhandled("l:[aa],y");
		break;
		}
	case 1328: { // l:[aa],a
		unhandled("l:[aa],a");
		break;
		}
	case 1329: { // l:[aa],b
		unhandled("l:[aa],b");
		break;
		}
	case 1330: { // l:[aa],ab
		unhandled("l:[aa],ab");
		break;
		}
	case 1331: { // l:[aa],ba
		unhandled("l:[aa],ba");
		break;
		}
	case 1332: { // a10,l:(r)-n
		unhandled("a10,l:(r)-n");
		break;
		}
	case 1333: { // a10,l:(r)+n
		unhandled("a10,l:(r)+n");
		break;
		}
	case 1334: { // a10,l:(r)-
		unhandled("a10,l:(r)-");
		break;
		}
	case 1335: { // a10,l:(r)+
		unhandled("a10,l:(r)+");
		break;
		}
	case 1336: { // a10,l:(r)
		unhandled("a10,l:(r)");
		break;
		}
	case 1337: { // a10,l:(r+n)
		unhandled("a10,l:(r+n)");
		break;
		}
	case 1338: { // a10,l:-(r)
		unhandled("a10,l:-(r)");
		break;
		}
	case 1339: { // b10,l:(r)-n
		unhandled("b10,l:(r)-n");
		break;
		}
	case 1340: { // b10,l:(r)+n
		unhandled("b10,l:(r)+n");
		break;
		}
	case 1341: { // b10,l:(r)-
		unhandled("b10,l:(r)-");
		break;
		}
	case 1342: { // b10,l:(r)+
		unhandled("b10,l:(r)+");
		break;
		}
	case 1343: { // b10,l:(r)
		unhandled("b10,l:(r)");
		break;
		}
	case 1344: { // b10,l:(r+n)
		unhandled("b10,l:(r+n)");
		break;
		}
	case 1345: { // b10,l:-(r)
		unhandled("b10,l:-(r)");
		break;
		}
	case 1346: { // x,l:(r)-n
		unhandled("x,l:(r)-n");
		break;
		}
	case 1347: { // x,l:(r)+n
		unhandled("x,l:(r)+n");
		break;
		}
	case 1348: { // x,l:(r)-
		unhandled("x,l:(r)-");
		break;
		}
	case 1349: { // x,l:(r)+
		unhandled("x,l:(r)+");
		break;
		}
	case 1350: { // x,l:(r)
		unhandled("x,l:(r)");
		break;
		}
	case 1351: { // x,l:(r+n)
		unhandled("x,l:(r+n)");
		break;
		}
	case 1352: { // x,l:-(r)
		unhandled("x,l:-(r)");
		break;
		}
	case 1353: { // y,l:(r)-n
		unhandled("y,l:(r)-n");
		break;
		}
	case 1354: { // y,l:(r)+n
		unhandled("y,l:(r)+n");
		break;
		}
	case 1355: { // y,l:(r)-
		unhandled("y,l:(r)-");
		break;
		}
	case 1356: { // y,l:(r)+
		unhandled("y,l:(r)+");
		break;
		}
	case 1357: { // y,l:(r)
		unhandled("y,l:(r)");
		break;
		}
	case 1358: { // y,l:(r+n)
		unhandled("y,l:(r+n)");
		break;
		}
	case 1359: { // y,l:-(r)
		unhandled("y,l:-(r)");
		break;
		}
	case 1360: { // a,l:(r)-n
		unhandled("a,l:(r)-n");
		break;
		}
	case 1361: { // a,l:(r)+n
		unhandled("a,l:(r)+n");
		break;
		}
	case 1362: { // a,l:(r)-
		unhandled("a,l:(r)-");
		break;
		}
	case 1363: { // a,l:(r)+
		unhandled("a,l:(r)+");
		break;
		}
	case 1364: { // a,l:(r)
		unhandled("a,l:(r)");
		break;
		}
	case 1365: { // a,l:(r+n)
		unhandled("a,l:(r+n)");
		break;
		}
	case 1366: { // a,l:-(r)
		unhandled("a,l:-(r)");
		break;
		}
	case 1367: { // b,l:(r)-n
		unhandled("b,l:(r)-n");
		break;
		}
	case 1368: { // b,l:(r)+n
		unhandled("b,l:(r)+n");
		break;
		}
	case 1369: { // b,l:(r)-
		unhandled("b,l:(r)-");
		break;
		}
	case 1370: { // b,l:(r)+
		unhandled("b,l:(r)+");
		break;
		}
	case 1371: { // b,l:(r)
		unhandled("b,l:(r)");
		break;
		}
	case 1372: { // b,l:(r+n)
		unhandled("b,l:(r+n)");
		break;
		}
	case 1373: { // b,l:-(r)
		unhandled("b,l:-(r)");
		break;
		}
	case 1374: { // ab,l:(r)-n
		unhandled("ab,l:(r)-n");
		break;
		}
	case 1375: { // ab,l:(r)+n
		unhandled("ab,l:(r)+n");
		break;
		}
	case 1376: { // ab,l:(r)-
		unhandled("ab,l:(r)-");
		break;
		}
	case 1377: { // ab,l:(r)+
		unhandled("ab,l:(r)+");
		break;
		}
	case 1378: { // ab,l:(r)
		unhandled("ab,l:(r)");
		break;
		}
	case 1379: { // ab,l:(r+n)
		unhandled("ab,l:(r+n)");
		break;
		}
	case 1380: { // ab,l:-(r)
		unhandled("ab,l:-(r)");
		break;
		}
	case 1381: { // ba,l:(r)-n
		unhandled("ba,l:(r)-n");
		break;
		}
	case 1382: { // ba,l:(r)+n
		unhandled("ba,l:(r)+n");
		break;
		}
	case 1383: { // ba,l:(r)-
		unhandled("ba,l:(r)-");
		break;
		}
	case 1384: { // ba,l:(r)+
		unhandled("ba,l:(r)+");
		break;
		}
	case 1385: { // ba,l:(r)
		unhandled("ba,l:(r)");
		break;
		}
	case 1386: { // ba,l:(r+n)
		unhandled("ba,l:(r+n)");
		break;
		}
	case 1387: { // ba,l:-(r)
		unhandled("ba,l:-(r)");
		break;
		}
	case 1388: { // a10,l:[aa]
		unhandled("a10,l:[aa]");
		break;
		}
	case 1389: { // b10,l:[aa]
		unhandled("b10,l:[aa]");
		break;
		}
	case 1390: { // x,l:[aa]
		unhandled("x,l:[aa]");
		break;
		}
	case 1391: { // y,l:[aa]
		unhandled("y,l:[aa]");
		break;
		}
	case 1392: { // a,l:[aa]
		unhandled("a,l:[aa]");
		break;
		}
	case 1393: { // b,l:[aa]
		unhandled("b,l:[aa]");
		break;
		}
	case 1394: { // ab,l:[aa]
		unhandled("ab,l:[aa]");
		break;
		}
	case 1395: { // ba,l:[aa]
		unhandled("ba,l:[aa]");
		break;
		}
	case 1396: { // x:(r)+n,x0 y:(rh)+n,y0
		unhandled("x:(r)+n,x0 y:(rh)+n,y0");
		break;
		}
	case 1397: { // x:(r)+n,x0 y:(rh)+n,y1
		unhandled("x:(r)+n,x0 y:(rh)+n,y1");
		break;
		}
	case 1398: { // x:(r)+n,x0 y:(rh)+n,a
		unhandled("x:(r)+n,x0 y:(rh)+n,a");
		break;
		}
	case 1399: { // x:(r)+n,x0 y:(rh)+n,b
		unhandled("x:(r)+n,x0 y:(rh)+n,b");
		break;
		}
	case 1400: { // x:(r)+n,x1 y:(rh)+n,y0
		unhandled("x:(r)+n,x1 y:(rh)+n,y0");
		break;
		}
	case 1401: { // x:(r)+n,x1 y:(rh)+n,y1
		unhandled("x:(r)+n,x1 y:(rh)+n,y1");
		break;
		}
	case 1402: { // x:(r)+n,x1 y:(rh)+n,a
		unhandled("x:(r)+n,x1 y:(rh)+n,a");
		break;
		}
	case 1403: { // x:(r)+n,x1 y:(rh)+n,b
		unhandled("x:(r)+n,x1 y:(rh)+n,b");
		break;
		}
	case 1404: { // x:(r)+n,a y:(rh)+n,y0
		unhandled("x:(r)+n,a y:(rh)+n,y0");
		break;
		}
	case 1405: { // x:(r)+n,a y:(rh)+n,y1
		unhandled("x:(r)+n,a y:(rh)+n,y1");
		break;
		}
	case 1406: { // x:(r)+n,a y:(rh)+n,a
		unhandled("x:(r)+n,a y:(rh)+n,a");
		break;
		}
	case 1407: { // x:(r)+n,a y:(rh)+n,b
		unhandled("x:(r)+n,a y:(rh)+n,b");
		break;
		}
	case 1408: { // x:(r)+n,b y:(rh)+n,y0
		unhandled("x:(r)+n,b y:(rh)+n,y0");
		break;
		}
	case 1409: { // x:(r)+n,b y:(rh)+n,y1
		unhandled("x:(r)+n,b y:(rh)+n,y1");
		break;
		}
	case 1410: { // x:(r)+n,b y:(rh)+n,a
		unhandled("x:(r)+n,b y:(rh)+n,a");
		break;
		}
	case 1411: { // x:(r)+n,b y:(rh)+n,b
		unhandled("x:(r)+n,b y:(rh)+n,b");
		break;
		}
	case 1412: { // x:(r)+n,x0 y:(rh)-,y0
		unhandled("x:(r)+n,x0 y:(rh)-,y0");
		break;
		}
	case 1413: { // x:(r)+n,x0 y:(rh)-,y1
		unhandled("x:(r)+n,x0 y:(rh)-,y1");
		break;
		}
	case 1414: { // x:(r)+n,x0 y:(rh)-,a
		unhandled("x:(r)+n,x0 y:(rh)-,a");
		break;
		}
	case 1415: { // x:(r)+n,x0 y:(rh)-,b
		unhandled("x:(r)+n,x0 y:(rh)-,b");
		break;
		}
	case 1416: { // x:(r)+n,x1 y:(rh)-,y0
		unhandled("x:(r)+n,x1 y:(rh)-,y0");
		break;
		}
	case 1417: { // x:(r)+n,x1 y:(rh)-,y1
		unhandled("x:(r)+n,x1 y:(rh)-,y1");
		break;
		}
	case 1418: { // x:(r)+n,x1 y:(rh)-,a
		unhandled("x:(r)+n,x1 y:(rh)-,a");
		break;
		}
	case 1419: { // x:(r)+n,x1 y:(rh)-,b
		unhandled("x:(r)+n,x1 y:(rh)-,b");
		break;
		}
	case 1420: { // x:(r)+n,a y:(rh)-,y0
		unhandled("x:(r)+n,a y:(rh)-,y0");
		break;
		}
	case 1421: { // x:(r)+n,a y:(rh)-,y1
		unhandled("x:(r)+n,a y:(rh)-,y1");
		break;
		}
	case 1422: { // x:(r)+n,a y:(rh)-,a
		unhandled("x:(r)+n,a y:(rh)-,a");
		break;
		}
	case 1423: { // x:(r)+n,a y:(rh)-,b
		unhandled("x:(r)+n,a y:(rh)-,b");
		break;
		}
	case 1424: { // x:(r)+n,b y:(rh)-,y0
		unhandled("x:(r)+n,b y:(rh)-,y0");
		break;
		}
	case 1425: { // x:(r)+n,b y:(rh)-,y1
		unhandled("x:(r)+n,b y:(rh)-,y1");
		break;
		}
	case 1426: { // x:(r)+n,b y:(rh)-,a
		unhandled("x:(r)+n,b y:(rh)-,a");
		break;
		}
	case 1427: { // x:(r)+n,b y:(rh)-,b
		unhandled("x:(r)+n,b y:(rh)-,b");
		break;
		}
	case 1428: { // x:(r)+n,x0 y:(rh)+,y0
		unhandled("x:(r)+n,x0 y:(rh)+,y0");
		break;
		}
	case 1429: { // x:(r)+n,x0 y:(rh)+,y1
		unhandled("x:(r)+n,x0 y:(rh)+,y1");
		break;
		}
	case 1430: { // x:(r)+n,x0 y:(rh)+,a
		unhandled("x:(r)+n,x0 y:(rh)+,a");
		break;
		}
	case 1431: { // x:(r)+n,x0 y:(rh)+,b
		unhandled("x:(r)+n,x0 y:(rh)+,b");
		break;
		}
	case 1432: { // x:(r)+n,x1 y:(rh)+,y0
		unhandled("x:(r)+n,x1 y:(rh)+,y0");
		break;
		}
	case 1433: { // x:(r)+n,x1 y:(rh)+,y1
		unhandled("x:(r)+n,x1 y:(rh)+,y1");
		break;
		}
	case 1434: { // x:(r)+n,x1 y:(rh)+,a
		unhandled("x:(r)+n,x1 y:(rh)+,a");
		break;
		}
	case 1435: { // x:(r)+n,x1 y:(rh)+,b
		unhandled("x:(r)+n,x1 y:(rh)+,b");
		break;
		}
	case 1436: { // x:(r)+n,a y:(rh)+,y0
		unhandled("x:(r)+n,a y:(rh)+,y0");
		break;
		}
	case 1437: { // x:(r)+n,a y:(rh)+,y1
		unhandled("x:(r)+n,a y:(rh)+,y1");
		break;
		}
	case 1438: { // x:(r)+n,a y:(rh)+,a
		unhandled("x:(r)+n,a y:(rh)+,a");
		break;
		}
	case 1439: { // x:(r)+n,a y:(rh)+,b
		unhandled("x:(r)+n,a y:(rh)+,b");
		break;
		}
	case 1440: { // x:(r)+n,b y:(rh)+,y0
		unhandled("x:(r)+n,b y:(rh)+,y0");
		break;
		}
	case 1441: { // x:(r)+n,b y:(rh)+,y1
		unhandled("x:(r)+n,b y:(rh)+,y1");
		break;
		}
	case 1442: { // x:(r)+n,b y:(rh)+,a
		unhandled("x:(r)+n,b y:(rh)+,a");
		break;
		}
	case 1443: { // x:(r)+n,b y:(rh)+,b
		unhandled("x:(r)+n,b y:(rh)+,b");
		break;
		}
	case 1444: { // x:(r)+n,x0 y:(rh),y0
		unhandled("x:(r)+n,x0 y:(rh),y0");
		break;
		}
	case 1445: { // x:(r)+n,x0 y:(rh),y1
		unhandled("x:(r)+n,x0 y:(rh),y1");
		break;
		}
	case 1446: { // x:(r)+n,x0 y:(rh),a
		unhandled("x:(r)+n,x0 y:(rh),a");
		break;
		}
	case 1447: { // x:(r)+n,x0 y:(rh),b
		unhandled("x:(r)+n,x0 y:(rh),b");
		break;
		}
	case 1448: { // x:(r)+n,x1 y:(rh),y0
		unhandled("x:(r)+n,x1 y:(rh),y0");
		break;
		}
	case 1449: { // x:(r)+n,x1 y:(rh),y1
		unhandled("x:(r)+n,x1 y:(rh),y1");
		break;
		}
	case 1450: { // x:(r)+n,x1 y:(rh),a
		unhandled("x:(r)+n,x1 y:(rh),a");
		break;
		}
	case 1451: { // x:(r)+n,x1 y:(rh),b
		unhandled("x:(r)+n,x1 y:(rh),b");
		break;
		}
	case 1452: { // x:(r)+n,a y:(rh),y0
		unhandled("x:(r)+n,a y:(rh),y0");
		break;
		}
	case 1453: { // x:(r)+n,a y:(rh),y1
		unhandled("x:(r)+n,a y:(rh),y1");
		break;
		}
	case 1454: { // x:(r)+n,a y:(rh),a
		unhandled("x:(r)+n,a y:(rh),a");
		break;
		}
	case 1455: { // x:(r)+n,a y:(rh),b
		unhandled("x:(r)+n,a y:(rh),b");
		break;
		}
	case 1456: { // x:(r)+n,b y:(rh),y0
		unhandled("x:(r)+n,b y:(rh),y0");
		break;
		}
	case 1457: { // x:(r)+n,b y:(rh),y1
		unhandled("x:(r)+n,b y:(rh),y1");
		break;
		}
	case 1458: { // x:(r)+n,b y:(rh),a
		unhandled("x:(r)+n,b y:(rh),a");
		break;
		}
	case 1459: { // x:(r)+n,b y:(rh),b
		unhandled("x:(r)+n,b y:(rh),b");
		break;
		}
	case 1460: { // x:(r)-,x0 y:(rh)+n,y0
		unhandled("x:(r)-,x0 y:(rh)+n,y0");
		break;
		}
	case 1461: { // x:(r)-,x0 y:(rh)+n,y1
		unhandled("x:(r)-,x0 y:(rh)+n,y1");
		break;
		}
	case 1462: { // x:(r)-,x0 y:(rh)+n,a
		unhandled("x:(r)-,x0 y:(rh)+n,a");
		break;
		}
	case 1463: { // x:(r)-,x0 y:(rh)+n,b
		unhandled("x:(r)-,x0 y:(rh)+n,b");
		break;
		}
	case 1464: { // x:(r)-,x1 y:(rh)+n,y0
		unhandled("x:(r)-,x1 y:(rh)+n,y0");
		break;
		}
	case 1465: { // x:(r)-,x1 y:(rh)+n,y1
		unhandled("x:(r)-,x1 y:(rh)+n,y1");
		break;
		}
	case 1466: { // x:(r)-,x1 y:(rh)+n,a
		unhandled("x:(r)-,x1 y:(rh)+n,a");
		break;
		}
	case 1467: { // x:(r)-,x1 y:(rh)+n,b
		unhandled("x:(r)-,x1 y:(rh)+n,b");
		break;
		}
	case 1468: { // x:(r)-,a y:(rh)+n,y0
		unhandled("x:(r)-,a y:(rh)+n,y0");
		break;
		}
	case 1469: { // x:(r)-,a y:(rh)+n,y1
		unhandled("x:(r)-,a y:(rh)+n,y1");
		break;
		}
	case 1470: { // x:(r)-,a y:(rh)+n,a
		unhandled("x:(r)-,a y:(rh)+n,a");
		break;
		}
	case 1471: { // x:(r)-,a y:(rh)+n,b
		unhandled("x:(r)-,a y:(rh)+n,b");
		break;
		}
	case 1472: { // x:(r)-,b y:(rh)+n,y0
		unhandled("x:(r)-,b y:(rh)+n,y0");
		break;
		}
	case 1473: { // x:(r)-,b y:(rh)+n,y1
		unhandled("x:(r)-,b y:(rh)+n,y1");
		break;
		}
	case 1474: { // x:(r)-,b y:(rh)+n,a
		unhandled("x:(r)-,b y:(rh)+n,a");
		break;
		}
	case 1475: { // x:(r)-,b y:(rh)+n,b
		unhandled("x:(r)-,b y:(rh)+n,b");
		break;
		}
	case 1476: { // x:(r)-,x0 y:(rh)-,y0
		unhandled("x:(r)-,x0 y:(rh)-,y0");
		break;
		}
	case 1477: { // x:(r)-,x0 y:(rh)-,y1
		unhandled("x:(r)-,x0 y:(rh)-,y1");
		break;
		}
	case 1478: { // x:(r)-,x0 y:(rh)-,a
		unhandled("x:(r)-,x0 y:(rh)-,a");
		break;
		}
	case 1479: { // x:(r)-,x0 y:(rh)-,b
		unhandled("x:(r)-,x0 y:(rh)-,b");
		break;
		}
	case 1480: { // x:(r)-,x1 y:(rh)-,y0
		unhandled("x:(r)-,x1 y:(rh)-,y0");
		break;
		}
	case 1481: { // x:(r)-,x1 y:(rh)-,y1
		unhandled("x:(r)-,x1 y:(rh)-,y1");
		break;
		}
	case 1482: { // x:(r)-,x1 y:(rh)-,a
		unhandled("x:(r)-,x1 y:(rh)-,a");
		break;
		}
	case 1483: { // x:(r)-,x1 y:(rh)-,b
		unhandled("x:(r)-,x1 y:(rh)-,b");
		break;
		}
	case 1484: { // x:(r)-,a y:(rh)-,y0
		unhandled("x:(r)-,a y:(rh)-,y0");
		break;
		}
	case 1485: { // x:(r)-,a y:(rh)-,y1
		unhandled("x:(r)-,a y:(rh)-,y1");
		break;
		}
	case 1486: { // x:(r)-,a y:(rh)-,a
		unhandled("x:(r)-,a y:(rh)-,a");
		break;
		}
	case 1487: { // x:(r)-,a y:(rh)-,b
		unhandled("x:(r)-,a y:(rh)-,b");
		break;
		}
	case 1488: { // x:(r)-,b y:(rh)-,y0
		unhandled("x:(r)-,b y:(rh)-,y0");
		break;
		}
	case 1489: { // x:(r)-,b y:(rh)-,y1
		unhandled("x:(r)-,b y:(rh)-,y1");
		break;
		}
	case 1490: { // x:(r)-,b y:(rh)-,a
		unhandled("x:(r)-,b y:(rh)-,a");
		break;
		}
	case 1491: { // x:(r)-,b y:(rh)-,b
		unhandled("x:(r)-,b y:(rh)-,b");
		break;
		}
	case 1492: { // x:(r)-,x0 y:(rh)+,y0
		unhandled("x:(r)-,x0 y:(rh)+,y0");
		break;
		}
	case 1493: { // x:(r)-,x0 y:(rh)+,y1
		unhandled("x:(r)-,x0 y:(rh)+,y1");
		break;
		}
	case 1494: { // x:(r)-,x0 y:(rh)+,a
		unhandled("x:(r)-,x0 y:(rh)+,a");
		break;
		}
	case 1495: { // x:(r)-,x0 y:(rh)+,b
		unhandled("x:(r)-,x0 y:(rh)+,b");
		break;
		}
	case 1496: { // x:(r)-,x1 y:(rh)+,y0
		unhandled("x:(r)-,x1 y:(rh)+,y0");
		break;
		}
	case 1497: { // x:(r)-,x1 y:(rh)+,y1
		unhandled("x:(r)-,x1 y:(rh)+,y1");
		break;
		}
	case 1498: { // x:(r)-,x1 y:(rh)+,a
		unhandled("x:(r)-,x1 y:(rh)+,a");
		break;
		}
	case 1499: { // x:(r)-,x1 y:(rh)+,b
		unhandled("x:(r)-,x1 y:(rh)+,b");
		break;
		}
	case 1500: { // x:(r)-,a y:(rh)+,y0
		unhandled("x:(r)-,a y:(rh)+,y0");
		break;
		}
	case 1501: { // x:(r)-,a y:(rh)+,y1
		unhandled("x:(r)-,a y:(rh)+,y1");
		break;
		}
	case 1502: { // x:(r)-,a y:(rh)+,a
		unhandled("x:(r)-,a y:(rh)+,a");
		break;
		}
	case 1503: { // x:(r)-,a y:(rh)+,b
		unhandled("x:(r)-,a y:(rh)+,b");
		break;
		}
	case 1504: { // x:(r)-,b y:(rh)+,y0
		unhandled("x:(r)-,b y:(rh)+,y0");
		break;
		}
	case 1505: { // x:(r)-,b y:(rh)+,y1
		unhandled("x:(r)-,b y:(rh)+,y1");
		break;
		}
	case 1506: { // x:(r)-,b y:(rh)+,a
		unhandled("x:(r)-,b y:(rh)+,a");
		break;
		}
	case 1507: { // x:(r)-,b y:(rh)+,b
		unhandled("x:(r)-,b y:(rh)+,b");
		break;
		}
	case 1508: { // x:(r)-,x0 y:(rh),y0
		unhandled("x:(r)-,x0 y:(rh),y0");
		break;
		}
	case 1509: { // x:(r)-,x0 y:(rh),y1
		unhandled("x:(r)-,x0 y:(rh),y1");
		break;
		}
	case 1510: { // x:(r)-,x0 y:(rh),a
		unhandled("x:(r)-,x0 y:(rh),a");
		break;
		}
	case 1511: { // x:(r)-,x0 y:(rh),b
		unhandled("x:(r)-,x0 y:(rh),b");
		break;
		}
	case 1512: { // x:(r)-,x1 y:(rh),y0
		unhandled("x:(r)-,x1 y:(rh),y0");
		break;
		}
	case 1513: { // x:(r)-,x1 y:(rh),y1
		unhandled("x:(r)-,x1 y:(rh),y1");
		break;
		}
	case 1514: { // x:(r)-,x1 y:(rh),a
		unhandled("x:(r)-,x1 y:(rh),a");
		break;
		}
	case 1515: { // x:(r)-,x1 y:(rh),b
		unhandled("x:(r)-,x1 y:(rh),b");
		break;
		}
	case 1516: { // x:(r)-,a y:(rh),y0
		unhandled("x:(r)-,a y:(rh),y0");
		break;
		}
	case 1517: { // x:(r)-,a y:(rh),y1
		unhandled("x:(r)-,a y:(rh),y1");
		break;
		}
	case 1518: { // x:(r)-,a y:(rh),a
		unhandled("x:(r)-,a y:(rh),a");
		break;
		}
	case 1519: { // x:(r)-,a y:(rh),b
		unhandled("x:(r)-,a y:(rh),b");
		break;
		}
	case 1520: { // x:(r)-,b y:(rh),y0
		unhandled("x:(r)-,b y:(rh),y0");
		break;
		}
	case 1521: { // x:(r)-,b y:(rh),y1
		unhandled("x:(r)-,b y:(rh),y1");
		break;
		}
	case 1522: { // x:(r)-,b y:(rh),a
		unhandled("x:(r)-,b y:(rh),a");
		break;
		}
	case 1523: { // x:(r)-,b y:(rh),b
		unhandled("x:(r)-,b y:(rh),b");
		break;
		}
	case 1524: { // x:(r)+,x0 y:(rh)+n,y0
		unhandled("x:(r)+,x0 y:(rh)+n,y0");
		break;
		}
	case 1525: { // x:(r)+,x0 y:(rh)+n,y1
		unhandled("x:(r)+,x0 y:(rh)+n,y1");
		break;
		}
	case 1526: { // x:(r)+,x0 y:(rh)+n,a
		unhandled("x:(r)+,x0 y:(rh)+n,a");
		break;
		}
	case 1527: { // x:(r)+,x0 y:(rh)+n,b
		unhandled("x:(r)+,x0 y:(rh)+n,b");
		break;
		}
	case 1528: { // x:(r)+,x1 y:(rh)+n,y0
		unhandled("x:(r)+,x1 y:(rh)+n,y0");
		break;
		}
	case 1529: { // x:(r)+,x1 y:(rh)+n,y1
		unhandled("x:(r)+,x1 y:(rh)+n,y1");
		break;
		}
	case 1530: { // x:(r)+,x1 y:(rh)+n,a
		unhandled("x:(r)+,x1 y:(rh)+n,a");
		break;
		}
	case 1531: { // x:(r)+,x1 y:(rh)+n,b
		unhandled("x:(r)+,x1 y:(rh)+n,b");
		break;
		}
	case 1532: { // x:(r)+,a y:(rh)+n,y0
		unhandled("x:(r)+,a y:(rh)+n,y0");
		break;
		}
	case 1533: { // x:(r)+,a y:(rh)+n,y1
		unhandled("x:(r)+,a y:(rh)+n,y1");
		break;
		}
	case 1534: { // x:(r)+,a y:(rh)+n,a
		unhandled("x:(r)+,a y:(rh)+n,a");
		break;
		}
	case 1535: { // x:(r)+,a y:(rh)+n,b
		unhandled("x:(r)+,a y:(rh)+n,b");
		break;
		}
	case 1536: { // x:(r)+,b y:(rh)+n,y0
		unhandled("x:(r)+,b y:(rh)+n,y0");
		break;
		}
	case 1537: { // x:(r)+,b y:(rh)+n,y1
		unhandled("x:(r)+,b y:(rh)+n,y1");
		break;
		}
	case 1538: { // x:(r)+,b y:(rh)+n,a
		unhandled("x:(r)+,b y:(rh)+n,a");
		break;
		}
	case 1539: { // x:(r)+,b y:(rh)+n,b
		unhandled("x:(r)+,b y:(rh)+n,b");
		break;
		}
	case 1540: { // x:(r)+,x0 y:(rh)-,y0
		unhandled("x:(r)+,x0 y:(rh)-,y0");
		break;
		}
	case 1541: { // x:(r)+,x0 y:(rh)-,y1
		unhandled("x:(r)+,x0 y:(rh)-,y1");
		break;
		}
	case 1542: { // x:(r)+,x0 y:(rh)-,a
		unhandled("x:(r)+,x0 y:(rh)-,a");
		break;
		}
	case 1543: { // x:(r)+,x0 y:(rh)-,b
		unhandled("x:(r)+,x0 y:(rh)-,b");
		break;
		}
	case 1544: { // x:(r)+,x1 y:(rh)-,y0
		unhandled("x:(r)+,x1 y:(rh)-,y0");
		break;
		}
	case 1545: { // x:(r)+,x1 y:(rh)-,y1
		unhandled("x:(r)+,x1 y:(rh)-,y1");
		break;
		}
	case 1546: { // x:(r)+,x1 y:(rh)-,a
		unhandled("x:(r)+,x1 y:(rh)-,a");
		break;
		}
	case 1547: { // x:(r)+,x1 y:(rh)-,b
		unhandled("x:(r)+,x1 y:(rh)-,b");
		break;
		}
	case 1548: { // x:(r)+,a y:(rh)-,y0
		unhandled("x:(r)+,a y:(rh)-,y0");
		break;
		}
	case 1549: { // x:(r)+,a y:(rh)-,y1
		unhandled("x:(r)+,a y:(rh)-,y1");
		break;
		}
	case 1550: { // x:(r)+,a y:(rh)-,a
		unhandled("x:(r)+,a y:(rh)-,a");
		break;
		}
	case 1551: { // x:(r)+,a y:(rh)-,b
		unhandled("x:(r)+,a y:(rh)-,b");
		break;
		}
	case 1552: { // x:(r)+,b y:(rh)-,y0
		unhandled("x:(r)+,b y:(rh)-,y0");
		break;
		}
	case 1553: { // x:(r)+,b y:(rh)-,y1
		unhandled("x:(r)+,b y:(rh)-,y1");
		break;
		}
	case 1554: { // x:(r)+,b y:(rh)-,a
		unhandled("x:(r)+,b y:(rh)-,a");
		break;
		}
	case 1555: { // x:(r)+,b y:(rh)-,b
		unhandled("x:(r)+,b y:(rh)-,b");
		break;
		}
	case 1556: { // x:(r)+,x0 y:(rh)+,y0
		unhandled("x:(r)+,x0 y:(rh)+,y0");
		break;
		}
	case 1557: { // x:(r)+,x0 y:(rh)+,y1
		unhandled("x:(r)+,x0 y:(rh)+,y1");
		break;
		}
	case 1558: { // x:(r)+,x0 y:(rh)+,a
		unhandled("x:(r)+,x0 y:(rh)+,a");
		break;
		}
	case 1559: { // x:(r)+,x0 y:(rh)+,b
		unhandled("x:(r)+,x0 y:(rh)+,b");
		break;
		}
	case 1560: { // x:(r)+,x1 y:(rh)+,y0
		unhandled("x:(r)+,x1 y:(rh)+,y0");
		break;
		}
	case 1561: { // x:(r)+,x1 y:(rh)+,y1
		unhandled("x:(r)+,x1 y:(rh)+,y1");
		break;
		}
	case 1562: { // x:(r)+,x1 y:(rh)+,a
		unhandled("x:(r)+,x1 y:(rh)+,a");
		break;
		}
	case 1563: { // x:(r)+,x1 y:(rh)+,b
		unhandled("x:(r)+,x1 y:(rh)+,b");
		break;
		}
	case 1564: { // x:(r)+,a y:(rh)+,y0
		unhandled("x:(r)+,a y:(rh)+,y0");
		break;
		}
	case 1565: { // x:(r)+,a y:(rh)+,y1
		unhandled("x:(r)+,a y:(rh)+,y1");
		break;
		}
	case 1566: { // x:(r)+,a y:(rh)+,a
		unhandled("x:(r)+,a y:(rh)+,a");
		break;
		}
	case 1567: { // x:(r)+,a y:(rh)+,b
		unhandled("x:(r)+,a y:(rh)+,b");
		break;
		}
	case 1568: { // x:(r)+,b y:(rh)+,y0
		unhandled("x:(r)+,b y:(rh)+,y0");
		break;
		}
	case 1569: { // x:(r)+,b y:(rh)+,y1
		unhandled("x:(r)+,b y:(rh)+,y1");
		break;
		}
	case 1570: { // x:(r)+,b y:(rh)+,a
		unhandled("x:(r)+,b y:(rh)+,a");
		break;
		}
	case 1571: { // x:(r)+,b y:(rh)+,b
		unhandled("x:(r)+,b y:(rh)+,b");
		break;
		}
	case 1572: { // x:(r)+,x0 y:(rh),y0
		unhandled("x:(r)+,x0 y:(rh),y0");
		break;
		}
	case 1573: { // x:(r)+,x0 y:(rh),y1
		unhandled("x:(r)+,x0 y:(rh),y1");
		break;
		}
	case 1574: { // x:(r)+,x0 y:(rh),a
		unhandled("x:(r)+,x0 y:(rh),a");
		break;
		}
	case 1575: { // x:(r)+,x0 y:(rh),b
		unhandled("x:(r)+,x0 y:(rh),b");
		break;
		}
	case 1576: { // x:(r)+,x1 y:(rh),y0
		unhandled("x:(r)+,x1 y:(rh),y0");
		break;
		}
	case 1577: { // x:(r)+,x1 y:(rh),y1
		unhandled("x:(r)+,x1 y:(rh),y1");
		break;
		}
	case 1578: { // x:(r)+,x1 y:(rh),a
		unhandled("x:(r)+,x1 y:(rh),a");
		break;
		}
	case 1579: { // x:(r)+,x1 y:(rh),b
		unhandled("x:(r)+,x1 y:(rh),b");
		break;
		}
	case 1580: { // x:(r)+,a y:(rh),y0
		unhandled("x:(r)+,a y:(rh),y0");
		break;
		}
	case 1581: { // x:(r)+,a y:(rh),y1
		unhandled("x:(r)+,a y:(rh),y1");
		break;
		}
	case 1582: { // x:(r)+,a y:(rh),a
		unhandled("x:(r)+,a y:(rh),a");
		break;
		}
	case 1583: { // x:(r)+,a y:(rh),b
		unhandled("x:(r)+,a y:(rh),b");
		break;
		}
	case 1584: { // x:(r)+,b y:(rh),y0
		unhandled("x:(r)+,b y:(rh),y0");
		break;
		}
	case 1585: { // x:(r)+,b y:(rh),y1
		unhandled("x:(r)+,b y:(rh),y1");
		break;
		}
	case 1586: { // x:(r)+,b y:(rh),a
		unhandled("x:(r)+,b y:(rh),a");
		break;
		}
	case 1587: { // x:(r)+,b y:(rh),b
		unhandled("x:(r)+,b y:(rh),b");
		break;
		}
	case 1588: { // x:(r),x0 y:(rh)+n,y0
		unhandled("x:(r),x0 y:(rh)+n,y0");
		break;
		}
	case 1589: { // x:(r),x0 y:(rh)+n,y1
		unhandled("x:(r),x0 y:(rh)+n,y1");
		break;
		}
	case 1590: { // x:(r),x0 y:(rh)+n,a
		unhandled("x:(r),x0 y:(rh)+n,a");
		break;
		}
	case 1591: { // x:(r),x0 y:(rh)+n,b
		unhandled("x:(r),x0 y:(rh)+n,b");
		break;
		}
	case 1592: { // x:(r),x1 y:(rh)+n,y0
		unhandled("x:(r),x1 y:(rh)+n,y0");
		break;
		}
	case 1593: { // x:(r),x1 y:(rh)+n,y1
		unhandled("x:(r),x1 y:(rh)+n,y1");
		break;
		}
	case 1594: { // x:(r),x1 y:(rh)+n,a
		unhandled("x:(r),x1 y:(rh)+n,a");
		break;
		}
	case 1595: { // x:(r),x1 y:(rh)+n,b
		unhandled("x:(r),x1 y:(rh)+n,b");
		break;
		}
	case 1596: { // x:(r),a y:(rh)+n,y0
		unhandled("x:(r),a y:(rh)+n,y0");
		break;
		}
	case 1597: { // x:(r),a y:(rh)+n,y1
		unhandled("x:(r),a y:(rh)+n,y1");
		break;
		}
	case 1598: { // x:(r),a y:(rh)+n,a
		unhandled("x:(r),a y:(rh)+n,a");
		break;
		}
	case 1599: { // x:(r),a y:(rh)+n,b
		unhandled("x:(r),a y:(rh)+n,b");
		break;
		}
	case 1600: { // x:(r),b y:(rh)+n,y0
		unhandled("x:(r),b y:(rh)+n,y0");
		break;
		}
	case 1601: { // x:(r),b y:(rh)+n,y1
		unhandled("x:(r),b y:(rh)+n,y1");
		break;
		}
	case 1602: { // x:(r),b y:(rh)+n,a
		unhandled("x:(r),b y:(rh)+n,a");
		break;
		}
	case 1603: { // x:(r),b y:(rh)+n,b
		unhandled("x:(r),b y:(rh)+n,b");
		break;
		}
	case 1604: { // x:(r),x0 y:(rh)-,y0
		unhandled("x:(r),x0 y:(rh)-,y0");
		break;
		}
	case 1605: { // x:(r),x0 y:(rh)-,y1
		unhandled("x:(r),x0 y:(rh)-,y1");
		break;
		}
	case 1606: { // x:(r),x0 y:(rh)-,a
		unhandled("x:(r),x0 y:(rh)-,a");
		break;
		}
	case 1607: { // x:(r),x0 y:(rh)-,b
		unhandled("x:(r),x0 y:(rh)-,b");
		break;
		}
	case 1608: { // x:(r),x1 y:(rh)-,y0
		unhandled("x:(r),x1 y:(rh)-,y0");
		break;
		}
	case 1609: { // x:(r),x1 y:(rh)-,y1
		unhandled("x:(r),x1 y:(rh)-,y1");
		break;
		}
	case 1610: { // x:(r),x1 y:(rh)-,a
		unhandled("x:(r),x1 y:(rh)-,a");
		break;
		}
	case 1611: { // x:(r),x1 y:(rh)-,b
		unhandled("x:(r),x1 y:(rh)-,b");
		break;
		}
	case 1612: { // x:(r),a y:(rh)-,y0
		unhandled("x:(r),a y:(rh)-,y0");
		break;
		}
	case 1613: { // x:(r),a y:(rh)-,y1
		unhandled("x:(r),a y:(rh)-,y1");
		break;
		}
	case 1614: { // x:(r),a y:(rh)-,a
		unhandled("x:(r),a y:(rh)-,a");
		break;
		}
	case 1615: { // x:(r),a y:(rh)-,b
		unhandled("x:(r),a y:(rh)-,b");
		break;
		}
	case 1616: { // x:(r),b y:(rh)-,y0
		unhandled("x:(r),b y:(rh)-,y0");
		break;
		}
	case 1617: { // x:(r),b y:(rh)-,y1
		unhandled("x:(r),b y:(rh)-,y1");
		break;
		}
	case 1618: { // x:(r),b y:(rh)-,a
		unhandled("x:(r),b y:(rh)-,a");
		break;
		}
	case 1619: { // x:(r),b y:(rh)-,b
		unhandled("x:(r),b y:(rh)-,b");
		break;
		}
	case 1620: { // x:(r),x0 y:(rh)+,y0
		unhandled("x:(r),x0 y:(rh)+,y0");
		break;
		}
	case 1621: { // x:(r),x0 y:(rh)+,y1
		unhandled("x:(r),x0 y:(rh)+,y1");
		break;
		}
	case 1622: { // x:(r),x0 y:(rh)+,a
		unhandled("x:(r),x0 y:(rh)+,a");
		break;
		}
	case 1623: { // x:(r),x0 y:(rh)+,b
		unhandled("x:(r),x0 y:(rh)+,b");
		break;
		}
	case 1624: { // x:(r),x1 y:(rh)+,y0
		unhandled("x:(r),x1 y:(rh)+,y0");
		break;
		}
	case 1625: { // x:(r),x1 y:(rh)+,y1
		unhandled("x:(r),x1 y:(rh)+,y1");
		break;
		}
	case 1626: { // x:(r),x1 y:(rh)+,a
		unhandled("x:(r),x1 y:(rh)+,a");
		break;
		}
	case 1627: { // x:(r),x1 y:(rh)+,b
		unhandled("x:(r),x1 y:(rh)+,b");
		break;
		}
	case 1628: { // x:(r),a y:(rh)+,y0
		unhandled("x:(r),a y:(rh)+,y0");
		break;
		}
	case 1629: { // x:(r),a y:(rh)+,y1
		unhandled("x:(r),a y:(rh)+,y1");
		break;
		}
	case 1630: { // x:(r),a y:(rh)+,a
		unhandled("x:(r),a y:(rh)+,a");
		break;
		}
	case 1631: { // x:(r),a y:(rh)+,b
		unhandled("x:(r),a y:(rh)+,b");
		break;
		}
	case 1632: { // x:(r),b y:(rh)+,y0
		unhandled("x:(r),b y:(rh)+,y0");
		break;
		}
	case 1633: { // x:(r),b y:(rh)+,y1
		unhandled("x:(r),b y:(rh)+,y1");
		break;
		}
	case 1634: { // x:(r),b y:(rh)+,a
		unhandled("x:(r),b y:(rh)+,a");
		break;
		}
	case 1635: { // x:(r),b y:(rh)+,b
		unhandled("x:(r),b y:(rh)+,b");
		break;
		}
	case 1636: { // x:(r),x0 y:(rh),y0
		unhandled("x:(r),x0 y:(rh),y0");
		break;
		}
	case 1637: { // x:(r),x0 y:(rh),y1
		unhandled("x:(r),x0 y:(rh),y1");
		break;
		}
	case 1638: { // x:(r),x0 y:(rh),a
		unhandled("x:(r),x0 y:(rh),a");
		break;
		}
	case 1639: { // x:(r),x0 y:(rh),b
		unhandled("x:(r),x0 y:(rh),b");
		break;
		}
	case 1640: { // x:(r),x1 y:(rh),y0
		unhandled("x:(r),x1 y:(rh),y0");
		break;
		}
	case 1641: { // x:(r),x1 y:(rh),y1
		unhandled("x:(r),x1 y:(rh),y1");
		break;
		}
	case 1642: { // x:(r),x1 y:(rh),a
		unhandled("x:(r),x1 y:(rh),a");
		break;
		}
	case 1643: { // x:(r),x1 y:(rh),b
		unhandled("x:(r),x1 y:(rh),b");
		break;
		}
	case 1644: { // x:(r),a y:(rh),y0
		unhandled("x:(r),a y:(rh),y0");
		break;
		}
	case 1645: { // x:(r),a y:(rh),y1
		unhandled("x:(r),a y:(rh),y1");
		break;
		}
	case 1646: { // x:(r),a y:(rh),a
		unhandled("x:(r),a y:(rh),a");
		break;
		}
	case 1647: { // x:(r),a y:(rh),b
		unhandled("x:(r),a y:(rh),b");
		break;
		}
	case 1648: { // x:(r),b y:(rh),y0
		unhandled("x:(r),b y:(rh),y0");
		break;
		}
	case 1649: { // x:(r),b y:(rh),y1
		unhandled("x:(r),b y:(rh),y1");
		break;
		}
	case 1650: { // x:(r),b y:(rh),a
		unhandled("x:(r),b y:(rh),a");
		break;
		}
	case 1651: { // x:(r),b y:(rh),b
		unhandled("x:(r),b y:(rh),b");
		break;
		}
	case 1652: { // x:(r)+n,x0 y0,y:(rh)+n
		unhandled("x:(r)+n,x0 y0,y:(rh)+n");
		break;
		}
	case 1653: { // x:(r)+n,x0 y1,y:(rh)+n
		unhandled("x:(r)+n,x0 y1,y:(rh)+n");
		break;
		}
	case 1654: { // x:(r)+n,x0 a,y:(rh)+n
		unhandled("x:(r)+n,x0 a,y:(rh)+n");
		break;
		}
	case 1655: { // x:(r)+n,x0 b,y:(rh)+n
		unhandled("x:(r)+n,x0 b,y:(rh)+n");
		break;
		}
	case 1656: { // x:(r)+n,x1 y0,y:(rh)+n
		unhandled("x:(r)+n,x1 y0,y:(rh)+n");
		break;
		}
	case 1657: { // x:(r)+n,x1 y1,y:(rh)+n
		unhandled("x:(r)+n,x1 y1,y:(rh)+n");
		break;
		}
	case 1658: { // x:(r)+n,x1 a,y:(rh)+n
		unhandled("x:(r)+n,x1 a,y:(rh)+n");
		break;
		}
	case 1659: { // x:(r)+n,x1 b,y:(rh)+n
		unhandled("x:(r)+n,x1 b,y:(rh)+n");
		break;
		}
	case 1660: { // x:(r)+n,a y0,y:(rh)+n
		unhandled("x:(r)+n,a y0,y:(rh)+n");
		break;
		}
	case 1661: { // x:(r)+n,a y1,y:(rh)+n
		unhandled("x:(r)+n,a y1,y:(rh)+n");
		break;
		}
	case 1662: { // x:(r)+n,a a,y:(rh)+n
		unhandled("x:(r)+n,a a,y:(rh)+n");
		break;
		}
	case 1663: { // x:(r)+n,a b,y:(rh)+n
		unhandled("x:(r)+n,a b,y:(rh)+n");
		break;
		}
	case 1664: { // x:(r)+n,b y0,y:(rh)+n
		unhandled("x:(r)+n,b y0,y:(rh)+n");
		break;
		}
	case 1665: { // x:(r)+n,b y1,y:(rh)+n
		unhandled("x:(r)+n,b y1,y:(rh)+n");
		break;
		}
	case 1666: { // x:(r)+n,b a,y:(rh)+n
		unhandled("x:(r)+n,b a,y:(rh)+n");
		break;
		}
	case 1667: { // x:(r)+n,b b,y:(rh)+n
		unhandled("x:(r)+n,b b,y:(rh)+n");
		break;
		}
	case 1668: { // x:(r)+n,x0 y0,y:(rh)-
		unhandled("x:(r)+n,x0 y0,y:(rh)-");
		break;
		}
	case 1669: { // x:(r)+n,x0 y1,y:(rh)-
		unhandled("x:(r)+n,x0 y1,y:(rh)-");
		break;
		}
	case 1670: { // x:(r)+n,x0 a,y:(rh)-
		unhandled("x:(r)+n,x0 a,y:(rh)-");
		break;
		}
	case 1671: { // x:(r)+n,x0 b,y:(rh)-
		unhandled("x:(r)+n,x0 b,y:(rh)-");
		break;
		}
	case 1672: { // x:(r)+n,x1 y0,y:(rh)-
		unhandled("x:(r)+n,x1 y0,y:(rh)-");
		break;
		}
	case 1673: { // x:(r)+n,x1 y1,y:(rh)-
		unhandled("x:(r)+n,x1 y1,y:(rh)-");
		break;
		}
	case 1674: { // x:(r)+n,x1 a,y:(rh)-
		unhandled("x:(r)+n,x1 a,y:(rh)-");
		break;
		}
	case 1675: { // x:(r)+n,x1 b,y:(rh)-
		unhandled("x:(r)+n,x1 b,y:(rh)-");
		break;
		}
	case 1676: { // x:(r)+n,a y0,y:(rh)-
		unhandled("x:(r)+n,a y0,y:(rh)-");
		break;
		}
	case 1677: { // x:(r)+n,a y1,y:(rh)-
		unhandled("x:(r)+n,a y1,y:(rh)-");
		break;
		}
	case 1678: { // x:(r)+n,a a,y:(rh)-
		unhandled("x:(r)+n,a a,y:(rh)-");
		break;
		}
	case 1679: { // x:(r)+n,a b,y:(rh)-
		unhandled("x:(r)+n,a b,y:(rh)-");
		break;
		}
	case 1680: { // x:(r)+n,b y0,y:(rh)-
		unhandled("x:(r)+n,b y0,y:(rh)-");
		break;
		}
	case 1681: { // x:(r)+n,b y1,y:(rh)-
		unhandled("x:(r)+n,b y1,y:(rh)-");
		break;
		}
	case 1682: { // x:(r)+n,b a,y:(rh)-
		unhandled("x:(r)+n,b a,y:(rh)-");
		break;
		}
	case 1683: { // x:(r)+n,b b,y:(rh)-
		unhandled("x:(r)+n,b b,y:(rh)-");
		break;
		}
	case 1684: { // x:(r)+n,x0 y0,y:(rh)+
		unhandled("x:(r)+n,x0 y0,y:(rh)+");
		break;
		}
	case 1685: { // x:(r)+n,x0 y1,y:(rh)+
		unhandled("x:(r)+n,x0 y1,y:(rh)+");
		break;
		}
	case 1686: { // x:(r)+n,x0 a,y:(rh)+
		unhandled("x:(r)+n,x0 a,y:(rh)+");
		break;
		}
	case 1687: { // x:(r)+n,x0 b,y:(rh)+
		unhandled("x:(r)+n,x0 b,y:(rh)+");
		break;
		}
	case 1688: { // x:(r)+n,x1 y0,y:(rh)+
		unhandled("x:(r)+n,x1 y0,y:(rh)+");
		break;
		}
	case 1689: { // x:(r)+n,x1 y1,y:(rh)+
		unhandled("x:(r)+n,x1 y1,y:(rh)+");
		break;
		}
	case 1690: { // x:(r)+n,x1 a,y:(rh)+
		unhandled("x:(r)+n,x1 a,y:(rh)+");
		break;
		}
	case 1691: { // x:(r)+n,x1 b,y:(rh)+
		unhandled("x:(r)+n,x1 b,y:(rh)+");
		break;
		}
	case 1692: { // x:(r)+n,a y0,y:(rh)+
		unhandled("x:(r)+n,a y0,y:(rh)+");
		break;
		}
	case 1693: { // x:(r)+n,a y1,y:(rh)+
		unhandled("x:(r)+n,a y1,y:(rh)+");
		break;
		}
	case 1694: { // x:(r)+n,a a,y:(rh)+
		unhandled("x:(r)+n,a a,y:(rh)+");
		break;
		}
	case 1695: { // x:(r)+n,a b,y:(rh)+
		unhandled("x:(r)+n,a b,y:(rh)+");
		break;
		}
	case 1696: { // x:(r)+n,b y0,y:(rh)+
		unhandled("x:(r)+n,b y0,y:(rh)+");
		break;
		}
	case 1697: { // x:(r)+n,b y1,y:(rh)+
		unhandled("x:(r)+n,b y1,y:(rh)+");
		break;
		}
	case 1698: { // x:(r)+n,b a,y:(rh)+
		unhandled("x:(r)+n,b a,y:(rh)+");
		break;
		}
	case 1699: { // x:(r)+n,b b,y:(rh)+
		unhandled("x:(r)+n,b b,y:(rh)+");
		break;
		}
	case 1700: { // x:(r)+n,x0 y0,y:(rh)
		unhandled("x:(r)+n,x0 y0,y:(rh)");
		break;
		}
	case 1701: { // x:(r)+n,x0 y1,y:(rh)
		unhandled("x:(r)+n,x0 y1,y:(rh)");
		break;
		}
	case 1702: { // x:(r)+n,x0 a,y:(rh)
		unhandled("x:(r)+n,x0 a,y:(rh)");
		break;
		}
	case 1703: { // x:(r)+n,x0 b,y:(rh)
		unhandled("x:(r)+n,x0 b,y:(rh)");
		break;
		}
	case 1704: { // x:(r)+n,x1 y0,y:(rh)
		unhandled("x:(r)+n,x1 y0,y:(rh)");
		break;
		}
	case 1705: { // x:(r)+n,x1 y1,y:(rh)
		unhandled("x:(r)+n,x1 y1,y:(rh)");
		break;
		}
	case 1706: { // x:(r)+n,x1 a,y:(rh)
		unhandled("x:(r)+n,x1 a,y:(rh)");
		break;
		}
	case 1707: { // x:(r)+n,x1 b,y:(rh)
		unhandled("x:(r)+n,x1 b,y:(rh)");
		break;
		}
	case 1708: { // x:(r)+n,a y0,y:(rh)
		unhandled("x:(r)+n,a y0,y:(rh)");
		break;
		}
	case 1709: { // x:(r)+n,a y1,y:(rh)
		unhandled("x:(r)+n,a y1,y:(rh)");
		break;
		}
	case 1710: { // x:(r)+n,a a,y:(rh)
		unhandled("x:(r)+n,a a,y:(rh)");
		break;
		}
	case 1711: { // x:(r)+n,a b,y:(rh)
		unhandled("x:(r)+n,a b,y:(rh)");
		break;
		}
	case 1712: { // x:(r)+n,b y0,y:(rh)
		unhandled("x:(r)+n,b y0,y:(rh)");
		break;
		}
	case 1713: { // x:(r)+n,b y1,y:(rh)
		unhandled("x:(r)+n,b y1,y:(rh)");
		break;
		}
	case 1714: { // x:(r)+n,b a,y:(rh)
		unhandled("x:(r)+n,b a,y:(rh)");
		break;
		}
	case 1715: { // x:(r)+n,b b,y:(rh)
		unhandled("x:(r)+n,b b,y:(rh)");
		break;
		}
	case 1716: { // x:(r)-,x0 y0,y:(rh)+n
		unhandled("x:(r)-,x0 y0,y:(rh)+n");
		break;
		}
	case 1717: { // x:(r)-,x0 y1,y:(rh)+n
		unhandled("x:(r)-,x0 y1,y:(rh)+n");
		break;
		}
	case 1718: { // x:(r)-,x0 a,y:(rh)+n
		unhandled("x:(r)-,x0 a,y:(rh)+n");
		break;
		}
	case 1719: { // x:(r)-,x0 b,y:(rh)+n
		unhandled("x:(r)-,x0 b,y:(rh)+n");
		break;
		}
	case 1720: { // x:(r)-,x1 y0,y:(rh)+n
		unhandled("x:(r)-,x1 y0,y:(rh)+n");
		break;
		}
	case 1721: { // x:(r)-,x1 y1,y:(rh)+n
		unhandled("x:(r)-,x1 y1,y:(rh)+n");
		break;
		}
	case 1722: { // x:(r)-,x1 a,y:(rh)+n
		unhandled("x:(r)-,x1 a,y:(rh)+n");
		break;
		}
	case 1723: { // x:(r)-,x1 b,y:(rh)+n
		unhandled("x:(r)-,x1 b,y:(rh)+n");
		break;
		}
	case 1724: { // x:(r)-,a y0,y:(rh)+n
		unhandled("x:(r)-,a y0,y:(rh)+n");
		break;
		}
	case 1725: { // x:(r)-,a y1,y:(rh)+n
		unhandled("x:(r)-,a y1,y:(rh)+n");
		break;
		}
	case 1726: { // x:(r)-,a a,y:(rh)+n
		unhandled("x:(r)-,a a,y:(rh)+n");
		break;
		}
	case 1727: { // x:(r)-,a b,y:(rh)+n
		unhandled("x:(r)-,a b,y:(rh)+n");
		break;
		}
	case 1728: { // x:(r)-,b y0,y:(rh)+n
		unhandled("x:(r)-,b y0,y:(rh)+n");
		break;
		}
	case 1729: { // x:(r)-,b y1,y:(rh)+n
		unhandled("x:(r)-,b y1,y:(rh)+n");
		break;
		}
	case 1730: { // x:(r)-,b a,y:(rh)+n
		unhandled("x:(r)-,b a,y:(rh)+n");
		break;
		}
	case 1731: { // x:(r)-,b b,y:(rh)+n
		unhandled("x:(r)-,b b,y:(rh)+n");
		break;
		}
	case 1732: { // x:(r)-,x0 y0,y:(rh)-
		unhandled("x:(r)-,x0 y0,y:(rh)-");
		break;
		}
	case 1733: { // x:(r)-,x0 y1,y:(rh)-
		unhandled("x:(r)-,x0 y1,y:(rh)-");
		break;
		}
	case 1734: { // x:(r)-,x0 a,y:(rh)-
		unhandled("x:(r)-,x0 a,y:(rh)-");
		break;
		}
	case 1735: { // x:(r)-,x0 b,y:(rh)-
		unhandled("x:(r)-,x0 b,y:(rh)-");
		break;
		}
	case 1736: { // x:(r)-,x1 y0,y:(rh)-
		unhandled("x:(r)-,x1 y0,y:(rh)-");
		break;
		}
	case 1737: { // x:(r)-,x1 y1,y:(rh)-
		unhandled("x:(r)-,x1 y1,y:(rh)-");
		break;
		}
	case 1738: { // x:(r)-,x1 a,y:(rh)-
		unhandled("x:(r)-,x1 a,y:(rh)-");
		break;
		}
	case 1739: { // x:(r)-,x1 b,y:(rh)-
		unhandled("x:(r)-,x1 b,y:(rh)-");
		break;
		}
	case 1740: { // x:(r)-,a y0,y:(rh)-
		unhandled("x:(r)-,a y0,y:(rh)-");
		break;
		}
	case 1741: { // x:(r)-,a y1,y:(rh)-
		unhandled("x:(r)-,a y1,y:(rh)-");
		break;
		}
	case 1742: { // x:(r)-,a a,y:(rh)-
		unhandled("x:(r)-,a a,y:(rh)-");
		break;
		}
	case 1743: { // x:(r)-,a b,y:(rh)-
		unhandled("x:(r)-,a b,y:(rh)-");
		break;
		}
	case 1744: { // x:(r)-,b y0,y:(rh)-
		unhandled("x:(r)-,b y0,y:(rh)-");
		break;
		}
	case 1745: { // x:(r)-,b y1,y:(rh)-
		unhandled("x:(r)-,b y1,y:(rh)-");
		break;
		}
	case 1746: { // x:(r)-,b a,y:(rh)-
		unhandled("x:(r)-,b a,y:(rh)-");
		break;
		}
	case 1747: { // x:(r)-,b b,y:(rh)-
		unhandled("x:(r)-,b b,y:(rh)-");
		break;
		}
	case 1748: { // x:(r)-,x0 y0,y:(rh)+
		unhandled("x:(r)-,x0 y0,y:(rh)+");
		break;
		}
	case 1749: { // x:(r)-,x0 y1,y:(rh)+
		unhandled("x:(r)-,x0 y1,y:(rh)+");
		break;
		}
	case 1750: { // x:(r)-,x0 a,y:(rh)+
		unhandled("x:(r)-,x0 a,y:(rh)+");
		break;
		}
	case 1751: { // x:(r)-,x0 b,y:(rh)+
		unhandled("x:(r)-,x0 b,y:(rh)+");
		break;
		}
	case 1752: { // x:(r)-,x1 y0,y:(rh)+
		unhandled("x:(r)-,x1 y0,y:(rh)+");
		break;
		}
	case 1753: { // x:(r)-,x1 y1,y:(rh)+
		unhandled("x:(r)-,x1 y1,y:(rh)+");
		break;
		}
	case 1754: { // x:(r)-,x1 a,y:(rh)+
		unhandled("x:(r)-,x1 a,y:(rh)+");
		break;
		}
	case 1755: { // x:(r)-,x1 b,y:(rh)+
		unhandled("x:(r)-,x1 b,y:(rh)+");
		break;
		}
	case 1756: { // x:(r)-,a y0,y:(rh)+
		unhandled("x:(r)-,a y0,y:(rh)+");
		break;
		}
	case 1757: { // x:(r)-,a y1,y:(rh)+
		unhandled("x:(r)-,a y1,y:(rh)+");
		break;
		}
	case 1758: { // x:(r)-,a a,y:(rh)+
		unhandled("x:(r)-,a a,y:(rh)+");
		break;
		}
	case 1759: { // x:(r)-,a b,y:(rh)+
		unhandled("x:(r)-,a b,y:(rh)+");
		break;
		}
	case 1760: { // x:(r)-,b y0,y:(rh)+
		unhandled("x:(r)-,b y0,y:(rh)+");
		break;
		}
	case 1761: { // x:(r)-,b y1,y:(rh)+
		unhandled("x:(r)-,b y1,y:(rh)+");
		break;
		}
	case 1762: { // x:(r)-,b a,y:(rh)+
		unhandled("x:(r)-,b a,y:(rh)+");
		break;
		}
	case 1763: { // x:(r)-,b b,y:(rh)+
		unhandled("x:(r)-,b b,y:(rh)+");
		break;
		}
	case 1764: { // x:(r)-,x0 y0,y:(rh)
		unhandled("x:(r)-,x0 y0,y:(rh)");
		break;
		}
	case 1765: { // x:(r)-,x0 y1,y:(rh)
		unhandled("x:(r)-,x0 y1,y:(rh)");
		break;
		}
	case 1766: { // x:(r)-,x0 a,y:(rh)
		unhandled("x:(r)-,x0 a,y:(rh)");
		break;
		}
	case 1767: { // x:(r)-,x0 b,y:(rh)
		unhandled("x:(r)-,x0 b,y:(rh)");
		break;
		}
	case 1768: { // x:(r)-,x1 y0,y:(rh)
		unhandled("x:(r)-,x1 y0,y:(rh)");
		break;
		}
	case 1769: { // x:(r)-,x1 y1,y:(rh)
		unhandled("x:(r)-,x1 y1,y:(rh)");
		break;
		}
	case 1770: { // x:(r)-,x1 a,y:(rh)
		unhandled("x:(r)-,x1 a,y:(rh)");
		break;
		}
	case 1771: { // x:(r)-,x1 b,y:(rh)
		unhandled("x:(r)-,x1 b,y:(rh)");
		break;
		}
	case 1772: { // x:(r)-,a y0,y:(rh)
		unhandled("x:(r)-,a y0,y:(rh)");
		break;
		}
	case 1773: { // x:(r)-,a y1,y:(rh)
		unhandled("x:(r)-,a y1,y:(rh)");
		break;
		}
	case 1774: { // x:(r)-,a a,y:(rh)
		unhandled("x:(r)-,a a,y:(rh)");
		break;
		}
	case 1775: { // x:(r)-,a b,y:(rh)
		unhandled("x:(r)-,a b,y:(rh)");
		break;
		}
	case 1776: { // x:(r)-,b y0,y:(rh)
		unhandled("x:(r)-,b y0,y:(rh)");
		break;
		}
	case 1777: { // x:(r)-,b y1,y:(rh)
		unhandled("x:(r)-,b y1,y:(rh)");
		break;
		}
	case 1778: { // x:(r)-,b a,y:(rh)
		unhandled("x:(r)-,b a,y:(rh)");
		break;
		}
	case 1779: { // x:(r)-,b b,y:(rh)
		unhandled("x:(r)-,b b,y:(rh)");
		break;
		}
	case 1780: { // x:(r)+,x0 y0,y:(rh)+n
		unhandled("x:(r)+,x0 y0,y:(rh)+n");
		break;
		}
	case 1781: { // x:(r)+,x0 y1,y:(rh)+n
		unhandled("x:(r)+,x0 y1,y:(rh)+n");
		break;
		}
	case 1782: { // x:(r)+,x0 a,y:(rh)+n
		unhandled("x:(r)+,x0 a,y:(rh)+n");
		break;
		}
	case 1783: { // x:(r)+,x0 b,y:(rh)+n
		unhandled("x:(r)+,x0 b,y:(rh)+n");
		break;
		}
	case 1784: { // x:(r)+,x1 y0,y:(rh)+n
		unhandled("x:(r)+,x1 y0,y:(rh)+n");
		break;
		}
	case 1785: { // x:(r)+,x1 y1,y:(rh)+n
		unhandled("x:(r)+,x1 y1,y:(rh)+n");
		break;
		}
	case 1786: { // x:(r)+,x1 a,y:(rh)+n
		unhandled("x:(r)+,x1 a,y:(rh)+n");
		break;
		}
	case 1787: { // x:(r)+,x1 b,y:(rh)+n
		unhandled("x:(r)+,x1 b,y:(rh)+n");
		break;
		}
	case 1788: { // x:(r)+,a y0,y:(rh)+n
		unhandled("x:(r)+,a y0,y:(rh)+n");
		break;
		}
	case 1789: { // x:(r)+,a y1,y:(rh)+n
		unhandled("x:(r)+,a y1,y:(rh)+n");
		break;
		}
	case 1790: { // x:(r)+,a a,y:(rh)+n
		unhandled("x:(r)+,a a,y:(rh)+n");
		break;
		}
	case 1791: { // x:(r)+,a b,y:(rh)+n
		unhandled("x:(r)+,a b,y:(rh)+n");
		break;
		}
	case 1792: { // x:(r)+,b y0,y:(rh)+n
		unhandled("x:(r)+,b y0,y:(rh)+n");
		break;
		}
	case 1793: { // x:(r)+,b y1,y:(rh)+n
		unhandled("x:(r)+,b y1,y:(rh)+n");
		break;
		}
	case 1794: { // x:(r)+,b a,y:(rh)+n
		unhandled("x:(r)+,b a,y:(rh)+n");
		break;
		}
	case 1795: { // x:(r)+,b b,y:(rh)+n
		unhandled("x:(r)+,b b,y:(rh)+n");
		break;
		}
	case 1796: { // x:(r)+,x0 y0,y:(rh)-
		unhandled("x:(r)+,x0 y0,y:(rh)-");
		break;
		}
	case 1797: { // x:(r)+,x0 y1,y:(rh)-
		unhandled("x:(r)+,x0 y1,y:(rh)-");
		break;
		}
	case 1798: { // x:(r)+,x0 a,y:(rh)-
		unhandled("x:(r)+,x0 a,y:(rh)-");
		break;
		}
	case 1799: { // x:(r)+,x0 b,y:(rh)-
		unhandled("x:(r)+,x0 b,y:(rh)-");
		break;
		}
	case 1800: { // x:(r)+,x1 y0,y:(rh)-
		unhandled("x:(r)+,x1 y0,y:(rh)-");
		break;
		}
	case 1801: { // x:(r)+,x1 y1,y:(rh)-
		unhandled("x:(r)+,x1 y1,y:(rh)-");
		break;
		}
	case 1802: { // x:(r)+,x1 a,y:(rh)-
		unhandled("x:(r)+,x1 a,y:(rh)-");
		break;
		}
	case 1803: { // x:(r)+,x1 b,y:(rh)-
		unhandled("x:(r)+,x1 b,y:(rh)-");
		break;
		}
	case 1804: { // x:(r)+,a y0,y:(rh)-
		unhandled("x:(r)+,a y0,y:(rh)-");
		break;
		}
	case 1805: { // x:(r)+,a y1,y:(rh)-
		unhandled("x:(r)+,a y1,y:(rh)-");
		break;
		}
	case 1806: { // x:(r)+,a a,y:(rh)-
		unhandled("x:(r)+,a a,y:(rh)-");
		break;
		}
	case 1807: { // x:(r)+,a b,y:(rh)-
		unhandled("x:(r)+,a b,y:(rh)-");
		break;
		}
	case 1808: { // x:(r)+,b y0,y:(rh)-
		unhandled("x:(r)+,b y0,y:(rh)-");
		break;
		}
	case 1809: { // x:(r)+,b y1,y:(rh)-
		unhandled("x:(r)+,b y1,y:(rh)-");
		break;
		}
	case 1810: { // x:(r)+,b a,y:(rh)-
		unhandled("x:(r)+,b a,y:(rh)-");
		break;
		}
	case 1811: { // x:(r)+,b b,y:(rh)-
		unhandled("x:(r)+,b b,y:(rh)-");
		break;
		}
	case 1812: { // x:(r)+,x0 y0,y:(rh)+
		unhandled("x:(r)+,x0 y0,y:(rh)+");
		break;
		}
	case 1813: { // x:(r)+,x0 y1,y:(rh)+
		unhandled("x:(r)+,x0 y1,y:(rh)+");
		break;
		}
	case 1814: { // x:(r)+,x0 a,y:(rh)+
		unhandled("x:(r)+,x0 a,y:(rh)+");
		break;
		}
	case 1815: { // x:(r)+,x0 b,y:(rh)+
		unhandled("x:(r)+,x0 b,y:(rh)+");
		break;
		}
	case 1816: { // x:(r)+,x1 y0,y:(rh)+
		unhandled("x:(r)+,x1 y0,y:(rh)+");
		break;
		}
	case 1817: { // x:(r)+,x1 y1,y:(rh)+
		unhandled("x:(r)+,x1 y1,y:(rh)+");
		break;
		}
	case 1818: { // x:(r)+,x1 a,y:(rh)+
		unhandled("x:(r)+,x1 a,y:(rh)+");
		break;
		}
	case 1819: { // x:(r)+,x1 b,y:(rh)+
		unhandled("x:(r)+,x1 b,y:(rh)+");
		break;
		}
	case 1820: { // x:(r)+,a y0,y:(rh)+
		unhandled("x:(r)+,a y0,y:(rh)+");
		break;
		}
	case 1821: { // x:(r)+,a y1,y:(rh)+
		unhandled("x:(r)+,a y1,y:(rh)+");
		break;
		}
	case 1822: { // x:(r)+,a a,y:(rh)+
		unhandled("x:(r)+,a a,y:(rh)+");
		break;
		}
	case 1823: { // x:(r)+,a b,y:(rh)+
		unhandled("x:(r)+,a b,y:(rh)+");
		break;
		}
	case 1824: { // x:(r)+,b y0,y:(rh)+
		unhandled("x:(r)+,b y0,y:(rh)+");
		break;
		}
	case 1825: { // x:(r)+,b y1,y:(rh)+
		unhandled("x:(r)+,b y1,y:(rh)+");
		break;
		}
	case 1826: { // x:(r)+,b a,y:(rh)+
		unhandled("x:(r)+,b a,y:(rh)+");
		break;
		}
	case 1827: { // x:(r)+,b b,y:(rh)+
		unhandled("x:(r)+,b b,y:(rh)+");
		break;
		}
	case 1828: { // x:(r)+,x0 y0,y:(rh)
		unhandled("x:(r)+,x0 y0,y:(rh)");
		break;
		}
	case 1829: { // x:(r)+,x0 y1,y:(rh)
		unhandled("x:(r)+,x0 y1,y:(rh)");
		break;
		}
	case 1830: { // x:(r)+,x0 a,y:(rh)
		unhandled("x:(r)+,x0 a,y:(rh)");
		break;
		}
	case 1831: { // x:(r)+,x0 b,y:(rh)
		unhandled("x:(r)+,x0 b,y:(rh)");
		break;
		}
	case 1832: { // x:(r)+,x1 y0,y:(rh)
		unhandled("x:(r)+,x1 y0,y:(rh)");
		break;
		}
	case 1833: { // x:(r)+,x1 y1,y:(rh)
		unhandled("x:(r)+,x1 y1,y:(rh)");
		break;
		}
	case 1834: { // x:(r)+,x1 a,y:(rh)
		unhandled("x:(r)+,x1 a,y:(rh)");
		break;
		}
	case 1835: { // x:(r)+,x1 b,y:(rh)
		unhandled("x:(r)+,x1 b,y:(rh)");
		break;
		}
	case 1836: { // x:(r)+,a y0,y:(rh)
		unhandled("x:(r)+,a y0,y:(rh)");
		break;
		}
	case 1837: { // x:(r)+,a y1,y:(rh)
		unhandled("x:(r)+,a y1,y:(rh)");
		break;
		}
	case 1838: { // x:(r)+,a a,y:(rh)
		unhandled("x:(r)+,a a,y:(rh)");
		break;
		}
	case 1839: { // x:(r)+,a b,y:(rh)
		unhandled("x:(r)+,a b,y:(rh)");
		break;
		}
	case 1840: { // x:(r)+,b y0,y:(rh)
		unhandled("x:(r)+,b y0,y:(rh)");
		break;
		}
	case 1841: { // x:(r)+,b y1,y:(rh)
		unhandled("x:(r)+,b y1,y:(rh)");
		break;
		}
	case 1842: { // x:(r)+,b a,y:(rh)
		unhandled("x:(r)+,b a,y:(rh)");
		break;
		}
	case 1843: { // x:(r)+,b b,y:(rh)
		unhandled("x:(r)+,b b,y:(rh)");
		break;
		}
	case 1844: { // x:(r),x0 y0,y:(rh)+n
		unhandled("x:(r),x0 y0,y:(rh)+n");
		break;
		}
	case 1845: { // x:(r),x0 y1,y:(rh)+n
		unhandled("x:(r),x0 y1,y:(rh)+n");
		break;
		}
	case 1846: { // x:(r),x0 a,y:(rh)+n
		unhandled("x:(r),x0 a,y:(rh)+n");
		break;
		}
	case 1847: { // x:(r),x0 b,y:(rh)+n
		unhandled("x:(r),x0 b,y:(rh)+n");
		break;
		}
	case 1848: { // x:(r),x1 y0,y:(rh)+n
		unhandled("x:(r),x1 y0,y:(rh)+n");
		break;
		}
	case 1849: { // x:(r),x1 y1,y:(rh)+n
		unhandled("x:(r),x1 y1,y:(rh)+n");
		break;
		}
	case 1850: { // x:(r),x1 a,y:(rh)+n
		unhandled("x:(r),x1 a,y:(rh)+n");
		break;
		}
	case 1851: { // x:(r),x1 b,y:(rh)+n
		unhandled("x:(r),x1 b,y:(rh)+n");
		break;
		}
	case 1852: { // x:(r),a y0,y:(rh)+n
		unhandled("x:(r),a y0,y:(rh)+n");
		break;
		}
	case 1853: { // x:(r),a y1,y:(rh)+n
		unhandled("x:(r),a y1,y:(rh)+n");
		break;
		}
	case 1854: { // x:(r),a a,y:(rh)+n
		unhandled("x:(r),a a,y:(rh)+n");
		break;
		}
	case 1855: { // x:(r),a b,y:(rh)+n
		unhandled("x:(r),a b,y:(rh)+n");
		break;
		}
	case 1856: { // x:(r),b y0,y:(rh)+n
		unhandled("x:(r),b y0,y:(rh)+n");
		break;
		}
	case 1857: { // x:(r),b y1,y:(rh)+n
		unhandled("x:(r),b y1,y:(rh)+n");
		break;
		}
	case 1858: { // x:(r),b a,y:(rh)+n
		unhandled("x:(r),b a,y:(rh)+n");
		break;
		}
	case 1859: { // x:(r),b b,y:(rh)+n
		unhandled("x:(r),b b,y:(rh)+n");
		break;
		}
	case 1860: { // x:(r),x0 y0,y:(rh)-
		unhandled("x:(r),x0 y0,y:(rh)-");
		break;
		}
	case 1861: { // x:(r),x0 y1,y:(rh)-
		unhandled("x:(r),x0 y1,y:(rh)-");
		break;
		}
	case 1862: { // x:(r),x0 a,y:(rh)-
		unhandled("x:(r),x0 a,y:(rh)-");
		break;
		}
	case 1863: { // x:(r),x0 b,y:(rh)-
		unhandled("x:(r),x0 b,y:(rh)-");
		break;
		}
	case 1864: { // x:(r),x1 y0,y:(rh)-
		unhandled("x:(r),x1 y0,y:(rh)-");
		break;
		}
	case 1865: { // x:(r),x1 y1,y:(rh)-
		unhandled("x:(r),x1 y1,y:(rh)-");
		break;
		}
	case 1866: { // x:(r),x1 a,y:(rh)-
		unhandled("x:(r),x1 a,y:(rh)-");
		break;
		}
	case 1867: { // x:(r),x1 b,y:(rh)-
		unhandled("x:(r),x1 b,y:(rh)-");
		break;
		}
	case 1868: { // x:(r),a y0,y:(rh)-
		unhandled("x:(r),a y0,y:(rh)-");
		break;
		}
	case 1869: { // x:(r),a y1,y:(rh)-
		unhandled("x:(r),a y1,y:(rh)-");
		break;
		}
	case 1870: { // x:(r),a a,y:(rh)-
		unhandled("x:(r),a a,y:(rh)-");
		break;
		}
	case 1871: { // x:(r),a b,y:(rh)-
		unhandled("x:(r),a b,y:(rh)-");
		break;
		}
	case 1872: { // x:(r),b y0,y:(rh)-
		unhandled("x:(r),b y0,y:(rh)-");
		break;
		}
	case 1873: { // x:(r),b y1,y:(rh)-
		unhandled("x:(r),b y1,y:(rh)-");
		break;
		}
	case 1874: { // x:(r),b a,y:(rh)-
		unhandled("x:(r),b a,y:(rh)-");
		break;
		}
	case 1875: { // x:(r),b b,y:(rh)-
		unhandled("x:(r),b b,y:(rh)-");
		break;
		}
	case 1876: { // x:(r),x0 y0,y:(rh)+
		unhandled("x:(r),x0 y0,y:(rh)+");
		break;
		}
	case 1877: { // x:(r),x0 y1,y:(rh)+
		unhandled("x:(r),x0 y1,y:(rh)+");
		break;
		}
	case 1878: { // x:(r),x0 a,y:(rh)+
		unhandled("x:(r),x0 a,y:(rh)+");
		break;
		}
	case 1879: { // x:(r),x0 b,y:(rh)+
		unhandled("x:(r),x0 b,y:(rh)+");
		break;
		}
	case 1880: { // x:(r),x1 y0,y:(rh)+
		unhandled("x:(r),x1 y0,y:(rh)+");
		break;
		}
	case 1881: { // x:(r),x1 y1,y:(rh)+
		unhandled("x:(r),x1 y1,y:(rh)+");
		break;
		}
	case 1882: { // x:(r),x1 a,y:(rh)+
		unhandled("x:(r),x1 a,y:(rh)+");
		break;
		}
	case 1883: { // x:(r),x1 b,y:(rh)+
		unhandled("x:(r),x1 b,y:(rh)+");
		break;
		}
	case 1884: { // x:(r),a y0,y:(rh)+
		unhandled("x:(r),a y0,y:(rh)+");
		break;
		}
	case 1885: { // x:(r),a y1,y:(rh)+
		unhandled("x:(r),a y1,y:(rh)+");
		break;
		}
	case 1886: { // x:(r),a a,y:(rh)+
		unhandled("x:(r),a a,y:(rh)+");
		break;
		}
	case 1887: { // x:(r),a b,y:(rh)+
		unhandled("x:(r),a b,y:(rh)+");
		break;
		}
	case 1888: { // x:(r),b y0,y:(rh)+
		unhandled("x:(r),b y0,y:(rh)+");
		break;
		}
	case 1889: { // x:(r),b y1,y:(rh)+
		unhandled("x:(r),b y1,y:(rh)+");
		break;
		}
	case 1890: { // x:(r),b a,y:(rh)+
		unhandled("x:(r),b a,y:(rh)+");
		break;
		}
	case 1891: { // x:(r),b b,y:(rh)+
		unhandled("x:(r),b b,y:(rh)+");
		break;
		}
	case 1892: { // x:(r),x0 y0,y:(rh)
		unhandled("x:(r),x0 y0,y:(rh)");
		break;
		}
	case 1893: { // x:(r),x0 y1,y:(rh)
		unhandled("x:(r),x0 y1,y:(rh)");
		break;
		}
	case 1894: { // x:(r),x0 a,y:(rh)
		unhandled("x:(r),x0 a,y:(rh)");
		break;
		}
	case 1895: { // x:(r),x0 b,y:(rh)
		unhandled("x:(r),x0 b,y:(rh)");
		break;
		}
	case 1896: { // x:(r),x1 y0,y:(rh)
		unhandled("x:(r),x1 y0,y:(rh)");
		break;
		}
	case 1897: { // x:(r),x1 y1,y:(rh)
		unhandled("x:(r),x1 y1,y:(rh)");
		break;
		}
	case 1898: { // x:(r),x1 a,y:(rh)
		unhandled("x:(r),x1 a,y:(rh)");
		break;
		}
	case 1899: { // x:(r),x1 b,y:(rh)
		unhandled("x:(r),x1 b,y:(rh)");
		break;
		}
	case 1900: { // x:(r),a y0,y:(rh)
		unhandled("x:(r),a y0,y:(rh)");
		break;
		}
	case 1901: { // x:(r),a y1,y:(rh)
		unhandled("x:(r),a y1,y:(rh)");
		break;
		}
	case 1902: { // x:(r),a a,y:(rh)
		unhandled("x:(r),a a,y:(rh)");
		break;
		}
	case 1903: { // x:(r),a b,y:(rh)
		unhandled("x:(r),a b,y:(rh)");
		break;
		}
	case 1904: { // x:(r),b y0,y:(rh)
		unhandled("x:(r),b y0,y:(rh)");
		break;
		}
	case 1905: { // x:(r),b y1,y:(rh)
		unhandled("x:(r),b y1,y:(rh)");
		break;
		}
	case 1906: { // x:(r),b a,y:(rh)
		unhandled("x:(r),b a,y:(rh)");
		break;
		}
	case 1907: { // x:(r),b b,y:(rh)
		unhandled("x:(r),b b,y:(rh)");
		break;
		}
	case 1908: { // x0,x:(r)+n y:(rh)+n,y0
		unhandled("x0,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1909: { // x0,x:(r)+n y:(rh)+n,y1
		unhandled("x0,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1910: { // x0,x:(r)+n y:(rh)+n,a
		unhandled("x0,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1911: { // x0,x:(r)+n y:(rh)+n,b
		unhandled("x0,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1912: { // x1,x:(r)+n y:(rh)+n,y0
		unhandled("x1,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1913: { // x1,x:(r)+n y:(rh)+n,y1
		unhandled("x1,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1914: { // x1,x:(r)+n y:(rh)+n,a
		unhandled("x1,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1915: { // x1,x:(r)+n y:(rh)+n,b
		unhandled("x1,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1916: { // a,x:(r)+n y:(rh)+n,y0
		unhandled("a,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1917: { // a,x:(r)+n y:(rh)+n,y1
		unhandled("a,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1918: { // a,x:(r)+n y:(rh)+n,a
		unhandled("a,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1919: { // a,x:(r)+n y:(rh)+n,b
		unhandled("a,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1920: { // b,x:(r)+n y:(rh)+n,y0
		unhandled("b,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1921: { // b,x:(r)+n y:(rh)+n,y1
		unhandled("b,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1922: { // b,x:(r)+n y:(rh)+n,a
		unhandled("b,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1923: { // b,x:(r)+n y:(rh)+n,b
		unhandled("b,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1924: { // x0,x:(r)+n y:(rh)-,y0
		unhandled("x0,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1925: { // x0,x:(r)+n y:(rh)-,y1
		unhandled("x0,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1926: { // x0,x:(r)+n y:(rh)-,a
		unhandled("x0,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1927: { // x0,x:(r)+n y:(rh)-,b
		unhandled("x0,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1928: { // x1,x:(r)+n y:(rh)-,y0
		unhandled("x1,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1929: { // x1,x:(r)+n y:(rh)-,y1
		unhandled("x1,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1930: { // x1,x:(r)+n y:(rh)-,a
		unhandled("x1,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1931: { // x1,x:(r)+n y:(rh)-,b
		unhandled("x1,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1932: { // a,x:(r)+n y:(rh)-,y0
		unhandled("a,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1933: { // a,x:(r)+n y:(rh)-,y1
		unhandled("a,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1934: { // a,x:(r)+n y:(rh)-,a
		unhandled("a,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1935: { // a,x:(r)+n y:(rh)-,b
		unhandled("a,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1936: { // b,x:(r)+n y:(rh)-,y0
		unhandled("b,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1937: { // b,x:(r)+n y:(rh)-,y1
		unhandled("b,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1938: { // b,x:(r)+n y:(rh)-,a
		unhandled("b,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1939: { // b,x:(r)+n y:(rh)-,b
		unhandled("b,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1940: { // x0,x:(r)+n y:(rh)+,y0
		unhandled("x0,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1941: { // x0,x:(r)+n y:(rh)+,y1
		unhandled("x0,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1942: { // x0,x:(r)+n y:(rh)+,a
		unhandled("x0,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1943: { // x0,x:(r)+n y:(rh)+,b
		unhandled("x0,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1944: { // x1,x:(r)+n y:(rh)+,y0
		unhandled("x1,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1945: { // x1,x:(r)+n y:(rh)+,y1
		unhandled("x1,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1946: { // x1,x:(r)+n y:(rh)+,a
		unhandled("x1,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1947: { // x1,x:(r)+n y:(rh)+,b
		unhandled("x1,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1948: { // a,x:(r)+n y:(rh)+,y0
		unhandled("a,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1949: { // a,x:(r)+n y:(rh)+,y1
		unhandled("a,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1950: { // a,x:(r)+n y:(rh)+,a
		unhandled("a,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1951: { // a,x:(r)+n y:(rh)+,b
		unhandled("a,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1952: { // b,x:(r)+n y:(rh)+,y0
		unhandled("b,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1953: { // b,x:(r)+n y:(rh)+,y1
		unhandled("b,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1954: { // b,x:(r)+n y:(rh)+,a
		unhandled("b,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1955: { // b,x:(r)+n y:(rh)+,b
		unhandled("b,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1956: { // x0,x:(r)+n y:(rh),y0
		unhandled("x0,x:(r)+n y:(rh),y0");
		break;
		}
	case 1957: { // x0,x:(r)+n y:(rh),y1
		unhandled("x0,x:(r)+n y:(rh),y1");
		break;
		}
	case 1958: { // x0,x:(r)+n y:(rh),a
		unhandled("x0,x:(r)+n y:(rh),a");
		break;
		}
	case 1959: { // x0,x:(r)+n y:(rh),b
		unhandled("x0,x:(r)+n y:(rh),b");
		break;
		}
	case 1960: { // x1,x:(r)+n y:(rh),y0
		unhandled("x1,x:(r)+n y:(rh),y0");
		break;
		}
	case 1961: { // x1,x:(r)+n y:(rh),y1
		unhandled("x1,x:(r)+n y:(rh),y1");
		break;
		}
	case 1962: { // x1,x:(r)+n y:(rh),a
		unhandled("x1,x:(r)+n y:(rh),a");
		break;
		}
	case 1963: { // x1,x:(r)+n y:(rh),b
		unhandled("x1,x:(r)+n y:(rh),b");
		break;
		}
	case 1964: { // a,x:(r)+n y:(rh),y0
		unhandled("a,x:(r)+n y:(rh),y0");
		break;
		}
	case 1965: { // a,x:(r)+n y:(rh),y1
		unhandled("a,x:(r)+n y:(rh),y1");
		break;
		}
	case 1966: { // a,x:(r)+n y:(rh),a
		unhandled("a,x:(r)+n y:(rh),a");
		break;
		}
	case 1967: { // a,x:(r)+n y:(rh),b
		unhandled("a,x:(r)+n y:(rh),b");
		break;
		}
	case 1968: { // b,x:(r)+n y:(rh),y0
		unhandled("b,x:(r)+n y:(rh),y0");
		break;
		}
	case 1969: { // b,x:(r)+n y:(rh),y1
		unhandled("b,x:(r)+n y:(rh),y1");
		break;
		}
	case 1970: { // b,x:(r)+n y:(rh),a
		unhandled("b,x:(r)+n y:(rh),a");
		break;
		}
	case 1971: { // b,x:(r)+n y:(rh),b
		unhandled("b,x:(r)+n y:(rh),b");
		break;
		}
	case 1972: { // x0,x:(r)- y:(rh)+n,y0
		unhandled("x0,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1973: { // x0,x:(r)- y:(rh)+n,y1
		unhandled("x0,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1974: { // x0,x:(r)- y:(rh)+n,a
		unhandled("x0,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1975: { // x0,x:(r)- y:(rh)+n,b
		unhandled("x0,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1976: { // x1,x:(r)- y:(rh)+n,y0
		unhandled("x1,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1977: { // x1,x:(r)- y:(rh)+n,y1
		unhandled("x1,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1978: { // x1,x:(r)- y:(rh)+n,a
		unhandled("x1,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1979: { // x1,x:(r)- y:(rh)+n,b
		unhandled("x1,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1980: { // a,x:(r)- y:(rh)+n,y0
		unhandled("a,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1981: { // a,x:(r)- y:(rh)+n,y1
		unhandled("a,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1982: { // a,x:(r)- y:(rh)+n,a
		unhandled("a,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1983: { // a,x:(r)- y:(rh)+n,b
		unhandled("a,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1984: { // b,x:(r)- y:(rh)+n,y0
		unhandled("b,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1985: { // b,x:(r)- y:(rh)+n,y1
		unhandled("b,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1986: { // b,x:(r)- y:(rh)+n,a
		unhandled("b,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1987: { // b,x:(r)- y:(rh)+n,b
		unhandled("b,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1988: { // x0,x:(r)- y:(rh)-,y0
		unhandled("x0,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1989: { // x0,x:(r)- y:(rh)-,y1
		unhandled("x0,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1990: { // x0,x:(r)- y:(rh)-,a
		unhandled("x0,x:(r)- y:(rh)-,a");
		break;
		}
	case 1991: { // x0,x:(r)- y:(rh)-,b
		unhandled("x0,x:(r)- y:(rh)-,b");
		break;
		}
	case 1992: { // x1,x:(r)- y:(rh)-,y0
		unhandled("x1,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1993: { // x1,x:(r)- y:(rh)-,y1
		unhandled("x1,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1994: { // x1,x:(r)- y:(rh)-,a
		unhandled("x1,x:(r)- y:(rh)-,a");
		break;
		}
	case 1995: { // x1,x:(r)- y:(rh)-,b
		unhandled("x1,x:(r)- y:(rh)-,b");
		break;
		}
	case 1996: { // a,x:(r)- y:(rh)-,y0
		unhandled("a,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1997: { // a,x:(r)- y:(rh)-,y1
		unhandled("a,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1998: { // a,x:(r)- y:(rh)-,a
		unhandled("a,x:(r)- y:(rh)-,a");
		break;
		}
	case 1999: { // a,x:(r)- y:(rh)-,b
		unhandled("a,x:(r)- y:(rh)-,b");
		break;
		}
	case 2000: { // b,x:(r)- y:(rh)-,y0
		unhandled("b,x:(r)- y:(rh)-,y0");
		break;
		}
	case 2001: { // b,x:(r)- y:(rh)-,y1
		unhandled("b,x:(r)- y:(rh)-,y1");
		break;
		}
	case 2002: { // b,x:(r)- y:(rh)-,a
		unhandled("b,x:(r)- y:(rh)-,a");
		break;
		}
	case 2003: { // b,x:(r)- y:(rh)-,b
		unhandled("b,x:(r)- y:(rh)-,b");
		break;
		}
	case 2004: { // x0,x:(r)- y:(rh)+,y0
		unhandled("x0,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2005: { // x0,x:(r)- y:(rh)+,y1
		unhandled("x0,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2006: { // x0,x:(r)- y:(rh)+,a
		unhandled("x0,x:(r)- y:(rh)+,a");
		break;
		}
	case 2007: { // x0,x:(r)- y:(rh)+,b
		unhandled("x0,x:(r)- y:(rh)+,b");
		break;
		}
	case 2008: { // x1,x:(r)- y:(rh)+,y0
		unhandled("x1,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2009: { // x1,x:(r)- y:(rh)+,y1
		unhandled("x1,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2010: { // x1,x:(r)- y:(rh)+,a
		unhandled("x1,x:(r)- y:(rh)+,a");
		break;
		}
	case 2011: { // x1,x:(r)- y:(rh)+,b
		unhandled("x1,x:(r)- y:(rh)+,b");
		break;
		}
	case 2012: { // a,x:(r)- y:(rh)+,y0
		unhandled("a,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2013: { // a,x:(r)- y:(rh)+,y1
		unhandled("a,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2014: { // a,x:(r)- y:(rh)+,a
		unhandled("a,x:(r)- y:(rh)+,a");
		break;
		}
	case 2015: { // a,x:(r)- y:(rh)+,b
		unhandled("a,x:(r)- y:(rh)+,b");
		break;
		}
	case 2016: { // b,x:(r)- y:(rh)+,y0
		unhandled("b,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2017: { // b,x:(r)- y:(rh)+,y1
		unhandled("b,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2018: { // b,x:(r)- y:(rh)+,a
		unhandled("b,x:(r)- y:(rh)+,a");
		break;
		}
	case 2019: { // b,x:(r)- y:(rh)+,b
		unhandled("b,x:(r)- y:(rh)+,b");
		break;
		}
	case 2020: { // x0,x:(r)- y:(rh),y0
		unhandled("x0,x:(r)- y:(rh),y0");
		break;
		}
	case 2021: { // x0,x:(r)- y:(rh),y1
		unhandled("x0,x:(r)- y:(rh),y1");
		break;
		}
	case 2022: { // x0,x:(r)- y:(rh),a
		unhandled("x0,x:(r)- y:(rh),a");
		break;
		}
	case 2023: { // x0,x:(r)- y:(rh),b
		unhandled("x0,x:(r)- y:(rh),b");
		break;
		}
	case 2024: { // x1,x:(r)- y:(rh),y0
		unhandled("x1,x:(r)- y:(rh),y0");
		break;
		}
	case 2025: { // x1,x:(r)- y:(rh),y1
		unhandled("x1,x:(r)- y:(rh),y1");
		break;
		}
	case 2026: { // x1,x:(r)- y:(rh),a
		unhandled("x1,x:(r)- y:(rh),a");
		break;
		}
	case 2027: { // x1,x:(r)- y:(rh),b
		unhandled("x1,x:(r)- y:(rh),b");
		break;
		}
	case 2028: { // a,x:(r)- y:(rh),y0
		unhandled("a,x:(r)- y:(rh),y0");
		break;
		}
	case 2029: { // a,x:(r)- y:(rh),y1
		unhandled("a,x:(r)- y:(rh),y1");
		break;
		}
	case 2030: { // a,x:(r)- y:(rh),a
		unhandled("a,x:(r)- y:(rh),a");
		break;
		}
	case 2031: { // a,x:(r)- y:(rh),b
		unhandled("a,x:(r)- y:(rh),b");
		break;
		}
	case 2032: { // b,x:(r)- y:(rh),y0
		unhandled("b,x:(r)- y:(rh),y0");
		break;
		}
	case 2033: { // b,x:(r)- y:(rh),y1
		unhandled("b,x:(r)- y:(rh),y1");
		break;
		}
	case 2034: { // b,x:(r)- y:(rh),a
		unhandled("b,x:(r)- y:(rh),a");
		break;
		}
	case 2035: { // b,x:(r)- y:(rh),b
		unhandled("b,x:(r)- y:(rh),b");
		break;
		}
	case 2036: { // x0,x:(r)+ y:(rh)+n,y0
		unhandled("x0,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2037: { // x0,x:(r)+ y:(rh)+n,y1
		unhandled("x0,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2038: { // x0,x:(r)+ y:(rh)+n,a
		unhandled("x0,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2039: { // x0,x:(r)+ y:(rh)+n,b
		unhandled("x0,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2040: { // x1,x:(r)+ y:(rh)+n,y0
		unhandled("x1,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2041: { // x1,x:(r)+ y:(rh)+n,y1
		unhandled("x1,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2042: { // x1,x:(r)+ y:(rh)+n,a
		unhandled("x1,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2043: { // x1,x:(r)+ y:(rh)+n,b
		unhandled("x1,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2044: { // a,x:(r)+ y:(rh)+n,y0
		unhandled("a,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2045: { // a,x:(r)+ y:(rh)+n,y1
		unhandled("a,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2046: { // a,x:(r)+ y:(rh)+n,a
		unhandled("a,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2047: { // a,x:(r)+ y:(rh)+n,b
		unhandled("a,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2048: { // b,x:(r)+ y:(rh)+n,y0
		unhandled("b,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2049: { // b,x:(r)+ y:(rh)+n,y1
		unhandled("b,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2050: { // b,x:(r)+ y:(rh)+n,a
		unhandled("b,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2051: { // b,x:(r)+ y:(rh)+n,b
		unhandled("b,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2052: { // x0,x:(r)+ y:(rh)-,y0
		unhandled("x0,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2053: { // x0,x:(r)+ y:(rh)-,y1
		unhandled("x0,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2054: { // x0,x:(r)+ y:(rh)-,a
		unhandled("x0,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2055: { // x0,x:(r)+ y:(rh)-,b
		unhandled("x0,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2056: { // x1,x:(r)+ y:(rh)-,y0
		unhandled("x1,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2057: { // x1,x:(r)+ y:(rh)-,y1
		unhandled("x1,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2058: { // x1,x:(r)+ y:(rh)-,a
		unhandled("x1,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2059: { // x1,x:(r)+ y:(rh)-,b
		unhandled("x1,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2060: { // a,x:(r)+ y:(rh)-,y0
		unhandled("a,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2061: { // a,x:(r)+ y:(rh)-,y1
		unhandled("a,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2062: { // a,x:(r)+ y:(rh)-,a
		unhandled("a,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2063: { // a,x:(r)+ y:(rh)-,b
		unhandled("a,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2064: { // b,x:(r)+ y:(rh)-,y0
		unhandled("b,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2065: { // b,x:(r)+ y:(rh)-,y1
		unhandled("b,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2066: { // b,x:(r)+ y:(rh)-,a
		unhandled("b,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2067: { // b,x:(r)+ y:(rh)-,b
		unhandled("b,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2068: { // x0,x:(r)+ y:(rh)+,y0
		unhandled("x0,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2069: { // x0,x:(r)+ y:(rh)+,y1
		unhandled("x0,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2070: { // x0,x:(r)+ y:(rh)+,a
		unhandled("x0,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2071: { // x0,x:(r)+ y:(rh)+,b
		unhandled("x0,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2072: { // x1,x:(r)+ y:(rh)+,y0
		unhandled("x1,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2073: { // x1,x:(r)+ y:(rh)+,y1
		unhandled("x1,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2074: { // x1,x:(r)+ y:(rh)+,a
		unhandled("x1,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2075: { // x1,x:(r)+ y:(rh)+,b
		unhandled("x1,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2076: { // a,x:(r)+ y:(rh)+,y0
		unhandled("a,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2077: { // a,x:(r)+ y:(rh)+,y1
		unhandled("a,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2078: { // a,x:(r)+ y:(rh)+,a
		unhandled("a,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2079: { // a,x:(r)+ y:(rh)+,b
		unhandled("a,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2080: { // b,x:(r)+ y:(rh)+,y0
		unhandled("b,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2081: { // b,x:(r)+ y:(rh)+,y1
		unhandled("b,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2082: { // b,x:(r)+ y:(rh)+,a
		unhandled("b,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2083: { // b,x:(r)+ y:(rh)+,b
		unhandled("b,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2084: { // x0,x:(r)+ y:(rh),y0
		unhandled("x0,x:(r)+ y:(rh),y0");
		break;
		}
	case 2085: { // x0,x:(r)+ y:(rh),y1
		unhandled("x0,x:(r)+ y:(rh),y1");
		break;
		}
	case 2086: { // x0,x:(r)+ y:(rh),a
		unhandled("x0,x:(r)+ y:(rh),a");
		break;
		}
	case 2087: { // x0,x:(r)+ y:(rh),b
		unhandled("x0,x:(r)+ y:(rh),b");
		break;
		}
	case 2088: { // x1,x:(r)+ y:(rh),y0
		unhandled("x1,x:(r)+ y:(rh),y0");
		break;
		}
	case 2089: { // x1,x:(r)+ y:(rh),y1
		unhandled("x1,x:(r)+ y:(rh),y1");
		break;
		}
	case 2090: { // x1,x:(r)+ y:(rh),a
		unhandled("x1,x:(r)+ y:(rh),a");
		break;
		}
	case 2091: { // x1,x:(r)+ y:(rh),b
		unhandled("x1,x:(r)+ y:(rh),b");
		break;
		}
	case 2092: { // a,x:(r)+ y:(rh),y0
		unhandled("a,x:(r)+ y:(rh),y0");
		break;
		}
	case 2093: { // a,x:(r)+ y:(rh),y1
		unhandled("a,x:(r)+ y:(rh),y1");
		break;
		}
	case 2094: { // a,x:(r)+ y:(rh),a
		unhandled("a,x:(r)+ y:(rh),a");
		break;
		}
	case 2095: { // a,x:(r)+ y:(rh),b
		unhandled("a,x:(r)+ y:(rh),b");
		break;
		}
	case 2096: { // b,x:(r)+ y:(rh),y0
		unhandled("b,x:(r)+ y:(rh),y0");
		break;
		}
	case 2097: { // b,x:(r)+ y:(rh),y1
		unhandled("b,x:(r)+ y:(rh),y1");
		break;
		}
	case 2098: { // b,x:(r)+ y:(rh),a
		unhandled("b,x:(r)+ y:(rh),a");
		break;
		}
	case 2099: { // b,x:(r)+ y:(rh),b
		unhandled("b,x:(r)+ y:(rh),b");
		break;
		}
	case 2100: { // x0,x:(r) y:(rh)+n,y0
		unhandled("x0,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2101: { // x0,x:(r) y:(rh)+n,y1
		unhandled("x0,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2102: { // x0,x:(r) y:(rh)+n,a
		unhandled("x0,x:(r) y:(rh)+n,a");
		break;
		}
	case 2103: { // x0,x:(r) y:(rh)+n,b
		unhandled("x0,x:(r) y:(rh)+n,b");
		break;
		}
	case 2104: { // x1,x:(r) y:(rh)+n,y0
		unhandled("x1,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2105: { // x1,x:(r) y:(rh)+n,y1
		unhandled("x1,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2106: { // x1,x:(r) y:(rh)+n,a
		unhandled("x1,x:(r) y:(rh)+n,a");
		break;
		}
	case 2107: { // x1,x:(r) y:(rh)+n,b
		unhandled("x1,x:(r) y:(rh)+n,b");
		break;
		}
	case 2108: { // a,x:(r) y:(rh)+n,y0
		unhandled("a,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2109: { // a,x:(r) y:(rh)+n,y1
		unhandled("a,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2110: { // a,x:(r) y:(rh)+n,a
		unhandled("a,x:(r) y:(rh)+n,a");
		break;
		}
	case 2111: { // a,x:(r) y:(rh)+n,b
		unhandled("a,x:(r) y:(rh)+n,b");
		break;
		}
	case 2112: { // b,x:(r) y:(rh)+n,y0
		unhandled("b,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2113: { // b,x:(r) y:(rh)+n,y1
		unhandled("b,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2114: { // b,x:(r) y:(rh)+n,a
		unhandled("b,x:(r) y:(rh)+n,a");
		break;
		}
	case 2115: { // b,x:(r) y:(rh)+n,b
		unhandled("b,x:(r) y:(rh)+n,b");
		break;
		}
	case 2116: { // x0,x:(r) y:(rh)-,y0
		unhandled("x0,x:(r) y:(rh)-,y0");
		break;
		}
	case 2117: { // x0,x:(r) y:(rh)-,y1
		unhandled("x0,x:(r) y:(rh)-,y1");
		break;
		}
	case 2118: { // x0,x:(r) y:(rh)-,a
		unhandled("x0,x:(r) y:(rh)-,a");
		break;
		}
	case 2119: { // x0,x:(r) y:(rh)-,b
		unhandled("x0,x:(r) y:(rh)-,b");
		break;
		}
	case 2120: { // x1,x:(r) y:(rh)-,y0
		unhandled("x1,x:(r) y:(rh)-,y0");
		break;
		}
	case 2121: { // x1,x:(r) y:(rh)-,y1
		unhandled("x1,x:(r) y:(rh)-,y1");
		break;
		}
	case 2122: { // x1,x:(r) y:(rh)-,a
		unhandled("x1,x:(r) y:(rh)-,a");
		break;
		}
	case 2123: { // x1,x:(r) y:(rh)-,b
		unhandled("x1,x:(r) y:(rh)-,b");
		break;
		}
	case 2124: { // a,x:(r) y:(rh)-,y0
		unhandled("a,x:(r) y:(rh)-,y0");
		break;
		}
	case 2125: { // a,x:(r) y:(rh)-,y1
		unhandled("a,x:(r) y:(rh)-,y1");
		break;
		}
	case 2126: { // a,x:(r) y:(rh)-,a
		unhandled("a,x:(r) y:(rh)-,a");
		break;
		}
	case 2127: { // a,x:(r) y:(rh)-,b
		unhandled("a,x:(r) y:(rh)-,b");
		break;
		}
	case 2128: { // b,x:(r) y:(rh)-,y0
		unhandled("b,x:(r) y:(rh)-,y0");
		break;
		}
	case 2129: { // b,x:(r) y:(rh)-,y1
		unhandled("b,x:(r) y:(rh)-,y1");
		break;
		}
	case 2130: { // b,x:(r) y:(rh)-,a
		unhandled("b,x:(r) y:(rh)-,a");
		break;
		}
	case 2131: { // b,x:(r) y:(rh)-,b
		unhandled("b,x:(r) y:(rh)-,b");
		break;
		}
	case 2132: { // x0,x:(r) y:(rh)+,y0
		unhandled("x0,x:(r) y:(rh)+,y0");
		break;
		}
	case 2133: { // x0,x:(r) y:(rh)+,y1
		unhandled("x0,x:(r) y:(rh)+,y1");
		break;
		}
	case 2134: { // x0,x:(r) y:(rh)+,a
		unhandled("x0,x:(r) y:(rh)+,a");
		break;
		}
	case 2135: { // x0,x:(r) y:(rh)+,b
		unhandled("x0,x:(r) y:(rh)+,b");
		break;
		}
	case 2136: { // x1,x:(r) y:(rh)+,y0
		unhandled("x1,x:(r) y:(rh)+,y0");
		break;
		}
	case 2137: { // x1,x:(r) y:(rh)+,y1
		unhandled("x1,x:(r) y:(rh)+,y1");
		break;
		}
	case 2138: { // x1,x:(r) y:(rh)+,a
		unhandled("x1,x:(r) y:(rh)+,a");
		break;
		}
	case 2139: { // x1,x:(r) y:(rh)+,b
		unhandled("x1,x:(r) y:(rh)+,b");
		break;
		}
	case 2140: { // a,x:(r) y:(rh)+,y0
		unhandled("a,x:(r) y:(rh)+,y0");
		break;
		}
	case 2141: { // a,x:(r) y:(rh)+,y1
		unhandled("a,x:(r) y:(rh)+,y1");
		break;
		}
	case 2142: { // a,x:(r) y:(rh)+,a
		unhandled("a,x:(r) y:(rh)+,a");
		break;
		}
	case 2143: { // a,x:(r) y:(rh)+,b
		unhandled("a,x:(r) y:(rh)+,b");
		break;
		}
	case 2144: { // b,x:(r) y:(rh)+,y0
		unhandled("b,x:(r) y:(rh)+,y0");
		break;
		}
	case 2145: { // b,x:(r) y:(rh)+,y1
		unhandled("b,x:(r) y:(rh)+,y1");
		break;
		}
	case 2146: { // b,x:(r) y:(rh)+,a
		unhandled("b,x:(r) y:(rh)+,a");
		break;
		}
	case 2147: { // b,x:(r) y:(rh)+,b
		unhandled("b,x:(r) y:(rh)+,b");
		break;
		}
	case 2148: { // x0,x:(r) y:(rh),y0
		unhandled("x0,x:(r) y:(rh),y0");
		break;
		}
	case 2149: { // x0,x:(r) y:(rh),y1
		unhandled("x0,x:(r) y:(rh),y1");
		break;
		}
	case 2150: { // x0,x:(r) y:(rh),a
		unhandled("x0,x:(r) y:(rh),a");
		break;
		}
	case 2151: { // x0,x:(r) y:(rh),b
		unhandled("x0,x:(r) y:(rh),b");
		break;
		}
	case 2152: { // x1,x:(r) y:(rh),y0
		unhandled("x1,x:(r) y:(rh),y0");
		break;
		}
	case 2153: { // x1,x:(r) y:(rh),y1
		unhandled("x1,x:(r) y:(rh),y1");
		break;
		}
	case 2154: { // x1,x:(r) y:(rh),a
		unhandled("x1,x:(r) y:(rh),a");
		break;
		}
	case 2155: { // x1,x:(r) y:(rh),b
		unhandled("x1,x:(r) y:(rh),b");
		break;
		}
	case 2156: { // a,x:(r) y:(rh),y0
		unhandled("a,x:(r) y:(rh),y0");
		break;
		}
	case 2157: { // a,x:(r) y:(rh),y1
		unhandled("a,x:(r) y:(rh),y1");
		break;
		}
	case 2158: { // a,x:(r) y:(rh),a
		unhandled("a,x:(r) y:(rh),a");
		break;
		}
	case 2159: { // a,x:(r) y:(rh),b
		unhandled("a,x:(r) y:(rh),b");
		break;
		}
	case 2160: { // b,x:(r) y:(rh),y0
		unhandled("b,x:(r) y:(rh),y0");
		break;
		}
	case 2161: { // b,x:(r) y:(rh),y1
		unhandled("b,x:(r) y:(rh),y1");
		break;
		}
	case 2162: { // b,x:(r) y:(rh),a
		unhandled("b,x:(r) y:(rh),a");
		break;
		}
	case 2163: { // b,x:(r) y:(rh),b
		unhandled("b,x:(r) y:(rh),b");
		break;
		}
	case 2164: { // x0,x:(r)+n y0,y:(rh)+n
		unhandled("x0,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2165: { // x0,x:(r)+n y1,y:(rh)+n
		unhandled("x0,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2166: { // x0,x:(r)+n a,y:(rh)+n
		unhandled("x0,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2167: { // x0,x:(r)+n b,y:(rh)+n
		unhandled("x0,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2168: { // x1,x:(r)+n y0,y:(rh)+n
		unhandled("x1,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2169: { // x1,x:(r)+n y1,y:(rh)+n
		unhandled("x1,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2170: { // x1,x:(r)+n a,y:(rh)+n
		unhandled("x1,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2171: { // x1,x:(r)+n b,y:(rh)+n
		unhandled("x1,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2172: { // a,x:(r)+n y0,y:(rh)+n
		unhandled("a,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2173: { // a,x:(r)+n y1,y:(rh)+n
		unhandled("a,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2174: { // a,x:(r)+n a,y:(rh)+n
		unhandled("a,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2175: { // a,x:(r)+n b,y:(rh)+n
		unhandled("a,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2176: { // b,x:(r)+n y0,y:(rh)+n
		unhandled("b,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2177: { // b,x:(r)+n y1,y:(rh)+n
		unhandled("b,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2178: { // b,x:(r)+n a,y:(rh)+n
		unhandled("b,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2179: { // b,x:(r)+n b,y:(rh)+n
		unhandled("b,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2180: { // x0,x:(r)+n y0,y:(rh)-
		unhandled("x0,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2181: { // x0,x:(r)+n y1,y:(rh)-
		unhandled("x0,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2182: { // x0,x:(r)+n a,y:(rh)-
		unhandled("x0,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2183: { // x0,x:(r)+n b,y:(rh)-
		unhandled("x0,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2184: { // x1,x:(r)+n y0,y:(rh)-
		unhandled("x1,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2185: { // x1,x:(r)+n y1,y:(rh)-
		unhandled("x1,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2186: { // x1,x:(r)+n a,y:(rh)-
		unhandled("x1,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2187: { // x1,x:(r)+n b,y:(rh)-
		unhandled("x1,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2188: { // a,x:(r)+n y0,y:(rh)-
		unhandled("a,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2189: { // a,x:(r)+n y1,y:(rh)-
		unhandled("a,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2190: { // a,x:(r)+n a,y:(rh)-
		unhandled("a,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2191: { // a,x:(r)+n b,y:(rh)-
		unhandled("a,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2192: { // b,x:(r)+n y0,y:(rh)-
		unhandled("b,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2193: { // b,x:(r)+n y1,y:(rh)-
		unhandled("b,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2194: { // b,x:(r)+n a,y:(rh)-
		unhandled("b,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2195: { // b,x:(r)+n b,y:(rh)-
		unhandled("b,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2196: { // x0,x:(r)+n y0,y:(rh)+
		unhandled("x0,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2197: { // x0,x:(r)+n y1,y:(rh)+
		unhandled("x0,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2198: { // x0,x:(r)+n a,y:(rh)+
		unhandled("x0,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2199: { // x0,x:(r)+n b,y:(rh)+
		unhandled("x0,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2200: { // x1,x:(r)+n y0,y:(rh)+
		unhandled("x1,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2201: { // x1,x:(r)+n y1,y:(rh)+
		unhandled("x1,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2202: { // x1,x:(r)+n a,y:(rh)+
		unhandled("x1,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2203: { // x1,x:(r)+n b,y:(rh)+
		unhandled("x1,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2204: { // a,x:(r)+n y0,y:(rh)+
		unhandled("a,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2205: { // a,x:(r)+n y1,y:(rh)+
		unhandled("a,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2206: { // a,x:(r)+n a,y:(rh)+
		unhandled("a,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2207: { // a,x:(r)+n b,y:(rh)+
		unhandled("a,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2208: { // b,x:(r)+n y0,y:(rh)+
		unhandled("b,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2209: { // b,x:(r)+n y1,y:(rh)+
		unhandled("b,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2210: { // b,x:(r)+n a,y:(rh)+
		unhandled("b,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2211: { // b,x:(r)+n b,y:(rh)+
		unhandled("b,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2212: { // x0,x:(r)+n y0,y:(rh)
		unhandled("x0,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2213: { // x0,x:(r)+n y1,y:(rh)
		unhandled("x0,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2214: { // x0,x:(r)+n a,y:(rh)
		unhandled("x0,x:(r)+n a,y:(rh)");
		break;
		}
	case 2215: { // x0,x:(r)+n b,y:(rh)
		unhandled("x0,x:(r)+n b,y:(rh)");
		break;
		}
	case 2216: { // x1,x:(r)+n y0,y:(rh)
		unhandled("x1,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2217: { // x1,x:(r)+n y1,y:(rh)
		unhandled("x1,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2218: { // x1,x:(r)+n a,y:(rh)
		unhandled("x1,x:(r)+n a,y:(rh)");
		break;
		}
	case 2219: { // x1,x:(r)+n b,y:(rh)
		unhandled("x1,x:(r)+n b,y:(rh)");
		break;
		}
	case 2220: { // a,x:(r)+n y0,y:(rh)
		unhandled("a,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2221: { // a,x:(r)+n y1,y:(rh)
		unhandled("a,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2222: { // a,x:(r)+n a,y:(rh)
		unhandled("a,x:(r)+n a,y:(rh)");
		break;
		}
	case 2223: { // a,x:(r)+n b,y:(rh)
		unhandled("a,x:(r)+n b,y:(rh)");
		break;
		}
	case 2224: { // b,x:(r)+n y0,y:(rh)
		unhandled("b,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2225: { // b,x:(r)+n y1,y:(rh)
		unhandled("b,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2226: { // b,x:(r)+n a,y:(rh)
		unhandled("b,x:(r)+n a,y:(rh)");
		break;
		}
	case 2227: { // b,x:(r)+n b,y:(rh)
		unhandled("b,x:(r)+n b,y:(rh)");
		break;
		}
	case 2228: { // x0,x:(r)- y0,y:(rh)+n
		unhandled("x0,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2229: { // x0,x:(r)- y1,y:(rh)+n
		unhandled("x0,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2230: { // x0,x:(r)- a,y:(rh)+n
		unhandled("x0,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2231: { // x0,x:(r)- b,y:(rh)+n
		unhandled("x0,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2232: { // x1,x:(r)- y0,y:(rh)+n
		unhandled("x1,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2233: { // x1,x:(r)- y1,y:(rh)+n
		unhandled("x1,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2234: { // x1,x:(r)- a,y:(rh)+n
		unhandled("x1,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2235: { // x1,x:(r)- b,y:(rh)+n
		unhandled("x1,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2236: { // a,x:(r)- y0,y:(rh)+n
		unhandled("a,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2237: { // a,x:(r)- y1,y:(rh)+n
		unhandled("a,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2238: { // a,x:(r)- a,y:(rh)+n
		unhandled("a,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2239: { // a,x:(r)- b,y:(rh)+n
		unhandled("a,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2240: { // b,x:(r)- y0,y:(rh)+n
		unhandled("b,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2241: { // b,x:(r)- y1,y:(rh)+n
		unhandled("b,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2242: { // b,x:(r)- a,y:(rh)+n
		unhandled("b,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2243: { // b,x:(r)- b,y:(rh)+n
		unhandled("b,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2244: { // x0,x:(r)- y0,y:(rh)-
		unhandled("x0,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2245: { // x0,x:(r)- y1,y:(rh)-
		unhandled("x0,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2246: { // x0,x:(r)- a,y:(rh)-
		unhandled("x0,x:(r)- a,y:(rh)-");
		break;
		}
	case 2247: { // x0,x:(r)- b,y:(rh)-
		unhandled("x0,x:(r)- b,y:(rh)-");
		break;
		}
	case 2248: { // x1,x:(r)- y0,y:(rh)-
		unhandled("x1,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2249: { // x1,x:(r)- y1,y:(rh)-
		unhandled("x1,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2250: { // x1,x:(r)- a,y:(rh)-
		unhandled("x1,x:(r)- a,y:(rh)-");
		break;
		}
	case 2251: { // x1,x:(r)- b,y:(rh)-
		unhandled("x1,x:(r)- b,y:(rh)-");
		break;
		}
	case 2252: { // a,x:(r)- y0,y:(rh)-
		unhandled("a,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2253: { // a,x:(r)- y1,y:(rh)-
		unhandled("a,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2254: { // a,x:(r)- a,y:(rh)-
		unhandled("a,x:(r)- a,y:(rh)-");
		break;
		}
	case 2255: { // a,x:(r)- b,y:(rh)-
		unhandled("a,x:(r)- b,y:(rh)-");
		break;
		}
	case 2256: { // b,x:(r)- y0,y:(rh)-
		unhandled("b,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2257: { // b,x:(r)- y1,y:(rh)-
		unhandled("b,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2258: { // b,x:(r)- a,y:(rh)-
		unhandled("b,x:(r)- a,y:(rh)-");
		break;
		}
	case 2259: { // b,x:(r)- b,y:(rh)-
		unhandled("b,x:(r)- b,y:(rh)-");
		break;
		}
	case 2260: { // x0,x:(r)- y0,y:(rh)+
		unhandled("x0,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2261: { // x0,x:(r)- y1,y:(rh)+
		unhandled("x0,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2262: { // x0,x:(r)- a,y:(rh)+
		unhandled("x0,x:(r)- a,y:(rh)+");
		break;
		}
	case 2263: { // x0,x:(r)- b,y:(rh)+
		unhandled("x0,x:(r)- b,y:(rh)+");
		break;
		}
	case 2264: { // x1,x:(r)- y0,y:(rh)+
		unhandled("x1,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2265: { // x1,x:(r)- y1,y:(rh)+
		unhandled("x1,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2266: { // x1,x:(r)- a,y:(rh)+
		unhandled("x1,x:(r)- a,y:(rh)+");
		break;
		}
	case 2267: { // x1,x:(r)- b,y:(rh)+
		unhandled("x1,x:(r)- b,y:(rh)+");
		break;
		}
	case 2268: { // a,x:(r)- y0,y:(rh)+
		unhandled("a,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2269: { // a,x:(r)- y1,y:(rh)+
		unhandled("a,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2270: { // a,x:(r)- a,y:(rh)+
		unhandled("a,x:(r)- a,y:(rh)+");
		break;
		}
	case 2271: { // a,x:(r)- b,y:(rh)+
		unhandled("a,x:(r)- b,y:(rh)+");
		break;
		}
	case 2272: { // b,x:(r)- y0,y:(rh)+
		unhandled("b,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2273: { // b,x:(r)- y1,y:(rh)+
		unhandled("b,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2274: { // b,x:(r)- a,y:(rh)+
		unhandled("b,x:(r)- a,y:(rh)+");
		break;
		}
	case 2275: { // b,x:(r)- b,y:(rh)+
		unhandled("b,x:(r)- b,y:(rh)+");
		break;
		}
	case 2276: { // x0,x:(r)- y0,y:(rh)
		unhandled("x0,x:(r)- y0,y:(rh)");
		break;
		}
	case 2277: { // x0,x:(r)- y1,y:(rh)
		unhandled("x0,x:(r)- y1,y:(rh)");
		break;
		}
	case 2278: { // x0,x:(r)- a,y:(rh)
		unhandled("x0,x:(r)- a,y:(rh)");
		break;
		}
	case 2279: { // x0,x:(r)- b,y:(rh)
		unhandled("x0,x:(r)- b,y:(rh)");
		break;
		}
	case 2280: { // x1,x:(r)- y0,y:(rh)
		unhandled("x1,x:(r)- y0,y:(rh)");
		break;
		}
	case 2281: { // x1,x:(r)- y1,y:(rh)
		unhandled("x1,x:(r)- y1,y:(rh)");
		break;
		}
	case 2282: { // x1,x:(r)- a,y:(rh)
		unhandled("x1,x:(r)- a,y:(rh)");
		break;
		}
	case 2283: { // x1,x:(r)- b,y:(rh)
		unhandled("x1,x:(r)- b,y:(rh)");
		break;
		}
	case 2284: { // a,x:(r)- y0,y:(rh)
		unhandled("a,x:(r)- y0,y:(rh)");
		break;
		}
	case 2285: { // a,x:(r)- y1,y:(rh)
		unhandled("a,x:(r)- y1,y:(rh)");
		break;
		}
	case 2286: { // a,x:(r)- a,y:(rh)
		unhandled("a,x:(r)- a,y:(rh)");
		break;
		}
	case 2287: { // a,x:(r)- b,y:(rh)
		unhandled("a,x:(r)- b,y:(rh)");
		break;
		}
	case 2288: { // b,x:(r)- y0,y:(rh)
		unhandled("b,x:(r)- y0,y:(rh)");
		break;
		}
	case 2289: { // b,x:(r)- y1,y:(rh)
		unhandled("b,x:(r)- y1,y:(rh)");
		break;
		}
	case 2290: { // b,x:(r)- a,y:(rh)
		unhandled("b,x:(r)- a,y:(rh)");
		break;
		}
	case 2291: { // b,x:(r)- b,y:(rh)
		unhandled("b,x:(r)- b,y:(rh)");
		break;
		}
	case 2292: { // x0,x:(r)+ y0,y:(rh)+n
		unhandled("x0,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2293: { // x0,x:(r)+ y1,y:(rh)+n
		unhandled("x0,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2294: { // x0,x:(r)+ a,y:(rh)+n
		unhandled("x0,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2295: { // x0,x:(r)+ b,y:(rh)+n
		unhandled("x0,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2296: { // x1,x:(r)+ y0,y:(rh)+n
		unhandled("x1,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2297: { // x1,x:(r)+ y1,y:(rh)+n
		unhandled("x1,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2298: { // x1,x:(r)+ a,y:(rh)+n
		unhandled("x1,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2299: { // x1,x:(r)+ b,y:(rh)+n
		unhandled("x1,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2300: { // a,x:(r)+ y0,y:(rh)+n
		unhandled("a,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2301: { // a,x:(r)+ y1,y:(rh)+n
		unhandled("a,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2302: { // a,x:(r)+ a,y:(rh)+n
		unhandled("a,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2303: { // a,x:(r)+ b,y:(rh)+n
		unhandled("a,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2304: { // b,x:(r)+ y0,y:(rh)+n
		unhandled("b,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2305: { // b,x:(r)+ y1,y:(rh)+n
		unhandled("b,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2306: { // b,x:(r)+ a,y:(rh)+n
		unhandled("b,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2307: { // b,x:(r)+ b,y:(rh)+n
		unhandled("b,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2308: { // x0,x:(r)+ y0,y:(rh)-
		unhandled("x0,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2309: { // x0,x:(r)+ y1,y:(rh)-
		unhandled("x0,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2310: { // x0,x:(r)+ a,y:(rh)-
		unhandled("x0,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2311: { // x0,x:(r)+ b,y:(rh)-
		unhandled("x0,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2312: { // x1,x:(r)+ y0,y:(rh)-
		unhandled("x1,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2313: { // x1,x:(r)+ y1,y:(rh)-
		unhandled("x1,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2314: { // x1,x:(r)+ a,y:(rh)-
		unhandled("x1,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2315: { // x1,x:(r)+ b,y:(rh)-
		unhandled("x1,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2316: { // a,x:(r)+ y0,y:(rh)-
		unhandled("a,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2317: { // a,x:(r)+ y1,y:(rh)-
		unhandled("a,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2318: { // a,x:(r)+ a,y:(rh)-
		unhandled("a,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2319: { // a,x:(r)+ b,y:(rh)-
		unhandled("a,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2320: { // b,x:(r)+ y0,y:(rh)-
		unhandled("b,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2321: { // b,x:(r)+ y1,y:(rh)-
		unhandled("b,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2322: { // b,x:(r)+ a,y:(rh)-
		unhandled("b,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2323: { // b,x:(r)+ b,y:(rh)-
		unhandled("b,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2324: { // x0,x:(r)+ y0,y:(rh)+
		unhandled("x0,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2325: { // x0,x:(r)+ y1,y:(rh)+
		unhandled("x0,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2326: { // x0,x:(r)+ a,y:(rh)+
		unhandled("x0,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2327: { // x0,x:(r)+ b,y:(rh)+
		unhandled("x0,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2328: { // x1,x:(r)+ y0,y:(rh)+
		unhandled("x1,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2329: { // x1,x:(r)+ y1,y:(rh)+
		unhandled("x1,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2330: { // x1,x:(r)+ a,y:(rh)+
		unhandled("x1,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2331: { // x1,x:(r)+ b,y:(rh)+
		unhandled("x1,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2332: { // a,x:(r)+ y0,y:(rh)+
		unhandled("a,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2333: { // a,x:(r)+ y1,y:(rh)+
		unhandled("a,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2334: { // a,x:(r)+ a,y:(rh)+
		unhandled("a,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2335: { // a,x:(r)+ b,y:(rh)+
		unhandled("a,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2336: { // b,x:(r)+ y0,y:(rh)+
		unhandled("b,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2337: { // b,x:(r)+ y1,y:(rh)+
		unhandled("b,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2338: { // b,x:(r)+ a,y:(rh)+
		unhandled("b,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2339: { // b,x:(r)+ b,y:(rh)+
		unhandled("b,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2340: { // x0,x:(r)+ y0,y:(rh)
		unhandled("x0,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2341: { // x0,x:(r)+ y1,y:(rh)
		unhandled("x0,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2342: { // x0,x:(r)+ a,y:(rh)
		unhandled("x0,x:(r)+ a,y:(rh)");
		break;
		}
	case 2343: { // x0,x:(r)+ b,y:(rh)
		unhandled("x0,x:(r)+ b,y:(rh)");
		break;
		}
	case 2344: { // x1,x:(r)+ y0,y:(rh)
		unhandled("x1,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2345: { // x1,x:(r)+ y1,y:(rh)
		unhandled("x1,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2346: { // x1,x:(r)+ a,y:(rh)
		unhandled("x1,x:(r)+ a,y:(rh)");
		break;
		}
	case 2347: { // x1,x:(r)+ b,y:(rh)
		unhandled("x1,x:(r)+ b,y:(rh)");
		break;
		}
	case 2348: { // a,x:(r)+ y0,y:(rh)
		unhandled("a,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2349: { // a,x:(r)+ y1,y:(rh)
		unhandled("a,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2350: { // a,x:(r)+ a,y:(rh)
		unhandled("a,x:(r)+ a,y:(rh)");
		break;
		}
	case 2351: { // a,x:(r)+ b,y:(rh)
		unhandled("a,x:(r)+ b,y:(rh)");
		break;
		}
	case 2352: { // b,x:(r)+ y0,y:(rh)
		unhandled("b,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2353: { // b,x:(r)+ y1,y:(rh)
		unhandled("b,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2354: { // b,x:(r)+ a,y:(rh)
		unhandled("b,x:(r)+ a,y:(rh)");
		break;
		}
	case 2355: { // b,x:(r)+ b,y:(rh)
		unhandled("b,x:(r)+ b,y:(rh)");
		break;
		}
	case 2356: { // x0,x:(r) y0,y:(rh)+n
		unhandled("x0,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2357: { // x0,x:(r) y1,y:(rh)+n
		unhandled("x0,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2358: { // x0,x:(r) a,y:(rh)+n
		unhandled("x0,x:(r) a,y:(rh)+n");
		break;
		}
	case 2359: { // x0,x:(r) b,y:(rh)+n
		unhandled("x0,x:(r) b,y:(rh)+n");
		break;
		}
	case 2360: { // x1,x:(r) y0,y:(rh)+n
		unhandled("x1,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2361: { // x1,x:(r) y1,y:(rh)+n
		unhandled("x1,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2362: { // x1,x:(r) a,y:(rh)+n
		unhandled("x1,x:(r) a,y:(rh)+n");
		break;
		}
	case 2363: { // x1,x:(r) b,y:(rh)+n
		unhandled("x1,x:(r) b,y:(rh)+n");
		break;
		}
	case 2364: { // a,x:(r) y0,y:(rh)+n
		unhandled("a,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2365: { // a,x:(r) y1,y:(rh)+n
		unhandled("a,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2366: { // a,x:(r) a,y:(rh)+n
		unhandled("a,x:(r) a,y:(rh)+n");
		break;
		}
	case 2367: { // a,x:(r) b,y:(rh)+n
		unhandled("a,x:(r) b,y:(rh)+n");
		break;
		}
	case 2368: { // b,x:(r) y0,y:(rh)+n
		unhandled("b,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2369: { // b,x:(r) y1,y:(rh)+n
		unhandled("b,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2370: { // b,x:(r) a,y:(rh)+n
		unhandled("b,x:(r) a,y:(rh)+n");
		break;
		}
	case 2371: { // b,x:(r) b,y:(rh)+n
		unhandled("b,x:(r) b,y:(rh)+n");
		break;
		}
	case 2372: { // x0,x:(r) y0,y:(rh)-
		unhandled("x0,x:(r) y0,y:(rh)-");
		break;
		}
	case 2373: { // x0,x:(r) y1,y:(rh)-
		unhandled("x0,x:(r) y1,y:(rh)-");
		break;
		}
	case 2374: { // x0,x:(r) a,y:(rh)-
		unhandled("x0,x:(r) a,y:(rh)-");
		break;
		}
	case 2375: { // x0,x:(r) b,y:(rh)-
		unhandled("x0,x:(r) b,y:(rh)-");
		break;
		}
	case 2376: { // x1,x:(r) y0,y:(rh)-
		unhandled("x1,x:(r) y0,y:(rh)-");
		break;
		}
	case 2377: { // x1,x:(r) y1,y:(rh)-
		unhandled("x1,x:(r) y1,y:(rh)-");
		break;
		}
	case 2378: { // x1,x:(r) a,y:(rh)-
		unhandled("x1,x:(r) a,y:(rh)-");
		break;
		}
	case 2379: { // x1,x:(r) b,y:(rh)-
		unhandled("x1,x:(r) b,y:(rh)-");
		break;
		}
	case 2380: { // a,x:(r) y0,y:(rh)-
		unhandled("a,x:(r) y0,y:(rh)-");
		break;
		}
	case 2381: { // a,x:(r) y1,y:(rh)-
		unhandled("a,x:(r) y1,y:(rh)-");
		break;
		}
	case 2382: { // a,x:(r) a,y:(rh)-
		unhandled("a,x:(r) a,y:(rh)-");
		break;
		}
	case 2383: { // a,x:(r) b,y:(rh)-
		unhandled("a,x:(r) b,y:(rh)-");
		break;
		}
	case 2384: { // b,x:(r) y0,y:(rh)-
		unhandled("b,x:(r) y0,y:(rh)-");
		break;
		}
	case 2385: { // b,x:(r) y1,y:(rh)-
		unhandled("b,x:(r) y1,y:(rh)-");
		break;
		}
	case 2386: { // b,x:(r) a,y:(rh)-
		unhandled("b,x:(r) a,y:(rh)-");
		break;
		}
	case 2387: { // b,x:(r) b,y:(rh)-
		unhandled("b,x:(r) b,y:(rh)-");
		break;
		}
	case 2388: { // x0,x:(r) y0,y:(rh)+
		unhandled("x0,x:(r) y0,y:(rh)+");
		break;
		}
	case 2389: { // x0,x:(r) y1,y:(rh)+
		unhandled("x0,x:(r) y1,y:(rh)+");
		break;
		}
	case 2390: { // x0,x:(r) a,y:(rh)+
		unhandled("x0,x:(r) a,y:(rh)+");
		break;
		}
	case 2391: { // x0,x:(r) b,y:(rh)+
		unhandled("x0,x:(r) b,y:(rh)+");
		break;
		}
	case 2392: { // x1,x:(r) y0,y:(rh)+
		unhandled("x1,x:(r) y0,y:(rh)+");
		break;
		}
	case 2393: { // x1,x:(r) y1,y:(rh)+
		unhandled("x1,x:(r) y1,y:(rh)+");
		break;
		}
	case 2394: { // x1,x:(r) a,y:(rh)+
		unhandled("x1,x:(r) a,y:(rh)+");
		break;
		}
	case 2395: { // x1,x:(r) b,y:(rh)+
		unhandled("x1,x:(r) b,y:(rh)+");
		break;
		}
	case 2396: { // a,x:(r) y0,y:(rh)+
		unhandled("a,x:(r) y0,y:(rh)+");
		break;
		}
	case 2397: { // a,x:(r) y1,y:(rh)+
		unhandled("a,x:(r) y1,y:(rh)+");
		break;
		}
	case 2398: { // a,x:(r) a,y:(rh)+
		unhandled("a,x:(r) a,y:(rh)+");
		break;
		}
	case 2399: { // a,x:(r) b,y:(rh)+
		unhandled("a,x:(r) b,y:(rh)+");
		break;
		}
	case 2400: { // b,x:(r) y0,y:(rh)+
		unhandled("b,x:(r) y0,y:(rh)+");
		break;
		}
	case 2401: { // b,x:(r) y1,y:(rh)+
		unhandled("b,x:(r) y1,y:(rh)+");
		break;
		}
	case 2402: { // b,x:(r) a,y:(rh)+
		unhandled("b,x:(r) a,y:(rh)+");
		break;
		}
	case 2403: { // b,x:(r) b,y:(rh)+
		unhandled("b,x:(r) b,y:(rh)+");
		break;
		}
	case 2404: { // x0,x:(r) y0,y:(rh)
		unhandled("x0,x:(r) y0,y:(rh)");
		break;
		}
	case 2405: { // x0,x:(r) y1,y:(rh)
		unhandled("x0,x:(r) y1,y:(rh)");
		break;
		}
	case 2406: { // x0,x:(r) a,y:(rh)
		unhandled("x0,x:(r) a,y:(rh)");
		break;
		}
	case 2407: { // x0,x:(r) b,y:(rh)
		unhandled("x0,x:(r) b,y:(rh)");
		break;
		}
	case 2408: { // x1,x:(r) y0,y:(rh)
		unhandled("x1,x:(r) y0,y:(rh)");
		break;
		}
	case 2409: { // x1,x:(r) y1,y:(rh)
		unhandled("x1,x:(r) y1,y:(rh)");
		break;
		}
	case 2410: { // x1,x:(r) a,y:(rh)
		unhandled("x1,x:(r) a,y:(rh)");
		break;
		}
	case 2411: { // x1,x:(r) b,y:(rh)
		unhandled("x1,x:(r) b,y:(rh)");
		break;
		}
	case 2412: { // a,x:(r) y0,y:(rh)
		unhandled("a,x:(r) y0,y:(rh)");
		break;
		}
	case 2413: { // a,x:(r) y1,y:(rh)
		unhandled("a,x:(r) y1,y:(rh)");
		break;
		}
	case 2414: { // a,x:(r) a,y:(rh)
		unhandled("a,x:(r) a,y:(rh)");
		break;
		}
	case 2415: { // a,x:(r) b,y:(rh)
		unhandled("a,x:(r) b,y:(rh)");
		break;
		}
	case 2416: { // b,x:(r) y0,y:(rh)
		unhandled("b,x:(r) y0,y:(rh)");
		break;
		}
	case 2417: { // b,x:(r) y1,y:(rh)
		unhandled("b,x:(r) y1,y:(rh)");
		break;
		}
	case 2418: { // b,x:(r) a,y:(rh)
		unhandled("b,x:(r) a,y:(rh)");
		break;
		}
	case 2419: { // b,x:(r) b,y:(rh)
		unhandled("b,x:(r) b,y:(rh)");
		break;
		}
	case 2420: { // ifcc
		unhandled("ifcc");
		break;
		}
	case 2421: { // ifge
		unhandled("ifge");
		break;
		}
	case 2422: { // ifne
		unhandled("ifne");
		break;
		}
	case 2423: { // ifpl
		unhandled("ifpl");
		break;
		}
	case 2424: { // ifnn
		unhandled("ifnn");
		break;
		}
	case 2425: { // ifec
		unhandled("ifec");
		break;
		}
	case 2426: { // iflc
		unhandled("iflc");
		break;
		}
	case 2427: { // ifgt
		unhandled("ifgt");
		break;
		}
	case 2428: { // ifcs
		unhandled("ifcs");
		break;
		}
	case 2429: { // iflt
		unhandled("iflt");
		break;
		}
	case 2430: { // ifeq
		unhandled("ifeq");
		break;
		}
	case 2431: { // ifmi
		unhandled("ifmi");
		break;
		}
	case 2432: { // ifnr
		unhandled("ifnr");
		break;
		}
	case 2433: { // ifes
		unhandled("ifes");
		break;
		}
	case 2434: { // ifls
		unhandled("ifls");
		break;
		}
	case 2435: { // ifle
		unhandled("ifle");
		break;
		}
	case 2436: { // ifcc.u
		unhandled("ifcc.u");
		break;
		}
	case 2437: { // ifge.u
		unhandled("ifge.u");
		break;
		}
	case 2438: { // ifne.u
		unhandled("ifne.u");
		break;
		}
	case 2439: { // ifpl.u
		unhandled("ifpl.u");
		break;
		}
	case 2440: { // ifnn.u
		unhandled("ifnn.u");
		break;
		}
	case 2441: { // ifec.u
		unhandled("ifec.u");
		break;
		}
	case 2442: { // iflc.u
		unhandled("iflc.u");
		break;
		}
	case 2443: { // ifgt.u
		unhandled("ifgt.u");
		break;
		}
	case 2444: { // ifcs.u
		unhandled("ifcs.u");
		break;
		}
	case 2445: { // iflt.u
		unhandled("iflt.u");
		break;
		}
	case 2446: { // ifeq.u
		unhandled("ifeq.u");
		break;
		}
	case 2447: { // ifmi.u
		unhandled("ifmi.u");
		break;
		}
	case 2448: { // ifnr.u
		unhandled("ifnr.u");
		break;
		}
	case 2449: { // ifes.u
		unhandled("ifes.u");
		break;
		}
	case 2450: { // ifls.u
		unhandled("ifls.u");
		break;
		}
	case 2451: { // ifle.u
		unhandled("ifle.u");
		break;
		}
	}
}

void dsp563xx_device::execute_post_move(u16 kmove, u32 opcode, u32 exv)
{
	switch(kmove) {
	case 0: { // -
		break;
		}
	case 1: { // 
		break;
		}
	case 2: { // #[i],x0
		u32 i = BIT(opcode, 8, 8);
		set_x0(i);
		break;
		}
	case 3: { // #[i],x1
		u32 i = BIT(opcode, 8, 8);
		set_x1(i);
		break;
		}
	case 4: { // #[i],y0
		u32 i = BIT(opcode, 8, 8);
		set_y0(i);
		break;
		}
	case 5: { // #[i],y1
		u32 i = BIT(opcode, 8, 8);
		set_y1(i);
		break;
		}
	case 6: { // #[i],a0
		u32 i = BIT(opcode, 8, 8);
		set_a0(i);
		break;
		}
	case 7: { // #[i],b0
		u32 i = BIT(opcode, 8, 8);
		set_b0(i);
		break;
		}
	case 8: { // #[i],a2
		u32 i = BIT(opcode, 8, 8);
		set_a2(i);
		break;
		}
	case 9: { // #[i],b2
		u32 i = BIT(opcode, 8, 8);
		set_b2(i);
		break;
		}
	case 10: { // #[i],a1
		u32 i = BIT(opcode, 8, 8);
		set_a1(i);
		break;
		}
	case 11: { // #[i],b1
		u32 i = BIT(opcode, 8, 8);
		set_b1(i);
		break;
		}
	case 12: { // #[i],a
		u32 i = BIT(opcode, 8, 8);
		set_a(i);
		break;
		}
	case 13: { // #[i],b
		u32 i = BIT(opcode, 8, 8);
		set_b(i);
		break;
		}
	case 14: { // #[i],r
		u32 i = BIT(opcode, 8, 8);
		set_r(BIT(opcode, 16, 5) & 7, i);
		break;
		}
	case 15: { // #[i],n
		u32 i = BIT(opcode, 8, 8);
		set_n(BIT(opcode, 16, 5) & 7, i);
		break;
		}
	case 16: { // x0,x0
		set_x0(m_tmp1);
		break;
		}
	case 17: { // x0,x1
		set_x1(m_tmp1);
		break;
		}
	case 18: { // x0,y0
		set_y0(m_tmp1);
		break;
		}
	case 19: { // x0,y1
		set_y1(m_tmp1);
		break;
		}
	case 20: { // x0,a0
		set_a0(m_tmp1);
		break;
		}
	case 21: { // x0,b0
		set_b0(m_tmp1);
		break;
		}
	case 22: { // x0,a2
		set_a2(m_tmp1);
		break;
		}
	case 23: { // x0,b2
		set_b2(m_tmp1);
		break;
		}
	case 24: { // x0,a1
		set_a1(m_tmp1);
		break;
		}
	case 25: { // x0,b1
		set_b1(m_tmp1);
		break;
		}
	case 26: { // x0,a
		set_a(m_tmp1);
		break;
		}
	case 27: { // x0,b
		set_b(m_tmp1);
		break;
		}
	case 28: { // x0,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 29: { // x0,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 30: { // x1,x0
		set_x0(m_tmp1);
		break;
		}
	case 31: { // x1,x1
		set_x1(m_tmp1);
		break;
		}
	case 32: { // x1,y0
		set_y0(m_tmp1);
		break;
		}
	case 33: { // x1,y1
		set_y1(m_tmp1);
		break;
		}
	case 34: { // x1,a0
		set_a0(m_tmp1);
		break;
		}
	case 35: { // x1,b0
		set_b0(m_tmp1);
		break;
		}
	case 36: { // x1,a2
		set_a2(m_tmp1);
		break;
		}
	case 37: { // x1,b2
		set_b2(m_tmp1);
		break;
		}
	case 38: { // x1,a1
		set_a1(m_tmp1);
		break;
		}
	case 39: { // x1,b1
		set_b1(m_tmp1);
		break;
		}
	case 40: { // x1,a
		set_a(m_tmp1);
		break;
		}
	case 41: { // x1,b
		set_b(m_tmp1);
		break;
		}
	case 42: { // x1,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 43: { // x1,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 44: { // y0,x0
		set_x0(m_tmp1);
		break;
		}
	case 45: { // y0,x1
		set_x1(m_tmp1);
		break;
		}
	case 46: { // y0,y0
		set_y0(m_tmp1);
		break;
		}
	case 47: { // y0,y1
		set_y1(m_tmp1);
		break;
		}
	case 48: { // y0,a0
		set_a0(m_tmp1);
		break;
		}
	case 49: { // y0,b0
		set_b0(m_tmp1);
		break;
		}
	case 50: { // y0,a2
		set_a2(m_tmp1);
		break;
		}
	case 51: { // y0,b2
		set_b2(m_tmp1);
		break;
		}
	case 52: { // y0,a1
		set_a1(m_tmp1);
		break;
		}
	case 53: { // y0,b1
		set_b1(m_tmp1);
		break;
		}
	case 54: { // y0,a
		set_a(m_tmp1);
		break;
		}
	case 55: { // y0,b
		set_b(m_tmp1);
		break;
		}
	case 56: { // y0,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 57: { // y0,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 58: { // y1,x0
		set_x0(m_tmp1);
		break;
		}
	case 59: { // y1,x1
		set_x1(m_tmp1);
		break;
		}
	case 60: { // y1,y0
		set_y0(m_tmp1);
		break;
		}
	case 61: { // y1,y1
		set_y1(m_tmp1);
		break;
		}
	case 62: { // y1,a0
		set_a0(m_tmp1);
		break;
		}
	case 63: { // y1,b0
		set_b0(m_tmp1);
		break;
		}
	case 64: { // y1,a2
		set_a2(m_tmp1);
		break;
		}
	case 65: { // y1,b2
		set_b2(m_tmp1);
		break;
		}
	case 66: { // y1,a1
		set_a1(m_tmp1);
		break;
		}
	case 67: { // y1,b1
		set_b1(m_tmp1);
		break;
		}
	case 68: { // y1,a
		set_a(m_tmp1);
		break;
		}
	case 69: { // y1,b
		set_b(m_tmp1);
		break;
		}
	case 70: { // y1,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 71: { // y1,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 72: { // a0,x0
		set_x0(m_tmp1);
		break;
		}
	case 73: { // a0,x1
		set_x1(m_tmp1);
		break;
		}
	case 74: { // a0,y0
		set_y0(m_tmp1);
		break;
		}
	case 75: { // a0,y1
		set_y1(m_tmp1);
		break;
		}
	case 76: { // a0,a0
		set_a0(m_tmp1);
		break;
		}
	case 77: { // a0,b0
		set_b0(m_tmp1);
		break;
		}
	case 78: { // a0,a2
		set_a2(m_tmp1);
		break;
		}
	case 79: { // a0,b2
		set_b2(m_tmp1);
		break;
		}
	case 80: { // a0,a1
		set_a1(m_tmp1);
		break;
		}
	case 81: { // a0,b1
		set_b1(m_tmp1);
		break;
		}
	case 82: { // a0,a
		set_a(m_tmp1);
		break;
		}
	case 83: { // a0,b
		set_b(m_tmp1);
		break;
		}
	case 84: { // a0,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 85: { // a0,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 86: { // b0,x0
		set_x0(m_tmp1);
		break;
		}
	case 87: { // b0,x1
		set_x1(m_tmp1);
		break;
		}
	case 88: { // b0,y0
		set_y0(m_tmp1);
		break;
		}
	case 89: { // b0,y1
		set_y1(m_tmp1);
		break;
		}
	case 90: { // b0,a0
		set_a0(m_tmp1);
		break;
		}
	case 91: { // b0,b0
		set_b0(m_tmp1);
		break;
		}
	case 92: { // b0,a2
		set_a2(m_tmp1);
		break;
		}
	case 93: { // b0,b2
		set_b2(m_tmp1);
		break;
		}
	case 94: { // b0,a1
		set_a1(m_tmp1);
		break;
		}
	case 95: { // b0,b1
		set_b1(m_tmp1);
		break;
		}
	case 96: { // b0,a
		set_a(m_tmp1);
		break;
		}
	case 97: { // b0,b
		set_b(m_tmp1);
		break;
		}
	case 98: { // b0,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 99: { // b0,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 100: { // a2,x0
		set_x0(m_tmp1);
		break;
		}
	case 101: { // a2,x1
		set_x1(m_tmp1);
		break;
		}
	case 102: { // a2,y0
		set_y0(m_tmp1);
		break;
		}
	case 103: { // a2,y1
		set_y1(m_tmp1);
		break;
		}
	case 104: { // a2,a0
		set_a0(m_tmp1);
		break;
		}
	case 105: { // a2,b0
		set_b0(m_tmp1);
		break;
		}
	case 106: { // a2,a2
		set_a2(m_tmp1);
		break;
		}
	case 107: { // a2,b2
		set_b2(m_tmp1);
		break;
		}
	case 108: { // a2,a1
		set_a1(m_tmp1);
		break;
		}
	case 109: { // a2,b1
		set_b1(m_tmp1);
		break;
		}
	case 110: { // a2,a
		set_a(m_tmp1);
		break;
		}
	case 111: { // a2,b
		set_b(m_tmp1);
		break;
		}
	case 112: { // a2,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 113: { // a2,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 114: { // b2,x0
		set_x0(m_tmp1);
		break;
		}
	case 115: { // b2,x1
		set_x1(m_tmp1);
		break;
		}
	case 116: { // b2,y0
		set_y0(m_tmp1);
		break;
		}
	case 117: { // b2,y1
		set_y1(m_tmp1);
		break;
		}
	case 118: { // b2,a0
		set_a0(m_tmp1);
		break;
		}
	case 119: { // b2,b0
		set_b0(m_tmp1);
		break;
		}
	case 120: { // b2,a2
		set_a2(m_tmp1);
		break;
		}
	case 121: { // b2,b2
		set_b2(m_tmp1);
		break;
		}
	case 122: { // b2,a1
		set_a1(m_tmp1);
		break;
		}
	case 123: { // b2,b1
		set_b1(m_tmp1);
		break;
		}
	case 124: { // b2,a
		set_a(m_tmp1);
		break;
		}
	case 125: { // b2,b
		set_b(m_tmp1);
		break;
		}
	case 126: { // b2,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 127: { // b2,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 128: { // a1,x0
		set_x0(m_tmp1);
		break;
		}
	case 129: { // a1,x1
		set_x1(m_tmp1);
		break;
		}
	case 130: { // a1,y0
		set_y0(m_tmp1);
		break;
		}
	case 131: { // a1,y1
		set_y1(m_tmp1);
		break;
		}
	case 132: { // a1,a0
		set_a0(m_tmp1);
		break;
		}
	case 133: { // a1,b0
		set_b0(m_tmp1);
		break;
		}
	case 134: { // a1,a2
		set_a2(m_tmp1);
		break;
		}
	case 135: { // a1,b2
		set_b2(m_tmp1);
		break;
		}
	case 136: { // a1,a1
		set_a1(m_tmp1);
		break;
		}
	case 137: { // a1,b1
		set_b1(m_tmp1);
		break;
		}
	case 138: { // a1,a
		set_a(m_tmp1);
		break;
		}
	case 139: { // a1,b
		set_b(m_tmp1);
		break;
		}
	case 140: { // a1,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 141: { // a1,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 142: { // b1,x0
		set_x0(m_tmp1);
		break;
		}
	case 143: { // b1,x1
		set_x1(m_tmp1);
		break;
		}
	case 144: { // b1,y0
		set_y0(m_tmp1);
		break;
		}
	case 145: { // b1,y1
		set_y1(m_tmp1);
		break;
		}
	case 146: { // b1,a0
		set_a0(m_tmp1);
		break;
		}
	case 147: { // b1,b0
		set_b0(m_tmp1);
		break;
		}
	case 148: { // b1,a2
		set_a2(m_tmp1);
		break;
		}
	case 149: { // b1,b2
		set_b2(m_tmp1);
		break;
		}
	case 150: { // b1,a1
		set_a1(m_tmp1);
		break;
		}
	case 151: { // b1,b1
		set_b1(m_tmp1);
		break;
		}
	case 152: { // b1,a
		set_a(m_tmp1);
		break;
		}
	case 153: { // b1,b
		set_b(m_tmp1);
		break;
		}
	case 154: { // b1,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 155: { // b1,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 156: { // a,x0
		set_x0(m_tmp1);
		break;
		}
	case 157: { // a,x1
		set_x1(m_tmp1);
		break;
		}
	case 158: { // a,y0
		set_y0(m_tmp1);
		break;
		}
	case 159: { // a,y1
		set_y1(m_tmp1);
		break;
		}
	case 160: { // a,a0
		set_a0(m_tmp1);
		break;
		}
	case 161: { // a,b0
		set_b0(m_tmp1);
		break;
		}
	case 162: { // a,a2
		set_a2(m_tmp1);
		break;
		}
	case 163: { // a,b2
		set_b2(m_tmp1);
		break;
		}
	case 164: { // a,a1
		set_a1(m_tmp1);
		break;
		}
	case 165: { // a,b1
		set_b1(m_tmp1);
		break;
		}
	case 166: { // a,a
		set_a(m_tmp1);
		break;
		}
	case 167: { // a,b
		set_b(m_tmp1);
		break;
		}
	case 168: { // a,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 169: { // a,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 170: { // b,x0
		set_x0(m_tmp1);
		break;
		}
	case 171: { // b,x1
		set_x1(m_tmp1);
		break;
		}
	case 172: { // b,y0
		set_y0(m_tmp1);
		break;
		}
	case 173: { // b,y1
		set_y1(m_tmp1);
		break;
		}
	case 174: { // b,a0
		set_a0(m_tmp1);
		break;
		}
	case 175: { // b,b0
		set_b0(m_tmp1);
		break;
		}
	case 176: { // b,a2
		set_a2(m_tmp1);
		break;
		}
	case 177: { // b,b2
		set_b2(m_tmp1);
		break;
		}
	case 178: { // b,a1
		set_a1(m_tmp1);
		break;
		}
	case 179: { // b,b1
		set_b1(m_tmp1);
		break;
		}
	case 180: { // b,a
		set_a(m_tmp1);
		break;
		}
	case 181: { // b,b
		set_b(m_tmp1);
		break;
		}
	case 182: { // b,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 183: { // b,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 184: { // r,x0
		set_x0(m_tmp1);
		break;
		}
	case 185: { // r,x1
		set_x1(m_tmp1);
		break;
		}
	case 186: { // r,y0
		set_y0(m_tmp1);
		break;
		}
	case 187: { // r,y1
		set_y1(m_tmp1);
		break;
		}
	case 188: { // r,a0
		set_a0(m_tmp1);
		break;
		}
	case 189: { // r,b0
		set_b0(m_tmp1);
		break;
		}
	case 190: { // r,a2
		set_a2(m_tmp1);
		break;
		}
	case 191: { // r,b2
		set_b2(m_tmp1);
		break;
		}
	case 192: { // r,a1
		set_a1(m_tmp1);
		break;
		}
	case 193: { // r,b1
		set_b1(m_tmp1);
		break;
		}
	case 194: { // r,a
		set_a(m_tmp1);
		break;
		}
	case 195: { // r,b
		set_b(m_tmp1);
		break;
		}
	case 196: { // r,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 197: { // r,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 198: { // n,x0
		set_x0(m_tmp1);
		break;
		}
	case 199: { // n,x1
		set_x1(m_tmp1);
		break;
		}
	case 200: { // n,y0
		set_y0(m_tmp1);
		break;
		}
	case 201: { // n,y1
		set_y1(m_tmp1);
		break;
		}
	case 202: { // n,a0
		set_a0(m_tmp1);
		break;
		}
	case 203: { // n,b0
		set_b0(m_tmp1);
		break;
		}
	case 204: { // n,a2
		set_a2(m_tmp1);
		break;
		}
	case 205: { // n,b2
		set_b2(m_tmp1);
		break;
		}
	case 206: { // n,a1
		set_a1(m_tmp1);
		break;
		}
	case 207: { // n,b1
		set_b1(m_tmp1);
		break;
		}
	case 208: { // n,a
		set_a(m_tmp1);
		break;
		}
	case 209: { // n,b
		set_b(m_tmp1);
		break;
		}
	case 210: { // n,r
		set_r(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 211: { // n,n
		set_n(BIT(opcode, 8, 5) & 7, m_tmp1);
		break;
		}
	case 212: { // (r)-n
		unhandled("(r)-n");
		break;
		}
	case 213: { // (r)+n
		unhandled("(r)+n");
		break;
		}
	case 214: { // (r)-
		unhandled("(r)-");
		break;
		}
	case 215: { // (r)+
		unhandled("(r)+");
		break;
		}
	case 216: { // x:(r)-n,x0
		unhandled("x:(r)-n,x0");
		break;
		}
	case 217: { // x:(r)+n,x0
		unhandled("x:(r)+n,x0");
		break;
		}
	case 218: { // x:(r)-,x0
		unhandled("x:(r)-,x0");
		break;
		}
	case 219: { // x:(r)+,x0
		unhandled("x:(r)+,x0");
		break;
		}
	case 220: { // x:(r),x0
		unhandled("x:(r),x0");
		break;
		}
	case 221: { // x:(r+n),x0
		unhandled("x:(r+n),x0");
		break;
		}
	case 222: { // x:-(r),x0
		unhandled("x:-(r),x0");
		break;
		}
	case 223: { // x:(r)-n,x1
		unhandled("x:(r)-n,x1");
		break;
		}
	case 224: { // x:(r)+n,x1
		unhandled("x:(r)+n,x1");
		break;
		}
	case 225: { // x:(r)-,x1
		unhandled("x:(r)-,x1");
		break;
		}
	case 226: { // x:(r)+,x1
		unhandled("x:(r)+,x1");
		break;
		}
	case 227: { // x:(r),x1
		unhandled("x:(r),x1");
		break;
		}
	case 228: { // x:(r+n),x1
		unhandled("x:(r+n),x1");
		break;
		}
	case 229: { // x:-(r),x1
		unhandled("x:-(r),x1");
		break;
		}
	case 230: { // x:(r)-n,y0
		unhandled("x:(r)-n,y0");
		break;
		}
	case 231: { // x:(r)+n,y0
		unhandled("x:(r)+n,y0");
		break;
		}
	case 232: { // x:(r)-,y0
		unhandled("x:(r)-,y0");
		break;
		}
	case 233: { // x:(r)+,y0
		unhandled("x:(r)+,y0");
		break;
		}
	case 234: { // x:(r),y0
		unhandled("x:(r),y0");
		break;
		}
	case 235: { // x:(r+n),y0
		unhandled("x:(r+n),y0");
		break;
		}
	case 236: { // x:-(r),y0
		unhandled("x:-(r),y0");
		break;
		}
	case 237: { // x:(r)-n,y1
		unhandled("x:(r)-n,y1");
		break;
		}
	case 238: { // x:(r)+n,y1
		unhandled("x:(r)+n,y1");
		break;
		}
	case 239: { // x:(r)-,y1
		unhandled("x:(r)-,y1");
		break;
		}
	case 240: { // x:(r)+,y1
		unhandled("x:(r)+,y1");
		break;
		}
	case 241: { // x:(r),y1
		unhandled("x:(r),y1");
		break;
		}
	case 242: { // x:(r+n),y1
		unhandled("x:(r+n),y1");
		break;
		}
	case 243: { // x:-(r),y1
		unhandled("x:-(r),y1");
		break;
		}
	case 244: { // x:(r)-n,a0
		unhandled("x:(r)-n,a0");
		break;
		}
	case 245: { // x:(r)+n,a0
		unhandled("x:(r)+n,a0");
		break;
		}
	case 246: { // x:(r)-,a0
		unhandled("x:(r)-,a0");
		break;
		}
	case 247: { // x:(r)+,a0
		unhandled("x:(r)+,a0");
		break;
		}
	case 248: { // x:(r),a0
		unhandled("x:(r),a0");
		break;
		}
	case 249: { // x:(r+n),a0
		unhandled("x:(r+n),a0");
		break;
		}
	case 250: { // x:-(r),a0
		unhandled("x:-(r),a0");
		break;
		}
	case 251: { // x:(r)-n,b0
		unhandled("x:(r)-n,b0");
		break;
		}
	case 252: { // x:(r)+n,b0
		unhandled("x:(r)+n,b0");
		break;
		}
	case 253: { // x:(r)-,b0
		unhandled("x:(r)-,b0");
		break;
		}
	case 254: { // x:(r)+,b0
		unhandled("x:(r)+,b0");
		break;
		}
	case 255: { // x:(r),b0
		unhandled("x:(r),b0");
		break;
		}
	case 256: { // x:(r+n),b0
		unhandled("x:(r+n),b0");
		break;
		}
	case 257: { // x:-(r),b0
		unhandled("x:-(r),b0");
		break;
		}
	case 258: { // x:(r)-n,a2
		unhandled("x:(r)-n,a2");
		break;
		}
	case 259: { // x:(r)+n,a2
		unhandled("x:(r)+n,a2");
		break;
		}
	case 260: { // x:(r)-,a2
		unhandled("x:(r)-,a2");
		break;
		}
	case 261: { // x:(r)+,a2
		unhandled("x:(r)+,a2");
		break;
		}
	case 262: { // x:(r),a2
		unhandled("x:(r),a2");
		break;
		}
	case 263: { // x:(r+n),a2
		unhandled("x:(r+n),a2");
		break;
		}
	case 264: { // x:-(r),a2
		unhandled("x:-(r),a2");
		break;
		}
	case 265: { // x:(r)-n,b2
		unhandled("x:(r)-n,b2");
		break;
		}
	case 266: { // x:(r)+n,b2
		unhandled("x:(r)+n,b2");
		break;
		}
	case 267: { // x:(r)-,b2
		unhandled("x:(r)-,b2");
		break;
		}
	case 268: { // x:(r)+,b2
		unhandled("x:(r)+,b2");
		break;
		}
	case 269: { // x:(r),b2
		unhandled("x:(r),b2");
		break;
		}
	case 270: { // x:(r+n),b2
		unhandled("x:(r+n),b2");
		break;
		}
	case 271: { // x:-(r),b2
		unhandled("x:-(r),b2");
		break;
		}
	case 272: { // x:(r)-n,a1
		unhandled("x:(r)-n,a1");
		break;
		}
	case 273: { // x:(r)+n,a1
		unhandled("x:(r)+n,a1");
		break;
		}
	case 274: { // x:(r)-,a1
		unhandled("x:(r)-,a1");
		break;
		}
	case 275: { // x:(r)+,a1
		unhandled("x:(r)+,a1");
		break;
		}
	case 276: { // x:(r),a1
		unhandled("x:(r),a1");
		break;
		}
	case 277: { // x:(r+n),a1
		unhandled("x:(r+n),a1");
		break;
		}
	case 278: { // x:-(r),a1
		unhandled("x:-(r),a1");
		break;
		}
	case 279: { // x:(r)-n,b1
		unhandled("x:(r)-n,b1");
		break;
		}
	case 280: { // x:(r)+n,b1
		unhandled("x:(r)+n,b1");
		break;
		}
	case 281: { // x:(r)-,b1
		unhandled("x:(r)-,b1");
		break;
		}
	case 282: { // x:(r)+,b1
		unhandled("x:(r)+,b1");
		break;
		}
	case 283: { // x:(r),b1
		unhandled("x:(r),b1");
		break;
		}
	case 284: { // x:(r+n),b1
		unhandled("x:(r+n),b1");
		break;
		}
	case 285: { // x:-(r),b1
		unhandled("x:-(r),b1");
		break;
		}
	case 286: { // x:(r)-n,a
		unhandled("x:(r)-n,a");
		break;
		}
	case 287: { // x:(r)+n,a
		unhandled("x:(r)+n,a");
		break;
		}
	case 288: { // x:(r)-,a
		unhandled("x:(r)-,a");
		break;
		}
	case 289: { // x:(r)+,a
		unhandled("x:(r)+,a");
		break;
		}
	case 290: { // x:(r),a
		unhandled("x:(r),a");
		break;
		}
	case 291: { // x:(r+n),a
		unhandled("x:(r+n),a");
		break;
		}
	case 292: { // x:-(r),a
		unhandled("x:-(r),a");
		break;
		}
	case 293: { // x:(r)-n,b
		unhandled("x:(r)-n,b");
		break;
		}
	case 294: { // x:(r)+n,b
		unhandled("x:(r)+n,b");
		break;
		}
	case 295: { // x:(r)-,b
		unhandled("x:(r)-,b");
		break;
		}
	case 296: { // x:(r)+,b
		unhandled("x:(r)+,b");
		break;
		}
	case 297: { // x:(r),b
		unhandled("x:(r),b");
		break;
		}
	case 298: { // x:(r+n),b
		unhandled("x:(r+n),b");
		break;
		}
	case 299: { // x:-(r),b
		unhandled("x:-(r),b");
		break;
		}
	case 300: { // x:(r)-n,r
		unhandled("x:(r)-n,r");
		break;
		}
	case 301: { // x:(r)+n,r
		unhandled("x:(r)+n,r");
		break;
		}
	case 302: { // x:(r)-,r
		unhandled("x:(r)-,r");
		break;
		}
	case 303: { // x:(r)+,r
		unhandled("x:(r)+,r");
		break;
		}
	case 304: { // x:(r),r
		unhandled("x:(r),r");
		break;
		}
	case 305: { // x:(r+n),r
		unhandled("x:(r+n),r");
		break;
		}
	case 306: { // x:-(r),r
		unhandled("x:-(r),r");
		break;
		}
	case 307: { // x:(r)-n,n
		unhandled("x:(r)-n,n");
		break;
		}
	case 308: { // x:(r)+n,n
		unhandled("x:(r)+n,n");
		break;
		}
	case 309: { // x:(r)-,n
		unhandled("x:(r)-,n");
		break;
		}
	case 310: { // x:(r)+,n
		unhandled("x:(r)+,n");
		break;
		}
	case 311: { // x:(r),n
		unhandled("x:(r),n");
		break;
		}
	case 312: { // x:(r+n),n
		unhandled("x:(r+n),n");
		break;
		}
	case 313: { // x:-(r),n
		unhandled("x:-(r),n");
		break;
		}
	case 314: { // x0,x:(r)-n
		unhandled("x0,x:(r)-n");
		break;
		}
	case 315: { // x0,x:(r)+n
		unhandled("x0,x:(r)+n");
		break;
		}
	case 316: { // x0,x:(r)-
		unhandled("x0,x:(r)-");
		break;
		}
	case 317: { // x0,x:(r)+
		unhandled("x0,x:(r)+");
		break;
		}
	case 318: { // x0,x:(r)
		unhandled("x0,x:(r)");
		break;
		}
	case 319: { // x0,x:(r+n)
		unhandled("x0,x:(r+n)");
		break;
		}
	case 320: { // x0,x:-(r)
		unhandled("x0,x:-(r)");
		break;
		}
	case 321: { // x1,x:(r)-n
		unhandled("x1,x:(r)-n");
		break;
		}
	case 322: { // x1,x:(r)+n
		unhandled("x1,x:(r)+n");
		break;
		}
	case 323: { // x1,x:(r)-
		unhandled("x1,x:(r)-");
		break;
		}
	case 324: { // x1,x:(r)+
		unhandled("x1,x:(r)+");
		break;
		}
	case 325: { // x1,x:(r)
		unhandled("x1,x:(r)");
		break;
		}
	case 326: { // x1,x:(r+n)
		unhandled("x1,x:(r+n)");
		break;
		}
	case 327: { // x1,x:-(r)
		unhandled("x1,x:-(r)");
		break;
		}
	case 328: { // y0,x:(r)-n
		unhandled("y0,x:(r)-n");
		break;
		}
	case 329: { // y0,x:(r)+n
		unhandled("y0,x:(r)+n");
		break;
		}
	case 330: { // y0,x:(r)-
		unhandled("y0,x:(r)-");
		break;
		}
	case 331: { // y0,x:(r)+
		unhandled("y0,x:(r)+");
		break;
		}
	case 332: { // y0,x:(r)
		unhandled("y0,x:(r)");
		break;
		}
	case 333: { // y0,x:(r+n)
		unhandled("y0,x:(r+n)");
		break;
		}
	case 334: { // y0,x:-(r)
		unhandled("y0,x:-(r)");
		break;
		}
	case 335: { // y1,x:(r)-n
		unhandled("y1,x:(r)-n");
		break;
		}
	case 336: { // y1,x:(r)+n
		unhandled("y1,x:(r)+n");
		break;
		}
	case 337: { // y1,x:(r)-
		unhandled("y1,x:(r)-");
		break;
		}
	case 338: { // y1,x:(r)+
		unhandled("y1,x:(r)+");
		break;
		}
	case 339: { // y1,x:(r)
		unhandled("y1,x:(r)");
		break;
		}
	case 340: { // y1,x:(r+n)
		unhandled("y1,x:(r+n)");
		break;
		}
	case 341: { // y1,x:-(r)
		unhandled("y1,x:-(r)");
		break;
		}
	case 342: { // a0,x:(r)-n
		unhandled("a0,x:(r)-n");
		break;
		}
	case 343: { // a0,x:(r)+n
		unhandled("a0,x:(r)+n");
		break;
		}
	case 344: { // a0,x:(r)-
		unhandled("a0,x:(r)-");
		break;
		}
	case 345: { // a0,x:(r)+
		unhandled("a0,x:(r)+");
		break;
		}
	case 346: { // a0,x:(r)
		unhandled("a0,x:(r)");
		break;
		}
	case 347: { // a0,x:(r+n)
		unhandled("a0,x:(r+n)");
		break;
		}
	case 348: { // a0,x:-(r)
		unhandled("a0,x:-(r)");
		break;
		}
	case 349: { // b0,x:(r)-n
		unhandled("b0,x:(r)-n");
		break;
		}
	case 350: { // b0,x:(r)+n
		unhandled("b0,x:(r)+n");
		break;
		}
	case 351: { // b0,x:(r)-
		unhandled("b0,x:(r)-");
		break;
		}
	case 352: { // b0,x:(r)+
		unhandled("b0,x:(r)+");
		break;
		}
	case 353: { // b0,x:(r)
		unhandled("b0,x:(r)");
		break;
		}
	case 354: { // b0,x:(r+n)
		unhandled("b0,x:(r+n)");
		break;
		}
	case 355: { // b0,x:-(r)
		unhandled("b0,x:-(r)");
		break;
		}
	case 356: { // a2,x:(r)-n
		unhandled("a2,x:(r)-n");
		break;
		}
	case 357: { // a2,x:(r)+n
		unhandled("a2,x:(r)+n");
		break;
		}
	case 358: { // a2,x:(r)-
		unhandled("a2,x:(r)-");
		break;
		}
	case 359: { // a2,x:(r)+
		unhandled("a2,x:(r)+");
		break;
		}
	case 360: { // a2,x:(r)
		unhandled("a2,x:(r)");
		break;
		}
	case 361: { // a2,x:(r+n)
		unhandled("a2,x:(r+n)");
		break;
		}
	case 362: { // a2,x:-(r)
		unhandled("a2,x:-(r)");
		break;
		}
	case 363: { // b2,x:(r)-n
		unhandled("b2,x:(r)-n");
		break;
		}
	case 364: { // b2,x:(r)+n
		unhandled("b2,x:(r)+n");
		break;
		}
	case 365: { // b2,x:(r)-
		unhandled("b2,x:(r)-");
		break;
		}
	case 366: { // b2,x:(r)+
		unhandled("b2,x:(r)+");
		break;
		}
	case 367: { // b2,x:(r)
		unhandled("b2,x:(r)");
		break;
		}
	case 368: { // b2,x:(r+n)
		unhandled("b2,x:(r+n)");
		break;
		}
	case 369: { // b2,x:-(r)
		unhandled("b2,x:-(r)");
		break;
		}
	case 370: { // a1,x:(r)-n
		unhandled("a1,x:(r)-n");
		break;
		}
	case 371: { // a1,x:(r)+n
		unhandled("a1,x:(r)+n");
		break;
		}
	case 372: { // a1,x:(r)-
		unhandled("a1,x:(r)-");
		break;
		}
	case 373: { // a1,x:(r)+
		unhandled("a1,x:(r)+");
		break;
		}
	case 374: { // a1,x:(r)
		unhandled("a1,x:(r)");
		break;
		}
	case 375: { // a1,x:(r+n)
		unhandled("a1,x:(r+n)");
		break;
		}
	case 376: { // a1,x:-(r)
		unhandled("a1,x:-(r)");
		break;
		}
	case 377: { // b1,x:(r)-n
		unhandled("b1,x:(r)-n");
		break;
		}
	case 378: { // b1,x:(r)+n
		unhandled("b1,x:(r)+n");
		break;
		}
	case 379: { // b1,x:(r)-
		unhandled("b1,x:(r)-");
		break;
		}
	case 380: { // b1,x:(r)+
		unhandled("b1,x:(r)+");
		break;
		}
	case 381: { // b1,x:(r)
		unhandled("b1,x:(r)");
		break;
		}
	case 382: { // b1,x:(r+n)
		unhandled("b1,x:(r+n)");
		break;
		}
	case 383: { // b1,x:-(r)
		unhandled("b1,x:-(r)");
		break;
		}
	case 384: { // a,x:(r)-n
		unhandled("a,x:(r)-n");
		break;
		}
	case 385: { // a,x:(r)+n
		unhandled("a,x:(r)+n");
		break;
		}
	case 386: { // a,x:(r)-
		unhandled("a,x:(r)-");
		break;
		}
	case 387: { // a,x:(r)+
		unhandled("a,x:(r)+");
		break;
		}
	case 388: { // a,x:(r)
		unhandled("a,x:(r)");
		break;
		}
	case 389: { // a,x:(r+n)
		unhandled("a,x:(r+n)");
		break;
		}
	case 390: { // a,x:-(r)
		unhandled("a,x:-(r)");
		break;
		}
	case 391: { // b,x:(r)-n
		unhandled("b,x:(r)-n");
		break;
		}
	case 392: { // b,x:(r)+n
		unhandled("b,x:(r)+n");
		break;
		}
	case 393: { // b,x:(r)-
		unhandled("b,x:(r)-");
		break;
		}
	case 394: { // b,x:(r)+
		unhandled("b,x:(r)+");
		break;
		}
	case 395: { // b,x:(r)
		unhandled("b,x:(r)");
		break;
		}
	case 396: { // b,x:(r+n)
		unhandled("b,x:(r+n)");
		break;
		}
	case 397: { // b,x:-(r)
		unhandled("b,x:-(r)");
		break;
		}
	case 398: { // r,x:(r)-n
		unhandled("r,x:(r)-n");
		break;
		}
	case 399: { // r,x:(r)+n
		unhandled("r,x:(r)+n");
		break;
		}
	case 400: { // r,x:(r)-
		unhandled("r,x:(r)-");
		break;
		}
	case 401: { // r,x:(r)+
		unhandled("r,x:(r)+");
		break;
		}
	case 402: { // r,x:(r)
		unhandled("r,x:(r)");
		break;
		}
	case 403: { // r,x:(r+n)
		unhandled("r,x:(r+n)");
		break;
		}
	case 404: { // r,x:-(r)
		unhandled("r,x:-(r)");
		break;
		}
	case 405: { // n,x:(r)-n
		unhandled("n,x:(r)-n");
		break;
		}
	case 406: { // n,x:(r)+n
		unhandled("n,x:(r)+n");
		break;
		}
	case 407: { // n,x:(r)-
		unhandled("n,x:(r)-");
		break;
		}
	case 408: { // n,x:(r)+
		unhandled("n,x:(r)+");
		break;
		}
	case 409: { // n,x:(r)
		unhandled("n,x:(r)");
		break;
		}
	case 410: { // n,x:(r+n)
		unhandled("n,x:(r+n)");
		break;
		}
	case 411: { // n,x:-(r)
		unhandled("n,x:-(r)");
		break;
		}
	case 412: { // [abs],x0
		unhandled("[abs],x0");
		break;
		}
	case 413: { // [abs],x1
		unhandled("[abs],x1");
		break;
		}
	case 414: { // [abs],y0
		unhandled("[abs],y0");
		break;
		}
	case 415: { // [abs],y1
		unhandled("[abs],y1");
		break;
		}
	case 416: { // [abs],a0
		unhandled("[abs],a0");
		break;
		}
	case 417: { // [abs],b0
		unhandled("[abs],b0");
		break;
		}
	case 418: { // [abs],a2
		unhandled("[abs],a2");
		break;
		}
	case 419: { // [abs],b2
		unhandled("[abs],b2");
		break;
		}
	case 420: { // [abs],a1
		unhandled("[abs],a1");
		break;
		}
	case 421: { // [abs],b1
		unhandled("[abs],b1");
		break;
		}
	case 422: { // [abs],a
		unhandled("[abs],a");
		break;
		}
	case 423: { // [abs],b
		unhandled("[abs],b");
		break;
		}
	case 424: { // [abs],r
		unhandled("[abs],r");
		break;
		}
	case 425: { // [abs],n
		unhandled("[abs],n");
		break;
		}
	case 426: { // #[i],x0
		unhandled("#[i],x0");
		break;
		}
	case 427: { // #[i],x1
		unhandled("#[i],x1");
		break;
		}
	case 428: { // #[i],y0
		unhandled("#[i],y0");
		break;
		}
	case 429: { // #[i],y1
		unhandled("#[i],y1");
		break;
		}
	case 430: { // #[i],a0
		unhandled("#[i],a0");
		break;
		}
	case 431: { // #[i],b0
		unhandled("#[i],b0");
		break;
		}
	case 432: { // #[i],a2
		unhandled("#[i],a2");
		break;
		}
	case 433: { // #[i],b2
		unhandled("#[i],b2");
		break;
		}
	case 434: { // #[i],a1
		unhandled("#[i],a1");
		break;
		}
	case 435: { // #[i],b1
		unhandled("#[i],b1");
		break;
		}
	case 436: { // #[i],a
		unhandled("#[i],a");
		break;
		}
	case 437: { // #[i],b
		unhandled("#[i],b");
		break;
		}
	case 438: { // #[i],r
		unhandled("#[i],r");
		break;
		}
	case 439: { // #[i],n
		unhandled("#[i],n");
		break;
		}
	case 440: { // x:[aa],x0
		unhandled("x:[aa],x0");
		break;
		}
	case 441: { // x:[aa],x1
		unhandled("x:[aa],x1");
		break;
		}
	case 442: { // x:[aa],y0
		unhandled("x:[aa],y0");
		break;
		}
	case 443: { // x:[aa],y1
		unhandled("x:[aa],y1");
		break;
		}
	case 444: { // x:[aa],a0
		unhandled("x:[aa],a0");
		break;
		}
	case 445: { // x:[aa],b0
		unhandled("x:[aa],b0");
		break;
		}
	case 446: { // x:[aa],a2
		unhandled("x:[aa],a2");
		break;
		}
	case 447: { // x:[aa],b2
		unhandled("x:[aa],b2");
		break;
		}
	case 448: { // x:[aa],a1
		unhandled("x:[aa],a1");
		break;
		}
	case 449: { // x:[aa],b1
		unhandled("x:[aa],b1");
		break;
		}
	case 450: { // x:[aa],a
		unhandled("x:[aa],a");
		break;
		}
	case 451: { // x:[aa],b
		unhandled("x:[aa],b");
		break;
		}
	case 452: { // x:[aa],r
		unhandled("x:[aa],r");
		break;
		}
	case 453: { // x:[aa],n
		unhandled("x:[aa],n");
		break;
		}
	case 454: { // x0,x:[aa]
		unhandled("x0,x:[aa]");
		break;
		}
	case 455: { // x1,x:[aa]
		unhandled("x1,x:[aa]");
		break;
		}
	case 456: { // y0,x:[aa]
		unhandled("y0,x:[aa]");
		break;
		}
	case 457: { // y1,x:[aa]
		unhandled("y1,x:[aa]");
		break;
		}
	case 458: { // a0,x:[aa]
		unhandled("a0,x:[aa]");
		break;
		}
	case 459: { // b0,x:[aa]
		unhandled("b0,x:[aa]");
		break;
		}
	case 460: { // a2,x:[aa]
		unhandled("a2,x:[aa]");
		break;
		}
	case 461: { // b2,x:[aa]
		unhandled("b2,x:[aa]");
		break;
		}
	case 462: { // a1,x:[aa]
		unhandled("a1,x:[aa]");
		break;
		}
	case 463: { // b1,x:[aa]
		unhandled("b1,x:[aa]");
		break;
		}
	case 464: { // a,x:[aa]
		unhandled("a,x:[aa]");
		break;
		}
	case 465: { // b,x:[aa]
		unhandled("b,x:[aa]");
		break;
		}
	case 466: { // r,x:[aa]
		unhandled("r,x:[aa]");
		break;
		}
	case 467: { // n,x:[aa]
		unhandled("n,x:[aa]");
		break;
		}
	case 468: { // x:(r)-n,x0 a,y0
		unhandled("x:(r)-n,x0 a,y0");
		break;
		}
	case 469: { // x:(r)+n,x0 a,y0
		unhandled("x:(r)+n,x0 a,y0");
		break;
		}
	case 470: { // x:(r)-,x0 a,y0
		unhandled("x:(r)-,x0 a,y0");
		break;
		}
	case 471: { // x:(r)+,x0 a,y0
		unhandled("x:(r)+,x0 a,y0");
		break;
		}
	case 472: { // x:(r),x0 a,y0
		unhandled("x:(r),x0 a,y0");
		break;
		}
	case 473: { // x:(r+n),x0 a,y0
		unhandled("x:(r+n),x0 a,y0");
		break;
		}
	case 474: { // x:-(r),x0 a,y0
		unhandled("x:-(r),x0 a,y0");
		break;
		}
	case 475: { // x:(r)-n,x0 a,y1
		unhandled("x:(r)-n,x0 a,y1");
		break;
		}
	case 476: { // x:(r)+n,x0 a,y1
		unhandled("x:(r)+n,x0 a,y1");
		break;
		}
	case 477: { // x:(r)-,x0 a,y1
		unhandled("x:(r)-,x0 a,y1");
		break;
		}
	case 478: { // x:(r)+,x0 a,y1
		unhandled("x:(r)+,x0 a,y1");
		break;
		}
	case 479: { // x:(r),x0 a,y1
		unhandled("x:(r),x0 a,y1");
		break;
		}
	case 480: { // x:(r+n),x0 a,y1
		unhandled("x:(r+n),x0 a,y1");
		break;
		}
	case 481: { // x:-(r),x0 a,y1
		unhandled("x:-(r),x0 a,y1");
		break;
		}
	case 482: { // x:(r)-n,x0 b,y0
		unhandled("x:(r)-n,x0 b,y0");
		break;
		}
	case 483: { // x:(r)+n,x0 b,y0
		unhandled("x:(r)+n,x0 b,y0");
		break;
		}
	case 484: { // x:(r)-,x0 b,y0
		unhandled("x:(r)-,x0 b,y0");
		break;
		}
	case 485: { // x:(r)+,x0 b,y0
		unhandled("x:(r)+,x0 b,y0");
		break;
		}
	case 486: { // x:(r),x0 b,y0
		unhandled("x:(r),x0 b,y0");
		break;
		}
	case 487: { // x:(r+n),x0 b,y0
		unhandled("x:(r+n),x0 b,y0");
		break;
		}
	case 488: { // x:-(r),x0 b,y0
		unhandled("x:-(r),x0 b,y0");
		break;
		}
	case 489: { // x:(r)-n,x0 b,y1
		unhandled("x:(r)-n,x0 b,y1");
		break;
		}
	case 490: { // x:(r)+n,x0 b,y1
		unhandled("x:(r)+n,x0 b,y1");
		break;
		}
	case 491: { // x:(r)-,x0 b,y1
		unhandled("x:(r)-,x0 b,y1");
		break;
		}
	case 492: { // x:(r)+,x0 b,y1
		unhandled("x:(r)+,x0 b,y1");
		break;
		}
	case 493: { // x:(r),x0 b,y1
		unhandled("x:(r),x0 b,y1");
		break;
		}
	case 494: { // x:(r+n),x0 b,y1
		unhandled("x:(r+n),x0 b,y1");
		break;
		}
	case 495: { // x:-(r),x0 b,y1
		unhandled("x:-(r),x0 b,y1");
		break;
		}
	case 496: { // x:(r)-n,x1 a,y0
		unhandled("x:(r)-n,x1 a,y0");
		break;
		}
	case 497: { // x:(r)+n,x1 a,y0
		unhandled("x:(r)+n,x1 a,y0");
		break;
		}
	case 498: { // x:(r)-,x1 a,y0
		unhandled("x:(r)-,x1 a,y0");
		break;
		}
	case 499: { // x:(r)+,x1 a,y0
		unhandled("x:(r)+,x1 a,y0");
		break;
		}
	case 500: { // x:(r),x1 a,y0
		unhandled("x:(r),x1 a,y0");
		break;
		}
	case 501: { // x:(r+n),x1 a,y0
		unhandled("x:(r+n),x1 a,y0");
		break;
		}
	case 502: { // x:-(r),x1 a,y0
		unhandled("x:-(r),x1 a,y0");
		break;
		}
	case 503: { // x:(r)-n,x1 a,y1
		unhandled("x:(r)-n,x1 a,y1");
		break;
		}
	case 504: { // x:(r)+n,x1 a,y1
		unhandled("x:(r)+n,x1 a,y1");
		break;
		}
	case 505: { // x:(r)-,x1 a,y1
		unhandled("x:(r)-,x1 a,y1");
		break;
		}
	case 506: { // x:(r)+,x1 a,y1
		unhandled("x:(r)+,x1 a,y1");
		break;
		}
	case 507: { // x:(r),x1 a,y1
		unhandled("x:(r),x1 a,y1");
		break;
		}
	case 508: { // x:(r+n),x1 a,y1
		unhandled("x:(r+n),x1 a,y1");
		break;
		}
	case 509: { // x:-(r),x1 a,y1
		unhandled("x:-(r),x1 a,y1");
		break;
		}
	case 510: { // x:(r)-n,x1 b,y0
		unhandled("x:(r)-n,x1 b,y0");
		break;
		}
	case 511: { // x:(r)+n,x1 b,y0
		unhandled("x:(r)+n,x1 b,y0");
		break;
		}
	case 512: { // x:(r)-,x1 b,y0
		unhandled("x:(r)-,x1 b,y0");
		break;
		}
	case 513: { // x:(r)+,x1 b,y0
		unhandled("x:(r)+,x1 b,y0");
		break;
		}
	case 514: { // x:(r),x1 b,y0
		unhandled("x:(r),x1 b,y0");
		break;
		}
	case 515: { // x:(r+n),x1 b,y0
		unhandled("x:(r+n),x1 b,y0");
		break;
		}
	case 516: { // x:-(r),x1 b,y0
		unhandled("x:-(r),x1 b,y0");
		break;
		}
	case 517: { // x:(r)-n,x1 b,y1
		unhandled("x:(r)-n,x1 b,y1");
		break;
		}
	case 518: { // x:(r)+n,x1 b,y1
		unhandled("x:(r)+n,x1 b,y1");
		break;
		}
	case 519: { // x:(r)-,x1 b,y1
		unhandled("x:(r)-,x1 b,y1");
		break;
		}
	case 520: { // x:(r)+,x1 b,y1
		unhandled("x:(r)+,x1 b,y1");
		break;
		}
	case 521: { // x:(r),x1 b,y1
		unhandled("x:(r),x1 b,y1");
		break;
		}
	case 522: { // x:(r+n),x1 b,y1
		unhandled("x:(r+n),x1 b,y1");
		break;
		}
	case 523: { // x:-(r),x1 b,y1
		unhandled("x:-(r),x1 b,y1");
		break;
		}
	case 524: { // x:(r)-n,a a,y0
		unhandled("x:(r)-n,a a,y0");
		break;
		}
	case 525: { // x:(r)+n,a a,y0
		unhandled("x:(r)+n,a a,y0");
		break;
		}
	case 526: { // x:(r)-,a a,y0
		unhandled("x:(r)-,a a,y0");
		break;
		}
	case 527: { // x:(r)+,a a,y0
		unhandled("x:(r)+,a a,y0");
		break;
		}
	case 528: { // x:(r),a a,y0
		unhandled("x:(r),a a,y0");
		break;
		}
	case 529: { // x:(r+n),a a,y0
		unhandled("x:(r+n),a a,y0");
		break;
		}
	case 530: { // x:-(r),a a,y0
		unhandled("x:-(r),a a,y0");
		break;
		}
	case 531: { // x:(r)-n,a a,y1
		unhandled("x:(r)-n,a a,y1");
		break;
		}
	case 532: { // x:(r)+n,a a,y1
		unhandled("x:(r)+n,a a,y1");
		break;
		}
	case 533: { // x:(r)-,a a,y1
		unhandled("x:(r)-,a a,y1");
		break;
		}
	case 534: { // x:(r)+,a a,y1
		unhandled("x:(r)+,a a,y1");
		break;
		}
	case 535: { // x:(r),a a,y1
		unhandled("x:(r),a a,y1");
		break;
		}
	case 536: { // x:(r+n),a a,y1
		unhandled("x:(r+n),a a,y1");
		break;
		}
	case 537: { // x:-(r),a a,y1
		unhandled("x:-(r),a a,y1");
		break;
		}
	case 538: { // x:(r)-n,a b,y0
		unhandled("x:(r)-n,a b,y0");
		break;
		}
	case 539: { // x:(r)+n,a b,y0
		unhandled("x:(r)+n,a b,y0");
		break;
		}
	case 540: { // x:(r)-,a b,y0
		unhandled("x:(r)-,a b,y0");
		break;
		}
	case 541: { // x:(r)+,a b,y0
		unhandled("x:(r)+,a b,y0");
		break;
		}
	case 542: { // x:(r),a b,y0
		unhandled("x:(r),a b,y0");
		break;
		}
	case 543: { // x:(r+n),a b,y0
		unhandled("x:(r+n),a b,y0");
		break;
		}
	case 544: { // x:-(r),a b,y0
		unhandled("x:-(r),a b,y0");
		break;
		}
	case 545: { // x:(r)-n,a b,y1
		unhandled("x:(r)-n,a b,y1");
		break;
		}
	case 546: { // x:(r)+n,a b,y1
		unhandled("x:(r)+n,a b,y1");
		break;
		}
	case 547: { // x:(r)-,a b,y1
		unhandled("x:(r)-,a b,y1");
		break;
		}
	case 548: { // x:(r)+,a b,y1
		unhandled("x:(r)+,a b,y1");
		break;
		}
	case 549: { // x:(r),a b,y1
		unhandled("x:(r),a b,y1");
		break;
		}
	case 550: { // x:(r+n),a b,y1
		unhandled("x:(r+n),a b,y1");
		break;
		}
	case 551: { // x:-(r),a b,y1
		unhandled("x:-(r),a b,y1");
		break;
		}
	case 552: { // x:(r)-n,b a,y0
		unhandled("x:(r)-n,b a,y0");
		break;
		}
	case 553: { // x:(r)+n,b a,y0
		unhandled("x:(r)+n,b a,y0");
		break;
		}
	case 554: { // x:(r)-,b a,y0
		unhandled("x:(r)-,b a,y0");
		break;
		}
	case 555: { // x:(r)+,b a,y0
		unhandled("x:(r)+,b a,y0");
		break;
		}
	case 556: { // x:(r),b a,y0
		unhandled("x:(r),b a,y0");
		break;
		}
	case 557: { // x:(r+n),b a,y0
		unhandled("x:(r+n),b a,y0");
		break;
		}
	case 558: { // x:-(r),b a,y0
		unhandled("x:-(r),b a,y0");
		break;
		}
	case 559: { // x:(r)-n,b a,y1
		unhandled("x:(r)-n,b a,y1");
		break;
		}
	case 560: { // x:(r)+n,b a,y1
		unhandled("x:(r)+n,b a,y1");
		break;
		}
	case 561: { // x:(r)-,b a,y1
		unhandled("x:(r)-,b a,y1");
		break;
		}
	case 562: { // x:(r)+,b a,y1
		unhandled("x:(r)+,b a,y1");
		break;
		}
	case 563: { // x:(r),b a,y1
		unhandled("x:(r),b a,y1");
		break;
		}
	case 564: { // x:(r+n),b a,y1
		unhandled("x:(r+n),b a,y1");
		break;
		}
	case 565: { // x:-(r),b a,y1
		unhandled("x:-(r),b a,y1");
		break;
		}
	case 566: { // x:(r)-n,b b,y0
		unhandled("x:(r)-n,b b,y0");
		break;
		}
	case 567: { // x:(r)+n,b b,y0
		unhandled("x:(r)+n,b b,y0");
		break;
		}
	case 568: { // x:(r)-,b b,y0
		unhandled("x:(r)-,b b,y0");
		break;
		}
	case 569: { // x:(r)+,b b,y0
		unhandled("x:(r)+,b b,y0");
		break;
		}
	case 570: { // x:(r),b b,y0
		unhandled("x:(r),b b,y0");
		break;
		}
	case 571: { // x:(r+n),b b,y0
		unhandled("x:(r+n),b b,y0");
		break;
		}
	case 572: { // x:-(r),b b,y0
		unhandled("x:-(r),b b,y0");
		break;
		}
	case 573: { // x:(r)-n,b b,y1
		unhandled("x:(r)-n,b b,y1");
		break;
		}
	case 574: { // x:(r)+n,b b,y1
		unhandled("x:(r)+n,b b,y1");
		break;
		}
	case 575: { // x:(r)-,b b,y1
		unhandled("x:(r)-,b b,y1");
		break;
		}
	case 576: { // x:(r)+,b b,y1
		unhandled("x:(r)+,b b,y1");
		break;
		}
	case 577: { // x:(r),b b,y1
		unhandled("x:(r),b b,y1");
		break;
		}
	case 578: { // x:(r+n),b b,y1
		unhandled("x:(r+n),b b,y1");
		break;
		}
	case 579: { // x:-(r),b b,y1
		unhandled("x:-(r),b b,y1");
		break;
		}
	case 580: { // x0,x:(r)-n a,y0
		unhandled("x0,x:(r)-n a,y0");
		break;
		}
	case 581: { // x0,x:(r)+n a,y0
		unhandled("x0,x:(r)+n a,y0");
		break;
		}
	case 582: { // x0,x:(r)- a,y0
		unhandled("x0,x:(r)- a,y0");
		break;
		}
	case 583: { // x0,x:(r)+ a,y0
		unhandled("x0,x:(r)+ a,y0");
		break;
		}
	case 584: { // x0,x:(r) a,y0
		unhandled("x0,x:(r) a,y0");
		break;
		}
	case 585: { // x0,x:(r+n) a,y0
		unhandled("x0,x:(r+n) a,y0");
		break;
		}
	case 586: { // x0,x:-(r) a,y0
		unhandled("x0,x:-(r) a,y0");
		break;
		}
	case 587: { // x0,x:(r)-n a,y1
		unhandled("x0,x:(r)-n a,y1");
		break;
		}
	case 588: { // x0,x:(r)+n a,y1
		unhandled("x0,x:(r)+n a,y1");
		break;
		}
	case 589: { // x0,x:(r)- a,y1
		unhandled("x0,x:(r)- a,y1");
		break;
		}
	case 590: { // x0,x:(r)+ a,y1
		unhandled("x0,x:(r)+ a,y1");
		break;
		}
	case 591: { // x0,x:(r) a,y1
		unhandled("x0,x:(r) a,y1");
		break;
		}
	case 592: { // x0,x:(r+n) a,y1
		unhandled("x0,x:(r+n) a,y1");
		break;
		}
	case 593: { // x0,x:-(r) a,y1
		unhandled("x0,x:-(r) a,y1");
		break;
		}
	case 594: { // x0,x:(r)-n b,y0
		unhandled("x0,x:(r)-n b,y0");
		break;
		}
	case 595: { // x0,x:(r)+n b,y0
		unhandled("x0,x:(r)+n b,y0");
		break;
		}
	case 596: { // x0,x:(r)- b,y0
		unhandled("x0,x:(r)- b,y0");
		break;
		}
	case 597: { // x0,x:(r)+ b,y0
		unhandled("x0,x:(r)+ b,y0");
		break;
		}
	case 598: { // x0,x:(r) b,y0
		unhandled("x0,x:(r) b,y0");
		break;
		}
	case 599: { // x0,x:(r+n) b,y0
		unhandled("x0,x:(r+n) b,y0");
		break;
		}
	case 600: { // x0,x:-(r) b,y0
		unhandled("x0,x:-(r) b,y0");
		break;
		}
	case 601: { // x0,x:(r)-n b,y1
		unhandled("x0,x:(r)-n b,y1");
		break;
		}
	case 602: { // x0,x:(r)+n b,y1
		unhandled("x0,x:(r)+n b,y1");
		break;
		}
	case 603: { // x0,x:(r)- b,y1
		unhandled("x0,x:(r)- b,y1");
		break;
		}
	case 604: { // x0,x:(r)+ b,y1
		unhandled("x0,x:(r)+ b,y1");
		break;
		}
	case 605: { // x0,x:(r) b,y1
		unhandled("x0,x:(r) b,y1");
		break;
		}
	case 606: { // x0,x:(r+n) b,y1
		unhandled("x0,x:(r+n) b,y1");
		break;
		}
	case 607: { // x0,x:-(r) b,y1
		unhandled("x0,x:-(r) b,y1");
		break;
		}
	case 608: { // x1,x:(r)-n a,y0
		unhandled("x1,x:(r)-n a,y0");
		break;
		}
	case 609: { // x1,x:(r)+n a,y0
		unhandled("x1,x:(r)+n a,y0");
		break;
		}
	case 610: { // x1,x:(r)- a,y0
		unhandled("x1,x:(r)- a,y0");
		break;
		}
	case 611: { // x1,x:(r)+ a,y0
		unhandled("x1,x:(r)+ a,y0");
		break;
		}
	case 612: { // x1,x:(r) a,y0
		unhandled("x1,x:(r) a,y0");
		break;
		}
	case 613: { // x1,x:(r+n) a,y0
		unhandled("x1,x:(r+n) a,y0");
		break;
		}
	case 614: { // x1,x:-(r) a,y0
		unhandled("x1,x:-(r) a,y0");
		break;
		}
	case 615: { // x1,x:(r)-n a,y1
		unhandled("x1,x:(r)-n a,y1");
		break;
		}
	case 616: { // x1,x:(r)+n a,y1
		unhandled("x1,x:(r)+n a,y1");
		break;
		}
	case 617: { // x1,x:(r)- a,y1
		unhandled("x1,x:(r)- a,y1");
		break;
		}
	case 618: { // x1,x:(r)+ a,y1
		unhandled("x1,x:(r)+ a,y1");
		break;
		}
	case 619: { // x1,x:(r) a,y1
		unhandled("x1,x:(r) a,y1");
		break;
		}
	case 620: { // x1,x:(r+n) a,y1
		unhandled("x1,x:(r+n) a,y1");
		break;
		}
	case 621: { // x1,x:-(r) a,y1
		unhandled("x1,x:-(r) a,y1");
		break;
		}
	case 622: { // x1,x:(r)-n b,y0
		unhandled("x1,x:(r)-n b,y0");
		break;
		}
	case 623: { // x1,x:(r)+n b,y0
		unhandled("x1,x:(r)+n b,y0");
		break;
		}
	case 624: { // x1,x:(r)- b,y0
		unhandled("x1,x:(r)- b,y0");
		break;
		}
	case 625: { // x1,x:(r)+ b,y0
		unhandled("x1,x:(r)+ b,y0");
		break;
		}
	case 626: { // x1,x:(r) b,y0
		unhandled("x1,x:(r) b,y0");
		break;
		}
	case 627: { // x1,x:(r+n) b,y0
		unhandled("x1,x:(r+n) b,y0");
		break;
		}
	case 628: { // x1,x:-(r) b,y0
		unhandled("x1,x:-(r) b,y0");
		break;
		}
	case 629: { // x1,x:(r)-n b,y1
		unhandled("x1,x:(r)-n b,y1");
		break;
		}
	case 630: { // x1,x:(r)+n b,y1
		unhandled("x1,x:(r)+n b,y1");
		break;
		}
	case 631: { // x1,x:(r)- b,y1
		unhandled("x1,x:(r)- b,y1");
		break;
		}
	case 632: { // x1,x:(r)+ b,y1
		unhandled("x1,x:(r)+ b,y1");
		break;
		}
	case 633: { // x1,x:(r) b,y1
		unhandled("x1,x:(r) b,y1");
		break;
		}
	case 634: { // x1,x:(r+n) b,y1
		unhandled("x1,x:(r+n) b,y1");
		break;
		}
	case 635: { // x1,x:-(r) b,y1
		unhandled("x1,x:-(r) b,y1");
		break;
		}
	case 636: { // a,x:(r)-n a,y0
		unhandled("a,x:(r)-n a,y0");
		break;
		}
	case 637: { // a,x:(r)+n a,y0
		unhandled("a,x:(r)+n a,y0");
		break;
		}
	case 638: { // a,x:(r)- a,y0
		unhandled("a,x:(r)- a,y0");
		break;
		}
	case 639: { // a,x:(r)+ a,y0
		unhandled("a,x:(r)+ a,y0");
		break;
		}
	case 640: { // a,x:(r) a,y0
		unhandled("a,x:(r) a,y0");
		break;
		}
	case 641: { // a,x:(r+n) a,y0
		unhandled("a,x:(r+n) a,y0");
		break;
		}
	case 642: { // a,x:-(r) a,y0
		unhandled("a,x:-(r) a,y0");
		break;
		}
	case 643: { // a,x:(r)-n a,y1
		unhandled("a,x:(r)-n a,y1");
		break;
		}
	case 644: { // a,x:(r)+n a,y1
		unhandled("a,x:(r)+n a,y1");
		break;
		}
	case 645: { // a,x:(r)- a,y1
		unhandled("a,x:(r)- a,y1");
		break;
		}
	case 646: { // a,x:(r)+ a,y1
		unhandled("a,x:(r)+ a,y1");
		break;
		}
	case 647: { // a,x:(r) a,y1
		unhandled("a,x:(r) a,y1");
		break;
		}
	case 648: { // a,x:(r+n) a,y1
		unhandled("a,x:(r+n) a,y1");
		break;
		}
	case 649: { // a,x:-(r) a,y1
		unhandled("a,x:-(r) a,y1");
		break;
		}
	case 650: { // a,x:(r)-n b,y0
		unhandled("a,x:(r)-n b,y0");
		break;
		}
	case 651: { // a,x:(r)+n b,y0
		unhandled("a,x:(r)+n b,y0");
		break;
		}
	case 652: { // a,x:(r)- b,y0
		unhandled("a,x:(r)- b,y0");
		break;
		}
	case 653: { // a,x:(r)+ b,y0
		unhandled("a,x:(r)+ b,y0");
		break;
		}
	case 654: { // a,x:(r) b,y0
		unhandled("a,x:(r) b,y0");
		break;
		}
	case 655: { // a,x:(r+n) b,y0
		unhandled("a,x:(r+n) b,y0");
		break;
		}
	case 656: { // a,x:-(r) b,y0
		unhandled("a,x:-(r) b,y0");
		break;
		}
	case 657: { // a,x:(r)-n b,y1
		unhandled("a,x:(r)-n b,y1");
		break;
		}
	case 658: { // a,x:(r)+n b,y1
		unhandled("a,x:(r)+n b,y1");
		break;
		}
	case 659: { // a,x:(r)- b,y1
		unhandled("a,x:(r)- b,y1");
		break;
		}
	case 660: { // a,x:(r)+ b,y1
		unhandled("a,x:(r)+ b,y1");
		break;
		}
	case 661: { // a,x:(r) b,y1
		unhandled("a,x:(r) b,y1");
		break;
		}
	case 662: { // a,x:(r+n) b,y1
		unhandled("a,x:(r+n) b,y1");
		break;
		}
	case 663: { // a,x:-(r) b,y1
		unhandled("a,x:-(r) b,y1");
		break;
		}
	case 664: { // b,x:(r)-n a,y0
		unhandled("b,x:(r)-n a,y0");
		break;
		}
	case 665: { // b,x:(r)+n a,y0
		unhandled("b,x:(r)+n a,y0");
		break;
		}
	case 666: { // b,x:(r)- a,y0
		unhandled("b,x:(r)- a,y0");
		break;
		}
	case 667: { // b,x:(r)+ a,y0
		unhandled("b,x:(r)+ a,y0");
		break;
		}
	case 668: { // b,x:(r) a,y0
		unhandled("b,x:(r) a,y0");
		break;
		}
	case 669: { // b,x:(r+n) a,y0
		unhandled("b,x:(r+n) a,y0");
		break;
		}
	case 670: { // b,x:-(r) a,y0
		unhandled("b,x:-(r) a,y0");
		break;
		}
	case 671: { // b,x:(r)-n a,y1
		unhandled("b,x:(r)-n a,y1");
		break;
		}
	case 672: { // b,x:(r)+n a,y1
		unhandled("b,x:(r)+n a,y1");
		break;
		}
	case 673: { // b,x:(r)- a,y1
		unhandled("b,x:(r)- a,y1");
		break;
		}
	case 674: { // b,x:(r)+ a,y1
		unhandled("b,x:(r)+ a,y1");
		break;
		}
	case 675: { // b,x:(r) a,y1
		unhandled("b,x:(r) a,y1");
		break;
		}
	case 676: { // b,x:(r+n) a,y1
		unhandled("b,x:(r+n) a,y1");
		break;
		}
	case 677: { // b,x:-(r) a,y1
		unhandled("b,x:-(r) a,y1");
		break;
		}
	case 678: { // b,x:(r)-n b,y0
		unhandled("b,x:(r)-n b,y0");
		break;
		}
	case 679: { // b,x:(r)+n b,y0
		unhandled("b,x:(r)+n b,y0");
		break;
		}
	case 680: { // b,x:(r)- b,y0
		unhandled("b,x:(r)- b,y0");
		break;
		}
	case 681: { // b,x:(r)+ b,y0
		unhandled("b,x:(r)+ b,y0");
		break;
		}
	case 682: { // b,x:(r) b,y0
		unhandled("b,x:(r) b,y0");
		break;
		}
	case 683: { // b,x:(r+n) b,y0
		unhandled("b,x:(r+n) b,y0");
		break;
		}
	case 684: { // b,x:-(r) b,y0
		unhandled("b,x:-(r) b,y0");
		break;
		}
	case 685: { // b,x:(r)-n b,y1
		unhandled("b,x:(r)-n b,y1");
		break;
		}
	case 686: { // b,x:(r)+n b,y1
		unhandled("b,x:(r)+n b,y1");
		break;
		}
	case 687: { // b,x:(r)- b,y1
		unhandled("b,x:(r)- b,y1");
		break;
		}
	case 688: { // b,x:(r)+ b,y1
		unhandled("b,x:(r)+ b,y1");
		break;
		}
	case 689: { // b,x:(r) b,y1
		unhandled("b,x:(r) b,y1");
		break;
		}
	case 690: { // b,x:(r+n) b,y1
		unhandled("b,x:(r+n) b,y1");
		break;
		}
	case 691: { // b,x:-(r) b,y1
		unhandled("b,x:-(r) b,y1");
		break;
		}
	case 692: { // x:[abs],x0 a,y0
		unhandled("x:[abs],x0 a,y0");
		break;
		}
	case 693: { // x:[abs],x0 a,y1
		unhandled("x:[abs],x0 a,y1");
		break;
		}
	case 694: { // x:[abs],x0 b,y0
		unhandled("x:[abs],x0 b,y0");
		break;
		}
	case 695: { // x:[abs],x0 b,y1
		unhandled("x:[abs],x0 b,y1");
		break;
		}
	case 696: { // x:[abs],x1 a,y0
		unhandled("x:[abs],x1 a,y0");
		break;
		}
	case 697: { // x:[abs],x1 a,y1
		unhandled("x:[abs],x1 a,y1");
		break;
		}
	case 698: { // x:[abs],x1 b,y0
		unhandled("x:[abs],x1 b,y0");
		break;
		}
	case 699: { // x:[abs],x1 b,y1
		unhandled("x:[abs],x1 b,y1");
		break;
		}
	case 700: { // x:[abs],a a,y0
		unhandled("x:[abs],a a,y0");
		break;
		}
	case 701: { // x:[abs],a a,y1
		unhandled("x:[abs],a a,y1");
		break;
		}
	case 702: { // x:[abs],a b,y0
		unhandled("x:[abs],a b,y0");
		break;
		}
	case 703: { // x:[abs],a b,y1
		unhandled("x:[abs],a b,y1");
		break;
		}
	case 704: { // x:[abs],b a,y0
		unhandled("x:[abs],b a,y0");
		break;
		}
	case 705: { // x:[abs],b a,y1
		unhandled("x:[abs],b a,y1");
		break;
		}
	case 706: { // x:[abs],b b,y0
		unhandled("x:[abs],b b,y0");
		break;
		}
	case 707: { // x:[abs],b b,y1
		unhandled("x:[abs],b b,y1");
		break;
		}
	case 708: { // x:#[i],x0 a,y0
		unhandled("x:#[i],x0 a,y0");
		break;
		}
	case 709: { // x:#[i],x0 a,y1
		unhandled("x:#[i],x0 a,y1");
		break;
		}
	case 710: { // x:#[i],x0 b,y0
		unhandled("x:#[i],x0 b,y0");
		break;
		}
	case 711: { // x:#[i],x0 b,y1
		unhandled("x:#[i],x0 b,y1");
		break;
		}
	case 712: { // x:#[i],x1 a,y0
		unhandled("x:#[i],x1 a,y0");
		break;
		}
	case 713: { // x:#[i],x1 a,y1
		unhandled("x:#[i],x1 a,y1");
		break;
		}
	case 714: { // x:#[i],x1 b,y0
		unhandled("x:#[i],x1 b,y0");
		break;
		}
	case 715: { // x:#[i],x1 b,y1
		unhandled("x:#[i],x1 b,y1");
		break;
		}
	case 716: { // x:#[i],a a,y0
		unhandled("x:#[i],a a,y0");
		break;
		}
	case 717: { // x:#[i],a a,y1
		unhandled("x:#[i],a a,y1");
		break;
		}
	case 718: { // x:#[i],a b,y0
		unhandled("x:#[i],a b,y0");
		break;
		}
	case 719: { // x:#[i],a b,y1
		unhandled("x:#[i],a b,y1");
		break;
		}
	case 720: { // x:#[i],b a,y0
		unhandled("x:#[i],b a,y0");
		break;
		}
	case 721: { // x:#[i],b a,y1
		unhandled("x:#[i],b a,y1");
		break;
		}
	case 722: { // x:#[i],b b,y0
		unhandled("x:#[i],b b,y0");
		break;
		}
	case 723: { // x:#[i],b b,y1
		unhandled("x:#[i],b b,y1");
		break;
		}
	case 724: { // a,x:(r)-n x0,a
		unhandled("a,x:(r)-n x0,a");
		break;
		}
	case 725: { // a,x:(r)+n x0,a
		unhandled("a,x:(r)+n x0,a");
		break;
		}
	case 726: { // a,x:(r)- x0,a
		unhandled("a,x:(r)- x0,a");
		break;
		}
	case 727: { // a,x:(r)+ x0,a
		unhandled("a,x:(r)+ x0,a");
		break;
		}
	case 728: { // a,x:(r) x0,a
		unhandled("a,x:(r) x0,a");
		break;
		}
	case 729: { // a,x:(r+n) x0,a
		unhandled("a,x:(r+n) x0,a");
		break;
		}
	case 730: { // a,x:-(r) x0,a
		unhandled("a,x:-(r) x0,a");
		break;
		}
	case 731: { // b,x:(r)-n x0,b
		unhandled("b,x:(r)-n x0,b");
		break;
		}
	case 732: { // b,x:(r)+n x0,b
		unhandled("b,x:(r)+n x0,b");
		break;
		}
	case 733: { // b,x:(r)- x0,b
		unhandled("b,x:(r)- x0,b");
		break;
		}
	case 734: { // b,x:(r)+ x0,b
		unhandled("b,x:(r)+ x0,b");
		break;
		}
	case 735: { // b,x:(r) x0,b
		unhandled("b,x:(r) x0,b");
		break;
		}
	case 736: { // b,x:(r+n) x0,b
		unhandled("b,x:(r+n) x0,b");
		break;
		}
	case 737: { // b,x:-(r) x0,b
		unhandled("b,x:-(r) x0,b");
		break;
		}
	case 738: { // y:(r)-n,x0
		unhandled("y:(r)-n,x0");
		break;
		}
	case 739: { // y:(r)+n,x0
		unhandled("y:(r)+n,x0");
		break;
		}
	case 740: { // y:(r)-,x0
		unhandled("y:(r)-,x0");
		break;
		}
	case 741: { // y:(r)+,x0
		unhandled("y:(r)+,x0");
		break;
		}
	case 742: { // y:(r),x0
		unhandled("y:(r),x0");
		break;
		}
	case 743: { // y:(r+n),x0
		unhandled("y:(r+n),x0");
		break;
		}
	case 744: { // y:-(r),x0
		unhandled("y:-(r),x0");
		break;
		}
	case 745: { // y:(r)-n,x1
		unhandled("y:(r)-n,x1");
		break;
		}
	case 746: { // y:(r)+n,x1
		unhandled("y:(r)+n,x1");
		break;
		}
	case 747: { // y:(r)-,x1
		unhandled("y:(r)-,x1");
		break;
		}
	case 748: { // y:(r)+,x1
		unhandled("y:(r)+,x1");
		break;
		}
	case 749: { // y:(r),x1
		unhandled("y:(r),x1");
		break;
		}
	case 750: { // y:(r+n),x1
		unhandled("y:(r+n),x1");
		break;
		}
	case 751: { // y:-(r),x1
		unhandled("y:-(r),x1");
		break;
		}
	case 752: { // y:(r)-n,y0
		unhandled("y:(r)-n,y0");
		break;
		}
	case 753: { // y:(r)+n,y0
		unhandled("y:(r)+n,y0");
		break;
		}
	case 754: { // y:(r)-,y0
		unhandled("y:(r)-,y0");
		break;
		}
	case 755: { // y:(r)+,y0
		unhandled("y:(r)+,y0");
		break;
		}
	case 756: { // y:(r),y0
		unhandled("y:(r),y0");
		break;
		}
	case 757: { // y:(r+n),y0
		unhandled("y:(r+n),y0");
		break;
		}
	case 758: { // y:-(r),y0
		unhandled("y:-(r),y0");
		break;
		}
	case 759: { // y:(r)-n,y1
		unhandled("y:(r)-n,y1");
		break;
		}
	case 760: { // y:(r)+n,y1
		unhandled("y:(r)+n,y1");
		break;
		}
	case 761: { // y:(r)-,y1
		unhandled("y:(r)-,y1");
		break;
		}
	case 762: { // y:(r)+,y1
		unhandled("y:(r)+,y1");
		break;
		}
	case 763: { // y:(r),y1
		unhandled("y:(r),y1");
		break;
		}
	case 764: { // y:(r+n),y1
		unhandled("y:(r+n),y1");
		break;
		}
	case 765: { // y:-(r),y1
		unhandled("y:-(r),y1");
		break;
		}
	case 766: { // y:(r)-n,a0
		unhandled("y:(r)-n,a0");
		break;
		}
	case 767: { // y:(r)+n,a0
		unhandled("y:(r)+n,a0");
		break;
		}
	case 768: { // y:(r)-,a0
		unhandled("y:(r)-,a0");
		break;
		}
	case 769: { // y:(r)+,a0
		unhandled("y:(r)+,a0");
		break;
		}
	case 770: { // y:(r),a0
		unhandled("y:(r),a0");
		break;
		}
	case 771: { // y:(r+n),a0
		unhandled("y:(r+n),a0");
		break;
		}
	case 772: { // y:-(r),a0
		unhandled("y:-(r),a0");
		break;
		}
	case 773: { // y:(r)-n,b0
		unhandled("y:(r)-n,b0");
		break;
		}
	case 774: { // y:(r)+n,b0
		unhandled("y:(r)+n,b0");
		break;
		}
	case 775: { // y:(r)-,b0
		unhandled("y:(r)-,b0");
		break;
		}
	case 776: { // y:(r)+,b0
		unhandled("y:(r)+,b0");
		break;
		}
	case 777: { // y:(r),b0
		unhandled("y:(r),b0");
		break;
		}
	case 778: { // y:(r+n),b0
		unhandled("y:(r+n),b0");
		break;
		}
	case 779: { // y:-(r),b0
		unhandled("y:-(r),b0");
		break;
		}
	case 780: { // y:(r)-n,a2
		unhandled("y:(r)-n,a2");
		break;
		}
	case 781: { // y:(r)+n,a2
		unhandled("y:(r)+n,a2");
		break;
		}
	case 782: { // y:(r)-,a2
		unhandled("y:(r)-,a2");
		break;
		}
	case 783: { // y:(r)+,a2
		unhandled("y:(r)+,a2");
		break;
		}
	case 784: { // y:(r),a2
		unhandled("y:(r),a2");
		break;
		}
	case 785: { // y:(r+n),a2
		unhandled("y:(r+n),a2");
		break;
		}
	case 786: { // y:-(r),a2
		unhandled("y:-(r),a2");
		break;
		}
	case 787: { // y:(r)-n,b2
		unhandled("y:(r)-n,b2");
		break;
		}
	case 788: { // y:(r)+n,b2
		unhandled("y:(r)+n,b2");
		break;
		}
	case 789: { // y:(r)-,b2
		unhandled("y:(r)-,b2");
		break;
		}
	case 790: { // y:(r)+,b2
		unhandled("y:(r)+,b2");
		break;
		}
	case 791: { // y:(r),b2
		unhandled("y:(r),b2");
		break;
		}
	case 792: { // y:(r+n),b2
		unhandled("y:(r+n),b2");
		break;
		}
	case 793: { // y:-(r),b2
		unhandled("y:-(r),b2");
		break;
		}
	case 794: { // y:(r)-n,a1
		unhandled("y:(r)-n,a1");
		break;
		}
	case 795: { // y:(r)+n,a1
		unhandled("y:(r)+n,a1");
		break;
		}
	case 796: { // y:(r)-,a1
		unhandled("y:(r)-,a1");
		break;
		}
	case 797: { // y:(r)+,a1
		unhandled("y:(r)+,a1");
		break;
		}
	case 798: { // y:(r),a1
		unhandled("y:(r),a1");
		break;
		}
	case 799: { // y:(r+n),a1
		unhandled("y:(r+n),a1");
		break;
		}
	case 800: { // y:-(r),a1
		unhandled("y:-(r),a1");
		break;
		}
	case 801: { // y:(r)-n,b1
		unhandled("y:(r)-n,b1");
		break;
		}
	case 802: { // y:(r)+n,b1
		unhandled("y:(r)+n,b1");
		break;
		}
	case 803: { // y:(r)-,b1
		unhandled("y:(r)-,b1");
		break;
		}
	case 804: { // y:(r)+,b1
		unhandled("y:(r)+,b1");
		break;
		}
	case 805: { // y:(r),b1
		unhandled("y:(r),b1");
		break;
		}
	case 806: { // y:(r+n),b1
		unhandled("y:(r+n),b1");
		break;
		}
	case 807: { // y:-(r),b1
		unhandled("y:-(r),b1");
		break;
		}
	case 808: { // y:(r)-n,a
		unhandled("y:(r)-n,a");
		break;
		}
	case 809: { // y:(r)+n,a
		unhandled("y:(r)+n,a");
		break;
		}
	case 810: { // y:(r)-,a
		unhandled("y:(r)-,a");
		break;
		}
	case 811: { // y:(r)+,a
		unhandled("y:(r)+,a");
		break;
		}
	case 812: { // y:(r),a
		unhandled("y:(r),a");
		break;
		}
	case 813: { // y:(r+n),a
		unhandled("y:(r+n),a");
		break;
		}
	case 814: { // y:-(r),a
		unhandled("y:-(r),a");
		break;
		}
	case 815: { // y:(r)-n,b
		unhandled("y:(r)-n,b");
		break;
		}
	case 816: { // y:(r)+n,b
		unhandled("y:(r)+n,b");
		break;
		}
	case 817: { // y:(r)-,b
		unhandled("y:(r)-,b");
		break;
		}
	case 818: { // y:(r)+,b
		unhandled("y:(r)+,b");
		break;
		}
	case 819: { // y:(r),b
		unhandled("y:(r),b");
		break;
		}
	case 820: { // y:(r+n),b
		unhandled("y:(r+n),b");
		break;
		}
	case 821: { // y:-(r),b
		unhandled("y:-(r),b");
		break;
		}
	case 822: { // y:(r)-n,r
		unhandled("y:(r)-n,r");
		break;
		}
	case 823: { // y:(r)+n,r
		unhandled("y:(r)+n,r");
		break;
		}
	case 824: { // y:(r)-,r
		unhandled("y:(r)-,r");
		break;
		}
	case 825: { // y:(r)+,r
		unhandled("y:(r)+,r");
		break;
		}
	case 826: { // y:(r),r
		unhandled("y:(r),r");
		break;
		}
	case 827: { // y:(r+n),r
		unhandled("y:(r+n),r");
		break;
		}
	case 828: { // y:-(r),r
		unhandled("y:-(r),r");
		break;
		}
	case 829: { // y:(r)-n,n
		unhandled("y:(r)-n,n");
		break;
		}
	case 830: { // y:(r)+n,n
		unhandled("y:(r)+n,n");
		break;
		}
	case 831: { // y:(r)-,n
		unhandled("y:(r)-,n");
		break;
		}
	case 832: { // y:(r)+,n
		unhandled("y:(r)+,n");
		break;
		}
	case 833: { // y:(r),n
		unhandled("y:(r),n");
		break;
		}
	case 834: { // y:(r+n),n
		unhandled("y:(r+n),n");
		break;
		}
	case 835: { // y:-(r),n
		unhandled("y:-(r),n");
		break;
		}
	case 836: { // x0,y:(r)-n
		unhandled("x0,y:(r)-n");
		break;
		}
	case 837: { // x0,y:(r)+n
		unhandled("x0,y:(r)+n");
		break;
		}
	case 838: { // x0,y:(r)-
		unhandled("x0,y:(r)-");
		break;
		}
	case 839: { // x0,y:(r)+
		unhandled("x0,y:(r)+");
		break;
		}
	case 840: { // x0,y:(r)
		unhandled("x0,y:(r)");
		break;
		}
	case 841: { // x0,y:(r+n)
		unhandled("x0,y:(r+n)");
		break;
		}
	case 842: { // x0,y:-(r)
		unhandled("x0,y:-(r)");
		break;
		}
	case 843: { // x1,y:(r)-n
		unhandled("x1,y:(r)-n");
		break;
		}
	case 844: { // x1,y:(r)+n
		unhandled("x1,y:(r)+n");
		break;
		}
	case 845: { // x1,y:(r)-
		unhandled("x1,y:(r)-");
		break;
		}
	case 846: { // x1,y:(r)+
		unhandled("x1,y:(r)+");
		break;
		}
	case 847: { // x1,y:(r)
		unhandled("x1,y:(r)");
		break;
		}
	case 848: { // x1,y:(r+n)
		unhandled("x1,y:(r+n)");
		break;
		}
	case 849: { // x1,y:-(r)
		unhandled("x1,y:-(r)");
		break;
		}
	case 850: { // y0,y:(r)-n
		unhandled("y0,y:(r)-n");
		break;
		}
	case 851: { // y0,y:(r)+n
		unhandled("y0,y:(r)+n");
		break;
		}
	case 852: { // y0,y:(r)-
		unhandled("y0,y:(r)-");
		break;
		}
	case 853: { // y0,y:(r)+
		unhandled("y0,y:(r)+");
		break;
		}
	case 854: { // y0,y:(r)
		unhandled("y0,y:(r)");
		break;
		}
	case 855: { // y0,y:(r+n)
		unhandled("y0,y:(r+n)");
		break;
		}
	case 856: { // y0,y:-(r)
		unhandled("y0,y:-(r)");
		break;
		}
	case 857: { // y1,y:(r)-n
		unhandled("y1,y:(r)-n");
		break;
		}
	case 858: { // y1,y:(r)+n
		unhandled("y1,y:(r)+n");
		break;
		}
	case 859: { // y1,y:(r)-
		unhandled("y1,y:(r)-");
		break;
		}
	case 860: { // y1,y:(r)+
		unhandled("y1,y:(r)+");
		break;
		}
	case 861: { // y1,y:(r)
		unhandled("y1,y:(r)");
		break;
		}
	case 862: { // y1,y:(r+n)
		unhandled("y1,y:(r+n)");
		break;
		}
	case 863: { // y1,y:-(r)
		unhandled("y1,y:-(r)");
		break;
		}
	case 864: { // a0,y:(r)-n
		unhandled("a0,y:(r)-n");
		break;
		}
	case 865: { // a0,y:(r)+n
		unhandled("a0,y:(r)+n");
		break;
		}
	case 866: { // a0,y:(r)-
		unhandled("a0,y:(r)-");
		break;
		}
	case 867: { // a0,y:(r)+
		unhandled("a0,y:(r)+");
		break;
		}
	case 868: { // a0,y:(r)
		unhandled("a0,y:(r)");
		break;
		}
	case 869: { // a0,y:(r+n)
		unhandled("a0,y:(r+n)");
		break;
		}
	case 870: { // a0,y:-(r)
		unhandled("a0,y:-(r)");
		break;
		}
	case 871: { // b0,y:(r)-n
		unhandled("b0,y:(r)-n");
		break;
		}
	case 872: { // b0,y:(r)+n
		unhandled("b0,y:(r)+n");
		break;
		}
	case 873: { // b0,y:(r)-
		unhandled("b0,y:(r)-");
		break;
		}
	case 874: { // b0,y:(r)+
		unhandled("b0,y:(r)+");
		break;
		}
	case 875: { // b0,y:(r)
		unhandled("b0,y:(r)");
		break;
		}
	case 876: { // b0,y:(r+n)
		unhandled("b0,y:(r+n)");
		break;
		}
	case 877: { // b0,y:-(r)
		unhandled("b0,y:-(r)");
		break;
		}
	case 878: { // a2,y:(r)-n
		unhandled("a2,y:(r)-n");
		break;
		}
	case 879: { // a2,y:(r)+n
		unhandled("a2,y:(r)+n");
		break;
		}
	case 880: { // a2,y:(r)-
		unhandled("a2,y:(r)-");
		break;
		}
	case 881: { // a2,y:(r)+
		unhandled("a2,y:(r)+");
		break;
		}
	case 882: { // a2,y:(r)
		unhandled("a2,y:(r)");
		break;
		}
	case 883: { // a2,y:(r+n)
		unhandled("a2,y:(r+n)");
		break;
		}
	case 884: { // a2,y:-(r)
		unhandled("a2,y:-(r)");
		break;
		}
	case 885: { // b2,y:(r)-n
		unhandled("b2,y:(r)-n");
		break;
		}
	case 886: { // b2,y:(r)+n
		unhandled("b2,y:(r)+n");
		break;
		}
	case 887: { // b2,y:(r)-
		unhandled("b2,y:(r)-");
		break;
		}
	case 888: { // b2,y:(r)+
		unhandled("b2,y:(r)+");
		break;
		}
	case 889: { // b2,y:(r)
		unhandled("b2,y:(r)");
		break;
		}
	case 890: { // b2,y:(r+n)
		unhandled("b2,y:(r+n)");
		break;
		}
	case 891: { // b2,y:-(r)
		unhandled("b2,y:-(r)");
		break;
		}
	case 892: { // a1,y:(r)-n
		unhandled("a1,y:(r)-n");
		break;
		}
	case 893: { // a1,y:(r)+n
		unhandled("a1,y:(r)+n");
		break;
		}
	case 894: { // a1,y:(r)-
		unhandled("a1,y:(r)-");
		break;
		}
	case 895: { // a1,y:(r)+
		unhandled("a1,y:(r)+");
		break;
		}
	case 896: { // a1,y:(r)
		unhandled("a1,y:(r)");
		break;
		}
	case 897: { // a1,y:(r+n)
		unhandled("a1,y:(r+n)");
		break;
		}
	case 898: { // a1,y:-(r)
		unhandled("a1,y:-(r)");
		break;
		}
	case 899: { // b1,y:(r)-n
		unhandled("b1,y:(r)-n");
		break;
		}
	case 900: { // b1,y:(r)+n
		unhandled("b1,y:(r)+n");
		break;
		}
	case 901: { // b1,y:(r)-
		unhandled("b1,y:(r)-");
		break;
		}
	case 902: { // b1,y:(r)+
		unhandled("b1,y:(r)+");
		break;
		}
	case 903: { // b1,y:(r)
		unhandled("b1,y:(r)");
		break;
		}
	case 904: { // b1,y:(r+n)
		unhandled("b1,y:(r+n)");
		break;
		}
	case 905: { // b1,y:-(r)
		unhandled("b1,y:-(r)");
		break;
		}
	case 906: { // a,y:(r)-n
		unhandled("a,y:(r)-n");
		break;
		}
	case 907: { // a,y:(r)+n
		unhandled("a,y:(r)+n");
		break;
		}
	case 908: { // a,y:(r)-
		unhandled("a,y:(r)-");
		break;
		}
	case 909: { // a,y:(r)+
		unhandled("a,y:(r)+");
		break;
		}
	case 910: { // a,y:(r)
		unhandled("a,y:(r)");
		break;
		}
	case 911: { // a,y:(r+n)
		unhandled("a,y:(r+n)");
		break;
		}
	case 912: { // a,y:-(r)
		unhandled("a,y:-(r)");
		break;
		}
	case 913: { // b,y:(r)-n
		unhandled("b,y:(r)-n");
		break;
		}
	case 914: { // b,y:(r)+n
		unhandled("b,y:(r)+n");
		break;
		}
	case 915: { // b,y:(r)-
		unhandled("b,y:(r)-");
		break;
		}
	case 916: { // b,y:(r)+
		unhandled("b,y:(r)+");
		break;
		}
	case 917: { // b,y:(r)
		unhandled("b,y:(r)");
		break;
		}
	case 918: { // b,y:(r+n)
		unhandled("b,y:(r+n)");
		break;
		}
	case 919: { // b,y:-(r)
		unhandled("b,y:-(r)");
		break;
		}
	case 920: { // r,y:(r)-n
		unhandled("r,y:(r)-n");
		break;
		}
	case 921: { // r,y:(r)+n
		unhandled("r,y:(r)+n");
		break;
		}
	case 922: { // r,y:(r)-
		unhandled("r,y:(r)-");
		break;
		}
	case 923: { // r,y:(r)+
		unhandled("r,y:(r)+");
		break;
		}
	case 924: { // r,y:(r)
		unhandled("r,y:(r)");
		break;
		}
	case 925: { // r,y:(r+n)
		unhandled("r,y:(r+n)");
		break;
		}
	case 926: { // r,y:-(r)
		unhandled("r,y:-(r)");
		break;
		}
	case 927: { // n,y:(r)-n
		unhandled("n,y:(r)-n");
		break;
		}
	case 928: { // n,y:(r)+n
		unhandled("n,y:(r)+n");
		break;
		}
	case 929: { // n,y:(r)-
		unhandled("n,y:(r)-");
		break;
		}
	case 930: { // n,y:(r)+
		unhandled("n,y:(r)+");
		break;
		}
	case 931: { // n,y:(r)
		unhandled("n,y:(r)");
		break;
		}
	case 932: { // n,y:(r+n)
		unhandled("n,y:(r+n)");
		break;
		}
	case 933: { // n,y:-(r)
		unhandled("n,y:-(r)");
		break;
		}
	case 934: { // [abs],x0
		unhandled("[abs],x0");
		break;
		}
	case 935: { // [abs],x1
		unhandled("[abs],x1");
		break;
		}
	case 936: { // [abs],y0
		unhandled("[abs],y0");
		break;
		}
	case 937: { // [abs],y1
		unhandled("[abs],y1");
		break;
		}
	case 938: { // [abs],a0
		unhandled("[abs],a0");
		break;
		}
	case 939: { // [abs],b0
		unhandled("[abs],b0");
		break;
		}
	case 940: { // [abs],a2
		unhandled("[abs],a2");
		break;
		}
	case 941: { // [abs],b2
		unhandled("[abs],b2");
		break;
		}
	case 942: { // [abs],a1
		unhandled("[abs],a1");
		break;
		}
	case 943: { // [abs],b1
		unhandled("[abs],b1");
		break;
		}
	case 944: { // [abs],a
		unhandled("[abs],a");
		break;
		}
	case 945: { // [abs],b
		unhandled("[abs],b");
		break;
		}
	case 946: { // [abs],r
		unhandled("[abs],r");
		break;
		}
	case 947: { // [abs],n
		unhandled("[abs],n");
		break;
		}
	case 948: { // #[i],x0
		unhandled("#[i],x0");
		break;
		}
	case 949: { // #[i],x1
		unhandled("#[i],x1");
		break;
		}
	case 950: { // #[i],y0
		unhandled("#[i],y0");
		break;
		}
	case 951: { // #[i],y1
		unhandled("#[i],y1");
		break;
		}
	case 952: { // #[i],a0
		unhandled("#[i],a0");
		break;
		}
	case 953: { // #[i],b0
		unhandled("#[i],b0");
		break;
		}
	case 954: { // #[i],a2
		unhandled("#[i],a2");
		break;
		}
	case 955: { // #[i],b2
		unhandled("#[i],b2");
		break;
		}
	case 956: { // #[i],a1
		unhandled("#[i],a1");
		break;
		}
	case 957: { // #[i],b1
		unhandled("#[i],b1");
		break;
		}
	case 958: { // #[i],a
		unhandled("#[i],a");
		break;
		}
	case 959: { // #[i],b
		unhandled("#[i],b");
		break;
		}
	case 960: { // #[i],r
		unhandled("#[i],r");
		break;
		}
	case 961: { // #[i],n
		unhandled("#[i],n");
		break;
		}
	case 962: { // y:[aa],x0
		unhandled("y:[aa],x0");
		break;
		}
	case 963: { // y:[aa],x1
		unhandled("y:[aa],x1");
		break;
		}
	case 964: { // y:[aa],y0
		unhandled("y:[aa],y0");
		break;
		}
	case 965: { // y:[aa],y1
		unhandled("y:[aa],y1");
		break;
		}
	case 966: { // y:[aa],a0
		unhandled("y:[aa],a0");
		break;
		}
	case 967: { // y:[aa],b0
		unhandled("y:[aa],b0");
		break;
		}
	case 968: { // y:[aa],a2
		unhandled("y:[aa],a2");
		break;
		}
	case 969: { // y:[aa],b2
		unhandled("y:[aa],b2");
		break;
		}
	case 970: { // y:[aa],a1
		unhandled("y:[aa],a1");
		break;
		}
	case 971: { // y:[aa],b1
		unhandled("y:[aa],b1");
		break;
		}
	case 972: { // y:[aa],a
		unhandled("y:[aa],a");
		break;
		}
	case 973: { // y:[aa],b
		unhandled("y:[aa],b");
		break;
		}
	case 974: { // y:[aa],r
		unhandled("y:[aa],r");
		break;
		}
	case 975: { // y:[aa],n
		unhandled("y:[aa],n");
		break;
		}
	case 976: { // x0,y:[aa]
		unhandled("x0,y:[aa]");
		break;
		}
	case 977: { // x1,y:[aa]
		unhandled("x1,y:[aa]");
		break;
		}
	case 978: { // y0,y:[aa]
		unhandled("y0,y:[aa]");
		break;
		}
	case 979: { // y1,y:[aa]
		unhandled("y1,y:[aa]");
		break;
		}
	case 980: { // a0,y:[aa]
		unhandled("a0,y:[aa]");
		break;
		}
	case 981: { // b0,y:[aa]
		unhandled("b0,y:[aa]");
		break;
		}
	case 982: { // a2,y:[aa]
		unhandled("a2,y:[aa]");
		break;
		}
	case 983: { // b2,y:[aa]
		unhandled("b2,y:[aa]");
		break;
		}
	case 984: { // a1,y:[aa]
		unhandled("a1,y:[aa]");
		break;
		}
	case 985: { // b1,y:[aa]
		unhandled("b1,y:[aa]");
		break;
		}
	case 986: { // a,y:[aa]
		unhandled("a,y:[aa]");
		break;
		}
	case 987: { // b,y:[aa]
		unhandled("b,y:[aa]");
		break;
		}
	case 988: { // r,y:[aa]
		unhandled("r,y:[aa]");
		break;
		}
	case 989: { // n,y:[aa]
		unhandled("n,y:[aa]");
		break;
		}
	case 990: { // a,x0 y:(r)-n,y0
		unhandled("a,x0 y:(r)-n,y0");
		break;
		}
	case 991: { // a,x0 y:(r)+n,y0
		unhandled("a,x0 y:(r)+n,y0");
		break;
		}
	case 992: { // a,x0 y:(r)-,y0
		unhandled("a,x0 y:(r)-,y0");
		break;
		}
	case 993: { // a,x0 y:(r)+,y0
		unhandled("a,x0 y:(r)+,y0");
		break;
		}
	case 994: { // a,x0 y:(r),y0
		unhandled("a,x0 y:(r),y0");
		break;
		}
	case 995: { // a,x0 y:(r+n),y0
		unhandled("a,x0 y:(r+n),y0");
		break;
		}
	case 996: { // a,x0 y:-(r),y0
		unhandled("a,x0 y:-(r),y0");
		break;
		}
	case 997: { // a,x1 y:(r)-n,y0
		unhandled("a,x1 y:(r)-n,y0");
		break;
		}
	case 998: { // a,x1 y:(r)+n,y0
		unhandled("a,x1 y:(r)+n,y0");
		break;
		}
	case 999: { // a,x1 y:(r)-,y0
		unhandled("a,x1 y:(r)-,y0");
		break;
		}
	case 1000: { // a,x1 y:(r)+,y0
		unhandled("a,x1 y:(r)+,y0");
		break;
		}
	case 1001: { // a,x1 y:(r),y0
		unhandled("a,x1 y:(r),y0");
		break;
		}
	case 1002: { // a,x1 y:(r+n),y0
		unhandled("a,x1 y:(r+n),y0");
		break;
		}
	case 1003: { // a,x1 y:-(r),y0
		unhandled("a,x1 y:-(r),y0");
		break;
		}
	case 1004: { // b,x0 y:(r)-n,y0
		unhandled("b,x0 y:(r)-n,y0");
		break;
		}
	case 1005: { // b,x0 y:(r)+n,y0
		unhandled("b,x0 y:(r)+n,y0");
		break;
		}
	case 1006: { // b,x0 y:(r)-,y0
		unhandled("b,x0 y:(r)-,y0");
		break;
		}
	case 1007: { // b,x0 y:(r)+,y0
		unhandled("b,x0 y:(r)+,y0");
		break;
		}
	case 1008: { // b,x0 y:(r),y0
		unhandled("b,x0 y:(r),y0");
		break;
		}
	case 1009: { // b,x0 y:(r+n),y0
		unhandled("b,x0 y:(r+n),y0");
		break;
		}
	case 1010: { // b,x0 y:-(r),y0
		unhandled("b,x0 y:-(r),y0");
		break;
		}
	case 1011: { // b,x1 y:(r)-n,y0
		unhandled("b,x1 y:(r)-n,y0");
		break;
		}
	case 1012: { // b,x1 y:(r)+n,y0
		unhandled("b,x1 y:(r)+n,y0");
		break;
		}
	case 1013: { // b,x1 y:(r)-,y0
		unhandled("b,x1 y:(r)-,y0");
		break;
		}
	case 1014: { // b,x1 y:(r)+,y0
		unhandled("b,x1 y:(r)+,y0");
		break;
		}
	case 1015: { // b,x1 y:(r),y0
		unhandled("b,x1 y:(r),y0");
		break;
		}
	case 1016: { // b,x1 y:(r+n),y0
		unhandled("b,x1 y:(r+n),y0");
		break;
		}
	case 1017: { // b,x1 y:-(r),y0
		unhandled("b,x1 y:-(r),y0");
		break;
		}
	case 1018: { // a,x0 y:(r)-n,y1
		unhandled("a,x0 y:(r)-n,y1");
		break;
		}
	case 1019: { // a,x0 y:(r)+n,y1
		unhandled("a,x0 y:(r)+n,y1");
		break;
		}
	case 1020: { // a,x0 y:(r)-,y1
		unhandled("a,x0 y:(r)-,y1");
		break;
		}
	case 1021: { // a,x0 y:(r)+,y1
		unhandled("a,x0 y:(r)+,y1");
		break;
		}
	case 1022: { // a,x0 y:(r),y1
		unhandled("a,x0 y:(r),y1");
		break;
		}
	case 1023: { // a,x0 y:(r+n),y1
		unhandled("a,x0 y:(r+n),y1");
		break;
		}
	case 1024: { // a,x0 y:-(r),y1
		unhandled("a,x0 y:-(r),y1");
		break;
		}
	case 1025: { // a,x1 y:(r)-n,y1
		unhandled("a,x1 y:(r)-n,y1");
		break;
		}
	case 1026: { // a,x1 y:(r)+n,y1
		unhandled("a,x1 y:(r)+n,y1");
		break;
		}
	case 1027: { // a,x1 y:(r)-,y1
		unhandled("a,x1 y:(r)-,y1");
		break;
		}
	case 1028: { // a,x1 y:(r)+,y1
		unhandled("a,x1 y:(r)+,y1");
		break;
		}
	case 1029: { // a,x1 y:(r),y1
		unhandled("a,x1 y:(r),y1");
		break;
		}
	case 1030: { // a,x1 y:(r+n),y1
		unhandled("a,x1 y:(r+n),y1");
		break;
		}
	case 1031: { // a,x1 y:-(r),y1
		unhandled("a,x1 y:-(r),y1");
		break;
		}
	case 1032: { // b,x0 y:(r)-n,y1
		unhandled("b,x0 y:(r)-n,y1");
		break;
		}
	case 1033: { // b,x0 y:(r)+n,y1
		unhandled("b,x0 y:(r)+n,y1");
		break;
		}
	case 1034: { // b,x0 y:(r)-,y1
		unhandled("b,x0 y:(r)-,y1");
		break;
		}
	case 1035: { // b,x0 y:(r)+,y1
		unhandled("b,x0 y:(r)+,y1");
		break;
		}
	case 1036: { // b,x0 y:(r),y1
		unhandled("b,x0 y:(r),y1");
		break;
		}
	case 1037: { // b,x0 y:(r+n),y1
		unhandled("b,x0 y:(r+n),y1");
		break;
		}
	case 1038: { // b,x0 y:-(r),y1
		unhandled("b,x0 y:-(r),y1");
		break;
		}
	case 1039: { // b,x1 y:(r)-n,y1
		unhandled("b,x1 y:(r)-n,y1");
		break;
		}
	case 1040: { // b,x1 y:(r)+n,y1
		unhandled("b,x1 y:(r)+n,y1");
		break;
		}
	case 1041: { // b,x1 y:(r)-,y1
		unhandled("b,x1 y:(r)-,y1");
		break;
		}
	case 1042: { // b,x1 y:(r)+,y1
		unhandled("b,x1 y:(r)+,y1");
		break;
		}
	case 1043: { // b,x1 y:(r),y1
		unhandled("b,x1 y:(r),y1");
		break;
		}
	case 1044: { // b,x1 y:(r+n),y1
		unhandled("b,x1 y:(r+n),y1");
		break;
		}
	case 1045: { // b,x1 y:-(r),y1
		unhandled("b,x1 y:-(r),y1");
		break;
		}
	case 1046: { // a,x0 y:(r)-n,a
		unhandled("a,x0 y:(r)-n,a");
		break;
		}
	case 1047: { // a,x0 y:(r)+n,a
		unhandled("a,x0 y:(r)+n,a");
		break;
		}
	case 1048: { // a,x0 y:(r)-,a
		unhandled("a,x0 y:(r)-,a");
		break;
		}
	case 1049: { // a,x0 y:(r)+,a
		unhandled("a,x0 y:(r)+,a");
		break;
		}
	case 1050: { // a,x0 y:(r),a
		unhandled("a,x0 y:(r),a");
		break;
		}
	case 1051: { // a,x0 y:(r+n),a
		unhandled("a,x0 y:(r+n),a");
		break;
		}
	case 1052: { // a,x0 y:-(r),a
		unhandled("a,x0 y:-(r),a");
		break;
		}
	case 1053: { // a,x1 y:(r)-n,a
		unhandled("a,x1 y:(r)-n,a");
		break;
		}
	case 1054: { // a,x1 y:(r)+n,a
		unhandled("a,x1 y:(r)+n,a");
		break;
		}
	case 1055: { // a,x1 y:(r)-,a
		unhandled("a,x1 y:(r)-,a");
		break;
		}
	case 1056: { // a,x1 y:(r)+,a
		unhandled("a,x1 y:(r)+,a");
		break;
		}
	case 1057: { // a,x1 y:(r),a
		unhandled("a,x1 y:(r),a");
		break;
		}
	case 1058: { // a,x1 y:(r+n),a
		unhandled("a,x1 y:(r+n),a");
		break;
		}
	case 1059: { // a,x1 y:-(r),a
		unhandled("a,x1 y:-(r),a");
		break;
		}
	case 1060: { // b,x0 y:(r)-n,a
		unhandled("b,x0 y:(r)-n,a");
		break;
		}
	case 1061: { // b,x0 y:(r)+n,a
		unhandled("b,x0 y:(r)+n,a");
		break;
		}
	case 1062: { // b,x0 y:(r)-,a
		unhandled("b,x0 y:(r)-,a");
		break;
		}
	case 1063: { // b,x0 y:(r)+,a
		unhandled("b,x0 y:(r)+,a");
		break;
		}
	case 1064: { // b,x0 y:(r),a
		unhandled("b,x0 y:(r),a");
		break;
		}
	case 1065: { // b,x0 y:(r+n),a
		unhandled("b,x0 y:(r+n),a");
		break;
		}
	case 1066: { // b,x0 y:-(r),a
		unhandled("b,x0 y:-(r),a");
		break;
		}
	case 1067: { // b,x1 y:(r)-n,a
		unhandled("b,x1 y:(r)-n,a");
		break;
		}
	case 1068: { // b,x1 y:(r)+n,a
		unhandled("b,x1 y:(r)+n,a");
		break;
		}
	case 1069: { // b,x1 y:(r)-,a
		unhandled("b,x1 y:(r)-,a");
		break;
		}
	case 1070: { // b,x1 y:(r)+,a
		unhandled("b,x1 y:(r)+,a");
		break;
		}
	case 1071: { // b,x1 y:(r),a
		unhandled("b,x1 y:(r),a");
		break;
		}
	case 1072: { // b,x1 y:(r+n),a
		unhandled("b,x1 y:(r+n),a");
		break;
		}
	case 1073: { // b,x1 y:-(r),a
		unhandled("b,x1 y:-(r),a");
		break;
		}
	case 1074: { // a,x0 y:(r)-n,b
		unhandled("a,x0 y:(r)-n,b");
		break;
		}
	case 1075: { // a,x0 y:(r)+n,b
		unhandled("a,x0 y:(r)+n,b");
		break;
		}
	case 1076: { // a,x0 y:(r)-,b
		unhandled("a,x0 y:(r)-,b");
		break;
		}
	case 1077: { // a,x0 y:(r)+,b
		unhandled("a,x0 y:(r)+,b");
		break;
		}
	case 1078: { // a,x0 y:(r),b
		unhandled("a,x0 y:(r),b");
		break;
		}
	case 1079: { // a,x0 y:(r+n),b
		unhandled("a,x0 y:(r+n),b");
		break;
		}
	case 1080: { // a,x0 y:-(r),b
		unhandled("a,x0 y:-(r),b");
		break;
		}
	case 1081: { // a,x1 y:(r)-n,b
		unhandled("a,x1 y:(r)-n,b");
		break;
		}
	case 1082: { // a,x1 y:(r)+n,b
		unhandled("a,x1 y:(r)+n,b");
		break;
		}
	case 1083: { // a,x1 y:(r)-,b
		unhandled("a,x1 y:(r)-,b");
		break;
		}
	case 1084: { // a,x1 y:(r)+,b
		unhandled("a,x1 y:(r)+,b");
		break;
		}
	case 1085: { // a,x1 y:(r),b
		unhandled("a,x1 y:(r),b");
		break;
		}
	case 1086: { // a,x1 y:(r+n),b
		unhandled("a,x1 y:(r+n),b");
		break;
		}
	case 1087: { // a,x1 y:-(r),b
		unhandled("a,x1 y:-(r),b");
		break;
		}
	case 1088: { // b,x0 y:(r)-n,b
		unhandled("b,x0 y:(r)-n,b");
		break;
		}
	case 1089: { // b,x0 y:(r)+n,b
		unhandled("b,x0 y:(r)+n,b");
		break;
		}
	case 1090: { // b,x0 y:(r)-,b
		unhandled("b,x0 y:(r)-,b");
		break;
		}
	case 1091: { // b,x0 y:(r)+,b
		unhandled("b,x0 y:(r)+,b");
		break;
		}
	case 1092: { // b,x0 y:(r),b
		unhandled("b,x0 y:(r),b");
		break;
		}
	case 1093: { // b,x0 y:(r+n),b
		unhandled("b,x0 y:(r+n),b");
		break;
		}
	case 1094: { // b,x0 y:-(r),b
		unhandled("b,x0 y:-(r),b");
		break;
		}
	case 1095: { // b,x1 y:(r)-n,b
		unhandled("b,x1 y:(r)-n,b");
		break;
		}
	case 1096: { // b,x1 y:(r)+n,b
		unhandled("b,x1 y:(r)+n,b");
		break;
		}
	case 1097: { // b,x1 y:(r)-,b
		unhandled("b,x1 y:(r)-,b");
		break;
		}
	case 1098: { // b,x1 y:(r)+,b
		unhandled("b,x1 y:(r)+,b");
		break;
		}
	case 1099: { // b,x1 y:(r),b
		unhandled("b,x1 y:(r),b");
		break;
		}
	case 1100: { // b,x1 y:(r+n),b
		unhandled("b,x1 y:(r+n),b");
		break;
		}
	case 1101: { // b,x1 y:-(r),b
		unhandled("b,x1 y:-(r),b");
		break;
		}
	case 1102: { // a,x0 y0,y:(r)-n
		unhandled("a,x0 y0,y:(r)-n");
		break;
		}
	case 1103: { // a,x0 y0,y:(r)+n
		unhandled("a,x0 y0,y:(r)+n");
		break;
		}
	case 1104: { // a,x0 y0,y:(r)-
		unhandled("a,x0 y0,y:(r)-");
		break;
		}
	case 1105: { // a,x0 y0,y:(r)+
		unhandled("a,x0 y0,y:(r)+");
		break;
		}
	case 1106: { // a,x0 y0,y:(r)
		unhandled("a,x0 y0,y:(r)");
		break;
		}
	case 1107: { // a,x0 y0,y:(r+n)
		unhandled("a,x0 y0,y:(r+n)");
		break;
		}
	case 1108: { // a,x0 y0,y:-(r)
		unhandled("a,x0 y0,y:-(r)");
		break;
		}
	case 1109: { // a,x1 y0,y:(r)-n
		unhandled("a,x1 y0,y:(r)-n");
		break;
		}
	case 1110: { // a,x1 y0,y:(r)+n
		unhandled("a,x1 y0,y:(r)+n");
		break;
		}
	case 1111: { // a,x1 y0,y:(r)-
		unhandled("a,x1 y0,y:(r)-");
		break;
		}
	case 1112: { // a,x1 y0,y:(r)+
		unhandled("a,x1 y0,y:(r)+");
		break;
		}
	case 1113: { // a,x1 y0,y:(r)
		unhandled("a,x1 y0,y:(r)");
		break;
		}
	case 1114: { // a,x1 y0,y:(r+n)
		unhandled("a,x1 y0,y:(r+n)");
		break;
		}
	case 1115: { // a,x1 y0,y:-(r)
		unhandled("a,x1 y0,y:-(r)");
		break;
		}
	case 1116: { // b,x0 y0,y:(r)-n
		unhandled("b,x0 y0,y:(r)-n");
		break;
		}
	case 1117: { // b,x0 y0,y:(r)+n
		unhandled("b,x0 y0,y:(r)+n");
		break;
		}
	case 1118: { // b,x0 y0,y:(r)-
		unhandled("b,x0 y0,y:(r)-");
		break;
		}
	case 1119: { // b,x0 y0,y:(r)+
		unhandled("b,x0 y0,y:(r)+");
		break;
		}
	case 1120: { // b,x0 y0,y:(r)
		unhandled("b,x0 y0,y:(r)");
		break;
		}
	case 1121: { // b,x0 y0,y:(r+n)
		unhandled("b,x0 y0,y:(r+n)");
		break;
		}
	case 1122: { // b,x0 y0,y:-(r)
		unhandled("b,x0 y0,y:-(r)");
		break;
		}
	case 1123: { // b,x1 y0,y:(r)-n
		unhandled("b,x1 y0,y:(r)-n");
		break;
		}
	case 1124: { // b,x1 y0,y:(r)+n
		unhandled("b,x1 y0,y:(r)+n");
		break;
		}
	case 1125: { // b,x1 y0,y:(r)-
		unhandled("b,x1 y0,y:(r)-");
		break;
		}
	case 1126: { // b,x1 y0,y:(r)+
		unhandled("b,x1 y0,y:(r)+");
		break;
		}
	case 1127: { // b,x1 y0,y:(r)
		unhandled("b,x1 y0,y:(r)");
		break;
		}
	case 1128: { // b,x1 y0,y:(r+n)
		unhandled("b,x1 y0,y:(r+n)");
		break;
		}
	case 1129: { // b,x1 y0,y:-(r)
		unhandled("b,x1 y0,y:-(r)");
		break;
		}
	case 1130: { // a,x0 y1,y:(r)-n
		unhandled("a,x0 y1,y:(r)-n");
		break;
		}
	case 1131: { // a,x0 y1,y:(r)+n
		unhandled("a,x0 y1,y:(r)+n");
		break;
		}
	case 1132: { // a,x0 y1,y:(r)-
		unhandled("a,x0 y1,y:(r)-");
		break;
		}
	case 1133: { // a,x0 y1,y:(r)+
		unhandled("a,x0 y1,y:(r)+");
		break;
		}
	case 1134: { // a,x0 y1,y:(r)
		unhandled("a,x0 y1,y:(r)");
		break;
		}
	case 1135: { // a,x0 y1,y:(r+n)
		unhandled("a,x0 y1,y:(r+n)");
		break;
		}
	case 1136: { // a,x0 y1,y:-(r)
		unhandled("a,x0 y1,y:-(r)");
		break;
		}
	case 1137: { // a,x1 y1,y:(r)-n
		unhandled("a,x1 y1,y:(r)-n");
		break;
		}
	case 1138: { // a,x1 y1,y:(r)+n
		unhandled("a,x1 y1,y:(r)+n");
		break;
		}
	case 1139: { // a,x1 y1,y:(r)-
		unhandled("a,x1 y1,y:(r)-");
		break;
		}
	case 1140: { // a,x1 y1,y:(r)+
		unhandled("a,x1 y1,y:(r)+");
		break;
		}
	case 1141: { // a,x1 y1,y:(r)
		unhandled("a,x1 y1,y:(r)");
		break;
		}
	case 1142: { // a,x1 y1,y:(r+n)
		unhandled("a,x1 y1,y:(r+n)");
		break;
		}
	case 1143: { // a,x1 y1,y:-(r)
		unhandled("a,x1 y1,y:-(r)");
		break;
		}
	case 1144: { // b,x0 y1,y:(r)-n
		unhandled("b,x0 y1,y:(r)-n");
		break;
		}
	case 1145: { // b,x0 y1,y:(r)+n
		unhandled("b,x0 y1,y:(r)+n");
		break;
		}
	case 1146: { // b,x0 y1,y:(r)-
		unhandled("b,x0 y1,y:(r)-");
		break;
		}
	case 1147: { // b,x0 y1,y:(r)+
		unhandled("b,x0 y1,y:(r)+");
		break;
		}
	case 1148: { // b,x0 y1,y:(r)
		unhandled("b,x0 y1,y:(r)");
		break;
		}
	case 1149: { // b,x0 y1,y:(r+n)
		unhandled("b,x0 y1,y:(r+n)");
		break;
		}
	case 1150: { // b,x0 y1,y:-(r)
		unhandled("b,x0 y1,y:-(r)");
		break;
		}
	case 1151: { // b,x1 y1,y:(r)-n
		unhandled("b,x1 y1,y:(r)-n");
		break;
		}
	case 1152: { // b,x1 y1,y:(r)+n
		unhandled("b,x1 y1,y:(r)+n");
		break;
		}
	case 1153: { // b,x1 y1,y:(r)-
		unhandled("b,x1 y1,y:(r)-");
		break;
		}
	case 1154: { // b,x1 y1,y:(r)+
		unhandled("b,x1 y1,y:(r)+");
		break;
		}
	case 1155: { // b,x1 y1,y:(r)
		unhandled("b,x1 y1,y:(r)");
		break;
		}
	case 1156: { // b,x1 y1,y:(r+n)
		unhandled("b,x1 y1,y:(r+n)");
		break;
		}
	case 1157: { // b,x1 y1,y:-(r)
		unhandled("b,x1 y1,y:-(r)");
		break;
		}
	case 1158: { // a,x0 a,y:(r)-n
		unhandled("a,x0 a,y:(r)-n");
		break;
		}
	case 1159: { // a,x0 a,y:(r)+n
		unhandled("a,x0 a,y:(r)+n");
		break;
		}
	case 1160: { // a,x0 a,y:(r)-
		unhandled("a,x0 a,y:(r)-");
		break;
		}
	case 1161: { // a,x0 a,y:(r)+
		unhandled("a,x0 a,y:(r)+");
		break;
		}
	case 1162: { // a,x0 a,y:(r)
		unhandled("a,x0 a,y:(r)");
		break;
		}
	case 1163: { // a,x0 a,y:(r+n)
		unhandled("a,x0 a,y:(r+n)");
		break;
		}
	case 1164: { // a,x0 a,y:-(r)
		unhandled("a,x0 a,y:-(r)");
		break;
		}
	case 1165: { // a,x1 a,y:(r)-n
		unhandled("a,x1 a,y:(r)-n");
		break;
		}
	case 1166: { // a,x1 a,y:(r)+n
		unhandled("a,x1 a,y:(r)+n");
		break;
		}
	case 1167: { // a,x1 a,y:(r)-
		unhandled("a,x1 a,y:(r)-");
		break;
		}
	case 1168: { // a,x1 a,y:(r)+
		unhandled("a,x1 a,y:(r)+");
		break;
		}
	case 1169: { // a,x1 a,y:(r)
		unhandled("a,x1 a,y:(r)");
		break;
		}
	case 1170: { // a,x1 a,y:(r+n)
		unhandled("a,x1 a,y:(r+n)");
		break;
		}
	case 1171: { // a,x1 a,y:-(r)
		unhandled("a,x1 a,y:-(r)");
		break;
		}
	case 1172: { // b,x0 a,y:(r)-n
		unhandled("b,x0 a,y:(r)-n");
		break;
		}
	case 1173: { // b,x0 a,y:(r)+n
		unhandled("b,x0 a,y:(r)+n");
		break;
		}
	case 1174: { // b,x0 a,y:(r)-
		unhandled("b,x0 a,y:(r)-");
		break;
		}
	case 1175: { // b,x0 a,y:(r)+
		unhandled("b,x0 a,y:(r)+");
		break;
		}
	case 1176: { // b,x0 a,y:(r)
		unhandled("b,x0 a,y:(r)");
		break;
		}
	case 1177: { // b,x0 a,y:(r+n)
		unhandled("b,x0 a,y:(r+n)");
		break;
		}
	case 1178: { // b,x0 a,y:-(r)
		unhandled("b,x0 a,y:-(r)");
		break;
		}
	case 1179: { // b,x1 a,y:(r)-n
		unhandled("b,x1 a,y:(r)-n");
		break;
		}
	case 1180: { // b,x1 a,y:(r)+n
		unhandled("b,x1 a,y:(r)+n");
		break;
		}
	case 1181: { // b,x1 a,y:(r)-
		unhandled("b,x1 a,y:(r)-");
		break;
		}
	case 1182: { // b,x1 a,y:(r)+
		unhandled("b,x1 a,y:(r)+");
		break;
		}
	case 1183: { // b,x1 a,y:(r)
		unhandled("b,x1 a,y:(r)");
		break;
		}
	case 1184: { // b,x1 a,y:(r+n)
		unhandled("b,x1 a,y:(r+n)");
		break;
		}
	case 1185: { // b,x1 a,y:-(r)
		unhandled("b,x1 a,y:-(r)");
		break;
		}
	case 1186: { // a,x0 b,y:(r)-n
		unhandled("a,x0 b,y:(r)-n");
		break;
		}
	case 1187: { // a,x0 b,y:(r)+n
		unhandled("a,x0 b,y:(r)+n");
		break;
		}
	case 1188: { // a,x0 b,y:(r)-
		unhandled("a,x0 b,y:(r)-");
		break;
		}
	case 1189: { // a,x0 b,y:(r)+
		unhandled("a,x0 b,y:(r)+");
		break;
		}
	case 1190: { // a,x0 b,y:(r)
		unhandled("a,x0 b,y:(r)");
		break;
		}
	case 1191: { // a,x0 b,y:(r+n)
		unhandled("a,x0 b,y:(r+n)");
		break;
		}
	case 1192: { // a,x0 b,y:-(r)
		unhandled("a,x0 b,y:-(r)");
		break;
		}
	case 1193: { // a,x1 b,y:(r)-n
		unhandled("a,x1 b,y:(r)-n");
		break;
		}
	case 1194: { // a,x1 b,y:(r)+n
		unhandled("a,x1 b,y:(r)+n");
		break;
		}
	case 1195: { // a,x1 b,y:(r)-
		unhandled("a,x1 b,y:(r)-");
		break;
		}
	case 1196: { // a,x1 b,y:(r)+
		unhandled("a,x1 b,y:(r)+");
		break;
		}
	case 1197: { // a,x1 b,y:(r)
		unhandled("a,x1 b,y:(r)");
		break;
		}
	case 1198: { // a,x1 b,y:(r+n)
		unhandled("a,x1 b,y:(r+n)");
		break;
		}
	case 1199: { // a,x1 b,y:-(r)
		unhandled("a,x1 b,y:-(r)");
		break;
		}
	case 1200: { // b,x0 b,y:(r)-n
		unhandled("b,x0 b,y:(r)-n");
		break;
		}
	case 1201: { // b,x0 b,y:(r)+n
		unhandled("b,x0 b,y:(r)+n");
		break;
		}
	case 1202: { // b,x0 b,y:(r)-
		unhandled("b,x0 b,y:(r)-");
		break;
		}
	case 1203: { // b,x0 b,y:(r)+
		unhandled("b,x0 b,y:(r)+");
		break;
		}
	case 1204: { // b,x0 b,y:(r)
		unhandled("b,x0 b,y:(r)");
		break;
		}
	case 1205: { // b,x0 b,y:(r+n)
		unhandled("b,x0 b,y:(r+n)");
		break;
		}
	case 1206: { // b,x0 b,y:-(r)
		unhandled("b,x0 b,y:-(r)");
		break;
		}
	case 1207: { // b,x1 b,y:(r)-n
		unhandled("b,x1 b,y:(r)-n");
		break;
		}
	case 1208: { // b,x1 b,y:(r)+n
		unhandled("b,x1 b,y:(r)+n");
		break;
		}
	case 1209: { // b,x1 b,y:(r)-
		unhandled("b,x1 b,y:(r)-");
		break;
		}
	case 1210: { // b,x1 b,y:(r)+
		unhandled("b,x1 b,y:(r)+");
		break;
		}
	case 1211: { // b,x1 b,y:(r)
		unhandled("b,x1 b,y:(r)");
		break;
		}
	case 1212: { // b,x1 b,y:(r+n)
		unhandled("b,x1 b,y:(r+n)");
		break;
		}
	case 1213: { // b,x1 b,y:-(r)
		unhandled("b,x1 b,y:-(r)");
		break;
		}
	case 1214: { // a,x0 x:[abs],y0
		unhandled("a,x0 x:[abs],y0");
		break;
		}
	case 1215: { // a,x1 x:[abs],y0
		unhandled("a,x1 x:[abs],y0");
		break;
		}
	case 1216: { // b,x0 x:[abs],y0
		unhandled("b,x0 x:[abs],y0");
		break;
		}
	case 1217: { // b,x1 x:[abs],y0
		unhandled("b,x1 x:[abs],y0");
		break;
		}
	case 1218: { // a,x0 x:[abs],y1
		unhandled("a,x0 x:[abs],y1");
		break;
		}
	case 1219: { // a,x1 x:[abs],y1
		unhandled("a,x1 x:[abs],y1");
		break;
		}
	case 1220: { // b,x0 x:[abs],y1
		unhandled("b,x0 x:[abs],y1");
		break;
		}
	case 1221: { // b,x1 x:[abs],y1
		unhandled("b,x1 x:[abs],y1");
		break;
		}
	case 1222: { // a,x0 x:[abs],a
		unhandled("a,x0 x:[abs],a");
		break;
		}
	case 1223: { // a,x1 x:[abs],a
		unhandled("a,x1 x:[abs],a");
		break;
		}
	case 1224: { // b,x0 x:[abs],a
		unhandled("b,x0 x:[abs],a");
		break;
		}
	case 1225: { // b,x1 x:[abs],a
		unhandled("b,x1 x:[abs],a");
		break;
		}
	case 1226: { // a,x0 x:[abs],b
		unhandled("a,x0 x:[abs],b");
		break;
		}
	case 1227: { // a,x1 x:[abs],b
		unhandled("a,x1 x:[abs],b");
		break;
		}
	case 1228: { // b,x0 x:[abs],b
		unhandled("b,x0 x:[abs],b");
		break;
		}
	case 1229: { // b,x1 x:[abs],b
		unhandled("b,x1 x:[abs],b");
		break;
		}
	case 1230: { // a,x0 x:#[i],y0
		unhandled("a,x0 x:#[i],y0");
		break;
		}
	case 1231: { // a,x1 x:#[i],y0
		unhandled("a,x1 x:#[i],y0");
		break;
		}
	case 1232: { // b,x0 x:#[i],y0
		unhandled("b,x0 x:#[i],y0");
		break;
		}
	case 1233: { // b,x1 x:#[i],y0
		unhandled("b,x1 x:#[i],y0");
		break;
		}
	case 1234: { // a,x0 x:#[i],y1
		unhandled("a,x0 x:#[i],y1");
		break;
		}
	case 1235: { // a,x1 x:#[i],y1
		unhandled("a,x1 x:#[i],y1");
		break;
		}
	case 1236: { // b,x0 x:#[i],y1
		unhandled("b,x0 x:#[i],y1");
		break;
		}
	case 1237: { // b,x1 x:#[i],y1
		unhandled("b,x1 x:#[i],y1");
		break;
		}
	case 1238: { // a,x0 x:#[i],a
		unhandled("a,x0 x:#[i],a");
		break;
		}
	case 1239: { // a,x1 x:#[i],a
		unhandled("a,x1 x:#[i],a");
		break;
		}
	case 1240: { // b,x0 x:#[i],a
		unhandled("b,x0 x:#[i],a");
		break;
		}
	case 1241: { // b,x1 x:#[i],a
		unhandled("b,x1 x:#[i],a");
		break;
		}
	case 1242: { // a,x0 x:#[i],b
		unhandled("a,x0 x:#[i],b");
		break;
		}
	case 1243: { // a,x1 x:#[i],b
		unhandled("a,x1 x:#[i],b");
		break;
		}
	case 1244: { // b,x0 x:#[i],b
		unhandled("b,x0 x:#[i],b");
		break;
		}
	case 1245: { // b,x1 x:#[i],b
		unhandled("b,x1 x:#[i],b");
		break;
		}
	case 1246: { // a,y:(r)-n y0,a
		unhandled("a,y:(r)-n y0,a");
		break;
		}
	case 1247: { // a,y:(r)+n y0,a
		unhandled("a,y:(r)+n y0,a");
		break;
		}
	case 1248: { // a,y:(r)- y0,a
		unhandled("a,y:(r)- y0,a");
		break;
		}
	case 1249: { // a,y:(r)+ y0,a
		unhandled("a,y:(r)+ y0,a");
		break;
		}
	case 1250: { // a,y:(r) y0,a
		unhandled("a,y:(r) y0,a");
		break;
		}
	case 1251: { // a,y:(r+n) y0,a
		unhandled("a,y:(r+n) y0,a");
		break;
		}
	case 1252: { // a,y:-(r) y0,a
		unhandled("a,y:-(r) y0,a");
		break;
		}
	case 1253: { // b,y:(r)-n y0,b
		unhandled("b,y:(r)-n y0,b");
		break;
		}
	case 1254: { // b,y:(r)+n y0,b
		unhandled("b,y:(r)+n y0,b");
		break;
		}
	case 1255: { // b,y:(r)- y0,b
		unhandled("b,y:(r)- y0,b");
		break;
		}
	case 1256: { // b,y:(r)+ y0,b
		unhandled("b,y:(r)+ y0,b");
		break;
		}
	case 1257: { // b,y:(r) y0,b
		unhandled("b,y:(r) y0,b");
		break;
		}
	case 1258: { // b,y:(r+n) y0,b
		unhandled("b,y:(r+n) y0,b");
		break;
		}
	case 1259: { // b,y:-(r) y0,b
		unhandled("b,y:-(r) y0,b");
		break;
		}
	case 1260: { // l:(r)-n,a10
		unhandled("l:(r)-n,a10");
		break;
		}
	case 1261: { // l:(r)+n,a10
		unhandled("l:(r)+n,a10");
		break;
		}
	case 1262: { // l:(r)-,a10
		unhandled("l:(r)-,a10");
		break;
		}
	case 1263: { // l:(r)+,a10
		unhandled("l:(r)+,a10");
		break;
		}
	case 1264: { // l:(r),a10
		unhandled("l:(r),a10");
		break;
		}
	case 1265: { // l:(r+n),a10
		unhandled("l:(r+n),a10");
		break;
		}
	case 1266: { // l:-(r),a10
		unhandled("l:-(r),a10");
		break;
		}
	case 1267: { // l:(r)-n,b10
		unhandled("l:(r)-n,b10");
		break;
		}
	case 1268: { // l:(r)+n,b10
		unhandled("l:(r)+n,b10");
		break;
		}
	case 1269: { // l:(r)-,b10
		unhandled("l:(r)-,b10");
		break;
		}
	case 1270: { // l:(r)+,b10
		unhandled("l:(r)+,b10");
		break;
		}
	case 1271: { // l:(r),b10
		unhandled("l:(r),b10");
		break;
		}
	case 1272: { // l:(r+n),b10
		unhandled("l:(r+n),b10");
		break;
		}
	case 1273: { // l:-(r),b10
		unhandled("l:-(r),b10");
		break;
		}
	case 1274: { // l:(r)-n,x
		unhandled("l:(r)-n,x");
		break;
		}
	case 1275: { // l:(r)+n,x
		unhandled("l:(r)+n,x");
		break;
		}
	case 1276: { // l:(r)-,x
		unhandled("l:(r)-,x");
		break;
		}
	case 1277: { // l:(r)+,x
		unhandled("l:(r)+,x");
		break;
		}
	case 1278: { // l:(r),x
		unhandled("l:(r),x");
		break;
		}
	case 1279: { // l:(r+n),x
		unhandled("l:(r+n),x");
		break;
		}
	case 1280: { // l:-(r),x
		unhandled("l:-(r),x");
		break;
		}
	case 1281: { // l:(r)-n,y
		unhandled("l:(r)-n,y");
		break;
		}
	case 1282: { // l:(r)+n,y
		unhandled("l:(r)+n,y");
		break;
		}
	case 1283: { // l:(r)-,y
		unhandled("l:(r)-,y");
		break;
		}
	case 1284: { // l:(r)+,y
		unhandled("l:(r)+,y");
		break;
		}
	case 1285: { // l:(r),y
		unhandled("l:(r),y");
		break;
		}
	case 1286: { // l:(r+n),y
		unhandled("l:(r+n),y");
		break;
		}
	case 1287: { // l:-(r),y
		unhandled("l:-(r),y");
		break;
		}
	case 1288: { // l:(r)-n,a
		unhandled("l:(r)-n,a");
		break;
		}
	case 1289: { // l:(r)+n,a
		unhandled("l:(r)+n,a");
		break;
		}
	case 1290: { // l:(r)-,a
		unhandled("l:(r)-,a");
		break;
		}
	case 1291: { // l:(r)+,a
		unhandled("l:(r)+,a");
		break;
		}
	case 1292: { // l:(r),a
		unhandled("l:(r),a");
		break;
		}
	case 1293: { // l:(r+n),a
		unhandled("l:(r+n),a");
		break;
		}
	case 1294: { // l:-(r),a
		unhandled("l:-(r),a");
		break;
		}
	case 1295: { // l:(r)-n,b
		unhandled("l:(r)-n,b");
		break;
		}
	case 1296: { // l:(r)+n,b
		unhandled("l:(r)+n,b");
		break;
		}
	case 1297: { // l:(r)-,b
		unhandled("l:(r)-,b");
		break;
		}
	case 1298: { // l:(r)+,b
		unhandled("l:(r)+,b");
		break;
		}
	case 1299: { // l:(r),b
		unhandled("l:(r),b");
		break;
		}
	case 1300: { // l:(r+n),b
		unhandled("l:(r+n),b");
		break;
		}
	case 1301: { // l:-(r),b
		unhandled("l:-(r),b");
		break;
		}
	case 1302: { // l:(r)-n,ab
		unhandled("l:(r)-n,ab");
		break;
		}
	case 1303: { // l:(r)+n,ab
		unhandled("l:(r)+n,ab");
		break;
		}
	case 1304: { // l:(r)-,ab
		unhandled("l:(r)-,ab");
		break;
		}
	case 1305: { // l:(r)+,ab
		unhandled("l:(r)+,ab");
		break;
		}
	case 1306: { // l:(r),ab
		unhandled("l:(r),ab");
		break;
		}
	case 1307: { // l:(r+n),ab
		unhandled("l:(r+n),ab");
		break;
		}
	case 1308: { // l:-(r),ab
		unhandled("l:-(r),ab");
		break;
		}
	case 1309: { // l:(r)-n,ba
		unhandled("l:(r)-n,ba");
		break;
		}
	case 1310: { // l:(r)+n,ba
		unhandled("l:(r)+n,ba");
		break;
		}
	case 1311: { // l:(r)-,ba
		unhandled("l:(r)-,ba");
		break;
		}
	case 1312: { // l:(r)+,ba
		unhandled("l:(r)+,ba");
		break;
		}
	case 1313: { // l:(r),ba
		unhandled("l:(r),ba");
		break;
		}
	case 1314: { // l:(r+n),ba
		unhandled("l:(r+n),ba");
		break;
		}
	case 1315: { // l:-(r),ba
		unhandled("l:-(r),ba");
		break;
		}
	case 1316: { // l:[abs],a10
		unhandled("l:[abs],a10");
		break;
		}
	case 1317: { // l:[abs],b10
		unhandled("l:[abs],b10");
		break;
		}
	case 1318: { // l:[abs],x
		unhandled("l:[abs],x");
		break;
		}
	case 1319: { // l:[abs],y
		unhandled("l:[abs],y");
		break;
		}
	case 1320: { // l:[abs],a
		unhandled("l:[abs],a");
		break;
		}
	case 1321: { // l:[abs],b
		unhandled("l:[abs],b");
		break;
		}
	case 1322: { // l:[abs],ab
		unhandled("l:[abs],ab");
		break;
		}
	case 1323: { // l:[abs],ba
		unhandled("l:[abs],ba");
		break;
		}
	case 1324: { // l:[aa],a10
		unhandled("l:[aa],a10");
		break;
		}
	case 1325: { // l:[aa],b10
		unhandled("l:[aa],b10");
		break;
		}
	case 1326: { // l:[aa],x
		unhandled("l:[aa],x");
		break;
		}
	case 1327: { // l:[aa],y
		unhandled("l:[aa],y");
		break;
		}
	case 1328: { // l:[aa],a
		unhandled("l:[aa],a");
		break;
		}
	case 1329: { // l:[aa],b
		unhandled("l:[aa],b");
		break;
		}
	case 1330: { // l:[aa],ab
		unhandled("l:[aa],ab");
		break;
		}
	case 1331: { // l:[aa],ba
		unhandled("l:[aa],ba");
		break;
		}
	case 1332: { // a10,l:(r)-n
		unhandled("a10,l:(r)-n");
		break;
		}
	case 1333: { // a10,l:(r)+n
		unhandled("a10,l:(r)+n");
		break;
		}
	case 1334: { // a10,l:(r)-
		unhandled("a10,l:(r)-");
		break;
		}
	case 1335: { // a10,l:(r)+
		unhandled("a10,l:(r)+");
		break;
		}
	case 1336: { // a10,l:(r)
		unhandled("a10,l:(r)");
		break;
		}
	case 1337: { // a10,l:(r+n)
		unhandled("a10,l:(r+n)");
		break;
		}
	case 1338: { // a10,l:-(r)
		unhandled("a10,l:-(r)");
		break;
		}
	case 1339: { // b10,l:(r)-n
		unhandled("b10,l:(r)-n");
		break;
		}
	case 1340: { // b10,l:(r)+n
		unhandled("b10,l:(r)+n");
		break;
		}
	case 1341: { // b10,l:(r)-
		unhandled("b10,l:(r)-");
		break;
		}
	case 1342: { // b10,l:(r)+
		unhandled("b10,l:(r)+");
		break;
		}
	case 1343: { // b10,l:(r)
		unhandled("b10,l:(r)");
		break;
		}
	case 1344: { // b10,l:(r+n)
		unhandled("b10,l:(r+n)");
		break;
		}
	case 1345: { // b10,l:-(r)
		unhandled("b10,l:-(r)");
		break;
		}
	case 1346: { // x,l:(r)-n
		unhandled("x,l:(r)-n");
		break;
		}
	case 1347: { // x,l:(r)+n
		unhandled("x,l:(r)+n");
		break;
		}
	case 1348: { // x,l:(r)-
		unhandled("x,l:(r)-");
		break;
		}
	case 1349: { // x,l:(r)+
		unhandled("x,l:(r)+");
		break;
		}
	case 1350: { // x,l:(r)
		unhandled("x,l:(r)");
		break;
		}
	case 1351: { // x,l:(r+n)
		unhandled("x,l:(r+n)");
		break;
		}
	case 1352: { // x,l:-(r)
		unhandled("x,l:-(r)");
		break;
		}
	case 1353: { // y,l:(r)-n
		unhandled("y,l:(r)-n");
		break;
		}
	case 1354: { // y,l:(r)+n
		unhandled("y,l:(r)+n");
		break;
		}
	case 1355: { // y,l:(r)-
		unhandled("y,l:(r)-");
		break;
		}
	case 1356: { // y,l:(r)+
		unhandled("y,l:(r)+");
		break;
		}
	case 1357: { // y,l:(r)
		unhandled("y,l:(r)");
		break;
		}
	case 1358: { // y,l:(r+n)
		unhandled("y,l:(r+n)");
		break;
		}
	case 1359: { // y,l:-(r)
		unhandled("y,l:-(r)");
		break;
		}
	case 1360: { // a,l:(r)-n
		unhandled("a,l:(r)-n");
		break;
		}
	case 1361: { // a,l:(r)+n
		unhandled("a,l:(r)+n");
		break;
		}
	case 1362: { // a,l:(r)-
		unhandled("a,l:(r)-");
		break;
		}
	case 1363: { // a,l:(r)+
		unhandled("a,l:(r)+");
		break;
		}
	case 1364: { // a,l:(r)
		unhandled("a,l:(r)");
		break;
		}
	case 1365: { // a,l:(r+n)
		unhandled("a,l:(r+n)");
		break;
		}
	case 1366: { // a,l:-(r)
		unhandled("a,l:-(r)");
		break;
		}
	case 1367: { // b,l:(r)-n
		unhandled("b,l:(r)-n");
		break;
		}
	case 1368: { // b,l:(r)+n
		unhandled("b,l:(r)+n");
		break;
		}
	case 1369: { // b,l:(r)-
		unhandled("b,l:(r)-");
		break;
		}
	case 1370: { // b,l:(r)+
		unhandled("b,l:(r)+");
		break;
		}
	case 1371: { // b,l:(r)
		unhandled("b,l:(r)");
		break;
		}
	case 1372: { // b,l:(r+n)
		unhandled("b,l:(r+n)");
		break;
		}
	case 1373: { // b,l:-(r)
		unhandled("b,l:-(r)");
		break;
		}
	case 1374: { // ab,l:(r)-n
		unhandled("ab,l:(r)-n");
		break;
		}
	case 1375: { // ab,l:(r)+n
		unhandled("ab,l:(r)+n");
		break;
		}
	case 1376: { // ab,l:(r)-
		unhandled("ab,l:(r)-");
		break;
		}
	case 1377: { // ab,l:(r)+
		unhandled("ab,l:(r)+");
		break;
		}
	case 1378: { // ab,l:(r)
		unhandled("ab,l:(r)");
		break;
		}
	case 1379: { // ab,l:(r+n)
		unhandled("ab,l:(r+n)");
		break;
		}
	case 1380: { // ab,l:-(r)
		unhandled("ab,l:-(r)");
		break;
		}
	case 1381: { // ba,l:(r)-n
		unhandled("ba,l:(r)-n");
		break;
		}
	case 1382: { // ba,l:(r)+n
		unhandled("ba,l:(r)+n");
		break;
		}
	case 1383: { // ba,l:(r)-
		unhandled("ba,l:(r)-");
		break;
		}
	case 1384: { // ba,l:(r)+
		unhandled("ba,l:(r)+");
		break;
		}
	case 1385: { // ba,l:(r)
		unhandled("ba,l:(r)");
		break;
		}
	case 1386: { // ba,l:(r+n)
		unhandled("ba,l:(r+n)");
		break;
		}
	case 1387: { // ba,l:-(r)
		unhandled("ba,l:-(r)");
		break;
		}
	case 1388: { // a10,l:[aa]
		unhandled("a10,l:[aa]");
		break;
		}
	case 1389: { // b10,l:[aa]
		unhandled("b10,l:[aa]");
		break;
		}
	case 1390: { // x,l:[aa]
		unhandled("x,l:[aa]");
		break;
		}
	case 1391: { // y,l:[aa]
		unhandled("y,l:[aa]");
		break;
		}
	case 1392: { // a,l:[aa]
		unhandled("a,l:[aa]");
		break;
		}
	case 1393: { // b,l:[aa]
		unhandled("b,l:[aa]");
		break;
		}
	case 1394: { // ab,l:[aa]
		unhandled("ab,l:[aa]");
		break;
		}
	case 1395: { // ba,l:[aa]
		unhandled("ba,l:[aa]");
		break;
		}
	case 1396: { // x:(r)+n,x0 y:(rh)+n,y0
		unhandled("x:(r)+n,x0 y:(rh)+n,y0");
		break;
		}
	case 1397: { // x:(r)+n,x0 y:(rh)+n,y1
		unhandled("x:(r)+n,x0 y:(rh)+n,y1");
		break;
		}
	case 1398: { // x:(r)+n,x0 y:(rh)+n,a
		unhandled("x:(r)+n,x0 y:(rh)+n,a");
		break;
		}
	case 1399: { // x:(r)+n,x0 y:(rh)+n,b
		unhandled("x:(r)+n,x0 y:(rh)+n,b");
		break;
		}
	case 1400: { // x:(r)+n,x1 y:(rh)+n,y0
		unhandled("x:(r)+n,x1 y:(rh)+n,y0");
		break;
		}
	case 1401: { // x:(r)+n,x1 y:(rh)+n,y1
		unhandled("x:(r)+n,x1 y:(rh)+n,y1");
		break;
		}
	case 1402: { // x:(r)+n,x1 y:(rh)+n,a
		unhandled("x:(r)+n,x1 y:(rh)+n,a");
		break;
		}
	case 1403: { // x:(r)+n,x1 y:(rh)+n,b
		unhandled("x:(r)+n,x1 y:(rh)+n,b");
		break;
		}
	case 1404: { // x:(r)+n,a y:(rh)+n,y0
		unhandled("x:(r)+n,a y:(rh)+n,y0");
		break;
		}
	case 1405: { // x:(r)+n,a y:(rh)+n,y1
		unhandled("x:(r)+n,a y:(rh)+n,y1");
		break;
		}
	case 1406: { // x:(r)+n,a y:(rh)+n,a
		unhandled("x:(r)+n,a y:(rh)+n,a");
		break;
		}
	case 1407: { // x:(r)+n,a y:(rh)+n,b
		unhandled("x:(r)+n,a y:(rh)+n,b");
		break;
		}
	case 1408: { // x:(r)+n,b y:(rh)+n,y0
		unhandled("x:(r)+n,b y:(rh)+n,y0");
		break;
		}
	case 1409: { // x:(r)+n,b y:(rh)+n,y1
		unhandled("x:(r)+n,b y:(rh)+n,y1");
		break;
		}
	case 1410: { // x:(r)+n,b y:(rh)+n,a
		unhandled("x:(r)+n,b y:(rh)+n,a");
		break;
		}
	case 1411: { // x:(r)+n,b y:(rh)+n,b
		unhandled("x:(r)+n,b y:(rh)+n,b");
		break;
		}
	case 1412: { // x:(r)+n,x0 y:(rh)-,y0
		unhandled("x:(r)+n,x0 y:(rh)-,y0");
		break;
		}
	case 1413: { // x:(r)+n,x0 y:(rh)-,y1
		unhandled("x:(r)+n,x0 y:(rh)-,y1");
		break;
		}
	case 1414: { // x:(r)+n,x0 y:(rh)-,a
		unhandled("x:(r)+n,x0 y:(rh)-,a");
		break;
		}
	case 1415: { // x:(r)+n,x0 y:(rh)-,b
		unhandled("x:(r)+n,x0 y:(rh)-,b");
		break;
		}
	case 1416: { // x:(r)+n,x1 y:(rh)-,y0
		unhandled("x:(r)+n,x1 y:(rh)-,y0");
		break;
		}
	case 1417: { // x:(r)+n,x1 y:(rh)-,y1
		unhandled("x:(r)+n,x1 y:(rh)-,y1");
		break;
		}
	case 1418: { // x:(r)+n,x1 y:(rh)-,a
		unhandled("x:(r)+n,x1 y:(rh)-,a");
		break;
		}
	case 1419: { // x:(r)+n,x1 y:(rh)-,b
		unhandled("x:(r)+n,x1 y:(rh)-,b");
		break;
		}
	case 1420: { // x:(r)+n,a y:(rh)-,y0
		unhandled("x:(r)+n,a y:(rh)-,y0");
		break;
		}
	case 1421: { // x:(r)+n,a y:(rh)-,y1
		unhandled("x:(r)+n,a y:(rh)-,y1");
		break;
		}
	case 1422: { // x:(r)+n,a y:(rh)-,a
		unhandled("x:(r)+n,a y:(rh)-,a");
		break;
		}
	case 1423: { // x:(r)+n,a y:(rh)-,b
		unhandled("x:(r)+n,a y:(rh)-,b");
		break;
		}
	case 1424: { // x:(r)+n,b y:(rh)-,y0
		unhandled("x:(r)+n,b y:(rh)-,y0");
		break;
		}
	case 1425: { // x:(r)+n,b y:(rh)-,y1
		unhandled("x:(r)+n,b y:(rh)-,y1");
		break;
		}
	case 1426: { // x:(r)+n,b y:(rh)-,a
		unhandled("x:(r)+n,b y:(rh)-,a");
		break;
		}
	case 1427: { // x:(r)+n,b y:(rh)-,b
		unhandled("x:(r)+n,b y:(rh)-,b");
		break;
		}
	case 1428: { // x:(r)+n,x0 y:(rh)+,y0
		unhandled("x:(r)+n,x0 y:(rh)+,y0");
		break;
		}
	case 1429: { // x:(r)+n,x0 y:(rh)+,y1
		unhandled("x:(r)+n,x0 y:(rh)+,y1");
		break;
		}
	case 1430: { // x:(r)+n,x0 y:(rh)+,a
		unhandled("x:(r)+n,x0 y:(rh)+,a");
		break;
		}
	case 1431: { // x:(r)+n,x0 y:(rh)+,b
		unhandled("x:(r)+n,x0 y:(rh)+,b");
		break;
		}
	case 1432: { // x:(r)+n,x1 y:(rh)+,y0
		unhandled("x:(r)+n,x1 y:(rh)+,y0");
		break;
		}
	case 1433: { // x:(r)+n,x1 y:(rh)+,y1
		unhandled("x:(r)+n,x1 y:(rh)+,y1");
		break;
		}
	case 1434: { // x:(r)+n,x1 y:(rh)+,a
		unhandled("x:(r)+n,x1 y:(rh)+,a");
		break;
		}
	case 1435: { // x:(r)+n,x1 y:(rh)+,b
		unhandled("x:(r)+n,x1 y:(rh)+,b");
		break;
		}
	case 1436: { // x:(r)+n,a y:(rh)+,y0
		unhandled("x:(r)+n,a y:(rh)+,y0");
		break;
		}
	case 1437: { // x:(r)+n,a y:(rh)+,y1
		unhandled("x:(r)+n,a y:(rh)+,y1");
		break;
		}
	case 1438: { // x:(r)+n,a y:(rh)+,a
		unhandled("x:(r)+n,a y:(rh)+,a");
		break;
		}
	case 1439: { // x:(r)+n,a y:(rh)+,b
		unhandled("x:(r)+n,a y:(rh)+,b");
		break;
		}
	case 1440: { // x:(r)+n,b y:(rh)+,y0
		unhandled("x:(r)+n,b y:(rh)+,y0");
		break;
		}
	case 1441: { // x:(r)+n,b y:(rh)+,y1
		unhandled("x:(r)+n,b y:(rh)+,y1");
		break;
		}
	case 1442: { // x:(r)+n,b y:(rh)+,a
		unhandled("x:(r)+n,b y:(rh)+,a");
		break;
		}
	case 1443: { // x:(r)+n,b y:(rh)+,b
		unhandled("x:(r)+n,b y:(rh)+,b");
		break;
		}
	case 1444: { // x:(r)+n,x0 y:(rh),y0
		unhandled("x:(r)+n,x0 y:(rh),y0");
		break;
		}
	case 1445: { // x:(r)+n,x0 y:(rh),y1
		unhandled("x:(r)+n,x0 y:(rh),y1");
		break;
		}
	case 1446: { // x:(r)+n,x0 y:(rh),a
		unhandled("x:(r)+n,x0 y:(rh),a");
		break;
		}
	case 1447: { // x:(r)+n,x0 y:(rh),b
		unhandled("x:(r)+n,x0 y:(rh),b");
		break;
		}
	case 1448: { // x:(r)+n,x1 y:(rh),y0
		unhandled("x:(r)+n,x1 y:(rh),y0");
		break;
		}
	case 1449: { // x:(r)+n,x1 y:(rh),y1
		unhandled("x:(r)+n,x1 y:(rh),y1");
		break;
		}
	case 1450: { // x:(r)+n,x1 y:(rh),a
		unhandled("x:(r)+n,x1 y:(rh),a");
		break;
		}
	case 1451: { // x:(r)+n,x1 y:(rh),b
		unhandled("x:(r)+n,x1 y:(rh),b");
		break;
		}
	case 1452: { // x:(r)+n,a y:(rh),y0
		unhandled("x:(r)+n,a y:(rh),y0");
		break;
		}
	case 1453: { // x:(r)+n,a y:(rh),y1
		unhandled("x:(r)+n,a y:(rh),y1");
		break;
		}
	case 1454: { // x:(r)+n,a y:(rh),a
		unhandled("x:(r)+n,a y:(rh),a");
		break;
		}
	case 1455: { // x:(r)+n,a y:(rh),b
		unhandled("x:(r)+n,a y:(rh),b");
		break;
		}
	case 1456: { // x:(r)+n,b y:(rh),y0
		unhandled("x:(r)+n,b y:(rh),y0");
		break;
		}
	case 1457: { // x:(r)+n,b y:(rh),y1
		unhandled("x:(r)+n,b y:(rh),y1");
		break;
		}
	case 1458: { // x:(r)+n,b y:(rh),a
		unhandled("x:(r)+n,b y:(rh),a");
		break;
		}
	case 1459: { // x:(r)+n,b y:(rh),b
		unhandled("x:(r)+n,b y:(rh),b");
		break;
		}
	case 1460: { // x:(r)-,x0 y:(rh)+n,y0
		unhandled("x:(r)-,x0 y:(rh)+n,y0");
		break;
		}
	case 1461: { // x:(r)-,x0 y:(rh)+n,y1
		unhandled("x:(r)-,x0 y:(rh)+n,y1");
		break;
		}
	case 1462: { // x:(r)-,x0 y:(rh)+n,a
		unhandled("x:(r)-,x0 y:(rh)+n,a");
		break;
		}
	case 1463: { // x:(r)-,x0 y:(rh)+n,b
		unhandled("x:(r)-,x0 y:(rh)+n,b");
		break;
		}
	case 1464: { // x:(r)-,x1 y:(rh)+n,y0
		unhandled("x:(r)-,x1 y:(rh)+n,y0");
		break;
		}
	case 1465: { // x:(r)-,x1 y:(rh)+n,y1
		unhandled("x:(r)-,x1 y:(rh)+n,y1");
		break;
		}
	case 1466: { // x:(r)-,x1 y:(rh)+n,a
		unhandled("x:(r)-,x1 y:(rh)+n,a");
		break;
		}
	case 1467: { // x:(r)-,x1 y:(rh)+n,b
		unhandled("x:(r)-,x1 y:(rh)+n,b");
		break;
		}
	case 1468: { // x:(r)-,a y:(rh)+n,y0
		unhandled("x:(r)-,a y:(rh)+n,y0");
		break;
		}
	case 1469: { // x:(r)-,a y:(rh)+n,y1
		unhandled("x:(r)-,a y:(rh)+n,y1");
		break;
		}
	case 1470: { // x:(r)-,a y:(rh)+n,a
		unhandled("x:(r)-,a y:(rh)+n,a");
		break;
		}
	case 1471: { // x:(r)-,a y:(rh)+n,b
		unhandled("x:(r)-,a y:(rh)+n,b");
		break;
		}
	case 1472: { // x:(r)-,b y:(rh)+n,y0
		unhandled("x:(r)-,b y:(rh)+n,y0");
		break;
		}
	case 1473: { // x:(r)-,b y:(rh)+n,y1
		unhandled("x:(r)-,b y:(rh)+n,y1");
		break;
		}
	case 1474: { // x:(r)-,b y:(rh)+n,a
		unhandled("x:(r)-,b y:(rh)+n,a");
		break;
		}
	case 1475: { // x:(r)-,b y:(rh)+n,b
		unhandled("x:(r)-,b y:(rh)+n,b");
		break;
		}
	case 1476: { // x:(r)-,x0 y:(rh)-,y0
		unhandled("x:(r)-,x0 y:(rh)-,y0");
		break;
		}
	case 1477: { // x:(r)-,x0 y:(rh)-,y1
		unhandled("x:(r)-,x0 y:(rh)-,y1");
		break;
		}
	case 1478: { // x:(r)-,x0 y:(rh)-,a
		unhandled("x:(r)-,x0 y:(rh)-,a");
		break;
		}
	case 1479: { // x:(r)-,x0 y:(rh)-,b
		unhandled("x:(r)-,x0 y:(rh)-,b");
		break;
		}
	case 1480: { // x:(r)-,x1 y:(rh)-,y0
		unhandled("x:(r)-,x1 y:(rh)-,y0");
		break;
		}
	case 1481: { // x:(r)-,x1 y:(rh)-,y1
		unhandled("x:(r)-,x1 y:(rh)-,y1");
		break;
		}
	case 1482: { // x:(r)-,x1 y:(rh)-,a
		unhandled("x:(r)-,x1 y:(rh)-,a");
		break;
		}
	case 1483: { // x:(r)-,x1 y:(rh)-,b
		unhandled("x:(r)-,x1 y:(rh)-,b");
		break;
		}
	case 1484: { // x:(r)-,a y:(rh)-,y0
		unhandled("x:(r)-,a y:(rh)-,y0");
		break;
		}
	case 1485: { // x:(r)-,a y:(rh)-,y1
		unhandled("x:(r)-,a y:(rh)-,y1");
		break;
		}
	case 1486: { // x:(r)-,a y:(rh)-,a
		unhandled("x:(r)-,a y:(rh)-,a");
		break;
		}
	case 1487: { // x:(r)-,a y:(rh)-,b
		unhandled("x:(r)-,a y:(rh)-,b");
		break;
		}
	case 1488: { // x:(r)-,b y:(rh)-,y0
		unhandled("x:(r)-,b y:(rh)-,y0");
		break;
		}
	case 1489: { // x:(r)-,b y:(rh)-,y1
		unhandled("x:(r)-,b y:(rh)-,y1");
		break;
		}
	case 1490: { // x:(r)-,b y:(rh)-,a
		unhandled("x:(r)-,b y:(rh)-,a");
		break;
		}
	case 1491: { // x:(r)-,b y:(rh)-,b
		unhandled("x:(r)-,b y:(rh)-,b");
		break;
		}
	case 1492: { // x:(r)-,x0 y:(rh)+,y0
		unhandled("x:(r)-,x0 y:(rh)+,y0");
		break;
		}
	case 1493: { // x:(r)-,x0 y:(rh)+,y1
		unhandled("x:(r)-,x0 y:(rh)+,y1");
		break;
		}
	case 1494: { // x:(r)-,x0 y:(rh)+,a
		unhandled("x:(r)-,x0 y:(rh)+,a");
		break;
		}
	case 1495: { // x:(r)-,x0 y:(rh)+,b
		unhandled("x:(r)-,x0 y:(rh)+,b");
		break;
		}
	case 1496: { // x:(r)-,x1 y:(rh)+,y0
		unhandled("x:(r)-,x1 y:(rh)+,y0");
		break;
		}
	case 1497: { // x:(r)-,x1 y:(rh)+,y1
		unhandled("x:(r)-,x1 y:(rh)+,y1");
		break;
		}
	case 1498: { // x:(r)-,x1 y:(rh)+,a
		unhandled("x:(r)-,x1 y:(rh)+,a");
		break;
		}
	case 1499: { // x:(r)-,x1 y:(rh)+,b
		unhandled("x:(r)-,x1 y:(rh)+,b");
		break;
		}
	case 1500: { // x:(r)-,a y:(rh)+,y0
		unhandled("x:(r)-,a y:(rh)+,y0");
		break;
		}
	case 1501: { // x:(r)-,a y:(rh)+,y1
		unhandled("x:(r)-,a y:(rh)+,y1");
		break;
		}
	case 1502: { // x:(r)-,a y:(rh)+,a
		unhandled("x:(r)-,a y:(rh)+,a");
		break;
		}
	case 1503: { // x:(r)-,a y:(rh)+,b
		unhandled("x:(r)-,a y:(rh)+,b");
		break;
		}
	case 1504: { // x:(r)-,b y:(rh)+,y0
		unhandled("x:(r)-,b y:(rh)+,y0");
		break;
		}
	case 1505: { // x:(r)-,b y:(rh)+,y1
		unhandled("x:(r)-,b y:(rh)+,y1");
		break;
		}
	case 1506: { // x:(r)-,b y:(rh)+,a
		unhandled("x:(r)-,b y:(rh)+,a");
		break;
		}
	case 1507: { // x:(r)-,b y:(rh)+,b
		unhandled("x:(r)-,b y:(rh)+,b");
		break;
		}
	case 1508: { // x:(r)-,x0 y:(rh),y0
		unhandled("x:(r)-,x0 y:(rh),y0");
		break;
		}
	case 1509: { // x:(r)-,x0 y:(rh),y1
		unhandled("x:(r)-,x0 y:(rh),y1");
		break;
		}
	case 1510: { // x:(r)-,x0 y:(rh),a
		unhandled("x:(r)-,x0 y:(rh),a");
		break;
		}
	case 1511: { // x:(r)-,x0 y:(rh),b
		unhandled("x:(r)-,x0 y:(rh),b");
		break;
		}
	case 1512: { // x:(r)-,x1 y:(rh),y0
		unhandled("x:(r)-,x1 y:(rh),y0");
		break;
		}
	case 1513: { // x:(r)-,x1 y:(rh),y1
		unhandled("x:(r)-,x1 y:(rh),y1");
		break;
		}
	case 1514: { // x:(r)-,x1 y:(rh),a
		unhandled("x:(r)-,x1 y:(rh),a");
		break;
		}
	case 1515: { // x:(r)-,x1 y:(rh),b
		unhandled("x:(r)-,x1 y:(rh),b");
		break;
		}
	case 1516: { // x:(r)-,a y:(rh),y0
		unhandled("x:(r)-,a y:(rh),y0");
		break;
		}
	case 1517: { // x:(r)-,a y:(rh),y1
		unhandled("x:(r)-,a y:(rh),y1");
		break;
		}
	case 1518: { // x:(r)-,a y:(rh),a
		unhandled("x:(r)-,a y:(rh),a");
		break;
		}
	case 1519: { // x:(r)-,a y:(rh),b
		unhandled("x:(r)-,a y:(rh),b");
		break;
		}
	case 1520: { // x:(r)-,b y:(rh),y0
		unhandled("x:(r)-,b y:(rh),y0");
		break;
		}
	case 1521: { // x:(r)-,b y:(rh),y1
		unhandled("x:(r)-,b y:(rh),y1");
		break;
		}
	case 1522: { // x:(r)-,b y:(rh),a
		unhandled("x:(r)-,b y:(rh),a");
		break;
		}
	case 1523: { // x:(r)-,b y:(rh),b
		unhandled("x:(r)-,b y:(rh),b");
		break;
		}
	case 1524: { // x:(r)+,x0 y:(rh)+n,y0
		unhandled("x:(r)+,x0 y:(rh)+n,y0");
		break;
		}
	case 1525: { // x:(r)+,x0 y:(rh)+n,y1
		unhandled("x:(r)+,x0 y:(rh)+n,y1");
		break;
		}
	case 1526: { // x:(r)+,x0 y:(rh)+n,a
		unhandled("x:(r)+,x0 y:(rh)+n,a");
		break;
		}
	case 1527: { // x:(r)+,x0 y:(rh)+n,b
		unhandled("x:(r)+,x0 y:(rh)+n,b");
		break;
		}
	case 1528: { // x:(r)+,x1 y:(rh)+n,y0
		unhandled("x:(r)+,x1 y:(rh)+n,y0");
		break;
		}
	case 1529: { // x:(r)+,x1 y:(rh)+n,y1
		unhandled("x:(r)+,x1 y:(rh)+n,y1");
		break;
		}
	case 1530: { // x:(r)+,x1 y:(rh)+n,a
		unhandled("x:(r)+,x1 y:(rh)+n,a");
		break;
		}
	case 1531: { // x:(r)+,x1 y:(rh)+n,b
		unhandled("x:(r)+,x1 y:(rh)+n,b");
		break;
		}
	case 1532: { // x:(r)+,a y:(rh)+n,y0
		unhandled("x:(r)+,a y:(rh)+n,y0");
		break;
		}
	case 1533: { // x:(r)+,a y:(rh)+n,y1
		unhandled("x:(r)+,a y:(rh)+n,y1");
		break;
		}
	case 1534: { // x:(r)+,a y:(rh)+n,a
		unhandled("x:(r)+,a y:(rh)+n,a");
		break;
		}
	case 1535: { // x:(r)+,a y:(rh)+n,b
		unhandled("x:(r)+,a y:(rh)+n,b");
		break;
		}
	case 1536: { // x:(r)+,b y:(rh)+n,y0
		unhandled("x:(r)+,b y:(rh)+n,y0");
		break;
		}
	case 1537: { // x:(r)+,b y:(rh)+n,y1
		unhandled("x:(r)+,b y:(rh)+n,y1");
		break;
		}
	case 1538: { // x:(r)+,b y:(rh)+n,a
		unhandled("x:(r)+,b y:(rh)+n,a");
		break;
		}
	case 1539: { // x:(r)+,b y:(rh)+n,b
		unhandled("x:(r)+,b y:(rh)+n,b");
		break;
		}
	case 1540: { // x:(r)+,x0 y:(rh)-,y0
		unhandled("x:(r)+,x0 y:(rh)-,y0");
		break;
		}
	case 1541: { // x:(r)+,x0 y:(rh)-,y1
		unhandled("x:(r)+,x0 y:(rh)-,y1");
		break;
		}
	case 1542: { // x:(r)+,x0 y:(rh)-,a
		unhandled("x:(r)+,x0 y:(rh)-,a");
		break;
		}
	case 1543: { // x:(r)+,x0 y:(rh)-,b
		unhandled("x:(r)+,x0 y:(rh)-,b");
		break;
		}
	case 1544: { // x:(r)+,x1 y:(rh)-,y0
		unhandled("x:(r)+,x1 y:(rh)-,y0");
		break;
		}
	case 1545: { // x:(r)+,x1 y:(rh)-,y1
		unhandled("x:(r)+,x1 y:(rh)-,y1");
		break;
		}
	case 1546: { // x:(r)+,x1 y:(rh)-,a
		unhandled("x:(r)+,x1 y:(rh)-,a");
		break;
		}
	case 1547: { // x:(r)+,x1 y:(rh)-,b
		unhandled("x:(r)+,x1 y:(rh)-,b");
		break;
		}
	case 1548: { // x:(r)+,a y:(rh)-,y0
		unhandled("x:(r)+,a y:(rh)-,y0");
		break;
		}
	case 1549: { // x:(r)+,a y:(rh)-,y1
		unhandled("x:(r)+,a y:(rh)-,y1");
		break;
		}
	case 1550: { // x:(r)+,a y:(rh)-,a
		unhandled("x:(r)+,a y:(rh)-,a");
		break;
		}
	case 1551: { // x:(r)+,a y:(rh)-,b
		unhandled("x:(r)+,a y:(rh)-,b");
		break;
		}
	case 1552: { // x:(r)+,b y:(rh)-,y0
		unhandled("x:(r)+,b y:(rh)-,y0");
		break;
		}
	case 1553: { // x:(r)+,b y:(rh)-,y1
		unhandled("x:(r)+,b y:(rh)-,y1");
		break;
		}
	case 1554: { // x:(r)+,b y:(rh)-,a
		unhandled("x:(r)+,b y:(rh)-,a");
		break;
		}
	case 1555: { // x:(r)+,b y:(rh)-,b
		unhandled("x:(r)+,b y:(rh)-,b");
		break;
		}
	case 1556: { // x:(r)+,x0 y:(rh)+,y0
		unhandled("x:(r)+,x0 y:(rh)+,y0");
		break;
		}
	case 1557: { // x:(r)+,x0 y:(rh)+,y1
		unhandled("x:(r)+,x0 y:(rh)+,y1");
		break;
		}
	case 1558: { // x:(r)+,x0 y:(rh)+,a
		unhandled("x:(r)+,x0 y:(rh)+,a");
		break;
		}
	case 1559: { // x:(r)+,x0 y:(rh)+,b
		unhandled("x:(r)+,x0 y:(rh)+,b");
		break;
		}
	case 1560: { // x:(r)+,x1 y:(rh)+,y0
		unhandled("x:(r)+,x1 y:(rh)+,y0");
		break;
		}
	case 1561: { // x:(r)+,x1 y:(rh)+,y1
		unhandled("x:(r)+,x1 y:(rh)+,y1");
		break;
		}
	case 1562: { // x:(r)+,x1 y:(rh)+,a
		unhandled("x:(r)+,x1 y:(rh)+,a");
		break;
		}
	case 1563: { // x:(r)+,x1 y:(rh)+,b
		unhandled("x:(r)+,x1 y:(rh)+,b");
		break;
		}
	case 1564: { // x:(r)+,a y:(rh)+,y0
		unhandled("x:(r)+,a y:(rh)+,y0");
		break;
		}
	case 1565: { // x:(r)+,a y:(rh)+,y1
		unhandled("x:(r)+,a y:(rh)+,y1");
		break;
		}
	case 1566: { // x:(r)+,a y:(rh)+,a
		unhandled("x:(r)+,a y:(rh)+,a");
		break;
		}
	case 1567: { // x:(r)+,a y:(rh)+,b
		unhandled("x:(r)+,a y:(rh)+,b");
		break;
		}
	case 1568: { // x:(r)+,b y:(rh)+,y0
		unhandled("x:(r)+,b y:(rh)+,y0");
		break;
		}
	case 1569: { // x:(r)+,b y:(rh)+,y1
		unhandled("x:(r)+,b y:(rh)+,y1");
		break;
		}
	case 1570: { // x:(r)+,b y:(rh)+,a
		unhandled("x:(r)+,b y:(rh)+,a");
		break;
		}
	case 1571: { // x:(r)+,b y:(rh)+,b
		unhandled("x:(r)+,b y:(rh)+,b");
		break;
		}
	case 1572: { // x:(r)+,x0 y:(rh),y0
		unhandled("x:(r)+,x0 y:(rh),y0");
		break;
		}
	case 1573: { // x:(r)+,x0 y:(rh),y1
		unhandled("x:(r)+,x0 y:(rh),y1");
		break;
		}
	case 1574: { // x:(r)+,x0 y:(rh),a
		unhandled("x:(r)+,x0 y:(rh),a");
		break;
		}
	case 1575: { // x:(r)+,x0 y:(rh),b
		unhandled("x:(r)+,x0 y:(rh),b");
		break;
		}
	case 1576: { // x:(r)+,x1 y:(rh),y0
		unhandled("x:(r)+,x1 y:(rh),y0");
		break;
		}
	case 1577: { // x:(r)+,x1 y:(rh),y1
		unhandled("x:(r)+,x1 y:(rh),y1");
		break;
		}
	case 1578: { // x:(r)+,x1 y:(rh),a
		unhandled("x:(r)+,x1 y:(rh),a");
		break;
		}
	case 1579: { // x:(r)+,x1 y:(rh),b
		unhandled("x:(r)+,x1 y:(rh),b");
		break;
		}
	case 1580: { // x:(r)+,a y:(rh),y0
		unhandled("x:(r)+,a y:(rh),y0");
		break;
		}
	case 1581: { // x:(r)+,a y:(rh),y1
		unhandled("x:(r)+,a y:(rh),y1");
		break;
		}
	case 1582: { // x:(r)+,a y:(rh),a
		unhandled("x:(r)+,a y:(rh),a");
		break;
		}
	case 1583: { // x:(r)+,a y:(rh),b
		unhandled("x:(r)+,a y:(rh),b");
		break;
		}
	case 1584: { // x:(r)+,b y:(rh),y0
		unhandled("x:(r)+,b y:(rh),y0");
		break;
		}
	case 1585: { // x:(r)+,b y:(rh),y1
		unhandled("x:(r)+,b y:(rh),y1");
		break;
		}
	case 1586: { // x:(r)+,b y:(rh),a
		unhandled("x:(r)+,b y:(rh),a");
		break;
		}
	case 1587: { // x:(r)+,b y:(rh),b
		unhandled("x:(r)+,b y:(rh),b");
		break;
		}
	case 1588: { // x:(r),x0 y:(rh)+n,y0
		unhandled("x:(r),x0 y:(rh)+n,y0");
		break;
		}
	case 1589: { // x:(r),x0 y:(rh)+n,y1
		unhandled("x:(r),x0 y:(rh)+n,y1");
		break;
		}
	case 1590: { // x:(r),x0 y:(rh)+n,a
		unhandled("x:(r),x0 y:(rh)+n,a");
		break;
		}
	case 1591: { // x:(r),x0 y:(rh)+n,b
		unhandled("x:(r),x0 y:(rh)+n,b");
		break;
		}
	case 1592: { // x:(r),x1 y:(rh)+n,y0
		unhandled("x:(r),x1 y:(rh)+n,y0");
		break;
		}
	case 1593: { // x:(r),x1 y:(rh)+n,y1
		unhandled("x:(r),x1 y:(rh)+n,y1");
		break;
		}
	case 1594: { // x:(r),x1 y:(rh)+n,a
		unhandled("x:(r),x1 y:(rh)+n,a");
		break;
		}
	case 1595: { // x:(r),x1 y:(rh)+n,b
		unhandled("x:(r),x1 y:(rh)+n,b");
		break;
		}
	case 1596: { // x:(r),a y:(rh)+n,y0
		unhandled("x:(r),a y:(rh)+n,y0");
		break;
		}
	case 1597: { // x:(r),a y:(rh)+n,y1
		unhandled("x:(r),a y:(rh)+n,y1");
		break;
		}
	case 1598: { // x:(r),a y:(rh)+n,a
		unhandled("x:(r),a y:(rh)+n,a");
		break;
		}
	case 1599: { // x:(r),a y:(rh)+n,b
		unhandled("x:(r),a y:(rh)+n,b");
		break;
		}
	case 1600: { // x:(r),b y:(rh)+n,y0
		unhandled("x:(r),b y:(rh)+n,y0");
		break;
		}
	case 1601: { // x:(r),b y:(rh)+n,y1
		unhandled("x:(r),b y:(rh)+n,y1");
		break;
		}
	case 1602: { // x:(r),b y:(rh)+n,a
		unhandled("x:(r),b y:(rh)+n,a");
		break;
		}
	case 1603: { // x:(r),b y:(rh)+n,b
		unhandled("x:(r),b y:(rh)+n,b");
		break;
		}
	case 1604: { // x:(r),x0 y:(rh)-,y0
		unhandled("x:(r),x0 y:(rh)-,y0");
		break;
		}
	case 1605: { // x:(r),x0 y:(rh)-,y1
		unhandled("x:(r),x0 y:(rh)-,y1");
		break;
		}
	case 1606: { // x:(r),x0 y:(rh)-,a
		unhandled("x:(r),x0 y:(rh)-,a");
		break;
		}
	case 1607: { // x:(r),x0 y:(rh)-,b
		unhandled("x:(r),x0 y:(rh)-,b");
		break;
		}
	case 1608: { // x:(r),x1 y:(rh)-,y0
		unhandled("x:(r),x1 y:(rh)-,y0");
		break;
		}
	case 1609: { // x:(r),x1 y:(rh)-,y1
		unhandled("x:(r),x1 y:(rh)-,y1");
		break;
		}
	case 1610: { // x:(r),x1 y:(rh)-,a
		unhandled("x:(r),x1 y:(rh)-,a");
		break;
		}
	case 1611: { // x:(r),x1 y:(rh)-,b
		unhandled("x:(r),x1 y:(rh)-,b");
		break;
		}
	case 1612: { // x:(r),a y:(rh)-,y0
		unhandled("x:(r),a y:(rh)-,y0");
		break;
		}
	case 1613: { // x:(r),a y:(rh)-,y1
		unhandled("x:(r),a y:(rh)-,y1");
		break;
		}
	case 1614: { // x:(r),a y:(rh)-,a
		unhandled("x:(r),a y:(rh)-,a");
		break;
		}
	case 1615: { // x:(r),a y:(rh)-,b
		unhandled("x:(r),a y:(rh)-,b");
		break;
		}
	case 1616: { // x:(r),b y:(rh)-,y0
		unhandled("x:(r),b y:(rh)-,y0");
		break;
		}
	case 1617: { // x:(r),b y:(rh)-,y1
		unhandled("x:(r),b y:(rh)-,y1");
		break;
		}
	case 1618: { // x:(r),b y:(rh)-,a
		unhandled("x:(r),b y:(rh)-,a");
		break;
		}
	case 1619: { // x:(r),b y:(rh)-,b
		unhandled("x:(r),b y:(rh)-,b");
		break;
		}
	case 1620: { // x:(r),x0 y:(rh)+,y0
		unhandled("x:(r),x0 y:(rh)+,y0");
		break;
		}
	case 1621: { // x:(r),x0 y:(rh)+,y1
		unhandled("x:(r),x0 y:(rh)+,y1");
		break;
		}
	case 1622: { // x:(r),x0 y:(rh)+,a
		unhandled("x:(r),x0 y:(rh)+,a");
		break;
		}
	case 1623: { // x:(r),x0 y:(rh)+,b
		unhandled("x:(r),x0 y:(rh)+,b");
		break;
		}
	case 1624: { // x:(r),x1 y:(rh)+,y0
		unhandled("x:(r),x1 y:(rh)+,y0");
		break;
		}
	case 1625: { // x:(r),x1 y:(rh)+,y1
		unhandled("x:(r),x1 y:(rh)+,y1");
		break;
		}
	case 1626: { // x:(r),x1 y:(rh)+,a
		unhandled("x:(r),x1 y:(rh)+,a");
		break;
		}
	case 1627: { // x:(r),x1 y:(rh)+,b
		unhandled("x:(r),x1 y:(rh)+,b");
		break;
		}
	case 1628: { // x:(r),a y:(rh)+,y0
		unhandled("x:(r),a y:(rh)+,y0");
		break;
		}
	case 1629: { // x:(r),a y:(rh)+,y1
		unhandled("x:(r),a y:(rh)+,y1");
		break;
		}
	case 1630: { // x:(r),a y:(rh)+,a
		unhandled("x:(r),a y:(rh)+,a");
		break;
		}
	case 1631: { // x:(r),a y:(rh)+,b
		unhandled("x:(r),a y:(rh)+,b");
		break;
		}
	case 1632: { // x:(r),b y:(rh)+,y0
		unhandled("x:(r),b y:(rh)+,y0");
		break;
		}
	case 1633: { // x:(r),b y:(rh)+,y1
		unhandled("x:(r),b y:(rh)+,y1");
		break;
		}
	case 1634: { // x:(r),b y:(rh)+,a
		unhandled("x:(r),b y:(rh)+,a");
		break;
		}
	case 1635: { // x:(r),b y:(rh)+,b
		unhandled("x:(r),b y:(rh)+,b");
		break;
		}
	case 1636: { // x:(r),x0 y:(rh),y0
		unhandled("x:(r),x0 y:(rh),y0");
		break;
		}
	case 1637: { // x:(r),x0 y:(rh),y1
		unhandled("x:(r),x0 y:(rh),y1");
		break;
		}
	case 1638: { // x:(r),x0 y:(rh),a
		unhandled("x:(r),x0 y:(rh),a");
		break;
		}
	case 1639: { // x:(r),x0 y:(rh),b
		unhandled("x:(r),x0 y:(rh),b");
		break;
		}
	case 1640: { // x:(r),x1 y:(rh),y0
		unhandled("x:(r),x1 y:(rh),y0");
		break;
		}
	case 1641: { // x:(r),x1 y:(rh),y1
		unhandled("x:(r),x1 y:(rh),y1");
		break;
		}
	case 1642: { // x:(r),x1 y:(rh),a
		unhandled("x:(r),x1 y:(rh),a");
		break;
		}
	case 1643: { // x:(r),x1 y:(rh),b
		unhandled("x:(r),x1 y:(rh),b");
		break;
		}
	case 1644: { // x:(r),a y:(rh),y0
		unhandled("x:(r),a y:(rh),y0");
		break;
		}
	case 1645: { // x:(r),a y:(rh),y1
		unhandled("x:(r),a y:(rh),y1");
		break;
		}
	case 1646: { // x:(r),a y:(rh),a
		unhandled("x:(r),a y:(rh),a");
		break;
		}
	case 1647: { // x:(r),a y:(rh),b
		unhandled("x:(r),a y:(rh),b");
		break;
		}
	case 1648: { // x:(r),b y:(rh),y0
		unhandled("x:(r),b y:(rh),y0");
		break;
		}
	case 1649: { // x:(r),b y:(rh),y1
		unhandled("x:(r),b y:(rh),y1");
		break;
		}
	case 1650: { // x:(r),b y:(rh),a
		unhandled("x:(r),b y:(rh),a");
		break;
		}
	case 1651: { // x:(r),b y:(rh),b
		unhandled("x:(r),b y:(rh),b");
		break;
		}
	case 1652: { // x:(r)+n,x0 y0,y:(rh)+n
		unhandled("x:(r)+n,x0 y0,y:(rh)+n");
		break;
		}
	case 1653: { // x:(r)+n,x0 y1,y:(rh)+n
		unhandled("x:(r)+n,x0 y1,y:(rh)+n");
		break;
		}
	case 1654: { // x:(r)+n,x0 a,y:(rh)+n
		unhandled("x:(r)+n,x0 a,y:(rh)+n");
		break;
		}
	case 1655: { // x:(r)+n,x0 b,y:(rh)+n
		unhandled("x:(r)+n,x0 b,y:(rh)+n");
		break;
		}
	case 1656: { // x:(r)+n,x1 y0,y:(rh)+n
		unhandled("x:(r)+n,x1 y0,y:(rh)+n");
		break;
		}
	case 1657: { // x:(r)+n,x1 y1,y:(rh)+n
		unhandled("x:(r)+n,x1 y1,y:(rh)+n");
		break;
		}
	case 1658: { // x:(r)+n,x1 a,y:(rh)+n
		unhandled("x:(r)+n,x1 a,y:(rh)+n");
		break;
		}
	case 1659: { // x:(r)+n,x1 b,y:(rh)+n
		unhandled("x:(r)+n,x1 b,y:(rh)+n");
		break;
		}
	case 1660: { // x:(r)+n,a y0,y:(rh)+n
		unhandled("x:(r)+n,a y0,y:(rh)+n");
		break;
		}
	case 1661: { // x:(r)+n,a y1,y:(rh)+n
		unhandled("x:(r)+n,a y1,y:(rh)+n");
		break;
		}
	case 1662: { // x:(r)+n,a a,y:(rh)+n
		unhandled("x:(r)+n,a a,y:(rh)+n");
		break;
		}
	case 1663: { // x:(r)+n,a b,y:(rh)+n
		unhandled("x:(r)+n,a b,y:(rh)+n");
		break;
		}
	case 1664: { // x:(r)+n,b y0,y:(rh)+n
		unhandled("x:(r)+n,b y0,y:(rh)+n");
		break;
		}
	case 1665: { // x:(r)+n,b y1,y:(rh)+n
		unhandled("x:(r)+n,b y1,y:(rh)+n");
		break;
		}
	case 1666: { // x:(r)+n,b a,y:(rh)+n
		unhandled("x:(r)+n,b a,y:(rh)+n");
		break;
		}
	case 1667: { // x:(r)+n,b b,y:(rh)+n
		unhandled("x:(r)+n,b b,y:(rh)+n");
		break;
		}
	case 1668: { // x:(r)+n,x0 y0,y:(rh)-
		unhandled("x:(r)+n,x0 y0,y:(rh)-");
		break;
		}
	case 1669: { // x:(r)+n,x0 y1,y:(rh)-
		unhandled("x:(r)+n,x0 y1,y:(rh)-");
		break;
		}
	case 1670: { // x:(r)+n,x0 a,y:(rh)-
		unhandled("x:(r)+n,x0 a,y:(rh)-");
		break;
		}
	case 1671: { // x:(r)+n,x0 b,y:(rh)-
		unhandled("x:(r)+n,x0 b,y:(rh)-");
		break;
		}
	case 1672: { // x:(r)+n,x1 y0,y:(rh)-
		unhandled("x:(r)+n,x1 y0,y:(rh)-");
		break;
		}
	case 1673: { // x:(r)+n,x1 y1,y:(rh)-
		unhandled("x:(r)+n,x1 y1,y:(rh)-");
		break;
		}
	case 1674: { // x:(r)+n,x1 a,y:(rh)-
		unhandled("x:(r)+n,x1 a,y:(rh)-");
		break;
		}
	case 1675: { // x:(r)+n,x1 b,y:(rh)-
		unhandled("x:(r)+n,x1 b,y:(rh)-");
		break;
		}
	case 1676: { // x:(r)+n,a y0,y:(rh)-
		unhandled("x:(r)+n,a y0,y:(rh)-");
		break;
		}
	case 1677: { // x:(r)+n,a y1,y:(rh)-
		unhandled("x:(r)+n,a y1,y:(rh)-");
		break;
		}
	case 1678: { // x:(r)+n,a a,y:(rh)-
		unhandled("x:(r)+n,a a,y:(rh)-");
		break;
		}
	case 1679: { // x:(r)+n,a b,y:(rh)-
		unhandled("x:(r)+n,a b,y:(rh)-");
		break;
		}
	case 1680: { // x:(r)+n,b y0,y:(rh)-
		unhandled("x:(r)+n,b y0,y:(rh)-");
		break;
		}
	case 1681: { // x:(r)+n,b y1,y:(rh)-
		unhandled("x:(r)+n,b y1,y:(rh)-");
		break;
		}
	case 1682: { // x:(r)+n,b a,y:(rh)-
		unhandled("x:(r)+n,b a,y:(rh)-");
		break;
		}
	case 1683: { // x:(r)+n,b b,y:(rh)-
		unhandled("x:(r)+n,b b,y:(rh)-");
		break;
		}
	case 1684: { // x:(r)+n,x0 y0,y:(rh)+
		unhandled("x:(r)+n,x0 y0,y:(rh)+");
		break;
		}
	case 1685: { // x:(r)+n,x0 y1,y:(rh)+
		unhandled("x:(r)+n,x0 y1,y:(rh)+");
		break;
		}
	case 1686: { // x:(r)+n,x0 a,y:(rh)+
		unhandled("x:(r)+n,x0 a,y:(rh)+");
		break;
		}
	case 1687: { // x:(r)+n,x0 b,y:(rh)+
		unhandled("x:(r)+n,x0 b,y:(rh)+");
		break;
		}
	case 1688: { // x:(r)+n,x1 y0,y:(rh)+
		unhandled("x:(r)+n,x1 y0,y:(rh)+");
		break;
		}
	case 1689: { // x:(r)+n,x1 y1,y:(rh)+
		unhandled("x:(r)+n,x1 y1,y:(rh)+");
		break;
		}
	case 1690: { // x:(r)+n,x1 a,y:(rh)+
		unhandled("x:(r)+n,x1 a,y:(rh)+");
		break;
		}
	case 1691: { // x:(r)+n,x1 b,y:(rh)+
		unhandled("x:(r)+n,x1 b,y:(rh)+");
		break;
		}
	case 1692: { // x:(r)+n,a y0,y:(rh)+
		unhandled("x:(r)+n,a y0,y:(rh)+");
		break;
		}
	case 1693: { // x:(r)+n,a y1,y:(rh)+
		unhandled("x:(r)+n,a y1,y:(rh)+");
		break;
		}
	case 1694: { // x:(r)+n,a a,y:(rh)+
		unhandled("x:(r)+n,a a,y:(rh)+");
		break;
		}
	case 1695: { // x:(r)+n,a b,y:(rh)+
		unhandled("x:(r)+n,a b,y:(rh)+");
		break;
		}
	case 1696: { // x:(r)+n,b y0,y:(rh)+
		unhandled("x:(r)+n,b y0,y:(rh)+");
		break;
		}
	case 1697: { // x:(r)+n,b y1,y:(rh)+
		unhandled("x:(r)+n,b y1,y:(rh)+");
		break;
		}
	case 1698: { // x:(r)+n,b a,y:(rh)+
		unhandled("x:(r)+n,b a,y:(rh)+");
		break;
		}
	case 1699: { // x:(r)+n,b b,y:(rh)+
		unhandled("x:(r)+n,b b,y:(rh)+");
		break;
		}
	case 1700: { // x:(r)+n,x0 y0,y:(rh)
		unhandled("x:(r)+n,x0 y0,y:(rh)");
		break;
		}
	case 1701: { // x:(r)+n,x0 y1,y:(rh)
		unhandled("x:(r)+n,x0 y1,y:(rh)");
		break;
		}
	case 1702: { // x:(r)+n,x0 a,y:(rh)
		unhandled("x:(r)+n,x0 a,y:(rh)");
		break;
		}
	case 1703: { // x:(r)+n,x0 b,y:(rh)
		unhandled("x:(r)+n,x0 b,y:(rh)");
		break;
		}
	case 1704: { // x:(r)+n,x1 y0,y:(rh)
		unhandled("x:(r)+n,x1 y0,y:(rh)");
		break;
		}
	case 1705: { // x:(r)+n,x1 y1,y:(rh)
		unhandled("x:(r)+n,x1 y1,y:(rh)");
		break;
		}
	case 1706: { // x:(r)+n,x1 a,y:(rh)
		unhandled("x:(r)+n,x1 a,y:(rh)");
		break;
		}
	case 1707: { // x:(r)+n,x1 b,y:(rh)
		unhandled("x:(r)+n,x1 b,y:(rh)");
		break;
		}
	case 1708: { // x:(r)+n,a y0,y:(rh)
		unhandled("x:(r)+n,a y0,y:(rh)");
		break;
		}
	case 1709: { // x:(r)+n,a y1,y:(rh)
		unhandled("x:(r)+n,a y1,y:(rh)");
		break;
		}
	case 1710: { // x:(r)+n,a a,y:(rh)
		unhandled("x:(r)+n,a a,y:(rh)");
		break;
		}
	case 1711: { // x:(r)+n,a b,y:(rh)
		unhandled("x:(r)+n,a b,y:(rh)");
		break;
		}
	case 1712: { // x:(r)+n,b y0,y:(rh)
		unhandled("x:(r)+n,b y0,y:(rh)");
		break;
		}
	case 1713: { // x:(r)+n,b y1,y:(rh)
		unhandled("x:(r)+n,b y1,y:(rh)");
		break;
		}
	case 1714: { // x:(r)+n,b a,y:(rh)
		unhandled("x:(r)+n,b a,y:(rh)");
		break;
		}
	case 1715: { // x:(r)+n,b b,y:(rh)
		unhandled("x:(r)+n,b b,y:(rh)");
		break;
		}
	case 1716: { // x:(r)-,x0 y0,y:(rh)+n
		unhandled("x:(r)-,x0 y0,y:(rh)+n");
		break;
		}
	case 1717: { // x:(r)-,x0 y1,y:(rh)+n
		unhandled("x:(r)-,x0 y1,y:(rh)+n");
		break;
		}
	case 1718: { // x:(r)-,x0 a,y:(rh)+n
		unhandled("x:(r)-,x0 a,y:(rh)+n");
		break;
		}
	case 1719: { // x:(r)-,x0 b,y:(rh)+n
		unhandled("x:(r)-,x0 b,y:(rh)+n");
		break;
		}
	case 1720: { // x:(r)-,x1 y0,y:(rh)+n
		unhandled("x:(r)-,x1 y0,y:(rh)+n");
		break;
		}
	case 1721: { // x:(r)-,x1 y1,y:(rh)+n
		unhandled("x:(r)-,x1 y1,y:(rh)+n");
		break;
		}
	case 1722: { // x:(r)-,x1 a,y:(rh)+n
		unhandled("x:(r)-,x1 a,y:(rh)+n");
		break;
		}
	case 1723: { // x:(r)-,x1 b,y:(rh)+n
		unhandled("x:(r)-,x1 b,y:(rh)+n");
		break;
		}
	case 1724: { // x:(r)-,a y0,y:(rh)+n
		unhandled("x:(r)-,a y0,y:(rh)+n");
		break;
		}
	case 1725: { // x:(r)-,a y1,y:(rh)+n
		unhandled("x:(r)-,a y1,y:(rh)+n");
		break;
		}
	case 1726: { // x:(r)-,a a,y:(rh)+n
		unhandled("x:(r)-,a a,y:(rh)+n");
		break;
		}
	case 1727: { // x:(r)-,a b,y:(rh)+n
		unhandled("x:(r)-,a b,y:(rh)+n");
		break;
		}
	case 1728: { // x:(r)-,b y0,y:(rh)+n
		unhandled("x:(r)-,b y0,y:(rh)+n");
		break;
		}
	case 1729: { // x:(r)-,b y1,y:(rh)+n
		unhandled("x:(r)-,b y1,y:(rh)+n");
		break;
		}
	case 1730: { // x:(r)-,b a,y:(rh)+n
		unhandled("x:(r)-,b a,y:(rh)+n");
		break;
		}
	case 1731: { // x:(r)-,b b,y:(rh)+n
		unhandled("x:(r)-,b b,y:(rh)+n");
		break;
		}
	case 1732: { // x:(r)-,x0 y0,y:(rh)-
		unhandled("x:(r)-,x0 y0,y:(rh)-");
		break;
		}
	case 1733: { // x:(r)-,x0 y1,y:(rh)-
		unhandled("x:(r)-,x0 y1,y:(rh)-");
		break;
		}
	case 1734: { // x:(r)-,x0 a,y:(rh)-
		unhandled("x:(r)-,x0 a,y:(rh)-");
		break;
		}
	case 1735: { // x:(r)-,x0 b,y:(rh)-
		unhandled("x:(r)-,x0 b,y:(rh)-");
		break;
		}
	case 1736: { // x:(r)-,x1 y0,y:(rh)-
		unhandled("x:(r)-,x1 y0,y:(rh)-");
		break;
		}
	case 1737: { // x:(r)-,x1 y1,y:(rh)-
		unhandled("x:(r)-,x1 y1,y:(rh)-");
		break;
		}
	case 1738: { // x:(r)-,x1 a,y:(rh)-
		unhandled("x:(r)-,x1 a,y:(rh)-");
		break;
		}
	case 1739: { // x:(r)-,x1 b,y:(rh)-
		unhandled("x:(r)-,x1 b,y:(rh)-");
		break;
		}
	case 1740: { // x:(r)-,a y0,y:(rh)-
		unhandled("x:(r)-,a y0,y:(rh)-");
		break;
		}
	case 1741: { // x:(r)-,a y1,y:(rh)-
		unhandled("x:(r)-,a y1,y:(rh)-");
		break;
		}
	case 1742: { // x:(r)-,a a,y:(rh)-
		unhandled("x:(r)-,a a,y:(rh)-");
		break;
		}
	case 1743: { // x:(r)-,a b,y:(rh)-
		unhandled("x:(r)-,a b,y:(rh)-");
		break;
		}
	case 1744: { // x:(r)-,b y0,y:(rh)-
		unhandled("x:(r)-,b y0,y:(rh)-");
		break;
		}
	case 1745: { // x:(r)-,b y1,y:(rh)-
		unhandled("x:(r)-,b y1,y:(rh)-");
		break;
		}
	case 1746: { // x:(r)-,b a,y:(rh)-
		unhandled("x:(r)-,b a,y:(rh)-");
		break;
		}
	case 1747: { // x:(r)-,b b,y:(rh)-
		unhandled("x:(r)-,b b,y:(rh)-");
		break;
		}
	case 1748: { // x:(r)-,x0 y0,y:(rh)+
		unhandled("x:(r)-,x0 y0,y:(rh)+");
		break;
		}
	case 1749: { // x:(r)-,x0 y1,y:(rh)+
		unhandled("x:(r)-,x0 y1,y:(rh)+");
		break;
		}
	case 1750: { // x:(r)-,x0 a,y:(rh)+
		unhandled("x:(r)-,x0 a,y:(rh)+");
		break;
		}
	case 1751: { // x:(r)-,x0 b,y:(rh)+
		unhandled("x:(r)-,x0 b,y:(rh)+");
		break;
		}
	case 1752: { // x:(r)-,x1 y0,y:(rh)+
		unhandled("x:(r)-,x1 y0,y:(rh)+");
		break;
		}
	case 1753: { // x:(r)-,x1 y1,y:(rh)+
		unhandled("x:(r)-,x1 y1,y:(rh)+");
		break;
		}
	case 1754: { // x:(r)-,x1 a,y:(rh)+
		unhandled("x:(r)-,x1 a,y:(rh)+");
		break;
		}
	case 1755: { // x:(r)-,x1 b,y:(rh)+
		unhandled("x:(r)-,x1 b,y:(rh)+");
		break;
		}
	case 1756: { // x:(r)-,a y0,y:(rh)+
		unhandled("x:(r)-,a y0,y:(rh)+");
		break;
		}
	case 1757: { // x:(r)-,a y1,y:(rh)+
		unhandled("x:(r)-,a y1,y:(rh)+");
		break;
		}
	case 1758: { // x:(r)-,a a,y:(rh)+
		unhandled("x:(r)-,a a,y:(rh)+");
		break;
		}
	case 1759: { // x:(r)-,a b,y:(rh)+
		unhandled("x:(r)-,a b,y:(rh)+");
		break;
		}
	case 1760: { // x:(r)-,b y0,y:(rh)+
		unhandled("x:(r)-,b y0,y:(rh)+");
		break;
		}
	case 1761: { // x:(r)-,b y1,y:(rh)+
		unhandled("x:(r)-,b y1,y:(rh)+");
		break;
		}
	case 1762: { // x:(r)-,b a,y:(rh)+
		unhandled("x:(r)-,b a,y:(rh)+");
		break;
		}
	case 1763: { // x:(r)-,b b,y:(rh)+
		unhandled("x:(r)-,b b,y:(rh)+");
		break;
		}
	case 1764: { // x:(r)-,x0 y0,y:(rh)
		unhandled("x:(r)-,x0 y0,y:(rh)");
		break;
		}
	case 1765: { // x:(r)-,x0 y1,y:(rh)
		unhandled("x:(r)-,x0 y1,y:(rh)");
		break;
		}
	case 1766: { // x:(r)-,x0 a,y:(rh)
		unhandled("x:(r)-,x0 a,y:(rh)");
		break;
		}
	case 1767: { // x:(r)-,x0 b,y:(rh)
		unhandled("x:(r)-,x0 b,y:(rh)");
		break;
		}
	case 1768: { // x:(r)-,x1 y0,y:(rh)
		unhandled("x:(r)-,x1 y0,y:(rh)");
		break;
		}
	case 1769: { // x:(r)-,x1 y1,y:(rh)
		unhandled("x:(r)-,x1 y1,y:(rh)");
		break;
		}
	case 1770: { // x:(r)-,x1 a,y:(rh)
		unhandled("x:(r)-,x1 a,y:(rh)");
		break;
		}
	case 1771: { // x:(r)-,x1 b,y:(rh)
		unhandled("x:(r)-,x1 b,y:(rh)");
		break;
		}
	case 1772: { // x:(r)-,a y0,y:(rh)
		unhandled("x:(r)-,a y0,y:(rh)");
		break;
		}
	case 1773: { // x:(r)-,a y1,y:(rh)
		unhandled("x:(r)-,a y1,y:(rh)");
		break;
		}
	case 1774: { // x:(r)-,a a,y:(rh)
		unhandled("x:(r)-,a a,y:(rh)");
		break;
		}
	case 1775: { // x:(r)-,a b,y:(rh)
		unhandled("x:(r)-,a b,y:(rh)");
		break;
		}
	case 1776: { // x:(r)-,b y0,y:(rh)
		unhandled("x:(r)-,b y0,y:(rh)");
		break;
		}
	case 1777: { // x:(r)-,b y1,y:(rh)
		unhandled("x:(r)-,b y1,y:(rh)");
		break;
		}
	case 1778: { // x:(r)-,b a,y:(rh)
		unhandled("x:(r)-,b a,y:(rh)");
		break;
		}
	case 1779: { // x:(r)-,b b,y:(rh)
		unhandled("x:(r)-,b b,y:(rh)");
		break;
		}
	case 1780: { // x:(r)+,x0 y0,y:(rh)+n
		unhandled("x:(r)+,x0 y0,y:(rh)+n");
		break;
		}
	case 1781: { // x:(r)+,x0 y1,y:(rh)+n
		unhandled("x:(r)+,x0 y1,y:(rh)+n");
		break;
		}
	case 1782: { // x:(r)+,x0 a,y:(rh)+n
		unhandled("x:(r)+,x0 a,y:(rh)+n");
		break;
		}
	case 1783: { // x:(r)+,x0 b,y:(rh)+n
		unhandled("x:(r)+,x0 b,y:(rh)+n");
		break;
		}
	case 1784: { // x:(r)+,x1 y0,y:(rh)+n
		unhandled("x:(r)+,x1 y0,y:(rh)+n");
		break;
		}
	case 1785: { // x:(r)+,x1 y1,y:(rh)+n
		unhandled("x:(r)+,x1 y1,y:(rh)+n");
		break;
		}
	case 1786: { // x:(r)+,x1 a,y:(rh)+n
		unhandled("x:(r)+,x1 a,y:(rh)+n");
		break;
		}
	case 1787: { // x:(r)+,x1 b,y:(rh)+n
		unhandled("x:(r)+,x1 b,y:(rh)+n");
		break;
		}
	case 1788: { // x:(r)+,a y0,y:(rh)+n
		unhandled("x:(r)+,a y0,y:(rh)+n");
		break;
		}
	case 1789: { // x:(r)+,a y1,y:(rh)+n
		unhandled("x:(r)+,a y1,y:(rh)+n");
		break;
		}
	case 1790: { // x:(r)+,a a,y:(rh)+n
		unhandled("x:(r)+,a a,y:(rh)+n");
		break;
		}
	case 1791: { // x:(r)+,a b,y:(rh)+n
		unhandled("x:(r)+,a b,y:(rh)+n");
		break;
		}
	case 1792: { // x:(r)+,b y0,y:(rh)+n
		unhandled("x:(r)+,b y0,y:(rh)+n");
		break;
		}
	case 1793: { // x:(r)+,b y1,y:(rh)+n
		unhandled("x:(r)+,b y1,y:(rh)+n");
		break;
		}
	case 1794: { // x:(r)+,b a,y:(rh)+n
		unhandled("x:(r)+,b a,y:(rh)+n");
		break;
		}
	case 1795: { // x:(r)+,b b,y:(rh)+n
		unhandled("x:(r)+,b b,y:(rh)+n");
		break;
		}
	case 1796: { // x:(r)+,x0 y0,y:(rh)-
		unhandled("x:(r)+,x0 y0,y:(rh)-");
		break;
		}
	case 1797: { // x:(r)+,x0 y1,y:(rh)-
		unhandled("x:(r)+,x0 y1,y:(rh)-");
		break;
		}
	case 1798: { // x:(r)+,x0 a,y:(rh)-
		unhandled("x:(r)+,x0 a,y:(rh)-");
		break;
		}
	case 1799: { // x:(r)+,x0 b,y:(rh)-
		unhandled("x:(r)+,x0 b,y:(rh)-");
		break;
		}
	case 1800: { // x:(r)+,x1 y0,y:(rh)-
		unhandled("x:(r)+,x1 y0,y:(rh)-");
		break;
		}
	case 1801: { // x:(r)+,x1 y1,y:(rh)-
		unhandled("x:(r)+,x1 y1,y:(rh)-");
		break;
		}
	case 1802: { // x:(r)+,x1 a,y:(rh)-
		unhandled("x:(r)+,x1 a,y:(rh)-");
		break;
		}
	case 1803: { // x:(r)+,x1 b,y:(rh)-
		unhandled("x:(r)+,x1 b,y:(rh)-");
		break;
		}
	case 1804: { // x:(r)+,a y0,y:(rh)-
		unhandled("x:(r)+,a y0,y:(rh)-");
		break;
		}
	case 1805: { // x:(r)+,a y1,y:(rh)-
		unhandled("x:(r)+,a y1,y:(rh)-");
		break;
		}
	case 1806: { // x:(r)+,a a,y:(rh)-
		unhandled("x:(r)+,a a,y:(rh)-");
		break;
		}
	case 1807: { // x:(r)+,a b,y:(rh)-
		unhandled("x:(r)+,a b,y:(rh)-");
		break;
		}
	case 1808: { // x:(r)+,b y0,y:(rh)-
		unhandled("x:(r)+,b y0,y:(rh)-");
		break;
		}
	case 1809: { // x:(r)+,b y1,y:(rh)-
		unhandled("x:(r)+,b y1,y:(rh)-");
		break;
		}
	case 1810: { // x:(r)+,b a,y:(rh)-
		unhandled("x:(r)+,b a,y:(rh)-");
		break;
		}
	case 1811: { // x:(r)+,b b,y:(rh)-
		unhandled("x:(r)+,b b,y:(rh)-");
		break;
		}
	case 1812: { // x:(r)+,x0 y0,y:(rh)+
		unhandled("x:(r)+,x0 y0,y:(rh)+");
		break;
		}
	case 1813: { // x:(r)+,x0 y1,y:(rh)+
		unhandled("x:(r)+,x0 y1,y:(rh)+");
		break;
		}
	case 1814: { // x:(r)+,x0 a,y:(rh)+
		unhandled("x:(r)+,x0 a,y:(rh)+");
		break;
		}
	case 1815: { // x:(r)+,x0 b,y:(rh)+
		unhandled("x:(r)+,x0 b,y:(rh)+");
		break;
		}
	case 1816: { // x:(r)+,x1 y0,y:(rh)+
		unhandled("x:(r)+,x1 y0,y:(rh)+");
		break;
		}
	case 1817: { // x:(r)+,x1 y1,y:(rh)+
		unhandled("x:(r)+,x1 y1,y:(rh)+");
		break;
		}
	case 1818: { // x:(r)+,x1 a,y:(rh)+
		unhandled("x:(r)+,x1 a,y:(rh)+");
		break;
		}
	case 1819: { // x:(r)+,x1 b,y:(rh)+
		unhandled("x:(r)+,x1 b,y:(rh)+");
		break;
		}
	case 1820: { // x:(r)+,a y0,y:(rh)+
		unhandled("x:(r)+,a y0,y:(rh)+");
		break;
		}
	case 1821: { // x:(r)+,a y1,y:(rh)+
		unhandled("x:(r)+,a y1,y:(rh)+");
		break;
		}
	case 1822: { // x:(r)+,a a,y:(rh)+
		unhandled("x:(r)+,a a,y:(rh)+");
		break;
		}
	case 1823: { // x:(r)+,a b,y:(rh)+
		unhandled("x:(r)+,a b,y:(rh)+");
		break;
		}
	case 1824: { // x:(r)+,b y0,y:(rh)+
		unhandled("x:(r)+,b y0,y:(rh)+");
		break;
		}
	case 1825: { // x:(r)+,b y1,y:(rh)+
		unhandled("x:(r)+,b y1,y:(rh)+");
		break;
		}
	case 1826: { // x:(r)+,b a,y:(rh)+
		unhandled("x:(r)+,b a,y:(rh)+");
		break;
		}
	case 1827: { // x:(r)+,b b,y:(rh)+
		unhandled("x:(r)+,b b,y:(rh)+");
		break;
		}
	case 1828: { // x:(r)+,x0 y0,y:(rh)
		unhandled("x:(r)+,x0 y0,y:(rh)");
		break;
		}
	case 1829: { // x:(r)+,x0 y1,y:(rh)
		unhandled("x:(r)+,x0 y1,y:(rh)");
		break;
		}
	case 1830: { // x:(r)+,x0 a,y:(rh)
		unhandled("x:(r)+,x0 a,y:(rh)");
		break;
		}
	case 1831: { // x:(r)+,x0 b,y:(rh)
		unhandled("x:(r)+,x0 b,y:(rh)");
		break;
		}
	case 1832: { // x:(r)+,x1 y0,y:(rh)
		unhandled("x:(r)+,x1 y0,y:(rh)");
		break;
		}
	case 1833: { // x:(r)+,x1 y1,y:(rh)
		unhandled("x:(r)+,x1 y1,y:(rh)");
		break;
		}
	case 1834: { // x:(r)+,x1 a,y:(rh)
		unhandled("x:(r)+,x1 a,y:(rh)");
		break;
		}
	case 1835: { // x:(r)+,x1 b,y:(rh)
		unhandled("x:(r)+,x1 b,y:(rh)");
		break;
		}
	case 1836: { // x:(r)+,a y0,y:(rh)
		unhandled("x:(r)+,a y0,y:(rh)");
		break;
		}
	case 1837: { // x:(r)+,a y1,y:(rh)
		unhandled("x:(r)+,a y1,y:(rh)");
		break;
		}
	case 1838: { // x:(r)+,a a,y:(rh)
		unhandled("x:(r)+,a a,y:(rh)");
		break;
		}
	case 1839: { // x:(r)+,a b,y:(rh)
		unhandled("x:(r)+,a b,y:(rh)");
		break;
		}
	case 1840: { // x:(r)+,b y0,y:(rh)
		unhandled("x:(r)+,b y0,y:(rh)");
		break;
		}
	case 1841: { // x:(r)+,b y1,y:(rh)
		unhandled("x:(r)+,b y1,y:(rh)");
		break;
		}
	case 1842: { // x:(r)+,b a,y:(rh)
		unhandled("x:(r)+,b a,y:(rh)");
		break;
		}
	case 1843: { // x:(r)+,b b,y:(rh)
		unhandled("x:(r)+,b b,y:(rh)");
		break;
		}
	case 1844: { // x:(r),x0 y0,y:(rh)+n
		unhandled("x:(r),x0 y0,y:(rh)+n");
		break;
		}
	case 1845: { // x:(r),x0 y1,y:(rh)+n
		unhandled("x:(r),x0 y1,y:(rh)+n");
		break;
		}
	case 1846: { // x:(r),x0 a,y:(rh)+n
		unhandled("x:(r),x0 a,y:(rh)+n");
		break;
		}
	case 1847: { // x:(r),x0 b,y:(rh)+n
		unhandled("x:(r),x0 b,y:(rh)+n");
		break;
		}
	case 1848: { // x:(r),x1 y0,y:(rh)+n
		unhandled("x:(r),x1 y0,y:(rh)+n");
		break;
		}
	case 1849: { // x:(r),x1 y1,y:(rh)+n
		unhandled("x:(r),x1 y1,y:(rh)+n");
		break;
		}
	case 1850: { // x:(r),x1 a,y:(rh)+n
		unhandled("x:(r),x1 a,y:(rh)+n");
		break;
		}
	case 1851: { // x:(r),x1 b,y:(rh)+n
		unhandled("x:(r),x1 b,y:(rh)+n");
		break;
		}
	case 1852: { // x:(r),a y0,y:(rh)+n
		unhandled("x:(r),a y0,y:(rh)+n");
		break;
		}
	case 1853: { // x:(r),a y1,y:(rh)+n
		unhandled("x:(r),a y1,y:(rh)+n");
		break;
		}
	case 1854: { // x:(r),a a,y:(rh)+n
		unhandled("x:(r),a a,y:(rh)+n");
		break;
		}
	case 1855: { // x:(r),a b,y:(rh)+n
		unhandled("x:(r),a b,y:(rh)+n");
		break;
		}
	case 1856: { // x:(r),b y0,y:(rh)+n
		unhandled("x:(r),b y0,y:(rh)+n");
		break;
		}
	case 1857: { // x:(r),b y1,y:(rh)+n
		unhandled("x:(r),b y1,y:(rh)+n");
		break;
		}
	case 1858: { // x:(r),b a,y:(rh)+n
		unhandled("x:(r),b a,y:(rh)+n");
		break;
		}
	case 1859: { // x:(r),b b,y:(rh)+n
		unhandled("x:(r),b b,y:(rh)+n");
		break;
		}
	case 1860: { // x:(r),x0 y0,y:(rh)-
		unhandled("x:(r),x0 y0,y:(rh)-");
		break;
		}
	case 1861: { // x:(r),x0 y1,y:(rh)-
		unhandled("x:(r),x0 y1,y:(rh)-");
		break;
		}
	case 1862: { // x:(r),x0 a,y:(rh)-
		unhandled("x:(r),x0 a,y:(rh)-");
		break;
		}
	case 1863: { // x:(r),x0 b,y:(rh)-
		unhandled("x:(r),x0 b,y:(rh)-");
		break;
		}
	case 1864: { // x:(r),x1 y0,y:(rh)-
		unhandled("x:(r),x1 y0,y:(rh)-");
		break;
		}
	case 1865: { // x:(r),x1 y1,y:(rh)-
		unhandled("x:(r),x1 y1,y:(rh)-");
		break;
		}
	case 1866: { // x:(r),x1 a,y:(rh)-
		unhandled("x:(r),x1 a,y:(rh)-");
		break;
		}
	case 1867: { // x:(r),x1 b,y:(rh)-
		unhandled("x:(r),x1 b,y:(rh)-");
		break;
		}
	case 1868: { // x:(r),a y0,y:(rh)-
		unhandled("x:(r),a y0,y:(rh)-");
		break;
		}
	case 1869: { // x:(r),a y1,y:(rh)-
		unhandled("x:(r),a y1,y:(rh)-");
		break;
		}
	case 1870: { // x:(r),a a,y:(rh)-
		unhandled("x:(r),a a,y:(rh)-");
		break;
		}
	case 1871: { // x:(r),a b,y:(rh)-
		unhandled("x:(r),a b,y:(rh)-");
		break;
		}
	case 1872: { // x:(r),b y0,y:(rh)-
		unhandled("x:(r),b y0,y:(rh)-");
		break;
		}
	case 1873: { // x:(r),b y1,y:(rh)-
		unhandled("x:(r),b y1,y:(rh)-");
		break;
		}
	case 1874: { // x:(r),b a,y:(rh)-
		unhandled("x:(r),b a,y:(rh)-");
		break;
		}
	case 1875: { // x:(r),b b,y:(rh)-
		unhandled("x:(r),b b,y:(rh)-");
		break;
		}
	case 1876: { // x:(r),x0 y0,y:(rh)+
		unhandled("x:(r),x0 y0,y:(rh)+");
		break;
		}
	case 1877: { // x:(r),x0 y1,y:(rh)+
		unhandled("x:(r),x0 y1,y:(rh)+");
		break;
		}
	case 1878: { // x:(r),x0 a,y:(rh)+
		unhandled("x:(r),x0 a,y:(rh)+");
		break;
		}
	case 1879: { // x:(r),x0 b,y:(rh)+
		unhandled("x:(r),x0 b,y:(rh)+");
		break;
		}
	case 1880: { // x:(r),x1 y0,y:(rh)+
		unhandled("x:(r),x1 y0,y:(rh)+");
		break;
		}
	case 1881: { // x:(r),x1 y1,y:(rh)+
		unhandled("x:(r),x1 y1,y:(rh)+");
		break;
		}
	case 1882: { // x:(r),x1 a,y:(rh)+
		unhandled("x:(r),x1 a,y:(rh)+");
		break;
		}
	case 1883: { // x:(r),x1 b,y:(rh)+
		unhandled("x:(r),x1 b,y:(rh)+");
		break;
		}
	case 1884: { // x:(r),a y0,y:(rh)+
		unhandled("x:(r),a y0,y:(rh)+");
		break;
		}
	case 1885: { // x:(r),a y1,y:(rh)+
		unhandled("x:(r),a y1,y:(rh)+");
		break;
		}
	case 1886: { // x:(r),a a,y:(rh)+
		unhandled("x:(r),a a,y:(rh)+");
		break;
		}
	case 1887: { // x:(r),a b,y:(rh)+
		unhandled("x:(r),a b,y:(rh)+");
		break;
		}
	case 1888: { // x:(r),b y0,y:(rh)+
		unhandled("x:(r),b y0,y:(rh)+");
		break;
		}
	case 1889: { // x:(r),b y1,y:(rh)+
		unhandled("x:(r),b y1,y:(rh)+");
		break;
		}
	case 1890: { // x:(r),b a,y:(rh)+
		unhandled("x:(r),b a,y:(rh)+");
		break;
		}
	case 1891: { // x:(r),b b,y:(rh)+
		unhandled("x:(r),b b,y:(rh)+");
		break;
		}
	case 1892: { // x:(r),x0 y0,y:(rh)
		unhandled("x:(r),x0 y0,y:(rh)");
		break;
		}
	case 1893: { // x:(r),x0 y1,y:(rh)
		unhandled("x:(r),x0 y1,y:(rh)");
		break;
		}
	case 1894: { // x:(r),x0 a,y:(rh)
		unhandled("x:(r),x0 a,y:(rh)");
		break;
		}
	case 1895: { // x:(r),x0 b,y:(rh)
		unhandled("x:(r),x0 b,y:(rh)");
		break;
		}
	case 1896: { // x:(r),x1 y0,y:(rh)
		unhandled("x:(r),x1 y0,y:(rh)");
		break;
		}
	case 1897: { // x:(r),x1 y1,y:(rh)
		unhandled("x:(r),x1 y1,y:(rh)");
		break;
		}
	case 1898: { // x:(r),x1 a,y:(rh)
		unhandled("x:(r),x1 a,y:(rh)");
		break;
		}
	case 1899: { // x:(r),x1 b,y:(rh)
		unhandled("x:(r),x1 b,y:(rh)");
		break;
		}
	case 1900: { // x:(r),a y0,y:(rh)
		unhandled("x:(r),a y0,y:(rh)");
		break;
		}
	case 1901: { // x:(r),a y1,y:(rh)
		unhandled("x:(r),a y1,y:(rh)");
		break;
		}
	case 1902: { // x:(r),a a,y:(rh)
		unhandled("x:(r),a a,y:(rh)");
		break;
		}
	case 1903: { // x:(r),a b,y:(rh)
		unhandled("x:(r),a b,y:(rh)");
		break;
		}
	case 1904: { // x:(r),b y0,y:(rh)
		unhandled("x:(r),b y0,y:(rh)");
		break;
		}
	case 1905: { // x:(r),b y1,y:(rh)
		unhandled("x:(r),b y1,y:(rh)");
		break;
		}
	case 1906: { // x:(r),b a,y:(rh)
		unhandled("x:(r),b a,y:(rh)");
		break;
		}
	case 1907: { // x:(r),b b,y:(rh)
		unhandled("x:(r),b b,y:(rh)");
		break;
		}
	case 1908: { // x0,x:(r)+n y:(rh)+n,y0
		unhandled("x0,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1909: { // x0,x:(r)+n y:(rh)+n,y1
		unhandled("x0,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1910: { // x0,x:(r)+n y:(rh)+n,a
		unhandled("x0,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1911: { // x0,x:(r)+n y:(rh)+n,b
		unhandled("x0,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1912: { // x1,x:(r)+n y:(rh)+n,y0
		unhandled("x1,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1913: { // x1,x:(r)+n y:(rh)+n,y1
		unhandled("x1,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1914: { // x1,x:(r)+n y:(rh)+n,a
		unhandled("x1,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1915: { // x1,x:(r)+n y:(rh)+n,b
		unhandled("x1,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1916: { // a,x:(r)+n y:(rh)+n,y0
		unhandled("a,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1917: { // a,x:(r)+n y:(rh)+n,y1
		unhandled("a,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1918: { // a,x:(r)+n y:(rh)+n,a
		unhandled("a,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1919: { // a,x:(r)+n y:(rh)+n,b
		unhandled("a,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1920: { // b,x:(r)+n y:(rh)+n,y0
		unhandled("b,x:(r)+n y:(rh)+n,y0");
		break;
		}
	case 1921: { // b,x:(r)+n y:(rh)+n,y1
		unhandled("b,x:(r)+n y:(rh)+n,y1");
		break;
		}
	case 1922: { // b,x:(r)+n y:(rh)+n,a
		unhandled("b,x:(r)+n y:(rh)+n,a");
		break;
		}
	case 1923: { // b,x:(r)+n y:(rh)+n,b
		unhandled("b,x:(r)+n y:(rh)+n,b");
		break;
		}
	case 1924: { // x0,x:(r)+n y:(rh)-,y0
		unhandled("x0,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1925: { // x0,x:(r)+n y:(rh)-,y1
		unhandled("x0,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1926: { // x0,x:(r)+n y:(rh)-,a
		unhandled("x0,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1927: { // x0,x:(r)+n y:(rh)-,b
		unhandled("x0,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1928: { // x1,x:(r)+n y:(rh)-,y0
		unhandled("x1,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1929: { // x1,x:(r)+n y:(rh)-,y1
		unhandled("x1,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1930: { // x1,x:(r)+n y:(rh)-,a
		unhandled("x1,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1931: { // x1,x:(r)+n y:(rh)-,b
		unhandled("x1,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1932: { // a,x:(r)+n y:(rh)-,y0
		unhandled("a,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1933: { // a,x:(r)+n y:(rh)-,y1
		unhandled("a,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1934: { // a,x:(r)+n y:(rh)-,a
		unhandled("a,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1935: { // a,x:(r)+n y:(rh)-,b
		unhandled("a,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1936: { // b,x:(r)+n y:(rh)-,y0
		unhandled("b,x:(r)+n y:(rh)-,y0");
		break;
		}
	case 1937: { // b,x:(r)+n y:(rh)-,y1
		unhandled("b,x:(r)+n y:(rh)-,y1");
		break;
		}
	case 1938: { // b,x:(r)+n y:(rh)-,a
		unhandled("b,x:(r)+n y:(rh)-,a");
		break;
		}
	case 1939: { // b,x:(r)+n y:(rh)-,b
		unhandled("b,x:(r)+n y:(rh)-,b");
		break;
		}
	case 1940: { // x0,x:(r)+n y:(rh)+,y0
		unhandled("x0,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1941: { // x0,x:(r)+n y:(rh)+,y1
		unhandled("x0,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1942: { // x0,x:(r)+n y:(rh)+,a
		unhandled("x0,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1943: { // x0,x:(r)+n y:(rh)+,b
		unhandled("x0,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1944: { // x1,x:(r)+n y:(rh)+,y0
		unhandled("x1,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1945: { // x1,x:(r)+n y:(rh)+,y1
		unhandled("x1,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1946: { // x1,x:(r)+n y:(rh)+,a
		unhandled("x1,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1947: { // x1,x:(r)+n y:(rh)+,b
		unhandled("x1,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1948: { // a,x:(r)+n y:(rh)+,y0
		unhandled("a,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1949: { // a,x:(r)+n y:(rh)+,y1
		unhandled("a,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1950: { // a,x:(r)+n y:(rh)+,a
		unhandled("a,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1951: { // a,x:(r)+n y:(rh)+,b
		unhandled("a,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1952: { // b,x:(r)+n y:(rh)+,y0
		unhandled("b,x:(r)+n y:(rh)+,y0");
		break;
		}
	case 1953: { // b,x:(r)+n y:(rh)+,y1
		unhandled("b,x:(r)+n y:(rh)+,y1");
		break;
		}
	case 1954: { // b,x:(r)+n y:(rh)+,a
		unhandled("b,x:(r)+n y:(rh)+,a");
		break;
		}
	case 1955: { // b,x:(r)+n y:(rh)+,b
		unhandled("b,x:(r)+n y:(rh)+,b");
		break;
		}
	case 1956: { // x0,x:(r)+n y:(rh),y0
		unhandled("x0,x:(r)+n y:(rh),y0");
		break;
		}
	case 1957: { // x0,x:(r)+n y:(rh),y1
		unhandled("x0,x:(r)+n y:(rh),y1");
		break;
		}
	case 1958: { // x0,x:(r)+n y:(rh),a
		unhandled("x0,x:(r)+n y:(rh),a");
		break;
		}
	case 1959: { // x0,x:(r)+n y:(rh),b
		unhandled("x0,x:(r)+n y:(rh),b");
		break;
		}
	case 1960: { // x1,x:(r)+n y:(rh),y0
		unhandled("x1,x:(r)+n y:(rh),y0");
		break;
		}
	case 1961: { // x1,x:(r)+n y:(rh),y1
		unhandled("x1,x:(r)+n y:(rh),y1");
		break;
		}
	case 1962: { // x1,x:(r)+n y:(rh),a
		unhandled("x1,x:(r)+n y:(rh),a");
		break;
		}
	case 1963: { // x1,x:(r)+n y:(rh),b
		unhandled("x1,x:(r)+n y:(rh),b");
		break;
		}
	case 1964: { // a,x:(r)+n y:(rh),y0
		unhandled("a,x:(r)+n y:(rh),y0");
		break;
		}
	case 1965: { // a,x:(r)+n y:(rh),y1
		unhandled("a,x:(r)+n y:(rh),y1");
		break;
		}
	case 1966: { // a,x:(r)+n y:(rh),a
		unhandled("a,x:(r)+n y:(rh),a");
		break;
		}
	case 1967: { // a,x:(r)+n y:(rh),b
		unhandled("a,x:(r)+n y:(rh),b");
		break;
		}
	case 1968: { // b,x:(r)+n y:(rh),y0
		unhandled("b,x:(r)+n y:(rh),y0");
		break;
		}
	case 1969: { // b,x:(r)+n y:(rh),y1
		unhandled("b,x:(r)+n y:(rh),y1");
		break;
		}
	case 1970: { // b,x:(r)+n y:(rh),a
		unhandled("b,x:(r)+n y:(rh),a");
		break;
		}
	case 1971: { // b,x:(r)+n y:(rh),b
		unhandled("b,x:(r)+n y:(rh),b");
		break;
		}
	case 1972: { // x0,x:(r)- y:(rh)+n,y0
		unhandled("x0,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1973: { // x0,x:(r)- y:(rh)+n,y1
		unhandled("x0,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1974: { // x0,x:(r)- y:(rh)+n,a
		unhandled("x0,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1975: { // x0,x:(r)- y:(rh)+n,b
		unhandled("x0,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1976: { // x1,x:(r)- y:(rh)+n,y0
		unhandled("x1,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1977: { // x1,x:(r)- y:(rh)+n,y1
		unhandled("x1,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1978: { // x1,x:(r)- y:(rh)+n,a
		unhandled("x1,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1979: { // x1,x:(r)- y:(rh)+n,b
		unhandled("x1,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1980: { // a,x:(r)- y:(rh)+n,y0
		unhandled("a,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1981: { // a,x:(r)- y:(rh)+n,y1
		unhandled("a,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1982: { // a,x:(r)- y:(rh)+n,a
		unhandled("a,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1983: { // a,x:(r)- y:(rh)+n,b
		unhandled("a,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1984: { // b,x:(r)- y:(rh)+n,y0
		unhandled("b,x:(r)- y:(rh)+n,y0");
		break;
		}
	case 1985: { // b,x:(r)- y:(rh)+n,y1
		unhandled("b,x:(r)- y:(rh)+n,y1");
		break;
		}
	case 1986: { // b,x:(r)- y:(rh)+n,a
		unhandled("b,x:(r)- y:(rh)+n,a");
		break;
		}
	case 1987: { // b,x:(r)- y:(rh)+n,b
		unhandled("b,x:(r)- y:(rh)+n,b");
		break;
		}
	case 1988: { // x0,x:(r)- y:(rh)-,y0
		unhandled("x0,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1989: { // x0,x:(r)- y:(rh)-,y1
		unhandled("x0,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1990: { // x0,x:(r)- y:(rh)-,a
		unhandled("x0,x:(r)- y:(rh)-,a");
		break;
		}
	case 1991: { // x0,x:(r)- y:(rh)-,b
		unhandled("x0,x:(r)- y:(rh)-,b");
		break;
		}
	case 1992: { // x1,x:(r)- y:(rh)-,y0
		unhandled("x1,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1993: { // x1,x:(r)- y:(rh)-,y1
		unhandled("x1,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1994: { // x1,x:(r)- y:(rh)-,a
		unhandled("x1,x:(r)- y:(rh)-,a");
		break;
		}
	case 1995: { // x1,x:(r)- y:(rh)-,b
		unhandled("x1,x:(r)- y:(rh)-,b");
		break;
		}
	case 1996: { // a,x:(r)- y:(rh)-,y0
		unhandled("a,x:(r)- y:(rh)-,y0");
		break;
		}
	case 1997: { // a,x:(r)- y:(rh)-,y1
		unhandled("a,x:(r)- y:(rh)-,y1");
		break;
		}
	case 1998: { // a,x:(r)- y:(rh)-,a
		unhandled("a,x:(r)- y:(rh)-,a");
		break;
		}
	case 1999: { // a,x:(r)- y:(rh)-,b
		unhandled("a,x:(r)- y:(rh)-,b");
		break;
		}
	case 2000: { // b,x:(r)- y:(rh)-,y0
		unhandled("b,x:(r)- y:(rh)-,y0");
		break;
		}
	case 2001: { // b,x:(r)- y:(rh)-,y1
		unhandled("b,x:(r)- y:(rh)-,y1");
		break;
		}
	case 2002: { // b,x:(r)- y:(rh)-,a
		unhandled("b,x:(r)- y:(rh)-,a");
		break;
		}
	case 2003: { // b,x:(r)- y:(rh)-,b
		unhandled("b,x:(r)- y:(rh)-,b");
		break;
		}
	case 2004: { // x0,x:(r)- y:(rh)+,y0
		unhandled("x0,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2005: { // x0,x:(r)- y:(rh)+,y1
		unhandled("x0,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2006: { // x0,x:(r)- y:(rh)+,a
		unhandled("x0,x:(r)- y:(rh)+,a");
		break;
		}
	case 2007: { // x0,x:(r)- y:(rh)+,b
		unhandled("x0,x:(r)- y:(rh)+,b");
		break;
		}
	case 2008: { // x1,x:(r)- y:(rh)+,y0
		unhandled("x1,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2009: { // x1,x:(r)- y:(rh)+,y1
		unhandled("x1,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2010: { // x1,x:(r)- y:(rh)+,a
		unhandled("x1,x:(r)- y:(rh)+,a");
		break;
		}
	case 2011: { // x1,x:(r)- y:(rh)+,b
		unhandled("x1,x:(r)- y:(rh)+,b");
		break;
		}
	case 2012: { // a,x:(r)- y:(rh)+,y0
		unhandled("a,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2013: { // a,x:(r)- y:(rh)+,y1
		unhandled("a,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2014: { // a,x:(r)- y:(rh)+,a
		unhandled("a,x:(r)- y:(rh)+,a");
		break;
		}
	case 2015: { // a,x:(r)- y:(rh)+,b
		unhandled("a,x:(r)- y:(rh)+,b");
		break;
		}
	case 2016: { // b,x:(r)- y:(rh)+,y0
		unhandled("b,x:(r)- y:(rh)+,y0");
		break;
		}
	case 2017: { // b,x:(r)- y:(rh)+,y1
		unhandled("b,x:(r)- y:(rh)+,y1");
		break;
		}
	case 2018: { // b,x:(r)- y:(rh)+,a
		unhandled("b,x:(r)- y:(rh)+,a");
		break;
		}
	case 2019: { // b,x:(r)- y:(rh)+,b
		unhandled("b,x:(r)- y:(rh)+,b");
		break;
		}
	case 2020: { // x0,x:(r)- y:(rh),y0
		unhandled("x0,x:(r)- y:(rh),y0");
		break;
		}
	case 2021: { // x0,x:(r)- y:(rh),y1
		unhandled("x0,x:(r)- y:(rh),y1");
		break;
		}
	case 2022: { // x0,x:(r)- y:(rh),a
		unhandled("x0,x:(r)- y:(rh),a");
		break;
		}
	case 2023: { // x0,x:(r)- y:(rh),b
		unhandled("x0,x:(r)- y:(rh),b");
		break;
		}
	case 2024: { // x1,x:(r)- y:(rh),y0
		unhandled("x1,x:(r)- y:(rh),y0");
		break;
		}
	case 2025: { // x1,x:(r)- y:(rh),y1
		unhandled("x1,x:(r)- y:(rh),y1");
		break;
		}
	case 2026: { // x1,x:(r)- y:(rh),a
		unhandled("x1,x:(r)- y:(rh),a");
		break;
		}
	case 2027: { // x1,x:(r)- y:(rh),b
		unhandled("x1,x:(r)- y:(rh),b");
		break;
		}
	case 2028: { // a,x:(r)- y:(rh),y0
		unhandled("a,x:(r)- y:(rh),y0");
		break;
		}
	case 2029: { // a,x:(r)- y:(rh),y1
		unhandled("a,x:(r)- y:(rh),y1");
		break;
		}
	case 2030: { // a,x:(r)- y:(rh),a
		unhandled("a,x:(r)- y:(rh),a");
		break;
		}
	case 2031: { // a,x:(r)- y:(rh),b
		unhandled("a,x:(r)- y:(rh),b");
		break;
		}
	case 2032: { // b,x:(r)- y:(rh),y0
		unhandled("b,x:(r)- y:(rh),y0");
		break;
		}
	case 2033: { // b,x:(r)- y:(rh),y1
		unhandled("b,x:(r)- y:(rh),y1");
		break;
		}
	case 2034: { // b,x:(r)- y:(rh),a
		unhandled("b,x:(r)- y:(rh),a");
		break;
		}
	case 2035: { // b,x:(r)- y:(rh),b
		unhandled("b,x:(r)- y:(rh),b");
		break;
		}
	case 2036: { // x0,x:(r)+ y:(rh)+n,y0
		unhandled("x0,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2037: { // x0,x:(r)+ y:(rh)+n,y1
		unhandled("x0,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2038: { // x0,x:(r)+ y:(rh)+n,a
		unhandled("x0,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2039: { // x0,x:(r)+ y:(rh)+n,b
		unhandled("x0,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2040: { // x1,x:(r)+ y:(rh)+n,y0
		unhandled("x1,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2041: { // x1,x:(r)+ y:(rh)+n,y1
		unhandled("x1,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2042: { // x1,x:(r)+ y:(rh)+n,a
		unhandled("x1,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2043: { // x1,x:(r)+ y:(rh)+n,b
		unhandled("x1,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2044: { // a,x:(r)+ y:(rh)+n,y0
		unhandled("a,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2045: { // a,x:(r)+ y:(rh)+n,y1
		unhandled("a,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2046: { // a,x:(r)+ y:(rh)+n,a
		unhandled("a,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2047: { // a,x:(r)+ y:(rh)+n,b
		unhandled("a,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2048: { // b,x:(r)+ y:(rh)+n,y0
		unhandled("b,x:(r)+ y:(rh)+n,y0");
		break;
		}
	case 2049: { // b,x:(r)+ y:(rh)+n,y1
		unhandled("b,x:(r)+ y:(rh)+n,y1");
		break;
		}
	case 2050: { // b,x:(r)+ y:(rh)+n,a
		unhandled("b,x:(r)+ y:(rh)+n,a");
		break;
		}
	case 2051: { // b,x:(r)+ y:(rh)+n,b
		unhandled("b,x:(r)+ y:(rh)+n,b");
		break;
		}
	case 2052: { // x0,x:(r)+ y:(rh)-,y0
		unhandled("x0,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2053: { // x0,x:(r)+ y:(rh)-,y1
		unhandled("x0,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2054: { // x0,x:(r)+ y:(rh)-,a
		unhandled("x0,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2055: { // x0,x:(r)+ y:(rh)-,b
		unhandled("x0,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2056: { // x1,x:(r)+ y:(rh)-,y0
		unhandled("x1,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2057: { // x1,x:(r)+ y:(rh)-,y1
		unhandled("x1,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2058: { // x1,x:(r)+ y:(rh)-,a
		unhandled("x1,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2059: { // x1,x:(r)+ y:(rh)-,b
		unhandled("x1,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2060: { // a,x:(r)+ y:(rh)-,y0
		unhandled("a,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2061: { // a,x:(r)+ y:(rh)-,y1
		unhandled("a,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2062: { // a,x:(r)+ y:(rh)-,a
		unhandled("a,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2063: { // a,x:(r)+ y:(rh)-,b
		unhandled("a,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2064: { // b,x:(r)+ y:(rh)-,y0
		unhandled("b,x:(r)+ y:(rh)-,y0");
		break;
		}
	case 2065: { // b,x:(r)+ y:(rh)-,y1
		unhandled("b,x:(r)+ y:(rh)-,y1");
		break;
		}
	case 2066: { // b,x:(r)+ y:(rh)-,a
		unhandled("b,x:(r)+ y:(rh)-,a");
		break;
		}
	case 2067: { // b,x:(r)+ y:(rh)-,b
		unhandled("b,x:(r)+ y:(rh)-,b");
		break;
		}
	case 2068: { // x0,x:(r)+ y:(rh)+,y0
		unhandled("x0,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2069: { // x0,x:(r)+ y:(rh)+,y1
		unhandled("x0,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2070: { // x0,x:(r)+ y:(rh)+,a
		unhandled("x0,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2071: { // x0,x:(r)+ y:(rh)+,b
		unhandled("x0,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2072: { // x1,x:(r)+ y:(rh)+,y0
		unhandled("x1,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2073: { // x1,x:(r)+ y:(rh)+,y1
		unhandled("x1,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2074: { // x1,x:(r)+ y:(rh)+,a
		unhandled("x1,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2075: { // x1,x:(r)+ y:(rh)+,b
		unhandled("x1,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2076: { // a,x:(r)+ y:(rh)+,y0
		unhandled("a,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2077: { // a,x:(r)+ y:(rh)+,y1
		unhandled("a,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2078: { // a,x:(r)+ y:(rh)+,a
		unhandled("a,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2079: { // a,x:(r)+ y:(rh)+,b
		unhandled("a,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2080: { // b,x:(r)+ y:(rh)+,y0
		unhandled("b,x:(r)+ y:(rh)+,y0");
		break;
		}
	case 2081: { // b,x:(r)+ y:(rh)+,y1
		unhandled("b,x:(r)+ y:(rh)+,y1");
		break;
		}
	case 2082: { // b,x:(r)+ y:(rh)+,a
		unhandled("b,x:(r)+ y:(rh)+,a");
		break;
		}
	case 2083: { // b,x:(r)+ y:(rh)+,b
		unhandled("b,x:(r)+ y:(rh)+,b");
		break;
		}
	case 2084: { // x0,x:(r)+ y:(rh),y0
		unhandled("x0,x:(r)+ y:(rh),y0");
		break;
		}
	case 2085: { // x0,x:(r)+ y:(rh),y1
		unhandled("x0,x:(r)+ y:(rh),y1");
		break;
		}
	case 2086: { // x0,x:(r)+ y:(rh),a
		unhandled("x0,x:(r)+ y:(rh),a");
		break;
		}
	case 2087: { // x0,x:(r)+ y:(rh),b
		unhandled("x0,x:(r)+ y:(rh),b");
		break;
		}
	case 2088: { // x1,x:(r)+ y:(rh),y0
		unhandled("x1,x:(r)+ y:(rh),y0");
		break;
		}
	case 2089: { // x1,x:(r)+ y:(rh),y1
		unhandled("x1,x:(r)+ y:(rh),y1");
		break;
		}
	case 2090: { // x1,x:(r)+ y:(rh),a
		unhandled("x1,x:(r)+ y:(rh),a");
		break;
		}
	case 2091: { // x1,x:(r)+ y:(rh),b
		unhandled("x1,x:(r)+ y:(rh),b");
		break;
		}
	case 2092: { // a,x:(r)+ y:(rh),y0
		unhandled("a,x:(r)+ y:(rh),y0");
		break;
		}
	case 2093: { // a,x:(r)+ y:(rh),y1
		unhandled("a,x:(r)+ y:(rh),y1");
		break;
		}
	case 2094: { // a,x:(r)+ y:(rh),a
		unhandled("a,x:(r)+ y:(rh),a");
		break;
		}
	case 2095: { // a,x:(r)+ y:(rh),b
		unhandled("a,x:(r)+ y:(rh),b");
		break;
		}
	case 2096: { // b,x:(r)+ y:(rh),y0
		unhandled("b,x:(r)+ y:(rh),y0");
		break;
		}
	case 2097: { // b,x:(r)+ y:(rh),y1
		unhandled("b,x:(r)+ y:(rh),y1");
		break;
		}
	case 2098: { // b,x:(r)+ y:(rh),a
		unhandled("b,x:(r)+ y:(rh),a");
		break;
		}
	case 2099: { // b,x:(r)+ y:(rh),b
		unhandled("b,x:(r)+ y:(rh),b");
		break;
		}
	case 2100: { // x0,x:(r) y:(rh)+n,y0
		unhandled("x0,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2101: { // x0,x:(r) y:(rh)+n,y1
		unhandled("x0,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2102: { // x0,x:(r) y:(rh)+n,a
		unhandled("x0,x:(r) y:(rh)+n,a");
		break;
		}
	case 2103: { // x0,x:(r) y:(rh)+n,b
		unhandled("x0,x:(r) y:(rh)+n,b");
		break;
		}
	case 2104: { // x1,x:(r) y:(rh)+n,y0
		unhandled("x1,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2105: { // x1,x:(r) y:(rh)+n,y1
		unhandled("x1,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2106: { // x1,x:(r) y:(rh)+n,a
		unhandled("x1,x:(r) y:(rh)+n,a");
		break;
		}
	case 2107: { // x1,x:(r) y:(rh)+n,b
		unhandled("x1,x:(r) y:(rh)+n,b");
		break;
		}
	case 2108: { // a,x:(r) y:(rh)+n,y0
		unhandled("a,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2109: { // a,x:(r) y:(rh)+n,y1
		unhandled("a,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2110: { // a,x:(r) y:(rh)+n,a
		unhandled("a,x:(r) y:(rh)+n,a");
		break;
		}
	case 2111: { // a,x:(r) y:(rh)+n,b
		unhandled("a,x:(r) y:(rh)+n,b");
		break;
		}
	case 2112: { // b,x:(r) y:(rh)+n,y0
		unhandled("b,x:(r) y:(rh)+n,y0");
		break;
		}
	case 2113: { // b,x:(r) y:(rh)+n,y1
		unhandled("b,x:(r) y:(rh)+n,y1");
		break;
		}
	case 2114: { // b,x:(r) y:(rh)+n,a
		unhandled("b,x:(r) y:(rh)+n,a");
		break;
		}
	case 2115: { // b,x:(r) y:(rh)+n,b
		unhandled("b,x:(r) y:(rh)+n,b");
		break;
		}
	case 2116: { // x0,x:(r) y:(rh)-,y0
		unhandled("x0,x:(r) y:(rh)-,y0");
		break;
		}
	case 2117: { // x0,x:(r) y:(rh)-,y1
		unhandled("x0,x:(r) y:(rh)-,y1");
		break;
		}
	case 2118: { // x0,x:(r) y:(rh)-,a
		unhandled("x0,x:(r) y:(rh)-,a");
		break;
		}
	case 2119: { // x0,x:(r) y:(rh)-,b
		unhandled("x0,x:(r) y:(rh)-,b");
		break;
		}
	case 2120: { // x1,x:(r) y:(rh)-,y0
		unhandled("x1,x:(r) y:(rh)-,y0");
		break;
		}
	case 2121: { // x1,x:(r) y:(rh)-,y1
		unhandled("x1,x:(r) y:(rh)-,y1");
		break;
		}
	case 2122: { // x1,x:(r) y:(rh)-,a
		unhandled("x1,x:(r) y:(rh)-,a");
		break;
		}
	case 2123: { // x1,x:(r) y:(rh)-,b
		unhandled("x1,x:(r) y:(rh)-,b");
		break;
		}
	case 2124: { // a,x:(r) y:(rh)-,y0
		unhandled("a,x:(r) y:(rh)-,y0");
		break;
		}
	case 2125: { // a,x:(r) y:(rh)-,y1
		unhandled("a,x:(r) y:(rh)-,y1");
		break;
		}
	case 2126: { // a,x:(r) y:(rh)-,a
		unhandled("a,x:(r) y:(rh)-,a");
		break;
		}
	case 2127: { // a,x:(r) y:(rh)-,b
		unhandled("a,x:(r) y:(rh)-,b");
		break;
		}
	case 2128: { // b,x:(r) y:(rh)-,y0
		unhandled("b,x:(r) y:(rh)-,y0");
		break;
		}
	case 2129: { // b,x:(r) y:(rh)-,y1
		unhandled("b,x:(r) y:(rh)-,y1");
		break;
		}
	case 2130: { // b,x:(r) y:(rh)-,a
		unhandled("b,x:(r) y:(rh)-,a");
		break;
		}
	case 2131: { // b,x:(r) y:(rh)-,b
		unhandled("b,x:(r) y:(rh)-,b");
		break;
		}
	case 2132: { // x0,x:(r) y:(rh)+,y0
		unhandled("x0,x:(r) y:(rh)+,y0");
		break;
		}
	case 2133: { // x0,x:(r) y:(rh)+,y1
		unhandled("x0,x:(r) y:(rh)+,y1");
		break;
		}
	case 2134: { // x0,x:(r) y:(rh)+,a
		unhandled("x0,x:(r) y:(rh)+,a");
		break;
		}
	case 2135: { // x0,x:(r) y:(rh)+,b
		unhandled("x0,x:(r) y:(rh)+,b");
		break;
		}
	case 2136: { // x1,x:(r) y:(rh)+,y0
		unhandled("x1,x:(r) y:(rh)+,y0");
		break;
		}
	case 2137: { // x1,x:(r) y:(rh)+,y1
		unhandled("x1,x:(r) y:(rh)+,y1");
		break;
		}
	case 2138: { // x1,x:(r) y:(rh)+,a
		unhandled("x1,x:(r) y:(rh)+,a");
		break;
		}
	case 2139: { // x1,x:(r) y:(rh)+,b
		unhandled("x1,x:(r) y:(rh)+,b");
		break;
		}
	case 2140: { // a,x:(r) y:(rh)+,y0
		unhandled("a,x:(r) y:(rh)+,y0");
		break;
		}
	case 2141: { // a,x:(r) y:(rh)+,y1
		unhandled("a,x:(r) y:(rh)+,y1");
		break;
		}
	case 2142: { // a,x:(r) y:(rh)+,a
		unhandled("a,x:(r) y:(rh)+,a");
		break;
		}
	case 2143: { // a,x:(r) y:(rh)+,b
		unhandled("a,x:(r) y:(rh)+,b");
		break;
		}
	case 2144: { // b,x:(r) y:(rh)+,y0
		unhandled("b,x:(r) y:(rh)+,y0");
		break;
		}
	case 2145: { // b,x:(r) y:(rh)+,y1
		unhandled("b,x:(r) y:(rh)+,y1");
		break;
		}
	case 2146: { // b,x:(r) y:(rh)+,a
		unhandled("b,x:(r) y:(rh)+,a");
		break;
		}
	case 2147: { // b,x:(r) y:(rh)+,b
		unhandled("b,x:(r) y:(rh)+,b");
		break;
		}
	case 2148: { // x0,x:(r) y:(rh),y0
		unhandled("x0,x:(r) y:(rh),y0");
		break;
		}
	case 2149: { // x0,x:(r) y:(rh),y1
		unhandled("x0,x:(r) y:(rh),y1");
		break;
		}
	case 2150: { // x0,x:(r) y:(rh),a
		unhandled("x0,x:(r) y:(rh),a");
		break;
		}
	case 2151: { // x0,x:(r) y:(rh),b
		unhandled("x0,x:(r) y:(rh),b");
		break;
		}
	case 2152: { // x1,x:(r) y:(rh),y0
		unhandled("x1,x:(r) y:(rh),y0");
		break;
		}
	case 2153: { // x1,x:(r) y:(rh),y1
		unhandled("x1,x:(r) y:(rh),y1");
		break;
		}
	case 2154: { // x1,x:(r) y:(rh),a
		unhandled("x1,x:(r) y:(rh),a");
		break;
		}
	case 2155: { // x1,x:(r) y:(rh),b
		unhandled("x1,x:(r) y:(rh),b");
		break;
		}
	case 2156: { // a,x:(r) y:(rh),y0
		unhandled("a,x:(r) y:(rh),y0");
		break;
		}
	case 2157: { // a,x:(r) y:(rh),y1
		unhandled("a,x:(r) y:(rh),y1");
		break;
		}
	case 2158: { // a,x:(r) y:(rh),a
		unhandled("a,x:(r) y:(rh),a");
		break;
		}
	case 2159: { // a,x:(r) y:(rh),b
		unhandled("a,x:(r) y:(rh),b");
		break;
		}
	case 2160: { // b,x:(r) y:(rh),y0
		unhandled("b,x:(r) y:(rh),y0");
		break;
		}
	case 2161: { // b,x:(r) y:(rh),y1
		unhandled("b,x:(r) y:(rh),y1");
		break;
		}
	case 2162: { // b,x:(r) y:(rh),a
		unhandled("b,x:(r) y:(rh),a");
		break;
		}
	case 2163: { // b,x:(r) y:(rh),b
		unhandled("b,x:(r) y:(rh),b");
		break;
		}
	case 2164: { // x0,x:(r)+n y0,y:(rh)+n
		unhandled("x0,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2165: { // x0,x:(r)+n y1,y:(rh)+n
		unhandled("x0,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2166: { // x0,x:(r)+n a,y:(rh)+n
		unhandled("x0,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2167: { // x0,x:(r)+n b,y:(rh)+n
		unhandled("x0,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2168: { // x1,x:(r)+n y0,y:(rh)+n
		unhandled("x1,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2169: { // x1,x:(r)+n y1,y:(rh)+n
		unhandled("x1,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2170: { // x1,x:(r)+n a,y:(rh)+n
		unhandled("x1,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2171: { // x1,x:(r)+n b,y:(rh)+n
		unhandled("x1,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2172: { // a,x:(r)+n y0,y:(rh)+n
		unhandled("a,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2173: { // a,x:(r)+n y1,y:(rh)+n
		unhandled("a,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2174: { // a,x:(r)+n a,y:(rh)+n
		unhandled("a,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2175: { // a,x:(r)+n b,y:(rh)+n
		unhandled("a,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2176: { // b,x:(r)+n y0,y:(rh)+n
		unhandled("b,x:(r)+n y0,y:(rh)+n");
		break;
		}
	case 2177: { // b,x:(r)+n y1,y:(rh)+n
		unhandled("b,x:(r)+n y1,y:(rh)+n");
		break;
		}
	case 2178: { // b,x:(r)+n a,y:(rh)+n
		unhandled("b,x:(r)+n a,y:(rh)+n");
		break;
		}
	case 2179: { // b,x:(r)+n b,y:(rh)+n
		unhandled("b,x:(r)+n b,y:(rh)+n");
		break;
		}
	case 2180: { // x0,x:(r)+n y0,y:(rh)-
		unhandled("x0,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2181: { // x0,x:(r)+n y1,y:(rh)-
		unhandled("x0,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2182: { // x0,x:(r)+n a,y:(rh)-
		unhandled("x0,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2183: { // x0,x:(r)+n b,y:(rh)-
		unhandled("x0,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2184: { // x1,x:(r)+n y0,y:(rh)-
		unhandled("x1,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2185: { // x1,x:(r)+n y1,y:(rh)-
		unhandled("x1,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2186: { // x1,x:(r)+n a,y:(rh)-
		unhandled("x1,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2187: { // x1,x:(r)+n b,y:(rh)-
		unhandled("x1,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2188: { // a,x:(r)+n y0,y:(rh)-
		unhandled("a,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2189: { // a,x:(r)+n y1,y:(rh)-
		unhandled("a,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2190: { // a,x:(r)+n a,y:(rh)-
		unhandled("a,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2191: { // a,x:(r)+n b,y:(rh)-
		unhandled("a,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2192: { // b,x:(r)+n y0,y:(rh)-
		unhandled("b,x:(r)+n y0,y:(rh)-");
		break;
		}
	case 2193: { // b,x:(r)+n y1,y:(rh)-
		unhandled("b,x:(r)+n y1,y:(rh)-");
		break;
		}
	case 2194: { // b,x:(r)+n a,y:(rh)-
		unhandled("b,x:(r)+n a,y:(rh)-");
		break;
		}
	case 2195: { // b,x:(r)+n b,y:(rh)-
		unhandled("b,x:(r)+n b,y:(rh)-");
		break;
		}
	case 2196: { // x0,x:(r)+n y0,y:(rh)+
		unhandled("x0,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2197: { // x0,x:(r)+n y1,y:(rh)+
		unhandled("x0,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2198: { // x0,x:(r)+n a,y:(rh)+
		unhandled("x0,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2199: { // x0,x:(r)+n b,y:(rh)+
		unhandled("x0,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2200: { // x1,x:(r)+n y0,y:(rh)+
		unhandled("x1,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2201: { // x1,x:(r)+n y1,y:(rh)+
		unhandled("x1,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2202: { // x1,x:(r)+n a,y:(rh)+
		unhandled("x1,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2203: { // x1,x:(r)+n b,y:(rh)+
		unhandled("x1,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2204: { // a,x:(r)+n y0,y:(rh)+
		unhandled("a,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2205: { // a,x:(r)+n y1,y:(rh)+
		unhandled("a,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2206: { // a,x:(r)+n a,y:(rh)+
		unhandled("a,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2207: { // a,x:(r)+n b,y:(rh)+
		unhandled("a,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2208: { // b,x:(r)+n y0,y:(rh)+
		unhandled("b,x:(r)+n y0,y:(rh)+");
		break;
		}
	case 2209: { // b,x:(r)+n y1,y:(rh)+
		unhandled("b,x:(r)+n y1,y:(rh)+");
		break;
		}
	case 2210: { // b,x:(r)+n a,y:(rh)+
		unhandled("b,x:(r)+n a,y:(rh)+");
		break;
		}
	case 2211: { // b,x:(r)+n b,y:(rh)+
		unhandled("b,x:(r)+n b,y:(rh)+");
		break;
		}
	case 2212: { // x0,x:(r)+n y0,y:(rh)
		unhandled("x0,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2213: { // x0,x:(r)+n y1,y:(rh)
		unhandled("x0,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2214: { // x0,x:(r)+n a,y:(rh)
		unhandled("x0,x:(r)+n a,y:(rh)");
		break;
		}
	case 2215: { // x0,x:(r)+n b,y:(rh)
		unhandled("x0,x:(r)+n b,y:(rh)");
		break;
		}
	case 2216: { // x1,x:(r)+n y0,y:(rh)
		unhandled("x1,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2217: { // x1,x:(r)+n y1,y:(rh)
		unhandled("x1,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2218: { // x1,x:(r)+n a,y:(rh)
		unhandled("x1,x:(r)+n a,y:(rh)");
		break;
		}
	case 2219: { // x1,x:(r)+n b,y:(rh)
		unhandled("x1,x:(r)+n b,y:(rh)");
		break;
		}
	case 2220: { // a,x:(r)+n y0,y:(rh)
		unhandled("a,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2221: { // a,x:(r)+n y1,y:(rh)
		unhandled("a,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2222: { // a,x:(r)+n a,y:(rh)
		unhandled("a,x:(r)+n a,y:(rh)");
		break;
		}
	case 2223: { // a,x:(r)+n b,y:(rh)
		unhandled("a,x:(r)+n b,y:(rh)");
		break;
		}
	case 2224: { // b,x:(r)+n y0,y:(rh)
		unhandled("b,x:(r)+n y0,y:(rh)");
		break;
		}
	case 2225: { // b,x:(r)+n y1,y:(rh)
		unhandled("b,x:(r)+n y1,y:(rh)");
		break;
		}
	case 2226: { // b,x:(r)+n a,y:(rh)
		unhandled("b,x:(r)+n a,y:(rh)");
		break;
		}
	case 2227: { // b,x:(r)+n b,y:(rh)
		unhandled("b,x:(r)+n b,y:(rh)");
		break;
		}
	case 2228: { // x0,x:(r)- y0,y:(rh)+n
		unhandled("x0,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2229: { // x0,x:(r)- y1,y:(rh)+n
		unhandled("x0,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2230: { // x0,x:(r)- a,y:(rh)+n
		unhandled("x0,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2231: { // x0,x:(r)- b,y:(rh)+n
		unhandled("x0,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2232: { // x1,x:(r)- y0,y:(rh)+n
		unhandled("x1,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2233: { // x1,x:(r)- y1,y:(rh)+n
		unhandled("x1,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2234: { // x1,x:(r)- a,y:(rh)+n
		unhandled("x1,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2235: { // x1,x:(r)- b,y:(rh)+n
		unhandled("x1,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2236: { // a,x:(r)- y0,y:(rh)+n
		unhandled("a,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2237: { // a,x:(r)- y1,y:(rh)+n
		unhandled("a,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2238: { // a,x:(r)- a,y:(rh)+n
		unhandled("a,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2239: { // a,x:(r)- b,y:(rh)+n
		unhandled("a,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2240: { // b,x:(r)- y0,y:(rh)+n
		unhandled("b,x:(r)- y0,y:(rh)+n");
		break;
		}
	case 2241: { // b,x:(r)- y1,y:(rh)+n
		unhandled("b,x:(r)- y1,y:(rh)+n");
		break;
		}
	case 2242: { // b,x:(r)- a,y:(rh)+n
		unhandled("b,x:(r)- a,y:(rh)+n");
		break;
		}
	case 2243: { // b,x:(r)- b,y:(rh)+n
		unhandled("b,x:(r)- b,y:(rh)+n");
		break;
		}
	case 2244: { // x0,x:(r)- y0,y:(rh)-
		unhandled("x0,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2245: { // x0,x:(r)- y1,y:(rh)-
		unhandled("x0,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2246: { // x0,x:(r)- a,y:(rh)-
		unhandled("x0,x:(r)- a,y:(rh)-");
		break;
		}
	case 2247: { // x0,x:(r)- b,y:(rh)-
		unhandled("x0,x:(r)- b,y:(rh)-");
		break;
		}
	case 2248: { // x1,x:(r)- y0,y:(rh)-
		unhandled("x1,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2249: { // x1,x:(r)- y1,y:(rh)-
		unhandled("x1,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2250: { // x1,x:(r)- a,y:(rh)-
		unhandled("x1,x:(r)- a,y:(rh)-");
		break;
		}
	case 2251: { // x1,x:(r)- b,y:(rh)-
		unhandled("x1,x:(r)- b,y:(rh)-");
		break;
		}
	case 2252: { // a,x:(r)- y0,y:(rh)-
		unhandled("a,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2253: { // a,x:(r)- y1,y:(rh)-
		unhandled("a,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2254: { // a,x:(r)- a,y:(rh)-
		unhandled("a,x:(r)- a,y:(rh)-");
		break;
		}
	case 2255: { // a,x:(r)- b,y:(rh)-
		unhandled("a,x:(r)- b,y:(rh)-");
		break;
		}
	case 2256: { // b,x:(r)- y0,y:(rh)-
		unhandled("b,x:(r)- y0,y:(rh)-");
		break;
		}
	case 2257: { // b,x:(r)- y1,y:(rh)-
		unhandled("b,x:(r)- y1,y:(rh)-");
		break;
		}
	case 2258: { // b,x:(r)- a,y:(rh)-
		unhandled("b,x:(r)- a,y:(rh)-");
		break;
		}
	case 2259: { // b,x:(r)- b,y:(rh)-
		unhandled("b,x:(r)- b,y:(rh)-");
		break;
		}
	case 2260: { // x0,x:(r)- y0,y:(rh)+
		unhandled("x0,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2261: { // x0,x:(r)- y1,y:(rh)+
		unhandled("x0,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2262: { // x0,x:(r)- a,y:(rh)+
		unhandled("x0,x:(r)- a,y:(rh)+");
		break;
		}
	case 2263: { // x0,x:(r)- b,y:(rh)+
		unhandled("x0,x:(r)- b,y:(rh)+");
		break;
		}
	case 2264: { // x1,x:(r)- y0,y:(rh)+
		unhandled("x1,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2265: { // x1,x:(r)- y1,y:(rh)+
		unhandled("x1,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2266: { // x1,x:(r)- a,y:(rh)+
		unhandled("x1,x:(r)- a,y:(rh)+");
		break;
		}
	case 2267: { // x1,x:(r)- b,y:(rh)+
		unhandled("x1,x:(r)- b,y:(rh)+");
		break;
		}
	case 2268: { // a,x:(r)- y0,y:(rh)+
		unhandled("a,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2269: { // a,x:(r)- y1,y:(rh)+
		unhandled("a,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2270: { // a,x:(r)- a,y:(rh)+
		unhandled("a,x:(r)- a,y:(rh)+");
		break;
		}
	case 2271: { // a,x:(r)- b,y:(rh)+
		unhandled("a,x:(r)- b,y:(rh)+");
		break;
		}
	case 2272: { // b,x:(r)- y0,y:(rh)+
		unhandled("b,x:(r)- y0,y:(rh)+");
		break;
		}
	case 2273: { // b,x:(r)- y1,y:(rh)+
		unhandled("b,x:(r)- y1,y:(rh)+");
		break;
		}
	case 2274: { // b,x:(r)- a,y:(rh)+
		unhandled("b,x:(r)- a,y:(rh)+");
		break;
		}
	case 2275: { // b,x:(r)- b,y:(rh)+
		unhandled("b,x:(r)- b,y:(rh)+");
		break;
		}
	case 2276: { // x0,x:(r)- y0,y:(rh)
		unhandled("x0,x:(r)- y0,y:(rh)");
		break;
		}
	case 2277: { // x0,x:(r)- y1,y:(rh)
		unhandled("x0,x:(r)- y1,y:(rh)");
		break;
		}
	case 2278: { // x0,x:(r)- a,y:(rh)
		unhandled("x0,x:(r)- a,y:(rh)");
		break;
		}
	case 2279: { // x0,x:(r)- b,y:(rh)
		unhandled("x0,x:(r)- b,y:(rh)");
		break;
		}
	case 2280: { // x1,x:(r)- y0,y:(rh)
		unhandled("x1,x:(r)- y0,y:(rh)");
		break;
		}
	case 2281: { // x1,x:(r)- y1,y:(rh)
		unhandled("x1,x:(r)- y1,y:(rh)");
		break;
		}
	case 2282: { // x1,x:(r)- a,y:(rh)
		unhandled("x1,x:(r)- a,y:(rh)");
		break;
		}
	case 2283: { // x1,x:(r)- b,y:(rh)
		unhandled("x1,x:(r)- b,y:(rh)");
		break;
		}
	case 2284: { // a,x:(r)- y0,y:(rh)
		unhandled("a,x:(r)- y0,y:(rh)");
		break;
		}
	case 2285: { // a,x:(r)- y1,y:(rh)
		unhandled("a,x:(r)- y1,y:(rh)");
		break;
		}
	case 2286: { // a,x:(r)- a,y:(rh)
		unhandled("a,x:(r)- a,y:(rh)");
		break;
		}
	case 2287: { // a,x:(r)- b,y:(rh)
		unhandled("a,x:(r)- b,y:(rh)");
		break;
		}
	case 2288: { // b,x:(r)- y0,y:(rh)
		unhandled("b,x:(r)- y0,y:(rh)");
		break;
		}
	case 2289: { // b,x:(r)- y1,y:(rh)
		unhandled("b,x:(r)- y1,y:(rh)");
		break;
		}
	case 2290: { // b,x:(r)- a,y:(rh)
		unhandled("b,x:(r)- a,y:(rh)");
		break;
		}
	case 2291: { // b,x:(r)- b,y:(rh)
		unhandled("b,x:(r)- b,y:(rh)");
		break;
		}
	case 2292: { // x0,x:(r)+ y0,y:(rh)+n
		unhandled("x0,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2293: { // x0,x:(r)+ y1,y:(rh)+n
		unhandled("x0,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2294: { // x0,x:(r)+ a,y:(rh)+n
		unhandled("x0,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2295: { // x0,x:(r)+ b,y:(rh)+n
		unhandled("x0,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2296: { // x1,x:(r)+ y0,y:(rh)+n
		unhandled("x1,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2297: { // x1,x:(r)+ y1,y:(rh)+n
		unhandled("x1,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2298: { // x1,x:(r)+ a,y:(rh)+n
		unhandled("x1,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2299: { // x1,x:(r)+ b,y:(rh)+n
		unhandled("x1,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2300: { // a,x:(r)+ y0,y:(rh)+n
		unhandled("a,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2301: { // a,x:(r)+ y1,y:(rh)+n
		unhandled("a,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2302: { // a,x:(r)+ a,y:(rh)+n
		unhandled("a,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2303: { // a,x:(r)+ b,y:(rh)+n
		unhandled("a,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2304: { // b,x:(r)+ y0,y:(rh)+n
		unhandled("b,x:(r)+ y0,y:(rh)+n");
		break;
		}
	case 2305: { // b,x:(r)+ y1,y:(rh)+n
		unhandled("b,x:(r)+ y1,y:(rh)+n");
		break;
		}
	case 2306: { // b,x:(r)+ a,y:(rh)+n
		unhandled("b,x:(r)+ a,y:(rh)+n");
		break;
		}
	case 2307: { // b,x:(r)+ b,y:(rh)+n
		unhandled("b,x:(r)+ b,y:(rh)+n");
		break;
		}
	case 2308: { // x0,x:(r)+ y0,y:(rh)-
		unhandled("x0,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2309: { // x0,x:(r)+ y1,y:(rh)-
		unhandled("x0,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2310: { // x0,x:(r)+ a,y:(rh)-
		unhandled("x0,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2311: { // x0,x:(r)+ b,y:(rh)-
		unhandled("x0,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2312: { // x1,x:(r)+ y0,y:(rh)-
		unhandled("x1,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2313: { // x1,x:(r)+ y1,y:(rh)-
		unhandled("x1,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2314: { // x1,x:(r)+ a,y:(rh)-
		unhandled("x1,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2315: { // x1,x:(r)+ b,y:(rh)-
		unhandled("x1,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2316: { // a,x:(r)+ y0,y:(rh)-
		unhandled("a,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2317: { // a,x:(r)+ y1,y:(rh)-
		unhandled("a,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2318: { // a,x:(r)+ a,y:(rh)-
		unhandled("a,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2319: { // a,x:(r)+ b,y:(rh)-
		unhandled("a,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2320: { // b,x:(r)+ y0,y:(rh)-
		unhandled("b,x:(r)+ y0,y:(rh)-");
		break;
		}
	case 2321: { // b,x:(r)+ y1,y:(rh)-
		unhandled("b,x:(r)+ y1,y:(rh)-");
		break;
		}
	case 2322: { // b,x:(r)+ a,y:(rh)-
		unhandled("b,x:(r)+ a,y:(rh)-");
		break;
		}
	case 2323: { // b,x:(r)+ b,y:(rh)-
		unhandled("b,x:(r)+ b,y:(rh)-");
		break;
		}
	case 2324: { // x0,x:(r)+ y0,y:(rh)+
		unhandled("x0,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2325: { // x0,x:(r)+ y1,y:(rh)+
		unhandled("x0,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2326: { // x0,x:(r)+ a,y:(rh)+
		unhandled("x0,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2327: { // x0,x:(r)+ b,y:(rh)+
		unhandled("x0,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2328: { // x1,x:(r)+ y0,y:(rh)+
		unhandled("x1,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2329: { // x1,x:(r)+ y1,y:(rh)+
		unhandled("x1,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2330: { // x1,x:(r)+ a,y:(rh)+
		unhandled("x1,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2331: { // x1,x:(r)+ b,y:(rh)+
		unhandled("x1,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2332: { // a,x:(r)+ y0,y:(rh)+
		unhandled("a,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2333: { // a,x:(r)+ y1,y:(rh)+
		unhandled("a,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2334: { // a,x:(r)+ a,y:(rh)+
		unhandled("a,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2335: { // a,x:(r)+ b,y:(rh)+
		unhandled("a,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2336: { // b,x:(r)+ y0,y:(rh)+
		unhandled("b,x:(r)+ y0,y:(rh)+");
		break;
		}
	case 2337: { // b,x:(r)+ y1,y:(rh)+
		unhandled("b,x:(r)+ y1,y:(rh)+");
		break;
		}
	case 2338: { // b,x:(r)+ a,y:(rh)+
		unhandled("b,x:(r)+ a,y:(rh)+");
		break;
		}
	case 2339: { // b,x:(r)+ b,y:(rh)+
		unhandled("b,x:(r)+ b,y:(rh)+");
		break;
		}
	case 2340: { // x0,x:(r)+ y0,y:(rh)
		unhandled("x0,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2341: { // x0,x:(r)+ y1,y:(rh)
		unhandled("x0,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2342: { // x0,x:(r)+ a,y:(rh)
		unhandled("x0,x:(r)+ a,y:(rh)");
		break;
		}
	case 2343: { // x0,x:(r)+ b,y:(rh)
		unhandled("x0,x:(r)+ b,y:(rh)");
		break;
		}
	case 2344: { // x1,x:(r)+ y0,y:(rh)
		unhandled("x1,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2345: { // x1,x:(r)+ y1,y:(rh)
		unhandled("x1,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2346: { // x1,x:(r)+ a,y:(rh)
		unhandled("x1,x:(r)+ a,y:(rh)");
		break;
		}
	case 2347: { // x1,x:(r)+ b,y:(rh)
		unhandled("x1,x:(r)+ b,y:(rh)");
		break;
		}
	case 2348: { // a,x:(r)+ y0,y:(rh)
		unhandled("a,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2349: { // a,x:(r)+ y1,y:(rh)
		unhandled("a,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2350: { // a,x:(r)+ a,y:(rh)
		unhandled("a,x:(r)+ a,y:(rh)");
		break;
		}
	case 2351: { // a,x:(r)+ b,y:(rh)
		unhandled("a,x:(r)+ b,y:(rh)");
		break;
		}
	case 2352: { // b,x:(r)+ y0,y:(rh)
		unhandled("b,x:(r)+ y0,y:(rh)");
		break;
		}
	case 2353: { // b,x:(r)+ y1,y:(rh)
		unhandled("b,x:(r)+ y1,y:(rh)");
		break;
		}
	case 2354: { // b,x:(r)+ a,y:(rh)
		unhandled("b,x:(r)+ a,y:(rh)");
		break;
		}
	case 2355: { // b,x:(r)+ b,y:(rh)
		unhandled("b,x:(r)+ b,y:(rh)");
		break;
		}
	case 2356: { // x0,x:(r) y0,y:(rh)+n
		unhandled("x0,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2357: { // x0,x:(r) y1,y:(rh)+n
		unhandled("x0,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2358: { // x0,x:(r) a,y:(rh)+n
		unhandled("x0,x:(r) a,y:(rh)+n");
		break;
		}
	case 2359: { // x0,x:(r) b,y:(rh)+n
		unhandled("x0,x:(r) b,y:(rh)+n");
		break;
		}
	case 2360: { // x1,x:(r) y0,y:(rh)+n
		unhandled("x1,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2361: { // x1,x:(r) y1,y:(rh)+n
		unhandled("x1,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2362: { // x1,x:(r) a,y:(rh)+n
		unhandled("x1,x:(r) a,y:(rh)+n");
		break;
		}
	case 2363: { // x1,x:(r) b,y:(rh)+n
		unhandled("x1,x:(r) b,y:(rh)+n");
		break;
		}
	case 2364: { // a,x:(r) y0,y:(rh)+n
		unhandled("a,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2365: { // a,x:(r) y1,y:(rh)+n
		unhandled("a,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2366: { // a,x:(r) a,y:(rh)+n
		unhandled("a,x:(r) a,y:(rh)+n");
		break;
		}
	case 2367: { // a,x:(r) b,y:(rh)+n
		unhandled("a,x:(r) b,y:(rh)+n");
		break;
		}
	case 2368: { // b,x:(r) y0,y:(rh)+n
		unhandled("b,x:(r) y0,y:(rh)+n");
		break;
		}
	case 2369: { // b,x:(r) y1,y:(rh)+n
		unhandled("b,x:(r) y1,y:(rh)+n");
		break;
		}
	case 2370: { // b,x:(r) a,y:(rh)+n
		unhandled("b,x:(r) a,y:(rh)+n");
		break;
		}
	case 2371: { // b,x:(r) b,y:(rh)+n
		unhandled("b,x:(r) b,y:(rh)+n");
		break;
		}
	case 2372: { // x0,x:(r) y0,y:(rh)-
		unhandled("x0,x:(r) y0,y:(rh)-");
		break;
		}
	case 2373: { // x0,x:(r) y1,y:(rh)-
		unhandled("x0,x:(r) y1,y:(rh)-");
		break;
		}
	case 2374: { // x0,x:(r) a,y:(rh)-
		unhandled("x0,x:(r) a,y:(rh)-");
		break;
		}
	case 2375: { // x0,x:(r) b,y:(rh)-
		unhandled("x0,x:(r) b,y:(rh)-");
		break;
		}
	case 2376: { // x1,x:(r) y0,y:(rh)-
		unhandled("x1,x:(r) y0,y:(rh)-");
		break;
		}
	case 2377: { // x1,x:(r) y1,y:(rh)-
		unhandled("x1,x:(r) y1,y:(rh)-");
		break;
		}
	case 2378: { // x1,x:(r) a,y:(rh)-
		unhandled("x1,x:(r) a,y:(rh)-");
		break;
		}
	case 2379: { // x1,x:(r) b,y:(rh)-
		unhandled("x1,x:(r) b,y:(rh)-");
		break;
		}
	case 2380: { // a,x:(r) y0,y:(rh)-
		unhandled("a,x:(r) y0,y:(rh)-");
		break;
		}
	case 2381: { // a,x:(r) y1,y:(rh)-
		unhandled("a,x:(r) y1,y:(rh)-");
		break;
		}
	case 2382: { // a,x:(r) a,y:(rh)-
		unhandled("a,x:(r) a,y:(rh)-");
		break;
		}
	case 2383: { // a,x:(r) b,y:(rh)-
		unhandled("a,x:(r) b,y:(rh)-");
		break;
		}
	case 2384: { // b,x:(r) y0,y:(rh)-
		unhandled("b,x:(r) y0,y:(rh)-");
		break;
		}
	case 2385: { // b,x:(r) y1,y:(rh)-
		unhandled("b,x:(r) y1,y:(rh)-");
		break;
		}
	case 2386: { // b,x:(r) a,y:(rh)-
		unhandled("b,x:(r) a,y:(rh)-");
		break;
		}
	case 2387: { // b,x:(r) b,y:(rh)-
		unhandled("b,x:(r) b,y:(rh)-");
		break;
		}
	case 2388: { // x0,x:(r) y0,y:(rh)+
		unhandled("x0,x:(r) y0,y:(rh)+");
		break;
		}
	case 2389: { // x0,x:(r) y1,y:(rh)+
		unhandled("x0,x:(r) y1,y:(rh)+");
		break;
		}
	case 2390: { // x0,x:(r) a,y:(rh)+
		unhandled("x0,x:(r) a,y:(rh)+");
		break;
		}
	case 2391: { // x0,x:(r) b,y:(rh)+
		unhandled("x0,x:(r) b,y:(rh)+");
		break;
		}
	case 2392: { // x1,x:(r) y0,y:(rh)+
		unhandled("x1,x:(r) y0,y:(rh)+");
		break;
		}
	case 2393: { // x1,x:(r) y1,y:(rh)+
		unhandled("x1,x:(r) y1,y:(rh)+");
		break;
		}
	case 2394: { // x1,x:(r) a,y:(rh)+
		unhandled("x1,x:(r) a,y:(rh)+");
		break;
		}
	case 2395: { // x1,x:(r) b,y:(rh)+
		unhandled("x1,x:(r) b,y:(rh)+");
		break;
		}
	case 2396: { // a,x:(r) y0,y:(rh)+
		unhandled("a,x:(r) y0,y:(rh)+");
		break;
		}
	case 2397: { // a,x:(r) y1,y:(rh)+
		unhandled("a,x:(r) y1,y:(rh)+");
		break;
		}
	case 2398: { // a,x:(r) a,y:(rh)+
		unhandled("a,x:(r) a,y:(rh)+");
		break;
		}
	case 2399: { // a,x:(r) b,y:(rh)+
		unhandled("a,x:(r) b,y:(rh)+");
		break;
		}
	case 2400: { // b,x:(r) y0,y:(rh)+
		unhandled("b,x:(r) y0,y:(rh)+");
		break;
		}
	case 2401: { // b,x:(r) y1,y:(rh)+
		unhandled("b,x:(r) y1,y:(rh)+");
		break;
		}
	case 2402: { // b,x:(r) a,y:(rh)+
		unhandled("b,x:(r) a,y:(rh)+");
		break;
		}
	case 2403: { // b,x:(r) b,y:(rh)+
		unhandled("b,x:(r) b,y:(rh)+");
		break;
		}
	case 2404: { // x0,x:(r) y0,y:(rh)
		unhandled("x0,x:(r) y0,y:(rh)");
		break;
		}
	case 2405: { // x0,x:(r) y1,y:(rh)
		unhandled("x0,x:(r) y1,y:(rh)");
		break;
		}
	case 2406: { // x0,x:(r) a,y:(rh)
		unhandled("x0,x:(r) a,y:(rh)");
		break;
		}
	case 2407: { // x0,x:(r) b,y:(rh)
		unhandled("x0,x:(r) b,y:(rh)");
		break;
		}
	case 2408: { // x1,x:(r) y0,y:(rh)
		unhandled("x1,x:(r) y0,y:(rh)");
		break;
		}
	case 2409: { // x1,x:(r) y1,y:(rh)
		unhandled("x1,x:(r) y1,y:(rh)");
		break;
		}
	case 2410: { // x1,x:(r) a,y:(rh)
		unhandled("x1,x:(r) a,y:(rh)");
		break;
		}
	case 2411: { // x1,x:(r) b,y:(rh)
		unhandled("x1,x:(r) b,y:(rh)");
		break;
		}
	case 2412: { // a,x:(r) y0,y:(rh)
		unhandled("a,x:(r) y0,y:(rh)");
		break;
		}
	case 2413: { // a,x:(r) y1,y:(rh)
		unhandled("a,x:(r) y1,y:(rh)");
		break;
		}
	case 2414: { // a,x:(r) a,y:(rh)
		unhandled("a,x:(r) a,y:(rh)");
		break;
		}
	case 2415: { // a,x:(r) b,y:(rh)
		unhandled("a,x:(r) b,y:(rh)");
		break;
		}
	case 2416: { // b,x:(r) y0,y:(rh)
		unhandled("b,x:(r) y0,y:(rh)");
		break;
		}
	case 2417: { // b,x:(r) y1,y:(rh)
		unhandled("b,x:(r) y1,y:(rh)");
		break;
		}
	case 2418: { // b,x:(r) a,y:(rh)
		unhandled("b,x:(r) a,y:(rh)");
		break;
		}
	case 2419: { // b,x:(r) b,y:(rh)
		unhandled("b,x:(r) b,y:(rh)");
		break;
		}
	case 2420: { // ifcc
		unhandled("ifcc");
		break;
		}
	case 2421: { // ifge
		unhandled("ifge");
		break;
		}
	case 2422: { // ifne
		unhandled("ifne");
		break;
		}
	case 2423: { // ifpl
		unhandled("ifpl");
		break;
		}
	case 2424: { // ifnn
		unhandled("ifnn");
		break;
		}
	case 2425: { // ifec
		unhandled("ifec");
		break;
		}
	case 2426: { // iflc
		unhandled("iflc");
		break;
		}
	case 2427: { // ifgt
		unhandled("ifgt");
		break;
		}
	case 2428: { // ifcs
		unhandled("ifcs");
		break;
		}
	case 2429: { // iflt
		unhandled("iflt");
		break;
		}
	case 2430: { // ifeq
		unhandled("ifeq");
		break;
		}
	case 2431: { // ifmi
		unhandled("ifmi");
		break;
		}
	case 2432: { // ifnr
		unhandled("ifnr");
		break;
		}
	case 2433: { // ifes
		unhandled("ifes");
		break;
		}
	case 2434: { // ifls
		unhandled("ifls");
		break;
		}
	case 2435: { // ifle
		unhandled("ifle");
		break;
		}
	case 2436: { // ifcc.u
		unhandled("ifcc.u");
		break;
		}
	case 2437: { // ifge.u
		unhandled("ifge.u");
		break;
		}
	case 2438: { // ifne.u
		unhandled("ifne.u");
		break;
		}
	case 2439: { // ifpl.u
		unhandled("ifpl.u");
		break;
		}
	case 2440: { // ifnn.u
		unhandled("ifnn.u");
		break;
		}
	case 2441: { // ifec.u
		unhandled("ifec.u");
		break;
		}
	case 2442: { // iflc.u
		unhandled("iflc.u");
		break;
		}
	case 2443: { // ifgt.u
		unhandled("ifgt.u");
		break;
		}
	case 2444: { // ifcs.u
		unhandled("ifcs.u");
		break;
		}
	case 2445: { // iflt.u
		unhandled("iflt.u");
		break;
		}
	case 2446: { // ifeq.u
		unhandled("ifeq.u");
		break;
		}
	case 2447: { // ifmi.u
		unhandled("ifmi.u");
		break;
		}
	case 2448: { // ifnr.u
		unhandled("ifnr.u");
		break;
		}
	case 2449: { // ifes.u
		unhandled("ifes.u");
		break;
		}
	case 2450: { // ifls.u
		unhandled("ifls.u");
		break;
		}
	case 2451: { // ifle.u
		unhandled("ifle.u");
		break;
		}
	}
}

void dsp563xx_device::execute_npar(u16 knpar, u32 opcode, u32 exv)
{
	switch(knpar) {
	case 0: { // -
		break;
		}
	case 1: { // 
		break;
		}
	case 2: { // move x:(r+[o]),x0
		unhandled("move x:(r+[o]),x0");
		break;
		}
	case 3: { // move x:(r+[o]),x1
		unhandled("move x:(r+[o]),x1");
		break;
		}
	case 4: { // move x:(r+[o]),y0
		unhandled("move x:(r+[o]),y0");
		break;
		}
	case 5: { // move x:(r+[o]),y1
		unhandled("move x:(r+[o]),y1");
		break;
		}
	case 6: { // move x:(r+[o]),a0
		unhandled("move x:(r+[o]),a0");
		break;
		}
	case 7: { // move x:(r+[o]),b0
		unhandled("move x:(r+[o]),b0");
		break;
		}
	case 8: { // move x:(r+[o]),a2
		unhandled("move x:(r+[o]),a2");
		break;
		}
	case 9: { // move x:(r+[o]),b2
		unhandled("move x:(r+[o]),b2");
		break;
		}
	case 10: { // move x:(r+[o]),a1
		unhandled("move x:(r+[o]),a1");
		break;
		}
	case 11: { // move x:(r+[o]),b1
		unhandled("move x:(r+[o]),b1");
		break;
		}
	case 12: { // move x:(r+[o]),a
		unhandled("move x:(r+[o]),a");
		break;
		}
	case 13: { // move x:(r+[o]),b
		unhandled("move x:(r+[o]),b");
		break;
		}
	case 14: { // move x:(r+[o]),r
		unhandled("move x:(r+[o]),r");
		break;
		}
	case 15: { // move x:(r+[o]),n
		unhandled("move x:(r+[o]),n");
		break;
		}
	case 16: { // move x:(r+[o]),m
		unhandled("move x:(r+[o]),m");
		break;
		}
	case 17: { // move x:(r+[o]),ep
		unhandled("move x:(r+[o]),ep");
		break;
		}
	case 18: { // move x:(r+[o]),vba
		unhandled("move x:(r+[o]),vba");
		break;
		}
	case 19: { // move x:(r+[o]),sc
		unhandled("move x:(r+[o]),sc");
		break;
		}
	case 20: { // move x:(r+[o]),sz
		unhandled("move x:(r+[o]),sz");
		break;
		}
	case 21: { // move x:(r+[o]),sr
		unhandled("move x:(r+[o]),sr");
		break;
		}
	case 22: { // move x:(r+[o]),omr
		unhandled("move x:(r+[o]),omr");
		break;
		}
	case 23: { // move x:(r+[o]),sp
		unhandled("move x:(r+[o]),sp");
		break;
		}
	case 24: { // move x:(r+[o]),ssh
		unhandled("move x:(r+[o]),ssh");
		break;
		}
	case 25: { // move x:(r+[o]),ssl
		unhandled("move x:(r+[o]),ssl");
		break;
		}
	case 26: { // move x:(r+[o]),la
		unhandled("move x:(r+[o]),la");
		break;
		}
	case 27: { // move x:(r+[o]),lc
		unhandled("move x:(r+[o]),lc");
		break;
		}
	case 28: { // move x0,x:(r+[o])
		unhandled("move x0,x:(r+[o])");
		break;
		}
	case 29: { // move x1,x:(r+[o])
		unhandled("move x1,x:(r+[o])");
		break;
		}
	case 30: { // move y0,x:(r+[o])
		unhandled("move y0,x:(r+[o])");
		break;
		}
	case 31: { // move y1,x:(r+[o])
		unhandled("move y1,x:(r+[o])");
		break;
		}
	case 32: { // move a0,x:(r+[o])
		unhandled("move a0,x:(r+[o])");
		break;
		}
	case 33: { // move b0,x:(r+[o])
		unhandled("move b0,x:(r+[o])");
		break;
		}
	case 34: { // move a2,x:(r+[o])
		unhandled("move a2,x:(r+[o])");
		break;
		}
	case 35: { // move b2,x:(r+[o])
		unhandled("move b2,x:(r+[o])");
		break;
		}
	case 36: { // move a1,x:(r+[o])
		unhandled("move a1,x:(r+[o])");
		break;
		}
	case 37: { // move b1,x:(r+[o])
		unhandled("move b1,x:(r+[o])");
		break;
		}
	case 38: { // move a,x:(r+[o])
		unhandled("move a,x:(r+[o])");
		break;
		}
	case 39: { // move b,x:(r+[o])
		unhandled("move b,x:(r+[o])");
		break;
		}
	case 40: { // move r,x:(r+[o])
		unhandled("move r,x:(r+[o])");
		break;
		}
	case 41: { // move n,x:(r+[o])
		unhandled("move n,x:(r+[o])");
		break;
		}
	case 42: { // move m,x:(r+[o])
		unhandled("move m,x:(r+[o])");
		break;
		}
	case 43: { // move ep,x:(r+[o])
		unhandled("move ep,x:(r+[o])");
		break;
		}
	case 44: { // move vba,x:(r+[o])
		unhandled("move vba,x:(r+[o])");
		break;
		}
	case 45: { // move sc,x:(r+[o])
		unhandled("move sc,x:(r+[o])");
		break;
		}
	case 46: { // move sz,x:(r+[o])
		unhandled("move sz,x:(r+[o])");
		break;
		}
	case 47: { // move sr,x:(r+[o])
		unhandled("move sr,x:(r+[o])");
		break;
		}
	case 48: { // move omr,x:(r+[o])
		unhandled("move omr,x:(r+[o])");
		break;
		}
	case 49: { // move sp,x:(r+[o])
		unhandled("move sp,x:(r+[o])");
		break;
		}
	case 50: { // move ssh,x:(r+[o])
		unhandled("move ssh,x:(r+[o])");
		break;
		}
	case 51: { // move ssl,x:(r+[o])
		unhandled("move ssl,x:(r+[o])");
		break;
		}
	case 52: { // move la,x:(r+[o])
		unhandled("move la,x:(r+[o])");
		break;
		}
	case 53: { // move lc,x:(r+[o])
		unhandled("move lc,x:(r+[o])");
		break;
		}
	case 54: { // move x:(r+[o]),x0
		unhandled("move x:(r+[o]),x0");
		break;
		}
	case 55: { // move x:(r+[o]),x1
		unhandled("move x:(r+[o]),x1");
		break;
		}
	case 56: { // move x:(r+[o]),y0
		unhandled("move x:(r+[o]),y0");
		break;
		}
	case 57: { // move x:(r+[o]),y1
		unhandled("move x:(r+[o]),y1");
		break;
		}
	case 58: { // move x:(r+[o]),a0
		unhandled("move x:(r+[o]),a0");
		break;
		}
	case 59: { // move x:(r+[o]),b0
		unhandled("move x:(r+[o]),b0");
		break;
		}
	case 60: { // move x:(r+[o]),a2
		unhandled("move x:(r+[o]),a2");
		break;
		}
	case 61: { // move x:(r+[o]),b2
		unhandled("move x:(r+[o]),b2");
		break;
		}
	case 62: { // move x:(r+[o]),a1
		unhandled("move x:(r+[o]),a1");
		break;
		}
	case 63: { // move x:(r+[o]),b1
		unhandled("move x:(r+[o]),b1");
		break;
		}
	case 64: { // move x:(r+[o]),a
		unhandled("move x:(r+[o]),a");
		break;
		}
	case 65: { // move x:(r+[o]),b
		unhandled("move x:(r+[o]),b");
		break;
		}
	case 66: { // move x0,x:(r+[o])
		unhandled("move x0,x:(r+[o])");
		break;
		}
	case 67: { // move x1,x:(r+[o])
		unhandled("move x1,x:(r+[o])");
		break;
		}
	case 68: { // move y0,x:(r+[o])
		unhandled("move y0,x:(r+[o])");
		break;
		}
	case 69: { // move y1,x:(r+[o])
		unhandled("move y1,x:(r+[o])");
		break;
		}
	case 70: { // move a0,x:(r+[o])
		unhandled("move a0,x:(r+[o])");
		break;
		}
	case 71: { // move b0,x:(r+[o])
		unhandled("move b0,x:(r+[o])");
		break;
		}
	case 72: { // move a2,x:(r+[o])
		unhandled("move a2,x:(r+[o])");
		break;
		}
	case 73: { // move b2,x:(r+[o])
		unhandled("move b2,x:(r+[o])");
		break;
		}
	case 74: { // move a1,x:(r+[o])
		unhandled("move a1,x:(r+[o])");
		break;
		}
	case 75: { // move b1,x:(r+[o])
		unhandled("move b1,x:(r+[o])");
		break;
		}
	case 76: { // move a,x:(r+[o])
		unhandled("move a,x:(r+[o])");
		break;
		}
	case 77: { // move b,x:(r+[o])
		unhandled("move b,x:(r+[o])");
		break;
		}
	case 78: { // move y:(r+[o]),x0
		unhandled("move y:(r+[o]),x0");
		break;
		}
	case 79: { // move y:(r+[o]),x1
		unhandled("move y:(r+[o]),x1");
		break;
		}
	case 80: { // move y:(r+[o]),y0
		unhandled("move y:(r+[o]),y0");
		break;
		}
	case 81: { // move y:(r+[o]),y1
		unhandled("move y:(r+[o]),y1");
		break;
		}
	case 82: { // move y:(r+[o]),a0
		unhandled("move y:(r+[o]),a0");
		break;
		}
	case 83: { // move y:(r+[o]),b0
		unhandled("move y:(r+[o]),b0");
		break;
		}
	case 84: { // move y:(r+[o]),a2
		unhandled("move y:(r+[o]),a2");
		break;
		}
	case 85: { // move y:(r+[o]),b2
		unhandled("move y:(r+[o]),b2");
		break;
		}
	case 86: { // move y:(r+[o]),a1
		unhandled("move y:(r+[o]),a1");
		break;
		}
	case 87: { // move y:(r+[o]),b1
		unhandled("move y:(r+[o]),b1");
		break;
		}
	case 88: { // move y:(r+[o]),a
		unhandled("move y:(r+[o]),a");
		break;
		}
	case 89: { // move y:(r+[o]),b
		unhandled("move y:(r+[o]),b");
		break;
		}
	case 90: { // move y:(r+[o]),r
		unhandled("move y:(r+[o]),r");
		break;
		}
	case 91: { // move y:(r+[o]),n
		unhandled("move y:(r+[o]),n");
		break;
		}
	case 92: { // move y:(r+[o]),m
		unhandled("move y:(r+[o]),m");
		break;
		}
	case 93: { // move y:(r+[o]),ep
		unhandled("move y:(r+[o]),ep");
		break;
		}
	case 94: { // move y:(r+[o]),vba
		unhandled("move y:(r+[o]),vba");
		break;
		}
	case 95: { // move y:(r+[o]),sc
		unhandled("move y:(r+[o]),sc");
		break;
		}
	case 96: { // move y:(r+[o]),sz
		unhandled("move y:(r+[o]),sz");
		break;
		}
	case 97: { // move y:(r+[o]),sr
		unhandled("move y:(r+[o]),sr");
		break;
		}
	case 98: { // move y:(r+[o]),omr
		unhandled("move y:(r+[o]),omr");
		break;
		}
	case 99: { // move y:(r+[o]),sp
		unhandled("move y:(r+[o]),sp");
		break;
		}
	case 100: { // move y:(r+[o]),ssh
		unhandled("move y:(r+[o]),ssh");
		break;
		}
	case 101: { // move y:(r+[o]),ssl
		unhandled("move y:(r+[o]),ssl");
		break;
		}
	case 102: { // move y:(r+[o]),la
		unhandled("move y:(r+[o]),la");
		break;
		}
	case 103: { // move y:(r+[o]),lc
		unhandled("move y:(r+[o]),lc");
		break;
		}
	case 104: { // move x0,y:(r+[o])
		unhandled("move x0,y:(r+[o])");
		break;
		}
	case 105: { // move x1,y:(r+[o])
		unhandled("move x1,y:(r+[o])");
		break;
		}
	case 106: { // move y0,y:(r+[o])
		unhandled("move y0,y:(r+[o])");
		break;
		}
	case 107: { // move y1,y:(r+[o])
		unhandled("move y1,y:(r+[o])");
		break;
		}
	case 108: { // move a0,y:(r+[o])
		unhandled("move a0,y:(r+[o])");
		break;
		}
	case 109: { // move b0,y:(r+[o])
		unhandled("move b0,y:(r+[o])");
		break;
		}
	case 110: { // move a2,y:(r+[o])
		unhandled("move a2,y:(r+[o])");
		break;
		}
	case 111: { // move b2,y:(r+[o])
		unhandled("move b2,y:(r+[o])");
		break;
		}
	case 112: { // move a1,y:(r+[o])
		unhandled("move a1,y:(r+[o])");
		break;
		}
	case 113: { // move b1,y:(r+[o])
		unhandled("move b1,y:(r+[o])");
		break;
		}
	case 114: { // move a,y:(r+[o])
		unhandled("move a,y:(r+[o])");
		break;
		}
	case 115: { // move b,y:(r+[o])
		unhandled("move b,y:(r+[o])");
		break;
		}
	case 116: { // move r,y:(r+[o])
		unhandled("move r,y:(r+[o])");
		break;
		}
	case 117: { // move n,y:(r+[o])
		unhandled("move n,y:(r+[o])");
		break;
		}
	case 118: { // move m,y:(r+[o])
		unhandled("move m,y:(r+[o])");
		break;
		}
	case 119: { // move ep,y:(r+[o])
		unhandled("move ep,y:(r+[o])");
		break;
		}
	case 120: { // move vba,y:(r+[o])
		unhandled("move vba,y:(r+[o])");
		break;
		}
	case 121: { // move sc,y:(r+[o])
		unhandled("move sc,y:(r+[o])");
		break;
		}
	case 122: { // move sz,y:(r+[o])
		unhandled("move sz,y:(r+[o])");
		break;
		}
	case 123: { // move sr,y:(r+[o])
		unhandled("move sr,y:(r+[o])");
		break;
		}
	case 124: { // move omr,y:(r+[o])
		unhandled("move omr,y:(r+[o])");
		break;
		}
	case 125: { // move sp,y:(r+[o])
		unhandled("move sp,y:(r+[o])");
		break;
		}
	case 126: { // move ssh,y:(r+[o])
		unhandled("move ssh,y:(r+[o])");
		break;
		}
	case 127: { // move ssl,y:(r+[o])
		unhandled("move ssl,y:(r+[o])");
		break;
		}
	case 128: { // move la,y:(r+[o])
		unhandled("move la,y:(r+[o])");
		break;
		}
	case 129: { // move lc,y:(r+[o])
		unhandled("move lc,y:(r+[o])");
		break;
		}
	case 130: { // move y:(r+[o]),x0
		unhandled("move y:(r+[o]),x0");
		break;
		}
	case 131: { // move y:(r+[o]),x1
		unhandled("move y:(r+[o]),x1");
		break;
		}
	case 132: { // move y:(r+[o]),y0
		unhandled("move y:(r+[o]),y0");
		break;
		}
	case 133: { // move y:(r+[o]),y1
		unhandled("move y:(r+[o]),y1");
		break;
		}
	case 134: { // move y:(r+[o]),a0
		unhandled("move y:(r+[o]),a0");
		break;
		}
	case 135: { // move y:(r+[o]),b0
		unhandled("move y:(r+[o]),b0");
		break;
		}
	case 136: { // move y:(r+[o]),a2
		unhandled("move y:(r+[o]),a2");
		break;
		}
	case 137: { // move y:(r+[o]),b2
		unhandled("move y:(r+[o]),b2");
		break;
		}
	case 138: { // move y:(r+[o]),a1
		unhandled("move y:(r+[o]),a1");
		break;
		}
	case 139: { // move y:(r+[o]),b1
		unhandled("move y:(r+[o]),b1");
		break;
		}
	case 140: { // move y:(r+[o]),a
		unhandled("move y:(r+[o]),a");
		break;
		}
	case 141: { // move y:(r+[o]),b
		unhandled("move y:(r+[o]),b");
		break;
		}
	case 142: { // move x0,y:(r+[o])
		unhandled("move x0,y:(r+[o])");
		break;
		}
	case 143: { // move x1,y:(r+[o])
		unhandled("move x1,y:(r+[o])");
		break;
		}
	case 144: { // move y0,y:(r+[o])
		unhandled("move y0,y:(r+[o])");
		break;
		}
	case 145: { // move y1,y:(r+[o])
		unhandled("move y1,y:(r+[o])");
		break;
		}
	case 146: { // move a0,y:(r+[o])
		unhandled("move a0,y:(r+[o])");
		break;
		}
	case 147: { // move b0,y:(r+[o])
		unhandled("move b0,y:(r+[o])");
		break;
		}
	case 148: { // move a2,y:(r+[o])
		unhandled("move a2,y:(r+[o])");
		break;
		}
	case 149: { // move b2,y:(r+[o])
		unhandled("move b2,y:(r+[o])");
		break;
		}
	case 150: { // move a1,y:(r+[o])
		unhandled("move a1,y:(r+[o])");
		break;
		}
	case 151: { // move b1,y:(r+[o])
		unhandled("move b1,y:(r+[o])");
		break;
		}
	case 152: { // move a,y:(r+[o])
		unhandled("move a,y:(r+[o])");
		break;
		}
	case 153: { // move b,y:(r+[o])
		unhandled("move b,y:(r+[o])");
		break;
		}
	case 154: { // add #[i],a
		unhandled("add #[i],a");
		break;
		}
	case 155: { // add #[i],b
		unhandled("add #[i],b");
		break;
		}
	case 156: { // add #[i],a
		unhandled("add #[i],a");
		break;
		}
	case 157: { // add #[i],b
		unhandled("add #[i],b");
		break;
		}
	case 158: { // and #[i],a
		unhandled("and #[i],a");
		break;
		}
	case 159: { // and #[i],b
		unhandled("and #[i],b");
		break;
		}
	case 160: { // and #[i],a
		unhandled("and #[i],a");
		break;
		}
	case 161: { // and #[i],b
		unhandled("and #[i],b");
		break;
		}
	case 162: { // andi #[i],mr
		u32 ctrl = get_mr();
		u32 i = BIT(opcode, 8, 8);
		set_mr(ctrl & i);
		break;
		}
	case 163: { // andi #[i],ccr
		u32 ctrl = get_ccr();
		u32 i = BIT(opcode, 8, 8);
		set_ccr(ctrl & i);
		break;
		}
	case 164: { // andi #[i],com
		u32 ctrl = get_com();
		u32 i = BIT(opcode, 8, 8);
		set_com(ctrl & i);
		break;
		}
	case 165: { // andi #[i],eom
		u32 ctrl = get_eom();
		u32 i = BIT(opcode, 8, 8);
		set_eom(ctrl & i);
		break;
		}
	case 166: { // asl #[i],a,a
		unhandled("asl #[i],a,a");
		break;
		}
	case 167: { // asl #[i],a,b
		unhandled("asl #[i],a,b");
		break;
		}
	case 168: { // asl #[i],b,a
		unhandled("asl #[i],b,a");
		break;
		}
	case 169: { // asl #[i],b,b
		unhandled("asl #[i],b,b");
		break;
		}
	case 170: { // asl a1,a,a
		unhandled("asl a1,a,a");
		break;
		}
	case 171: { // asl a1,a,b
		unhandled("asl a1,a,b");
		break;
		}
	case 172: { // asl b1,a,a
		unhandled("asl b1,a,a");
		break;
		}
	case 173: { // asl b1,a,b
		unhandled("asl b1,a,b");
		break;
		}
	case 174: { // asl x0,a,a
		unhandled("asl x0,a,a");
		break;
		}
	case 175: { // asl x0,a,b
		unhandled("asl x0,a,b");
		break;
		}
	case 176: { // asl y0,a,a
		unhandled("asl y0,a,a");
		break;
		}
	case 177: { // asl y0,a,b
		unhandled("asl y0,a,b");
		break;
		}
	case 178: { // asl x1,a,a
		unhandled("asl x1,a,a");
		break;
		}
	case 179: { // asl x1,a,b
		unhandled("asl x1,a,b");
		break;
		}
	case 180: { // asl y1,a,a
		unhandled("asl y1,a,a");
		break;
		}
	case 181: { // asl y1,a,b
		unhandled("asl y1,a,b");
		break;
		}
	case 182: { // asl a1,b,a
		unhandled("asl a1,b,a");
		break;
		}
	case 183: { // asl a1,b,b
		unhandled("asl a1,b,b");
		break;
		}
	case 184: { // asl b1,b,a
		unhandled("asl b1,b,a");
		break;
		}
	case 185: { // asl b1,b,b
		unhandled("asl b1,b,b");
		break;
		}
	case 186: { // asl x0,b,a
		unhandled("asl x0,b,a");
		break;
		}
	case 187: { // asl x0,b,b
		unhandled("asl x0,b,b");
		break;
		}
	case 188: { // asl y0,b,a
		unhandled("asl y0,b,a");
		break;
		}
	case 189: { // asl y0,b,b
		unhandled("asl y0,b,b");
		break;
		}
	case 190: { // asl x1,b,a
		unhandled("asl x1,b,a");
		break;
		}
	case 191: { // asl x1,b,b
		unhandled("asl x1,b,b");
		break;
		}
	case 192: { // asl y1,b,a
		unhandled("asl y1,b,a");
		break;
		}
	case 193: { // asl y1,b,b
		unhandled("asl y1,b,b");
		break;
		}
	case 194: { // asr #[i],a,a
		unhandled("asr #[i],a,a");
		break;
		}
	case 195: { // asr #[i],a,b
		unhandled("asr #[i],a,b");
		break;
		}
	case 196: { // asr #[i],b,a
		unhandled("asr #[i],b,a");
		break;
		}
	case 197: { // asr #[i],b,b
		unhandled("asr #[i],b,b");
		break;
		}
	case 198: { // asr a1,a,a
		unhandled("asr a1,a,a");
		break;
		}
	case 199: { // asr a1,a,b
		unhandled("asr a1,a,b");
		break;
		}
	case 200: { // asr b1,a,a
		unhandled("asr b1,a,a");
		break;
		}
	case 201: { // asr b1,a,b
		unhandled("asr b1,a,b");
		break;
		}
	case 202: { // asr x0,a,a
		unhandled("asr x0,a,a");
		break;
		}
	case 203: { // asr x0,a,b
		unhandled("asr x0,a,b");
		break;
		}
	case 204: { // asr y0,a,a
		unhandled("asr y0,a,a");
		break;
		}
	case 205: { // asr y0,a,b
		unhandled("asr y0,a,b");
		break;
		}
	case 206: { // asr x1,a,a
		unhandled("asr x1,a,a");
		break;
		}
	case 207: { // asr x1,a,b
		unhandled("asr x1,a,b");
		break;
		}
	case 208: { // asr y1,a,a
		unhandled("asr y1,a,a");
		break;
		}
	case 209: { // asr y1,a,b
		unhandled("asr y1,a,b");
		break;
		}
	case 210: { // asr a1,b,a
		unhandled("asr a1,b,a");
		break;
		}
	case 211: { // asr a1,b,b
		unhandled("asr a1,b,b");
		break;
		}
	case 212: { // asr b1,b,a
		unhandled("asr b1,b,a");
		break;
		}
	case 213: { // asr b1,b,b
		unhandled("asr b1,b,b");
		break;
		}
	case 214: { // asr x0,b,a
		unhandled("asr x0,b,a");
		break;
		}
	case 215: { // asr x0,b,b
		unhandled("asr x0,b,b");
		break;
		}
	case 216: { // asr y0,b,a
		unhandled("asr y0,b,a");
		break;
		}
	case 217: { // asr y0,b,b
		unhandled("asr y0,b,b");
		break;
		}
	case 218: { // asr x1,b,a
		unhandled("asr x1,b,a");
		break;
		}
	case 219: { // asr x1,b,b
		unhandled("asr x1,b,b");
		break;
		}
	case 220: { // asr y1,b,a
		unhandled("asr y1,b,a");
		break;
		}
	case 221: { // asr y1,b,b
		unhandled("asr y1,b,b");
		break;
		}
	case 222: { // bcc [x]
		unhandled("bcc [x]");
		break;
		}
	case 223: { // bge [x]
		unhandled("bge [x]");
		break;
		}
	case 224: { // bne [x]
		unhandled("bne [x]");
		break;
		}
	case 225: { // bpl [x]
		unhandled("bpl [x]");
		break;
		}
	case 226: { // bnn [x]
		unhandled("bnn [x]");
		break;
		}
	case 227: { // bec [x]
		unhandled("bec [x]");
		break;
		}
	case 228: { // blc [x]
		unhandled("blc [x]");
		break;
		}
	case 229: { // bgt [x]
		unhandled("bgt [x]");
		break;
		}
	case 230: { // bcs [x]
		unhandled("bcs [x]");
		break;
		}
	case 231: { // blt [x]
		unhandled("blt [x]");
		break;
		}
	case 232: { // beq [x]
		unhandled("beq [x]");
		break;
		}
	case 233: { // bmi [x]
		unhandled("bmi [x]");
		break;
		}
	case 234: { // bnr [x]
		unhandled("bnr [x]");
		break;
		}
	case 235: { // bes [x]
		unhandled("bes [x]");
		break;
		}
	case 236: { // bls [x]
		unhandled("bls [x]");
		break;
		}
	case 237: { // ble [x]
		unhandled("ble [x]");
		break;
		}
	case 238: { // bcc [x]
		unhandled("bcc [x]");
		break;
		}
	case 239: { // bge [x]
		unhandled("bge [x]");
		break;
		}
	case 240: { // bne [x]
		unhandled("bne [x]");
		break;
		}
	case 241: { // bpl [x]
		unhandled("bpl [x]");
		break;
		}
	case 242: { // bnn [x]
		unhandled("bnn [x]");
		break;
		}
	case 243: { // bec [x]
		unhandled("bec [x]");
		break;
		}
	case 244: { // blc [x]
		unhandled("blc [x]");
		break;
		}
	case 245: { // bgt [x]
		unhandled("bgt [x]");
		break;
		}
	case 246: { // bcs [x]
		unhandled("bcs [x]");
		break;
		}
	case 247: { // blt [x]
		unhandled("blt [x]");
		break;
		}
	case 248: { // beq [x]
		unhandled("beq [x]");
		break;
		}
	case 249: { // bmi [x]
		unhandled("bmi [x]");
		break;
		}
	case 250: { // bnr [x]
		unhandled("bnr [x]");
		break;
		}
	case 251: { // bes [x]
		unhandled("bes [x]");
		break;
		}
	case 252: { // bls [x]
		unhandled("bls [x]");
		break;
		}
	case 253: { // ble [x]
		unhandled("ble [x]");
		break;
		}
	case 254: { // bcc r
		unhandled("bcc r");
		break;
		}
	case 255: { // bge r
		unhandled("bge r");
		break;
		}
	case 256: { // bne r
		unhandled("bne r");
		break;
		}
	case 257: { // bpl r
		unhandled("bpl r");
		break;
		}
	case 258: { // bnn r
		unhandled("bnn r");
		break;
		}
	case 259: { // bec r
		unhandled("bec r");
		break;
		}
	case 260: { // blc r
		unhandled("blc r");
		break;
		}
	case 261: { // bgt r
		unhandled("bgt r");
		break;
		}
	case 262: { // bcs r
		unhandled("bcs r");
		break;
		}
	case 263: { // blt r
		unhandled("blt r");
		break;
		}
	case 264: { // beq r
		unhandled("beq r");
		break;
		}
	case 265: { // bmi r
		unhandled("bmi r");
		break;
		}
	case 266: { // bnr r
		unhandled("bnr r");
		break;
		}
	case 267: { // bes r
		unhandled("bes r");
		break;
		}
	case 268: { // bls r
		unhandled("bls r");
		break;
		}
	case 269: { // ble r
		unhandled("ble r");
		break;
		}
	case 270: { // bchg #[n],x:(r)-n
		unhandled("bchg #[n],x:(r)-n");
		break;
		}
	case 271: { // bchg #[n],y:(r)-n
		unhandled("bchg #[n],y:(r)-n");
		break;
		}
	case 272: { // bchg #[n],x:(r)+n
		unhandled("bchg #[n],x:(r)+n");
		break;
		}
	case 273: { // bchg #[n],y:(r)+n
		unhandled("bchg #[n],y:(r)+n");
		break;
		}
	case 274: { // bchg #[n],x:(r)-
		unhandled("bchg #[n],x:(r)-");
		break;
		}
	case 275: { // bchg #[n],y:(r)-
		unhandled("bchg #[n],y:(r)-");
		break;
		}
	case 276: { // bchg #[n],x:(r)+
		unhandled("bchg #[n],x:(r)+");
		break;
		}
	case 277: { // bchg #[n],y:(r)+
		unhandled("bchg #[n],y:(r)+");
		break;
		}
	case 278: { // bchg #[n],x:(r)
		unhandled("bchg #[n],x:(r)");
		break;
		}
	case 279: { // bchg #[n],y:(r)
		unhandled("bchg #[n],y:(r)");
		break;
		}
	case 280: { // bchg #[n],x:(r+n)
		unhandled("bchg #[n],x:(r+n)");
		break;
		}
	case 281: { // bchg #[n],y:(r+n)
		unhandled("bchg #[n],y:(r+n)");
		break;
		}
	case 282: { // bchg #[n],x:-(r)
		unhandled("bchg #[n],x:-(r)");
		break;
		}
	case 283: { // bchg #[n],y:-(r)
		unhandled("bchg #[n],y:-(r)");
		break;
		}
	case 284: { // bchg #[n],x:[abs]
		unhandled("bchg #[n],x:[abs]");
		break;
		}
	case 285: { // bchg #[n],y:[abs]
		unhandled("bchg #[n],y:[abs]");
		break;
		}
	case 286: { // bchg #[n],x:[aa]
		unhandled("bchg #[n],x:[aa]");
		break;
		}
	case 287: { // bchg #[n],y:[aa]
		unhandled("bchg #[n],y:[aa]");
		break;
		}
	case 288: { // bchg #[n],x:[pp]
		unhandled("bchg #[n],x:[pp]");
		break;
		}
	case 289: { // bchg #[n],y:[pp]
		unhandled("bchg #[n],y:[pp]");
		break;
		}
	case 290: { // bchg #[n],x:[qq]
		unhandled("bchg #[n],x:[qq]");
		break;
		}
	case 291: { // bchg #[n],y:[qq]
		unhandled("bchg #[n],y:[qq]");
		break;
		}
	case 292: { // bchg #[n],x0
		unhandled("bchg #[n],x0");
		break;
		}
	case 293: { // bchg #[n],x1
		unhandled("bchg #[n],x1");
		break;
		}
	case 294: { // bchg #[n],y0
		unhandled("bchg #[n],y0");
		break;
		}
	case 295: { // bchg #[n],y1
		unhandled("bchg #[n],y1");
		break;
		}
	case 296: { // bchg #[n],a0
		unhandled("bchg #[n],a0");
		break;
		}
	case 297: { // bchg #[n],b0
		unhandled("bchg #[n],b0");
		break;
		}
	case 298: { // bchg #[n],a2
		unhandled("bchg #[n],a2");
		break;
		}
	case 299: { // bchg #[n],b2
		unhandled("bchg #[n],b2");
		break;
		}
	case 300: { // bchg #[n],a1
		unhandled("bchg #[n],a1");
		break;
		}
	case 301: { // bchg #[n],b1
		unhandled("bchg #[n],b1");
		break;
		}
	case 302: { // bchg #[n],a
		unhandled("bchg #[n],a");
		break;
		}
	case 303: { // bchg #[n],b
		unhandled("bchg #[n],b");
		break;
		}
	case 304: { // bchg #[n],r
		unhandled("bchg #[n],r");
		break;
		}
	case 305: { // bchg #[n],n
		unhandled("bchg #[n],n");
		break;
		}
	case 306: { // bchg #[n],m
		unhandled("bchg #[n],m");
		break;
		}
	case 307: { // bchg #[n],ep
		unhandled("bchg #[n],ep");
		break;
		}
	case 308: { // bchg #[n],vba
		unhandled("bchg #[n],vba");
		break;
		}
	case 309: { // bchg #[n],sc
		unhandled("bchg #[n],sc");
		break;
		}
	case 310: { // bchg #[n],sz
		unhandled("bchg #[n],sz");
		break;
		}
	case 311: { // bchg #[n],sr
		unhandled("bchg #[n],sr");
		break;
		}
	case 312: { // bchg #[n],omr
		unhandled("bchg #[n],omr");
		break;
		}
	case 313: { // bchg #[n],sp
		unhandled("bchg #[n],sp");
		break;
		}
	case 314: { // bchg #[n],ssh
		unhandled("bchg #[n],ssh");
		break;
		}
	case 315: { // bchg #[n],ssl
		unhandled("bchg #[n],ssl");
		break;
		}
	case 316: { // bchg #[n],la
		unhandled("bchg #[n],la");
		break;
		}
	case 317: { // bchg #[n],lc
		unhandled("bchg #[n],lc");
		break;
		}
	case 318: { // bclr #[n],x:(r)-n
		unhandled("bclr #[n],x:(r)-n");
		break;
		}
	case 319: { // bclr #[n],y:(r)-n
		unhandled("bclr #[n],y:(r)-n");
		break;
		}
	case 320: { // bclr #[n],x:(r)+n
		unhandled("bclr #[n],x:(r)+n");
		break;
		}
	case 321: { // bclr #[n],y:(r)+n
		unhandled("bclr #[n],y:(r)+n");
		break;
		}
	case 322: { // bclr #[n],x:(r)-
		unhandled("bclr #[n],x:(r)-");
		break;
		}
	case 323: { // bclr #[n],y:(r)-
		unhandled("bclr #[n],y:(r)-");
		break;
		}
	case 324: { // bclr #[n],x:(r)+
		unhandled("bclr #[n],x:(r)+");
		break;
		}
	case 325: { // bclr #[n],y:(r)+
		unhandled("bclr #[n],y:(r)+");
		break;
		}
	case 326: { // bclr #[n],x:(r)
		unhandled("bclr #[n],x:(r)");
		break;
		}
	case 327: { // bclr #[n],y:(r)
		unhandled("bclr #[n],y:(r)");
		break;
		}
	case 328: { // bclr #[n],x:(r+n)
		unhandled("bclr #[n],x:(r+n)");
		break;
		}
	case 329: { // bclr #[n],y:(r+n)
		unhandled("bclr #[n],y:(r+n)");
		break;
		}
	case 330: { // bclr #[n],x:-(r)
		unhandled("bclr #[n],x:-(r)");
		break;
		}
	case 331: { // bclr #[n],y:-(r)
		unhandled("bclr #[n],y:-(r)");
		break;
		}
	case 332: { // bclr #[n],x:[abs]
		unhandled("bclr #[n],x:[abs]");
		break;
		}
	case 333: { // bclr #[n],y:[abs]
		unhandled("bclr #[n],y:[abs]");
		break;
		}
	case 334: { // bclr #[n],x:[aa]
		unhandled("bclr #[n],x:[aa]");
		break;
		}
	case 335: { // bclr #[n],y:[aa]
		unhandled("bclr #[n],y:[aa]");
		break;
		}
	case 336: { // bclr #[n],x:[pp]
		unhandled("bclr #[n],x:[pp]");
		break;
		}
	case 337: { // bclr #[n],y:[pp]
		unhandled("bclr #[n],y:[pp]");
		break;
		}
	case 338: { // bclr #[n],x:[qq]
		unhandled("bclr #[n],x:[qq]");
		break;
		}
	case 339: { // bclr #[n],y:[qq]
		unhandled("bclr #[n],y:[qq]");
		break;
		}
	case 340: { // bclr #[n],x0
		unhandled("bclr #[n],x0");
		break;
		}
	case 341: { // bclr #[n],x1
		unhandled("bclr #[n],x1");
		break;
		}
	case 342: { // bclr #[n],y0
		unhandled("bclr #[n],y0");
		break;
		}
	case 343: { // bclr #[n],y1
		unhandled("bclr #[n],y1");
		break;
		}
	case 344: { // bclr #[n],a0
		unhandled("bclr #[n],a0");
		break;
		}
	case 345: { // bclr #[n],b0
		unhandled("bclr #[n],b0");
		break;
		}
	case 346: { // bclr #[n],a2
		unhandled("bclr #[n],a2");
		break;
		}
	case 347: { // bclr #[n],b2
		unhandled("bclr #[n],b2");
		break;
		}
	case 348: { // bclr #[n],a1
		unhandled("bclr #[n],a1");
		break;
		}
	case 349: { // bclr #[n],b1
		unhandled("bclr #[n],b1");
		break;
		}
	case 350: { // bclr #[n],a
		unhandled("bclr #[n],a");
		break;
		}
	case 351: { // bclr #[n],b
		unhandled("bclr #[n],b");
		break;
		}
	case 352: { // bclr #[n],r
		unhandled("bclr #[n],r");
		break;
		}
	case 353: { // bclr #[n],n
		unhandled("bclr #[n],n");
		break;
		}
	case 354: { // bclr #[n],m
		unhandled("bclr #[n],m");
		break;
		}
	case 355: { // bclr #[n],ep
		unhandled("bclr #[n],ep");
		break;
		}
	case 356: { // bclr #[n],vba
		unhandled("bclr #[n],vba");
		break;
		}
	case 357: { // bclr #[n],sc
		unhandled("bclr #[n],sc");
		break;
		}
	case 358: { // bclr #[n],sz
		unhandled("bclr #[n],sz");
		break;
		}
	case 359: { // bclr #[n],sr
		unhandled("bclr #[n],sr");
		break;
		}
	case 360: { // bclr #[n],omr
		unhandled("bclr #[n],omr");
		break;
		}
	case 361: { // bclr #[n],sp
		unhandled("bclr #[n],sp");
		break;
		}
	case 362: { // bclr #[n],ssh
		unhandled("bclr #[n],ssh");
		break;
		}
	case 363: { // bclr #[n],ssl
		unhandled("bclr #[n],ssl");
		break;
		}
	case 364: { // bclr #[n],la
		unhandled("bclr #[n],la");
		break;
		}
	case 365: { // bclr #[n],lc
		unhandled("bclr #[n],lc");
		break;
		}
	case 366: { // bra [x]
		u32 x = (m_pc+exv) & 0xffffff;
		m_npc = x;
		break;
		}
	case 367: { // bra [x]
		u32 x = m_pc + bitswap<9>(opcode, 9, 8, 7, 6, 4, 3, 2, 1, 0);
		m_npc = x;
		break;
		}
	case 368: { // bra r
		u32 r = get_r(BIT(opcode, 8, 3) & 7);
		m_npc = r;
		break;
		}
	case 369: { // brclr #[n],x:(r)-n,[x]
		unhandled("brclr #[n],x:(r)-n,[x]");
		break;
		}
	case 370: { // brclr #[n],y:(r)-n,[x]
		unhandled("brclr #[n],y:(r)-n,[x]");
		break;
		}
	case 371: { // brclr #[n],x:(r)+n,[x]
		unhandled("brclr #[n],x:(r)+n,[x]");
		break;
		}
	case 372: { // brclr #[n],y:(r)+n,[x]
		unhandled("brclr #[n],y:(r)+n,[x]");
		break;
		}
	case 373: { // brclr #[n],x:(r)-,[x]
		unhandled("brclr #[n],x:(r)-,[x]");
		break;
		}
	case 374: { // brclr #[n],y:(r)-,[x]
		unhandled("brclr #[n],y:(r)-,[x]");
		break;
		}
	case 375: { // brclr #[n],x:(r)+,[x]
		unhandled("brclr #[n],x:(r)+,[x]");
		break;
		}
	case 376: { // brclr #[n],y:(r)+,[x]
		unhandled("brclr #[n],y:(r)+,[x]");
		break;
		}
	case 377: { // brclr #[n],x:(r),[x]
		unhandled("brclr #[n],x:(r),[x]");
		break;
		}
	case 378: { // brclr #[n],y:(r),[x]
		unhandled("brclr #[n],y:(r),[x]");
		break;
		}
	case 379: { // brclr #[n],x:(r+n),[x]
		unhandled("brclr #[n],x:(r+n),[x]");
		break;
		}
	case 380: { // brclr #[n],y:(r+n),[x]
		unhandled("brclr #[n],y:(r+n),[x]");
		break;
		}
	case 381: { // brclr #[n],x:-(r),[x]
		unhandled("brclr #[n],x:-(r),[x]");
		break;
		}
	case 382: { // brclr #[n],y:-(r),[x]
		unhandled("brclr #[n],y:-(r),[x]");
		break;
		}
	case 383: { // brclr #[n],x:[abs],[x]
		unhandled("brclr #[n],x:[abs],[x]");
		break;
		}
	case 384: { // brclr #[n],y:[abs],[x]
		unhandled("brclr #[n],y:[abs],[x]");
		break;
		}
	case 385: { // brclr #[n],x:[aa],[x]
		unhandled("brclr #[n],x:[aa],[x]");
		break;
		}
	case 386: { // brclr #[n],y:[aa],[x]
		unhandled("brclr #[n],y:[aa],[x]");
		break;
		}
	case 387: { // brclr #[n],x:[pp],[x]
		unhandled("brclr #[n],x:[pp],[x]");
		break;
		}
	case 388: { // brclr #[n],y:[pp],[x]
		unhandled("brclr #[n],y:[pp],[x]");
		break;
		}
	case 389: { // brclr #[n],x:[qq],[x]
		unhandled("brclr #[n],x:[qq],[x]");
		break;
		}
	case 390: { // brclr #[n],y:[qq],[x]
		unhandled("brclr #[n],y:[qq],[x]");
		break;
		}
	case 391: { // brclr #[n],x0,x
		unhandled("brclr #[n],x0,x");
		break;
		}
	case 392: { // brclr #[n],x1,x
		unhandled("brclr #[n],x1,x");
		break;
		}
	case 393: { // brclr #[n],y0,x
		unhandled("brclr #[n],y0,x");
		break;
		}
	case 394: { // brclr #[n],y1,x
		unhandled("brclr #[n],y1,x");
		break;
		}
	case 395: { // brclr #[n],a0,x
		unhandled("brclr #[n],a0,x");
		break;
		}
	case 396: { // brclr #[n],b0,x
		unhandled("brclr #[n],b0,x");
		break;
		}
	case 397: { // brclr #[n],a2,x
		unhandled("brclr #[n],a2,x");
		break;
		}
	case 398: { // brclr #[n],b2,x
		unhandled("brclr #[n],b2,x");
		break;
		}
	case 399: { // brclr #[n],a1,x
		unhandled("brclr #[n],a1,x");
		break;
		}
	case 400: { // brclr #[n],b1,x
		unhandled("brclr #[n],b1,x");
		break;
		}
	case 401: { // brclr #[n],a,x
		unhandled("brclr #[n],a,x");
		break;
		}
	case 402: { // brclr #[n],b,x
		unhandled("brclr #[n],b,x");
		break;
		}
	case 403: { // brclr #[n],r,x
		unhandled("brclr #[n],r,x");
		break;
		}
	case 404: { // brclr #[n],n,x
		unhandled("brclr #[n],n,x");
		break;
		}
	case 405: { // brclr #[n],m,x
		unhandled("brclr #[n],m,x");
		break;
		}
	case 406: { // brclr #[n],ep,x
		unhandled("brclr #[n],ep,x");
		break;
		}
	case 407: { // brclr #[n],vba,x
		unhandled("brclr #[n],vba,x");
		break;
		}
	case 408: { // brclr #[n],sc,x
		unhandled("brclr #[n],sc,x");
		break;
		}
	case 409: { // brclr #[n],sz,x
		unhandled("brclr #[n],sz,x");
		break;
		}
	case 410: { // brclr #[n],sr,x
		unhandled("brclr #[n],sr,x");
		break;
		}
	case 411: { // brclr #[n],omr,x
		unhandled("brclr #[n],omr,x");
		break;
		}
	case 412: { // brclr #[n],sp,x
		unhandled("brclr #[n],sp,x");
		break;
		}
	case 413: { // brclr #[n],ssh,x
		unhandled("brclr #[n],ssh,x");
		break;
		}
	case 414: { // brclr #[n],ssl,x
		unhandled("brclr #[n],ssl,x");
		break;
		}
	case 415: { // brclr #[n],la,x
		unhandled("brclr #[n],la,x");
		break;
		}
	case 416: { // brclr #[n],lc,x
		unhandled("brclr #[n],lc,x");
		break;
		}
	case 417: { // brkcc
		unhandled("brkcc");
		break;
		}
	case 418: { // brkge
		unhandled("brkge");
		break;
		}
	case 419: { // brkne
		unhandled("brkne");
		break;
		}
	case 420: { // brkpl
		unhandled("brkpl");
		break;
		}
	case 421: { // brknn
		unhandled("brknn");
		break;
		}
	case 422: { // brkec
		unhandled("brkec");
		break;
		}
	case 423: { // brklc
		unhandled("brklc");
		break;
		}
	case 424: { // brkgt
		unhandled("brkgt");
		break;
		}
	case 425: { // brkcs
		unhandled("brkcs");
		break;
		}
	case 426: { // brklt
		unhandled("brklt");
		break;
		}
	case 427: { // brkeq
		unhandled("brkeq");
		break;
		}
	case 428: { // brkmi
		unhandled("brkmi");
		break;
		}
	case 429: { // brknr
		unhandled("brknr");
		break;
		}
	case 430: { // brkes
		unhandled("brkes");
		break;
		}
	case 431: { // brkls
		unhandled("brkls");
		break;
		}
	case 432: { // brkle
		unhandled("brkle");
		break;
		}
	case 433: { // brset #[n],x:(r)-n,[x]
		unhandled("brset #[n],x:(r)-n,[x]");
		break;
		}
	case 434: { // brset #[n],y:(r)-n,[x]
		unhandled("brset #[n],y:(r)-n,[x]");
		break;
		}
	case 435: { // brset #[n],x:(r)+n,[x]
		unhandled("brset #[n],x:(r)+n,[x]");
		break;
		}
	case 436: { // brset #[n],y:(r)+n,[x]
		unhandled("brset #[n],y:(r)+n,[x]");
		break;
		}
	case 437: { // brset #[n],x:(r)-,[x]
		unhandled("brset #[n],x:(r)-,[x]");
		break;
		}
	case 438: { // brset #[n],y:(r)-,[x]
		unhandled("brset #[n],y:(r)-,[x]");
		break;
		}
	case 439: { // brset #[n],x:(r)+,[x]
		unhandled("brset #[n],x:(r)+,[x]");
		break;
		}
	case 440: { // brset #[n],y:(r)+,[x]
		unhandled("brset #[n],y:(r)+,[x]");
		break;
		}
	case 441: { // brset #[n],x:(r),[x]
		unhandled("brset #[n],x:(r),[x]");
		break;
		}
	case 442: { // brset #[n],y:(r),[x]
		unhandled("brset #[n],y:(r),[x]");
		break;
		}
	case 443: { // brset #[n],x:(r+n),[x]
		unhandled("brset #[n],x:(r+n),[x]");
		break;
		}
	case 444: { // brset #[n],y:(r+n),[x]
		unhandled("brset #[n],y:(r+n),[x]");
		break;
		}
	case 445: { // brset #[n],x:-(r),[x]
		unhandled("brset #[n],x:-(r),[x]");
		break;
		}
	case 446: { // brset #[n],y:-(r),[x]
		unhandled("brset #[n],y:-(r),[x]");
		break;
		}
	case 447: { // brset #[n],x:[abs],[x]
		unhandled("brset #[n],x:[abs],[x]");
		break;
		}
	case 448: { // brset #[n],y:[abs],[x]
		unhandled("brset #[n],y:[abs],[x]");
		break;
		}
	case 449: { // brset #[n],x:[aa],[x]
		unhandled("brset #[n],x:[aa],[x]");
		break;
		}
	case 450: { // brset #[n],y:[aa],[x]
		unhandled("brset #[n],y:[aa],[x]");
		break;
		}
	case 451: { // brset #[n],x:[pp],[x]
		unhandled("brset #[n],x:[pp],[x]");
		break;
		}
	case 452: { // brset #[n],y:[pp],[x]
		unhandled("brset #[n],y:[pp],[x]");
		break;
		}
	case 453: { // brset #[n],x:[qq],[x]
		unhandled("brset #[n],x:[qq],[x]");
		break;
		}
	case 454: { // brset #[n],y:[qq],[x]
		unhandled("brset #[n],y:[qq],[x]");
		break;
		}
	case 455: { // brset #[n],x0,[x]
		unhandled("brset #[n],x0,[x]");
		break;
		}
	case 456: { // brset #[n],x1,[x]
		unhandled("brset #[n],x1,[x]");
		break;
		}
	case 457: { // brset #[n],y0,[x]
		unhandled("brset #[n],y0,[x]");
		break;
		}
	case 458: { // brset #[n],y1,[x]
		unhandled("brset #[n],y1,[x]");
		break;
		}
	case 459: { // brset #[n],a0,[x]
		unhandled("brset #[n],a0,[x]");
		break;
		}
	case 460: { // brset #[n],b0,[x]
		unhandled("brset #[n],b0,[x]");
		break;
		}
	case 461: { // brset #[n],a2,[x]
		unhandled("brset #[n],a2,[x]");
		break;
		}
	case 462: { // brset #[n],b2,[x]
		unhandled("brset #[n],b2,[x]");
		break;
		}
	case 463: { // brset #[n],a1,[x]
		unhandled("brset #[n],a1,[x]");
		break;
		}
	case 464: { // brset #[n],b1,[x]
		unhandled("brset #[n],b1,[x]");
		break;
		}
	case 465: { // brset #[n],a,[x]
		unhandled("brset #[n],a,[x]");
		break;
		}
	case 466: { // brset #[n],b,[x]
		unhandled("brset #[n],b,[x]");
		break;
		}
	case 467: { // brset #[n],r,[x]
		unhandled("brset #[n],r,[x]");
		break;
		}
	case 468: { // brset #[n],n,[x]
		unhandled("brset #[n],n,[x]");
		break;
		}
	case 469: { // brset #[n],m,[x]
		unhandled("brset #[n],m,[x]");
		break;
		}
	case 470: { // brset #[n],ep,[x]
		unhandled("brset #[n],ep,[x]");
		break;
		}
	case 471: { // brset #[n],vba,[x]
		unhandled("brset #[n],vba,[x]");
		break;
		}
	case 472: { // brset #[n],sc,[x]
		unhandled("brset #[n],sc,[x]");
		break;
		}
	case 473: { // brset #[n],sz,[x]
		unhandled("brset #[n],sz,[x]");
		break;
		}
	case 474: { // brset #[n],sr,[x]
		unhandled("brset #[n],sr,[x]");
		break;
		}
	case 475: { // brset #[n],omr,[x]
		unhandled("brset #[n],omr,[x]");
		break;
		}
	case 476: { // brset #[n],sp,[x]
		unhandled("brset #[n],sp,[x]");
		break;
		}
	case 477: { // brset #[n],ssh,[x]
		unhandled("brset #[n],ssh,[x]");
		break;
		}
	case 478: { // brset #[n],ssl,[x]
		unhandled("brset #[n],ssl,[x]");
		break;
		}
	case 479: { // brset #[n],la,[x]
		unhandled("brset #[n],la,[x]");
		break;
		}
	case 480: { // brset #[n],lc,[x]
		unhandled("brset #[n],lc,[x]");
		break;
		}
	case 481: { // bscc [x]
		unhandled("bscc [x]");
		break;
		}
	case 482: { // bsge [x]
		unhandled("bsge [x]");
		break;
		}
	case 483: { // bsne [x]
		unhandled("bsne [x]");
		break;
		}
	case 484: { // bspl [x]
		unhandled("bspl [x]");
		break;
		}
	case 485: { // bsnn [x]
		unhandled("bsnn [x]");
		break;
		}
	case 486: { // bsec [x]
		unhandled("bsec [x]");
		break;
		}
	case 487: { // bslc [x]
		unhandled("bslc [x]");
		break;
		}
	case 488: { // bsgt [x]
		unhandled("bsgt [x]");
		break;
		}
	case 489: { // bscs [x]
		unhandled("bscs [x]");
		break;
		}
	case 490: { // bslt [x]
		unhandled("bslt [x]");
		break;
		}
	case 491: { // bseq [x]
		unhandled("bseq [x]");
		break;
		}
	case 492: { // bsmi [x]
		unhandled("bsmi [x]");
		break;
		}
	case 493: { // bsnr [x]
		unhandled("bsnr [x]");
		break;
		}
	case 494: { // bses [x]
		unhandled("bses [x]");
		break;
		}
	case 495: { // bsls [x]
		unhandled("bsls [x]");
		break;
		}
	case 496: { // bsle [x]
		unhandled("bsle [x]");
		break;
		}
	case 497: { // bscc [x]
		unhandled("bscc [x]");
		break;
		}
	case 498: { // bsge [x]
		unhandled("bsge [x]");
		break;
		}
	case 499: { // bsne [x]
		unhandled("bsne [x]");
		break;
		}
	case 500: { // bspl [x]
		unhandled("bspl [x]");
		break;
		}
	case 501: { // bsnn [x]
		unhandled("bsnn [x]");
		break;
		}
	case 502: { // bsec [x]
		unhandled("bsec [x]");
		break;
		}
	case 503: { // bslc [x]
		unhandled("bslc [x]");
		break;
		}
	case 504: { // bsgt [x]
		unhandled("bsgt [x]");
		break;
		}
	case 505: { // bscs [x]
		unhandled("bscs [x]");
		break;
		}
	case 506: { // bslt [x]
		unhandled("bslt [x]");
		break;
		}
	case 507: { // bseq [x]
		unhandled("bseq [x]");
		break;
		}
	case 508: { // bsmi [x]
		unhandled("bsmi [x]");
		break;
		}
	case 509: { // bsnr [x]
		unhandled("bsnr [x]");
		break;
		}
	case 510: { // bses [x]
		unhandled("bses [x]");
		break;
		}
	case 511: { // bsls [x]
		unhandled("bsls [x]");
		break;
		}
	case 512: { // bsle [x]
		unhandled("bsle [x]");
		break;
		}
	case 513: { // bscc r
		unhandled("bscc r");
		break;
		}
	case 514: { // bsge r
		unhandled("bsge r");
		break;
		}
	case 515: { // bsne r
		unhandled("bsne r");
		break;
		}
	case 516: { // bspl r
		unhandled("bspl r");
		break;
		}
	case 517: { // bsnn r
		unhandled("bsnn r");
		break;
		}
	case 518: { // bsec r
		unhandled("bsec r");
		break;
		}
	case 519: { // bslc r
		unhandled("bslc r");
		break;
		}
	case 520: { // bsgt r
		unhandled("bsgt r");
		break;
		}
	case 521: { // bscs r
		unhandled("bscs r");
		break;
		}
	case 522: { // bslt r
		unhandled("bslt r");
		break;
		}
	case 523: { // bseq r
		unhandled("bseq r");
		break;
		}
	case 524: { // bsmi r
		unhandled("bsmi r");
		break;
		}
	case 525: { // bsnr r
		unhandled("bsnr r");
		break;
		}
	case 526: { // bses r
		unhandled("bses r");
		break;
		}
	case 527: { // bsls r
		unhandled("bsls r");
		break;
		}
	case 528: { // bsle r
		unhandled("bsle r");
		break;
		}
	case 529: { // bsclr #[n],x:(r)-n,[x]
		unhandled("bsclr #[n],x:(r)-n,[x]");
		break;
		}
	case 530: { // bsclr #[n],y:(r)-n,[x]
		unhandled("bsclr #[n],y:(r)-n,[x]");
		break;
		}
	case 531: { // bsclr #[n],x:(r)+n,[x]
		unhandled("bsclr #[n],x:(r)+n,[x]");
		break;
		}
	case 532: { // bsclr #[n],y:(r)+n,[x]
		unhandled("bsclr #[n],y:(r)+n,[x]");
		break;
		}
	case 533: { // bsclr #[n],x:(r)-,[x]
		unhandled("bsclr #[n],x:(r)-,[x]");
		break;
		}
	case 534: { // bsclr #[n],y:(r)-,[x]
		unhandled("bsclr #[n],y:(r)-,[x]");
		break;
		}
	case 535: { // bsclr #[n],x:(r)+,[x]
		unhandled("bsclr #[n],x:(r)+,[x]");
		break;
		}
	case 536: { // bsclr #[n],y:(r)+,[x]
		unhandled("bsclr #[n],y:(r)+,[x]");
		break;
		}
	case 537: { // bsclr #[n],x:(r),[x]
		unhandled("bsclr #[n],x:(r),[x]");
		break;
		}
	case 538: { // bsclr #[n],y:(r),[x]
		unhandled("bsclr #[n],y:(r),[x]");
		break;
		}
	case 539: { // bsclr #[n],x:(r+n),[x]
		unhandled("bsclr #[n],x:(r+n),[x]");
		break;
		}
	case 540: { // bsclr #[n],y:(r+n),[x]
		unhandled("bsclr #[n],y:(r+n),[x]");
		break;
		}
	case 541: { // bsclr #[n],x:-(r),[x]
		unhandled("bsclr #[n],x:-(r),[x]");
		break;
		}
	case 542: { // bsclr #[n],y:-(r),[x]
		unhandled("bsclr #[n],y:-(r),[x]");
		break;
		}
	case 543: { // bsclr #[n],x:[abs],[x]
		unhandled("bsclr #[n],x:[abs],[x]");
		break;
		}
	case 544: { // bsclr #[n],y:[abs],[x]
		unhandled("bsclr #[n],y:[abs],[x]");
		break;
		}
	case 545: { // bsclr #[n],x:[aa],[x]
		unhandled("bsclr #[n],x:[aa],[x]");
		break;
		}
	case 546: { // bsclr #[n],y:[aa],[x]
		unhandled("bsclr #[n],y:[aa],[x]");
		break;
		}
	case 547: { // bsclr #[n],x:[pp],[x]
		unhandled("bsclr #[n],x:[pp],[x]");
		break;
		}
	case 548: { // bsclr #[n],y:[pp],[x]
		unhandled("bsclr #[n],y:[pp],[x]");
		break;
		}
	case 549: { // bsclr #[n],x:[qq],[x]
		unhandled("bsclr #[n],x:[qq],[x]");
		break;
		}
	case 550: { // bsclr #[n],y:[qq],[x]
		unhandled("bsclr #[n],y:[qq],[x]");
		break;
		}
	case 551: { // bsclr #[n],x0,[x]
		unhandled("bsclr #[n],x0,[x]");
		break;
		}
	case 552: { // bsclr #[n],x1,[x]
		unhandled("bsclr #[n],x1,[x]");
		break;
		}
	case 553: { // bsclr #[n],y0,[x]
		unhandled("bsclr #[n],y0,[x]");
		break;
		}
	case 554: { // bsclr #[n],y1,[x]
		unhandled("bsclr #[n],y1,[x]");
		break;
		}
	case 555: { // bsclr #[n],a0,[x]
		unhandled("bsclr #[n],a0,[x]");
		break;
		}
	case 556: { // bsclr #[n],b0,[x]
		unhandled("bsclr #[n],b0,[x]");
		break;
		}
	case 557: { // bsclr #[n],a2,[x]
		unhandled("bsclr #[n],a2,[x]");
		break;
		}
	case 558: { // bsclr #[n],b2,[x]
		unhandled("bsclr #[n],b2,[x]");
		break;
		}
	case 559: { // bsclr #[n],a1,[x]
		unhandled("bsclr #[n],a1,[x]");
		break;
		}
	case 560: { // bsclr #[n],b1,[x]
		unhandled("bsclr #[n],b1,[x]");
		break;
		}
	case 561: { // bsclr #[n],a,[x]
		unhandled("bsclr #[n],a,[x]");
		break;
		}
	case 562: { // bsclr #[n],b,[x]
		unhandled("bsclr #[n],b,[x]");
		break;
		}
	case 563: { // bsclr #[n],r,[x]
		unhandled("bsclr #[n],r,[x]");
		break;
		}
	case 564: { // bsclr #[n],n,[x]
		unhandled("bsclr #[n],n,[x]");
		break;
		}
	case 565: { // bsclr #[n],m,[x]
		unhandled("bsclr #[n],m,[x]");
		break;
		}
	case 566: { // bsclr #[n],ep,[x]
		unhandled("bsclr #[n],ep,[x]");
		break;
		}
	case 567: { // bsclr #[n],vba,[x]
		unhandled("bsclr #[n],vba,[x]");
		break;
		}
	case 568: { // bsclr #[n],sc,[x]
		unhandled("bsclr #[n],sc,[x]");
		break;
		}
	case 569: { // bsclr #[n],sz,[x]
		unhandled("bsclr #[n],sz,[x]");
		break;
		}
	case 570: { // bsclr #[n],sr,[x]
		unhandled("bsclr #[n],sr,[x]");
		break;
		}
	case 571: { // bsclr #[n],omr,[x]
		unhandled("bsclr #[n],omr,[x]");
		break;
		}
	case 572: { // bsclr #[n],sp,[x]
		unhandled("bsclr #[n],sp,[x]");
		break;
		}
	case 573: { // bsclr #[n],ssh,[x]
		unhandled("bsclr #[n],ssh,[x]");
		break;
		}
	case 574: { // bsclr #[n],ssl,[x]
		unhandled("bsclr #[n],ssl,[x]");
		break;
		}
	case 575: { // bsclr #[n],la,[x]
		unhandled("bsclr #[n],la,[x]");
		break;
		}
	case 576: { // bsclr #[n],lc,[x]
		unhandled("bsclr #[n],lc,[x]");
		break;
		}
	case 577: { // bset #[n],x:(r)-n
		unhandled("bset #[n],x:(r)-n");
		break;
		}
	case 578: { // bset #[n],y:(r)-n
		unhandled("bset #[n],y:(r)-n");
		break;
		}
	case 579: { // bset #[n],x:(r)+n
		unhandled("bset #[n],x:(r)+n");
		break;
		}
	case 580: { // bset #[n],y:(r)+n
		unhandled("bset #[n],y:(r)+n");
		break;
		}
	case 581: { // bset #[n],x:(r)-
		unhandled("bset #[n],x:(r)-");
		break;
		}
	case 582: { // bset #[n],y:(r)-
		unhandled("bset #[n],y:(r)-");
		break;
		}
	case 583: { // bset #[n],x:(r)+
		unhandled("bset #[n],x:(r)+");
		break;
		}
	case 584: { // bset #[n],y:(r)+
		unhandled("bset #[n],y:(r)+");
		break;
		}
	case 585: { // bset #[n],x:(r)
		unhandled("bset #[n],x:(r)");
		break;
		}
	case 586: { // bset #[n],y:(r)
		unhandled("bset #[n],y:(r)");
		break;
		}
	case 587: { // bset #[n],x:(r+n)
		unhandled("bset #[n],x:(r+n)");
		break;
		}
	case 588: { // bset #[n],y:(r+n)
		unhandled("bset #[n],y:(r+n)");
		break;
		}
	case 589: { // bset #[n],x:-(r)
		unhandled("bset #[n],x:-(r)");
		break;
		}
	case 590: { // bset #[n],y:-(r)
		unhandled("bset #[n],y:-(r)");
		break;
		}
	case 591: { // bset #[n],x:[abs]
		unhandled("bset #[n],x:[abs]");
		break;
		}
	case 592: { // bset #[n],y:[abs]
		unhandled("bset #[n],y:[abs]");
		break;
		}
	case 593: { // bset #[n],x:[aa]
		unhandled("bset #[n],x:[aa]");
		break;
		}
	case 594: { // bset #[n],y:[aa]
		unhandled("bset #[n],y:[aa]");
		break;
		}
	case 595: { // bset #[n],x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 t = m_x.read_dword(pp);
		u32 m = 1 << n;
		if(t & m) {
		m_ccr |= CCR_C;
		} else {
		m_ccr &= ~CCR_C;
		m_x.write_dword(pp, t | m);
		}
		break;
		}
	case 596: { // bset #[n],y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 t = m_y.read_dword(pp);
		u32 m = 1 << n;
		if(t & m) {
		m_ccr |= CCR_C;
		} else {
		m_ccr &= ~CCR_C;
		m_y.write_dword(pp, t | m);
		}
		break;
		}
	case 597: { // bset #[n],x:[qq]
		unhandled("bset #[n],x:[qq]");
		break;
		}
	case 598: { // bset #[n],y:[qq]
		unhandled("bset #[n],y:[qq]");
		break;
		}
	case 599: { // bset #[n],x0
		unhandled("bset #[n],x0");
		break;
		}
	case 600: { // bset #[n],x1
		unhandled("bset #[n],x1");
		break;
		}
	case 601: { // bset #[n],y0
		unhandled("bset #[n],y0");
		break;
		}
	case 602: { // bset #[n],y1
		unhandled("bset #[n],y1");
		break;
		}
	case 603: { // bset #[n],a0
		unhandled("bset #[n],a0");
		break;
		}
	case 604: { // bset #[n],b0
		unhandled("bset #[n],b0");
		break;
		}
	case 605: { // bset #[n],a2
		unhandled("bset #[n],a2");
		break;
		}
	case 606: { // bset #[n],b2
		unhandled("bset #[n],b2");
		break;
		}
	case 607: { // bset #[n],a1
		unhandled("bset #[n],a1");
		break;
		}
	case 608: { // bset #[n],b1
		unhandled("bset #[n],b1");
		break;
		}
	case 609: { // bset #[n],a
		unhandled("bset #[n],a");
		break;
		}
	case 610: { // bset #[n],b
		unhandled("bset #[n],b");
		break;
		}
	case 611: { // bset #[n],r
		unhandled("bset #[n],r");
		break;
		}
	case 612: { // bset #[n],n
		unhandled("bset #[n],n");
		break;
		}
	case 613: { // bset #[n],m
		unhandled("bset #[n],m");
		break;
		}
	case 614: { // bset #[n],ep
		unhandled("bset #[n],ep");
		break;
		}
	case 615: { // bset #[n],vba
		unhandled("bset #[n],vba");
		break;
		}
	case 616: { // bset #[n],sc
		unhandled("bset #[n],sc");
		break;
		}
	case 617: { // bset #[n],sz
		unhandled("bset #[n],sz");
		break;
		}
	case 618: { // bset #[n],sr
		unhandled("bset #[n],sr");
		break;
		}
	case 619: { // bset #[n],omr
		unhandled("bset #[n],omr");
		break;
		}
	case 620: { // bset #[n],sp
		unhandled("bset #[n],sp");
		break;
		}
	case 621: { // bset #[n],ssh
		unhandled("bset #[n],ssh");
		break;
		}
	case 622: { // bset #[n],ssl
		unhandled("bset #[n],ssl");
		break;
		}
	case 623: { // bset #[n],la
		unhandled("bset #[n],la");
		break;
		}
	case 624: { // bset #[n],lc
		unhandled("bset #[n],lc");
		break;
		}
	case 625: { // bsr [x]
		unhandled("bsr [x]");
		break;
		}
	case 626: { // bsr [x]
		unhandled("bsr [x]");
		break;
		}
	case 627: { // bsr r
		unhandled("bsr r");
		break;
		}
	case 628: { // bsset #[n],x:(r)-n,[x]
		unhandled("bsset #[n],x:(r)-n,[x]");
		break;
		}
	case 629: { // bsset #[n],y:(r)-n,[x]
		unhandled("bsset #[n],y:(r)-n,[x]");
		break;
		}
	case 630: { // bsset #[n],x:(r)+n,[x]
		unhandled("bsset #[n],x:(r)+n,[x]");
		break;
		}
	case 631: { // bsset #[n],y:(r)+n,[x]
		unhandled("bsset #[n],y:(r)+n,[x]");
		break;
		}
	case 632: { // bsset #[n],x:(r)-,[x]
		unhandled("bsset #[n],x:(r)-,[x]");
		break;
		}
	case 633: { // bsset #[n],y:(r)-,[x]
		unhandled("bsset #[n],y:(r)-,[x]");
		break;
		}
	case 634: { // bsset #[n],x:(r)+,[x]
		unhandled("bsset #[n],x:(r)+,[x]");
		break;
		}
	case 635: { // bsset #[n],y:(r)+,[x]
		unhandled("bsset #[n],y:(r)+,[x]");
		break;
		}
	case 636: { // bsset #[n],x:(r),[x]
		unhandled("bsset #[n],x:(r),[x]");
		break;
		}
	case 637: { // bsset #[n],y:(r),[x]
		unhandled("bsset #[n],y:(r),[x]");
		break;
		}
	case 638: { // bsset #[n],x:(r+n),[x]
		unhandled("bsset #[n],x:(r+n),[x]");
		break;
		}
	case 639: { // bsset #[n],y:(r+n),[x]
		unhandled("bsset #[n],y:(r+n),[x]");
		break;
		}
	case 640: { // bsset #[n],x:-(r),[x]
		unhandled("bsset #[n],x:-(r),[x]");
		break;
		}
	case 641: { // bsset #[n],y:-(r),[x]
		unhandled("bsset #[n],y:-(r),[x]");
		break;
		}
	case 642: { // bsset #[n],x:[abs],[x]
		unhandled("bsset #[n],x:[abs],[x]");
		break;
		}
	case 643: { // bsset #[n],y:[abs],[x]
		unhandled("bsset #[n],y:[abs],[x]");
		break;
		}
	case 644: { // bsset #[n],x:[aa],[x]
		unhandled("bsset #[n],x:[aa],[x]");
		break;
		}
	case 645: { // bsset #[n],y:[aa],[x]
		unhandled("bsset #[n],y:[aa],[x]");
		break;
		}
	case 646: { // bsset #[n],x:[pp],[x]
		unhandled("bsset #[n],x:[pp],[x]");
		break;
		}
	case 647: { // bsset #[n],y:[pp],[x]
		unhandled("bsset #[n],y:[pp],[x]");
		break;
		}
	case 648: { // bsset #[n],x:[qq],[x]
		unhandled("bsset #[n],x:[qq],[x]");
		break;
		}
	case 649: { // bsset #[n],y:[qq],[x]
		unhandled("bsset #[n],y:[qq],[x]");
		break;
		}
	case 650: { // bsset #[n],x0,[x]
		unhandled("bsset #[n],x0,[x]");
		break;
		}
	case 651: { // bsset #[n],x1,[x]
		unhandled("bsset #[n],x1,[x]");
		break;
		}
	case 652: { // bsset #[n],y0,[x]
		unhandled("bsset #[n],y0,[x]");
		break;
		}
	case 653: { // bsset #[n],y1,[x]
		unhandled("bsset #[n],y1,[x]");
		break;
		}
	case 654: { // bsset #[n],a0,[x]
		unhandled("bsset #[n],a0,[x]");
		break;
		}
	case 655: { // bsset #[n],b0,[x]
		unhandled("bsset #[n],b0,[x]");
		break;
		}
	case 656: { // bsset #[n],a2,[x]
		unhandled("bsset #[n],a2,[x]");
		break;
		}
	case 657: { // bsset #[n],b2,[x]
		unhandled("bsset #[n],b2,[x]");
		break;
		}
	case 658: { // bsset #[n],a1,[x]
		unhandled("bsset #[n],a1,[x]");
		break;
		}
	case 659: { // bsset #[n],b1,[x]
		unhandled("bsset #[n],b1,[x]");
		break;
		}
	case 660: { // bsset #[n],a,[x]
		unhandled("bsset #[n],a,[x]");
		break;
		}
	case 661: { // bsset #[n],b,[x]
		unhandled("bsset #[n],b,[x]");
		break;
		}
	case 662: { // bsset #[n],r,[x]
		unhandled("bsset #[n],r,[x]");
		break;
		}
	case 663: { // bsset #[n],n,[x]
		unhandled("bsset #[n],n,[x]");
		break;
		}
	case 664: { // bsset #[n],m,[x]
		unhandled("bsset #[n],m,[x]");
		break;
		}
	case 665: { // bsset #[n],ep,[x]
		unhandled("bsset #[n],ep,[x]");
		break;
		}
	case 666: { // bsset #[n],vba,[x]
		unhandled("bsset #[n],vba,[x]");
		break;
		}
	case 667: { // bsset #[n],sc,[x]
		unhandled("bsset #[n],sc,[x]");
		break;
		}
	case 668: { // bsset #[n],sz,[x]
		unhandled("bsset #[n],sz,[x]");
		break;
		}
	case 669: { // bsset #[n],sr,[x]
		unhandled("bsset #[n],sr,[x]");
		break;
		}
	case 670: { // bsset #[n],omr,[x]
		unhandled("bsset #[n],omr,[x]");
		break;
		}
	case 671: { // bsset #[n],sp,[x]
		unhandled("bsset #[n],sp,[x]");
		break;
		}
	case 672: { // bsset #[n],ssh,[x]
		unhandled("bsset #[n],ssh,[x]");
		break;
		}
	case 673: { // bsset #[n],ssl,[x]
		unhandled("bsset #[n],ssl,[x]");
		break;
		}
	case 674: { // bsset #[n],la,[x]
		unhandled("bsset #[n],la,[x]");
		break;
		}
	case 675: { // bsset #[n],lc,[x]
		unhandled("bsset #[n],lc,[x]");
		break;
		}
	case 676: { // btst #[n],x:(r)-n
		unhandled("btst #[n],x:(r)-n");
		break;
		}
	case 677: { // btst #[n],y:(r)-n
		unhandled("btst #[n],y:(r)-n");
		break;
		}
	case 678: { // btst #[n],x:(r)+n
		unhandled("btst #[n],x:(r)+n");
		break;
		}
	case 679: { // btst #[n],y:(r)+n
		unhandled("btst #[n],y:(r)+n");
		break;
		}
	case 680: { // btst #[n],x:(r)-
		unhandled("btst #[n],x:(r)-");
		break;
		}
	case 681: { // btst #[n],y:(r)-
		unhandled("btst #[n],y:(r)-");
		break;
		}
	case 682: { // btst #[n],x:(r)+
		unhandled("btst #[n],x:(r)+");
		break;
		}
	case 683: { // btst #[n],y:(r)+
		unhandled("btst #[n],y:(r)+");
		break;
		}
	case 684: { // btst #[n],x:(r)
		unhandled("btst #[n],x:(r)");
		break;
		}
	case 685: { // btst #[n],y:(r)
		unhandled("btst #[n],y:(r)");
		break;
		}
	case 686: { // btst #[n],x:(r+n)
		unhandled("btst #[n],x:(r+n)");
		break;
		}
	case 687: { // btst #[n],y:(r+n)
		unhandled("btst #[n],y:(r+n)");
		break;
		}
	case 688: { // btst #[n],x:-(r)
		unhandled("btst #[n],x:-(r)");
		break;
		}
	case 689: { // btst #[n],y:-(r)
		unhandled("btst #[n],y:-(r)");
		break;
		}
	case 690: { // btst #[n],x:[abs]
		unhandled("btst #[n],x:[abs]");
		break;
		}
	case 691: { // btst #[n],y:[abs]
		unhandled("btst #[n],y:[abs]");
		break;
		}
	case 692: { // btst #[n],x:[aa]
		unhandled("btst #[n],x:[aa]");
		break;
		}
	case 693: { // btst #[n],y:[aa]
		unhandled("btst #[n],y:[aa]");
		break;
		}
	case 694: { // btst #[n],x:[pp]
		unhandled("btst #[n],x:[pp]");
		break;
		}
	case 695: { // btst #[n],y:[pp]
		unhandled("btst #[n],y:[pp]");
		break;
		}
	case 696: { // btst #[n],x:[qq]
		unhandled("btst #[n],x:[qq]");
		break;
		}
	case 697: { // btst #[n],y:[qq]
		unhandled("btst #[n],y:[qq]");
		break;
		}
	case 698: { // btst #[n],x0
		unhandled("btst #[n],x0");
		break;
		}
	case 699: { // btst #[n],x1
		unhandled("btst #[n],x1");
		break;
		}
	case 700: { // btst #[n],y0
		unhandled("btst #[n],y0");
		break;
		}
	case 701: { // btst #[n],y1
		unhandled("btst #[n],y1");
		break;
		}
	case 702: { // btst #[n],a0
		unhandled("btst #[n],a0");
		break;
		}
	case 703: { // btst #[n],b0
		unhandled("btst #[n],b0");
		break;
		}
	case 704: { // btst #[n],a2
		unhandled("btst #[n],a2");
		break;
		}
	case 705: { // btst #[n],b2
		unhandled("btst #[n],b2");
		break;
		}
	case 706: { // btst #[n],a1
		unhandled("btst #[n],a1");
		break;
		}
	case 707: { // btst #[n],b1
		unhandled("btst #[n],b1");
		break;
		}
	case 708: { // btst #[n],a
		unhandled("btst #[n],a");
		break;
		}
	case 709: { // btst #[n],b
		unhandled("btst #[n],b");
		break;
		}
	case 710: { // btst #[n],r
		unhandled("btst #[n],r");
		break;
		}
	case 711: { // btst #[n],n
		unhandled("btst #[n],n");
		break;
		}
	case 712: { // btst #[n],m
		unhandled("btst #[n],m");
		break;
		}
	case 713: { // btst #[n],ep
		unhandled("btst #[n],ep");
		break;
		}
	case 714: { // btst #[n],vba
		unhandled("btst #[n],vba");
		break;
		}
	case 715: { // btst #[n],sc
		unhandled("btst #[n],sc");
		break;
		}
	case 716: { // btst #[n],sz
		unhandled("btst #[n],sz");
		break;
		}
	case 717: { // btst #[n],sr
		unhandled("btst #[n],sr");
		break;
		}
	case 718: { // btst #[n],omr
		unhandled("btst #[n],omr");
		break;
		}
	case 719: { // btst #[n],sp
		unhandled("btst #[n],sp");
		break;
		}
	case 720: { // btst #[n],ssh
		unhandled("btst #[n],ssh");
		break;
		}
	case 721: { // btst #[n],ssl
		unhandled("btst #[n],ssl");
		break;
		}
	case 722: { // btst #[n],la
		unhandled("btst #[n],la");
		break;
		}
	case 723: { // btst #[n],lc
		unhandled("btst #[n],lc");
		break;
		}
	case 724: { // clb a,a
		unhandled("clb a,a");
		break;
		}
	case 725: { // clb a,b
		unhandled("clb a,b");
		break;
		}
	case 726: { // clb b,a
		unhandled("clb b,a");
		break;
		}
	case 727: { // clb b,b
		unhandled("clb b,b");
		break;
		}
	case 728: { // cmp #[i],a
		unhandled("cmp #[i],a");
		break;
		}
	case 729: { // cmp #[i],b
		unhandled("cmp #[i],b");
		break;
		}
	case 730: { // cmp #[i],a
		unhandled("cmp #[i],a");
		break;
		}
	case 731: { // cmp #[i],b
		unhandled("cmp #[i],b");
		break;
		}
	case 732: { // cmpu a1,a
		unhandled("cmpu a1,a");
		break;
		}
	case 733: { // cmpu a1,b
		unhandled("cmpu a1,b");
		break;
		}
	case 734: { // cmpu b1,a
		unhandled("cmpu b1,a");
		break;
		}
	case 735: { // cmpu b1,b
		unhandled("cmpu b1,b");
		break;
		}
	case 736: { // cmpu x0,a
		unhandled("cmpu x0,a");
		break;
		}
	case 737: { // cmpu x0,b
		unhandled("cmpu x0,b");
		break;
		}
	case 738: { // cmpu y0,a
		unhandled("cmpu y0,a");
		break;
		}
	case 739: { // cmpu y0,b
		unhandled("cmpu y0,b");
		break;
		}
	case 740: { // cmpu x1,a
		unhandled("cmpu x1,a");
		break;
		}
	case 741: { // cmpu x1,b
		unhandled("cmpu x1,b");
		break;
		}
	case 742: { // cmpu y1,a
		unhandled("cmpu y1,a");
		break;
		}
	case 743: { // cmpu y1,b
		unhandled("cmpu y1,b");
		break;
		}
	case 744: { // debug
		unhandled("debug");
		break;
		}
	case 745: { // debugcc
		unhandled("debugcc");
		break;
		}
	case 746: { // debugge
		unhandled("debugge");
		break;
		}
	case 747: { // debugne
		unhandled("debugne");
		break;
		}
	case 748: { // debugpl
		unhandled("debugpl");
		break;
		}
	case 749: { // debugnn
		unhandled("debugnn");
		break;
		}
	case 750: { // debugec
		unhandled("debugec");
		break;
		}
	case 751: { // debuglc
		unhandled("debuglc");
		break;
		}
	case 752: { // debuggt
		unhandled("debuggt");
		break;
		}
	case 753: { // debugcs
		unhandled("debugcs");
		break;
		}
	case 754: { // debuglt
		unhandled("debuglt");
		break;
		}
	case 755: { // debugeq
		unhandled("debugeq");
		break;
		}
	case 756: { // debugmi
		unhandled("debugmi");
		break;
		}
	case 757: { // debugnr
		unhandled("debugnr");
		break;
		}
	case 758: { // debuges
		unhandled("debuges");
		break;
		}
	case 759: { // debugls
		unhandled("debugls");
		break;
		}
	case 760: { // debugle
		unhandled("debugle");
		break;
		}
	case 761: { // dec a
		unhandled("dec a");
		break;
		}
	case 762: { // dec b
		unhandled("dec b");
		break;
		}
	case 763: { // div x0,a
		unhandled("div x0,a");
		break;
		}
	case 764: { // div x0,b
		unhandled("div x0,b");
		break;
		}
	case 765: { // div y0,a
		unhandled("div y0,a");
		break;
		}
	case 766: { // div y0,b
		unhandled("div y0,b");
		break;
		}
	case 767: { // div x1,a
		unhandled("div x1,a");
		break;
		}
	case 768: { // div x1,b
		unhandled("div x1,b");
		break;
		}
	case 769: { // div y1,a
		unhandled("div y1,a");
		break;
		}
	case 770: { // div y1,b
		unhandled("div y1,b");
		break;
		}
	case 771: { // dmacss +x0,x0,a
		unhandled("dmacss +x0,x0,a");
		break;
		}
	case 772: { // dmacss +y0,y0,a
		unhandled("dmacss +y0,y0,a");
		break;
		}
	case 773: { // dmacss +x1,x0,a
		unhandled("dmacss +x1,x0,a");
		break;
		}
	case 774: { // dmacss +y1,y0,a
		unhandled("dmacss +y1,y0,a");
		break;
		}
	case 775: { // dmacss +x1,x1,a
		unhandled("dmacss +x1,x1,a");
		break;
		}
	case 776: { // dmacss +y1,y1,a
		unhandled("dmacss +y1,y1,a");
		break;
		}
	case 777: { // dmacss +x0,x1,a
		unhandled("dmacss +x0,x1,a");
		break;
		}
	case 778: { // dmacss +y0,y1,a
		unhandled("dmacss +y0,y1,a");
		break;
		}
	case 779: { // dmacss +x0,y1,a
		unhandled("dmacss +x0,y1,a");
		break;
		}
	case 780: { // dmacss +y0,x0,a
		unhandled("dmacss +y0,x0,a");
		break;
		}
	case 781: { // dmacss +x1,y0,a
		unhandled("dmacss +x1,y0,a");
		break;
		}
	case 782: { // dmacss +y1,x1,a
		unhandled("dmacss +y1,x1,a");
		break;
		}
	case 783: { // dmacss +y1,x0,a
		unhandled("dmacss +y1,x0,a");
		break;
		}
	case 784: { // dmacss +x0,y0,a
		unhandled("dmacss +x0,y0,a");
		break;
		}
	case 785: { // dmacss +y0,x1,a
		unhandled("dmacss +y0,x1,a");
		break;
		}
	case 786: { // dmacss +x1,y1,a
		unhandled("dmacss +x1,y1,a");
		break;
		}
	case 787: { // dmacss -x0,x0,a
		unhandled("dmacss -x0,x0,a");
		break;
		}
	case 788: { // dmacss -y0,y0,a
		unhandled("dmacss -y0,y0,a");
		break;
		}
	case 789: { // dmacss -x1,x0,a
		unhandled("dmacss -x1,x0,a");
		break;
		}
	case 790: { // dmacss -y1,y0,a
		unhandled("dmacss -y1,y0,a");
		break;
		}
	case 791: { // dmacss -x1,x1,a
		unhandled("dmacss -x1,x1,a");
		break;
		}
	case 792: { // dmacss -y1,y1,a
		unhandled("dmacss -y1,y1,a");
		break;
		}
	case 793: { // dmacss -x0,x1,a
		unhandled("dmacss -x0,x1,a");
		break;
		}
	case 794: { // dmacss -y0,y1,a
		unhandled("dmacss -y0,y1,a");
		break;
		}
	case 795: { // dmacss -x0,y1,a
		unhandled("dmacss -x0,y1,a");
		break;
		}
	case 796: { // dmacss -y0,x0,a
		unhandled("dmacss -y0,x0,a");
		break;
		}
	case 797: { // dmacss -x1,y0,a
		unhandled("dmacss -x1,y0,a");
		break;
		}
	case 798: { // dmacss -y1,x1,a
		unhandled("dmacss -y1,x1,a");
		break;
		}
	case 799: { // dmacss -y1,x0,a
		unhandled("dmacss -y1,x0,a");
		break;
		}
	case 800: { // dmacss -x0,y0,a
		unhandled("dmacss -x0,y0,a");
		break;
		}
	case 801: { // dmacss -y0,x1,a
		unhandled("dmacss -y0,x1,a");
		break;
		}
	case 802: { // dmacss -x1,y1,a
		unhandled("dmacss -x1,y1,a");
		break;
		}
	case 803: { // dmacss +x0,x0,b
		unhandled("dmacss +x0,x0,b");
		break;
		}
	case 804: { // dmacss +y0,y0,b
		unhandled("dmacss +y0,y0,b");
		break;
		}
	case 805: { // dmacss +x1,x0,b
		unhandled("dmacss +x1,x0,b");
		break;
		}
	case 806: { // dmacss +y1,y0,b
		unhandled("dmacss +y1,y0,b");
		break;
		}
	case 807: { // dmacss +x1,x1,b
		unhandled("dmacss +x1,x1,b");
		break;
		}
	case 808: { // dmacss +y1,y1,b
		unhandled("dmacss +y1,y1,b");
		break;
		}
	case 809: { // dmacss +x0,x1,b
		unhandled("dmacss +x0,x1,b");
		break;
		}
	case 810: { // dmacss +y0,y1,b
		unhandled("dmacss +y0,y1,b");
		break;
		}
	case 811: { // dmacss +x0,y1,b
		unhandled("dmacss +x0,y1,b");
		break;
		}
	case 812: { // dmacss +y0,x0,b
		unhandled("dmacss +y0,x0,b");
		break;
		}
	case 813: { // dmacss +x1,y0,b
		unhandled("dmacss +x1,y0,b");
		break;
		}
	case 814: { // dmacss +y1,x1,b
		unhandled("dmacss +y1,x1,b");
		break;
		}
	case 815: { // dmacss +y1,x0,b
		unhandled("dmacss +y1,x0,b");
		break;
		}
	case 816: { // dmacss +x0,y0,b
		unhandled("dmacss +x0,y0,b");
		break;
		}
	case 817: { // dmacss +y0,x1,b
		unhandled("dmacss +y0,x1,b");
		break;
		}
	case 818: { // dmacss +x1,y1,b
		unhandled("dmacss +x1,y1,b");
		break;
		}
	case 819: { // dmacss -x0,x0,b
		unhandled("dmacss -x0,x0,b");
		break;
		}
	case 820: { // dmacss -y0,y0,b
		unhandled("dmacss -y0,y0,b");
		break;
		}
	case 821: { // dmacss -x1,x0,b
		unhandled("dmacss -x1,x0,b");
		break;
		}
	case 822: { // dmacss -y1,y0,b
		unhandled("dmacss -y1,y0,b");
		break;
		}
	case 823: { // dmacss -x1,x1,b
		unhandled("dmacss -x1,x1,b");
		break;
		}
	case 824: { // dmacss -y1,y1,b
		unhandled("dmacss -y1,y1,b");
		break;
		}
	case 825: { // dmacss -x0,x1,b
		unhandled("dmacss -x0,x1,b");
		break;
		}
	case 826: { // dmacss -y0,y1,b
		unhandled("dmacss -y0,y1,b");
		break;
		}
	case 827: { // dmacss -x0,y1,b
		unhandled("dmacss -x0,y1,b");
		break;
		}
	case 828: { // dmacss -y0,x0,b
		unhandled("dmacss -y0,x0,b");
		break;
		}
	case 829: { // dmacss -x1,y0,b
		unhandled("dmacss -x1,y0,b");
		break;
		}
	case 830: { // dmacss -y1,x1,b
		unhandled("dmacss -y1,x1,b");
		break;
		}
	case 831: { // dmacss -y1,x0,b
		unhandled("dmacss -y1,x0,b");
		break;
		}
	case 832: { // dmacss -x0,y0,b
		unhandled("dmacss -x0,y0,b");
		break;
		}
	case 833: { // dmacss -y0,x1,b
		unhandled("dmacss -y0,x1,b");
		break;
		}
	case 834: { // dmacss -x1,y1,b
		unhandled("dmacss -x1,y1,b");
		break;
		}
	case 835: { // dmacsu +x0,x0,a
		unhandled("dmacsu +x0,x0,a");
		break;
		}
	case 836: { // dmacsu +y0,y0,a
		unhandled("dmacsu +y0,y0,a");
		break;
		}
	case 837: { // dmacsu +x1,x0,a
		unhandled("dmacsu +x1,x0,a");
		break;
		}
	case 838: { // dmacsu +y1,y0,a
		unhandled("dmacsu +y1,y0,a");
		break;
		}
	case 839: { // dmacsu +x1,x1,a
		unhandled("dmacsu +x1,x1,a");
		break;
		}
	case 840: { // dmacsu +y1,y1,a
		unhandled("dmacsu +y1,y1,a");
		break;
		}
	case 841: { // dmacsu +x0,x1,a
		unhandled("dmacsu +x0,x1,a");
		break;
		}
	case 842: { // dmacsu +y0,y1,a
		unhandled("dmacsu +y0,y1,a");
		break;
		}
	case 843: { // dmacsu +x0,y1,a
		unhandled("dmacsu +x0,y1,a");
		break;
		}
	case 844: { // dmacsu +y0,x0,a
		unhandled("dmacsu +y0,x0,a");
		break;
		}
	case 845: { // dmacsu +x1,y0,a
		unhandled("dmacsu +x1,y0,a");
		break;
		}
	case 846: { // dmacsu +y1,x1,a
		unhandled("dmacsu +y1,x1,a");
		break;
		}
	case 847: { // dmacsu +y1,x0,a
		unhandled("dmacsu +y1,x0,a");
		break;
		}
	case 848: { // dmacsu +x0,y0,a
		unhandled("dmacsu +x0,y0,a");
		break;
		}
	case 849: { // dmacsu +y0,x1,a
		unhandled("dmacsu +y0,x1,a");
		break;
		}
	case 850: { // dmacsu +x1,y1,a
		unhandled("dmacsu +x1,y1,a");
		break;
		}
	case 851: { // dmacsu -x0,x0,a
		unhandled("dmacsu -x0,x0,a");
		break;
		}
	case 852: { // dmacsu -y0,y0,a
		unhandled("dmacsu -y0,y0,a");
		break;
		}
	case 853: { // dmacsu -x1,x0,a
		unhandled("dmacsu -x1,x0,a");
		break;
		}
	case 854: { // dmacsu -y1,y0,a
		unhandled("dmacsu -y1,y0,a");
		break;
		}
	case 855: { // dmacsu -x1,x1,a
		unhandled("dmacsu -x1,x1,a");
		break;
		}
	case 856: { // dmacsu -y1,y1,a
		unhandled("dmacsu -y1,y1,a");
		break;
		}
	case 857: { // dmacsu -x0,x1,a
		unhandled("dmacsu -x0,x1,a");
		break;
		}
	case 858: { // dmacsu -y0,y1,a
		unhandled("dmacsu -y0,y1,a");
		break;
		}
	case 859: { // dmacsu -x0,y1,a
		unhandled("dmacsu -x0,y1,a");
		break;
		}
	case 860: { // dmacsu -y0,x0,a
		unhandled("dmacsu -y0,x0,a");
		break;
		}
	case 861: { // dmacsu -x1,y0,a
		unhandled("dmacsu -x1,y0,a");
		break;
		}
	case 862: { // dmacsu -y1,x1,a
		unhandled("dmacsu -y1,x1,a");
		break;
		}
	case 863: { // dmacsu -y1,x0,a
		unhandled("dmacsu -y1,x0,a");
		break;
		}
	case 864: { // dmacsu -x0,y0,a
		unhandled("dmacsu -x0,y0,a");
		break;
		}
	case 865: { // dmacsu -y0,x1,a
		unhandled("dmacsu -y0,x1,a");
		break;
		}
	case 866: { // dmacsu -x1,y1,a
		unhandled("dmacsu -x1,y1,a");
		break;
		}
	case 867: { // dmacsu +x0,x0,b
		unhandled("dmacsu +x0,x0,b");
		break;
		}
	case 868: { // dmacsu +y0,y0,b
		unhandled("dmacsu +y0,y0,b");
		break;
		}
	case 869: { // dmacsu +x1,x0,b
		unhandled("dmacsu +x1,x0,b");
		break;
		}
	case 870: { // dmacsu +y1,y0,b
		unhandled("dmacsu +y1,y0,b");
		break;
		}
	case 871: { // dmacsu +x1,x1,b
		unhandled("dmacsu +x1,x1,b");
		break;
		}
	case 872: { // dmacsu +y1,y1,b
		unhandled("dmacsu +y1,y1,b");
		break;
		}
	case 873: { // dmacsu +x0,x1,b
		unhandled("dmacsu +x0,x1,b");
		break;
		}
	case 874: { // dmacsu +y0,y1,b
		unhandled("dmacsu +y0,y1,b");
		break;
		}
	case 875: { // dmacsu +x0,y1,b
		unhandled("dmacsu +x0,y1,b");
		break;
		}
	case 876: { // dmacsu +y0,x0,b
		unhandled("dmacsu +y0,x0,b");
		break;
		}
	case 877: { // dmacsu +x1,y0,b
		unhandled("dmacsu +x1,y0,b");
		break;
		}
	case 878: { // dmacsu +y1,x1,b
		unhandled("dmacsu +y1,x1,b");
		break;
		}
	case 879: { // dmacsu +y1,x0,b
		unhandled("dmacsu +y1,x0,b");
		break;
		}
	case 880: { // dmacsu +x0,y0,b
		unhandled("dmacsu +x0,y0,b");
		break;
		}
	case 881: { // dmacsu +y0,x1,b
		unhandled("dmacsu +y0,x1,b");
		break;
		}
	case 882: { // dmacsu +x1,y1,b
		unhandled("dmacsu +x1,y1,b");
		break;
		}
	case 883: { // dmacsu -x0,x0,b
		unhandled("dmacsu -x0,x0,b");
		break;
		}
	case 884: { // dmacsu -y0,y0,b
		unhandled("dmacsu -y0,y0,b");
		break;
		}
	case 885: { // dmacsu -x1,x0,b
		unhandled("dmacsu -x1,x0,b");
		break;
		}
	case 886: { // dmacsu -y1,y0,b
		unhandled("dmacsu -y1,y0,b");
		break;
		}
	case 887: { // dmacsu -x1,x1,b
		unhandled("dmacsu -x1,x1,b");
		break;
		}
	case 888: { // dmacsu -y1,y1,b
		unhandled("dmacsu -y1,y1,b");
		break;
		}
	case 889: { // dmacsu -x0,x1,b
		unhandled("dmacsu -x0,x1,b");
		break;
		}
	case 890: { // dmacsu -y0,y1,b
		unhandled("dmacsu -y0,y1,b");
		break;
		}
	case 891: { // dmacsu -x0,y1,b
		unhandled("dmacsu -x0,y1,b");
		break;
		}
	case 892: { // dmacsu -y0,x0,b
		unhandled("dmacsu -y0,x0,b");
		break;
		}
	case 893: { // dmacsu -x1,y0,b
		unhandled("dmacsu -x1,y0,b");
		break;
		}
	case 894: { // dmacsu -y1,x1,b
		unhandled("dmacsu -y1,x1,b");
		break;
		}
	case 895: { // dmacsu -y1,x0,b
		unhandled("dmacsu -y1,x0,b");
		break;
		}
	case 896: { // dmacsu -x0,y0,b
		unhandled("dmacsu -x0,y0,b");
		break;
		}
	case 897: { // dmacsu -y0,x1,b
		unhandled("dmacsu -y0,x1,b");
		break;
		}
	case 898: { // dmacsu -x1,y1,b
		unhandled("dmacsu -x1,y1,b");
		break;
		}
	case 899: { // dmacuu +x0,x0,a
		unhandled("dmacuu +x0,x0,a");
		break;
		}
	case 900: { // dmacuu +y0,y0,a
		unhandled("dmacuu +y0,y0,a");
		break;
		}
	case 901: { // dmacuu +x1,x0,a
		unhandled("dmacuu +x1,x0,a");
		break;
		}
	case 902: { // dmacuu +y1,y0,a
		unhandled("dmacuu +y1,y0,a");
		break;
		}
	case 903: { // dmacuu +x1,x1,a
		unhandled("dmacuu +x1,x1,a");
		break;
		}
	case 904: { // dmacuu +y1,y1,a
		unhandled("dmacuu +y1,y1,a");
		break;
		}
	case 905: { // dmacuu +x0,x1,a
		unhandled("dmacuu +x0,x1,a");
		break;
		}
	case 906: { // dmacuu +y0,y1,a
		unhandled("dmacuu +y0,y1,a");
		break;
		}
	case 907: { // dmacuu +x0,y1,a
		unhandled("dmacuu +x0,y1,a");
		break;
		}
	case 908: { // dmacuu +y0,x0,a
		unhandled("dmacuu +y0,x0,a");
		break;
		}
	case 909: { // dmacuu +x1,y0,a
		unhandled("dmacuu +x1,y0,a");
		break;
		}
	case 910: { // dmacuu +y1,x1,a
		unhandled("dmacuu +y1,x1,a");
		break;
		}
	case 911: { // dmacuu +y1,x0,a
		unhandled("dmacuu +y1,x0,a");
		break;
		}
	case 912: { // dmacuu +x0,y0,a
		unhandled("dmacuu +x0,y0,a");
		break;
		}
	case 913: { // dmacuu +y0,x1,a
		unhandled("dmacuu +y0,x1,a");
		break;
		}
	case 914: { // dmacuu +x1,y1,a
		unhandled("dmacuu +x1,y1,a");
		break;
		}
	case 915: { // dmacuu -x0,x0,a
		unhandled("dmacuu -x0,x0,a");
		break;
		}
	case 916: { // dmacuu -y0,y0,a
		unhandled("dmacuu -y0,y0,a");
		break;
		}
	case 917: { // dmacuu -x1,x0,a
		unhandled("dmacuu -x1,x0,a");
		break;
		}
	case 918: { // dmacuu -y1,y0,a
		unhandled("dmacuu -y1,y0,a");
		break;
		}
	case 919: { // dmacuu -x1,x1,a
		unhandled("dmacuu -x1,x1,a");
		break;
		}
	case 920: { // dmacuu -y1,y1,a
		unhandled("dmacuu -y1,y1,a");
		break;
		}
	case 921: { // dmacuu -x0,x1,a
		unhandled("dmacuu -x0,x1,a");
		break;
		}
	case 922: { // dmacuu -y0,y1,a
		unhandled("dmacuu -y0,y1,a");
		break;
		}
	case 923: { // dmacuu -x0,y1,a
		unhandled("dmacuu -x0,y1,a");
		break;
		}
	case 924: { // dmacuu -y0,x0,a
		unhandled("dmacuu -y0,x0,a");
		break;
		}
	case 925: { // dmacuu -x1,y0,a
		unhandled("dmacuu -x1,y0,a");
		break;
		}
	case 926: { // dmacuu -y1,x1,a
		unhandled("dmacuu -y1,x1,a");
		break;
		}
	case 927: { // dmacuu -y1,x0,a
		unhandled("dmacuu -y1,x0,a");
		break;
		}
	case 928: { // dmacuu -x0,y0,a
		unhandled("dmacuu -x0,y0,a");
		break;
		}
	case 929: { // dmacuu -y0,x1,a
		unhandled("dmacuu -y0,x1,a");
		break;
		}
	case 930: { // dmacuu -x1,y1,a
		unhandled("dmacuu -x1,y1,a");
		break;
		}
	case 931: { // dmacuu +x0,x0,b
		unhandled("dmacuu +x0,x0,b");
		break;
		}
	case 932: { // dmacuu +y0,y0,b
		unhandled("dmacuu +y0,y0,b");
		break;
		}
	case 933: { // dmacuu +x1,x0,b
		unhandled("dmacuu +x1,x0,b");
		break;
		}
	case 934: { // dmacuu +y1,y0,b
		unhandled("dmacuu +y1,y0,b");
		break;
		}
	case 935: { // dmacuu +x1,x1,b
		unhandled("dmacuu +x1,x1,b");
		break;
		}
	case 936: { // dmacuu +y1,y1,b
		unhandled("dmacuu +y1,y1,b");
		break;
		}
	case 937: { // dmacuu +x0,x1,b
		unhandled("dmacuu +x0,x1,b");
		break;
		}
	case 938: { // dmacuu +y0,y1,b
		unhandled("dmacuu +y0,y1,b");
		break;
		}
	case 939: { // dmacuu +x0,y1,b
		unhandled("dmacuu +x0,y1,b");
		break;
		}
	case 940: { // dmacuu +y0,x0,b
		unhandled("dmacuu +y0,x0,b");
		break;
		}
	case 941: { // dmacuu +x1,y0,b
		unhandled("dmacuu +x1,y0,b");
		break;
		}
	case 942: { // dmacuu +y1,x1,b
		unhandled("dmacuu +y1,x1,b");
		break;
		}
	case 943: { // dmacuu +y1,x0,b
		unhandled("dmacuu +y1,x0,b");
		break;
		}
	case 944: { // dmacuu +x0,y0,b
		unhandled("dmacuu +x0,y0,b");
		break;
		}
	case 945: { // dmacuu +y0,x1,b
		unhandled("dmacuu +y0,x1,b");
		break;
		}
	case 946: { // dmacuu +x1,y1,b
		unhandled("dmacuu +x1,y1,b");
		break;
		}
	case 947: { // dmacuu -x0,x0,b
		unhandled("dmacuu -x0,x0,b");
		break;
		}
	case 948: { // dmacuu -y0,y0,b
		unhandled("dmacuu -y0,y0,b");
		break;
		}
	case 949: { // dmacuu -x1,x0,b
		unhandled("dmacuu -x1,x0,b");
		break;
		}
	case 950: { // dmacuu -y1,y0,b
		unhandled("dmacuu -y1,y0,b");
		break;
		}
	case 951: { // dmacuu -x1,x1,b
		unhandled("dmacuu -x1,x1,b");
		break;
		}
	case 952: { // dmacuu -y1,y1,b
		unhandled("dmacuu -y1,y1,b");
		break;
		}
	case 953: { // dmacuu -x0,x1,b
		unhandled("dmacuu -x0,x1,b");
		break;
		}
	case 954: { // dmacuu -y0,y1,b
		unhandled("dmacuu -y0,y1,b");
		break;
		}
	case 955: { // dmacuu -x0,y1,b
		unhandled("dmacuu -x0,y1,b");
		break;
		}
	case 956: { // dmacuu -y0,x0,b
		unhandled("dmacuu -y0,x0,b");
		break;
		}
	case 957: { // dmacuu -x1,y0,b
		unhandled("dmacuu -x1,y0,b");
		break;
		}
	case 958: { // dmacuu -y1,x1,b
		unhandled("dmacuu -y1,x1,b");
		break;
		}
	case 959: { // dmacuu -y1,x0,b
		unhandled("dmacuu -y1,x0,b");
		break;
		}
	case 960: { // dmacuu -x0,y0,b
		unhandled("dmacuu -x0,y0,b");
		break;
		}
	case 961: { // dmacuu -y0,x1,b
		unhandled("dmacuu -y0,x1,b");
		break;
		}
	case 962: { // dmacuu -x1,y1,b
		unhandled("dmacuu -x1,y1,b");
		break;
		}
	case 963: { // do x:(r)-n,[expr]
		unhandled("do x:(r)-n,[expr]");
		break;
		}
	case 964: { // do y:(r)-n,[expr]
		unhandled("do y:(r)-n,[expr]");
		break;
		}
	case 965: { // do x:(r)+n,[expr]
		unhandled("do x:(r)+n,[expr]");
		break;
		}
	case 966: { // do y:(r)+n,[expr]
		unhandled("do y:(r)+n,[expr]");
		break;
		}
	case 967: { // do x:(r)-,[expr]
		unhandled("do x:(r)-,[expr]");
		break;
		}
	case 968: { // do y:(r)-,[expr]
		unhandled("do y:(r)-,[expr]");
		break;
		}
	case 969: { // do x:(r)+,[expr]
		unhandled("do x:(r)+,[expr]");
		break;
		}
	case 970: { // do y:(r)+,[expr]
		unhandled("do y:(r)+,[expr]");
		break;
		}
	case 971: { // do x:(r),[expr]
		unhandled("do x:(r),[expr]");
		break;
		}
	case 972: { // do y:(r),[expr]
		unhandled("do y:(r),[expr]");
		break;
		}
	case 973: { // do x:(r+n),[expr]
		unhandled("do x:(r+n),[expr]");
		break;
		}
	case 974: { // do y:(r+n),[expr]
		unhandled("do y:(r+n),[expr]");
		break;
		}
	case 975: { // do x:-(r),[expr]
		unhandled("do x:-(r),[expr]");
		break;
		}
	case 976: { // do y:-(r),[expr]
		unhandled("do y:-(r),[expr]");
		break;
		}
	case 977: { // do x:[aa],[expr]
		unhandled("do x:[aa],[expr]");
		break;
		}
	case 978: { // do y:[aa],[expr]
		unhandled("do y:[aa],[expr]");
		break;
		}
	case 979: { // do #[i],[expr]
		u32 i = bitswap<12>(opcode, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(i);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 980: { // do x0,[expr]
		u32 s = get_x0();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 981: { // do x1,[expr]
		u32 s = get_x1();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 982: { // do y0,[expr]
		u32 s = get_y0();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 983: { // do y1,[expr]
		u32 s = get_y1();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 984: { // do a0,[expr]
		u32 s = get_a0();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 985: { // do b0,[expr]
		u32 s = get_b0();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 986: { // do a2,[expr]
		u32 s = get_a2();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 987: { // do b2,[expr]
		u32 s = get_b2();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 988: { // do a1,[expr]
		u32 s = get_a1();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 989: { // do b1,[expr]
		u32 s = get_b1();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 990: { // do a,[expr]
		u64 s = get_a();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 991: { // do b,[expr]
		u64 s = get_b();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 992: { // do r,[expr]
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 993: { // do n,[expr]
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 994: { // do m,[expr]
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 995: { // do ep,[expr]
		u32 s = get_ep();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 996: { // do vba,[expr]
		u32 s = get_vba();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 997: { // do sc,[expr]
		u32 s = get_sc();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 998: { // do sz,[expr]
		u32 s = get_sz();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 999: { // do sr,[expr]
		u32 s = get_sr();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1000: { // do omr,[expr]
		u32 s = get_omr();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1001: { // do sp,[expr]
		u32 s = get_sp();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1002: { // do ssl,[expr]
		u32 s = get_ssl();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1003: { // do la,[expr]
		u32 s = get_la();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1004: { // do lc,[expr]
		u32 s = get_lc();
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1005: { // do forever,[expr]
		u32 expr = exv;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		m_emr |= EMR_FV;
		break;
		}
	case 1006: { // dor x:(r)-n,[expr]
		unhandled("dor x:(r)-n,[expr]");
		break;
		}
	case 1007: { // dor y:(r)-n,[expr]
		unhandled("dor y:(r)-n,[expr]");
		break;
		}
	case 1008: { // dor x:(r)+n,[expr]
		unhandled("dor x:(r)+n,[expr]");
		break;
		}
	case 1009: { // dor y:(r)+n,[expr]
		unhandled("dor y:(r)+n,[expr]");
		break;
		}
	case 1010: { // dor x:(r)-,[expr]
		unhandled("dor x:(r)-,[expr]");
		break;
		}
	case 1011: { // dor y:(r)-,[expr]
		unhandled("dor y:(r)-,[expr]");
		break;
		}
	case 1012: { // dor x:(r)+,[expr]
		unhandled("dor x:(r)+,[expr]");
		break;
		}
	case 1013: { // dor y:(r)+,[expr]
		unhandled("dor y:(r)+,[expr]");
		break;
		}
	case 1014: { // dor x:(r),[expr]
		unhandled("dor x:(r),[expr]");
		break;
		}
	case 1015: { // dor y:(r),[expr]
		unhandled("dor y:(r),[expr]");
		break;
		}
	case 1016: { // dor x:(r+n),[expr]
		unhandled("dor x:(r+n),[expr]");
		break;
		}
	case 1017: { // dor y:(r+n),[expr]
		unhandled("dor y:(r+n),[expr]");
		break;
		}
	case 1018: { // dor x:-(r),[expr]
		unhandled("dor x:-(r),[expr]");
		break;
		}
	case 1019: { // dor y:-(r),[expr]
		unhandled("dor y:-(r),[expr]");
		break;
		}
	case 1020: { // dor x:[aa],[expr]
		unhandled("dor x:[aa],[expr]");
		break;
		}
	case 1021: { // dor y:[aa],[expr]
		unhandled("dor y:[aa],[expr]");
		break;
		}
	case 1022: { // dor #[i],[expr]
		u32 i = bitswap<12>(opcode, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(i);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1023: { // dor x0,[expr]
		u32 s = get_x0();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1024: { // dor x1,[expr]
		u32 s = get_x1();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1025: { // dor y0,[expr]
		u32 s = get_y0();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1026: { // dor y1,[expr]
		u32 s = get_y1();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1027: { // dor a0,[expr]
		u32 s = get_a0();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1028: { // dor b0,[expr]
		u32 s = get_b0();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1029: { // dor a2,[expr]
		u32 s = get_a2();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1030: { // dor b2,[expr]
		u32 s = get_b2();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1031: { // dor a1,[expr]
		u32 s = get_a1();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1032: { // dor b1,[expr]
		u32 s = get_b1();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1033: { // dor a,[expr]
		u64 s = get_a();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1034: { // dor b,[expr]
		u64 s = get_b();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1035: { // dor r,[expr]
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1036: { // dor n,[expr]
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1037: { // dor m,[expr]
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1038: { // dor ep,[expr]
		u32 s = get_ep();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1039: { // dor vba,[expr]
		u32 s = get_vba();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1040: { // dor sc,[expr]
		u32 s = get_sc();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1041: { // dor sz,[expr]
		u32 s = get_sz();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1042: { // dor sr,[expr]
		u32 s = get_sr();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1043: { // dor omr,[expr]
		u32 s = get_omr();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1044: { // dor sp,[expr]
		u32 s = get_sp();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1045: { // dor ssl,[expr]
		u32 s = get_ssl();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1046: { // dor la,[expr]
		u32 s = get_la();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1047: { // dor lc,[expr]
		u32 s = get_lc();
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		set_lc(s);
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		break;
		}
	case 1048: { // dor forever,[expr]
		u32 expr = (m_pc+exv) & 0xffffff;
		inc_sp();
		set_ssh(get_la());
		set_ssl(get_lc());
		inc_sp();
		set_ssh(m_pc+2);
		set_ssl(get_sr());
		set_la(expr-1);
		m_mr |= MR_LF;
		m_emr |= EMR_FV;
		break;
		}
	case 1049: { // enddo
		unhandled("enddo");
		break;
		}
	case 1050: { // eor #[i],a
		unhandled("eor #[i],a");
		break;
		}
	case 1051: { // eor #[i],b
		unhandled("eor #[i],b");
		break;
		}
	case 1052: { // eor #[i],a
		unhandled("eor #[i],a");
		break;
		}
	case 1053: { // eor #[i],b
		unhandled("eor #[i],b");
		break;
		}
	case 1054: { // extract a1,a,d
		unhandled("extract a1,a,d");
		break;
		}
	case 1055: { // extract a1,b,d
		unhandled("extract a1,b,d");
		break;
		}
	case 1056: { // extract b1,a,d
		unhandled("extract b1,a,d");
		break;
		}
	case 1057: { // extract b1,b,d
		unhandled("extract b1,b,d");
		break;
		}
	case 1058: { // extract x0,a,d
		unhandled("extract x0,a,d");
		break;
		}
	case 1059: { // extract x0,b,d
		unhandled("extract x0,b,d");
		break;
		}
	case 1060: { // extract y0,a,d
		unhandled("extract y0,a,d");
		break;
		}
	case 1061: { // extract y0,b,d
		unhandled("extract y0,b,d");
		break;
		}
	case 1062: { // extract x1,a,d
		unhandled("extract x1,a,d");
		break;
		}
	case 1063: { // extract x1,b,d
		unhandled("extract x1,b,d");
		break;
		}
	case 1064: { // extract y1,a,d
		unhandled("extract y1,a,d");
		break;
		}
	case 1065: { // extract y1,b,d
		unhandled("extract y1,b,d");
		break;
		}
	case 1066: { // extract #[co],a,a
		unhandled("extract #[co],a,a");
		break;
		}
	case 1067: { // extract #[co],a,b
		unhandled("extract #[co],a,b");
		break;
		}
	case 1068: { // extract #[co],b,a
		unhandled("extract #[co],b,a");
		break;
		}
	case 1069: { // extract #[co],b,b
		unhandled("extract #[co],b,b");
		break;
		}
	case 1070: { // extractu a1,a,d
		unhandled("extractu a1,a,d");
		break;
		}
	case 1071: { // extractu a1,b,d
		unhandled("extractu a1,b,d");
		break;
		}
	case 1072: { // extractu b1,a,d
		unhandled("extractu b1,a,d");
		break;
		}
	case 1073: { // extractu b1,b,d
		unhandled("extractu b1,b,d");
		break;
		}
	case 1074: { // extractu x0,a,d
		unhandled("extractu x0,a,d");
		break;
		}
	case 1075: { // extractu x0,b,d
		unhandled("extractu x0,b,d");
		break;
		}
	case 1076: { // extractu y0,a,d
		unhandled("extractu y0,a,d");
		break;
		}
	case 1077: { // extractu y0,b,d
		unhandled("extractu y0,b,d");
		break;
		}
	case 1078: { // extractu x1,a,d
		unhandled("extractu x1,a,d");
		break;
		}
	case 1079: { // extractu x1,b,d
		unhandled("extractu x1,b,d");
		break;
		}
	case 1080: { // extractu y1,a,d
		unhandled("extractu y1,a,d");
		break;
		}
	case 1081: { // extractu y1,b,d
		unhandled("extractu y1,b,d");
		break;
		}
	case 1082: { // extractu #[co],a,a
		unhandled("extractu #[co],a,a");
		break;
		}
	case 1083: { // extractu #[co],a,b
		unhandled("extractu #[co],a,b");
		break;
		}
	case 1084: { // extractu #[co],b,a
		unhandled("extractu #[co],b,a");
		break;
		}
	case 1085: { // extractu #[co],b,b
		unhandled("extractu #[co],b,b");
		break;
		}
	case 1086: { // illegal
		unhandled("illegal");
		break;
		}
	case 1087: { // inc a
		unhandled("inc a");
		break;
		}
	case 1088: { // inc b
		unhandled("inc b");
		break;
		}
	case 1089: { // insert a1,a0,d
		unhandled("insert a1,a0,d");
		break;
		}
	case 1090: { // insert a1,b0,d
		unhandled("insert a1,b0,d");
		break;
		}
	case 1091: { // insert a1,x0,d
		unhandled("insert a1,x0,d");
		break;
		}
	case 1092: { // insert a1,y0,d
		unhandled("insert a1,y0,d");
		break;
		}
	case 1093: { // insert a1,x1,d
		unhandled("insert a1,x1,d");
		break;
		}
	case 1094: { // insert a1,y1,d
		unhandled("insert a1,y1,d");
		break;
		}
	case 1095: { // insert b1,a0,d
		unhandled("insert b1,a0,d");
		break;
		}
	case 1096: { // insert b1,b0,d
		unhandled("insert b1,b0,d");
		break;
		}
	case 1097: { // insert b1,x0,d
		unhandled("insert b1,x0,d");
		break;
		}
	case 1098: { // insert b1,y0,d
		unhandled("insert b1,y0,d");
		break;
		}
	case 1099: { // insert b1,x1,d
		unhandled("insert b1,x1,d");
		break;
		}
	case 1100: { // insert b1,y1,d
		unhandled("insert b1,y1,d");
		break;
		}
	case 1101: { // insert x0,a0,d
		unhandled("insert x0,a0,d");
		break;
		}
	case 1102: { // insert x0,b0,d
		unhandled("insert x0,b0,d");
		break;
		}
	case 1103: { // insert x0,x0,d
		unhandled("insert x0,x0,d");
		break;
		}
	case 1104: { // insert x0,y0,d
		unhandled("insert x0,y0,d");
		break;
		}
	case 1105: { // insert x0,x1,d
		unhandled("insert x0,x1,d");
		break;
		}
	case 1106: { // insert x0,y1,d
		unhandled("insert x0,y1,d");
		break;
		}
	case 1107: { // insert y0,a0,d
		unhandled("insert y0,a0,d");
		break;
		}
	case 1108: { // insert y0,b0,d
		unhandled("insert y0,b0,d");
		break;
		}
	case 1109: { // insert y0,x0,d
		unhandled("insert y0,x0,d");
		break;
		}
	case 1110: { // insert y0,y0,d
		unhandled("insert y0,y0,d");
		break;
		}
	case 1111: { // insert y0,x1,d
		unhandled("insert y0,x1,d");
		break;
		}
	case 1112: { // insert y0,y1,d
		unhandled("insert y0,y1,d");
		break;
		}
	case 1113: { // insert x1,a0,d
		unhandled("insert x1,a0,d");
		break;
		}
	case 1114: { // insert x1,b0,d
		unhandled("insert x1,b0,d");
		break;
		}
	case 1115: { // insert x1,x0,d
		unhandled("insert x1,x0,d");
		break;
		}
	case 1116: { // insert x1,y0,d
		unhandled("insert x1,y0,d");
		break;
		}
	case 1117: { // insert x1,x1,d
		unhandled("insert x1,x1,d");
		break;
		}
	case 1118: { // insert x1,y1,d
		unhandled("insert x1,y1,d");
		break;
		}
	case 1119: { // insert y1,a0,d
		unhandled("insert y1,a0,d");
		break;
		}
	case 1120: { // insert y1,b0,d
		unhandled("insert y1,b0,d");
		break;
		}
	case 1121: { // insert y1,x0,d
		unhandled("insert y1,x0,d");
		break;
		}
	case 1122: { // insert y1,y0,d
		unhandled("insert y1,y0,d");
		break;
		}
	case 1123: { // insert y1,x1,d
		unhandled("insert y1,x1,d");
		break;
		}
	case 1124: { // insert y1,y1,d
		unhandled("insert y1,y1,d");
		break;
		}
	case 1125: { // insert #[co],a0,a
		unhandled("insert #[co],a0,a");
		break;
		}
	case 1126: { // insert #[co],a0,b
		unhandled("insert #[co],a0,b");
		break;
		}
	case 1127: { // insert #[co],b0,a
		unhandled("insert #[co],b0,a");
		break;
		}
	case 1128: { // insert #[co],b0,b
		unhandled("insert #[co],b0,b");
		break;
		}
	case 1129: { // insert #[co],x0,a
		unhandled("insert #[co],x0,a");
		break;
		}
	case 1130: { // insert #[co],x0,b
		unhandled("insert #[co],x0,b");
		break;
		}
	case 1131: { // insert #[co],y0,a
		unhandled("insert #[co],y0,a");
		break;
		}
	case 1132: { // insert #[co],y0,b
		unhandled("insert #[co],y0,b");
		break;
		}
	case 1133: { // insert #[co],x1,a
		unhandled("insert #[co],x1,a");
		break;
		}
	case 1134: { // insert #[co],x1,b
		unhandled("insert #[co],x1,b");
		break;
		}
	case 1135: { // insert #[co],y1,a
		unhandled("insert #[co],y1,a");
		break;
		}
	case 1136: { // insert #[co],y1,b
		unhandled("insert #[co],y1,b");
		break;
		}
	case 1137: { // jcc [x]
		unhandled("jcc [x]");
		break;
		}
	case 1138: { // jge [x]
		unhandled("jge [x]");
		break;
		}
	case 1139: { // jne [x]
		unhandled("jne [x]");
		break;
		}
	case 1140: { // jpl [x]
		unhandled("jpl [x]");
		break;
		}
	case 1141: { // jnn [x]
		unhandled("jnn [x]");
		break;
		}
	case 1142: { // jec [x]
		unhandled("jec [x]");
		break;
		}
	case 1143: { // jlc [x]
		unhandled("jlc [x]");
		break;
		}
	case 1144: { // jgt [x]
		unhandled("jgt [x]");
		break;
		}
	case 1145: { // jcs [x]
		unhandled("jcs [x]");
		break;
		}
	case 1146: { // jlt [x]
		unhandled("jlt [x]");
		break;
		}
	case 1147: { // jeq [x]
		unhandled("jeq [x]");
		break;
		}
	case 1148: { // jmi [x]
		unhandled("jmi [x]");
		break;
		}
	case 1149: { // jnr [x]
		unhandled("jnr [x]");
		break;
		}
	case 1150: { // jes [x]
		unhandled("jes [x]");
		break;
		}
	case 1151: { // jls [x]
		unhandled("jls [x]");
		break;
		}
	case 1152: { // jle [x]
		unhandled("jle [x]");
		break;
		}
	case 1153: { // jcc (r)-n
		unhandled("jcc (r)-n");
		break;
		}
	case 1154: { // jge (r)-n
		unhandled("jge (r)-n");
		break;
		}
	case 1155: { // jne (r)-n
		unhandled("jne (r)-n");
		break;
		}
	case 1156: { // jpl (r)-n
		unhandled("jpl (r)-n");
		break;
		}
	case 1157: { // jnn (r)-n
		unhandled("jnn (r)-n");
		break;
		}
	case 1158: { // jec (r)-n
		unhandled("jec (r)-n");
		break;
		}
	case 1159: { // jlc (r)-n
		unhandled("jlc (r)-n");
		break;
		}
	case 1160: { // jgt (r)-n
		unhandled("jgt (r)-n");
		break;
		}
	case 1161: { // jcs (r)-n
		unhandled("jcs (r)-n");
		break;
		}
	case 1162: { // jlt (r)-n
		unhandled("jlt (r)-n");
		break;
		}
	case 1163: { // jeq (r)-n
		unhandled("jeq (r)-n");
		break;
		}
	case 1164: { // jmi (r)-n
		unhandled("jmi (r)-n");
		break;
		}
	case 1165: { // jnr (r)-n
		unhandled("jnr (r)-n");
		break;
		}
	case 1166: { // jes (r)-n
		unhandled("jes (r)-n");
		break;
		}
	case 1167: { // jls (r)-n
		unhandled("jls (r)-n");
		break;
		}
	case 1168: { // jle (r)-n
		unhandled("jle (r)-n");
		break;
		}
	case 1169: { // jcc (r)+n
		unhandled("jcc (r)+n");
		break;
		}
	case 1170: { // jge (r)+n
		unhandled("jge (r)+n");
		break;
		}
	case 1171: { // jne (r)+n
		unhandled("jne (r)+n");
		break;
		}
	case 1172: { // jpl (r)+n
		unhandled("jpl (r)+n");
		break;
		}
	case 1173: { // jnn (r)+n
		unhandled("jnn (r)+n");
		break;
		}
	case 1174: { // jec (r)+n
		unhandled("jec (r)+n");
		break;
		}
	case 1175: { // jlc (r)+n
		unhandled("jlc (r)+n");
		break;
		}
	case 1176: { // jgt (r)+n
		unhandled("jgt (r)+n");
		break;
		}
	case 1177: { // jcs (r)+n
		unhandled("jcs (r)+n");
		break;
		}
	case 1178: { // jlt (r)+n
		unhandled("jlt (r)+n");
		break;
		}
	case 1179: { // jeq (r)+n
		unhandled("jeq (r)+n");
		break;
		}
	case 1180: { // jmi (r)+n
		unhandled("jmi (r)+n");
		break;
		}
	case 1181: { // jnr (r)+n
		unhandled("jnr (r)+n");
		break;
		}
	case 1182: { // jes (r)+n
		unhandled("jes (r)+n");
		break;
		}
	case 1183: { // jls (r)+n
		unhandled("jls (r)+n");
		break;
		}
	case 1184: { // jle (r)+n
		unhandled("jle (r)+n");
		break;
		}
	case 1185: { // jcc (r)-
		unhandled("jcc (r)-");
		break;
		}
	case 1186: { // jge (r)-
		unhandled("jge (r)-");
		break;
		}
	case 1187: { // jne (r)-
		unhandled("jne (r)-");
		break;
		}
	case 1188: { // jpl (r)-
		unhandled("jpl (r)-");
		break;
		}
	case 1189: { // jnn (r)-
		unhandled("jnn (r)-");
		break;
		}
	case 1190: { // jec (r)-
		unhandled("jec (r)-");
		break;
		}
	case 1191: { // jlc (r)-
		unhandled("jlc (r)-");
		break;
		}
	case 1192: { // jgt (r)-
		unhandled("jgt (r)-");
		break;
		}
	case 1193: { // jcs (r)-
		unhandled("jcs (r)-");
		break;
		}
	case 1194: { // jlt (r)-
		unhandled("jlt (r)-");
		break;
		}
	case 1195: { // jeq (r)-
		unhandled("jeq (r)-");
		break;
		}
	case 1196: { // jmi (r)-
		unhandled("jmi (r)-");
		break;
		}
	case 1197: { // jnr (r)-
		unhandled("jnr (r)-");
		break;
		}
	case 1198: { // jes (r)-
		unhandled("jes (r)-");
		break;
		}
	case 1199: { // jls (r)-
		unhandled("jls (r)-");
		break;
		}
	case 1200: { // jle (r)-
		unhandled("jle (r)-");
		break;
		}
	case 1201: { // jcc (r)+
		unhandled("jcc (r)+");
		break;
		}
	case 1202: { // jge (r)+
		unhandled("jge (r)+");
		break;
		}
	case 1203: { // jne (r)+
		unhandled("jne (r)+");
		break;
		}
	case 1204: { // jpl (r)+
		unhandled("jpl (r)+");
		break;
		}
	case 1205: { // jnn (r)+
		unhandled("jnn (r)+");
		break;
		}
	case 1206: { // jec (r)+
		unhandled("jec (r)+");
		break;
		}
	case 1207: { // jlc (r)+
		unhandled("jlc (r)+");
		break;
		}
	case 1208: { // jgt (r)+
		unhandled("jgt (r)+");
		break;
		}
	case 1209: { // jcs (r)+
		unhandled("jcs (r)+");
		break;
		}
	case 1210: { // jlt (r)+
		unhandled("jlt (r)+");
		break;
		}
	case 1211: { // jeq (r)+
		unhandled("jeq (r)+");
		break;
		}
	case 1212: { // jmi (r)+
		unhandled("jmi (r)+");
		break;
		}
	case 1213: { // jnr (r)+
		unhandled("jnr (r)+");
		break;
		}
	case 1214: { // jes (r)+
		unhandled("jes (r)+");
		break;
		}
	case 1215: { // jls (r)+
		unhandled("jls (r)+");
		break;
		}
	case 1216: { // jle (r)+
		unhandled("jle (r)+");
		break;
		}
	case 1217: { // jcc (r)
		unhandled("jcc (r)");
		break;
		}
	case 1218: { // jge (r)
		unhandled("jge (r)");
		break;
		}
	case 1219: { // jne (r)
		unhandled("jne (r)");
		break;
		}
	case 1220: { // jpl (r)
		unhandled("jpl (r)");
		break;
		}
	case 1221: { // jnn (r)
		unhandled("jnn (r)");
		break;
		}
	case 1222: { // jec (r)
		unhandled("jec (r)");
		break;
		}
	case 1223: { // jlc (r)
		unhandled("jlc (r)");
		break;
		}
	case 1224: { // jgt (r)
		unhandled("jgt (r)");
		break;
		}
	case 1225: { // jcs (r)
		unhandled("jcs (r)");
		break;
		}
	case 1226: { // jlt (r)
		unhandled("jlt (r)");
		break;
		}
	case 1227: { // jeq (r)
		unhandled("jeq (r)");
		break;
		}
	case 1228: { // jmi (r)
		unhandled("jmi (r)");
		break;
		}
	case 1229: { // jnr (r)
		unhandled("jnr (r)");
		break;
		}
	case 1230: { // jes (r)
		unhandled("jes (r)");
		break;
		}
	case 1231: { // jls (r)
		unhandled("jls (r)");
		break;
		}
	case 1232: { // jle (r)
		unhandled("jle (r)");
		break;
		}
	case 1233: { // jcc (r+n)
		unhandled("jcc (r+n)");
		break;
		}
	case 1234: { // jge (r+n)
		unhandled("jge (r+n)");
		break;
		}
	case 1235: { // jne (r+n)
		unhandled("jne (r+n)");
		break;
		}
	case 1236: { // jpl (r+n)
		unhandled("jpl (r+n)");
		break;
		}
	case 1237: { // jnn (r+n)
		unhandled("jnn (r+n)");
		break;
		}
	case 1238: { // jec (r+n)
		unhandled("jec (r+n)");
		break;
		}
	case 1239: { // jlc (r+n)
		unhandled("jlc (r+n)");
		break;
		}
	case 1240: { // jgt (r+n)
		unhandled("jgt (r+n)");
		break;
		}
	case 1241: { // jcs (r+n)
		unhandled("jcs (r+n)");
		break;
		}
	case 1242: { // jlt (r+n)
		unhandled("jlt (r+n)");
		break;
		}
	case 1243: { // jeq (r+n)
		unhandled("jeq (r+n)");
		break;
		}
	case 1244: { // jmi (r+n)
		unhandled("jmi (r+n)");
		break;
		}
	case 1245: { // jnr (r+n)
		unhandled("jnr (r+n)");
		break;
		}
	case 1246: { // jes (r+n)
		unhandled("jes (r+n)");
		break;
		}
	case 1247: { // jls (r+n)
		unhandled("jls (r+n)");
		break;
		}
	case 1248: { // jle (r+n)
		unhandled("jle (r+n)");
		break;
		}
	case 1249: { // jcc -(r)
		unhandled("jcc -(r)");
		break;
		}
	case 1250: { // jge -(r)
		unhandled("jge -(r)");
		break;
		}
	case 1251: { // jne -(r)
		unhandled("jne -(r)");
		break;
		}
	case 1252: { // jpl -(r)
		unhandled("jpl -(r)");
		break;
		}
	case 1253: { // jnn -(r)
		unhandled("jnn -(r)");
		break;
		}
	case 1254: { // jec -(r)
		unhandled("jec -(r)");
		break;
		}
	case 1255: { // jlc -(r)
		unhandled("jlc -(r)");
		break;
		}
	case 1256: { // jgt -(r)
		unhandled("jgt -(r)");
		break;
		}
	case 1257: { // jcs -(r)
		unhandled("jcs -(r)");
		break;
		}
	case 1258: { // jlt -(r)
		unhandled("jlt -(r)");
		break;
		}
	case 1259: { // jeq -(r)
		unhandled("jeq -(r)");
		break;
		}
	case 1260: { // jmi -(r)
		unhandled("jmi -(r)");
		break;
		}
	case 1261: { // jnr -(r)
		unhandled("jnr -(r)");
		break;
		}
	case 1262: { // jes -(r)
		unhandled("jes -(r)");
		break;
		}
	case 1263: { // jls -(r)
		unhandled("jls -(r)");
		break;
		}
	case 1264: { // jle -(r)
		unhandled("jle -(r)");
		break;
		}
	case 1265: { // jcc [abs]
		unhandled("jcc [abs]");
		break;
		}
	case 1266: { // jge [abs]
		unhandled("jge [abs]");
		break;
		}
	case 1267: { // jne [abs]
		unhandled("jne [abs]");
		break;
		}
	case 1268: { // jpl [abs]
		unhandled("jpl [abs]");
		break;
		}
	case 1269: { // jnn [abs]
		unhandled("jnn [abs]");
		break;
		}
	case 1270: { // jec [abs]
		unhandled("jec [abs]");
		break;
		}
	case 1271: { // jlc [abs]
		unhandled("jlc [abs]");
		break;
		}
	case 1272: { // jgt [abs]
		unhandled("jgt [abs]");
		break;
		}
	case 1273: { // jcs [abs]
		unhandled("jcs [abs]");
		break;
		}
	case 1274: { // jlt [abs]
		unhandled("jlt [abs]");
		break;
		}
	case 1275: { // jeq [abs]
		unhandled("jeq [abs]");
		break;
		}
	case 1276: { // jmi [abs]
		unhandled("jmi [abs]");
		break;
		}
	case 1277: { // jnr [abs]
		unhandled("jnr [abs]");
		break;
		}
	case 1278: { // jes [abs]
		unhandled("jes [abs]");
		break;
		}
	case 1279: { // jls [abs]
		unhandled("jls [abs]");
		break;
		}
	case 1280: { // jle [abs]
		unhandled("jle [abs]");
		break;
		}
	case 1281: { // jclr #[n],x:(r)-n,[x]
		unhandled("jclr #[n],x:(r)-n,[x]");
		break;
		}
	case 1282: { // jclr #[n],y:(r)-n,[x]
		unhandled("jclr #[n],y:(r)-n,[x]");
		break;
		}
	case 1283: { // jclr #[n],x:(r)+n,[x]
		unhandled("jclr #[n],x:(r)+n,[x]");
		break;
		}
	case 1284: { // jclr #[n],y:(r)+n,[x]
		unhandled("jclr #[n],y:(r)+n,[x]");
		break;
		}
	case 1285: { // jclr #[n],x:(r)-,[x]
		unhandled("jclr #[n],x:(r)-,[x]");
		break;
		}
	case 1286: { // jclr #[n],y:(r)-,[x]
		unhandled("jclr #[n],y:(r)-,[x]");
		break;
		}
	case 1287: { // jclr #[n],x:(r)+,[x]
		unhandled("jclr #[n],x:(r)+,[x]");
		break;
		}
	case 1288: { // jclr #[n],y:(r)+,[x]
		unhandled("jclr #[n],y:(r)+,[x]");
		break;
		}
	case 1289: { // jclr #[n],x:(r),[x]
		unhandled("jclr #[n],x:(r),[x]");
		break;
		}
	case 1290: { // jclr #[n],y:(r),[x]
		unhandled("jclr #[n],y:(r),[x]");
		break;
		}
	case 1291: { // jclr #[n],x:(r+n),[x]
		unhandled("jclr #[n],x:(r+n),[x]");
		break;
		}
	case 1292: { // jclr #[n],y:(r+n),[x]
		unhandled("jclr #[n],y:(r+n),[x]");
		break;
		}
	case 1293: { // jclr #[n],x:-(r),[x]
		unhandled("jclr #[n],x:-(r),[x]");
		break;
		}
	case 1294: { // jclr #[n],y:-(r),[x]
		unhandled("jclr #[n],y:-(r),[x]");
		break;
		}
	case 1295: { // jclr #[n],x:[aa],[x]
		unhandled("jclr #[n],x:[aa],[x]");
		break;
		}
	case 1296: { // jclr #[n],y:[aa],[x]
		unhandled("jclr #[n],y:[aa],[x]");
		break;
		}
	case 1297: { // jclr #[n],x:[pp],[x]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(m_x.read_dword(pp), n))
		m_npc = x;
		break;
		}
	case 1298: { // jclr #[n],y:[pp],[x]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(m_y.read_dword(pp), n))
		m_npc = x;
		break;
		}
	case 1299: { // jclr #[n],x:[qq],[x]
		u32 qq = 0xffff80 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(m_x.read_dword(qq), n))
		m_npc = x;
		break;
		}
	case 1300: { // jclr #[n],y:[qq],[x]
		u32 qq = 0xffff80 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(m_y.read_dword(qq), n))
		m_npc = x;
		break;
		}
	case 1301: { // jclr #[n],x0,[x]
		u32 s = get_x0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1302: { // jclr #[n],x1,[x]
		u32 s = get_x1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1303: { // jclr #[n],y0,[x]
		u32 s = get_y0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1304: { // jclr #[n],y1,[x]
		u32 s = get_y1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1305: { // jclr #[n],a0,[x]
		u32 s = get_a0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1306: { // jclr #[n],b0,[x]
		u32 s = get_b0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1307: { // jclr #[n],a2,[x]
		u32 s = get_a2();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1308: { // jclr #[n],b2,[x]
		u32 s = get_b2();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1309: { // jclr #[n],a1,[x]
		u32 s = get_a1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1310: { // jclr #[n],b1,[x]
		u32 s = get_b1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1311: { // jclr #[n],a,[x]
		u64 s = get_a();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1312: { // jclr #[n],b,[x]
		u64 s = get_b();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1313: { // jclr #[n],r,[x]
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1314: { // jclr #[n],n,[x]
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1315: { // jclr #[n],m,[x]
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1316: { // jclr #[n],ep,[x]
		u32 s = get_ep();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1317: { // jclr #[n],vba,[x]
		u32 s = get_vba();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1318: { // jclr #[n],sc,[x]
		u32 s = get_sc();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1319: { // jclr #[n],sz,[x]
		u32 s = get_sz();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1320: { // jclr #[n],sr,[x]
		u32 s = get_sr();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1321: { // jclr #[n],omr,[x]
		u32 s = get_omr();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1322: { // jclr #[n],sp,[x]
		u32 s = get_sp();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1323: { // jclr #[n],ssh,[x]
		u32 s = get_ssh();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1324: { // jclr #[n],ssl,[x]
		u32 s = get_ssl();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1325: { // jclr #[n],la,[x]
		u32 s = get_la();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1326: { // jclr #[n],lc,[x]
		u32 s = get_lc();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(!BIT(s, n))
		m_npc = x;
		break;
		}
	case 1327: { // jmp (r)-n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -m_n[ea_r]);
		m_npc = ea;
		break;
		}
	case 1328: { // jmp (r)+n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, m_n[ea_r]);
		m_npc = ea;
		break;
		}
	case 1329: { // jmp (r)-
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -1);
		m_npc = ea;
		break;
		}
	case 1330: { // jmp (r)+
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, 1);
		m_npc = ea;
		break;
		}
	case 1331: { // jmp (r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		m_npc = ea;
		break;
		}
	case 1332: { // jmp (r+n)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = calc_add_r(ea_r, m_n[ea_r]);
		m_npc = ea;
		break;
		}
	case 1333: { // jmp -(r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		add_r(ea_r, -1);
		u32 ea = get_r(ea_r);
		m_npc = ea;
		break;
		}
	case 1334: { // jmp [abs]
		u32 abs = exv;
		m_npc = abs;
		break;
		}
	case 1335: { // jmp [x]
		u32 x = BIT(opcode, 0, 12);
		m_npc = x;
		break;
		}
	case 1336: { // jscc [x]
		unhandled("jscc [x]");
		break;
		}
	case 1337: { // jsge [x]
		unhandled("jsge [x]");
		break;
		}
	case 1338: { // jsne [x]
		unhandled("jsne [x]");
		break;
		}
	case 1339: { // jspl [x]
		unhandled("jspl [x]");
		break;
		}
	case 1340: { // jsnn [x]
		unhandled("jsnn [x]");
		break;
		}
	case 1341: { // jsec [x]
		unhandled("jsec [x]");
		break;
		}
	case 1342: { // jslc [x]
		unhandled("jslc [x]");
		break;
		}
	case 1343: { // jsgt [x]
		unhandled("jsgt [x]");
		break;
		}
	case 1344: { // jscs [x]
		unhandled("jscs [x]");
		break;
		}
	case 1345: { // jslt [x]
		unhandled("jslt [x]");
		break;
		}
	case 1346: { // jseq [x]
		unhandled("jseq [x]");
		break;
		}
	case 1347: { // jsmi [x]
		unhandled("jsmi [x]");
		break;
		}
	case 1348: { // jsnr [x]
		unhandled("jsnr [x]");
		break;
		}
	case 1349: { // jses [x]
		unhandled("jses [x]");
		break;
		}
	case 1350: { // jsls [x]
		unhandled("jsls [x]");
		break;
		}
	case 1351: { // jsle [x]
		unhandled("jsle [x]");
		break;
		}
	case 1352: { // jscc (r)-n
		unhandled("jscc (r)-n");
		break;
		}
	case 1353: { // jsge (r)-n
		unhandled("jsge (r)-n");
		break;
		}
	case 1354: { // jsne (r)-n
		unhandled("jsne (r)-n");
		break;
		}
	case 1355: { // jspl (r)-n
		unhandled("jspl (r)-n");
		break;
		}
	case 1356: { // jsnn (r)-n
		unhandled("jsnn (r)-n");
		break;
		}
	case 1357: { // jsec (r)-n
		unhandled("jsec (r)-n");
		break;
		}
	case 1358: { // jslc (r)-n
		unhandled("jslc (r)-n");
		break;
		}
	case 1359: { // jsgt (r)-n
		unhandled("jsgt (r)-n");
		break;
		}
	case 1360: { // jscs (r)-n
		unhandled("jscs (r)-n");
		break;
		}
	case 1361: { // jslt (r)-n
		unhandled("jslt (r)-n");
		break;
		}
	case 1362: { // jseq (r)-n
		unhandled("jseq (r)-n");
		break;
		}
	case 1363: { // jsmi (r)-n
		unhandled("jsmi (r)-n");
		break;
		}
	case 1364: { // jsnr (r)-n
		unhandled("jsnr (r)-n");
		break;
		}
	case 1365: { // jses (r)-n
		unhandled("jses (r)-n");
		break;
		}
	case 1366: { // jsls (r)-n
		unhandled("jsls (r)-n");
		break;
		}
	case 1367: { // jsle (r)-n
		unhandled("jsle (r)-n");
		break;
		}
	case 1368: { // jscc (r)+n
		unhandled("jscc (r)+n");
		break;
		}
	case 1369: { // jsge (r)+n
		unhandled("jsge (r)+n");
		break;
		}
	case 1370: { // jsne (r)+n
		unhandled("jsne (r)+n");
		break;
		}
	case 1371: { // jspl (r)+n
		unhandled("jspl (r)+n");
		break;
		}
	case 1372: { // jsnn (r)+n
		unhandled("jsnn (r)+n");
		break;
		}
	case 1373: { // jsec (r)+n
		unhandled("jsec (r)+n");
		break;
		}
	case 1374: { // jslc (r)+n
		unhandled("jslc (r)+n");
		break;
		}
	case 1375: { // jsgt (r)+n
		unhandled("jsgt (r)+n");
		break;
		}
	case 1376: { // jscs (r)+n
		unhandled("jscs (r)+n");
		break;
		}
	case 1377: { // jslt (r)+n
		unhandled("jslt (r)+n");
		break;
		}
	case 1378: { // jseq (r)+n
		unhandled("jseq (r)+n");
		break;
		}
	case 1379: { // jsmi (r)+n
		unhandled("jsmi (r)+n");
		break;
		}
	case 1380: { // jsnr (r)+n
		unhandled("jsnr (r)+n");
		break;
		}
	case 1381: { // jses (r)+n
		unhandled("jses (r)+n");
		break;
		}
	case 1382: { // jsls (r)+n
		unhandled("jsls (r)+n");
		break;
		}
	case 1383: { // jsle (r)+n
		unhandled("jsle (r)+n");
		break;
		}
	case 1384: { // jscc (r)-
		unhandled("jscc (r)-");
		break;
		}
	case 1385: { // jsge (r)-
		unhandled("jsge (r)-");
		break;
		}
	case 1386: { // jsne (r)-
		unhandled("jsne (r)-");
		break;
		}
	case 1387: { // jspl (r)-
		unhandled("jspl (r)-");
		break;
		}
	case 1388: { // jsnn (r)-
		unhandled("jsnn (r)-");
		break;
		}
	case 1389: { // jsec (r)-
		unhandled("jsec (r)-");
		break;
		}
	case 1390: { // jslc (r)-
		unhandled("jslc (r)-");
		break;
		}
	case 1391: { // jsgt (r)-
		unhandled("jsgt (r)-");
		break;
		}
	case 1392: { // jscs (r)-
		unhandled("jscs (r)-");
		break;
		}
	case 1393: { // jslt (r)-
		unhandled("jslt (r)-");
		break;
		}
	case 1394: { // jseq (r)-
		unhandled("jseq (r)-");
		break;
		}
	case 1395: { // jsmi (r)-
		unhandled("jsmi (r)-");
		break;
		}
	case 1396: { // jsnr (r)-
		unhandled("jsnr (r)-");
		break;
		}
	case 1397: { // jses (r)-
		unhandled("jses (r)-");
		break;
		}
	case 1398: { // jsls (r)-
		unhandled("jsls (r)-");
		break;
		}
	case 1399: { // jsle (r)-
		unhandled("jsle (r)-");
		break;
		}
	case 1400: { // jscc (r)+
		unhandled("jscc (r)+");
		break;
		}
	case 1401: { // jsge (r)+
		unhandled("jsge (r)+");
		break;
		}
	case 1402: { // jsne (r)+
		unhandled("jsne (r)+");
		break;
		}
	case 1403: { // jspl (r)+
		unhandled("jspl (r)+");
		break;
		}
	case 1404: { // jsnn (r)+
		unhandled("jsnn (r)+");
		break;
		}
	case 1405: { // jsec (r)+
		unhandled("jsec (r)+");
		break;
		}
	case 1406: { // jslc (r)+
		unhandled("jslc (r)+");
		break;
		}
	case 1407: { // jsgt (r)+
		unhandled("jsgt (r)+");
		break;
		}
	case 1408: { // jscs (r)+
		unhandled("jscs (r)+");
		break;
		}
	case 1409: { // jslt (r)+
		unhandled("jslt (r)+");
		break;
		}
	case 1410: { // jseq (r)+
		unhandled("jseq (r)+");
		break;
		}
	case 1411: { // jsmi (r)+
		unhandled("jsmi (r)+");
		break;
		}
	case 1412: { // jsnr (r)+
		unhandled("jsnr (r)+");
		break;
		}
	case 1413: { // jses (r)+
		unhandled("jses (r)+");
		break;
		}
	case 1414: { // jsls (r)+
		unhandled("jsls (r)+");
		break;
		}
	case 1415: { // jsle (r)+
		unhandled("jsle (r)+");
		break;
		}
	case 1416: { // jscc (r)
		unhandled("jscc (r)");
		break;
		}
	case 1417: { // jsge (r)
		unhandled("jsge (r)");
		break;
		}
	case 1418: { // jsne (r)
		unhandled("jsne (r)");
		break;
		}
	case 1419: { // jspl (r)
		unhandled("jspl (r)");
		break;
		}
	case 1420: { // jsnn (r)
		unhandled("jsnn (r)");
		break;
		}
	case 1421: { // jsec (r)
		unhandled("jsec (r)");
		break;
		}
	case 1422: { // jslc (r)
		unhandled("jslc (r)");
		break;
		}
	case 1423: { // jsgt (r)
		unhandled("jsgt (r)");
		break;
		}
	case 1424: { // jscs (r)
		unhandled("jscs (r)");
		break;
		}
	case 1425: { // jslt (r)
		unhandled("jslt (r)");
		break;
		}
	case 1426: { // jseq (r)
		unhandled("jseq (r)");
		break;
		}
	case 1427: { // jsmi (r)
		unhandled("jsmi (r)");
		break;
		}
	case 1428: { // jsnr (r)
		unhandled("jsnr (r)");
		break;
		}
	case 1429: { // jses (r)
		unhandled("jses (r)");
		break;
		}
	case 1430: { // jsls (r)
		unhandled("jsls (r)");
		break;
		}
	case 1431: { // jsle (r)
		unhandled("jsle (r)");
		break;
		}
	case 1432: { // jscc (r+n)
		unhandled("jscc (r+n)");
		break;
		}
	case 1433: { // jsge (r+n)
		unhandled("jsge (r+n)");
		break;
		}
	case 1434: { // jsne (r+n)
		unhandled("jsne (r+n)");
		break;
		}
	case 1435: { // jspl (r+n)
		unhandled("jspl (r+n)");
		break;
		}
	case 1436: { // jsnn (r+n)
		unhandled("jsnn (r+n)");
		break;
		}
	case 1437: { // jsec (r+n)
		unhandled("jsec (r+n)");
		break;
		}
	case 1438: { // jslc (r+n)
		unhandled("jslc (r+n)");
		break;
		}
	case 1439: { // jsgt (r+n)
		unhandled("jsgt (r+n)");
		break;
		}
	case 1440: { // jscs (r+n)
		unhandled("jscs (r+n)");
		break;
		}
	case 1441: { // jslt (r+n)
		unhandled("jslt (r+n)");
		break;
		}
	case 1442: { // jseq (r+n)
		unhandled("jseq (r+n)");
		break;
		}
	case 1443: { // jsmi (r+n)
		unhandled("jsmi (r+n)");
		break;
		}
	case 1444: { // jsnr (r+n)
		unhandled("jsnr (r+n)");
		break;
		}
	case 1445: { // jses (r+n)
		unhandled("jses (r+n)");
		break;
		}
	case 1446: { // jsls (r+n)
		unhandled("jsls (r+n)");
		break;
		}
	case 1447: { // jsle (r+n)
		unhandled("jsle (r+n)");
		break;
		}
	case 1448: { // jscc -(r)
		unhandled("jscc -(r)");
		break;
		}
	case 1449: { // jsge -(r)
		unhandled("jsge -(r)");
		break;
		}
	case 1450: { // jsne -(r)
		unhandled("jsne -(r)");
		break;
		}
	case 1451: { // jspl -(r)
		unhandled("jspl -(r)");
		break;
		}
	case 1452: { // jsnn -(r)
		unhandled("jsnn -(r)");
		break;
		}
	case 1453: { // jsec -(r)
		unhandled("jsec -(r)");
		break;
		}
	case 1454: { // jslc -(r)
		unhandled("jslc -(r)");
		break;
		}
	case 1455: { // jsgt -(r)
		unhandled("jsgt -(r)");
		break;
		}
	case 1456: { // jscs -(r)
		unhandled("jscs -(r)");
		break;
		}
	case 1457: { // jslt -(r)
		unhandled("jslt -(r)");
		break;
		}
	case 1458: { // jseq -(r)
		unhandled("jseq -(r)");
		break;
		}
	case 1459: { // jsmi -(r)
		unhandled("jsmi -(r)");
		break;
		}
	case 1460: { // jsnr -(r)
		unhandled("jsnr -(r)");
		break;
		}
	case 1461: { // jses -(r)
		unhandled("jses -(r)");
		break;
		}
	case 1462: { // jsls -(r)
		unhandled("jsls -(r)");
		break;
		}
	case 1463: { // jsle -(r)
		unhandled("jsle -(r)");
		break;
		}
	case 1464: { // jscc [abs]
		unhandled("jscc [abs]");
		break;
		}
	case 1465: { // jsge [abs]
		unhandled("jsge [abs]");
		break;
		}
	case 1466: { // jsne [abs]
		unhandled("jsne [abs]");
		break;
		}
	case 1467: { // jspl [abs]
		unhandled("jspl [abs]");
		break;
		}
	case 1468: { // jsnn [abs]
		unhandled("jsnn [abs]");
		break;
		}
	case 1469: { // jsec [abs]
		unhandled("jsec [abs]");
		break;
		}
	case 1470: { // jslc [abs]
		unhandled("jslc [abs]");
		break;
		}
	case 1471: { // jsgt [abs]
		unhandled("jsgt [abs]");
		break;
		}
	case 1472: { // jscs [abs]
		unhandled("jscs [abs]");
		break;
		}
	case 1473: { // jslt [abs]
		unhandled("jslt [abs]");
		break;
		}
	case 1474: { // jseq [abs]
		unhandled("jseq [abs]");
		break;
		}
	case 1475: { // jsmi [abs]
		unhandled("jsmi [abs]");
		break;
		}
	case 1476: { // jsnr [abs]
		unhandled("jsnr [abs]");
		break;
		}
	case 1477: { // jses [abs]
		unhandled("jses [abs]");
		break;
		}
	case 1478: { // jsls [abs]
		unhandled("jsls [abs]");
		break;
		}
	case 1479: { // jsle [abs]
		unhandled("jsle [abs]");
		break;
		}
	case 1480: { // jsclr #[n],x:(r)-n,[x]
		unhandled("jsclr #[n],x:(r)-n,[x]");
		break;
		}
	case 1481: { // jsclr #[n],y:(r)-n,[x]
		unhandled("jsclr #[n],y:(r)-n,[x]");
		break;
		}
	case 1482: { // jsclr #[n],x:(r)+n,[x]
		unhandled("jsclr #[n],x:(r)+n,[x]");
		break;
		}
	case 1483: { // jsclr #[n],y:(r)+n,[x]
		unhandled("jsclr #[n],y:(r)+n,[x]");
		break;
		}
	case 1484: { // jsclr #[n],x:(r)-,[x]
		unhandled("jsclr #[n],x:(r)-,[x]");
		break;
		}
	case 1485: { // jsclr #[n],y:(r)-,[x]
		unhandled("jsclr #[n],y:(r)-,[x]");
		break;
		}
	case 1486: { // jsclr #[n],x:(r)+,[x]
		unhandled("jsclr #[n],x:(r)+,[x]");
		break;
		}
	case 1487: { // jsclr #[n],y:(r)+,[x]
		unhandled("jsclr #[n],y:(r)+,[x]");
		break;
		}
	case 1488: { // jsclr #[n],x:(r),[x]
		unhandled("jsclr #[n],x:(r),[x]");
		break;
		}
	case 1489: { // jsclr #[n],y:(r),[x]
		unhandled("jsclr #[n],y:(r),[x]");
		break;
		}
	case 1490: { // jsclr #[n],x:(r+n),[x]
		unhandled("jsclr #[n],x:(r+n),[x]");
		break;
		}
	case 1491: { // jsclr #[n],y:(r+n),[x]
		unhandled("jsclr #[n],y:(r+n),[x]");
		break;
		}
	case 1492: { // jsclr #[n],x:-(r),[x]
		unhandled("jsclr #[n],x:-(r),[x]");
		break;
		}
	case 1493: { // jsclr #[n],y:-(r),[x]
		unhandled("jsclr #[n],y:-(r),[x]");
		break;
		}
	case 1494: { // jsclr #[n],x:[aa],[x]
		unhandled("jsclr #[n],x:[aa],[x]");
		break;
		}
	case 1495: { // jsclr #[n],y:[aa],[x]
		unhandled("jsclr #[n],y:[aa],[x]");
		break;
		}
	case 1496: { // jsclr #[n],x:[pp],[x]
		unhandled("jsclr #[n],x:[pp],[x]");
		break;
		}
	case 1497: { // jsclr #[n],y:[pp],[x]
		unhandled("jsclr #[n],y:[pp],[x]");
		break;
		}
	case 1498: { // jsclr #[n],x:[qq],[x]
		unhandled("jsclr #[n],x:[qq],[x]");
		break;
		}
	case 1499: { // jsclr #[n],y:[qq],[x]
		unhandled("jsclr #[n],y:[qq],[x]");
		break;
		}
	case 1500: { // jsclr #[n],x0,[x]
		unhandled("jsclr #[n],x0,[x]");
		break;
		}
	case 1501: { // jsclr #[n],x1,[x]
		unhandled("jsclr #[n],x1,[x]");
		break;
		}
	case 1502: { // jsclr #[n],y0,[x]
		unhandled("jsclr #[n],y0,[x]");
		break;
		}
	case 1503: { // jsclr #[n],y1,[x]
		unhandled("jsclr #[n],y1,[x]");
		break;
		}
	case 1504: { // jsclr #[n],a0,[x]
		unhandled("jsclr #[n],a0,[x]");
		break;
		}
	case 1505: { // jsclr #[n],b0,[x]
		unhandled("jsclr #[n],b0,[x]");
		break;
		}
	case 1506: { // jsclr #[n],a2,[x]
		unhandled("jsclr #[n],a2,[x]");
		break;
		}
	case 1507: { // jsclr #[n],b2,[x]
		unhandled("jsclr #[n],b2,[x]");
		break;
		}
	case 1508: { // jsclr #[n],a1,[x]
		unhandled("jsclr #[n],a1,[x]");
		break;
		}
	case 1509: { // jsclr #[n],b1,[x]
		unhandled("jsclr #[n],b1,[x]");
		break;
		}
	case 1510: { // jsclr #[n],a,[x]
		unhandled("jsclr #[n],a,[x]");
		break;
		}
	case 1511: { // jsclr #[n],b,[x]
		unhandled("jsclr #[n],b,[x]");
		break;
		}
	case 1512: { // jsclr #[n],r,[x]
		unhandled("jsclr #[n],r,[x]");
		break;
		}
	case 1513: { // jsclr #[n],n,[x]
		unhandled("jsclr #[n],n,[x]");
		break;
		}
	case 1514: { // jsclr #[n],m,[x]
		unhandled("jsclr #[n],m,[x]");
		break;
		}
	case 1515: { // jsclr #[n],ep,[x]
		unhandled("jsclr #[n],ep,[x]");
		break;
		}
	case 1516: { // jsclr #[n],vba,[x]
		unhandled("jsclr #[n],vba,[x]");
		break;
		}
	case 1517: { // jsclr #[n],sc,[x]
		unhandled("jsclr #[n],sc,[x]");
		break;
		}
	case 1518: { // jsclr #[n],sz,[x]
		unhandled("jsclr #[n],sz,[x]");
		break;
		}
	case 1519: { // jsclr #[n],sr,[x]
		unhandled("jsclr #[n],sr,[x]");
		break;
		}
	case 1520: { // jsclr #[n],omr,[x]
		unhandled("jsclr #[n],omr,[x]");
		break;
		}
	case 1521: { // jsclr #[n],sp,[x]
		unhandled("jsclr #[n],sp,[x]");
		break;
		}
	case 1522: { // jsclr #[n],ssh,[x]
		unhandled("jsclr #[n],ssh,[x]");
		break;
		}
	case 1523: { // jsclr #[n],ssl,[x]
		unhandled("jsclr #[n],ssl,[x]");
		break;
		}
	case 1524: { // jsclr #[n],la,[x]
		unhandled("jsclr #[n],la,[x]");
		break;
		}
	case 1525: { // jsclr #[n],lc,[x]
		unhandled("jsclr #[n],lc,[x]");
		break;
		}
	case 1526: { // jset #[n],x:(r)-n,[x]
		unhandled("jset #[n],x:(r)-n,[x]");
		break;
		}
	case 1527: { // jset #[n],y:(r)-n,[x]
		unhandled("jset #[n],y:(r)-n,[x]");
		break;
		}
	case 1528: { // jset #[n],x:(r)+n,[x]
		unhandled("jset #[n],x:(r)+n,[x]");
		break;
		}
	case 1529: { // jset #[n],y:(r)+n,[x]
		unhandled("jset #[n],y:(r)+n,[x]");
		break;
		}
	case 1530: { // jset #[n],x:(r)-,[x]
		unhandled("jset #[n],x:(r)-,[x]");
		break;
		}
	case 1531: { // jset #[n],y:(r)-,[x]
		unhandled("jset #[n],y:(r)-,[x]");
		break;
		}
	case 1532: { // jset #[n],x:(r)+,[x]
		unhandled("jset #[n],x:(r)+,[x]");
		break;
		}
	case 1533: { // jset #[n],y:(r)+,[x]
		unhandled("jset #[n],y:(r)+,[x]");
		break;
		}
	case 1534: { // jset #[n],x:(r),[x]
		unhandled("jset #[n],x:(r),[x]");
		break;
		}
	case 1535: { // jset #[n],y:(r),[x]
		unhandled("jset #[n],y:(r),[x]");
		break;
		}
	case 1536: { // jset #[n],x:(r+n),[x]
		unhandled("jset #[n],x:(r+n),[x]");
		break;
		}
	case 1537: { // jset #[n],y:(r+n),[x]
		unhandled("jset #[n],y:(r+n),[x]");
		break;
		}
	case 1538: { // jset #[n],x:-(r),[x]
		unhandled("jset #[n],x:-(r),[x]");
		break;
		}
	case 1539: { // jset #[n],y:-(r),[x]
		unhandled("jset #[n],y:-(r),[x]");
		break;
		}
	case 1540: { // jset #[n],x:[aa],[x]
		unhandled("jset #[n],x:[aa],[x]");
		break;
		}
	case 1541: { // jset #[n],y:[aa],[x]
		unhandled("jset #[n],y:[aa],[x]");
		break;
		}
	case 1542: { // jset #[n],x:[pp],[x]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(m_x.read_dword(pp), n))
		m_npc = x;
		break;
		}
	case 1543: { // jset #[n],y:[pp],[x]
		u32 pp = 0xffffc0 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(m_y.read_dword(pp), n))
		m_npc = x;
		break;
		}
	case 1544: { // jset #[n],x:[qq],[x]
		u32 qq = 0xffff80 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(m_x.read_dword(qq), n))
		m_npc = x;
		break;
		}
	case 1545: { // jset #[n],y:[qq],[x]
		u32 qq = 0xffff80 + BIT(opcode, 8, 6);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(m_y.read_dword(qq), n))
		m_npc = x;
		break;
		}
	case 1546: { // jset #[n],x0,[x]
		u32 s = get_x0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1547: { // jset #[n],x1,[x]
		u32 s = get_x1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1548: { // jset #[n],y0,[x]
		u32 s = get_y0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1549: { // jset #[n],y1,[x]
		u32 s = get_y1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1550: { // jset #[n],a0,[x]
		u32 s = get_a0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1551: { // jset #[n],b0,[x]
		u32 s = get_b0();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1552: { // jset #[n],a2,[x]
		u32 s = get_a2();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1553: { // jset #[n],b2,[x]
		u32 s = get_b2();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1554: { // jset #[n],a1,[x]
		u32 s = get_a1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1555: { // jset #[n],b1,[x]
		u32 s = get_b1();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1556: { // jset #[n],a,[x]
		u64 s = get_a();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1557: { // jset #[n],b,[x]
		u64 s = get_b();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1558: { // jset #[n],r,[x]
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1559: { // jset #[n],n,[x]
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1560: { // jset #[n],m,[x]
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1561: { // jset #[n],ep,[x]
		u32 s = get_ep();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1562: { // jset #[n],vba,[x]
		u32 s = get_vba();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1563: { // jset #[n],sc,[x]
		u32 s = get_sc();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1564: { // jset #[n],sz,[x]
		u32 s = get_sz();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1565: { // jset #[n],sr,[x]
		u32 s = get_sr();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1566: { // jset #[n],omr,[x]
		u32 s = get_omr();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1567: { // jset #[n],sp,[x]
		u32 s = get_sp();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1568: { // jset #[n],ssh,[x]
		u32 s = get_ssh();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1569: { // jset #[n],ssl,[x]
		u32 s = get_ssl();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1570: { // jset #[n],la,[x]
		u32 s = get_la();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1571: { // jset #[n],lc,[x]
		u32 s = get_lc();
		u32 n = BIT(opcode, 0, 5);
		u32 x = exv;
		if(BIT(s, n))
		m_npc = x;
		break;
		}
	case 1572: { // jsr (r)-n
		unhandled("jsr (r)-n");
		break;
		}
	case 1573: { // jsr (r)+n
		unhandled("jsr (r)+n");
		break;
		}
	case 1574: { // jsr (r)-
		unhandled("jsr (r)-");
		break;
		}
	case 1575: { // jsr (r)+
		unhandled("jsr (r)+");
		break;
		}
	case 1576: { // jsr (r)
		unhandled("jsr (r)");
		break;
		}
	case 1577: { // jsr (r+n)
		unhandled("jsr (r+n)");
		break;
		}
	case 1578: { // jsr -(r)
		unhandled("jsr -(r)");
		break;
		}
	case 1579: { // jsr [abs]
		unhandled("jsr [abs]");
		break;
		}
	case 1580: { // jsr [x]
		unhandled("jsr [x]");
		break;
		}
	case 1581: { // jsset #[n],x:(r)-n,[x]
		unhandled("jsset #[n],x:(r)-n,[x]");
		break;
		}
	case 1582: { // jsset #[n],y:(r)-n,[x]
		unhandled("jsset #[n],y:(r)-n,[x]");
		break;
		}
	case 1583: { // jsset #[n],x:(r)+n,[x]
		unhandled("jsset #[n],x:(r)+n,[x]");
		break;
		}
	case 1584: { // jsset #[n],y:(r)+n,[x]
		unhandled("jsset #[n],y:(r)+n,[x]");
		break;
		}
	case 1585: { // jsset #[n],x:(r)-,[x]
		unhandled("jsset #[n],x:(r)-,[x]");
		break;
		}
	case 1586: { // jsset #[n],y:(r)-,[x]
		unhandled("jsset #[n],y:(r)-,[x]");
		break;
		}
	case 1587: { // jsset #[n],x:(r)+,[x]
		unhandled("jsset #[n],x:(r)+,[x]");
		break;
		}
	case 1588: { // jsset #[n],y:(r)+,[x]
		unhandled("jsset #[n],y:(r)+,[x]");
		break;
		}
	case 1589: { // jsset #[n],x:(r),[x]
		unhandled("jsset #[n],x:(r),[x]");
		break;
		}
	case 1590: { // jsset #[n],y:(r),[x]
		unhandled("jsset #[n],y:(r),[x]");
		break;
		}
	case 1591: { // jsset #[n],x:(r+n),[x]
		unhandled("jsset #[n],x:(r+n),[x]");
		break;
		}
	case 1592: { // jsset #[n],y:(r+n),[x]
		unhandled("jsset #[n],y:(r+n),[x]");
		break;
		}
	case 1593: { // jsset #[n],x:-(r),[x]
		unhandled("jsset #[n],x:-(r),[x]");
		break;
		}
	case 1594: { // jsset #[n],y:-(r),[x]
		unhandled("jsset #[n],y:-(r),[x]");
		break;
		}
	case 1595: { // jsset #[n],x:[aa],[x]
		unhandled("jsset #[n],x:[aa],[x]");
		break;
		}
	case 1596: { // jsset #[n],y:[aa],[x]
		unhandled("jsset #[n],y:[aa],[x]");
		break;
		}
	case 1597: { // jsset #[n],x:[pp],[x]
		unhandled("jsset #[n],x:[pp],[x]");
		break;
		}
	case 1598: { // jsset #[n],y:[pp],[x]
		unhandled("jsset #[n],y:[pp],[x]");
		break;
		}
	case 1599: { // jsset #[n],x:[qq],[x]
		unhandled("jsset #[n],x:[qq],[x]");
		break;
		}
	case 1600: { // jsset #[n],y:[qq],[x]
		unhandled("jsset #[n],y:[qq],[x]");
		break;
		}
	case 1601: { // jsset #[n],x0,[x]
		unhandled("jsset #[n],x0,[x]");
		break;
		}
	case 1602: { // jsset #[n],x1,[x]
		unhandled("jsset #[n],x1,[x]");
		break;
		}
	case 1603: { // jsset #[n],y0,[x]
		unhandled("jsset #[n],y0,[x]");
		break;
		}
	case 1604: { // jsset #[n],y1,[x]
		unhandled("jsset #[n],y1,[x]");
		break;
		}
	case 1605: { // jsset #[n],a0,[x]
		unhandled("jsset #[n],a0,[x]");
		break;
		}
	case 1606: { // jsset #[n],b0,[x]
		unhandled("jsset #[n],b0,[x]");
		break;
		}
	case 1607: { // jsset #[n],a2,[x]
		unhandled("jsset #[n],a2,[x]");
		break;
		}
	case 1608: { // jsset #[n],b2,[x]
		unhandled("jsset #[n],b2,[x]");
		break;
		}
	case 1609: { // jsset #[n],a1,[x]
		unhandled("jsset #[n],a1,[x]");
		break;
		}
	case 1610: { // jsset #[n],b1,[x]
		unhandled("jsset #[n],b1,[x]");
		break;
		}
	case 1611: { // jsset #[n],a,[x]
		unhandled("jsset #[n],a,[x]");
		break;
		}
	case 1612: { // jsset #[n],b,[x]
		unhandled("jsset #[n],b,[x]");
		break;
		}
	case 1613: { // jsset #[n],r,[x]
		unhandled("jsset #[n],r,[x]");
		break;
		}
	case 1614: { // jsset #[n],n,[x]
		unhandled("jsset #[n],n,[x]");
		break;
		}
	case 1615: { // jsset #[n],m,[x]
		unhandled("jsset #[n],m,[x]");
		break;
		}
	case 1616: { // jsset #[n],ep,[x]
		unhandled("jsset #[n],ep,[x]");
		break;
		}
	case 1617: { // jsset #[n],vba,[x]
		unhandled("jsset #[n],vba,[x]");
		break;
		}
	case 1618: { // jsset #[n],sc,[x]
		unhandled("jsset #[n],sc,[x]");
		break;
		}
	case 1619: { // jsset #[n],sz,[x]
		unhandled("jsset #[n],sz,[x]");
		break;
		}
	case 1620: { // jsset #[n],sr,[x]
		unhandled("jsset #[n],sr,[x]");
		break;
		}
	case 1621: { // jsset #[n],omr,[x]
		unhandled("jsset #[n],omr,[x]");
		break;
		}
	case 1622: { // jsset #[n],sp,[x]
		unhandled("jsset #[n],sp,[x]");
		break;
		}
	case 1623: { // jsset #[n],ssh,[x]
		unhandled("jsset #[n],ssh,[x]");
		break;
		}
	case 1624: { // jsset #[n],ssl,[x]
		unhandled("jsset #[n],ssl,[x]");
		break;
		}
	case 1625: { // jsset #[n],la,[x]
		unhandled("jsset #[n],la,[x]");
		break;
		}
	case 1626: { // jsset #[n],lc,[x]
		unhandled("jsset #[n],lc,[x]");
		break;
		}
	case 1627: { // lra r,x0
		unhandled("lra r,x0");
		break;
		}
	case 1628: { // lra r,x1
		unhandled("lra r,x1");
		break;
		}
	case 1629: { // lra r,y0
		unhandled("lra r,y0");
		break;
		}
	case 1630: { // lra r,y1
		unhandled("lra r,y1");
		break;
		}
	case 1631: { // lra r,a0
		unhandled("lra r,a0");
		break;
		}
	case 1632: { // lra r,b0
		unhandled("lra r,b0");
		break;
		}
	case 1633: { // lra r,a2
		unhandled("lra r,a2");
		break;
		}
	case 1634: { // lra r,b2
		unhandled("lra r,b2");
		break;
		}
	case 1635: { // lra r,a1
		unhandled("lra r,a1");
		break;
		}
	case 1636: { // lra r,b1
		unhandled("lra r,b1");
		break;
		}
	case 1637: { // lra r,a
		unhandled("lra r,a");
		break;
		}
	case 1638: { // lra r,b
		unhandled("lra r,b");
		break;
		}
	case 1639: { // lra r,r
		unhandled("lra r,r");
		break;
		}
	case 1640: { // lra r,n
		unhandled("lra r,n");
		break;
		}
	case 1641: { // lra #[i],x0
		unhandled("lra #[i],x0");
		break;
		}
	case 1642: { // lra #[i],x1
		unhandled("lra #[i],x1");
		break;
		}
	case 1643: { // lra #[i],y0
		unhandled("lra #[i],y0");
		break;
		}
	case 1644: { // lra #[i],y1
		unhandled("lra #[i],y1");
		break;
		}
	case 1645: { // lra #[i],a0
		unhandled("lra #[i],a0");
		break;
		}
	case 1646: { // lra #[i],b0
		unhandled("lra #[i],b0");
		break;
		}
	case 1647: { // lra #[i],a2
		unhandled("lra #[i],a2");
		break;
		}
	case 1648: { // lra #[i],b2
		unhandled("lra #[i],b2");
		break;
		}
	case 1649: { // lra #[i],a1
		unhandled("lra #[i],a1");
		break;
		}
	case 1650: { // lra #[i],b1
		unhandled("lra #[i],b1");
		break;
		}
	case 1651: { // lra #[i],a
		unhandled("lra #[i],a");
		break;
		}
	case 1652: { // lra #[i],b
		unhandled("lra #[i],b");
		break;
		}
	case 1653: { // lra #[i],r
		unhandled("lra #[i],r");
		break;
		}
	case 1654: { // lra #[i],n
		unhandled("lra #[i],n");
		break;
		}
	case 1655: { // lsl #[i],a
		unhandled("lsl #[i],a");
		break;
		}
	case 1656: { // lsl #[i],b
		unhandled("lsl #[i],b");
		break;
		}
	case 1657: { // lsl a1,a
		unhandled("lsl a1,a");
		break;
		}
	case 1658: { // lsl a1,b
		unhandled("lsl a1,b");
		break;
		}
	case 1659: { // lsl b1,a
		unhandled("lsl b1,a");
		break;
		}
	case 1660: { // lsl b1,b
		unhandled("lsl b1,b");
		break;
		}
	case 1661: { // lsl x0,a
		unhandled("lsl x0,a");
		break;
		}
	case 1662: { // lsl x0,b
		unhandled("lsl x0,b");
		break;
		}
	case 1663: { // lsl y0,a
		unhandled("lsl y0,a");
		break;
		}
	case 1664: { // lsl y0,b
		unhandled("lsl y0,b");
		break;
		}
	case 1665: { // lsl x1,a
		unhandled("lsl x1,a");
		break;
		}
	case 1666: { // lsl x1,b
		unhandled("lsl x1,b");
		break;
		}
	case 1667: { // lsl y1,a
		unhandled("lsl y1,a");
		break;
		}
	case 1668: { // lsl y1,b
		unhandled("lsl y1,b");
		break;
		}
	case 1669: { // lsr #[i],a
		unhandled("lsr #[i],a");
		break;
		}
	case 1670: { // lsr #[i],b
		unhandled("lsr #[i],b");
		break;
		}
	case 1671: { // lsr a1,a
		unhandled("lsr a1,a");
		break;
		}
	case 1672: { // lsr a1,b
		unhandled("lsr a1,b");
		break;
		}
	case 1673: { // lsr b1,a
		unhandled("lsr b1,a");
		break;
		}
	case 1674: { // lsr b1,b
		unhandled("lsr b1,b");
		break;
		}
	case 1675: { // lsr x0,a
		unhandled("lsr x0,a");
		break;
		}
	case 1676: { // lsr x0,b
		unhandled("lsr x0,b");
		break;
		}
	case 1677: { // lsr y0,a
		unhandled("lsr y0,a");
		break;
		}
	case 1678: { // lsr y0,b
		unhandled("lsr y0,b");
		break;
		}
	case 1679: { // lsr x1,a
		unhandled("lsr x1,a");
		break;
		}
	case 1680: { // lsr x1,b
		unhandled("lsr x1,b");
		break;
		}
	case 1681: { // lsr y1,a
		unhandled("lsr y1,a");
		break;
		}
	case 1682: { // lsr y1,b
		unhandled("lsr y1,b");
		break;
		}
	case 1683: { // lua (r)-n,x0
		unhandled("lua (r)-n,x0");
		break;
		}
	case 1684: { // lua (r)-n,x1
		unhandled("lua (r)-n,x1");
		break;
		}
	case 1685: { // lua (r)-n,y0
		unhandled("lua (r)-n,y0");
		break;
		}
	case 1686: { // lua (r)-n,y1
		unhandled("lua (r)-n,y1");
		break;
		}
	case 1687: { // lua (r)-n,a0
		unhandled("lua (r)-n,a0");
		break;
		}
	case 1688: { // lua (r)-n,b0
		unhandled("lua (r)-n,b0");
		break;
		}
	case 1689: { // lua (r)-n,a2
		unhandled("lua (r)-n,a2");
		break;
		}
	case 1690: { // lua (r)-n,b2
		unhandled("lua (r)-n,b2");
		break;
		}
	case 1691: { // lua (r)-n,a1
		unhandled("lua (r)-n,a1");
		break;
		}
	case 1692: { // lua (r)-n,b1
		unhandled("lua (r)-n,b1");
		break;
		}
	case 1693: { // lua (r)-n,a
		unhandled("lua (r)-n,a");
		break;
		}
	case 1694: { // lua (r)-n,b
		unhandled("lua (r)-n,b");
		break;
		}
	case 1695: { // lua (r)-n,r
		unhandled("lua (r)-n,r");
		break;
		}
	case 1696: { // lua (r)-n,n
		unhandled("lua (r)-n,n");
		break;
		}
	case 1697: { // lua (r)+n,x0
		unhandled("lua (r)+n,x0");
		break;
		}
	case 1698: { // lua (r)+n,x1
		unhandled("lua (r)+n,x1");
		break;
		}
	case 1699: { // lua (r)+n,y0
		unhandled("lua (r)+n,y0");
		break;
		}
	case 1700: { // lua (r)+n,y1
		unhandled("lua (r)+n,y1");
		break;
		}
	case 1701: { // lua (r)+n,a0
		unhandled("lua (r)+n,a0");
		break;
		}
	case 1702: { // lua (r)+n,b0
		unhandled("lua (r)+n,b0");
		break;
		}
	case 1703: { // lua (r)+n,a2
		unhandled("lua (r)+n,a2");
		break;
		}
	case 1704: { // lua (r)+n,b2
		unhandled("lua (r)+n,b2");
		break;
		}
	case 1705: { // lua (r)+n,a1
		unhandled("lua (r)+n,a1");
		break;
		}
	case 1706: { // lua (r)+n,b1
		unhandled("lua (r)+n,b1");
		break;
		}
	case 1707: { // lua (r)+n,a
		unhandled("lua (r)+n,a");
		break;
		}
	case 1708: { // lua (r)+n,b
		unhandled("lua (r)+n,b");
		break;
		}
	case 1709: { // lua (r)+n,r
		unhandled("lua (r)+n,r");
		break;
		}
	case 1710: { // lua (r)+n,n
		unhandled("lua (r)+n,n");
		break;
		}
	case 1711: { // lua (r)-,x0
		unhandled("lua (r)-,x0");
		break;
		}
	case 1712: { // lua (r)-,x1
		unhandled("lua (r)-,x1");
		break;
		}
	case 1713: { // lua (r)-,y0
		unhandled("lua (r)-,y0");
		break;
		}
	case 1714: { // lua (r)-,y1
		unhandled("lua (r)-,y1");
		break;
		}
	case 1715: { // lua (r)-,a0
		unhandled("lua (r)-,a0");
		break;
		}
	case 1716: { // lua (r)-,b0
		unhandled("lua (r)-,b0");
		break;
		}
	case 1717: { // lua (r)-,a2
		unhandled("lua (r)-,a2");
		break;
		}
	case 1718: { // lua (r)-,b2
		unhandled("lua (r)-,b2");
		break;
		}
	case 1719: { // lua (r)-,a1
		unhandled("lua (r)-,a1");
		break;
		}
	case 1720: { // lua (r)-,b1
		unhandled("lua (r)-,b1");
		break;
		}
	case 1721: { // lua (r)-,a
		unhandled("lua (r)-,a");
		break;
		}
	case 1722: { // lua (r)-,b
		unhandled("lua (r)-,b");
		break;
		}
	case 1723: { // lua (r)-,r
		unhandled("lua (r)-,r");
		break;
		}
	case 1724: { // lua (r)-,n
		unhandled("lua (r)-,n");
		break;
		}
	case 1725: { // lua (r)+,x0
		unhandled("lua (r)+,x0");
		break;
		}
	case 1726: { // lua (r)+,x1
		unhandled("lua (r)+,x1");
		break;
		}
	case 1727: { // lua (r)+,y0
		unhandled("lua (r)+,y0");
		break;
		}
	case 1728: { // lua (r)+,y1
		unhandled("lua (r)+,y1");
		break;
		}
	case 1729: { // lua (r)+,a0
		unhandled("lua (r)+,a0");
		break;
		}
	case 1730: { // lua (r)+,b0
		unhandled("lua (r)+,b0");
		break;
		}
	case 1731: { // lua (r)+,a2
		unhandled("lua (r)+,a2");
		break;
		}
	case 1732: { // lua (r)+,b2
		unhandled("lua (r)+,b2");
		break;
		}
	case 1733: { // lua (r)+,a1
		unhandled("lua (r)+,a1");
		break;
		}
	case 1734: { // lua (r)+,b1
		unhandled("lua (r)+,b1");
		break;
		}
	case 1735: { // lua (r)+,a
		unhandled("lua (r)+,a");
		break;
		}
	case 1736: { // lua (r)+,b
		unhandled("lua (r)+,b");
		break;
		}
	case 1737: { // lua (r)+,r
		unhandled("lua (r)+,r");
		break;
		}
	case 1738: { // lua (r)+,n
		unhandled("lua (r)+,n");
		break;
		}
	case 1739: { // lua (r+[o]),r
		unhandled("lua (r+[o]),r");
		break;
		}
	case 1740: { // lua (r+[o]),n
		unhandled("lua (r+[o]),n");
		break;
		}
	case 1741: { // mac +y1,#[i],a
		unhandled("mac +y1,#[i],a");
		break;
		}
	case 1742: { // mac -y1,#[i],a
		unhandled("mac -y1,#[i],a");
		break;
		}
	case 1743: { // mac +y1,#[i],b
		unhandled("mac +y1,#[i],b");
		break;
		}
	case 1744: { // mac -y1,#[i],b
		unhandled("mac -y1,#[i],b");
		break;
		}
	case 1745: { // mac +x0,#[i],a
		unhandled("mac +x0,#[i],a");
		break;
		}
	case 1746: { // mac -x0,#[i],a
		unhandled("mac -x0,#[i],a");
		break;
		}
	case 1747: { // mac +x0,#[i],b
		unhandled("mac +x0,#[i],b");
		break;
		}
	case 1748: { // mac -x0,#[i],b
		unhandled("mac -x0,#[i],b");
		break;
		}
	case 1749: { // mac +y0,#[i],a
		unhandled("mac +y0,#[i],a");
		break;
		}
	case 1750: { // mac -y0,#[i],a
		unhandled("mac -y0,#[i],a");
		break;
		}
	case 1751: { // mac +y0,#[i],b
		unhandled("mac +y0,#[i],b");
		break;
		}
	case 1752: { // mac -y0,#[i],b
		unhandled("mac -y0,#[i],b");
		break;
		}
	case 1753: { // mac +x1,#[i],a
		unhandled("mac +x1,#[i],a");
		break;
		}
	case 1754: { // mac -x1,#[i],a
		unhandled("mac -x1,#[i],a");
		break;
		}
	case 1755: { // mac +x1,#[i],b
		unhandled("mac +x1,#[i],b");
		break;
		}
	case 1756: { // mac -x1,#[i],b
		unhandled("mac -x1,#[i],b");
		break;
		}
	case 1757: { // maci +#[i],y1,a
		unhandled("maci +#[i],y1,a");
		break;
		}
	case 1758: { // maci -#[i],y1,a
		unhandled("maci -#[i],y1,a");
		break;
		}
	case 1759: { // maci +#[i],y1,b
		unhandled("maci +#[i],y1,b");
		break;
		}
	case 1760: { // maci -#[i],y1,b
		unhandled("maci -#[i],y1,b");
		break;
		}
	case 1761: { // maci +#[i],x0,a
		unhandled("maci +#[i],x0,a");
		break;
		}
	case 1762: { // maci -#[i],x0,a
		unhandled("maci -#[i],x0,a");
		break;
		}
	case 1763: { // maci +#[i],x0,b
		unhandled("maci +#[i],x0,b");
		break;
		}
	case 1764: { // maci -#[i],x0,b
		unhandled("maci -#[i],x0,b");
		break;
		}
	case 1765: { // maci +#[i],y0,a
		unhandled("maci +#[i],y0,a");
		break;
		}
	case 1766: { // maci -#[i],y0,a
		unhandled("maci -#[i],y0,a");
		break;
		}
	case 1767: { // maci +#[i],y0,b
		unhandled("maci +#[i],y0,b");
		break;
		}
	case 1768: { // maci -#[i],y0,b
		unhandled("maci -#[i],y0,b");
		break;
		}
	case 1769: { // maci +#[i],x1,a
		unhandled("maci +#[i],x1,a");
		break;
		}
	case 1770: { // maci -#[i],x1,a
		unhandled("maci -#[i],x1,a");
		break;
		}
	case 1771: { // maci +#[i],x1,b
		unhandled("maci +#[i],x1,b");
		break;
		}
	case 1772: { // maci -#[i],x1,b
		unhandled("maci -#[i],x1,b");
		break;
		}
	case 1773: { // macsu +x0,x0,a
		unhandled("macsu +x0,x0,a");
		break;
		}
	case 1774: { // macsu +y0,y0,a
		unhandled("macsu +y0,y0,a");
		break;
		}
	case 1775: { // macsu +x1,x0,a
		unhandled("macsu +x1,x0,a");
		break;
		}
	case 1776: { // macsu +y1,y0,a
		unhandled("macsu +y1,y0,a");
		break;
		}
	case 1777: { // macsu +x1,x1,a
		unhandled("macsu +x1,x1,a");
		break;
		}
	case 1778: { // macsu +y1,y1,a
		unhandled("macsu +y1,y1,a");
		break;
		}
	case 1779: { // macsu +x0,x1,a
		unhandled("macsu +x0,x1,a");
		break;
		}
	case 1780: { // macsu +y0,y1,a
		unhandled("macsu +y0,y1,a");
		break;
		}
	case 1781: { // macsu +x0,y1,a
		unhandled("macsu +x0,y1,a");
		break;
		}
	case 1782: { // macsu +y0,x0,a
		unhandled("macsu +y0,x0,a");
		break;
		}
	case 1783: { // macsu +x1,y0,a
		unhandled("macsu +x1,y0,a");
		break;
		}
	case 1784: { // macsu +y1,x1,a
		unhandled("macsu +y1,x1,a");
		break;
		}
	case 1785: { // macsu +y1,x0,a
		unhandled("macsu +y1,x0,a");
		break;
		}
	case 1786: { // macsu +x0,y0,a
		unhandled("macsu +x0,y0,a");
		break;
		}
	case 1787: { // macsu +y0,x1,a
		unhandled("macsu +y0,x1,a");
		break;
		}
	case 1788: { // macsu +x1,y1,a
		unhandled("macsu +x1,y1,a");
		break;
		}
	case 1789: { // macsu -x0,x0,a
		unhandled("macsu -x0,x0,a");
		break;
		}
	case 1790: { // macsu -y0,y0,a
		unhandled("macsu -y0,y0,a");
		break;
		}
	case 1791: { // macsu -x1,x0,a
		unhandled("macsu -x1,x0,a");
		break;
		}
	case 1792: { // macsu -y1,y0,a
		unhandled("macsu -y1,y0,a");
		break;
		}
	case 1793: { // macsu -x1,x1,a
		unhandled("macsu -x1,x1,a");
		break;
		}
	case 1794: { // macsu -y1,y1,a
		unhandled("macsu -y1,y1,a");
		break;
		}
	case 1795: { // macsu -x0,x1,a
		unhandled("macsu -x0,x1,a");
		break;
		}
	case 1796: { // macsu -y0,y1,a
		unhandled("macsu -y0,y1,a");
		break;
		}
	case 1797: { // macsu -x0,y1,a
		unhandled("macsu -x0,y1,a");
		break;
		}
	case 1798: { // macsu -y0,x0,a
		unhandled("macsu -y0,x0,a");
		break;
		}
	case 1799: { // macsu -x1,y0,a
		unhandled("macsu -x1,y0,a");
		break;
		}
	case 1800: { // macsu -y1,x1,a
		unhandled("macsu -y1,x1,a");
		break;
		}
	case 1801: { // macsu -y1,x0,a
		unhandled("macsu -y1,x0,a");
		break;
		}
	case 1802: { // macsu -x0,y0,a
		unhandled("macsu -x0,y0,a");
		break;
		}
	case 1803: { // macsu -y0,x1,a
		unhandled("macsu -y0,x1,a");
		break;
		}
	case 1804: { // macsu -x1,y1,a
		unhandled("macsu -x1,y1,a");
		break;
		}
	case 1805: { // macsu +x0,x0,b
		unhandled("macsu +x0,x0,b");
		break;
		}
	case 1806: { // macsu +y0,y0,b
		unhandled("macsu +y0,y0,b");
		break;
		}
	case 1807: { // macsu +x1,x0,b
		unhandled("macsu +x1,x0,b");
		break;
		}
	case 1808: { // macsu +y1,y0,b
		unhandled("macsu +y1,y0,b");
		break;
		}
	case 1809: { // macsu +x1,x1,b
		unhandled("macsu +x1,x1,b");
		break;
		}
	case 1810: { // macsu +y1,y1,b
		unhandled("macsu +y1,y1,b");
		break;
		}
	case 1811: { // macsu +x0,x1,b
		unhandled("macsu +x0,x1,b");
		break;
		}
	case 1812: { // macsu +y0,y1,b
		unhandled("macsu +y0,y1,b");
		break;
		}
	case 1813: { // macsu +x0,y1,b
		unhandled("macsu +x0,y1,b");
		break;
		}
	case 1814: { // macsu +y0,x0,b
		unhandled("macsu +y0,x0,b");
		break;
		}
	case 1815: { // macsu +x1,y0,b
		unhandled("macsu +x1,y0,b");
		break;
		}
	case 1816: { // macsu +y1,x1,b
		unhandled("macsu +y1,x1,b");
		break;
		}
	case 1817: { // macsu +y1,x0,b
		unhandled("macsu +y1,x0,b");
		break;
		}
	case 1818: { // macsu +x0,y0,b
		unhandled("macsu +x0,y0,b");
		break;
		}
	case 1819: { // macsu +y0,x1,b
		unhandled("macsu +y0,x1,b");
		break;
		}
	case 1820: { // macsu +x1,y1,b
		unhandled("macsu +x1,y1,b");
		break;
		}
	case 1821: { // macsu -x0,x0,b
		unhandled("macsu -x0,x0,b");
		break;
		}
	case 1822: { // macsu -y0,y0,b
		unhandled("macsu -y0,y0,b");
		break;
		}
	case 1823: { // macsu -x1,x0,b
		unhandled("macsu -x1,x0,b");
		break;
		}
	case 1824: { // macsu -y1,y0,b
		unhandled("macsu -y1,y0,b");
		break;
		}
	case 1825: { // macsu -x1,x1,b
		unhandled("macsu -x1,x1,b");
		break;
		}
	case 1826: { // macsu -y1,y1,b
		unhandled("macsu -y1,y1,b");
		break;
		}
	case 1827: { // macsu -x0,x1,b
		unhandled("macsu -x0,x1,b");
		break;
		}
	case 1828: { // macsu -y0,y1,b
		unhandled("macsu -y0,y1,b");
		break;
		}
	case 1829: { // macsu -x0,y1,b
		unhandled("macsu -x0,y1,b");
		break;
		}
	case 1830: { // macsu -y0,x0,b
		unhandled("macsu -y0,x0,b");
		break;
		}
	case 1831: { // macsu -x1,y0,b
		unhandled("macsu -x1,y0,b");
		break;
		}
	case 1832: { // macsu -y1,x1,b
		unhandled("macsu -y1,x1,b");
		break;
		}
	case 1833: { // macsu -y1,x0,b
		unhandled("macsu -y1,x0,b");
		break;
		}
	case 1834: { // macsu -x0,y0,b
		unhandled("macsu -x0,y0,b");
		break;
		}
	case 1835: { // macsu -y0,x1,b
		unhandled("macsu -y0,x1,b");
		break;
		}
	case 1836: { // macsu -x1,y1,b
		unhandled("macsu -x1,y1,b");
		break;
		}
	case 1837: { // macuu +x0,x0,a
		unhandled("macuu +x0,x0,a");
		break;
		}
	case 1838: { // macuu +y0,y0,a
		unhandled("macuu +y0,y0,a");
		break;
		}
	case 1839: { // macuu +x1,x0,a
		unhandled("macuu +x1,x0,a");
		break;
		}
	case 1840: { // macuu +y1,y0,a
		unhandled("macuu +y1,y0,a");
		break;
		}
	case 1841: { // macuu +x1,x1,a
		unhandled("macuu +x1,x1,a");
		break;
		}
	case 1842: { // macuu +y1,y1,a
		unhandled("macuu +y1,y1,a");
		break;
		}
	case 1843: { // macuu +x0,x1,a
		unhandled("macuu +x0,x1,a");
		break;
		}
	case 1844: { // macuu +y0,y1,a
		unhandled("macuu +y0,y1,a");
		break;
		}
	case 1845: { // macuu +x0,y1,a
		unhandled("macuu +x0,y1,a");
		break;
		}
	case 1846: { // macuu +y0,x0,a
		unhandled("macuu +y0,x0,a");
		break;
		}
	case 1847: { // macuu +x1,y0,a
		unhandled("macuu +x1,y0,a");
		break;
		}
	case 1848: { // macuu +y1,x1,a
		unhandled("macuu +y1,x1,a");
		break;
		}
	case 1849: { // macuu +y1,x0,a
		unhandled("macuu +y1,x0,a");
		break;
		}
	case 1850: { // macuu +x0,y0,a
		unhandled("macuu +x0,y0,a");
		break;
		}
	case 1851: { // macuu +y0,x1,a
		unhandled("macuu +y0,x1,a");
		break;
		}
	case 1852: { // macuu +x1,y1,a
		unhandled("macuu +x1,y1,a");
		break;
		}
	case 1853: { // macuu -x0,x0,a
		unhandled("macuu -x0,x0,a");
		break;
		}
	case 1854: { // macuu -y0,y0,a
		unhandled("macuu -y0,y0,a");
		break;
		}
	case 1855: { // macuu -x1,x0,a
		unhandled("macuu -x1,x0,a");
		break;
		}
	case 1856: { // macuu -y1,y0,a
		unhandled("macuu -y1,y0,a");
		break;
		}
	case 1857: { // macuu -x1,x1,a
		unhandled("macuu -x1,x1,a");
		break;
		}
	case 1858: { // macuu -y1,y1,a
		unhandled("macuu -y1,y1,a");
		break;
		}
	case 1859: { // macuu -x0,x1,a
		unhandled("macuu -x0,x1,a");
		break;
		}
	case 1860: { // macuu -y0,y1,a
		unhandled("macuu -y0,y1,a");
		break;
		}
	case 1861: { // macuu -x0,y1,a
		unhandled("macuu -x0,y1,a");
		break;
		}
	case 1862: { // macuu -y0,x0,a
		unhandled("macuu -y0,x0,a");
		break;
		}
	case 1863: { // macuu -x1,y0,a
		unhandled("macuu -x1,y0,a");
		break;
		}
	case 1864: { // macuu -y1,x1,a
		unhandled("macuu -y1,x1,a");
		break;
		}
	case 1865: { // macuu -y1,x0,a
		unhandled("macuu -y1,x0,a");
		break;
		}
	case 1866: { // macuu -x0,y0,a
		unhandled("macuu -x0,y0,a");
		break;
		}
	case 1867: { // macuu -y0,x1,a
		unhandled("macuu -y0,x1,a");
		break;
		}
	case 1868: { // macuu -x1,y1,a
		unhandled("macuu -x1,y1,a");
		break;
		}
	case 1869: { // macuu +x0,x0,b
		unhandled("macuu +x0,x0,b");
		break;
		}
	case 1870: { // macuu +y0,y0,b
		unhandled("macuu +y0,y0,b");
		break;
		}
	case 1871: { // macuu +x1,x0,b
		unhandled("macuu +x1,x0,b");
		break;
		}
	case 1872: { // macuu +y1,y0,b
		unhandled("macuu +y1,y0,b");
		break;
		}
	case 1873: { // macuu +x1,x1,b
		unhandled("macuu +x1,x1,b");
		break;
		}
	case 1874: { // macuu +y1,y1,b
		unhandled("macuu +y1,y1,b");
		break;
		}
	case 1875: { // macuu +x0,x1,b
		unhandled("macuu +x0,x1,b");
		break;
		}
	case 1876: { // macuu +y0,y1,b
		unhandled("macuu +y0,y1,b");
		break;
		}
	case 1877: { // macuu +x0,y1,b
		unhandled("macuu +x0,y1,b");
		break;
		}
	case 1878: { // macuu +y0,x0,b
		unhandled("macuu +y0,x0,b");
		break;
		}
	case 1879: { // macuu +x1,y0,b
		unhandled("macuu +x1,y0,b");
		break;
		}
	case 1880: { // macuu +y1,x1,b
		unhandled("macuu +y1,x1,b");
		break;
		}
	case 1881: { // macuu +y1,x0,b
		unhandled("macuu +y1,x0,b");
		break;
		}
	case 1882: { // macuu +x0,y0,b
		unhandled("macuu +x0,y0,b");
		break;
		}
	case 1883: { // macuu +y0,x1,b
		unhandled("macuu +y0,x1,b");
		break;
		}
	case 1884: { // macuu +x1,y1,b
		unhandled("macuu +x1,y1,b");
		break;
		}
	case 1885: { // macuu -x0,x0,b
		unhandled("macuu -x0,x0,b");
		break;
		}
	case 1886: { // macuu -y0,y0,b
		unhandled("macuu -y0,y0,b");
		break;
		}
	case 1887: { // macuu -x1,x0,b
		unhandled("macuu -x1,x0,b");
		break;
		}
	case 1888: { // macuu -y1,y0,b
		unhandled("macuu -y1,y0,b");
		break;
		}
	case 1889: { // macuu -x1,x1,b
		unhandled("macuu -x1,x1,b");
		break;
		}
	case 1890: { // macuu -y1,y1,b
		unhandled("macuu -y1,y1,b");
		break;
		}
	case 1891: { // macuu -x0,x1,b
		unhandled("macuu -x0,x1,b");
		break;
		}
	case 1892: { // macuu -y0,y1,b
		unhandled("macuu -y0,y1,b");
		break;
		}
	case 1893: { // macuu -x0,y1,b
		unhandled("macuu -x0,y1,b");
		break;
		}
	case 1894: { // macuu -y0,x0,b
		unhandled("macuu -y0,x0,b");
		break;
		}
	case 1895: { // macuu -x1,y0,b
		unhandled("macuu -x1,y0,b");
		break;
		}
	case 1896: { // macuu -y1,x1,b
		unhandled("macuu -y1,x1,b");
		break;
		}
	case 1897: { // macuu -y1,x0,b
		unhandled("macuu -y1,x0,b");
		break;
		}
	case 1898: { // macuu -x0,y0,b
		unhandled("macuu -x0,y0,b");
		break;
		}
	case 1899: { // macuu -y0,x1,b
		unhandled("macuu -y0,x1,b");
		break;
		}
	case 1900: { // macuu -x1,y1,b
		unhandled("macuu -x1,y1,b");
		break;
		}
	case 1901: { // maccr +y1,#[i],a
		unhandled("maccr +y1,#[i],a");
		break;
		}
	case 1902: { // maccr -y1,#[i],a
		unhandled("maccr -y1,#[i],a");
		break;
		}
	case 1903: { // maccr +y1,#[i],b
		unhandled("maccr +y1,#[i],b");
		break;
		}
	case 1904: { // maccr -y1,#[i],b
		unhandled("maccr -y1,#[i],b");
		break;
		}
	case 1905: { // maccr +x0,#[i],a
		unhandled("maccr +x0,#[i],a");
		break;
		}
	case 1906: { // maccr -x0,#[i],a
		unhandled("maccr -x0,#[i],a");
		break;
		}
	case 1907: { // maccr +x0,#[i],b
		unhandled("maccr +x0,#[i],b");
		break;
		}
	case 1908: { // maccr -x0,#[i],b
		unhandled("maccr -x0,#[i],b");
		break;
		}
	case 1909: { // maccr +y0,#[i],a
		unhandled("maccr +y0,#[i],a");
		break;
		}
	case 1910: { // maccr -y0,#[i],a
		unhandled("maccr -y0,#[i],a");
		break;
		}
	case 1911: { // maccr +y0,#[i],b
		unhandled("maccr +y0,#[i],b");
		break;
		}
	case 1912: { // maccr -y0,#[i],b
		unhandled("maccr -y0,#[i],b");
		break;
		}
	case 1913: { // maccr +x1,#[i],a
		unhandled("maccr +x1,#[i],a");
		break;
		}
	case 1914: { // maccr -x1,#[i],a
		unhandled("maccr -x1,#[i],a");
		break;
		}
	case 1915: { // maccr +x1,#[i],b
		unhandled("maccr +x1,#[i],b");
		break;
		}
	case 1916: { // maccr -x1,#[i],b
		unhandled("maccr -x1,#[i],b");
		break;
		}
	case 1917: { // macri +#[i],y1,a
		unhandled("macri +#[i],y1,a");
		break;
		}
	case 1918: { // macri -#[i],y1,a
		unhandled("macri -#[i],y1,a");
		break;
		}
	case 1919: { // macri +#[i],y1,b
		unhandled("macri +#[i],y1,b");
		break;
		}
	case 1920: { // macri -#[i],y1,b
		unhandled("macri -#[i],y1,b");
		break;
		}
	case 1921: { // macri +#[i],x0,a
		unhandled("macri +#[i],x0,a");
		break;
		}
	case 1922: { // macri -#[i],x0,a
		unhandled("macri -#[i],x0,a");
		break;
		}
	case 1923: { // macri +#[i],x0,b
		unhandled("macri +#[i],x0,b");
		break;
		}
	case 1924: { // macri -#[i],x0,b
		unhandled("macri -#[i],x0,b");
		break;
		}
	case 1925: { // macri +#[i],y0,a
		unhandled("macri +#[i],y0,a");
		break;
		}
	case 1926: { // macri -#[i],y0,a
		unhandled("macri -#[i],y0,a");
		break;
		}
	case 1927: { // macri +#[i],y0,b
		unhandled("macri +#[i],y0,b");
		break;
		}
	case 1928: { // macri -#[i],y0,b
		unhandled("macri -#[i],y0,b");
		break;
		}
	case 1929: { // macri +#[i],x1,a
		unhandled("macri +#[i],x1,a");
		break;
		}
	case 1930: { // macri -#[i],x1,a
		unhandled("macri -#[i],x1,a");
		break;
		}
	case 1931: { // macri +#[i],x1,b
		unhandled("macri +#[i],x1,b");
		break;
		}
	case 1932: { // macri -#[i],x1,b
		unhandled("macri -#[i],x1,b");
		break;
		}
	case 1933: { // merge a1,a
		unhandled("merge a1,a");
		break;
		}
	case 1934: { // merge a1,b
		unhandled("merge a1,b");
		break;
		}
	case 1935: { // merge b1,a
		unhandled("merge b1,a");
		break;
		}
	case 1936: { // merge b1,b
		unhandled("merge b1,b");
		break;
		}
	case 1937: { // merge x0,a
		unhandled("merge x0,a");
		break;
		}
	case 1938: { // merge x0,b
		unhandled("merge x0,b");
		break;
		}
	case 1939: { // merge y0,a
		unhandled("merge y0,a");
		break;
		}
	case 1940: { // merge y0,b
		unhandled("merge y0,b");
		break;
		}
	case 1941: { // merge x1,a
		unhandled("merge x1,a");
		break;
		}
	case 1942: { // merge x1,b
		unhandled("merge x1,b");
		break;
		}
	case 1943: { // merge y1,a
		unhandled("merge y1,a");
		break;
		}
	case 1944: { // merge y1,b
		unhandled("merge y1,b");
		break;
		}
	case 1945: { // movec x:(r)-n,m
		unhandled("movec x:(r)-n,m");
		break;
		}
	case 1946: { // movec x:(r)-n,ep
		unhandled("movec x:(r)-n,ep");
		break;
		}
	case 1947: { // movec x:(r)-n,vba
		unhandled("movec x:(r)-n,vba");
		break;
		}
	case 1948: { // movec x:(r)-n,sc
		unhandled("movec x:(r)-n,sc");
		break;
		}
	case 1949: { // movec x:(r)-n,sz
		unhandled("movec x:(r)-n,sz");
		break;
		}
	case 1950: { // movec x:(r)-n,sr
		unhandled("movec x:(r)-n,sr");
		break;
		}
	case 1951: { // movec x:(r)-n,omr
		unhandled("movec x:(r)-n,omr");
		break;
		}
	case 1952: { // movec x:(r)-n,sp
		unhandled("movec x:(r)-n,sp");
		break;
		}
	case 1953: { // movec x:(r)-n,ssh
		unhandled("movec x:(r)-n,ssh");
		break;
		}
	case 1954: { // movec x:(r)-n,ssl
		unhandled("movec x:(r)-n,ssl");
		break;
		}
	case 1955: { // movec x:(r)-n,la
		unhandled("movec x:(r)-n,la");
		break;
		}
	case 1956: { // movec x:(r)-n,lc
		unhandled("movec x:(r)-n,lc");
		break;
		}
	case 1957: { // movec y:(r)-n,m
		unhandled("movec y:(r)-n,m");
		break;
		}
	case 1958: { // movec y:(r)-n,ep
		unhandled("movec y:(r)-n,ep");
		break;
		}
	case 1959: { // movec y:(r)-n,vba
		unhandled("movec y:(r)-n,vba");
		break;
		}
	case 1960: { // movec y:(r)-n,sc
		unhandled("movec y:(r)-n,sc");
		break;
		}
	case 1961: { // movec y:(r)-n,sz
		unhandled("movec y:(r)-n,sz");
		break;
		}
	case 1962: { // movec y:(r)-n,sr
		unhandled("movec y:(r)-n,sr");
		break;
		}
	case 1963: { // movec y:(r)-n,omr
		unhandled("movec y:(r)-n,omr");
		break;
		}
	case 1964: { // movec y:(r)-n,sp
		unhandled("movec y:(r)-n,sp");
		break;
		}
	case 1965: { // movec y:(r)-n,ssh
		unhandled("movec y:(r)-n,ssh");
		break;
		}
	case 1966: { // movec y:(r)-n,ssl
		unhandled("movec y:(r)-n,ssl");
		break;
		}
	case 1967: { // movec y:(r)-n,la
		unhandled("movec y:(r)-n,la");
		break;
		}
	case 1968: { // movec y:(r)-n,lc
		unhandled("movec y:(r)-n,lc");
		break;
		}
	case 1969: { // movec x:(r)+n,m
		unhandled("movec x:(r)+n,m");
		break;
		}
	case 1970: { // movec x:(r)+n,ep
		unhandled("movec x:(r)+n,ep");
		break;
		}
	case 1971: { // movec x:(r)+n,vba
		unhandled("movec x:(r)+n,vba");
		break;
		}
	case 1972: { // movec x:(r)+n,sc
		unhandled("movec x:(r)+n,sc");
		break;
		}
	case 1973: { // movec x:(r)+n,sz
		unhandled("movec x:(r)+n,sz");
		break;
		}
	case 1974: { // movec x:(r)+n,sr
		unhandled("movec x:(r)+n,sr");
		break;
		}
	case 1975: { // movec x:(r)+n,omr
		unhandled("movec x:(r)+n,omr");
		break;
		}
	case 1976: { // movec x:(r)+n,sp
		unhandled("movec x:(r)+n,sp");
		break;
		}
	case 1977: { // movec x:(r)+n,ssh
		unhandled("movec x:(r)+n,ssh");
		break;
		}
	case 1978: { // movec x:(r)+n,ssl
		unhandled("movec x:(r)+n,ssl");
		break;
		}
	case 1979: { // movec x:(r)+n,la
		unhandled("movec x:(r)+n,la");
		break;
		}
	case 1980: { // movec x:(r)+n,lc
		unhandled("movec x:(r)+n,lc");
		break;
		}
	case 1981: { // movec y:(r)+n,m
		unhandled("movec y:(r)+n,m");
		break;
		}
	case 1982: { // movec y:(r)+n,ep
		unhandled("movec y:(r)+n,ep");
		break;
		}
	case 1983: { // movec y:(r)+n,vba
		unhandled("movec y:(r)+n,vba");
		break;
		}
	case 1984: { // movec y:(r)+n,sc
		unhandled("movec y:(r)+n,sc");
		break;
		}
	case 1985: { // movec y:(r)+n,sz
		unhandled("movec y:(r)+n,sz");
		break;
		}
	case 1986: { // movec y:(r)+n,sr
		unhandled("movec y:(r)+n,sr");
		break;
		}
	case 1987: { // movec y:(r)+n,omr
		unhandled("movec y:(r)+n,omr");
		break;
		}
	case 1988: { // movec y:(r)+n,sp
		unhandled("movec y:(r)+n,sp");
		break;
		}
	case 1989: { // movec y:(r)+n,ssh
		unhandled("movec y:(r)+n,ssh");
		break;
		}
	case 1990: { // movec y:(r)+n,ssl
		unhandled("movec y:(r)+n,ssl");
		break;
		}
	case 1991: { // movec y:(r)+n,la
		unhandled("movec y:(r)+n,la");
		break;
		}
	case 1992: { // movec y:(r)+n,lc
		unhandled("movec y:(r)+n,lc");
		break;
		}
	case 1993: { // movec x:(r)-,m
		unhandled("movec x:(r)-,m");
		break;
		}
	case 1994: { // movec x:(r)-,ep
		unhandled("movec x:(r)-,ep");
		break;
		}
	case 1995: { // movec x:(r)-,vba
		unhandled("movec x:(r)-,vba");
		break;
		}
	case 1996: { // movec x:(r)-,sc
		unhandled("movec x:(r)-,sc");
		break;
		}
	case 1997: { // movec x:(r)-,sz
		unhandled("movec x:(r)-,sz");
		break;
		}
	case 1998: { // movec x:(r)-,sr
		unhandled("movec x:(r)-,sr");
		break;
		}
	case 1999: { // movec x:(r)-,omr
		unhandled("movec x:(r)-,omr");
		break;
		}
	case 2000: { // movec x:(r)-,sp
		unhandled("movec x:(r)-,sp");
		break;
		}
	case 2001: { // movec x:(r)-,ssh
		unhandled("movec x:(r)-,ssh");
		break;
		}
	case 2002: { // movec x:(r)-,ssl
		unhandled("movec x:(r)-,ssl");
		break;
		}
	case 2003: { // movec x:(r)-,la
		unhandled("movec x:(r)-,la");
		break;
		}
	case 2004: { // movec x:(r)-,lc
		unhandled("movec x:(r)-,lc");
		break;
		}
	case 2005: { // movec y:(r)-,m
		unhandled("movec y:(r)-,m");
		break;
		}
	case 2006: { // movec y:(r)-,ep
		unhandled("movec y:(r)-,ep");
		break;
		}
	case 2007: { // movec y:(r)-,vba
		unhandled("movec y:(r)-,vba");
		break;
		}
	case 2008: { // movec y:(r)-,sc
		unhandled("movec y:(r)-,sc");
		break;
		}
	case 2009: { // movec y:(r)-,sz
		unhandled("movec y:(r)-,sz");
		break;
		}
	case 2010: { // movec y:(r)-,sr
		unhandled("movec y:(r)-,sr");
		break;
		}
	case 2011: { // movec y:(r)-,omr
		unhandled("movec y:(r)-,omr");
		break;
		}
	case 2012: { // movec y:(r)-,sp
		unhandled("movec y:(r)-,sp");
		break;
		}
	case 2013: { // movec y:(r)-,ssh
		unhandled("movec y:(r)-,ssh");
		break;
		}
	case 2014: { // movec y:(r)-,ssl
		unhandled("movec y:(r)-,ssl");
		break;
		}
	case 2015: { // movec y:(r)-,la
		unhandled("movec y:(r)-,la");
		break;
		}
	case 2016: { // movec y:(r)-,lc
		unhandled("movec y:(r)-,lc");
		break;
		}
	case 2017: { // movec x:(r)+,m
		unhandled("movec x:(r)+,m");
		break;
		}
	case 2018: { // movec x:(r)+,ep
		unhandled("movec x:(r)+,ep");
		break;
		}
	case 2019: { // movec x:(r)+,vba
		unhandled("movec x:(r)+,vba");
		break;
		}
	case 2020: { // movec x:(r)+,sc
		unhandled("movec x:(r)+,sc");
		break;
		}
	case 2021: { // movec x:(r)+,sz
		unhandled("movec x:(r)+,sz");
		break;
		}
	case 2022: { // movec x:(r)+,sr
		unhandled("movec x:(r)+,sr");
		break;
		}
	case 2023: { // movec x:(r)+,omr
		unhandled("movec x:(r)+,omr");
		break;
		}
	case 2024: { // movec x:(r)+,sp
		unhandled("movec x:(r)+,sp");
		break;
		}
	case 2025: { // movec x:(r)+,ssh
		unhandled("movec x:(r)+,ssh");
		break;
		}
	case 2026: { // movec x:(r)+,ssl
		unhandled("movec x:(r)+,ssl");
		break;
		}
	case 2027: { // movec x:(r)+,la
		unhandled("movec x:(r)+,la");
		break;
		}
	case 2028: { // movec x:(r)+,lc
		unhandled("movec x:(r)+,lc");
		break;
		}
	case 2029: { // movec y:(r)+,m
		unhandled("movec y:(r)+,m");
		break;
		}
	case 2030: { // movec y:(r)+,ep
		unhandled("movec y:(r)+,ep");
		break;
		}
	case 2031: { // movec y:(r)+,vba
		unhandled("movec y:(r)+,vba");
		break;
		}
	case 2032: { // movec y:(r)+,sc
		unhandled("movec y:(r)+,sc");
		break;
		}
	case 2033: { // movec y:(r)+,sz
		unhandled("movec y:(r)+,sz");
		break;
		}
	case 2034: { // movec y:(r)+,sr
		unhandled("movec y:(r)+,sr");
		break;
		}
	case 2035: { // movec y:(r)+,omr
		unhandled("movec y:(r)+,omr");
		break;
		}
	case 2036: { // movec y:(r)+,sp
		unhandled("movec y:(r)+,sp");
		break;
		}
	case 2037: { // movec y:(r)+,ssh
		unhandled("movec y:(r)+,ssh");
		break;
		}
	case 2038: { // movec y:(r)+,ssl
		unhandled("movec y:(r)+,ssl");
		break;
		}
	case 2039: { // movec y:(r)+,la
		unhandled("movec y:(r)+,la");
		break;
		}
	case 2040: { // movec y:(r)+,lc
		unhandled("movec y:(r)+,lc");
		break;
		}
	case 2041: { // movec x:(r),m
		unhandled("movec x:(r),m");
		break;
		}
	case 2042: { // movec x:(r),ep
		unhandled("movec x:(r),ep");
		break;
		}
	case 2043: { // movec x:(r),vba
		unhandled("movec x:(r),vba");
		break;
		}
	case 2044: { // movec x:(r),sc
		unhandled("movec x:(r),sc");
		break;
		}
	case 2045: { // movec x:(r),sz
		unhandled("movec x:(r),sz");
		break;
		}
	case 2046: { // movec x:(r),sr
		unhandled("movec x:(r),sr");
		break;
		}
	case 2047: { // movec x:(r),omr
		unhandled("movec x:(r),omr");
		break;
		}
	case 2048: { // movec x:(r),sp
		unhandled("movec x:(r),sp");
		break;
		}
	case 2049: { // movec x:(r),ssh
		unhandled("movec x:(r),ssh");
		break;
		}
	case 2050: { // movec x:(r),ssl
		unhandled("movec x:(r),ssl");
		break;
		}
	case 2051: { // movec x:(r),la
		unhandled("movec x:(r),la");
		break;
		}
	case 2052: { // movec x:(r),lc
		unhandled("movec x:(r),lc");
		break;
		}
	case 2053: { // movec y:(r),m
		unhandled("movec y:(r),m");
		break;
		}
	case 2054: { // movec y:(r),ep
		unhandled("movec y:(r),ep");
		break;
		}
	case 2055: { // movec y:(r),vba
		unhandled("movec y:(r),vba");
		break;
		}
	case 2056: { // movec y:(r),sc
		unhandled("movec y:(r),sc");
		break;
		}
	case 2057: { // movec y:(r),sz
		unhandled("movec y:(r),sz");
		break;
		}
	case 2058: { // movec y:(r),sr
		unhandled("movec y:(r),sr");
		break;
		}
	case 2059: { // movec y:(r),omr
		unhandled("movec y:(r),omr");
		break;
		}
	case 2060: { // movec y:(r),sp
		unhandled("movec y:(r),sp");
		break;
		}
	case 2061: { // movec y:(r),ssh
		unhandled("movec y:(r),ssh");
		break;
		}
	case 2062: { // movec y:(r),ssl
		unhandled("movec y:(r),ssl");
		break;
		}
	case 2063: { // movec y:(r),la
		unhandled("movec y:(r),la");
		break;
		}
	case 2064: { // movec y:(r),lc
		unhandled("movec y:(r),lc");
		break;
		}
	case 2065: { // movec x:(r+n),m
		unhandled("movec x:(r+n),m");
		break;
		}
	case 2066: { // movec x:(r+n),ep
		unhandled("movec x:(r+n),ep");
		break;
		}
	case 2067: { // movec x:(r+n),vba
		unhandled("movec x:(r+n),vba");
		break;
		}
	case 2068: { // movec x:(r+n),sc
		unhandled("movec x:(r+n),sc");
		break;
		}
	case 2069: { // movec x:(r+n),sz
		unhandled("movec x:(r+n),sz");
		break;
		}
	case 2070: { // movec x:(r+n),sr
		unhandled("movec x:(r+n),sr");
		break;
		}
	case 2071: { // movec x:(r+n),omr
		unhandled("movec x:(r+n),omr");
		break;
		}
	case 2072: { // movec x:(r+n),sp
		unhandled("movec x:(r+n),sp");
		break;
		}
	case 2073: { // movec x:(r+n),ssh
		unhandled("movec x:(r+n),ssh");
		break;
		}
	case 2074: { // movec x:(r+n),ssl
		unhandled("movec x:(r+n),ssl");
		break;
		}
	case 2075: { // movec x:(r+n),la
		unhandled("movec x:(r+n),la");
		break;
		}
	case 2076: { // movec x:(r+n),lc
		unhandled("movec x:(r+n),lc");
		break;
		}
	case 2077: { // movec y:(r+n),m
		unhandled("movec y:(r+n),m");
		break;
		}
	case 2078: { // movec y:(r+n),ep
		unhandled("movec y:(r+n),ep");
		break;
		}
	case 2079: { // movec y:(r+n),vba
		unhandled("movec y:(r+n),vba");
		break;
		}
	case 2080: { // movec y:(r+n),sc
		unhandled("movec y:(r+n),sc");
		break;
		}
	case 2081: { // movec y:(r+n),sz
		unhandled("movec y:(r+n),sz");
		break;
		}
	case 2082: { // movec y:(r+n),sr
		unhandled("movec y:(r+n),sr");
		break;
		}
	case 2083: { // movec y:(r+n),omr
		unhandled("movec y:(r+n),omr");
		break;
		}
	case 2084: { // movec y:(r+n),sp
		unhandled("movec y:(r+n),sp");
		break;
		}
	case 2085: { // movec y:(r+n),ssh
		unhandled("movec y:(r+n),ssh");
		break;
		}
	case 2086: { // movec y:(r+n),ssl
		unhandled("movec y:(r+n),ssl");
		break;
		}
	case 2087: { // movec y:(r+n),la
		unhandled("movec y:(r+n),la");
		break;
		}
	case 2088: { // movec y:(r+n),lc
		unhandled("movec y:(r+n),lc");
		break;
		}
	case 2089: { // movec x:-(r),m
		unhandled("movec x:-(r),m");
		break;
		}
	case 2090: { // movec x:-(r),ep
		unhandled("movec x:-(r),ep");
		break;
		}
	case 2091: { // movec x:-(r),vba
		unhandled("movec x:-(r),vba");
		break;
		}
	case 2092: { // movec x:-(r),sc
		unhandled("movec x:-(r),sc");
		break;
		}
	case 2093: { // movec x:-(r),sz
		unhandled("movec x:-(r),sz");
		break;
		}
	case 2094: { // movec x:-(r),sr
		unhandled("movec x:-(r),sr");
		break;
		}
	case 2095: { // movec x:-(r),omr
		unhandled("movec x:-(r),omr");
		break;
		}
	case 2096: { // movec x:-(r),sp
		unhandled("movec x:-(r),sp");
		break;
		}
	case 2097: { // movec x:-(r),ssh
		unhandled("movec x:-(r),ssh");
		break;
		}
	case 2098: { // movec x:-(r),ssl
		unhandled("movec x:-(r),ssl");
		break;
		}
	case 2099: { // movec x:-(r),la
		unhandled("movec x:-(r),la");
		break;
		}
	case 2100: { // movec x:-(r),lc
		unhandled("movec x:-(r),lc");
		break;
		}
	case 2101: { // movec y:-(r),m
		unhandled("movec y:-(r),m");
		break;
		}
	case 2102: { // movec y:-(r),ep
		unhandled("movec y:-(r),ep");
		break;
		}
	case 2103: { // movec y:-(r),vba
		unhandled("movec y:-(r),vba");
		break;
		}
	case 2104: { // movec y:-(r),sc
		unhandled("movec y:-(r),sc");
		break;
		}
	case 2105: { // movec y:-(r),sz
		unhandled("movec y:-(r),sz");
		break;
		}
	case 2106: { // movec y:-(r),sr
		unhandled("movec y:-(r),sr");
		break;
		}
	case 2107: { // movec y:-(r),omr
		unhandled("movec y:-(r),omr");
		break;
		}
	case 2108: { // movec y:-(r),sp
		unhandled("movec y:-(r),sp");
		break;
		}
	case 2109: { // movec y:-(r),ssh
		unhandled("movec y:-(r),ssh");
		break;
		}
	case 2110: { // movec y:-(r),ssl
		unhandled("movec y:-(r),ssl");
		break;
		}
	case 2111: { // movec y:-(r),la
		unhandled("movec y:-(r),la");
		break;
		}
	case 2112: { // movec y:-(r),lc
		unhandled("movec y:-(r),lc");
		break;
		}
	case 2113: { // movec m,x:(r)-n
		unhandled("movec m,x:(r)-n");
		break;
		}
	case 2114: { // movec ep,x:(r)-n
		unhandled("movec ep,x:(r)-n");
		break;
		}
	case 2115: { // movec vba,x:(r)-n
		unhandled("movec vba,x:(r)-n");
		break;
		}
	case 2116: { // movec sc,x:(r)-n
		unhandled("movec sc,x:(r)-n");
		break;
		}
	case 2117: { // movec sz,x:(r)-n
		unhandled("movec sz,x:(r)-n");
		break;
		}
	case 2118: { // movec sr,x:(r)-n
		unhandled("movec sr,x:(r)-n");
		break;
		}
	case 2119: { // movec omr,x:(r)-n
		unhandled("movec omr,x:(r)-n");
		break;
		}
	case 2120: { // movec sp,x:(r)-n
		unhandled("movec sp,x:(r)-n");
		break;
		}
	case 2121: { // movec ssh,x:(r)-n
		unhandled("movec ssh,x:(r)-n");
		break;
		}
	case 2122: { // movec ssl,x:(r)-n
		unhandled("movec ssl,x:(r)-n");
		break;
		}
	case 2123: { // movec la,x:(r)-n
		unhandled("movec la,x:(r)-n");
		break;
		}
	case 2124: { // movec lc,x:(r)-n
		unhandled("movec lc,x:(r)-n");
		break;
		}
	case 2125: { // movec m,y:(r)-n
		unhandled("movec m,y:(r)-n");
		break;
		}
	case 2126: { // movec ep,y:(r)-n
		unhandled("movec ep,y:(r)-n");
		break;
		}
	case 2127: { // movec vba,y:(r)-n
		unhandled("movec vba,y:(r)-n");
		break;
		}
	case 2128: { // movec sc,y:(r)-n
		unhandled("movec sc,y:(r)-n");
		break;
		}
	case 2129: { // movec sz,y:(r)-n
		unhandled("movec sz,y:(r)-n");
		break;
		}
	case 2130: { // movec sr,y:(r)-n
		unhandled("movec sr,y:(r)-n");
		break;
		}
	case 2131: { // movec omr,y:(r)-n
		unhandled("movec omr,y:(r)-n");
		break;
		}
	case 2132: { // movec sp,y:(r)-n
		unhandled("movec sp,y:(r)-n");
		break;
		}
	case 2133: { // movec ssh,y:(r)-n
		unhandled("movec ssh,y:(r)-n");
		break;
		}
	case 2134: { // movec ssl,y:(r)-n
		unhandled("movec ssl,y:(r)-n");
		break;
		}
	case 2135: { // movec la,y:(r)-n
		unhandled("movec la,y:(r)-n");
		break;
		}
	case 2136: { // movec lc,y:(r)-n
		unhandled("movec lc,y:(r)-n");
		break;
		}
	case 2137: { // movec m,x:(r)+n
		unhandled("movec m,x:(r)+n");
		break;
		}
	case 2138: { // movec ep,x:(r)+n
		unhandled("movec ep,x:(r)+n");
		break;
		}
	case 2139: { // movec vba,x:(r)+n
		unhandled("movec vba,x:(r)+n");
		break;
		}
	case 2140: { // movec sc,x:(r)+n
		unhandled("movec sc,x:(r)+n");
		break;
		}
	case 2141: { // movec sz,x:(r)+n
		unhandled("movec sz,x:(r)+n");
		break;
		}
	case 2142: { // movec sr,x:(r)+n
		unhandled("movec sr,x:(r)+n");
		break;
		}
	case 2143: { // movec omr,x:(r)+n
		unhandled("movec omr,x:(r)+n");
		break;
		}
	case 2144: { // movec sp,x:(r)+n
		unhandled("movec sp,x:(r)+n");
		break;
		}
	case 2145: { // movec ssh,x:(r)+n
		unhandled("movec ssh,x:(r)+n");
		break;
		}
	case 2146: { // movec ssl,x:(r)+n
		unhandled("movec ssl,x:(r)+n");
		break;
		}
	case 2147: { // movec la,x:(r)+n
		unhandled("movec la,x:(r)+n");
		break;
		}
	case 2148: { // movec lc,x:(r)+n
		unhandled("movec lc,x:(r)+n");
		break;
		}
	case 2149: { // movec m,y:(r)+n
		unhandled("movec m,y:(r)+n");
		break;
		}
	case 2150: { // movec ep,y:(r)+n
		unhandled("movec ep,y:(r)+n");
		break;
		}
	case 2151: { // movec vba,y:(r)+n
		unhandled("movec vba,y:(r)+n");
		break;
		}
	case 2152: { // movec sc,y:(r)+n
		unhandled("movec sc,y:(r)+n");
		break;
		}
	case 2153: { // movec sz,y:(r)+n
		unhandled("movec sz,y:(r)+n");
		break;
		}
	case 2154: { // movec sr,y:(r)+n
		unhandled("movec sr,y:(r)+n");
		break;
		}
	case 2155: { // movec omr,y:(r)+n
		unhandled("movec omr,y:(r)+n");
		break;
		}
	case 2156: { // movec sp,y:(r)+n
		unhandled("movec sp,y:(r)+n");
		break;
		}
	case 2157: { // movec ssh,y:(r)+n
		unhandled("movec ssh,y:(r)+n");
		break;
		}
	case 2158: { // movec ssl,y:(r)+n
		unhandled("movec ssl,y:(r)+n");
		break;
		}
	case 2159: { // movec la,y:(r)+n
		unhandled("movec la,y:(r)+n");
		break;
		}
	case 2160: { // movec lc,y:(r)+n
		unhandled("movec lc,y:(r)+n");
		break;
		}
	case 2161: { // movec m,x:(r)-
		unhandled("movec m,x:(r)-");
		break;
		}
	case 2162: { // movec ep,x:(r)-
		unhandled("movec ep,x:(r)-");
		break;
		}
	case 2163: { // movec vba,x:(r)-
		unhandled("movec vba,x:(r)-");
		break;
		}
	case 2164: { // movec sc,x:(r)-
		unhandled("movec sc,x:(r)-");
		break;
		}
	case 2165: { // movec sz,x:(r)-
		unhandled("movec sz,x:(r)-");
		break;
		}
	case 2166: { // movec sr,x:(r)-
		unhandled("movec sr,x:(r)-");
		break;
		}
	case 2167: { // movec omr,x:(r)-
		unhandled("movec omr,x:(r)-");
		break;
		}
	case 2168: { // movec sp,x:(r)-
		unhandled("movec sp,x:(r)-");
		break;
		}
	case 2169: { // movec ssh,x:(r)-
		unhandled("movec ssh,x:(r)-");
		break;
		}
	case 2170: { // movec ssl,x:(r)-
		unhandled("movec ssl,x:(r)-");
		break;
		}
	case 2171: { // movec la,x:(r)-
		unhandled("movec la,x:(r)-");
		break;
		}
	case 2172: { // movec lc,x:(r)-
		unhandled("movec lc,x:(r)-");
		break;
		}
	case 2173: { // movec m,y:(r)-
		unhandled("movec m,y:(r)-");
		break;
		}
	case 2174: { // movec ep,y:(r)-
		unhandled("movec ep,y:(r)-");
		break;
		}
	case 2175: { // movec vba,y:(r)-
		unhandled("movec vba,y:(r)-");
		break;
		}
	case 2176: { // movec sc,y:(r)-
		unhandled("movec sc,y:(r)-");
		break;
		}
	case 2177: { // movec sz,y:(r)-
		unhandled("movec sz,y:(r)-");
		break;
		}
	case 2178: { // movec sr,y:(r)-
		unhandled("movec sr,y:(r)-");
		break;
		}
	case 2179: { // movec omr,y:(r)-
		unhandled("movec omr,y:(r)-");
		break;
		}
	case 2180: { // movec sp,y:(r)-
		unhandled("movec sp,y:(r)-");
		break;
		}
	case 2181: { // movec ssh,y:(r)-
		unhandled("movec ssh,y:(r)-");
		break;
		}
	case 2182: { // movec ssl,y:(r)-
		unhandled("movec ssl,y:(r)-");
		break;
		}
	case 2183: { // movec la,y:(r)-
		unhandled("movec la,y:(r)-");
		break;
		}
	case 2184: { // movec lc,y:(r)-
		unhandled("movec lc,y:(r)-");
		break;
		}
	case 2185: { // movec m,x:(r)+
		unhandled("movec m,x:(r)+");
		break;
		}
	case 2186: { // movec ep,x:(r)+
		unhandled("movec ep,x:(r)+");
		break;
		}
	case 2187: { // movec vba,x:(r)+
		unhandled("movec vba,x:(r)+");
		break;
		}
	case 2188: { // movec sc,x:(r)+
		unhandled("movec sc,x:(r)+");
		break;
		}
	case 2189: { // movec sz,x:(r)+
		unhandled("movec sz,x:(r)+");
		break;
		}
	case 2190: { // movec sr,x:(r)+
		unhandled("movec sr,x:(r)+");
		break;
		}
	case 2191: { // movec omr,x:(r)+
		unhandled("movec omr,x:(r)+");
		break;
		}
	case 2192: { // movec sp,x:(r)+
		unhandled("movec sp,x:(r)+");
		break;
		}
	case 2193: { // movec ssh,x:(r)+
		unhandled("movec ssh,x:(r)+");
		break;
		}
	case 2194: { // movec ssl,x:(r)+
		unhandled("movec ssl,x:(r)+");
		break;
		}
	case 2195: { // movec la,x:(r)+
		unhandled("movec la,x:(r)+");
		break;
		}
	case 2196: { // movec lc,x:(r)+
		unhandled("movec lc,x:(r)+");
		break;
		}
	case 2197: { // movec m,y:(r)+
		unhandled("movec m,y:(r)+");
		break;
		}
	case 2198: { // movec ep,y:(r)+
		unhandled("movec ep,y:(r)+");
		break;
		}
	case 2199: { // movec vba,y:(r)+
		unhandled("movec vba,y:(r)+");
		break;
		}
	case 2200: { // movec sc,y:(r)+
		unhandled("movec sc,y:(r)+");
		break;
		}
	case 2201: { // movec sz,y:(r)+
		unhandled("movec sz,y:(r)+");
		break;
		}
	case 2202: { // movec sr,y:(r)+
		unhandled("movec sr,y:(r)+");
		break;
		}
	case 2203: { // movec omr,y:(r)+
		unhandled("movec omr,y:(r)+");
		break;
		}
	case 2204: { // movec sp,y:(r)+
		unhandled("movec sp,y:(r)+");
		break;
		}
	case 2205: { // movec ssh,y:(r)+
		unhandled("movec ssh,y:(r)+");
		break;
		}
	case 2206: { // movec ssl,y:(r)+
		unhandled("movec ssl,y:(r)+");
		break;
		}
	case 2207: { // movec la,y:(r)+
		unhandled("movec la,y:(r)+");
		break;
		}
	case 2208: { // movec lc,y:(r)+
		unhandled("movec lc,y:(r)+");
		break;
		}
	case 2209: { // movec m,x:(r)
		unhandled("movec m,x:(r)");
		break;
		}
	case 2210: { // movec ep,x:(r)
		unhandled("movec ep,x:(r)");
		break;
		}
	case 2211: { // movec vba,x:(r)
		unhandled("movec vba,x:(r)");
		break;
		}
	case 2212: { // movec sc,x:(r)
		unhandled("movec sc,x:(r)");
		break;
		}
	case 2213: { // movec sz,x:(r)
		unhandled("movec sz,x:(r)");
		break;
		}
	case 2214: { // movec sr,x:(r)
		unhandled("movec sr,x:(r)");
		break;
		}
	case 2215: { // movec omr,x:(r)
		unhandled("movec omr,x:(r)");
		break;
		}
	case 2216: { // movec sp,x:(r)
		unhandled("movec sp,x:(r)");
		break;
		}
	case 2217: { // movec ssh,x:(r)
		unhandled("movec ssh,x:(r)");
		break;
		}
	case 2218: { // movec ssl,x:(r)
		unhandled("movec ssl,x:(r)");
		break;
		}
	case 2219: { // movec la,x:(r)
		unhandled("movec la,x:(r)");
		break;
		}
	case 2220: { // movec lc,x:(r)
		unhandled("movec lc,x:(r)");
		break;
		}
	case 2221: { // movec m,y:(r)
		unhandled("movec m,y:(r)");
		break;
		}
	case 2222: { // movec ep,y:(r)
		unhandled("movec ep,y:(r)");
		break;
		}
	case 2223: { // movec vba,y:(r)
		unhandled("movec vba,y:(r)");
		break;
		}
	case 2224: { // movec sc,y:(r)
		unhandled("movec sc,y:(r)");
		break;
		}
	case 2225: { // movec sz,y:(r)
		unhandled("movec sz,y:(r)");
		break;
		}
	case 2226: { // movec sr,y:(r)
		unhandled("movec sr,y:(r)");
		break;
		}
	case 2227: { // movec omr,y:(r)
		unhandled("movec omr,y:(r)");
		break;
		}
	case 2228: { // movec sp,y:(r)
		unhandled("movec sp,y:(r)");
		break;
		}
	case 2229: { // movec ssh,y:(r)
		unhandled("movec ssh,y:(r)");
		break;
		}
	case 2230: { // movec ssl,y:(r)
		unhandled("movec ssl,y:(r)");
		break;
		}
	case 2231: { // movec la,y:(r)
		unhandled("movec la,y:(r)");
		break;
		}
	case 2232: { // movec lc,y:(r)
		unhandled("movec lc,y:(r)");
		break;
		}
	case 2233: { // movec m,x:(r+n)
		unhandled("movec m,x:(r+n)");
		break;
		}
	case 2234: { // movec ep,x:(r+n)
		unhandled("movec ep,x:(r+n)");
		break;
		}
	case 2235: { // movec vba,x:(r+n)
		unhandled("movec vba,x:(r+n)");
		break;
		}
	case 2236: { // movec sc,x:(r+n)
		unhandled("movec sc,x:(r+n)");
		break;
		}
	case 2237: { // movec sz,x:(r+n)
		unhandled("movec sz,x:(r+n)");
		break;
		}
	case 2238: { // movec sr,x:(r+n)
		unhandled("movec sr,x:(r+n)");
		break;
		}
	case 2239: { // movec omr,x:(r+n)
		unhandled("movec omr,x:(r+n)");
		break;
		}
	case 2240: { // movec sp,x:(r+n)
		unhandled("movec sp,x:(r+n)");
		break;
		}
	case 2241: { // movec ssh,x:(r+n)
		unhandled("movec ssh,x:(r+n)");
		break;
		}
	case 2242: { // movec ssl,x:(r+n)
		unhandled("movec ssl,x:(r+n)");
		break;
		}
	case 2243: { // movec la,x:(r+n)
		unhandled("movec la,x:(r+n)");
		break;
		}
	case 2244: { // movec lc,x:(r+n)
		unhandled("movec lc,x:(r+n)");
		break;
		}
	case 2245: { // movec m,y:(r+n)
		unhandled("movec m,y:(r+n)");
		break;
		}
	case 2246: { // movec ep,y:(r+n)
		unhandled("movec ep,y:(r+n)");
		break;
		}
	case 2247: { // movec vba,y:(r+n)
		unhandled("movec vba,y:(r+n)");
		break;
		}
	case 2248: { // movec sc,y:(r+n)
		unhandled("movec sc,y:(r+n)");
		break;
		}
	case 2249: { // movec sz,y:(r+n)
		unhandled("movec sz,y:(r+n)");
		break;
		}
	case 2250: { // movec sr,y:(r+n)
		unhandled("movec sr,y:(r+n)");
		break;
		}
	case 2251: { // movec omr,y:(r+n)
		unhandled("movec omr,y:(r+n)");
		break;
		}
	case 2252: { // movec sp,y:(r+n)
		unhandled("movec sp,y:(r+n)");
		break;
		}
	case 2253: { // movec ssh,y:(r+n)
		unhandled("movec ssh,y:(r+n)");
		break;
		}
	case 2254: { // movec ssl,y:(r+n)
		unhandled("movec ssl,y:(r+n)");
		break;
		}
	case 2255: { // movec la,y:(r+n)
		unhandled("movec la,y:(r+n)");
		break;
		}
	case 2256: { // movec lc,y:(r+n)
		unhandled("movec lc,y:(r+n)");
		break;
		}
	case 2257: { // movec m,x:-(r)
		unhandled("movec m,x:-(r)");
		break;
		}
	case 2258: { // movec ep,x:-(r)
		unhandled("movec ep,x:-(r)");
		break;
		}
	case 2259: { // movec vba,x:-(r)
		unhandled("movec vba,x:-(r)");
		break;
		}
	case 2260: { // movec sc,x:-(r)
		unhandled("movec sc,x:-(r)");
		break;
		}
	case 2261: { // movec sz,x:-(r)
		unhandled("movec sz,x:-(r)");
		break;
		}
	case 2262: { // movec sr,x:-(r)
		unhandled("movec sr,x:-(r)");
		break;
		}
	case 2263: { // movec omr,x:-(r)
		unhandled("movec omr,x:-(r)");
		break;
		}
	case 2264: { // movec sp,x:-(r)
		unhandled("movec sp,x:-(r)");
		break;
		}
	case 2265: { // movec ssh,x:-(r)
		unhandled("movec ssh,x:-(r)");
		break;
		}
	case 2266: { // movec ssl,x:-(r)
		unhandled("movec ssl,x:-(r)");
		break;
		}
	case 2267: { // movec la,x:-(r)
		unhandled("movec la,x:-(r)");
		break;
		}
	case 2268: { // movec lc,x:-(r)
		unhandled("movec lc,x:-(r)");
		break;
		}
	case 2269: { // movec m,y:-(r)
		unhandled("movec m,y:-(r)");
		break;
		}
	case 2270: { // movec ep,y:-(r)
		unhandled("movec ep,y:-(r)");
		break;
		}
	case 2271: { // movec vba,y:-(r)
		unhandled("movec vba,y:-(r)");
		break;
		}
	case 2272: { // movec sc,y:-(r)
		unhandled("movec sc,y:-(r)");
		break;
		}
	case 2273: { // movec sz,y:-(r)
		unhandled("movec sz,y:-(r)");
		break;
		}
	case 2274: { // movec sr,y:-(r)
		unhandled("movec sr,y:-(r)");
		break;
		}
	case 2275: { // movec omr,y:-(r)
		unhandled("movec omr,y:-(r)");
		break;
		}
	case 2276: { // movec sp,y:-(r)
		unhandled("movec sp,y:-(r)");
		break;
		}
	case 2277: { // movec ssh,y:-(r)
		unhandled("movec ssh,y:-(r)");
		break;
		}
	case 2278: { // movec ssl,y:-(r)
		unhandled("movec ssl,y:-(r)");
		break;
		}
	case 2279: { // movec la,y:-(r)
		unhandled("movec la,y:-(r)");
		break;
		}
	case 2280: { // movec lc,y:-(r)
		unhandled("movec lc,y:-(r)");
		break;
		}
	case 2281: { // movec x:[abs],m
		unhandled("movec x:[abs],m");
		break;
		}
	case 2282: { // movec x:[abs],ep
		unhandled("movec x:[abs],ep");
		break;
		}
	case 2283: { // movec x:[abs],vba
		unhandled("movec x:[abs],vba");
		break;
		}
	case 2284: { // movec x:[abs],sc
		unhandled("movec x:[abs],sc");
		break;
		}
	case 2285: { // movec x:[abs],sz
		unhandled("movec x:[abs],sz");
		break;
		}
	case 2286: { // movec x:[abs],sr
		unhandled("movec x:[abs],sr");
		break;
		}
	case 2287: { // movec x:[abs],omr
		unhandled("movec x:[abs],omr");
		break;
		}
	case 2288: { // movec x:[abs],sp
		unhandled("movec x:[abs],sp");
		break;
		}
	case 2289: { // movec x:[abs],ssh
		unhandled("movec x:[abs],ssh");
		break;
		}
	case 2290: { // movec x:[abs],ssl
		unhandled("movec x:[abs],ssl");
		break;
		}
	case 2291: { // movec x:[abs],la
		unhandled("movec x:[abs],la");
		break;
		}
	case 2292: { // movec x:[abs],lc
		unhandled("movec x:[abs],lc");
		break;
		}
	case 2293: { // movec y:[abs],m
		unhandled("movec y:[abs],m");
		break;
		}
	case 2294: { // movec y:[abs],ep
		unhandled("movec y:[abs],ep");
		break;
		}
	case 2295: { // movec y:[abs],vba
		unhandled("movec y:[abs],vba");
		break;
		}
	case 2296: { // movec y:[abs],sc
		unhandled("movec y:[abs],sc");
		break;
		}
	case 2297: { // movec y:[abs],sz
		unhandled("movec y:[abs],sz");
		break;
		}
	case 2298: { // movec y:[abs],sr
		unhandled("movec y:[abs],sr");
		break;
		}
	case 2299: { // movec y:[abs],omr
		unhandled("movec y:[abs],omr");
		break;
		}
	case 2300: { // movec y:[abs],sp
		unhandled("movec y:[abs],sp");
		break;
		}
	case 2301: { // movec y:[abs],ssh
		unhandled("movec y:[abs],ssh");
		break;
		}
	case 2302: { // movec y:[abs],ssl
		unhandled("movec y:[abs],ssl");
		break;
		}
	case 2303: { // movec y:[abs],la
		unhandled("movec y:[abs],la");
		break;
		}
	case 2304: { // movec y:[abs],lc
		unhandled("movec y:[abs],lc");
		break;
		}
	case 2305: { // movec #[i],m
		unhandled("movec #[i],m");
		break;
		}
	case 2306: { // movec #[i],ep
		unhandled("movec #[i],ep");
		break;
		}
	case 2307: { // movec #[i],vba
		unhandled("movec #[i],vba");
		break;
		}
	case 2308: { // movec #[i],sc
		unhandled("movec #[i],sc");
		break;
		}
	case 2309: { // movec #[i],sz
		unhandled("movec #[i],sz");
		break;
		}
	case 2310: { // movec #[i],sr
		unhandled("movec #[i],sr");
		break;
		}
	case 2311: { // movec #[i],omr
		unhandled("movec #[i],omr");
		break;
		}
	case 2312: { // movec #[i],sp
		unhandled("movec #[i],sp");
		break;
		}
	case 2313: { // movec #[i],ssh
		unhandled("movec #[i],ssh");
		break;
		}
	case 2314: { // movec #[i],ssl
		unhandled("movec #[i],ssl");
		break;
		}
	case 2315: { // movec #[i],la
		unhandled("movec #[i],la");
		break;
		}
	case 2316: { // movec #[i],lc
		unhandled("movec #[i],lc");
		break;
		}
	case 2317: { // movec x:[aa],m
		unhandled("movec x:[aa],m");
		break;
		}
	case 2318: { // movec x:[aa],ep
		unhandled("movec x:[aa],ep");
		break;
		}
	case 2319: { // movec x:[aa],vba
		unhandled("movec x:[aa],vba");
		break;
		}
	case 2320: { // movec x:[aa],sc
		unhandled("movec x:[aa],sc");
		break;
		}
	case 2321: { // movec x:[aa],sz
		unhandled("movec x:[aa],sz");
		break;
		}
	case 2322: { // movec x:[aa],sr
		unhandled("movec x:[aa],sr");
		break;
		}
	case 2323: { // movec x:[aa],omr
		unhandled("movec x:[aa],omr");
		break;
		}
	case 2324: { // movec x:[aa],sp
		unhandled("movec x:[aa],sp");
		break;
		}
	case 2325: { // movec x:[aa],ssh
		unhandled("movec x:[aa],ssh");
		break;
		}
	case 2326: { // movec x:[aa],ssl
		unhandled("movec x:[aa],ssl");
		break;
		}
	case 2327: { // movec x:[aa],la
		unhandled("movec x:[aa],la");
		break;
		}
	case 2328: { // movec x:[aa],lc
		unhandled("movec x:[aa],lc");
		break;
		}
	case 2329: { // movec y:[aa],m
		unhandled("movec y:[aa],m");
		break;
		}
	case 2330: { // movec y:[aa],ep
		unhandled("movec y:[aa],ep");
		break;
		}
	case 2331: { // movec y:[aa],vba
		unhandled("movec y:[aa],vba");
		break;
		}
	case 2332: { // movec y:[aa],sc
		unhandled("movec y:[aa],sc");
		break;
		}
	case 2333: { // movec y:[aa],sz
		unhandled("movec y:[aa],sz");
		break;
		}
	case 2334: { // movec y:[aa],sr
		unhandled("movec y:[aa],sr");
		break;
		}
	case 2335: { // movec y:[aa],omr
		unhandled("movec y:[aa],omr");
		break;
		}
	case 2336: { // movec y:[aa],sp
		unhandled("movec y:[aa],sp");
		break;
		}
	case 2337: { // movec y:[aa],ssh
		unhandled("movec y:[aa],ssh");
		break;
		}
	case 2338: { // movec y:[aa],ssl
		unhandled("movec y:[aa],ssl");
		break;
		}
	case 2339: { // movec y:[aa],la
		unhandled("movec y:[aa],la");
		break;
		}
	case 2340: { // movec y:[aa],lc
		unhandled("movec y:[aa],lc");
		break;
		}
	case 2341: { // movec m,x:[aa]
		unhandled("movec m,x:[aa]");
		break;
		}
	case 2342: { // movec ep,x:[aa]
		unhandled("movec ep,x:[aa]");
		break;
		}
	case 2343: { // movec vba,x:[aa]
		unhandled("movec vba,x:[aa]");
		break;
		}
	case 2344: { // movec sc,x:[aa]
		unhandled("movec sc,x:[aa]");
		break;
		}
	case 2345: { // movec sz,x:[aa]
		unhandled("movec sz,x:[aa]");
		break;
		}
	case 2346: { // movec sr,x:[aa]
		unhandled("movec sr,x:[aa]");
		break;
		}
	case 2347: { // movec omr,x:[aa]
		unhandled("movec omr,x:[aa]");
		break;
		}
	case 2348: { // movec sp,x:[aa]
		unhandled("movec sp,x:[aa]");
		break;
		}
	case 2349: { // movec ssh,x:[aa]
		unhandled("movec ssh,x:[aa]");
		break;
		}
	case 2350: { // movec ssl,x:[aa]
		unhandled("movec ssl,x:[aa]");
		break;
		}
	case 2351: { // movec la,x:[aa]
		unhandled("movec la,x:[aa]");
		break;
		}
	case 2352: { // movec lc,x:[aa]
		unhandled("movec lc,x:[aa]");
		break;
		}
	case 2353: { // movec m,y:[aa]
		unhandled("movec m,y:[aa]");
		break;
		}
	case 2354: { // movec ep,y:[aa]
		unhandled("movec ep,y:[aa]");
		break;
		}
	case 2355: { // movec vba,y:[aa]
		unhandled("movec vba,y:[aa]");
		break;
		}
	case 2356: { // movec sc,y:[aa]
		unhandled("movec sc,y:[aa]");
		break;
		}
	case 2357: { // movec sz,y:[aa]
		unhandled("movec sz,y:[aa]");
		break;
		}
	case 2358: { // movec sr,y:[aa]
		unhandled("movec sr,y:[aa]");
		break;
		}
	case 2359: { // movec omr,y:[aa]
		unhandled("movec omr,y:[aa]");
		break;
		}
	case 2360: { // movec sp,y:[aa]
		unhandled("movec sp,y:[aa]");
		break;
		}
	case 2361: { // movec ssh,y:[aa]
		unhandled("movec ssh,y:[aa]");
		break;
		}
	case 2362: { // movec ssl,y:[aa]
		unhandled("movec ssl,y:[aa]");
		break;
		}
	case 2363: { // movec la,y:[aa]
		unhandled("movec la,y:[aa]");
		break;
		}
	case 2364: { // movec lc,y:[aa]
		unhandled("movec lc,y:[aa]");
		break;
		}
	case 2365: { // movec x0,m
		unhandled("movec x0,m");
		break;
		}
	case 2366: { // movec x0,ep
		unhandled("movec x0,ep");
		break;
		}
	case 2367: { // movec x0,vba
		unhandled("movec x0,vba");
		break;
		}
	case 2368: { // movec x0,sc
		unhandled("movec x0,sc");
		break;
		}
	case 2369: { // movec x0,sz
		unhandled("movec x0,sz");
		break;
		}
	case 2370: { // movec x0,sr
		unhandled("movec x0,sr");
		break;
		}
	case 2371: { // movec x0,omr
		unhandled("movec x0,omr");
		break;
		}
	case 2372: { // movec x0,sp
		unhandled("movec x0,sp");
		break;
		}
	case 2373: { // movec x0,ssh
		unhandled("movec x0,ssh");
		break;
		}
	case 2374: { // movec x0,ssl
		unhandled("movec x0,ssl");
		break;
		}
	case 2375: { // movec x0,la
		unhandled("movec x0,la");
		break;
		}
	case 2376: { // movec x0,lc
		unhandled("movec x0,lc");
		break;
		}
	case 2377: { // movec x1,m
		unhandled("movec x1,m");
		break;
		}
	case 2378: { // movec x1,ep
		unhandled("movec x1,ep");
		break;
		}
	case 2379: { // movec x1,vba
		unhandled("movec x1,vba");
		break;
		}
	case 2380: { // movec x1,sc
		unhandled("movec x1,sc");
		break;
		}
	case 2381: { // movec x1,sz
		unhandled("movec x1,sz");
		break;
		}
	case 2382: { // movec x1,sr
		unhandled("movec x1,sr");
		break;
		}
	case 2383: { // movec x1,omr
		unhandled("movec x1,omr");
		break;
		}
	case 2384: { // movec x1,sp
		unhandled("movec x1,sp");
		break;
		}
	case 2385: { // movec x1,ssh
		unhandled("movec x1,ssh");
		break;
		}
	case 2386: { // movec x1,ssl
		unhandled("movec x1,ssl");
		break;
		}
	case 2387: { // movec x1,la
		unhandled("movec x1,la");
		break;
		}
	case 2388: { // movec x1,lc
		unhandled("movec x1,lc");
		break;
		}
	case 2389: { // movec y0,m
		unhandled("movec y0,m");
		break;
		}
	case 2390: { // movec y0,ep
		unhandled("movec y0,ep");
		break;
		}
	case 2391: { // movec y0,vba
		unhandled("movec y0,vba");
		break;
		}
	case 2392: { // movec y0,sc
		unhandled("movec y0,sc");
		break;
		}
	case 2393: { // movec y0,sz
		unhandled("movec y0,sz");
		break;
		}
	case 2394: { // movec y0,sr
		unhandled("movec y0,sr");
		break;
		}
	case 2395: { // movec y0,omr
		unhandled("movec y0,omr");
		break;
		}
	case 2396: { // movec y0,sp
		unhandled("movec y0,sp");
		break;
		}
	case 2397: { // movec y0,ssh
		unhandled("movec y0,ssh");
		break;
		}
	case 2398: { // movec y0,ssl
		unhandled("movec y0,ssl");
		break;
		}
	case 2399: { // movec y0,la
		unhandled("movec y0,la");
		break;
		}
	case 2400: { // movec y0,lc
		unhandled("movec y0,lc");
		break;
		}
	case 2401: { // movec y1,m
		unhandled("movec y1,m");
		break;
		}
	case 2402: { // movec y1,ep
		unhandled("movec y1,ep");
		break;
		}
	case 2403: { // movec y1,vba
		unhandled("movec y1,vba");
		break;
		}
	case 2404: { // movec y1,sc
		unhandled("movec y1,sc");
		break;
		}
	case 2405: { // movec y1,sz
		unhandled("movec y1,sz");
		break;
		}
	case 2406: { // movec y1,sr
		unhandled("movec y1,sr");
		break;
		}
	case 2407: { // movec y1,omr
		unhandled("movec y1,omr");
		break;
		}
	case 2408: { // movec y1,sp
		unhandled("movec y1,sp");
		break;
		}
	case 2409: { // movec y1,ssh
		unhandled("movec y1,ssh");
		break;
		}
	case 2410: { // movec y1,ssl
		unhandled("movec y1,ssl");
		break;
		}
	case 2411: { // movec y1,la
		unhandled("movec y1,la");
		break;
		}
	case 2412: { // movec y1,lc
		unhandled("movec y1,lc");
		break;
		}
	case 2413: { // movec a0,m
		unhandled("movec a0,m");
		break;
		}
	case 2414: { // movec a0,ep
		unhandled("movec a0,ep");
		break;
		}
	case 2415: { // movec a0,vba
		unhandled("movec a0,vba");
		break;
		}
	case 2416: { // movec a0,sc
		unhandled("movec a0,sc");
		break;
		}
	case 2417: { // movec a0,sz
		unhandled("movec a0,sz");
		break;
		}
	case 2418: { // movec a0,sr
		unhandled("movec a0,sr");
		break;
		}
	case 2419: { // movec a0,omr
		unhandled("movec a0,omr");
		break;
		}
	case 2420: { // movec a0,sp
		unhandled("movec a0,sp");
		break;
		}
	case 2421: { // movec a0,ssh
		unhandled("movec a0,ssh");
		break;
		}
	case 2422: { // movec a0,ssl
		unhandled("movec a0,ssl");
		break;
		}
	case 2423: { // movec a0,la
		unhandled("movec a0,la");
		break;
		}
	case 2424: { // movec a0,lc
		unhandled("movec a0,lc");
		break;
		}
	case 2425: { // movec b0,m
		unhandled("movec b0,m");
		break;
		}
	case 2426: { // movec b0,ep
		unhandled("movec b0,ep");
		break;
		}
	case 2427: { // movec b0,vba
		unhandled("movec b0,vba");
		break;
		}
	case 2428: { // movec b0,sc
		unhandled("movec b0,sc");
		break;
		}
	case 2429: { // movec b0,sz
		unhandled("movec b0,sz");
		break;
		}
	case 2430: { // movec b0,sr
		unhandled("movec b0,sr");
		break;
		}
	case 2431: { // movec b0,omr
		unhandled("movec b0,omr");
		break;
		}
	case 2432: { // movec b0,sp
		unhandled("movec b0,sp");
		break;
		}
	case 2433: { // movec b0,ssh
		unhandled("movec b0,ssh");
		break;
		}
	case 2434: { // movec b0,ssl
		unhandled("movec b0,ssl");
		break;
		}
	case 2435: { // movec b0,la
		unhandled("movec b0,la");
		break;
		}
	case 2436: { // movec b0,lc
		unhandled("movec b0,lc");
		break;
		}
	case 2437: { // movec a2,m
		unhandled("movec a2,m");
		break;
		}
	case 2438: { // movec a2,ep
		unhandled("movec a2,ep");
		break;
		}
	case 2439: { // movec a2,vba
		unhandled("movec a2,vba");
		break;
		}
	case 2440: { // movec a2,sc
		unhandled("movec a2,sc");
		break;
		}
	case 2441: { // movec a2,sz
		unhandled("movec a2,sz");
		break;
		}
	case 2442: { // movec a2,sr
		unhandled("movec a2,sr");
		break;
		}
	case 2443: { // movec a2,omr
		unhandled("movec a2,omr");
		break;
		}
	case 2444: { // movec a2,sp
		unhandled("movec a2,sp");
		break;
		}
	case 2445: { // movec a2,ssh
		unhandled("movec a2,ssh");
		break;
		}
	case 2446: { // movec a2,ssl
		unhandled("movec a2,ssl");
		break;
		}
	case 2447: { // movec a2,la
		unhandled("movec a2,la");
		break;
		}
	case 2448: { // movec a2,lc
		unhandled("movec a2,lc");
		break;
		}
	case 2449: { // movec b2,m
		unhandled("movec b2,m");
		break;
		}
	case 2450: { // movec b2,ep
		unhandled("movec b2,ep");
		break;
		}
	case 2451: { // movec b2,vba
		unhandled("movec b2,vba");
		break;
		}
	case 2452: { // movec b2,sc
		unhandled("movec b2,sc");
		break;
		}
	case 2453: { // movec b2,sz
		unhandled("movec b2,sz");
		break;
		}
	case 2454: { // movec b2,sr
		unhandled("movec b2,sr");
		break;
		}
	case 2455: { // movec b2,omr
		unhandled("movec b2,omr");
		break;
		}
	case 2456: { // movec b2,sp
		unhandled("movec b2,sp");
		break;
		}
	case 2457: { // movec b2,ssh
		unhandled("movec b2,ssh");
		break;
		}
	case 2458: { // movec b2,ssl
		unhandled("movec b2,ssl");
		break;
		}
	case 2459: { // movec b2,la
		unhandled("movec b2,la");
		break;
		}
	case 2460: { // movec b2,lc
		unhandled("movec b2,lc");
		break;
		}
	case 2461: { // movec a1,m
		unhandled("movec a1,m");
		break;
		}
	case 2462: { // movec a1,ep
		unhandled("movec a1,ep");
		break;
		}
	case 2463: { // movec a1,vba
		unhandled("movec a1,vba");
		break;
		}
	case 2464: { // movec a1,sc
		unhandled("movec a1,sc");
		break;
		}
	case 2465: { // movec a1,sz
		unhandled("movec a1,sz");
		break;
		}
	case 2466: { // movec a1,sr
		unhandled("movec a1,sr");
		break;
		}
	case 2467: { // movec a1,omr
		unhandled("movec a1,omr");
		break;
		}
	case 2468: { // movec a1,sp
		unhandled("movec a1,sp");
		break;
		}
	case 2469: { // movec a1,ssh
		unhandled("movec a1,ssh");
		break;
		}
	case 2470: { // movec a1,ssl
		unhandled("movec a1,ssl");
		break;
		}
	case 2471: { // movec a1,la
		unhandled("movec a1,la");
		break;
		}
	case 2472: { // movec a1,lc
		unhandled("movec a1,lc");
		break;
		}
	case 2473: { // movec b1,m
		unhandled("movec b1,m");
		break;
		}
	case 2474: { // movec b1,ep
		unhandled("movec b1,ep");
		break;
		}
	case 2475: { // movec b1,vba
		unhandled("movec b1,vba");
		break;
		}
	case 2476: { // movec b1,sc
		unhandled("movec b1,sc");
		break;
		}
	case 2477: { // movec b1,sz
		unhandled("movec b1,sz");
		break;
		}
	case 2478: { // movec b1,sr
		unhandled("movec b1,sr");
		break;
		}
	case 2479: { // movec b1,omr
		unhandled("movec b1,omr");
		break;
		}
	case 2480: { // movec b1,sp
		unhandled("movec b1,sp");
		break;
		}
	case 2481: { // movec b1,ssh
		unhandled("movec b1,ssh");
		break;
		}
	case 2482: { // movec b1,ssl
		unhandled("movec b1,ssl");
		break;
		}
	case 2483: { // movec b1,la
		unhandled("movec b1,la");
		break;
		}
	case 2484: { // movec b1,lc
		unhandled("movec b1,lc");
		break;
		}
	case 2485: { // movec a,m
		unhandled("movec a,m");
		break;
		}
	case 2486: { // movec a,ep
		unhandled("movec a,ep");
		break;
		}
	case 2487: { // movec a,vba
		unhandled("movec a,vba");
		break;
		}
	case 2488: { // movec a,sc
		unhandled("movec a,sc");
		break;
		}
	case 2489: { // movec a,sz
		unhandled("movec a,sz");
		break;
		}
	case 2490: { // movec a,sr
		unhandled("movec a,sr");
		break;
		}
	case 2491: { // movec a,omr
		unhandled("movec a,omr");
		break;
		}
	case 2492: { // movec a,sp
		unhandled("movec a,sp");
		break;
		}
	case 2493: { // movec a,ssh
		unhandled("movec a,ssh");
		break;
		}
	case 2494: { // movec a,ssl
		unhandled("movec a,ssl");
		break;
		}
	case 2495: { // movec a,la
		unhandled("movec a,la");
		break;
		}
	case 2496: { // movec a,lc
		unhandled("movec a,lc");
		break;
		}
	case 2497: { // movec b,m
		unhandled("movec b,m");
		break;
		}
	case 2498: { // movec b,ep
		unhandled("movec b,ep");
		break;
		}
	case 2499: { // movec b,vba
		unhandled("movec b,vba");
		break;
		}
	case 2500: { // movec b,sc
		unhandled("movec b,sc");
		break;
		}
	case 2501: { // movec b,sz
		unhandled("movec b,sz");
		break;
		}
	case 2502: { // movec b,sr
		unhandled("movec b,sr");
		break;
		}
	case 2503: { // movec b,omr
		unhandled("movec b,omr");
		break;
		}
	case 2504: { // movec b,sp
		unhandled("movec b,sp");
		break;
		}
	case 2505: { // movec b,ssh
		unhandled("movec b,ssh");
		break;
		}
	case 2506: { // movec b,ssl
		unhandled("movec b,ssl");
		break;
		}
	case 2507: { // movec b,la
		unhandled("movec b,la");
		break;
		}
	case 2508: { // movec b,lc
		unhandled("movec b,lc");
		break;
		}
	case 2509: { // movec r,m
		unhandled("movec r,m");
		break;
		}
	case 2510: { // movec r,ep
		unhandled("movec r,ep");
		break;
		}
	case 2511: { // movec r,vba
		unhandled("movec r,vba");
		break;
		}
	case 2512: { // movec r,sc
		unhandled("movec r,sc");
		break;
		}
	case 2513: { // movec r,sz
		unhandled("movec r,sz");
		break;
		}
	case 2514: { // movec r,sr
		unhandled("movec r,sr");
		break;
		}
	case 2515: { // movec r,omr
		unhandled("movec r,omr");
		break;
		}
	case 2516: { // movec r,sp
		unhandled("movec r,sp");
		break;
		}
	case 2517: { // movec r,ssh
		unhandled("movec r,ssh");
		break;
		}
	case 2518: { // movec r,ssl
		unhandled("movec r,ssl");
		break;
		}
	case 2519: { // movec r,la
		unhandled("movec r,la");
		break;
		}
	case 2520: { // movec r,lc
		unhandled("movec r,lc");
		break;
		}
	case 2521: { // movec n,m
		unhandled("movec n,m");
		break;
		}
	case 2522: { // movec n,ep
		unhandled("movec n,ep");
		break;
		}
	case 2523: { // movec n,vba
		unhandled("movec n,vba");
		break;
		}
	case 2524: { // movec n,sc
		unhandled("movec n,sc");
		break;
		}
	case 2525: { // movec n,sz
		unhandled("movec n,sz");
		break;
		}
	case 2526: { // movec n,sr
		unhandled("movec n,sr");
		break;
		}
	case 2527: { // movec n,omr
		unhandled("movec n,omr");
		break;
		}
	case 2528: { // movec n,sp
		unhandled("movec n,sp");
		break;
		}
	case 2529: { // movec n,ssh
		unhandled("movec n,ssh");
		break;
		}
	case 2530: { // movec n,ssl
		unhandled("movec n,ssl");
		break;
		}
	case 2531: { // movec n,la
		unhandled("movec n,la");
		break;
		}
	case 2532: { // movec n,lc
		unhandled("movec n,lc");
		break;
		}
	case 2533: { // movec m,m
		unhandled("movec m,m");
		break;
		}
	case 2534: { // movec m,ep
		unhandled("movec m,ep");
		break;
		}
	case 2535: { // movec m,vba
		unhandled("movec m,vba");
		break;
		}
	case 2536: { // movec m,sc
		unhandled("movec m,sc");
		break;
		}
	case 2537: { // movec m,sz
		unhandled("movec m,sz");
		break;
		}
	case 2538: { // movec m,sr
		unhandled("movec m,sr");
		break;
		}
	case 2539: { // movec m,omr
		unhandled("movec m,omr");
		break;
		}
	case 2540: { // movec m,sp
		unhandled("movec m,sp");
		break;
		}
	case 2541: { // movec m,ssh
		unhandled("movec m,ssh");
		break;
		}
	case 2542: { // movec m,ssl
		unhandled("movec m,ssl");
		break;
		}
	case 2543: { // movec m,la
		unhandled("movec m,la");
		break;
		}
	case 2544: { // movec m,lc
		unhandled("movec m,lc");
		break;
		}
	case 2545: { // movec ep,m
		unhandled("movec ep,m");
		break;
		}
	case 2546: { // movec ep,ep
		unhandled("movec ep,ep");
		break;
		}
	case 2547: { // movec ep,vba
		unhandled("movec ep,vba");
		break;
		}
	case 2548: { // movec ep,sc
		unhandled("movec ep,sc");
		break;
		}
	case 2549: { // movec ep,sz
		unhandled("movec ep,sz");
		break;
		}
	case 2550: { // movec ep,sr
		unhandled("movec ep,sr");
		break;
		}
	case 2551: { // movec ep,omr
		unhandled("movec ep,omr");
		break;
		}
	case 2552: { // movec ep,sp
		unhandled("movec ep,sp");
		break;
		}
	case 2553: { // movec ep,ssh
		unhandled("movec ep,ssh");
		break;
		}
	case 2554: { // movec ep,ssl
		unhandled("movec ep,ssl");
		break;
		}
	case 2555: { // movec ep,la
		unhandled("movec ep,la");
		break;
		}
	case 2556: { // movec ep,lc
		unhandled("movec ep,lc");
		break;
		}
	case 2557: { // movec vba,m
		unhandled("movec vba,m");
		break;
		}
	case 2558: { // movec vba,ep
		unhandled("movec vba,ep");
		break;
		}
	case 2559: { // movec vba,vba
		unhandled("movec vba,vba");
		break;
		}
	case 2560: { // movec vba,sc
		unhandled("movec vba,sc");
		break;
		}
	case 2561: { // movec vba,sz
		unhandled("movec vba,sz");
		break;
		}
	case 2562: { // movec vba,sr
		unhandled("movec vba,sr");
		break;
		}
	case 2563: { // movec vba,omr
		unhandled("movec vba,omr");
		break;
		}
	case 2564: { // movec vba,sp
		unhandled("movec vba,sp");
		break;
		}
	case 2565: { // movec vba,ssh
		unhandled("movec vba,ssh");
		break;
		}
	case 2566: { // movec vba,ssl
		unhandled("movec vba,ssl");
		break;
		}
	case 2567: { // movec vba,la
		unhandled("movec vba,la");
		break;
		}
	case 2568: { // movec vba,lc
		unhandled("movec vba,lc");
		break;
		}
	case 2569: { // movec sc,m
		unhandled("movec sc,m");
		break;
		}
	case 2570: { // movec sc,ep
		unhandled("movec sc,ep");
		break;
		}
	case 2571: { // movec sc,vba
		unhandled("movec sc,vba");
		break;
		}
	case 2572: { // movec sc,sc
		unhandled("movec sc,sc");
		break;
		}
	case 2573: { // movec sc,sz
		unhandled("movec sc,sz");
		break;
		}
	case 2574: { // movec sc,sr
		unhandled("movec sc,sr");
		break;
		}
	case 2575: { // movec sc,omr
		unhandled("movec sc,omr");
		break;
		}
	case 2576: { // movec sc,sp
		unhandled("movec sc,sp");
		break;
		}
	case 2577: { // movec sc,ssh
		unhandled("movec sc,ssh");
		break;
		}
	case 2578: { // movec sc,ssl
		unhandled("movec sc,ssl");
		break;
		}
	case 2579: { // movec sc,la
		unhandled("movec sc,la");
		break;
		}
	case 2580: { // movec sc,lc
		unhandled("movec sc,lc");
		break;
		}
	case 2581: { // movec sz,m
		unhandled("movec sz,m");
		break;
		}
	case 2582: { // movec sz,ep
		unhandled("movec sz,ep");
		break;
		}
	case 2583: { // movec sz,vba
		unhandled("movec sz,vba");
		break;
		}
	case 2584: { // movec sz,sc
		unhandled("movec sz,sc");
		break;
		}
	case 2585: { // movec sz,sz
		unhandled("movec sz,sz");
		break;
		}
	case 2586: { // movec sz,sr
		unhandled("movec sz,sr");
		break;
		}
	case 2587: { // movec sz,omr
		unhandled("movec sz,omr");
		break;
		}
	case 2588: { // movec sz,sp
		unhandled("movec sz,sp");
		break;
		}
	case 2589: { // movec sz,ssh
		unhandled("movec sz,ssh");
		break;
		}
	case 2590: { // movec sz,ssl
		unhandled("movec sz,ssl");
		break;
		}
	case 2591: { // movec sz,la
		unhandled("movec sz,la");
		break;
		}
	case 2592: { // movec sz,lc
		unhandled("movec sz,lc");
		break;
		}
	case 2593: { // movec sr,m
		unhandled("movec sr,m");
		break;
		}
	case 2594: { // movec sr,ep
		unhandled("movec sr,ep");
		break;
		}
	case 2595: { // movec sr,vba
		unhandled("movec sr,vba");
		break;
		}
	case 2596: { // movec sr,sc
		unhandled("movec sr,sc");
		break;
		}
	case 2597: { // movec sr,sz
		unhandled("movec sr,sz");
		break;
		}
	case 2598: { // movec sr,sr
		unhandled("movec sr,sr");
		break;
		}
	case 2599: { // movec sr,omr
		unhandled("movec sr,omr");
		break;
		}
	case 2600: { // movec sr,sp
		unhandled("movec sr,sp");
		break;
		}
	case 2601: { // movec sr,ssh
		unhandled("movec sr,ssh");
		break;
		}
	case 2602: { // movec sr,ssl
		unhandled("movec sr,ssl");
		break;
		}
	case 2603: { // movec sr,la
		unhandled("movec sr,la");
		break;
		}
	case 2604: { // movec sr,lc
		unhandled("movec sr,lc");
		break;
		}
	case 2605: { // movec omr,m
		unhandled("movec omr,m");
		break;
		}
	case 2606: { // movec omr,ep
		unhandled("movec omr,ep");
		break;
		}
	case 2607: { // movec omr,vba
		unhandled("movec omr,vba");
		break;
		}
	case 2608: { // movec omr,sc
		unhandled("movec omr,sc");
		break;
		}
	case 2609: { // movec omr,sz
		unhandled("movec omr,sz");
		break;
		}
	case 2610: { // movec omr,sr
		unhandled("movec omr,sr");
		break;
		}
	case 2611: { // movec omr,omr
		unhandled("movec omr,omr");
		break;
		}
	case 2612: { // movec omr,sp
		unhandled("movec omr,sp");
		break;
		}
	case 2613: { // movec omr,ssh
		unhandled("movec omr,ssh");
		break;
		}
	case 2614: { // movec omr,ssl
		unhandled("movec omr,ssl");
		break;
		}
	case 2615: { // movec omr,la
		unhandled("movec omr,la");
		break;
		}
	case 2616: { // movec omr,lc
		unhandled("movec omr,lc");
		break;
		}
	case 2617: { // movec sp,m
		unhandled("movec sp,m");
		break;
		}
	case 2618: { // movec sp,ep
		unhandled("movec sp,ep");
		break;
		}
	case 2619: { // movec sp,vba
		unhandled("movec sp,vba");
		break;
		}
	case 2620: { // movec sp,sc
		unhandled("movec sp,sc");
		break;
		}
	case 2621: { // movec sp,sz
		unhandled("movec sp,sz");
		break;
		}
	case 2622: { // movec sp,sr
		unhandled("movec sp,sr");
		break;
		}
	case 2623: { // movec sp,omr
		unhandled("movec sp,omr");
		break;
		}
	case 2624: { // movec sp,sp
		unhandled("movec sp,sp");
		break;
		}
	case 2625: { // movec sp,ssh
		unhandled("movec sp,ssh");
		break;
		}
	case 2626: { // movec sp,ssl
		unhandled("movec sp,ssl");
		break;
		}
	case 2627: { // movec sp,la
		unhandled("movec sp,la");
		break;
		}
	case 2628: { // movec sp,lc
		unhandled("movec sp,lc");
		break;
		}
	case 2629: { // movec ssh,m
		unhandled("movec ssh,m");
		break;
		}
	case 2630: { // movec ssh,ep
		unhandled("movec ssh,ep");
		break;
		}
	case 2631: { // movec ssh,vba
		unhandled("movec ssh,vba");
		break;
		}
	case 2632: { // movec ssh,sc
		unhandled("movec ssh,sc");
		break;
		}
	case 2633: { // movec ssh,sz
		unhandled("movec ssh,sz");
		break;
		}
	case 2634: { // movec ssh,sr
		unhandled("movec ssh,sr");
		break;
		}
	case 2635: { // movec ssh,omr
		unhandled("movec ssh,omr");
		break;
		}
	case 2636: { // movec ssh,sp
		unhandled("movec ssh,sp");
		break;
		}
	case 2637: { // movec ssh,ssh
		unhandled("movec ssh,ssh");
		break;
		}
	case 2638: { // movec ssh,ssl
		unhandled("movec ssh,ssl");
		break;
		}
	case 2639: { // movec ssh,la
		unhandled("movec ssh,la");
		break;
		}
	case 2640: { // movec ssh,lc
		unhandled("movec ssh,lc");
		break;
		}
	case 2641: { // movec ssl,m
		unhandled("movec ssl,m");
		break;
		}
	case 2642: { // movec ssl,ep
		unhandled("movec ssl,ep");
		break;
		}
	case 2643: { // movec ssl,vba
		unhandled("movec ssl,vba");
		break;
		}
	case 2644: { // movec ssl,sc
		unhandled("movec ssl,sc");
		break;
		}
	case 2645: { // movec ssl,sz
		unhandled("movec ssl,sz");
		break;
		}
	case 2646: { // movec ssl,sr
		unhandled("movec ssl,sr");
		break;
		}
	case 2647: { // movec ssl,omr
		unhandled("movec ssl,omr");
		break;
		}
	case 2648: { // movec ssl,sp
		unhandled("movec ssl,sp");
		break;
		}
	case 2649: { // movec ssl,ssh
		unhandled("movec ssl,ssh");
		break;
		}
	case 2650: { // movec ssl,ssl
		unhandled("movec ssl,ssl");
		break;
		}
	case 2651: { // movec ssl,la
		unhandled("movec ssl,la");
		break;
		}
	case 2652: { // movec ssl,lc
		unhandled("movec ssl,lc");
		break;
		}
	case 2653: { // movec la,m
		unhandled("movec la,m");
		break;
		}
	case 2654: { // movec la,ep
		unhandled("movec la,ep");
		break;
		}
	case 2655: { // movec la,vba
		unhandled("movec la,vba");
		break;
		}
	case 2656: { // movec la,sc
		unhandled("movec la,sc");
		break;
		}
	case 2657: { // movec la,sz
		unhandled("movec la,sz");
		break;
		}
	case 2658: { // movec la,sr
		unhandled("movec la,sr");
		break;
		}
	case 2659: { // movec la,omr
		unhandled("movec la,omr");
		break;
		}
	case 2660: { // movec la,sp
		unhandled("movec la,sp");
		break;
		}
	case 2661: { // movec la,ssh
		unhandled("movec la,ssh");
		break;
		}
	case 2662: { // movec la,ssl
		unhandled("movec la,ssl");
		break;
		}
	case 2663: { // movec la,la
		unhandled("movec la,la");
		break;
		}
	case 2664: { // movec la,lc
		unhandled("movec la,lc");
		break;
		}
	case 2665: { // movec lc,m
		unhandled("movec lc,m");
		break;
		}
	case 2666: { // movec lc,ep
		unhandled("movec lc,ep");
		break;
		}
	case 2667: { // movec lc,vba
		unhandled("movec lc,vba");
		break;
		}
	case 2668: { // movec lc,sc
		unhandled("movec lc,sc");
		break;
		}
	case 2669: { // movec lc,sz
		unhandled("movec lc,sz");
		break;
		}
	case 2670: { // movec lc,sr
		unhandled("movec lc,sr");
		break;
		}
	case 2671: { // movec lc,omr
		unhandled("movec lc,omr");
		break;
		}
	case 2672: { // movec lc,sp
		unhandled("movec lc,sp");
		break;
		}
	case 2673: { // movec lc,ssh
		unhandled("movec lc,ssh");
		break;
		}
	case 2674: { // movec lc,ssl
		unhandled("movec lc,ssl");
		break;
		}
	case 2675: { // movec lc,la
		unhandled("movec lc,la");
		break;
		}
	case 2676: { // movec lc,lc
		unhandled("movec lc,lc");
		break;
		}
	case 2677: { // movec m,x0
		unhandled("movec m,x0");
		break;
		}
	case 2678: { // movec ep,x0
		unhandled("movec ep,x0");
		break;
		}
	case 2679: { // movec vba,x0
		unhandled("movec vba,x0");
		break;
		}
	case 2680: { // movec sc,x0
		unhandled("movec sc,x0");
		break;
		}
	case 2681: { // movec sz,x0
		unhandled("movec sz,x0");
		break;
		}
	case 2682: { // movec sr,x0
		unhandled("movec sr,x0");
		break;
		}
	case 2683: { // movec omr,x0
		unhandled("movec omr,x0");
		break;
		}
	case 2684: { // movec sp,x0
		unhandled("movec sp,x0");
		break;
		}
	case 2685: { // movec ssh,x0
		unhandled("movec ssh,x0");
		break;
		}
	case 2686: { // movec ssl,x0
		unhandled("movec ssl,x0");
		break;
		}
	case 2687: { // movec la,x0
		unhandled("movec la,x0");
		break;
		}
	case 2688: { // movec lc,x0
		unhandled("movec lc,x0");
		break;
		}
	case 2689: { // movec m,x1
		unhandled("movec m,x1");
		break;
		}
	case 2690: { // movec ep,x1
		unhandled("movec ep,x1");
		break;
		}
	case 2691: { // movec vba,x1
		unhandled("movec vba,x1");
		break;
		}
	case 2692: { // movec sc,x1
		unhandled("movec sc,x1");
		break;
		}
	case 2693: { // movec sz,x1
		unhandled("movec sz,x1");
		break;
		}
	case 2694: { // movec sr,x1
		unhandled("movec sr,x1");
		break;
		}
	case 2695: { // movec omr,x1
		unhandled("movec omr,x1");
		break;
		}
	case 2696: { // movec sp,x1
		unhandled("movec sp,x1");
		break;
		}
	case 2697: { // movec ssh,x1
		unhandled("movec ssh,x1");
		break;
		}
	case 2698: { // movec ssl,x1
		unhandled("movec ssl,x1");
		break;
		}
	case 2699: { // movec la,x1
		unhandled("movec la,x1");
		break;
		}
	case 2700: { // movec lc,x1
		unhandled("movec lc,x1");
		break;
		}
	case 2701: { // movec m,y0
		unhandled("movec m,y0");
		break;
		}
	case 2702: { // movec ep,y0
		unhandled("movec ep,y0");
		break;
		}
	case 2703: { // movec vba,y0
		unhandled("movec vba,y0");
		break;
		}
	case 2704: { // movec sc,y0
		unhandled("movec sc,y0");
		break;
		}
	case 2705: { // movec sz,y0
		unhandled("movec sz,y0");
		break;
		}
	case 2706: { // movec sr,y0
		unhandled("movec sr,y0");
		break;
		}
	case 2707: { // movec omr,y0
		unhandled("movec omr,y0");
		break;
		}
	case 2708: { // movec sp,y0
		unhandled("movec sp,y0");
		break;
		}
	case 2709: { // movec ssh,y0
		unhandled("movec ssh,y0");
		break;
		}
	case 2710: { // movec ssl,y0
		unhandled("movec ssl,y0");
		break;
		}
	case 2711: { // movec la,y0
		unhandled("movec la,y0");
		break;
		}
	case 2712: { // movec lc,y0
		unhandled("movec lc,y0");
		break;
		}
	case 2713: { // movec m,y1
		unhandled("movec m,y1");
		break;
		}
	case 2714: { // movec ep,y1
		unhandled("movec ep,y1");
		break;
		}
	case 2715: { // movec vba,y1
		unhandled("movec vba,y1");
		break;
		}
	case 2716: { // movec sc,y1
		unhandled("movec sc,y1");
		break;
		}
	case 2717: { // movec sz,y1
		unhandled("movec sz,y1");
		break;
		}
	case 2718: { // movec sr,y1
		unhandled("movec sr,y1");
		break;
		}
	case 2719: { // movec omr,y1
		unhandled("movec omr,y1");
		break;
		}
	case 2720: { // movec sp,y1
		unhandled("movec sp,y1");
		break;
		}
	case 2721: { // movec ssh,y1
		unhandled("movec ssh,y1");
		break;
		}
	case 2722: { // movec ssl,y1
		unhandled("movec ssl,y1");
		break;
		}
	case 2723: { // movec la,y1
		unhandled("movec la,y1");
		break;
		}
	case 2724: { // movec lc,y1
		unhandled("movec lc,y1");
		break;
		}
	case 2725: { // movec m,a0
		unhandled("movec m,a0");
		break;
		}
	case 2726: { // movec ep,a0
		unhandled("movec ep,a0");
		break;
		}
	case 2727: { // movec vba,a0
		unhandled("movec vba,a0");
		break;
		}
	case 2728: { // movec sc,a0
		unhandled("movec sc,a0");
		break;
		}
	case 2729: { // movec sz,a0
		unhandled("movec sz,a0");
		break;
		}
	case 2730: { // movec sr,a0
		unhandled("movec sr,a0");
		break;
		}
	case 2731: { // movec omr,a0
		unhandled("movec omr,a0");
		break;
		}
	case 2732: { // movec sp,a0
		unhandled("movec sp,a0");
		break;
		}
	case 2733: { // movec ssh,a0
		unhandled("movec ssh,a0");
		break;
		}
	case 2734: { // movec ssl,a0
		unhandled("movec ssl,a0");
		break;
		}
	case 2735: { // movec la,a0
		unhandled("movec la,a0");
		break;
		}
	case 2736: { // movec lc,a0
		unhandled("movec lc,a0");
		break;
		}
	case 2737: { // movec m,b0
		unhandled("movec m,b0");
		break;
		}
	case 2738: { // movec ep,b0
		unhandled("movec ep,b0");
		break;
		}
	case 2739: { // movec vba,b0
		unhandled("movec vba,b0");
		break;
		}
	case 2740: { // movec sc,b0
		unhandled("movec sc,b0");
		break;
		}
	case 2741: { // movec sz,b0
		unhandled("movec sz,b0");
		break;
		}
	case 2742: { // movec sr,b0
		unhandled("movec sr,b0");
		break;
		}
	case 2743: { // movec omr,b0
		unhandled("movec omr,b0");
		break;
		}
	case 2744: { // movec sp,b0
		unhandled("movec sp,b0");
		break;
		}
	case 2745: { // movec ssh,b0
		unhandled("movec ssh,b0");
		break;
		}
	case 2746: { // movec ssl,b0
		unhandled("movec ssl,b0");
		break;
		}
	case 2747: { // movec la,b0
		unhandled("movec la,b0");
		break;
		}
	case 2748: { // movec lc,b0
		unhandled("movec lc,b0");
		break;
		}
	case 2749: { // movec m,a2
		unhandled("movec m,a2");
		break;
		}
	case 2750: { // movec ep,a2
		unhandled("movec ep,a2");
		break;
		}
	case 2751: { // movec vba,a2
		unhandled("movec vba,a2");
		break;
		}
	case 2752: { // movec sc,a2
		unhandled("movec sc,a2");
		break;
		}
	case 2753: { // movec sz,a2
		unhandled("movec sz,a2");
		break;
		}
	case 2754: { // movec sr,a2
		unhandled("movec sr,a2");
		break;
		}
	case 2755: { // movec omr,a2
		unhandled("movec omr,a2");
		break;
		}
	case 2756: { // movec sp,a2
		unhandled("movec sp,a2");
		break;
		}
	case 2757: { // movec ssh,a2
		unhandled("movec ssh,a2");
		break;
		}
	case 2758: { // movec ssl,a2
		unhandled("movec ssl,a2");
		break;
		}
	case 2759: { // movec la,a2
		unhandled("movec la,a2");
		break;
		}
	case 2760: { // movec lc,a2
		unhandled("movec lc,a2");
		break;
		}
	case 2761: { // movec m,b2
		unhandled("movec m,b2");
		break;
		}
	case 2762: { // movec ep,b2
		unhandled("movec ep,b2");
		break;
		}
	case 2763: { // movec vba,b2
		unhandled("movec vba,b2");
		break;
		}
	case 2764: { // movec sc,b2
		unhandled("movec sc,b2");
		break;
		}
	case 2765: { // movec sz,b2
		unhandled("movec sz,b2");
		break;
		}
	case 2766: { // movec sr,b2
		unhandled("movec sr,b2");
		break;
		}
	case 2767: { // movec omr,b2
		unhandled("movec omr,b2");
		break;
		}
	case 2768: { // movec sp,b2
		unhandled("movec sp,b2");
		break;
		}
	case 2769: { // movec ssh,b2
		unhandled("movec ssh,b2");
		break;
		}
	case 2770: { // movec ssl,b2
		unhandled("movec ssl,b2");
		break;
		}
	case 2771: { // movec la,b2
		unhandled("movec la,b2");
		break;
		}
	case 2772: { // movec lc,b2
		unhandled("movec lc,b2");
		break;
		}
	case 2773: { // movec m,a1
		unhandled("movec m,a1");
		break;
		}
	case 2774: { // movec ep,a1
		unhandled("movec ep,a1");
		break;
		}
	case 2775: { // movec vba,a1
		unhandled("movec vba,a1");
		break;
		}
	case 2776: { // movec sc,a1
		unhandled("movec sc,a1");
		break;
		}
	case 2777: { // movec sz,a1
		unhandled("movec sz,a1");
		break;
		}
	case 2778: { // movec sr,a1
		unhandled("movec sr,a1");
		break;
		}
	case 2779: { // movec omr,a1
		unhandled("movec omr,a1");
		break;
		}
	case 2780: { // movec sp,a1
		unhandled("movec sp,a1");
		break;
		}
	case 2781: { // movec ssh,a1
		unhandled("movec ssh,a1");
		break;
		}
	case 2782: { // movec ssl,a1
		unhandled("movec ssl,a1");
		break;
		}
	case 2783: { // movec la,a1
		unhandled("movec la,a1");
		break;
		}
	case 2784: { // movec lc,a1
		unhandled("movec lc,a1");
		break;
		}
	case 2785: { // movec m,b1
		unhandled("movec m,b1");
		break;
		}
	case 2786: { // movec ep,b1
		unhandled("movec ep,b1");
		break;
		}
	case 2787: { // movec vba,b1
		unhandled("movec vba,b1");
		break;
		}
	case 2788: { // movec sc,b1
		unhandled("movec sc,b1");
		break;
		}
	case 2789: { // movec sz,b1
		unhandled("movec sz,b1");
		break;
		}
	case 2790: { // movec sr,b1
		unhandled("movec sr,b1");
		break;
		}
	case 2791: { // movec omr,b1
		unhandled("movec omr,b1");
		break;
		}
	case 2792: { // movec sp,b1
		unhandled("movec sp,b1");
		break;
		}
	case 2793: { // movec ssh,b1
		unhandled("movec ssh,b1");
		break;
		}
	case 2794: { // movec ssl,b1
		unhandled("movec ssl,b1");
		break;
		}
	case 2795: { // movec la,b1
		unhandled("movec la,b1");
		break;
		}
	case 2796: { // movec lc,b1
		unhandled("movec lc,b1");
		break;
		}
	case 2797: { // movec m,a
		unhandled("movec m,a");
		break;
		}
	case 2798: { // movec ep,a
		unhandled("movec ep,a");
		break;
		}
	case 2799: { // movec vba,a
		unhandled("movec vba,a");
		break;
		}
	case 2800: { // movec sc,a
		unhandled("movec sc,a");
		break;
		}
	case 2801: { // movec sz,a
		unhandled("movec sz,a");
		break;
		}
	case 2802: { // movec sr,a
		unhandled("movec sr,a");
		break;
		}
	case 2803: { // movec omr,a
		unhandled("movec omr,a");
		break;
		}
	case 2804: { // movec sp,a
		unhandled("movec sp,a");
		break;
		}
	case 2805: { // movec ssh,a
		unhandled("movec ssh,a");
		break;
		}
	case 2806: { // movec ssl,a
		unhandled("movec ssl,a");
		break;
		}
	case 2807: { // movec la,a
		unhandled("movec la,a");
		break;
		}
	case 2808: { // movec lc,a
		unhandled("movec lc,a");
		break;
		}
	case 2809: { // movec m,b
		unhandled("movec m,b");
		break;
		}
	case 2810: { // movec ep,b
		unhandled("movec ep,b");
		break;
		}
	case 2811: { // movec vba,b
		unhandled("movec vba,b");
		break;
		}
	case 2812: { // movec sc,b
		unhandled("movec sc,b");
		break;
		}
	case 2813: { // movec sz,b
		unhandled("movec sz,b");
		break;
		}
	case 2814: { // movec sr,b
		unhandled("movec sr,b");
		break;
		}
	case 2815: { // movec omr,b
		unhandled("movec omr,b");
		break;
		}
	case 2816: { // movec sp,b
		unhandled("movec sp,b");
		break;
		}
	case 2817: { // movec ssh,b
		unhandled("movec ssh,b");
		break;
		}
	case 2818: { // movec ssl,b
		unhandled("movec ssl,b");
		break;
		}
	case 2819: { // movec la,b
		unhandled("movec la,b");
		break;
		}
	case 2820: { // movec lc,b
		unhandled("movec lc,b");
		break;
		}
	case 2821: { // movec m,r
		unhandled("movec m,r");
		break;
		}
	case 2822: { // movec ep,r
		unhandled("movec ep,r");
		break;
		}
	case 2823: { // movec vba,r
		unhandled("movec vba,r");
		break;
		}
	case 2824: { // movec sc,r
		unhandled("movec sc,r");
		break;
		}
	case 2825: { // movec sz,r
		unhandled("movec sz,r");
		break;
		}
	case 2826: { // movec sr,r
		unhandled("movec sr,r");
		break;
		}
	case 2827: { // movec omr,r
		unhandled("movec omr,r");
		break;
		}
	case 2828: { // movec sp,r
		unhandled("movec sp,r");
		break;
		}
	case 2829: { // movec ssh,r
		unhandled("movec ssh,r");
		break;
		}
	case 2830: { // movec ssl,r
		unhandled("movec ssl,r");
		break;
		}
	case 2831: { // movec la,r
		unhandled("movec la,r");
		break;
		}
	case 2832: { // movec lc,r
		unhandled("movec lc,r");
		break;
		}
	case 2833: { // movec m,n
		unhandled("movec m,n");
		break;
		}
	case 2834: { // movec ep,n
		unhandled("movec ep,n");
		break;
		}
	case 2835: { // movec vba,n
		unhandled("movec vba,n");
		break;
		}
	case 2836: { // movec sc,n
		unhandled("movec sc,n");
		break;
		}
	case 2837: { // movec sz,n
		unhandled("movec sz,n");
		break;
		}
	case 2838: { // movec sr,n
		unhandled("movec sr,n");
		break;
		}
	case 2839: { // movec omr,n
		unhandled("movec omr,n");
		break;
		}
	case 2840: { // movec sp,n
		unhandled("movec sp,n");
		break;
		}
	case 2841: { // movec ssh,n
		unhandled("movec ssh,n");
		break;
		}
	case 2842: { // movec ssl,n
		unhandled("movec ssl,n");
		break;
		}
	case 2843: { // movec la,n
		unhandled("movec la,n");
		break;
		}
	case 2844: { // movec lc,n
		unhandled("movec lc,n");
		break;
		}
	case 2845: { // movec m,m
		unhandled("movec m,m");
		break;
		}
	case 2846: { // movec ep,m
		unhandled("movec ep,m");
		break;
		}
	case 2847: { // movec vba,m
		unhandled("movec vba,m");
		break;
		}
	case 2848: { // movec sc,m
		unhandled("movec sc,m");
		break;
		}
	case 2849: { // movec sz,m
		unhandled("movec sz,m");
		break;
		}
	case 2850: { // movec sr,m
		unhandled("movec sr,m");
		break;
		}
	case 2851: { // movec omr,m
		unhandled("movec omr,m");
		break;
		}
	case 2852: { // movec sp,m
		unhandled("movec sp,m");
		break;
		}
	case 2853: { // movec ssh,m
		unhandled("movec ssh,m");
		break;
		}
	case 2854: { // movec ssl,m
		unhandled("movec ssl,m");
		break;
		}
	case 2855: { // movec la,m
		unhandled("movec la,m");
		break;
		}
	case 2856: { // movec lc,m
		unhandled("movec lc,m");
		break;
		}
	case 2857: { // movec m,ep
		unhandled("movec m,ep");
		break;
		}
	case 2858: { // movec ep,ep
		unhandled("movec ep,ep");
		break;
		}
	case 2859: { // movec vba,ep
		unhandled("movec vba,ep");
		break;
		}
	case 2860: { // movec sc,ep
		unhandled("movec sc,ep");
		break;
		}
	case 2861: { // movec sz,ep
		unhandled("movec sz,ep");
		break;
		}
	case 2862: { // movec sr,ep
		unhandled("movec sr,ep");
		break;
		}
	case 2863: { // movec omr,ep
		unhandled("movec omr,ep");
		break;
		}
	case 2864: { // movec sp,ep
		unhandled("movec sp,ep");
		break;
		}
	case 2865: { // movec ssh,ep
		unhandled("movec ssh,ep");
		break;
		}
	case 2866: { // movec ssl,ep
		unhandled("movec ssl,ep");
		break;
		}
	case 2867: { // movec la,ep
		unhandled("movec la,ep");
		break;
		}
	case 2868: { // movec lc,ep
		unhandled("movec lc,ep");
		break;
		}
	case 2869: { // movec m,vba
		unhandled("movec m,vba");
		break;
		}
	case 2870: { // movec ep,vba
		unhandled("movec ep,vba");
		break;
		}
	case 2871: { // movec vba,vba
		unhandled("movec vba,vba");
		break;
		}
	case 2872: { // movec sc,vba
		unhandled("movec sc,vba");
		break;
		}
	case 2873: { // movec sz,vba
		unhandled("movec sz,vba");
		break;
		}
	case 2874: { // movec sr,vba
		unhandled("movec sr,vba");
		break;
		}
	case 2875: { // movec omr,vba
		unhandled("movec omr,vba");
		break;
		}
	case 2876: { // movec sp,vba
		unhandled("movec sp,vba");
		break;
		}
	case 2877: { // movec ssh,vba
		unhandled("movec ssh,vba");
		break;
		}
	case 2878: { // movec ssl,vba
		unhandled("movec ssl,vba");
		break;
		}
	case 2879: { // movec la,vba
		unhandled("movec la,vba");
		break;
		}
	case 2880: { // movec lc,vba
		unhandled("movec lc,vba");
		break;
		}
	case 2881: { // movec m,sc
		unhandled("movec m,sc");
		break;
		}
	case 2882: { // movec ep,sc
		unhandled("movec ep,sc");
		break;
		}
	case 2883: { // movec vba,sc
		unhandled("movec vba,sc");
		break;
		}
	case 2884: { // movec sc,sc
		unhandled("movec sc,sc");
		break;
		}
	case 2885: { // movec sz,sc
		unhandled("movec sz,sc");
		break;
		}
	case 2886: { // movec sr,sc
		unhandled("movec sr,sc");
		break;
		}
	case 2887: { // movec omr,sc
		unhandled("movec omr,sc");
		break;
		}
	case 2888: { // movec sp,sc
		unhandled("movec sp,sc");
		break;
		}
	case 2889: { // movec ssh,sc
		unhandled("movec ssh,sc");
		break;
		}
	case 2890: { // movec ssl,sc
		unhandled("movec ssl,sc");
		break;
		}
	case 2891: { // movec la,sc
		unhandled("movec la,sc");
		break;
		}
	case 2892: { // movec lc,sc
		unhandled("movec lc,sc");
		break;
		}
	case 2893: { // movec m,sz
		unhandled("movec m,sz");
		break;
		}
	case 2894: { // movec ep,sz
		unhandled("movec ep,sz");
		break;
		}
	case 2895: { // movec vba,sz
		unhandled("movec vba,sz");
		break;
		}
	case 2896: { // movec sc,sz
		unhandled("movec sc,sz");
		break;
		}
	case 2897: { // movec sz,sz
		unhandled("movec sz,sz");
		break;
		}
	case 2898: { // movec sr,sz
		unhandled("movec sr,sz");
		break;
		}
	case 2899: { // movec omr,sz
		unhandled("movec omr,sz");
		break;
		}
	case 2900: { // movec sp,sz
		unhandled("movec sp,sz");
		break;
		}
	case 2901: { // movec ssh,sz
		unhandled("movec ssh,sz");
		break;
		}
	case 2902: { // movec ssl,sz
		unhandled("movec ssl,sz");
		break;
		}
	case 2903: { // movec la,sz
		unhandled("movec la,sz");
		break;
		}
	case 2904: { // movec lc,sz
		unhandled("movec lc,sz");
		break;
		}
	case 2905: { // movec m,sr
		unhandled("movec m,sr");
		break;
		}
	case 2906: { // movec ep,sr
		unhandled("movec ep,sr");
		break;
		}
	case 2907: { // movec vba,sr
		unhandled("movec vba,sr");
		break;
		}
	case 2908: { // movec sc,sr
		unhandled("movec sc,sr");
		break;
		}
	case 2909: { // movec sz,sr
		unhandled("movec sz,sr");
		break;
		}
	case 2910: { // movec sr,sr
		unhandled("movec sr,sr");
		break;
		}
	case 2911: { // movec omr,sr
		unhandled("movec omr,sr");
		break;
		}
	case 2912: { // movec sp,sr
		unhandled("movec sp,sr");
		break;
		}
	case 2913: { // movec ssh,sr
		unhandled("movec ssh,sr");
		break;
		}
	case 2914: { // movec ssl,sr
		unhandled("movec ssl,sr");
		break;
		}
	case 2915: { // movec la,sr
		unhandled("movec la,sr");
		break;
		}
	case 2916: { // movec lc,sr
		unhandled("movec lc,sr");
		break;
		}
	case 2917: { // movec m,omr
		unhandled("movec m,omr");
		break;
		}
	case 2918: { // movec ep,omr
		unhandled("movec ep,omr");
		break;
		}
	case 2919: { // movec vba,omr
		unhandled("movec vba,omr");
		break;
		}
	case 2920: { // movec sc,omr
		unhandled("movec sc,omr");
		break;
		}
	case 2921: { // movec sz,omr
		unhandled("movec sz,omr");
		break;
		}
	case 2922: { // movec sr,omr
		unhandled("movec sr,omr");
		break;
		}
	case 2923: { // movec omr,omr
		unhandled("movec omr,omr");
		break;
		}
	case 2924: { // movec sp,omr
		unhandled("movec sp,omr");
		break;
		}
	case 2925: { // movec ssh,omr
		unhandled("movec ssh,omr");
		break;
		}
	case 2926: { // movec ssl,omr
		unhandled("movec ssl,omr");
		break;
		}
	case 2927: { // movec la,omr
		unhandled("movec la,omr");
		break;
		}
	case 2928: { // movec lc,omr
		unhandled("movec lc,omr");
		break;
		}
	case 2929: { // movec m,sp
		unhandled("movec m,sp");
		break;
		}
	case 2930: { // movec ep,sp
		unhandled("movec ep,sp");
		break;
		}
	case 2931: { // movec vba,sp
		unhandled("movec vba,sp");
		break;
		}
	case 2932: { // movec sc,sp
		unhandled("movec sc,sp");
		break;
		}
	case 2933: { // movec sz,sp
		unhandled("movec sz,sp");
		break;
		}
	case 2934: { // movec sr,sp
		unhandled("movec sr,sp");
		break;
		}
	case 2935: { // movec omr,sp
		unhandled("movec omr,sp");
		break;
		}
	case 2936: { // movec sp,sp
		unhandled("movec sp,sp");
		break;
		}
	case 2937: { // movec ssh,sp
		unhandled("movec ssh,sp");
		break;
		}
	case 2938: { // movec ssl,sp
		unhandled("movec ssl,sp");
		break;
		}
	case 2939: { // movec la,sp
		unhandled("movec la,sp");
		break;
		}
	case 2940: { // movec lc,sp
		unhandled("movec lc,sp");
		break;
		}
	case 2941: { // movec m,ssh
		unhandled("movec m,ssh");
		break;
		}
	case 2942: { // movec ep,ssh
		unhandled("movec ep,ssh");
		break;
		}
	case 2943: { // movec vba,ssh
		unhandled("movec vba,ssh");
		break;
		}
	case 2944: { // movec sc,ssh
		unhandled("movec sc,ssh");
		break;
		}
	case 2945: { // movec sz,ssh
		unhandled("movec sz,ssh");
		break;
		}
	case 2946: { // movec sr,ssh
		unhandled("movec sr,ssh");
		break;
		}
	case 2947: { // movec omr,ssh
		unhandled("movec omr,ssh");
		break;
		}
	case 2948: { // movec sp,ssh
		unhandled("movec sp,ssh");
		break;
		}
	case 2949: { // movec ssh,ssh
		unhandled("movec ssh,ssh");
		break;
		}
	case 2950: { // movec ssl,ssh
		unhandled("movec ssl,ssh");
		break;
		}
	case 2951: { // movec la,ssh
		unhandled("movec la,ssh");
		break;
		}
	case 2952: { // movec lc,ssh
		unhandled("movec lc,ssh");
		break;
		}
	case 2953: { // movec m,ssl
		unhandled("movec m,ssl");
		break;
		}
	case 2954: { // movec ep,ssl
		unhandled("movec ep,ssl");
		break;
		}
	case 2955: { // movec vba,ssl
		unhandled("movec vba,ssl");
		break;
		}
	case 2956: { // movec sc,ssl
		unhandled("movec sc,ssl");
		break;
		}
	case 2957: { // movec sz,ssl
		unhandled("movec sz,ssl");
		break;
		}
	case 2958: { // movec sr,ssl
		unhandled("movec sr,ssl");
		break;
		}
	case 2959: { // movec omr,ssl
		unhandled("movec omr,ssl");
		break;
		}
	case 2960: { // movec sp,ssl
		unhandled("movec sp,ssl");
		break;
		}
	case 2961: { // movec ssh,ssl
		unhandled("movec ssh,ssl");
		break;
		}
	case 2962: { // movec ssl,ssl
		unhandled("movec ssl,ssl");
		break;
		}
	case 2963: { // movec la,ssl
		unhandled("movec la,ssl");
		break;
		}
	case 2964: { // movec lc,ssl
		unhandled("movec lc,ssl");
		break;
		}
	case 2965: { // movec m,la
		unhandled("movec m,la");
		break;
		}
	case 2966: { // movec ep,la
		unhandled("movec ep,la");
		break;
		}
	case 2967: { // movec vba,la
		unhandled("movec vba,la");
		break;
		}
	case 2968: { // movec sc,la
		unhandled("movec sc,la");
		break;
		}
	case 2969: { // movec sz,la
		unhandled("movec sz,la");
		break;
		}
	case 2970: { // movec sr,la
		unhandled("movec sr,la");
		break;
		}
	case 2971: { // movec omr,la
		unhandled("movec omr,la");
		break;
		}
	case 2972: { // movec sp,la
		unhandled("movec sp,la");
		break;
		}
	case 2973: { // movec ssh,la
		unhandled("movec ssh,la");
		break;
		}
	case 2974: { // movec ssl,la
		unhandled("movec ssl,la");
		break;
		}
	case 2975: { // movec la,la
		unhandled("movec la,la");
		break;
		}
	case 2976: { // movec lc,la
		unhandled("movec lc,la");
		break;
		}
	case 2977: { // movec m,lc
		unhandled("movec m,lc");
		break;
		}
	case 2978: { // movec ep,lc
		unhandled("movec ep,lc");
		break;
		}
	case 2979: { // movec vba,lc
		unhandled("movec vba,lc");
		break;
		}
	case 2980: { // movec sc,lc
		unhandled("movec sc,lc");
		break;
		}
	case 2981: { // movec sz,lc
		unhandled("movec sz,lc");
		break;
		}
	case 2982: { // movec sr,lc
		unhandled("movec sr,lc");
		break;
		}
	case 2983: { // movec omr,lc
		unhandled("movec omr,lc");
		break;
		}
	case 2984: { // movec sp,lc
		unhandled("movec sp,lc");
		break;
		}
	case 2985: { // movec ssh,lc
		unhandled("movec ssh,lc");
		break;
		}
	case 2986: { // movec ssl,lc
		unhandled("movec ssl,lc");
		break;
		}
	case 2987: { // movec la,lc
		unhandled("movec la,lc");
		break;
		}
	case 2988: { // movec lc,lc
		unhandled("movec lc,lc");
		break;
		}
	case 2989: { // movec #[i],m
		unhandled("movec #[i],m");
		break;
		}
	case 2990: { // movec #[i],ep
		unhandled("movec #[i],ep");
		break;
		}
	case 2991: { // movec #[i],vba
		unhandled("movec #[i],vba");
		break;
		}
	case 2992: { // movec #[i],sc
		unhandled("movec #[i],sc");
		break;
		}
	case 2993: { // movec #[i],sz
		unhandled("movec #[i],sz");
		break;
		}
	case 2994: { // movec #[i],sr
		unhandled("movec #[i],sr");
		break;
		}
	case 2995: { // movec #[i],omr
		unhandled("movec #[i],omr");
		break;
		}
	case 2996: { // movec #[i],sp
		unhandled("movec #[i],sp");
		break;
		}
	case 2997: { // movec #[i],ssh
		unhandled("movec #[i],ssh");
		break;
		}
	case 2998: { // movec #[i],ssl
		unhandled("movec #[i],ssl");
		break;
		}
	case 2999: { // movec #[i],la
		unhandled("movec #[i],la");
		break;
		}
	case 3000: { // movec #[i],lc
		unhandled("movec #[i],lc");
		break;
		}
	case 3001: { // movem p:(r)-n,x0
		unhandled("movem p:(r)-n,x0");
		break;
		}
	case 3002: { // movem p:(r)-n,x1
		unhandled("movem p:(r)-n,x1");
		break;
		}
	case 3003: { // movem p:(r)-n,y0
		unhandled("movem p:(r)-n,y0");
		break;
		}
	case 3004: { // movem p:(r)-n,y1
		unhandled("movem p:(r)-n,y1");
		break;
		}
	case 3005: { // movem p:(r)-n,a0
		unhandled("movem p:(r)-n,a0");
		break;
		}
	case 3006: { // movem p:(r)-n,b0
		unhandled("movem p:(r)-n,b0");
		break;
		}
	case 3007: { // movem p:(r)-n,a2
		unhandled("movem p:(r)-n,a2");
		break;
		}
	case 3008: { // movem p:(r)-n,b2
		unhandled("movem p:(r)-n,b2");
		break;
		}
	case 3009: { // movem p:(r)-n,a1
		unhandled("movem p:(r)-n,a1");
		break;
		}
	case 3010: { // movem p:(r)-n,b1
		unhandled("movem p:(r)-n,b1");
		break;
		}
	case 3011: { // movem p:(r)-n,a
		unhandled("movem p:(r)-n,a");
		break;
		}
	case 3012: { // movem p:(r)-n,b
		unhandled("movem p:(r)-n,b");
		break;
		}
	case 3013: { // movem p:(r)-n,r
		unhandled("movem p:(r)-n,r");
		break;
		}
	case 3014: { // movem p:(r)-n,n
		unhandled("movem p:(r)-n,n");
		break;
		}
	case 3015: { // movem p:(r)-n,m
		unhandled("movem p:(r)-n,m");
		break;
		}
	case 3016: { // movem p:(r)-n,ep
		unhandled("movem p:(r)-n,ep");
		break;
		}
	case 3017: { // movem p:(r)-n,vba
		unhandled("movem p:(r)-n,vba");
		break;
		}
	case 3018: { // movem p:(r)-n,sc
		unhandled("movem p:(r)-n,sc");
		break;
		}
	case 3019: { // movem p:(r)-n,sz
		unhandled("movem p:(r)-n,sz");
		break;
		}
	case 3020: { // movem p:(r)-n,sr
		unhandled("movem p:(r)-n,sr");
		break;
		}
	case 3021: { // movem p:(r)-n,omr
		unhandled("movem p:(r)-n,omr");
		break;
		}
	case 3022: { // movem p:(r)-n,sp
		unhandled("movem p:(r)-n,sp");
		break;
		}
	case 3023: { // movem p:(r)-n,ssh
		unhandled("movem p:(r)-n,ssh");
		break;
		}
	case 3024: { // movem p:(r)-n,ssl
		unhandled("movem p:(r)-n,ssl");
		break;
		}
	case 3025: { // movem p:(r)-n,la
		unhandled("movem p:(r)-n,la");
		break;
		}
	case 3026: { // movem p:(r)-n,lc
		unhandled("movem p:(r)-n,lc");
		break;
		}
	case 3027: { // movem p:(r)+n,x0
		unhandled("movem p:(r)+n,x0");
		break;
		}
	case 3028: { // movem p:(r)+n,x1
		unhandled("movem p:(r)+n,x1");
		break;
		}
	case 3029: { // movem p:(r)+n,y0
		unhandled("movem p:(r)+n,y0");
		break;
		}
	case 3030: { // movem p:(r)+n,y1
		unhandled("movem p:(r)+n,y1");
		break;
		}
	case 3031: { // movem p:(r)+n,a0
		unhandled("movem p:(r)+n,a0");
		break;
		}
	case 3032: { // movem p:(r)+n,b0
		unhandled("movem p:(r)+n,b0");
		break;
		}
	case 3033: { // movem p:(r)+n,a2
		unhandled("movem p:(r)+n,a2");
		break;
		}
	case 3034: { // movem p:(r)+n,b2
		unhandled("movem p:(r)+n,b2");
		break;
		}
	case 3035: { // movem p:(r)+n,a1
		unhandled("movem p:(r)+n,a1");
		break;
		}
	case 3036: { // movem p:(r)+n,b1
		unhandled("movem p:(r)+n,b1");
		break;
		}
	case 3037: { // movem p:(r)+n,a
		unhandled("movem p:(r)+n,a");
		break;
		}
	case 3038: { // movem p:(r)+n,b
		unhandled("movem p:(r)+n,b");
		break;
		}
	case 3039: { // movem p:(r)+n,r
		unhandled("movem p:(r)+n,r");
		break;
		}
	case 3040: { // movem p:(r)+n,n
		unhandled("movem p:(r)+n,n");
		break;
		}
	case 3041: { // movem p:(r)+n,m
		unhandled("movem p:(r)+n,m");
		break;
		}
	case 3042: { // movem p:(r)+n,ep
		unhandled("movem p:(r)+n,ep");
		break;
		}
	case 3043: { // movem p:(r)+n,vba
		unhandled("movem p:(r)+n,vba");
		break;
		}
	case 3044: { // movem p:(r)+n,sc
		unhandled("movem p:(r)+n,sc");
		break;
		}
	case 3045: { // movem p:(r)+n,sz
		unhandled("movem p:(r)+n,sz");
		break;
		}
	case 3046: { // movem p:(r)+n,sr
		unhandled("movem p:(r)+n,sr");
		break;
		}
	case 3047: { // movem p:(r)+n,omr
		unhandled("movem p:(r)+n,omr");
		break;
		}
	case 3048: { // movem p:(r)+n,sp
		unhandled("movem p:(r)+n,sp");
		break;
		}
	case 3049: { // movem p:(r)+n,ssh
		unhandled("movem p:(r)+n,ssh");
		break;
		}
	case 3050: { // movem p:(r)+n,ssl
		unhandled("movem p:(r)+n,ssl");
		break;
		}
	case 3051: { // movem p:(r)+n,la
		unhandled("movem p:(r)+n,la");
		break;
		}
	case 3052: { // movem p:(r)+n,lc
		unhandled("movem p:(r)+n,lc");
		break;
		}
	case 3053: { // movem p:(r)-,x0
		unhandled("movem p:(r)-,x0");
		break;
		}
	case 3054: { // movem p:(r)-,x1
		unhandled("movem p:(r)-,x1");
		break;
		}
	case 3055: { // movem p:(r)-,y0
		unhandled("movem p:(r)-,y0");
		break;
		}
	case 3056: { // movem p:(r)-,y1
		unhandled("movem p:(r)-,y1");
		break;
		}
	case 3057: { // movem p:(r)-,a0
		unhandled("movem p:(r)-,a0");
		break;
		}
	case 3058: { // movem p:(r)-,b0
		unhandled("movem p:(r)-,b0");
		break;
		}
	case 3059: { // movem p:(r)-,a2
		unhandled("movem p:(r)-,a2");
		break;
		}
	case 3060: { // movem p:(r)-,b2
		unhandled("movem p:(r)-,b2");
		break;
		}
	case 3061: { // movem p:(r)-,a1
		unhandled("movem p:(r)-,a1");
		break;
		}
	case 3062: { // movem p:(r)-,b1
		unhandled("movem p:(r)-,b1");
		break;
		}
	case 3063: { // movem p:(r)-,a
		unhandled("movem p:(r)-,a");
		break;
		}
	case 3064: { // movem p:(r)-,b
		unhandled("movem p:(r)-,b");
		break;
		}
	case 3065: { // movem p:(r)-,r
		unhandled("movem p:(r)-,r");
		break;
		}
	case 3066: { // movem p:(r)-,n
		unhandled("movem p:(r)-,n");
		break;
		}
	case 3067: { // movem p:(r)-,m
		unhandled("movem p:(r)-,m");
		break;
		}
	case 3068: { // movem p:(r)-,ep
		unhandled("movem p:(r)-,ep");
		break;
		}
	case 3069: { // movem p:(r)-,vba
		unhandled("movem p:(r)-,vba");
		break;
		}
	case 3070: { // movem p:(r)-,sc
		unhandled("movem p:(r)-,sc");
		break;
		}
	case 3071: { // movem p:(r)-,sz
		unhandled("movem p:(r)-,sz");
		break;
		}
	case 3072: { // movem p:(r)-,sr
		unhandled("movem p:(r)-,sr");
		break;
		}
	case 3073: { // movem p:(r)-,omr
		unhandled("movem p:(r)-,omr");
		break;
		}
	case 3074: { // movem p:(r)-,sp
		unhandled("movem p:(r)-,sp");
		break;
		}
	case 3075: { // movem p:(r)-,ssh
		unhandled("movem p:(r)-,ssh");
		break;
		}
	case 3076: { // movem p:(r)-,ssl
		unhandled("movem p:(r)-,ssl");
		break;
		}
	case 3077: { // movem p:(r)-,la
		unhandled("movem p:(r)-,la");
		break;
		}
	case 3078: { // movem p:(r)-,lc
		unhandled("movem p:(r)-,lc");
		break;
		}
	case 3079: { // movem p:(r)+,x0
		unhandled("movem p:(r)+,x0");
		break;
		}
	case 3080: { // movem p:(r)+,x1
		unhandled("movem p:(r)+,x1");
		break;
		}
	case 3081: { // movem p:(r)+,y0
		unhandled("movem p:(r)+,y0");
		break;
		}
	case 3082: { // movem p:(r)+,y1
		unhandled("movem p:(r)+,y1");
		break;
		}
	case 3083: { // movem p:(r)+,a0
		unhandled("movem p:(r)+,a0");
		break;
		}
	case 3084: { // movem p:(r)+,b0
		unhandled("movem p:(r)+,b0");
		break;
		}
	case 3085: { // movem p:(r)+,a2
		unhandled("movem p:(r)+,a2");
		break;
		}
	case 3086: { // movem p:(r)+,b2
		unhandled("movem p:(r)+,b2");
		break;
		}
	case 3087: { // movem p:(r)+,a1
		unhandled("movem p:(r)+,a1");
		break;
		}
	case 3088: { // movem p:(r)+,b1
		unhandled("movem p:(r)+,b1");
		break;
		}
	case 3089: { // movem p:(r)+,a
		unhandled("movem p:(r)+,a");
		break;
		}
	case 3090: { // movem p:(r)+,b
		unhandled("movem p:(r)+,b");
		break;
		}
	case 3091: { // movem p:(r)+,r
		unhandled("movem p:(r)+,r");
		break;
		}
	case 3092: { // movem p:(r)+,n
		unhandled("movem p:(r)+,n");
		break;
		}
	case 3093: { // movem p:(r)+,m
		unhandled("movem p:(r)+,m");
		break;
		}
	case 3094: { // movem p:(r)+,ep
		unhandled("movem p:(r)+,ep");
		break;
		}
	case 3095: { // movem p:(r)+,vba
		unhandled("movem p:(r)+,vba");
		break;
		}
	case 3096: { // movem p:(r)+,sc
		unhandled("movem p:(r)+,sc");
		break;
		}
	case 3097: { // movem p:(r)+,sz
		unhandled("movem p:(r)+,sz");
		break;
		}
	case 3098: { // movem p:(r)+,sr
		unhandled("movem p:(r)+,sr");
		break;
		}
	case 3099: { // movem p:(r)+,omr
		unhandled("movem p:(r)+,omr");
		break;
		}
	case 3100: { // movem p:(r)+,sp
		unhandled("movem p:(r)+,sp");
		break;
		}
	case 3101: { // movem p:(r)+,ssh
		unhandled("movem p:(r)+,ssh");
		break;
		}
	case 3102: { // movem p:(r)+,ssl
		unhandled("movem p:(r)+,ssl");
		break;
		}
	case 3103: { // movem p:(r)+,la
		unhandled("movem p:(r)+,la");
		break;
		}
	case 3104: { // movem p:(r)+,lc
		unhandled("movem p:(r)+,lc");
		break;
		}
	case 3105: { // movem p:(r),x0
		unhandled("movem p:(r),x0");
		break;
		}
	case 3106: { // movem p:(r),x1
		unhandled("movem p:(r),x1");
		break;
		}
	case 3107: { // movem p:(r),y0
		unhandled("movem p:(r),y0");
		break;
		}
	case 3108: { // movem p:(r),y1
		unhandled("movem p:(r),y1");
		break;
		}
	case 3109: { // movem p:(r),a0
		unhandled("movem p:(r),a0");
		break;
		}
	case 3110: { // movem p:(r),b0
		unhandled("movem p:(r),b0");
		break;
		}
	case 3111: { // movem p:(r),a2
		unhandled("movem p:(r),a2");
		break;
		}
	case 3112: { // movem p:(r),b2
		unhandled("movem p:(r),b2");
		break;
		}
	case 3113: { // movem p:(r),a1
		unhandled("movem p:(r),a1");
		break;
		}
	case 3114: { // movem p:(r),b1
		unhandled("movem p:(r),b1");
		break;
		}
	case 3115: { // movem p:(r),a
		unhandled("movem p:(r),a");
		break;
		}
	case 3116: { // movem p:(r),b
		unhandled("movem p:(r),b");
		break;
		}
	case 3117: { // movem p:(r),r
		unhandled("movem p:(r),r");
		break;
		}
	case 3118: { // movem p:(r),n
		unhandled("movem p:(r),n");
		break;
		}
	case 3119: { // movem p:(r),m
		unhandled("movem p:(r),m");
		break;
		}
	case 3120: { // movem p:(r),ep
		unhandled("movem p:(r),ep");
		break;
		}
	case 3121: { // movem p:(r),vba
		unhandled("movem p:(r),vba");
		break;
		}
	case 3122: { // movem p:(r),sc
		unhandled("movem p:(r),sc");
		break;
		}
	case 3123: { // movem p:(r),sz
		unhandled("movem p:(r),sz");
		break;
		}
	case 3124: { // movem p:(r),sr
		unhandled("movem p:(r),sr");
		break;
		}
	case 3125: { // movem p:(r),omr
		unhandled("movem p:(r),omr");
		break;
		}
	case 3126: { // movem p:(r),sp
		unhandled("movem p:(r),sp");
		break;
		}
	case 3127: { // movem p:(r),ssh
		unhandled("movem p:(r),ssh");
		break;
		}
	case 3128: { // movem p:(r),ssl
		unhandled("movem p:(r),ssl");
		break;
		}
	case 3129: { // movem p:(r),la
		unhandled("movem p:(r),la");
		break;
		}
	case 3130: { // movem p:(r),lc
		unhandled("movem p:(r),lc");
		break;
		}
	case 3131: { // movem p:(r+n),x0
		unhandled("movem p:(r+n),x0");
		break;
		}
	case 3132: { // movem p:(r+n),x1
		unhandled("movem p:(r+n),x1");
		break;
		}
	case 3133: { // movem p:(r+n),y0
		unhandled("movem p:(r+n),y0");
		break;
		}
	case 3134: { // movem p:(r+n),y1
		unhandled("movem p:(r+n),y1");
		break;
		}
	case 3135: { // movem p:(r+n),a0
		unhandled("movem p:(r+n),a0");
		break;
		}
	case 3136: { // movem p:(r+n),b0
		unhandled("movem p:(r+n),b0");
		break;
		}
	case 3137: { // movem p:(r+n),a2
		unhandled("movem p:(r+n),a2");
		break;
		}
	case 3138: { // movem p:(r+n),b2
		unhandled("movem p:(r+n),b2");
		break;
		}
	case 3139: { // movem p:(r+n),a1
		unhandled("movem p:(r+n),a1");
		break;
		}
	case 3140: { // movem p:(r+n),b1
		unhandled("movem p:(r+n),b1");
		break;
		}
	case 3141: { // movem p:(r+n),a
		unhandled("movem p:(r+n),a");
		break;
		}
	case 3142: { // movem p:(r+n),b
		unhandled("movem p:(r+n),b");
		break;
		}
	case 3143: { // movem p:(r+n),r
		unhandled("movem p:(r+n),r");
		break;
		}
	case 3144: { // movem p:(r+n),n
		unhandled("movem p:(r+n),n");
		break;
		}
	case 3145: { // movem p:(r+n),m
		unhandled("movem p:(r+n),m");
		break;
		}
	case 3146: { // movem p:(r+n),ep
		unhandled("movem p:(r+n),ep");
		break;
		}
	case 3147: { // movem p:(r+n),vba
		unhandled("movem p:(r+n),vba");
		break;
		}
	case 3148: { // movem p:(r+n),sc
		unhandled("movem p:(r+n),sc");
		break;
		}
	case 3149: { // movem p:(r+n),sz
		unhandled("movem p:(r+n),sz");
		break;
		}
	case 3150: { // movem p:(r+n),sr
		unhandled("movem p:(r+n),sr");
		break;
		}
	case 3151: { // movem p:(r+n),omr
		unhandled("movem p:(r+n),omr");
		break;
		}
	case 3152: { // movem p:(r+n),sp
		unhandled("movem p:(r+n),sp");
		break;
		}
	case 3153: { // movem p:(r+n),ssh
		unhandled("movem p:(r+n),ssh");
		break;
		}
	case 3154: { // movem p:(r+n),ssl
		unhandled("movem p:(r+n),ssl");
		break;
		}
	case 3155: { // movem p:(r+n),la
		unhandled("movem p:(r+n),la");
		break;
		}
	case 3156: { // movem p:(r+n),lc
		unhandled("movem p:(r+n),lc");
		break;
		}
	case 3157: { // movem p:-(r),x0
		unhandled("movem p:-(r),x0");
		break;
		}
	case 3158: { // movem p:-(r),x1
		unhandled("movem p:-(r),x1");
		break;
		}
	case 3159: { // movem p:-(r),y0
		unhandled("movem p:-(r),y0");
		break;
		}
	case 3160: { // movem p:-(r),y1
		unhandled("movem p:-(r),y1");
		break;
		}
	case 3161: { // movem p:-(r),a0
		unhandled("movem p:-(r),a0");
		break;
		}
	case 3162: { // movem p:-(r),b0
		unhandled("movem p:-(r),b0");
		break;
		}
	case 3163: { // movem p:-(r),a2
		unhandled("movem p:-(r),a2");
		break;
		}
	case 3164: { // movem p:-(r),b2
		unhandled("movem p:-(r),b2");
		break;
		}
	case 3165: { // movem p:-(r),a1
		unhandled("movem p:-(r),a1");
		break;
		}
	case 3166: { // movem p:-(r),b1
		unhandled("movem p:-(r),b1");
		break;
		}
	case 3167: { // movem p:-(r),a
		unhandled("movem p:-(r),a");
		break;
		}
	case 3168: { // movem p:-(r),b
		unhandled("movem p:-(r),b");
		break;
		}
	case 3169: { // movem p:-(r),r
		unhandled("movem p:-(r),r");
		break;
		}
	case 3170: { // movem p:-(r),n
		unhandled("movem p:-(r),n");
		break;
		}
	case 3171: { // movem p:-(r),m
		unhandled("movem p:-(r),m");
		break;
		}
	case 3172: { // movem p:-(r),ep
		unhandled("movem p:-(r),ep");
		break;
		}
	case 3173: { // movem p:-(r),vba
		unhandled("movem p:-(r),vba");
		break;
		}
	case 3174: { // movem p:-(r),sc
		unhandled("movem p:-(r),sc");
		break;
		}
	case 3175: { // movem p:-(r),sz
		unhandled("movem p:-(r),sz");
		break;
		}
	case 3176: { // movem p:-(r),sr
		unhandled("movem p:-(r),sr");
		break;
		}
	case 3177: { // movem p:-(r),omr
		unhandled("movem p:-(r),omr");
		break;
		}
	case 3178: { // movem p:-(r),sp
		unhandled("movem p:-(r),sp");
		break;
		}
	case 3179: { // movem p:-(r),ssh
		unhandled("movem p:-(r),ssh");
		break;
		}
	case 3180: { // movem p:-(r),ssl
		unhandled("movem p:-(r),ssl");
		break;
		}
	case 3181: { // movem p:-(r),la
		unhandled("movem p:-(r),la");
		break;
		}
	case 3182: { // movem p:-(r),lc
		unhandled("movem p:-(r),lc");
		break;
		}
	case 3183: { // movem x0,p:(r)-n
		unhandled("movem x0,p:(r)-n");
		break;
		}
	case 3184: { // movem x1,p:(r)-n
		unhandled("movem x1,p:(r)-n");
		break;
		}
	case 3185: { // movem y0,p:(r)-n
		unhandled("movem y0,p:(r)-n");
		break;
		}
	case 3186: { // movem y1,p:(r)-n
		unhandled("movem y1,p:(r)-n");
		break;
		}
	case 3187: { // movem a0,p:(r)-n
		unhandled("movem a0,p:(r)-n");
		break;
		}
	case 3188: { // movem b0,p:(r)-n
		unhandled("movem b0,p:(r)-n");
		break;
		}
	case 3189: { // movem a2,p:(r)-n
		unhandled("movem a2,p:(r)-n");
		break;
		}
	case 3190: { // movem b2,p:(r)-n
		unhandled("movem b2,p:(r)-n");
		break;
		}
	case 3191: { // movem a1,p:(r)-n
		unhandled("movem a1,p:(r)-n");
		break;
		}
	case 3192: { // movem b1,p:(r)-n
		unhandled("movem b1,p:(r)-n");
		break;
		}
	case 3193: { // movem a,p:(r)-n
		unhandled("movem a,p:(r)-n");
		break;
		}
	case 3194: { // movem b,p:(r)-n
		unhandled("movem b,p:(r)-n");
		break;
		}
	case 3195: { // movem r,p:(r)-n
		unhandled("movem r,p:(r)-n");
		break;
		}
	case 3196: { // movem n,p:(r)-n
		unhandled("movem n,p:(r)-n");
		break;
		}
	case 3197: { // movem m,p:(r)-n
		unhandled("movem m,p:(r)-n");
		break;
		}
	case 3198: { // movem ep,p:(r)-n
		unhandled("movem ep,p:(r)-n");
		break;
		}
	case 3199: { // movem vba,p:(r)-n
		unhandled("movem vba,p:(r)-n");
		break;
		}
	case 3200: { // movem sc,p:(r)-n
		unhandled("movem sc,p:(r)-n");
		break;
		}
	case 3201: { // movem sz,p:(r)-n
		unhandled("movem sz,p:(r)-n");
		break;
		}
	case 3202: { // movem sr,p:(r)-n
		unhandled("movem sr,p:(r)-n");
		break;
		}
	case 3203: { // movem omr,p:(r)-n
		unhandled("movem omr,p:(r)-n");
		break;
		}
	case 3204: { // movem sp,p:(r)-n
		unhandled("movem sp,p:(r)-n");
		break;
		}
	case 3205: { // movem ssh,p:(r)-n
		unhandled("movem ssh,p:(r)-n");
		break;
		}
	case 3206: { // movem ssl,p:(r)-n
		unhandled("movem ssl,p:(r)-n");
		break;
		}
	case 3207: { // movem la,p:(r)-n
		unhandled("movem la,p:(r)-n");
		break;
		}
	case 3208: { // movem lc,p:(r)-n
		unhandled("movem lc,p:(r)-n");
		break;
		}
	case 3209: { // movem x0,p:(r)+n
		unhandled("movem x0,p:(r)+n");
		break;
		}
	case 3210: { // movem x1,p:(r)+n
		unhandled("movem x1,p:(r)+n");
		break;
		}
	case 3211: { // movem y0,p:(r)+n
		unhandled("movem y0,p:(r)+n");
		break;
		}
	case 3212: { // movem y1,p:(r)+n
		unhandled("movem y1,p:(r)+n");
		break;
		}
	case 3213: { // movem a0,p:(r)+n
		unhandled("movem a0,p:(r)+n");
		break;
		}
	case 3214: { // movem b0,p:(r)+n
		unhandled("movem b0,p:(r)+n");
		break;
		}
	case 3215: { // movem a2,p:(r)+n
		unhandled("movem a2,p:(r)+n");
		break;
		}
	case 3216: { // movem b2,p:(r)+n
		unhandled("movem b2,p:(r)+n");
		break;
		}
	case 3217: { // movem a1,p:(r)+n
		unhandled("movem a1,p:(r)+n");
		break;
		}
	case 3218: { // movem b1,p:(r)+n
		unhandled("movem b1,p:(r)+n");
		break;
		}
	case 3219: { // movem a,p:(r)+n
		unhandled("movem a,p:(r)+n");
		break;
		}
	case 3220: { // movem b,p:(r)+n
		unhandled("movem b,p:(r)+n");
		break;
		}
	case 3221: { // movem r,p:(r)+n
		unhandled("movem r,p:(r)+n");
		break;
		}
	case 3222: { // movem n,p:(r)+n
		unhandled("movem n,p:(r)+n");
		break;
		}
	case 3223: { // movem m,p:(r)+n
		unhandled("movem m,p:(r)+n");
		break;
		}
	case 3224: { // movem ep,p:(r)+n
		unhandled("movem ep,p:(r)+n");
		break;
		}
	case 3225: { // movem vba,p:(r)+n
		unhandled("movem vba,p:(r)+n");
		break;
		}
	case 3226: { // movem sc,p:(r)+n
		unhandled("movem sc,p:(r)+n");
		break;
		}
	case 3227: { // movem sz,p:(r)+n
		unhandled("movem sz,p:(r)+n");
		break;
		}
	case 3228: { // movem sr,p:(r)+n
		unhandled("movem sr,p:(r)+n");
		break;
		}
	case 3229: { // movem omr,p:(r)+n
		unhandled("movem omr,p:(r)+n");
		break;
		}
	case 3230: { // movem sp,p:(r)+n
		unhandled("movem sp,p:(r)+n");
		break;
		}
	case 3231: { // movem ssh,p:(r)+n
		unhandled("movem ssh,p:(r)+n");
		break;
		}
	case 3232: { // movem ssl,p:(r)+n
		unhandled("movem ssl,p:(r)+n");
		break;
		}
	case 3233: { // movem la,p:(r)+n
		unhandled("movem la,p:(r)+n");
		break;
		}
	case 3234: { // movem lc,p:(r)+n
		unhandled("movem lc,p:(r)+n");
		break;
		}
	case 3235: { // movem x0,p:(r)-
		unhandled("movem x0,p:(r)-");
		break;
		}
	case 3236: { // movem x1,p:(r)-
		unhandled("movem x1,p:(r)-");
		break;
		}
	case 3237: { // movem y0,p:(r)-
		unhandled("movem y0,p:(r)-");
		break;
		}
	case 3238: { // movem y1,p:(r)-
		unhandled("movem y1,p:(r)-");
		break;
		}
	case 3239: { // movem a0,p:(r)-
		unhandled("movem a0,p:(r)-");
		break;
		}
	case 3240: { // movem b0,p:(r)-
		unhandled("movem b0,p:(r)-");
		break;
		}
	case 3241: { // movem a2,p:(r)-
		unhandled("movem a2,p:(r)-");
		break;
		}
	case 3242: { // movem b2,p:(r)-
		unhandled("movem b2,p:(r)-");
		break;
		}
	case 3243: { // movem a1,p:(r)-
		unhandled("movem a1,p:(r)-");
		break;
		}
	case 3244: { // movem b1,p:(r)-
		unhandled("movem b1,p:(r)-");
		break;
		}
	case 3245: { // movem a,p:(r)-
		unhandled("movem a,p:(r)-");
		break;
		}
	case 3246: { // movem b,p:(r)-
		unhandled("movem b,p:(r)-");
		break;
		}
	case 3247: { // movem r,p:(r)-
		unhandled("movem r,p:(r)-");
		break;
		}
	case 3248: { // movem n,p:(r)-
		unhandled("movem n,p:(r)-");
		break;
		}
	case 3249: { // movem m,p:(r)-
		unhandled("movem m,p:(r)-");
		break;
		}
	case 3250: { // movem ep,p:(r)-
		unhandled("movem ep,p:(r)-");
		break;
		}
	case 3251: { // movem vba,p:(r)-
		unhandled("movem vba,p:(r)-");
		break;
		}
	case 3252: { // movem sc,p:(r)-
		unhandled("movem sc,p:(r)-");
		break;
		}
	case 3253: { // movem sz,p:(r)-
		unhandled("movem sz,p:(r)-");
		break;
		}
	case 3254: { // movem sr,p:(r)-
		unhandled("movem sr,p:(r)-");
		break;
		}
	case 3255: { // movem omr,p:(r)-
		unhandled("movem omr,p:(r)-");
		break;
		}
	case 3256: { // movem sp,p:(r)-
		unhandled("movem sp,p:(r)-");
		break;
		}
	case 3257: { // movem ssh,p:(r)-
		unhandled("movem ssh,p:(r)-");
		break;
		}
	case 3258: { // movem ssl,p:(r)-
		unhandled("movem ssl,p:(r)-");
		break;
		}
	case 3259: { // movem la,p:(r)-
		unhandled("movem la,p:(r)-");
		break;
		}
	case 3260: { // movem lc,p:(r)-
		unhandled("movem lc,p:(r)-");
		break;
		}
	case 3261: { // movem x0,p:(r)+
		unhandled("movem x0,p:(r)+");
		break;
		}
	case 3262: { // movem x1,p:(r)+
		unhandled("movem x1,p:(r)+");
		break;
		}
	case 3263: { // movem y0,p:(r)+
		unhandled("movem y0,p:(r)+");
		break;
		}
	case 3264: { // movem y1,p:(r)+
		unhandled("movem y1,p:(r)+");
		break;
		}
	case 3265: { // movem a0,p:(r)+
		unhandled("movem a0,p:(r)+");
		break;
		}
	case 3266: { // movem b0,p:(r)+
		unhandled("movem b0,p:(r)+");
		break;
		}
	case 3267: { // movem a2,p:(r)+
		unhandled("movem a2,p:(r)+");
		break;
		}
	case 3268: { // movem b2,p:(r)+
		unhandled("movem b2,p:(r)+");
		break;
		}
	case 3269: { // movem a1,p:(r)+
		unhandled("movem a1,p:(r)+");
		break;
		}
	case 3270: { // movem b1,p:(r)+
		unhandled("movem b1,p:(r)+");
		break;
		}
	case 3271: { // movem a,p:(r)+
		unhandled("movem a,p:(r)+");
		break;
		}
	case 3272: { // movem b,p:(r)+
		unhandled("movem b,p:(r)+");
		break;
		}
	case 3273: { // movem r,p:(r)+
		unhandled("movem r,p:(r)+");
		break;
		}
	case 3274: { // movem n,p:(r)+
		unhandled("movem n,p:(r)+");
		break;
		}
	case 3275: { // movem m,p:(r)+
		unhandled("movem m,p:(r)+");
		break;
		}
	case 3276: { // movem ep,p:(r)+
		unhandled("movem ep,p:(r)+");
		break;
		}
	case 3277: { // movem vba,p:(r)+
		unhandled("movem vba,p:(r)+");
		break;
		}
	case 3278: { // movem sc,p:(r)+
		unhandled("movem sc,p:(r)+");
		break;
		}
	case 3279: { // movem sz,p:(r)+
		unhandled("movem sz,p:(r)+");
		break;
		}
	case 3280: { // movem sr,p:(r)+
		unhandled("movem sr,p:(r)+");
		break;
		}
	case 3281: { // movem omr,p:(r)+
		unhandled("movem omr,p:(r)+");
		break;
		}
	case 3282: { // movem sp,p:(r)+
		unhandled("movem sp,p:(r)+");
		break;
		}
	case 3283: { // movem ssh,p:(r)+
		unhandled("movem ssh,p:(r)+");
		break;
		}
	case 3284: { // movem ssl,p:(r)+
		unhandled("movem ssl,p:(r)+");
		break;
		}
	case 3285: { // movem la,p:(r)+
		unhandled("movem la,p:(r)+");
		break;
		}
	case 3286: { // movem lc,p:(r)+
		unhandled("movem lc,p:(r)+");
		break;
		}
	case 3287: { // movem x0,p:(r)
		unhandled("movem x0,p:(r)");
		break;
		}
	case 3288: { // movem x1,p:(r)
		unhandled("movem x1,p:(r)");
		break;
		}
	case 3289: { // movem y0,p:(r)
		unhandled("movem y0,p:(r)");
		break;
		}
	case 3290: { // movem y1,p:(r)
		unhandled("movem y1,p:(r)");
		break;
		}
	case 3291: { // movem a0,p:(r)
		unhandled("movem a0,p:(r)");
		break;
		}
	case 3292: { // movem b0,p:(r)
		unhandled("movem b0,p:(r)");
		break;
		}
	case 3293: { // movem a2,p:(r)
		unhandled("movem a2,p:(r)");
		break;
		}
	case 3294: { // movem b2,p:(r)
		unhandled("movem b2,p:(r)");
		break;
		}
	case 3295: { // movem a1,p:(r)
		unhandled("movem a1,p:(r)");
		break;
		}
	case 3296: { // movem b1,p:(r)
		unhandled("movem b1,p:(r)");
		break;
		}
	case 3297: { // movem a,p:(r)
		unhandled("movem a,p:(r)");
		break;
		}
	case 3298: { // movem b,p:(r)
		unhandled("movem b,p:(r)");
		break;
		}
	case 3299: { // movem r,p:(r)
		unhandled("movem r,p:(r)");
		break;
		}
	case 3300: { // movem n,p:(r)
		unhandled("movem n,p:(r)");
		break;
		}
	case 3301: { // movem m,p:(r)
		unhandled("movem m,p:(r)");
		break;
		}
	case 3302: { // movem ep,p:(r)
		unhandled("movem ep,p:(r)");
		break;
		}
	case 3303: { // movem vba,p:(r)
		unhandled("movem vba,p:(r)");
		break;
		}
	case 3304: { // movem sc,p:(r)
		unhandled("movem sc,p:(r)");
		break;
		}
	case 3305: { // movem sz,p:(r)
		unhandled("movem sz,p:(r)");
		break;
		}
	case 3306: { // movem sr,p:(r)
		unhandled("movem sr,p:(r)");
		break;
		}
	case 3307: { // movem omr,p:(r)
		unhandled("movem omr,p:(r)");
		break;
		}
	case 3308: { // movem sp,p:(r)
		unhandled("movem sp,p:(r)");
		break;
		}
	case 3309: { // movem ssh,p:(r)
		unhandled("movem ssh,p:(r)");
		break;
		}
	case 3310: { // movem ssl,p:(r)
		unhandled("movem ssl,p:(r)");
		break;
		}
	case 3311: { // movem la,p:(r)
		unhandled("movem la,p:(r)");
		break;
		}
	case 3312: { // movem lc,p:(r)
		unhandled("movem lc,p:(r)");
		break;
		}
	case 3313: { // movem x0,p:(r+n)
		unhandled("movem x0,p:(r+n)");
		break;
		}
	case 3314: { // movem x1,p:(r+n)
		unhandled("movem x1,p:(r+n)");
		break;
		}
	case 3315: { // movem y0,p:(r+n)
		unhandled("movem y0,p:(r+n)");
		break;
		}
	case 3316: { // movem y1,p:(r+n)
		unhandled("movem y1,p:(r+n)");
		break;
		}
	case 3317: { // movem a0,p:(r+n)
		unhandled("movem a0,p:(r+n)");
		break;
		}
	case 3318: { // movem b0,p:(r+n)
		unhandled("movem b0,p:(r+n)");
		break;
		}
	case 3319: { // movem a2,p:(r+n)
		unhandled("movem a2,p:(r+n)");
		break;
		}
	case 3320: { // movem b2,p:(r+n)
		unhandled("movem b2,p:(r+n)");
		break;
		}
	case 3321: { // movem a1,p:(r+n)
		unhandled("movem a1,p:(r+n)");
		break;
		}
	case 3322: { // movem b1,p:(r+n)
		unhandled("movem b1,p:(r+n)");
		break;
		}
	case 3323: { // movem a,p:(r+n)
		unhandled("movem a,p:(r+n)");
		break;
		}
	case 3324: { // movem b,p:(r+n)
		unhandled("movem b,p:(r+n)");
		break;
		}
	case 3325: { // movem r,p:(r+n)
		unhandled("movem r,p:(r+n)");
		break;
		}
	case 3326: { // movem n,p:(r+n)
		unhandled("movem n,p:(r+n)");
		break;
		}
	case 3327: { // movem m,p:(r+n)
		unhandled("movem m,p:(r+n)");
		break;
		}
	case 3328: { // movem ep,p:(r+n)
		unhandled("movem ep,p:(r+n)");
		break;
		}
	case 3329: { // movem vba,p:(r+n)
		unhandled("movem vba,p:(r+n)");
		break;
		}
	case 3330: { // movem sc,p:(r+n)
		unhandled("movem sc,p:(r+n)");
		break;
		}
	case 3331: { // movem sz,p:(r+n)
		unhandled("movem sz,p:(r+n)");
		break;
		}
	case 3332: { // movem sr,p:(r+n)
		unhandled("movem sr,p:(r+n)");
		break;
		}
	case 3333: { // movem omr,p:(r+n)
		unhandled("movem omr,p:(r+n)");
		break;
		}
	case 3334: { // movem sp,p:(r+n)
		unhandled("movem sp,p:(r+n)");
		break;
		}
	case 3335: { // movem ssh,p:(r+n)
		unhandled("movem ssh,p:(r+n)");
		break;
		}
	case 3336: { // movem ssl,p:(r+n)
		unhandled("movem ssl,p:(r+n)");
		break;
		}
	case 3337: { // movem la,p:(r+n)
		unhandled("movem la,p:(r+n)");
		break;
		}
	case 3338: { // movem lc,p:(r+n)
		unhandled("movem lc,p:(r+n)");
		break;
		}
	case 3339: { // movem x0,p:-(r)
		unhandled("movem x0,p:-(r)");
		break;
		}
	case 3340: { // movem x1,p:-(r)
		unhandled("movem x1,p:-(r)");
		break;
		}
	case 3341: { // movem y0,p:-(r)
		unhandled("movem y0,p:-(r)");
		break;
		}
	case 3342: { // movem y1,p:-(r)
		unhandled("movem y1,p:-(r)");
		break;
		}
	case 3343: { // movem a0,p:-(r)
		unhandled("movem a0,p:-(r)");
		break;
		}
	case 3344: { // movem b0,p:-(r)
		unhandled("movem b0,p:-(r)");
		break;
		}
	case 3345: { // movem a2,p:-(r)
		unhandled("movem a2,p:-(r)");
		break;
		}
	case 3346: { // movem b2,p:-(r)
		unhandled("movem b2,p:-(r)");
		break;
		}
	case 3347: { // movem a1,p:-(r)
		unhandled("movem a1,p:-(r)");
		break;
		}
	case 3348: { // movem b1,p:-(r)
		unhandled("movem b1,p:-(r)");
		break;
		}
	case 3349: { // movem a,p:-(r)
		unhandled("movem a,p:-(r)");
		break;
		}
	case 3350: { // movem b,p:-(r)
		unhandled("movem b,p:-(r)");
		break;
		}
	case 3351: { // movem r,p:-(r)
		unhandled("movem r,p:-(r)");
		break;
		}
	case 3352: { // movem n,p:-(r)
		unhandled("movem n,p:-(r)");
		break;
		}
	case 3353: { // movem m,p:-(r)
		unhandled("movem m,p:-(r)");
		break;
		}
	case 3354: { // movem ep,p:-(r)
		unhandled("movem ep,p:-(r)");
		break;
		}
	case 3355: { // movem vba,p:-(r)
		unhandled("movem vba,p:-(r)");
		break;
		}
	case 3356: { // movem sc,p:-(r)
		unhandled("movem sc,p:-(r)");
		break;
		}
	case 3357: { // movem sz,p:-(r)
		unhandled("movem sz,p:-(r)");
		break;
		}
	case 3358: { // movem sr,p:-(r)
		unhandled("movem sr,p:-(r)");
		break;
		}
	case 3359: { // movem omr,p:-(r)
		unhandled("movem omr,p:-(r)");
		break;
		}
	case 3360: { // movem sp,p:-(r)
		unhandled("movem sp,p:-(r)");
		break;
		}
	case 3361: { // movem ssh,p:-(r)
		unhandled("movem ssh,p:-(r)");
		break;
		}
	case 3362: { // movem ssl,p:-(r)
		unhandled("movem ssl,p:-(r)");
		break;
		}
	case 3363: { // movem la,p:-(r)
		unhandled("movem la,p:-(r)");
		break;
		}
	case 3364: { // movem lc,p:-(r)
		unhandled("movem lc,p:-(r)");
		break;
		}
	case 3365: { // movem p:[abs],x0
		unhandled("movem p:[abs],x0");
		break;
		}
	case 3366: { // movem p:[abs],x1
		unhandled("movem p:[abs],x1");
		break;
		}
	case 3367: { // movem p:[abs],y0
		unhandled("movem p:[abs],y0");
		break;
		}
	case 3368: { // movem p:[abs],y1
		unhandled("movem p:[abs],y1");
		break;
		}
	case 3369: { // movem p:[abs],a0
		unhandled("movem p:[abs],a0");
		break;
		}
	case 3370: { // movem p:[abs],b0
		unhandled("movem p:[abs],b0");
		break;
		}
	case 3371: { // movem p:[abs],a2
		unhandled("movem p:[abs],a2");
		break;
		}
	case 3372: { // movem p:[abs],b2
		unhandled("movem p:[abs],b2");
		break;
		}
	case 3373: { // movem p:[abs],a1
		unhandled("movem p:[abs],a1");
		break;
		}
	case 3374: { // movem p:[abs],b1
		unhandled("movem p:[abs],b1");
		break;
		}
	case 3375: { // movem p:[abs],a
		unhandled("movem p:[abs],a");
		break;
		}
	case 3376: { // movem p:[abs],b
		unhandled("movem p:[abs],b");
		break;
		}
	case 3377: { // movem p:[abs],r
		unhandled("movem p:[abs],r");
		break;
		}
	case 3378: { // movem p:[abs],n
		unhandled("movem p:[abs],n");
		break;
		}
	case 3379: { // movem p:[abs],m
		unhandled("movem p:[abs],m");
		break;
		}
	case 3380: { // movem p:[abs],ep
		unhandled("movem p:[abs],ep");
		break;
		}
	case 3381: { // movem p:[abs],vba
		unhandled("movem p:[abs],vba");
		break;
		}
	case 3382: { // movem p:[abs],sc
		unhandled("movem p:[abs],sc");
		break;
		}
	case 3383: { // movem p:[abs],sz
		unhandled("movem p:[abs],sz");
		break;
		}
	case 3384: { // movem p:[abs],sr
		unhandled("movem p:[abs],sr");
		break;
		}
	case 3385: { // movem p:[abs],omr
		unhandled("movem p:[abs],omr");
		break;
		}
	case 3386: { // movem p:[abs],sp
		unhandled("movem p:[abs],sp");
		break;
		}
	case 3387: { // movem p:[abs],ssh
		unhandled("movem p:[abs],ssh");
		break;
		}
	case 3388: { // movem p:[abs],ssl
		unhandled("movem p:[abs],ssl");
		break;
		}
	case 3389: { // movem p:[abs],la
		unhandled("movem p:[abs],la");
		break;
		}
	case 3390: { // movem p:[abs],lc
		unhandled("movem p:[abs],lc");
		break;
		}
	case 3391: { // movem p:[aa],x0
		unhandled("movem p:[aa],x0");
		break;
		}
	case 3392: { // movem p:[aa],x1
		unhandled("movem p:[aa],x1");
		break;
		}
	case 3393: { // movem p:[aa],y0
		unhandled("movem p:[aa],y0");
		break;
		}
	case 3394: { // movem p:[aa],y1
		unhandled("movem p:[aa],y1");
		break;
		}
	case 3395: { // movem p:[aa],a0
		unhandled("movem p:[aa],a0");
		break;
		}
	case 3396: { // movem p:[aa],b0
		unhandled("movem p:[aa],b0");
		break;
		}
	case 3397: { // movem p:[aa],a2
		unhandled("movem p:[aa],a2");
		break;
		}
	case 3398: { // movem p:[aa],b2
		unhandled("movem p:[aa],b2");
		break;
		}
	case 3399: { // movem p:[aa],a1
		unhandled("movem p:[aa],a1");
		break;
		}
	case 3400: { // movem p:[aa],b1
		unhandled("movem p:[aa],b1");
		break;
		}
	case 3401: { // movem p:[aa],a
		unhandled("movem p:[aa],a");
		break;
		}
	case 3402: { // movem p:[aa],b
		unhandled("movem p:[aa],b");
		break;
		}
	case 3403: { // movem p:[aa],r
		unhandled("movem p:[aa],r");
		break;
		}
	case 3404: { // movem p:[aa],n
		unhandled("movem p:[aa],n");
		break;
		}
	case 3405: { // movem p:[aa],m
		unhandled("movem p:[aa],m");
		break;
		}
	case 3406: { // movem p:[aa],ep
		unhandled("movem p:[aa],ep");
		break;
		}
	case 3407: { // movem p:[aa],vba
		unhandled("movem p:[aa],vba");
		break;
		}
	case 3408: { // movem p:[aa],sc
		unhandled("movem p:[aa],sc");
		break;
		}
	case 3409: { // movem p:[aa],sz
		unhandled("movem p:[aa],sz");
		break;
		}
	case 3410: { // movem p:[aa],sr
		unhandled("movem p:[aa],sr");
		break;
		}
	case 3411: { // movem p:[aa],omr
		unhandled("movem p:[aa],omr");
		break;
		}
	case 3412: { // movem p:[aa],sp
		unhandled("movem p:[aa],sp");
		break;
		}
	case 3413: { // movem p:[aa],ssh
		unhandled("movem p:[aa],ssh");
		break;
		}
	case 3414: { // movem p:[aa],ssl
		unhandled("movem p:[aa],ssl");
		break;
		}
	case 3415: { // movem p:[aa],la
		unhandled("movem p:[aa],la");
		break;
		}
	case 3416: { // movem p:[aa],lc
		unhandled("movem p:[aa],lc");
		break;
		}
	case 3417: { // movem x0,p:[aa]
		unhandled("movem x0,p:[aa]");
		break;
		}
	case 3418: { // movem x1,p:[aa]
		unhandled("movem x1,p:[aa]");
		break;
		}
	case 3419: { // movem y0,p:[aa]
		unhandled("movem y0,p:[aa]");
		break;
		}
	case 3420: { // movem y1,p:[aa]
		unhandled("movem y1,p:[aa]");
		break;
		}
	case 3421: { // movem a0,p:[aa]
		unhandled("movem a0,p:[aa]");
		break;
		}
	case 3422: { // movem b0,p:[aa]
		unhandled("movem b0,p:[aa]");
		break;
		}
	case 3423: { // movem a2,p:[aa]
		unhandled("movem a2,p:[aa]");
		break;
		}
	case 3424: { // movem b2,p:[aa]
		unhandled("movem b2,p:[aa]");
		break;
		}
	case 3425: { // movem a1,p:[aa]
		unhandled("movem a1,p:[aa]");
		break;
		}
	case 3426: { // movem b1,p:[aa]
		unhandled("movem b1,p:[aa]");
		break;
		}
	case 3427: { // movem a,p:[aa]
		unhandled("movem a,p:[aa]");
		break;
		}
	case 3428: { // movem b,p:[aa]
		unhandled("movem b,p:[aa]");
		break;
		}
	case 3429: { // movem r,p:[aa]
		unhandled("movem r,p:[aa]");
		break;
		}
	case 3430: { // movem n,p:[aa]
		unhandled("movem n,p:[aa]");
		break;
		}
	case 3431: { // movem m,p:[aa]
		unhandled("movem m,p:[aa]");
		break;
		}
	case 3432: { // movem ep,p:[aa]
		unhandled("movem ep,p:[aa]");
		break;
		}
	case 3433: { // movem vba,p:[aa]
		unhandled("movem vba,p:[aa]");
		break;
		}
	case 3434: { // movem sc,p:[aa]
		unhandled("movem sc,p:[aa]");
		break;
		}
	case 3435: { // movem sz,p:[aa]
		unhandled("movem sz,p:[aa]");
		break;
		}
	case 3436: { // movem sr,p:[aa]
		unhandled("movem sr,p:[aa]");
		break;
		}
	case 3437: { // movem omr,p:[aa]
		unhandled("movem omr,p:[aa]");
		break;
		}
	case 3438: { // movem sp,p:[aa]
		unhandled("movem sp,p:[aa]");
		break;
		}
	case 3439: { // movem ssh,p:[aa]
		unhandled("movem ssh,p:[aa]");
		break;
		}
	case 3440: { // movem ssl,p:[aa]
		unhandled("movem ssl,p:[aa]");
		break;
		}
	case 3441: { // movem la,p:[aa]
		unhandled("movem la,p:[aa]");
		break;
		}
	case 3442: { // movem lc,p:[aa]
		unhandled("movem lc,p:[aa]");
		break;
		}
	case 3443: { // movep x:(r)-n,x:[pp]
		unhandled("movep x:(r)-n,x:[pp]");
		break;
		}
	case 3444: { // movep y:(r)-n,x:[pp]
		unhandled("movep y:(r)-n,x:[pp]");
		break;
		}
	case 3445: { // movep x:(r)+n,x:[pp]
		unhandled("movep x:(r)+n,x:[pp]");
		break;
		}
	case 3446: { // movep y:(r)+n,x:[pp]
		unhandled("movep y:(r)+n,x:[pp]");
		break;
		}
	case 3447: { // movep x:(r)-,x:[pp]
		unhandled("movep x:(r)-,x:[pp]");
		break;
		}
	case 3448: { // movep y:(r)-,x:[pp]
		unhandled("movep y:(r)-,x:[pp]");
		break;
		}
	case 3449: { // movep x:(r)+,x:[pp]
		unhandled("movep x:(r)+,x:[pp]");
		break;
		}
	case 3450: { // movep y:(r)+,x:[pp]
		unhandled("movep y:(r)+,x:[pp]");
		break;
		}
	case 3451: { // movep x:(r),x:[pp]
		unhandled("movep x:(r),x:[pp]");
		break;
		}
	case 3452: { // movep y:(r),x:[pp]
		unhandled("movep y:(r),x:[pp]");
		break;
		}
	case 3453: { // movep x:(r+n),x:[pp]
		unhandled("movep x:(r+n),x:[pp]");
		break;
		}
	case 3454: { // movep y:(r+n),x:[pp]
		unhandled("movep y:(r+n),x:[pp]");
		break;
		}
	case 3455: { // movep x:-(r),x:[pp]
		unhandled("movep x:-(r),x:[pp]");
		break;
		}
	case 3456: { // movep y:-(r),x:[pp]
		unhandled("movep y:-(r),x:[pp]");
		break;
		}
	case 3457: { // movep x:(r)-n,y:[pp]
		unhandled("movep x:(r)-n,y:[pp]");
		break;
		}
	case 3458: { // movep y:(r)-n,y:[pp]
		unhandled("movep y:(r)-n,y:[pp]");
		break;
		}
	case 3459: { // movep x:(r)+n,y:[pp]
		unhandled("movep x:(r)+n,y:[pp]");
		break;
		}
	case 3460: { // movep y:(r)+n,y:[pp]
		unhandled("movep y:(r)+n,y:[pp]");
		break;
		}
	case 3461: { // movep x:(r)-,y:[pp]
		unhandled("movep x:(r)-,y:[pp]");
		break;
		}
	case 3462: { // movep y:(r)-,y:[pp]
		unhandled("movep y:(r)-,y:[pp]");
		break;
		}
	case 3463: { // movep x:(r)+,y:[pp]
		unhandled("movep x:(r)+,y:[pp]");
		break;
		}
	case 3464: { // movep y:(r)+,y:[pp]
		unhandled("movep y:(r)+,y:[pp]");
		break;
		}
	case 3465: { // movep x:(r),y:[pp]
		unhandled("movep x:(r),y:[pp]");
		break;
		}
	case 3466: { // movep y:(r),y:[pp]
		unhandled("movep y:(r),y:[pp]");
		break;
		}
	case 3467: { // movep x:(r+n),y:[pp]
		unhandled("movep x:(r+n),y:[pp]");
		break;
		}
	case 3468: { // movep y:(r+n),y:[pp]
		unhandled("movep y:(r+n),y:[pp]");
		break;
		}
	case 3469: { // movep x:-(r),y:[pp]
		unhandled("movep x:-(r),y:[pp]");
		break;
		}
	case 3470: { // movep y:-(r),y:[pp]
		unhandled("movep y:-(r),y:[pp]");
		break;
		}
	case 3471: { // movep x:[pp],x:(r)-n
		unhandled("movep x:[pp],x:(r)-n");
		break;
		}
	case 3472: { // movep x:[pp],y:(r)-n
		unhandled("movep x:[pp],y:(r)-n");
		break;
		}
	case 3473: { // movep x:[pp],x:(r)+n
		unhandled("movep x:[pp],x:(r)+n");
		break;
		}
	case 3474: { // movep x:[pp],y:(r)+n
		unhandled("movep x:[pp],y:(r)+n");
		break;
		}
	case 3475: { // movep x:[pp],x:(r)-
		unhandled("movep x:[pp],x:(r)-");
		break;
		}
	case 3476: { // movep x:[pp],y:(r)-
		unhandled("movep x:[pp],y:(r)-");
		break;
		}
	case 3477: { // movep x:[pp],x:(r)+
		unhandled("movep x:[pp],x:(r)+");
		break;
		}
	case 3478: { // movep x:[pp],y:(r)+
		unhandled("movep x:[pp],y:(r)+");
		break;
		}
	case 3479: { // movep x:[pp],x:(r)
		unhandled("movep x:[pp],x:(r)");
		break;
		}
	case 3480: { // movep x:[pp],y:(r)
		unhandled("movep x:[pp],y:(r)");
		break;
		}
	case 3481: { // movep x:[pp],x:(r+n)
		unhandled("movep x:[pp],x:(r+n)");
		break;
		}
	case 3482: { // movep x:[pp],y:(r+n)
		unhandled("movep x:[pp],y:(r+n)");
		break;
		}
	case 3483: { // movep x:[pp],x:-(r)
		unhandled("movep x:[pp],x:-(r)");
		break;
		}
	case 3484: { // movep x:[pp],y:-(r)
		unhandled("movep x:[pp],y:-(r)");
		break;
		}
	case 3485: { // movep y:[pp],x:(r)-n
		unhandled("movep y:[pp],x:(r)-n");
		break;
		}
	case 3486: { // movep y:[pp],y:(r)-n
		unhandled("movep y:[pp],y:(r)-n");
		break;
		}
	case 3487: { // movep y:[pp],x:(r)+n
		unhandled("movep y:[pp],x:(r)+n");
		break;
		}
	case 3488: { // movep y:[pp],y:(r)+n
		unhandled("movep y:[pp],y:(r)+n");
		break;
		}
	case 3489: { // movep y:[pp],x:(r)-
		unhandled("movep y:[pp],x:(r)-");
		break;
		}
	case 3490: { // movep y:[pp],y:(r)-
		unhandled("movep y:[pp],y:(r)-");
		break;
		}
	case 3491: { // movep y:[pp],x:(r)+
		unhandled("movep y:[pp],x:(r)+");
		break;
		}
	case 3492: { // movep y:[pp],y:(r)+
		unhandled("movep y:[pp],y:(r)+");
		break;
		}
	case 3493: { // movep y:[pp],x:(r)
		unhandled("movep y:[pp],x:(r)");
		break;
		}
	case 3494: { // movep y:[pp],y:(r)
		unhandled("movep y:[pp],y:(r)");
		break;
		}
	case 3495: { // movep y:[pp],x:(r+n)
		unhandled("movep y:[pp],x:(r+n)");
		break;
		}
	case 3496: { // movep y:[pp],y:(r+n)
		unhandled("movep y:[pp],y:(r+n)");
		break;
		}
	case 3497: { // movep y:[pp],x:-(r)
		unhandled("movep y:[pp],x:-(r)");
		break;
		}
	case 3498: { // movep y:[pp],y:-(r)
		unhandled("movep y:[pp],y:-(r)");
		break;
		}
	case 3499: { // movep x:[abs],x:[pp]
		unhandled("movep x:[abs],x:[pp]");
		break;
		}
	case 3500: { // movep y:[abs],x:[pp]
		unhandled("movep y:[abs],x:[pp]");
		break;
		}
	case 3501: { // movep x:[abs],y:[pp]
		unhandled("movep x:[abs],y:[pp]");
		break;
		}
	case 3502: { // movep y:[abs],y:[pp]
		unhandled("movep y:[abs],y:[pp]");
		break;
		}
	case 3503: { // movep #[i],x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 i = exv;
		m_x.write_dword(pp, i);
		break;
		}
	case 3504: { // movep #[i],y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 i = exv;
		m_y.write_dword(pp, i);
		break;
		}
	case 3505: { // movep x:(r)-n,x:[qq]
		unhandled("movep x:(r)-n,x:[qq]");
		break;
		}
	case 3506: { // movep y:(r)-n,x:[qq]
		unhandled("movep y:(r)-n,x:[qq]");
		break;
		}
	case 3507: { // movep x:(r)+n,x:[qq]
		unhandled("movep x:(r)+n,x:[qq]");
		break;
		}
	case 3508: { // movep y:(r)+n,x:[qq]
		unhandled("movep y:(r)+n,x:[qq]");
		break;
		}
	case 3509: { // movep x:(r)-,x:[qq]
		unhandled("movep x:(r)-,x:[qq]");
		break;
		}
	case 3510: { // movep y:(r)-,x:[qq]
		unhandled("movep y:(r)-,x:[qq]");
		break;
		}
	case 3511: { // movep x:(r)+,x:[qq]
		unhandled("movep x:(r)+,x:[qq]");
		break;
		}
	case 3512: { // movep y:(r)+,x:[qq]
		unhandled("movep y:(r)+,x:[qq]");
		break;
		}
	case 3513: { // movep x:(r),x:[qq]
		unhandled("movep x:(r),x:[qq]");
		break;
		}
	case 3514: { // movep y:(r),x:[qq]
		unhandled("movep y:(r),x:[qq]");
		break;
		}
	case 3515: { // movep x:(r+n),x:[qq]
		unhandled("movep x:(r+n),x:[qq]");
		break;
		}
	case 3516: { // movep y:(r+n),x:[qq]
		unhandled("movep y:(r+n),x:[qq]");
		break;
		}
	case 3517: { // movep x:-(r),x:[qq]
		unhandled("movep x:-(r),x:[qq]");
		break;
		}
	case 3518: { // movep y:-(r),x:[qq]
		unhandled("movep y:-(r),x:[qq]");
		break;
		}
	case 3519: { // movep x:[qq],x:(r)-n
		unhandled("movep x:[qq],x:(r)-n");
		break;
		}
	case 3520: { // movep x:[qq],y:(r)-n
		unhandled("movep x:[qq],y:(r)-n");
		break;
		}
	case 3521: { // movep x:[qq],x:(r)+n
		unhandled("movep x:[qq],x:(r)+n");
		break;
		}
	case 3522: { // movep x:[qq],y:(r)+n
		unhandled("movep x:[qq],y:(r)+n");
		break;
		}
	case 3523: { // movep x:[qq],x:(r)-
		unhandled("movep x:[qq],x:(r)-");
		break;
		}
	case 3524: { // movep x:[qq],y:(r)-
		unhandled("movep x:[qq],y:(r)-");
		break;
		}
	case 3525: { // movep x:[qq],x:(r)+
		unhandled("movep x:[qq],x:(r)+");
		break;
		}
	case 3526: { // movep x:[qq],y:(r)+
		unhandled("movep x:[qq],y:(r)+");
		break;
		}
	case 3527: { // movep x:[qq],x:(r)
		unhandled("movep x:[qq],x:(r)");
		break;
		}
	case 3528: { // movep x:[qq],y:(r)
		unhandled("movep x:[qq],y:(r)");
		break;
		}
	case 3529: { // movep x:[qq],x:(r+n)
		unhandled("movep x:[qq],x:(r+n)");
		break;
		}
	case 3530: { // movep x:[qq],y:(r+n)
		unhandled("movep x:[qq],y:(r+n)");
		break;
		}
	case 3531: { // movep x:[qq],x:-(r)
		unhandled("movep x:[qq],x:-(r)");
		break;
		}
	case 3532: { // movep x:[qq],y:-(r)
		unhandled("movep x:[qq],y:-(r)");
		break;
		}
	case 3533: { // movep x:[abs],x:[qq]
		unhandled("movep x:[abs],x:[qq]");
		break;
		}
	case 3534: { // movep y:[abs],x:[qq]
		unhandled("movep y:[abs],x:[qq]");
		break;
		}
	case 3535: { // movep #[i],x:[qq]
		u32 qq = 0xffff80 + BIT(opcode, 0, 6);
		u32 i = exv;
		m_x.write_dword(qq, i);
		break;
		}
	case 3536: { // movep x:(r)-n,y:[qq]
		unhandled("movep x:(r)-n,y:[qq]");
		break;
		}
	case 3537: { // movep y:(r)-n,y:[qq]
		unhandled("movep y:(r)-n,y:[qq]");
		break;
		}
	case 3538: { // movep x:(r)+n,y:[qq]
		unhandled("movep x:(r)+n,y:[qq]");
		break;
		}
	case 3539: { // movep y:(r)+n,y:[qq]
		unhandled("movep y:(r)+n,y:[qq]");
		break;
		}
	case 3540: { // movep x:(r)-,y:[qq]
		unhandled("movep x:(r)-,y:[qq]");
		break;
		}
	case 3541: { // movep y:(r)-,y:[qq]
		unhandled("movep y:(r)-,y:[qq]");
		break;
		}
	case 3542: { // movep x:(r)+,y:[qq]
		unhandled("movep x:(r)+,y:[qq]");
		break;
		}
	case 3543: { // movep y:(r)+,y:[qq]
		unhandled("movep y:(r)+,y:[qq]");
		break;
		}
	case 3544: { // movep x:(r),y:[qq]
		unhandled("movep x:(r),y:[qq]");
		break;
		}
	case 3545: { // movep y:(r),y:[qq]
		unhandled("movep y:(r),y:[qq]");
		break;
		}
	case 3546: { // movep x:(r+n),y:[qq]
		unhandled("movep x:(r+n),y:[qq]");
		break;
		}
	case 3547: { // movep y:(r+n),y:[qq]
		unhandled("movep y:(r+n),y:[qq]");
		break;
		}
	case 3548: { // movep x:-(r),y:[qq]
		unhandled("movep x:-(r),y:[qq]");
		break;
		}
	case 3549: { // movep y:-(r),y:[qq]
		unhandled("movep y:-(r),y:[qq]");
		break;
		}
	case 3550: { // movep y:[qq],x:(r)-n
		unhandled("movep y:[qq],x:(r)-n");
		break;
		}
	case 3551: { // movep y:[qq],y:(r)-n
		unhandled("movep y:[qq],y:(r)-n");
		break;
		}
	case 3552: { // movep y:[qq],x:(r)+n
		unhandled("movep y:[qq],x:(r)+n");
		break;
		}
	case 3553: { // movep y:[qq],y:(r)+n
		unhandled("movep y:[qq],y:(r)+n");
		break;
		}
	case 3554: { // movep y:[qq],x:(r)-
		unhandled("movep y:[qq],x:(r)-");
		break;
		}
	case 3555: { // movep y:[qq],y:(r)-
		unhandled("movep y:[qq],y:(r)-");
		break;
		}
	case 3556: { // movep y:[qq],x:(r)+
		unhandled("movep y:[qq],x:(r)+");
		break;
		}
	case 3557: { // movep y:[qq],y:(r)+
		unhandled("movep y:[qq],y:(r)+");
		break;
		}
	case 3558: { // movep y:[qq],x:(r)
		unhandled("movep y:[qq],x:(r)");
		break;
		}
	case 3559: { // movep y:[qq],y:(r)
		unhandled("movep y:[qq],y:(r)");
		break;
		}
	case 3560: { // movep y:[qq],x:(r+n)
		unhandled("movep y:[qq],x:(r+n)");
		break;
		}
	case 3561: { // movep y:[qq],y:(r+n)
		unhandled("movep y:[qq],y:(r+n)");
		break;
		}
	case 3562: { // movep y:[qq],x:-(r)
		unhandled("movep y:[qq],x:-(r)");
		break;
		}
	case 3563: { // movep y:[qq],y:-(r)
		unhandled("movep y:[qq],y:-(r)");
		break;
		}
	case 3564: { // movep x:[abs],y:[qq]
		unhandled("movep x:[abs],y:[qq]");
		break;
		}
	case 3565: { // movep y:[abs],y:[qq]
		unhandled("movep y:[abs],y:[qq]");
		break;
		}
	case 3566: { // movep #[i],y:[qq]
		unhandled("movep #[i],y:[qq]");
		break;
		}
	case 3567: { // movep p:(r)-n,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -m_n[ea_r]);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3568: { // movep p:(r)+n,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, m_n[ea_r]);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3569: { // movep p:(r)-,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -1);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3570: { // movep p:(r)+,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, 1);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3571: { // movep p:(r),x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3572: { // movep p:(r+n),x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = calc_add_r(ea_r, m_n[ea_r]);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3573: { // movep p:-(r),x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		add_r(ea_r, -1);
		u32 ea = get_r(ea_r);
		m_x.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3574: { // movep p:(r)-n,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -m_n[ea_r]);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3575: { // movep p:(r)+n,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, m_n[ea_r]);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3576: { // movep p:(r)-,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -1);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3577: { // movep p:(r)+,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, 1);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3578: { // movep p:(r),y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3579: { // movep p:(r+n),y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = calc_add_r(ea_r, m_n[ea_r]);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3580: { // movep p:-(r),y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		int ea_r = BIT(opcode, 8, 6) & 7;
		add_r(ea_r, -1);
		u32 ea = get_r(ea_r);
		m_y.write_dword(pp, m_p.read_dword(ea));
		break;
		}
	case 3581: { // movep x:[pp],p:(r)-n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3582: { // movep x:[pp],p:(r)+n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3583: { // movep x:[pp],p:(r)-
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -1);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3584: { // movep x:[pp],p:(r)+
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, 1);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3585: { // movep x:[pp],p:(r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3586: { // movep x:[pp],p:(r+n)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = calc_add_r(ea_r, m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3587: { // movep x:[pp],p:-(r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		add_r(ea_r, -1);
		u32 ea = get_r(ea_r);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_x.read_dword(pp));
		break;
		}
	case 3588: { // movep y:[pp],p:(r)-n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3589: { // movep y:[pp],p:(r)+n
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3590: { // movep y:[pp],p:(r)-
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, -1);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3591: { // movep y:[pp],p:(r)+
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		add_r(ea_r, 1);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3592: { // movep y:[pp],p:(r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = get_r(ea_r);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3593: { // movep y:[pp],p:(r+n)
		int ea_r = BIT(opcode, 8, 6) & 7;
		u32 ea = calc_add_r(ea_r, m_n[ea_r]);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3594: { // movep y:[pp],p:-(r)
		int ea_r = BIT(opcode, 8, 6) & 7;
		add_r(ea_r, -1);
		u32 ea = get_r(ea_r);
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		m_p.write_dword(ea, m_y.read_dword(pp));
		break;
		}
	case 3595: { // movep p:[abs],x:[pp]
		unhandled("movep p:[abs],x:[pp]");
		break;
		}
	case 3596: { // movep p:[abs],y:[pp]
		unhandled("movep p:[abs],y:[pp]");
		break;
		}
	case 3597: { // movep p:(r)-n,x:[qq]
		unhandled("movep p:(r)-n,x:[qq]");
		break;
		}
	case 3598: { // movep p:(r)+n,x:[qq]
		unhandled("movep p:(r)+n,x:[qq]");
		break;
		}
	case 3599: { // movep p:(r)-,x:[qq]
		unhandled("movep p:(r)-,x:[qq]");
		break;
		}
	case 3600: { // movep p:(r)+,x:[qq]
		unhandled("movep p:(r)+,x:[qq]");
		break;
		}
	case 3601: { // movep p:(r),x:[qq]
		unhandled("movep p:(r),x:[qq]");
		break;
		}
	case 3602: { // movep p:(r+n),x:[qq]
		unhandled("movep p:(r+n),x:[qq]");
		break;
		}
	case 3603: { // movep p:-(r),x:[qq]
		unhandled("movep p:-(r),x:[qq]");
		break;
		}
	case 3604: { // movep p:(r)-n,y:[qq]
		unhandled("movep p:(r)-n,y:[qq]");
		break;
		}
	case 3605: { // movep p:(r)+n,y:[qq]
		unhandled("movep p:(r)+n,y:[qq]");
		break;
		}
	case 3606: { // movep p:(r)-,y:[qq]
		unhandled("movep p:(r)-,y:[qq]");
		break;
		}
	case 3607: { // movep p:(r)+,y:[qq]
		unhandled("movep p:(r)+,y:[qq]");
		break;
		}
	case 3608: { // movep p:(r),y:[qq]
		unhandled("movep p:(r),y:[qq]");
		break;
		}
	case 3609: { // movep p:(r+n),y:[qq]
		unhandled("movep p:(r+n),y:[qq]");
		break;
		}
	case 3610: { // movep p:-(r),y:[qq]
		unhandled("movep p:-(r),y:[qq]");
		break;
		}
	case 3611: { // movep x:[qq],p:(r)-n
		unhandled("movep x:[qq],p:(r)-n");
		break;
		}
	case 3612: { // movep x:[qq],p:(r)+n
		unhandled("movep x:[qq],p:(r)+n");
		break;
		}
	case 3613: { // movep x:[qq],p:(r)-
		unhandled("movep x:[qq],p:(r)-");
		break;
		}
	case 3614: { // movep x:[qq],p:(r)+
		unhandled("movep x:[qq],p:(r)+");
		break;
		}
	case 3615: { // movep x:[qq],p:(r)
		unhandled("movep x:[qq],p:(r)");
		break;
		}
	case 3616: { // movep x:[qq],p:(r+n)
		unhandled("movep x:[qq],p:(r+n)");
		break;
		}
	case 3617: { // movep x:[qq],p:-(r)
		unhandled("movep x:[qq],p:-(r)");
		break;
		}
	case 3618: { // movep y:[qq],p:(r)-n
		unhandled("movep y:[qq],p:(r)-n");
		break;
		}
	case 3619: { // movep y:[qq],p:(r)+n
		unhandled("movep y:[qq],p:(r)+n");
		break;
		}
	case 3620: { // movep y:[qq],p:(r)-
		unhandled("movep y:[qq],p:(r)-");
		break;
		}
	case 3621: { // movep y:[qq],p:(r)+
		unhandled("movep y:[qq],p:(r)+");
		break;
		}
	case 3622: { // movep y:[qq],p:(r)
		unhandled("movep y:[qq],p:(r)");
		break;
		}
	case 3623: { // movep y:[qq],p:(r+n)
		unhandled("movep y:[qq],p:(r+n)");
		break;
		}
	case 3624: { // movep y:[qq],p:-(r)
		unhandled("movep y:[qq],p:-(r)");
		break;
		}
	case 3625: { // movep p:[abs],x:[qq]
		unhandled("movep p:[abs],x:[qq]");
		break;
		}
	case 3626: { // movep p:[abs],y:[qq]
		unhandled("movep p:[abs],y:[qq]");
		break;
		}
	case 3627: { // movep x0,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_x0();
		m_x.write_dword(pp, s);
		break;
		}
	case 3628: { // movep x1,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_x1();
		m_x.write_dword(pp, s);
		break;
		}
	case 3629: { // movep y0,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_y0();
		m_x.write_dword(pp, s);
		break;
		}
	case 3630: { // movep y1,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_y1();
		m_x.write_dword(pp, s);
		break;
		}
	case 3631: { // movep a0,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a0();
		m_x.write_dword(pp, s);
		break;
		}
	case 3632: { // movep b0,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b0();
		m_x.write_dword(pp, s);
		break;
		}
	case 3633: { // movep a2,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a2();
		m_x.write_dword(pp, s);
		break;
		}
	case 3634: { // movep b2,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b2();
		m_x.write_dword(pp, s);
		break;
		}
	case 3635: { // movep a1,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a1();
		m_x.write_dword(pp, s);
		break;
		}
	case 3636: { // movep b1,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b1();
		m_x.write_dword(pp, s);
		break;
		}
	case 3637: { // movep a,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u64 s = get_a();
		m_x.write_dword(pp, s);
		break;
		}
	case 3638: { // movep b,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u64 s = get_b();
		m_x.write_dword(pp, s);
		break;
		}
	case 3639: { // movep r,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(pp, s);
		break;
		}
	case 3640: { // movep n,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(pp, s);
		break;
		}
	case 3641: { // movep m,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(pp, s);
		break;
		}
	case 3642: { // movep ep,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ep();
		m_x.write_dword(pp, s);
		break;
		}
	case 3643: { // movep vba,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_vba();
		m_x.write_dword(pp, s);
		break;
		}
	case 3644: { // movep sc,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sc();
		m_x.write_dword(pp, s);
		break;
		}
	case 3645: { // movep sz,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sz();
		m_x.write_dword(pp, s);
		break;
		}
	case 3646: { // movep sr,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sr();
		m_x.write_dword(pp, s);
		break;
		}
	case 3647: { // movep omr,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_omr();
		m_x.write_dword(pp, s);
		break;
		}
	case 3648: { // movep sp,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sp();
		m_x.write_dword(pp, s);
		break;
		}
	case 3649: { // movep ssh,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ssh();
		m_x.write_dword(pp, s);
		break;
		}
	case 3650: { // movep ssl,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ssl();
		m_x.write_dword(pp, s);
		break;
		}
	case 3651: { // movep la,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_la();
		m_x.write_dword(pp, s);
		break;
		}
	case 3652: { // movep lc,x:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_lc();
		m_x.write_dword(pp, s);
		break;
		}
	case 3653: { // movep x0,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_x0();
		m_y.write_dword(pp, s);
		break;
		}
	case 3654: { // movep x1,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_x1();
		m_y.write_dword(pp, s);
		break;
		}
	case 3655: { // movep y0,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_y0();
		m_y.write_dword(pp, s);
		break;
		}
	case 3656: { // movep y1,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_y1();
		m_y.write_dword(pp, s);
		break;
		}
	case 3657: { // movep a0,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a0();
		m_y.write_dword(pp, s);
		break;
		}
	case 3658: { // movep b0,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b0();
		m_y.write_dword(pp, s);
		break;
		}
	case 3659: { // movep a2,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a2();
		m_y.write_dword(pp, s);
		break;
		}
	case 3660: { // movep b2,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b2();
		m_y.write_dword(pp, s);
		break;
		}
	case 3661: { // movep a1,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_a1();
		m_y.write_dword(pp, s);
		break;
		}
	case 3662: { // movep b1,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_b1();
		m_y.write_dword(pp, s);
		break;
		}
	case 3663: { // movep a,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u64 s = get_a();
		m_y.write_dword(pp, s);
		break;
		}
	case 3664: { // movep b,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u64 s = get_b();
		m_y.write_dword(pp, s);
		break;
		}
	case 3665: { // movep r,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(pp, s);
		break;
		}
	case 3666: { // movep n,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(pp, s);
		break;
		}
	case 3667: { // movep m,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(pp, s);
		break;
		}
	case 3668: { // movep ep,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ep();
		m_y.write_dword(pp, s);
		break;
		}
	case 3669: { // movep vba,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_vba();
		m_y.write_dword(pp, s);
		break;
		}
	case 3670: { // movep sc,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sc();
		m_y.write_dword(pp, s);
		break;
		}
	case 3671: { // movep sz,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sz();
		m_y.write_dword(pp, s);
		break;
		}
	case 3672: { // movep sr,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sr();
		m_y.write_dword(pp, s);
		break;
		}
	case 3673: { // movep omr,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_omr();
		m_y.write_dword(pp, s);
		break;
		}
	case 3674: { // movep sp,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_sp();
		m_y.write_dword(pp, s);
		break;
		}
	case 3675: { // movep ssh,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ssh();
		m_y.write_dword(pp, s);
		break;
		}
	case 3676: { // movep ssl,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_ssl();
		m_y.write_dword(pp, s);
		break;
		}
	case 3677: { // movep la,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_la();
		m_y.write_dword(pp, s);
		break;
		}
	case 3678: { // movep lc,y:[pp]
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		u32 s = get_lc();
		m_y.write_dword(pp, s);
		break;
		}
	case 3679: { // movep x:[pp],x0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_x0(m_x.read_dword(pp));
		break;
		}
	case 3680: { // movep x:[pp],x1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_x1(m_x.read_dword(pp));
		break;
		}
	case 3681: { // movep x:[pp],y0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_y0(m_x.read_dword(pp));
		break;
		}
	case 3682: { // movep x:[pp],y1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_y1(m_x.read_dword(pp));
		break;
		}
	case 3683: { // movep x:[pp],a0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a0(m_x.read_dword(pp));
		break;
		}
	case 3684: { // movep x:[pp],b0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b0(m_x.read_dword(pp));
		break;
		}
	case 3685: { // movep x:[pp],a2
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a2(m_x.read_dword(pp));
		break;
		}
	case 3686: { // movep x:[pp],b2
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b2(m_x.read_dword(pp));
		break;
		}
	case 3687: { // movep x:[pp],a1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a1(m_x.read_dword(pp));
		break;
		}
	case 3688: { // movep x:[pp],b1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b1(m_x.read_dword(pp));
		break;
		}
	case 3689: { // movep x:[pp],a
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a(m_x.read_dword(pp));
		break;
		}
	case 3690: { // movep x:[pp],b
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b(m_x.read_dword(pp));
		break;
		}
	case 3691: { // movep x:[pp],r
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_r(BIT(opcode, 8, 6) & 7, m_x.read_dword(pp));
		break;
		}
	case 3692: { // movep x:[pp],n
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_n(BIT(opcode, 8, 6) & 7, m_x.read_dword(pp));
		break;
		}
	case 3693: { // movep x:[pp],m
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_m(BIT(opcode, 8, 6) & 7, m_x.read_dword(pp));
		break;
		}
	case 3694: { // movep x:[pp],ep
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ep(m_x.read_dword(pp));
		break;
		}
	case 3695: { // movep x:[pp],vba
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_vba(m_x.read_dword(pp));
		break;
		}
	case 3696: { // movep x:[pp],sc
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sc(m_x.read_dword(pp));
		break;
		}
	case 3697: { // movep x:[pp],sz
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sz(m_x.read_dword(pp));
		break;
		}
	case 3698: { // movep x:[pp],sr
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sr(m_x.read_dword(pp));
		break;
		}
	case 3699: { // movep x:[pp],omr
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_omr(m_x.read_dword(pp));
		break;
		}
	case 3700: { // movep x:[pp],sp
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sp(m_x.read_dword(pp));
		break;
		}
	case 3701: { // movep x:[pp],ssh
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ssh(m_x.read_dword(pp));
		break;
		}
	case 3702: { // movep x:[pp],ssl
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ssl(m_x.read_dword(pp));
		break;
		}
	case 3703: { // movep x:[pp],la
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_la(m_x.read_dword(pp));
		break;
		}
	case 3704: { // movep x:[pp],lc
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_lc(m_x.read_dword(pp));
		break;
		}
	case 3705: { // movep y:[pp],x0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_x0(m_y.read_dword(pp));
		break;
		}
	case 3706: { // movep y:[pp],x1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_x1(m_y.read_dword(pp));
		break;
		}
	case 3707: { // movep y:[pp],y0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_y0(m_y.read_dword(pp));
		break;
		}
	case 3708: { // movep y:[pp],y1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_y1(m_y.read_dword(pp));
		break;
		}
	case 3709: { // movep y:[pp],a0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a0(m_y.read_dword(pp));
		break;
		}
	case 3710: { // movep y:[pp],b0
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b0(m_y.read_dword(pp));
		break;
		}
	case 3711: { // movep y:[pp],a2
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a2(m_y.read_dword(pp));
		break;
		}
	case 3712: { // movep y:[pp],b2
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b2(m_y.read_dword(pp));
		break;
		}
	case 3713: { // movep y:[pp],a1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a1(m_y.read_dword(pp));
		break;
		}
	case 3714: { // movep y:[pp],b1
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b1(m_y.read_dword(pp));
		break;
		}
	case 3715: { // movep y:[pp],a
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_a(m_y.read_dword(pp));
		break;
		}
	case 3716: { // movep y:[pp],b
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_b(m_y.read_dword(pp));
		break;
		}
	case 3717: { // movep y:[pp],r
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_r(BIT(opcode, 8, 6) & 7, m_y.read_dword(pp));
		break;
		}
	case 3718: { // movep y:[pp],n
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_n(BIT(opcode, 8, 6) & 7, m_y.read_dword(pp));
		break;
		}
	case 3719: { // movep y:[pp],m
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_m(BIT(opcode, 8, 6) & 7, m_y.read_dword(pp));
		break;
		}
	case 3720: { // movep y:[pp],ep
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ep(m_y.read_dword(pp));
		break;
		}
	case 3721: { // movep y:[pp],vba
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_vba(m_y.read_dword(pp));
		break;
		}
	case 3722: { // movep y:[pp],sc
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sc(m_y.read_dword(pp));
		break;
		}
	case 3723: { // movep y:[pp],sz
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sz(m_y.read_dword(pp));
		break;
		}
	case 3724: { // movep y:[pp],sr
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sr(m_y.read_dword(pp));
		break;
		}
	case 3725: { // movep y:[pp],omr
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_omr(m_y.read_dword(pp));
		break;
		}
	case 3726: { // movep y:[pp],sp
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_sp(m_y.read_dword(pp));
		break;
		}
	case 3727: { // movep y:[pp],ssh
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ssh(m_y.read_dword(pp));
		break;
		}
	case 3728: { // movep y:[pp],ssl
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_ssl(m_y.read_dword(pp));
		break;
		}
	case 3729: { // movep y:[pp],la
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_la(m_y.read_dword(pp));
		break;
		}
	case 3730: { // movep y:[pp],lc
		u32 pp = 0xffffc0 + BIT(opcode, 0, 6);
		set_lc(m_y.read_dword(pp));
		break;
		}
	case 3731: { // movep x0,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_x0();
		m_x.write_dword(qq, s);
		break;
		}
	case 3732: { // movep x1,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_x1();
		m_x.write_dword(qq, s);
		break;
		}
	case 3733: { // movep y0,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_y0();
		m_x.write_dword(qq, s);
		break;
		}
	case 3734: { // movep y1,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_y1();
		m_x.write_dword(qq, s);
		break;
		}
	case 3735: { // movep a0,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a0();
		m_x.write_dword(qq, s);
		break;
		}
	case 3736: { // movep b0,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b0();
		m_x.write_dword(qq, s);
		break;
		}
	case 3737: { // movep a2,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a2();
		m_x.write_dword(qq, s);
		break;
		}
	case 3738: { // movep b2,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b2();
		m_x.write_dword(qq, s);
		break;
		}
	case 3739: { // movep a1,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a1();
		m_x.write_dword(qq, s);
		break;
		}
	case 3740: { // movep b1,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b1();
		m_x.write_dword(qq, s);
		break;
		}
	case 3741: { // movep a,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u64 s = get_a();
		m_x.write_dword(qq, s);
		break;
		}
	case 3742: { // movep b,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u64 s = get_b();
		m_x.write_dword(qq, s);
		break;
		}
	case 3743: { // movep r,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(qq, s);
		break;
		}
	case 3744: { // movep n,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(qq, s);
		break;
		}
	case 3745: { // movep m,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		m_x.write_dword(qq, s);
		break;
		}
	case 3746: { // movep ep,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ep();
		m_x.write_dword(qq, s);
		break;
		}
	case 3747: { // movep vba,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_vba();
		m_x.write_dword(qq, s);
		break;
		}
	case 3748: { // movep sc,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sc();
		m_x.write_dword(qq, s);
		break;
		}
	case 3749: { // movep sz,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sz();
		m_x.write_dword(qq, s);
		break;
		}
	case 3750: { // movep sr,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sr();
		m_x.write_dword(qq, s);
		break;
		}
	case 3751: { // movep omr,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_omr();
		m_x.write_dword(qq, s);
		break;
		}
	case 3752: { // movep sp,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sp();
		m_x.write_dword(qq, s);
		break;
		}
	case 3753: { // movep ssh,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ssh();
		m_x.write_dword(qq, s);
		break;
		}
	case 3754: { // movep ssl,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ssl();
		m_x.write_dword(qq, s);
		break;
		}
	case 3755: { // movep la,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_la();
		m_x.write_dword(qq, s);
		break;
		}
	case 3756: { // movep lc,x:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_lc();
		m_x.write_dword(qq, s);
		break;
		}
	case 3757: { // movep x:[qq],x0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_x0(m_x.read_dword(qq));
		break;
		}
	case 3758: { // movep x:[qq],x1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_x1(m_x.read_dword(qq));
		break;
		}
	case 3759: { // movep x:[qq],y0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_y0(m_x.read_dword(qq));
		break;
		}
	case 3760: { // movep x:[qq],y1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_y1(m_x.read_dword(qq));
		break;
		}
	case 3761: { // movep x:[qq],a0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a0(m_x.read_dword(qq));
		break;
		}
	case 3762: { // movep x:[qq],b0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b0(m_x.read_dword(qq));
		break;
		}
	case 3763: { // movep x:[qq],a2
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a2(m_x.read_dword(qq));
		break;
		}
	case 3764: { // movep x:[qq],b2
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b2(m_x.read_dword(qq));
		break;
		}
	case 3765: { // movep x:[qq],a1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a1(m_x.read_dword(qq));
		break;
		}
	case 3766: { // movep x:[qq],b1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b1(m_x.read_dword(qq));
		break;
		}
	case 3767: { // movep x:[qq],a
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a(m_x.read_dword(qq));
		break;
		}
	case 3768: { // movep x:[qq],b
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b(m_x.read_dword(qq));
		break;
		}
	case 3769: { // movep x:[qq],r
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_r(BIT(opcode, 8, 6) & 7, m_x.read_dword(qq));
		break;
		}
	case 3770: { // movep x:[qq],n
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_n(BIT(opcode, 8, 6) & 7, m_x.read_dword(qq));
		break;
		}
	case 3771: { // movep x:[qq],m
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_m(BIT(opcode, 8, 6) & 7, m_x.read_dword(qq));
		break;
		}
	case 3772: { // movep x:[qq],ep
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ep(m_x.read_dword(qq));
		break;
		}
	case 3773: { // movep x:[qq],vba
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_vba(m_x.read_dword(qq));
		break;
		}
	case 3774: { // movep x:[qq],sc
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sc(m_x.read_dword(qq));
		break;
		}
	case 3775: { // movep x:[qq],sz
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sz(m_x.read_dword(qq));
		break;
		}
	case 3776: { // movep x:[qq],sr
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sr(m_x.read_dword(qq));
		break;
		}
	case 3777: { // movep x:[qq],omr
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_omr(m_x.read_dword(qq));
		break;
		}
	case 3778: { // movep x:[qq],sp
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sp(m_x.read_dword(qq));
		break;
		}
	case 3779: { // movep x:[qq],ssh
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ssh(m_x.read_dword(qq));
		break;
		}
	case 3780: { // movep x:[qq],ssl
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ssl(m_x.read_dword(qq));
		break;
		}
	case 3781: { // movep x:[qq],la
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_la(m_x.read_dword(qq));
		break;
		}
	case 3782: { // movep x:[qq],lc
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_lc(m_x.read_dword(qq));
		break;
		}
	case 3783: { // movep x0,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_x0();
		m_y.write_dword(qq, s);
		break;
		}
	case 3784: { // movep x1,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_x1();
		m_y.write_dword(qq, s);
		break;
		}
	case 3785: { // movep y0,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_y0();
		m_y.write_dword(qq, s);
		break;
		}
	case 3786: { // movep y1,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_y1();
		m_y.write_dword(qq, s);
		break;
		}
	case 3787: { // movep a0,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a0();
		m_y.write_dword(qq, s);
		break;
		}
	case 3788: { // movep b0,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b0();
		m_y.write_dword(qq, s);
		break;
		}
	case 3789: { // movep a2,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a2();
		m_y.write_dword(qq, s);
		break;
		}
	case 3790: { // movep b2,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b2();
		m_y.write_dword(qq, s);
		break;
		}
	case 3791: { // movep a1,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_a1();
		m_y.write_dword(qq, s);
		break;
		}
	case 3792: { // movep b1,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_b1();
		m_y.write_dword(qq, s);
		break;
		}
	case 3793: { // movep a,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u64 s = get_a();
		m_y.write_dword(qq, s);
		break;
		}
	case 3794: { // movep b,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u64 s = get_b();
		m_y.write_dword(qq, s);
		break;
		}
	case 3795: { // movep r,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_r(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(qq, s);
		break;
		}
	case 3796: { // movep n,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_n(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(qq, s);
		break;
		}
	case 3797: { // movep m,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_m(BIT(opcode, 8, 6) & 7);
		m_y.write_dword(qq, s);
		break;
		}
	case 3798: { // movep ep,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ep();
		m_y.write_dword(qq, s);
		break;
		}
	case 3799: { // movep vba,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_vba();
		m_y.write_dword(qq, s);
		break;
		}
	case 3800: { // movep sc,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sc();
		m_y.write_dword(qq, s);
		break;
		}
	case 3801: { // movep sz,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sz();
		m_y.write_dword(qq, s);
		break;
		}
	case 3802: { // movep sr,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sr();
		m_y.write_dword(qq, s);
		break;
		}
	case 3803: { // movep omr,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_omr();
		m_y.write_dword(qq, s);
		break;
		}
	case 3804: { // movep sp,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_sp();
		m_y.write_dword(qq, s);
		break;
		}
	case 3805: { // movep ssh,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ssh();
		m_y.write_dword(qq, s);
		break;
		}
	case 3806: { // movep ssl,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_ssl();
		m_y.write_dword(qq, s);
		break;
		}
	case 3807: { // movep la,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_la();
		m_y.write_dword(qq, s);
		break;
		}
	case 3808: { // movep lc,y:[qq]
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		u32 s = get_lc();
		m_y.write_dword(qq, s);
		break;
		}
	case 3809: { // movep y:[qq],x0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_x0(m_y.read_dword(qq));
		break;
		}
	case 3810: { // movep y:[qq],x1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_x1(m_y.read_dword(qq));
		break;
		}
	case 3811: { // movep y:[qq],y0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_y0(m_y.read_dword(qq));
		break;
		}
	case 3812: { // movep y:[qq],y1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_y1(m_y.read_dword(qq));
		break;
		}
	case 3813: { // movep y:[qq],a0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a0(m_y.read_dword(qq));
		break;
		}
	case 3814: { // movep y:[qq],b0
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b0(m_y.read_dword(qq));
		break;
		}
	case 3815: { // movep y:[qq],a2
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a2(m_y.read_dword(qq));
		break;
		}
	case 3816: { // movep y:[qq],b2
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b2(m_y.read_dword(qq));
		break;
		}
	case 3817: { // movep y:[qq],a1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a1(m_y.read_dword(qq));
		break;
		}
	case 3818: { // movep y:[qq],b1
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b1(m_y.read_dword(qq));
		break;
		}
	case 3819: { // movep y:[qq],a
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_a(m_y.read_dword(qq));
		break;
		}
	case 3820: { // movep y:[qq],b
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_b(m_y.read_dword(qq));
		break;
		}
	case 3821: { // movep y:[qq],r
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_r(BIT(opcode, 8, 6) & 7, m_y.read_dword(qq));
		break;
		}
	case 3822: { // movep y:[qq],n
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_n(BIT(opcode, 8, 6) & 7, m_y.read_dword(qq));
		break;
		}
	case 3823: { // movep y:[qq],m
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_m(BIT(opcode, 8, 6) & 7, m_y.read_dword(qq));
		break;
		}
	case 3824: { // movep y:[qq],ep
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ep(m_y.read_dword(qq));
		break;
		}
	case 3825: { // movep y:[qq],vba
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_vba(m_y.read_dword(qq));
		break;
		}
	case 3826: { // movep y:[qq],sc
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sc(m_y.read_dword(qq));
		break;
		}
	case 3827: { // movep y:[qq],sz
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sz(m_y.read_dword(qq));
		break;
		}
	case 3828: { // movep y:[qq],sr
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sr(m_y.read_dword(qq));
		break;
		}
	case 3829: { // movep y:[qq],omr
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_omr(m_y.read_dword(qq));
		break;
		}
	case 3830: { // movep y:[qq],sp
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_sp(m_y.read_dword(qq));
		break;
		}
	case 3831: { // movep y:[qq],ssh
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ssh(m_y.read_dword(qq));
		break;
		}
	case 3832: { // movep y:[qq],ssl
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_ssl(m_y.read_dword(qq));
		break;
		}
	case 3833: { // movep y:[qq],la
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_la(m_y.read_dword(qq));
		break;
		}
	case 3834: { // movep y:[qq],lc
		u32 qq = 0xffff80 + bitswap<6>(opcode, 6, 4, 3, 2, 1, 0);
		set_lc(m_y.read_dword(qq));
		break;
		}
	case 3835: { // mpy +y1,#[i],a
		unhandled("mpy +y1,#[i],a");
		break;
		}
	case 3836: { // mpy -y1,#[i],a
		unhandled("mpy -y1,#[i],a");
		break;
		}
	case 3837: { // mpy +y1,#[i],b
		unhandled("mpy +y1,#[i],b");
		break;
		}
	case 3838: { // mpy -y1,#[i],b
		unhandled("mpy -y1,#[i],b");
		break;
		}
	case 3839: { // mpy +x0,#[i],a
		unhandled("mpy +x0,#[i],a");
		break;
		}
	case 3840: { // mpy -x0,#[i],a
		unhandled("mpy -x0,#[i],a");
		break;
		}
	case 3841: { // mpy +x0,#[i],b
		unhandled("mpy +x0,#[i],b");
		break;
		}
	case 3842: { // mpy -x0,#[i],b
		unhandled("mpy -x0,#[i],b");
		break;
		}
	case 3843: { // mpy +y0,#[i],a
		unhandled("mpy +y0,#[i],a");
		break;
		}
	case 3844: { // mpy -y0,#[i],a
		unhandled("mpy -y0,#[i],a");
		break;
		}
	case 3845: { // mpy +y0,#[i],b
		unhandled("mpy +y0,#[i],b");
		break;
		}
	case 3846: { // mpy -y0,#[i],b
		unhandled("mpy -y0,#[i],b");
		break;
		}
	case 3847: { // mpy +x1,#[i],a
		unhandled("mpy +x1,#[i],a");
		break;
		}
	case 3848: { // mpy -x1,#[i],a
		unhandled("mpy -x1,#[i],a");
		break;
		}
	case 3849: { // mpy +x1,#[i],b
		unhandled("mpy +x1,#[i],b");
		break;
		}
	case 3850: { // mpy -x1,#[i],b
		unhandled("mpy -x1,#[i],b");
		break;
		}
	case 3851: { // mpysu +x0,x0,a
		unhandled("mpysu +x0,x0,a");
		break;
		}
	case 3852: { // mpysu +y0,y0,a
		unhandled("mpysu +y0,y0,a");
		break;
		}
	case 3853: { // mpysu +x1,x0,a
		unhandled("mpysu +x1,x0,a");
		break;
		}
	case 3854: { // mpysu +y1,y0,a
		unhandled("mpysu +y1,y0,a");
		break;
		}
	case 3855: { // mpysu +x1,x1,a
		unhandled("mpysu +x1,x1,a");
		break;
		}
	case 3856: { // mpysu +y1,y1,a
		unhandled("mpysu +y1,y1,a");
		break;
		}
	case 3857: { // mpysu +x0,x1,a
		unhandled("mpysu +x0,x1,a");
		break;
		}
	case 3858: { // mpysu +y0,y1,a
		unhandled("mpysu +y0,y1,a");
		break;
		}
	case 3859: { // mpysu +x0,y1,a
		unhandled("mpysu +x0,y1,a");
		break;
		}
	case 3860: { // mpysu +y0,x0,a
		unhandled("mpysu +y0,x0,a");
		break;
		}
	case 3861: { // mpysu +x1,y0,a
		unhandled("mpysu +x1,y0,a");
		break;
		}
	case 3862: { // mpysu +y1,x1,a
		unhandled("mpysu +y1,x1,a");
		break;
		}
	case 3863: { // mpysu +y1,x0,a
		unhandled("mpysu +y1,x0,a");
		break;
		}
	case 3864: { // mpysu +x0,y0,a
		unhandled("mpysu +x0,y0,a");
		break;
		}
	case 3865: { // mpysu +y0,x1,a
		unhandled("mpysu +y0,x1,a");
		break;
		}
	case 3866: { // mpysu +x1,y1,a
		unhandled("mpysu +x1,y1,a");
		break;
		}
	case 3867: { // mpysu -x0,x0,a
		unhandled("mpysu -x0,x0,a");
		break;
		}
	case 3868: { // mpysu -y0,y0,a
		unhandled("mpysu -y0,y0,a");
		break;
		}
	case 3869: { // mpysu -x1,x0,a
		unhandled("mpysu -x1,x0,a");
		break;
		}
	case 3870: { // mpysu -y1,y0,a
		unhandled("mpysu -y1,y0,a");
		break;
		}
	case 3871: { // mpysu -x1,x1,a
		unhandled("mpysu -x1,x1,a");
		break;
		}
	case 3872: { // mpysu -y1,y1,a
		unhandled("mpysu -y1,y1,a");
		break;
		}
	case 3873: { // mpysu -x0,x1,a
		unhandled("mpysu -x0,x1,a");
		break;
		}
	case 3874: { // mpysu -y0,y1,a
		unhandled("mpysu -y0,y1,a");
		break;
		}
	case 3875: { // mpysu -x0,y1,a
		unhandled("mpysu -x0,y1,a");
		break;
		}
	case 3876: { // mpysu -y0,x0,a
		unhandled("mpysu -y0,x0,a");
		break;
		}
	case 3877: { // mpysu -x1,y0,a
		unhandled("mpysu -x1,y0,a");
		break;
		}
	case 3878: { // mpysu -y1,x1,a
		unhandled("mpysu -y1,x1,a");
		break;
		}
	case 3879: { // mpysu -y1,x0,a
		unhandled("mpysu -y1,x0,a");
		break;
		}
	case 3880: { // mpysu -x0,y0,a
		unhandled("mpysu -x0,y0,a");
		break;
		}
	case 3881: { // mpysu -y0,x1,a
		unhandled("mpysu -y0,x1,a");
		break;
		}
	case 3882: { // mpysu -x1,y1,a
		unhandled("mpysu -x1,y1,a");
		break;
		}
	case 3883: { // mpysu +x0,x0,b
		unhandled("mpysu +x0,x0,b");
		break;
		}
	case 3884: { // mpysu +y0,y0,b
		unhandled("mpysu +y0,y0,b");
		break;
		}
	case 3885: { // mpysu +x1,x0,b
		unhandled("mpysu +x1,x0,b");
		break;
		}
	case 3886: { // mpysu +y1,y0,b
		unhandled("mpysu +y1,y0,b");
		break;
		}
	case 3887: { // mpysu +x1,x1,b
		unhandled("mpysu +x1,x1,b");
		break;
		}
	case 3888: { // mpysu +y1,y1,b
		unhandled("mpysu +y1,y1,b");
		break;
		}
	case 3889: { // mpysu +x0,x1,b
		unhandled("mpysu +x0,x1,b");
		break;
		}
	case 3890: { // mpysu +y0,y1,b
		unhandled("mpysu +y0,y1,b");
		break;
		}
	case 3891: { // mpysu +x0,y1,b
		unhandled("mpysu +x0,y1,b");
		break;
		}
	case 3892: { // mpysu +y0,x0,b
		unhandled("mpysu +y0,x0,b");
		break;
		}
	case 3893: { // mpysu +x1,y0,b
		unhandled("mpysu +x1,y0,b");
		break;
		}
	case 3894: { // mpysu +y1,x1,b
		unhandled("mpysu +y1,x1,b");
		break;
		}
	case 3895: { // mpysu +y1,x0,b
		unhandled("mpysu +y1,x0,b");
		break;
		}
	case 3896: { // mpysu +x0,y0,b
		unhandled("mpysu +x0,y0,b");
		break;
		}
	case 3897: { // mpysu +y0,x1,b
		unhandled("mpysu +y0,x1,b");
		break;
		}
	case 3898: { // mpysu +x1,y1,b
		unhandled("mpysu +x1,y1,b");
		break;
		}
	case 3899: { // mpysu -x0,x0,b
		unhandled("mpysu -x0,x0,b");
		break;
		}
	case 3900: { // mpysu -y0,y0,b
		unhandled("mpysu -y0,y0,b");
		break;
		}
	case 3901: { // mpysu -x1,x0,b
		unhandled("mpysu -x1,x0,b");
		break;
		}
	case 3902: { // mpysu -y1,y0,b
		unhandled("mpysu -y1,y0,b");
		break;
		}
	case 3903: { // mpysu -x1,x1,b
		unhandled("mpysu -x1,x1,b");
		break;
		}
	case 3904: { // mpysu -y1,y1,b
		unhandled("mpysu -y1,y1,b");
		break;
		}
	case 3905: { // mpysu -x0,x1,b
		unhandled("mpysu -x0,x1,b");
		break;
		}
	case 3906: { // mpysu -y0,y1,b
		unhandled("mpysu -y0,y1,b");
		break;
		}
	case 3907: { // mpysu -x0,y1,b
		unhandled("mpysu -x0,y1,b");
		break;
		}
	case 3908: { // mpysu -y0,x0,b
		unhandled("mpysu -y0,x0,b");
		break;
		}
	case 3909: { // mpysu -x1,y0,b
		unhandled("mpysu -x1,y0,b");
		break;
		}
	case 3910: { // mpysu -y1,x1,b
		unhandled("mpysu -y1,x1,b");
		break;
		}
	case 3911: { // mpysu -y1,x0,b
		unhandled("mpysu -y1,x0,b");
		break;
		}
	case 3912: { // mpysu -x0,y0,b
		unhandled("mpysu -x0,y0,b");
		break;
		}
	case 3913: { // mpysu -y0,x1,b
		unhandled("mpysu -y0,x1,b");
		break;
		}
	case 3914: { // mpysu -x1,y1,b
		unhandled("mpysu -x1,y1,b");
		break;
		}
	case 3915: { // mpyuu +x0,x0,a
		unhandled("mpyuu +x0,x0,a");
		break;
		}
	case 3916: { // mpyuu +y0,y0,a
		unhandled("mpyuu +y0,y0,a");
		break;
		}
	case 3917: { // mpyuu +x1,x0,a
		unhandled("mpyuu +x1,x0,a");
		break;
		}
	case 3918: { // mpyuu +y1,y0,a
		unhandled("mpyuu +y1,y0,a");
		break;
		}
	case 3919: { // mpyuu +x1,x1,a
		unhandled("mpyuu +x1,x1,a");
		break;
		}
	case 3920: { // mpyuu +y1,y1,a
		unhandled("mpyuu +y1,y1,a");
		break;
		}
	case 3921: { // mpyuu +x0,x1,a
		unhandled("mpyuu +x0,x1,a");
		break;
		}
	case 3922: { // mpyuu +y0,y1,a
		unhandled("mpyuu +y0,y1,a");
		break;
		}
	case 3923: { // mpyuu +x0,y1,a
		unhandled("mpyuu +x0,y1,a");
		break;
		}
	case 3924: { // mpyuu +y0,x0,a
		unhandled("mpyuu +y0,x0,a");
		break;
		}
	case 3925: { // mpyuu +x1,y0,a
		unhandled("mpyuu +x1,y0,a");
		break;
		}
	case 3926: { // mpyuu +y1,x1,a
		unhandled("mpyuu +y1,x1,a");
		break;
		}
	case 3927: { // mpyuu +y1,x0,a
		unhandled("mpyuu +y1,x0,a");
		break;
		}
	case 3928: { // mpyuu +x0,y0,a
		unhandled("mpyuu +x0,y0,a");
		break;
		}
	case 3929: { // mpyuu +y0,x1,a
		unhandled("mpyuu +y0,x1,a");
		break;
		}
	case 3930: { // mpyuu +x1,y1,a
		unhandled("mpyuu +x1,y1,a");
		break;
		}
	case 3931: { // mpyuu -x0,x0,a
		unhandled("mpyuu -x0,x0,a");
		break;
		}
	case 3932: { // mpyuu -y0,y0,a
		unhandled("mpyuu -y0,y0,a");
		break;
		}
	case 3933: { // mpyuu -x1,x0,a
		unhandled("mpyuu -x1,x0,a");
		break;
		}
	case 3934: { // mpyuu -y1,y0,a
		unhandled("mpyuu -y1,y0,a");
		break;
		}
	case 3935: { // mpyuu -x1,x1,a
		unhandled("mpyuu -x1,x1,a");
		break;
		}
	case 3936: { // mpyuu -y1,y1,a
		unhandled("mpyuu -y1,y1,a");
		break;
		}
	case 3937: { // mpyuu -x0,x1,a
		unhandled("mpyuu -x0,x1,a");
		break;
		}
	case 3938: { // mpyuu -y0,y1,a
		unhandled("mpyuu -y0,y1,a");
		break;
		}
	case 3939: { // mpyuu -x0,y1,a
		unhandled("mpyuu -x0,y1,a");
		break;
		}
	case 3940: { // mpyuu -y0,x0,a
		unhandled("mpyuu -y0,x0,a");
		break;
		}
	case 3941: { // mpyuu -x1,y0,a
		unhandled("mpyuu -x1,y0,a");
		break;
		}
	case 3942: { // mpyuu -y1,x1,a
		unhandled("mpyuu -y1,x1,a");
		break;
		}
	case 3943: { // mpyuu -y1,x0,a
		unhandled("mpyuu -y1,x0,a");
		break;
		}
	case 3944: { // mpyuu -x0,y0,a
		unhandled("mpyuu -x0,y0,a");
		break;
		}
	case 3945: { // mpyuu -y0,x1,a
		unhandled("mpyuu -y0,x1,a");
		break;
		}
	case 3946: { // mpyuu -x1,y1,a
		unhandled("mpyuu -x1,y1,a");
		break;
		}
	case 3947: { // mpyuu +x0,x0,b
		unhandled("mpyuu +x0,x0,b");
		break;
		}
	case 3948: { // mpyuu +y0,y0,b
		unhandled("mpyuu +y0,y0,b");
		break;
		}
	case 3949: { // mpyuu +x1,x0,b
		unhandled("mpyuu +x1,x0,b");
		break;
		}
	case 3950: { // mpyuu +y1,y0,b
		unhandled("mpyuu +y1,y0,b");
		break;
		}
	case 3951: { // mpyuu +x1,x1,b
		unhandled("mpyuu +x1,x1,b");
		break;
		}
	case 3952: { // mpyuu +y1,y1,b
		unhandled("mpyuu +y1,y1,b");
		break;
		}
	case 3953: { // mpyuu +x0,x1,b
		unhandled("mpyuu +x0,x1,b");
		break;
		}
	case 3954: { // mpyuu +y0,y1,b
		unhandled("mpyuu +y0,y1,b");
		break;
		}
	case 3955: { // mpyuu +x0,y1,b
		unhandled("mpyuu +x0,y1,b");
		break;
		}
	case 3956: { // mpyuu +y0,x0,b
		unhandled("mpyuu +y0,x0,b");
		break;
		}
	case 3957: { // mpyuu +x1,y0,b
		unhandled("mpyuu +x1,y0,b");
		break;
		}
	case 3958: { // mpyuu +y1,x1,b
		unhandled("mpyuu +y1,x1,b");
		break;
		}
	case 3959: { // mpyuu +y1,x0,b
		unhandled("mpyuu +y1,x0,b");
		break;
		}
	case 3960: { // mpyuu +x0,y0,b
		unhandled("mpyuu +x0,y0,b");
		break;
		}
	case 3961: { // mpyuu +y0,x1,b
		unhandled("mpyuu +y0,x1,b");
		break;
		}
	case 3962: { // mpyuu +x1,y1,b
		unhandled("mpyuu +x1,y1,b");
		break;
		}
	case 3963: { // mpyuu -x0,x0,b
		unhandled("mpyuu -x0,x0,b");
		break;
		}
	case 3964: { // mpyuu -y0,y0,b
		unhandled("mpyuu -y0,y0,b");
		break;
		}
	case 3965: { // mpyuu -x1,x0,b
		unhandled("mpyuu -x1,x0,b");
		break;
		}
	case 3966: { // mpyuu -y1,y0,b
		unhandled("mpyuu -y1,y0,b");
		break;
		}
	case 3967: { // mpyuu -x1,x1,b
		unhandled("mpyuu -x1,x1,b");
		break;
		}
	case 3968: { // mpyuu -y1,y1,b
		unhandled("mpyuu -y1,y1,b");
		break;
		}
	case 3969: { // mpyuu -x0,x1,b
		unhandled("mpyuu -x0,x1,b");
		break;
		}
	case 3970: { // mpyuu -y0,y1,b
		unhandled("mpyuu -y0,y1,b");
		break;
		}
	case 3971: { // mpyuu -x0,y1,b
		unhandled("mpyuu -x0,y1,b");
		break;
		}
	case 3972: { // mpyuu -y0,x0,b
		unhandled("mpyuu -y0,x0,b");
		break;
		}
	case 3973: { // mpyuu -x1,y0,b
		unhandled("mpyuu -x1,y0,b");
		break;
		}
	case 3974: { // mpyuu -y1,x1,b
		unhandled("mpyuu -y1,x1,b");
		break;
		}
	case 3975: { // mpyuu -y1,x0,b
		unhandled("mpyuu -y1,x0,b");
		break;
		}
	case 3976: { // mpyuu -x0,y0,b
		unhandled("mpyuu -x0,y0,b");
		break;
		}
	case 3977: { // mpyuu -y0,x1,b
		unhandled("mpyuu -y0,x1,b");
		break;
		}
	case 3978: { // mpyuu -x1,y1,b
		unhandled("mpyuu -x1,y1,b");
		break;
		}
	case 3979: { // mpyi +#[i],y1,a
		unhandled("mpyi +#[i],y1,a");
		break;
		}
	case 3980: { // mpyi -#[i],y1,a
		unhandled("mpyi -#[i],y1,a");
		break;
		}
	case 3981: { // mpyi +#[i],y1,b
		unhandled("mpyi +#[i],y1,b");
		break;
		}
	case 3982: { // mpyi -#[i],y1,b
		unhandled("mpyi -#[i],y1,b");
		break;
		}
	case 3983: { // mpyi +#[i],x0,a
		unhandled("mpyi +#[i],x0,a");
		break;
		}
	case 3984: { // mpyi -#[i],x0,a
		unhandled("mpyi -#[i],x0,a");
		break;
		}
	case 3985: { // mpyi +#[i],x0,b
		unhandled("mpyi +#[i],x0,b");
		break;
		}
	case 3986: { // mpyi -#[i],x0,b
		unhandled("mpyi -#[i],x0,b");
		break;
		}
	case 3987: { // mpyi +#[i],y0,a
		unhandled("mpyi +#[i],y0,a");
		break;
		}
	case 3988: { // mpyi -#[i],y0,a
		unhandled("mpyi -#[i],y0,a");
		break;
		}
	case 3989: { // mpyi +#[i],y0,b
		unhandled("mpyi +#[i],y0,b");
		break;
		}
	case 3990: { // mpyi -#[i],y0,b
		unhandled("mpyi -#[i],y0,b");
		break;
		}
	case 3991: { // mpyi +#[i],x1,a
		unhandled("mpyi +#[i],x1,a");
		break;
		}
	case 3992: { // mpyi -#[i],x1,a
		unhandled("mpyi -#[i],x1,a");
		break;
		}
	case 3993: { // mpyi +#[i],x1,b
		unhandled("mpyi +#[i],x1,b");
		break;
		}
	case 3994: { // mpyi -#[i],x1,b
		unhandled("mpyi -#[i],x1,b");
		break;
		}
	case 3995: { // mpyr +y1,#[i],a
		unhandled("mpyr +y1,#[i],a");
		break;
		}
	case 3996: { // mpyr -y1,#[i],a
		unhandled("mpyr -y1,#[i],a");
		break;
		}
	case 3997: { // mpyr +y1,#[i],b
		unhandled("mpyr +y1,#[i],b");
		break;
		}
	case 3998: { // mpyr -y1,#[i],b
		unhandled("mpyr -y1,#[i],b");
		break;
		}
	case 3999: { // mpyr +x0,#[i],a
		unhandled("mpyr +x0,#[i],a");
		break;
		}
	case 4000: { // mpyr -x0,#[i],a
		unhandled("mpyr -x0,#[i],a");
		break;
		}
	case 4001: { // mpyr +x0,#[i],b
		unhandled("mpyr +x0,#[i],b");
		break;
		}
	case 4002: { // mpyr -x0,#[i],b
		unhandled("mpyr -x0,#[i],b");
		break;
		}
	case 4003: { // mpyr +y0,#[i],a
		unhandled("mpyr +y0,#[i],a");
		break;
		}
	case 4004: { // mpyr -y0,#[i],a
		unhandled("mpyr -y0,#[i],a");
		break;
		}
	case 4005: { // mpyr +y0,#[i],b
		unhandled("mpyr +y0,#[i],b");
		break;
		}
	case 4006: { // mpyr -y0,#[i],b
		unhandled("mpyr -y0,#[i],b");
		break;
		}
	case 4007: { // mpyr +x1,#[i],a
		unhandled("mpyr +x1,#[i],a");
		break;
		}
	case 4008: { // mpyr -x1,#[i],a
		unhandled("mpyr -x1,#[i],a");
		break;
		}
	case 4009: { // mpyr +x1,#[i],b
		unhandled("mpyr +x1,#[i],b");
		break;
		}
	case 4010: { // mpyr -x1,#[i],b
		unhandled("mpyr -x1,#[i],b");
		break;
		}
	case 4011: { // mpyri +#[i],y1,a
		unhandled("mpyri +#[i],y1,a");
		break;
		}
	case 4012: { // mpyri -#[i],y1,a
		unhandled("mpyri -#[i],y1,a");
		break;
		}
	case 4013: { // mpyri +#[i],y1,b
		unhandled("mpyri +#[i],y1,b");
		break;
		}
	case 4014: { // mpyri -#[i],y1,b
		unhandled("mpyri -#[i],y1,b");
		break;
		}
	case 4015: { // mpyri +#[i],x0,a
		unhandled("mpyri +#[i],x0,a");
		break;
		}
	case 4016: { // mpyri -#[i],x0,a
		unhandled("mpyri -#[i],x0,a");
		break;
		}
	case 4017: { // mpyri +#[i],x0,b
		unhandled("mpyri +#[i],x0,b");
		break;
		}
	case 4018: { // mpyri -#[i],x0,b
		unhandled("mpyri -#[i],x0,b");
		break;
		}
	case 4019: { // mpyri +#[i],y0,a
		unhandled("mpyri +#[i],y0,a");
		break;
		}
	case 4020: { // mpyri -#[i],y0,a
		unhandled("mpyri -#[i],y0,a");
		break;
		}
	case 4021: { // mpyri +#[i],y0,b
		unhandled("mpyri +#[i],y0,b");
		break;
		}
	case 4022: { // mpyri -#[i],y0,b
		unhandled("mpyri -#[i],y0,b");
		break;
		}
	case 4023: { // mpyri +#[i],x1,a
		unhandled("mpyri +#[i],x1,a");
		break;
		}
	case 4024: { // mpyri -#[i],x1,a
		unhandled("mpyri -#[i],x1,a");
		break;
		}
	case 4025: { // mpyri +#[i],x1,b
		unhandled("mpyri +#[i],x1,b");
		break;
		}
	case 4026: { // mpyri -#[i],x1,b
		unhandled("mpyri -#[i],x1,b");
		break;
		}
	case 4027: { // norm r,a
		unhandled("norm r,a");
		break;
		}
	case 4028: { // norm r,b
		unhandled("norm r,b");
		break;
		}
	case 4029: { // normf a1,a
		unhandled("normf a1,a");
		break;
		}
	case 4030: { // normf a1,b
		unhandled("normf a1,b");
		break;
		}
	case 4031: { // normf b1,a
		unhandled("normf b1,a");
		break;
		}
	case 4032: { // normf b1,b
		unhandled("normf b1,b");
		break;
		}
	case 4033: { // normf x0,a
		unhandled("normf x0,a");
		break;
		}
	case 4034: { // normf x0,b
		unhandled("normf x0,b");
		break;
		}
	case 4035: { // normf y0,a
		unhandled("normf y0,a");
		break;
		}
	case 4036: { // normf y0,b
		unhandled("normf y0,b");
		break;
		}
	case 4037: { // normf x1,a
		unhandled("normf x1,a");
		break;
		}
	case 4038: { // normf x1,b
		unhandled("normf x1,b");
		break;
		}
	case 4039: { // normf y1,a
		unhandled("normf y1,a");
		break;
		}
	case 4040: { // normf y1,b
		unhandled("normf y1,b");
		break;
		}
	case 4041: { // or #[i],a
		unhandled("or #[i],a");
		break;
		}
	case 4042: { // or #[i],b
		unhandled("or #[i],b");
		break;
		}
	case 4043: { // or #[i],a
		unhandled("or #[i],a");
		break;
		}
	case 4044: { // or #[i],b
		unhandled("or #[i],b");
		break;
		}
	case 4045: { // ori #[i],mr
		unhandled("ori #[i],mr");
		break;
		}
	case 4046: { // ori #[i],ccr
		unhandled("ori #[i],ccr");
		break;
		}
	case 4047: { // ori #[i],com
		unhandled("ori #[i],com");
		break;
		}
	case 4048: { // ori #[i],eom
		unhandled("ori #[i],eom");
		break;
		}
	case 4049: { // pflush
		unhandled("pflush");
		break;
		}
	case 4050: { // pflushun
		unhandled("pflushun");
		break;
		}
	case 4051: { // pfree
		unhandled("pfree");
		break;
		}
	case 4052: { // plock (r)-n
		unhandled("plock (r)-n");
		break;
		}
	case 4053: { // plock (r)+n
		unhandled("plock (r)+n");
		break;
		}
	case 4054: { // plock (r)-
		unhandled("plock (r)-");
		break;
		}
	case 4055: { // plock (r)+
		unhandled("plock (r)+");
		break;
		}
	case 4056: { // plock (r)
		unhandled("plock (r)");
		break;
		}
	case 4057: { // plock (r+n)
		unhandled("plock (r+n)");
		break;
		}
	case 4058: { // plock -(r)
		unhandled("plock -(r)");
		break;
		}
	case 4059: { // plock [abs]
		unhandled("plock [abs]");
		break;
		}
	case 4060: { // plockr [x]
		unhandled("plockr [x]");
		break;
		}
	case 4061: { // punlock (r)-n
		unhandled("punlock (r)-n");
		break;
		}
	case 4062: { // punlock (r)+n
		unhandled("punlock (r)+n");
		break;
		}
	case 4063: { // punlock (r)-
		unhandled("punlock (r)-");
		break;
		}
	case 4064: { // punlock (r)+
		unhandled("punlock (r)+");
		break;
		}
	case 4065: { // punlock (r)
		unhandled("punlock (r)");
		break;
		}
	case 4066: { // punlock (r+n)
		unhandled("punlock (r+n)");
		break;
		}
	case 4067: { // punlock -(r)
		unhandled("punlock -(r)");
		break;
		}
	case 4068: { // punlock [abs]
		unhandled("punlock [abs]");
		break;
		}
	case 4069: { // punlockr [x]
		unhandled("punlockr [x]");
		break;
		}
	case 4070: { // rep x:(r)-n
		unhandled("rep x:(r)-n");
		break;
		}
	case 4071: { // rep y:(r)-n
		unhandled("rep y:(r)-n");
		break;
		}
	case 4072: { // rep x:(r)+n
		unhandled("rep x:(r)+n");
		break;
		}
	case 4073: { // rep y:(r)+n
		unhandled("rep y:(r)+n");
		break;
		}
	case 4074: { // rep x:(r)-
		unhandled("rep x:(r)-");
		break;
		}
	case 4075: { // rep y:(r)-
		unhandled("rep y:(r)-");
		break;
		}
	case 4076: { // rep x:(r)+
		unhandled("rep x:(r)+");
		break;
		}
	case 4077: { // rep y:(r)+
		unhandled("rep y:(r)+");
		break;
		}
	case 4078: { // rep x:(r)
		unhandled("rep x:(r)");
		break;
		}
	case 4079: { // rep y:(r)
		unhandled("rep y:(r)");
		break;
		}
	case 4080: { // rep x:(r+n)
		unhandled("rep x:(r+n)");
		break;
		}
	case 4081: { // rep y:(r+n)
		unhandled("rep y:(r+n)");
		break;
		}
	case 4082: { // rep x:-(r)
		unhandled("rep x:-(r)");
		break;
		}
	case 4083: { // rep y:-(r)
		unhandled("rep y:-(r)");
		break;
		}
	case 4084: { // rep x:[aa]
		unhandled("rep x:[aa]");
		break;
		}
	case 4085: { // rep y:[aa]
		unhandled("rep y:[aa]");
		break;
		}
	case 4086: { // rep #[i]
		unhandled("rep #[i]");
		break;
		}
	case 4087: { // rep x0
		unhandled("rep x0");
		break;
		}
	case 4088: { // rep x1
		unhandled("rep x1");
		break;
		}
	case 4089: { // rep y0
		unhandled("rep y0");
		break;
		}
	case 4090: { // rep y1
		unhandled("rep y1");
		break;
		}
	case 4091: { // rep a0
		unhandled("rep a0");
		break;
		}
	case 4092: { // rep b0
		unhandled("rep b0");
		break;
		}
	case 4093: { // rep a2
		unhandled("rep a2");
		break;
		}
	case 4094: { // rep b2
		unhandled("rep b2");
		break;
		}
	case 4095: { // rep a1
		unhandled("rep a1");
		break;
		}
	case 4096: { // rep b1
		unhandled("rep b1");
		break;
		}
	case 4097: { // rep a
		unhandled("rep a");
		break;
		}
	case 4098: { // rep b
		unhandled("rep b");
		break;
		}
	case 4099: { // rep r
		unhandled("rep r");
		break;
		}
	case 4100: { // rep n
		unhandled("rep n");
		break;
		}
	case 4101: { // rep m
		unhandled("rep m");
		break;
		}
	case 4102: { // rep ep
		unhandled("rep ep");
		break;
		}
	case 4103: { // rep vba
		unhandled("rep vba");
		break;
		}
	case 4104: { // rep sc
		unhandled("rep sc");
		break;
		}
	case 4105: { // rep sz
		unhandled("rep sz");
		break;
		}
	case 4106: { // rep sr
		unhandled("rep sr");
		break;
		}
	case 4107: { // rep omr
		unhandled("rep omr");
		break;
		}
	case 4108: { // rep sp
		unhandled("rep sp");
		break;
		}
	case 4109: { // rep ssh
		unhandled("rep ssh");
		break;
		}
	case 4110: { // rep ssl
		unhandled("rep ssl");
		break;
		}
	case 4111: { // rep la
		unhandled("rep la");
		break;
		}
	case 4112: { // rep lc
		unhandled("rep lc");
		break;
		}
	case 4113: { // reset
		unhandled("reset");
		break;
		}
	case 4114: { // rti
		unhandled("rti");
		break;
		}
	case 4115: { // rts
		unhandled("rts");
		break;
		}
	case 4116: { // stop
		unhandled("stop");
		break;
		}
	case 4117: { // sub #[i],a
		unhandled("sub #[i],a");
		break;
		}
	case 4118: { // sub #[i],b
		unhandled("sub #[i],b");
		break;
		}
	case 4119: { // sub #[i],a
		unhandled("sub #[i],a");
		break;
		}
	case 4120: { // sub #[i],b
		unhandled("sub #[i],b");
		break;
		}
	case 4121: { // tcc b,a
		unhandled("tcc b,a");
		break;
		}
	case 4122: { // tcc b,b
		unhandled("tcc b,b");
		break;
		}
	case 4123: { // tcc x0,a
		unhandled("tcc x0,a");
		break;
		}
	case 4124: { // tcc x0,b
		unhandled("tcc x0,b");
		break;
		}
	case 4125: { // tcc y0,a
		unhandled("tcc y0,a");
		break;
		}
	case 4126: { // tcc y0,b
		unhandled("tcc y0,b");
		break;
		}
	case 4127: { // tcc x1,a
		unhandled("tcc x1,a");
		break;
		}
	case 4128: { // tcc x1,b
		unhandled("tcc x1,b");
		break;
		}
	case 4129: { // tcc y1,a
		unhandled("tcc y1,a");
		break;
		}
	case 4130: { // tcc y1,b
		unhandled("tcc y1,b");
		break;
		}
	case 4131: { // tge b,a
		unhandled("tge b,a");
		break;
		}
	case 4132: { // tge b,b
		unhandled("tge b,b");
		break;
		}
	case 4133: { // tge x0,a
		unhandled("tge x0,a");
		break;
		}
	case 4134: { // tge x0,b
		unhandled("tge x0,b");
		break;
		}
	case 4135: { // tge y0,a
		unhandled("tge y0,a");
		break;
		}
	case 4136: { // tge y0,b
		unhandled("tge y0,b");
		break;
		}
	case 4137: { // tge x1,a
		unhandled("tge x1,a");
		break;
		}
	case 4138: { // tge x1,b
		unhandled("tge x1,b");
		break;
		}
	case 4139: { // tge y1,a
		unhandled("tge y1,a");
		break;
		}
	case 4140: { // tge y1,b
		unhandled("tge y1,b");
		break;
		}
	case 4141: { // tne b,a
		unhandled("tne b,a");
		break;
		}
	case 4142: { // tne b,b
		unhandled("tne b,b");
		break;
		}
	case 4143: { // tne x0,a
		unhandled("tne x0,a");
		break;
		}
	case 4144: { // tne x0,b
		unhandled("tne x0,b");
		break;
		}
	case 4145: { // tne y0,a
		unhandled("tne y0,a");
		break;
		}
	case 4146: { // tne y0,b
		unhandled("tne y0,b");
		break;
		}
	case 4147: { // tne x1,a
		unhandled("tne x1,a");
		break;
		}
	case 4148: { // tne x1,b
		unhandled("tne x1,b");
		break;
		}
	case 4149: { // tne y1,a
		unhandled("tne y1,a");
		break;
		}
	case 4150: { // tne y1,b
		unhandled("tne y1,b");
		break;
		}
	case 4151: { // tpl b,a
		unhandled("tpl b,a");
		break;
		}
	case 4152: { // tpl b,b
		unhandled("tpl b,b");
		break;
		}
	case 4153: { // tpl x0,a
		unhandled("tpl x0,a");
		break;
		}
	case 4154: { // tpl x0,b
		unhandled("tpl x0,b");
		break;
		}
	case 4155: { // tpl y0,a
		unhandled("tpl y0,a");
		break;
		}
	case 4156: { // tpl y0,b
		unhandled("tpl y0,b");
		break;
		}
	case 4157: { // tpl x1,a
		unhandled("tpl x1,a");
		break;
		}
	case 4158: { // tpl x1,b
		unhandled("tpl x1,b");
		break;
		}
	case 4159: { // tpl y1,a
		unhandled("tpl y1,a");
		break;
		}
	case 4160: { // tpl y1,b
		unhandled("tpl y1,b");
		break;
		}
	case 4161: { // tnn b,a
		unhandled("tnn b,a");
		break;
		}
	case 4162: { // tnn b,b
		unhandled("tnn b,b");
		break;
		}
	case 4163: { // tnn x0,a
		unhandled("tnn x0,a");
		break;
		}
	case 4164: { // tnn x0,b
		unhandled("tnn x0,b");
		break;
		}
	case 4165: { // tnn y0,a
		unhandled("tnn y0,a");
		break;
		}
	case 4166: { // tnn y0,b
		unhandled("tnn y0,b");
		break;
		}
	case 4167: { // tnn x1,a
		unhandled("tnn x1,a");
		break;
		}
	case 4168: { // tnn x1,b
		unhandled("tnn x1,b");
		break;
		}
	case 4169: { // tnn y1,a
		unhandled("tnn y1,a");
		break;
		}
	case 4170: { // tnn y1,b
		unhandled("tnn y1,b");
		break;
		}
	case 4171: { // tec b,a
		unhandled("tec b,a");
		break;
		}
	case 4172: { // tec b,b
		unhandled("tec b,b");
		break;
		}
	case 4173: { // tec x0,a
		unhandled("tec x0,a");
		break;
		}
	case 4174: { // tec x0,b
		unhandled("tec x0,b");
		break;
		}
	case 4175: { // tec y0,a
		unhandled("tec y0,a");
		break;
		}
	case 4176: { // tec y0,b
		unhandled("tec y0,b");
		break;
		}
	case 4177: { // tec x1,a
		unhandled("tec x1,a");
		break;
		}
	case 4178: { // tec x1,b
		unhandled("tec x1,b");
		break;
		}
	case 4179: { // tec y1,a
		unhandled("tec y1,a");
		break;
		}
	case 4180: { // tec y1,b
		unhandled("tec y1,b");
		break;
		}
	case 4181: { // tlc b,a
		unhandled("tlc b,a");
		break;
		}
	case 4182: { // tlc b,b
		unhandled("tlc b,b");
		break;
		}
	case 4183: { // tlc x0,a
		unhandled("tlc x0,a");
		break;
		}
	case 4184: { // tlc x0,b
		unhandled("tlc x0,b");
		break;
		}
	case 4185: { // tlc y0,a
		unhandled("tlc y0,a");
		break;
		}
	case 4186: { // tlc y0,b
		unhandled("tlc y0,b");
		break;
		}
	case 4187: { // tlc x1,a
		unhandled("tlc x1,a");
		break;
		}
	case 4188: { // tlc x1,b
		unhandled("tlc x1,b");
		break;
		}
	case 4189: { // tlc y1,a
		unhandled("tlc y1,a");
		break;
		}
	case 4190: { // tlc y1,b
		unhandled("tlc y1,b");
		break;
		}
	case 4191: { // tgt b,a
		unhandled("tgt b,a");
		break;
		}
	case 4192: { // tgt b,b
		unhandled("tgt b,b");
		break;
		}
	case 4193: { // tgt x0,a
		unhandled("tgt x0,a");
		break;
		}
	case 4194: { // tgt x0,b
		unhandled("tgt x0,b");
		break;
		}
	case 4195: { // tgt y0,a
		unhandled("tgt y0,a");
		break;
		}
	case 4196: { // tgt y0,b
		unhandled("tgt y0,b");
		break;
		}
	case 4197: { // tgt x1,a
		unhandled("tgt x1,a");
		break;
		}
	case 4198: { // tgt x1,b
		unhandled("tgt x1,b");
		break;
		}
	case 4199: { // tgt y1,a
		unhandled("tgt y1,a");
		break;
		}
	case 4200: { // tgt y1,b
		unhandled("tgt y1,b");
		break;
		}
	case 4201: { // tcs b,a
		unhandled("tcs b,a");
		break;
		}
	case 4202: { // tcs b,b
		unhandled("tcs b,b");
		break;
		}
	case 4203: { // tcs x0,a
		unhandled("tcs x0,a");
		break;
		}
	case 4204: { // tcs x0,b
		unhandled("tcs x0,b");
		break;
		}
	case 4205: { // tcs y0,a
		unhandled("tcs y0,a");
		break;
		}
	case 4206: { // tcs y0,b
		unhandled("tcs y0,b");
		break;
		}
	case 4207: { // tcs x1,a
		unhandled("tcs x1,a");
		break;
		}
	case 4208: { // tcs x1,b
		unhandled("tcs x1,b");
		break;
		}
	case 4209: { // tcs y1,a
		unhandled("tcs y1,a");
		break;
		}
	case 4210: { // tcs y1,b
		unhandled("tcs y1,b");
		break;
		}
	case 4211: { // tlt b,a
		unhandled("tlt b,a");
		break;
		}
	case 4212: { // tlt b,b
		unhandled("tlt b,b");
		break;
		}
	case 4213: { // tlt x0,a
		unhandled("tlt x0,a");
		break;
		}
	case 4214: { // tlt x0,b
		unhandled("tlt x0,b");
		break;
		}
	case 4215: { // tlt y0,a
		unhandled("tlt y0,a");
		break;
		}
	case 4216: { // tlt y0,b
		unhandled("tlt y0,b");
		break;
		}
	case 4217: { // tlt x1,a
		unhandled("tlt x1,a");
		break;
		}
	case 4218: { // tlt x1,b
		unhandled("tlt x1,b");
		break;
		}
	case 4219: { // tlt y1,a
		unhandled("tlt y1,a");
		break;
		}
	case 4220: { // tlt y1,b
		unhandled("tlt y1,b");
		break;
		}
	case 4221: { // teq b,a
		unhandled("teq b,a");
		break;
		}
	case 4222: { // teq b,b
		unhandled("teq b,b");
		break;
		}
	case 4223: { // teq x0,a
		unhandled("teq x0,a");
		break;
		}
	case 4224: { // teq x0,b
		unhandled("teq x0,b");
		break;
		}
	case 4225: { // teq y0,a
		unhandled("teq y0,a");
		break;
		}
	case 4226: { // teq y0,b
		unhandled("teq y0,b");
		break;
		}
	case 4227: { // teq x1,a
		unhandled("teq x1,a");
		break;
		}
	case 4228: { // teq x1,b
		unhandled("teq x1,b");
		break;
		}
	case 4229: { // teq y1,a
		unhandled("teq y1,a");
		break;
		}
	case 4230: { // teq y1,b
		unhandled("teq y1,b");
		break;
		}
	case 4231: { // tmi b,a
		unhandled("tmi b,a");
		break;
		}
	case 4232: { // tmi b,b
		unhandled("tmi b,b");
		break;
		}
	case 4233: { // tmi x0,a
		unhandled("tmi x0,a");
		break;
		}
	case 4234: { // tmi x0,b
		unhandled("tmi x0,b");
		break;
		}
	case 4235: { // tmi y0,a
		unhandled("tmi y0,a");
		break;
		}
	case 4236: { // tmi y0,b
		unhandled("tmi y0,b");
		break;
		}
	case 4237: { // tmi x1,a
		unhandled("tmi x1,a");
		break;
		}
	case 4238: { // tmi x1,b
		unhandled("tmi x1,b");
		break;
		}
	case 4239: { // tmi y1,a
		unhandled("tmi y1,a");
		break;
		}
	case 4240: { // tmi y1,b
		unhandled("tmi y1,b");
		break;
		}
	case 4241: { // tnr b,a
		unhandled("tnr b,a");
		break;
		}
	case 4242: { // tnr b,b
		unhandled("tnr b,b");
		break;
		}
	case 4243: { // tnr x0,a
		unhandled("tnr x0,a");
		break;
		}
	case 4244: { // tnr x0,b
		unhandled("tnr x0,b");
		break;
		}
	case 4245: { // tnr y0,a
		unhandled("tnr y0,a");
		break;
		}
	case 4246: { // tnr y0,b
		unhandled("tnr y0,b");
		break;
		}
	case 4247: { // tnr x1,a
		unhandled("tnr x1,a");
		break;
		}
	case 4248: { // tnr x1,b
		unhandled("tnr x1,b");
		break;
		}
	case 4249: { // tnr y1,a
		unhandled("tnr y1,a");
		break;
		}
	case 4250: { // tnr y1,b
		unhandled("tnr y1,b");
		break;
		}
	case 4251: { // tes b,a
		unhandled("tes b,a");
		break;
		}
	case 4252: { // tes b,b
		unhandled("tes b,b");
		break;
		}
	case 4253: { // tes x0,a
		unhandled("tes x0,a");
		break;
		}
	case 4254: { // tes x0,b
		unhandled("tes x0,b");
		break;
		}
	case 4255: { // tes y0,a
		unhandled("tes y0,a");
		break;
		}
	case 4256: { // tes y0,b
		unhandled("tes y0,b");
		break;
		}
	case 4257: { // tes x1,a
		unhandled("tes x1,a");
		break;
		}
	case 4258: { // tes x1,b
		unhandled("tes x1,b");
		break;
		}
	case 4259: { // tes y1,a
		unhandled("tes y1,a");
		break;
		}
	case 4260: { // tes y1,b
		unhandled("tes y1,b");
		break;
		}
	case 4261: { // tls b,a
		unhandled("tls b,a");
		break;
		}
	case 4262: { // tls b,b
		unhandled("tls b,b");
		break;
		}
	case 4263: { // tls x0,a
		unhandled("tls x0,a");
		break;
		}
	case 4264: { // tls x0,b
		unhandled("tls x0,b");
		break;
		}
	case 4265: { // tls y0,a
		unhandled("tls y0,a");
		break;
		}
	case 4266: { // tls y0,b
		unhandled("tls y0,b");
		break;
		}
	case 4267: { // tls x1,a
		unhandled("tls x1,a");
		break;
		}
	case 4268: { // tls x1,b
		unhandled("tls x1,b");
		break;
		}
	case 4269: { // tls y1,a
		unhandled("tls y1,a");
		break;
		}
	case 4270: { // tls y1,b
		unhandled("tls y1,b");
		break;
		}
	case 4271: { // tle b,a
		unhandled("tle b,a");
		break;
		}
	case 4272: { // tle b,b
		unhandled("tle b,b");
		break;
		}
	case 4273: { // tle x0,a
		unhandled("tle x0,a");
		break;
		}
	case 4274: { // tle x0,b
		unhandled("tle x0,b");
		break;
		}
	case 4275: { // tle y0,a
		unhandled("tle y0,a");
		break;
		}
	case 4276: { // tle y0,b
		unhandled("tle y0,b");
		break;
		}
	case 4277: { // tle x1,a
		unhandled("tle x1,a");
		break;
		}
	case 4278: { // tle x1,b
		unhandled("tle x1,b");
		break;
		}
	case 4279: { // tle y1,a
		unhandled("tle y1,a");
		break;
		}
	case 4280: { // tle y1,b
		unhandled("tle y1,b");
		break;
		}
	case 4281: { // tcc b,a r,r
		unhandled("tcc b,a r,r");
		break;
		}
	case 4282: { // tcc b,b r,r
		unhandled("tcc b,b r,r");
		break;
		}
	case 4283: { // tcc x0,a r,r
		unhandled("tcc x0,a r,r");
		break;
		}
	case 4284: { // tcc x0,b r,r
		unhandled("tcc x0,b r,r");
		break;
		}
	case 4285: { // tcc y0,a r,r
		unhandled("tcc y0,a r,r");
		break;
		}
	case 4286: { // tcc y0,b r,r
		unhandled("tcc y0,b r,r");
		break;
		}
	case 4287: { // tcc x1,a r,r
		unhandled("tcc x1,a r,r");
		break;
		}
	case 4288: { // tcc x1,b r,r
		unhandled("tcc x1,b r,r");
		break;
		}
	case 4289: { // tcc y1,a r,r
		unhandled("tcc y1,a r,r");
		break;
		}
	case 4290: { // tcc y1,b r,r
		unhandled("tcc y1,b r,r");
		break;
		}
	case 4291: { // tge b,a r,r
		unhandled("tge b,a r,r");
		break;
		}
	case 4292: { // tge b,b r,r
		unhandled("tge b,b r,r");
		break;
		}
	case 4293: { // tge x0,a r,r
		unhandled("tge x0,a r,r");
		break;
		}
	case 4294: { // tge x0,b r,r
		unhandled("tge x0,b r,r");
		break;
		}
	case 4295: { // tge y0,a r,r
		unhandled("tge y0,a r,r");
		break;
		}
	case 4296: { // tge y0,b r,r
		unhandled("tge y0,b r,r");
		break;
		}
	case 4297: { // tge x1,a r,r
		unhandled("tge x1,a r,r");
		break;
		}
	case 4298: { // tge x1,b r,r
		unhandled("tge x1,b r,r");
		break;
		}
	case 4299: { // tge y1,a r,r
		unhandled("tge y1,a r,r");
		break;
		}
	case 4300: { // tge y1,b r,r
		unhandled("tge y1,b r,r");
		break;
		}
	case 4301: { // tne b,a r,r
		unhandled("tne b,a r,r");
		break;
		}
	case 4302: { // tne b,b r,r
		unhandled("tne b,b r,r");
		break;
		}
	case 4303: { // tne x0,a r,r
		unhandled("tne x0,a r,r");
		break;
		}
	case 4304: { // tne x0,b r,r
		unhandled("tne x0,b r,r");
		break;
		}
	case 4305: { // tne y0,a r,r
		unhandled("tne y0,a r,r");
		break;
		}
	case 4306: { // tne y0,b r,r
		unhandled("tne y0,b r,r");
		break;
		}
	case 4307: { // tne x1,a r,r
		unhandled("tne x1,a r,r");
		break;
		}
	case 4308: { // tne x1,b r,r
		unhandled("tne x1,b r,r");
		break;
		}
	case 4309: { // tne y1,a r,r
		unhandled("tne y1,a r,r");
		break;
		}
	case 4310: { // tne y1,b r,r
		unhandled("tne y1,b r,r");
		break;
		}
	case 4311: { // tpl b,a r,r
		unhandled("tpl b,a r,r");
		break;
		}
	case 4312: { // tpl b,b r,r
		unhandled("tpl b,b r,r");
		break;
		}
	case 4313: { // tpl x0,a r,r
		unhandled("tpl x0,a r,r");
		break;
		}
	case 4314: { // tpl x0,b r,r
		unhandled("tpl x0,b r,r");
		break;
		}
	case 4315: { // tpl y0,a r,r
		unhandled("tpl y0,a r,r");
		break;
		}
	case 4316: { // tpl y0,b r,r
		unhandled("tpl y0,b r,r");
		break;
		}
	case 4317: { // tpl x1,a r,r
		unhandled("tpl x1,a r,r");
		break;
		}
	case 4318: { // tpl x1,b r,r
		unhandled("tpl x1,b r,r");
		break;
		}
	case 4319: { // tpl y1,a r,r
		unhandled("tpl y1,a r,r");
		break;
		}
	case 4320: { // tpl y1,b r,r
		unhandled("tpl y1,b r,r");
		break;
		}
	case 4321: { // tnn b,a r,r
		unhandled("tnn b,a r,r");
		break;
		}
	case 4322: { // tnn b,b r,r
		unhandled("tnn b,b r,r");
		break;
		}
	case 4323: { // tnn x0,a r,r
		unhandled("tnn x0,a r,r");
		break;
		}
	case 4324: { // tnn x0,b r,r
		unhandled("tnn x0,b r,r");
		break;
		}
	case 4325: { // tnn y0,a r,r
		unhandled("tnn y0,a r,r");
		break;
		}
	case 4326: { // tnn y0,b r,r
		unhandled("tnn y0,b r,r");
		break;
		}
	case 4327: { // tnn x1,a r,r
		unhandled("tnn x1,a r,r");
		break;
		}
	case 4328: { // tnn x1,b r,r
		unhandled("tnn x1,b r,r");
		break;
		}
	case 4329: { // tnn y1,a r,r
		unhandled("tnn y1,a r,r");
		break;
		}
	case 4330: { // tnn y1,b r,r
		unhandled("tnn y1,b r,r");
		break;
		}
	case 4331: { // tec b,a r,r
		unhandled("tec b,a r,r");
		break;
		}
	case 4332: { // tec b,b r,r
		unhandled("tec b,b r,r");
		break;
		}
	case 4333: { // tec x0,a r,r
		unhandled("tec x0,a r,r");
		break;
		}
	case 4334: { // tec x0,b r,r
		unhandled("tec x0,b r,r");
		break;
		}
	case 4335: { // tec y0,a r,r
		unhandled("tec y0,a r,r");
		break;
		}
	case 4336: { // tec y0,b r,r
		unhandled("tec y0,b r,r");
		break;
		}
	case 4337: { // tec x1,a r,r
		unhandled("tec x1,a r,r");
		break;
		}
	case 4338: { // tec x1,b r,r
		unhandled("tec x1,b r,r");
		break;
		}
	case 4339: { // tec y1,a r,r
		unhandled("tec y1,a r,r");
		break;
		}
	case 4340: { // tec y1,b r,r
		unhandled("tec y1,b r,r");
		break;
		}
	case 4341: { // tlc b,a r,r
		unhandled("tlc b,a r,r");
		break;
		}
	case 4342: { // tlc b,b r,r
		unhandled("tlc b,b r,r");
		break;
		}
	case 4343: { // tlc x0,a r,r
		unhandled("tlc x0,a r,r");
		break;
		}
	case 4344: { // tlc x0,b r,r
		unhandled("tlc x0,b r,r");
		break;
		}
	case 4345: { // tlc y0,a r,r
		unhandled("tlc y0,a r,r");
		break;
		}
	case 4346: { // tlc y0,b r,r
		unhandled("tlc y0,b r,r");
		break;
		}
	case 4347: { // tlc x1,a r,r
		unhandled("tlc x1,a r,r");
		break;
		}
	case 4348: { // tlc x1,b r,r
		unhandled("tlc x1,b r,r");
		break;
		}
	case 4349: { // tlc y1,a r,r
		unhandled("tlc y1,a r,r");
		break;
		}
	case 4350: { // tlc y1,b r,r
		unhandled("tlc y1,b r,r");
		break;
		}
	case 4351: { // tgt b,a r,r
		unhandled("tgt b,a r,r");
		break;
		}
	case 4352: { // tgt b,b r,r
		unhandled("tgt b,b r,r");
		break;
		}
	case 4353: { // tgt x0,a r,r
		unhandled("tgt x0,a r,r");
		break;
		}
	case 4354: { // tgt x0,b r,r
		unhandled("tgt x0,b r,r");
		break;
		}
	case 4355: { // tgt y0,a r,r
		unhandled("tgt y0,a r,r");
		break;
		}
	case 4356: { // tgt y0,b r,r
		unhandled("tgt y0,b r,r");
		break;
		}
	case 4357: { // tgt x1,a r,r
		unhandled("tgt x1,a r,r");
		break;
		}
	case 4358: { // tgt x1,b r,r
		unhandled("tgt x1,b r,r");
		break;
		}
	case 4359: { // tgt y1,a r,r
		unhandled("tgt y1,a r,r");
		break;
		}
	case 4360: { // tgt y1,b r,r
		unhandled("tgt y1,b r,r");
		break;
		}
	case 4361: { // tcs b,a r,r
		unhandled("tcs b,a r,r");
		break;
		}
	case 4362: { // tcs b,b r,r
		unhandled("tcs b,b r,r");
		break;
		}
	case 4363: { // tcs x0,a r,r
		unhandled("tcs x0,a r,r");
		break;
		}
	case 4364: { // tcs x0,b r,r
		unhandled("tcs x0,b r,r");
		break;
		}
	case 4365: { // tcs y0,a r,r
		unhandled("tcs y0,a r,r");
		break;
		}
	case 4366: { // tcs y0,b r,r
		unhandled("tcs y0,b r,r");
		break;
		}
	case 4367: { // tcs x1,a r,r
		unhandled("tcs x1,a r,r");
		break;
		}
	case 4368: { // tcs x1,b r,r
		unhandled("tcs x1,b r,r");
		break;
		}
	case 4369: { // tcs y1,a r,r
		unhandled("tcs y1,a r,r");
		break;
		}
	case 4370: { // tcs y1,b r,r
		unhandled("tcs y1,b r,r");
		break;
		}
	case 4371: { // tlt b,a r,r
		unhandled("tlt b,a r,r");
		break;
		}
	case 4372: { // tlt b,b r,r
		unhandled("tlt b,b r,r");
		break;
		}
	case 4373: { // tlt x0,a r,r
		unhandled("tlt x0,a r,r");
		break;
		}
	case 4374: { // tlt x0,b r,r
		unhandled("tlt x0,b r,r");
		break;
		}
	case 4375: { // tlt y0,a r,r
		unhandled("tlt y0,a r,r");
		break;
		}
	case 4376: { // tlt y0,b r,r
		unhandled("tlt y0,b r,r");
		break;
		}
	case 4377: { // tlt x1,a r,r
		unhandled("tlt x1,a r,r");
		break;
		}
	case 4378: { // tlt x1,b r,r
		unhandled("tlt x1,b r,r");
		break;
		}
	case 4379: { // tlt y1,a r,r
		unhandled("tlt y1,a r,r");
		break;
		}
	case 4380: { // tlt y1,b r,r
		unhandled("tlt y1,b r,r");
		break;
		}
	case 4381: { // teq b,a r,r
		unhandled("teq b,a r,r");
		break;
		}
	case 4382: { // teq b,b r,r
		unhandled("teq b,b r,r");
		break;
		}
	case 4383: { // teq x0,a r,r
		unhandled("teq x0,a r,r");
		break;
		}
	case 4384: { // teq x0,b r,r
		unhandled("teq x0,b r,r");
		break;
		}
	case 4385: { // teq y0,a r,r
		unhandled("teq y0,a r,r");
		break;
		}
	case 4386: { // teq y0,b r,r
		unhandled("teq y0,b r,r");
		break;
		}
	case 4387: { // teq x1,a r,r
		unhandled("teq x1,a r,r");
		break;
		}
	case 4388: { // teq x1,b r,r
		unhandled("teq x1,b r,r");
		break;
		}
	case 4389: { // teq y1,a r,r
		unhandled("teq y1,a r,r");
		break;
		}
	case 4390: { // teq y1,b r,r
		unhandled("teq y1,b r,r");
		break;
		}
	case 4391: { // tmi b,a r,r
		unhandled("tmi b,a r,r");
		break;
		}
	case 4392: { // tmi b,b r,r
		unhandled("tmi b,b r,r");
		break;
		}
	case 4393: { // tmi x0,a r,r
		unhandled("tmi x0,a r,r");
		break;
		}
	case 4394: { // tmi x0,b r,r
		unhandled("tmi x0,b r,r");
		break;
		}
	case 4395: { // tmi y0,a r,r
		unhandled("tmi y0,a r,r");
		break;
		}
	case 4396: { // tmi y0,b r,r
		unhandled("tmi y0,b r,r");
		break;
		}
	case 4397: { // tmi x1,a r,r
		unhandled("tmi x1,a r,r");
		break;
		}
	case 4398: { // tmi x1,b r,r
		unhandled("tmi x1,b r,r");
		break;
		}
	case 4399: { // tmi y1,a r,r
		unhandled("tmi y1,a r,r");
		break;
		}
	case 4400: { // tmi y1,b r,r
		unhandled("tmi y1,b r,r");
		break;
		}
	case 4401: { // tnr b,a r,r
		unhandled("tnr b,a r,r");
		break;
		}
	case 4402: { // tnr b,b r,r
		unhandled("tnr b,b r,r");
		break;
		}
	case 4403: { // tnr x0,a r,r
		unhandled("tnr x0,a r,r");
		break;
		}
	case 4404: { // tnr x0,b r,r
		unhandled("tnr x0,b r,r");
		break;
		}
	case 4405: { // tnr y0,a r,r
		unhandled("tnr y0,a r,r");
		break;
		}
	case 4406: { // tnr y0,b r,r
		unhandled("tnr y0,b r,r");
		break;
		}
	case 4407: { // tnr x1,a r,r
		unhandled("tnr x1,a r,r");
		break;
		}
	case 4408: { // tnr x1,b r,r
		unhandled("tnr x1,b r,r");
		break;
		}
	case 4409: { // tnr y1,a r,r
		unhandled("tnr y1,a r,r");
		break;
		}
	case 4410: { // tnr y1,b r,r
		unhandled("tnr y1,b r,r");
		break;
		}
	case 4411: { // tes b,a r,r
		unhandled("tes b,a r,r");
		break;
		}
	case 4412: { // tes b,b r,r
		unhandled("tes b,b r,r");
		break;
		}
	case 4413: { // tes x0,a r,r
		unhandled("tes x0,a r,r");
		break;
		}
	case 4414: { // tes x0,b r,r
		unhandled("tes x0,b r,r");
		break;
		}
	case 4415: { // tes y0,a r,r
		unhandled("tes y0,a r,r");
		break;
		}
	case 4416: { // tes y0,b r,r
		unhandled("tes y0,b r,r");
		break;
		}
	case 4417: { // tes x1,a r,r
		unhandled("tes x1,a r,r");
		break;
		}
	case 4418: { // tes x1,b r,r
		unhandled("tes x1,b r,r");
		break;
		}
	case 4419: { // tes y1,a r,r
		unhandled("tes y1,a r,r");
		break;
		}
	case 4420: { // tes y1,b r,r
		unhandled("tes y1,b r,r");
		break;
		}
	case 4421: { // tls b,a r,r
		unhandled("tls b,a r,r");
		break;
		}
	case 4422: { // tls b,b r,r
		unhandled("tls b,b r,r");
		break;
		}
	case 4423: { // tls x0,a r,r
		unhandled("tls x0,a r,r");
		break;
		}
	case 4424: { // tls x0,b r,r
		unhandled("tls x0,b r,r");
		break;
		}
	case 4425: { // tls y0,a r,r
		unhandled("tls y0,a r,r");
		break;
		}
	case 4426: { // tls y0,b r,r
		unhandled("tls y0,b r,r");
		break;
		}
	case 4427: { // tls x1,a r,r
		unhandled("tls x1,a r,r");
		break;
		}
	case 4428: { // tls x1,b r,r
		unhandled("tls x1,b r,r");
		break;
		}
	case 4429: { // tls y1,a r,r
		unhandled("tls y1,a r,r");
		break;
		}
	case 4430: { // tls y1,b r,r
		unhandled("tls y1,b r,r");
		break;
		}
	case 4431: { // tle b,a r,r
		unhandled("tle b,a r,r");
		break;
		}
	case 4432: { // tle b,b r,r
		unhandled("tle b,b r,r");
		break;
		}
	case 4433: { // tle x0,a r,r
		unhandled("tle x0,a r,r");
		break;
		}
	case 4434: { // tle x0,b r,r
		unhandled("tle x0,b r,r");
		break;
		}
	case 4435: { // tle y0,a r,r
		unhandled("tle y0,a r,r");
		break;
		}
	case 4436: { // tle y0,b r,r
		unhandled("tle y0,b r,r");
		break;
		}
	case 4437: { // tle x1,a r,r
		unhandled("tle x1,a r,r");
		break;
		}
	case 4438: { // tle x1,b r,r
		unhandled("tle x1,b r,r");
		break;
		}
	case 4439: { // tle y1,a r,r
		unhandled("tle y1,a r,r");
		break;
		}
	case 4440: { // tle y1,b r,r
		unhandled("tle y1,b r,r");
		break;
		}
	case 4441: { // tcc r,r
		unhandled("tcc r,r");
		break;
		}
	case 4442: { // tge r,r
		unhandled("tge r,r");
		break;
		}
	case 4443: { // tne r,r
		unhandled("tne r,r");
		break;
		}
	case 4444: { // tpl r,r
		unhandled("tpl r,r");
		break;
		}
	case 4445: { // tnn r,r
		unhandled("tnn r,r");
		break;
		}
	case 4446: { // tec r,r
		unhandled("tec r,r");
		break;
		}
	case 4447: { // tlc r,r
		unhandled("tlc r,r");
		break;
		}
	case 4448: { // tgt r,r
		unhandled("tgt r,r");
		break;
		}
	case 4449: { // tcs r,r
		unhandled("tcs r,r");
		break;
		}
	case 4450: { // tlt r,r
		unhandled("tlt r,r");
		break;
		}
	case 4451: { // teq r,r
		unhandled("teq r,r");
		break;
		}
	case 4452: { // tmi r,r
		unhandled("tmi r,r");
		break;
		}
	case 4453: { // tnr r,r
		unhandled("tnr r,r");
		break;
		}
	case 4454: { // tes r,r
		unhandled("tes r,r");
		break;
		}
	case 4455: { // tls r,r
		unhandled("tls r,r");
		break;
		}
	case 4456: { // tle r,r
		unhandled("tle r,r");
		break;
		}
	case 4457: { // trap
		unhandled("trap");
		break;
		}
	case 4458: { // trapcc
		unhandled("trapcc");
		break;
		}
	case 4459: { // trapge
		unhandled("trapge");
		break;
		}
	case 4460: { // trapne
		unhandled("trapne");
		break;
		}
	case 4461: { // trappl
		unhandled("trappl");
		break;
		}
	case 4462: { // trapnn
		unhandled("trapnn");
		break;
		}
	case 4463: { // trapec
		unhandled("trapec");
		break;
		}
	case 4464: { // traplc
		unhandled("traplc");
		break;
		}
	case 4465: { // trapgt
		unhandled("trapgt");
		break;
		}
	case 4466: { // trapcs
		unhandled("trapcs");
		break;
		}
	case 4467: { // traplt
		unhandled("traplt");
		break;
		}
	case 4468: { // trapeq
		unhandled("trapeq");
		break;
		}
	case 4469: { // trapmi
		unhandled("trapmi");
		break;
		}
	case 4470: { // trapnr
		unhandled("trapnr");
		break;
		}
	case 4471: { // trapes
		unhandled("trapes");
		break;
		}
	case 4472: { // trapls
		unhandled("trapls");
		break;
		}
	case 4473: { // traple
		unhandled("traple");
		break;
		}
	case 4474: { // vsl a,[i],l:(r)-n
		unhandled("vsl a,[i],l:(r)-n");
		break;
		}
	case 4475: { // vsl a,[i],l:(r)+n
		unhandled("vsl a,[i],l:(r)+n");
		break;
		}
	case 4476: { // vsl a,[i],l:(r)-
		unhandled("vsl a,[i],l:(r)-");
		break;
		}
	case 4477: { // vsl a,[i],l:(r)+
		unhandled("vsl a,[i],l:(r)+");
		break;
		}
	case 4478: { // vsl a,[i],l:(r)
		unhandled("vsl a,[i],l:(r)");
		break;
		}
	case 4479: { // vsl a,[i],l:(r+n)
		unhandled("vsl a,[i],l:(r+n)");
		break;
		}
	case 4480: { // vsl a,[i],l:-(r)
		unhandled("vsl a,[i],l:-(r)");
		break;
		}
	case 4481: { // vsl b,[i],l:(r)-n
		unhandled("vsl b,[i],l:(r)-n");
		break;
		}
	case 4482: { // vsl b,[i],l:(r)+n
		unhandled("vsl b,[i],l:(r)+n");
		break;
		}
	case 4483: { // vsl b,[i],l:(r)-
		unhandled("vsl b,[i],l:(r)-");
		break;
		}
	case 4484: { // vsl b,[i],l:(r)+
		unhandled("vsl b,[i],l:(r)+");
		break;
		}
	case 4485: { // vsl b,[i],l:(r)
		unhandled("vsl b,[i],l:(r)");
		break;
		}
	case 4486: { // vsl b,[i],l:(r+n)
		unhandled("vsl b,[i],l:(r+n)");
		break;
		}
	case 4487: { // vsl b,[i],l:-(r)
		unhandled("vsl b,[i],l:-(r)");
		break;
		}
	case 4488: { // vsl a,[i],l:[abs]
		unhandled("vsl a,[i],l:[abs]");
		break;
		}
	case 4489: { // vsl b,[i],l:[abs]
		unhandled("vsl b,[i],l:[abs]");
		break;
		}
	case 4490: { // wait
		unhandled("wait");
		break;
		}
	}
}
