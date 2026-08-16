#pragma once

/*---------------
	ConsoleLog
----------------*/

enum class ConsoleColor
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class ConsoleLog
{
	enum { BUFFER_SIZE = 4096 };

public:
	ConsoleLog();
	~ConsoleLog();

public:
	void		WriteStdOut(ConsoleColor color, const WCHAR* str, ...);
	void		WriteStdErr(ConsoleColor color, const WCHAR* str, ...);

protected:
	void		SetColor(bool stdOut, ConsoleColor color);

private:
	HANDLE		_stdOut;
	HANDLE		_stdErr;
};