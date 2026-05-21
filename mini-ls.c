#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>
#include <time.h>

bool run_recursive = false;

void print_usage(void) {
    fprintf(stderr,"Usage: ./mini-ls [-r] [FILE...]\n");
}

bool is_current_or_parent(char file_name[]) {
    return strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0;
}

static void format_mode(mode_t mode, char out[11]) {
    if      (S_ISDIR(mode))  out[0] = 'd';
    else if (S_ISLNK(mode))  out[0] = 'l';
    else if (S_ISCHR(mode))  out[0] = 'c';
    else if (S_ISBLK(mode))  out[0] = 'b';
    else if (S_ISFIFO(mode)) out[0] = 'p';
    else if (S_ISSOCK(mode)) out[0] = 's';
    else                     out[0] = '-';

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

void write_file_information(const struct stat * sb, char file_name[]) {
    struct passwd *pw;
    char buf[64];
    char mode_str[11];
    struct tm tm_local;

    if ((pw = getpwuid(sb->st_uid)) == NULL) {
        fprintf(stderr, "Failed to get user details %s.\n", file_name);
        return;
    }

    format_mode(sb->st_mode, mode_str);
    printf("%s ", mode_str);

    printf("%s ", pw->pw_name);
    
    localtime_r(&sb->st_mtime, &tm_local);
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S %Z", &tm_local);
    printf("%s ", buf);
    
    printf("%s", file_name);

    if (S_ISDIR(sb->st_mode)) {
        printf("/\n");
    } else {
        printf("\n");
    }
}

void run_ls_on_file(char file_name[]) {
    struct stat sb;
    struct stat child_sb;
    struct dirent *de;
    DIR *dr;
    
    if (lstat(file_name, &sb) >= 0) {
        if (S_ISDIR(sb.st_mode)) {
            write_file_information(&sb, file_name);
            dr = opendir(file_name);
            if (dr == NULL)
            {
                fprintf(stderr, "Could not open current directory %s.\n", file_name);
                return;
            }
            while ((de = readdir(dr)) != NULL)
            {
                if (is_current_or_parent(de->d_name))
                {
                    continue;
                }
                char next_path[PATH_MAX]; 
                snprintf(next_path, sizeof(next_path), "%s/%s", file_name, de->d_name);
                
                if (run_recursive)
                {
                    run_ls_on_file(next_path);
                } else {
                    if (lstat(next_path, &child_sb) >= 0)
                    {
                        write_file_information(&child_sb, next_path); 
                    } else {
                        fprintf(stderr, "File %s does not exist.\n", next_path);
                        continue;
                    }
                }
            }
            closedir(dr);
        } else {
            write_file_information(&sb, file_name);
        }
    } else {
        fprintf(stderr, "File %s does not exist.\n", file_name);
        return;
    }
}

void check_files(char* file_names[], int file_count) {
    
    for (int i = 0; i < file_count; i++) {
        run_ls_on_file(file_names[i]);
    }
}


int main(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
            case 'r':
                run_recursive = true;
                break;
            default:
                print_usage();
                return 1;
        }
    }

    if (optind == argc) 
    {
        run_ls_on_file(".");
    } else {
        check_files(argv + optind, argc - optind);
    }

    return 0;
}
