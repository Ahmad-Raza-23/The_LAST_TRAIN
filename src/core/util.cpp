#include "util.h"
#include "config.h"
#include <iostream>
#include <cctype>
#include <cstring>
using namespace std;
// Primitive helper utilities, clamping, and screen control
int clampInt(int val, int minVal, int maxVal)
{
if ( val < minVal)
     return minVal;
if ( val >  minVal)
     return val;     
}

// removes spaces/tabs/newlines/from both ends of C-string(in place)
void trimWhitespace( char* str)
{
    if (str == nullptr)
    {
    return;
    }
    int len = (int)strlen(str);
    int start =0;
    while(start < len && isspace((unsigned char)str[start]))
    {
        start ++;
    }
    
    int end = len - 1;
    while(end >= start && isspace((unsigned char)str[start]))
    {
        end --;
    }
    
    int j = 0;
    for (int i = start; i <= end; i++)
    {
        str[j]=str[i];
        j++;
    }
    str[j] = '\0';
}    
// case insensitive comparison of two C-strings("MOVE" == "move")
bool stringEquals(const char* a,const char* b)
{
    if(a == nullptr || b == nullptr)
    {
        return false;
    } 

    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
        {
            return false;
        }
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

void printHeader(const char* title)
{
    cout << "\n=================================";
    cout << " "<< title << "\n";
    cout << "=================================";
}
void clearScreen()
{
   // ANSI escape : clear screen and move cursor to top-left
   cout << "\033[2J\033[H";
}
//turn text like " F2-R3" or (or "f2-r3") into floor = 2 and room = 3
bool parseRoomCode(const char* text, int& floor, int& room)
{
    if (text==nullptr || strlen(text) != 5)
    {
        return false;
    }
    if (toupper((unsigned char)text[0]) != 'F')
    {
        return false;
    }
     if (!isdigit((unsigned char)text[1]))
    {
        return false;
    }
     if (text[2] != '-')
    {
        return false;
    }
     if (toupper((unsigned char)text[3]) != 'R')
    {
        return false;
    }
     if (!isdigit((unsigned char)text[4]))
    {
        return false;
    }
    int f = text[1] - '0';
    int r = text[4] - '0';
    if (f < 1 || f > NUM_FLOORS || r < 1 || r > ROOMS_PER_FLOOR)
    {
        return false;
    }
    floor = f;
    room = r;
    return true;
}
