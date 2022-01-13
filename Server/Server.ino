#define SHARED
/* Choose between:
  - LAB
  - HOME_ROUTER
  - SHARED
*/

#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Arduino.h>

#include "Keypad.h"
#include "LCD.h"
#include "Pong3D.h"
#include "Timer.h"


byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };

#if defined(LAB)
    // Lab connection
    IPAddress ip(172,28,2,163);
    IPAddress gw(172,28,2,1);
#elif defined(HOME_ROUTER)
    // Connected to my router
    IPAddress ip(192,168,0,64);
    IPAddress gw(192,168,0,1);
#elif defined(SHARED)
    // Ethernet wired to my PC with shared connection
    IPAddress ip(10,42,0,2);
    IPAddress gw(10,42,0,1);
#endif

// DNS information
IPAddress mydns(8, 8, 8, 8);

// Broker information
const char broker_name[] = "tom.uib.es"; //"labauto.sytes.net";
const unsigned int broker_port = 1883;   // 8080;
const char client_id[] = "svv459";       // Update

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

// Publish topics
#define TOPIC_BALL_X "/pong3d/ball/x"
#define TOPIC_BALL_Y "/pong3d/ball/y"
#define TOPIC_BALL_Z "/pong3d/ball/z"

#define TOPIC_P1_REQ "/pong3d/paddle1/request"
#define TOPIC_P2_REQ "/pong3d/paddle2/request"

#define TOPIC_PF_WIDTH  "/pong3d/playfield/width"
#define TOPIC_PF_HEIGHT "/pong3d/playfield/height"
#define TOPIC_PF_DEPTH  "/pong3d/playfield/depth"

#define TOPIC_ASSIGN "/pong3d/assign"

// Subscribe topics
#define TOPIC_RESPONSE  "/pong3d/+/response/#"
#define TOPIC_CONNECTED "/pong3d/connected"
#define TOPIC_READY     "/pong3d/ready"

// Paddle response struct
struct response_t {
    volatile int32_t x, y, vx, vy;
    volatile bool    x_set, y_set, vx_set, vy_set;
    
    bool checkAll() {
        return (x_set && y_set && vx_set && vy_set);
    }
    
    void setAllFalse() {
        x_set  = false;
        y_set  = false;
        vx_set = false;
        vy_set = false;
    }
} response;

// Pong3D logic
Pong3D p3d;

// Timers
#define FPS     20
#define TIME_MS 1000 / FPS
#define TIMER   1
#define TIMERV  TIMER1_COMPA_vect

Timer timer(TIMER);

volatile bool timer_flag = false;
ISR(TIMERV) {
    timer_flag = true;
}

// Auxiliary variables
#define MSG_SIZE 10
uint8_t* msg;

bool pub_ok;
bool sub_ok;
gameevent_t game_event;
coord_t coords;

/******************************************************************************/
/** SETUP *********************************************************************/
/******************************************************************************/

void setup() {
    Serial.begin(115200);

    Ethernet.begin(mac, ip, mydns, gw);

    mqttClient.setServer(broker_name, broker_port);
    mqttClient.setCallback(callback);
    
    delay(1500);
    
    // Auxiliary variables initialization
    msg = (uint8_t *) malloc(MSG_SIZE);
    response.setAllFalse();

    // Timer
    timer.setTime(TIME_MS);
    timer.load();
    
    // Pong3D
    p3d.setDimensions(500,300,800);
    p3d.setInitialBallSpeed(8);
}


/******************************************************************************/
/** LOOP **********************************************************************/
/******************************************************************************/

