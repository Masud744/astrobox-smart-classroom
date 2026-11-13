#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <vector>

// ── SmartProv: Self-contained Captive Portal & Provisioning ─
// Manages NVS credentials, fallback AP mode, and field storage

struct ProvField {
  String name;
  String label;
  String value;
  String defVal;
};

class SmartProv {
private:
  Preferences prefs;
  WebServer* server = nullptr;
  std::vector<ProvField> fields;
  bool connected = false;
  bool inApMode = false;
  String apSsid;
  String apPass;
  String savedSsid;
  String savedPass;
  unsigned long connectStart = 0;

  void setupApPortal();

public:
  SmartProv();
  ~SmartProv();

  void addField(const String& name, const String& label, const String& defaultVal = "");
  void begin(int bootPin = 0, int ledPin = 48);
  void update();

  bool isConnected();
  String getField(const String& name);
  void setField(const String& name, const String& val);

  String getIP();
  int getRSSI();
  void clearCredentials();
};

extern SmartProv prov;

