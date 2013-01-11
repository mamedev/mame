//============================================================
//
//  debugwin.c - SDL/GTK+ debug window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if !defined(NO_DEBUGGER)

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "dview.h"
#include "osdsdl.h"
#include "debugger.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvstate.h"
#include "debug-intf.h"
#include "debug-sup.h"
#include "debug-cb.h"

#include "config.h"

//============================================================
//  PARAMETERS
//============================================================

#if 1
#define LOG(x...)
#else
#define LOG(x) printf x
#endif

#define MAX_VIEWS       (4)

enum {
	WIN_TYPE_MAIN       = 0x01,
	WIN_TYPE_MEMORY     = 0x02,
	WIN_TYPE_DISASM     = 0x04,
	WIN_TYPE_LOG        = 0x08,
	WIN_TYPE_ALL        = 0x0f
};

//============================================================
//  TYPES
//============================================================


struct hentry  {
	struct hentry *h;
	char *e;
};

struct win_i;

struct edit {
	GtkEntry *edit_w;
	struct hentry *h, *ch;
	char *hold;
	int keep_last;
	void (*cb)(win_i *,const char *);
	win_i *cbp;
};

struct win_i {
	int                     type;
	win_i *                 next;
	GtkWidget *             win;
	edit                    ed;
	running_machine *       machine;    // machine
	DView *                 views[MAX_VIEWS];
	device_t *      cpu;    // current CPU
};

struct windowState
{
	windowState() : type(0),
					sizeX(-1), sizeY(-1),
					positionX(-1), positionY(-1) { }
	~windowState() { }

	void loadFromXmlDataNode(xml_data_node* wnode)
	{
		if (!wnode) return;

		type = xml_get_attribute_int(wnode, "type", type);

		sizeX = xml_get_attribute_int(wnode, "size_x", sizeX);
		sizeY = xml_get_attribute_int(wnode, "size_y", sizeY);
		positionX = xml_get_attribute_int(wnode, "position_x", positionX);
		positionY = xml_get_attribute_int(wnode, "position_y", positionY);
	}

	int type;
	int sizeX;
	int sizeY;
	int positionX;
	int positionY;
};
windowState windowStateArray[64];
int windowStateCount = 0;

//============================================================
//  LOCAL VARIABLES
//============================================================

static win_i *win_list;

//============================================================
//  PROTOTYPES
//============================================================

static void debugmain_init(running_machine &machine);
static void memorywin_new(running_machine &machine);
static void disasmwin_new(running_machine &machine);
static void logwin_new(running_machine &machine);


//============================================================
//  run_func_on_win_list
//============================================================

static void run_func_on_win_list(void (*f)(GtkWidget *widget), int win_type_mask)
{
	win_i *p;
	for(p = win_list; p; p = p->next)
		if ((p->type & win_type_mask) != 0)
			f(p->win);
}



//============================================================
//  get_win_i
//============================================================

static win_i *get_win_i(GtkWidget *win, int win_type_mask)
{
	win_i *p;

	assert(win != NULL);

	LOG(("====== %p %x\n", win, win_type_mask));
	for(p = win_list; p != NULL; p = p->next)
	{
		LOG(("%p %p %x\n", p, p->win, p->type));
		if (((p->type & win_type_mask) != 0) && (win == p->win))
			return p;
	}
	assert(0);
	return NULL;
}


//============================================================
//  get_view
//============================================================

static DView *get_view(win_i *win, int type)
{
	int i;
	for (i=0; i<MAX_VIEWS; i++)
		if (win->views[i]->dv_type == type)
			return win->views[i];
	return 0;
	assert(0);
}



//============================================================
//  get_running_machine
//============================================================

static running_machine *get_running_machine(GtkWidget *win)
{
	win_i *p = get_win_i(win, WIN_TYPE_ALL);
	return p->machine; /* fixme*/
}



//============================================================
//  get_first_win_i
//============================================================

static win_i *get_first_win_i(int win_type_mask)
{
	win_i *p;

	for(p = win_list; p; p = p->next)
	{
		if ((p->type & win_type_mask) != 0)
			return p;
	}
	return NULL;
}



//============================================================
//  add_win_i
//============================================================

