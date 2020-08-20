
#ifndef SERVER_H
#define SERVER_H

#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include "Matrix.h"

#ifndef HAVE_HWEspSerial
#include "SoftwareSerial.h"
#endif

// receive commands over WiFi
class MatrixServer
{
private:
    int m_status = WL_IDLE_STATUS;
    short m_currentScrollDelay = 50;

#ifndef HAVE_HWEspSerial
    SoftwareSerial m_espSerial;
#endif

    WiFiEspServer m_server;

    void printWifiStatus()
    {
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal strength (RSSI): ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    }

public:
    MatrixServer(const char *ssid, const char *password, int port = 80)
        : m_espSerial(2, 3), m_server(port)
    {
        m_espSerial.begin(9600);
        delay(100);
        WiFi.init(&m_espSerial);

        if (WiFi.status() == WL_NO_SHIELD) {
            Serial.println("WiFi shield not present");
            while (true);
        }

        while (m_status != WL_CONNECTED) {
            Serial.print("Attempting to connect to WPA SSID ");
            Serial.println(ssid);
            m_status = WiFi.begin(ssid, password);
        }

        Serial.println("Connected");
        printWifiStatus();

        m_server.begin();
        Serial.print("Server started on port ");
        Serial.println(port);
    }

    // endlessly wait for clients and execute their commands
    void enterLoop(Matrix &matrix)
    {
        while (true) {
            WiFiEspClient client = m_server.available();
            if (client) {
                Serial.println("New client");
                while (client.connected()) {
                    if (client.available()) {
                        char op;
                        do {
                            if (client.available()) {
                                op = client.read();
                                Serial.println((short)op);
                            }
                        // skip meaningless characters
                        } while (op == '\r' || op == '\n' || op == '\0' || op == -1);

                        Serial.print("Operation ");
                        Serial.println((short)op);

                        if (op == 'd') {
                            m_currentScrollDelay = client.parseInt();
                            Serial.print("Set scroll delay to ");
                            Serial.println(m_currentScrollDelay);
                            client.print("Set scroll delay to ");
                            client.println(m_currentScrollDelay);
                        } else if (op == 's') {
                            String str = client.readString();
                            str.replace("\n", "");
                            str.replace("\r", "");
                            for (char c : str)
                                Serial.println((int)c);
                            Serial.print("Displaying string ");
                            Serial.print(str);
                            Serial.print("...");
                            client.print("Displaying string ");
                            client.print(str);
                            client.print("...");
                            matrix.displayString(str, m_currentScrollDelay);
                            while (!matrix.isFinished()) {
                                matrix.updateScroll();
                            }
                            Serial.println("finished");
                            client.println("finished");
                        }
                    }
                }

                delay(10);
                client.stop();
                Serial.println("Client disconnected");
            }
        }
    }
};

#endif