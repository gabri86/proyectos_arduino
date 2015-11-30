#include <Wire.h>
#include <DS3231.h>
#include <DHT.h>

DS3231 clock;
RTCDateTime dt;

#define DHTPIN 2 //Seleccionamos el pin en el que se conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT11
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor

int redPin = 5; //Color rojo, conectado al pin digital 5
int greenPin = 6; //Color verde, conectado al pin digital 6
int bluePin = 3; //Color azul, conectado al pin digital 3

void setup()
{
	Serial.begin(9600);

	//Inicializar el  DS3231
	Serial.println("Inicializando DS3231");;
	clock.begin();

	//Seteo de la hora en el DS3231
	//clock.setDateTime(__DATE__, __TIME__); //Obtiene la fecha y hora del sistema
	//clock.setDateTime(2014, 4, 13, 07, 00, 00); //Fecha y hora personalizada

	//Inicializar el DHT11
	Serial.println("Inicializando DHT11");
	dht.begin();

	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
}

void loop()
{
	dt = clock.getDateTime();

	Serial.println(clock.dateFormat("d-m-Y H:i:s", dt));
	dia(600, 1100, 1700, 2100);
	getTempHum();
	delay(1000);
}

void dia(float lightsMan, float lightsDia, float lightsTarde, float lightsNoche)
{
	int redMan, greenMan, blueMan, 
		redTarde, greenTarde, blueTarde, 
		redFadeMan, blueFadeMan, greenFadeMan, 
		redFadeTarde, blueFadeTarde, greenFadeTarde; // Los enteros aca definidos son usados para controlar cada canal en cada una de las fases del día

	float eqpotpwmman, eqpotpwmtarde; //Estas variables se utilizan para ajustar el tiempo de regulación total de los leds

	int hora, minuto;

	int mil_time = 0, dec_time = 0;

	hora = dt.hour;
	minuto = dt.minute;

	mil_time = (hora * 100) + minuto;
	dec_time = (hora * 100) + (minuto * 1.666);

	Serial.print("mil_time: ");
	Serial.print(mil_time);
	Serial.print(" - dec_time: ");
	Serial.println(dec_time);
	
	if (lightsMan < mil_time && lightsDia >= mil_time) //Determina si es el amanecer
	{
		Serial.println("AMANECER");

		redFadeMan = 3; //El índice de la curva de atenuación en el canal rojo durante el amanecer
		greenFadeMan = 4; //El índice de la curva de atenuación en el canal verde durante el amanecer
		blueFadeMan = 7; //El índice de la curva de atenuación en el canal azul durante el amanecer

		int difDia = lightsMan - lightsDia;
		int difDecDia = dec_time - lightsDia;
		int difDifDia = difDia - difDecDia;

		//Ajusta el tiempo total de la decoloración con el tiempo total de la mañana
		eqpotpwmman = 255 / (difDia);

		//Establece una curva a apagar los leds durante el amanecer
		redMan = (((pow(difDifDia, redFadeMan)*(eqpotpwmman)) / pow(difDia, (redFadeMan - 1))));
		greenMan = (((pow(difDifDia, greenFadeMan)*(eqpotpwmman)) / pow(difDia, (greenFadeMan - 1))));
		blueMan = (((pow(difDifDia, blueFadeMan)*(eqpotpwmman)) / pow(difDia, (blueFadeMan - 1)))) + 1;
		
		//Envía el PWM a la salida correspondiente
		analogWrite(redPin, redMan); 
		analogWrite(greenPin, greenMan);
		analogWrite(bluePin, blueMan);

		//Lineas para controlar los valores en la salida analoga.
		Serial.print("greenMan=");
		Serial.print(greenMan);
		Serial.print(" redMan=");
		Serial.print(redMan);
		Serial.print(" blueMan=");
		Serial.print(blueMan);
		Serial.print(" eqpotpwmman=");
		Serial.println(eqpotpwmman);
	}
	else
	{
		redMan = 0;
		blueMan = 0;
		greenMan = 0;
		if (lightsDia < mil_time && lightsTarde >= mil_time) //Determina si es el día
		{
			Serial.println("DIA");
			analogWrite(redPin, 255);
			analogWrite(greenPin, 255);
			analogWrite(bluePin, 255);
		}
		else
		{

			if (lightsTarde < mil_time && lightsNoche >= mil_time) //Determina si es el atardecer
			{
				Serial.println("ATARDECER");

				redFadeTarde = 2;
				greenFadeTarde = 3;
				blueFadeTarde = 1;

				int difTarde = lightsNoche - lightsTarde;
				int difDecTarde = dec_time - lightsTarde;
				int difDifTarde = difTarde - difDecTarde;

				eqpotpwmtarde = (255 / difTarde);


				redTarde = ((pow(difDifTarde, redFadeTarde)*(eqpotpwmtarde)) / pow(difTarde, (redFadeTarde - 1)));
				greenTarde = ((pow(difDifTarde, greenFadeTarde)*(eqpotpwmtarde)) / pow(difTarde, (greenFadeTarde - 1)));
				blueTarde = ((pow(difDifTarde, blueFadeTarde)*(eqpotpwmtarde)) / pow(difTarde, (blueFadeTarde - 1))) + 1;
				
				analogWrite(redPin, redTarde);
				analogWrite(greenPin, greenTarde);
				analogWrite(bluePin, blueTarde);

				//Lineas para controlar los valores en la salida analoga.
				Serial.print("greenTarde=");
				Serial.print(greenTarde);
				Serial.print(" redTarde=");
				Serial.print(redTarde);
				Serial.print(" blueTarde=");
				Serial.print(blueTarde);
				Serial.print(" pwmtartde=");
				Serial.println(eqpotpwmtarde);
			}
			else  //Determina si es la noche
			{

				Serial.println("NOCHE");

				redTarde = 0;
				blueTarde = 0;
				greenTarde = 0;

				if (mil_time > 0 && mil_time < 500) 
				{
					analogWrite(redPin, 0);
					analogWrite(greenPin, 0);
					analogWrite(bluePin, 0);
				}
				else
				{
					analogWrite(redPin, 0);
					analogWrite(greenPin, 0);
					analogWrite(bluePin, 1);
				}
			}
		}
	}
}

void getTempHum() {
	Serial.print("Temperatura: ");
	Serial.println(dht.readTemperature());
	Serial.print("Humedad: ");
	Serial.println(dht.readHumidity());
}

