#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "Globals.h"
#include "Encode.h"
#include "MSC.h"
#include "demos.h"

static bool MSC_Pong_Left_executed = false;
static void MSC_Pong_Left(void)
{
    MSC_Pong_Left_executed = true;
}
static bool MSC_Pong_Right_executed = false;
static void MSC_Pong_Right(void)
{
    MSC_Pong_Right_executed = true;
}
static bool MSC_Pong_Stop_executed = false;
static void MSC_Pong_Stop(void)
{
    MSC_Pong_Stop_executed = true;
}

static void MSC_Pong2_Run(bool headless)
{
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(!headless);
    puts(headless ? ">>MSC Pong start (headless)" : ">>MSC Pong start");
    MSC_AddOperation(Encode_Term("op_left"), MSC_Pong_Left);
    MSC_AddOperation(Encode_Term("op_right"), MSC_Pong_Right);
    MSC_AddOperation(Encode_Term("op_stop"), MSC_Pong_Stop);
    int szX = 50;
    int szY = 20;
    int ballX = szX/2;
    int ballY = szY/5;
    int batX = 20;
    int batVX = 0;
    int batWidth = 6;
    int vX = 1;
    int vY = 1;
    int hits = 0;
    int misses = 0;
    int t = 0;
    while(1)
    {
        t++;
        if(!headless)
        {
            fputs("[1;1H[2J", stdout);
            for(int i = 0; i < batX - batWidth + 1; i++)
            {
                fputs(" ", stdout);
            }
            for(int i = 0; i < batWidth * 2 - 1 + MIN(0, batX); i++)
            {
                fputs("@", stdout);
            }
            puts("");
            for(int i = 0; i < ballY; i++)
            {
                for(int k = 0; k < szX; k++)
                {
                    fputs(" ", stdout);
                }
                puts("|");
            }
            for(int i = 0; i < ballX; i++)
            {
                fputs(" ", stdout);
            }
            fputs("#", stdout);
            for(int i = ballX + 1; i < szX; i++)
            {
                fputs(" ", stdout);
            }
            puts("|");
            for(int i = ballY + 1; i < szY; i++)
            {
                for(int k = 0; k < szX; k++)
                {
                    fputs(" ", stdout);
                }
                puts("|");
            }
        }
        if(batX <= ballX - batWidth)
        {
            MSC_AddInputBelief(Encode_Term("ball_right"), 0);
        }
        else if(ballX + batWidth < batX)
        {
            MSC_AddInputBelief(Encode_Term("ball_left"), 0);
        }
        else
        {
            MSC_AddInputBelief(Encode_Term("ball_equal"), 0);
        }
        MSC_AddInputGoal(Encode_Term("good_msc"));
        if(ballX <= 0)
        {
            vX = 1;
        }
        if(ballX >= szX - 1)
        {
            vX = -1;
        }
        if(ballY <= 0)
        {
            vY = 1;
        }
        if(ballY >= szY - 1)
        {
            vY = -1;
        }
        if(t % 2 == -1)
        {
            ballX += vX;
        }
        ballY += vY;
        if(ballY == 0)
        {
            if(abs(ballX - batX) <= batWidth)
            {
                MSC_AddInputBelief(Encode_Term("good_msc"), 0);
                if(!headless)
                {
                    puts("good");
                }
                hits++;
            }
            else
            {
                if(!headless)
                {
                    puts("bad");
                }
                misses++;
            }
        }
        if(ballY == 0 || ballX == 0 || ballX >= szX - 1)
        {
            ballY = szY / 2 + rand() % (szY / 2);
            ballX = rand() % szX;
            vX = rand() % 2 == 0 ? 1 : -1;
        }
        if(MSC_Pong_Left_executed)
        {
            MSC_Pong_Left_executed = false;
            if(!headless)
            {
                puts("Exec: op_left");
            }
            batVX = -3;
        }
        if(MSC_Pong_Right_executed)
        {
            MSC_Pong_Right_executed = false;
            if(!headless)
            {
                puts("Exec: op_right");
            }
            batVX = 3;
        }
        if(MSC_Pong_Stop_executed)
        {
            MSC_Pong_Stop_executed = false;
            if(!headless)
            {
                puts("Exec: op_stop");
            }
            batVX = 0;
        }
        batX = MAX(-batWidth * 2, MIN(szX - 1 + batWidth, batX + batVX * batWidth / 2));
        printf("Hits=%d misses=%d ratio=%f time=%ld\n", hits, misses, (float)(((float)hits) / ((float)misses)), currentTime);
        if(!headless)
        {
            nanosleep((struct timespec[]){{0, 20000000L}}, NULL);
        }
    }
}

