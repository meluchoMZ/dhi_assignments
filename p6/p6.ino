/**
  Xerador PWM
  Miguel Blanco Godón
*/

#include <MsTimer2.h>

#define ANALOG_WRITE 0x05 // pin de escritura PWM
#define ANALOG_READ 0x00 // pin de lectura do sinal
#define POTENTIOMETER 0x02 // pin de lectura do potenciómetro
#define T 2 // período de interrupción do timer 2
#define Tg 1 // período de xeración por interrupción do timer 2. Período real Tgreal = T*Tg
#define Ts 1 // período de mostraxe. Período real Tsreal = T*Ts
#define VMAX 5
#define VMIN 0
#define VDC 2.5 // tensión de contínua
#define TMEAN 100 // tempo de promediado de ruido do potenciómetro
#define SIGNAL_SELECTOR 0x02 // pin dixital 2. Ao premelo modifica o sinal utilizado
#define FREQZ_SELECTOR 0x04 // pin dixital 4. Ao premelo modifica a frecuencia do sinal
#define AMPLITUDE_FILTERING_INTERVAL 50 // mostras para o filtrado


void interrupt_handler(void);

volatile short N = 100; // mostras xeradas por ciclo
volatile short n = 0; // mostra actual
volatile byte sig = 0x00; // indicador do sinal a utilizar 
volatile float generated; // valores xerados dos sinais
volatile short received; // valores recibidos dos sinais
volatile byte ctg = 0x00; // contador de xeración
volatile byte cts = 0x00; // contador de mostraxe
volatile byte camp = 0x01; // contador de mostras de filtrado de amplitude
volatile unsigned long amplitude = 0;

void setup(void)
{
	// inicialización do porto serie
	Serial.begin(230400);
	// Pins dixitais como entradas 
	pinMode(SIGNAL_SELECTOR, INPUT_PULLUP);
	pinMode(FREQZ_SELECTOR, INPUT_PULLUP);
	// modifícase o rexistro para que a frecuencia de PWM sexa de 62.5 kHz
	TCCR0B &= 0xF8;
	TCCR0B |= 0x01;
	// Inicialización das interrupcións do timer 2
	MsTimer2::set(T, interrupt_handler);
	MsTimer2::start();
	// Escritura da lenda no plotter
	Serial.println("MAT+5:,PWM:");
}

void loop(void)
{
	// lectura dos valores dos pulsadores
	bool s = digitalRead(SIGNAL_SELECTOR);
	bool f = digitalRead(FREQZ_SELECTOR);
	if (s == LOW) {
		// modifica o sinal a utilizar
		sig = (sig+1)%4;
		// para asegurar a depulsación ao non poder usar delay nin millis 
		while (digitalRead(SIGNAL_SELECTOR) != HIGH){;}
		while (digitalRead(SIGNAL_SELECTOR) != HIGH){;}
		while (digitalRead(SIGNAL_SELECTOR) != HIGH){;}
		delayMicroseconds(16383);
		delayMicroseconds(16383);
		delayMicroseconds(16383);
	}
	if (f == LOW) {
		// modifica a frecuencia
		N = (N >= 1000) ? 100 : (N + 100);
		// igual ca no caso de modificación do sinal
		while (digitalRead(FREQZ_SELECTOR) != HIGH){;}
		while (digitalRead(FREQZ_SELECTOR) != HIGH){;}
		while (digitalRead(FREQZ_SELECTOR) != HIGH){;}
		delayMicroseconds(16383);
		delayMicroseconds(16383);
		delayMicroseconds(16383);
	}
}

// manexador da interrupción do timer 2
void interrupt_handler(void)
{
	// actualiza o contador de mostraxe
	if (++n >= N) {
		n = 0;
	}

	// xérase 1 mostra cada Tg*T milisegundos
	if (++ctg >= Tg) {
		// obtense a amplitude en función do potenciómetro e pásase de 10 a 7 bits
		// úsase un filtro de media cun intervalo de 100 ms para o control de amplitude
		if (++camp == AMPLITUDE_FILTERING_INTERVAL+1) {
			amplitude = 0;
			camp = 0x01;
		}
		amplitude += map(analogRead(POTENTIOMETER), 0, 1023, 0, 127);

		switch (sig)
		{
			case 0x00:
				// sinal senoidal
				generated = 128+(amplitude/camp)*sin(2*PI*n/N);
				break;
			case 0x01:
				// sinal triangular
				generated = (n<(N/4)) ? (128-(amplitude/camp)*(-4*(float)n/N)) : ((n<(3*N/4)) ? (128+(amplitude/camp)*(2-4*(float)n/N)) : (128-(amplitude/camp)*(4-4*(float)n/N)));
				break;
			case 0x02:
				// sinal cadrada
				generated = (n<N/2) ? (128+(amplitude/camp)) : (128 - (amplitude/camp));
				break;
			case 0x03:
				// sinal contínua
				generated = 2*(amplitude/camp);
				break;
			default:
				break;
		}
		// envíase o sinal en forma de PWM
		analogWrite(ANALOG_WRITE, generated);
		ctg = 0x00;
	}

	// lese unha mostra cada Ts*T miligsegundos
	if (++cts >= Ts) {
		received = analogRead(ANALOG_READ);
		cts = 0x00;
	}


	// onda calculada polo plotter
	Serial.print((float) 5+4*generated*VMAX/1024);
	Serial.print(" ");
	// onda recibida do circuito
	Serial.println((float) received*VMAX/1024);
}

