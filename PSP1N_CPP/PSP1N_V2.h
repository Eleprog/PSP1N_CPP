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

typedef enum ResultDecode {
	END,
	DECODE_OK,
	NOT_DECODE
}ResultDecode;

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

class DataPSP
{
public:
	DataPSP(byte* data, uint16_t sizeData) {
		this->data = data;
		this->sizeData = sizeData;
	}
	uint16_t getSize() {
		return sizeData;
	}
	byte* getData() {
		return data;
	}

private:
	byte* data;
	uint16_t sizeData;
};

//Data packet
class PackagePSP
{
public:
	//Package data
	uint32_t getItemValue(uint16_t itemIndex) {
		return item[itemIndexCheck(itemIndex)].getValue();
	}
	void setItemValue(uint16_t itemIndex, uint32_t value) {
		item[itemIndexCheck(itemIndex)].setValue(value);
	}
	byte getItemSize(uint16_t itemIndex) {
		return item[itemIndexCheck(itemIndex)].getSize();
	}
	//The amount of data in the package
	uint16_t getItemCount() {
		return itemCount;
	}
	//Packet start bit
	StartBit getStartBit() {
		return startBit;
	}

	DataPSP encode() {
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

				buffer[y] |= (byte)(item[i].getValue() >> offsetRight << x);
				size -= freeBits;

				if (getStartBit() == ONE)
					buffer[y] &= ~(1 << 7);
				else buffer[y] |= 1 << 7;

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
				buffer[0] &= ~(1 << 7);
			else buffer[0] |= 1 << 7;
		}
		DataPSP dataPSP(buffer, sizeCodedBytes);
		return dataPSP;
	}

	/*Decode package
		dataByte - data byte encoded
		packagePSP - put the result of decoding data bytes
		matrix - intermediate byte array
		RETURN - result package decode*/
	ResultDecode decode(int dataByte) {		
		if (dataByte == -1) return ResultDecode::END;
		buffer[countData] = dataByte;
		if (buffer[countData] & (1 << 7) == getStartBit()) {
			buffer[0] = buffer[countData];
			countData = 1;
			return ResultDecode::NOT_DECODE;
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
					byte temp = buffer[y] << freeBits;
					uint32_t temp2 = temp >> (freeBits + x);
					item[i].setValue(item[i].getValue() | (temp2 << (item[i].getSize() - size)));

					//Serial.println(item[i].getValue());

					byte bit = DATA_BITS - (freeBits - 1) - x;
					position += bit;
					size -= bit;
				}
			}
			countData = 0;
			return ResultDecode::DECODE_OK;
		}
		return ResultDecode::NOT_DECODE;
	}

	//Constructor
	PackagePSP() {}

	template <uint16_t sizeMatrix, uint16_t sizeCodedBytes>
	PackagePSP(StartBit startBit, StructurePackagePSP(&dataUnits)[sizeMatrix], byte(&buffer)[sizeCodedBytes]) {
		init(startBit, dataUnits, buffer);
	}

	template <uint16_t sizeMatrix, uint16_t sizeCodedBytes>
	void init(StartBit startBit, StructurePackagePSP(&dataUnits)[sizeMatrix], byte(&buffer)[sizeCodedBytes]) {
		this->startBit = startBit;
		this->item = dataUnits;
		this->itemCount = sizeMatrix;
		this->buffer = buffer;
		this->sizeCodedBytes = sizeCodedBytes;
	}
private:
	uint16_t itemCount;
	StartBit startBit;
	StructurePackagePSP* item;
	uint16_t countData = 0;
	byte* buffer;
	uint16_t sizeCodedBytes;	
	void clearCodedBytes() {
		for (size_t i = 0; i < sizeCodedBytes; i++)
		{
			buffer[i] = 0;
		}
	}
	uint16_t itemIndexCheck(uint16_t itemIndex) {
		if (itemIndex >= getItemCount())
		{
			itemIndex = getItemCount() - 1;
		}
		return itemIndex;
	}
};




#endif

