#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Term.h"
#include "Memory.h"
#include "Concept.h"
#include "Encode.h"
#include "MSC.h"
#include "PriorityQueue.h"
#include "Table.h"
#include "Implication.h"
#include "Decision.h"
#include "Globals.h"
#include "Truth.h"
#include "tests.h"

#define EXP1_OP_LEFT_ID 1
#define EXP1_OP_RIGHT_ID 2

static Concept *Exp1_FindConcept(const Term *term)
{
    int index;
    if(Memory_FindConceptByTerm((Term *)term, &index))
    {
        return concepts.items[index].address;
    }
    return NULL;
}

static double Exp1_BestExpectationFor(Concept *goalConcept, int operation_id, const Term *precondition)
{
    if(goalConcept == NULL)
    {
        return 0.0;
    }
    Table *table = &goalConcept->precondition_beliefs[operation_id];
    double best = 0.0;
    for(int i = 0; i < table->itemsAmount; i++)
    {
        Implication *imp = &table->array[i];
        if(Term_Equal(&imp->term, (Term *)precondition))
        {
            double expectation = Truth_Expectation(imp->truth);
            if(expectation > best)
            {
                best = expectation;
            }
        }
    }
    return best;
}

void FIFO_Test(void)
{
    puts(">>FIFO test start");
    FIFO fifo = (FIFO) {0};
    for(int i = FIFO_SIZE * 2; i >= 1; i--)
    {
        Event event1 = {
            .term = Encode_Term("test"),
            .type = EVENT_TYPE_BELIEF,
            .truth = { .frequency = 1.0, .confidence = 0.9 },
            .stamp = { .evidentalBase = { i } },
            .occurrenceTime = FIFO_SIZE * 2 - i * 10
        };
        FIFO_Add(&event1, &fifo);
    }
    for(int i = 0; i < FIFO_SIZE; i++)
    {
        assert(FIFO_SIZE - i == fifo.array[0][i].stamp.evidentalBase[0], "Item at FIFO position has to be right");
    }
    int newbase = FIFO_SIZE * 2 + 1;
    Event event2 = {
        .term = Encode_Term("test"),
        .type = EVENT_TYPE_BELIEF,
        .truth = { .frequency = 1.0, .confidence = 0.9 },
        .stamp = { .evidentalBase = { newbase } },
        .occurrenceTime = 3 * 10 + 3
    };
    FIFO fifo2 = (FIFO) {0};
    for(int i = 0; i < FIFO_SIZE * 2; i++)
    {
        Term zero = (Term) {0};
        FIFO_Add(&event2, &fifo2);
        if(i < FIFO_SIZE && i < MAX_SEQUENCE_LEN)
        {
            char buf[100];
            Event *ev = FIFO_GetKthNewestSequence(&fifo2, 0, i);
            sprintf(buf, "This event Term is not allowed to be zero, sequence length=%d\n", i + 1);
            assert(!Term_Equal(&zero, &ev->term), buf);
        }
    }
    assert(fifo2.itemsAmount == FIFO_SIZE, "FIFO size differs");
    puts("<<FIFO Test successful");
}

void Stamp_Test(void)
{
    puts(">>Stamp test start");
    Stamp stamp1 = { .evidentalBase = {1,2} };
    Stamp_print(&stamp1);
    Stamp stamp2 = { .evidentalBase = {2,3,4} };
    Stamp_print(&stamp2);
    Stamp stamp3 = Stamp_make(&stamp1, &stamp2);
    fputs("zipped:", stdout);
    Stamp_print(&stamp3);
    assert(Stamp_checkOverlap(&stamp1, &stamp2) == true, "Stamp should overlap");
    puts("<<Stamp test successful");
}

void PriorityQueue_Test(void)
{
    puts(">>PriorityQueue test start");
    PriorityQueue queue;
    int n_items = 10;
    Item items[n_items];
    for(int i = 0; i < n_items; i++)
    {
        items[i].address = (void*) ((long) i + 1);
        items[i].priority = 0;
    }
    PriorityQueue_RESET(&queue, items, n_items);
    for(int i = 0, evictions = 0; i < n_items * 2; i++)
    {
        PriorityQueue_Push_Feedback feedback = PriorityQueue_Push(&queue, 1.0 / ((double) (n_items * 2 - i)));
        if(feedback.added)
        {
            printf("item was added %f %ld\n", feedback.addedItem.priority, (long) feedback.addedItem.address);
        }
        if(feedback.evicted)
        {
            printf("evicted item %f %ld\n", feedback.evictedItem.priority, (long) feedback.evictedItem.address);
            assert(evictions > 0 || feedback.evictedItem.priority == (double) (1.0 / ((double) (n_items * 2))), "the evicted item has to be the lowest priority one");
            assert(queue.itemsAmount < n_items + 1, "eviction should only happen when full!");
            evictions++;
        }
    }
    puts("<<PriorityQueue test successful");
}

