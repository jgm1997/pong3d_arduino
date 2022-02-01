#include <stdio.h>
#include <stdlib.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Arduino.h>
//#include "lib/Keypad.h"
//#include "lib/LCD.h"
#include "Joystick.h"

#define FORMAT_0 "/pong3d/%s/%s"
#define FORMAT_1 "/pong3d/%s/response/%s"
#define FORMAT_SUB "Suscrito al topic \"%s\""

// Joystick definition
#define JS_X A14
#define JS_Y A15
#define JS_SW 47

Joystick joy(JS_X, JS_Y, JS_SW);

char *player_id, *aux, *aux_topic;
uint8_t *msg;
volatile bool timer_flag = false, conn = false;
bool pub_ok;
bool player_assigned = false;

// Valores a actuaizar y cambiar por cada Arduino
byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

IPAddress ip(192, 168, 0, 20);
IPAddress gw(192, 168, 0, 1);

// IPAddress ip(172, 28, 2, 161);
// IPAddress gw(172, 28, 2, 1);

IPAddress mydns(8, 8, 8, 8);

const char broker_name[] = "tom.uib.es"; //"labauto.sytes.net";
const unsigned int broker_port = 1883;   // 8080;
const char client_id[] = "jgm";          // Update

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

ISR(TIMER1_COMPA_vect)
{
    timer_flag = true;
}

#define sensibilidad 0.02

int32_t pf_width = 500;
int32_t pf_height = 300;

#define BOUND_UP pf_height / 2
#define BOUND_DOWN -pf_height / 2
#define BOUND_LEFT -pf_width / 2
#define BOUND_RIGHT pf_width / 2

#define PADDLE_WIDTH pf_width / 5
#define PADDLE_HEIGHT pf_height / 5

#define PADDLE_HALF_WIDTH PADDLE_WIDTH / 2
#define PADDLE_HALF_HEIGHT PADDLE_HEIGHT / 2

int32_t x = 0;
int32_t y = 0;
int32_t sign = 0;

void setup()
{
    Serial.begin(115200);

    msg = (uint8_t *)malloc(8);
    player_id = (char *)malloc(20);
    aux = (char *)malloc(50);
    aux_topic = (char *)malloc(70);
    Ethernet.begin(mac, ip, mydns, gw);

    mqttClient.setServer(broker_name, broker_port);
    mqttClient.setCallback(callback);
    /*
        // Configuración del timer
        noInterrupts();
        TCCR1A = 0;
        TCCR1B = 0;
        // modo ctc
        TCCR1B |= (1 << WGM12); // posam un 1 a nes bit wgmn2 del registre tccr1b

        OCR1A = 62500; // 1 segundo

        TIMSK1 |= (1 << OCIE1A); // output compare match(OCIEnX)

        TCCR1B |= (1 << CS12); // pre-escalado 1:256
        interrupts();
    */
    joy.init();

    delay(1500);
}

