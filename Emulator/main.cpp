#include <cstdint>
#include <iostream>
#include <bitset>
#include <string>
#include <memory>

#include <windows.h>
#include <thread>

using Byte = uint8_t;

struct Bus {
	uint16_t address;
	Byte data;
	bool RWB;
};

struct Memory {
	Byte data[(unsigned int)65536];

	void Initialize() {
		for (int i = 0; i < sizeof(data); i++) {
			data[i] = 0;
		}
	}

	void Read(Bus& bus) const{
		bus.data = data[bus.address];
	}

	void Write(Bus& bus) {
		data[bus.address] = bus.data;
	}

	Byte& operator[](int index){
		return data[index];
	}

	int GetSize() const noexcept {
		return sizeof(data);
	}
};

struct Flags {
	uint8_t N : 1;
	uint8_t V : 1;
	uint8_t B : 1;
	uint8_t D : 1;
	uint8_t I : 1;
	uint8_t Z : 1;
	uint8_t C : 1;
};

struct OpCodes {
	//opcodes
	static constexpr Byte LDA_ZP = 0xA5;
	static constexpr Byte LDA_IM = 0xA9;
	static constexpr Byte LDA_AB = 0xA3;
	static constexpr Byte LDA_IN = 0xA1;

	static constexpr Byte LDX_IM = 0xA2;
	static constexpr Byte LDX_ZP = 0xA6;
	static constexpr Byte LDX_AB = 0xAE;

	static constexpr Byte LDY_IM = 0xA0;
	static constexpr Byte LDY_ZP = 0xA4;
	static constexpr Byte LDY_AB = 0xAC;

	static constexpr Byte STA_ZP = 0x89;
	static constexpr Byte STA_AB = 0x8D;

	static constexpr Byte STX_ZP = 0x86;
	static constexpr Byte STX_AB = 0x8E;

	static constexpr Byte STY_ZP = 0x84;
	static constexpr Byte STY_AB = 0x8C;

	static constexpr Byte TAX_IM = 0xAA;
	static constexpr Byte TXA_IM = 0x8A;

	static constexpr Byte TAY_IM = 0xA8;
	static constexpr Byte TYA_IM = 0x98;

	static constexpr Byte JMP_AB = 0x4C;

	static constexpr Byte AND_IM = 0x29;
	static constexpr Byte AND_ZP = 0x25;
	static constexpr Byte AND_AB = 0x2D;
};

struct CPU {
	Byte IR;
	Byte X;
	Byte Y;
	Byte A;
	uint16_t PC;
	
	Bus dataBus;

	Flags flagRegister;

	bool RDY;

	bool clock;

	void Reset(Memory& memory) {
		IR = 0x00;
		X = 0x00;
		Y = 0x00;
		A = 0x00;
		
		dataBus.address = 0x0;
		dataBus.data = 0x0;
		dataBus.RWB = 0;

		flagRegister.B = flagRegister.C = flagRegister.D = flagRegister.I = flagRegister.N = flagRegister.V = flagRegister.Z = 0;

		RDY = false;
		PC = 0x8000;
		std::cout << "Inital PC: " << std::bitset<16>(PC) << "\n";
		memory.Initialize();
	}

	Byte& FetchByte(int& cycles, Memory& mem) {
		dataBus.address = PC;
		dataBus.RWB = 1;

		mem.Read(dataBus);
		Byte data = dataBus.data;

		PC++;
		cycles--;
		return data;
	}

