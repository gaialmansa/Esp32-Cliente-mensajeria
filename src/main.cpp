
#include <beeper.h>
#include <tft.h>
#include <WiFibeeper.h>

void setup() 
{
    String mensaje;

    // Inicializar pantalla
    iniScreen();
    pinMode(TFT_BL, OUTPUT);  // pin para la iluminacion de la pantalla. Lo manejaremos con tftOn y tftOff  
    // inicializamos wifi
    WiFiStart();
    // Puesta en hora
    timeClient.setTimeOffset(3600);
    timeClient.begin();
    if (!timeClient.update()) 
      tftPrint("Error al sincronizar NTP: SSL/conexion fallida");
    mensaje = "Ajustando hora:" + strNow();
    // Imprimimos la MAC en la pantalla
    cls();
    tft.println("MAC: ");
    tftPrint(WiFi.macAddress());
    apicall.setTimeout(5000);   // establecemos el tiempo maximo antes de dar un timeout al conectar con el API
    regSys();     // Chequeamos hasta que esta registrado en el sistema
    cls();       // y borramos la pantalla 

    // configuracion de los pines de entrada del mando y el vibrador
    pinMode(UP_PIN, INPUT_PULLUP);
    pinMode(DOWN_PIN, INPUT_PULLUP);
    pinMode(PUSH_PIN, INPUT_PULLUP);
    pinMode(VIBRADOR,OUTPUT);
    
    // Configurar el temporizador para despertar en 5 segundos
    esp_sleep_enable_timer_wakeup(5 * 1000000);
    gpio_wakeup_enable(WAKEPIN, GPIO_INTR_LOW_LEVEL); // Habilitamos el wake up por GPIO
    esp_sleep_enable_gpio_wakeup(); // Habilitamos el wake up por tiempo
    attachInterrupt(PUSH_PIN, pushISR, FALLING);    // pulsador
}
void loop() 
{
   chkMsg();    // consulta la API para ver si hay mensajes nuevos para el usuario
   dormir();  // apagamos la pantalla y ponemos el procesador en modo light sleep durante 5 segundos
}
void chkMsg() // Consulta el API para ver si hay mensajes no vistos para el usuario. Si hay varios mensajes pendientes, los recupera todos.
{
  String User[1];
  User[0] = "id_usuario=" + String(user);
  while (true)
  {
  Api("mnv",User,1);  // llamamos al api para recuperar el primer mensaje sin leer del usuario
  if(doc.containsKey("mensaje") ) // esto es que hay mensaje
   {
    hayMensajeNuevo();
    continue;
   }
  if (eventoWake == 7 or pulsado) // se desperto por pulsacion del boton o se pulso el boton cuando estaba despierto
   {
    listarMensajes();
    pulsado = false;
   }
   
  return;
  }
  
}
void hayMensajeNuevo() //enciende la pantalla y enseña el mensaje que se acaba de leer. Al terminar lo marca en la API como leido.Se queda en bucle hasta que se pulsa el joystick
{
  int anterior;
  flechasEnable();  // Habilitamos las teclas arriba y abajo
  pulsado = false;
  while (!pulsado)  //se queda vibrando hasta que se pulse
    vibrar();
  despertarTFT();  //despierta la pantalla
  cls();
  imprimirMensaje(false); // flag false para mensaje nuevo 
  aiMenu();
  anterior =0;  // opcion anteriormente seleccionada
  pulsado = false;
  while(!pulsado) // se queda bloqueado aqui hasta que pulse el mando
  {
    if (opt != anterior)  // se pulso arriba o abajo
    {
      aiMenu();
      anterior = opt;
    } 
    delay(100);
  }
  String parms[1];
  parms[0] = "id=" + doc["id"].as<String>();
  Api("mver",parms,1);
  if(opt == 0) // aceptar
  {
    //if ()
    Api("matender",parms,1);   // si se ignora, no hace nada
  }
  cls(); 
  flechasDisable();
}
void vibrar() //Activa el motor excéntrico
{
  digitalWrite(VIBRADOR,HIGH);  // conectamos el zumbador
  delay(200);
  digitalWrite(VIBRADOR,LOW); // Paramos el zumbador
  delay(200);
}
void aiMenu() //Gestiona el menu Atender/Ignorar
{
  if (opt < 0)
    opt = 1;
  if (opt > 1)
    opt = 0;
  if(opt == 0)
  {
    tft.fillRect(0,109,54,8,TFT_WHITE); // fondo blanco
    tft.setTextColor(TFT_BLACK);
    tft.drawString("<Aceptar>",0,109);
    tft.fillRect(0,118,54,8,TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Ignorar>",0,118);
  }
  else
  {
    tft.fillRect(0,109,54,8,TFT_BLACK); // fondo negro
    tft.drawString("<Aceptar>",0,109);
    tft.fillRect(0,118,54,8,TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("<Ignorar>",0,118);
    tft.setTextColor(TFT_WHITE);
  }
  
}
void aitMenu() //Gestiona el menu Atender/Ignorar/Terminar
{
  if (opt < 0)
    opt = 2;
  if (opt > 2)
    opt = 0;
  if(opt == 0)
  {
    tft.fillRect(0,100,54,8,TFT_WHITE); // fondo blanco
    tft.setTextColor(TFT_BLACK);
    tft.drawString("<Aceptar>",0,100);
    tft.fillRect(0,109,54,8,TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Ignorar>",0,109);
    tft.fillRect(0,118,54,8,TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Terminar>",0,118);
  }
  if(opt == 1)
  {
    tft.fillRect(0,100,54,8,TFT_BLACK); // fondo negro
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Aceptar>",0,100);
    tft.fillRect(0,109,54,8,TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("<Ignorar>",0,108);
    tft.fillRect(0,118,54,8,TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Terminar>",0,118);
  }
  if(opt == 2)
  {
    tft.fillRect(0,100,54,8,TFT_BLACK); // fondo negro
    tft.setTextColor(TFT_WHITE);
    tft.drawString("<Aceptar>",0,100);
    tft.fillRect(0,109,54,8,TFT_BLACK);
    tft.drawString("<Ignorar>",0,109);
    tft.fillRect(0,118,54,8,TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("<Terminar>",0,118);
    tft.setTextColor(TFT_WHITE);
  }
  
}
void imprimirMensaje(bool $pendiente) // Imprime el mensaje leido del Api en la tft
{
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);  // Yelow= azul claro. White=blanco. Blue = rojo, Brown = azul. Black = marror muy oscuro (ilegible)
  if($pendiente)
    tft.drawString("MENSAJE PENDIENTE",0,0);  
  else
    tft.drawString("MENSAJE NUEVO",0,0);  //Caset = rojo oscuro . Cyan = amarillo . DarkCyan = amarillo oscuro.Magenta = magenta. Green=verde
  tft.setTextSize(1);                   //Orange = azul claro. Pink = morado claro
  tft.drawFastHLine(0, 17, 160, TFT_YELLOW);
  tft.setTextColor(TFT_PINK);
  tft.setCursor(0,30);
  tft.print("De ");
  tftPrint(doc["origen"].as<String>());
  tft.setTextColor(TFT_CYAN);
  tftPrint(doc["mensaje"].as<String>());
  tft.setTextColor(TFT_PINK);
  tft.print("Enviado a las ");
  String timestamp = doc["hora"].as<String>();
  tftPrint(timestamp.substring(11,19));
  tft.print("Recibido a las ");
  tft.println(strNow());
  tft.println();
  
}
void listarMensajes() // Se activa cuando se pulsa el boton con la pantalla apagada. Muestra los mensajes no atendidos.
{
  int anterior;
  int offset = 0;   // el offset para recuperar los no leidos. 0 es el mas antiguo.
  String parms[2];
  despertarTFT();  //despierta la pantalla
  cls();
  parms[0] = "id_usuario="+user;
  parms[1] = "offset="+0;
  Api("mrpnat",parms,2);  // llamamos a la api para recuperar el primer mensaje no atendido
  imprimirMensaje(true);  // lo sacamos por la tft. Flag de antiguo a true.
  aitMenu();
  anterior =0;  // opcion anteriormente seleccionada
  pulsado = false;
  //attachInterrupt(PUSH_PIN, pushISR, FALLING);    // pulsador
  flechasEnable();  // habilitamos las teclas arriba y abajo

  while(!pulsado) // se queda bloqueado aqui hasta que pulse el mando
  {
    if (opt != anterior)  // se pulso arriba o abajo
    {
      aitMenu();
      anterior = opt;
    } 
    delay(100);
  }
  
  parms[0] = "id=" + doc["id"].as<String>();
  if(opt == 0) // aceptar
  {
    //if ()
    Api("matender",parms,1);   // si se ignora, no hace nada
  }
  cls(); 
  flechasDisable(); // deshabilitamos las interrupciones de arriba y abajo
}
void dormir() //Pone el dispositivo en modo Light Sleep. Al despertar, reinicia la wifi y la tft
{
  dormirTFT();
  // Primero desconectamos WiFi pero mantenemos la configuración
  WiFi.disconnect(false);
  delay(100);
  esp_wifi_stop();
  gpio_wakeup_enable(WAKEPIN, GPIO_INTR_LOW_LEVEL); // Habilitamos el wake up por GPIO
  gpio_hold_en(GPIO_NUM_6); // preservamos el contenido del gpio_4. Con esto consigo que la pantalla no se quede blanca durante la hibernacion
  eventoWake = (esp_sleep_wakeup_cause_t)0; // borramos el contenido anterior. Hay que hacer un cast al tipo especifico de dato
  esp_light_sleep_start();          // ponemos el ESP32 en light sleep para 5 segundos. En el setup hemos configurado que despierte al pulsar el boton del joystick
  eventoWake = esp_sleep_get_wakeup_cause(); // identificamos por qué salio del sueño ligero
  gpio_hold_dis(GPIO_NUM_6);
  // Al despertar, reiniciamos el WiFi
  esp_wifi_start();
  delay(100);
  // Reconectamos
  WiFi.begin(ssid, pass);
  // Esperamos a la conexión con timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) 
  {
      delay(100);
  }
}
void flechasEnable() //Habilita el manejo de los botones arriba y abajo
{
    attachInterrupt(UP_PIN, upISR, FALLING);
    attachInterrupt(DOWN_PIN, downISR, FALLING);
}
void flechasDisable() //Deshabilita el manejo de los botones arriba y abajo
{
    detachInterrupt(UP_PIN);
    detachInterrupt(DOWN_PIN);
}
void regSys() //Registra al usuario en el sistema.
{
  String nombre,usuario = "null";
  String mac[1];
  mac[0] = "mac=";
  mac[0] += WiFi.macAddress();
  tftPrint("Registrando el dispositivo ");
  tft.println(mac[0]);
  while (usuario == "null") // repetiremos hasta que el usuario tenga un valor
  {
    Api("bregister",mac,1);
    usuario = doc["usuario"].as<String>();
    nombre = doc["nombre" ].as<String>();
    user = doc["id_usuario"].as<int>();
    #ifdef _DEBUG
    Serial.println("No registrado.");
    #endif
    delay(1000);  // esperamos un segundo
  }
  tftPrint("**");
  tft.println();
  tftPrint("Registrado para el usuario " + nombre);
  #ifdef _DEBUG
  Serial.printf("Registrado para el usuario %d\n %s", user, nombre.c_str());
  #endif
} 
void Api(char metodo[], String parametros[],int numparam) //Hace una llamada al metodo del Api indicado con los parametros que van en el array Los parametros van en formato 'paramname=paramvalue'
{
   int f, responsecode;
  String postData, url, payload;
  DeserializationError error;   // por si esta mal formada la respuesta

  postData = parametros[0];
  if(numparam >1)
  {
    for (f = 1; f < numparam; f++) // concatenamos los elementos pasados en parametros en el formato parm1=valor1&param2=valor2....
      {
        postData += "&";
        postData += parametros[f];
      }
  }
  url = _URL;
  url += metodo; // con esto queda formada la url del API con su metodo al final
  #ifdef _DEBUG
  Serial.println(postData);
  Serial.println(url);
  #endif
  apicall.begin(url);    // iniciamos la llamada al api
  // Especificamos el header content-type
  apicall.addHeader("Content-Type", "application/x-www-form-urlencoded");
  responsecode = apicall.POST(postData);  
    if(responsecode != 200)
    {
      tft.printf("Ha habido un error %d al comunicar con el api.\n",responsecode);
      tft.println(url);
      tft.println(postData);
      #ifdef _DEBUG
      Serial.printf("Ha habido un error %d al comunicar con el api.\n",responsecode);
      apiError = true;
      #endif

    }
  payload = apicall.getString();
  error = deserializeJson(doc,payload);   // deserializamos la respuesta y la metemos en el objeto doc
  /*#ifdef _DEBUG
  Serial.print("payload: ");
  Serial.println(payload);
    if (error) 
    {
    tft.print("Error al parsear JSON: ");
    tft.println(error.f_str());
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.f_str());
    Serial.println(payload);
    return;
    }
  #endif*/
}
void recuperarUsuariosGrupo(int grupo) //Recupera la lista de usuarios de un grupo dado
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
String replaceSpaces(String str) //Reemplaza los espacios en str por %20
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
String strNow() //Recupera la hora actual y devuelve un string para imprimir
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
void IRAM_ATTR upISR() // ISR up
{
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime[0] > debounceDelay) 
  {
    lastDebounceTime[0] = currentTime;
    opt --;
  }
}
void IRAM_ATTR downISR() // ISR down
{
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime[1] > debounceDelay) 
  {
    lastDebounceTime[1] = currentTime;
    opt ++;
  }
}
void IRAM_ATTR pushISR() // ISR push
{
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime[4] > debounceDelay) 
  {
    pulsado = true;
    lastDebounceTime[2] = currentTime;
  }

}
