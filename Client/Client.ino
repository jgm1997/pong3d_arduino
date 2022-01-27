#include <stdlib.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Arduino.h>
//#include "lib/Keypad.h"
//#include "lib/LCD.h"
#include "Joystick.h"

// Joystick definition
#define JS_X A14
#define JS_Y A15
#define JS_SW 47

Joystick joy(JS_X, JS_Y, JS_SW);

String sTemp = "", player_id = "/pong3d/";
uint8_t *msg;
volatile bool timer_flag = false, conn = false;
bool pub_ok;

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

int32_t x = 0;
int32_t y = 0;

void setup()
{
    Serial.begin(115200);

    msg = (uint8_t *)malloc(8);
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
            if (pub_ok) {
                Serial.println("Player connected");
                conn = true;
            }
            else {
                Serial.println("Player not connected");
            }
        }

        x += (int32_t)(joy.PosX() * sensibilidad);
        memcpy(msg, &x, 4);
        pub_ok = mqttClient.publish((player_id + "/x").c_str(), msg, 4);
        if (pub_ok)
        {
            Serial.println("\nX = " + String(joy.PosX()));
        }
        else
        {
            Serial.println("\nWaiting for player 1...");
        }

        y += (int32_t)(joy.PosY() * sensibilidad);
        memcpy(msg, &y, 4);
        pub_ok = mqttClient.publish((player_id + "/y").c_str(), msg, 4);
        if (pub_ok)
        {
            Serial.println("\nY = " + String(joy.PosY()));
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
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();
        mqttClient.loop();

        delay(50);
    }
    delay(1000);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (strcmp(topic, "/pong3d/player_id") == 0)
    {

        for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
            player_id += "paddle" + (char)payload[i];
        }

        // Salto de línea
        // Serial.println();
    }
    else if (strcmp(topic, (player_id + "/x").c_str()) == 0)
    {
        // switch
        for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
        }

        // Salto de línea
        // Serial.println();
    }
    else if (strcmp(topic, (player_id + "/y").c_str()) == 0)
    {
        // switch
        for (int i = 0; i < length; i++)
        {
            // Casting a char de cada valor para comprobación en monitor serie
            Serial.print((char)payload[i]);
        }
    }
    else if (strcmp(topic, (player_id + "/request").c_str()) == 0)
    {

        memcpy(msg, &x, 4);
        pub_ok = mqttClient.publish((player_id + "/response/x").c_str(), msg, 4);
        if (pub_ok)
        {
            Serial.println("\nX = " + String(joy.PosX()));
        }
        else
        {
            Serial.println("\nWaiting for player 1...");
        }

        memcpy(msg, &y, 4);
        pub_ok = mqttClient.publish((player_id + "/response/y").c_str(), msg, 4);
        if (pub_ok)
        {
            Serial.println("\nY = " + String(joy.PosY()));
        }
        else
        {
            Serial.println("\nWaiting for player 1...");
        }
    }
    else if (strcmp(topic, "/pong3d/ready") == 0)
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
            /*
            if (mqttClient.subscribe("/pong3d/paddle1"))
            {
                Serial.println("Suscrito al topico: /pong3d/player");
            }

            if (mqttClient.subscribe("/pong3d/paddle1/x"))
            {
                Serial.println("Suscrito al topico: /pong3d/player/x");
            }

            if (mqttClient.subscribe("/pong3d/paddle1/y"))
            {
                Serial.println("Suscrito al topico: /pong3d/player/y");
            }

            if (mqttClient.subscribe("/pong3d/ready"))
            {
                Serial.println("Suscrito al topico: /pong3d/player/sw");
            }
            */

            if (mqttClient.subscribe("/pong3d/player_id"))
            {
                Serial.println("Suscrito al topico: /pong3d/player_id");
            }
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