void MSC_Pong2(void)
{
    MSC_Pong2_Run(false);
}

void MSC_Pong2_Headless(void)
{
    MSC_Pong2_Run(true);
}

static void MSC_Pong_Run(bool headless)
{
    OUTPUT = 0;
    MSC_INIT();
    MSC_SetInputLogging(!headless);
    puts(headless ? ">>MSC Pong start (headless)" : ">>MSC Pong start");
    MSC_AddOperation(Encode_Term("op_left"), MSC_Pong_Left);
    MSC_AddOperation(Encode_Term("op_right"), MSC_Pong_Right);
    int szX = 50;
    int szY = 20;
    int ballX = szX/2;
    int ballY = szY/5;
    int batX = 20;
    int batVX = 0;
    int batWidth = 4;
    int vX = 1;
    int vY = 1;
    int hits = 0;
    int misses = 0;
    while(1)
    {
        if(!headless)
        {
            fputs("[1;1H[2J", stdout);
            for(int i = 0; i < batX - batWidth + 1; i++)
            {
                fputs(" ", stdout);
            }
            for(int i = 0; i < batWidth * 2 - 1; i++)
            {
                fputs("@", stdout);
            }
            puts("");
            for(int i = 0; i < ballY; i++)
            {
                for(int k = 0; k < szX; k++)
                {
                    fputs(" ", stdout);
                }
                puts("|");
            }
            for(int i = 0; i < ballX; i++)
            {
                fputs(" ", stdout);
            }
            fputs("#", stdout);
            for(int i = ballX + 1; i < szX; i++)
            {
                fputs(" ", stdout);
            }
            puts("|");
            for(int i = ballY + 1; i < szY; i++)
            {
                for(int k = 0; k < szX; k++)
                {
                    fputs(" ", stdout);
                }
                puts("|");
            }
        }
        if(batX < ballX)
        {
            MSC_AddInputBelief(Encode_Term("ball_right"), 0);
        }
        if(ballX < batX)
        {
            MSC_AddInputBelief(Encode_Term("ball_left"), 0);
        }
        MSC_AddInputGoal(Encode_Term("good_msc"));
        if(ballX <= 0)
        {
            vX = 1;
        }
        if(ballX >= szX - 1)
        {
            vX = -1;
        }
        if(ballY <= 0)
        {
            vY = 1;
        }
        if(ballY >= szY - 1)
        {
            vY = -1;
        }
        ballX += vX;
        ballY += vY;
        if(ballY == 0)
        {
            if(abs(ballX - batX) <= batWidth)
            {
                MSC_AddInputBelief(Encode_Term("good_msc"), 0);
                if(!headless)
                {
                    puts("good");
                }
                hits++;
            }
            else
            {
                if(!headless)
                {
                    puts("bad");
                }
                misses++;
            }
        }
        if(ballY == 0 || ballX == 0 || ballX >= szX - 1)
        {
            ballY = szY / 2 + rand() % (szY / 2);
            ballX = rand() % szX;
            vX = rand() % 2 == 0 ? 1 : -1;
        }
        if(MSC_Pong_Left_executed)
        {
            MSC_Pong_Left_executed = false;
            if(!headless)
            {
                puts("Exec: op_left");
            }
            batVX = -2;
        }
        if(MSC_Pong_Right_executed)
        {
            MSC_Pong_Right_executed = false;
            if(!headless)
            {
                puts("Exec: op_right");
            }
            batVX = 2;
        }
        batX = MAX(0, MIN(szX - 1, batX + batVX * batWidth / 2));
        printf("Hits=%d misses=%d ratio=%f time=%ld\n", hits, misses, (float)(((float)hits) / ((float)misses)), currentTime);
        if(!headless)
        {
            nanosleep((struct timespec[]){{0, 20000000L}}, NULL);
        }
    }
}

void MSC_Pong(void)
{
    MSC_Pong_Run(false);
}

void MSC_Pong_Headless(void)
{
    MSC_Pong_Run(true);
}

