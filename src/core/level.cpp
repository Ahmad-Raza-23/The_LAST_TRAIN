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
//
// Reads data/initial-state.txt, a list of "KEY: VALUE" lines.
// START_ROOM may be PLAYER_SELECTS (playerFloor = 0 means "ask the player")
// or a room code such as F1-R1.

bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft) 
{
    // Default values from config.h
    playerFloor = 0;
    playerRoom = 0;
    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;
    inventoryCount = 0;
    emergencyTripsLeft = 0;
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++) 
        inventory[i] = ENT_NONE;
    for (int i = 0; i < NUM_OBJECTIVES; i++) 
        objectives[i] = false;

    ifstream fin(filename);
    if (!fin) 
        return false;

    char line[MAX_LINE_LEN];
    while (fin.getline(line, MAX_LINE_LEN)) 
    {
        char key[64];
        char value[64];
        if (sscanf(line, " %63[^:]: %63s", key, value) != 2) 
            continue;
        trimWhitespace(key);

        if (stringEquals(key, "START_ROOM")) 
        {
            int f = 0, r = 0;
            if (parseRoomCode(value, f, r)) 
            {
                playerFloor = f;
                playerRoom = r;
            }
        } 
        else if (stringEquals(key, "INITIAL_ENERGY")) 
        {
            energy = atoi(value);
        } 
        else if (stringEquals(key, "INITIAL_TIME")) 
        {
            time = atoi(value);
        } 
        else if (stringEquals(key, "INITIAL_SCORE")) 
        {
            score = atoi(value);
        } 
        else if (stringEquals(key, "INITIAL_INVENTORY")) 
        {
            if (!stringEquals(value, "NONE")) 
            {
                for (int i = 0; value[i] != '\0' && inventoryCount < MAX_INVENTORY_CAPACITY; i++) 
                {
                    if (value[i] != ',') 
                    {
                        inventory[inventoryCount] = value[i];
                        inventoryCount++;
                    }
                }
            }
        } 
        else if (stringEquals(key, "CLUES_COLLECTED")) 
        {
            if (!stringEquals(value, "NONE")) 
            {
                for (int i = 0; value[i] != '\0'; i++) 
                {
                    if (value[i] == ENT_CLUE_4) 
                        objectives[OBJ_CLUE_4] = true;
                    if (value[i] == ENT_CLUE_2) 
                        objectives[OBJ_CLUE_2] = true;
                    if (value[i] == ENT_CLUE_7) 
                        objectives[OBJ_CLUE_7] = true;
                }
            }
        } 
        else if (stringEquals(key, "PASSENGER_RESCUED")) 
        {
            objectives[OBJ_PASSENGER_RESCUED] = isYes(value);
        } 
        else if (stringEquals(key, "CAT_RESCUED")) 
        {
            objectives[OBJ_CAT_SECURED] = isYes(value);
        } 
        else if (stringEquals(key, "CABINET_UNLOCKED")) 
        {
            objectives[OBJ_CABINET_UNLOCKED] = isYes(value);
        } 
        else if (stringEquals(key, "TRAIN_KEY_COLLECTED")) 
        {
            objectives[OBJ_TRAIN_KEY] = isYes(value);
        } 
        else if (stringEquals(key, "FUSE_INSTALLED")) 
        {
            objectives[OBJ_FUSE_INSTALLED] = isYes(value);
        }
         else if (stringEquals(key, "LIFT_PASS_COLLECTED"))
        {
            objectives[OBJ_LIFT_PASS] = isYes(value);
        } 
        else if (stringEquals(key, "TRAIN_PREPARED")) 
        {
            objectives[OBJ_TRAIN_PREPARED] = isYes(value);
        }
        // GAME_STATUS, MAX_ENERGY, MAX_TIME, PLAYER_AT_TRAIN: informational only
    }

    energy = clampInt(energy, 0, MAX_ENERGY);
    time = clampInt(time, 0, MAX_TIME);
    return true;
}

// A starting room must exist; the train platform is the goal, so you cannot start there
bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])                           
{
    (void)horizontalConn; // every room in this building has at least one exit

    if (startFloor < 1 || startFloor > NUM_FLOORS) 
        return false;
    if (startRoom < 1 || startRoom > ROOMS_PER_FLOOR) 
        return false;
    if (startFloor == 5 && startRoom == 2) 
        return false;
    return true;
}

const char* getEntityName(char entitySymbol) {
    switch (entitySymbol) {
        case ENT_MASTER_KEY:        
            return "Master Key";
        case ENT_ENERGY_PACK:       
            return "Energy Pack";
        case ENT_EMERGENCY_TOOLKIT: 
            return "Emergency Toolkit";
        case ENT_CLUE_4:            
            return "Clue Digit 4";
        case ENT_FUSE:              
            return "Fuse";
        case ENT_CAT:               
            return "Cat";
        case ENT_CONTROL_PANEL:     
            return "Control Panel";
        case ENT_LIFT_PASS:         
            return "Lift Pass";
        case ENT_PASSENGER:         
            return "Injured Passenger";
        case ENT_MEDICAL_KIT:       
            return "Medical Kit";
        case ENT_EMERGENCY_CARD:    
            return "Emergency Access Card";
        case ENT_CLUE_2:            
            return "Clue Digit 2";
        case ENT_CLUE_7:            
            return "Clue Digit 7";
        case ENT_CABINET_TRAIN_KEY: 
            return "Cabinet / Train Key";
        case ENT_CAT_CARRIER:       
            return "Cat Carrier";
        case ENT_OBSERVATION_TOKEN: 
            return "Observation Token";
        default:                    
            return "Unknown Item";
    }
}

const char* getRoomName(int floor, int room) 
{
    if (floor < 1 || floor > NUM_FLOORS || room < 1 || room > ROOMS_PER_FLOOR) 
    return "Unknown Room";
    return ROOM_NAMES[floor - 1][room - 1];
}

        
