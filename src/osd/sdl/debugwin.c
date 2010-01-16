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

typedef struct hentry {
	struct hentry *h;
	char *e;
} hentry;

typedef struct {
	GtkEntry *edit_w;
	struct hentry *h, *ch;
	char *hold;
	int keep_last;
	void (*cb)(running_machine *,const char *, void *);
	void *cbp;
	running_machine *machine;
} edit;

typedef struct {
	GtkWidget *win;
	DView *console_w, *disasm_w, *registers_w;
	edit ed;
	debug_view *console;
	debug_view *disasm;
	debug_view *registers;
	const device_config *cpu;	// current CPU
	running_machine *machine;	// machine
} debugmain_i;

typedef struct memorywin_i {
	struct memorywin_i *next;
	GtkWidget *win;
	DView *memory_w;
	edit ed;
	GtkComboBox *zone_w;
	debug_view *memory;
	running_machine *machine;

} memorywin_i;

typedef struct disasmwin_i {
	struct disasmwin_i *next;
	GtkWidget *win;
	DView *disasm_w;
	edit ed;
	GtkComboBox *cpu_w;
	debug_view *disasm;
	running_machine *machine;
} disasmwin_i;

typedef struct logwin_i {
	struct logwin_i *next;
	GtkWidget *win;
	DView *log_w;
	debug_view *log;
	running_machine *machine;
} logwin_i;

typedef struct memorycombo_item
{
	struct memorycombo_item *next;
	char					name[256];
	UINT8					prefsize;
	running_machine 			*machine;	// machine
	const memory_subview_item		*subview;	// subview
	const device_config			*device;	// CPU device
} memorycombo_item;


static debugmain_i *dmain;
static memorywin_i *memorywin_list;
static disasmwin_i *disasmwin_list;
static logwin_i    *logwin_list;

static memorycombo_item *memorycombo;

static void debugmain_init(running_machine *machine);

static void debugwin_show(int show)
{
	memorywin_i *p1;
	disasmwin_i *p2;
	logwin_i *p3;
	void (*f)(GtkWidget *widget) = show ? gtk_widget_show : gtk_widget_hide;
	if(dmain) {
		f(dmain->win);
		//      dview_set_updatable(dmain->console_w, show);
	}
	for(p1 = memorywin_list; p1; p1 = p1->next)
		f(p1->win);
	for(p2 = disasmwin_list; p2; p2 = p2->next)
		f(p2->win);
	for(p3 = logwin_list; p3; p3 = p3->next)
		f(p3->win);
}

static void edit_add_hist(edit *e, const char *text)
{
	if(e->ch)
		e->ch = 0;

	if(e->hold) {
		free(e->hold);
		e->hold =0;
	}

	if(!e->h || (text[0] && strcmp(text, e->h->e))) {
		hentry *h = (hentry *) malloc(sizeof(hentry));
		h->h = e->h;
		h->e = mame_strdup(text);
		e->h = h;
	}
}

static void edit_set_field(edit *e)
{
	if(e->keep_last) {
		gtk_entry_set_text(e->edit_w, e->h->e);
		gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
	} else
		gtk_entry_set_text(e->edit_w, "");
}

static void edit_activate(GtkEntry *item, gpointer user_data)
{
	edit *e = (edit *) user_data;
	const char *text = gtk_entry_get_text(e->edit_w);
	edit_add_hist(e, text);
	e->cb(e->machine, text, e->cbp);
	edit_set_field(e);

}

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
		free(e->hold);
		e->hold = 0;
		e->ch = 0;
	} else {
		gtk_entry_set_text(e->edit_w, "");
		e->ch = 0;
	}
	gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
}

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

static void edit_init(running_machine *machine, edit *e, GtkWidget *w, const char *ft, int kl, void (*cb)(running_machine *, const char *, void *), void *cbp)
{
	e->edit_w = GTK_ENTRY(w);
	e->h = 0;
	e->ch = 0;
	e->hold = 0;
	e->keep_last = kl;
	e->cb = cb;
	e->cbp = cbp;
	e->machine = machine;
	if(ft)
		edit_add_hist(e, ft);
	edit_set_field(e);
	g_signal_connect(e->edit_w, "activate", G_CALLBACK(edit_activate), e);
	g_signal_connect(e->edit_w, "key-press-event", G_CALLBACK(edit_key), e);
}

