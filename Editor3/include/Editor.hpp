#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP

#include "Terminal.hpp"

/// The editor.
class Editor
{
public:
	/// Redraw the editor screen.
	void RefreshScreen(Terminal& terminal);
	/// Process the key. Return true if the editor is alive, otherwise, false.
	bool ProcessKey(char key, Terminal& terminal);
};

#endif // EDITOR_EDITOR_HPP
