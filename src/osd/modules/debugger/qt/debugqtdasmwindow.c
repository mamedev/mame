#define NO_MEM_TRACKING

#include "debugqtdasmwindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"


DasmWindow::DasmWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, NULL)
{
	setWindowTitle("Debug: Disassembly View");

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame* mainWindowFrame = new QFrame(this);

	// The top frame & groupbox that contains the input widgets
	QFrame* topSubFrame = new QFrame(mainWindowFrame);

	// The input edit
	m_inputEdit = new QLineEdit(topSubFrame);
	connect(m_inputEdit, SIGNAL(returnPressed()), this, SLOT(expressionSubmitted()));

	// The cpu combo box
	m_cpuComboBox = new QComboBox(topSubFrame);
	m_cpuComboBox->setObjectName("cpu");
	m_cpuComboBox->setMinimumWidth(300);
	connect(m_cpuComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(cpuChanged(int)));

	// The main disasm window
	m_dasmView = new DebuggerView(DVT_DISASSEMBLY,
									m_machine,
									this);

	// Force a recompute of the disassembly region
	downcast<debug_view_disasm*>(m_dasmView->view())->set_expression("curpc");

	// Populate the combo box & set the proper cpu
	populateComboBox();
	//const debug_view_source *source = mem->views[0]->view->source_for_device(curcpu);
	//gtk_combo_box_set_active(zone_w, mem->views[0]->view->source_list().indexof(*source));
	//mem->views[0]->view->set_source(*source);


	// Layout
	QHBoxLayout* subLayout = new QHBoxLayout(topSubFrame);
	subLayout->addWidget(m_inputEdit);
	subLayout->addWidget(m_cpuComboBox);
	subLayout->setSpacing(3);
	subLayout->setContentsMargins(2,2,2,2);

	QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(topSubFrame);
	vLayout->addWidget(m_dasmView);

	setCentralWidget(mainWindowFrame);

	//
	// Menu bars
	//
	// Create two commands
	QAction* breakpointSetAct = new QAction("Toggle Breakpoint At Cursor", this);
	QAction* runToCursorAct = new QAction("Run To Cursor", this);
	breakpointSetAct->setShortcut(Qt::Key_F9);
	runToCursorAct->setShortcut(Qt::Key_F4);
	connect(breakpointSetAct, SIGNAL(triggered(bool)), this, SLOT(toggleBreakpointAtCursor(bool)));
	connect(runToCursorAct, SIGNAL(triggered(bool)), this, SLOT(runToCursor(bool)));

	// Right bar options
	QActionGroup* rightBarGroup = new QActionGroup(this);
	rightBarGroup->setObjectName("rightbargroup");
	QAction* rightActRaw = new QAction("Raw Opcodes", this);
	QAction* rightActEncrypted = new QAction("Encrypted Opcodes", this);
	QAction* rightActComments = new QAction("Comments", this);
	rightActRaw->setCheckable(true);
	rightActEncrypted->setCheckable(true);
	rightActComments->setCheckable(true);
	rightActRaw->setActionGroup(rightBarGroup);
	rightActEncrypted->setActionGroup(rightBarGroup);
	rightActComments->setActionGroup(rightBarGroup);
	rightActRaw->setShortcut(QKeySequence("Ctrl+R"));
	rightActEncrypted->setShortcut(QKeySequence("Ctrl+E"));
	rightActComments->setShortcut(QKeySequence("Ctrl+C"));
	rightActRaw->setChecked(true);
	connect(rightBarGroup, SIGNAL(triggered(QAction*)), this, SLOT(rightBarChanged(QAction*)));

	// Assemble the options menu
	QMenu* optionsMenu = menuBar()->addMenu("&Options");
	optionsMenu->addAction(breakpointSetAct);
	optionsMenu->addAction(runToCursorAct);
	optionsMenu->addSeparator();
	optionsMenu->addActions(rightBarGroup->actions());
}


DasmWindow::~DasmWindow()
{
}


void DasmWindow::cpuChanged(int index)
{
	m_dasmView->view()->set_source(*m_dasmView->view()->source_list().find(index));
	m_dasmView->viewport()->update();
}


