#include <Ethernet2.h>
#include <PubSubClient.h>

String sTemp = "";

byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

IPAddress ip(172, 28, 2, 161);
IPAddress gw(172, 28, 2, 1);

IPAddress mydns(8, 8, 8, 8);

const char broker_name[] = "tom.uib.es"; //"labauto.sytes.net";
const unsigned int broker_port = 1883;   // 8080;
const char client_id[] = "jgm";          // Update

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

void setup()
{
  Serial.begin(115200);

  Ethernet.begin(mac, ip, mydns, gw);

  mqttClient.setServer(broker_name, broker_port);
  mqttClient.setCallback(callback);

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
    sTemp = "Topic creado para PONG 3D --> Lab. de sist. basados en microcomputador (21738)";
    pub_ok = mqttClient.publish("/pong3d/", c_str(sTemp));
    if (pub_ok)
    {
      Serial.println("Advice message Published");
    }
    else
    {
      Serial.println("Advice message NOT Published");
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
