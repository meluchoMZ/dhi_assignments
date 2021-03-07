/*
   Reloxo
   Miguel Blanco Godón
 */
#include <MsTimer2.h>
#include <avr/sleep.h>

const int button = 3;
const int green = 5;
const int red = 10;
volatile int h = 0, m = 0, s = 0;
volatile bool pinState = HIGH, sleeping = false;

void timer2_interruption_handler(void);
void goto_sleep(void);
void sleep_irs(void);

void setup(void)
{
	Serial.begin(9600);
	pinMode(button, INPUT);
	pinMode(green, OUTPUT);
	pinMode(red, OUTPUT);
	MsTimer2::set(1000, timer2_interruption_handler);
	MsTimer2::start();
	attachInterrupt(digitalPinToInterrupt(3), sleep_irs, RISING);
	set_sleep_mode(SLEEP_MODE_IDLE);
}

void loop(void)
{
	int hprov, mprov, sprov;
	if (Serial.available() > 0) {
		hprov = Serial.parseInt(SKIP_ALL);
		mprov = Serial.parseInt(SKIP_ALL);
		sprov = Serial.parseInt(SKIP_ALL);
		while (Serial.available() > 0)
		{
			Serial.read();
		}
		if ((hprov<0)||(hprov>23)||(mprov<0)||(mprov>59)||(sprov<0)||(sprov>59)) {
			Serial.println("Paso a modo sleep");
			goto_sleep();
		} else {
			h = hprov; m = mprov; s = sprov;
		}
	}
}

void timer2_interruption_handler(void)
{
	if (++s > 59) {
		s = 0;
		m++;
	}
	if (m > 59) {
		m = 0;
		h++;
	}
	if (h > 23) {
		h = 0;
	}
	if (!sleeping) {
		Serial.print(h, DEC); Serial.print(":");
		Serial.print(m, DEC); Serial.print(":");
		Serial.println(s, DEC); 
		digitalWrite(green, pinState);
		digitalWrite(red, !pinState);
	}
	pinState = !pinState;
}

void sleep_irs(void)
{
	sleeping = false;
}

void goto_sleep(void)
{
	sleeping = true;
	digitalWrite(green, LOW);
	digitalWrite(red, LOW);
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}
