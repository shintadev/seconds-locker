#ifndef WEBSOCKET_HANDLERS_H
#define WEBSOCKET_HANDLERS_H

#include <ArduinoJson.h>
#include <SocketIoClient.h>

enum ConnectionState {
  DISCONNECTED,
  CONNECTING_WIFI,
  CONNECTING_WEBSOCKET,
  CONNECTED,
};

extern SocketIoClient webSocket;
extern ConnectionState connectionState;

void setupWebSocket();
void connectWebsocket();
void socket_Connected(const char* payload, size_t length);
void socket_Disconnected(const char* payload, size_t length);
void socket_error(const char* payload, size_t length);
void socket_verifyCodeResult(const char* payload, size_t length);
void socket_statusCheck(const char* payload, size_t length);
void socket_openDoor(const char* payload, size_t length);
void sendVerifyCode(const char* otp);
void sendBoxUsage(const String& doorId, bool isObject);
void sendWarning();
#endif
