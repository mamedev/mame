#define RETROK_TILDE 178

static UINT16 retrokbd_state2[RETROK_LAST];

struct kt_table {
	const char  *   mame_key_name;
	int retro_key_name;
        input_item_id   mame_key;
};

static kt_table ktable[]={
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

enum
{
        RETROPAD_B,
        RETROPAD_Y,
        RETROPAD_SELECT,
        RETROPAD_START,
        RETROPAD_PAD_UP,
        RETROPAD_PAD_DOWN,
        RETROPAD_PAD_LEFT,
        RETROPAD_PAD_RIGHT,
        RETROPAD_A,
        RETROPAD_X,
        RETROPAD_L,
        RETROPAD_R,
        RETROPAD_L2,
        RETROPAD_R2,
        RETROPAD_L3,
        RETROPAD_R3,
        RETROPAD_TOTAL
};

input_item_id PAD_DIR[4][4]={
	{ITEM_ID_UP,ITEM_ID_DOWN,ITEM_ID_LEFT,ITEM_ID_RIGHT },
	{ITEM_ID_R ,ITEM_ID_F   ,ITEM_ID_D   ,ITEM_ID_G     },
	{ITEM_ID_I,ITEM_ID_K,ITEM_ID_J,ITEM_ID_L },
	{ITEM_ID_8_PAD ,ITEM_ID_2_PAD   ,ITEM_ID_4_PAD   ,ITEM_ID_6_PAD     }
};


//    Default : A ->B1 | B ->B2 | X ->B3 | Y ->B4 | L ->B5 | R ->B6
int   Buttons_mapping[6]={RETROPAD_A,RETROPAD_B,RETROPAD_X,RETROPAD_Y,RETROPAD_L,RETROPAD_R};
static const char *Buttons_Name[MAX_BUTTONS]=
{
        "B",		//0
        "Y",		//1
        "SELECT",	//2
        "START",	//3
        "Pad UP",	//4
        "Pad DOWN",	//5
        "Pad LEFT",	//6
        "Pad RIGHT",	//7
        "A",		//8
        "X",		//9
        "L",		//10
        "R",		//11
        "L2",		//12
        "R2",		//13
        "L3",		//14
        "R3",		//15
};

static void Input_Binding(running_machine &machine);

static INT32 retrokbd_get_state(void *device_internal, void *item_internal)
{
	UINT8 *itemdata = (UINT8 *)item_internal;
	return *itemdata ;
}

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	INT32 *axisdata = (INT32 *) item_internal;
	return *axisdata;
}

static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	INT32 *itemdata = (INT32 *) item_internal;
	return *itemdata >> 7;
}

#define input_device_item_add_joy(a,b,c,d,e)   joy_device[a]->add_item(b,d,e,c)
#define input_device_item_add_mouse(a,b,c,d,e) mouse_device->add_item(b,d,e,c)
#define input_device_item_add_kbd(a,b,c,d,e)   retrokbd_device->add_item(b,d,e,c)
#define input_device_item_add_pad(a,b,c,d,e)   Pad_device[a]->add_item(b,d,e,c)

