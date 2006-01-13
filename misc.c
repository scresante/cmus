/* 
 * Copyright 2004-2005 Timo Hirvonen
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <misc.h>
#include <prog.h>
#include <xmalloc.h>
#include <xstrjoin.h>
#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>

const char *cmus_config_dir = NULL;
const char *cmus_cache_dir = NULL;
const char *home_dir = NULL;
const char *user_name = NULL;

char **get_words(const char *text)
{
	char **words;
	int i, j, count;

	while (*text == ' ')
		text++;

	count = 0;
	i = 0;
	while (text[i]) {
		count++;
		while (text[i] && text[i] != ' ')
			i++;
		while (text[i] == ' ')
			i++;
	}
	words = xnew(char *, count + 1);

	i = 0;
	j = 0;
	while (text[i]) {
		int start = i;

		while (text[i] && text[i] != ' ')
			i++;
		words[j++] = xstrndup(text + start, i - start);
		while (text[i] == ' ')
			i++;
	}
	words[j] = NULL;
	return words;
}

static int dir_exists(const char *dirname)
{
	DIR *dir;

	dir = opendir(dirname);
	if (dir == NULL) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	closedir(dir);
	return 1;
}

static void make_dir(const char *dirname)
{
	int rc;

	rc = dir_exists(dirname);
	if (rc == 1)
		return;
	if (rc == -1)
		die_errno("error: opening `%s'", dirname);
	rc = mkdir(dirname, 0700);
	if (rc == -1)
		die_errno("error: creating directory `%s'", dirname);
}

static char *get_non_empty_env(const char *name)
{
	const char *val;

	val = getenv(name);
	if (val == NULL || val[0] == 0)
		return NULL;
	return xstrdup(val);
}

int misc_init(void)
{
#ifdef CONFIG_XDG
	char *xdg_config_home, *xdg_cache_home;
#endif

	home_dir = get_non_empty_env("HOME");
	if (home_dir == NULL)
		die("error: environment variable HOME not set\n");

	user_name = get_non_empty_env("USER");
	if (user_name == NULL) {
		user_name = get_non_empty_env("USERNAME");
		if (user_name == NULL)
			die("error: neither USER or USERNAME environment variable set\n");
	}

#ifdef CONFIG_XDG
	/* ensure that configuration directories exist */
	xdg_config_home = get_non_empty_env("XDG_CONFIG_HOME");
	xdg_cache_home = get_non_empty_env("XDG_CACHE_HOME");
	if (xdg_config_home == NULL)
		xdg_config_home = xstrjoin(home_dir, "/.config");
	if (xdg_cache_home == NULL)
		xdg_cache_home = xstrjoin(home_dir, "/.cache");
	make_dir(xdg_config_home);
	make_dir(xdg_cache_home);

	cmus_config_dir = xstrjoin(xdg_config_home, "/cmus");
	cmus_cache_dir = xstrjoin(xdg_cache_home, "/cmus");
	free(xdg_config_home);
	free(xdg_cache_home);
	make_dir(cmus_config_dir);
	make_dir(cmus_cache_dir);
#else
	cmus_config_dir = xstrjoin(home_dir, "/.cmus");
	cmus_cache_dir = cmus_config_dir;
	make_dir(cmus_config_dir);
#endif
	return 0;
}