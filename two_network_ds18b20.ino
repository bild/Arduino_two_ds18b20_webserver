/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
*/

#include <SPI.h>
#include <Ethernet.h> // https://github.com/arduino-libraries/Ethernet

#include <OneWire.h> // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xFE, 0xFE, 0xFE, 0xFE, 0x00 };
IPAddress ip(172,31,172,101);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

/*
 * source https://www.arduinoclub.de/2015/01/23/arduino-ds18b20-temperatur-sensor-mit-filter/
 * DS18B20 Resolution read timed, filter result
 * Ralf Bohnen, 2013
 * This example code is in the public domain.

DALLES 18B50 CONNECTION
Dallas       | Arduino
----------------------
PIN 1 GND    |      GND
PIN 2 Data   |      D2/D3
PIN 3 VCC    |      5V
  _______
 / T O P \
/_________\
  |  |  |
  1  2  3

4.7KOhm zwischen PIN 2 und PIN 3
*/

//SETTINGS
#define ONE_WIRE_BUS_ONE 2 //DATA PORT DALLAS 18B50, DIGITAL 2
#define ONE_WIRE_BUS_TWO 3 //DATA PORT DALLAS 18B50, DIGITAL 3
int res = 11; //TEMPERATURE RESOLUTION
//END SETTINGS

OneWire oneWire_one(ONE_WIRE_BUS_ONE);
DallasTemperature sensor_one(&oneWire_one);
OneWire oneWire_two(ONE_WIRE_BUS_TWO);
DallasTemperature sensor_two(&oneWire_two);
DeviceAddress tempDeviceAddress_one;
DeviceAddress tempDeviceAddress_two;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature_one = 0.0;
float temperature_two = 0.0;
//END DALLAS

//Filter
float temp_one = 0;
float temp_two = 0;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Serial.println();

  Serial.println(F("DALLA 18B20"));
  Serial.print(F("DALLAS Library Version: "));
  Serial.println(DALLASTEMPLIBVERSION);
  Serial.println("");

  sensor_one.begin();
  sensor_one.setResolution(tempDeviceAddress_one, res);
  sensor_one.setWaitForConversion(false);
  sensor_one.requestTemperatures();

  sensor_two.begin();
  sensor_two.setResolution(tempDeviceAddress_two, res);
  sensor_two.setWaitForConversion(false);
  sensor_two.requestTemperatures();

  delayInMillis = 750 / (1 << (12 - res));
  lastTempRequest = millis();

  if (!sensor_one.getAddress(tempDeviceAddress_one, 0)) {
    Serial.println(F("Error connecting to Sensor."));
  } else {
    Serial.print(F("Sensor found at address :"));
    printAddress(tempDeviceAddress_one);
    Serial.println("");
    Serial.print(F("Test read temperature : "));
    temperature_one = sensor_one.getTempCByIndex(0);
    Serial.print(temperature_one);
    Serial.println(F(" degree Celsius"));
    sensor_one.setResolution(tempDeviceAddress_one, res);
    sensor_one.requestTemperatures();
    Serial.println("");
  }

  if (!sensor_two.getAddress(tempDeviceAddress_two, 0)) {
    Serial.println(F("Error connecting to Sensor."));
  } else {
    Serial.print(F("Sensor found at address :"));
    printAddress(tempDeviceAddress_two);
    Serial.println("");
    Serial.print(F("Test read temperature : "));
    temperature_two = sensor_two.getTempCByIndex(0);
    Serial.print(temperature_two);
    Serial.println(F(" degree Celsius"));
    sensor_two.setResolution(tempDeviceAddress_two, res);
    sensor_two.requestTemperatures();
    Serial.println("");
  }
}

void loop()
{

 if (millis() - lastTempRequest >= delayInMillis) {
    temp_one = sensor_one.getTempCByIndex(0);
    sensor_one.setResolution(tempDeviceAddress_one, res);
    sensor_one.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - res));

    //Output
    Serial.print(F("Sensor One raw: "));
    Serial.print(temp_one);
    Serial.println (F(" degree Celsius"));

    temp_two = sensor_two.getTempCByIndex(0);
    sensor_two.setResolution(tempDeviceAddress_two, res);
    sensor_two.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - res));
    lastTempRequest = millis();

    //Output
    Serial.print(F("Sensor Two raw: "));
    Serial.print(temp_two);
    Serial.println (F(" degree Celsius"));

    Serial.print(F("Sensor delta: "));
    Serial.print(temp_one-temp_two);
    Serial.println (F(" degree Celsius"));
    Serial.println ("");
  }

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
	        client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<p>Sensor 1: ");
          client.println(temp_one);
          client.println(" degree Celsius</p>");
          client.println("<p>Sensor 2: ");
          client.println(temp_two);
          client.println(" degree Celsius</p>");

          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
