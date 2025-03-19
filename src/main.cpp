
#include <beeper.h>
#include <tft.h>
#include <WiFibeeper.h>

String borrame;


void setup() 
{
    String mensaje;
    // Inicializar pantalla
    iniScreen();
    cls();
    tft.println("MAC: ");
    tftPrint(WiFi.macAddress());
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
  
    apicall.setTimeout(5000);   // establecemos el tiempo maximo antes de dar un timeout al conectar con el API
    regSys();     // Chequeamos hasta que esta registrado en el sistema
    cls();       // y borramos la pantalla 

    // configuracion de los pines de entrada del mando y el vibrador
    pinMode(UP_PIN, INPUT_PULLUP);
    pinMode(DOWN_PIN, INPUT_PULLUP);
    pinMode(PUSH_PIN, INPUT_PULLUP);
    pinMode(VIBRADOR,OUTPUT);
    
    // Configurar el temporizador para despertar en 5 segundos
    //esp_sleep_enable_timer_wakeup(5 * 1000000);
    //gpio_wakeup_enable(WAKEPIN, GPIO_INTR_LOW_LEVEL); // Habilitamos el wake up por GPIO
    //esp_sleep_enable_gpio_wakeup(); // Habilitamos el wake up por tiempo
    attachInterrupt(PUSH_PIN, pushISR, FALLING);    // pulsador
}
void loop() 
{
  if (WiFi.status() != WL_CONNECTED)
    {
      //Serial.println("Error: Conexión WiFi perdida.");
      WiFiStart();
    }
   chkMsg();    // consulta la API para ver si hay mensajes nuevos para el usuario
   dormir();  // apagamos la pantalla y ponemos el procesador en modo light sleep durante 5 segundos
}
void chkMsg() // Consulta el API para ver si hay mensajes no vistos para el usuario. Si hay varios mensajes pendientes, los recupera todos.
{
  String User[1];
  User[0] = "id_usuario=" + String(user);
  while (true)
  {
  //Api("exrtime",User,1);
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
  String retApi;
  flechasEnable();  // Habilitamos las teclas arriba y abajo
  pulsado = false;
  while (!pulsado)  //se queda vibrando hasta que se pulse
    vibrar();
  despertarTFT();  //despierta la pantalla
  cls();
  imprimirMensaje(false,doc.as<JsonObject>()); // flag false para mensaje nuevo 
  aiMenu();
  String parms[1];
  parms[0] = "id=" + doc["id"].as<String>();
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
  String p[1];
  p[0] = "id_mensaje=" + doc["id_mensaje"].as<String>(); // con el doc de la lectura del mensaje, guardamos para la llamada de comprobacion
  Api("mver",parms,1);    // marca el mensaje como visto
  if(opt == 0) // aceptar
  {
    retApi = Api("mat",p,1); // chequeamos a ver si el mensaje ya lo ha aceptado otro usuario
    delay(1000); // se queda bloqueado aqui hasta que pulse el mando
    if ( retApi == "null" ) // esto es que el mensaje no ha sido atendido
      {
      Api("matender",parms,1);   // si se ignora, no hace nada
      }
    else
      {
      atendido();
      }
  }
  cls(); 
  flechasDisable();
  pulsado = false;
}
void atendido() //El mensaje ya ha sido atendido
{
  cls();
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);  // Yelow= azul claro. White=blanco. Blue = rojo, Brown = azul. Black = marror muy oscuro (ilegible)
  tft.setCursor(0,50);
  tftPrint("Otro usuario atendio el mensaje.");
  delay(2000);   
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
void noHayPendientes() // Imprime en pantalla que no hay mensajes pendientes
{
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW);  // Yelow= azul claro. White=blanco. Blue = rojo, Brown = azul. Black = marror muy oscuro (ilegible)
    tft.setCursor(0,50);
    tftPrint("No hay mensajes pendientes");
    delay(2000);    
}
void imprimirMensaje(bool $pendiente, JsonObject cdoc) // Imprime el mensaje leido del Api en la tft
{
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);  // Yelow= azul claro. White=blanco. Blue = rojo, Brown = azul. Black = marror muy oscuro (ilegible)
  if($pendiente)
    tft.drawString("MSG PENDIENTE",0,0);  
  else
    tft.drawString("MENSAJE NUEVO",0,0);  //Caset = rojo oscuro . Cyan = amarillo . DarkCyan = amarillo oscuro.Magenta = magenta. Green=verde
  tft.setTextSize(1);                   //Orange = azul claro. Pink = morado claro
  tft.drawFastHLine(0, 17, 160, TFT_YELLOW);
  tft.setTextColor(TFT_PINK);
  tft.setCursor(0,30);
  tft.print("De ");
  tftPrint(cdoc["origen"].as<String>());
  tft.setTextColor(TFT_CYAN);
  tftPrint(cdoc["mensaje"].as<String>());
  tft.setTextColor(TFT_PINK);
  tft.print("Enviado a las ");
  String timestamp = cdoc["hora"].as<String>();
  tft.println(timestamp.substring(11,19));
  tft.print("Recibido a las ");
  tft.println(strNow());
  //tft.println(borrame);
}
void listarMensajes() // Se activa cuando se pulsa el boton con la pantalla apagada. Muestra los mensajes no atendidos.
{
  int anterior,numeroMensajes, mensajeActual = 0;
  int offset = 0;   // el offset para recuperar los no leidos. 0 es el mas antiguo.
  String parms[2];
  despertarTFT();  //despierta la pantalla
  cls();
  parms[0] = "id_usuario="+(String)user;
  parms[1] += "offset=0";
  Api("mrnat",parms,2);  // llamamos a la api para recuperar el primer mensaje no atendido
  if( doc.isNull() ) // esto es que no hay mensaje
   {
    noHayPendientes();
    return;
   }
  JsonArray mensajesPendientes = doc.as<JsonArray>(); // El Api devuelve un array Json. Ahora esta en mensajesPendientes
  numeroMensajes = mensajesPendientes.size();
  while (mensajeActual < numeroMensajes)
  {
    cls();
    imprimirMensaje(true,mensajesPendientes[mensajeActual]);  // lo sacamos por la tft. Flag de antiguo a true.ponemos el mensaje en curso 
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
      Api("matender",parms,1);   // si se ignora, no hace nada
    
    if (opt == 1)  // ignorar
      mensajeActual++;
    if (opt == 3)   // terminar
      mensajeActual = numeroMensajes; // fin del bucle
  } 
  cls(); 
  flechasDisable(); // deshabilitamos las interrupciones de arriba y abajo
}
void dormir() //Pone el dispositivo en modo Light Sleep. Al despertar, reinicia la wifi y la tft
{
  dormirTFT();
  delay(1000);
  // Primero desconectamos WiFi pero mantenemos la configuración
  //WiFi.disconnect(false);
  //delay(100);
  //esp_wifi_stop();
  //gpio_wakeup_enable(WAKEPIN, GPIO_INTR_LOW_LEVEL); // Habilitamos el wake up por GPIO
  //gpio_hold_en(GPIO_NUM_6); // preservamos el contenido del gpio_4. Con esto consigo que la pantalla no se quede blanca durante la hibernacion
  //eventoWake = (esp_sleep_wakeup_cause_t)0; // borramos el contenido anterior. Hay que hacer un cast al tipo especifico de dato
  //esp_light_sleep_start();          // ponemos el ESP32 en light sleep para 5 segundos. En el setup hemos configurado que despierte al pulsar el boton del joystick
  //eventoWake = esp_sleep_get_wakeup_cause(); // identificamos por qué salio del sueño ligero
  //gpio_hold_dis(GPIO_NUM_6);
  // Al despertar, reiniciamos el WiFi
  //esp_wifi_start();
  //delay(100);
  // Reconectamos
  //WiFi.begin(ssid, pass);
  // Esperamos a la conexión con timeout
  //unsigned long startTime = millis();
  //while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) 
  //{
  //    delay(100);
  //}
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
  cls();
  tftPrint("Registrando el dispositivo ");
  tft.println(mac[0]);
  while (usuario == "null") // repetiremos hasta que el usuario tenga un valor
  {
    Api("bregister",mac,1);
    usuario = doc["usuario"].as<String>();
    nombre = doc["nombre" ].as<String>();
    user = doc["id_usuario"].as<int>();
    delay(1000);  // esperamos un segundo
  }
  tftPrint("**");
  tft.println();
  tftPrint("Registrado para el usuario " + nombre);
} 
String Api(char metodo[], String parametros[],int numparam) //Hace una llamada al metodo del Api indicado con los parametros que van en el array Los parametros van en formato 'paramname=paramvalue'
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
  borrame = postData;
  url = _URL;
  url += metodo; // con esto queda formada la url del API con su metodo al final
  apicall.begin(url);    // iniciamos la llamada al api
  // Especificamos el header content-type
  apicall.addHeader("Content-Type", "application/x-www-form-urlencoded");
  responsecode = apicall.POST(postData);  
  if(responsecode != 200)
    {
      tft.printf("Ha habido un error %d al comunicar con el api.\n",responsecode);
      tft.println(url);
      tft.println(postData);
    }
  payload = apicall.getString();
  error = deserializeJson(doc,payload);   // deserializamos la respuesta y la metemos en el objeto doc
  return(payload);
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
