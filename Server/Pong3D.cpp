#include <Arduino.h>
#include "Pong3D.h"

// PUBLIC METHODS
Pong3D::Pong3D(){
    this->gameState = GAME_WAITING;
    this->playersConnected = 0;
    this->playersReady = 0;
    this->ballPosition = coord_t{0, 0, 0};
    this->ballVelocity = velocity_t{1, 1, 1};
    this->ballRadius   = 10;
    this->paddleWidth  = 500/5;
    this->paddleHeight = 300/5;
    this->target       = 6;
    this->initialBallSpeed = 8;
}

// Game flow
uint8_t Pong3D::playerConnected(){
    uint8_t id = this->playersConnected;
    this->playersConnected++;
    if(this->playersConnected == 2) {
        this->gameState = GAME_READY;
    }
    return id;
}

void Pong3D::playerReady(){
    this->playersReady++;
    if(this->playersReady == 2) {
        // Start game
        this->score1 = 0;
        this->score2 = 0;
        resetPosition(0);
        this->gameState = GAME_PLAYING;
    }
}

gameevent_t Pong3D::updateGame(){
    const int32_t r = (int32_t) ballRadius;
    gameevent_t event = NO_EVENT;

    ballPosition.move(ballVelocity);
    
    // Invert on collision
    if(ballPosition.x - r < BOUND_LEFT || BOUND_RIGHT < ballPosition.x + r) {
        invertVelocityX();
        event = BALL_COLLISION;
    }
    if(ballPosition.y - r < BOUND_DOWN || BOUND_UP < ballPosition.y + r) {
        invertVelocityY();
        event = BALL_COLLISION;
    }

    if(ballPosition.z - r < BOUND_FRONT) {
        event = BALL_P1_REACH;
    } else if(BOUND_BACK < ballPosition.z + r) {
        event = BALL_P2_REACH;
    }

    fixPosition(); // take ball off the wall

    return event;
}

void Pong3D::resetGame(){
    this->playersReady = 0;
    this->gameState = GAME_READY;
}

void Pong3D::endGame(uint8_t winner){
    this->gameState = GAME_OVER;
}

// Game options (before start)
void Pong3D::setDimensions(uint32_t width, uint32_t height, uint32_t depth){
    this->width  = width;
    this->height = height;
    this->depth  = depth;
}

void Pong3D::setInitialBallSpeed(uint32_t speed){
    this->initialBallSpeed = speed;
}

// Ball interaction
void Pong3D::invertVelocityX(){
    this->ballVelocity.vx *= -1;
}

void Pong3D::invertVelocityY(){
    this->ballVelocity.vy *= -1;
}

void Pong3D::invertVelocityZ(){
    this->ballVelocity.vz *= -1;
}

void Pong3D::fixPosition(){
    const int32_t r = (int32_t) ballRadius;
    if(ballPosition.x - r < BOUND_LEFT)
        ballPosition.x = 2*BOUND_LEFT - ballPosition.x;
    else if(ballPosition.x + r> BOUND_RIGHT)
        ballPosition.x = 2*BOUND_RIGHT - ballPosition.x;
    
    if(ballPosition.y - r < BOUND_DOWN)
        ballPosition.y = 2*BOUND_DOWN - ballPosition.y;
    else if(ballPosition.y + r > BOUND_UP)
        ballPosition.y = 2*BOUND_UP - ballPosition.y;
    
    if(ballPosition.z - r < BOUND_FRONT)
        ballPosition.z = 2*BOUND_FRONT - ballPosition.z;
    else if(ballPosition.z + r > BOUND_BACK)
        ballPosition.z = 2*BOUND_BACK - ballPosition.z;
}

bool Pong3D::computePaddleCollision(int32_t px, int32_t py, int32_t pvx, int32_t pvy, uint8_t paddle){
    const int32_t paddleUp    = py + (int32_t) paddleHeight / 2;
    const int32_t paddleDown  = py - (int32_t) paddleHeight / 2;
    const int32_t paddleRight = px + (int32_t) paddleWidth  / 2;
    const int32_t paddleLeft  = px - (int32_t) paddleWidth  / 2;
    const int32_t r = (int32_t) ballRadius;
    
    if(        paddleLeft  < ballPosition.x + r
            && paddleRight > ballPosition.x - r
            && paddleDown  < ballPosition.y + r
            && paddleUp    > ballPosition.y - r) {
        invertVelocityZ();
        return true;
    } else {
        score(paddle ? 0 : 1);
        resetPosition(paddle);
        return false;
    }
}

// Getters
coord_t Pong3D::getBallPosition(){
    return this->ballPosition;
}

velocity_t Pong3D::getBallVelocity(){
    return this->ballVelocity;
}

gamestate_t Pong3D::getGameState(){
    return this->gameState;
}

uint8_t Pong3D::getScore1() {
    return this->score1;
}

uint8_t Pong3D::getScore2() {
    return this->score2;
}

// PRIVATE METHODS

// Game modify (in game)
void Pong3D::setBallSpeed(uint32_t speed){
    this->ballVelocity.setSpeed(speed);
}

void Pong3D::setBallVelocity(uint32_t vx, uint32_t vy, uint32_t vz){
    this->ballVelocity.vx = vx;
    this->ballVelocity.vy = vy;
    this->ballVelocity.vz = vz;
}

void Pong3D::score(uint8_t player) {
    switch(player) {
        case 0: score1++; break;
        case 1: score2++; break;
    }

    if(score1 == target || score2 == target) endGame(player);
}

void Pong3D::resetPosition(uint8_t paddle) {
    this->ballPosition = coord_t{0, 0, (BOUND_FRONT + BOUND_BACK)/2};
    this->ballVelocity = paddle ? velocity_t{1, 1, -1} : velocity_t{1, 1, 1};
    this->ballVelocity.setSpeed(this->initialBallSpeed);
}
