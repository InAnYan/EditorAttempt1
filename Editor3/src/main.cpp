/*
 * main.cpp - the main entry of the text editor.
 * Copyright (C) 2022 Ruslan Popov <ruslanpopov1512@gmail.com>.
 * 
 * This file is a part of project Editor3. 
 * This project is MIT licensed.
 */

#include <Terminal.hpp>
#include <Editor.hpp>

#include <stdlib.h>

/// Enter the raw mode. May throw an UnrecoverableTerminalImplementationError.
void EnterRawMode(Terminal& terminal);
/// Exit the raw mode. Ignore all UnrecoverableTerminalImplementationError.
void ExitRawMode(Terminal& terminal);

/// Full exit of the program with specific status. Print msg.
void Die(int status, Terminal& terminal, const std::string& msg);
/// Full exit of the program with specific status. Print prefixStr first, then postfixStr.
void Die(int status, Terminal& terminal, const std::string& prefixStr, const std::string& postfixStr);

int main(int argc, char* argv[])
{
	Terminal* terminal = CreateStdTerminal();

	try
	{
		EnterRawMode(*terminal);

		Editor editor;
		bool running = true;
		
		while (running)
		{
			editor.RefreshScreen(*terminal);
			char pressedKey = terminal->WaitAndReadKey();
			running = editor.ProcessKey(pressedKey, *terminal);
		}
	}
	catch (const UnrecoverableTerminalImplementationError& e)
	{
		Die(1, *terminal, "FATAL ERROR: ", e.what());
	}

	ExitRawMode(*terminal);
	return 0;
}

TerminalFeature g_RawModeFeaturesDisable[] =
{
	TerminalFeature::ECHOING,
	TerminalFeature::CANONICAL_MODE,
	TerminalFeature::SIGNALS,
	TerminalFeature::SOFTWARE_FLOW_CONTROL,
	TerminalFeature::LITERAL_SEND,
	TerminalFeature::CR_NL_TRANSFORM,
	TerminalFeature::OUTPUT_PROCESSING
};

void EnterRawMode(Terminal& terminal)
{
	for (TerminalFeature feature : g_RawModeFeaturesDisable)
	{
		terminal.DisableFeature(feature);
	}
}

void ExitRawMode(Terminal& terminal)
{
	try
	{
		terminal.ClearScreen();
	}
	catch (const UnrecoverableTerminalImplementationError& e)
	{
		// NOTE: Ignoring. Don't know what to do. Also because that is exit.
	}

	for (TerminalFeature feature : g_RawModeFeaturesDisable)
	{
		try
		{
			terminal.EnableFeature(feature);
		}
		catch (const UnrecoverableTerminalImplementationError& e)
		{
			// NOTE: Ignoring. Don't know what to do. Also because that is exit.
		}
	}
}

void Die(int status, Terminal& terminal, const std::string& msg)
{
	ExitRawMode(terminal);

	terminal.PrintString(msg);
	terminal.PrintString("\n");
	
	exit(status);
}

void Die(int status, Terminal& terminal, const std::string& prefixStr, const std::string& postfixStr)
{
	ExitRawMode(terminal);

	terminal.PrintString(prefixStr);
	terminal.PrintString(postfixStr);
	terminal.PrintString("\n");
	
	exit(status);
}