static win_i *add_win_i(running_machine &machine, int win_type)
{
	win_i *win = (win_i *) osd_malloc(sizeof(*win));
	memset(win, 0, sizeof(*win));
	win->cpu = NULL;
	win->machine = &machine;
	win->type = win_type;

	win->next = win_list;
	win_list = win;
	return win;
}

//============================================================
//  remove_win_i
//============================================================

static void remove_win_i(win_i *win)
{
	win_i **p = &win_list;
	while(*p != win)
		p = &(*p)->next;
	if(win->next)
		*p = win->next;
	else
		*p = NULL;
}



//============================================================
//  debugwin_show
//============================================================

static void debugwin_show(int show)
{
	void (*f)(GtkWidget *widget) = show ? gtk_widget_show : gtk_widget_hide;

	run_func_on_win_list(f, WIN_TYPE_ALL);
}


//============================================================
//  edit_add_hist
//============================================================

static void edit_add_hist(edit *e, const char *text)
{
	if(e->ch)
		e->ch = 0;

	if(e->hold) {
		osd_free(e->hold);
		e->hold =0;
	}

	if(!e->h || (text[0] && strcmp(text, e->h->e))) {
		hentry *h = (hentry *) osd_malloc(sizeof(hentry));
		h->h = e->h;
		h->e = mame_strdup(text);
		e->h = h;
	}
}



//============================================================
//  edit_set_field
//============================================================

static void edit_set_field(edit *e)
{
	if(e->keep_last) {
		gtk_entry_set_text(e->edit_w, e->h->e);
		gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
	} else
		gtk_entry_set_text(e->edit_w, "");
}



//============================================================
//  edit_activate
//============================================================

static void edit_activate(GtkEntry *item, gpointer user_data)
{
	edit *e = (edit *) user_data;
	const char *text = gtk_entry_get_text(e->edit_w);
	edit_add_hist(e, text);
	e->cb(e->cbp, text);
	edit_set_field(e);

}



//============================================================
//  edit_hist_back
//============================================================

static void edit_hist_back(edit *e)
{
	const char *text = gtk_entry_get_text(e->edit_w);
	if(e->ch) {
		if(!e->ch->h)
			return;
		e->ch = e->ch->h;
	} else {
		if(!e->h)
			return;
		if(!strcmp(text, e->h->e)) {
			if(!e->h->h)
				return;
			e->ch = e->h->h;
		} else {
			if(text[0])
				e->hold = mame_strdup(text);
			e->ch = e->h;
		}
	}
	gtk_entry_set_text(e->edit_w, e->ch->e);
	gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
}



//============================================================
//  dedit_hist_forward
//============================================================

static void edit_hist_forward(edit *e)
{
	hentry *h, *hp;
	if(!e->ch)
		return;
	h = e->h;
	hp = 0;
	while(h != e->ch) {
		hp = h;
		h = h->h;
	}

	if(hp) {
		e->ch = hp;
		gtk_entry_set_text(e->edit_w, hp->e);
	} else if(e->hold) {
		gtk_entry_set_text(e->edit_w, e->hold);
		osd_free(e->hold);
		e->hold = 0;
		e->ch = 0;
	} else {
		gtk_entry_set_text(e->edit_w, "");
		e->ch = 0;
	}
	gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
}



//============================================================
//  edit_key
//============================================================

static gboolean edit_key(GtkWidget *item, GdkEventKey *k, gpointer user_data)
{
	edit *e = (edit *)user_data;

	switch(k->keyval) {
	case GDK_Up:
		edit_hist_back(e);
		return TRUE;
	case GDK_Down:
		edit_hist_forward(e);
		return TRUE;
	}
	return FALSE;
}



//============================================================
//  edit_init
//============================================================

static void edit_init(edit *e, GtkWidget *w, const char *ft, int kl, void (*cb)(win_i *, const char *), win_i *cbp)
{
	e->edit_w = GTK_ENTRY(w);
	e->h = 0;
	e->ch = 0;
	e->hold = 0;
	e->keep_last = kl;
	e->cb = cb;
	e->cbp = cbp;
	if(ft)
		edit_add_hist(e, ft);
	edit_set_field(e);
	g_signal_connect(e->edit_w, "activate", G_CALLBACK(edit_activate), e);
	g_signal_connect(e->edit_w, "key-press-event", G_CALLBACK(edit_key), e);
}



//============================================================
//  debugwin_show
//============================================================

