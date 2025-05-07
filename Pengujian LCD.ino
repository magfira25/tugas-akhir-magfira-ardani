#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4); 

void setup(){
  lcd.init();
  lcd.backlight();
}

void loop(){
  lcd.setCursor(0,0);
  lcd.print("MAGFIRA ARDANI");
  lcd.setCursor(0,1);
  lcd.print("21104470001");
  lcd.setCursor(0,2);
  lcd.print("FISIKA");
  lcd.setCursor(0,3);
  lcd.print("UNIVERSITAS ANDALAS");

}
