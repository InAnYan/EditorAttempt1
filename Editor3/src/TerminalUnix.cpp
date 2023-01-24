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
		char c;
		while((nread = read(STDIN_FILENO, &c, 1)) != 1)
		{
			if (nread == -1 && errno != EAGAIN)
			{
				throw UnrecoverableTerminalImplementationError("unable to read a character from the terminal");
			}
		}
		return c;
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

		if (write(STDIN_FILENO, &buffer, strlen(buffer)) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to write a message to the terminal");
		}
	}
	
	virtual void PrintString(const std::string& str) override
	{
		if (write(STDIN_FILENO, str.c_str(), str.size()) == -1)
		{
			throw UnrecoverableTerminalImplementationError("unable to print a string to the terminal");
		}
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

	virtual void HideCursor() override
	{
		if (write(STDOUT_FILENO, "\x1b[?25l", 6) != 6)
		{
			throw UnrecoverableTerminalImplementationError("unable to hide the cursor of the terminal");
		}
	}
	
	virtual void ShowCursor() override
	{
		if (write(STDOUT_FILENO, "\x1b[?25h", 6) != 6)
		{
			throw UnrecoverableTerminalImplementationError("unable to show the cursor of the terminal");
		}
	}

	virtual void ClearCurrentRow() override
	{
		if (write(STDOUT_FILENO, "\x1b[K", 3) != 3)
		{
			throw UnrecoverableTerminalImplementationError("unable to clear current row");
		}
	}
	
private:
	const TerminalCoord GetSizeFallback()
	{
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
		{
			throw UnrecoverableTerminalImplementationError("unable to get the size of the terminal");
		}

		return GetCursorPosition();
	}
};

Terminal* CreateStdTerminal()
{
	return new UnixTerminal;
}

#endif // EDITOR_COMPILE_UNIX