static void edit_del(edit *e)
{
	hentry *h = e->h;
	while(h) {
		hentry *he = h->h;
		osd_free(h->e);
		osd_free(h);
		h = he;
	}
}



//============================================================
//  disasmview_update_checks
//============================================================

static void disasmview_update_checks(win_i *info)
{
	DView *disasm;
	assert(info != NULL);
	// assert(info->type == WIN_TYPE_MAIN);

	disasm = get_view(info, DVT_DISASSEMBLY);

	int rc = downcast<debug_view_disasm *>(disasm->view)->right_column();
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}



//============================================================
//  debugmain_set_cpu
//============================================================

static void debugmain_set_cpu(device_t *device)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);
	DView *dv;

	if (device != dmain->cpu)
	{
		astring title;

		dmain->cpu = device;

		// first set all the views to the new cpu
		// FIXME: Iterate over all views !
		dv = get_view(dmain, DVT_DISASSEMBLY);
		dv->view->set_source(*dv->view->source_list().match_device(device));
		dv = get_view(dmain, DVT_STATE);
		dv->view->set_source(*dv->view->source_list().match_device(device));

		// then update the caption
		title.printf("Debug: %s - %s '%s'", device->machine().system().name, device->name(), device->tag());
		gtk_window_set_title(GTK_WINDOW(dmain->win), title);
		disasmview_update_checks(dmain);
	}
}


//============================================================
//  configuration_load
//============================================================

static void configuration_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *wnode;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* configuration load */
	int i = 0;
	for (wnode = xml_get_sibling(parentnode->child, "window"); wnode != NULL; wnode = xml_get_sibling(wnode->next, "window"))
	{
		windowStateArray[i].loadFromXmlDataNode(wnode);
		i++;
	}
	windowStateCount = i;
}

//============================================================
//  configuration_save
//============================================================

static void configuration_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// Loop over all the nodes
	for(win_i *p = win_list; p != NULL; p = p->next)
	{
		/* create a node */
		xml_data_node *debugger_node;
		debugger_node = xml_add_child(parentnode, "window", NULL);

		xml_set_attribute_int(debugger_node, "type", p->type);

		if (debugger_node != NULL)
		{
			int x, y;
			gtk_window_get_position(GTK_WINDOW(p->win), &x, &y);
			xml_set_attribute_int(debugger_node, "position_x", x);
			xml_set_attribute_int(debugger_node, "position_y", y);

			gtk_window_get_size(GTK_WINDOW(p->win), &x, &y);
			xml_set_attribute_int(debugger_node, "size_x", x);
			xml_set_attribute_int(debugger_node, "size_y", y);
		}
	}
}


//============================================================
//  sdl_osd_interface::init_debugger
//============================================================

void sdl_osd_interface::init_debugger()
{
	/* register callbacks */
	config_register(machine(), "debugger", config_saveload_delegate(FUNC(configuration_load), &machine()), config_saveload_delegate(FUNC(configuration_save), &machine()));
}


//============================================================
//  wait_for_debugger
//============================================================

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);

	// Create a console window if one doesn't already exist
	if(!dmain)
	{
		// GTK init should probably be done earlier
		gtk_init(0, 0);
		debugmain_init(machine());

		// Resize the main window
		for (int i = 0; i < windowStateCount; i++)
		{
			if (windowStateArray[i].type == WIN_TYPE_MAIN)
			{
				gtk_window_move(GTK_WINDOW(win_list->win), windowStateArray[i].positionX, windowStateArray[i].positionY);
				gtk_window_resize(GTK_WINDOW(win_list->win), windowStateArray[i].sizeX, windowStateArray[i].sizeY);
			}
		}

		// Respawn and reposition every other window
		for (int i = 0; i < windowStateCount; i++)
		{
			if (!windowStateArray[i].type || windowStateArray[i].type == WIN_TYPE_MAIN)
				continue;

			switch (windowStateArray[i].type)
			{
				case WIN_TYPE_MEMORY: memorywin_new(machine()); break;
				case WIN_TYPE_DISASM: disasmwin_new(machine()); break;
				case WIN_TYPE_LOG:    logwin_new(machine()); break;
				default: break;
			}

			gtk_window_move(GTK_WINDOW(win_list->win), windowStateArray[i].positionX, windowStateArray[i].positionY);
			gtk_window_resize(GTK_WINDOW(win_list->win), windowStateArray[i].sizeX, windowStateArray[i].sizeY);
		}

		// Bring focus back to the main window
		gtk_window_present(GTK_WINDOW(get_first_win_i(WIN_TYPE_MAIN)->win));
	}

	// update the views in the console to reflect the current CPU
	debugmain_set_cpu(&device);

	debugwin_show(1);
	gtk_main_iteration();
}


