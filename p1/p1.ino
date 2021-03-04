/*
Reflexos sen usar interrupcións
Miguel Blanco Godón
*/

const int button = 3;
const int green = 5;
const int red = 10;

void setup(void)
{
	Serial.begin(9600);
	pinMode(button, INPUT);
	pinMode(green, OUTPUT);
	pinMode(red, OUTPUT);
	Serial.println("Benvido ao xogo dos reflexos!");
	Serial.println("Prema o boton para comezar o xogo");
	randomSeed(analogRead(0));
}

void loop(void)
{
	int del, count, start, time;

	digitalWrite(green, HIGH);
	while(digitalRead(button) != HIGH)
	{
		;
	}
	digitalWrite(green, LOW);
	delay(20);
	while(digitalRead(button) != LOW) 
	{
		;
	}

	del = random(400, 1201);
	count = del;
	start = millis();
	// pooling cada 10ms mentres non se pulse o boton ou pase o tempo
	while(digitalRead(button) != HIGH && count > 0)
	{
		delay(1);
		count--;
	}
	// so se encende o led vermello se se fixo a espera completa
	if (count <= 0) {
		digitalWrite(red, HIGH);
		// esperase a que pulsen o led vermello
		while(digitalRead(button) !=  HIGH) 
		{
			;
		}
		digitalWrite(red, LOW);
	}
	// calculanse os tempos
	time = millis()-start;
	// esperase depulsarse
	while (digitalRead(button) != LOW) 
	{
		;
	}
	// imprimese por porto serie
	if (time <= del) {
		Serial.println("Non se precipite. Tente de novo");
	} else {
		if ((time - del) < 100) {
			Serial.print(time - del, DEC);
			Serial.println("ms PARABENS! Excelentes reflexos");
		} else if ((time-del) > 200) {
			Serial.print(time - del, DEC);
			Serial.println("ms MOI LENTO! Tente de novo");
		} else {
			Serial.print(time - del, DEC);
			Serial.println("ms BEN! Bos reflexos");
		}
	}
	delay(50);
}
