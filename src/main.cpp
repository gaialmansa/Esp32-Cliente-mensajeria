#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>

void cls();
void recuperarUsuariosGrupo(int grupo);
void enviarPlaca();
String replaceSpaces(String);
String strNow();
void registrar();
int id_user = 0;

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
    delay(1000);
    if (id_user == 0)   // no tenemos usuario
        registrar();
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
void registrar() //  sacar un numero aleatorio por pantalla. Luego consulta el api hasta que devuelve un numero de usuario.
{
    int numero = random(100,999);
    tft.println(numero);
    
}