void Table_Test(void)
{
    puts(">>Table test start");
    Table table = (Table) {0};
    for(int i = TABLE_SIZE * 2; i >= 1; i--)
    {
        Implication imp = {
            .term = Encode_Term("test"),
            .truth = { .frequency = 1.0, .confidence = 1.0 / ((double) (i + 1)) },
            .stamp = { .evidentalBase = { i } },
            .occurrenceTimeOffset = 10
        };
        Table_Add(&table, &imp);
    }
    for(int i = 0; i < TABLE_SIZE; i++)
    {
        assert(i + 1 == table.array[i].stamp.evidentalBase[0], "Item at table position has to be right");
    }
    Implication imp = {
        .term = Encode_Term("test"),
        .truth = { .frequency = 1.0, .confidence = 0.9 },
        .stamp = { .evidentalBase = { TABLE_SIZE * 2 + 1 } },
        .occurrenceTimeOffset = 10
    };
    assert(table.array[0].truth.confidence == 0.5, "The highest confidence one should be the first.");
    Table_AddAndRevise(&table, &imp, "");
    assert(table.array[0].truth.confidence > 0.5, "The revision result should be more confident than the table element that existed.");
    puts("<<Table test successful");
}

void Memory_Test(void)
{
    MSC_INIT();
    puts(">>Memory test start");
    Event e = Event_InputEvent(
        Encode_Term("a"),
        EVENT_TYPE_BELIEF,
        (Truth) { .frequency = 1, .confidence = 0.9 },
        1337
    );
    Memory_addEvent(&e);
    assert(belief_events.array[0][0].truth.confidence == (double) 0.9, "event has to be there");
    int returnIndex;
    assert(!Memory_FindConceptByTerm(&e.term, &returnIndex), "a concept doesn't exist yet!");
    Memory_Conceptualize(&e.term);
    int concept_i;
    assert(Memory_FindConceptByTerm(&e.term, &concept_i), "Concept should have been created!");
    Concept *c = concepts.items[concept_i].address;
    assert(Memory_FindConceptByTerm(&e.term, &returnIndex), "Concept should be found!");
    assert(c == concepts.items[returnIndex].address, "e should match to c!");
    Event e2 = Event_InputEvent(
        Encode_Term("b"),
        EVENT_TYPE_BELIEF,
        (Truth) { .frequency = 1, .confidence = 0.9 },
        1337
    );
    Memory_addEvent(&e2);
    Memory_Conceptualize(&e2.term);
    assert(Memory_FindConceptByTerm(&e2.term, &concept_i), "Concept should have been created!");
    Concept *c2 = concepts.items[concept_i].address;
    Concept_Print(c2);
    assert(Memory_FindConceptByTerm(&e2.term, &returnIndex), "Concept should be found!");
    assert(c2 == concepts.items[returnIndex].address, "e2 should closest-match to c2!");
    assert(Memory_FindConceptByTerm(&e.term, &returnIndex), "Concept should be found!");
    assert(c == concepts.items[returnIndex].address, "e should closest-match to c!");
    puts("<<Memory test successful");
}

void MSC_Alphabet_Test(void)
{
    MSC_INIT();
    puts(">>MSC Alphabet test start");
    MSC_AddInput(Encode_Term("a"), EVENT_TYPE_BELIEF, MSC_DEFAULT_TRUTH, 0);
    for(int i = 0; i < 50; i++)
    {
        int k = i % 10;
        if(i % 3 == 0)
        {
            char c[2] = { (char)('a' + k), 0 };
            MSC_AddInput(Encode_Term(c), EVENT_TYPE_BELIEF, MSC_DEFAULT_TRUTH, 0);
        }
        MSC_Cycles(1);
        puts("TICK");
    }
    puts("<<MSC Alphabet test successful");
}

static bool MSC_Procedure_Test_Op_executed = false;
void MSC_Procedure_Test_Op(void)
{
    puts("op executed by MSC");
    MSC_Procedure_Test_Op_executed = true;
}

