#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

unsigned long messageInterval = 2000;
bool connected = false;
const char* ssid = "Robolab";
const char* password = "wifi123123123";
const char* websockets_server_host = "servicerobot.pro"; // Enter server adress
//const char* websockets_server_host = "192.168.0.101"; // Enter server adress
const uint16_t websockets_server_port = 81; // Enter server port

using namespace websockets;

unsigned long lastUpdate = millis();
unsigned long lastUpdate15 = millis();

String output;

#include "ServoEasing.hpp"
#include "PinDefinitionsAndMore.h"

ServoEasing Servo1;
ServoEasing Servo2;
#define START_DEGREE_VALUE  90 // The degree value written to the servo at time of attach.

unsigned long timing;

StaticJsonDocument<200> doc;
StaticJsonDocument<512> doc2;

int stop = 0;
const char* method = "";
const char* username = "";
float message = 0;
float message2 = 0;
int posMessage = 90;
int posMessage2 = 90;
int accel = 1;

WebsocketsClient client;

void onMessageCallback(WebsocketsMessage messageSocket) {
    Serial.print("Got Message: ");
    Serial.println(messageSocket.data());

    DeserializationError error = deserializeJson(doc, messageSocket.data());
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    method = doc["method"];

    if(String(method) == "connection"){
        Serial.printf("[WSc] WStype_CONNECTED\n");
    }
        
    if(String(method) == "messages"){
        stop = doc["stop"];
        accel = doc["accel"];
        
        message = doc["message"];
        message2 = doc["message2"];

        Serial.printf("message = %s\n", String(message));
        Serial.printf("message2 = %s\n", String(message2));

        doc2["method"] = "messages";
        doc2["message"] = message;
        doc2["message2"] = message2;
        String output = doc2.as<String>();
        client.send(output);


        // message = message * 27;
        // message2 = message2 * 75;
        message = message * 90;
        message2 = message2 * 90;
        message = map(message, 0, 90, 90, 180);
        message2 = map(message2, 0, 90, 90, 180);

        
    //     Servo1.easeTo(message);
    //     Servo2.easeTo(message2);

        if(stop == 1){
            posMessage = message;
            posMessage2 = message;
            Servo1.easeTo(message);
            Servo2.easeTo(message2);
            stop = 0;
        }
     }

}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        connected = true;
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        connected = false;
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}


void socketSetup(){
    // run callback when messages are received
    client.onMessage(onMessageCallback);
    
    // run callback when events are occuring
    client.onEvent(onEventsCallback);

    // Connect to server
    client.connect(websockets_server_host, websockets_server_port, "/");

    // Send a message
    //client.send("Hello Server");
    doc2["method"] = "connection";
    doc2["id"] = "b078167f69934795e54a54dc831acea8|a46d12213abfad52db817c17e1fec1ae";
    output = doc2.as<String>();
    client.send(output);
    // Send a ping
    client.ping();
};

void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    };

    socketSetup();

    #if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
        delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
    #endif
            // Just to know which program is running on my Arduino
        Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_SERVO_EASING));
    if (Servo1.attach(SERVO1_PIN, START_DEGREE_VALUE) == INVALID_SERVO) {
            Serial.println(F("Error attaching servo"));
    }
        if (Servo2.attach(SERVO2_PIN, START_DEGREE_VALUE) == INVALID_SERVO) {
            Serial.println(F("Error attaching servo"));
    }
    Servo1.setSpeed(450);
    Servo2.setSpeed(450);

    //message = message * 25;
    message = message * 90;
    message2 = message2 * 90;

    message = map(message, 0, 90, 90, 180);
    message2 = map(message2, 0, 90, 90, 180);

}

void socetConnected(){

    client.connect(websockets_server_host, websockets_server_port, "/");
    Serial.println("[WSc] SENT: Simple js client message!!");
    doc2["method"] = "connection";
    doc2["id"] = "b078167f69934795e54a54dc831acea8|a46d12213abfad52db817c17e1fec1ae";
    output = doc2.as<String>();
    client.send(output);
    
};

void loop(){
   
    client.poll();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi.reconnect()-----------------------------------");
        Serial.println("WiFi.reconnect()-----------------------------------");
        //WiFi.disconnect();
        //WiFi.reconnect();
        ESP.restart();
        socketSetup();
        socetConnected();
        // ESP.reset(); 
        // WiFi.disconnect();
        // WiFi.reconnect();
    };

    if (lastUpdate + messageInterval < millis()){
        if (connected == false){
            Serial.printf(", WS_CONNECTED_connected_false =================================== %s\n", String(connected));
            socetConnected();
        };
        lastUpdate = millis();
    };

    if (lastUpdate15 + 15000 < millis()){
        Serial.printf("millis() = %s", String(millis()));
        Serial.printf(", WiFi.status() = %s", String(WiFi.status()));
        Serial.printf(", WL_CONNECTED = %s", String(WL_CONNECTED));
        Serial.printf(", connected = %s\n", String(connected));
        lastUpdate15 = millis();
    };


    // if (millis() - timing > accel*10){ 
	// 	timing = millis();
	// 	if(stop == 0){

            if(message2 > posMessage2){
                Servo2.easeTo(posMessage2);
                posMessage2++;                 
            }
            else if(message2 < posMessage2){
                Servo2.easeTo(posMessage2);   
                posMessage2--; 
            }

            if(message > posMessage){
                Servo1.easeTo(posMessage);
                posMessage++;                 
            }
            else if(message < posMessage){
                Servo1.easeTo(posMessage);   
                posMessage--; 
            }

		//}
	// }
}