static void edit_del(edit *e)
{
	hentry *h = e->h;
	while(h) {
		hentry *he = h->h;
		free(h->e);
		free(h);
		h = he;
	}
}

static void debugmain_update_checks(debugmain_i *info)
{
	int rc = disasm_view_get_right_column(info->disasm);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}

void debugmain_raw_opcodes_activate(GtkMenuItem *item, gpointer user_data)
{
	debugmain_i *info = (debugmain_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm, DASM_RIGHTCOL_RAW);
		debug_view_end_update(info->disasm);
	}
}

void debugmain_enc_opcodes_activate(GtkMenuItem *item, gpointer user_data)
{
	debugmain_i *info = (debugmain_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm, DASM_RIGHTCOL_ENCRYPTED);
		debug_view_end_update(info->disasm);
	}
}

void debugmain_comments_activate(GtkMenuItem *item, gpointer user_data)
{
	debugmain_i *info = (debugmain_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm, DASM_RIGHTCOL_COMMENTS);
		debug_view_end_update(info->disasm);
	}
}

static void debugmain_set_cpu(const device_config *cpu)
{
	if (cpu != dmain->cpu)
	{
		char title[256];
		const registers_subview_item *regsubitem;
		const disasm_subview_item *dasmsubitem;

		dmain->cpu = cpu;

		// first set all the views to the new cpu
		for (dasmsubitem = disasm_view_get_subview_list(dmain->disasm); dasmsubitem != NULL; dasmsubitem = dasmsubitem->next)
			if (dasmsubitem->space->cpu == cpu)
			{
				disasm_view_set_subview(dmain->disasm, dasmsubitem->index);
				break;
			}

		for (regsubitem = registers_view_get_subview_list(dmain->registers); regsubitem != NULL; regsubitem = regsubitem->next)
			if (regsubitem->device == cpu)
			{
				registers_view_set_subview(dmain->registers, regsubitem->index);
				break;
			}

		// then update the caption
		snprintf(title, ARRAY_LENGTH(title), "Debug: %s - %s", cpu->machine->gamedrv->name, regsubitem->name);
		gtk_window_set_title(GTK_WINDOW(dmain->win), title);
		debugmain_update_checks(dmain);
	}
}

// The entry point

void osd_wait_for_debugger(const device_config *device, int firststop)
{
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

void debugwin_update_during_game(running_machine *machine)
{
	if(dmain)
	{
		gtk_main_iteration_do(FALSE);
	}
}

static void debugmain_process_string(running_machine *machine, const char *str, void *dmp)
{
	if(!str[0])
		debug_cpu_single_step(machine, 1);
	else
		debug_console_execute_command(machine, str, 1);
}

static void debugmain_destroy(GtkObject *obj, gpointer user_data)
{
	debugmain_i *dmain = (debugmain_i *)user_data;

	mame_schedule_exit(dmain->machine);
}

static void debugmain_init(running_machine *machine)
{
	dmain = (debugmain_i *) malloc(sizeof(*dmain));
	memset(dmain, 0, sizeof(*dmain));
	dmain->win = create_debugmain(machine);
	dmain->cpu = NULL;
	dmain->machine = machine;

	dmain->console_w   = DVIEW(lookup_widget(dmain->win, "console"));
	dmain->disasm_w    = DVIEW(lookup_widget(dmain->win, "disasm"));
	dmain->registers_w = DVIEW(lookup_widget(dmain->win, "registers"));

	dview_set_debug_view(dmain->console_w,   machine, DVT_CONSOLE, &dmain->console);
	dview_set_debug_view(dmain->disasm_w,    machine, DVT_DISASSEMBLY, &dmain->disasm);
	dview_set_debug_view(dmain->registers_w, machine, DVT_REGISTERS, &dmain->registers);

	edit_init(machine, &dmain->ed, lookup_widget(dmain->win, "edit"), 0, 0, debugmain_process_string, &dmain);

	debug_view_begin_update(dmain->disasm);
	disasm_view_set_expression(dmain->disasm, "curpc");
//  debug_view_set_property_UINT32(dmain->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dmain->disasm);

	g_signal_connect(dmain->win, "destroy", G_CALLBACK(debugmain_destroy), dmain);
	g_signal_connect(lookup_widget(dmain->win, "raw_opcodes"), "activate", G_CALLBACK(debugmain_raw_opcodes_activate), dmain);
	g_signal_connect(lookup_widget(dmain->win, "enc_opcodes"), "activate", G_CALLBACK(debugmain_enc_opcodes_activate), dmain);
	g_signal_connect(lookup_widget(dmain->win, "comments"),    "activate", G_CALLBACK(debugmain_comments_activate), dmain);

	gtk_widget_show_all(dmain->win);
}

static void memorywin_update_checks(memorywin_i *info)
{
	int bpc = memory_view_get_bytes_per_chunk(info->memory);
	int rev = memory_view_get_reverse(info->memory);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")), bpc == 1);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")), bpc == 2);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")), bpc == 4);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse")), rev);
}

