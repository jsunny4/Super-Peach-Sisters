#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
#include <iomanip>  // defines the manipulator setw
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_peach(nullptr), complete(false), hasWon(false)
{
}
//init method.,
int StudentWorld::init()
{
    Level lev(assetPath());
    ostringstream oss;
    oss.fill('0');
    oss << setw(2) << getLevel();
    string l = oss.str();
    string file = "level" + l + ".txt";
    Level::LoadResult result = lev.loadLevel(file);
    if (result == Level::load_fail_file_not_found || result == Level::load_fail_bad_format)
    {
         return GWSTATUS_LEVEL_ERROR;
    }
    else if (result == Level::load_success) {
        for (int x = 0; x < 32; x++) {
            for (int y = 0; y < 32; y++) {
                Level::GridEntry ge = lev.getContentsOf(x, y);
                switch (ge) {
                    case Level::peach: {
                        m_peach = new Peach(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        break;
                    }
                    case Level::block: {
                        Block* block = new Block(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(block);
                        break;
                    }
                    case Level::mushroom_goodie_block: {
                        Block* block = new Block(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT, 1);
                        m_actors.push_back(block);
                        break;
                    }
                    case Level::flower_goodie_block: {
                        Block* block = new Block(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT, 2);
                        m_actors.push_back(block);
                        break;
                    }
                    case Level::star_goodie_block: {
                        Block* block = new Block(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT, 3);
                        m_actors.push_back(block);
                        break;
                    }
                    case Level::pipe: {
                        Pipe* pipe = new Pipe(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(pipe);
                        break;
                    }
                    case Level::goomba: {
                       Goomba* goomba = new Goomba(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(goomba);
                        break;
                    }
                    case Level::koopa: {
                        Koopa* koopa = new Koopa(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(koopa);
                        break;
                    }
                    case Level::piranha: {
                        Piranha* piranha = new Piranha(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(piranha);
                        break;
                    }
                    case Level::flag: {
                        Flag* flag = new Flag(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(flag);
                        break;
                    }
                    case Level::mario: {
                        Mario* mario = new Mario(this,x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(mario);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    setGame(false);
    return GWSTATUS_CONTINUE_GAME;
}
//move method
int StudentWorld::move()
{
    m_peach->doSomething();
    for (Actor* actor : m_actors) {
        actor->doSomething();
        if (!m_peach->isAlive()) {
            playSound(SOUND_PLAYER_DIE);
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }

        if (complete) {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
        if (hasWon) {
            playSound(SOUND_GAME_OVER);
            return GWSTATUS_PLAYER_WON;
        }
    }
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        if (!(*it)->isAlive()) {
            delete *it;
            m_actors.erase(it);
            it--;
        }
    }
    string star = "", shoot = "", jump = "";
    if (m_peach->star() > 0) {
        star = " StarPower!";
    }
    if (m_peach->flower()) {
        shoot = " ShootPower!";
    }
    if (m_peach->jump()) {
        jump = " JumpPower!";
    }
    ostringstream o;
    o.fill('0');
    o << setw(6) << getScore();
    ostringstream oss;
    oss << "Lives: " << getLives() << "  Level: " << getLevel() << "  Points: " << o.str() << star << shoot << jump;
    string text = oss.str();
    setGameStatText(text);
    return GWSTATUS_CONTINUE_GAME;
}
//cleanup method
void StudentWorld::cleanUp()
{
    delete m_peach;
    for (Actor* actor : m_actors) {
        delete actor;
    }
    m_actors.clear();
    m_peach = nullptr;
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

void StudentWorld::addActor(Actor* a) {
    m_actors.push_back(a);
}
//returns true if coordinates is blcoked by a block or pipe actor
bool StudentWorld:: isBlocking(int x, int y, Actor*& a) {
    for (Actor* actor: m_actors) {
        if (actor->isAlive() && x + SPRITE_WIDTH - 1 >= actor->getX() && x <= actor->getX() + SPRITE_WIDTH - 1 && y + SPRITE_HEIGHT - 1 >= actor->getY() && y <= actor->getY() + SPRITE_HEIGHT - 1) {
            a = actor;
            if (!actor->isKillable())
                return true;
        }
    }
    return false;
}

Peach* StudentWorld::getPeach() {
    return m_peach;
}
//returns true if off edge
bool StudentWorld::offEdge(int x, int y) {
    Actor *a = nullptr;
    if (!isBlocking(x+7, y-1, a) || (!isBlocking(x-7, y-1, a))) {
        return true;
    }
    return false;
}
//returns true if touching an enemy and the enemy, false otherwise
bool StudentWorld::isTouchingEnemy(Actor* a, Actor*& act) {
    int x = a->getX();
    int y = a->getY();
    for (Actor* actor: m_actors) {
        if (actor->isEnemy())
            if (actor->getX() + SPRITE_WIDTH - 1 >= x && actor->getX() <= x + SPRITE_WIDTH - 1 && actor->getY() + SPRITE_HEIGHT - 1 >= y && actor->getY() <= y + SPRITE_HEIGHT - 1) {
            act = actor;
            return true;
        }
    }
    return false;
}
//returns true if touching peach
bool StudentWorld::isTouchingPeach(Actor* actor) {
    if (m_peach->getX() + SPRITE_WIDTH - 1 >= actor->getX() && m_peach->getX() <= actor->getX() + SPRITE_WIDTH - 1 && m_peach->getY() + SPRITE_HEIGHT - 1 >= actor->getY() && m_peach->getY() <= actor->getY() + SPRITE_HEIGHT - 1) {
        return true;
    }
    return false;
}

void StudentWorld::setGame(bool b) {
    complete = b;
}

void StudentWorld::won() {
    hasWon = true;
}
