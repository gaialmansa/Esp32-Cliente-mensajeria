#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


void cls();
void recuperarUsuariosGrupo(int grupo);

/*Definimos nuestras credenciales de la red WiFi*/

// Hospital
const char* ssid = "WPROFESIONALES";
const char* pass = "temporal%";
TFT_eSPI tft = TFT_eSPI();
void setup() 
{
    String mensaje;
    Serial.begin(9600);
    // Inicializar pantalla
    tft.init();
    tft.setRotation(1);  // Landscape
    tft.fillScreen(TFT_BLACK);
    
    // Configurar retroiluminación
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.setTextSize(1);
    tft.setTextFont(1); // usaremos 4 para texto y 8 para numeros grandes

    /*
    // Demostración de texto
    tft.setTextDatum(TC_DATUM);  // Top-Center
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Diferentes tamaños de texto
    tft.setTextSize(1);
    tft.drawString("Tamaño 1", 120, 0);
    tft.setTextSize(2);
    tft.drawString("Tamaño 2", 120, 20);
    
    // Diferentes colores
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED);
    tft.drawString("Texto Rojo", 120, 50);
    tft.setTextColor(TFT_GREEN);
    tft.drawString("Texto Verde", 120, 60);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Texto Azul", 120, 70);
    
    // Dibujar un marco
    tft.drawRect(0, 90, tft.width(), 45, TFT_WHITE);
    */
    cls();
    tft.println("Inicializando WiFI ");
    // inicializamos wifi
    WiFi.begin(ssid, pass);
    delay(2000);
    cls();
    mensaje = "Intentando conectar a "+String(ssid);
    tft.println(mensaje);
    Serial.println(mensaje);
    while (WiFi.status() != WL_CONNECTED) 
    {
     delay(500);
     tft.print(".");
     Serial.println("Aún no.");
    }
    mensaje = "Conectado a "+String(ssid);
    tft.println();
    tft.print(mensaje);
    Serial.println(mensaje);
    delay(1000);
    
    cls();
}

void loop() 
{
    recuperarUsuariosGrupo(4);
    for(;;);
   
}
void cls()
{
 tft.fillScreen(TFT_BLACK);
 tft.setTextColor(TFT_WHITE, TFT_BLACK); // imprimiremos en blanco sobre negro
 tft.setCursor(0,0);
}
void recuperarUsuariosGrupo(int grupo)
{
 HTTPClient http;
 int f;    
    // Iniciamos la conexión
    http.begin("http://intjk.es:27031/mensajeria/api/recuperarusuariosgrupo");
    //http.begin("http://192.168.10.152/mensajeria/api/recuperarusuariosgrupo");
    // Especificamos el header content-type
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Preparamos los datos POST
    String postData = "id_grupo="+ String(grupo); 
    // Hacemos la petición POST
    int httpResponseCode = http.POST(postData);
    // Verificamos la respuesta
    if(httpResponseCode > 0)
    {
      String response = http.getString();
      cls();
      tft.setTextFont(2);
      //tft.println(response);
      Serial.println(response);
      const size_t capacity = JSON_ARRAY_SIZE(10) * (JSON_OBJECT_SIZE(7) + 50); // Ajustar el tamaño según tu JSON
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, response);
Serial.println("Ya ha desserializado la respuesta");
        if(!error)
        {
            Serial.println("No error. ");
            
           JsonArray usuarios = doc.as<JsonArray>();
Serial.println(usuarios.size());
            for (JsonVariant usuario : usuarios) 
            {
                Serial.println("Estamos en el bucle");
                String id_grupo = usuario["id_grupo"];
                String id_usuario = usuario["id_usuario"];
                String id = usuario["id"];
                String usuario_nombre = usuario["usuario"];
                String nombre = usuario["nombre"];
                String observaciones = usuario["observaciones"];
                String grupo = usuario["grupo"];

                Serial.print("ID Grupo: ");
                Serial.println(id_grupo);
                Serial.println(nombre);
                tft.println(nombre);
                // ... (Imprimir otros campos)
            }
        }
        else
        {
            tft.println("Se ha producido un error");
            Serial.println("Se ha producido un error");
            Serial.println(response);
        }
    }
    else 
    {
      Serial.println("Error en petición HTTP: " + String(httpResponseCode));
      tft.println("Error en petición HTTP: " + String(httpResponseCode));
    }
    
    // Liberamos recursos
    http.end();
}