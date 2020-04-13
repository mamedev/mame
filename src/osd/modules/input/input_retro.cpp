//============================================================
//
//  input_retro.cpp - retro input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

// MAME headers
#include "emu.h"
#include "uiinput.h"

#include "libretro/libretro-internal/libretro.h"
#include "libretro/libretro-internal/libretro_shared.h"

#include "../lib/osdobj_common.h"

#include "input_common.h"
#include "input_retro.h"
#include "../../libretro/osdretro.h"
#include "../../libretro/window.h"

static bool libretro_supports_bitmasks = false;
uint16_t retrokbd_state[RETROK_LAST];
uint16_t retrokbd_state2[RETROK_LAST];
int mouseLX;
int mouseLY;
int mouseBUT[4];
Joystate joystate[4];

int lightgunX, lightgunY;
int lightgunBUT[4];

#ifndef RETROK_TILDE
#define RETROK_TILDE 178
#endif

kt_table ktable[]={
{"A",RETROK_a,ITEM_ID_A},
{"B",RETROK_b,ITEM_ID_B},
{"C",RETROK_c,ITEM_ID_C},
{"D",RETROK_d,ITEM_ID_D},
{"E",RETROK_e,ITEM_ID_E},
{"F",RETROK_f,ITEM_ID_F},
{"G",RETROK_g,ITEM_ID_G},
{"H",RETROK_h,ITEM_ID_H},
{"I",RETROK_i,ITEM_ID_I},
{"J",RETROK_j,ITEM_ID_J},
{"K",RETROK_k,ITEM_ID_K},
{"L",RETROK_l,ITEM_ID_L},
{"M",RETROK_m,ITEM_ID_M},
{"N",RETROK_n,ITEM_ID_N},
{"O",RETROK_o,ITEM_ID_O},
{"P",RETROK_p,ITEM_ID_P},
{"Q",RETROK_q,ITEM_ID_Q},
{"R",RETROK_r,ITEM_ID_R},
{"S",RETROK_s,ITEM_ID_S},
{"T",RETROK_t,ITEM_ID_T},
{"U",RETROK_u,ITEM_ID_U},
{"V",RETROK_v,ITEM_ID_V},
{"W",RETROK_w,ITEM_ID_W},
{"X",RETROK_x,ITEM_ID_X},
{"Y",RETROK_y,ITEM_ID_Y},
{"Z",RETROK_z,ITEM_ID_Z},
{"0",RETROK_0,ITEM_ID_0},
{"1",RETROK_1,ITEM_ID_1},
{"2",RETROK_2,ITEM_ID_2},
{"3",RETROK_3,ITEM_ID_3},
{"4",RETROK_4,ITEM_ID_4},
{"5",RETROK_5,ITEM_ID_5},
{"6",RETROK_6,ITEM_ID_6},
{"7",RETROK_7,ITEM_ID_7},
{"8",RETROK_8,ITEM_ID_8},
{"9",RETROK_9,ITEM_ID_9},
{"F1",RETROK_F1,ITEM_ID_F1},
{"F2",RETROK_F2,ITEM_ID_F2},
{"F3",RETROK_F3,ITEM_ID_F3},
{"F4",RETROK_F4,ITEM_ID_F4},
{"F5",RETROK_F5,ITEM_ID_F5},
{"F6",RETROK_F6,ITEM_ID_F6},
{"F7",RETROK_F7,ITEM_ID_F7},
{"F8",RETROK_F8,ITEM_ID_F8},
{"F9",RETROK_F9,ITEM_ID_F9},
{"F10",RETROK_F10,ITEM_ID_F10},
{"F11",RETROK_F11,ITEM_ID_F11},
{"F12",RETROK_F12,ITEM_ID_F12},
{"F13",RETROK_F13,ITEM_ID_F13},
{"F14",RETROK_F14,ITEM_ID_F14},
{"F15",RETROK_F15,ITEM_ID_F15},
{"Esc",RETROK_ESCAPE,ITEM_ID_ESC},
{"TILDE",RETROK_TILDE,ITEM_ID_TILDE},
{"MINUS",RETROK_MINUS,ITEM_ID_MINUS},
{"EQUALS",RETROK_EQUALS,ITEM_ID_EQUALS},
{"BKCSPACE",RETROK_BACKSPACE,ITEM_ID_BACKSPACE},
{"TAB",RETROK_TAB,ITEM_ID_TAB},
{"(",RETROK_LEFTPAREN,ITEM_ID_OPENBRACE},
{")",RETROK_RIGHTPAREN,ITEM_ID_CLOSEBRACE},
{"ENTER",RETROK_RETURN,ITEM_ID_ENTER},
{"·",RETROK_COLON,ITEM_ID_COLON},
{"\'",RETROK_QUOTE,ITEM_ID_QUOTE},
{"BCKSLASH",RETROK_BACKSLASH,ITEM_ID_BACKSLASH},
///**/BCKSLASH2*/RETROK_,ITEM_ID_BACKSLASH2},
{",",RETROK_COMMA,ITEM_ID_COMMA},
///**/STOP*/RETROK_,ITEM_ID_STOP},
{"/",RETROK_SLASH,ITEM_ID_SLASH},
{"SPACE",RETROK_SPACE,ITEM_ID_SPACE},
{"INS",RETROK_INSERT,ITEM_ID_INSERT},
{"DEL",RETROK_DELETE,ITEM_ID_DEL},
{"HOME",RETROK_HOME,ITEM_ID_HOME},
{"END",RETROK_END,ITEM_ID_END},
{"PGUP",RETROK_PAGEUP,ITEM_ID_PGUP},
{"PGDW",RETROK_PAGEDOWN,ITEM_ID_PGDN},
{"LEFT",RETROK_LEFT,ITEM_ID_LEFT},
{"RIGHT",RETROK_RIGHT,ITEM_ID_RIGHT},
{"UP",RETROK_UP,ITEM_ID_UP},
{"DOWN",RETROK_DOWN,ITEM_ID_DOWN},
{"KO",RETROK_KP0,ITEM_ID_0_PAD},
{"K1",RETROK_KP1,ITEM_ID_1_PAD},
{"K2",RETROK_KP2,ITEM_ID_2_PAD},
{"K3",RETROK_KP3,ITEM_ID_3_PAD},
{"K4",RETROK_KP4,ITEM_ID_4_PAD},
{"K5",RETROK_KP5,ITEM_ID_5_PAD},
{"K6",RETROK_KP6,ITEM_ID_6_PAD},
{"K7",RETROK_KP7,ITEM_ID_7_PAD},
{"K8",RETROK_KP8,ITEM_ID_8_PAD},
{"K9",RETROK_KP9,ITEM_ID_9_PAD},
{"K/",RETROK_KP_DIVIDE,ITEM_ID_SLASH_PAD},
{"K*",RETROK_KP_MULTIPLY,ITEM_ID_ASTERISK},
{"K-",RETROK_KP_MINUS,ITEM_ID_MINUS_PAD},
{"K+",RETROK_KP_PLUS,ITEM_ID_PLUS_PAD},
{"KDEL",RETROK_KP_PERIOD,ITEM_ID_DEL_PAD},
{"KRTRN",RETROK_KP_ENTER,ITEM_ID_ENTER_PAD},
{"PRINT",RETROK_PRINT,ITEM_ID_PRTSCR},
{"PAUSE",RETROK_PAUSE,ITEM_ID_PAUSE},
{"LSHFT",RETROK_LSHIFT,ITEM_ID_LSHIFT},
{"RSHFT",RETROK_RSHIFT,ITEM_ID_RSHIFT},
{"LCTRL",RETROK_LCTRL,ITEM_ID_LCONTROL},
{"RCTRL",RETROK_RCTRL,ITEM_ID_RCONTROL},
{"LALT",RETROK_LALT,ITEM_ID_LALT},
{"RALT",RETROK_RALT,ITEM_ID_RALT},
{"SCRLOCK",RETROK_SCROLLOCK,ITEM_ID_SCRLOCK},
{"NUMLOCK",RETROK_NUMLOCK,ITEM_ID_NUMLOCK},
{"CPSLOCK",RETROK_CAPSLOCK,ITEM_ID_CAPSLOCK},
{"LMETA",RETROK_LMETA,ITEM_ID_LWIN},
{"RMETA",RETROK_RMETA,ITEM_ID_RWIN},
{"MENU",RETROK_MENU,ITEM_ID_MENU},
{"BREAK",RETROK_BREAK,ITEM_ID_CANCEL},
{"-1",-1,ITEM_ID_INVALID},
};

