//
// Created by 白杰 on 2025/8/9.
//

#include "pg_access.h"
#include <getopt.h>


#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

bool onlyNewTuple = false;
bool parserAllFiles = false;

bool printVerbose() {
    printf("Release version: v1.0.0 ");
    printf("Build timestamp: 2025-08-18 10:00:00");
}

int createParserApp(int argc, char* argv[], DumpOptions *opt) {
    int opta;
    int option_index = 0;

    static struct option long_options[] = {
            {"db_path",    required_argument,       0, 'D'},
            {"file",  required_argument, 0, 'f'},
            {"database_name", required_argument, 0, 'd'},
            {"table_name", required_argument,       0, 't'},
            {"export", required_argument,       0, 'e'},
            {"table_construct", required_argument,       0, 'T'},
            {"none_info", no_argument,       0, 'n'},
            {"output_sql", no_argument,       0, 's'},
            {"output_latest_version", no_argument,       0, 'o'},
            {"dump_all", no_argument,       0, 'a'},
            {"verbose", no_argument,       0, 'v'},
            {"help",    no_argument,       0, 'h'},
            {0, 0, 0, 0}  // 结束标志
    };

    while ((opta = getopt_long(argc, argv, "D:f:d:t:e:T:nsoavh", long_options, &option_index)) != -1) {
        switch (opta) {
            case 'D':
                if (!optarg) { fprintf(stderr, "-D requires value\n"); exit(1); }
                opt->db_path = strdup(optarg);
                break;

            case 'd':
                if (!optarg) { fprintf(stderr, "-d requires value\n"); exit(1); }
                opt->database = strdup(optarg);
                break;

            case 'f':
                if (!optarg) { fprintf(stderr, "-f requires value\n"); exit(1); }
                opt->file = strdup(optarg);
                break;

            case 't':
                if (!optarg) { fprintf(stderr, "-t requires value\n"); exit(1); }
                opt->table = strdup(optarg);
                break;
            case 'e':
                if (!optarg) { fprintf(stderr, "-e requires value\n"); exit(1); }
//                opt->output_name = strdup(optarg);
                break;
            case 'T':
                if (!optarg) { fprintf(stderr, "-c requires value\n"); exit(1); }
                opt->table_construct = strdup(optarg);
                break;
            case 'n':
                opt->none_info = 1;
                break;
            case 's':
                opt->show_sql = 1;
                break;
            case 'o':
                opt->only_visible = 1;
                break;
            case 'a':
                opt->dump_all = 1;
                break;

            case 'v':
                opt->verbose = 1;
                break;

            case 'h':
                print_dump_usage();
                exit(0);

            default:
                print_dump_usage();
                exit(1);
        }
    }

    if (optind < argc) {
        printf("Non-option arguments:\n");
        while (optind < argc) {
            printf("  %s\n", argv[optind++]);
        }
    }
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

static void validate_options(DumpOptions *opt)
{
    // 必填参数
    if (opt->db_path == NULL || opt->database == NULL) {
        fprintf(stderr, "Error: -D and -d are required.\n");
        print_dump_usage();
        exit(1);
    }

    // exactly one of -f, -t, -T must be specified
    int has_file  = (opt->file != NULL);
    int has_table = (opt->table != NULL);
    int has_table_construct = (opt->table_construct != NULL);

    if (has_file + has_table + has_table_construct != 1) {
        fprintf(stderr, "Error: must specify exactly one of -f or -t or -T.\n");
        print_dump_usage();
        exit(1);
    }
}

int parse_dump_options(int argc, char *argv[], DumpOptions *opt)
{
    int c;
    optind = 2;
    while ((c = getopt(argc, argv, "D:f:d:t:eT:nsoavh")) != -1)
    {
        switch (c)
        {
            case 'D':
                if (!optarg) { fprintf(stderr, "-D requires value\n"); exit(1); }
                opt->db_path = strdup(optarg);
                break;

            case 'd':
                if (!optarg) { fprintf(stderr, "-d requires value\n"); exit(1); }
                opt->database = strdup(optarg);
                break;

            case 'f':
                if (!optarg) { fprintf(stderr, "-f requires value\n"); exit(1); }
                opt->file = strdup(optarg);
                break;

            case 't':
                if (!optarg) { fprintf(stderr, "-t requires value\n"); exit(1); }
                opt->table = strdup(optarg);
                break;
            case 'e':
                opt->output_name = 1;
                break;
            case 'T':
                if (!optarg) { fprintf(stderr, "-c requires value\n"); exit(1); }
                opt->table_construct = strdup(optarg);
                break;
            case 'n':
                opt->none_info = 1;
                break;
            case 's':
                opt->show_sql = 1;
                break;
            case 'o':
                opt->only_visible = 1;
                break;
            case 'a':
                opt->dump_all = 1;
                break;

            case 'v':
                opt->verbose = 1;
                break;

            case 'h':
                print_dump_usage();
                exit(0);

            default:
                print_dump_usage();
                exit(1);
        }
    }

    validate_options(opt);

    return 0;
}

int main(int argc, char* argv[]) {

    char** args = argv;
    int arg = argc;
    char* programName = get_program_name(args);
    int pid = getpid();

    if (argc < 2) {
        print_global_usage();
        return 1;
    }

    DumpOptions* op = (DumpOptions*) malloc(sizeof(DumpOptions));

    memset(op, 0, sizeof(DumpOptions));

    parse_dump_options(argc, argv, op);

    if (strcmp(argv[1], "dump") == 0)
    {
        if (argc >= 3 && strcmp(argv[2], "-h") == 0)
        {
            print_dump_usage();
            return 0;
        }
        pg_forge(op, 'd');
    }
//    else if (strcmp(argv[1], "inspect") == 0)
//    {
//        if (argc >= 3 && strcmp(argv[2], "-h") == 0)
//        {
//            print_inspect_usage();
//            return 0;
//        }
//        return cmd_inspect(argc - 1, argv + 1);
//    }
//    else if (strcmp(argv[1], "catalog") == 0)
//    {
//        if (argc >= 3 && strcmp(argv[2], "-h") == 0)
//        {
//            print_catalog_usage();
//            return 0;
//        }
//        return cmd_catalog(argc - 1, argv + 1);
//    }
//    else
//    {
//        print_global_usage();
//        return 1;
//    }

    free(programName);
}


