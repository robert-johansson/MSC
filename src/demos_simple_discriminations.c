#include <stdio.h>
#include <stdbool.h>
#include "Globals.h"
#include "Encode.h"
#include "Term.h"
#include "MSC.h"
#include "Memory.h"
#include "Concept.h"
#include "Decision.h"
#include "Truth.h"
#include "demos.h"

#define OP_LEFT_ID 1
#define OP_RIGHT_ID 2

static int last_operation_executed = 0;

static void SimpleDiscrimination_OpLeft(void)
{
    last_operation_executed = OP_LEFT_ID;
    puts("Executed: op_left");
}

static void SimpleDiscrimination_OpRight(void)
{
    last_operation_executed = OP_RIGHT_ID;
    puts("Executed: op_right");
}

static Concept *FindConcept(const Term *term)
{
    int index;
    if(Memory_FindConceptByTerm((Term *)term, &index))
    {
        return concepts.items[index].address;
    }
    return NULL;
}

static double BestExpectationFor(Concept *target, int operation_id, const Term *precondition)
{
    if(target == NULL)
    {
        return 0.0;
    }
    double best = 0.0;
    Table *table = &target->precondition_beliefs[operation_id];
    for(int i = 0; i < table->itemsAmount; i++)
    {
        Implication *imp = &table->array[i];
        if(Term_Equal(&imp->term, (Term *)precondition))
        {
            double exp = Truth_Expectation(imp->truth);
            if(exp > best)
            {
                best = exp;
            }
        }
    }
    return best;
}

void MSC_SimpleDiscriminations(void)
{
    puts(">>MSC Simple Discriminations demo start");

    double old_babbling = MOTOR_BABBLING_CHANCE;
    MOTOR_BABBLING_CHANCE = 0.3;

    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(false);

    Term termA = Encode_Term("A");
    Term termB = Encode_Term("B");
    Term termG = Encode_Term("G");
    Term opLeftTerm = Encode_Term("op_left");
    Term opRightTerm = Encode_Term("op_right");

    MSC_AddOperation(opLeftTerm, SimpleDiscrimination_OpLeft);
    MSC_AddOperation(opRightTerm, SimpleDiscrimination_OpRight);

    int episodes = 200;
    int successA = 0;
    int successB = 0;

    for(int episode = 0; episode < episodes; episode++)
    {
        bool useA = (episode % 2 == 0);
        const Term *stimulus = useA ? &termA : &termB;
        last_operation_executed = 0;

        printf("\nEpisode %d stimulus: %s\n", episode + 1, useA ? "A" : "B");

        MSC_AddInputBelief(*stimulus, 0);
        MSC_AddInputGoal(termG);

        for(int i = 0; i < 32 && last_operation_executed == 0; i++)
        {
            MSC_Cycles(1);
        }

        if(useA && last_operation_executed == OP_LEFT_ID)
        {
            successA++;
            puts("Reward: G.");
            MSC_AddInputBelief(termG, 0);
        }
        else if(!useA && last_operation_executed == OP_RIGHT_ID)
        {
            successB++;
            puts("Reward: G.");
            MSC_AddInputBelief(termG, 0);
        }
        else
        {
            puts("No reward given.");
        }

        MSC_Cycles(4);

        if((episode + 1) % 20 == 0)
        {
            Concept *goalConcept = FindConcept(&termG);
            double expA = BestExpectationFor(goalConcept, OP_LEFT_ID, &termA);
            double expB = BestExpectationFor(goalConcept, OP_RIGHT_ID, &termB);
            printf("After %d episodes:\n", episode + 1);
            printf("  Success A -> left: %d/%d (exp %.3f)\n", successA, (episode + 2) / 2, expA);
            printf("  Success B -> right: %d/%d (exp %.3f)\n", successB, (episode + 1) / 2, expB);
        }
    }

    Concept *goalConcept = FindConcept(&termG);
    double finalExpA = BestExpectationFor(goalConcept, OP_LEFT_ID, &termA);
    double finalExpB = BestExpectationFor(goalConcept, OP_RIGHT_ID, &termB);

    puts("\nFinal report:");
    printf("  A -> op_left successes: %d / %d, expectation %.3f\n", successA, (episodes + 1) / 2, finalExpA);
    printf("  B -> op_right successes: %d / %d, expectation %.3f\n", successB, episodes / 2, finalExpB);

    MOTOR_BABBLING_CHANCE = old_babbling;
    puts("<<MSC Simple Discriminations demo end");
}