static void initInput(running_machine &machine)
{
	int i,j,button;
	char defname[20];

   if (mouse_enable)
   {
   	//MOUSE
      	mouse_device = machine.input().device_class(DEVICE_CLASS_MOUSE).add_device("Mice1");
      	// add the axes
      	input_device_item_add_mouse(mouse_device , "X", &mouseLX, ITEM_ID_XAXIS, generic_axis_get_state);
      	input_device_item_add_mouse(mouse_device , "Y", &mouseLY, ITEM_ID_YAXIS, generic_axis_get_state);
	// add the buttons
	for (button = 0; button < 4; button++)
        {
         	input_item_id itemid = (input_item_id) (ITEM_ID_BUTTON1+button);
        	sprintf(defname, "B%d", button + 1);
	        input_device_item_add_mouse(mouse_device, defname, &mouseBUT[button], itemid, generic_button_get_state);
      	}
   }

	//KEYBOARD
	retrokbd_device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device("Retrokdb");

	if (retrokbd_device == NULL)
			fatalerror("KBD Error creating keyboard device\n");


   	for(i = 0; i < RETROK_LAST; i++){
		retrokbd_state[i]=0;
 		retrokbd_state2[i]=0;
	}

   	i=0;
   	do{
 		input_device_item_add_kbd(retrokbd_device,\
		 ktable[i].mame_key_name, &retrokbd_state[ktable[i].retro_key_name],ktable[i].mame_key,retrokbd_get_state);
 		i++;
   	}while(ktable[i].retro_key_name!=-1);

	//JOY/PAD

	Input_Binding(machine);

	for(i=0;i<4;i++){

		sprintf(defname, "Joy%d", i);
		joy_device[i]=machine.input().device_class(DEVICE_CLASS_JOYSTICK).add_device(defname);

		// add the axes
		input_device_item_add_joy (i, "LX", &joystate[i].a1[0], ITEM_ID_XAXIS, generic_axis_get_state);
		input_device_item_add_joy (i, "LY", &joystate[i].a1[1], ITEM_ID_YAXIS, generic_axis_get_state);
		input_device_item_add_joy (i, "RX", &joystate[i].a2[0], ITEM_ID_RXAXIS, generic_axis_get_state);
		input_device_item_add_joy (i, "RY", &joystate[i].a2[1], ITEM_ID_RYAXIS, generic_axis_get_state);

	 	//add the buttons
		for(j=0;j<MAX_BUTTONS;j++)joystate[i].button[j] = 0;

		input_device_item_add_joy (i,Buttons_Name[RETROPAD_START],&joystate[i].button[RETROPAD_START],ITEM_ID_START,generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[RETROPAD_SELECT],&joystate[i].button[RETROPAD_SELECT],ITEM_ID_SELECT,generic_button_get_state );

		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[0]],\
			&joystate[i].button[Buttons_mapping[0]],(input_item_id)(ITEM_ID_BUTTON1+0),generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[1]],\
			&joystate[i].button[Buttons_mapping[1]],(input_item_id)(ITEM_ID_BUTTON1+1),generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[2]],\
			&joystate[i].button[Buttons_mapping[2]],(input_item_id)(ITEM_ID_BUTTON1+2),generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[3]],\
			&joystate[i].button[Buttons_mapping[3]],(input_item_id)(ITEM_ID_BUTTON1+3),generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[4]],\
			&joystate[i].button[Buttons_mapping[4]],(input_item_id)(ITEM_ID_BUTTON1+4),generic_button_get_state );
		input_device_item_add_joy (i,Buttons_Name[Buttons_mapping[5]],\
			&joystate[i].button[Buttons_mapping[5]],(input_item_id)(ITEM_ID_BUTTON1+5),generic_button_get_state );

		sprintf(defname, "Pad%d", i);
		Pad_device[i] = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device(defname);

		input_device_item_add_pad (i,Buttons_Name[RETROPAD_L2], &joystate[i].button[RETROPAD_L2],(input_item_id)(ITEM_ID_TAB+0),retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_R2], &joystate[i].button[RETROPAD_R2],(input_item_id)(ITEM_ID_F11+0),retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_L3], &joystate[i].button[RETROPAD_L3],(input_item_id)(ITEM_ID_F2+0),retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_R3], &joystate[i].button[RETROPAD_R3],(input_item_id)(ITEM_ID_F3+0),retrokbd_get_state );

		input_device_item_add_pad (i,Buttons_Name[RETROPAD_PAD_UP]   , &joystate[i].button[RETROPAD_PAD_UP]   ,PAD_DIR[i][0],retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_PAD_DOWN] , &joystate[i].button[RETROPAD_PAD_DOWN] ,PAD_DIR[i][1],retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_PAD_LEFT] , &joystate[i].button[RETROPAD_PAD_LEFT] ,PAD_DIR[i][2],retrokbd_get_state );
		input_device_item_add_pad (i,Buttons_Name[RETROPAD_PAD_RIGHT], &joystate[i].button[RETROPAD_PAD_RIGHT],PAD_DIR[i][3],retrokbd_get_state );

	}

}

