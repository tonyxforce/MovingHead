#include <Arduino.h>
#include <DMXSerial.h>

const int RedPin = 9;		// PWM output pin for Red Light.
const int GreenPin = 6; // PWM output pin for Green Light.
const int BluePin = 5;	// PWM output pin for Blue Light.

// first DMX start address
#define DMXSTART 1

// number of DMX channels used
#define DMXLENGTH 3

#include <Adafruit_NeoPixel.h>
#include <AccelStepper.h>

#define PIN 4
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
#define DELAYVAL 500

AccelStepper tilt(AccelStepper::FULL4WIRE, 3, 4, 5, 14);
AccelStepper pan(AccelStepper::FULL4WIRE, 7, 8, 12, 13);

void setup()
{
	DMXSerial.init(DMXProbe);

	// enable pwm outputs
	pinMode(RedPin, OUTPUT); // sets the digital pin as output
	pinMode(GreenPin, OUTPUT);
	pinMode(BluePin, OUTPUT);

	// D3 high, D2 LOW
	pinMode(3, OUTPUT);
	pinMode(2, OUTPUT);
	digitalWrite(3, HIGH);
	digitalWrite(2, LOW);

	DMXSerial.maxChannel(DMXLENGTH); // after 3 * pixel channels, the onUpdate will be called when new data arrived.

	// give them a decent color...
	DMXSerial.write(1, 5);
	DMXSerial.write(2, 10);
	DMXSerial.write(3, 20);

	pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

	pan.setMaxSpeed(200.0);
	pan.setAcceleration(100.0);
	pan.moveTo(1200);
	pan.runToPosition();
	pan.setCurrentPosition(0.0);

	tilt.setMaxSpeed(200.0);
	tilt.setAcceleration(100.0);
	tilt.moveTo(1200);
	tilt.runToPosition();
	tilt.setCurrentPosition(0.0);
}

// constantly fetch DMX data and update the led.
void loop()
{
	pan.run();
	tilt.run();

	// wait for an incoming DMX packet.
	if (DMXSerial.receive())
	{
		uint8_t r = DMXSerial.read(1),
						g = DMXSerial.read(DMXSTART + 1),
						b = DMXSerial.read(DMXSTART + 2);
		analogWrite(RedPin, r);
		analogWrite(GreenPin, g);
		analogWrite(BluePin, b);

		tilt.moveTo(DMXSerial.read(DMXSTART + 3));
		pan.moveTo(DMXSerial.read(DMXSTART + 4));

		pixels.setPixelColor(0, pixels.Color(r, g, b));
		pixels.show(); // This sends the updated pixel color to the hardware.
	}
	else
	{
		// don't update the led but signal a red.
		analogWrite(RedPin, 100);
		analogWrite(GreenPin, 0);
		analogWrite(BluePin, 0);
		pixels.setPixelColor(0, pixels.Color(100, 0, 0));
		pixels.show(); // This sends the updated pixel color to the hardware.
		tilt.moveTo(0);
		pan.moveTo(0);
	} // if
}