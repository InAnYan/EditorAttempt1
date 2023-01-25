/*
 * TerminalUnix.cpp - a standard terminal implementation for UNIX systems.
 * Copyright (C) 2022 Ruslan Popov <ruslanpopov1512@gmail.com>.
 * 
 * This file is a part of project Editor3. 
 * This project is MIT licensed.
 */

#ifdef EDITOR_COMPILE_UNIX

#include <Terminal.hpp>

#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sstream>

class UnixTerminal : public Terminal
{
public:
	UnixTerminal()
	{}
	
	virtual ~UnixTerminal() override
	{}
	
	virtual void DisableFeature(TerminalFeature feature) override
	{
		struct termios cur;
		if (tcgetattr(STDIN_FILENO, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
		}

		switch (feature)
		{
		case TerminalFeature::ECHOING:
			cur.c_lflag &= ~(ECHO);
			break;
		case TerminalFeature::CANONICAL_MODE:
			cur.c_lflag &= ~(ICANON);
			break;
		case TerminalFeature::SIGNALS:
			cur.c_lflag &= ~(ISIG);
			break;
		case TerminalFeature::SOFTWARE_FLOW_CONTROL:
			cur.c_iflag &= ~(IXON);
			break;
		case TerminalFeature::LITERAL_SEND:
			cur.c_lflag &= ~(IEXTEN);
			break;
		case TerminalFeature::CR_NL_TRANSFORM:
			cur.c_iflag &= ~(ICRNL);
			break;
		case TerminalFeature::OUTPUT_PROCESSING:
			cur.c_oflag &= ~(OPOST);
			break;
		}
		
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual void EnableFeature(TerminalFeature feature) override
	{
		struct termios cur;
		if (tcgetattr(STDIN_FILENO, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
		}

		switch (feature)
		{
		case TerminalFeature::ECHOING:
			cur.c_lflag |= (ECHO);
			break;
		case TerminalFeature::CANONICAL_MODE:
			cur.c_lflag |= (ICANON);
			break;
		case TerminalFeature::SIGNALS:
			cur.c_lflag |= (ISIG);
			break;
		case TerminalFeature::SOFTWARE_FLOW_CONTROL:
			cur.c_iflag |= (IXON);
			break;
		case TerminalFeature::LITERAL_SEND:
			cur.c_lflag |= (IEXTEN);
			break;
		case TerminalFeature::CR_NL_TRANSFORM:
			cur.c_iflag |= (ICRNL);
			break;
		case TerminalFeature::OUTPUT_PROCESSING:
			cur.c_oflag |= (OPOST);
			break;
		}
		
		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual void SetReadTimeout(unsigned timeout) override
	{
		struct termios cur;
		if (tcgetattr(STDIN_FILENO, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
		}
		
		cur.c_cc[VMIN] = 0;
		cur.c_cc[VTIME] = timeout / 100;

		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual TerminalKey WaitAndReadKey() override
	{
		int nread;
		unsigned c;
		while((nread = read(STDIN_FILENO, &c, 1)) != 1)
		{
			if (nread == -1 && errno != EAGAIN)
			{
				throw UnrecoverableTerminalImplementationError("unable to read a character from the terminal");
			}
		}

		if (c == '\x1b')
		{
			return WaitAndReadEscapeSequence();
		}
		
		return MakeKey(c);
	}

	virtual const TerminalCoord GetSize() override
	{
		struct winsize ws;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
		{
			return GetSizeFallback();	
		}
		else
		{
			return { ws.ws_col, ws.ws_row };
		}
	}

	virtual const TerminalCoord GetCursorPosition() override
	{
		char buf[16];
		unsigned i = 0;

		if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the size of the terminal");
		}
		
		while (i < sizeof(buf) - 1)
		{
			if (read(STDIN_FILENO, &buf[i], 1) != 1)
			{
				break;
			}
			if (buf[i] == 'R')
			{
				break;
			}
			i++;
		}
		
		buf[i] = '\0';

		if (buf[0] != '\x1b' || buf[1] != '[')
		{
			throw UnrecoverableTerminalImplementationError("unable to get the size of the terminal");
		}

		TerminalCoord size;
		if (sscanf(&buf[2], "%d;%d", &size.column, &size.row) != 2)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the size of the terminal");
		}

		return size;
	}
	
	virtual void ClearScreen() override
	{
		if (write(STDOUT_FILENO, "\x1b[2J", 4) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to clear the screen of the terminal");
		}
	}

	virtual void SetCursorPosition(TerminalCoord coord) override
	{
		char buffer[16];

		int result = snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", coord.row + 1, coord.column + 1);
		if (result > sizeof(buffer) || result < 0) // TODO: Is that right comparison?
		{
			throw UnrecoverableTerminalImplementationError("unable to change the position of the cursor because it is too big to form a message to the terminal");
		}

		m_Buffer << buffer;
	}
	
	virtual void WriteString(const std::string& str) override
	{
		m_Buffer << str;
	}

	virtual void HideCursor() override
	{
		m_Buffer << "\x1b[?25l";
	}
	
	virtual void ShowCursor() override
	{
		m_Buffer << "\x1b[?25h";
	}

	virtual void ClearCurrentRow() override
	{
		m_Buffer << "\x1b[K";
	}

	virtual void Flush() override
	{
		std::string string = m_Buffer.str();
		int result = write(STDOUT_FILENO, string.c_str(), string.size());
		m_Buffer.str("");

		if (result != string.size())
		{
			throw UnrecoverableTerminalImplementationError("unable to write to the terminal");
		}
	}
	
private:
	std::stringstream m_Buffer;

	/// Fallback, if the ioctl call does not work.
	const TerminalCoord GetSizeFallback()
	{
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the size of the terminal");
		}

		return GetCursorPosition();
	}

	/// Convert raw key value to TerminalKey.
	TerminalKey MakeKey(unsigned c)
	{
		// TODO: Support for Alt.
		
		if (c == (c & 0x1F))
		{
			return TerminalKey(c + 96, true, false);
		}
		else
		{
			return TerminalKey(c, false, false);
		}
	}

	/// Read the key that is represented by escape sequence.
	TerminalKey WaitAndReadEscapeSequence()
	{
		char seq[3];

		if (read(STDIN_FILENO, &seq[0], 1) != 1)
		{
			return MakeKey('\x1b');
		}

		if (read(STDIN_FILENO, &seq[1], 1) != 1)
		{
			return MakeKey('\x1b');
		}

		if (seq[0] != '[')
		{
			return MakeKey('\x1b');
		}

		if (seq[1] >= '0' && seq[1] <= '9')
		{
			if (read(STDIN_FILENO, &seq[2], 1) != 1)
			{
				return MakeKey('\x1b');
			}

			if (seq[2] == '~')
			{
				switch (seq[1])
				{
				case '5':
					return MakeKey(TerminalKeys::PAGE_UP);
				case '6':
					return MakeKey(TerminalKeys::PAGE_DOWN);
				default:
					return MakeKey('\x1b');
				}
			}			
		}
		
		switch (seq[1])
		{
		case 'A':
			return MakeKey(TerminalKeys::ARROW_UP);
		case 'B':
			return MakeKey(TerminalKeys::ARROW_DOWN);
		case 'C':
			return MakeKey(TerminalKeys::ARROW_RIGHT);
		case 'D':
			return MakeKey(TerminalKeys::ARROW_LEFT);
		}
		

		return MakeKey('\x1b');
	}
};

Terminal* CreateStdTerminal()
{
	return new UnixTerminal;
}

#endif // EDITOR_COMPILE_UNIX
