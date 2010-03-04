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
#include "debug-intf.h"
#include "debug-sup.h"
#include "debug-cb.h"

//============================================================
//  PARAMETERS
//============================================================

#if 1
#define LOG(x...)
#else
#define LOG(x) printf x
#endif

#define MAX_VIEWS		(4)

enum {
	WIN_TYPE_MAIN		= 0x01,
	WIN_TYPE_MEMORY 	= 0x02,
	WIN_TYPE_DISASM 	= 0x04,
	WIN_TYPE_LOG		= 0x08,
	WIN_TYPE_ALL		= 0x0f
};

//============================================================
//  TYPES
//============================================================


typedef struct hentry {
	struct hentry *h;
	char *e;
} hentry;

typedef struct _win_i win_i;

typedef struct _edit edit;
struct _edit {
	GtkEntry *edit_w;
	struct hentry *h, *ch;
	char *hold;
	int keep_last;
	void (*cb)(win_i *,const char *);
	win_i *cbp;
};

struct _win_i {
	int 					type;
	win_i *					next;
	GtkWidget *				win;
	edit					ed;
	running_machine *		machine;	// machine
	DView *					views[MAX_VIEWS];
	running_device *		cpu;	// current CPU
};


//============================================================
//  LOCAL VARIABLES
//============================================================

static win_i *win_list;

//============================================================
//  PROTOTYPES
//============================================================

static void debugmain_init(running_machine *machine);


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

static win_i *add_win_i(running_machine *machine, int win_type)
{
	win_i *win = (win_i *) osd_malloc(sizeof(*win));
	memset(win, 0, sizeof(*win));
	win->cpu = NULL;
	win->machine = machine;
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
	assert(info->type == WIN_TYPE_MAIN);

	disasm = get_view(info, DVT_DISASSEMBLY);

	int rc = disasm_view_get_right_column(disasm->view);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}



//============================================================
//  debugmain_set_cpu
//============================================================

static void debugmain_set_cpu(running_device *cpu)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);
	DView *dv;

	if (cpu != dmain->cpu)
	{
		char title[256];
		const registers_subview_item *regsubitem;
		const disasm_subview_item *dasmsubitem;

		dmain->cpu = cpu;

		// first set all the views to the new cpu
		// FIXME: Iterate over all views !
		dv = get_view(dmain, DVT_DISASSEMBLY);
		for (dasmsubitem = disasm_view_get_subview_list(dv->view); dasmsubitem != NULL; dasmsubitem = dasmsubitem->next)
			if (dasmsubitem->space->cpu == cpu)
			{
				disasm_view_set_subview(dv->view, dasmsubitem->index);
				break;
			}

		dv = get_view(dmain, DVT_REGISTERS);
		for (regsubitem = registers_view_get_subview_list(dv->view); regsubitem != NULL; regsubitem = regsubitem->next)
			if (regsubitem->device == cpu)
			{
				registers_view_set_subview(dv->view, regsubitem->index);
				break;
			}

		// then update the caption
		snprintf(title, ARRAY_LENGTH(title), "Debug: %s - %s", cpu->machine->gamedrv->name, regsubitem->name.cstr());
		gtk_window_set_title(GTK_WINDOW(dmain->win), title);
		disasmview_update_checks(dmain);
	}
}



//============================================================
//  osd_wait_for_debugger
//============================================================

void osd_wait_for_debugger(running_device *device, int firststop)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);
	// create a console window
	if(!dmain)
	{
		// GTK init should probably be done earlier
		gtk_init(0, 0);
		debugmain_init(device->machine);
	}

	// update the views in the console to reflect the current CPU
	debugmain_set_cpu(device);

	debugwin_show(1);
	gtk_main_iteration();
}



//============================================================
//  debugwin_update_during_game
//============================================================

void debugwin_update_during_game(running_machine *machine)
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
		debug_cpu_single_step(win->machine, 1);
	else
		debug_console_execute_command(win->machine, str, 1);
}



//============================================================
//  debugmain_destroy
//============================================================

static void debugmain_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *dmain = get_first_win_i(WIN_TYPE_MAIN);

	mame_schedule_exit(dmain->machine);
}



//============================================================
//  debugmain_init
//============================================================

