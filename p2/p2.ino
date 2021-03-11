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
void sleepNow(void);
void sleep_irs(void);

void setup(void)
{
	// inicializase comunicacion serie
	Serial.begin(9600);
	// inicializanse os pins
	pinMode(button, INPUT);
	pinMode(green, OUTPUT);
	pinMode(red, OUTPUT);
	// inicializase o timer 2
	MsTimer2::set(1000, timer2_interruption_handler);
	MsTimer2::start();
	// asignaselle á interrupcion provocada polo pin 3 a irs "sleep_irs"
	attachInterrupt(INT1, sleep_irs, HIGH);
	// configúrase o modo de sleep
	set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
}

void loop(void)
{
	int hprov, mprov, sprov;
	// compróbase se hai datos de entrada no porto serie
	if (sleeping) {
		// segue durmindo
		sleepNow();
	} else {
		if (Serial.available() > 0) {
			// extráense os valores para poñer o reloxo en hora
			hprov = Serial.parseInt();
			mprov = Serial.parseInt();
			sprov = Serial.parseInt();
			// baldeirase o buffer de entrada pra descartar os datos de mais
			while (Serial.available() > 0)
			{
				Serial.read();
			}
			// se a hora non cadra, pasa a modo sleep
			if ((hprov<0)||(hprov>23)||(mprov<0)||(mprov>59)||(sprov<0)||(sprov>59)) {
				Serial.println("Paso a modo sleep");
				// espérase a que baldeire o buffer de saída
				Serial.flush();
				sleeping = true;
				// apaga os leds 
				digitalWrite(green, LOW);
				digitalWrite(red, LOW);
				// modo sleep
				sleepNow();
				// baldeirase o buffer cando esperta por se quedaron datos no buffer pra que non cambie a hora
				while (Serial.available() > 0)
				{
					Serial.read();
				}
			} else {
				// actualízase a hora do reloxo
				h = hprov; m = mprov; s = sprov;
			}
		}
	}
}

// rutina que computa a hora e realiza a comunicación serie e controla os leds 
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
	// só se comunica e encende os leds en caso de non estar no modo de aforro de enerxía
	if (!sleeping) {
		Serial.print(h, DEC); Serial.print(":");
		Serial.print(m, DEC); Serial.print(":");
		Serial.println(s, DEC); Serial.flush();
		digitalWrite(green, pinState);
		digitalWrite(red, !pinState);
	}
	pinState = !pinState;
}

// rutina de interrupción do pulsador
void sleep_irs(void)
{
	sleeping = false;
}

// rutina de modo aforro de enerxía
void sleepNow(void)
{
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}
