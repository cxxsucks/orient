/*
 * Dirent interface for Microsoft Visual Studio
 *
 * Copyright (C) 1998-2019 Toni Ronkko
 * This file is part of dirent.  Dirent may be freely distributed
 * under the MIT license.  For all details and documentation, see
 * https://github.com/tronkko/dirent
 */
#ifndef DIRENT_H
#define DIRENT_H

/* Hide warnings about unreferenced local functions */
#if defined(__clang__)
#	pragma clang diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
#	pragma warning(disable:4505)
#elif defined(__GNUC__)
#	pragma GCC diagnostic ignored "-Wunused-function"
#endif

/*
 * Include windows.h without Windows Sockets 1.1 to prevent conflicts with
 * Windows Sockets 2.0.
 */
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

/* Indicates that d_type field is available in dirent structure */
#define _DIRENT_HAVE_D_TYPE

/* Indicates that d_namlen field is available in dirent structure */
#define _DIRENT_HAVE_D_NAMLEN

/* Entries missing from MSVC 6.0 */
#if !defined(FILE_ATTRIBUTE_DEVICE)
#	define FILE_ATTRIBUTE_DEVICE 0x40
#endif

/* File type and permission flags for stat(), general mask */
#if !defined(S_IFMT)
#	define S_IFMT _S_IFMT
#endif

/* Directory bit */
#if !defined(S_IFDIR)
#	define S_IFDIR _S_IFDIR
#endif

/* Character device bit */
#if !defined(S_IFCHR)
#	define S_IFCHR _S_IFCHR
#endif

/* Pipe bit */
#if !defined(S_IFFIFO)
#	define S_IFFIFO _S_IFFIFO
#endif

/* Regular file bit */
#if !defined(S_IFREG)
#	define S_IFREG _S_IFREG
#endif

/* Read permission */
#if !defined(S_IREAD)
#	define S_IREAD _S_IREAD
#endif

/* Write permission */
#if !defined(S_IWRITE)
#	define S_IWRITE _S_IWRITE
#endif

/* Execute permission */
#if !defined(S_IEXEC)
#	define S_IEXEC _S_IEXEC
#endif

/* Pipe */
#if !defined(S_IFIFO)
#	define S_IFIFO _S_IFIFO
#endif

/* Block device */
#if !defined(S_IFBLK)
#	define S_IFBLK 0
#endif

/* Link */
#if !defined(S_IFLNK)
#	define S_IFLNK 40960
#endif

/* Socket */
#if !defined(S_IFSOCK)
#	define S_IFSOCK 0
#endif

/* Read user permission */
#if !defined(S_IRUSR)
#	define S_IRUSR S_IREAD
#endif

/* Write user permission */
#if !defined(S_IWUSR)
#	define S_IWUSR S_IWRITE
#endif

/* Execute user permission */
#if !defined(S_IXUSR)
#	define S_IXUSR 0
#endif

/* Read group permission */
#if !defined(S_IRGRP)
#	define S_IRGRP 0
#endif

/* Write group permission */
#if !defined(S_IWGRP)
#	define S_IWGRP 0
#endif

/* Execute group permission */
#if !defined(S_IXGRP)
#	define S_IXGRP 0
#endif

/* Read others permission */
#if !defined(S_IROTH)
#	define S_IROTH 0
#endif

/* Write others permission */
#if !defined(S_IWOTH)
#	define S_IWOTH 0
#endif

/* Execute others permission */
#if !defined(S_IXOTH)
#	define S_IXOTH 0
#endif

/* Maximum length of file name */
#if !defined(PATH_MAX)
#	define PATH_MAX MAX_PATH
#endif
#if !defined(FILENAME_MAX)
#	define FILENAME_MAX MAX_PATH
#endif
#if !defined(NAME_MAX)
#	define NAME_MAX FILENAME_MAX
#endif

/* File type flags for d_type */
#define DT_UNKNOWN 0
#define DT_REG S_IFREG
#define DT_DIR S_IFDIR
#define DT_FIFO S_IFIFO
#define DT_SOCK S_IFSOCK
#define DT_CHR S_IFCHR
#define DT_BLK S_IFBLK
#define DT_LNK S_IFLNK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
 * File type macros.  Note that block devices, sockets and links cannot be
 * distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
 * only defined for compatibility.  These macros should always return false
 * on Windows.
 */
