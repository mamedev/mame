#include "debugqtlogwindow.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"


LogWindow::LogWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, parent)
{
	QPoint parentPos = parent->pos();
	setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	setWindowTitle("Debug: Machine Log");

	//
	// The main frame and its input and log widgets
	//
	QFrame* mainWindowFrame = new QFrame(this);

	// The main log view
	m_logView = new DebuggerView(DVT_LOG,
									m_machine,
									this);

	// Layout
	QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(m_logView);

	setCentralWidget(mainWindowFrame);
}