void MSC_Procedure_Test(void)
{
    MSC_INIT();
    puts(">>MSC Procedure test start");
    MSC_AddOperation(Encode_Term("op"), MSC_Procedure_Test_Op);
    MSC_AddInputBelief(Encode_Term("a"), 0);
    MSC_Cycles(1);
    puts("---------------");
    MSC_AddInputBelief(Encode_Term("op"), 1);
    MSC_Cycles(1);
    puts("---------------");
    MSC_AddInputBelief(Encode_Term("result"), 0);
    MSC_Cycles(1);
    puts("---------------");
    MSC_AddInputBelief(Encode_Term("a"), 0);
    MSC_Cycles(1);
    puts("---------------");
    MSC_AddInputGoal(Encode_Term("result"));
    MSC_Cycles(1);
    puts("---------------");
    assert(MSC_Procedure_Test_Op_executed, "MSC should have executed op!");
    puts("<<MSC Procedure test successful");
}

static bool MSC_Follow_Test_Left_executed = false;
static bool MSC_Follow_Test_Right_executed = false;

static void MSC_Follow_Test_Left(void)
{
    puts("left executed by MSC");
    MSC_Follow_Test_Left_executed = true;
}

static void MSC_Follow_Test_Right(void)
{
    puts("right executed by MSC");
    MSC_Follow_Test_Right_executed = true;
}

void MSC_Follow_Test(void)
{
    OUTPUT = 0;
    MSC_INIT();
    puts(">>MSC Follow test start");
    MSC_AddOperation(Encode_Term("op_left"), MSC_Follow_Test_Left);
    MSC_AddOperation(Encode_Term("op_right"), MSC_Follow_Test_Right);
    int simsteps = 1000000;
    const int LEFT = 0;
    const int RIGHT = 1;
    int BALL = RIGHT;
    int score = 0;
    int goods = 0;
    int bads = 0;
    for(int i = 0; i < simsteps; i++)
    {
        puts(BALL == LEFT ? "LEFT" : "RIGHT");
        MSC_AddInputBelief(BALL == LEFT ? Encode_Term("ball_left") : Encode_Term("ball_right"), 0);
        MSC_AddInputGoal(Encode_Term("good_msc"));
        if(MSC_Follow_Test_Right_executed)
        {
            if(BALL == RIGHT)
            {
                MSC_AddInputBelief(Encode_Term("good_msc"), 0);
                printf("(ball=%d) good\n", BALL);
                score++;
                goods++;
            }
            else
            {
                printf("(ball=%d) bad\n", BALL);
                score--;
                bads++;
            }
            MSC_Follow_Test_Right_executed = false;
        }
        if(MSC_Follow_Test_Left_executed)
        {
            if(BALL == LEFT)
            {
                MSC_AddInputBelief(Encode_Term("good_msc"), 0);
                printf("(ball=%d) good\n", BALL);
                score++;
                goods++;
            }
            else
            {
                printf("(ball=%d) bad\n", BALL);
                score--;
                bads++;
            }
            MSC_Follow_Test_Left_executed = false;
        }
        BALL = rand() % 2;
        printf("Score %i step%d=\n", score, i);
        assert(score > -100, "too bad score");
        assert(bads < 500, "too many wrong trials");
        if(score >= 500)
        {
            break;
        }
        MSC_Cycles(10);
    }
    printf("<<MSC Follow test successful goods=%d bads=%d\n", goods, bads);
}

static bool MSC_Lightswitch_GotoSwitch_executed = false;
static bool MSC_Lightswitch_ActivateSwitch_executed = false;

static void MSC_Lightswitch_GotoSwitch(void)
{
    MSC_Lightswitch_GotoSwitch_executed = true;
    puts("MSC invoked goto switch");
}

static void MSC_Lightswitch_ActivateSwitch(void)
{
    MSC_Lightswitch_ActivateSwitch_executed = true;
    puts("MSC invoked activate switch");
}