#if !defined(S_ISFIFO)
#	define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISDIR)
#	define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG)
#	define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISLNK)
#	define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK)
#	define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISCHR)
#	define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISBLK)
#	define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#endif

/* Return the exact length of the file name without zero terminator */
#define _D_EXACT_NAMLEN(p) ((p)->d_namlen)

/* Return the maximum size of a file name */
#define _D_ALLOC_NAMLEN(p) ((PATH_MAX)+1)

/* Wide-character version */
struct _wdirent {
	/* Always zero */
	long d_ino;

	/* File position within stream */
	long d_off;

	/* Structure size */
	unsigned short d_reclen;

	/* Length of name without \0 */
	size_t d_namlen;

	/* File type */
	int d_type;

	/* File name */
	wchar_t d_name[PATH_MAX+1];
};
typedef struct _wdirent _wdirent;

struct _WDIR {
	/* Current directory entry */
	struct _wdirent ent;

	/* Private file data */
	WIN32_FIND_DATAW data;

	/* True if data is valid */
	int cached;

	/* Win32 search handle */
	HANDLE handle;

	/* Initial directory name */
	wchar_t *patt;
};
typedef struct _WDIR _WDIR;

/* Multi-byte character version */
struct dirent {
	/* Always zero */
	long d_ino;

	/* File position within stream */
	long d_off;

	/* Structure size */
	unsigned short d_reclen;

	/* Length of name without \0 */
	size_t d_namlen;

	/* File type */
	int d_type;

	/* File name */
	char d_name[PATH_MAX+1];
};
typedef struct dirent dirent;

struct DIR {
	struct dirent ent;
	struct _WDIR *wdirp;
};
typedef struct DIR DIR;


/* Dirent functions */
DIR *opendir(const char *dirname);
_WDIR *_wopendir(const wchar_t *dirname);

struct dirent *readdir(DIR *dirp);
struct _wdirent *_wreaddir(_WDIR *dirp);

int readdir_r(
	DIR *dirp, struct dirent *entry, struct dirent **result);
int _wreaddir_r(
	_WDIR *dirp, struct _wdirent *entry, struct _wdirent **result);

int closedir(DIR *dirp);
int _wclosedir(_WDIR *dirp);

void rewinddir(DIR* dirp);
void _wrewinddir(_WDIR* dirp);

int scandir(const char *dirname, struct dirent ***namelist,
	int (*filter)(const struct dirent*),
	int (*compare)(const struct dirent**, const struct dirent**));

int alphasort(const struct dirent **a, const struct dirent **b);

int versionsort(const struct dirent **a, const struct dirent **b);

int strverscmp(const char *a, const char *b);

/* For compatibility with Symbian */
#define wdirent _wdirent
#define WDIR _WDIR
#define wopendir _wopendir
#define wreaddir _wreaddir
#define wclosedir _wclosedir
#define wrewinddir _wrewinddir

/* Compatibility with older Microsoft compilers and non-Microsoft compilers */
#if !defined(_MSC_VER) || _MSC_VER < 1400
#	define wcstombs_s dirent_wcstombs_s
#	define mbstowcs_s dirent_mbstowcs_s
#endif

/* Optimize dirent_set_errno() away on modern Microsoft compilers */
#if defined(_MSC_VER) && _MSC_VER >= 1400
#	define dirent_set_errno _set_errno
#endif


/* Internal utility functions */
WIN32_FIND_DATAW *dirent_first(_WDIR *dirp);
WIN32_FIND_DATAW *dirent_next(_WDIR *dirp);

#if !defined(_MSC_VER) || _MSC_VER < 1400
int dirent_mbstowcs_s(
	size_t *pReturnValue, wchar_t *wcstr, size_t sizeInWords,
	const char *mbstr, size_t count);
#endif

#if !defined(_MSC_VER) || _MSC_VER < 1400
int dirent_wcstombs_s(
	size_t *pReturnValue, char *mbstr, size_t sizeInBytes,
	const wchar_t *wcstr, size_t count);
#endif

#if !defined(_MSC_VER) || _MSC_VER < 1400
void dirent_set_errno(int error);
#endif

#endif // DIRENT_H