static void memorywin_zone_changed(GtkComboBox *zone_w, memorywin_i *mem)
{
	int sel = gtk_combo_box_get_active(mem->zone_w);
	char title[256];
	const memory_subview_item *subview;

	// update the subview
	memory_view_set_subview(mem->memory, sel);

	// change the checkmarks in the menu
	memorywin_update_checks(mem);

	// update the window title
	subview = memory_view_get_current_subview(mem->memory);
	sprintf(title, "Memory: %s", subview->name);
	gtk_window_set_title(GTK_WINDOW(mem->win), title);

}

static void memorywin_process_string(running_machine *machine, const char *str, void *memp)
{
	memorywin_i *mem = (memorywin_i *) memp;
	memory_view_set_expression(mem->memory, str);
}

static void memorywin_destroy(GtkObject *obj, gpointer user_data)
{
	memorywin_i *mem = (memorywin_i *) user_data;
	memorywin_i **p = &memorywin_list;
	while(*p != mem)
		p = &(*p)->next;
	if(mem->next)
		*p = mem->next;
	else
		*p = 0;
	edit_del(&mem->ed);
	free(mem);
}

static void memorywin_new(running_machine *machine)
{
	memorywin_i *mem;
	int item, cursel;
	const device_config *curcpu = debug_cpu_get_visible_cpu(machine);
	const memory_subview_item *subview;

	mem = (memorywin_i *) malloc(sizeof(*mem));
	memset(mem, 0, sizeof(*mem));
	mem->next = memorywin_list;
	memorywin_list = mem;
	mem->win = create_memorywin(machine);
	mem->machine = machine;

	mem->memory_w = DVIEW(lookup_widget(mem->win, "memoryview"));
	dview_set_debug_view(mem->memory_w, machine, DVT_MEMORY, &mem->memory);

	mem->zone_w   = GTK_COMBO_BOX(lookup_widget(mem->win, "zone"));

	edit_init(machine, &mem->ed, lookup_widget(mem->win, "edit"), "0", 1, memorywin_process_string, mem);

	debug_view_begin_update(mem->memory);
	memory_view_set_expression(mem->memory, "0");
	debug_view_end_update(mem->memory);

	// populate the combobox
	if (!memorycombo)
	{
		cursel = item = 0;

		for (subview = memory_view_get_subview_list(mem->memory); subview != NULL; subview = subview->next)
		{
			gtk_combo_box_append_text(mem->zone_w, subview->name);
			if (cursel == 0 && subview->space != NULL && subview->space->cpu == curcpu)
				cursel = item;

			item++;
		}

		memory_view_set_subview(mem->memory, cursel);
		gtk_combo_box_set_active(mem->zone_w, cursel);
	}

	g_signal_connect(mem->zone_w, "changed", G_CALLBACK(memorywin_zone_changed), mem);
	g_signal_connect(lookup_widget(mem->win, "chunks_1"), "activate", G_CALLBACK(on_chunks_1_activate), mem);
	g_signal_connect(lookup_widget(mem->win, "chunks_2"), "activate", G_CALLBACK(on_chunks_2_activate), mem);
	g_signal_connect(lookup_widget(mem->win, "chunks_4"), "activate", G_CALLBACK(on_chunks_4_activate), mem);
	g_signal_connect(lookup_widget(mem->win, "reverse"),  "activate", G_CALLBACK(on_reverse_activate),  mem);
	g_signal_connect(lookup_widget(mem->win, "ibpl"),     "activate", G_CALLBACK(on_ibpl_activate),     mem);
	g_signal_connect(lookup_widget(mem->win, "dbpl"),     "activate", G_CALLBACK(on_dbpl_activate),     mem);

	g_signal_connect(mem->win, "destroy", G_CALLBACK(memorywin_destroy), mem);
	gtk_widget_show_all(mem->win);
}

