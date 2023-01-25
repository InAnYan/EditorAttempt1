#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP

#include "Terminal.hpp"
#include <vector>

/// The editor.
class Editor
{
public:
	Editor();
	/// Redraw the editor screen.
	void RefreshScreen(Terminal& terminal);
	/// Process the key. Return true if the editor is alive, otherwise, false.
	bool ProcessKey(TerminalKey key);

private:
	/// The state of the terminal cursor.
	TerminalCoord m_Cursor;
	/// The last information about size of the terminal.
	TerminalCoord m_TerminalSize;

	// TODO: DOCUMENT.
	std::vector<std::vector<char>> m_Buffer;
	
	/// Draw editor main rows.
	void DrawRows(Terminal& terminal);
	/// Draw tilda or welcome message.
	void DrawDefaultRow(unsigned y, Terminal& terminal);
	
	/// Move cursor according to the key.
	void ProcessMoveCursor(TerminalKey key);
};

#endif // EDITOR_EDITOR_HPP