static void debugmain_init(running_machine *machine)
{
	win_i *dmain;

	dmain = add_win_i(machine, WIN_TYPE_MAIN);
	dmain->win = create_debugmain();

	dmain->views[0]	  = DVIEW(lookup_widget(dmain->win, "console"));
	dmain->views[1]   = DVIEW(lookup_widget(dmain->win, "disasm"));
	dmain->views[2]   = DVIEW(lookup_widget(dmain->win, "registers"));

	dview_set_debug_view(dmain->views[0],   machine, DVT_CONSOLE);
	dview_set_debug_view(dmain->views[1],  machine, DVT_DISASSEMBLY);
	dview_set_debug_view(dmain->views[2], machine, DVT_REGISTERS);

	edit_init(&dmain->ed, lookup_widget(dmain->win, "edit"), 0, 0, debugmain_process_string, dmain);

	/* set up disasm view */
	debug_view_begin_update(dmain->views[1]->view);
	disasm_view_set_expression(dmain->views[1]->view, "curpc");
//  debug_view_set_property_UINT32(dmain->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dmain->views[1]->view);

	g_signal_connect(dmain->win, "destroy", G_CALLBACK(debugmain_destroy), dmain);

	gtk_widget_show_all(dmain->win);
}



//============================================================
//  memorywin_update_checks
//============================================================

static void memorywin_update_checks(win_i *info)
{
	DView *memory = get_view(info, DVT_MEMORY);
	int bpc = memory_view_get_bytes_per_chunk(memory->view);
	int rev = memory_view_get_reverse(memory->view);
	int pa = memory_view_get_physical(memory->view);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")), bpc == 1);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")), bpc == 2);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")), bpc == 4);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "physical_addresses")), pa);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "locical_addresses")), !pa);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse")), rev);
}



//============================================================
//  memorywin_zone_changed
//============================================================

static void memorywin_zone_changed(GtkComboBox *zone_w, win_i *mem)
{
	int sel = gtk_combo_box_get_active(zone_w);
	char title[256];
	const memory_subview_item *subview;
	DView *memory = get_view(mem, DVT_MEMORY);

	assert(mem->type == WIN_TYPE_MEMORY);

	// update the subview
	memory_view_set_subview(memory->view, sel);

	// change the checkmarks in the menu
	memorywin_update_checks(mem);

	// update the window title
	subview = memory_view_get_current_subview(memory->view);
	sprintf(title, "Memory: %s", subview->name.cstr());
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

	memory_view_set_expression(memory->view, str);
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

static void memorywin_new(running_machine *machine)
{
	win_i *mem;
	int item, cursel;
	running_device *curcpu = debug_cpu_get_visible_cpu(machine);
	const memory_subview_item *subview;
	GtkComboBox *		zone_w;

	mem = add_win_i(machine, WIN_TYPE_MEMORY);
	mem->win = create_memorywin();

	mem->views[0] = DVIEW(lookup_widget(mem->win, "memoryview"));
	dview_set_debug_view(mem->views[0], machine, DVT_MEMORY);

	zone_w   = GTK_COMBO_BOX(lookup_widget(mem->win, "zone"));

	edit_init(&mem->ed, lookup_widget(mem->win, "edit"), "0", 1, memorywin_process_string, mem);

	debug_view_begin_update(mem->views[0]->view);
	memory_view_set_expression(mem->views[0]->view, "0");
	debug_view_end_update(mem->views[0]->view);

	// populate the combobox
	cursel = item = 0;

	for (subview = memory_view_get_subview_list(mem->views[0]->view); subview != NULL; subview = subview->next)
	{
		gtk_combo_box_append_text(zone_w, subview->name);
		if (cursel == 0 && subview->space != NULL && subview->space->cpu == curcpu)
			cursel = item;

		item++;
	}

	memory_view_set_subview(mem->views[0]->view, cursel);
	gtk_combo_box_set_active(zone_w, cursel);

	g_signal_connect(zone_w, "changed", G_CALLBACK(memorywin_zone_changed), mem);

	g_signal_connect(mem->win, "destroy", G_CALLBACK(memorywin_destroy), mem);
	gtk_widget_show_all(mem->win);
}



//============================================================
//  disasmwin_cpu_changed
//============================================================

static void disasmwin_cpu_changed(GtkComboBox *cpu_w, win_i *dis)
{
	char title[256];
	const disasm_subview_item *subview;
	DView *disasm = get_view(dis, DVT_DISASSEMBLY);

	disasm_view_set_subview(disasm->view, gtk_combo_box_get_active(cpu_w));

	disasmview_update_checks(dis);

	subview = disasm_view_get_current_subview(disasm->view);
	sprintf(title, "Disassembly: %s", subview->name.cstr());
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

	disasm_view_set_expression(disasm->view, str);
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

static void disasmwin_new(running_machine *machine)
{
	win_i *dis;
	int item, cursel;
	running_device *curcpu = debug_cpu_get_visible_cpu(machine);
	const disasm_subview_item *subview;
	GtkComboBox 		*cpu_w;
	char title[256];

	dis = add_win_i(machine, WIN_TYPE_DISASM);
	dis->win = create_disasmwin();

	dis->views[0] = DVIEW(lookup_widget(dis->win, "disasmview"));

	dview_set_debug_view(dis->views[0], machine, DVT_DISASSEMBLY);

	cpu_w    = GTK_COMBO_BOX(lookup_widget(dis->win, "cpu"));

	edit_init(&dis->ed, lookup_widget(dis->win, "edit"), "curpc", 1, disasmwin_process_string, dis);

	debug_view_begin_update(dis->views[0]->view);
	disasm_view_set_expression(dis->views[0]->view, "curpc");
//  debug_view_set_property_UINT32(dis->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dis->views[0]->view);

	// populate the combobox
	cursel = item = 0;
	for (subview = disasm_view_get_subview_list(dis->views[0]->view); subview != NULL; subview = subview->next)
	{
		gtk_combo_box_append_text(cpu_w, subview->name);
		if (cursel == 0 && subview->space->cpu == curcpu)
			cursel = item;

		item++;
	}

	gtk_combo_box_set_active(cpu_w, cursel);
	disasm_view_set_subview(dis->views[0]->view, cursel);

	subview = disasm_view_get_current_subview(dis->views[0]->view);
	sprintf(title, "Disassembly: %s", subview->name.cstr());
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

static void logwin_new(running_machine *machine)
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
		debug_view_begin_update(disasm->view);
		disasm_view_set_right_column(disasm->view, DASM_RIGHTCOL_RAW);
		debug_view_end_update(disasm->view);
	}
}

void on_enc_opcodes_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")))) {
		debug_view_begin_update(disasm->view);
		disasm_view_set_right_column(disasm->view, DASM_RIGHTCOL_ENCRYPTED);
		debug_view_end_update(disasm->view);
	}
}