static void disasmwin_update_checks(disasmwin_i *info)
{
	int rc = disasm_view_get_right_column(info->disasm);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}

static void disasmwin_cpu_changed(GtkComboBox *cpu_w, disasmwin_i *dis)
{
	char title[256];
	const disasm_subview_item *subview;

	disasm_view_set_subview(dis->disasm, gtk_combo_box_get_active(dis->cpu_w));

	disasmwin_update_checks(dis);

	subview = disasm_view_get_current_subview(dis->disasm);
	sprintf(title, "Disassembly: %s", subview->name);
	gtk_window_set_title(GTK_WINDOW(dis->win), title);
}

void disasmwin_raw_opcodes_activate(GtkMenuItem *item, gpointer user_data)
{
	disasmwin_i *info = (disasmwin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm,  DASM_RIGHTCOL_RAW);
		debug_view_end_update(info->disasm);
	}
}

void disasmwin_enc_opcodes_activate(GtkMenuItem *item, gpointer user_data)
{
	disasmwin_i *info = (disasmwin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm, DASM_RIGHTCOL_ENCRYPTED);
		debug_view_end_update(info->disasm);
	}
}

void disasmwin_comments_activate(GtkMenuItem *item, gpointer user_data)
{
	disasmwin_i *info = (disasmwin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")))) {
		debug_view_begin_update(info->disasm);
		disasm_view_set_right_column(info->disasm, DASM_RIGHTCOL_COMMENTS);
		debug_view_end_update(info->disasm);
	}
}

static void disasmwin_process_string(running_machine *machine, const char *str, void *disp)
{
	disasmwin_i *dis = (disasmwin_i *) disp;
	disasm_view_set_expression(dis->disasm, str);
}

static void disasmwin_destroy(GtkObject *obj, gpointer user_data)
{
	disasmwin_i *dis = (disasmwin_i *) user_data;
	disasmwin_i **p = &disasmwin_list;
	while(*p != dis)
		p = &(*p)->next;
	if(dis->next)
		*p = dis->next;
	else
		*p = 0;
	edit_del(&dis->ed);
	free(dis);
}

static void disasmwin_new(running_machine *machine)
{
	disasmwin_i *dis;
	int item, cursel;
	const device_config *curcpu = debug_cpu_get_visible_cpu(machine);
	const disasm_subview_item *subview;
	char title[256];

	dis = (disasmwin_i *)malloc(sizeof(*dis));
	memset(dis, 0, sizeof(*dis));
	dis->next = disasmwin_list;
	disasmwin_list = dis;
	dis->win = create_disasmwin(machine);
	dis->machine = machine;

	dis->disasm_w = DVIEW(lookup_widget(dis->win, "disasmview"));

	dview_set_debug_view(dis->disasm_w, machine, DVT_DISASSEMBLY, &dis->disasm);

	dis->cpu_w    = GTK_COMBO_BOX(lookup_widget(dis->win, "cpu"));

	edit_init(machine, &dis->ed, lookup_widget(dis->win, "edit"), "curpc", 1, disasmwin_process_string, dis);

	debug_view_begin_update(dis->disasm);
	disasm_view_set_expression(dis->disasm, "curpc");
//  debug_view_set_property_UINT32(dis->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dis->disasm);

	// populate the combobox
	cursel = item = 0;
	for (subview = disasm_view_get_subview_list(dis->disasm); subview != NULL; subview = subview->next)
	{
		gtk_combo_box_append_text(dis->cpu_w, subview->name);
		if (cursel == 0 && subview->space->cpu == curcpu)
			cursel = item;

		item++;
	}

	gtk_combo_box_set_active(dis->cpu_w, cursel);
	disasm_view_set_subview(dis->disasm, cursel);

	subview = disasm_view_get_current_subview(dis->disasm);
	sprintf(title, "Disassembly: %s", subview->name);
	gtk_window_set_title(GTK_WINDOW(dis->win), title);

	g_signal_connect(dis->cpu_w, "changed", G_CALLBACK(disasmwin_cpu_changed), dis);
	g_signal_connect(lookup_widget(dis->win, "raw_opcodes"), "activate", G_CALLBACK(disasmwin_raw_opcodes_activate), dis);
	g_signal_connect(lookup_widget(dis->win, "enc_opcodes"), "activate", G_CALLBACK(disasmwin_enc_opcodes_activate), dis);
	g_signal_connect(lookup_widget(dis->win, "comments"),    "activate", G_CALLBACK(disasmwin_comments_activate), dis);

	//  g_signal_connect(dis->edit_w, "activate", G_CALLBACK(disasmwin_process_string), dis);
	g_signal_connect(dis->win, "destroy", G_CALLBACK(disasmwin_destroy), dis);
	gtk_widget_show_all(dis->win);
}

