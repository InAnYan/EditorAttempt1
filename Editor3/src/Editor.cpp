#include "Editor.hpp"

#include <cassert>
#include <iomanip>
#include <fstream>

#define WELCOME_MESSAGE "Welcome to the Editor! Version 0.0.1."
#define HELP_MESSAGE "HELP: Ctrl-Q - exit | Ctrl-S - save file."

Editor::Editor(const std::filesystem::path& filePath)
	: m_FilePath(std::filesystem::absolute(filePath)),
	  m_FileName(filePath.filename())
{
	std::ifstream file(filePath);
	if (!file)
	{
		this->ShowMessage("New file: '" + filePath.string() + "'. " HELP_MESSAGE, 1);
		return;
	}
	
   	// TODO: Line endings.
	std::string lineOfFile;
	while (std::getline(file, lineOfFile))
	{
		this->AppendRow(lineOfFile);
	}

	if (file.fail() && !file.eof())
	{
		throw EditorFileIOError("an error occured while reading the file");
	}

	this->ShowMessage(HELP_MESSAGE, 1);
}

void Editor::RefreshScreen(std::shared_ptr<Terminal> terminal)
{
	m_TerminalSize = terminal->GetSize();
	m_BufferArea = m_TerminalSize;
	m_BufferArea.y -= 2;

	terminal->SetBackgroundColor(m_BackgroundColor);
	terminal->SetForegroundColor(m_ForegroundColor);
	
	this->Scroll();

	terminal->HideCursor();
	terminal->SetCursorPosition({ 0, 0 });
	
	this->DrawRows(terminal);
	this->DrawStatusBar(terminal);
	this->DrawMessageBar(terminal);
	
	terminal->SetCursorPosition({ m_Rx - m_Offset.x, m_Cursor.y - m_Offset.y });
	terminal->ShowCursor();

	terminal->RevertAllAttributes();
	
	terminal->Flush();

	m_MessageBarTextLifeTime--;
	if (m_MessageBarTextLifeTime <= 0)
	{
		m_MessageBarTextLifeTime = 0;
		m_MessageBarText = "";
	}
}

void Editor::Scroll()
{
	// TODO: Make two modes: 1-line scrolling and page scrolling.

	if (m_Cursor.y < m_Buffer.size())
	{
		this->ConvertCxToRx();
	}
	else
	{
		m_Rx = 0;
	}
	
	if (m_Cursor.y < m_Offset.y)
	{
		m_Offset.y = m_Cursor.y;
	}

	if (m_Cursor.y >= m_Offset.y + m_BufferArea.y)
	{
		m_Offset.y = m_Cursor.y - m_BufferArea.y + 1;
	}

	if (m_Rx < m_Offset.x)
	{
		m_Offset.x = m_Rx;
	}

	if (m_Rx >= m_Offset.x + m_BufferArea.x)
	{
		m_Offset.x = m_Rx - m_BufferArea.x + 1;
	}
}

void Editor::ConvertCxToRx()
{
	const Row& row = m_Buffer[m_Cursor.y];
	
	m_Rx = 0;
	for (int i = 0; i < m_Cursor.x; i++)
	{
		if (row.real[i] == '\t')
		{
			// **** TabStop
			// **   (rx % ts)
			//   ** ts - (rx % ts)
			m_Rx += m_TabStop - (m_Rx % m_TabStop);
		}
		else
		{
			m_Rx++;
		}
	}
}

void Editor::DrawRows(std::shared_ptr<Terminal> terminal)
{
	for (int y = 0; y < m_BufferArea.y; y++)
	{
		int fileRow = y + m_Offset.y;
		
		if (fileRow >= m_Buffer.size())
		{
			this->DrawDefaultRow(y, terminal);
		}
		else
		{
			// TODO: Two modes: the exceeding part of the line is not shown or it is printed on next line.
			const std::vector<char>& line = m_Buffer[fileRow].render;
			
			int sizeToPrint = line.size() - m_Offset.x;

			if (sizeToPrint < 0)
			{
				sizeToPrint = 0;
			}
			
			if (sizeToPrint > m_BufferArea.x)
			{
				sizeToPrint = m_BufferArea.x;
			}

			if (sizeToPrint != 0)
			{
				std::vector<char> subvec = std::vector<char>(line.begin() + m_Offset.x, line.begin() + m_Offset.x + sizeToPrint);

				
				terminal->WriteCharVector(line, m_Offset.x, sizeToPrint);
			}
		}

		terminal->ClearCurrentRow();
		terminal->WriteString("\r\n");
	}
}