void loop() {

    if (!mqttClient.connected()) reconnect();
    
    while(p3d.getGameState() == GAME_WAITING) {
        // Esperar a que se conecten los jugadores (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }
    
    while(p3d.getGameState() == GAME_READY) {
        // Esperar a que los jugadores den su visto bueno (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }
    
    while(p3d.getGameState() == GAME_PLAYING) {
        // En función del último evento de la actualización anterior
        switch(game_event) {
            case NO_EVENT: case BALL_COLLISION:
                if(timer_flag) {
                    game_event = p3d.updateGame();
                    
                    switch(game_event) {
                        // Movimiento natural de la pelota
                        case NO_EVENT: case BALL_COLLISION:
                            coords = p3d.getBallPosition();

                            memcpy(msg, &coords.x, 4);
                            pub_ok  = mqttClient.publish(TOPIC_BALL_X, msg, 4);

                            memcpy(msg, &coords.y, 4);
                            pub_ok &= mqttClient.publish(TOPIC_BALL_Y, msg, 4);

                            memcpy(msg, &coords.z, 4);
                            pub_ok &= mqttClient.publish(TOPIC_BALL_Z, msg, 4);
                            
                            break;
                            
                        // Solicitar coordenadas de la paleta
                        case BALL_P1_REACH:
                            pub_ok = mqttClient.publish(TOPIC_P1_REQ, msg, 0);
                            break;
                        case BALL_P2_REACH:
                            pub_ok = mqttClient.publish(TOPIC_P2_REQ, msg, 0);
                            break;
                    }
                    
                    if (pub_ok) {
                        Serial.println("All messages Published");
                    } else {
                        Serial.println("Some messages couldn't be Published");
                        if(game_event == BALL_P1_REACH ||
                                game_event == BALL_P2_REACH) {
                            p3d.invertVelocityZ(); // Count as hit
                        }
                    }
                        
                    timer_flag = false;
                }
                break;
            case BALL_P1_REACH: case BALL_P2_REACH:
                // Esperar a recibir la información de las paletas
                break;
        }

        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }
    
    while(p3d.getGameState() == GAME_OVER) {
        // Esperar a solicitud para repetir partida (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }

}


/******************************************************************************/
/** AUXILIARY FUNCTIONS *******************************************************/
/******************************************************************************/

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received from [ ");
    Serial.print(topic);
    Serial.println(" ]");
    
    // Tokenize topic string
    char* token;
    char* params[5];
    uint8_t i = 0;
    token = strtok(topic, "/");
    while(token) {
        params[i++] = token;
        token = strtok(NULL, "/");
    }
        
    // Se da por hecho que params[0] == "pong3d"
    
    if(
        strcmp(params[1], "connected") == 0 && 
        p3d.getGameState() == GAME_WAITING
    ) {
        uint8_t id = p3d.playerConnected();
        pub_ok = mqttClient.publish(TOPIC_ASSIGN, &id, 1);
        Serial.println("Jugador conectado");

    } else if(
        strcmp(params[1], "ready") == 0 &&
        p3d.getGameState() == GAME_READY
    ) {
        p3d.playerReady();
        Serial.print("Jugador listo: ");
        Serial.println((uint8_t) payload[0]);

    } else if(
        strcmp(params[2], "response") == 0 &&
        p3d.getGameState() == GAME_PLAYING
    ) {
        // Paddle response after request
        int32_t value;
        memcpy(&value, payload, length);
        
        uint8_t paddle = strcmp(params[1], "paddle1") == 0 ? 0 : 1;
        
        if(strcmp(params[3], "x") == 0) {
            response.x = value;
            response.x_set = true;
        } else if(strcmp(params[3], "y") == 0) {
            response.y = value;
            response.y_set = true;
        } else if(strcmp(params[3], "vx") == 0) {
            response.vx = value;
            response.vx_set = true;
        } else if(strcmp(params[3], "vy") == 0) {
            response.vy = value;
            response.vy_set = true;
        }
        
        // Esperar a que se hayan recibido todos los valores de la paleta
        if(response.checkAll()) {
            if(!p3d.computePaddleCollision(
                response.x, response.y, response.vx, response.vy, paddle)
            ) {
                Serial.print("Score: ");
                Serial.print(p3d.getScore1());
                Serial.print(" / ");
                Serial.println(p3d.getScore2());
            }
            response.setAllFalse();
            game_event = NO_EVENT;
     }
        
    } else if(
        strcmp(params[1], "ready") == 0 &&
        p3d.getGameState() == GAME_OVER
    ) {
        p3d.resetGame();
    }
}

void reconnect() {
    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Serial.println("Attempting MQTT connection ...");

        // Attempt to connect
        if (mqttClient.connect(client_id)) {
            Serial.println("connected");
            sub_ok = true;
            
            if (mqttClient.subscribe(TOPIC_RESPONSE)) {
                Serial.print("Suscrito al tópico: ");
                Serial.println(TOPIC_RESPONSE);
            } else sub_ok = false;

            if (mqttClient.subscribe(TOPIC_CONNECTED)) {
                Serial.print("Suscrito al tópico: ");
                Serial.println(TOPIC_CONNECTED);
            } else sub_ok = false;

            if (mqttClient.subscribe(TOPIC_READY)) {
                Serial.print("Suscrito al tópico: ");
                Serial.println(TOPIC_READY);
            } else sub_ok = false;
            
            if(sub_ok) {
                if(p3d.getGameState() == GAME_PLAYING &&
                    (game_event == BALL_P1_REACH ||
                     game_event == BALL_P2_REACH)) {
                    
                    // Revierte a un estado operativo
                    // si el juego se ha desconectado abruptamente
                    uint8_t paddle = game_event == BALL_P1_REACH ? 0 : 1;
                    p3d.resetPosition(paddle);
                    game_event = NO_EVENT;
                }
            } else {
                Serial.println("No se pudo suscribir a todos los tópicos");
                Serial.println("  Comprueba la sintaxis de las cadenas");
            }
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}
