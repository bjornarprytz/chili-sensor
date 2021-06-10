#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;          // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup()
{
    Serial.begin(115200); // initialize serial communication

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
                response = "ph: 420";
            }
            else if (currentLine.startsWith("GET /hum"))
            {
                response = "hum: 69";
            }
            else if (currentLine.startsWith("GET /ec"))
            {
                response = "ec: 1337";
            }
            else if (currentLine.length() == 0)
            {
                printOKHeader(client, response);
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

void printOKHeader(WiFiClient client, String response)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.print(response);
    // The HTTP response ends with another blank line:
    client.println();
}