//============================================================
//  debugwin_update_during_game
//============================================================

void debugwin_update_during_game(running_machine &machine)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);
	if(dmain)
	{
		gtk_main_iteration_do(FALSE);
	}
}


//============================================================
//  debugmain_process_string
//============================================================

static void debugmain_process_string(win_i *win, const char *str)
{
	if(!str[0])
		debug_cpu_get_visible_cpu(*win->machine)->debug()->single_step();
	else
		debug_console_execute_command(*win->machine, str, 1);
}



//============================================================
//  debugmain_destroy
//============================================================

static void debugmain_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);

	dmain->machine->schedule_exit();
}



//============================================================
//  debugmain_init
//============================================================

static void debugmain_init(running_machine &machine)
{
	win_i *dmain;

	dmain = add_win_i(machine, WIN_TYPE_MAIN);
	dmain->win = create_debugmain();

	dmain->views[0]   = DVIEW(lookup_widget(dmain->win, "console"));
	dmain->views[1]   = DVIEW(lookup_widget(dmain->win, "disasm"));
	dmain->views[2]   = DVIEW(lookup_widget(dmain->win, "registers"));

	dview_set_debug_view(dmain->views[0],   machine, DVT_CONSOLE);
	dview_set_debug_view(dmain->views[1],  machine, DVT_DISASSEMBLY);
	dview_set_debug_view(dmain->views[2], machine, DVT_STATE);

	edit_init(&dmain->ed, lookup_widget(dmain->win, "edit"), 0, 0, debugmain_process_string, dmain);

	/* set up disasm view */
	downcast<debug_view_disasm *>(dmain->views[1]->view)->set_expression("curpc");
//  debug_view_set_property_UINT32(dmain->disasm, DVP_DASM_TRACK_LIVE, 1);

	g_signal_connect(dmain->win, "destroy", G_CALLBACK(debugmain_destroy), dmain);

	gtk_widget_show_all(dmain->win);
}



//============================================================
//  memorywin_update_checks
//============================================================

static void memorywin_update_checks(win_i *info)
{
	DView *memory = get_view(info, DVT_MEMORY);
	debug_view_memory *memview = downcast<debug_view_memory *>(memory->view);
	int bpc = memview->bytes_per_chunk();
	bool rev = memview->reverse();
	bool pa = memview->physical();

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")), bpc == 1);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")), bpc == 2);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")), bpc == 4);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "physical_addresses")), pa);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "logical_addresses")), !pa);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse")), rev);
}



//============================================================
//  memorywin_zone_changed
//============================================================

static void memorywin_zone_changed(GtkComboBox *zone_w, win_i *mem)
{
	int sel = gtk_combo_box_get_active(zone_w);
	astring title;
	DView *memory = get_view(mem, DVT_MEMORY);

	assert(mem->type == WIN_TYPE_MEMORY);

	// update the source
	memory->view->set_source(*memory->view->source_list().by_index(sel));

	// change the checkmarks in the menu
	memorywin_update_checks(mem);

	// update the window title
	const debug_view_source *source = memory->view->source();
	title.printf("Memory: %s", source->name());
	gtk_window_set_title(GTK_WINDOW(mem->win), title);

}



//============================================================
//  memorywin_process_string
//============================================================

static void memorywin_process_string(win_i *win, const char *str)
{
	win_i *mem = (win_i *) win;
	DView *memory = get_view(mem, DVT_MEMORY);

	assert(mem->type == WIN_TYPE_MEMORY);

	downcast<debug_view_memory *>(memory->view)->set_expression(str);
}



//============================================================
//  memorywin_destroy
//============================================================

static void memorywin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *mem = (win_i *) user_data;

	remove_win_i(mem);
	edit_del(&mem->ed);
	osd_free(mem);
}



//============================================================
//  memorywin_new
//============================================================

