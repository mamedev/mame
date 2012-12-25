#include "debugqtmainwindow.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/dvdisasm.h"


MainWindow::MainWindow(device_t* processor, 
                       running_machine* machine, 
                       QWidget* parent) : 
    WindowQt(machine, parent),
    m_historyIndex(0),
    m_inputHistory()
{
    setGeometry(300, 300, 1000, 600);

    //
    // The main frame and its input and log widgets
    //
    QFrame* mainWindowFrame = new QFrame(this);

    // The input line
    m_inputEdit = new QLineEdit(mainWindowFrame);
    connect(m_inputEdit, SIGNAL(returnPressed()), this, SLOT(executeCommand()));
    m_inputEdit->installEventFilter(this);


    // The log view
    m_consoleView = new DebuggerView(DVT_CONSOLE, 
                                     m_machine, 
                                     mainWindowFrame);
    m_consoleView->setFocusPolicy(Qt::NoFocus);
    m_consoleView->setPreferBottom(true);

    QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
    vLayout->addWidget(m_consoleView);
    vLayout->addWidget(m_inputEdit);
    vLayout->setSpacing(3);
    vLayout->setContentsMargins(4,0,4,2);

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


    //
    // Dock windows
    //
    QMenu* dockMenu = menuBar()->addMenu("Doc&ks");

    setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    // The processor dock
    QDockWidget* cpuDock = new QDockWidget("processor", this);
    cpuDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_procFrame = new ProcessorDockWidget(m_machine, cpuDock);
    cpuDock->setWidget(dynamic_cast<QWidget*>(m_procFrame));

    addDockWidget(Qt::LeftDockWidgetArea, cpuDock);
    dockMenu->addAction(cpuDock->toggleViewAction());

    // The disassembly dock
    QDockWidget* dasmDock = new QDockWidget("dasm", this);
    dasmDock->setAllowedAreas(Qt::TopDockWidgetArea);
    m_dasmFrame = new DasmDockWidget(m_machine, dasmDock);
    dasmDock->setWidget(m_dasmFrame);

    addDockWidget(Qt::TopDockWidgetArea, dasmDock);
    dockMenu->addAction(dasmDock->toggleViewAction());

    // Window title
    astring title;
    title.printf("Debug: %s - %s '%s'", m_machine->system().name, processor->name(), processor->tag());
    setWindowTitle(title.cstr());
}


void MainWindow::setProcessor(device_t* processor)
{
    // Cpu swap
    m_procFrame->view()->view()->set_source(*m_procFrame->view()->view()->source_list().match_device(processor));
    m_dasmFrame->view()->view()->set_source(*m_dasmFrame->view()->view()->source_list().match_device(processor));

    // Scrollbar refresh - seems I should be able to do in the DebuggerView
    m_dasmFrame->view()->verticalScrollBar()->setValue(m_dasmFrame->view()->view()->visible_position().y);
    m_dasmFrame->view()->verticalScrollBar()->setValue(m_dasmFrame->view()->view()->visible_position().y);

    // Window title
    astring title;
    title.printf("Debug: %s - %s '%s'", m_machine->system().name, processor->name(), processor->tag());
    setWindowTitle(title.cstr());
}


// Used to intercept the user clicking 'X' in the upper corner
void MainWindow::closeEvent(QCloseEvent* event)
{
    debugActQuit();
}


// Used to intercept the user hitting the up arrow in the input widget
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    // Only filter keypresses
    QKeyEvent* keyEvent = NULL;
    if (event->type() == QEvent::KeyPress)
    {
        keyEvent = static_cast<QKeyEvent*>(event);
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }

    // Catch up & down keys
    if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
    {
        if (keyEvent->key() == Qt::Key_Up)
        {
            if (m_historyIndex > 0)
                m_historyIndex--;
        }
        else if (keyEvent->key() == Qt::Key_Down)
        {
            if (m_historyIndex < m_inputHistory.size())
                m_historyIndex++;
        }

        // Populate the input edit or clear it if you're at the end
        if (m_historyIndex == m_inputHistory.size())
        {
            m_inputEdit->setText("");
        }
        else
        {
            m_inputEdit->setText(m_inputHistory[m_historyIndex]);
        }
    }
    else if (keyEvent->key() == Qt::Key_Enter)
    {
        executeCommand(false);
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
    
    return true;
}


void MainWindow::toggleBreakpointAtCursor(bool changedTo)
{
    debug_view_disasm* dasmView = downcast<debug_view_disasm*>(m_dasmFrame->view()->view());
    if (dasmView->cursor_visible())
    {
        if (debug_cpu_get_visible_cpu(*m_machine) == dasmView->source()->device())
        {
            offs_t address = downcast<debug_view_disasm *>(dasmView)->selected_address();
            device_debug *cpuinfo = dasmView->source()->device()->debug();

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


void MainWindow::runToCursor(bool changedTo)
{
    debug_view_disasm* dasmView = downcast<debug_view_disasm*>(m_dasmFrame->view()->view());
    if (dasmView->cursor_visible())
    {
        if (debug_cpu_get_visible_cpu(*m_machine) == dasmView->source()->device())
        {
            offs_t address = downcast<debug_view_disasm*>(dasmView)->selected_address();
            astring command;
            command.printf("go 0x%X", address);
            debug_console_execute_command(*m_machine, command, 1);
        }
    }
}


void MainWindow::rightBarChanged(QAction* changedTo)
{
    debug_view_disasm* dasmView = downcast<debug_view_disasm*>(m_dasmFrame->view()->view());
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
    m_dasmFrame->view()->viewport()->update();
}


void MainWindow::executeCommand(bool withClear)
{
    debug_console_execute_command(*m_machine, 
                                  m_inputEdit->text().toLocal8Bit().data(), 
                                  true);

    // Add history & set the index to be the top of the stack
    addToHistory(m_inputEdit->text());

    // Clear out the text and reset the history pointer only if asked
    if (withClear)
    {
        m_inputEdit->clear();
        m_historyIndex = m_inputHistory.size();
    }

    // Refresh
    m_consoleView->viewport()->update();
    m_procFrame->view()->update();
    m_dasmFrame->view()->update();
}


void MainWindow::addToHistory(const QString& command)
{
    if (command == "")
        return;
    
    // Always push back when there is no previous history
    if (m_inputHistory.size() == 0)
    {
        m_inputHistory.push_back(m_inputEdit->text());
        return;
    }

    // If there is previous history, make sure it's not what you just executed
    if (m_inputHistory.back() != m_inputEdit->text())
    {
        m_inputHistory.push_back(m_inputEdit->text());
    }
}
