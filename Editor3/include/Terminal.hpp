/*
 * Terminal.hpp - terminal interface.
 * Copyright (C) 2022 Ruslan Popov <ruslanpopov1512@gmail.com>.
 * 
 * This file is a part of project Editor3. 
 * This project is MIT licensed.
 */

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <exception>
#include <string>

/// Exception that represents an error on the implementation side.
class UnrecoverableTerminalImplementationError : std::exception
{
public:
	/// The constructor of UnrecoverableTerminalImplementationError.
	UnrecoverableTerminalImplementationError(const char* msg)
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

/// Different features of a terminal.
enum class TerminalFeature
{
	ECHOING,               /// Show input characters.
	CANONICAL_MODE,        /// Send the input only after ENTER hit.
	SIGNALS,               /// SIGINT and SIGSTP support.
	SOFTWARE_FLOW_CONTROL, /// Support of input termination.
	LITERAL_SEND,          /// A way to send a key literally.
	CR_NL_TRANSFORM,       /// Transform NL into CRNL.
	OUTPUT_PROCESSING,     /// Different ways of processing the output.
};

/// Information about key in key events.
class Key
{
public:
	/// The constructructor of Key.
	Key(char character, bool isCtrl)
		: m_Char(character), m_IsCtrl(isCtrl)
	{}

	/// Return the raw character of the Key.
	char getChar() const
	{ return m_Char; }

	/// Return true wherether the Ctrl key was pressed.
	bool isCtrl() const
	{ return m_IsCtrl; }
	
private:
	/// Raw character.
	char m_Char;
	/// Is Crtl'ed key.
	bool m_IsCtrl;
};

/// Interface for Terminal
/// Note: all functions of a Terminal may raise UnrecoverableTerminalImplementationError.
class Terminal
{
public:
	/// Terminal is an interface, so the default constructor is deleted.
	//Terminal() = delete;
	/// A virtual destruct of a Terminal.
	virtual ~Terminal() {}

	/// Enable a terminal feature. Do not do anything if it is already on.
	virtual void EnableFeature(TerminalFeature feature) = 0;
	/// Disable a terminal feature. Do not do anything if it is already off.
	virtual void DisableFeature(TerminalFeature feature) = 0;

	/// Set the timeout for read. The timeout measured in miliseconds.
	/// Note: the implementation may not support the exact timeout, but it should set it to the most close possible value.
	virtual void SetReadTimeout(unsigned timeout) = 0;

	virtual char WaitAndReadKey() = 0;

	/// Clear entire screen.
	virtual void ClearScreen() = 0;

	/// Position the Terminal's cursor.
	/// Note: the origin is at the top left corner and its coordinate is (1;1).
	virtual void PositionCursor(unsigned x, unsigned y) = 0;
	
	/// Print a string.
	virtual void PrintString(const std::string& str) = 0;
};

#define CTRL_KEY(k) ((k) & 0x1f)

/// A terminal based on standard input and output streams.
// TODO: Should it be a raw pointer or smart pointer?
Terminal* CreateStdTerminal();

#endif // TERMINAL_HPP