static void memorywin_new(running_machine &machine)
{
	win_i *mem;
	int item; //, cursel;
	device_t *curcpu = debug_cpu_get_visible_cpu(machine);
	GtkComboBox *       zone_w;

	mem = add_win_i(machine, WIN_TYPE_MEMORY);
	mem->win = create_memorywin();

	mem->views[0] = DVIEW(lookup_widget(mem->win, "memoryview"));
	dview_set_debug_view(mem->views[0], machine, DVT_MEMORY);

	zone_w   = GTK_COMBO_BOX(lookup_widget(mem->win, "zone"));

	edit_init(&mem->ed, lookup_widget(mem->win, "edit"), "0", 1, memorywin_process_string, mem);

	downcast<debug_view_memory *>(mem->views[0]->view)->set_expression("0");

	// populate the combobox
//  cursel = 0;
	item = 0;

	for (const debug_view_source *source = mem->views[0]->view->source_list().head(); source != NULL; source = source->next())
	{
		gtk_combo_box_append_text(zone_w, source->name());
		item++;
	}
	const debug_view_source *source = mem->views[0]->view->source_list().match_device(curcpu);
	gtk_combo_box_set_active(zone_w, mem->views[0]->view->source_list().index(*source));
	mem->views[0]->view->set_source(*source);

	g_signal_connect(zone_w, "changed", G_CALLBACK(memorywin_zone_changed), mem);

	g_signal_connect(mem->win, "destroy", G_CALLBACK(memorywin_destroy), mem);
	gtk_widget_show_all(mem->win);
}



//============================================================
//  disasmwin_cpu_changed
//============================================================

static void disasmwin_cpu_changed(GtkComboBox *cpu_w, win_i *dis)
{
	astring title;
	DView *disasm = get_view(dis, DVT_DISASSEMBLY);

	disasm->view->set_source(*disasm->view->source_list().by_index(gtk_combo_box_get_active(cpu_w)));

	disasmview_update_checks(dis);

	const debug_view_source *source = disasm->view->source();
	title.printf("Disassembly: %s", source->name());
	gtk_window_set_title(GTK_WINDOW(dis->win), title);
}



//============================================================
//   disasmwin_process_string
//============================================================

static void disasmwin_process_string(win_i *win, const char *str)
{
	win_i *dis = (win_i *) win;
	DView *disasm = get_view(dis, DVT_DISASSEMBLY);

	assert(dis->type == WIN_TYPE_DISASM);

	downcast<debug_view_disasm *>(disasm->view)->set_expression(str);
}



//============================================================
//  disasmwin_destroy
//============================================================

static void disasmwin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *dis = (win_i *) user_data;

	remove_win_i(dis);
	edit_del(&dis->ed);
	osd_free(dis);
}



//============================================================
//  disasmwin_new
//============================================================

static void disasmwin_new(running_machine &machine)
{
	win_i *dis;
	int item; //, cursel;
	device_t *curcpu = debug_cpu_get_visible_cpu(machine);
	GtkComboBox         *cpu_w;
	astring title;

	dis = add_win_i(machine, WIN_TYPE_DISASM);
	dis->win = create_disasmwin();

	dis->views[0] = DVIEW(lookup_widget(dis->win, "disasmview"));

	dview_set_debug_view(dis->views[0], machine, DVT_DISASSEMBLY);

	cpu_w    = GTK_COMBO_BOX(lookup_widget(dis->win, "cpu"));

	edit_init(&dis->ed, lookup_widget(dis->win, "edit"), "curpc", 1, disasmwin_process_string, dis);

	downcast<debug_view_disasm *>(dis->views[0]->view)->set_expression("curpc");

	// populate the combobox
//  cursel = 0;
	item = 0;
	for (const debug_view_source *source = dis->views[0]->view->source_list().head(); source != NULL; source = source->next())
	{
		gtk_combo_box_append_text(cpu_w, source->name());
		item++;
	}
	const debug_view_source *source = dis->views[0]->view->source_list().match_device(curcpu);
	gtk_combo_box_set_active(cpu_w, dis->views[0]->view->source_list().index(*source));
	dis->views[0]->view->set_source(*source);

	title.printf("Disassembly: %s", source->name());
	gtk_window_set_title(GTK_WINDOW(dis->win), title);

	g_signal_connect(cpu_w, "changed", G_CALLBACK(disasmwin_cpu_changed), dis);

	//  g_signal_connect(dis->edit_w, "activate", G_CALLBACK(disasmwin_process_string), dis);
	g_signal_connect(dis->win, "destroy", G_CALLBACK(disasmwin_destroy), dis);
	gtk_widget_show_all(dis->win);
}