const char *Buttons_Name[RETRO_MAX_BUTTONS]=
{
	"B",           //0
	"Y",           //1
	"SELECT",      //2
	"START",       //3
	"D-Pad Up",    //4
	"D-Pad Down",  //5
	"D-Pad Left",  //6
	"D-Pad Right", //7
	"A",           //8
	"X",           //9
	"L1",          //10
	"R1",          //11
	"L2",          //12
	"R2",          //13
	"L3",          //14
	"R3",          //15
};

//    Default : A ->B1 | B ->B2 | X ->B3 | Y ->B4 | L ->B5 | R ->B6
int Buttons_mapping[]={RETROPAD_A,RETROPAD_B,RETROPAD_X,RETROPAD_Y,RETROPAD_L,RETROPAD_R};

void Input_Binding(running_machine &machine)
{
   log_cb(RETRO_LOG_INFO, "SOURCE FILE: %s\n", machine.system().type.source());
   log_cb(RETRO_LOG_INFO, "PARENT: %s\n", machine.system().parent);
   log_cb(RETRO_LOG_INFO, "NAME: %s\n", machine.system().name);
   log_cb(RETRO_LOG_INFO, "DESCRIPTION: %s\n", machine.system().type.fullname());
   log_cb(RETRO_LOG_INFO, "YEAR: %s\n", machine.system().year);
   log_cb(RETRO_LOG_INFO, "MANUFACTURER: %s\n", machine.system().manufacturer);

   Buttons_mapping[1]=RETROPAD_A;
   Buttons_mapping[0]=RETROPAD_B;
   Buttons_mapping[3]=RETROPAD_X;
   Buttons_mapping[2]=RETROPAD_Y;
   Buttons_mapping[4]=RETROPAD_L;
   Buttons_mapping[5]=RETROPAD_R;

   if (
         !core_stricmp(machine.system().name, "tekken")    ||
         !core_stricmp(machine.system().parent, "tekken")  ||
         !core_stricmp(machine.system().name, "tekken2")   ||
         !core_stricmp(machine.system().parent, "tekken2")
      )
   {
      /* Tekken 1/2 */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_B;
      Buttons_mapping[3]=RETROPAD_A;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "souledge")    ||
              !core_stricmp(machine.system().parent, "souledge")  ||
              !core_stricmp(machine.system().name, "soulclbr")    ||
              !core_stricmp(machine.system().parent, "soulclbr")
           )
   {
      /* Soul Edge/Soul Calibur */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_A;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "doapp")
           )
   {
      /* Dead or Alive++ */

      Buttons_mapping[0]=RETROPAD_B;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_A;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "vf") ||
              !core_stricmp(machine.system().parent, "vf")
           )
   {
      /* Virtua Fighter */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_B;
      Buttons_mapping[3]=RETROPAD_A;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "ehrgeiz") ||
              !core_stricmp(machine.system().parent, "ehrgeiz")
           )
   {
      /* Ehrgeiz */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_B;
      Buttons_mapping[2]=RETROPAD_A;
      Buttons_mapping[3]=RETROPAD_X;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "ts2") ||
              !core_stricmp(machine.system().parent, "ts2")
           )
   {
      /* Toshinden 2 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_R;
      Buttons_mapping[4]=RETROPAD_B;
      Buttons_mapping[5]=RETROPAD_A;

   }
   else if (
              (!core_stricmp(machine.system().name, "dstlk")) ||
              (!core_stricmp(machine.system().parent, "dstlk")) ||
              !core_stricmp(machine.system().name, "hsf2") ||
              !core_stricmp(machine.system().parent, "hsf2") ||
              !core_stricmp(machine.system().name, "msh") ||
              !core_stricmp(machine.system().parent, "msh") ||
              !core_stricmp(machine.system().name, "mshvsf") ||
              !core_stricmp(machine.system().parent, "mshvsf") ||
              !core_stricmp(machine.system().name, "mvsc") ||
              !core_stricmp(machine.system().parent, "mvsc") ||
              !core_stricmp(machine.system().name, "nwarr") ||
              !core_stricmp(machine.system().parent, "nwarr") ||
              !core_stricmp(machine.system().name, "rvschool") ||
              !core_stricmp(machine.system().parent, "rvschool") ||
              !core_stricmp(machine.system().name, "sf2") ||
              !core_stricmp(machine.system().parent, "sf2") ||
              !core_stricmp(machine.system().name, "sf2ce") ||
              !core_stricmp(machine.system().parent, "sf2ce") ||
              !core_stricmp(machine.system().name, "sf2hf") ||
              !core_stricmp(machine.system().parent, "sf2hf") ||
              !core_stricmp(machine.system().name, "sfa") ||
              !core_stricmp(machine.system().parent, "sfa") ||
              !core_stricmp(machine.system().name, "sfa2") ||
              !core_stricmp(machine.system().parent, "sfa2") ||
              !core_stricmp(machine.system().name, "sfa3") ||
              !core_stricmp(machine.system().parent, "sfa3") ||
              !core_stricmp(machine.system().name, "sfex") ||
              !core_stricmp(machine.system().parent, "sfex") ||
              !core_stricmp(machine.system().name, "sfex2") ||
              !core_stricmp(machine.system().parent, "sfex2") ||
              !core_stricmp(machine.system().name, "sfex2p") ||
              !core_stricmp(machine.system().parent, "sfex2p") ||
              !core_stricmp(machine.system().name, "sfexp") ||
              !core_stricmp(machine.system().parent, "sfexp") ||
              !core_stricmp(machine.system().name, "sfiii") ||
              !core_stricmp(machine.system().parent, "sfiii") ||
              !core_stricmp(machine.system().name, "sfiii2") ||
              !core_stricmp(machine.system().parent, "sfiii2") ||
              !core_stricmp(machine.system().name, "sfiii3") ||
              !core_stricmp(machine.system().parent, "sfiii3") ||
              !core_stricmp(machine.system().name, "sftm") ||
              !core_stricmp(machine.system().parent, "sftm") ||
              !core_stricmp(machine.system().name, "ssf2") ||
              !core_stricmp(machine.system().parent, "ssf2") ||
              !core_stricmp(machine.system().name, "ssf2t") ||
              !core_stricmp(machine.system().parent, "ssf2t") ||
              !core_stricmp(machine.system().name, "starglad") ||
              !core_stricmp(machine.system().parent, "starglad") ||
              !core_stricmp(machine.system().name, "vsav") ||
              !core_stricmp(machine.system().parent, "vsav") ||
              !core_stricmp(machine.system().name, "vsav2") ||
              !core_stricmp(machine.system().parent, "vsav2") ||
              !core_stricmp(machine.system().name, "xmcota") ||
              !core_stricmp(machine.system().parent, "xmcota") ||
              !core_stricmp(machine.system().name, "xmvsf") ||
              !core_stricmp(machine.system().parent, "xmvsf")
           )
   {
      /* Capcom CPS-1 and CPS-2 6-button fighting games */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_L;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().parent, "aof") ||
              !core_stricmp(machine.system().parent, "aof2") ||
              !core_stricmp(machine.system().parent, "aof3") ||
              !core_stricmp(machine.system().parent, "breakers") ||
              !core_stricmp(machine.system().parent, "breakrev") ||
              !core_stricmp(machine.system().parent, "doubledr") ||
              !core_stricmp(machine.system().parent, "fatfury1") ||
              !core_stricmp(machine.system().parent, "fatfury2") ||
              !core_stricmp(machine.system().parent, "fatfury3") ||
              !core_stricmp(machine.system().parent, "fatfursp") ||
              !core_stricmp(machine.system().parent, "fightfev") ||
              !core_stricmp(machine.system().parent, "galaxyfg") ||
              !core_stricmp(machine.system().parent, "garou") ||
              !core_stricmp(machine.system().parent, "gowcaizr") ||
              !core_stricmp(machine.system().parent, "neogeo") ||
              !core_stricmp(machine.system().parent, "karnovr") ||
              !core_stricmp(machine.system().parent, "kizuna") ||
              !core_stricmp(machine.system().parent, "kabukikl") ||
              !core_stricmp(machine.system().parent, "matrim") ||
              !core_stricmp(machine.system().parent, "mslug") ||
              !core_stricmp(machine.system().parent, "mslug2") ||
              !core_stricmp(machine.system().parent, "mslugx") ||
              !core_stricmp(machine.system().parent, "mslug3") ||
              !core_stricmp(machine.system().parent, "mslug4") ||
              !core_stricmp(machine.system().parent, "mslug5") ||
              !core_stricmp(machine.system().parent, "kof94") ||
              !core_stricmp(machine.system().parent, "kof95") ||
              !core_stricmp(machine.system().parent, "kof96") ||
              !core_stricmp(machine.system().parent, "kof97") ||
              !core_stricmp(machine.system().parent, "kof98") ||
              !core_stricmp(machine.system().parent, "kof99") ||
              !core_stricmp(machine.system().parent, "kof2000") ||
              !core_stricmp(machine.system().parent, "kof2001") ||
              !core_stricmp(machine.system().parent, "kof2002") ||
              !core_stricmp(machine.system().parent, "kof2003") ||
              !core_stricmp(machine.system().parent, "lresort") ||
              !core_stricmp(machine.system().parent, "lastblad") ||
              !core_stricmp(machine.system().parent, "lastbld2") ||
              !core_stricmp(machine.system().parent, "ninjamas") ||
              !core_stricmp(machine.system().parent, "rotd") ||
              !core_stricmp(machine.system().parent, "rbff1") ||
              !core_stricmp(machine.system().parent, "rbff2") ||
              !core_stricmp(machine.system().parent, "rbffspec") ||
              !core_stricmp(machine.system().parent, "savagere") ||
              !core_stricmp(machine.system().parent, "sengoku3") ||
              !core_stricmp(machine.system().parent, "samsho") ||
              !core_stricmp(machine.system().parent, "samsho2") ||
              !core_stricmp(machine.system().parent, "samsho3") ||
              !core_stricmp(machine.system().parent, "samsho4") ||
              !core_stricmp(machine.system().parent, "samsho5") ||
              !core_stricmp(machine.system().parent, "samsh5sp") ||
              !core_stricmp(machine.system().parent, "svc") ||
              !core_stricmp(machine.system().parent, "viewpoin") ||
              !core_stricmp(machine.system().parent, "wakuwak7") ||
              !core_stricmp(machine.system().parent, "wh1") ||
              !core_stricmp(machine.system().parent, "wh2") ||
              !core_stricmp(machine.system().parent, "wh2j") ||
              !core_stricmp(machine.system().parent, "whp")
           )
   {
      /* Neo Geo */

      Buttons_mapping[0]=RETROPAD_B;
      Buttons_mapping[1]=RETROPAD_A;
      Buttons_mapping[2]=RETROPAD_Y;
      Buttons_mapping[3]=RETROPAD_X;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "kinst") ||
              !core_stricmp(machine.system().parent, "kinst")
           )
   {
      /* Killer Instinct 1 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_R;
      Buttons_mapping[4]=RETROPAD_B;
      Buttons_mapping[5]=RETROPAD_A;

   }
   else if (
              !core_stricmp(machine.system().name, "kinst2") ||
              !core_stricmp(machine.system().parent, "kinst2")
           )
   {
      /* Killer Instinct 2 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "tektagt")   ||
              !core_stricmp(machine.system().parent, "tektagt") ||
              !core_stricmp(machine.system().name, "tekken3")   ||
              !core_stricmp(machine.system().parent, "tekken3")
           )
   {
      /* Tekken 3/Tekken Tag Tournament */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_R;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_L;

   }
   else if (
              !core_stricmp(machine.system().name, "mk")       ||
              !core_stricmp(machine.system().parent, "mk")     ||
              !core_stricmp(machine.system().name, "mk2")      ||
              !core_stricmp(machine.system().parent, "mk2")    ||
              !core_stricmp(machine.system().name, "mk3")      ||
              !core_stricmp(machine.system().parent, "mk3")    ||
              !core_stricmp(machine.system().name, "umk3")     ||
              !core_stricmp(machine.system().parent, "umk3")   ||
              !core_stricmp(machine.system().name, "wwfmania") ||
              !core_stricmp(machine.system().parent, "wwfmania")
           )
   {
      /* Mortal Kombat 1/2/3/Ultimate/WWF: Wrestlemania */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_L;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;
}

