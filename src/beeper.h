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
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include <esp_pm.h>
#include "driver/gpio.h"

#include <local.h>


#define UP_PIN 20           //pines del Joystick
#define DOWN_PIN 9
#define PUSH_PIN 21
#define WAKEPIN GPIO_NUM_21 // configuramos el pulsador del boton para salir del Light Sleep mode
#define VIBRADOR GPIO_NUM_0 // pin al que está conectado el vibrador

void cls();
void recuperarUsuariosGrupo(int grupo);
void enviarPlaca();
void obtenerMensajes();
void Api(char metodo[], String parametros[], int numparam);
void regSys();
void upISR();
void downISR();
void pushISR();
void despertarTFT();
void dormirTFT();
void dormir();
void chkMsg();
void vibrar();
void hayMensajeNuevo();
void iniScreen();
void WiFiStart();
void tftPrint(String);
String replaceSpaces(String);
String strNow();
void aiMenu();
void listarMensajes();
void imprimirMensaje(bool);
void aitMenu();
void flechasEnable();
void flechasDisable();


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
volatile unsigned long lastDebounceTime[3] = { 0, 0, 0};
const unsigned long debounceDelay = 200;  // Ajusta este valor según necesites
int opt = 0;            // opciones en el menu
esp_sleep_wakeup_cause_t eventoWake; // Causa de la salida del sueño ligero