//============================================================
//  logwin_destroy
//============================================================

static void logwin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *log = (win_i *) user_data;

	remove_win_i(log);
	osd_free(log);
}



//============================================================
//  logwin_new
//============================================================

static void logwin_new(running_machine &machine)
{
	win_i *log;

	log = add_win_i(machine, WIN_TYPE_LOG);
	log->win = create_logwin();

	log->views[0] = DVIEW(lookup_widget(log->win, "logview"));
	dview_set_debug_view(log->views[0], machine, DVT_LOG);

	g_signal_connect(log->win, "destroy", G_CALLBACK(logwin_destroy), log);

	gtk_widget_show_all(log->win);
}

//============================================================
//  Menu Callbacks
//============================================================

void on_raw_opcodes_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")))) {
		downcast<debug_view_disasm *>(disasm->view)->set_right_column(DASM_RIGHTCOL_RAW);
	}
}

void on_enc_opcodes_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")))) {
		downcast<debug_view_disasm *>(disasm->view)->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
	}
}

void on_comments_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")))) {
		downcast<debug_view_disasm *>(disasm->view)->set_right_column(DASM_RIGHTCOL_COMMENTS);
	}
}


void on_new_mem_activate(GtkWidget *win)
{
	memorywin_new(*get_running_machine(win));
}

void on_new_disasm_activate(GtkWidget *win)
{
	disasmwin_new(*get_running_machine(win));
}

void on_new_errorlog_activate(GtkWidget *win)
{
	logwin_new(*get_running_machine(win));
}

void on_run_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go();
}

void on_run_h_activate(GtkWidget *win)
{
	debugwin_show(0);
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go();
}

void on_run_cpu_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go_next_device();
}

void on_run_irq_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go_interrupt();
}

void on_run_vbl_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go_vblank();
}

void on_step_into_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->single_step();
}

void on_step_over_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->single_step_over();
}

void on_step_out_activate(GtkWidget *win)
{
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->single_step_out();
}

void on_hard_reset_activate(GtkWidget *win)
{
	get_running_machine(win)->schedule_hard_reset();
}

void on_soft_reset_activate(GtkWidget *win)
{
	get_running_machine(win)->schedule_soft_reset();
	debug_cpu_get_visible_cpu(*get_running_machine(win))->debug()->go();
}

void on_exit_activate(GtkWidget *win)
{
	get_running_machine(win)->schedule_exit();
}

void on_chunks_1_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")))) {
		downcast<debug_view_memory *>(memory->view)->set_bytes_per_chunk(1);
	}
}

void on_chunks_2_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")))) {
		downcast<debug_view_memory *>(memory->view)->set_bytes_per_chunk(2);
	}
}

void on_chunks_4_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")))) {
		downcast<debug_view_memory *>(memory->view)->set_bytes_per_chunk(4);
	}
}

void on_logical_addresses_group_changed(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "logical_addresses")))) {
		downcast<debug_view_memory *>(memory->view)->set_physical(false);
	}
}

void on_physical_addresses_group_changed(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "physical_addresses")))) {
		downcast<debug_view_memory *>(memory->view)->set_physical(true);
	}
}

void on_reverse_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	downcast<debug_view_memory *>(memory->view)->set_reverse(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse"))));
}

void on_ibpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	debug_view_memory *memview = downcast<debug_view_memory *>(memory->view);
	memview->set_chunks_per_row(memview->chunks_per_row() + 1);
}

void on_dbpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	debug_view_memory *memview = downcast<debug_view_memory *>(memory->view);
	memview->set_chunks_per_row(memview->chunks_per_row() - 1);
}

void on_run_to_cursor_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_ALL);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);
	astring command;

	if (disasm->view->cursor_visible())
	{
		if (debug_cpu_get_visible_cpu(*info->machine) == disasm->view->source()->device())
		{
			offs_t address = downcast<debug_view_disasm *>(disasm->view)->selected_address();
			command.printf("go 0x%X", address);
			debug_console_execute_command(*info->machine, command, 1);
		}
	}
}