void retro_osd_interface::release_keys()
{
	auto keybd = dynamic_cast<input_module_base*>(m_keyboard_input);
	if (keybd != nullptr)
		keybd->devicelist()->reset_devices();
}

void retro_osd_interface::process_keyboard_state(running_machine &machine)
{
   /* TODO: handle mods:SHIFT/CTRL/ALT/META/NUMLOCK/CAPSLOCK/SCROLLOCK */
   unsigned i = 0;
   do
   {
      retrokbd_state[ktable[i].retro_key_name] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,ktable[i].retro_key_name) ? 0x80 : 0;

      if(retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name] == 0)
      {
         //ui_ipt_pushchar=ktable[i].retro_key_name;
	//FIXME remove up/dw/lf/rg char from input ui
	 machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), ktable[i].retro_key_name);


         retrokbd_state2[ktable[i].retro_key_name]=1;
      }
      else if(!retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name] == 1)
         retrokbd_state2[ktable[i].retro_key_name]=0;

      i++;

   }while(ktable[i].retro_key_name!=-1);
}

void retro_osd_interface::process_joypad_state(running_machine &machine)
{
   unsigned i, j;
   int analog_l2, analog_r2;
   int16_t ret[4];

   if (libretro_supports_bitmasks)
   {
      for(j = 0;j < 4; j++)
      {
         ret[j] = 0;
         ret[j] = input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
      }
   }
   else
   {
      for(j = 0;j < 4; j++)
      {
         ret[j] = 0;
         for(i = 0;i < RETRO_MAX_BUTTONS; i++)
            if (input_state_cb(j, RETRO_DEVICE_JOYPAD, 0,i))
               ret[j] |= (1 << i);
      }
   }

   for(j = 0;j < 4; j++)
   {
      for(i = 0;i < RETRO_MAX_BUTTONS; i++)
      {
         if (ret[j] & (1 << i))
            joystate[j].button[i] = 0x80;
         else
            joystate[j].button[i] = 0;
      }

      joystate[j].a1[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystate[j].a1[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);
      joystate[j].a2[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystate[j].a2[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);

      analog_l2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_L2);
      analog_r2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_R2);
      /* Fallback, if no analog trigger support, use digital */
      if (analog_l2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_L2))
            analog_l2 = 32767;
      }
      if (analog_r2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_R2))
            analog_r2 = 32767;
      }
      joystate[j].a3[0] = -normalize_absolute_axis(analog_l2, 0, 32767);
      joystate[j].a3[1] = -normalize_absolute_axis(analog_r2, 0, 32767);
   }
}