	void Step(Memory& mem) {
		if (!RDY) return;

		int cycles = 1;

		IR = FetchByte(cycles, mem);
		switch (IR) {
		case OpCodes::LDA_IM:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::LDA_AB:
			ExecuteInstruction(3, mem);
			break;
		case OpCodes::LDA_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::LDX_IM:
			ExecuteInstruction(2, mem);
			std::cout << "LDX IM \n";
			break;
		case OpCodes::LDX_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::LDX_AB:
			ExecuteInstruction(3, mem);
			break;

		case OpCodes::LDY_IM:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::LDY_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::LDY_AB:
			ExecuteInstruction(3, mem);
			break;

		case OpCodes::STA_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::STA_AB:
			ExecuteInstruction(3, mem);
			break;

		case OpCodes::STX_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::STX_AB:
			ExecuteInstruction(3, mem);
			break;

		case OpCodes::STY_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::STY_AB:
			ExecuteInstruction(3, mem);
			break;

		case OpCodes::TAX_IM:
			ExecuteInstruction(1, mem);
			break;
		case OpCodes::TXA_IM:
			ExecuteInstruction(1, mem);
			break;
		case OpCodes::TAY_IM:
			ExecuteInstruction(1, mem);
			break;
		case OpCodes::TYA_IM:
			ExecuteInstruction(1, mem);
			break;

		case OpCodes::JMP_AB:
			ExecuteInstruction(3, mem);
			break;
			
		case OpCodes::AND_IM:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::AND_ZP:
			ExecuteInstruction(2, mem);
			break;
		case OpCodes::AND_AB:
			ExecuteInstruction(3, mem);
			break;
		}
	}
	void ExecuteInstruction(int cycles, Memory& mem) {
		if (!RDY) return;
		//IR = FetchByte(cycles, mem);

		switch (IR) {
		case OpCodes::LDA_ZP: //LDA
			A = ZeroFlag(cycles, mem);
			break;
		case OpCodes::LDA_IM:
			A = Immediate(cycles, mem);
			break;
		case OpCodes::LDA_AB:
			A = mem[Absolute(cycles, mem)];
			break;
		case OpCodes::LDX_IM:
			X = Immediate(cycles, mem);
			//std::cout << "LDX IM executed with " << std::bitset<8>(X) << "!\n";
 			break;
		case OpCodes::LDX_ZP:
			X = ZeroFlag(cycles, mem);
			break;
		case OpCodes::LDX_AB:
			X = mem[Absolute(cycles, mem)];
			break;

		case OpCodes::LDY_IM:
			Y = Immediate(cycles, mem);
			break;
		case OpCodes::LDY_ZP:
			Y = ZeroFlag(cycles, mem);
			break;
		case OpCodes::LDY_AB:
			Y = mem[Absolute(cycles, mem)];
			break;

		case OpCodes::STA_ZP:
			mem[ZeroFlag(cycles, mem)] = A;
			break;
		case OpCodes::STA_AB:
			mem[Absolute(cycles, mem)] = A;
			break;
		case OpCodes::STX_ZP:
			mem[ZeroFlag(cycles, mem)] = X;
			break;
		case OpCodes::STX_AB:
			mem[Absolute(cycles, mem)] = X;
			break;
		case OpCodes::STY_ZP:
			mem[ZeroFlag(cycles, mem)] = Y;
			break;
		case OpCodes::STY_AB:
			mem[Absolute(cycles, mem)] = Y;
			break;

		case OpCodes::TAX_IM:
			X = A;
			PC++;
			cycles--;
			break;

		case OpCodes::TXA_IM:
			A = X;
			PC++;
			cycles--;
			break;

		case OpCodes::TAY_IM:
			Y = A;
			PC++;
			cycles--;
			break;
		case OpCodes::TYA_IM:
			A = Y;
			PC++;
			cycles--;
			break;

		case OpCodes::JMP_AB: {
			Byte high = FetchByte(cycles, mem);
			Byte low = FetchByte(cycles, mem);
			PC = (high << 8) | low;
			break;
		}

		case OpCodes::AND_IM: {
			Byte data = FetchByte(cycles, mem);
			Byte op = A & data;
			A = op;
			break;
		}
		default:
			break;
		}
		
	}

	Byte Immediate(int& cycles, Memory& mem) {
		return FetchByte(cycles, mem);
	}

	Byte ZeroFlag(int& cycles, Memory& mem) {
		Byte data = FetchByte(cycles, mem);
		flagRegister.Z = (data == 0);
		return data;
	}

	uint16_t Absolute(int& cycles, Memory& mem){
		Byte high = FetchByte(cycles, mem);
		Byte low = FetchByte(cycles, mem);
		
		uint16_t address = (high << 8) | low;
		return address;
	}

	void DissasembleInstruction(uint16_t address, Memory& mem){
		std::cout << std::bitset<8>(mem[address]) << "\n";
	}
};


int main() {


	CPU computer = CPU();
	Memory mem = Memory();
	
	computer.Reset(mem);

	//inline a small program
	mem[0x8000] = OpCodes::LDA_IM;
	mem[0x8001] = 0x03;
	mem[0x8002] = OpCodes::STA_ZP;
	mem[0x8003] = 0x0;
	mem[0x8004] = OpCodes::TAX_IM;
	
	mem[0x8005] = OpCodes::JMP_AB;
	mem[0x8006] = 0x80;
	mem[0x8007] = 0x00;
	
	mem[0x8008] = OpCodes::AND_IM;
	mem[0x8009] = 0x01;

	computer.RDY = true;

	//computer.ExecuteInstruction(2, mem);

	int Clock = 0;
	const float Hertz = 1;

	//clock
	while (true) {
		Clock += 1 * Hertz;

		computer.Step(mem);

		Sleep(1000.0);

		std::cout << "Accumulator Register: " << std::bitset<8>(computer.A) << "\n";
		std::cout << "Address: " << std::bitset<16>(computer.PC) << "(" << std::to_string(computer.PC) << ")"
			<< " contains: " << std::bitset<8>(mem[0x0]) << "\n";
		std::cout << "Index X Register: " << std::bitset<8>(computer.X) << "\n";
	}
}
