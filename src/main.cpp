#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <local.h>
#define _URL "https://hospital.almansa.ovh/api/"


void cls();
void recuperarUsuariosGrupo(int grupo);
void enviarPlaca();
void Api(char metodo[], char *parametros[], int numparam);

String replaceSpaces(String);
String strNow();


/*Las credenciales de la red WiFi están definidas en el fichero local.h, que está en gitignore.*/


TFT_eSPI tft = TFT_eSPI();  // inicializamos pantalla
WiFiUDP ntpUDP;             // instanciamos WiFi
NTPClient timeClient(ntpUDP, "pool.ntp.org", +1 * 3600);  // Cliente NTP para ajustar la hora
HTTPClient apicall;         // cliente HTTP para llamar al API   
DynamicJsonDocument doc(512); // calculamos medio k para la respuesta del api

void setup() 
{
    String mensaje;
    #ifdef _DEBUG
    Serial.begin(9600);
    #endif
    // Inicializar pantalla
    tft.init();
    tft.setRotation(1);  // 0 Portrait/ 1 Landscape
    tft.fillScreen(TFT_BLACK);
    
    // Configurar retroiluminación
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.setTextSize(1);
    tft.setTextFont(4); // usaremos 4 para texto y 8 para numeros grandes

    
    //cls();
    tft.println("MAC: ");
    tft.println(WiFi.macAddress());
    tft.println("Inicializando WiFI ");
    // inicializamos wifi
    WiFi.begin(ssid, pass);
    delay(2000);
    mensaje = "Intentando conectar a "+String(ssid);
    tft.println(mensaje);

    #ifdef _DEBUG
    Serial.println(mensaje);
    #endif
    
    while (WiFi.status() != WL_CONNECTED) 
    {
     delay(500);
     tft.print(".");
     #ifdef _DEBUG
     Serial.println("Aún no.");
     #endif
    }
    mensaje = "Conectado a "+String(ssid);
    tft.println(mensaje);
    Serial.println(mensaje);
    timeClient.setTimeOffset(3600);
    timeClient.begin();
    if (!timeClient.update()) 
      tft.println("Error al sincronizar NTP: SSL/conexion fallida");
        
    mensaje = "Ajustando hora." + strNow();
 
    tft.println(mensaje);

    #ifdef _DEBUG
    Serial.println(mensaje);
    #endif

    delay(1000);
    //cls();
}

void loop() 
{
    //enviarPlaca();
    cls();
    char *parms[] = {"parametro1=valor1", "parametro2=valor2", "parametro3=valor3"};
    
    Api("Caca", parms, 3);

    for(;;); //lo dejamos colgado
    
}
/**
 *  Borrado de pantalla
 */
void cls()
{
 tft.fillScreen(TFT_BLACK);
 tft.setTextColor(TFT_WHITE, TFT_BLACK); // imprimiremos en blanco sobre negro
 tft.setCursor(0,0);
}
/**
 * Api
 * Hace una llamada al metodo del Api indicado con los parametros que van en el array
 * Los parametros van en formato 'paramname=paramvalue'
 */
void Api(char metodo[], char *parametros[],int numparam)
{
  int f, responsecode;
  char postData[250]; // suponemos que sera suficiente para el conjunto de parametros a pasar al api
  char url[250];
  String payload;
  DeserializationError error;   // por si esta mal formada la respuesta

  strcpy(postData, parametros[0]);
  if(numparam >1)
  {
    for (f = 1; f < numparam; f++) // concatenamos los elementos pasados en parametros en el formato parm1=valor1&param2=valor2....
      {
        strcat(postData,"&") ;
        strcat(postData, parametros[f]);
      }
      #ifdef _DEBUG
      Serial.print(postData);
      #endif
  }
  strcat(url, _URL);
  strcat(url,metodo); // con esto queda formada la url del API con su metodo al final
  apicall.begin(url);    // iniciamos la llamada al api
  // Especificamos el header content-type
  apicall.addHeader("Content-Type", "application/x-www-form-urlencoded");
  responsecode = apicall.POST(postData);  
  if(responsecode != 200)
    {
      tft.printf("Ha habido un error %d al comunicar con el api.",responsecode);
    }
  #ifdef _DEBUG
  Serial.print(responsecode);
  #endif
  payload = apicall.getString();
  error = deserializeJson(doc,payload);   // deserializamos la respuesta y la metemos en el objeto doc
  #ifdef _DEBUG
    if (error) 
    {
    tft.print("Error al parsear JSON: ");
    tft.println(error.f_str());
    return;
    }
  #endif
}


/**
 *  Envía un mensaje fijo. Esta funcion es para estar en sitios como la bandeja de Rx del control de enfermeria o en triaje
 *  TODO: hay que sacar la comunicacion con el cliente http y abstraerla en una funcion a que tome la URL de un #define en una cabecera
 *        y se pase el método del API  y los argumentos como parámetro, y que devuelva la respuesta del API como un array.
 */
void enviarPlaca()
{
    HTTPClient http;   
    // Iniciamos la conexión
    String mensaje;
    mensaje = replaceSpaces("Hay una petición de rayos en el control de urgencias"); 
    http.begin("https://hospital.almansa.ovh/api/mcrearg");
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
/*
*   Recupera la lista de usuarios de un grupo dado
*/
void recuperarUsuariosGrupo(int grupo)
{
 HTTPClient http;
 int f;    
    // Iniciamos la conexión
    http.begin("https://hospital.almansa.ovh/api/recuperarusuariosgrupo");
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
      tft.println(response);
      #ifdef _DEBUG
      Serial.println(response);
      #endif
      const size_t capacity = JSON_ARRAY_SIZE(10) * (JSON_OBJECT_SIZE(7) + 50); // Ajustar el tamaño según tu JSON
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, response);
      #ifdef _DEBUG
      Serial.println("Ya ha desserializado la respuesta");
      #endif
        if(!error)
        {
            JsonArray usuarios = doc.as<JsonArray>();
            #ifdef _DEBUG
            Serial.println("No error. ");
            Serial.println(usuarios.size());
            #endif
            for (JsonVariant usuario : usuarios) 
            {
                #ifdef _DEBUG
                Serial.println("Estamos en el bucle");
                #endif
                String id_grupo = usuario["id_grupo"];
                String id_usuario = usuario["id_usuario"];
                String id = usuario["id"];
                String usuario_nombre = usuario["usuario"];
                String nombre = usuario["nombre"];
                String observaciones = usuario["observaciones"];
                String grupo = usuario["grupo"];
                #ifdef _DEBUG
                Serial.print("ID Grupo: ");
                Serial.println(id_grupo);
                Serial.println(nombre);
                #endif
                tft.println(nombre);
                // ... (Imprimir otros campos)
            }
        }
        else
        {
            tft.println("Se ha producido un error");
            #ifdef _DEBUG
            Serial.println("Se ha producido un error");
            Serial.println(response);
            #endif
        }
    }
    else 
    {
      #ifdef _DEBUG
      Serial.println("Error en petición HTTP: " + String(httpResponseCode));
       #endif
      tft.println("Error en petición HTTP: " + String(httpResponseCode));
    }
    
    // Liberamos recursos
    http.end();
}
/*
*   Reemplaza los espacios en str por %20
*/
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
/**
 *  Recupera la hora actual y devuelve un string para imprimir
 */
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

