void WiFiStart()
{
    tft.println("Inicializando WiFI ");
    String mensaje;
    // Configurar WiFi en modo WIFI_PS_MIN_MODEM para mejor estabilidad
    WiFi.mode(WIFI_STA);
     // Configurar WiFi para permitir modem-sleep
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    WiFi.begin(ssid, pass);
    delay(2000);
    mensaje = "Intentando conectar a "+String(ssid);  // los parametros de la conexion se leen en local.h
    tftPrint(mensaje);
    delay(5000);
    #ifdef _DEBUG
    Serial.println(mensaje);
    #endif
    
    while (WiFi.status() != WL_CONNECTED) 
    {
     delay(500);
     tft.print(".");
     }
    mensaje = "Conectado a "+String(ssid);
    tftPrint(mensaje);
}