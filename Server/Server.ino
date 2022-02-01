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

#include "Menu.h"

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
const char client_id[] = "svv";          // Update

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void lcdPrintScore();

// Publish topics
#define TOPIC_BALL_X "/pong3d/ball/x"
#define TOPIC_BALL_Y "/pong3d/ball/y"
#define TOPIC_BALL_Z "/pong3d/ball/z"

#define TOPIC_P1_REQ "/pong3d/paddle1/request"
#define TOPIC_P2_REQ "/pong3d/paddle2/request"

#define TOPIC_PF_WIDTH  "/pong3d/playfield/width"
#define TOPIC_PF_HEIGHT "/pong3d/playfield/height"
#define TOPIC_PF_DEPTH  "/pong3d/playfield/depth"

#define TOPIC_ASSIGN "/pong3d/player_id"

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

#define STR_SIZE 50
char* str;

bool pub_ok;
bool sub_ok;
volatile gameevent_t game_event;
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
    str = (char *) malloc(STR_SIZE);
    response.setAllFalse();

    // Timer
    timer.setTime(TIME_MS);
    timer.load();
    
    // Pong3D
    p3d.setDimensions(
        currentValues.width, currentValues.height, currentValues.depth
    );
    p3d.setInitialBallSpeed(currentValues.speed);
    
    // From Menu.cpp
    valueIntroduced = 0;
    oks = 0;
    p_aux = NULL;
    
    // Keypad
	keypad.initKeypad(
		KEYPAD_COL0_PIN, KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN, 
		KEYPAD_ROW0_PIN, KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN
	);

    // LCD
	lcd.init();
    lcd.print(
        "     Pong3D     "
        "     Server     "
    );
    delay(2000);
}


/******************************************************************************/
/** LOOP **********************************************************************/
/******************************************************************************/

void loop() {

    if (!mqttClient.connected()) reconnect();
    
    /**************************************************************************/
    /** STATE GAME_WAITING ****************************************************/
    /**************************************************************************/
    if(p3d.getGameState() == GAME_WAITING) {
        lcd.clear();
        lcd.print(
            "  Waiting for   "
            "   players...   "
        );
        delay(1000);
    } while(p3d.getGameState() == GAME_WAITING) {
        // Esperar a que se conecten los jugadores (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
        
        // Keyboard
        //key = keypad.readKeypad(BLOCK); // Polling
	    //action(key);
    }
    
    /**************************************************************************/
    /** STATE GAME_READY ******************************************************/
    /**************************************************************************/
    if(p3d.getGameState() == GAME_READY) {
        lcd.clear();
        lcd.print(
            "   Game ready   "
            "                "
        );
    } while(p3d.getGameState() == GAME_READY) {
        // Esperar a que los jugadores den su visto bueno (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }
    
    /**************************************************************************/
    /** STATE GAME_PLAYING ****************************************************/
    /**************************************************************************/
    if(p3d.getGameState() == GAME_PLAYING) {
        lcd.clear();
        lcd.print(
            "     Start!     "
            "                "
        );
        delay(1000);
        lcdPrintScore();
    } while(p3d.getGameState() == GAME_PLAYING) {
        // En función del último evento de la actualización anterior
        if(game_event != BALL_P1_REACH && game_event != BALL_P2_REACH) {
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
                    //Serial.println("All messages Published");
                } else {
                    Serial.println("Some messages couldn't be Published");
                    if(game_event == BALL_P1_REACH ||
                            game_event == BALL_P2_REACH) {
                        p3d.invertVelocityZ(); // Count as hit
                    }
                }
                    
                timer_flag = false;
            }
        }

        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }
    
    /**************************************************************************/
    /** STATE GAME_OVER *******************************************************/
    /**************************************************************************/
    if(p3d.getGameState() == GAME_OVER) {
        delay(500);
        lcd.clear();
        lcdPrintWinner();
    } while(p3d.getGameState() == GAME_OVER) {
        // Esperar a solicitud para repetir partida (callback)
        mqttClient.loop();
        if(!mqttClient.connected()) break;
    }

}


/******************************************************************************/
/** AUXILIARY FUNCTIONS *******************************************************/
/******************************************************************************/

void callback(char *topic, byte *payload, unsigned int length) {
    //Serial.print("Message received from [ ");
    //Serial.print(topic);
    //Serial.println(" ]");
    
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
    
    if( // Topic is /pong3d/connected
        strcmp(params[1], "connected") == 0 && 
        p3d.getGameState() == GAME_WAITING
    ) {
        char* id = (char *) malloc(2);
        p3d.playerConnected() == 0 ? strcpy(id, "1") : strcpy(id, "2");
        pub_ok = mqttClient.publish(TOPIC_ASSIGN, id);
        
        sprintf(str,
            "   Player %s     "
            "   connected    ",
            id
        );
        lcd.clear();
        lcd.print(str);

    } else if( // Topic is /pong3d/ready
        strcmp(params[1], "ready") == 0 &&
        p3d.getGameState() == GAME_READY
    ) {
        p3d.playerReady();
        sprintf(str,
            "   Player %c     "
            "     ready      ",
            payload[0]
        );
        lcd.clear();
        lcd.print(str);

    } else if( // Topic is /pong3d/+/response
        strcmp(params[2], "response") == 0 &&
        (game_event == BALL_P1_REACH || game_event == BALL_P2_REACH) &&
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
                lcdPrintScore();
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
        lcd.clear();
        lcd.print("Attempting MQTT connection...");

        // Attempt to connect
        if (mqttClient.connect(client_id)) {
            lcd.clear();
            lcd.print(
                "   Connected    "
                "                "
            );
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
            
            delay(1000);
            lcd.clear();
        } else {
            lcd.clear();
            sprintf(str, "failed, rc=%d", mqttClient.state());
            lcd.print(str);
            lcd.moveCursor(2,1);
            lcd.print(" try again in 2s");
            delay(2000);
        }
    }
}

void lcdPrintScore() {
    lcd.clear();
    sprintf(str, "Score: %d / %d", p3d.getScore1(), p3d.getScore2());
    lcd.print(str);
    
    lcd.moveCursor(2,1);
    sprintf(str, " Goal: %d", p3d.getTarget());
    lcd.print(str);

    lcd.moveCursor(2,17);
}

void lcdPrintWinner() {
    lcd.clear();
    lcd.print("   Game Over!   ");
    
    lcd.moveCursor(2,1);
    uint8_t number = p3d.getScore1() == p3d.getTarget() ? 1 : 2;
    sprintf(str, " Player %d wins", number);
    lcd.print(str);

    lcd.moveCursor(2,17);
   
}