void retro_poll_mame_input(void)
{
	int i=0,j;

	input_poll_cb();

	if (mouse_enable)
	{
		//MOUSE
		static int mbL=0,mbR=0;
		int mouse_l;
		int mouse_r;
		int16_t mouse_x;
		int16_t mouse_y;

		mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
		mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
		mouse_l = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
		mouse_r = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
		mouseLX=mouse_x*INPUT_RELATIVE_PER_PIXEL;;
		mouseLY=mouse_y*INPUT_RELATIVE_PER_PIXEL;;

		if(mbL==0 && mouse_l)
		{
			mbL=1;
			mouseBUT[0]=0x80;
		}
		else if(mbL==1 && !mouse_l)
		{
			mouseBUT[0]=0;
			mbL=0;
		}

		if(mbR==0 && mouse_r)
		{
			mbR=1;
			mouseBUT[1]=1;
		}
		else if(mbR==1 && !mouse_r)
		{
			mouseBUT[1]=0;
			mbR=0;
		}
	}

	//KBD
	//TODO: handle mods:SHIFT/CTRL/ALT/META/NUMLOCK/CAPSLOCK/SCROLLOCK
	i=0;
	do{
 		retrokbd_state[ktable[i].retro_key_name]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,ktable[i].retro_key_name)?0x80:0;
 		if(retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name]==0){
			ui_ipt_pushchar=ktable[i].retro_key_name;
			retrokbd_state2[ktable[i].retro_key_name]=1;
 		}
		else if(!retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name]==1)
			retrokbd_state2[ktable[i].retro_key_name]=0;

 		i++;

	}while(ktable[i].retro_key_name!=-1);

	//JOY/PAD
	for(j=0;j<4;j++){

		for(i=0;i<MAX_BUTTONS;i++)
			joystate[j].button[i] = input_state_cb(j, RETRO_DEVICE_JOYPAD, 0,i)?0x80:0;

		joystate[j].a1[0] = 2*(input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
		joystate[j].a1[1] = 2*(input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
		joystate[j].a2[0] = 2*(input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
		joystate[j].a2[1] = 2*(input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
	}

}

static void Input_Binding(running_machine &machine){

   	fprintf(stderr, "SOURCE FILE: %s\n", machine.system().source_file);
   	fprintf(stderr, "PARENT: %s\n", machine.system().parent);
   	fprintf(stderr, "NAME: %s\n", machine.system().name);
   	fprintf(stderr, "DESCRIPTION: %s\n", machine.system().description);
   	fprintf(stderr, "YEAR: %s\n", machine.system().year);
   	fprintf(stderr, "MANUFACTURER: %s\n", machine.system().manufacturer);

	Buttons_mapping[0]=RETROPAD_A;
	Buttons_mapping[1]=RETROPAD_B;
	Buttons_mapping[2]=RETROPAD_X;
	Buttons_mapping[3]=RETROPAD_Y;
	Buttons_mapping[4]=RETROPAD_L;
	Buttons_mapping[5]=RETROPAD_R;

   if ((core_stricmp(machine.system().name, "tekken") == 0) ||
         (core_stricmp(machine.system().parent, "tekken") == 0) ||
         (core_stricmp(machine.system().name, "tekken2") == 0) ||
         (core_stricmp(machine.system().parent, "tekken2") == 0)
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
   else
   if ((core_stricmp(machine.system().name, "souledge") == 0) ||
         (core_stricmp(machine.system().parent, "souledge") == 0) ||
         (core_stricmp(machine.system().name, "soulclbr") == 0) ||
         (core_stricmp(machine.system().parent, "soulclbr") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "doapp") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "vf") == 0) ||
         (core_stricmp(machine.system().parent, "vf") == 0)
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
  else
   if (
         (core_stricmp(machine.system().name, "ehrgeiz") == 0) ||
         (core_stricmp(machine.system().parent, "ehrgeiz") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "ts2") == 0) ||
         (core_stricmp(machine.system().parent, "ts2") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "vsav") == 0) ||
         (core_stricmp(machine.system().parent, "vsav") == 0) ||
         (core_stricmp(machine.system().name, "vsav2") == 0) ||
         (core_stricmp(machine.system().parent, "vsav2") == 0) ||
         (core_stricmp(machine.system().name, "mshvsf") == 0) ||
         (core_stricmp(machine.system().parent, "mshvsf") == 0) ||
         (core_stricmp(machine.system().name, "msh") == 0) ||
         (core_stricmp(machine.system().parent, "msh") == 0) ||
         (core_stricmp(machine.system().name, "mvsc") == 0) ||
         (core_stricmp(machine.system().parent, "mvsc") == 0) ||
         (core_stricmp(machine.system().name, "xmcota") == 0) ||
         (core_stricmp(machine.system().parent, "xmcota") == 0) ||
         (core_stricmp(machine.system().name, "sf2") == 0) ||
         (core_stricmp(machine.system().parent, "sf2") == 0) ||
         (core_stricmp(machine.system().name, "ssf2") == 0) ||
         (core_stricmp(machine.system().parent, "ssf2") == 0) ||
         (core_stricmp(machine.system().name, "sfa") == 0) ||
         (core_stricmp(machine.system().parent, "sfa") == 0) ||
         (core_stricmp(machine.system().name, "sfa2") == 0) ||
         (core_stricmp(machine.system().parent, "sfa2") == 0) ||
         (core_stricmp(machine.system().name, "sfiii") == 0) ||
         (core_stricmp(machine.system().parent, "sfiii") == 0) ||
         (core_stricmp(machine.system().name, "sfiii2") == 0) ||
         (core_stricmp(machine.system().parent, "sfiii2") == 0) ||
         (core_stricmp(machine.system().name, "sfiii3") == 0) ||
         (core_stricmp(machine.system().parent, "sfiii3") == 0) ||
         (core_stricmp(machine.system().name, "xmvsf") == 0) ||
         (core_stricmp(machine.system().parent, "xmvsf") == 0) ||
         (core_stricmp(machine.system().name, "sftm") == 0) ||
         (core_stricmp(machine.system().parent, "sftm") == 0) ||
         (core_stricmp(machine.system().name, "dstlk") == 0) ||
         (core_stricmp(machine.system().parent, "dstlk") == 0) ||
         (core_stricmp(machine.system().name, "nwarr") == 0) ||
         (core_stricmp(machine.system().parent, "nwarr") == 0) ||
         (core_stricmp(machine.system().name, "sfex") == 0) ||
         (core_stricmp(machine.system().parent, "sfex") == 0) ||
         (core_stricmp(machine.system().name, "sfexp") == 0) ||
         (core_stricmp(machine.system().parent, "sfexp") == 0) ||
         (core_stricmp(machine.system().name, "sfex2") == 0) ||
         (core_stricmp(machine.system().parent, "sfex2") == 0) ||
         (core_stricmp(machine.system().name, "sfex2p") == 0) ||
         (core_stricmp(machine.system().parent, "sfex2p") == 0) ||
         (core_stricmp(machine.system().name, "rvschool") == 0) ||
         (core_stricmp(machine.system().parent, "rvschool") == 0) ||
         (core_stricmp(machine.system().name, "starglad") == 0) ||
         (core_stricmp(machine.system().parent, "starglad") == 0) ||
         (core_stricmp(machine.system().name, "sfa3") == 0) ||
         (core_stricmp(machine.system().parent, "sfa3") == 0)
         )
   {
      /* Capcom CPS2 6-button fighting games */

	Buttons_mapping[0]=RETROPAD_Y;
	Buttons_mapping[1]=RETROPAD_X;
	Buttons_mapping[2]=RETROPAD_L;
	Buttons_mapping[3]=RETROPAD_B;
	Buttons_mapping[4]=RETROPAD_A;
	Buttons_mapping[5]=RETROPAD_R;

   }
   else
   if (
         (core_stricmp(machine.system().parent, "aof") == 0) ||
         (core_stricmp(machine.system().parent, "aof2") == 0) ||
         (core_stricmp(machine.system().parent, "aof3") == 0) ||
         (core_stricmp(machine.system().parent, "breakers") == 0) ||
         (core_stricmp(machine.system().parent, "breakrev") == 0) ||
         (core_stricmp(machine.system().parent, "doubledr") == 0) ||
         (core_stricmp(machine.system().parent, "fatfury1") == 0) ||
         (core_stricmp(machine.system().parent, "fatfury2") == 0) ||
         (core_stricmp(machine.system().parent, "fatfury3") == 0) ||
         (core_stricmp(machine.system().parent, "fatfursp") == 0) ||
         (core_stricmp(machine.system().parent, "fightfev") == 0) ||
         (core_stricmp(machine.system().parent, "galaxyfg") == 0) ||
         (core_stricmp(machine.system().parent, "garou") == 0) ||
         (core_stricmp(machine.system().parent, "gowcaizr") == 0) ||
         (core_stricmp(machine.system().parent, "neogeo") == 0) ||
         (core_stricmp(machine.system().parent, "karnovr") == 0) ||
         (core_stricmp(machine.system().parent, "kizuna") == 0) ||
         (core_stricmp(machine.system().parent, "kabukikl") == 0) ||
         (core_stricmp(machine.system().parent, "matrim") == 0) ||
         (core_stricmp(machine.system().parent, "mslug") == 0) ||
         (core_stricmp(machine.system().parent, "mslug2") == 0) ||
         (core_stricmp(machine.system().parent, "mslugx") == 0) ||
         (core_stricmp(machine.system().parent, "mslug3") == 0) ||
         (core_stricmp(machine.system().parent, "mslug4") == 0) ||
         (core_stricmp(machine.system().parent, "mslug5") == 0) ||
         (core_stricmp(machine.system().parent, "kof94") == 0) ||
         (core_stricmp(machine.system().parent, "kof95") == 0) ||
         (core_stricmp(machine.system().parent, "kof96") == 0) ||
         (core_stricmp(machine.system().parent, "kof97") == 0) ||
         (core_stricmp(machine.system().parent, "kof98") == 0) ||
         (core_stricmp(machine.system().parent, "kof99") == 0) ||
         (core_stricmp(machine.system().parent, "kof2000") == 0) ||
         (core_stricmp(machine.system().parent, "kof2001") == 0) ||
         (core_stricmp(machine.system().parent, "kof2002") == 0) ||
         (core_stricmp(machine.system().parent, "kof2003") == 0) ||
         (core_stricmp(machine.system().parent, "lresort") == 0) ||
         (core_stricmp(machine.system().parent, "lastblad") == 0) ||
         (core_stricmp(machine.system().parent, "lastbld2") == 0) ||
         (core_stricmp(machine.system().parent, "ninjamas") == 0) ||
         (core_stricmp(machine.system().parent, "rotd") == 0) ||
         (core_stricmp(machine.system().parent, "rbff1") == 0) ||
         (core_stricmp(machine.system().parent, "rbff2") == 0) ||
         (core_stricmp(machine.system().parent, "rbffspec") == 0) ||
         (core_stricmp(machine.system().parent, "savagere") == 0) ||
         (core_stricmp(machine.system().parent, "sengoku3") == 0) ||
         (core_stricmp(machine.system().parent, "samsho") == 0) ||
         (core_stricmp(machine.system().parent, "samsho2") == 0) ||
         (core_stricmp(machine.system().parent, "samsho3") == 0) ||
         (core_stricmp(machine.system().parent, "samsho4") == 0) ||
         (core_stricmp(machine.system().parent, "samsho5") == 0) ||
         (core_stricmp(machine.system().parent, "samsh5sp") == 0) ||
         (core_stricmp(machine.system().parent, "svc") == 0) ||
         (core_stricmp(machine.system().parent, "viewpoin") == 0) ||
         (core_stricmp(machine.system().parent, "wakuwak7") == 0) ||
         (core_stricmp(machine.system().parent, "wh1") == 0) ||
         (core_stricmp(machine.system().parent, "wh2") == 0) ||
         (core_stricmp(machine.system().parent, "wh2j") == 0) ||
         (core_stricmp(machine.system().parent, "whp") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "kinst") == 0) ||
         (core_stricmp(machine.system().parent, "kinst") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "kinst2") == 0) ||
         (core_stricmp(machine.system().parent, "kinst2") == 0)
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
   else
   if ((core_stricmp(machine.system().name, "tektagt") == 0) ||
         (core_stricmp(machine.system().parent, "tektagt") == 0) ||
         (core_stricmp(machine.system().name, "tekken3") == 0) ||
         (core_stricmp(machine.system().parent, "tekken3") == 0)
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
   else
   if (
         (core_stricmp(machine.system().name, "mk") == 0) ||
         (core_stricmp(machine.system().parent, "mk") == 0) ||
         (core_stricmp(machine.system().name, "mk2") == 0) ||
         (core_stricmp(machine.system().parent, "mk2") == 0) ||
         (core_stricmp(machine.system().name, "mk3") == 0) ||
         (core_stricmp(machine.system().parent, "mk3") == 0) ||
         (core_stricmp(machine.system().name, "umk3") == 0) ||
         (core_stricmp(machine.system().parent, "umk3") == 0) ||
         (core_stricmp(machine.system().name, "wwfmania") == 0) ||
         (core_stricmp(machine.system().parent, "wwfmania") == 0)
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

}

