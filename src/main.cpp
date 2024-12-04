#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>



void cls();
void recuperarUsuariosGrupo(int grupo);
void enviarPlaca();
String replaceSpaces(String);
String strNow();


/*Definimos nuestras credenciales de la red WiFi*/

// Hospital
const char* ssid = "WPROFESIONALES";
const char* pass = "temporal%";
TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", +1 * 3600);
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
    
    mensaje = "Ajustando hora." + strNow();
 

    tft.print(mensaje);
    Serial.println(mensaje);
    
    delay(1000);
    cls();
}

void loop() 
{
    enviarPlaca();

    for(;;); //lo dejamos colgado
    
}
void cls()
{
 tft.fillScreen(TFT_BLACK);
 tft.setTextColor(TFT_WHITE, TFT_BLACK); // imprimiremos en blanco sobre negro
 tft.setCursor(0,0);
}
void enviarPlaca()
{
    HTTPClient http;   
    // Iniciamos la conexión
    String mensaje;
    mensaje = replaceSpaces("Hay una petición de rayos en el control de urgencias"); 
    http.begin("http://intjk.es:27031/mensajeria/api/mcrearg");
    // Especificamos el header content-type
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Preparamos los datos POST
    String postData = "id_grupo=4&id_usuario_o=1&mensaje=" + mensaje;

    // Hacemos la petición POST
    int httpResponseCode = http.POST(postData);
    // Verificamos la respuesta
    if(httpResponseCode < 0)
        tft.print("Error enviando mensaje");
    else
        {
        tft.setTextSize(2);
        cls();
        if (httpResponseCode == 200)
        {
            tft.println("Mensaje enviado");
            tft.println("a las " + strNow());
            Serial.println(httpResponseCode);
        }
        }

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

String replaceSpaces(String str) 
{
  String result = "";
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == ' ') {
      result += "%20";
    } else {
      result += str[i];
    }
  }
  return result;
}
String strNow()
{
  timeClient.update();

  // Obtener la hora en formato Unix
  unsigned long epochTime = timeClient.getEpochTime();

  // Convertir a una estructura de tiempo
  struct tm *ptm = gmtime((time_t *)&epochTime);

  // Formatear la hora como un string
  char formattedTime[32];
  sprintf(formattedTime, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
return(formattedTime);
}

