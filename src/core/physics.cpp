#include "physics.h"
#include "util.h"

// Stairs and lifts are in vertical connection not in horizontal connection 
const char STAIRS='S',LIFT1='1',LIFT2='2';
bool isDirectlyConnected(int fromF,int fromR,int toF,int toR,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1],char& outConnType){
    outConnType=CONN_NONE;//in case there isnt any connection
//checks boundary 
    if(fromF<0 || fromF>=NUM_FLOORS || toF<0 || toF>=NUM_FLOORS)
        return false;
    if(fromR<0 || fromR>=ROOMS_PER_FLOOR || toR<0 || toR>=ROOMS_PER_FLOOR)
        return false;
    //checks if its horizontal movement
    if(fromF==toF){
        if(fromR-toR!=1 && toR-fromR!=1)//so that players dont jump i-e if player jumps from 1 to 3
            return false;
        int left=fromR;//initializes
        if(toR<fromR)//in case you are moving from smaller to greater
            left = toR;
        outConnType=horizontalConn[fromF][left];//wall or window whatever
        return outConnType!=CONN_NONE;}//true or false
         if (fromR != toR) 
        return false;//Stairs and Lifts connect rooms directly above/below each other 
    int low=fromF,high=toF;
    if(toF<fromF){ //if moving down
        low=toF;
        high=fromF;}
    // Stairs:R1,F1-F2,F2-F3,F4-F5
    if(fromR==0 && toR==0 && high==low+1 && low!=2){//player can move from room 1 in same column through stairs except from room 3 to 4
     //for room 1          //every floor except F3-F4
		outConnType=STAIRS;
        return true;}
    // Lift 1:F1 and F2,rooms R2/R3
    if(low==0 && high==1  && fromR>=1 && fromR<=2 && toR>=1 && toR<=2){
    //works for f-1 and f-2   //room 2 and 3
		outConnType=LIFT1;
        return true;}
    // Lift 2: R4 on F2, F4, F5
    if(fromR==3 && toR==3 && fromF!=2 && toF!=2 && fromF!=0 && toF!=0){
    //for room 4 of f2,4,5
		outConnType=LIFT2;
        return true;}
    return false;}
bool canTraverse(int targetFloor,int targetRoom,char connType,const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,int emergencyTripsLeft,bool lift2Operational){
    (void)targetFloor;  //unused parameters
    (void)targetRoom;
    if(connType==CONN_DOOR||connType==CONN_WINDOW||connType == STAIRS || connType ==LIFT1)
	 return true;//allowed
    if(connType==CONN_LOCKED_DOOR)//f3-r1 and f3-r2(locked)
     return hasItem(inventory,inventoryCount,ENT_MASTER_KEY);//if player has master key,true
                    //array    //counter      //key
    if(connType==LIFT2)
      return (lift2Operational||emergencyTripsLeft>0);
              //fuse installed?   //emergency access card trips?
    return false;}   // if there is a wall,block
void applyResourceDelta(int& energy,int& time,int deltaEnergy,int deltaTime){  //for updating
 energy=clampInt(energy+deltaEnergy,0,MAX_ENERGY);
                    //value        //min  //max
 time=clampInt(time+deltaTime,0,MAX_TIME);}
                 //same
void applyScoreDelta(int& score,int deltaScore){ //updates score
  score+=deltaScore;}
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,char itemSymbol){//checks for items
  for(int i=0;i<inventoryCount;i++){
    if(inventory[i]==itemSymbol)
    return true;}
    return false;}
bool addItem(char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount, char itemSymbol){ 
  if(inventoryCount>=MAX_INVENTORY_CAPACITY)//max capacity=10
    return false;
    inventory[inventoryCount]=itemSymbol;//add items in array
    inventoryCount++;
return true;}
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,char itemSymbol){
 for(int i=0;i<inventoryCount;i++){
    if(inventory[i]==itemSymbol){//if the item is present here
     for(int j=i;j<inventoryCount-1;j++)
      inventory[j]=inventory[j + 1];   //shifts left
      inventoryCount--;//decrement
      return true;}}
    return false;}//if item is not found,it returns false