void retro_osd_interface::process_mouse_state(running_machine &machine)
{
   static int mbL = 0, mbR = 0;
   int mouse_l;
   int mouse_r;
   int16_t mouse_x;
   int16_t mouse_y;
   //printf("mouseneable=%d\n",mouse_enable);
   if (!mouse_enable)
      return;

   mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
   mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
   mouse_l = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
   mouse_r = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
   mouseLX = mouse_x*INPUT_RELATIVE_PER_PIXEL;
   mouseLY = mouse_y*INPUT_RELATIVE_PER_PIXEL;

static int vmx=fb_width/2,vmy=fb_height/2;
static int ovmx=fb_width/2,ovmy=fb_height/2;

vmx+=mouse_x;
vmy+=mouse_y;
if(vmx>fb_width)vmx=fb_width-1;
if(vmy>fb_height)vmy=fb_height-1;
if(vmx<0)vmx=0;
if(vmy<0)vmy=0;
if(vmx!=ovmx || vmy!=ovmy){
	int cx = -1, cy = -1;
	auto window = osd_common_t::s_window_list.front();

	if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
					machine.ui_input().push_mouse_move_event(window->target(), cx, cy);
}
ovmx=vmx;
ovmy=vmy;

   if(mbL==0 && mouse_l)
   {
      mbL=1;
      mouseBUT[0]=0x80;

	int cx = -1, cy = -1;
	auto window = osd_common_t::s_window_list.front();
	//FIXME doubleclick
	if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
		machine.ui_input().push_mouse_down_event(window->target(), cx, cy);


   }
   else if(mbL==1 && !mouse_l)
   {
      mouseBUT[0]=0;
      mbL=0;

	int cx = -1, cy = -1;
	auto window = osd_common_t::s_window_list.front();
	if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
		machine.ui_input().push_mouse_up_event(window->target(), cx, cy);

   }

   if(mbR==0 && mouse_r)
   {
      mbR=1;
      mouseBUT[1]=1;

	int cx = -1, cy = -1;
	auto window = osd_common_t::s_window_list.front();

	if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
		machine.ui_input().push_mouse_rdown_event(window->target(), cx, cy);


   }
   else if(mbR==1 && !mouse_r)
   {
      mouseBUT[1]=0;
      mbR=0;

	int cx = -1, cy = -1;
	auto window = osd_common_t::s_window_list.front();
	if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
		machine.ui_input().push_mouse_rup_event(window->target(), cx, cy);

   }

	//printf("vm(%d,%d) mc(%d,%d) mr(%d,%d)\n",vmx,vmy,mouse_x,mouse_y,mouseLX,mouseLY);
}