void MSC_Multistep_Test(void)
{
    MOTOR_BABBLING_CHANCE = 0;
    puts(">>MSC Multistep test start");
    OUTPUT = 0;
    MSC_INIT();
    MSC_AddOperation(Encode_Term("op_goto_switch"), MSC_Lightswitch_GotoSwitch);
    MSC_AddOperation(Encode_Term("op_activate_switch"), MSC_Lightswitch_ActivateSwitch);
    for(int i = 0; i < 5; i++)
    {
        MSC_AddInputBelief(Encode_Term("start_at"), 0);
        MSC_AddInputBelief(Encode_Term("op_goto_switch"), 1);
        MSC_Cycles(1);
        MSC_AddInputBelief(Encode_Term("switch_at"), 0);
        MSC_AddInputBelief(Encode_Term("op_activate_switch"), 2);
        MSC_AddInputBelief(Encode_Term("switch_active"), 0);
        MSC_Cycles(1);
        MSC_AddInputBelief(Encode_Term("light_active"), 0);
        MSC_Cycles(10);
    }
    MSC_Cycles(10);
    MSC_AddInputBelief(Encode_Term("start_at"), 0);
    MSC_AddInputGoal(Encode_Term("light_active"));
    MSC_Cycles(10);
    assert(MSC_Lightswitch_GotoSwitch_executed && !MSC_Lightswitch_ActivateSwitch_executed, "MSC needs to go to the switch first");
    MSC_Lightswitch_GotoSwitch_executed = false;
    puts("MSC arrived at the switch");
    MSC_AddInputBelief(Encode_Term("switch_at"), 0);
    MSC_AddInputGoal(Encode_Term("light_active"));
    assert(!MSC_Lightswitch_GotoSwitch_executed && MSC_Lightswitch_ActivateSwitch_executed, "MSC needs to activate the switch");
    MSC_Lightswitch_ActivateSwitch_executed = false;
    puts("<<MSC Multistep test successful");
}

void MSC_Multistep2_Test(void)
{
    MOTOR_BABBLING_CHANCE = 0;
    puts(">>MSC Multistep2 test start");
    OUTPUT = 0;
    MSC_INIT();
    MSC_AddOperation(Encode_Term("op_goto_switch"), MSC_Lightswitch_GotoSwitch);
    MSC_AddOperation(Encode_Term("op_activate_switch"), MSC_Lightswitch_ActivateSwitch);
    for(int i = 0; i < 5; i++)
    {
        MSC_AddInputBelief(Encode_Term("start_at"), 0);
        MSC_AddInputBelief(Encode_Term("op_goto_switch"), 1);
        MSC_Cycles(1);
        MSC_AddInputBelief(Encode_Term("switch_at"), 0);
        MSC_Cycles(10);
    }
    MSC_Cycles(1000);
    for(int i = 0; i < 5; i++)
    {
        MSC_AddInputBelief(Encode_Term("switch_at"), 0);
        MSC_AddInputBelief(Encode_Term("op_activate_switch"), 2);
        MSC_AddInputBelief(Encode_Term("switch_active"), 0);
        MSC_Cycles(1);
        MSC_AddInputBelief(Encode_Term("light_active"), 0);
        MSC_Cycles(10);
    }
    MSC_Cycles(10);
    MSC_AddInputBelief(Encode_Term("start_at"), 0);
    MSC_AddInputGoal(Encode_Term("light_active"));
    MSC_Cycles(10);
    assert(MSC_Lightswitch_GotoSwitch_executed && !MSC_Lightswitch_ActivateSwitch_executed, "MSC needs to go to the switch first (2)");
    MSC_Lightswitch_GotoSwitch_executed = false;
    puts("MSC arrived at the switch");
    MSC_AddInputBelief(Encode_Term("switch_at"), 0);
    MSC_AddInputGoal(Encode_Term("light_active"));
    assert(!MSC_Lightswitch_GotoSwitch_executed && MSC_Lightswitch_ActivateSwitch_executed, "MSC needs to activate the switch (2)");
    MSC_Lightswitch_ActivateSwitch_executed = false;
    puts("<<MSC Multistep2 test successful");
}

static bool op_1_executed = false;
static bool op_2_executed = false;
static bool op_3_executed = false;

static void op_1(void)
{
    op_1_executed = true;
}

static void op_2(void)
{
    op_2_executed = true;
}

static void op_3(void)
{
    op_3_executed = true;
}