void on_comments_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")))) {
		debug_view_begin_update(disasm->view);
		disasm_view_set_right_column(disasm->view, DASM_RIGHTCOL_COMMENTS);
		debug_view_end_update(disasm->view);
	}
}


void on_new_mem_activate(GtkWidget *win)
{
	memorywin_new(get_running_machine(win));
}

void on_new_disasm_activate(GtkWidget *win)
{
	disasmwin_new(get_running_machine(win));
}

void on_new_errorlog_activate(GtkWidget *win)
{
	logwin_new(get_running_machine(win));
}

void on_run_activate(GtkWidget *win)
{
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_run_h_activate(GtkWidget *win)
{
	debugwin_show(0);
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_run_cpu_activate(GtkWidget *win)
{
	debug_cpu_next_cpu(get_running_machine(win));
}

void on_run_irq_activate(GtkWidget *win)
{
	debug_cpu_go_interrupt(get_running_machine(win), -1);
}

void on_run_vbl_activate(GtkWidget *win)
{
	debug_cpu_go_vblank(get_running_machine(win));
}

void on_step_into_activate(GtkWidget *win)
{
	debug_cpu_single_step(get_running_machine(win), 1);
}

void on_step_over_activate(GtkWidget *win)
{
	debug_cpu_single_step_over(get_running_machine(win), 1);
}

void on_step_out_activate(GtkWidget *win)
{
	debug_cpu_single_step_out(get_running_machine(win));
}

void on_hard_reset_activate(GtkWidget *win)
{
	mame_schedule_hard_reset(get_running_machine(win));
}

void on_soft_reset_activate(GtkWidget *win)
{
	mame_schedule_soft_reset(get_running_machine(win));
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_exit_activate(GtkWidget *win)
{
	mame_schedule_exit(get_running_machine(win));
}

void on_chunks_1_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")))) {
		debug_view_begin_update(memory->view);
		memory_view_set_bytes_per_chunk(memory->view, 1);
		debug_view_end_update(memory->view);
	}
}

void on_chunks_2_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")))) {
		debug_view_begin_update(memory->view);
		memory_view_set_bytes_per_chunk(memory->view, 2);
		debug_view_end_update(memory->view);
	}
}

void on_chunks_4_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")))) {
		debug_view_begin_update(memory->view);
		memory_view_set_bytes_per_chunk(memory->view, 4);
		debug_view_end_update(memory->view);
	}
}

void on_logical_addresses_group_changed(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "logical_addresses")))) {
		debug_view_begin_update(memory->view);
		memory_view_set_physical(memory->view, FALSE);
		debug_view_end_update(memory->view);
	}
}

void on_physical_addresses_group_changed(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "physical_addresses")))) {
		debug_view_begin_update(memory->view);
		memory_view_set_physical(memory->view, TRUE);
		debug_view_end_update(memory->view);
	}
}

void on_reverse_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	debug_view_begin_update(memory->view);
	memory_view_set_reverse(memory->view, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse"))));
	debug_view_end_update(memory->view);
}

