#include "WiFiClientSecureBearSSL.h"
#include "NetClient.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *NetClient::password = "impacta2021";
const char *NetClient::ssid = "Faculdade Impacta v2";

std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
HTTPClient https;

void NetClient::connect(void (*callback)())
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        callback();
    }
    Serial.println("");
    Serial.println("WiFi connected");
    client->setInsecure();
}

int NetClient::get(String uri)
{
    https.begin(*client, uri);
    int const result = https.GET();
    https.end();
    return result;
}

int NetClient::post(String uri, String jsonData)
{
    https.begin(*client, uri);
    https.addHeader("Content-Type", "application/json");
    int const result = https.POST(jsonData);
    https.end();
    return result;
}