void Sequence_Test(void)
{
    OUTPUT = 0;
    MSC_INIT();
    MOTOR_BABBLING_CHANCE = 0;
    puts(">>Sequence test start");
    MSC_AddOperation(Encode_Term("op_1"), op_1);
    MSC_AddOperation(Encode_Term("op_2"), op_2);
    MSC_AddOperation(Encode_Term("op_3"), op_3);
    for(int i = 0; i < 5; i++)
    {
        MSC_AddInputBelief(Encode_Term("a"), 0);
        MSC_AddInputBelief(Encode_Term("b"), 0);
        MSC_AddInputBelief(Encode_Term("op_1"), 1);
        MSC_AddInputBelief(Encode_Term("g"), 0);
        MSC_Cycles(100);
    }
    for(int i = 0; i < 100; i++)
    {
        MSC_AddInputBelief(Encode_Term("a"), 0);
        MSC_AddInputBelief(Encode_Term("op_1"), 1);
        MSC_Cycles(100);
    }
    for(int i = 0; i < 100; i++)
    {
        MSC_AddInputBelief(Encode_Term("b"), 0);
        MSC_AddInputBelief(Encode_Term("op_1"), 1);
        MSC_Cycles(100);
    }
    for(int i = 0; i < 2; i++)
    {
        MSC_AddInputBelief(Encode_Term("b"), 0);
        MSC_AddInputBelief(Encode_Term("op_2"), 2);
        MSC_AddInputBelief(Encode_Term("g"), 0);
        MSC_Cycles(100);
    }
    for(int i = 0; i < 2; i++)
    {
        MSC_AddInputBelief(Encode_Term("a"), 0);
        MSC_AddInputBelief(Encode_Term("op_3"), 3);
        MSC_AddInputBelief(Encode_Term("g"), 0);
        MSC_Cycles(100);
    }
    MSC_AddInputBelief(Encode_Term("a"), 0);
    MSC_AddInputBelief(Encode_Term("b"), 0);
    MSC_AddInputGoal(Encode_Term("g"));
    assert(op_1_executed && !op_2_executed && !op_3_executed, "Expected op1 execution");
    op_1_executed = op_2_executed = op_3_executed = false;
    MSC_Cycles(100);
    MSC_AddInputBelief(Encode_Term("b"), 0);
    MSC_AddInputGoal(Encode_Term("g"));
    assert(!op_1_executed && op_2_executed && !op_3_executed, "Expected op2 execution");
    op_1_executed = op_2_executed = op_3_executed = false;
    MSC_Cycles(100);
    MSC_AddInputBelief(Encode_Term("a"), 0);
    MSC_AddInputGoal(Encode_Term("g"));
    assert(!op_1_executed && !op_2_executed && op_3_executed, "Expected op3 execution");
    op_1_executed = op_2_executed = op_3_executed = false;
    MOTOR_BABBLING_CHANCE = MOTOR_BABBLING_CHANCE_INITIAL;
    puts(">>Sequence Test successul");
}

static void seq3_op_left(void)
{
    // no-op placeholder for operation registration
}

void MSC_SequenceLen3_Test(void)
{
    OUTPUT = 0;
    MSC_INIT();
    MOTOR_BABBLING_CHANCE = 0;
    puts(">>Sequence len3 test start");
    Term termA = Encode_Term("seq3_A");
    Term termB = Encode_Term("seq3_B");
    Term termOpLeft = Encode_Term("seq3_op_left");
    Term termG = Encode_Term("seq3_G");
    MSC_AddOperation(termOpLeft, seq3_op_left);
    for(int i = 0; i < 5; i++)
    {
        MSC_AddInputBelief(termA, 0);
        MSC_AddInputBelief(termB, 0);
        MSC_AddInputBelief(termOpLeft, 1);
        MSC_AddInputBelief(termG, 0);
    }
    MSC_Cycles(10);
    Term seqAB = Term_Sequence(&termA, &termB);
    Term seqBOp = Term_Sequence(&termB, &termOpLeft);
    Term seqBOpG = Term_Sequence(&seqBOp, &termG);
    int conceptIndex = -1;
    assert(Memory_FindConceptByTerm(&seqAB, &conceptIndex), "Expected concept for (A &/ B).");
    assert(Memory_FindConceptByTerm(&seqBOpG, &conceptIndex), "Expected concept for (B &/ ^left &/ G).");
    int goalIndex = -1;
    assert(Memory_FindConceptByTerm(&termG, &goalIndex), "Expected G concept to exist.");
    Concept *goalConcept = concepts.items[goalIndex].address;
    Table *table = &goalConcept->precondition_beliefs[1];
    bool found = false;
    for(int i = 0; i < table->itemsAmount; i++)
    {
        if(Term_Equal(&table->array[i].term, &seqAB) && Truth_Expectation(table->array[i].truth) > 0.0)
        {
            found = true;
            break;
        }
    }
    assert(found, "Expected implication for (A &/ B) -> G with op_left.");
    MOTOR_BABBLING_CHANCE = MOTOR_BABBLING_CHANCE_INITIAL;
    puts("<<Sequence len3 test successful");
}

// -----------------------------------------------------------------------------
// Experiment 1: Simple discrimination (operant conditioning) replication
#define EXP_BLOCK_TRIALS 12
#define EXP1_BASELINE_BLOCKS 3
#define EXP1_TRAINING_BLOCKS 3
#define EXP1_TESTING_BLOCKS 3