void DasmWindow::expressionSubmitted()
{
	const QString expression = m_inputEdit->text();
	downcast<debug_view_disasm*>(m_dasmView->view())->set_expression(expression.toLocal8Bit().data());
	m_dasmView->viewport()->update();
}


void DasmWindow::toggleBreakpointAtCursor(bool changedTo)
{
	if (m_dasmView->view()->cursor_visible())
	{
		if (debug_cpu_get_visible_cpu(*m_machine) == m_dasmView->view()->source()->device())
		{
			offs_t address = downcast<debug_view_disasm *>(m_dasmView->view())->selected_address();
			device_debug *cpuinfo = m_dasmView->view()->source()->device()->debug();

			// Find an existing breakpoint at this address
			INT32 bpindex = -1;
			for (device_debug::breakpoint* bp = cpuinfo->breakpoint_first();
					bp != NULL;
					bp = bp->next())
			{
				if (address == bp->address())
				{
					bpindex = bp->index();
					break;
				}
			}

			// If none exists, add a new one
			astring command;
			if (bpindex == -1)
			{
				command.printf("bpset 0x%X", address);
			}
			else
			{
				command.printf("bpclear 0x%X", bpindex);
			}
			debug_console_execute_command(*m_machine, command, 1);
		}
	}

	refreshAll();
}


void DasmWindow::runToCursor(bool changedTo)
{
	if (m_dasmView->view()->cursor_visible())
	{
		if (debug_cpu_get_visible_cpu(*m_machine) == m_dasmView->view()->source()->device())
		{
			offs_t address = downcast<debug_view_disasm*>(m_dasmView->view())->selected_address();
			astring command;
			command.printf("go 0x%X", address);
			debug_console_execute_command(*m_machine, command, 1);
		}
	}
}


void DasmWindow::rightBarChanged(QAction* changedTo)
{
	debug_view_disasm* dasmView = downcast<debug_view_disasm*>(m_dasmView->view());
	if (changedTo->text() == "Raw Opcodes")
	{
		dasmView->set_right_column(DASM_RIGHTCOL_RAW);
	}
	else if (changedTo->text() == "Encrypted Opcodes")
	{
		dasmView->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
	}
	else if (changedTo->text() == "Comments")
	{
		dasmView->set_right_column(DASM_RIGHTCOL_COMMENTS);
	}
	m_dasmView->viewport()->update();
}


void DasmWindow::populateComboBox()
{
	if (m_dasmView == NULL)
		return;

	m_cpuComboBox->clear();
	for (const debug_view_source* source = m_dasmView->view()->first_source();
			source != NULL;
			source = source->next())
	{
		m_cpuComboBox->addItem(source->name());
	}
}


//=========================================================================
//  DasmWindowQtConfig
//=========================================================================
void DasmWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	DasmWindow* window = dynamic_cast<DasmWindow*>(widget);
	QComboBox* cpu = window->findChild<QComboBox*>("cpu");
	m_cpu = cpu->currentIndex();

	QActionGroup* rightBarGroup = window->findChild<QActionGroup*>("rightbargroup");
	if (rightBarGroup->checkedAction()->text() == "Raw Opcodes")
		m_rightBar = 0;
	else if (rightBarGroup->checkedAction()->text() == "Encrypted Opcodes")
		m_rightBar = 1;
	else if (rightBarGroup->checkedAction()->text() == "Comments")
		m_rightBar = 2;
}

void DasmWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
	DasmWindow* window = dynamic_cast<DasmWindow*>(widget);
	QComboBox* cpu = window->findChild<QComboBox*>("cpu");
	cpu->setCurrentIndex(m_cpu);

	QActionGroup* rightBarGroup = window->findChild<QActionGroup*>("rightbargroup");
	rightBarGroup->actions()[m_rightBar]->trigger();
}

void DasmWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
	xml_set_attribute_int(node, "cpu", m_cpu);
	xml_set_attribute_int(node, "rightbar", m_rightBar);
}

void DasmWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
	m_cpu = xml_get_attribute_int(node, "cpu", m_cpu);
	m_rightBar = xml_get_attribute_int(node, "rightbar", m_rightBar);
}
