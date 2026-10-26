#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// ─── forward declarations ──────────────────────────────────────────────
void handleButtonInterrupt();
void sendButtonData();
void webSocketEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t len);
void handleRoot();
// ───────────────────────────────────────────────────────────────────────

#define BUTTON_PIN 17
#define LED1_PIN   16
#define LED2_PIN    4

const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer        server(80);
WebSocketsServer webSocket(81);

// ─── state variables ───────────────────────────────────────────────────
volatile bool     buttonState       = HIGH;
volatile bool     lastButtonState   = HIGH;
volatile unsigned long debounceTime = 0;
volatile unsigned long buttonPressStart = 0;
volatile unsigned long buttonPressDuration = 0;

unsigned long totalPresses = 0;
unsigned long pressLimit   = 100;
bool          limitReached = false;

// LED‑2 / image flash control
bool           led2Flashing   = false;
bool           led2State      = false;
unsigned long  lastFlashTime  = 0;
// ───────────────────────────────────────────────────────────────────────

void IRAM_ATTR handleButtonInterrupt()
{
    unsigned long now = millis();
    if (now - debounceTime < 50) return;      // 50 ms debounce

    buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH && !limitReached) {
        buttonPressStart = now;
        digitalWrite(LED1_PIN, HIGH);          // LED‑1 ON immediately
    }
    else if (buttonState == HIGH && lastButtonState == LOW) {           // released
        buttonPressDuration = now - buttonPressStart;
        if (!limitReached) totalPresses++;

        if (totalPresses >= pressLimit) limitReached = true;

        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
        led2Flashing = false;
        led2State    = false;
    }

    lastButtonState = buttonState;
    debounceTime    = now;
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED1_PIN,  OUTPUT);
    pinMode(LED2_PIN,  OUTPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, CHANGE);

    WiFi.begin(ssid, password);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) { Serial.print('.'); delay(500); }
    Serial.print("\nIP: "); Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop()
{
    server.handleClient();
    webSocket.loop();

    unsigned long now = millis();

    // after 15 s of holding, start LED‑2 / image flashing
    if (buttonState == LOW && (now - buttonPressStart) >= 15000) {
        if (!led2Flashing) digitalWrite(LED1_PIN, LOW);  // ensure LED‑1 off once
        led2Flashing = true;
    }

    // manage LED‑2 blink (and image blink flag)
    if (led2Flashing && now - lastFlashTime >= 250) {
        lastFlashTime = now;
        led2State = !led2State;
        digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
    }

    // push updates every 100 ms
    static unsigned long lastSend = 0;
    if (now - lastSend >= 100) {
        lastSend = now;
        sendButtonData();
    }
}

// ─── web page ──────────────────────────────────────────────────────────
void handleRoot()
{
    String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Button Tracker</title>
  <style>
    img { width:300px; margin-top:20px; }
  </style>
  <script>
    let ws;
    window.onload = () => {
      ws = new WebSocket(`ws://${location.hostname}:81/`);
      ws.onmessage = (e) => {
        const d = JSON.parse(e.data);
        document.getElementById('presses').textContent = d.presses;
        document.getElementById('hold').textContent    = d.hold + ' ms';
        document.getElementById('limit').textContent   = d.limit;
        document.getElementById('status').textContent  =
            d.limitReached ? 'Press limit reached!' : '';

        // image blink
        document.getElementById('pic').style.display = d.imageOn ? 'block' : 'none';
      };
    };
    function setLimit(v){ ws.send('limit:'+v); }
  </script>
</head>
<body>
  <h1>ESP32 Button Tracker</h1>
  <p><strong>Presses:</strong> <span id="presses">0</span></p>
  <p><strong>Hold:</strong> <span id="hold">0 ms</span></p>
  <p><strong>Press limit:</strong> <span id="limit">100</span></p>
  <input type="range" min="1" max="500" value="100"
         oninput="setLimit(this.value)">
  <p id="status" style="color:red;"></p>

  <img id="pic"
       src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQhoZ7YbzCVDL9pL12fIwN2befbPFf5BXE6vQ&s"
       style="display:none">
</body>
</html>
)rawliteral";
    server.send(200, "text/html", page);
}

// ─── websocket helpers ─────────────────────────────────────────────────
void sendButtonData()
{
    unsigned long now   = millis();
    unsigned long hold  = (buttonState == LOW) ? now - buttonPressStart : buttonPressDuration;
    bool imageOn        = (led2Flashing ? led2State : false);

    String msg  = "{";
    msg += "\"presses\":"     + String(totalPresses) + ",";
    msg += "\"hold\":"        + String(hold)         + ",";
    msg += "\"limit\":"       + String(pressLimit)   + ",";
    msg += "\"limitReached\":" + String(limitReached ? 1 : 0) + ",";
    msg += "\"imageOn\":"     + String(imageOn ? 1 : 0);
    msg += "}";

    webSocket.broadcastTXT(msg);
}

void webSocketEvent(uint8_t n, WStype_t type, uint8_t* payload, size_t len)
{
    if (type == WStype_TEXT) {
        String msg = String((char*)payload);
        if (msg.startsWith("limit:")) {
            pressLimit   = msg.substring(6).toInt();
            limitReached = false;
            Serial.print("New limit: "); Serial.println(pressLimit);
        }
    }
}
