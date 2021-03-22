/**
  Videoxogo para Arduino: Monkey Traps
  Autor: Miguel Blanco Godón
  */

#include <LiquidCrystal.h>

#define VRx A0
#define VRy A1
#define SW A2
#define LCD_CONTRAST 6
#define MONKE_CHAR 1 
#define BLANK ' '
#define MONKE "M O N K E Y"

void clear_lcd(void);
void greetings(void);
void happy(void);
void trapped(void);
void died(void);

// inicio lcd e asignación de pins
LiquidCrystal lcd(8,9,5,4,3,2);

const byte monke[8] = {0b01110,0b01110, 0b00100, 0b01110, 0b10101, 0b00100, 0b01010, 0b10001};
byte monke_vx = 0b1;
byte monke_vy = 0b0;

void setup(void)
{
	// configuracion de pins
	pinMode(SW, INPUT_PULLUP);
	// configuración do lcd
	lcd.createChar(MONKE_CHAR, monke);
	lcd.begin(16,2);
	analogWrite(LCD_CONTRAST, 60);
	greetings();
	while (digitalRead(SW) != LOW) {
		;
	}
	clear_lcd();
}

void loop(void)
{
	lcd.setCursor(monke_vx, monke_vy);
	lcd.write(BLANK);
	if (analogRead(VRx) < 413 && monke_vx > 0x1) {
		monke_vx--;
	}
	if (analogRead(VRx) > 611 && monke_vx < 0xF) {
		monke_vx++;
	}
	if (analogRead(VRy) < 413) {
		monke_vy = 0;
	}
	if (analogRead(VRy) > 611) {
		monke_vy = 1;
	}
	lcd.setCursor(monke_vx, monke_vy);
	lcd.write(MONKE_CHAR);
	delay(100);
}

void clear_lcd(void)
{
	for (byte b = 0x0; b < 0x10; b++) {
		lcd.setCursor(b, 0);
		lcd.print(BLANK);
		lcd.setCursor(b, 1);
		lcd.print(BLANK);
	}
}
void greetings(void)
{
	clear_lcd();
	lcd.setCursor(2, 0);
	lcd.print("MOKEY TRAPS");
	lcd.setCursor(1, 1);
	lcd.print("Press Joystick");
}

void happy(void)
{
	clear_lcd();
	lcd.setCursor(3, 0);
	lcd.print("H A P P Y");
	lcd.setCursor(2, 1);
	lcd.print(MONKE);
}

void trapped(void)
{
	clear_lcd();
	lcd.setCursor(1, 0);
	lcd.print("T R A P P E D");
	lcd.setCursor(2, 1);
	lcd.print(MONKE);
}

void died(void)
{
	clear_lcd();
	lcd.setCursor(4, 0);
	lcd.print("D I E D");
	lcd.setCursor(2, 1);
	lcd.print(MONKE);
}
