#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP

#include "Terminal.hpp"

/// The editor.
class Editor
{
public:
	Editor();
	/// Redraw the editor screen.
	void RefreshScreen(Terminal& terminal);
	/// Process the key. Return true if the editor is alive, otherwise, false.
	bool ProcessKey(char key, Terminal& terminal);

private:
	/// The state of terminal cursor.
	TerminalCoord m_Cursor;

	/// Draw editor main rows.
	void DrawRows(const TerminalCoord size, Terminal& terminal);
	/// Move cursor according to the key.
	void ProcessMoveCursor(char key, Terminal& terminal);
};

#endif // EDITOR_EDITOR_HPP