void Editor::DrawDefaultRow(int y, std::shared_ptr<Terminal> terminal)
{
	// TODO: Delete the welcome message.
	if (y == m_BufferArea.y / 3 && m_Buffer.size() == 0)
	{
		if (m_BufferArea.x > sizeof(WELCOME_MESSAGE))
		{
			// TODO: Position the welcome string more pretty.
			int padding = (m_BufferArea.x - sizeof(WELCOME_MESSAGE)) / 2;
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

void Editor::DrawStatusBar(std::shared_ptr<Terminal> terminal)
{
	terminal->SetBackgroundColor(m_BackgroundColor.Inverted());
	terminal->SetForegroundColor(m_ForegroundColor.Inverted());

	// Style: name, percent, line, character.
	// " - name - percent - LN - CN - modified ------- "
	std::stringstream ss;
	ss << " - " << m_FileName << " - ";
	ss << std::setw(3);
	if (m_Buffer.size() == 0)
	{
		ss << 0;
	}
	else if (m_Cursor.y == m_Buffer.size())
	{
		ss << 100;
	}
	else
	{
		int percent = (m_Cursor.y + 1) / static_cast<float>(m_Buffer.size()) * 100;
		ss << percent;
	}
	ss << '%';
	ss << " - L" << (m_Cursor.y + 1) << " - C" << (m_Cursor.x + 1) << " -";
	ss << " " << (m_FileDirty ? "**" : "  ") << " -";
	std::string statusStr = ss.str();

	if (statusStr.size() <= m_TerminalSize.x)
	{
		terminal->WriteString(statusStr);

		for (int i = statusStr.size(); i < m_TerminalSize.x; i++)
		{
			terminal->WriteCharacter(i == (m_TerminalSize.x - 1) ? ' ' : '-');
		}
	}
	else
	{
		terminal->WriteString(std::string(m_TerminalSize.x, ' '));
	}
	
	terminal->SetBackgroundColor(m_BackgroundColor);
	terminal->SetForegroundColor(m_ForegroundColor);

	terminal->WriteString("\r\n");
}

void Editor::DrawMessageBar(std::shared_ptr<Terminal> terminal)
{
	if (m_MessageBarText.size() >= m_TerminalSize.x)
	{
		terminal->WriteString(m_MessageBarText.substr(0, m_TerminalSize.x - 1 - 3));
		terminal->WriteString("...");
	}
	else
	{
		terminal->WriteString(m_MessageBarText);
		terminal->WriteString(std::string(m_TerminalSize.x - m_MessageBarText.size(), ' '));
	}
}

bool Editor::ProcessKey(TerminalKey key)
{
	switch (key.GetChar())
	{
	case 'q':
		if (key.IsCtrl())
		{
			if (m_FileDirty && m_ExitConfirmations > 0)
			{
				// TODO: Unsafe exit confirmation integer to string.
				this->ShowMessage("WARNING: File has unsaved changes. Press Ctrl-Q " + std::to_string(m_ExitConfirmations) + " more to really exit.", 1);
				m_ExitConfirmations--;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			this->InsertChar(key.GetChar());
		}
		break;

	case 'm':
		if (key.IsCtrl())
		{
			this->InsertNewLine();
		}
		else
		{
			this->InsertChar(key.GetChar());
		}
		break;

	case 's':
		if (key.IsCtrl())
		{
			this->Save();
		}
		else
		{
			this->InsertChar(key.GetChar());
		}
		break;
		
	case TerminalKeys::DELETE:
	case TerminalKeys::BACKSPACE:
		if (key.GetChar() == TerminalKeys::DELETE)
		{
			ProcessMoveCursor(TerminalKey(TerminalKeys::ARROW_RIGHT, false, false));
		}
		this->DeleteChar();
		break;

	case TerminalKeys::ESCAPE:
		break;
		
	case TerminalKeys::ARROW_UP:
	case TerminalKeys::ARROW_DOWN:
	case TerminalKeys::ARROW_LEFT:
	case TerminalKeys::ARROW_RIGHT:
		this->ProcessMoveCursor(key);
		break;

	case TerminalKeys::PAGE_UP:
	case TerminalKeys::PAGE_DOWN:
	{
		if (key.GetChar() == TerminalKeys::PAGE_UP)
		{
			m_Cursor.y = m_Offset.y;
		}
		else if (key.GetChar() == TerminalKeys::PAGE_DOWN)
		{
			m_Cursor.y = m_Offset.y + m_BufferArea.y - 1;
			if (m_Cursor.y > m_Buffer.size())
			{
				m_Cursor.y = m_Buffer.size();
			}
		}
		
		int times = m_BufferArea.y;
		while (times--)
		{
			if (key.GetChar() == TerminalKeys::PAGE_UP)
			{
				this->ProcessMoveCursor(TerminalKey(TerminalKeys::ARROW_UP, false, false));
			}
			else if (key.GetChar() == TerminalKeys::PAGE_DOWN)
			{
				this->ProcessMoveCursor(TerminalKey(TerminalKeys::ARROW_DOWN, false, false));
			}
		}
		break;
	}
	
	case TerminalKeys::HOME:
		m_Cursor.x = 0;
		break;
		
	case TerminalKeys::END:
		if (m_Cursor.y < m_Buffer.size())
		{
			m_Cursor.x = m_Buffer[m_Cursor.y].real.size();
		}
		break;
		
	default:
		if (key.IsCtrl() || key.IsAlt())
		{
			// TODO: Show the key press text representation in wrong key press.
			this->ShowMessage("Unbound key press: ", 1);
		}
		else
		{
			this->InsertChar(key.GetChar());
		}
		break;
	}

	m_ExitConfirmations = 3;
	return true;
}

void Editor::ProcessMoveCursor(TerminalKey key)
{
	// TODO: 2 modes: free move and only line move.
	// TODO: Word jump.
	// TODO: Save last cursor pos, if entered a small line.
	
	const std::vector<char>* line = (m_Cursor.y >= m_Buffer.size() ? nullptr : &m_Buffer[m_Cursor.y].real);
	
	switch (key.GetChar())
	{
	case TerminalKeys::ARROW_UP:
	case 'w':
		if (m_Cursor.y != 0)
		{
			m_Cursor.y--;	
		}
		break;
	case TerminalKeys::ARROW_LEFT:
	case 'a':
		if (m_Cursor.x != 0)
		{
			m_Cursor.x--;	
		}
		else if (m_Cursor.y > 0)
		{
			m_Cursor.y--;
			m_Cursor.x = m_Buffer[m_Cursor.y].real.size();
		}
		break;
	case TerminalKeys::ARROW_DOWN:
	case 's':
		if (m_Cursor.y < m_Buffer.size())
		{
			m_Cursor.y++;	
		}
		break;
	case TerminalKeys::ARROW_RIGHT:
	case 'd':
		if (line != nullptr && m_Cursor.x < line->size())
		{
			m_Cursor.x++;
		}
		else if (line != nullptr && m_Cursor.x == line->size())
		{
			m_Cursor.y++;
			m_Cursor.x = 0;
		}
		break;
	default:
		assert(false && "This should not be occured. Key argument is wrong, probably there is a mistake in switch in Editor::ProcessKey.");
		break;
	}

	line = (m_Cursor.y >= m_Buffer.size() ? nullptr : &m_Buffer[m_Cursor.y].real);
	int rowLength = line != nullptr ? line->size() : 0;
	if (m_Cursor.x > rowLength)
	{
		m_Cursor.x = rowLength;
	}
}

void Editor::SetTabSize(int newSize)
{
	if (newSize == m_TabStop)
	{
		return;
	}

	if (newSize <= 0)
	{
		throw std::out_of_range("new tab stop size is 0 or negative, but should be greater than 0");
	}

	m_TabStop = newSize;

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
		if (row.real[i] == '\t') {
            
			row.render.push_back(' ');
			while (row.render.size() % m_TabStop != 0)
			{
				row.render.push_back(' ');
			}
		}
		else
		{
			row.render.push_back(row.real[i]);
		}
	}
}

void Editor::RowInsertChar(Row& row, int at, char ch)
{
	if (at < 0)
	{
		at = 0;
	}

	if (at > row.real.size())
	{
		at = row.real.size();
	}
	
	row.real.insert(row.real.begin() + at, ch);

	this->UpdateRow(row);
}

void Editor::InsertChar(char ch)
{
	if (m_Cursor.y == m_Buffer.size())
	{
		this->AppendRow("");
	}
	
	this->RowInsertChar(m_Buffer[m_Cursor.y], m_Cursor.x, ch);
	m_Cursor.x++;
	m_FileDirty = true;
}

void Editor::ShowMessage(const std::string& msg, int lifeTime)
{
	m_MessageBarText = msg;
	m_MessageBarTextLifeTime = lifeTime;
}

void Editor::Save()
{
	std::ofstream file(m_FilePath);
	if (!file)
	{
		throw EditorFileIOError("unable to write to the file");
	}

	for (Row& row : m_Buffer)
	{
		// TODO: Different types of line endings to save.
		for (char ch : row.real)
		{
			file << ch;
		}
		
		file << "\n";

		if (file.fail())
		{
			throw EditorFileIOError("an error occured during the write to the file");
		}
	}

	this->ShowMessage("Wrote '" + m_FilePath + "'.", 1);
	m_FileDirty = false;
}

void Editor::RowDeleteChar(Row& row, int at)
{
	if (at < 0 || at > row.real.size())
	{
		return;
	}

	row.real.erase(row.real.begin() + at);
	m_FileDirty = true;
	this->UpdateRow(row);
}

void Editor::DeleteChar()
{
	// TODO: Bug when there is no text, but only one line is left.
	if (m_Cursor.y == m_Buffer.size()
		|| (m_Cursor.y == 0 && m_Cursor.x == 0))
	{
		return;
	}

	Row& row = m_Buffer[m_Cursor.y];
	if (m_Cursor.x > 0)
	{
		this->RowDeleteChar(row, m_Cursor.x - 1);
		m_Cursor.x--;
	}
	else
	{
		Row& previousRow = m_Buffer[m_Cursor.y - 1];
		Row& currentRow  = m_Buffer[m_Cursor.y];
		
		m_Cursor.x = previousRow.real.size();

		previousRow.real.insert(previousRow.real.end(), currentRow.real.begin(), currentRow.real.end());
		m_Buffer.erase(m_Buffer.begin() + m_Cursor.y);
		this->UpdateRow(previousRow);

		m_Cursor.y--;
	}
}

void Editor::InsertNewLine()
{
	if (m_Cursor.x == 0)
	{
		m_Buffer.insert(m_Buffer.begin() + m_Cursor.y, Row());
	}
	else
	{
		m_Buffer.insert(m_Buffer.begin() + m_Cursor.y + 1, Row());

		Row& currentRow = m_Buffer[m_Cursor.y];
		Row& newRow = m_Buffer[m_Cursor.y + 1];

		newRow.real.insert(newRow.real.end(), currentRow.real.begin() + m_Cursor.x, currentRow.real.end());
		currentRow.real.erase(currentRow.real.begin() + m_Cursor.x, currentRow.real.end());

		this->UpdateRow(currentRow);
		this->UpdateRow(newRow);
	}
	
	m_Cursor.y++;
	m_Cursor.x = 0;
}
