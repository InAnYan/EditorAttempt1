#include "Editor.hpp"

void Editor::RefreshScreen(Terminal& terminal)
{
	terminal.ClearScreen();
	terminal.PositionCursor(1, 1);

	for (int y = 0; y < 24; y++)
	{
		terminal.PrintString("~\r\n");
	}

	terminal.PositionCursor(1, 1);
}

bool Editor::ProcessKey(char key, Terminal& terminal)
{
	switch (key)
	{
	case CTRL_KEY('q'):
		return false;
	default:
		return true;
	}
}
