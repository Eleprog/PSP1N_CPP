#include "PSP1N_V2.h"


#define SIZE_ARRAYBYTES 6
StructurePackagePSP structureTransmit[] = { 30,	10 };
byte bufferTransmit[SIZE_ARRAYBYTES];
PackagePSP packTransmit(StartBit::ZERO, structureTransmit, bufferTransmit);

StructurePackagePSP structureRecieve[] = { 30,	10 };
byte bufferRecieve[SIZE_ARRAYBYTES];
PackagePSP packRecieve(StartBit::ZERO, structureRecieve, bufferRecieve);

byte bufferData[SIZE_ARRAYBYTES];

void setup() {
	Serial.begin(250000);
	//7, 1023
	bufferData[0] = 0b00000111;
	bufferData[1] = 0b10000000;
	bufferData[2] = 0b10000000;
	bufferData[3] = 0b10000000;
	bufferData[4] = 0b11111100;
	bufferData[5] = 0b10011111;
}


void loop() {

	//Encoding buffer
	packTransmit.setItemValue(0, millis() / 1000);
	packTransmit.setItemValue(1, 1023);
	DataPSP data = packTransmit.encode();
	Serial.println("Data transmit:");
	for (size_t i = 0; i < data.getSize(); i++)
	{
		Serial.println(data.getData()[i], 2);
	}
	Serial.println("===Data transmit end===\n");


	//Decoding buffer
	Serial.println("Decoding buffer:");
	int counter = 0;
	while (packRecieve.decode(bufferData[counter])!=ResultDecode::DECODE_OK) {
		counter++;
	}
	Serial.println(packRecieve.getItemValue(0));
	Serial.println(packRecieve.getItemValue(1));
	Serial.println("===End recieve===\n");

	delay(1000);
}


