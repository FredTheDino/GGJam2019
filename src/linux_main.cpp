#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

inline 
int __errorf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	return 0;
}

int (*print)(const char *format, ...) = printf;
int (*error)(const char *format, ...) = __errorf;
int (*format)(char *buff, const char *format, ...) = sprintf;

// Actual game :D
#include "core/block_main.cpp"

u64 linux_file_timestamp(const char *file)
{
	struct stat attr;
    stat(file, &attr);
	return attr.st_mtim.tv_nsec;
}

const char *linux_read_entire_file(const char *file)
{
	FILE *stream = fopen(file, "r");
	if (!stream)
		return NULL;
	fseek(stream, 0, SEEK_END);
	u64 file_size = ftell(stream) + 1;
	fseek(stream, 0, SEEK_SET);
	char *content = (char *) push_memory(sizeof(char) * file_size);
	fread(content, file_size, 1, stream);
	content[file_size - 1] = '\0';
	fclose(stream);
	return content;
}

int main(int argc, char **argv)
{
	file_timestamp = linux_file_timestamp;
	read_entire_file = linux_read_entire_file;
	run();
	return 0;
}