void retro_osd_interface::process_lightgun_state(running_machine &machine)
{
   int16_t gun_x_raw, gun_y_raw;

   if ( lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_DISABLED ) {
      return;
   }

   for (int i = 0; i < 4; i++) {
      lightgunBUT[i] = 0;
   }

   if ( lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_POINTER ) {
      gun_x_raw = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
      gun_y_raw = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

      // handle pointer presses
      // use multi-touch to support different button inputs
      int touch_count = input_state_cb( 0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_COUNT );
      if ( touch_count > 0 && touch_count <= 4 ) {
         lightgunBUT[touch_count-1] = 0x80;
      }
   } else { // lightgun is default when enabled
      gun_x_raw = input_state_cb( 0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X );
      gun_y_raw = input_state_cb( 0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y );

      if ( input_state_cb( 0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER ) ) {
         lightgunBUT[0] = 0x80;
      }
      if ( input_state_cb( 0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A ) ) {
         lightgunBUT[1] = 0x80;
      }
   }
   lightgunX = gun_x_raw * 2;
   lightgunY = gun_y_raw * 2;
}

//============================================================
//  retro_keyboard_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:

	retro_keyboard_device(running_machine& machine, const char *name, const char *id, input_module &module)
		: event_based_device(machine, name, id, DEVICE_CLASS_KEYBOARD, module)
	{
	}

	void reset() override
	{
		int i;
   		for(i = 0; i < RETROK_LAST; i++){
      			retrokbd_state[i]=0;
      			retrokbd_state2[i]=0;
   		}
	}

