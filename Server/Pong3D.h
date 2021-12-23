#include <Arduino.h>

struct coord_t;
struct velocity_t;

struct coord_t {
    int32_t x;
    int32_t y;
    int32_t z;
    
    void update(const velocity_t v) {
        x += v.dx;
        y += v.dy;
        z += v.dz;
    }
};

struct velocity_t {
    int32_t vx;
    int32_t vy;
    int32_t vz;
    
    void decompose(int32_t speed, double alpha, double beta) {
        const double cos_b = ;

        vx = (int32_t) (speed * cos(beta) * cos(alpha))
        vy = (int32_t) (speed * cos(beta) * sin(alpha))
        vz = (int32_t) (speed * sin(beta));
    }
};

typedef enum gamestate_t {
    GAME_WAITING, GAME_READY, GAME_PLAYING, GAME_OVER
};

class Pong3D {
    public:
        Pong3D();
        
        // Game flow
        void newPlayer();
        void updateGame();
        void resetGame();
        void endGame(uint8_t winner);

        // Game options (before start)
        void setDimensions(uint32_t width, uint32_t height, uint32_t depth);
        
        void computePaddleCollision(velocity_t paddle_v);
        
        // Getters
        coord_t getBallPosition();
        velocity_t getBallVelocity();
        
    private:
        coord_t ballPosition;
        velocity_t ballVelocity;
        
        double alpha;
        double beta;
        
        // Game modify (in game)
        void setBallSpeed(uint32_t speed);
        void setBallVelocity(uint32_t vx, uint32_t vy, uint32_t vz);
        void setBallAngles(double alpha, double beta);
        
        void invertBeta();

};
