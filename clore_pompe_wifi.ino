//importation des bibliothèque
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//declaration des pins utilise
#define chlorep 16//declaration des bronches utiliser dans le système 
#define levelPin 5

//declaration des variables pour l'accès à l'internet via wifi
const char* ssid = "Redmi Note 9S";
const char* password = "123456789";

//declaration des variables pour la communication avec le mqtt broker
const char* mqtt_server = "192.168.187.64";
const char* mqtt_user = "root";
const char* mqtt_pwd = "root";
const char* mqtt_clientID = "Pump_Chlore";    

// declaration d'objet (wifi client)
WiFiClient espClient;
PubSubClient client(espClient);

//variables
int levelchlore, inchlore, levelState;
//String levelState;

//eleminate redandancy repetation des messqge
String chlore_p_redandancy = "";

// fonction pour connecter au routeur
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// fonction pour recuperer اخد le requete de topic abonner
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (topic == "chlore_S/V"){ // Chlore value from sensor
    Serial.print("chlore level is :");
    levelchlore = messageTemp.toInt();
    Serial.print(levelchlore);
  }

  if (topic == "chlore_V/IN"){ // desired value of Chlore
    Serial.print("Disered chlore level :");
    inchlore = messageTemp.toInt();
    Serial.print(inchlore);
  }
}

// This functions reconnects your ESP8266 to your MQTT broker
// qbonner des topic pricie
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_clientID, mqtt_user, mqtt_pwd)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("chlore_S/V");
      client.subscribe("chlore_V/IN");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// fonction d'initialisation de système (determination les etats initiale) 
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  Serial.begin(115200);
  pinMode(levelPin, INPUT);
  pinMode(chlorep, OUTPUT);
  digitalWrite(chlorep, HIGH);
  setup_wifi();//excuter une seule fois wifi
  client.setCallback(callback);
  client.setServer(mqtt_server, 1883);
}

// fonction principale repetitive
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop()){
    client.connect(mqtt_clientID, mqtt_user, mqtt_pwd);
  }
  
  Serial.print("Level is: ");
  Serial.println(levelState);
  Serial.print("chlore level is: ");
  Serial.println(levelchlore);
  Serial.print("chlore desired is: ");
  Serial.println(inchlore);
  levelState = digitalRead(levelPin);
  delay(1000);
  if (levelState == 1){
     client.publish("Level/V", "true", true);
      if (levelchlore < inchlore){
       if (chlore_p_redandancy != "on"){
        digitalWrite (chlorep, LOW);
        Serial.println("chlore POMPE is ON!!");
        client.publish("chlore/P", "ON", true);
        chlore_p_redandancy = "on";
       }
      }else if (levelchlore >= inchlore){
        if (chlore_p_redandancy != "off"){
          digitalWrite (chlorep, HIGH);
          Serial.println("chlore POMPE is OFF!!");
          client.publish("chlore/P", "OFF", true);
          chlore_p_redandancy = "off";
        }
      }
    }else if(levelState == 0){ 
      client.publish("Level/V", "false", true);
      if ((chlore_p_redandancy != "off")){
        digitalWrite (chlorep, HIGH);
        Serial.println("chlore POMPE is OFF!!");
        client.publish("chlore/P", "OFF", true);
        chlore_p_redandancy = "off";
      }
    }
}