protected:
	void process_event(KeyPressEventArgs &args) /*override*/
	{
//		printf("here\n");
	}
};

//============================================================
//  keyboard_input_retro - retro keyboard input module
//============================================================

class keyboard_input_retro : public retroinput_module
{
private:

public:
	keyboard_input_retro()
		: retroinput_module(OSD_KEYBOARDINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_keyboard_device *devinfo = devicelist()->create_device<retro_keyboard_device>(machine, "Retro Keyboard 1", "Retro Keyboard 1", *this);

		int i;
   		for(i = 0; i < RETROK_LAST; i++){
		      retrokbd_state[i]=0;
		      retrokbd_state2[i]=0;
		}

   		i=0;
   		do{
			devinfo->device()->add_item(\
				ktable[i].mame_key_name,\
				ktable[i].mame_key, \
				generic_button_get_state<std::uint8_t>,\
				&retrokbd_state[ktable[i].retro_key_name]);
      			i++;
   		}while(ktable[i].retro_key_name!=-1);

		m_global_inputs_enabled = true;

	}

	bool handle_input_event(void) override
	{
		if (!input_enabled())
			return false;
		return true;

	}
};


//============================================================
//  retro_mouse_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_mouse_device : public event_based_device<KeyPressEventArgs>
{
public:

	retro_mouse_device(running_machine& machine, const char *name, const char *id, input_module &module)
		: event_based_device(machine, name, id, DEVICE_CLASS_MOUSE, module)
	{
	}

	void poll() override
	{
event_based_device::poll();

	}

	void reset() override
	{
		mouseLX=fb_width/2;
		mouseLY=fb_height/2;

		int i;
   		for(i = 0; i < 4; i++)mouseBUT[i]=0;
	}

protected:
	void process_event(KeyPressEventArgs &args) /*override*/
	{
//		printf("here\n");
	}
};

