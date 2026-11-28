#include "switches.h"
#include "util.h"
#include "physics.h"

// Door lock mechanics, lifts, and cabinet verification without structures


// Lift 1 works without any condition between Floor 1 and Floor 2
bool canUseLift1(int fromFloor, int toFloor) 
{
    return (fromFloor == 1 && toFloor == 2) || (fromFloor == 2 && toFloor == 1);
}

static bool isLift2Floor(int floor) 
{
    return (floor == 2 || floor == 4 || floor == 5);
}

// Lift 2 works normally once repaired, otherwise only while emergency trips remain
bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft) 
{
    if (!isLift2Floor(fromFloor) || !isLift2Floor(toFloor)) 
        return false;
    if (fromFloor == toFloor) 
        return false;
    return lift2Operational || emergencyTripsLeft > 0;
}

// The locked door F3-R1 <-> F3-R2 opens only with the Master Key
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount) 
{
    return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
}

// Puts the Fuse into the Control Panel (location is checked by the caller)
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                             bool objectives[NUM_OBJECTIVES], int& score) 
{
    if (objectives[OBJ_FUSE_INSTALLED]) 
        return false;
    if (!removeItem(inventory, inventoryCount, ENT_FUSE)) 
        return false;

    objectives[OBJ_FUSE_INSTALLED] = true;
    applyScoreDelta(score, SCORE_INSTALL_FUSE);

    // Lift 2 needs BOTH the fuse and the lift pass
    if (objectives[OBJ_LIFT_PASS]) 
    {
        objectives[OBJ_LIFT2_ACTIVE] = true;
    }
    return true;
}

// Correct code is 427 and all three clue digits must have been found first
bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES]) 
{
    if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7]) 
    {
        return false;
    }
    return stringEquals(enteredCode, "427");
}