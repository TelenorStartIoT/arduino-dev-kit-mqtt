#include <MKRNB.h>
#include <Modem.h>
#include "PubSubClient.h"
#include "GSMSecurity.h"
#include "MICcertificatesYOUR_THING_ID.h"

const char PINNUMBER[]      = "1111";                                           // PIN Number
const char APN[]            = "telenor.iot";                                    // APN
NBSSLClient client;
GPRS         gprs;
NB           nbAccess(true);                                                    // true enables modem debug (i.e. print AT commands on serial)
GSMSecurity  profile;

const char client_id[]      = "00001770";                                       // Change this to your Thing ID
const char topic[]          = "$aws/things/00001770/shadow/update";             // Change this to your Thing ID
const char downlink_topic[] = "$aws/things/00001770/shadow/update";             // Change this to your Thing ID
const char aws_host[]       = "a3k7odshaiipe8.iot.eu-west-1.amazonaws.com";
uint16_t   port             = 8883;                                             // Port 8883 is the default for MQTT

PubSubClient mqttClient(aws_host, port, client);

void setup() {
  String response;

  // Reset the ublox module
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, HIGH);
  delay(100);
  digitalWrite(SARA_RESETN, LOW);

  // Open serial communications and wait for port to open
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting Arduino MQTT client.");

  // Wait for modem to become ready
  Serial.println("Waiting for modem to get ready...");
  MODEM.begin();
  while (!MODEM.noop());

  // Disconnect from any networks
  Serial.print("Disconnecting from network: ");
  MODEM.sendf("AT+COPS=2");
  MODEM.waitForResponse(2000);
  Serial.println("done.");

  // Set Radio Access Technology (RAT)
  Serial.println("Set Radio Access Technology to NB-IoT or Cat M1 (7 is for Cat M1 and 8 is for NB-IoT): ");
  MODEM.sendf("AT+URAT=7");
  MODEM.waitForResponse(100, &response);
  Serial.println("done.");

  // Apply changes
  Serial.print("Applying changes and saving configuration: ");
  MODEM.sendf("AT+CFUN=15");
  MODEM.waitForResponse(5000);
  delay(5000);

  while (MODEM.waitForResponse(1000) != 1) {
    delay(1000);
    MODEM.noop();
  }
  Serial.println("done.");

  // Set modem to "full functionality"
  Serial.println("Modem ready, turn radio on in order to configure it...");
  MODEM.sendf("AT+CFUN=1");
  MODEM.waitForResponse(2000, &response);
  Serial.println("done.");

  // Wait for a good signal strength (between 0 and 98)
  Serial.println("Check attachment until CSQ RSSI indicator is less than 99...");
  int status = 99;
  while (status > 98 && status > 0) {
    MODEM.sendf("AT+CSQ");
    MODEM.waitForResponse(2000, &response);

    String sub = response.substring(6,8);
    status = sub.toInt(); // Will return 0 if no valid number is found
    delay(1000);
  }
  Serial.println("done.");

  // Set operator to Telenor
  Serial.println("Set operator to Telenor...");
  MODEM.sendf("AT+COPS=1,2,\"24201\"");
  MODEM.waitForResponse(2000, &response);
  Serial.println("done.");

  // Set APN and check if network is ready
  boolean connected = false;
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER, APN, true) == NB_READY) && (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  // Setup MQTT socket for connection to MIC...
  Serial.println("\n *** Setup security profile to be used when connecting to MIC over MQTT ... ***");
  // profile.removeCertificate();
  profile.setRootCertificate(MIC_CERTS[0].name, MIC_CERTS[0].data, MIC_CERTS[0].size);
  profile.setClientCertificate(MIC_CERTS[1].name, MIC_CERTS[1].data, MIC_CERTS[1].size);
  profile.setPrivateKey(MIC_CERTS[2].name, MIC_CERTS[2].data, MIC_CERTS[2].size);

  profile.setValidation(SSL_VALIDATION_ROOT_CERT);
  profile.setVersion(SSL_VERSION_TLS_1_2);
  profile.setCipher(SSL_CIPHER_AUTO);
  // profile.listAllCertificates();

  connected = false;
  Serial.println("Not connected yet.");
  int i = 0;
  connected = mqttClient.connect(client_id);
  while (!connected) {
    i = i + 1;
    Serial.println("Still not connected to MQTT broker.");

    // Try again in a few seconds....
    delay(5000);
    if (i > 10) {
      break;
    }
    connected = mqttClient.connect(client_id);
  }

  if (!connected) {
    Serial.println("Tried to connect to the MQTT broker 10 times without success, exit...");
    exit(0);
  }

  Serial.println("Connected to AWS IoT!!!");

  // Seed the random number generator (used to generate varying MQTT payloads)
  randomSeed(analogRead(0));
}

void loop() {
  int i = 0;
  boolean connected = false;

  // Publish a message roughly every 25 seconds
  publishMessage();

  // Check if MQTT connection is still alive
  for (i = 0; i < 10; i++) {
    connected = mqttClient.loop();

    // Serial.println("Checking connection to broker..");
    if (!connected) {
      Serial.println("Lost connection to MQTT broker, exit...");
      exit(0);
    } else {
      // Serial.println("Connection to broker is fine");
    }
    delay(1000);
  }
}

void publishMessage() {
  boolean result           = false;
  double temperature       = 0.0;
  double humidity          = 0.0;
  String temperatureString = "";
  String humidityString    = "";
  char temperatureArray[6] = "";
  char humidityArray[6]    = "";
  char message[MQTT_MAX_PACKET_SIZE];

  Serial.println("Publishing message...");
  temperature       = randomDoubleString(22.00, 24.99);
  humidity          = randomDoubleString(65.00, 78.99);
  temperatureString = String(int(temperature)) + "." + String(getDecimal(temperature));
  humidityString    = String(int(humidity)) + "." + String(getDecimal(humidity));
  temperatureString.toCharArray(temperatureArray, temperatureString.length());
  humidityString.toCharArray(humidityArray, humidityString.length());

  sprintf(message, "{\"state\":{\"reported\": {\"temperature\":\"%s\", \"humidity\": \"%s\", \"latlng\": \"69.417383, 17.163101\"}}}", temperatureArray, humidityArray);

  Serial.print("Publish message: <");
  Serial.print(message);
  Serial.println(">");

  if (client.connected()) {
    if (mqttClient.connected()) {
      result = mqttClient.publish(topic, message);
      if (result) {
        Serial.println("Publish was a success!");
      } else {
        Serial.println("Publish failed!");
      }
    } else {
      Serial.println("MQTT client not connected!!");
    }
  } else {
    Serial.println("TCP/TLS client not connected!!");
  }
}

long getDecimal(float val) {
  int intPart = int(val);
  long decPart = 100 * (val - intPart);

  if (decPart > 0) return (decPart);
  else if (decPart < 0) return ((-1) * decPart);
  else if (decPart = 0) return (00);
}

double randomDoubleString(double minf, double maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}
