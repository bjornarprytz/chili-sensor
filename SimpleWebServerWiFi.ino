
#include <SPI.h>
#include <WiFiNINA.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;          // your network key index number (needed only for WEP)

/// pH

#define Vref 4.95

/// TEMP / HUM

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT10   // DHT 10
//#define DHTTYPE DHT20   // DHT 20

/*Notice: The DHT10 and DHT20 is different from other DHT* sensor ,it uses i2c interface rather than one wire*/
/*So it doesn't require a pin.*/
#define DHTPIN 2          // what pin we're connected to（DHT10 and DHT20 don't need define it）
DHT dht(DHTPIN, DHTTYPE); //   DHT11 DHT21 DHT22
//DHT dht(DHTTYPE);         //   DHT10 DHT20 don't need to define Pin

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

#define T_H_COOLDOWN 2000
#define PH_COOLDOWN 5000

float humidity;
float temperature;
int pH;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup()
{
    Serial.begin(115200); // initialize serial communication

    Wire.begin();

    dht.begin();

    pinMode(LED_BUILTIN, OUTPUT); // set the LED pin mode

    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        // stop here
        while (true)
            continue;
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("Please upgrade the firmware");
    }

    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to Network named: ");
        Serial.println(ssid); // print the network name (SSID);

        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(10000);
    }
    server.begin();    // start the web server on port 80
    printWifiStatus(); // you're connected now, so print out the status
}

void loop()
{
    read_ph();
    read_temp_hum();
    serve(server);
}

unsigned long prevPhReading;

void read_ph()
{
    unsigned long currMillis = millis();

    if (currMillis - prevPhReading < PH_COOLDOWN)
    {
        return;
    }

    prevPhReading = currMillis;

    int sensorValue;

    long sensorSum;

    for (int m = 0; m < 50; m++)
    {
        sensorValue = analogRead(A0); //Connect the PH Sensor to A0 port

        sensorSum += sensorValue;
    }

    sensorValue = sensorSum / 50;

    pH = 7 - 1000 * (sensorValue - 372) * Vref / 59.16 / 1023;
}

unsigned long prevTempHumReading = 0;

void read_temp_hum()
{
    unsigned long currMillis = millis();

    if (currMillis - prevTempHumReading < T_H_COOLDOWN)
    {
        return;
    }

    prevTempHumReading = currMillis;

    float temp_hum_val[2] = {0};
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

    if (!dht.readTempAndHumidity(temp_hum_val))
    {
        humidity = temp_hum_val[0];
        temperature = temp_hum_val[1];
    }
}

/// SERVER

void serve(WiFiServer server)
{
    WiFiClient client = server.available(); // listen for incoming clients

    if (client)
    {                                 // if you get a client,
        Serial.println("new client"); // print a message out the serial port
        String currentLine = "";      // make a String to hold incoming data from the client
        String response = "";
        while (client.connected())
        { // loop while the client's connected
            currentLine = readLine(client);

            Serial.println(currentLine);

            if (currentLine.startsWith("GET /ph"))
            {
                response = String(String(pH) + "pH");
            }
            else if (currentLine.startsWith("GET /hum"))
            {
                response = String(String(humidity) + "%");
            }
            else if (currentLine.startsWith("GET /temp"))
            {
                response = String(String(temperature) + "*C");
            }
            else if (currentLine.startsWith("GET /ec"))
            {
                response = "ec: 1337";
            }
            else if (currentLine.length() == 0)
            {
                respondOK(client, response);
                response = "";
                break;
            }
        }
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    // print where to go in a browser:
    Serial.println(ip);
}

String readLine(WiFiClient client)
{
    String currentLine = "";

    while (client.connected())
    { // loop while the client's connected
        if (client.available())
        {                           // if there's bytes to read from the client,
            char c = client.read(); // read a byte, then

            if (c == '\n')
            {
                return currentLine;
            }

            if (c != '\r')
            {                     // if you got anything else but a carriage return character,
                currentLine += c; // add it to the end of the currentLine
            }
        }
    }
}

void respondOK(WiFiClient client, String response)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.print(response);
    // The HTTP response ends with another blank line:
    client.println();
}