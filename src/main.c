#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MSC.h"
#include "tests.h"
#include "demos.h"

typedef void (*TestFunction)(void);

typedef struct
{
    const char *name;
    TestFunction function;
} RegressionTest;

static const RegressionTest kRegressionTests[] = {
    {"stamp", Stamp_Test},
    {"fifo", FIFO_Test},
    {"priority_queue", PriorityQueue_Test},
    {"table", Table_Test},
    {"alphabet", MSC_Alphabet_Test},
    {"procedure", MSC_Procedure_Test},
    {"memory", Memory_Test},
    {"follow", MSC_Follow_Test},
    {"multistep", MSC_Multistep_Test},
    {"multistep2", MSC_Multistep2_Test},
    {"sequence", Sequence_Test},
    {"sequence_len3", MSC_SequenceLen3_Test},
    {"exp1", MSC_Exp1_Test},
    {"exp1_training", MSC_Exp1_TrainingOnly},
    {"exp3", MSC_Exp3_Test},
};

static const size_t kRegressionTestCount = sizeof(kRegressionTests) / sizeof(kRegressionTests[0]);

static const RegressionTest *FindRegressionTest(const char *name)
{
    for(size_t i = 0; i < kRegressionTestCount; i++)
    {
        if(strcmp(name, kRegressionTests[i].name) == 0)
        {
            return &kRegressionTests[i];
        }
    }
    return NULL;
}

static void RunAllRegressionTests(void)
{
    for(size_t i = 0; i < kRegressionTestCount; i++)
    {
        kRegressionTests[i].function();
    }
}

static void PrintTestList(void)
{
    puts("Available regression tests:");
    for(size_t i = 0; i < kRegressionTestCount; i++)
    {
        printf("  %s\n", kRegressionTests[i].name);
    }
}

static void PrintUsage(const char *program)
{
    printf("Usage: %s [--run-all-tests | --test <name> | --list-tests | --exp1-csv <path> | --exp2-csv <path> | --exp3-csv <path> | pong | pongX | pong2 | pong2X | testchamber | alien | simple_discriminations]\n", program);
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
        {
            PrintUsage(argv[0]);
            PrintTestList();
            return 0;
        }
        if(!strcmp(argv[1], "--list-tests"))
        {
            PrintTestList();
            return 0;
        }
        if(!strcmp(argv[1], "--exp1-csv"))
        {
            const char *path = (argc >= 3) ? argv[2] : "exp1.csv";
            MSC_Exp1_ExportCSV(path);
            return 0;
        }
        if(!strcmp(argv[1], "--exp2-csv"))
        {
            const char *path = (argc >= 3) ? argv[2] : "exp2.csv";
            MSC_Exp2_ExportCSV(path);
            return 0;
        }
        if(!strcmp(argv[1], "--exp3-csv"))
        {
            const char *path = (argc >= 3) ? argv[2] : "exp3.csv";
            MSC_Exp3_ExportCSV(path);
            return 0;
        }
        if(!strcmp(argv[1], "--run-all-tests"))
        {
            srand(1337);
            MSC_INIT();
            OUTPUT = 0;
            RunAllRegressionTests();
            puts("\nAll tests ran successfully.");
            puts("Use '--test <name>' to run a single regression test or '--list-tests' to see options.");
            return 0;
        }
        if(!strcmp(argv[1], "--test"))
        {
            if(argc < 3)
            {
                fputs("Error: missing test name.\n", stderr);
                PrintUsage(argv[0]);
                return 1;
            }
            const RegressionTest *test = FindRegressionTest(argv[2]);
            if(!test)
            {
                fprintf(stderr, "Unknown test '%s'.\n", argv[2]);
                PrintTestList();
                return 1;
            }
            srand(1337);
            MSC_INIT();
            OUTPUT = 0;
            test->function();
            return 0;
        }
        if(!strcmp(argv[1], "pong"))
        {
            MSC_Pong();
            return 0;
        }
        if(!strcmp(argv[1], "pongX"))
        {
            MSC_Pong_Headless();
            return 0;
        }
        if(!strcmp(argv[1], "pong2"))
        {
            MSC_Pong2();
            return 0;
        }
        if(!strcmp(argv[1], "pong2X"))
        {
            MSC_Pong2_Headless();
            return 0;
        }
        if(!strcmp(argv[1], "testchamber"))
        {
            MSC_TestChamber();
            return 0;
        }
        if(!strcmp(argv[1], "alien"))
        {
            MSC_Alien();
            return 0;
        }
        if(!strcmp(argv[1], "simple_discriminations"))
        {
            MSC_SimpleDiscriminations();
            return 0;
        }
        fprintf(stderr, "Unknown argument '%s'.\n", argv[1]);
        PrintUsage(argv[0]);
        return 1;
    }
    PrintUsage(argv[0]);
    return 0;
}
