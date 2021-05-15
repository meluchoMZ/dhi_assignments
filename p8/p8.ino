/**
  Seguidor lumínico con control PID
Author: Miguel Blanco Godón
*/

#include <Servo.h>

#define K_p 0.025f
#define K_i 0.03f
#define K_d 0.01f

Servo motor;
volatile float previous_error = 0, integral = 0;

void setup(void)
{
	// inicia o motor
	motor.attach(9);
	// inicialmente o motor está a 0º
	motor.write(0);
}

void loop(void)
{
	float error = 0, linear = 0, differential = 0, angle = 0;

	// control PID
	// promediado de 40 lecturas
	for (int i = 0; i < 40; i++) {
		// medida e promediado de lectura dos fotorresistores
		error += analogRead(A0)-analogRead(A1);
		// faise os primeiros 40 milisegundos
		delay(1);
	}
	error /= 40;

	// sumase ao termo integral do controlador PID o erro ponderado
	integral += error*K_i; 
	// correción do termo integral para que estea nun rango de xiro horizontal
	integral = (integral > 179) ? 179 : integral;
	integral = (integral < 0) ? 0 : integral;

	// calulo dos termos lineais e diferenciais
	linear = K_p * error; 
	differential = K_d *(error-previous_error);
	previous_error = error;
	// cálculo do ángulo
	angle = linear + integral + differential;

	// corrección do ángulo
	angle = (angle > 179) ? 179 : angle;
	angle = (angle < 0) ? 0 : angle;

	// só move o motor se pasa dun marxe
	if (abs(error) >= 10) {
		motor.write((int) angle);
	}
	// os ultimos 40 ms espérase a que o motor faga o movemento
	delay(40);
}
