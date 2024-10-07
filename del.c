/* del.c
 *
 * Copyright (C) 2024 Dilnavas Roshan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/limits.h>

static void create_trash_info_file(const char *path, const char *original_path, char *postfix_str);
static void get_date_time_str(char *str);
static int need_postfix(char *trash_path, int *postfix);
static void print_help();

int
main(int argc, char *argv[])
{
    int opt;
    int postfix;
    char *xdg_data_dir, *home_dir;
    char info_path[PATH_MAX], file_path[PATH_MAX], path[PATH_MAX];
    char trash_prefix[PATH_MAX];
    char postfix_str[PATH_MAX];
    char *base_name;
    struct option del_options[] = {
        {"help", 0, NULL, 'h'},
        {"list", 0, NULL, 'l'}};

    while ((opt = getopt_long(argc, argv, ":lh", del_options, NULL)) != -1) {
        switch (opt) {
        case 'l':
            printf("list function\n");
            break;
        case 'h':
            print_help();
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    xdg_data_dir = getenv("XDG_DATA_HOME");
    if (!xdg_data_dir) {
        home_dir = getenv("HOME");
        if (!home_dir) {
            fprintf(stderr, "error: env variable HOME is not set\n"); 
            exit(EXIT_FAILURE);
        }
        strcpy(trash_prefix, home_dir);
        strcat(trash_prefix, "/.local/share/");
    } else {
        strcpy(trash_prefix, xdg_data_dir);
    }

    strcpy(info_path, trash_prefix);
    strcat(info_path, "Trash/info/");
    strcpy(file_path, trash_prefix);
    strcat(file_path, "Trash/files/");

    for (; optind < argc; optind++) {
        path[0] = 0;
        postfix_str[0] = 0;
        base_name = basename(argv[optind]);
        strcat(path, info_path);
        strcat(path, base_name);
        if (need_postfix(path, &postfix))
            sprintf(postfix_str, ".%d", postfix);
        create_trash_info_file(path, argv[optind], postfix_str);
        path[0] = 0;
        strcat(path, file_path);
        strcat(path, base_name);
        strcat(path, postfix_str);
        rename(argv[optind], path);
    }

    return EXIT_SUCCESS;
}

static void
create_trash_info_file(const char *path, const char *original_path, char *postfix_str)
{
    FILE *fp;
    char abs_path[PATH_MAX], trash_info_file_path[PATH_MAX];
    char time_stamp[32];

    if(!realpath(original_path, abs_path)) {
        perror("error: ");
        exit(EXIT_FAILURE);
    }

    strcpy(trash_info_file_path, path);
    strcat(trash_info_file_path, postfix_str);
    strcat(trash_info_file_path, ".trashinfo");
    fp = fopen(trash_info_file_path, "w");
    if (!fp) {
        fprintf(stderr, "error: failed creating trash info file\n");
        exit(EXIT_FAILURE);
    }

    get_date_time_str(time_stamp);

    fprintf(fp, "[Trash Info]\n");
    fprintf(fp, "Path=%s\n", abs_path);
    fprintf(fp, "DeletionDate=%s", time_stamp);

    fclose(fp);
}

static void
get_date_time_str(char *date_time_s)
{
    time_t time_val;
    struct tm *tm_ptr;

    time_val = time((time_t *) NULL);
    tm_ptr = localtime(&time_val);

    sprintf(date_time_s, "%d-%d-%dT%d:%d:%d", 1900 + tm_ptr->tm_year, tm_ptr->tm_mon + 1,
        tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
}

static int
need_postfix(char *trash_path, int *postfix)
{
    int i = 1;
    char tmp_path[PATH_MAX];

    sprintf(tmp_path, "%s.trashinfo", trash_path);
    if (access(tmp_path, F_OK))
        return 0;

    for (;;) {
        sprintf(tmp_path, "%s.%d.trashinfo", trash_path, i);
        if (access(trash_path, F_OK))
            break;
        i++;
    }

    *postfix = i;
    return 1;
}

static void
print_help()
{
    printf("Usage: del [OPTION] ... [FILE] ...\n");
    printf("del: command to delete files to trash\n");
    printf("Version: 1.0\n");
    printf("Copyright (C) 2024 Dilnavas Roshan\n");
}
