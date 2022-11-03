#include "Actor.h"
#include "StudentWorld.h"
#include <stdlib.h>
//constructor for all actors
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int startDirection, int depth, double size) : GraphObject(imageID, startX, startY, startDirection, depth, size), m_world(world), alive(1) {}

Pipe::Pipe(StudentWorld* world, int x, int y) : Actor(world, IID_PIPE, x, y, 0, 2) {}

Block::Block(StudentWorld* world, int x, int y, int goodie) : Actor(world, IID_BLOCK, x, y, 0, 2), m_goodie(goodie) {}

Killable::Killable(StudentWorld* world, int ID, int x, int y, int points, int startDirection, int depth, double size) : Actor(world, ID, x, y, startDirection, depth, size), m_points(points) {};

Projectile::Projectile(StudentWorld* world, int ID, int x, int y, int startDirection) : Killable(world, ID, x, y, 0, startDirection, 1) {}

PiranhaFireball::PiranhaFireball(StudentWorld* world, int x, int y, int startDirection) : Projectile(world, IID_PIRANHA_FIRE, x, y, startDirection) {}

PeachFireball::PeachFireball(StudentWorld* world, int x, int y, int startDirection) : Projectile(world, IID_PEACH_FIRE, x, y, startDirection) {}

Shell::Shell(StudentWorld* world, int x, int y, int startDirection) : Projectile(world, IID_SHELL, x, y, startDirection) {}

Enemy::Enemy(StudentWorld* world, int ID, int x, int y) : Killable(world, ID, x, y, 100, 180 * (rand() % 2), 0, 1) {};

Goomba::Goomba(StudentWorld* world, int x, int y) : Enemy(world, IID_GOOMBA, x, y) {}

Koopa::Koopa(StudentWorld* world, int x, int y) : Enemy(world, IID_KOOPA, x, y) {}

Piranha::Piranha(StudentWorld* world, int x, int y) : Enemy(world, IID_PIRANHA, x, y) , delay(0){}

Item::Item(StudentWorld* world, int ID, int x, int y, int bonus) : Killable(world, ID, x, y, bonus, 0, 1) {}

Mushroom::Mushroom(StudentWorld* world, int x, int y) : Item(world, IID_MUSHROOM, x, y, 75) {}

Flower::Flower(StudentWorld* world, int x, int y) : Item(world, IID_FLOWER, x, y, 50) {}

Star::Star(StudentWorld* world, int x, int y) : Item(world, IID_STAR, x, y, 100) {}

Peach::Peach(StudentWorld* world, int x, int y) : Killable(world, IID_PEACH, x, y), remaining_jump(0), jump_power(false), flower_power(false), star_power(0), fire_cooldown(0), inv_cooldown(0) {}

Ender::Ender(StudentWorld* world, int ID, int x, int y) : Killable(world, ID, x, y, 1000,0, 1) {}

Flag::Flag(StudentWorld* world, int x, int y) : Ender(world, IID_FLAG, x, y) {}

Mario::Mario(StudentWorld* world, int x, int y) : Ender(world, IID_MARIO, x, y) {}

//methods that classify the actors
void Actor::doSomething() {}
StudentWorld* Actor::getWorld() {return m_world;}
bool Actor::isAlive() {return alive > 0;}
void Actor::bonk() {};
bool Actor::isKillable() {return false;}
bool Actor::isEnemy() {return false;}
void Actor::changeLives(int n) {alive=n;}
int Actor::getLives() {return alive;}
void Actor::addScore(int n) {getWorld()->increaseScore(n);}
//makes actor try to move to given location, actor moves to location if able and returns true, false otherwise
bool Actor::go(int x, int y, Actor*& a) {
    if (!getWorld()->isBlocking(x, y, a)) {
        moveTo(x, y);
        return true;
    }
    return false;
}

int Block::goodie() {return m_goodie;}
void Block::empty() {m_goodie = 0;}
//if not a goodie block, play sound, else it creates a item goodie depending on the kind of block and makes the block empty
void Block::bonk() {
    if (goodie() == 0) {
        getWorld()->playSound(SOUND_PLAYER_BONK);
    }
    else {
        getWorld()->playSound(SOUND_POWERUP_APPEARS);
        switch (goodie()) {
            case 1:
                getWorld()->addActor(new Mushroom(getWorld(), getX(), getY() + 8));
                break;
            case 2:
                getWorld()->addActor(new Flower(getWorld(), getX(), getY() + 8));
                break;
            case 3:
                getWorld()->addActor(new Star(getWorld(), getX(), getY() + 8));
                break;
        }
        empty();
    }
}
//if the killable function is touching peach and is alive, bonk it
void Killable::doSomething() {
    if (getWorld()->isTouchingPeach(this) && isAlive()) {
        bonk();
    }
}