#define EXP2_BLOCK_TRIALS 12
#define EXP2_BASELINE_BLOCKS 2
#define EXP2_TRAINING1_BLOCKS 4
#define EXP2_TESTING1_BLOCKS 2
#define EXP2_TRAINING2_BLOCKS 9
#define EXP2_TESTING2_BLOCKS 2

static int exp1_last_operation = 0;

static void Exp1_OpLeft(void)
{
    exp1_last_operation = 1;
}

static void Exp1_OpRight(void)
{
    exp1_last_operation = 2;
}

static void Exp1_LogTrial(FILE *log,
                          const char *phase,
                          int block,
                          int trial,
                          bool a1_on_left,
                          int chosen_operation,
                          bool success,
                          Term termA1_left,
                          Term termA1_right,
                          Term termA2_left,
                          Term termA2_right,
                          Term termG)
{
    if(!log)
    {
        return;
    }
    Concept *goalConcept = Exp1_FindConcept(&termG);
    double exp_a1_left = Exp1_BestExpectationFor(goalConcept, EXP1_OP_LEFT_ID, &termA1_left);
    double exp_a1_right = Exp1_BestExpectationFor(goalConcept, EXP1_OP_RIGHT_ID, &termA1_right);
    double exp_a2_left = Exp1_BestExpectationFor(goalConcept, EXP1_OP_LEFT_ID, &termA2_left);
    double exp_a2_right = Exp1_BestExpectationFor(goalConcept, EXP1_OP_RIGHT_ID, &termA2_right);
    fprintf(log,
            "%s,%d,%d,%d,%d,%d,%.6f,%.6f,%.6f,%.6f\n",
            phase,
            block + 1,
            trial + 1,
            a1_on_left ? 1 : 0,
            chosen_operation,
            success ? 1 : 0,
            exp_a1_left,
            exp_a1_right,
            exp_a2_left,
            exp_a2_right);
}

static bool Exp_RunTrial(bool a1_on_left,
                         bool provide_feedback,
                         int expected_operation,
                         Term termA1_left,
                         Term termA1_right,
                         Term termA2_left,
                         Term termA2_right,
                         Term termOpLeft,
                         Term termOpRight,
                         Term termG,
                         const char *phase,
                         int block,
                         int trial,
                         FILE *log)
{
    if(a1_on_left)
    {
        MSC_AddInputBelief(termA1_left, 0);
        MSC_AddInputBelief(termA2_right, 0);
    }
    else
    {
        MSC_AddInputBelief(termA1_right, 0);
        MSC_AddInputBelief(termA2_left, 0);
    }

    exp1_last_operation = 0;
    MSC_AddInputGoal(termG);

    // Allow time for a motor babbling or inference-based decision.
    for(int i = 0; i < 64 && exp1_last_operation == 0; i++)
    {
        MSC_Cycles(1);
    }

    // Retry by re-issuing the goal a few times if nothing happened.
    for(int attempt = 0; attempt < 4 && exp1_last_operation == 0; attempt++)
    {
        MSC_AddInputGoal(termG);
        for(int i = 0; i < 64 && exp1_last_operation == 0; i++)
        {
            MSC_Cycles(1);
        }
    }

    if(exp1_last_operation == 0)
    {
        exp1_last_operation = (rand() % 2) ? 1 : 2;
        Term opTerm = exp1_last_operation == 1 ? termOpLeft : termOpRight;
        MSC_AddInputBelief(opTerm, exp1_last_operation);
    }

    bool success = (exp1_last_operation == expected_operation);

    if(provide_feedback)
    {
        if(success)
        {
            MSC_AddInputBelief(termG, 0);
        }
        else
        {
            Truth negative_feedback = { .frequency = 0.0, .confidence = 0.9 };
            MSC_AddInput(termG, EVENT_TYPE_BELIEF, negative_feedback, 0);
        }
    }

    MSC_Cycles(4);
    // Inter-trial interval of 100 time steps, mirroring the paper design.
    MSC_Cycles(100);

    Exp1_LogTrial(log,
                  phase,
                  block,
                  trial,
                  a1_on_left,
                  exp1_last_operation,
                  success,
                  termA1_left,
                  termA1_right,
                  termA2_left,
                  termA2_right,
                  termG);

    return success;
}

