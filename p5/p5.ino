/*
   Xerador de ondas
   Miguel Blanco Godón
*/
#include <MsTimer2.h>

#define N 200 // mostras/s
#define T 2 // período
#define PRESENTATION_TIME 5000 //intervalo entre sinais
#define VDC 2.5
#define VMAX 5
#define VMIN 0

// macros para computar as funcións sen necesidade de cambio de contexto
#define SINE(a) (VDC+1.25*sin(2*PI*(float)a/N))
#define COSINE(a) (VDC+1.25*cos(20*PI*(float)a/N))
#define AM(a) (VDC+(1+0.5*sin(2*PI*(float)a/N))*cos(20*PI*(float)a/N))
#define TRIANG(a) (((a)<(N/4))?(VDC-1.25*(-4*(float)a/N)):(((a)<(3*N/4))?(VDC+1.25*(2-4*(float)a/N)):(VDC-1.25*(4-4*(float)a/N))))
#define CLK(a) ((a<N/5)?VMAX:VMIN)
#define BPSK(a) ((a<N/5)?(VDC+1.25*cos(20*PI*(float)a/N)):(VDC-1.25*cos(20*PI*(float)a/N)))

volatile byte n = 0x0; // mostra actual (mod N)
volatile float x;  // valor de x(n)
volatile byte sig = 0x0; // indice para o sinal a amosar
volatile String legends[6] ={"sen:,2.5VDC:", "cos:,2.5VDC:", "AM:,2.5VDC:", "TRIANG:,2.5VDC:", "PWM:,2.5VDC:", "BPSK:,2.5VDC"}; // valores da lenda

void interrupt_handler(void);


void setup(void)
{
	Serial.begin(230400);
	MsTimer2::set(T, interrupt_handler);
	MsTimer2::start();
}

void loop(void)
{
	// inprime a lenda no serial plot
	Serial.println(legends[sig].c_str());
	// cada 5 segundos cambianse os sinais
	delay(PRESENTATION_TIME);
	sig = (sig+1)%6;
}

// devolve os valores dos sinais
void interrupt_handler(void)
{
	switch(sig)
	{
		case 0x0:
			x = SINE(n);
			break;
		case 0x1:
			x = COSINE(n);
			break;
		case 0x2:
			x = AM(n);
			break;
		case 0x3:
			x = TRIANG(n);
			break;
		case 0x4:
			x = CLK(n);
			break;
		case 0x5:
			x = BPSK(n);
			break;
		default:
			break;
	}
	if (++n == N) {
		n = 0x0;
	}
	Serial.print(x);
	Serial.print(" ");
	Serial.println(VDC);
}
