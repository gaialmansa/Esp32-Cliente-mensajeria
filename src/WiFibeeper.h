void WiFiStart() // Inicializa la conexion
{
    tft.println("Inicializando WiFI ");
    String mensaje;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    delay(200);
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