bool Killable::isKillable() {return true;}

bool Projectile::isGood(){return true;}
bool PiranhaFireball::isGood() {return false;}
//if projectile is good and is touching an enemy, bonk it, otherwise do killable's do something. otherwise follow its movement
void Projectile::doSomething() {
    Actor* a = nullptr;
    if (isGood()) {
        if (getWorld()->isTouchingEnemy(this, a) && a->isAlive() && isAlive()) {
            bonk();
        }
    }
    else {
        Killable::doSomething();
    }
    if (!isAlive()) {
        return;
    }
    go(getX(), getY() - 2, a);
    if (getDirection() == 0) {
        if (!go(getX() + 2, getY(), a)) {
            changeLives(0);
            return;
        }
    }
    else {
        if (!go(getX() - 2, getY(), a)) {
            changeLives(0);
            return;
        }
    }
}
//if touching enemy, kill this object and the enemy and add score.
void Projectile::bonk() {
    Actor* a = nullptr;
    if (getWorld()->isTouchingEnemy(this, a)) {
        a->changeLives(0);
        a->doSomething();
        a->addScore(100);
        changeLives(0);
    }
}
//attempts to hurt peach and then kills itself
void PiranhaFireball::bonk() {
    getWorld()->getPeach()->ouch();
    changeLives(0);
}

int Mushroom::power() {return 1;}
int Flower::power() {return 2;}
int Star::power() {return 3;}

bool Enemy::isEnemy() {return true;}
//attempts to hurt peach, if peach has star power, it dies instead
void Enemy::bonk() {
    if (!getWorld()->getPeach()->ouch()) {
        addPoints();
        changeLives(0);
        getWorld()->playSound(SOUND_PLAYER_KICK);
    }
}
//do killable's do something and does goomba's and koopa's specified movements
void Enemy::doSomething() {
    Killable::doSomething();
    if (!isAlive()) {
        return;
    }
    if (!getWorld()->getPeach()->isAlive() || !isAlive()) {
        return;
    }
    bool startsRight = false;
    Actor* a;
    if (getDirection() == 0) {
        startsRight = true;
        if (!getWorld()->isBlocking(getX() + 1, getY(), a) && !getWorld()->offEdge(getX() + 1, getY())) {
            go(getX() + 1, getY(), a);
            return;
        }
        setDirection(180);
    }
    if (!getWorld()->isBlocking(getX() - 1, getY(), a) && !getWorld()->offEdge(getX() - 1, getY())) {
        go(getX() - 1, getY(), a);
        return;
    }
    setDirection(0);
    if (!getWorld()->isBlocking(getX() + 1, getY(), a) && !getWorld()->offEdge(getX() + 1, getY())) {
        go(getX() + 1, getY(), a);
        return;
    }
    setDirection(0);
    if (startsRight) {
        setDirection(180);
    }
}

void Koopa::doSomething() {
    Enemy::doSomething();
    if (!isAlive()) {
        getWorld()->addActor(new Shell(getWorld(), getX(), getY(), getDirection()));
    }
}
//turns direciton based on peaches movements and shoots mario if close
void Piranha::doSomething() {
    Killable::doSomething();
    if (isAlive()) {
        increaseAnimationNumber();
        Peach* peach = getWorld()->getPeach();
        if (abs(peach->getY() - getY()) > SPRITE_WIDTH * 1.5) {
            return;
        }
        if (peach->getX() > getX()) {
            setDirection(0);
        }
        else if (peach->getX() < getX()) {
            setDirection(180);
        }
        if (delay > 0) {
            delay--;
            return;
        }
        if (abs(peach->getX() - getX()) < 8 * SPRITE_WIDTH) {
            getWorld()->addActor(new PiranhaFireball(getWorld(), getX(), getY(), getDirection()));
            getWorld()->playSound(SOUND_PIRANHA_FIRE);
            delay = 40;
        }
    }
}

