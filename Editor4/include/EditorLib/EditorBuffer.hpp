#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

namespace Editor
{
	class Buffer
	{
	public:
		void Insert(char ch);
		void Insert(const std::string& ch);

		void Remove(int count);

		char GetCharAt(Coord coord) const;

		int GetLinesCount() const;
		int GetLineSize(int line) const;
		
		void MoveCursorUp();
		void MoveCursorUp();
		void MoveCursorUp();
		void MoveCursorUp();

		Coord GetCursor() const;
		
	private:
		std::vector<std::vector<char>> m_Lines;
		Coord m_Cursor;
	}; // class Buffer

	class BufferView
	{
	public:
		BufferView(Buffer buf);
		
		void Draw(Terminal terminal, Rect rect);

		TermLib::Coord GetCursorInRect(Rect rect);

		Buffer GetBuffer();
		Buffer ChangeBuffer(Buffer buffer);
		
	private:
		Buffer buffer;
		TermLib::Coord m_Cursor;
		TermLib::Coord m_Offset;
		int m_Rx;

		SplitType split;
		Buffer* child;
		float splitFactor;
	};
} // namespace Buffer

#endif // EDITOR_BUFFER_HPP
