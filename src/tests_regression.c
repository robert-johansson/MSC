#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Term.h"
#include "Memory.h"
#include "Encode.h"
#include "MSC.h"
#include "PriorityQueue.h"
#include "Table.h"
#include "Implication.h"
#include "Decision.h"
#include "Globals.h"
#include "tests.h"

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
