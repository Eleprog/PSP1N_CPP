#include "PSP1N_V2.h"


#define SIZE_ARRAYBYTES 6
StructurePackagePSP dataTransmit[] = {
	30,
	10
};
byte bufferTransmit[SIZE_ARRAYBYTES];
PackagePSP packTransmit(StartBit::ZERO, dataTransmit, bufferTransmit);

StructurePackagePSP dataRecieve[] = {
	30,
	10
};
byte bufferRecieve[SIZE_ARRAYBYTES];
PackagePSP packRecieve(StartBit::ZERO, dataRecieve, bufferRecieve);



void setup() {
	Serial.begin(250000);
	//7, 1023
	bufferRecieve[0] = 0b00000111;
	bufferRecieve[1] = 0b10000000;
	bufferRecieve[2] = 0b10000000;
	bufferRecieve[3] = 0b10000000;
	bufferRecieve[4] = 0b11111100;
	bufferRecieve[5] = 0b10011111;	
}



void loop() {

	//Encoding buffer
	packTransmit.setItemValue(0, millis()/1000);
	packTransmit.setItemValue(1, 1023);
	packTransmit.encode();
	Serial.println("Data transmit:");
	for (size_t i = 0; i < SIZE_ARRAYBYTES; i++)
	{
		Serial.println(bufferTransmit[i], 2);
	}
	Serial.println("===Data transmit end===\n");


	//Decoding buffer
	Serial.println("Decoding buffer:");	
	int counter = 0;
	while (!packRecieve.decode(bufferRecieve[counter])) {
		counter++;
	}
	Serial.println(packRecieve.getItemValue(0));
	Serial.println(packRecieve.getItemValue(1));
	Serial.println("===End recieve===\n");

	delay(1000);
}


