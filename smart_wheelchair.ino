/*
 * ESP32 Wheelchair - COMPLETE EDITION
 * Features: Voice Control (BT), GPS Tracking, Obstacle Avoidance, Web Dashboard
 * Size: Optimized to fit ESP32
 */

#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <BluetoothSerial.h>

// ==================== WiFi CONFIGURATION ====================
const char* ssid = "blue";
const char* password = "123456789";

// ==================== PIN DEFINITIONS ====================
#define IN1    26
#define IN2    27
#define IN3    14
#define IN4    12
#define ENA    25
#define ENB    33
#define IR_SENSOR  13
#define BUZZER     5
#define GPS_RX     22
#define GPS_TX     23

// ==================== GLOBAL OBJECTS ====================
WebServer server(80);
TinyGPSPlus gps;
BluetoothSerial SerialBT;
HardwareSerial GPS_Serial(2);

// ==================== SYSTEM STATE ====================
String currentCmd = "S";
int motorSpeed = 180;
bool obstacle = false;
bool emergencyStop = false;

// GPS Variables
float latitude = 0.0, longitude = 0.0;
float speedKPH = 0.0;
int satellites = 0;
bool gpsFixed = false;

// ==================== MOTOR CONTROL ====================
void motorControl(String cmd, int spd) {
  if (emergencyStop || obstacle) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0); analogWrite(ENB, 0);
    return;
  }
  
  if (cmd == "F") {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, spd); analogWrite(ENB, spd);
  }
  else if (cmd == "B") {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, spd); analogWrite(ENB, spd);
  }
  else if (cmd == "L") {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, spd); analogWrite(ENB, spd);
  }
  else if (cmd == "R") {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, spd); analogWrite(ENB, spd);
  }
  else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0); analogWrite(ENB, 0);
  }
}