void on_ibpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	debug_view_begin_update(memory->view);
	memory_view_set_chunks_per_row(memory->view, memory_view_get_chunks_per_row(memory->view) + 1);
	debug_view_end_update(memory->view);
}

void on_dbpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	DView *memory = get_view(info, DVT_MEMORY);

	debug_view_begin_update(memory->view);
	memory_view_set_chunks_per_row(memory->view, memory_view_get_chunks_per_row(memory->view) - 1);
	debug_view_end_update(memory->view);
}

void on_run_to_cursor_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_ALL);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);
	char command[64];

	if (debug_view_get_cursor_visible(disasm->view))
	{
		const address_space *space = disasm_view_get_current_subview(disasm->view)->space;
		if (debug_cpu_get_visible_cpu(info->machine) == space->cpu)
		{
			offs_t address = disasm_view_get_selected_address(disasm->view);
			sprintf(command, "go %X", address);
			debug_console_execute_command(info->machine, command, 1);
		}
	}
}

void
on_set_breakpoint_at_cursor_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_ALL);
	DView *disasm = get_view(info, DVT_DISASSEMBLY);
	char command[64];

	if (debug_view_get_cursor_visible(disasm->view))
	{
		const address_space *space = disasm_view_get_current_subview(disasm->view)->space;
		if (debug_cpu_get_visible_cpu(info->machine) == space->cpu)
		{
			offs_t address = disasm_view_get_selected_address(disasm->view);
			cpu_debug_data *cpuinfo = cpu_get_debug_data(space->cpu);
			debug_cpu_breakpoint *bp;
			INT32 bpindex = -1;

			/* first find an existing breakpoint at this address */
			for (bp = cpuinfo->bplist; bp != NULL; bp = bp->next)
				if (address == bp->address)
				{
					bpindex = bp->index;
					break;
				}

			/* if it doesn't exist, add a new one */
			if (bpindex == -1)
				sprintf(command, "bpset 0x%X", address);
			else
				sprintf(command, "bpclear 0x%X", bpindex);
			debug_console_execute_command(info->machine, command, 1);
		}
	}
}

gboolean
on_disasm_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	DView *info = (DView *) widget;
	LOG(("gef %f %f %p %p\n", event->x, event->y, info, widget));
	if (debug_view_get_cursor_supported(info->view))
	{
		DViewClass *dvc = DVIEW_GET_CLASS(info);
		debug_view_xy topleft = debug_view_get_visible_position(info->view);
		debug_view_xy newpos;
		newpos.x = topleft.x + event->x / dvc->fixedfont_width;
		newpos.y = topleft.y + event->y / dvc->fixedfont_height;
		debug_view_set_cursor_position(info->view, newpos);
		debug_view_set_cursor_visible(info->view, TRUE);
		gtk_widget_grab_focus(widget);
		//SetFocus(wnd);
	}
	return true;
}

gboolean
on_memoryview_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	return on_disasm_button_press_event(widget, event, user_data);
}

gboolean
on_memoryview_key_press_event             (GtkWidget   *widget,
                                                        GdkEventKey *event,
                                                        gpointer     user_data)
{
	DView *info = (DView *) widget;
	//printf("%s\n", event->string);
	//printf("The name of this keysym is `%s'\n",
	//          gdk_keyval_name(event->keyval));
	//if (/*waiting_for_debugger ||*/ !debugwin_seq_pressed(info->view-> owner->machine))
	switch (event->keyval)
	{
		case GDK_Up:
			debug_view_type_character(info->view, DCH_UP);
			break;

		case GDK_Down:
			debug_view_type_character(info->view, DCH_DOWN);
			break;

		case GDK_Left:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLLEFT);
			else
				debug_view_type_character(info->view, DCH_LEFT);
			break;

		case GDK_Right:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLRIGHT);
			else
				debug_view_type_character(info->view, DCH_RIGHT);
			break;

		case GDK_Prior:
			debug_view_type_character(info->view, DCH_PUP);
			break;

		case GDK_Next:
			debug_view_type_character(info->view, DCH_PDOWN);
			break;

		case GDK_Home:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLHOME);
			else
				debug_view_type_character(info->view, DCH_HOME);
			break;

		case GDK_End:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLEND);
			else
				debug_view_type_character(info->view, DCH_END);
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
			if (event->keyval >= 32 && event->keyval < 127 && debug_view_get_cursor_supported(info->view))
				debug_view_type_character(info->view, event->keyval);
	}

	return true;
}

#else

#include "emu.h"

// win32 stubs for linking
void osd_wait_for_debugger(running_device *device, int firststop)
{
}

void debugwin_update_during_game(running_machine *machine)
{
}

#endif
