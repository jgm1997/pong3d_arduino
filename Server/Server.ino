#include <Ethernet2.h>
#include <PubSubClient.h>
#include <Arduino.h>

//Joystick
#define JOY_X A0
#define JOY_Y A1
#define JOY_SW A2

String sTemp = "";
volatile bool timer_flag = false;

// Valores a actuaizar y cambiar por cada Arduino
byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

IPAddress ip(172, 28, 2, 161);
IPAddress gw(172, 28, 2, 1);

IPAddress mydns(8, 8, 8, 8);

const char broker_name[] = "tom.uib.es"; //"labauto.sytes.net";
const unsigned int broker_port = 1883;   // 8080;
const char client_id[] = "jgm562";       // Update

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

ISR(TIMER1_COMPA_vect)
{
  timer_flag = true;
}

void setup()
{
  Serial.begin(115200);

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
  delay(1500);
}

void loop()
{
  bool pub_ok;

  while (true)
  {
    if (!mqttClient.connected())
    {
      reconnect();
    }

    // Se publica previamente un mensaje de aviso en caso de que haya más usuarios en el broker
    sTemp = "PONG 3D --> Lab. de sist. basados en microcomputador (21738)";
    pub_ok = mqttClient.publish("/pong3d/", sTemp.c_str());
    if (pub_ok)
    {
      Serial.println("\nAdvice message Published");
    }
    else
    {
      Serial.println("\nAdvice message NOT Published");
    }

    // Publicación de mensajes de juego
    sTemp = "Jugador 1 conectado";
    pub_ok = mqttClient.publish("/pong3d/player1", sTemp.c_str());
    if (pub_ok)
    {
      Serial.println("\nPlayer 1 connected");
    }
    else
    {
      Serial.println("\nWaiting for player 1...");
    }

    // Call regularly to process incoming messages
    // and maintain connection with the server
    mqttClient.loop();

    delay(1000);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (strcmp(topic, "/pong3d/player1") == 0)
  {

    for (int i = 0; i < length; i++)
    {
      // Casting a char de cada valor para comprobación en monitor serie
      Serial.print((char)payload[i]);
    }

    // Salto de línea
    // Serial.println();
  }
  else if (strcmp(topic, "/pong3d/player2") == 0)
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
  else if (strcmp(topic, "/pong3d/ball/") == 0)
  {
    // switch
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
      if (mqttClient.subscribe("/pong3d/player2"))
      {
        Serial.println("Suscrito al topico: /pong3d/player2");
      }

      if (mqttClient.subscribe("/pong3d/player1"))
      {
        Serial.println("Suscrito al topico: /pong3d/player2");
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
