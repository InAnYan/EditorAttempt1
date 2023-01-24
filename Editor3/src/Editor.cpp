#include "Editor.hpp"

#include <cassert>

#define WELCOME_MESSAGE "Welcome to the Editor! Version 0.0.1"

Editor::Editor()
	: m_Cursor{ 0, 0 }
{
	
}

void Editor::RefreshScreen(Terminal& terminal)
{
	const TerminalCoord size = terminal.GetSize();

	terminal.HideCursor();
	terminal.SetCursorPosition({ 0, 0 });

	this->DrawRows(size, terminal);
	
	terminal.SetCursorPosition(m_Cursor);
	terminal.ShowCursor();
}

void Editor::DrawRows(const TerminalCoord size, Terminal& terminal)
{
	for (int y = 0; y < size.row; y++)
	{
		if (y == size.row / 3)
		{
			if (size.column > sizeof(WELCOME_MESSAGE))
			{
				unsigned padding = (size.column - sizeof(WELCOME_MESSAGE)) / 2;
				if (padding != 0)
				{
					terminal.PrintString("~");
					padding--;
				}

				terminal.PrintString(std::string(padding, ' '));
				
				terminal.PrintString(WELCOME_MESSAGE);
			}
			else
			{
				terminal.PrintString("~");
			}
		}
		else
		{
			terminal.PrintString("~");		
		}

		terminal.ClearCurrentRow();
		
		if (y < size.row - 1)
		{
			terminal.PrintString("\r\n");
		}
	}
}

bool Editor::ProcessKey(char key, Terminal& terminal)
{
	switch (key)
	{
	case TerminalKeyCtrl('q'):
		terminal.SetCursorPosition({ 0, 0 });
		return false;

	case 'w':
	case 'a':
	case 's':
	case 'd':
		this->ProcessMoveCursor(key, terminal);
		return true;
		
	default:
		return true;
	}
}

void Editor::ProcessMoveCursor(char key, Terminal& terminal)
{
	switch (key)
	{
	case 'w':
		m_Cursor.row--;
		break;
	case 'a':
		m_Cursor.column--;
		break;
	case 's':
		m_Cursor.row++;
		break;
	case 'd':
		m_Cursor.column++;
		break;
	default:
		assert(false && "This should not be occured. Key argument is wrong, probably there is a mistake in switch in Editor::ProcessKey.");
		break;
	}
}
