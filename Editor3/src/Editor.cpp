#include "Editor.hpp"

#include <cassert>

#define WELCOME_MESSAGE "Welcome to the Editor! Version 0.0.1"

Editor::Editor()
	: m_Cursor{ 0, 0 }, m_TerminalSize{ 0, 0 }
{
	m_Buffer.push_back({ 'H', 'E', 'L', 'l', 'o', '!' });
}

void Editor::RefreshScreen(Terminal& terminal)
{
	m_TerminalSize = terminal.GetSize();

	terminal.HideCursor();
	terminal.SetCursorPosition({ 0, 0 });

	this->DrawRows(terminal);
	
	terminal.SetCursorPosition(m_Cursor);
	terminal.ShowCursor();

	terminal.Flush();
}

void Editor::DrawRows(Terminal& terminal)
{
	for (int y = 0; y < m_TerminalSize.row; y++)
	{
		if (y >= m_Buffer.size())
		{
			this->DrawDefaultRow(y, terminal);
		}
		else
		{
			// TODO: Two modes: the exceeding part of the line is not shown or it is printed on next line.
			std::vector<char>& line = m_Buffer[y];
			
			unsigned sizeToPrint = line.size();
			if (sizeToPrint > m_TerminalSize.column)
			{
				sizeToPrint = m_TerminalSize.column;
			}

			// TODO: Do not create a copy of editor row.
			terminal.WriteString(std::string(line.begin(), line.end()));
		}

		terminal.ClearCurrentRow();
		
		if (y < m_TerminalSize.row - 1)
		{
			terminal.WriteString("\r\n");
		}
	}
}

void Editor::DrawDefaultRow(unsigned y, Terminal& terminal)
{	
	if (y == m_TerminalSize.row / 3)
	{
		if (m_TerminalSize.column > sizeof(WELCOME_MESSAGE))
		{
			// TODO: Position the welcome string more pretty.
			unsigned padding = (m_TerminalSize.column - sizeof(WELCOME_MESSAGE)) / 2;
			if (padding != 0)
			{
				terminal.WriteString("~");
				padding--;
			}

			terminal.WriteString(std::string(padding, ' '));
				
			terminal.WriteString(WELCOME_MESSAGE);
		}
		else
		{
			terminal.WriteString("~");
		}
	}
	else
	{
		terminal.WriteString("~");		
	}
}

bool Editor::ProcessKey(TerminalKey key)
{
	switch (key.GetChar())
	{
	case 'q':
		if (key.IsCtrl())
		{
			return false;
		}
		break;

	case TerminalKeys::ARROW_UP:
	case TerminalKeys::ARROW_DOWN:
	case TerminalKeys::ARROW_LEFT:
	case TerminalKeys::ARROW_RIGHT:
	case 'w':
	case 'a':
	case 's':
	case 'd':
		this->ProcessMoveCursor(key);
		break;

	case TerminalKeys::PAGE_UP:
		m_Cursor.row = 0;
		break;
	case TerminalKeys::PAGE_DOWN:
		m_Cursor.row = m_TerminalSize.row - 1;
		break;

	case TerminalKeys::HOME:
		m_Cursor.column = 0;
		break;
	case TerminalKeys::END:
		m_Cursor.column = m_TerminalSize.column - 1;
		break;
	}

	return true;
}

void Editor::ProcessMoveCursor(TerminalKey key)
{
	switch (key.GetChar())
	{
	case TerminalKeys::ARROW_UP:
	case 'w':
		if (m_Cursor.row != 0)
		{
			m_Cursor.row--;	
		}
		break;
	case TerminalKeys::ARROW_LEFT:
	case 'a':
		if (m_Cursor.column != 0)
		{
			m_Cursor.column--;	
		}
		break;
	case TerminalKeys::ARROW_DOWN:
	case 's':
		if (m_Cursor.row != m_TerminalSize.row - 1)
		{
			m_Cursor.row++;	
		}
		break;
	case TerminalKeys::ARROW_RIGHT:
	case 'd':
		if (m_Cursor.column != m_TerminalSize.column - 1)
		{
			m_Cursor.column++;	
		}
		break;
	default:
		assert(false && "This should not be occured. Key argument is wrong, probably there is a mistake in switch in Editor::ProcessKey.");
		break;
	}
}
