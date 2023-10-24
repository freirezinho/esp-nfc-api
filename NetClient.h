#ifndef NETCLIENT_H
#define NETCLIENT_H
#include <ESP8266HTTPClient.h>
#include <stdlib.h>
#include <WiFiClientSecureBearSSL.h>

class NetClient
{
private:
    static const char *password;

public:
    NetClient();
    static const char *ssid;
    static void connect(void (*callback)());
    // Returns HTTP Status Code
    static int get(String uri);
    // Returns HTTP Status Code
    static int post(String uri, String jsonData);
};

#endif