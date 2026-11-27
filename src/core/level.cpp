#include "level.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cstring>
using namespace std;

// Modular implementation using primitive arrays for map parsing and verification

//room names , indexed [Floor-1][Room-1]
static const char* ROOM_NAMES[NUM_FLOORS][ROOMS_PER_FLOOR]={{"Enterance Lobby","Reception","Waiting Area","Utility Room"},
                                                            {"Lounge","Archive","Lab","Meeting Room"},
                                                            {"Stairwell Annex","Security Vestiblue","Animal Room","Control Room"},
                                                            {"First Aid Station","Medical Room","Maintenance Corridor","Office"},
                                                            {"Security Room","Train Boarding Platform","Storage Room","Observation Deck"}};

//Reading data/building.txt
//every floor in the uses two lines:
//name line  :"|  | F5-R1   D F5-R2   D F5-R3  W F5-R4   |"
//entity name:"|  |  7  ^   |         |     @  |     &   |"  
//every room cell is 11 characters wide and followed by one separator column,so starting from the left wall '|' of room 1 (column "base"):
//- the separator between room g+1 and g+2 is at base + 12 * (g+1)
//- the cell of room r (0-based) spans base + 1 + 12 * r ... base + 11 + 12 * r

bool loadBuildingMap( const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
                    {
                       //start with an empty building
                       for (int f=0; f<NUM_FLOORS;f++)
                       {
                            for (int g=0;g < ROOMS_PER_FLOOR - 1;g++)
                                {
                                    horizontalConn[f][g]=CONN_NONE;
                                }
                            for (int r=0; ROOMS_PER_FLOOR;r++)
                                {
                                    entityCounts[f][r]=0;
                                    for ( int e=0;e<MAX_ENTITIES_PER_ROOM;e++)
                                    {
                                        roomEntities[f][r][e]= ENT_NONE;
                                    }                                
                                }    
                        }
                        ifstream fin(filename);
                        if (!fin)
                        {
                             return false;
                        }

                        char line[MAX_LINE_LEN];
                        char entLine[MAX_LINE_LEN];
                        int floorsFound = 0;

                        while (fin.getline(line,MAX_LINE_LEN))
                        {
                           int len = (int)strlen(line);
                           //look for the token "F<digit>-R1" that starts a floor now
                           int pos = -1;
                           int floor = 0;
                           for( int i = 0; i + 4 < len;i++)
                           {
                            if(line[i]=='F' && isdigit((unsigned char)line[i+1]) && line[i+2]=='-' && line[i+3]=='R' && line[i+4]=='1')
                            {
                              pos = i;
                              floor = line[i+1]-'0';
                              break;  
                            }
                           }
                        // A real floor row has the wall '|' two columns before the room code
                        //this skips legend lines such as "S1 = staircase connecting F1-R1 and F2-R1" 
                            if (pos < 2 || line[pos-2] != '|' || floor < 1 || floor > NUM_FLOORS)
                            {
                                continue;
                            }
                            int base = pos - 2; // column of the left wall '|'
                            //1.horizontal connections between neigbouring rooms
                            for ( int g = 0; g < ROOMS_PER_FLOOR - 1;g++)
                            {
                                int col = base + 12 * (g + 1);
                                char c = (col < len) ? line[col] : ' ';
                                if (c == CONN_DOOR || c == CONN_WINDOW || c == CONN_LOCKED_DOOR || c == CONN_SOLID_WALL)
                                {
                                     horizontalConn[floor - 1][g] = c;   
                                }
                                //"L1" shows up here as '1' -> it is left shift,not a door i.e, stays CONN_NONE
                            }
                            //2.Entity line right below the name line
                            if (!fin.getline(entLine,MAX_LINE_LEN))
                            break;
                            int elen = (int)strlen(entLine);
                            for(int r=0; r< ROOMS_PER_FLOOR;r++)
                            {
                              int startCol = base + 1 + 12 * r;
                              int endCol   = base + 11 + 12 * r;
                              for (int col = startCol; col<= endCol && col <= elen;col++)
                                {
                                   char c = entLine[col];
                                   if( c != ' ' && c != '|' && entityCounts[floor - 1][r] < MAX_ENTITIES_PER_ROOM)
                                   {
                                    roomEntities[floor - 1][r][entityCounts[floor-1][r]]=c;
                                    entityCounts[floor - 1][r]++;
                                   }
                                }
                            }
                            floorsFound++;         
                        }
                        // the specification says F1-R4 holds Two energy packs,the map draws only one symbol
                        int packs=0;
                        for(int e = 0;e < entityCounts[0][3];e++)
                        {
                            if (roomEntities[0][3][e] == ENT_ENERGY_PACK) packs++; 
                        }
                        if (packs == 1 && entityCounts[0][3] < MAX_ENTITIES_PER_ROOM)
                        {
                            roomEntities[0][3][entityCounts[0][3]] = ENT_ENERGY_PACK;
                            entityCounts[0][3]++;  
                        }
                        return floorsFound == NUM_FLOORS;
                    }
                    // "YES"/"NO" into true/false
                    static bool isYes(const char* value)
                    {
                        return stringEquals(value, "YES") || stringEquals(value, "TRUE") || stringEquals(value, "1");  
                    }


        
