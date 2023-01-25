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
class TerminalKey
{
public:
	/// The constructructor of TerminalKey.
	TerminalKey(unsigned character, bool isCtrl, bool isAlt)
		: m_Char(character), m_IsCtrl(isCtrl), m_IsAlt(isAlt)
	{}

	/// Return the base character of the Key.
	unsigned GetChar() const
	{ return m_Char; }

	// Return true if the key was modified by Ctrl.
	bool IsCtrl() const
	{ return m_IsCtrl; }

	// Return true if the key was modified by Alt.
	bool IsAlt() const
	{ return m_IsAlt; }
	
private:
	/// Base character.
	unsigned m_Char;
	/// Is Crtl'ed key.
	bool m_IsCtrl;
	/// Is Alt'ed key.
	bool m_IsAlt;
};

// TODO: DOCUMENT.
namespace TerminalKeys
{
	enum
	{
		ARROW_LEFT = 1000,
		ARROW_RIGHT,
		ARROW_UP,
		ARROW_DOWN,

		PAGE_UP,
		PAGE_DOWN
	};
}

/// Terminal coordinate representation. Also used to represent size of a terminal.
struct TerminalCoord
{
	/// The X coordinate.
	unsigned column;
	/// The Y coordinate.
	unsigned row;
};

/// Interface for Terminal.
/// Note: all functions of a Terminal may raise UnrecoverableTerminalImplementationError.
/// Note: there are instant operations and buffered. The result of instant operation will be instant. The result of buffered operation may be seen only after call to Terminal::Flush or without the call.
/// Note: The implementation is permitted to perform buffered operations as instant.
class Terminal
{
public:
	/// Terminal is an interface, so the default constructor is deleted.
	//Terminal() = delete;
	/// A virtual destruct of a Terminal.
	virtual ~Terminal() {}

	/// Instant operations:
	
    /// Get the terminal size (instant operation). The result value may vary during the execution.
	virtual const TerminalCoord GetSize() = 0;
	
	/// Enable a terminal feature (instant operation). Do not do anything if it is already on.
	virtual void EnableFeature(TerminalFeature feature) = 0;
	/// Disable a terminal feature (instant operation). Do not do anything if it is already off.
	virtual void DisableFeature(TerminalFeature feature) = 0;

	/// Set the timeout for read (instant operation). The timeout measured in miliseconds.
	/// Note: the implementation may not support the exact timeout, but it should set it to the most close possible value.
	virtual void SetReadTimeout(unsigned timeout) = 0;

	/// Get the position of terminal cursor (instant operation).
	virtual const TerminalCoord GetCursorPosition() = 0;

	/// UNDOCUMENTED (immediate operation).
	// TODO: DOCUMENT.
	virtual TerminalKey WaitAndReadKey() = 0;

	/// Perform all buferred operations at once.
	virtual void Flush() = 0;
	
	/// Buffered operations:
	
	/// Clear entire screen (buffered operation).
	virtual void ClearScreen() = 0;
	/// Clear row on the cursor (buffered operation).
	virtual void ClearCurrentRow() = 0;

    
	/// Position the Terminal's cursor (buffered operation).
	/// Note: the origin is at the top left corner and its coordinate is (0; 0).
	virtual void SetCursorPosition(TerminalCoord coord) = 0;
	
	/// Hide terminal cursor (buffered operation). Do nothing if hidden,
	virtual void HideCursor() = 0;
	/// Show terminal cursor (buffered operation). Do nothing if shown.
	virtual void ShowCursor() = 0;
	
	/// Print a string (buffered operation).
	virtual void WriteString(const std::string& str) = 0;
};

/// A terminal based on standard input and output streams.
// TODO: Should it be a raw pointer or smart pointer?
Terminal* CreateStdTerminal();

#endif // TERMINAL_HPP