//============================================================
//  mouse_input_retro - retro mouse input module
//============================================================

class mouse_input_retro : public retroinput_module
{
private:

public:
	mouse_input_retro()
		: retroinput_module(OSD_MOUSEINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_mouse_device *devinfo;

		if (!input_enabled() || !mouse_enabled())
			return;

		devinfo = devicelist()->create_device<retro_mouse_device>(machine, "Retro mouse 1", "Retro mouse 1", *this);
		if (devinfo == nullptr)
			return;

		mouseLX=fb_width/2;
		mouseLY=fb_height/2;

		devinfo->device()->add_item(
				"X",
				static_cast<input_item_id>(ITEM_ID_XAXIS),
				generic_axis_get_state<std::int32_t>,
				&mouseLX);
		devinfo->device()->add_item(
				"Y",
				static_cast<input_item_id>(ITEM_ID_YAXIS),
				generic_axis_get_state<std::int32_t>,
				&mouseLY);

		int button;
		for (button = 0; button < 4; button++)
		{
			mouseBUT[button]=0;
			devinfo->device()->add_item(
				default_button_name(button),
				static_cast<input_item_id>(ITEM_ID_BUTTON1 + button),
				generic_button_get_state<std::int32_t>,
				&mouseBUT[button]);
		}

		m_global_inputs_enabled = true;

	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !mouse_enabled())
			return false;
		return true;

	}
};


//============================================================
//  retro_joystick_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_joystick_device : public event_based_device<KeyPressEventArgs>
{
public:

	retro_joystick_device(running_machine& machine, const char *name, const char *id, input_module &module)
		: event_based_device(machine, name, id, DEVICE_CLASS_JOYSTICK, module)
	{
	}

	void poll() override
	{
		event_based_device::poll();
	}

	void reset() override
	{
		memset(&joystate, 0, sizeof(joystate));
	}

protected:
	void process_event(KeyPressEventArgs &args) /*override*/
	{
	}
};

//============================================================
//  joystick_input_retro - retro joystick input module
//============================================================

