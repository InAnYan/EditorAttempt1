#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP

#include <vector>
#include <exception>
#include <filesystem>

#include "Terminal.hpp"

/// If some error occurse while opening, reading or writing to the file, then EditorFileIOError thrown.
class EditorFileIOError : std::exception
{
public:
	/// The constructor of EditorFileIOError.
	EditorFileIOError(const char* msg)
		: m_Msg(msg)
	{}

	/// Return the error message.
	virtual const char* what() const noexcept
	{
		return m_Msg;
	}

private:
	/// The message of error cause.
	const char* m_Msg;
};

/// The editor.
class Editor
{
public:
	/// Create an editor with file.
	/// Note: if the file does not exist, then a new file is created.
	Editor(const std::filesystem::path& filePath);
	
	/// Redraw the editor screen.
	void RefreshScreen(std::shared_ptr<Terminal> terminal);
	/// Process the key. Return true if the editor is alive, otherwise, false.
	/// Note: may throw an EditorFileIOError.
	bool ProcessKey(TerminalKey key);

	/// Show a message in the message bar.
	void ShowMessage(const std::string& msg, int lifeTime);
	
	/// Change the tab size.
	void SetTabSize(int newSize);
	
private:
	/// The state of the cursor in buffer.
	TerminalCoord m_Cursor = { 0, 0 };
	/// The real X coordinate of the screen. When there is no tabs it is equal to m_Cursor.x, else it will be greater depending on tab count and tab stop.
	int m_Rx = m_Cursor.x;
	/// The last known size of the terminal.
	TerminalCoord m_TerminalSize = { 0, 0 };
	/// The size of buffer area (origin at the (0;0) of the terminal).
	TerminalCoord m_BufferArea = { 0, 0 };

	/// Position in the file that is the top left corner of the editor terminal.
	TerminalCoord m_Offset = { 0, 0 };

	/// The count of spaces that is used for representing tabs.
	// TODO: It is not the count of spaces.
	int m_TabStop = 4;

	/// The color of characters' background.
	TerminalColor m_BackgroundColor = { 0, 0, 0 };
	/// The color of characters.
	TerminalColor m_ForegroundColor = { 255, 255, 255 };
	
	/// Convert buffer cursor X coordinate to real terminal X coordinate.
	void ConvertCxToRx();
	
	// TODO: DOCUMENT.
	struct Row
	{
		std::vector<char> real;
		std::vector<char> render;
	};

	/// Insert a character to a buffer row.
	void RowInsertChar(Row& row, int at, char ch);
	/// Delete a character in a buffer row.
	void RowDeleteChar(Row& row, int at);
	
	/// Insert the character to the current row.
	void InsertChar(char ch);
	/// Delete the character in the current row.
	void DeleteChar();
	/// Create new line.
	void InsertNewLine();
	
	/// The absolute path to the current opened file.
	std::string m_FilePath;
	/// The name of the current opened file.
	std::string m_FileName;

	// TODO: Document.
	std::vector<Row> m_Buffer;

	/// Was file modified.
	bool m_FileDirty = false;

	/// Save the buffer to the m_FilePath.
	void Save();
	
	/// The contents of editor's message bar.
	std::string m_MessageBarText;
	/// The count of screen refereshes before clearing m_StatusLine.
	int m_MessageBarTextLifeTime;
	
	void AppendRow(const std::string& str);
	void UpdateRow(Row& row);
	
	/// Draw editor main rows.
	void DrawRows(std::shared_ptr<Terminal> terminal);
	/// Draw tilda or welcome message.
	void DrawDefaultRow(int y, std::shared_ptr<Terminal> terminal);
	/// Draw the status bar.
	void DrawStatusBar(std::shared_ptr<Terminal> terminal);
	/// Draw the message bar.
	void DrawMessageBar(std::shared_ptr<Terminal> terminal);

	/// Change the m_RowOffset if the m_Cursor is out of range of m_TerminalScreen.
	void Scroll();
	
	/// Move cursor according to the key.
	void ProcessMoveCursor(TerminalKey key);

    // TODO: DOCUMENT
	int m_ExitConfirmations = 3;
};

#endif // EDITOR_EDITOR_HPP
