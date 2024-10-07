// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "dasmwindow.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"
#include "debug/points.h"

#include "util/xmlfile.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#else
#include <QtWidgets/QAction>
#endif
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>


namespace osd::debugger::qt {

DasmWindow::DasmWindow(DebuggerQt &debugger, QWidget *parent) :
	WindowQt(debugger, nullptr),
	m_inputHistory()
{
	setWindowTitle("Debug: Disassembly View");

	if (parent)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame *mainWindowFrame = new QFrame(this);

	// The top frame & groupbox that contains the input widgets
	QFrame *topSubFrame = new QFrame(mainWindowFrame);

	// The input edit
	m_inputEdit = new QLineEdit(topSubFrame);
	connect(m_inputEdit, &QLineEdit::returnPressed, this, &DasmWindow::expressionSubmitted);
	connect(m_inputEdit, &QLineEdit::textEdited, this, &DasmWindow::expressionEdited);
	m_inputEdit->installEventFilter(this);

	// The cpu combo box
	m_cpuComboBox = new QComboBox(topSubFrame);
	m_cpuComboBox->setObjectName("cpu");
	m_cpuComboBox->setMinimumWidth(300);
	connect(m_cpuComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DasmWindow::cpuChanged);

	// The main disasm window
	m_dasmView = new DebuggerView(DVT_DISASSEMBLY, m_machine, this);
	connect(m_dasmView, &DebuggerView::updated, this, &DasmWindow::dasmViewUpdated);

	// Force a recompute of the disassembly region
	m_dasmView->view<debug_view_disasm>()->set_expression("curpc");

	// Populate the combo box & set the proper CPU
	populateComboBox();
	setToCurrentCpu();

	// Layout
	QHBoxLayout *subLayout = new QHBoxLayout(topSubFrame);
	subLayout->addWidget(m_inputEdit);
	subLayout->addWidget(m_cpuComboBox);
	subLayout->setSpacing(3);
	subLayout->setContentsMargins(2,2,2,2);

	QVBoxLayout *vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(topSubFrame);
	vLayout->addWidget(m_dasmView);

	setCentralWidget(mainWindowFrame);

	//
	// Menu bars
	//
	// Create three commands
	m_breakpointToggleAct = new QAction("Toggle Breakpoint at Cursor", this);
	m_breakpointEnableAct = new QAction("Disable Breakpoint at Cursor", this);
	m_runToCursorAct = new QAction("Run to Cursor", this);
	m_breakpointToggleAct->setShortcut(Qt::Key_F9);
	m_breakpointEnableAct->setShortcut(Qt::SHIFT | Qt::Key_F9);
	m_runToCursorAct->setShortcut(Qt::Key_F4);
	connect(m_breakpointToggleAct, &QAction::triggered, this, &DasmWindow::toggleBreakpointAtCursor);
	connect(m_breakpointEnableAct, &QAction::triggered, this, &DasmWindow::enableBreakpointAtCursor);
	connect(m_runToCursorAct, &QAction::triggered, this, &DasmWindow::runToCursor);

	// Right bar options
	QActionGroup *rightBarGroup = new QActionGroup(this);
	rightBarGroup->setObjectName("rightbargroup");
	QAction *rightActRaw = new QAction("Raw Opcodes", this);
	QAction *rightActEncrypted = new QAction("Encrypted Opcodes", this);
	QAction *rightActComments = new QAction("Comments", this);
	rightActRaw->setData(int(DASM_RIGHTCOL_RAW));
	rightActEncrypted->setData(int(DASM_RIGHTCOL_ENCRYPTED));
	rightActComments->setData(int(DASM_RIGHTCOL_COMMENTS));
	rightActRaw->setCheckable(true);
	rightActEncrypted->setCheckable(true);
	rightActComments->setCheckable(true);
	rightActRaw->setActionGroup(rightBarGroup);
	rightActEncrypted->setActionGroup(rightBarGroup);
	rightActComments->setActionGroup(rightBarGroup);
	rightActRaw->setShortcut(QKeySequence("Ctrl+R"));
	rightActEncrypted->setShortcut(QKeySequence("Ctrl+E"));
	rightActComments->setShortcut(QKeySequence("Ctrl+N"));
	rightActRaw->setChecked(true);
	connect(rightBarGroup, &QActionGroup::triggered, this, &DasmWindow::rightBarChanged);

	// Assemble the options menu
	QMenu *optionsMenu = menuBar()->addMenu("&Options");
	optionsMenu->addAction(m_breakpointToggleAct);
	optionsMenu->addAction(m_breakpointEnableAct);
	optionsMenu->addAction(m_runToCursorAct);
	optionsMenu->addSeparator();
	optionsMenu->addActions(rightBarGroup->actions());
}


DasmWindow::~DasmWindow()
{
}


void DasmWindow::restoreConfiguration(util::xml::data_node const &node)
{
	WindowQt::restoreConfiguration(node);

	debug_view_disasm &dasmview = *m_dasmView->view<debug_view_disasm>();

	auto const cpu = node.get_attribute_int(ATTR_WINDOW_DISASSEMBLY_CPU, m_dasmView->sourceIndex());
	if ((0 <= cpu) && (m_cpuComboBox->count() > cpu))
		m_cpuComboBox->setCurrentIndex(cpu);

	auto const rightbar = node.get_attribute_int(ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmview.right_column());
	QActionGroup *const rightBarGroup = findChild<QActionGroup *>("rightbargroup");
	for (QAction *action : rightBarGroup->actions())
	{
		if (action->data().toInt() == rightbar)
		{
			action->trigger();
			break;
		}
	}

	util::xml::data_node const *const expression = node.get_child(NODE_WINDOW_EXPRESSION);
	if (expression && expression->get_value())
	{
		m_inputEdit->setText(QString::fromUtf8(expression->get_value()));
		expressionSubmitted();
	}

	m_dasmView->restoreConfigurationFromNode(node);
	m_inputHistory.restoreConfigurationFromNode(node);
}


void DasmWindow::saveConfigurationToNode(util::xml::data_node &node)
{
	WindowQt::saveConfigurationToNode(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_DISASSEMBLY_VIEWER);

	debug_view_disasm &dasmview = *m_dasmView->view<debug_view_disasm>();
	node.set_attribute_int(ATTR_WINDOW_DISASSEMBLY_CPU, m_dasmView->sourceIndex());
	node.set_attribute_int(ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmview.right_column());
	node.add_child(NODE_WINDOW_EXPRESSION, dasmview.expression());

	m_dasmView->saveConfigurationToNode(node);
	m_inputHistory.saveConfigurationToNode(node);
}


// Used to intercept the user hitting the up arrow in the input widget
bool DasmWindow::eventFilter(QObject *obj, QEvent *event)
{
	// Only filter keypresses
	if (event->type() != QEvent::KeyPress)
		return QObject::eventFilter(obj, event);

	QKeyEvent const &keyEvent = *static_cast<QKeyEvent *>(event);

	// Catch up & down keys
	if (keyEvent.key() == Qt::Key_Escape)
	{
		m_inputEdit->setText(QString::fromUtf8(m_dasmView->view<debug_view_disasm>()->expression()));
		m_inputEdit->selectAll();
		m_inputHistory.reset();
		return true;
	}
	else if (keyEvent.key() == Qt::Key_Up)
	{
		QString const *const hist = m_inputHistory.previous(m_inputEdit->text());
		if (hist)
		{
			m_inputEdit->setText(*hist);
			m_inputEdit->setSelection(hist->size(), 0);
		}
		return true;
	}
	else if (keyEvent.key() == Qt::Key_Down)
	{
		QString const *const hist = m_inputHistory.next(m_inputEdit->text());
		if (hist)
		{
			m_inputEdit->setText(*hist);
			m_inputEdit->setSelection(hist->size(), 0);
		}
		return true;
	}
	else
	{
		return QObject::eventFilter(obj, event);
	}
}


void DasmWindow::cpuChanged(int index)
{
	if (index < m_dasmView->view()->source_count())
	{
		m_dasmView->view()->set_source(*m_dasmView->view()->source(index));
		m_dasmView->viewport()->update();
	}
}


void DasmWindow::expressionSubmitted()
{
	const QString expression = m_inputEdit->text();
	m_dasmView->view<debug_view_disasm>()->set_expression(expression.toUtf8().data());
	m_inputEdit->selectAll();

	// Add history
	if (!expression.isEmpty())
		m_inputHistory.add(expression);
}


void DasmWindow::expressionEdited(QString const &text)
{
	m_inputHistory.edit();
}


void DasmWindow::toggleBreakpointAtCursor(bool changedTo)
{
	if (m_dasmView->view()->cursor_visible())
	{
		offs_t const address = m_dasmView->view<debug_view_disasm>()->selected_address();
		device_t *const device = m_dasmView->view()->source()->device();
		device_debug *const cpuinfo = device->debug();

		// Find an existing breakpoint at this address
		const debug_breakpoint *bp = cpuinfo->breakpoint_find(address);

		// If none exists, add a new one
		if (!bp)
		{
			int32_t bpindex = cpuinfo->breakpoint_set(address);
			m_machine.debugger().console().printf("Breakpoint %X set\n", bpindex);
		}
		else
		{
			int32_t bpindex = bp->index();
			cpuinfo->breakpoint_clear(bpindex);
			m_machine.debugger().console().printf("Breakpoint %X cleared\n", bpindex);
		}
		m_machine.debug_view().update_all();
		m_machine.debugger().refresh_display();
	}
}


void DasmWindow::enableBreakpointAtCursor(bool changedTo)
{
	if (m_dasmView->view()->cursor_visible())
	{
		offs_t const address = m_dasmView->view<debug_view_disasm>()->selected_address();
		device_t *const device = m_dasmView->view()->source()->device();
		device_debug *const cpuinfo = device->debug();

		// Find an existing breakpoint at this address
		const debug_breakpoint *bp = cpuinfo->breakpoint_find(address);

		if (bp)
		{
			cpuinfo->breakpoint_enable(bp->index(), !bp->enabled());
			m_machine.debugger().console().printf("Breakpoint %X %s\n", (uint32_t)bp->index(), bp->enabled() ? "enabled" : "disabled");
			m_machine.debug_view().update_all();
			m_machine.debugger().refresh_display();
		}
	}
}


void DasmWindow::runToCursor(bool changedTo)
{
	if (m_dasmView->view()->cursor_visible())
	{
		offs_t const address = m_dasmView->view<debug_view_disasm>()->selected_address();
		m_dasmView->view()->source()->device()->debug()->go(address);
	}
}


void DasmWindow::rightBarChanged(QAction* changedTo)
{
	debug_view_disasm *const dasmView = m_dasmView->view<debug_view_disasm>();
	dasmView->set_right_column(disasm_right_column(changedTo->data().toInt()));
	m_dasmView->viewport()->update();
}


void DasmWindow::dasmViewUpdated()
{
	bool const haveCursor = m_dasmView->view()->cursor_visible();
	bool haveBreakpoint = false;
	bool breakpointEnabled = false;
	if (haveCursor)
	{
		offs_t const address = m_dasmView->view<debug_view_disasm>()->selected_address();
		device_t *const device = m_dasmView->view()->source()->device();
		device_debug *const cpuinfo = device->debug();

		// Find an existing breakpoint at this address
		const debug_breakpoint *bp = cpuinfo->breakpoint_find(address);

		if (bp)
		{
			haveBreakpoint = true;
			breakpointEnabled = bp->enabled();
		}
	}

	m_breakpointToggleAct->setText(haveBreakpoint ? "Clear Breakpoint at Cursor" : haveCursor ? "Set Breakpoint at Cursor" : "Toggle Breakpoint at Cursor");
	m_breakpointEnableAct->setText((!haveBreakpoint || breakpointEnabled) ? "Disable Breakpoint at Cursor" : "Enable Breakpoint at Cursor");
	m_breakpointToggleAct->setEnabled(haveCursor);
	m_breakpointEnableAct->setEnabled(haveBreakpoint);
	m_runToCursorAct->setEnabled(haveCursor);
}


void DasmWindow::populateComboBox()
{
	if (!m_dasmView)
		return;

	m_cpuComboBox->clear();
	for (auto &source : m_dasmView->view()->source_list())
	{
		m_cpuComboBox->addItem(source->name());
	}
}


void DasmWindow::setToCurrentCpu()
{
	device_t *curCpu = m_machine.debugger().console().get_visible_cpu();
	if (curCpu)
	{
		const debug_view_source *source = m_dasmView->view()->source_for_device(curCpu);
		if (source)
		{
			const int listIndex = m_dasmView->view()->source_index(*source);
			m_cpuComboBox->setCurrentIndex(listIndex);
		}
	}
}

} // namespace osd::debugger::qt
