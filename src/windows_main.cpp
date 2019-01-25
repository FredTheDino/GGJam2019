#pragma warning( push, 0 )
#include <windows.h>
#include <stdio.h>
#pragma warning( pop )
#pragma warning( disable : 4201 )

int (*print)(const char *format, ...);
int (*error)(const char *format, ...);

// Actual game.
#include "core/block_main.cpp"

// A wrapper for printing strings in windows.
int win_printf(const char *format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	int len = vsprintf(buffer, format, args);
	OutputDebugString(buffer);
	return len;
}

const char *win_read_entire_file(const char *file)
{
	HANDLE handle = CreateFileA(
			file, GENERIC_READ, FILE_SHARE_READ, 
			NULL, OPEN_EXISTING, NULL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return NULL;
	LARGE_INTEGER file_size;
	GetFileSizeEx(handle, &file_size);

	file_size.QuadPart++;
	char *content = (char *) push_array(char, file_size.QuadPart);
	ASSERT(file_size.QuadPart <= 0xFFFFFFFF);

	ASSERT(ReadFile(handle, content, file_size.QuadPart, NULL, 0));

	CloseHandle(handle);

	content[file_size.QuadPart - 1] = '\0';
	return content;
}

u64 win_file_timestamp(const char *file)
{
	HANDLE handle = CreateFileA(
			file, 
			GENERIC_READ, 
			FILE_SHARE_READ, 
			NULL, 
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	FILETIME last_edit;
	GetFileTime(handle, NULL, NULL, &last_edit);
	CloseHandle(handle);
	return last_edit.dwLowDateTime;
}

int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow)
{
	print = win_printf;
	error = win_printf; // TODO: Do I need to make a seperate stream on windows?
	file_timestamp = win_file_timestamp;
	read_entire_file = win_read_entire_file;
	run();
	return 0;
}
