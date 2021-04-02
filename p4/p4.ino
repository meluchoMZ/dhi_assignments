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
#define BANANAS_CHAR 2
#define TIGER_CHAR 3
#define TRAP_CHAR 4
#define TRAPPED_MONKE_CHAR 5
#define BLANK ' '
#define MONKE "M O N K E Y"
#define DELAY 1000/8

void update_time(void);
void clear_lcd(void);
void update_level(void);
void greetings(void);
void happy(void);
void trapped(void);
void died(void);
void reset_game(void);

// inicio lcd e asignación de pins
LiquidCrystal lcd(8,9,5,4,3,2);

const byte monke[8] = {0b01110, 0b01110, 0b00100, 0b01110, 0b10101, 0b00100, 0b01010, 0b10001};
const byte bananas[8] = {0b10101, 0b10101, 0b10101, 0b00000, 0b10101, 0b10101, 0b10101, 0b00000};
const byte tiger[8] = {0b10001, 0b01110, 0b10001, 0b10101, 0b00100, 0b10001, 0b01010, 0b00100};
const byte trap[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
const byte trapped_monke[8] = {0b10001, 0b10001, 0b11011, 0b10001, 0b01010, 0b11011, 0b10101, 0b01110};
const byte traps_vx[10] = {2, 4, 5, 7, 8, 9, 11, 12, 13, 14};
byte traps_vy[10];
byte monke_vx;
byte monke_vy;
bool banana_vy;
bool is_trapped;
byte time_left;
byte cicles;

void setup(void)
{
	// configuracion de pins
	pinMode(SW, INPUT_PULLUP);
	// configuración do lcd
	lcd.createChar(MONKE_CHAR, monke);
	lcd.createChar(BANANAS_CHAR, bananas);
	lcd.createChar(TIGER_CHAR, tiger);
	lcd.createChar(TRAP_CHAR, trap);
	lcd.createChar(TRAPPED_MONKE_CHAR, trapped_monke);
	lcd.begin(16,2);
	analogWrite(LCD_CONTRAST, 60);
	greetings();
	while (digitalRead(SW) != LOW) {
		;
	}
	clear_lcd();
	reset_game();
}

void loop(void)
{
	if (monke_vx == 0xF) {
		if (monke_vy == banana_vy) {
			happy();
		} else {
			died();
		}
		while (digitalRead(SW) != LOW) {
			;
		}
		reset_game();
	}
	if (cicles == 0x7) {
		cicles = 0x0;
		if (time_left == 0x0) {
			died();
			while (digitalRead(SW) != LOW) {
				;
			}
			reset_game();
		} else {
			time_left--;
		}
		update_time();
		update_level();
	}
	cicles++;
	if (!is_trapped) {
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
		for (byte b = 0x0; b < 0xA; b++) {
			if (monke_vx == traps_vx[b] && monke_vy == traps_vy[b]) {
				is_trapped = true;
				break;
			}
		}
		if (is_trapped) {
			lcd.write(TRAPPED_MONKE_CHAR);
		} else {
			lcd.write(MONKE_CHAR);
		}
	}
	lcd.setCursor(15, banana_vy);
	lcd.write(BANANAS_CHAR);
	lcd.setCursor(15, !banana_vy);
	lcd.write(TIGER_CHAR);
	delay(DELAY);
}

void reset_game(void)
{
	monke_vx = 0b1;
	monke_vy = 0b1;
	time_left = 0x1E;
	cicles = 0x0;
	is_trapped = false;
	clear_lcd();
	banana_vy = 0;
	for (byte b = 0x0; b < 0xA; b++) {
		traps_vy[b] = random(1,3)>1;
	}
	update_time();
	update_level();
}

void update_time(void)
{
	lcd.setCursor(0,0); 
	lcd.print(time_left/10);
	lcd.setCursor(0,1);
	lcd.print(time_left-time_left/10*10);
}

void update_level(void)
{
	for (byte b = 0x0; b < 0xA; b++) {
		lcd.setCursor(traps_vx[b], 0);
		lcd.print(BLANK);
		lcd.setCursor(traps_vx[b], 1);
		lcd.print(BLANK);
		traps_vy[b] = random(1,3)>1;
		lcd.setCursor(traps_vx[b], traps_vy[b]);
		if (is_trapped && monke_vx == traps_vx[b] && monke_vy != traps_vy[b]) {
			is_trapped = false;
		}
		lcd.write(TRAP_CHAR);
	}
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
