#include "SmartProv.h"

SmartProv prov;

SmartProv::SmartProv() {}

SmartProv::~SmartProv() {
  if (server) {
    delete server;
    server = nullptr;
  }
}

void SmartProv::addField(const String& name, const String& label, const String& defaultVal) {
  ProvField f = {name, label, "", defaultVal};
  fields.push_back(f);
}

String SmartProv::getField(const String& name) {
  for (const auto& f : fields) {
    if (f.name == name) return f.value;
  }
  return "";
}

void SmartProv::setField(const String& name, const String& val) {
  for (auto& f : fields) {
    if (f.name == name) {
      f.value = val;
      prefs.begin("smartprov", false);
      prefs.putString(name.c_str(), val);
      prefs.end();
      return;
    }
  }
}

String SmartProv::getIP() {
  if (connected) return WiFi.localIP().toString();
  if (inApMode) return WiFi.softAPIP().toString();
  return "0.0.0.0";
}

int SmartProv::getRSSI() {
  return WiFi.RSSI();
}

bool SmartProv::isConnected() {
  return connected || (WiFi.status() == WL_CONNECTED);
}

void SmartProv::clearCredentials() {
  prefs.begin("smartprov", false);
  prefs.clear();
  prefs.end();
  WiFi.disconnect(true, true);
  Serial.println("[prov] NVS credentials cleared");
}

void SmartProv::setupApPortal() {
  inApMode = true;
  connected = false;

  // Easy 8-character password per question paper specification
  apPass = "12345678";
  apSsid = "SmartClass-T07"; // Team ID T07

  IPAddress apIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(apSsid.c_str(), apPass.c_str(), 1, 0, 4);

  Serial.printf("[NET] Setup AP: %s | password: %s\n", apSsid.c_str(), apPass.c_str());
  Serial.printf("[prov] Portal available at http://%s\n", WiFi.softAPIP().toString().c_str());

  if (!server) {
    server = new WebServer(80);
  }

  server->on("/", HTTP_GET, [this]() {
    // Scan nearby networks
    int n = WiFi.scanNetworks();
    String netOptions = "";
    for (int i = 0; i < n; i++) {
      netOptions += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }

    String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>SmartClass Setup</title>
<style>
body{font-family:sans-serif;background:#0d1117;color:#c9d1d9;padding:1.5rem;max-width:480px;margin:auto}
h2{color:#38bdf8;border-bottom:1px solid #30363d;padding-bottom:.5rem}
label{display:block;margin-top:.8rem;font-size:.9rem;color:#8b949e}
input,select{width:100%;padding:.6rem;margin-top:.3rem;background:#161b22;border:1px solid #30363d;color:#fff;border-radius:6px;box-sizing:border-box}
button{width:100%;background:#0284c7;color:#fff;border:none;padding:.8rem;border-radius:6px;font-size:1rem;margin-top:1.5rem;cursor:pointer}
</style></head><body>
<h2>SmartClass Setup — T07</h2>
<form action='/save' method='POST'>
<label>Wi-Fi Network (SSID)</label>
<select name='ssid' id='netSel' onchange="document.getElementById('customSsid').value=this.value">
<option value=''>-- Select Nearby Wi-Fi --</option>)" + netOptions + R"(</select>
<input type='text' id='customSsid' name='custom_ssid' placeholder='Or enter SSID manually' style='margin-top:.5rem'>

<label>Wi-Fi Password</label>
<input type='password' name='password' placeholder='Network password'>
)";

    for (const auto& f : fields) {
      html += "<label>" + f.label + "</label>";
      html += "<input type='text' name='" + f.name + "' value='" + f.value + "' placeholder='" + f.defVal + "'>";
    }

    html += R"(<button type='submit'>Save and Connect</button>
</form></body></html>)";

    server->send(200, "text/html", html);
  });

  server->on("/save", HTTP_POST, [this]() {
    String ssid = server->arg("custom_ssid");
    if (ssid.isEmpty()) ssid = server->arg("ssid");
    String pass = server->arg("password");

    prefs.begin("smartprov", false);
    if (!ssid.isEmpty()) {
      prefs.putString("ssid", ssid);
      prefs.putString("pass", pass);
    }
    for (auto& f : fields) {
      if (server->hasArg(f.name)) {
        f.value = server->arg(f.name);
        prefs.putString(f.name.c_str(), f.value);
      }
    }
    prefs.end();

    server->send(200, "text/html", "<h3>Configuration saved! Rebooting to connect...</h3>");
    delay(1000);
    ESP.restart();
  });

  server->begin();
}

void SmartProv::begin(int bootPin, int ledPin) {
  prefs.begin("smartprov", true);
  savedSsid = prefs.getString("ssid", "");
  savedPass = prefs.getString("pass", "");

  // Load custom fields from NVS
  for (auto& f : fields) {
    f.value = prefs.getString(f.name.c_str(), f.defVal);
  }
  prefs.end();

  // If saved WiFi exists, attempt connection
  if (savedSsid.length() > 0) {
    Serial.printf("[prov] Connecting to saved Wi-Fi '%s'...\n", savedSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());

    connectStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 8000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.printf("[NET] Connected | IP: %s\n", WiFi.localIP().toString().c_str());
      return;
    }
    Serial.println("[prov] Failed to connect to saved Wi-Fi. Launching Setup AP...");
  }

  // Fallback to AP Captive Portal
  setupApPortal();
}

void SmartProv::update() {
  if (inApMode && server) {
    server->handleClient();
  }
}

