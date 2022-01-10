#include <Arduino.h>

#ifndef PONG3D_H
#define PONG3D_H

#define BOUND_UP     (int32_t) height / 2
#define BOUND_DOWN  -(int32_t) height / 2
#define BOUND_RIGHT  (int32_t) width  / 2
#define BOUND_LEFT  -(int32_t) width  / 2
#define BOUND_FRONT  (int32_t) 0
#define BOUND_BACK   (int32_t) depth

// Auxiliary structs
struct velocity_t {
    int32_t vx;
    int32_t vy;
    int32_t vz;
    
    void setSpeed(int32_t speed) {
        double cur_speed = sqrt((double) vx*vx + vy*vy + vz*vz);
        
        // Get unitary vector
        double ux, uy, uz;
        if(cur_speed != 0.) {
            ux = vx / cur_speed;
            uy = vy / cur_speed;
            uz = vz / cur_speed;
        } else {
            ux = 0.;
            uy = 0.;
            uz = 1.;
        }
        
        vx = (int32_t) (ux * speed);
        vy = (int32_t) (uy * speed);
        vz = (int32_t) (uz * speed);
    }
    
    void decompose(int32_t speed, double alpha, double beta) {
        const double cos_b = cos(beta);

        vx = (int32_t) (speed * cos_b * cos(alpha));
        vy = (int32_t) (speed * cos_b * sin(alpha));
        vz = (int32_t) (speed * sin(beta));
    }
};

struct coord_t {
    int32_t x;
    int32_t y;
    int32_t z;
    
    void move(const velocity_t v) {
        x += v.vx;
        y += v.vy;
        z += v.vz;
    }
};

enum gamestate_t {
    GAME_WAITING = 0, GAME_READY = 1, GAME_PLAYING = 2, GAME_OVER = 3
};

enum gameevent_t {
    NO_EVENT, BALL_COLLISION, BALL_P1_REACH, BALL_P2_REACH
};



class Pong3D {
    public:
        Pong3D();
        
        // Game flow
        uint8_t playerConnected();
        void playerReady();
        gameevent_t updateGame();
        void resetGame();
        void endGame(uint8_t winner);

        // Game options (before start)
        void setDimensions(uint32_t width, uint32_t height, uint32_t depth);
        void setInitialBallSpeed(uint32_t speed);
        
        // Ball interaction
        void invertVelocityX();
        void invertVelocityY();
        void invertVelocityZ();
        void fixPosition();

        bool computePaddleCollision(int32_t px, int32_t py, int32_t pvx, int32_t pvy, uint8_t paddle);
        void resetPosition(uint8_t paddle);
        
        // Getters
        coord_t getBallPosition();
        velocity_t getBallVelocity();
        
        gamestate_t getGameState();
        
        uint8_t getScore1();
        uint8_t getScore2();
        
    private:
        // Playfield dimensions
        uint32_t width;
        uint32_t height;
        uint32_t depth;

        uint32_t initialBallSpeed;
        
        uint8_t playersConnected;
        uint8_t playersReady;

        coord_t    ballPosition;
        velocity_t ballVelocity;
        uint32_t   ballRadius;
        
        uint32_t paddleWidth;
        uint32_t paddleHeight;
        
        uint8_t score1;
        uint8_t score2;
        uint8_t target;
        
        gamestate_t gameState;
        
        // Game modify (in game)
        void setBallSpeed(uint32_t speed);
        void setBallVelocity(uint32_t vx, uint32_t vy, uint32_t vz);
        
        void score(uint8_t player);
        
};

#endif // PONG3D_H