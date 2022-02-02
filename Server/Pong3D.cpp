#include <Arduino.h>
#include "Pong3D.h"


// PUBLIC METHODS
Pong3D::Pong3D(){
    this->gameState = GAME_WAITING;

    this->playersConnected = 0;
    this->playersReady = 0;

    this->ballSpeedIncrement = 1;
    this->ballPosition = coord_t{0, 0, 0};
    this->ballVelocity = velocity_t{1, 1, -2};
    this->ballRadius   = 10;

    this->paddleWidth  = 500/5;
    this->paddleHeight = 300/5;

    this->target = 6;
    this->initialBallSpeed = 8;
}

// Game flow
int8_t Pong3D::playerConnected(){
    if(this->playersConnected == 2) {
        return -1;
    }
    int8_t id = this->playersConnected;
    this->playersConnected++;
    if(this->playersConnected == 2) {
        this->gameState = GAME_READY;
    }
    return id;
}

int8_t Pong3D::playerReady(){
    if(this->playersReady == 2) {
        // Already 2
        return -1;
    }
    int8_t readies = this->playersReady++;
    if(this->playersReady == 2) {
        // Start game
        this->score1 = 0;
        this->score2 = 0;
        resetPosition(0);
        this->gameState = GAME_PLAYING;
    }
    return readies;
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
        // Reach paddle 1
        event = BALL_P1_REACH;
    } else if(BOUND_BACK < ballPosition.z + r) {
        // Reach paddle2
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
    
    // Fix X
    if(ballPosition.x - r < BOUND_LEFT)
        ballPosition.x = 2*(BOUND_LEFT + r) - ballPosition.x;
    else if(ballPosition.x + r > BOUND_RIGHT)
        ballPosition.x = 2*(BOUND_RIGHT - r) - ballPosition.x;
    
    // Fix Y
    if(ballPosition.y - r < BOUND_DOWN)
        ballPosition.y = 2*(BOUND_DOWN + r) - ballPosition.y;
    else if(ballPosition.y + r > BOUND_UP)
        ballPosition.y = 2*(BOUND_UP - r) - ballPosition.y;
    
    // Fix Z
    if(ballPosition.z - r < BOUND_FRONT)
        ballPosition.z = 2*(BOUND_FRONT + r) - ballPosition.z;
    else if(ballPosition.z + r > BOUND_BACK)
        ballPosition.z = 2*(BOUND_BACK - r) - ballPosition.z;
}

int8_t Pong3D::computePaddleCollision(int32_t px, int32_t py, int32_t pvx, int32_t pvy, uint8_t paddle){
    // Get paddle borders
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
        ballSpeed += ballSpeedIncrement;

        int32_t new_vx = ballVelocity.vx + pvx;
        int32_t new_vy = ballVelocity.vy + pvy;
        ballVelocity.deduceZ(ballSpeed, new_vx, new_vy);
        return -1;
    } else {
        int8_t scr_paddle = paddle ? 0 : 1; // player who scores (invert)
        score(scr_paddle);
        resetPosition(paddle); // towards player who lost point
        return scr_paddle;
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

uint8_t Pong3D::getPlayersConnected() {
    return this->playersConnected;
}

uint8_t Pong3D::getPlayersReady() {
    return this->playersReady;
}

uint8_t Pong3D::getScore1() {
    return this->score1;
}

uint8_t Pong3D::getScore2() {
    return this->score2;
}

uint8_t Pong3D::getTarget() {
    return this->target;
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
    this->ballVelocity = paddle ? velocity_t{1, 1, 2} : velocity_t{1, 1, -2};
    this->ballSpeed = this->initialBallSpeed;
    this->ballVelocity.setSpeed(this->initialBallSpeed);
}
