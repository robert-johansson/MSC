#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "Globals.h"
#include "Encode.h"
#include "MSC.h"
#include "demos.h"

static bool MSC_Alien_Left_executed = false;
static void MSC_Alien_Left(void)
{
    puts("MSC invoked left");
    MSC_Alien_Left_executed = true;
}
static bool MSC_Alien_Right_executed = false;
static void MSC_Alien_Right(void)
{
    puts("MSC invoked right");
    MSC_Alien_Right_executed = true;
}
static bool MSC_Alien_Shoot_executed = false;
static void MSC_Alien_Shoot(void)
{
    puts("MSC invoked shoot");
    MSC_Alien_Shoot_executed = true;
}
void MSC_Alien(void)
{
    OUTPUT = 0;
    MSC_INIT();
    puts(">>MSC Alien1 start");
    MSC_AddOperation(Encode_Term("op_left"), MSC_Alien_Left); 
    MSC_AddOperation(Encode_Term("op_right"), MSC_Alien_Right); 
    MSC_AddOperation(Encode_Term("op_shoot"), MSC_Alien_Shoot); 
    double alien0X = 0.5;
    double defenderX = 0.5;
    double alienWidth = 0.18;
    int hits = 0;
    int shots = 0;
    int t=0;
    while(1)
    {
        if(t++%10000 == 0)
        {
            getchar();
        }
        fputs("\033[1;1H\033[2J", stdout); //POSIX clear screen
        bool cond1 = (defenderX <= alien0X - alienWidth);
        bool cond2 = (defenderX >  alien0X + alienWidth);
        if(cond1)
        {
            MSC_AddInputBelief(Encode_Term("r0"), 0);
        }
        else if(cond2)
        {
            MSC_AddInputBelief(Encode_Term("l0"), 0);
        }
        else
        {
            MSC_AddInputBelief(Encode_Term("c0"), 0);
        }
        MSC_AddInputGoal(Encode_Term("s0"));
        if(MSC_Alien_Shoot_executed)
        {
            MSC_Alien_Shoot_executed = false;
            shots++;
            if(!cond1 && !cond2)
            {
                hits++;
                MSC_AddInputBelief(Encode_Term("s0"), 0);
                alien0X = ((double)(rand() % 1000)) / 1000.0;
            }
        }
        if(MSC_Alien_Left_executed)
        {
            MSC_Alien_Left_executed = false;
            defenderX = MAX(0.0, defenderX-0.1);
        }
        if(MSC_Alien_Right_executed)
        {
            MSC_Alien_Right_executed = false;
            defenderX = MIN(1.0, defenderX+0.1);
        }
        printf("shots=%d hits=%d percenta=%f time=%ld\n", shots, hits, (float) (((float) hits) / ((float) shots)), currentTime);
        //nanosleep((struct timespec[]){{0, 10000000L}}, NULL); //POSIX sleep
        //MSC_Cycles(10);
    }
}