void
on_set_breakpoint_at_cursor_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_ALL);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);
	astring command;

	if (disasm->view->cursor_visible())
	{
		if (debug_cpu_get_visible_cpu(*info->machine) == disasm->view->source()->device())
		{
			offs_t address = downcast<debug_view_disasm *>(disasm->view)->selected_address();
			device_debug *cpuinfo = disasm->view->source()->device()->debug();
			device_debug::breakpoint *bp;
			INT32 bpindex = -1;

			/* first find an existing breakpoint at this address */
			for (bp = cpuinfo->breakpoint_first(); bp != NULL; bp = bp->next())
				if (address == bp->address())
				{
					bpindex = bp->index();
					break;
				}

			/* if it doesn't exist, add a new one */
			if (bpindex == -1)
				command.printf("bpset 0x%X", address);
			else
				command.printf("bpclear 0x%X", bpindex);
			debug_console_execute_command(*info->machine, command, 1);
		}
	}
}

gboolean
on_disasm_button_press_event(GtkWidget       *widget,
								GdkEventButton  *event,
								gpointer         user_data)
{
	DView *info = (DView *) widget;
	LOG(("gef %f %f %p %p\n", event->x, event->y, info, widget));
	if (info->view->cursor_supported())
	{
		DViewClass *dvc = DVIEW_GET_CLASS(info);
		debug_view_xy topleft = info->view->visible_position();
		debug_view_xy newpos;
		newpos.x = topleft.x + event->x / dvc->fixedfont_width;
		newpos.y = topleft.y + event->y / dvc->fixedfont_height;
		info->view->set_cursor_position(newpos);
		info->view->set_cursor_visible(true);
		gtk_widget_grab_focus(widget);
		//SetFocus(wnd);
	}
	return true;
}

gboolean
on_memoryview_button_press_event(GtkWidget       *widget,
									GdkEventButton  *event,
									gpointer         user_data)
{
	return on_disasm_button_press_event(widget, event, user_data);
}

gboolean
on_memoryview_key_press_event(GtkWidget   *widget,
								GdkEventKey *event,
								gpointer     user_data)
{
	DView *info = (DView *) widget;
	//printf("%s\n", event->string);
	//printf("The name of this keysym is `%s'\n",
	//          gdk_keyval_name(event->keyval));
	//if (/*waiting_for_debugger ||*/ !debugwin_seq_pressed(info->view-> owner->machine()))
	switch (event->keyval)
	{
		case GDK_Up:
			info->view->process_char(DCH_UP);
			break;

		case GDK_Down:
			info->view->process_char(DCH_DOWN);
			break;

		case GDK_Left:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				info->view->process_char(DCH_CTRLLEFT);
			else
				info->view->process_char(DCH_LEFT);
			break;

		case GDK_Right:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				info->view->process_char(DCH_CTRLRIGHT);
			else
				info->view->process_char(DCH_RIGHT);
			break;

		case GDK_Prior:
			info->view->process_char(DCH_PUP);
			break;

		case GDK_Next:
			info->view->process_char(DCH_PDOWN);
			break;

		case GDK_Home:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				info->view->process_char(DCH_CTRLHOME);
			else
				info->view->process_char(DCH_HOME);
			break;

		case GDK_End:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				info->view->process_char(DCH_CTRLEND);
			else
				info->view->process_char(DCH_END);
			break;

		case GDK_Escape:
			//if (info->owner->focuswnd != NULL)
			//  SetFocus(info->owner->focuswnd);
			//info->owner->ignore_char_lparam = lparam >> 16;
			break;

		case GDK_Tab:
			//if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			//  debugwin_view_prev_view(info->owner, info);
			//else
			//  debugwin_view_next_view(info->view, dvw);
			break;
		default:
			if (event->keyval >= 32 && event->keyval < 127 && info->view->cursor_supported())
				info->view->process_char(event->keyval);
	}

	return true;
}

#else

#include "sdlinc.h"

#include "emu.h"
#include "osdepend.h"
#include "osdsdl.h"

	// win32 stubs for linking
void sdl_osd_interface::init_debugger()
{
}

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
}

// win32 stubs for linking
void debugwin_update_during_game(running_machine &machine)
{
}

#endif
