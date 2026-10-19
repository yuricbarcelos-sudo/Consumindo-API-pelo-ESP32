#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Credenciais da rede Wi-Fi
const char* ssid = "Yurizada";
const char* password = "yuri290620";

WebServer server(80);

String currentAdvice = "Carregando conselho...";
int currentAdviceId = 0;

// Função para buscar o conselho original em inglês na API Advice Slip
void fetchAdvice() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    if (http.begin(client, "https://api.adviceslip.com/advice")) {
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          currentAdviceId = doc["slip"]["id"];
          const char* adviceText = doc["slip"]["advice"];
          currentAdvice = String(adviceText);
        }
      }
      http.end();
    }
  }
}

// Função que entrega a interface HTML + JavaScript de tradução
void handleRoot() {
  fetchAdvice(); // Busca um novo conselho na API

  String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 - Advice Generator (PT-BR)</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #121212; color: #e0e0e0; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; }";
  html += ".card { background: #1e1e1e; padding: 2rem; border-radius: 12px; box-shadow: 0 8px 24px rgba(0,0,0,0.5); text-align: center; max-width: 480px; width: 90%; border: 1px solid #333; }";
  html += "h1 { font-size: 0.9rem; text-transform: uppercase; letter-spacing: 2px; color: #00e676; margin-bottom: 1rem; }";
  html += "p { font-size: 1.2rem; line-height: 1.6; color: #ffffff; min-height: 80px; margin-bottom: 0.5rem; }";
  html += ".original { font-size: 0.85rem; color: #888; font-style: italic; margin-bottom: 1.5rem; min-height: 20px; }";
  html += ".btn { background-color: #00e676; color: #121212; border: none; padding: 12px 24px; font-size: 1rem; font-weight: bold; border-radius: 6px; cursor: pointer; transition: 0.2s; }";
  html += ".btn:hover { background-color: #00b359; }";
  html += ".btn:disabled { background-color: #555; cursor: not-allowed; }";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<h1>Conselho #" + String(currentAdviceId) + "</h1>";
  html += "<p id='advice-pt'>Traduzindo conselho...</p>";
  html += "<div class='original' id='advice-en'>\"" + currentAdvice + "\"</div>";
  html += "<button class='btn' id='btn-reload' onclick='location.reload()'>Gerar Novo Conselho</button>";
  html += "</div>";

  // Script em JS executado no navegador para traduzir o texto
  html += "<script>";
  html += "async function translateAdvice() {";
  html += "  const textEn = " + String("\"") + currentAdvice + String("\"") + ";";
  html += "  const ptElement = document.getElementById('advice-pt');";
  html += "  try {";
  html += "    const response = await fetch('https://api.mymemory.translated.net/get?q=' + encodeURIComponent(textEn) + '&langpair=en|pt-BR');";
  html += "    const data = await response.json();";
  html += "    if(data.responseData && data.responseData.translatedText) {";
  html += "      ptElement.innerText = '\"' + data.responseData.translatedText + '\"';";
  html += "    } else {";
  html += "      ptElement.innerText = '\"' + textEn + '\"';";
  html += "    }";
  html += "  } catch (err) {";
  html += "    ptElement.innerText = '\"' + textEn + '\" (Falha na tradução)';";
  html += "  }";
  html += "}";
  html += "translateAdvice();";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("Endereço IP do ESP32: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor HTTP iniciado com sucesso.");
}

void loop() {
  server.handleClient();
}