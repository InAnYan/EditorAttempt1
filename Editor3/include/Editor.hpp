#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP

#include <vector>
#include <fstream>
#include <exception>

#include "Terminal.hpp"

/// The editor.
class Editor
{
public:
	/// Create an editor with file.
	/// Note: the caller should ensure that there is no error with file read.
	// TODO: Argument type ?.
	Editor(std::ifstream& file);
	
	/// Redraw the editor screen.
	void RefreshScreen(std::shared_ptr<Terminal> terminal);
	/// Process the key. Return true if the editor is alive, otherwise, false.
	bool ProcessKey(TerminalKey key);

	/// Change the tab size.
	void SetTabSize(int newSize);
	
private:
	/// The state of the buffer window cursor.
	TerminalCoord m_Cursor = { 0, 0 };
	/// The last information about size of the terminal.
	TerminalCoord m_TerminalSize = { 0, 0 };

	/// Position in the file that is the top left corner of the editor terminal.
	TerminalCoord m_Offset = { 0, 0 };

	/// The count of spaces that is used for representing tabs.
	int m_TabSize = 4;

	// TODO: DOCUMENT.

	struct Row
	{
		std::vector<char> real;
		std::vector<char> render;
	};
	
	std::vector<Row> m_Buffer;

	void AppendRow(const std::string& str);
	void UpdateRow(Row& row);
	
	/// Draw editor main rows.
	void DrawRows(std::shared_ptr<Terminal> terminal);
	/// Draw tilda or welcome message.
	void DrawDefaultRow(int y, std::shared_ptr<Terminal> terminal);

	/// Change the m_RowOffset if the m_Cursor is out of range of m_TerminalScreen.
	void Scroll();
	
	/// Move cursor according to the key.
	void ProcessMoveCursor(TerminalKey key);
};

#endif // EDITOR_EDITOR_HPP
