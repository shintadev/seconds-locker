#ifndef WEBSOCKET_HANDLERS_H
#define WEBSOCKET_HANDLERS_H

#include <ArduinoJson.h>
#include <WebSocketsClient.h>

enum ConnectionState {
  DISCONNECTED,
  CONNECTING_WIFI,
  CONNECTING_WEBSOCKET,
  CONNECTED,
  AUTHENTICATING,
  AUTHENTICATED
};

extern WebSocketsClient webSocket;
extern ConnectionState connectionState;
extern String token;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
void handleMessage(uint8_t* payload, size_t length);
void handleRegisterResult(const JsonDocument& doc);
void handleAuthResult(const JsonDocument& doc);
void handleVerifyCodeResult(const JsonDocument& doc);
void handleCommand(const JsonDocument& doc);
void registerLocker();
void authenticate();
void sendHeartbeat();
void sendVerifyCode(const char* otp);
void sendBoxUsage(const String& doorId, bool isObject);
void sendWarning();

#endif
