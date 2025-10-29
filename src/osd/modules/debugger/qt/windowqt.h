// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_WINDOWQT_H
#define MAME_DEBUGGER_QT_WINDOWQT_H

#include "../xmlconfig.h"

#include <QtWidgets/QMainWindow>

#include <deque>
#include <memory>


namespace osd::debugger::qt {

//============================================================
//  The Qt debugger module interface
//============================================================
class DebuggerQt : public QObject
{
	Q_OBJECT

public:
	virtual ~DebuggerQt() { }

	virtual running_machine &machine() const = 0;

	void hideAll() { emit hideAllWindows(); }

signals:
	void exitDebugger();
	void hideAllWindows();
	void showAllWindows();
	void saveConfiguration(util::xml::data_node &parentnode);
};


//============================================================
//  The Qt window that everyone derives from.
//============================================================
class WindowQt : public QMainWindow
{
	Q_OBJECT

public:
	virtual ~WindowQt();

	virtual void restoreConfiguration(util::xml::data_node const &node);

protected slots:
	void debugActOpenMemory();
	void debugActOpenDasm();
	void debugActOpenLog();
	void debugActOpenPoints();
	void debugActOpenDevices();
	void debugActRun();
	void debugActRunAndHide();
	void debugActRunToNextCpu();
	void debugActRunNextInt();
	void debugActRunNextVBlank();
	virtual void debugActStepInto();
	virtual void debugActStepOver();
	virtual void debugActStepOut();
	void debugActSoftReset();
	void debugActHardReset();
	virtual void debugActClose();
	void debugActQuit();
	virtual void debuggerExit();

private slots:
	void saveConfiguration(util::xml::data_node &parentnode);

protected:
	WindowQt(DebuggerQt &debugger, QWidget *parent = nullptr);

	virtual void saveConfigurationToNode(util::xml::data_node &node);

	DebuggerQt &m_debugger;
	running_machine &m_machine;
};


//============================================================
//  Command history helper
//============================================================
class CommandHistory
{
public:
	CommandHistory();
	~CommandHistory();

	void add(QString const &entry);
	QString const *previous(QString const &current);
	QString const *next(QString const &current);
	void edit();
	void reset();
	void clear();

	void restoreConfigurationFromNode(util::xml::data_node const &node);
	void saveConfigurationToNode(util::xml::data_node &node);

private:
	static inline constexpr unsigned CAPACITY = 100U;

	std::deque<QString> m_history;
	std::unique_ptr<QString> m_current;
	int m_position;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_WINDOWQT_H
