#include "arena.h"


int main(int argc, char *argv[])
{
    MyArena Arena;

    InitArena(&Arena, 20);

    double a = 0.5, b = 0.25;
    ArenaPush(&Arena, &a, sizeof(double));
    ArenaPush(&Arena, &b, sizeof(double));
    ArenaPush(&Arena, &a, sizeof(double));

    FreeArena(&Arena);
    
    return 0;
}