/**
  Máquina de premios
  Dispositivos hardware e interfaces, enxeñaría de computadores
  Miguel Blanco Godón
  **/
#include <SPI.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define BUTTON 7 
#define LCD_CONTRAST 6
#define SS 10
#define LED_ROULETTE_MIN_TIME_MS 7000 // minimo tempo que duran os leds en ruleta
#define LED_ROULETTE_MAX_TIME_MS 8500 // maximo tempo que duran os leds en ruleta
#define LED_BLINK_DELAY 45 // retraso inicial dos leds da ruleta
#define EEPROM_VALID_BYTES 0x4 // numero valido de bytes da EEPROM, neste caso son 4 posto que son 4 premios e o máximo valor é 10
#define SPI_ERROR "Error de bus SPI"

// inicio lcd e asignación de pins
LiquidCrystal lcd(8,9,5,4,3,2);
// orde de alternancia dos leds
byte led_order[4] =  {0x1, 0x2, 0x4, 0x8};

bool fire = false; //indica se hai que executar o sorteo

void read_button(void);
void raffle(void);
void update_lcd(void);
void out_of_existences(void);
void clear_lcd(void);
void reset_EEPROM(void);


void setup(void)
{
	// semente pros números pseudoaleatorios
	randomSeed(analogRead(0));
	// configuración do botón de inicio
	pinMode(BUTTON, INPUT);
	//pinMode(BUTTON, LOW);
	// detección do reset
	if (digitalRead(BUTTON) == HIGH) {
		// reseteo de EEPROM
		reset_EEPROM();
	}
	// configuración LCD
	lcd.begin(16,2);
	analogWrite(LCD_CONTRAST,60);
	clear_lcd();
	update_lcd();
	// configuración de pins de SPI
	pinMode(SS, OUTPUT); // Slave Select
	digitalWrite(SS, LOW);
	SPI.begin();
	SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
	// transmitese 0xF para acender todos os leds xuntos
	SPI.transfer(0xF);
	digitalWrite(SS, HIGH);
	digitalWrite(SS, LOW);
	delay(500);
	// configuración do watch dog
	wdt_enable(WDTO_2S);
}

void loop(void)
{
	read_button();
	if (fire) {
		raffle();
		fire = false;
	}
	wdt_reset();
}

// le o valor do botón. Marca se se debe realizar o sorteo (FIRE)
void read_button(void)
{
	static unsigned long starttime = 0;
	static boolean enable = false;

	if (digitalRead(BUTTON)) {
		if (enable) {
			if (millis()-starttime>50) {
				fire = true;
				enable = false;
			}
		}
		starttime = millis();
	} else {
		enable = true;
	}
	return;
}

// executa o sorteo 
void raffle(void)
{
	// value e b son contadores
	short value = 0x0, del = random(LED_ROULETTE_MIN_TIME_MS, LED_ROULETTE_MAX_TIME_MS);
	byte b, b_old, winner;

	update_lcd();
	// calcula a o resultado da tirada
	for (b = 0b0; b < EEPROM_VALID_BYTES; b++) {
		value += EEPROM.read(b);
	}
	if (value == 0) {
		SPI.transfer(0xF);
		digitalWrite(SS, HIGH);
		digitalWrite(SS, LOW);
		out_of_existences();
		return;
	}
	winner = random(1,value);
	value = 0x0;
	// asigna o premio gañado
	for (b = 0b0; b < EEPROM_VALID_BYTES; b++) {
		value += EEPROM.read(b);
		if (winner <= value) {
			winner = b;
			break;
		}
	}
	value = 0x0;
	wdt_reset();
	// purgase o latch
	SPI.transfer(led_order[0x3]); digitalWrite(SS, HIGH); digitalWrite(SS, LOW);
	b_old = 0x3;

	// Efecto da ruleta de leds 
	while (del > 0)
	{
		value += LED_BLINK_DELAY;
		// acéndense os leds por orde e compróbase o bus
		for (b = 0x0; b < 0x4; b++) {
			if (SPI.transfer(led_order[b]) != led_order[b_old]) {
				clear_lcd();
				lcd.setCursor(0, 0);
				lcd.print(SPI_ERROR);
				return;
			}
			b_old = b;
			digitalWrite(SS, HIGH);
			digitalWrite(SS, LOW);
			delay(LED_BLINK_DELAY+value);
			del -= LED_BLINK_DELAY+value;
		}
		wdt_reset();
	}

	// move o led ata o correspondente a o premio gañado 
	if (b != winner) {
		for (b = 0x0; b <= winner; b++) {
			if (SPI.transfer(led_order[b]) != led_order[b_old]) {
				clear_lcd(); lcd.setCursor(0,0);
				lcd.print(SPI_ERROR); return;
			}
			digitalWrite(SS, HIGH); digitalWrite(SS, LOW);
			b_old = b;
			if (b != winner ) {delay(LED_BLINK_DELAY+value);}
			wdt_reset();
		}
	}
	wdt_reset();

	// actualizase o lcd (se hai premios resta, senón devolve unha mensaxe de erro)
	clear_lcd();
	if (EEPROM.read(winner) > 0) {
		EEPROM.update(winner, EEPROM.read(winner)-1);
		update_lcd();
	} else {
		out_of_existences();
	}
}


// pon a cantidade de premios restantes no display
void update_lcd(void)
{
	clear_lcd();
	lcd.setCursor(1,0);
	lcd.print("P1:");
	lcd.setCursor(4,0);
	lcd.print(EEPROM.read(0x0));
	lcd.setCursor(11,0);
	lcd.print("P2:");
	lcd.setCursor(14,0);
	lcd.print(EEPROM.read(0x1));
	lcd.setCursor(1,1);
	lcd.print("P3:");
	lcd.setCursor(4,1);
	lcd.print(EEPROM.read(0x2));
	lcd.setCursor(11,1);
	lcd.print("P4:");
	lcd.setCursor(14,1);
	lcd.print(EEPROM.read(0x3));
}

// imprime unha mensaxe cando non quedan premios
void out_of_existences(void)
{
	clear_lcd();
	lcd.setCursor(0,0);
	lcd.print("Esgotaronse");
	lcd.setCursor(0,1);
	lcd.print("os premios");
}

// limpa o LCD
void clear_lcd(void)
{
	char blank = ' ';
	for (byte b = 0x0; b <= 0xF; b++) {
		lcd.setCursor(b, 0);
		lcd.print(blank);
		lcd.setCursor(b, 1);
		lcd.print(blank);
	}
}

// Resetea os bytes 0 a 3 da memoria EEPROM
void reset_EEPROM(void)
{
	EEPROM.update(0x0, 0x1);
	EEPROM.update(0x1, 0x2);
	EEPROM.update(0x2, 0x5);
	EEPROM.update(0x3, 0xA);
}
