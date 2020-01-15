#include "utils.h"
#include "math/math_extra.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

#if PL_LINUX
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#elif PL_WINDOWS
#include <Windows.h>
#include <shlobj_core.h>
#include <stdio.h>
#include <sys/types.h>
#endif

#if PL_LINUX
int is_regular_file(const char* path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

int is_dir(const char* path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

size_t listdir(const char* dir, char** result, size_t size, size_t depth)
{
	DIR* dp = opendir(dir);

	struct dirent* ep;
	if (dp != NULL)
	{
		while ((ep = readdir(dp)))
		{
			char full_path[PATH_MAX];
			strcpy(full_path, dir);
			if (dir[strlen(dir) - 1] != '/')
				strcat(full_path, "/");
			strcat(full_path, ep->d_name);

			if (!strcmp(ep->d_name, "..") || !strcmp(ep->d_name, "."))
				continue;

			if (is_dir(full_path))
			{
				if (depth == 1)
					continue;
				size_t tmp = listdir(full_path, result, size, depth - 1);
				result += size - tmp;
				size = tmp;
			}
			else if (result && size)
			{
				strcpy(*result, full_path);
				result++;
				size--;
			}
			else
			{
				return 0;
			}
		}
		(void)closedir(dp);
	}
	else
	{
		printf("Unable to open directory %s\n", dir);
		return 0;
	}
	return size;
}

int find_file(const char* dir, char* result, size_t size, const char* filename)
{
	DIR* dp = opendir(dir);

	struct dirent* ep;
	if (dp != NULL)
	{
		while ((ep = readdir(dp)))
		{
			char full_path[PATH_MAX];
			strcpy(full_path, dir);
			if (dir[strlen(dir) - 1] != '/')
				strcat(full_path, "/");
			strcat(full_path, ep->d_name);

			if (!strcmp(ep->d_name, "..") || !strcmp(ep->d_name, "."))
				continue;

			if (is_dir(full_path))
			{
				// File was found in subdir
				if (find_file(full_path, result, size, filename) == EXIT_SUCCESS)
					return EXIT_SUCCESS;
			}
			else if (strcmp(ep->d_name, filename) == 0)
			{
				strncpy(result, full_path, size);
				return EXIT_SUCCESS;
			}
		}
		(void)closedir(dp);
	}
	else
	{
		printf("Unable to open directory %s\n", dir);
		return EXIT_FAILURE;
	}
	return EXIT_FAILURE;
}
void create_dirs(const char* path)
{
	char buf[2048];
	size_t len = strlen(path);
	for (size_t i = 0; i < len; i++)
	{
		if (path[i] == '/' || path[i] == '\\' || i == len - 1)
		{
			strncpy(buf, path, i + 1);
			buf[i + 1] = '\0';
			if (!strcmp(buf, "./") || !strcmp(buf, "../") || !strcmp(buf, "") || !strcmp(buf + 1, ":") ||
				!strcmp(buf, ".\\") || !strcmp(buf, "..\\"))
				continue;
			mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
	}
}
#elif PL_WINDOWS
int is_regular_file(const char* path)
{
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int is_dir(const char* path)
{
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

size_t listdir(const char* dir, char** result, size_t size, size_t depth)
{
	char pattern[2048];
	snprintf(pattern, 2048, "%s\\*", dir);

	WIN32_FIND_DATA data;
	HANDLE hFind;

	char fname[2048];
	if ((hFind = FindFirstFileA(pattern, &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(fname, "%ws", data.cFileName);
			if (!strcmp(fname, ".") || !strcmp(fname, ".."))
				continue;

			char full_path[2048];
			strcpy(full_path, dir);
			if (dir[strlen(dir) - 1] != '/')
				strcat(full_path, "/");
			strcat(full_path, fname);

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (depth == 1)
					continue;
				size_t tmp = listdir(full_path, result, size, depth - 1);
				result += size - tmp;
				size = tmp;
			}
			else if (result && size)
			{
				posix_path(full_path, full_path, strlen(full_path));
				strcpy(*result, full_path);
				result++;
				size--;
			}
			else
			{
				return 0;
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	return size;
}
int find_file(const char* dir, char* result, size_t size, const char* filename)
{
	char pattern[2048];
	snprintf(pattern, 2048, "%s\\*", dir);

	WIN32_FIND_DATA data;
	HANDLE hFind;

	char fname[2048];
	if ((hFind = FindFirstFileA(pattern, &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(fname, "%ws", data.cFileName);
			if (!strcmp(fname, ".") || !strcmp(fname, ".."))
				continue;

			char full_path[2048];
			strcpy(full_path, dir);
			if (dir[strlen(dir) - 1] != '/')
				strcat(full_path, "/");
			strcat(full_path, fname);

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (find_file(full_path, result, size, filename) == EXIT_SUCCESS)
					return EXIT_SUCCESS;
			}
			else if (strcmp(fname, filename) == 0)
			{
				posix_path(full_path, full_path, strlen(full_path));
				strncpy(result, full_path, size);
				return EXIT_SUCCESS;
			}
			else
			{
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	return EXIT_FAILURE;
}
void create_dirs(const char* path)
{
	char buf[2048];
	size_t len = strlen(path);
	for (size_t i = 0; i < len; i++)
	{
		if (path[i] == '/' || path[i] == '\\' || i == len - 1)
		{
			strncpy(buf, path, i + 1);
			buf[i + 1] = '\0';
			if (!strcmp(buf, "./") || !strcmp(buf, "../") || !strcmp(buf, "") || !strcmp(buf + 1, ":") ||
				!strcmp(buf, ".\\") || !strcmp(buf, "..\\"))
				continue;
			CreateDirectoryA(buf, NULL);
		}
	}
}
#endif

void get_filename(const char* path, char* result, size_t size)
{
	for (size_t i = strlen(path); i != 0; i--)
	{
		// If it is at a path delimeter
		if (path[i] == '\\' || path[i] == '/')
		{
			memmove(result, path + min(i + 1, size), min(i + 1, strlen(path) - size));
			result[min(i + 1, size)] = '\0';
			break;
		}
	}
}

void get_dir(const char* path, char* result, size_t size)
{
	for (size_t i = strlen(path); i != 0; i--)
	{
		// If it is at a path delimeter
		if (path[i] == '\\' || path[i] == '/')
		{
			memmove(result, path, min(i + 1, size));
			result[min(i + 1, size)] = '\0';
			break;
		}
	}
}

#if PL_LINUX
void set_workingdir(const char* dir)
{
	if (chdir(dir))
	{
		printf("Invalid working directory %s\n", dir);
	}
}
#elif PL_WINDOWS
void set_workingdir(const char* dir)
{
	_chdir(dir);
}
#endif

void dir_up(const char* path, char* result, size_t size, size_t steps)
{

	size_t len = strlen(path) - 1;
	for (size_t i = len; i != 0; i--)
	{
		if (steps == 0 || i == 1)
		{
			memmove(result, path, min(i + 1, size));
			result[min(i + 1, size)] = '\0';
			break;
		}

		// If it is at a path delimeter
		if ((path[i] == '\\' || path[i] == '/') && i != len)
		{
			steps--;
		}
	}
	if (strcmp(result, "./") == 0)
	{
		strcpy(result, "../");
		return;
	}
	int contains_dir = 0;
	for (char* p = result; *p != '\0'; p++)
	{
		if (*p != '/' && *p != '\\' && *p != '.')
			contains_dir = 1;
	}
	if (!contains_dir)
	{
		strncat(result, "../", size);
	}
}

void posix_path(const char* path, char* result, size_t size)
{
	if (path != result)
	{
		memmove(result, path, min(strlen(path), size));
		result[min(strlen(path), size)] = '\0';
	}

	for (char* p = result; *p != '\0'; p++)
	{
		if (*p == '\\')
			*p = '/';
	}
}

void replace_string(const char* src, char* result, size_t size, char find, char replace)
{
	if (src != result)
	{
		memmove(result, src, min(strlen(src), size));
		result[min(strlen(src), size)] = '\0';
	}

	for (char* p = result; *p != '\0'; p++)
	{
		if (*p == find)
			*p = replace;
	}
}
