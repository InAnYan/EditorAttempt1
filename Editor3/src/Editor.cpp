#include "Editor.hpp"

#include <cassert>

#define WELCOME_MESSAGE "Welcome to the Editor! Version 0.0.1"

Editor::Editor(std::ifstream& file)
{
	// TODO: Line endings.
	std::string lineOfFile;
	while (std::getline(file, lineOfFile))
	{
		this->AppendRow(lineOfFile);
		// m_Buffer.push_back(std::vector<char>(lineOfFile.begin(), lineOfFile.end()));
	}
}

void Editor::RefreshScreen(std::shared_ptr<Terminal> terminal)
{
	m_TerminalSize = terminal->GetSize();
	
	this->Scroll();

	terminal->HideCursor();
	terminal->SetCursorPosition({ 0, 0 });

	this->DrawRows(terminal);
	
	terminal->SetCursorPosition({ m_Cursor.column - m_Offset.column, m_Cursor.row - m_Offset.row });
	terminal->ShowCursor();

	terminal->Flush();
}

void Editor::Scroll()
{
	// TODO: Make two modes: 1-line scrolling and page scrolling.

	if (m_Cursor.row < m_Offset.row)
	{
		m_Offset.row = m_Cursor.row;
	}

	if (m_Cursor.row >= m_Offset.row + m_TerminalSize.row)
	{
		m_Offset.row = m_Cursor.row - m_TerminalSize.row + 1;
	}

	if (m_Cursor.column < m_Offset.column)
	{
		m_Offset.column = m_Cursor.column;
	}

	if (m_Cursor.column >= m_Offset.column + m_TerminalSize.column)
	{
		m_Offset.column = m_Cursor.column - m_TerminalSize.column + 1;
	}
}

void Editor::DrawRows(std::shared_ptr<Terminal> terminal)
{
	for (int y = 0; y < m_TerminalSize.row; y++)
	{
		int fileRow = y + m_Offset.row;
		
		if (fileRow >= m_Buffer.size())
		{
			this->DrawDefaultRow(y, terminal);
		}
		else
		{
			// TODO: Two modes: the exceeding part of the line is not shown or it is printed on next line.
			const std::vector<char>& line = m_Buffer[fileRow].real;
			
			int sizeToPrint = line.size() - m_Offset.column;

			if (sizeToPrint < 0)
			{
				sizeToPrint = 0;
			}
			
			if (sizeToPrint > m_TerminalSize.column)
			{
				sizeToPrint = m_TerminalSize.column;
			}

			// TODO: Do not create a copy of editor row.
			if (sizeToPrint != 0)
			{
				terminal->WriteString(std::string(line.begin() + m_Offset.column, line.end()));
			}
		}

		terminal->ClearCurrentRow();
		
		if (y < m_TerminalSize.row - 1)
		{
			terminal->WriteString("\r\n");
		}
	}
}

void Editor::DrawDefaultRow(int y, std::shared_ptr<Terminal> terminal)
{
	// TODO: Delete the welcome message.
	if (y == m_TerminalSize.row / 3 && m_Buffer.size() == 0)
	{
		if (m_TerminalSize.column > sizeof(WELCOME_MESSAGE))
		{
			// TODO: Position the welcome string more pretty.
			int padding = (m_TerminalSize.column - sizeof(WELCOME_MESSAGE)) / 2;
			if (padding != 0)
			{
				terminal->WriteString("~");
				padding--;
			}

			terminal->WriteString(std::string(padding, ' '));
				
			terminal->WriteString(WELCOME_MESSAGE);
		}
		else
		{
			terminal->WriteString("~");
		}
	}
	else
	{
		terminal->WriteString("~");		
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
	// TODO: 2 modes: free move and only line move.

	// TODO: Save last cursor pos, if entered a small line.
	
	const std::vector<char>* line = (m_Cursor.row >= m_Buffer.size() ? nullptr : &m_Buffer[m_Cursor.row].real);
	
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
		else if (m_Cursor.row > 0)
		{
			m_Cursor.row--;
			m_Cursor.column = m_Buffer[m_Cursor.row].real.size();
		}
		break;
	case TerminalKeys::ARROW_DOWN:
	case 's':
		if (m_Cursor.row < m_Buffer.size())
		{
			m_Cursor.row++;	
		}
		break;
	case TerminalKeys::ARROW_RIGHT:
	case 'd':
		if (line != nullptr && m_Cursor.column < line->size())
		{
			m_Cursor.column++;
		}
		else if (line != nullptr && m_Cursor.column == line->size())
		{
			m_Cursor.row++;
			m_Cursor.column = 0;
		}
		break;
	default:
		assert(false && "This should not be occured. Key argument is wrong, probably there is a mistake in switch in Editor::ProcessKey.");
		break;
	}

	// WARN: I've changed this.
	line = (m_Cursor.row >= m_Buffer.size() ? nullptr : &m_Buffer[m_Cursor.row].real);
	int rowLength = line != nullptr ? line->size() : 0;
	if (m_Cursor.column > rowLength)
	{
		m_Cursor.column = rowLength;
	}
}

void Editor::SetTabSize(int newSize)
{
	if (newSize == m_TabSize)
	{
		return;
	}

	m_TabSize = newSize;

	for (Row& row : m_Buffer)
	{
		this->UpdateRow(row);
	}
}

void Editor::AppendRow(const std::string& str)
{
	m_Buffer.push_back(Row());

	m_Buffer.back().real = std::vector<char>(str.begin(), str.end());
	this->UpdateRow(m_Buffer.back());
}	

void Editor::UpdateRow(Row& row)
{
	row.render.clear();
	
	for (int i = 0; i < row.real.size(); i++)
	{
          if (row.real[i] ==
              '\t') {
			  row.render.push_back(' ');
			  			  row.render.push_back(' ');
						  			  row.render.push_back(' ');
									  			  row.render.push_back(' ');

            /*
                             row.render.push_back(' ');
                             while (row.render.size() % m_TabSize != 0)
                             {
                                     row.render.push_back(' ');
                             }
					  */
		}
		else
		{
			row.render.push_back(row.real[i]);
		}
	}
}