void Killable::addPoints() {getWorld()->increaseScore(m_points);}
//if is touched by peach, die and give peavh powerup pased on what item it is
void Item::doSomething() {
    Killable::doSomething();
    if (!isAlive()) {
        return;
    }
    Actor* a = nullptr;
    go(getX(), getY() - 2, a);
    if (getDirection() == 0) {
        if (!go(getX() + 2, getY(), a)) {
            setDirection(180);
        }
    }
    else {
        if (!go(getX() - 2, getY(), a)) {
            setDirection(0);
        }
    }
}
//give peach a powerup based on what item it is
void Item::bonk() {
    addPoints();
    Peach* p = getWorld()->getPeach();
    switch (power()) {
        case 1:
            p->setJump(true);
            if (p->getLives() == 1) {
                p->changeLives(2);
            }
            break;
        case 2:
            p->setFlower(true);
            if (p->getLives() == 1) {
                p->changeLives(2);
            }
            break;
        case 3:
            p->setStar(150);
            break;
        default:
            break;
    }
    getWorld()->playSound(SOUND_PLAYER_POWERUP);
    changeLives(-1);
}

//Peach setter and getter methods
int Peach::remain() {return remaining_jump;}
void Peach::setRemain(int n) {remaining_jump = n;};
bool Peach::jump() {return jump_power;}
void Peach::setJump(bool b) {jump_power = b;}
bool Peach::flower() {return flower_power;}
void Peach::setFlower(bool b) {flower_power = b;}
int Peach::star() {return star_power;}
void Peach::setStar(int n) {star_power = n;}

//peaches doSomething per tick
void Peach::doSomething() {
    if (!isAlive()) {
        return;
    }
    if (inv_cooldown > 0) {
        inv_cooldown--;
    }
    if (star_power > 0) {
        star_power--;
    }
    if (fire_cooldown > 0) {
        fire_cooldown--;
    }
    Actor* a = nullptr;
    getWorld()->isBlocking(getX(), getY(), a);
    while (a != nullptr && getWorld()->isBlocking(getX(), getY(), a)) {
        if (a->isKillable()) {
            a->doSomething();
        }
    }
    if (remain() > 0) {
        if (go(getX(), getY() + 4, a)) {
            setRemain(remain() - 1);
        }
        else {
            a->bonk();
            setRemain(0);
        }
    }
    else {
        go(getX(), getY() - 4, a);
    }
    int ch;
    a = nullptr;
    if (getWorld()->getKey(ch)) {
        switch (ch) {
            case KEY_PRESS_LEFT:
                setDirection(180);
                if (!go(getX()-4, getY(), a))
                    a->bonk();
                break;
            case KEY_PRESS_RIGHT:
                setDirection(0);
                if (!go(getX()+4, getY(), a))
                    a->bonk();
                break;
            case KEY_PRESS_UP:
                if (getWorld()->isBlocking(getX(), getY() - 1, a)) {
                    jump() ? setRemain(12) : setRemain(8);
                    getWorld()->playSound(SOUND_PLAYER_JUMP);
                }
                break;
            case KEY_PRESS_SPACE:
                if (flower()) {
                    if (fire_cooldown == 0) {
                        getWorld()->playSound(SOUND_PLAYER_FIRE);
                        fire_cooldown = 8;
                        if (getDirection() == 0) {
                            getWorld()->addActor(new PeachFireball(getWorld(), getX() + 2, getY(), 0));
                        }
                        else {
                            getWorld()->addActor(new PeachFireball(getWorld(), getX() - 2, getY(), 180));
                        }
                    }
                }
                break;
        }
    }
}
//tries to hurt peach, returns false if this would instead kill the enemy trying to hurt
bool Peach::ouch() {
    if (star() > 0) {
        return false;
    }
    if (inv_cooldown > 0) {
        return true;
    }
    changeLives(getLives() - 1);
    setJump(false);
    setFlower(false);
    inv_cooldown = 10;
    if (isAlive()) {
        getWorld()->playSound(SOUND_PLAYER_HURT);
    }
    return true;
}

void Ender::bonk() {
    changeLives(0);
}
//goes to the next value
void Flag::bonk() {
    Ender::bonk();
    if (!isAlive())
        addPoints();
        getWorld()->setGame(true);
}
//finishes the game
void Mario::bonk() {
    Ender::bonk();
    if (!isAlive())
        addPoints();
        getWorld()->won();
}
