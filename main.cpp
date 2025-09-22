//
// Created by 白杰 on 2025/8/9.
//

#include "access.h"
#include "pg_log.h"
#include <getopt.h>

using namespace std;

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

bool onlyNewTuple = false;
bool parserAllFiles = false;

bool printVerbose() {
    printf("Release version: v1.0.0 ");
    printf("Build timestamp: 2025-08-18 10:00:00");
}

int createParserApp(int argc, char* argv[]) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
            {"help",    no_argument,       0, 'h'},
            {"file",  required_argument, 0, 'f'},
            {"verbose", no_argument,       0, 'v'},
            {"only_new_version", no_argument,       0, 'o'},
            {"parser_all_file", no_argument,       0, 'p'},
            {0, 0, 0, 0}  // 结束标志
    };

    while ((opt = getopt_long(argc, argv, "hvf:op", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [-h] [-v] [[-f data_file_name] [-o]]\n", argv[0]);
                exit(0);
            case 'v':
                printVerbose();
                break;
            case 'f':
                if (optarg[0] != '/') {
                    LOG(LOG_LEVEL_FATAL, "input file path must be absolute path, exit...");
                    exit(1);
                }
                LOG(LOG_LEVEL_INFO, "parser file: %s ", optarg);
                InitAccessForProcessRecover(optarg);
                break;
            case 'o':
                onlyNewTuple = true;
                break;
            case 'p':
//                parserAllFiles = true;
                break;
            case '?': // 未知选项
                // getopt_long 自动打印错误信息
                LOG(LOG_LEVEL_FATAL, "unknown command. please input correct command."
                                     "Usage: %s [[-f /pgdata/data/base/16384/50000] [-o]]", argv[0]);
                exit(1);
            default:
                break;
        }
    }

    if (optind < argc) {
        printf("Non-option arguments:\n");
        while (optind < argc) {
            printf("  %s\n", argv[optind++]);
        }
    }
    return 1;
}

char* get_program_name(char** args) {
    char *lastSlash = strrchr(args[0], '/');
    char* programName;
    char *programNameSlash = (lastSlash != NULL) ? lastSlash + 1 : args[0];
    programName = (char*) malloc(strlen(programNameSlash) + 1);
    if (!programName) {
        perror("malloc");
        exit(1);
    }
    strcpy(programName, programNameSlash);
    return programName;
}

int main(int argc, char* argv[]) {
    char** args = argv;
    char* programName = get_program_name(args);
    int pid = getpid();

    if (argc < 3) {
        LOG(LOG_LEVEL_FATAL, "Usage: %s [[-f /pgdata/data/base/16384/50000] [-o]]", argv[0]);
        return 1;
    }
    LOG(LOG_LEVEL_INFO, "start program %s pid: %d execute path: %s ", programName, pid, argv[0]);

    createParserApp(argc, argv);

    LOG(LOG_LEVEL_INFO, "finished program %s pid: %d.", programName, pid);

    free(programName);
}


