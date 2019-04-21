/******************************************************************************
	 * File: PSP1N.h
	 * Description: Protocol streaming packets(PSP1N)
	 * Created: 30.03.2019
	 * Author: Stupenkov Anton Sergeevich

******************************************************************************/

#ifndef _PSP1N_V2_h
#define _PSP1N_V2_h
#define DATA_BITS 7 //number of data bits per byte
//Start bit 
typedef enum StartBit
{
	ZERO,
	ONE
}StartBit;

//Data unit
struct StructurePackagePSP
{
	StructurePackagePSP(byte size, uint32_t value = 0) {
		this->size = size;
		this->value = value;
	}	
	friend class PackagePSP;
private:
	byte size = 0; //data size in bits
	uint32_t value = 0;
	byte getSize() {
		return size;
	}
	void setValue(uint32_t value) {
		this->value = value;
	}
	uint32_t getValue() {
		return value;
	}
};

//Data packet
class PackagePSP
{
public:
	//Package data
	uint32_t getItemValue(int itemIndex) {
		return item[itemIndex].getValue();
	}
	void setItemValue(int itemIndex, uint32_t value) {
		item[itemIndex].setValue(value);
	}
	byte getItemSize(int itemIndex) {
		return item[itemIndex].getSize();
	}
	//The amount of data in the package
	uint16_t getItemCount() {
		return itemCount;
	}
	//Packet start bit
	StartBit getStartBit() {
		return startBit;
	}

	void encode() {
		clearCodedBytes();
		int positionPush = 0;

		for (uint16_t i = 0; i < itemCount; i++)
		{
			int size = item[i].getSize(); //size of remaining bits not placed
			byte x = 0;
			uint16_t y = 0;
			while (size > 0)
			{
				x = positionPush % DATA_BITS; //
				y = positionPush / DATA_BITS;
				byte freeBits = DATA_BITS - x; //свободных бит в текущем байте
				byte offsetRight = item[i].getSize() - size; //смещение вправо

				codedBytes[y] |= (byte)(item[i].getValue() >> offsetRight << x);
				size -= freeBits;

				if (getStartBit() == ONE)
					codedBytes[y] &= ~(1 << 7);
				else codedBytes[y] |= 1 << 7;

				if (size <= 0)
				{
					positionPush += freeBits + size;
					break;
				}
				else
				{
					positionPush += freeBits;
				}
			}

			if (getStartBit() == ZERO)
				codedBytes[0] &= ~(1 << 7);
			else codedBytes[0] |= 1 << 7;
		}
	}

	/*Decode package
		dataByte - data byte encoded
		packagePSP - put the result of decoding data bytes
		matrix - intermediate byte array
		RETURN - result package decode*/
	bool decode(int dataByte) {		
		if (dataByte == -1) return false;
		codedBytes[countData] = dataByte;
		if (codedBytes[countData] & (1 << 7) == getStartBit()) {
			codedBytes[0] = codedBytes[countData];
			countData = 1;
			return false;
		}
		countData++;
		if (countData >= sizeCodedBytes)
		{
			int position = 0;
			for (size_t i = 0; i < getItemCount(); i++)
			{
				item[i].setValue(0);
				int size = getItemSize(i);
				while (size > 0)
				{
					byte x = position % DATA_BITS; //смещение вправо
					uint16_t y = position / DATA_BITS;
					int freeBits = 8 - (x + size); //свободных бит в текущем байте
					if (freeBits <= 0) freeBits = 1;
					byte temp = codedBytes[y] << freeBits;
					uint32_t temp2 = temp >> (freeBits + x);
					item[i].setValue(item[i].getValue() | (temp2 << (item[i].getSize() - size)));

					//Serial.println(item[i].getValue());

					byte bit = DATA_BITS - (freeBits - 1) - x;
					position += bit;
					size -= bit;
				}
			}
			countData = 0;
			return true;
		}
		return false;
	}

	//Constructor
	PackagePSP() {}

	template <uint16_t sizeMatrix, uint16_t sizeCodedBytes>
	PackagePSP(StartBit startBit, StructurePackagePSP(&dataUnits)[sizeMatrix], byte(&codedBytes)[sizeCodedBytes]) {
		init(startBit, dataUnits, codedBytes);
	}

	template <uint16_t sizeMatrix, uint16_t sizeCodedBytes>
	void init(StartBit startBit, StructurePackagePSP(&dataUnits)[sizeMatrix], byte(&codedBytes)[sizeCodedBytes]) {
		this->startBit = startBit;
		this->item = dataUnits;
		this->itemCount = sizeMatrix;
		this->codedBytes = codedBytes;
		this->sizeCodedBytes = sizeCodedBytes;
	}
private:
	uint16_t itemCount;
	StartBit startBit;
	StructurePackagePSP* item;
	uint16_t countData = 0;
	byte* codedBytes;
	uint16_t sizeCodedBytes;
	void clearCodedBytes() {
		for (size_t i = 0; i < sizeCodedBytes; i++)
		{
			codedBytes[i] = 0;
		}
	}
};

#endif

