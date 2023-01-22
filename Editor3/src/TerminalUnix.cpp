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
			throw new UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
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
			throw new UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual void EnableFeature(TerminalFeature feature) override
	{
		struct termios cur;
		if (tcgetattr(STDIN_FILENO, &cur) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
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
			throw new UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual void SetReadTimeout(unsigned timeout) override
	{
		struct termios cur;
		if (tcgetattr(STDIN_FILENO, &cur) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to get the state of the terminal");
		}
		
		cur.c_cc[VMIN] = 0;
		cur.c_cc[VTIME] = timeout / 100;

		if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to change the state of the terminal");
		}
	}

	virtual char WaitAndReadKey() override
	{
		int nread;
		char c;
		while((nread = read(STDIN_FILENO, &c, 1)) != 1)
		{
			if (nread == -1 && errno != EAGAIN)
			{
				throw new UnrecoverableTerminalImplementationError("unable to read a character from the terminal");
			}
		}
		return c;
	}

	/// Clear entire screen.
	virtual void ClearScreen() override
	{
		if (write(STDOUT_FILENO, "\x1b[2J", 4) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to clear the screen of the terminal");
		}
	}

	/// Position the Terminal's cursor.
	/// Note: the origin is at the top left corner and its coordinate is (1;1).
	virtual void PositionCursor(unsigned x, unsigned y) override
	{
		char buffer[100];

		int result = snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", x, y);
		if (result > sizeof(buffer) || result < 0) // TODO: Is that right comparison?
		{
			throw new UnrecoverableTerminalImplementationError("unable to change the position of the cursor because it is too big to form a message to the terminal");
		}

		if (write(STDIN_FILENO, &buffer, strlen(buffer)) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to write a message to the terminal");
		}
	}
	
	/// Print a string.
	virtual void PrintString(const std::string& str) override
	{
		if (write(STDIN_FILENO, str.c_str(), str.size()) == -1)
		{
			throw new UnrecoverableTerminalImplementationError("unable to print a string to the terminal");
		}
	}
};

Terminal* CreateStdTerminal()
{
	return new UnixTerminal;
}

#endif // EDITOR_COMPILE_UNIX