static int Exp_RunPhase(int blocks,
                        bool provide_feedback,
                        bool use_a1_mapping,
                        Term termA1_left,
                        Term termA1_right,
                        Term termA2_left,
                        Term termA2_right,
                        Term termOpLeft,
                        Term termOpRight,
                        Term termG,
                        const char *phase,
                        FILE *log,
                        int *last_block_correct)
{
    int correct = 0;
    int toggle = 0;
    for(int block = 0; block < blocks; block++)
    {
        int block_correct = 0;
        for(int trial = 0; trial < EXP_BLOCK_TRIALS; trial++)
        {
            bool a1_on_left = ((toggle++) % 2 == 0);
            int expected = use_a1_mapping ?
                            (a1_on_left ? EXP1_OP_LEFT_ID : EXP1_OP_RIGHT_ID) :
                            (a1_on_left ? EXP1_OP_RIGHT_ID : EXP1_OP_LEFT_ID);
            bool success = Exp_RunTrial(a1_on_left,
                                        provide_feedback,
                                        expected,
                                        termA1_left,
                                        termA1_right,
                                        termA2_left,
                                        termA2_right,
                                        termOpLeft,
                                        termOpRight,
                                        termG,
                                        phase,
                                        block,
                                        trial,
                                        log);
            if(success)
            {
                correct++;
                block_correct++;
            }
        }
        if(block == blocks - 1 && last_block_correct != NULL)
        {
            *last_block_correct = block_correct;
        }
    }
    return correct;
}

void MSC_Exp1_Test(void)
{
    puts(">>MSC Experiment 1 (simple discrimination) test start");

    double original_babbling = MOTOR_BABBLING_CHANCE;
    MOTOR_BABBLING_CHANCE = 0.2;

    srand(1337);
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(false);

    Term termA1_left = Encode_Term("exp1_A1_left");
    Term termA1_right = Encode_Term("exp1_A1_right");
    Term termA2_left = Encode_Term("exp1_A2_left");
    Term termA2_right = Encode_Term("exp1_A2_right");
    Term termG = Encode_Term("exp1_G");
    Term opLeft = Encode_Term("exp1_op_left");
    Term opRight = Encode_Term("exp1_op_right");

    MSC_AddOperation(opLeft, Exp1_OpLeft);
    MSC_AddOperation(opRight, Exp1_OpRight);

    int baseline_total = EXP1_BASELINE_BLOCKS * EXP_BLOCK_TRIALS;
    int testing_total = EXP1_TESTING_BLOCKS * EXP_BLOCK_TRIALS;

    int baseline_last_block = 0;
    int baseline_correct = Exp_RunPhase(EXP1_BASELINE_BLOCKS,
                                        false,
                                        true,
                                        termA1_left,
                                        termA1_right,
                                        termA2_left,
                                        termA2_right,
                                        opLeft,
                                        opRight,
                                        termG,
                                        "baseline",
                                        NULL,
                                        &baseline_last_block);

    int training_last_block = 0;
    Exp_RunPhase(EXP1_TRAINING_BLOCKS,
                 true,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "training",
                 NULL,
                 &training_last_block);

    int testing_last_block = 0;
    int testing_correct = Exp_RunPhase(EXP1_TESTING_BLOCKS,
                                       false,
                                       true,
                                       termA1_left,
                                       termA1_right,
                                       termA2_left,
                                       termA2_right,
                                       opLeft,
                                       opRight,
                                       termG,
                                       "testing",
                                       NULL,
                                       &testing_last_block);

    assert(baseline_correct < baseline_total, "Experiment 1: baseline should not be perfect");
    assert(training_last_block == EXP_BLOCK_TRIALS, "Experiment 1: final training block was not perfect");
    assert(testing_last_block == EXP_BLOCK_TRIALS, "Experiment 1: final testing block was not perfect");
    assert(testing_correct == testing_total, "Experiment 1: testing accuracy should remain perfect overall");

    MOTOR_BABBLING_CHANCE = original_babbling;
    puts("<<MSC Experiment 1 test successful");
}

