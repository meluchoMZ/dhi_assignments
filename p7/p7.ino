/**
  Termómetro dixital
  Miguel Blanco Godón
*/

#include <MsTimer2.h>

#define THERMISTOR A0
#define Ts 2 // período de mostraxe en milisegundos
#define AMPLITUDE_FILTERING_INTERVAL 200/Ts // intervalo de mostraxe para o filtrado
#define Tg 200/Ts // período de xeración, 0.2 segundos
// coeficientes para o modelo Steinhart-Hart
#define A 0.00111492089
#define B 0.0002372075385
#define C 0.00000006954079529
#define K 2.5 // mW/ºC

void interrupt_handler(void);
void update_buffer(float value);
void update_bcd(int value);

volatile float amplitude = 0, temperature = 0; // acumulador amplitudes
volatile byte ampc = 0x01; // contador de mostras de amplitude para filtrado de frecuencias de rede eléctrica
volatile byte genc = 0x00; // contador para a xeración de datos
volatile float amp_buffer[5] = {0,0,0,0,0}; // buffer circular que para implementar o filtro de media
volatile unsigned long time_delta = 0; // contador de tempos
const byte bcd[7] = {3,4,5,6,7,8,9}; // BCD inputs a,b,c,d,e,f,g
const byte multiplexing[4] = {10,11,12,13}; // números de esquerda a dereita
const byte digits[10] = 
{
	0b00111111, 0b00000110, 0b01011011,
	0b01001111, 0b01100110, 0b01101101,
	0b01111101, 0b00000111, 0b01111111,
	0b01101111
}; // números do 0 ao 10
const byte celsius = 0b00111001; // símbolo C
const byte grad = 0b01100011; // símbolo º
const byte minus = 0b01000000; // símbolo -

void setup(void)
{
	// Inicio do porto serie
	Serial.begin(9600);
	// Inicio e configuración do Timer 2
	MsTimer2::set(Ts, interrupt_handler);
	MsTimer2::start();
	// configuración dos pins dixitais como saídas
	for (byte b = 0x03; b < 0x0E; b++) {
		pinMode(b, OUTPUT);
	}
}


void loop(void)
{
}


// actualiza os valores do buffer circular
void update_buffer(float value)
{
	for (byte b = 0x4; b > 0x0; b--) {
		amp_buffer[b] = amp_buffer[b-1];
	}
	amp_buffer[0] = value;
}


// manexador da interrupción
void interrupt_handler(void)
{
	float Rm, logRm, Vm, buffer_mean;

	time_delta += Ts; //actualízase o tempo que vai pasando

	// filtrado de frecuencias cada 200 ms
	if (++ampc == AMPLITUDE_FILTERING_INTERVAL+1) {
		ampc = 0x01;
		amplitude = 0;
	}
	// lectura do valor de saída do termistor
	amplitude += analogRead(THERMISTOR);
	// actualizase o buffer circular
	update_buffer(amplitude/ampc);

	// actualízase a temperatura cada 0.2 s
	if (++genc == Tg) {
		genc = 0x00;
		// aplícase o filtro de media
		buffer_mean = (amp_buffer[0]+amp_buffer[1]+amp_buffer[2]+amp_buffer[3]+amp_buffer[4])/5;

		// calcúlase a temperatura segundo o modelo Steinhart-Hart
		Rm = (float) (10000*buffer_mean)/(1024-buffer_mean);
		logRm = log(Rm);
		Vm = (float) (5*buffer_mean)/1024;
		temperature = (1/(A+B*logRm+C*logRm*logRm*logRm)) - (1000*Vm*Vm)/(K*Rm);
		// paso a celsius
		temperature -= 273.15;

		// envíanse os datos polo porto serie
		Serial.print("Time(ms): ");
		Serial.print(time_delta, DEC);
		Serial.print(" Rm(ohm.): ");
		Serial.print(Rm, DEC);
		Serial.print(" T(ºC): ");
		Serial.println(temperature, DEC);
	}
	// conmútase o BCD cada 2 ms
	update_bcd((int)temperature);
}


// commuta os díxitos do BCD
void update_bcd(int value)
{
	static byte digit = 0x00;
	byte full_output[4]; // saída en cada díxito do BCD

	// apágase o BCD (en HIGH posto que polariza o diodo en inversa)
	for (byte b = 0x00; b < 0x04; b++) {
		digitalWrite(multiplexing[b], HIGH);
	}


	// lóxica para sacar os díxitos
	if (value > 9) {
		full_output[0] = digits[(int) value/10];
		full_output[1] = digits[value%10];
		full_output[2] = grad;
		full_output[3] = celsius;
	} else {
		if (value >= 0 && value <= 9) {
			full_output[0] = 0x0;
			full_output[1] = digits[value];
			full_output[2] = grad;
			full_output[3] = celsius;
		} else {
			if (value >= -9) {
				full_output[0] = minus;
				full_output[1] = digits[-value];
				full_output[2] = grad;
				full_output[3] = celsius;
			} else {
				full_output[0] = minus;
				full_output[1] = digits[(int)value/-10];
				full_output[2] = digits[(-value)%10];
				full_output[3] = grad;
			}
		}
	}

	// polarízanse os segmentos correspondentes
	for (byte b = 0x00; b < 0x07; b++) {
		digitalWrite(bcd[b], bitRead(full_output[digit], b));
	}

	// acéndese o díxito correspondente 
	digitalWrite(multiplexing[digit], LOW);

	// actualízase o seguinte díxito a acender
	digit = ++digit % 4;
}