void loop()
{

    // para el 2º jugador, publicar en topic pong3d/paddle2
    while (true)
    {
        if (!mqttClient.connected())
        {
            reconnect();
        }
        if (!conn)
        {
            pub_ok = mqttClient.publish("/pong3d/connected", "");
            if (pub_ok)
            {
                Serial.println("Player connected");
                conn = true;
            }
            else
            {
                Serial.println("Player not connected");
            }
        }
        if (player_assigned)
        {
            Serial.println("Jugador asignado");
            x += (int32_t)(joy.PosX() * sensibilidad * sign);
            if (x + PADDLE_HALF_WIDTH > BOUND_RIGHT)
                x = BOUND_RIGHT - PADDLE_HALF_WIDTH;
            else if (x - PADDLE_HALF_WIDTH < BOUND_LEFT)
                x = BOUND_LEFT + PADDLE_HALF_WIDTH;
            memcpy(msg, &x, 4);
            sprintf(aux, FORMAT_0, player_id, "x");
            pub_ok = mqttClient.publish(aux, msg, 4);
            if (pub_ok)
            {
                // Serial.println("\nX = " + String(joy.PosX()));
            }
            else
            {
                Serial.println("\nWaiting for player 1...");
            }

            y += (int32_t)(joy.PosY() * sensibilidad);
            if (y + PADDLE_HALF_HEIGHT > BOUND_UP)
                y = BOUND_UP - PADDLE_HALF_HEIGHT;
            else if (y - PADDLE_HALF_HEIGHT < BOUND_DOWN)
                y = BOUND_DOWN + PADDLE_HALF_HEIGHT;
            memcpy(msg, &y, 4);
            sprintf(aux, FORMAT_0, player_id, "y");
            pub_ok = mqttClient.publish(aux, msg, 4);
            if (pub_ok)
            {
                // Serial.println("\nY = " + String(joy.PosY()));
            }
            else
            {
                Serial.println("\nWaiting for player 1...");
            }

            if (joy.swStatus())
            {
                pub_ok = mqttClient.publish("/pong3d/ready", "");
                if (pub_ok)
                {
                    Serial.println("\nSW = " + String(joy.swStatus()));
                }
                else
                {
                    Serial.println("\nWaiting for player 1...");
                }
            }

            // Call regularly to process incoming messages
            // and maintain connection with the server

            delay(50);
        }
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();
    }
    delay(1000);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // Tokenize topic string
    char *token, *aux;
    char *params[5];
    uint8_t i = 0;
    token = strtok(topic, "/");
    while (token)
    {
        params[i++] = token;
        Serial.println(params[i]);
        token = strtok(NULL, "/");
    }

    if (strcmp(params[1], "player_id") == 0 && !player_assigned)
    {

        for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
        }

        if (payload[0] == '2')
        {
            sign = -1;
        }
        else
        {
            sign = 1;
        }

        // player_id += "paddle" + (char)payload[0];
        sprintf(player_id, "paddle%c", payload[0]);
        sprintf(aux_topic, FORMAT_SUB, player_id);
        Serial.println(aux_topic);

        sprintf(aux, FORMAT_0, player_id, "request");
        if (mqttClient.subscribe(aux))
        {
            sprintf(aux_topic, FORMAT_SUB, aux);
            Serial.println(aux_topic);
        }
        else
        {
            Serial.println("No se ha completado la suscripción a request");
        }

        player_assigned = true;

        // Salto de línea
        // Serial.println();
    }
    else if (strcmp(params[1], player_id) == 0)
    {
        if (strcmp(params[2], "x") == 0)
        {
            // switch
            for (int i = 0; i < length; i++)
            {
                // Casting a char de cada valor para comprobación en monitor serie
                Serial.print((char)payload[i]);
            }
        }

        if (strcmp(params[2], "y") == 0)
        {
            // switch
            for (int i = 0; i < length; i++)
            {
                // Casting a char de cada valor para comprobación en monitor serie
                Serial.print((char)payload[i]);
            }
        }
        if (strcmp(params[2], "request"))
        {
            memcpy(msg, &x, 4);
            sprintf(aux, FORMAT_1, player_id, "x");
            pub_ok = mqttClient.publish(aux, msg, 4);
            if (pub_ok)
            {
                // Serial.println("\nX = " + String(joy.PosX()));
            }
            else
            {
                Serial.println("\nWaiting for player 1...");
            }

            memcpy(msg, &y, 4);
            sprintf(aux, FORMAT_1, player_id, "y");
            pub_ok = mqttClient.publish(aux, msg, 4);
            if (pub_ok)
            {
                // Serial.println("\nY = " + String(joy.PosY()));
            }
            else
            {
                Serial.println("\nWaiting for player 1...");
            }
        }
        // switch
        /*for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
        }*/

        // Salto de línea
        // Serial.println();
    }
    else if (strcmp(params[1], "ready") == 0)
    {
        for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
        }
    }
    else
    {
        Serial.println("Revisa los campos del topic. No reconozco lo que se me ha enviado.");
        delay(2000);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.println("Attempting MQTT connection ...");

        // Attempt to connect
        if (mqttClient.connect(client_id))
        {
            Serial.println("connected");

            if (mqttClient.subscribe("/pong3d/player_id"))
            {
                Serial.println("Suscrito al topico: /pong3d/player_id");
            }
            /*

            if (mqttClient.subscribe("/pong3d/paddle1/y"))
            {
                Serial.println("Suscrito al topico: /pong3d/player/y");
            }

            if (mqttClient.subscribe("/pong3d/ready"))
            {
                Serial.println("Suscrito al topico: /pong3d/player/sw");
            }


            if (mqttClient.subscribe("/pong3d/player_id"))
            {
                Serial.println("Suscrito al topico: /pong3d/player_id");
            }
            */
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}
