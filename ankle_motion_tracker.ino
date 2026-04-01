#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <math.h>

const char* ssid = "ESP32_AnkleTracker";
const char* password = "12345678";

WebServer server(80);

const int MPU_ADDR = 0x68;

int stepCount = 0;
bool movementActive = false;
float gyroThreshold = 60.0;

unsigned long lastStepTime = 0;
float cadence = 0.0;

float angleX = 0.0;
float angleY = 0.0;
float gyroY_deg = 0.0;

String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Wearable Ankle Motion Tracker</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      background: #f7f7f7;
      color: #111;
    }
    h1 {
      margin-bottom: 10px;
    }
    .card {
      background: white;
      border-radius: 12px;
      padding: 16px;
      margin-bottom: 16px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    }
    .metric {
      font-size: 18px;
      margin: 8px 0;
    }
    canvas {
      background: white;
      border-radius: 12px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    }
  </style>
</head>
<body>
  <h1>Wearable Ankle Motion Tracker</h1>

  <div class="card">
    <div class="metric"><b>AngleX:</b> <span id="angleX">0</span> deg</div>
    <div class="metric"><b>AngleY:</b> <span id="angleY">0</span> deg</div>
    <div class="metric"><b>GyroY:</b> <span id="gyroY">0</span> deg/s</div>
    <div class="metric"><b>Step Count:</b> <span id="stepCount">0</span></div>
    <div class="metric"><b>Cadence:</b> <span id="cadence">0</span> steps/min</div>
  </div>

  <canvas id="graph" width="900" height="400"></canvas>

  <script>
    const canvas = document.getElementById("graph");
    const ctx = canvas.getContext("2d");

    const maxPoints = 120;
    const angleXData = [];
    const angleYData = [];
    const gyroYData = [];

    function addPoint(arr, val) {
      arr.push(val);
      if (arr.length > maxPoints) arr.shift();
    }

    function drawGraph() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      // axes
      ctx.strokeStyle = "#cccccc";
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(40, 20);
      ctx.lineTo(40, 360);
      ctx.lineTo(880, 360);
      ctx.stroke();

      // center line
      ctx.strokeStyle = "#dddddd";
      ctx.beginPath();
      ctx.moveTo(40, 190);
      ctx.lineTo(880, 190);
      ctx.stroke();

      function drawLine(data, color, scale) {
        if (data.length < 2) return;
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();

        for (let i = 0; i < data.length; i++) {
          const x = 40 + (i * (840 / (maxPoints - 1)));
          const y = 190 - (data[i] * scale);
          if (i === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
      }

      // labels
      ctx.fillStyle = "#111";
      ctx.font = "14px Arial";
      ctx.fillText("AngleX", 60, 25);
      ctx.fillStyle = "red";
      ctx.fillRect(120, 15, 20, 4);

      ctx.fillStyle = "#111";
      ctx.fillText("AngleY", 170, 25);
      ctx.fillStyle = "blue";
      ctx.fillRect(230, 15, 20, 4);

      ctx.fillStyle = "#111";
      ctx.fillText("GyroY", 280, 25);
      ctx.fillStyle = "green";
      ctx.fillRect(340, 15, 20, 4);

      drawLine(angleXData, "red", 2.0);
      drawLine(angleYData, "blue", 2.0);
      drawLine(gyroYData, "green", 1.0);
    }

    async function updateData() {
      try {
        const response = await fetch("/data");
        const data = await response.json();

        document.getElementById("angleX").textContent = data.angleX.toFixed(2);
        document.getElementById("angleY").textContent = data.angleY.toFixed(2);
        document.getElementById("gyroY").textContent = data.gyroY.toFixed(2);
        document.getElementById("stepCount").textContent = data.stepCount;
        document.getElementById("cadence").textContent = data.cadence.toFixed(2);

        addPoint(angleXData, data.angleX);
        addPoint(angleYData, data.angleY);
        addPoint(gyroYData, data.gyroY);

        drawGraph();
      } catch (err) {
        console.log("Fetch error:", err);
      }
    }

    setInterval(updateData, 200);
    updateData();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleData() {
  String json = "{";
  json += "\"angleX\":" + String(angleX, 2) + ",";
  json += "\"angleY\":" + String(angleY, 2) + ",";
  json += "\"gyroY\":" + String(gyroY_deg, 2) + ",";
  json += "\"stepCount\":" + String(stepCount) + ",";
  json += "\"cadence\":" + String(cadence, 2);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  WiFi.softAP(ssid, password);

  Serial.println("WiFi hotspot started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("Web server started");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  byte err = Wire.endTransmission(false);

  int count = Wire.requestFrom(MPU_ADDR, 14, true);

  if (err == 0 && count == 14) {
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();

    Wire.read(); Wire.read(); // skip temp

    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();

    angleX = atan2((float)ay, (float)az) * 180.0 / PI;
    angleY = atan2((float)ax, (float)az) * 180.0 / PI;
    gyroY_deg = gy / 131.0;

    if (fabs(gyroY_deg) > gyroThreshold && !movementActive) {
      unsigned long currentTime = millis();
      stepCount++;

      if (lastStepTime > 0) {
        float stepIntervalSec = (currentTime - lastStepTime) / 1000.0;
        cadence = 60.0 / stepIntervalSec;
      }

      lastStepTime = currentTime;
      movementActive = true;
    }

    if (fabs(gyroY_deg) < 20.0) {
      movementActive = false;
    }
  }

  server.handleClient();
  delay(50);
}