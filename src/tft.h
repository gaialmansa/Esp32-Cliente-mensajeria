#define ST7735
//#define ST7789

// Para ST7735S
#ifdef ST7735
  #define TFT_SLPIN   0x10
  #define TFT_SLPOUT  0x11
  #define INVERT false
  #define TFT_WIDTH  128
  #define TFT_HEIGHT 160
#endif

// Para ST7789
#ifdef ST7789
  #define TFT_SLPIN   0x10
  #define TFT_SLPOUT  0x11
  #define INVERT true
  #define TFT_WIDTH  240
  #define TFT_HEIGHT 320
#endif

void iniScreen()
{
    pinMode(TFT_BL, OUTPUT);  // pin para la iluminacion de la pantalla. Lo manejaremos con tftOn y tftOff  
    tft.init();
    tft.setRotation(1);  // 0 Portrait/ 1 Landscape
    tft.invertDisplay(INVERT);
    tft.setTextWrap(true);  // Habilita el ajuste de texto
    tft.setTextSize(1);
    cls();
    despertarTFT();              // terminamos de poner a punto la pantalla tft
    delay(200);
}
/**
 *  Despierta el chip de la TFT
 */
void despertarTFT()
{
  // Configurar retroiluminación. En realidad es encender o apagar la pantalla
  digitalWrite(TFT_BL, HIGH); // poniendolo en HIGH encendemos la pantalla
  tft.writecommand(TFT_DISPON);      // Encender la pantalla
  tft.writecommand(TFT_SLPOUT);      // Sacar controlador del modo sueño
  delay(120); // Esperar a que la pantalla se estabilice

}
/**
 *  Borrado de pantalla
 */
void cls()
{
 tft.setTextColor(TFT_WHITE, TFT_BLACK); // imprimiremos en blanco sobre negro
 tft.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
}
/**
 * Imprime una cadena en la tft. Si es demasiado ancha, la corta por donde haya un espacio 
 * e imprime el resto en la linea siguiente.
 * Siempre termina con CRLF
 */
void tftPrint(String message)
{
  #define ANCHOMENSAJE 27
  int lineHeight = 16;  // ajusta según tu tamaño de fuente
  
  while (message.length() > 0) 
  {
      //int charsWidth = tft.height() / (6 * tft.textsize);
      String line;
      
      if (message.length() > ANCHOMENSAJE) 
      {
          int lastSpace = message.substring(0, ANCHOMENSAJE).lastIndexOf(' ');
          if (lastSpace == -1) lastSpace = ANCHOMENSAJE;
          
          line = message.substring(0, lastSpace);
          message = message.substring(lastSpace + 1);
      } 
      else 
      {
          line = message;
          message = "";
      }
      
      tft.println(line);
      
  }
}