#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <regex.h>

bool no_line_numbers = false;

regex_t regex;

void print_usage(void) {
    fprintf(stderr, "Usage: ./mini-grep [-q] -e PATTERN [FILE...]\n");
}

int run_regex_on_stream(FILE *file) {
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;
    int regex_result;
    int line_index = 1;
    int any_match = false;

    while ((length = getline(&line, &capacity, file)) != -1) {
        regex_result = regexec( &regex, line, 0, NULL, 0);
        if (regex_result == 0)
        {
            if (!no_line_numbers) {
                printf("%d: %s", line_index, line);
                any_match = true;
            } else {
                printf("%s", line);
                any_match = true;
            }
        }
        line_index++;
    }
    free(line);
    return any_match;
}

int run_regex_on_file(char file_name[]) {
    int any_match = false;
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s.\n", file_name);
        return 2;
    }

    any_match = run_regex_on_stream(file);

    fclose(file);
    return any_match;
}

int run_regex_on_stdin(void) {
    return run_regex_on_stream(stdin);
} 

int main(int argc, char* argv[]) {
    int opt;
    bool has_e = false;
    bool regex_compiled = false;
    bool any_match = false;

    while ((opt = getopt(argc, argv, "qe:")) != -1) {
        switch (opt) {
            case 'q':
                no_line_numbers = true;
                break;
            case 'e':
                if (has_e) {
                    fprintf(stderr, "Multiple paters are not supported.\n");
                    regfree(&regex);
                    exit(2);
                }
                has_e = true;
                if (regcomp(&regex, optarg, REG_NOSUB|REG_NEWLINE) != 0) {
                    fprintf(stderr, "Invalid regex pattern: %s\n", optarg);
                    exit(2);
                }
                regex_compiled = true;
                break;
            default:
                print_usage();
                return 2;
        }
    }

    if (!has_e) {
        fprintf(stderr, "Error: Missing mandatory -e PATTERN option.\n");
        print_usage();
        exit(2);
    }

    if (optind == argc) {
        any_match = run_regex_on_stdin();
    } else {
        int regex_result = 0;
        for (int i = optind; i < argc; i++) {
            regex_result = run_regex_on_file(argv[i]);
            if (regex_result == 2) exit(2);
            any_match = any_match ? true : regex_result;
        }
    }

    if (regex_compiled) regfree(&regex);

    return any_match ? 0 : 1;
}