class joystick_input_retro : public retroinput_module
{
private:

public:
	joystick_input_retro()
		: retroinput_module(OSD_JOYSTICKINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{

		int i,j;
 		char defname[32];

		if (buttons_profiles)
			Input_Binding(machine);

		for (i = 0; i < 4; i++)
		{
 			sprintf(defname, "RetroPad%d", i);

			retro_joystick_device *devinfo;

			if (!input_enabled()/* || !joystick_enabled()*/)
				return;

			devinfo = devicelist()->create_device<retro_joystick_device>(machine, defname, defname, *this);
			if (devinfo == nullptr)
				continue;

			// add the axes
			devinfo->device()->add_item(
				"LSX",
				static_cast<input_item_id>(ITEM_ID_XAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a1[0]);
			devinfo->device()->add_item(
				"LSY",
				static_cast<input_item_id>(ITEM_ID_YAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a1[1]);

			devinfo->device()->add_item(
				"RSX",
				static_cast<input_item_id>(ITEM_ID_RXAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a2[0]);
			devinfo->device()->add_item(
				"RSY",
				static_cast<input_item_id>(ITEM_ID_RYAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a2[1]);

			devinfo->device()->add_item(
				"L2",
				static_cast<input_item_id>(ITEM_ID_RZAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a3[0]);

			devinfo->device()->add_item(
				"R2",
				static_cast<input_item_id>(ITEM_ID_ZAXIS),
				generic_axis_get_state<std::int32_t>,
				&joystate[i].a3[1]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_START], ITEM_ID_START,
				generic_button_get_state<std::int32_t>, &joystate[i].button[RETROPAD_START]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_SELECT], ITEM_ID_SELECT,
				generic_button_get_state<std::int32_t>, &joystate[i].button[RETROPAD_SELECT]);

			for(j = 0; j < 6; j++)
				devinfo->device()->add_item(Buttons_Name[Buttons_mapping[j]],
					 (input_item_id)(ITEM_ID_BUTTON1+j),
					 generic_button_get_state<std::int32_t>,
					  &joystate[i].button[Buttons_mapping[j]]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_L3], ITEM_ID_BUTTON9,
				generic_button_get_state<std::int32_t>, &joystate[i].button[RETROPAD_L3]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_R3], ITEM_ID_BUTTON10,
				generic_button_get_state<std::int32_t>, &joystate[i].button[RETROPAD_R3]);

			// D-Pad
			devinfo->device()->add_item(Buttons_Name[RETROPAD_PAD_UP], static_cast<input_item_id>(ITEM_ID_HAT1UP+i*4),
				generic_button_get_state<std::uint8_t>, &joystate[i].button[RETROPAD_PAD_UP]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_PAD_DOWN], static_cast<input_item_id>(ITEM_ID_HAT1DOWN+i*4),
				generic_button_get_state<std::uint8_t>, &joystate[i].button[RETROPAD_PAD_DOWN]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_PAD_LEFT], static_cast<input_item_id>(ITEM_ID_HAT1LEFT+i*4),
				generic_button_get_state<std::uint8_t>, &joystate[i].button[RETROPAD_PAD_LEFT]);

			devinfo->device()->add_item(Buttons_Name[RETROPAD_PAD_RIGHT], static_cast<input_item_id>(ITEM_ID_HAT1RIGHT+i*4),
				generic_button_get_state<std::uint8_t>, &joystate[i].button[RETROPAD_PAD_RIGHT]);
		}

		m_global_inputs_enabled = true;

	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() /*|| !joystick_enabled()*/)
			return false;
		return true;

	}
};

//============================================================
//  retro_lightgun_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_lightgun_device : public event_based_device<KeyPressEventArgs>
{
public:

	retro_lightgun_device(running_machine& machine, const char *name, const char *id, input_module &module)
		: event_based_device(machine, name, id, DEVICE_CLASS_LIGHTGUN, module)
	{
	}

	void poll() override
	{
event_based_device::poll();

	}

	void reset() override
	{
		lightgunX=fb_width/2;
		lightgunY=fb_height/2;
  		int i;
 		for(i = 0; i < 4; i++)lightgunBUT[i]=0;
	}

protected:
	void process_event(KeyPressEventArgs &args) /*override*/
	{
//		printf("here\n");
	}
};

//============================================================
//  lightgun_input_retro - retro lightgun input module
//============================================================

class lightgun_input_retro : public retroinput_module
{
private:

public:
	lightgun_input_retro()
		: retroinput_module(OSD_LIGHTGUNINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_lightgun_device *devinfo;
		if (!input_enabled() || !lightgun_enabled())
			return;
		devinfo = devicelist()->create_device<retro_lightgun_device>(machine, "Retro lightgun 1", "Retro lightgun 1", *this);
		if (devinfo == nullptr)
			return;

		lightgunX=fb_width/2;
		lightgunY=fb_height/2;
		devinfo->device()->add_item(
				"X",
				static_cast<input_item_id>(ITEM_ID_XAXIS),
				generic_axis_get_state<std::int32_t>,
				&lightgunX);
		devinfo->device()->add_item(
				"Y",
				static_cast<input_item_id>(ITEM_ID_YAXIS),
				generic_axis_get_state<std::int32_t>,
				&lightgunY);

		int button;
		for (button = 0; button < 4; button++)
		{
			lightgunBUT[button]=0;
			devinfo->device()->add_item(
				default_button_name(button),
				static_cast<input_item_id>(ITEM_ID_BUTTON1 + button),
				generic_button_get_state<std::int32_t>,
				&lightgunBUT[button]);
		}

		m_global_inputs_enabled = true;

	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !lightgun_enabled())
			return false;
		return true;

	}
};

void retro_osd_interface::process_events_buf()
{
	input_poll_cb();
}

void retro_osd_interface::poll_inputs(running_machine &machine)
{
	process_mouse_state(machine);
	process_keyboard_state(machine);
	process_joypad_state(machine);
	process_lightgun_state(machine);
}

MODULE_DEFINITION(KEYBOARDINPUT_RETRO, keyboard_input_retro)
MODULE_DEFINITION(MOUSEINPUT_RETRO, mouse_input_retro)
MODULE_DEFINITION(JOYSTICKINPUT_RETRO, joystick_input_retro)
MODULE_DEFINITION(LIGHTGUNINPUT_RETRO, lightgun_input_retro)