void MSC_Exp1_TrainingOnly(void)
{
    puts(">>MSC Experiment 1 (training only) start");

    double original_babbling = MOTOR_BABBLING_CHANCE;
    MOTOR_BABBLING_CHANCE = 0.9;

    srand(1337);
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(false);

    Term termA1_left = Encode_Term("exp1train_A1_left");
    Term termA1_right = Encode_Term("exp1train_A1_right");
    Term termA2_left = Encode_Term("exp1train_A2_left");
    Term termA2_right = Encode_Term("exp1train_A2_right");
    Term termG = Encode_Term("exp1train_G");
    Term opLeft = Encode_Term("exp1train_op_left");
    Term opRight = Encode_Term("exp1train_op_right");

    MSC_AddOperation(opLeft, Exp1_OpLeft);
    MSC_AddOperation(opRight, Exp1_OpRight);

    int training_last_block = 0;
    int training_correct = Exp_RunPhase(EXP1_TRAINING_BLOCKS,
                                        true,
                                        true,
                                        termA1_left,
                                        termA1_right,
                                        termA2_left,
                                        termA2_right,
                                        opLeft,
                                        opRight,
                                        termG,
                                        "training",
                                        NULL,
                                        &training_last_block);

    printf("Training accuracy: %d/%d, last block %d/%d\n",
           training_correct,
           EXP1_TRAINING_BLOCKS * EXP_BLOCK_TRIALS,
           training_last_block,
           EXP_BLOCK_TRIALS);

    MOTOR_BABBLING_CHANCE = original_babbling;
    puts("<<MSC Experiment 1 (training only) end");
}

void MSC_Exp1_ExportCSV(const char *path)
{
    FILE *csv = fopen(path, "w");
    if(!csv)
    {
        perror("Failed to open CSV output");
        return;
    }
    fprintf(csv, "phase,block,trial,a1_left,chosen_op,correct,exp_a1_left,exp_a1_right,exp_a2_left,exp_a2_right\n");

    double original_babbling = MOTOR_BABBLING_CHANCE;
    MOTOR_BABBLING_CHANCE = 0.2;

    srand(1337);
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(false);

    Term termA1_left = Encode_Term("exp1csv_A1_left");
    Term termA1_right = Encode_Term("exp1csv_A1_right");
    Term termA2_left = Encode_Term("exp1csv_A2_left");
    Term termA2_right = Encode_Term("exp1csv_A2_right");
    Term termG = Encode_Term("exp1csv_G");
    Term opLeft = Encode_Term("exp1csv_op_left");
    Term opRight = Encode_Term("exp1csv_op_right");

    MSC_AddOperation(opLeft, Exp1_OpLeft);
    MSC_AddOperation(opRight, Exp1_OpRight);

    int dummy_last_block = 0;
    Exp_RunPhase(EXP1_BASELINE_BLOCKS,
                 false,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "baseline",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP1_TRAINING_BLOCKS,
                 true,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "training",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP1_TESTING_BLOCKS,
                 false,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "testing",
                 csv,
                 &dummy_last_block);

    fclose(csv);
    MOTOR_BABBLING_CHANCE = original_babbling;
    printf("Experiment 1 CSV written to %s\n", path);
}

void MSC_Exp2_ExportCSV(const char *path)
{
    FILE *csv = fopen(path, "w");
    if(!csv)
    {
        perror("Failed to open CSV output");
        return;
    }
    fprintf(csv, "phase,block,trial,a1_left,chosen_op,correct,exp_a1_left,exp_a1_right,exp_a2_left,exp_a2_right\n");

    double original_babbling = MOTOR_BABBLING_CHANCE;
    MOTOR_BABBLING_CHANCE = 0.2;

    srand(1337);
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(false);

    Term termA1_left = Encode_Term("exp2_A1_left");
    Term termA1_right = Encode_Term("exp2_A1_right");
    Term termA2_left = Encode_Term("exp2_A2_left");
    Term termA2_right = Encode_Term("exp2_A2_right");
    Term termG = Encode_Term("exp2_G");
    Term opLeft = Encode_Term("exp2_op_left");
    Term opRight = Encode_Term("exp2_op_right");

    MSC_AddOperation(opLeft, Exp1_OpLeft);
    MSC_AddOperation(opRight, Exp1_OpRight);

    int dummy_last_block = 0;
    Exp_RunPhase(EXP2_BASELINE_BLOCKS,
                 false,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "baseline",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP2_TRAINING1_BLOCKS,
                 true,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "training1",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP2_TESTING1_BLOCKS,
                 false,
                 true,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "testing1",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP2_TRAINING2_BLOCKS,
                 true,
                 false,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "training2",
                 csv,
                 &dummy_last_block);

    Exp_RunPhase(EXP2_TESTING2_BLOCKS,
                 false,
                 false,
                 termA1_left,
                 termA1_right,
                 termA2_left,
                 termA2_right,
                 opLeft,
                 opRight,
                 termG,
                 "testing2",
                 csv,
                 &dummy_last_block);

    fclose(csv);
    MOTOR_BABBLING_CHANCE = original_babbling;
    printf("Experiment 2 CSV written to %s\n", path);
}
