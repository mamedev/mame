//============================================================
//
//  output.c - Win32 implementation of MAME output routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "osdepend.h"
#include "driver.h"

// MAMEOS headers
#include "output.h"



//============================================================
//  CONSTANTS
//============================================================

// window styles
#define WINDOW_STYLE		WS_OVERLAPPEDWINDOW
#define WINDOW_STYLE_EX		0



//============================================================
//  TYPEDEFS
//============================================================

typedef struct _registered_client registered_client;
struct _registered_client
{
	registered_client *	next;		// next client in the list
	LPARAM				id;			// client-specified ID
	HWND				hwnd;		// client HWND
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

// our HWND
static HWND					output_hwnd;

// client list
static registered_client *	clientlist;

// message IDs
static UINT					om_mame_start;
static UINT					om_mame_stop;
static UINT					om_mame_update_state;
static UINT					om_mame_register_client;
static UINT					om_mame_unregister_client;
static UINT					om_mame_get_id_string;



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static void winoutput_exit(running_machine *machine);
static int create_window_class(void);
static LRESULT CALLBACK output_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
static LRESULT register_client(HWND hwnd, LPARAM id);
static LRESULT unregister_client(HWND hwnd, LPARAM id);
static LRESULT send_id_string(HWND hwnd, LPARAM id);
static void notifier_callback(const char *outname, INT32 value, void *param);



//============================================================
//  winoutput_init
//============================================================

void winoutput_init(running_machine *machine)
{
	int result;

	// ensure we get cleaned up
	add_exit_callback(machine, winoutput_exit);

	// reset globals
	clientlist = NULL;

	// create our window class
	result = create_window_class();
	assert(result == 0);

	// create a window
	output_hwnd = CreateWindowEx(
						WINDOW_STYLE_EX,
						OUTPUT_WINDOW_CLASS,
						OUTPUT_WINDOW_NAME,
						WINDOW_STYLE,
						0, 0,
						1, 1,
						NULL,
						NULL,
						GetModuleHandle(NULL),
						NULL);
	assert(output_hwnd != NULL);

	// allocate message ids
	om_mame_start = RegisterWindowMessage(OM_MAME_START);
	assert(om_mame_start != 0);
	om_mame_stop = RegisterWindowMessage(OM_MAME_STOP);
	assert(om_mame_stop != 0);
	om_mame_update_state = RegisterWindowMessage(OM_MAME_UPDATE_STATE);
	assert(om_mame_update_state != 0);

	om_mame_register_client = RegisterWindowMessage(OM_MAME_REGISTER_CLIENT);
	assert(om_mame_register_client != 0);
	om_mame_unregister_client = RegisterWindowMessage(OM_MAME_UNREGISTER_CLIENT);
	assert(om_mame_unregister_client != 0);
	om_mame_get_id_string = RegisterWindowMessage(OM_MAME_GET_ID_STRING);
	assert(om_mame_get_id_string != 0);

	// broadcast a startup message
	PostMessage(HWND_BROADCAST, om_mame_start, (WPARAM)output_hwnd, 0);

	// register a notifier for output changes
	output_set_notifier(NULL, notifier_callback, NULL);
}


//============================================================
//  winoutput_exit
//============================================================

static void winoutput_exit(running_machine *machine)
{
	// free all the clients
	while (clientlist != NULL)
	{
		registered_client *temp = clientlist;
		clientlist = temp->next;
		free(temp);
	}

	// broadcast a shutdown message
	PostMessage(HWND_BROADCAST, om_mame_stop, (WPARAM)output_hwnd, 0);
}


//============================================================
//  create_window_class
//============================================================

static int create_window_class(void)
{
	static UINT8 classes_created = FALSE;

	/* only do this once */
	if (!classes_created)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName 	= OUTPUT_WINDOW_CLASS;
		wc.hInstance 		= GetModuleHandle(NULL);
		wc.lpfnWndProc		= output_window_proc;

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			return 1;
		classes_created = TRUE;
	}

	return 0;
}


//============================================================
//  output_window_proc
//============================================================

static LRESULT CALLBACK output_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// register a new client
	if (message == om_mame_register_client)
		return register_client((HWND)wparam, lparam);

	// unregister a client
	else if (message == om_mame_unregister_client)
		return unregister_client((HWND)wparam, lparam);

	// get a string for an ID
	else if (message == om_mame_get_id_string)
		return send_id_string((HWND)wparam, lparam);

	else
		return DefWindowProc(wnd, message, wparam, lparam);
}


//============================================================
//  register_client
//============================================================

static LRESULT register_client(HWND hwnd, LPARAM id)
{
	registered_client **client;

	// find the end of the list; if we find ourself already registered,
	// return 1
	for (client = &clientlist; *client != NULL; client = &(*client)->next)
		if ((*client)->id == id)
		{
			(*client)->hwnd = hwnd;
			output_notify_all(notifier_callback, *client);
			return 1;
		}

	// add us to the end
	*client = malloc_or_die(sizeof(**client));
	(*client)->next = NULL;
	(*client)->id = id;
	(*client)->hwnd = hwnd;

	// request a notification for all outputs
	output_notify_all(notifier_callback, *client);
	return 0;
}


//============================================================
//  unregister_client
//============================================================

static LRESULT unregister_client(HWND hwnd, LPARAM id)
{
	registered_client **client;
	int found = FALSE;

	// find any matching IDs in the list and remove them
	for (client = &clientlist; *client != NULL; client = &(*client)->next)
		if ((*client)->id == id)
		{
			registered_client *temp = *client;
			*client = (*client)->next;
			free(temp);
			found = TRUE;
		}

	// return an error if not found
	return found ? 0 : 1;
}


//============================================================
//  send_id_string
//============================================================

static LRESULT send_id_string(HWND hwnd, LPARAM id)
{
	copydata_id_string *temp;
	COPYDATASTRUCT copydata;
	const char *name;
	int datalen;

	// id 0 is the name of the game
	if (id == 0)
		name = Machine->gamedrv->name;
	else
		name = output_id_to_name(id);

	// a NULL name is an empty string
	if (name == NULL)
		name = "";

	// allocate memory for the message
	datalen = sizeof(*temp) + strlen(name);
	temp = malloc_or_die(datalen);
	temp->id = id;
	strcpy(temp->string, name);

	// reply by using SendMessage with WM_COPYDATA
	copydata.dwData = COPYDATA_MESSAGE_ID_STRING;
	copydata.cbData = datalen;
	copydata.lpData = temp;
	SendMessage(hwnd, WM_COPYDATA, (WPARAM)output_hwnd, (LPARAM)&copydata);

	// free the data
	free(temp);
	return 0;
}


//============================================================
//  send_id_string
//============================================================

static void notifier_callback(const char *outname, INT32 value, void *param)
{
	registered_client *client;

	// loop over clients and notify them
	for (client = clientlist; client != NULL; client = client->next)
		if (param == NULL || param == client)
			PostMessage(client->hwnd, om_mame_update_state, output_name_to_id(outname), value);
}
