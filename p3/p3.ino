/**
  Máquina de premios
  Dispositivos hardware e interfaces, enxeñaría de computadores
  Miguel Blanco Godón
  **/
#include <SPI.h>
#include <LiquidCrystal.h>

#define BUTTON 7 
#define LCD_CONTRAST 6
#define SS 10
#define LED_BLINK_ORDER 0b1000 // B = 0x1000; R = 0x0100; Y = 0x0010; G = 0x0001 Orde de cambio dos leds 
#define LED_ROULETTE_MIN_TIME_MS 7000 // minimo tempo que duran os leds en ruleta
#define LED_ROULETTE_MAX_TIME_MS 8500 // maximo tempo que duran os leds en ruleta
#define LED_BLINK_DELAY 75 // retraso inicial dos leds da ruleta
#define PRIZE_NUMBER 4 // numero de premios

byte prizes[PRIZE_NUMBER] = {0x1, 0x2, 0x5, 0xA}; // premios concretos 
// inicio lcd e asignación de pins
LiquidCrystal lcd(8,9,5,4,3,2);

bool fire = false; //indica se hai que executar o sorteo

void read_button(void);
void raffle(void);
void update_lcd(void);
void out_of_existences(void);
void clear_lcd(void);


void setup(void)
{
	// semente pros números pseudoaleatorios
	randomSeed(analogRead(0));
	// configuración do botón de inicio
	pinMode(BUTTON, INPUT);
	pinMode(BUTTON, LOW);
	// configuración LCD
	lcd.begin(16,2);
	analogWrite(LCD_CONTRAST,60);
	clear_lcd();
	lcd.setCursor(0,0); lcd.print("Xogue e ganhe!");
	lcd.setCursor(0,1); lcd.print("Prema o pulsador");
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
}

void loop(void)
{
	read_button();
	if (fire) {
		raffle();
		fire = false;
	}
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
	byte b, winner;

	// reset da pantalla lcd 
	clear_lcd(); update_lcd();
	// calcula a o resultado da tirada
	for (b = 0b0; b < PRIZE_NUMBER; b++) {
		value += prizes[b];
	}
	winner = random(1,value);
	value = 0x0;
	// asigna o premio gañado
	for (b = 0b0; b < PRIZE_NUMBER; b++) {
		value += prizes[b];
		if (winner <= value) {
			winner = b;
			break;
		}
	}
	value = 0x0;
	// Efecto da ruleta de leds 
	while (del > 0)
	{
		value += LED_BLINK_DELAY;
		// a orde faise con desprazamentos de bits sobre unha macro
		for (b = 0b0; b < LED_BLINK_ORDER >> 0b1; b++) {
			SPI.transfer(LED_BLINK_ORDER >> b);
			digitalWrite(SS, HIGH);
			digitalWrite(SS, LOW);
			delay(LED_BLINK_DELAY+value);
			del -= LED_BLINK_DELAY+value;
		}
	}

	// move o led ata o correspondente a o premio gañado 
	while ((b%0b100) != winner)
	{
		SPI.transfer(LED_BLINK_ORDER >> ((b++)%0b100));
		digitalWrite(SS, HIGH);
		digitalWrite(SS, LOW);
		delay(LED_BLINK_DELAY+value);
	}

	// actualizase o lcd (se hai premios resta, senón devolve unha mensaxe de erro)
	clear_lcd();
	if (prizes[winner] > 0) {
		prizes[winner]--;
		update_lcd();
	} else {
		out_of_existences();
	}
}


void update_lcd(void)
{
	lcd.setCursor(1,0);
	lcd.print("P1:");
	lcd.setCursor(4,0);
	lcd.print(prizes[0]);
	lcd.setCursor(11,0);
	lcd.print("P2:");
	lcd.setCursor(14,0);
	lcd.print(prizes[1]);
	lcd.setCursor(1,1);
	lcd.print("P3:");
	lcd.setCursor(4,1);
	lcd.print(prizes[2]);
	lcd.setCursor(11,1);
	lcd.print("P4:");
	lcd.setCursor(14,1);
	lcd.print(prizes[3]);
}

void out_of_existences(void)
{
	lcd.setCursor(0,0);
	lcd.print("Esgotaronse");
	lcd.setCursor(0,1);
	lcd.print("os premios");
}

void clear_lcd(void)
{
	for (byte b = 0x0; b <= 0xF; b++) {
		lcd.setCursor(b, 0);
		lcd.print(" ");
		lcd.setCursor(b, 1);
		lcd.print(" ");
	}
}
