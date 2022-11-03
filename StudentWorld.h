#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include "Actor.h"
#include <vector>
#include <string>
using namespace std;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    Peach* getPeach();
    void addActor(Actor* a);
    bool isBlocking(int x, int y, Actor*& a);
    ~StudentWorld();
    bool offEdge(int x, int y);
    void setGame(bool b);
    void won();
    bool isTouchingPeach(Actor* a);
    bool isTouchingEnemy(Actor* a, Actor*& act);
private:
    vector<Actor*> m_actors;
    Peach* m_peach;
    bool complete;
    bool hasWon;
};

#endif // STUDENTWORLD_H_