static void logwin_destroy(GtkObject *obj, gpointer user_data)
{
	logwin_i *log = (logwin_i *) user_data;
	logwin_i **p = &logwin_list;
	while(*p != log)
		p = &(*p)->next;
	if(log->next)
		*p = log->next;
	else
		*p = 0;
	free(log);
}

static void logwin_new(running_machine *machine)
{
	logwin_i *log;

	log = (logwin_i *) malloc(sizeof(*log));
	memset(log, 0, sizeof(*log));
	log->next = logwin_list;
	logwin_list = log;
	log->win = create_logwin(machine);
	log->machine = machine;

	log->log_w = DVIEW(lookup_widget(log->win, "logview"));
	dview_set_debug_view(log->log_w, machine, DVT_LOG, &log->log);

	g_signal_connect(log->win, "destroy", G_CALLBACK(logwin_destroy), log);

	gtk_widget_show_all(log->win);
}

void on_new_mem_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_new((running_machine *)user_data);
}

void on_new_disasm_activate(GtkMenuItem *item, gpointer user_data)
{
	disasmwin_new((running_machine *)user_data);
}

void on_new_errorlog_activate(GtkMenuItem *item, gpointer user_data)
{
	logwin_new((running_machine *)user_data);
}

void on_run_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_go((running_machine *)user_data, ~0);
}

void on_run_h_activate(GtkMenuItem *item, gpointer user_data)
{
	debugwin_show(0);
	debug_cpu_go((running_machine *)user_data, ~0);
}

void on_run_cpu_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_next_cpu((running_machine *)user_data);
}

void on_run_irq_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_go_interrupt((running_machine *)user_data, -1);
}

void on_run_vbl_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_go_vblank((running_machine *)user_data);
}

void on_step_into_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_single_step((running_machine *)user_data, 1);
}

void on_step_over_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_single_step_over((running_machine *)user_data, 1);
}

void on_step_out_activate(GtkMenuItem *item, gpointer user_data)
{
	debug_cpu_single_step_out((running_machine *)user_data);
}

void on_hard_reset_activate(GtkMenuItem *item, gpointer user_data)
{
	mame_schedule_hard_reset((running_machine *)user_data);
}

void on_soft_reset_activate(GtkMenuItem *item, gpointer user_data)
{
	mame_schedule_soft_reset((running_machine *)user_data);
	debug_cpu_go((running_machine *)user_data, ~0);
}

void on_exit_activate(GtkMenuItem *item, gpointer user_data)
{
	mame_schedule_exit((running_machine *)user_data);
}

void on_chunks_1_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")))) {
		debug_view_begin_update(info->memory);
		memory_view_set_bytes_per_chunk(info->memory, 1);
		debug_view_end_update(info->memory);
	}
}

void on_chunks_2_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")))) {
		debug_view_begin_update(info->memory);
		memory_view_set_bytes_per_chunk(info->memory, 2);
		debug_view_end_update(info->memory);
	}
}

void on_chunks_4_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")))) {
		debug_view_begin_update(info->memory);
		memory_view_set_bytes_per_chunk(info->memory, 4);
		debug_view_end_update(info->memory);
	}
}

void on_reverse_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	debug_view_begin_update(info->memory);
	memory_view_set_reverse(info->memory, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse"))));

	debug_view_end_update(info->memory);
}

void on_ibpl_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	debug_view_begin_update(info->memory);
	memory_view_set_chunks_per_row(info->memory, memory_view_get_chunks_per_row(info->memory) + 1);
	debug_view_end_update(info->memory);
}

void on_dbpl_activate(GtkMenuItem *item, gpointer user_data)
{
	memorywin_i *info = (memorywin_i *) user_data;
	debug_view_begin_update(info->memory);
	memory_view_set_chunks_per_row(info->memory, memory_view_get_chunks_per_row(info->memory) - 1);
	debug_view_end_update(info->memory);
}

#else

#include "emu.h"

// win32 stubs for linking
void osd_wait_for_debugger(const device_config *device, int firststop)
{
}

void debugwin_update_during_game(running_machine *machine)
{
}

#endif
