#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include <esp_pm.h>

#include <local.h>


#define nmensajes 7         // numero de mensajes que se recuperan por defecto. Como depende de la pantalla, lo dejamos aqui.
#ifdef C3
    #define UP_PIN 14           //pines del Joystick
    #define DOWN_PIN 13
    #define LEFT_PIN 12
    #define RIGHT_PIN 26
    #define PUSH_PIN 27
    #define WAKEPIN GPIO_NUM_27 // configuramos el pulsador del boton para salir del Light Sleep mode
#else
    #define UP_PIN 0           //pines del Joystick
    #define DOWN_PIN 1
    #define LEFT_PIN 8
    #define RIGHT_PIN 9
    #define PUSH_PIN 3
    #define WAKEPIN GPIO_NUM_3
#endif



void cls();
void recuperarUsuariosGrupo(int grupo);
void enviarPlaca();
void obtenerMensajes();
void Api(char metodo[], String parametros[], int numparam);
void regSys();
void upISR();
void downISR();
void leftISR();
void rightISR();
void pushISR();
void despertarTFT();
void dormir();
void chkMsg();
void hayMensajeNuevo();
void iniScreen();
void WiFiStart();
void tftPrint(String);
String replaceSpaces(String);
String strNow();
void aiMenu(int);


/*Las credenciales de la red WiFi están definidas en el fichero local.h, que está en gitignore.*/


TFT_eSPI tft = TFT_eSPI();  // inicializamos pantalla
WiFiUDP ntpUDP;             // instanciamos WiFi
NTPClient timeClient(ntpUDP, "pool.ntp.org", +1 * 3600);  // Cliente NTP para ajustar la hora
HTTPClient apicall;         // cliente HTTP para llamar al API   
DynamicJsonDocument doc(1024); // calculamos un k para la respuesta del api
int offset = 0;        // offset para mostrar mensajes, Inicialmente es cero
int user = 0;            // id del usuario registrado
bool pulsado = false;     // cambia de valor cuando se pulsa el joystick
bool apiError = false;    // flag de error en la llamada al api
// Variables para manejar el debounce
volatile unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0};
const unsigned long debounceDelay = 200;  // Ajusta este valor según necesites
int opt = 0;            // opciones en el menu
int totalOpciones = 1;   // opcion mas alta en el menu ( notese que empieza por cero)