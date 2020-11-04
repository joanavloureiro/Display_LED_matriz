

// Import required libraries
#include <LedControlSPIESP8266.h>

#include <FC16_Font.h>
#include <FC16.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char *ssid = "VIVOFIBRA-1C12";
const char *password = "33d7f81c12";

const char *http_username = "admin";
const char *http_password = "admin";

const char *PARAM_INPUT_1 = "state";

const int output = 2;

const int csPin = D8;

//Numero de displays que estamos usando
const int numDisp = 4;

//Tempo do scroll em milisegundos
const int scrollDelay = 150;

FC16 display = FC16(csPin, numDisp);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>DisplayPET</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      border: 0;
      outline: none;
      overflow: hidden;
    }

    body {
      background-color: #00B19F;
      height: 100vh;
      text-transform: uppercase;
      font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
    }

    h1 {
      text-align: center;
      font-size: 2.5rem;
      color: #fafafa;
      text-shadow: 0px 0px 12px #fff;
      padding: 24px 0;
      font-weight: 500;
      width: 100%;
      height: 12vh;
    }

    div {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: space-between;
      height: 70vh;
    }

    input {
      border-radius: 8px;
      cursor: pointer;
      text-align: center;
      padding: 16px;
      color: #232323;
      width: 180px;
      box-shadow: 4px 3px 12px rgba(0, 0, 0, 0.363);
    }

    .logout-button {
      position: absolute;
      top: 24px;
      right: 24px;
      font-size: 24px;
      background-color: red;
      color: white;
      border-radius: 50%;
      width: 60px;
      height: 60px;
      cursor: pointer;
      transition: 700ms;
    }

    .logout-button:hover {
      background-color: rgb(155, 2, 2);
    }

    @media(max-width:768px) {
      h1 {
        margin-top: 36px;
      }

      .logout-button {
        top: 0px;
        right: 5px;
        font-style: italic;
        font-size: 24px;
        background-color: inherit;
      }
    }
  </style>
</head>

<body>
  <h1>DISPLAY PET</h1>
  <div> <input type="button" value="BEM VINDO AO PET" onclick="bemVindo()" /> <input type="button" value="EM REUNIAO"
      onclick="emReuniao()" /> <input type="button" value="EM ALMOCO" onclick="emAlmoco()" /> <input type="button"
      value="SALA VAZIA" onclick="salaVazia()" /> <input type="button" value="LIMPAR DISPLAY" onclick="clrDisplay()" />
  </div>
  <script> function emReuniao() { window.location.href = '/0' } function emAlmoco() { window.location.href = '/1' } function salaVazia() { window.location.href = '/2' } function bemVindo() { window.location.href = '/3' } function clrDisplay() { window.location.href = '/4' } </script>
  <button onclick="logoutButton()" class="logout-button">Sair</button>
  <script>function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if (element.checked) { xhr.open("GET", "/update?state=1", true); }
      else { xhr.open("GET", "/update?state=0", true); }
      xhr.send();
    }
    function logoutButton() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/logout", true);
      xhr.send();
      setTimeout(function () { window.open("/logged-out", "_self"); }, 1000);
    }
  </script>
</body>

</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";
    String outputStateValue = outputState();
    buttons += "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></p>";
    return buttons;
  }
  return String();
}

String outputState()
{
  if (digitalRead(output))
  {
    return "checked";
  }
  else
  {
    return "";
  }
  return "";
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  display.begin();
  //Intensidade / Brilho
  display.setIntensity(8);
  //Apaga o display
  display.clearDisplay();
  //Texto a ser exibido no display
  display.setText("Ligado");

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/0", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
    Serial.println("Em reuniao");
    display.setText("EM REUNIAO");
  });

  server.on("/1", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
    display.setText("EM ALMOCO");
  });

  server.on("/2", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
    display.setText("SALA VAZIA");
  });

  server.on("/3", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
    display.setText("BEM VINDO AO PET");
  });

  server.on("/4", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
    display.setText(" ");
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", logout_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1))
    {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      digitalWrite(output, inputMessage.toInt());
    }
    else
    {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop()
{

  //Chama a rotina de scroll
  display.update();

  //Aguarda o tempo definido
  delay(scrollDelay);
}
