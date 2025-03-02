
#define TFT_BL  6  // BLK pin
#define TFT_SLPOUT 11
#define TFT_SLPIN 10
#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define TFT_MOSI 10
#define TFT_SCLK 8
#define TFT_CS   5  // Chip select control pin
#define TFT_DC    3  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

void iniScreen()
{
  digitalWrite(TFT_BL, HIGH); // poniendolo en HIGH encendemos la pantalla
  tft.init();          // Inicializa la pantalla
  tft.setRotation(1);  // Ajusta la orientación (0, 1, 2, 3)
  tft.fillScreen(TFT_BLACK); // Limpia la pantalla con color negro
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Color del texto y fondo
  tft.setTextSize(1); // Tamaño del texto
  cls();
}
void dormirTFT()
{
  digitalWrite(TFT_BL, LOW); // poniendolo en low, apagamos la retroiluminacion
  cls();
  tft.writecommand(TFT_SLPIN);      // Poner controlador en modo sueño
}
void despertarTFT() //Despierta el chip de la TFT
{
  iniScreen();
}
void cls() // Borrado de pantalla
{
 tft.setTextColor(TFT_WHITE, TFT_BLACK); // imprimiremos en blanco sobre negro
 tft.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
 tft.setCursor(0,0);
}
void tftPrint(String message) //Imprime una cadena en la tft. Si es demasiado ancha, la corta por donde haya un espacio
{                             //e imprime el resto en la linea siguiente.Siempre termina con CRLF
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