// ==================== OBSTACLE TASK ====================
void obstacleTask(void* parameter) {
  while(true) {
    if (digitalRead(IR_SENSOR) == LOW) {
      if (!obstacle) {
        obstacle = true;
        emergencyStop = true;
        motorControl("S", 0);
        digitalWrite(BUZZER, HIGH);
        Serial.println(F("[!] OBSTACLE!"));
        SerialBT.println("OBSTACLE");
      }
    } else {
      if (obstacle) {
        obstacle = false;
        emergencyStop = false;
        digitalWrite(BUZZER, LOW);
        Serial.println(F("[OK] PATH CLEAR"));
        SerialBT.println("CLEAR");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ==================== GPS TASK ====================
void gpsTask(void* parameter) {
  while(true) {
    while (GPS_Serial.available() > 0) {
      char c = GPS_Serial.read();
      gps.encode(c);
    }
    
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      speedKPH = gps.speed.kmph();
      satellites = gps.satellites.value();
      
      if (satellites >= 3 && !gpsFixed) {
        gpsFixed = true;
        Serial.println(F("[GPS] FIXED!"));
        SerialBT.println("GPS_FIXED");
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// ==================== BLUETOOTH VOICE TASK ====================
void bluetoothTask(void* parameter) {
  String command = "";
  
  while(true) {
    if (SerialBT.available()) {
      char c = SerialBT.read();
      
      if (c == '\n' || c == '\r') {
        if (command.length() > 0) {
          command.trim();
          command.toUpperCase();
          
          Serial.print(F("BT: "));
          Serial.println(command);
          
          // Motor Commands
          if (command == "F" || command == "FORWARD" || command == "GO") {
            currentCmd = "F";
            if (!obstacle) motorControl("F", motorSpeed);
            SerialBT.println("OK: FORWARD");
          }
          else if (command == "B" || command == "BACKWARD" || command == "BACK") {
            currentCmd = "B";
            motorControl("B", motorSpeed);
            SerialBT.println("OK: BACKWARD");
          }
          else if (command == "L" || command == "LEFT") {
            currentCmd = "L";
            motorControl("L", motorSpeed);
            SerialBT.println("OK: LEFT");
          }
          else if (command == "R" || command == "RIGHT") {
            currentCmd = "R";
            motorControl("R", motorSpeed);
            SerialBT.println("OK: RIGHT");
          }
          else if (command == "S" || command == "STOP" || command == "HALT") {
            currentCmd = "S";
            emergencyStop = true;
            motorControl("S", 0);
            SerialBT.println("OK: STOP");
          }
          
          // Speed Commands
          else if (command == "FASTER") {
            motorSpeed = min(255, motorSpeed + 25);
            SerialBT.printf("SPEED: %d\n", motorSpeed);
          }
          else if (command == "SLOWER") {
            motorSpeed = max(80, motorSpeed - 25);
            SerialBT.printf("SPEED: %d\n", motorSpeed);
          }
          
          // GPS Location Commands
          else if (command == "WHERE" || command == "LOCATION" || command == "GPS") {
            if (gpsFixed && latitude != 0) {
              SerialBT.printf("LAT:%.6f LNG:%.6f\n", latitude, longitude);
              SerialBT.printf("SPEED:%.1fkm/h SAT:%d\n", speedKPH, satellites);
              
              // Send Google Maps link
              SerialBT.printf("MAP:https://maps.google.com/?q=%.6f,%.6f\n", latitude, longitude);
            } else {
              SerialBT.println("No GPS fix! Wait for satellites...");
            }
          }
          
          // Status Command
          else if (command == "STATUS") {
            SerialBT.printf("CMD:%s SPD:%d OBS:%s GPS:%s\n", 
                           currentCmd.c_str(), motorSpeed,
                           obstacle ? "YES" : "NO",
                           gpsFixed ? "FIXED" : "WAIT");
          }
          
          // Help Command
          else if (command == "HELP") {
            SerialBT.println("CMDS: F,B,L,R,S | FASTER,SLOWER | WHERE,STATUS,HELP");
          }
          
          else if (command.length() > 0) {
            SerialBT.println("Unknown. Type HELP for commands");
          }
          
          command = "";
        }
      } else {
        command += c;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ==================== WEB PAGE ====================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no"><title>Smart Wheelchair</title><link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap" rel="stylesheet"><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Inter,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:15px}.c{max-width:1200px;margin:0 auto}.h{background:#fff;border-radius:15px;padding:20px;margin-bottom:20px}.h h1{font-size:24px;background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent}.g{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px}.card{background:#fff;border-radius:15px;padding:20px}.card h3{margin-bottom:15px;border-bottom:2px solid #f0f0f0;padding-bottom:10px}.pad{text-align:center}.dir{display:inline-grid;grid-template-columns:repeat(3,70px);gap:10px;background:#f8f9fa;padding:15px;border-radius:50px}.btn{width:70px;height:70px;border:none;border-radius:50%;font-size:28px;cursor:pointer;background:#fff;color:#667eea;box-shadow:0 4px 8px rgba(0,0,0,.1)}.btn:active{transform:scale(.95)}.stop{background:#ef4444;color:#fff}.spd{width:100%;height:6px;border-radius:3px;background:#e0e0e0}.map{height:300px;background:#f8f9fa;border-radius:10px;margin-top:10px}.ginfo{display:grid;grid-template-columns:repeat(2,1fr);gap:10px;margin-top:15px}.gi{background:#f8f9fa;padding:10px;border-radius:8px}.gl{font-size:11px;color:#666}.gv{font-size:16px;font-weight:700;margin-top:5px}.alert{background:#fef3c7;border-left:3px solid #f59e0b;padding:12px;border-radius:8px;margin-top:12px}.emer{width:100%;padding:12px;background:#ef4444;color:#fff;border:none;border-radius:10px;cursor:pointer;margin-top:12px;font-weight:700}.buzz{width:100%;padding:10px;background:#667eea;color:#fff;border:none;border-radius:8px;cursor:pointer;margin-top:8px}@media(max-width:768px){.dir{grid-template-columns:repeat(3,55px)}.btn{width:55px;height:55px;font-size:22px}}</style></head><body><div class=c><div class=h><h1>⚡ Smart Wheelchair</h1><span id=conn style="display:inline-block;padding:4px 12px;border-radius:20px;font-size:11px;background:#10b98120;color:#10b981">● LIVE</span></div><div class=g><div class=card><h3>🎮 Controls</h3><div class=pad><div class=dir><div></div><button class=btn onclick=send('F')>▲</button><div></div><button class=btn onclick=send('L')>◀</button><button class="btn stop" onclick=send('S')>■</button><button class=btn onclick=send('R')>▶</button><div></div><button class=btn onclick=send('B')>▼</button><div></div></div></div><div><label>Speed: <span id=sv>180</span></label><input type=range min=0 max=255 value=180 class=spd id=sl onchange=setSp(this.value)></div></div><div class=card><h3>📍 Live Location</h3><div class=map><iframe id=mf width=100% height=100% frameborder=0 src="https://maps.google.com/maps?q=0,0&z=15&output=embed"></iframe></div><div class=ginfo><div class=gi><div class=gl>Latitude</div><div class=gv id=lat>--</div></div><div class=gi><div class=gl>Longitude</div><div class=gv id=lng>--</div></div><div class=gi><div class=gl>Speed</div><div class=gv id=sp>-- km/h</div></div><div class=gi><div class=gl>Satellites</div><div class=gv id=sat>--</div></div></div></div><div class=card><h3>🛡️ Safety</h3><div id=obs class=alert style="display:none">⚠️ <strong>OBSTACLE!</strong> Movement Stopped</div><button class=emer onclick=send('S')>🛑 EMERGENCY STOP</button><button class=buzz onclick=toggleBuzz()>🔔 Test Buzzer</button></div></div></div><script>function send(c){fetch('/c?val='+c)}function setSp(v){document.getElementById('sv').innerText=v;fetch('/s?val='+v)}function toggleBuzz(){fetch('/b')}function upd(){fetch('/st').then(r=>r.json()).then(d=>{document.getElementById('lat').innerText=d.lat.toFixed(5);document.getElementById('lng').innerText=d.lng.toFixed(5);document.getElementById('sp').innerText=d.spd+' km/h';document.getElementById('sat').innerText=d.sat;document.getElementById('sv').innerText=d.mspd;if(d.lat!=0&&d.lng!=0)document.getElementById('mf').src='https://maps.google.com/maps?q='+d.lat+','+d.lng+'&z=15&output=embed';document.getElementById('obs').style.display=d.obs?'block':'none'})}setInterval(upd,2000);upd()</script></body></html>
)rawliteral";

// ==================== WEB HANDLERS ====================
void handleRoot() { server.send_P(200, "text/html", index_html); }

void handleCmd() {
  String c = server.arg("val");
  c.toUpperCase();
  if (c == "F") { currentCmd = "F"; if (!obstacle) motorControl("F", motorSpeed); }
  else if (c == "B") { currentCmd = "B"; motorControl("B", motorSpeed); }
  else if (c == "L") { currentCmd = "L"; motorControl("L", motorSpeed); }
  else if (c == "R") { currentCmd = "R"; motorControl("R", motorSpeed); }
  else if (c == "S") { currentCmd = "S"; emergencyStop = true; motorControl("S", 0); }
  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  int s = server.arg("val").toInt();
  motorSpeed = constrain(s, 0, 255);
  if (currentCmd != "S" && !obstacle) motorControl(currentCmd, motorSpeed);
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  String j = "{";
  j += "\"lat\":" + String(latitude, 6) + ",";
  j += "\"lng\":" + String(longitude, 6) + ",";
  j += "\"spd\":" + String(speedKPH, 1) + ",";
  j += "\"sat\":" + String(satellites) + ",";
  j += "\"mspd\":" + String(motorSpeed) + ",";
  j += "\"obs\":" + String(obstacle ? "true" : "false");
  j += "}";
  server.send(200, "application/json", j);
}

void handleBuzzer() {
  static bool buz = false;
  buz = !buz;
  digitalWrite(BUZZER, buz);
  server.send(200, "text/plain", buz ? "ON" : "OFF");
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("=== ESP32 WHEELCHAIR SYSTEM ==="));
  
  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  
  // Sensors
  pinMode(IR_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  
  // GPS
  GPS_Serial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println(F("[GPS] Serial started at 9600"));
  
  // Bluetooth
  SerialBT.begin("ESP32_WC");
  Serial.println(F("[BT] Name: ESP32_WC"));
  
  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(F("[WiFi] Connecting"));
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("[WiFi] Connected!"));
    Serial.print(F("[IP] "));
    Serial.println(WiFi.localIP());
    
    server.on("/", handleRoot);
    server.on("/c", handleCmd);
    server.on("/s", handleSpeed);
    server.on("/st", handleStatus);
    server.on("/b", handleBuzzer);
    server.begin();
    Serial.println(F("[Web] Server started"));
  } else {
    Serial.println(F("[WiFi] FAILED! Check SSID/password"));
  }
  
  // Create tasks
  xTaskCreatePinnedToCore(obstacleTask, "Obstacle", 2048, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(gpsTask, "GPS", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(bluetoothTask, "Bluetooth", 4096, NULL, 2, NULL, 1);
  
  Serial.println(F("=== SYSTEM READY ==="));
  Serial.println(F("Commands: F,B,L,R,S | FASTER,SLOWER | WHERE,STATUS,HELP"));
  
  // Beep ready
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
  delay(10);
}
