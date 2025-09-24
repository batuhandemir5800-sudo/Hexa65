#include <iostream>
#include <string>
#include <vector> 
#include <sstream> 
#include <iomanip> 
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <thread>
#include <chrono>
#include <cstdint>
#include <ranges>
#include <algorithm>
#include <random>
#include <map>
#include <stack>
#include <functional>
#include <tuple> 
#include <cctype>
#include <iterator>
#include <cmath>
#include <variant>
#include <cstdlib>
#pragma comment(lib, "winmm.lib")


std::vector<uint16_t> ram(65536, 0); //Ram 64KB
std::vector<std::string> basicLines(150); // Basic

uint8_t Reg_A = 0; //A değişkeni
uint8_t Reg_X = 0; //X değişkeni
uint8_t Reg_Y = 0; //Y değişkeni

uint16_t PC_Index; //PC sayacı
uint8_t PCRamData; //Ram verisi
uint8_t SP_Index = 0xFF; //SP sayacı

bool ZF = false;
bool NF = false;
bool CF = false;
bool OF = false;
bool DF = false;
uint16_t BasicOutAddr = 0x0200;
uint16_t JMPAdrress;

bool SystemScreenIO = true;
uint16_t PC_Start = 0x1204;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

const size_t SCREEN_BASE = 0x0200; // ekran RAM başlangıç adresi
const size_t COLOR_BASE = 0x1201;
const size_t SCREEN_WIDTH = 48;
const size_t SCREEN_HEIGHT = 40;
const size_t SCREEN_LAYER = SCREEN_HEIGHT * SCREEN_WIDTH;

bool processGraphics = true; // Grafik işlemeyi kontrol eden bayrak
bool processText = true;     // Metin işlemeyi kontrol eden bayrak

std::string DecToHex(unsigned int decimalValue) {

    std::stringstream ss;
    ss << std::hex << decimalValue;
    return ss.str();
}

bool Loadfile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Dosya acilamadi: " << filename << "\n";
        return false;
    }

    // İlk 2 bayt = yükleme adresi (little endian)
    uint8_t lo, hi;
    file.read(reinterpret_cast<char*>(&lo), 1);
    file.read(reinterpret_cast<char*>(&hi), 1);

    if (!file) {
        std::cerr << "PRG dosyasi hatali!\n";
        return false;
    }

    uint16_t loadAddr = lo | (hi << 8);
    std::cout << "Yukleme adresi: $" << std::hex << (int)loadAddr << std::dec << "\n";

    // Kalan veriyi oku
    std::vector<uint8_t> buffer(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    // RAM'e yükle
    for (size_t i = 0; i < buffer.size(); i++) {
        ram[loadAddr + i] = buffer[i];
    }

    // Program counter'ı başlangıca koy
    PC_Index = loadAddr;

    std::cout << "Toplam yuklenen byte: " << buffer.size() << "\n";
    return true;
}

void load() {
}

unsigned int HexToDec(const std::string& hexString) {

    return std::stoul(hexString, nullptr, 16);
}

void Owerflow_ctr() {

    if (Reg_A > 255) {

        Reg_A = 0;
    }

    if (Reg_X > 255) {

        Reg_X = 0;
    }

    if (Reg_Y > 255) {

        Reg_Y = 0;
    }
}

void RamSys()
{
    while (true) {

        std::string RamMod;
        std::string cin_adrress;
        std::string cin_data;
        int cout_adrress;
        int cout_data;

        std::cout << "\nRam:> ";
        std::cin >> RamMod;

        if (RamMod == "wr") {

            std::cout << "Adrress: ";
            std::cin >> cin_adrress;

            cout_adrress = HexToDec(cin_adrress);

            std::cout << "Data: ";
            std::cin >> cin_data;

            cout_data = HexToDec(cin_data);

            ram[cout_adrress] = static_cast<uint8_t>(cout_data);

            std::cout << "Bellekten okunan deger: 0x"
                << static_cast<int>(ram[cout_adrress]) << std::endl;

        }
        else if (RamMod == "rd") {

            std::cout << "Adrress: ";
            std::cin >> cin_adrress;

            cout_adrress = HexToDec(cin_adrress);

            std::cout << "Bellekten okunan deger: 0x"
                << static_cast<int>(ram[cout_adrress]) << std::endl;
        }
        else if (RamMod == "ex") {

            break;
        }

    }
}

void Flag_stat(uint8_t A) {

    if (A == 0) {

        ZF = true;
    }
    else {

        ZF = false;
    }

    if ((A & 0x80)) {

        NF = true;
    }
    else {

        NF = false;
    }
}

void push_byte(uint8_t value) {
    ram[0x0100 + SP_Index--] = value;
}

uint8_t pull_byte() {
    uint8_t high_byte = ram[0x0100 + ++SP_Index];
    return high_byte;
}

void push_word(uint16_t value) {
    ram[0x0100 + SP_Index--] = value & 0xFF;
    ram[0x0100 + SP_Index--] = (value >> 8) & 0xFF;
}

uint16_t pull_word() {
    uint16_t high_byte = ram[0x0100 + ++SP_Index];
    uint16_t low_byte = ram[0x0100 + ++SP_Index];

    return (high_byte << 8) | low_byte;
}

void SPlist() {
    int Read_Index = 256;
    if (false)
    {
        for (size_t i = 0; i < 256; i++)
        {
            std::cout << Read_Index << ": " << static_cast<int>(ram[Read_Index]) << "\n";
            Read_Index++;
        }
    }

}

void RegList() {
    std::cout << "A: " << static_cast<int>(Reg_A) << "\n";
    std::cout << "X: " << static_cast<int>(Reg_X) << "\n";
    std::cout << "Y: " << static_cast<int>(Reg_Y) << "\n";
    std::cout << "ZF: " << ZF << "\n";
    std::cout << "NF: " << NF << "\n";
    std::cout << "CF: " << CF << "\n";
    std::cout << "OF: " << OF << "\n";
    std::cout << "DF: " << OF << "\n";
}

void keyInput() { // I/O için
    char key;

    if (_kbhit()){
        key = _getch();
        ram[0xFF01] = 0x01;
        ram[0xFF00] = key; // Tuşu belleğe yaz
    }
}

uint8_t ADC_BCD(uint8_t operand) {
    // 1. Binary toplama
    uint16_t sum = Reg_A + operand + CF;

    // 2. Alt basamak düzeltmesi
    if ((sum & 0x0F) > 9) {
        sum += 0x06;
    }

    // 3. Üst basamak düzeltmesi ve Carry
    if (sum > 0x99) {
        sum += 0x60;
        CF = true;
    }
    else {
        CF = false;
    }

    // 4. Sonucu ata
    sum = sum & 0xFF;

    // 5. Sonuçları göster
    std::cout << "Reg_A (BCD) = 0x" << std::hex << (int)sum << std::endl;
    std::cout << "Carry = " << CF << std::endl;

    return sum;
}

uint8_t SBC_BCD(uint8_t operand) {
    // 1. Binary çıkarma
    uint16_t diff = Reg_A - operand - CF;

    // 2. Alt basamak düzeltmesi
    if ((diff & 0x0F) > 9) {
        diff -= 0x06;
    }

    // 3. Üst basamak düzeltmesi ve Borrow (Carry Flag)
    if (diff > 0x99) {
        diff -= 0x60;
        CF = false;  // borrow oldu
    }
    else {
        CF = true;   // borrow yok
    }

    // 4. Sonucu ata
    diff = diff & 0xFF;

    // 5. Sonuçları göster
    std::cout << "Reg_A (BCD) = 0x" << std::hex << (int)diff << std::endl;
    std::cout << "Carry (borrow yok=1, var=0) = " << CF << std::endl;

    return diff;
}

void HexaBuff65(int frequency = 440, int duration_ms = 200, int sampleRate = 44100) {
    // Örnek sayısı
    int samples = sampleRate * duration_ms / 1000;
    uint8_t* buffer = new uint8_t[samples];

    // Kare dalga üret
    int period = sampleRate / frequency;
    for (int i = 0; i < samples; i++) {
        buffer[i] = (i % period < period / 2) ? 255 : 0;
    }

    // WAVEFORMATEX ayarı
    WAVEFORMATEX wf = {};
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec = sampleRate;
    wf.wBitsPerSample = 8;
    wf.nBlockAlign = (wf.wBitsPerSample / 8) * wf.nChannels;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

    // Cihaz aç
    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        delete[] buffer;
        return;
    }

    // Buffer hazırla
    WAVEHDR header = {};
    header.lpData = (LPSTR)buffer;
    header.dwBufferLength = samples;
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));

    // Çal
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));

    // Bitene kadar bekle
    Sleep(static_cast<DWORD>(1000 * samples / sampleRate) + 20);

    // Temizle
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
        
    delete[] buffer;

}

void HexaGrap65() {
    // GrapMod parametresi artık bu yeni mantıkta kullanılmıyor, 
        // ancak programın çökmemesi için fonksiyon tanımında kalmalı.

    std::string screenBuffer;
    screenBuffer.reserve(SCREEN_HEIGHT * (SCREEN_WIDTH * 2 + 1));

    int TextIndex = 0;

    system("cls");
    // İmleci (1,1) pozisyonuna al
    screenBuffer += "\x1B[H";

    for (size_t y = 0; y < SCREEN_HEIGHT; ++y) {
        for (size_t x = 0; x < SCREEN_WIDTH; ++x) {
            const size_t offset = y * SCREEN_WIDTH + x;
            const size_t charAddr = SCREEN_BASE + offset;
            const size_t colorAddr = COLOR_BASE + offset;

            uint8_t rawChar = static_cast<uint8_t>(ram[charAddr] & 0xFF);
            uint8_t color = static_cast<uint8_t>(ram[colorAddr] & 0xFF);

            // Renk kodunu ayarla (Her zaman aktif)
            if (color == 0) {
                screenBuffer += "\x1B[39m"; // Varsayılan ön plan rengi
            }
            else {
                screenBuffer += "\x1B[38;5;";
                screenBuffer += std::to_string(static_cast<int>(color));
                screenBuffer += "m";
            }

            // --- YENİ AKILLI MANTIK ---
            // Karakterin koduna bakarak metin mi grafik mi olduğuna karar ver.

            if (rawChar < 128) {
                // KODU 128'DEN KÜÇÜK: Bu bir metin karakteridir.
               
                char c = (rawChar != 0x00) ? static_cast<char>(rawChar) : ' ';
                screenBuffer.push_back(c);

                // Grafik karakterleri 2 birim yer kapladığı için,
                // metin karakterinden sonra bir boşluk ekleyerek hizalamayı koruyalım.
                screenBuffer.push_back(' ');

            }
            else {
                // KODU 128 VEYA ÜSTÜ: Bu bir grafik karakteridir.
                if (rawChar == 0xDB)      screenBuffer += "\xDB\xDB";
                else if (rawChar == 0xB2) screenBuffer += "\xB2\xB2";
                else if (rawChar == 0xB1) screenBuffer += "\xB1\xB1";
                else if (rawChar == 0xB0) screenBuffer += "\xB0\xB0";
                else if (rawChar == 0x20) screenBuffer += "  ";
                else {
                    // Bilinmeyen bir grafik karakteri gelirse, boşluk bırak
                    screenBuffer += "  ";
                }
            }
        }
        screenBuffer += '\n'; // Her satırın sonunda yeni satıra geç
    }

    // Renk ayarlarını sıfırla
    screenBuffer += "\x1B[0m";

    // Oluşturulan tüm ekranı tek seferde yazdır
    std::cout << screenBuffer;
}

void PcSys() {
    std::string SystemCommand;
    std::uint8_t data;

    std::cout << "\nPC:> ";
    std::cin >> SystemCommand;

    if (SystemCommand == "wr") {
        std::cout << "Adrress: "; 
        std::cin >> data;
        PC_Start = data;
    }
    else if (SystemCommand == "rd") {
        std::cout << PC_Start << "\n";
    }
    else if (SystemCommand == "ex") {
        return;
    }

    PcSys();
}

void Control_Unit(int DATA) {
    int eks_data_int = 0;
    std::string eks_data_str;

    uint8_t lowByte;
    uint8_t highByte;
    uint8_t outdata;
    uint8_t inpdata;
    uint16_t adres;
    uint16_t result;
    bool operand1_sign;
    bool operand2_sign;
    bool result_sign;
    int DecA;
    int DecB;
    int8_t offset;
    uint8_t oldCarry;
    uint16_t temp_result;
    uint8_t operand;

    switch (DATA)
    {
        //LDA komutları
    case 169: //LDA Immediate A9
        PC_Index++;
        Reg_A = ram[PC_Index];
        Flag_stat(Reg_A);
        break;

    case 165: //LDA Zeropage A5
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_A = static_cast<int>(ram[eks_data_int]);
        Flag_stat(Reg_A);
        break;


    case 181: //LDA Zeropage,X B5
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_A = static_cast<int>(ram[eks_data_int + Reg_X]);
        Flag_stat(Reg_A);
        break;


    case 173: //LDA Absolute AD
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        std::cout << adres << "\n";

        Reg_A = ram[adres];
        Flag_stat(Reg_A);
        break;


    case 189: //LDA Absolute,X BD
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;

        Reg_A = ram[adres];
        Flag_stat(Reg_A);
        break;


    case 185: //LDA Absolute,Y B9
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;

        Reg_A = ram[adres];
        Flag_stat(Reg_A);
        break;

        //LDX komutları
    case 162: //LDX Immediate A2
        PC_Index++;
        Reg_X = ram[PC_Index];
        Flag_stat(Reg_X);
        break;

    case 166: //LDX Zeropage A6
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_X = static_cast<int>(ram[eks_data_int]);
        Flag_stat(Reg_X);
        break;

    case 182: //LDX Zeropage,Y B6
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_X = static_cast<int>(ram[eks_data_int + Reg_Y]);
        Flag_stat(Reg_X);
        break;

    case 174: //LDX Absolute AE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;

        Reg_X = ram[adres];
        Flag_stat(Reg_X);
        break;

    case 190: //LDX Absolute,Y BE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;

        Reg_X = ram[adres];
        Flag_stat(Reg_X);
        break;

        //LDY komutları
    case 160: //LDY Immediate A0
        PC_Index++;
        Reg_Y = ram[PC_Index];
        Flag_stat(Reg_Y);
        break;

    case 164: //LDY Zeropage A4
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_Y = static_cast<int>(ram[eks_data_int]);
        Flag_stat(Reg_Y);
        break;

    case 180: //LDY Zeropage,X B4
        PC_Index++;
        eks_data_int = ram[PC_Index];
        Reg_Y = static_cast<int>(ram[eks_data_int + Reg_X]);
        Flag_stat(Reg_Y);
        break;

    case 172: //LDY Absolute AC
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;

        Reg_Y = ram[adres];
        Flag_stat(Reg_Y);
        break;

    case 188: //LDY Absolute,X BC
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;

        Reg_Y = ram[adres];
        Flag_stat(Reg_Y);
        break;

        //NOP
    case 234: //NOP EA
        PC_Index++;
        break;

        //STA komutaları
    case 133: //STA Zeropage 85
        PC_Index++;
        lowByte = ram[PC_Index];
        ram[lowByte] = static_cast<uint8_t>(Reg_A);
        break;

    case 149: //STA Zeropage,X 95
        PC_Index++;
        lowByte = ram[PC_Index];
        ram[static_cast<uint8_t>(lowByte + Reg_X)] = static_cast<uint8_t>(Reg_A);
        break;

    case 141: //STA Absolute 8D
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;

        ram[adres] = static_cast<uint8_t>(Reg_A);
        break;

    case 157: //STA Absolute,X 9D
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;

        ram[adres] = static_cast<uint8_t>(Reg_A);
        break;

    case 153: //STA Absolute,Y 99
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;

        ram[adres] = static_cast<uint8_t>(Reg_A);
        break;

        // STX komutları
    case 134: //STX Zeropage 86
        PC_Index++;
        lowByte = ram[PC_Index];
        ram[lowByte] = static_cast<uint8_t>(Reg_X);
        break;

    case 150: //STX Zeropage,Y 96
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_Y;
        ram[lowByte] = static_cast<uint8_t>(Reg_X);
        break;

    case 142: //STX Absolute 8E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);
        ram[adres] = static_cast<uint8_t>(Reg_X);
        break;

        // STY komutları
    case 132: //STY Zeropage 84
        PC_Index++;
        lowByte = ram[PC_Index];
        ram[lowByte] = static_cast<uint8_t>(Reg_Y);
        break;

    case 148: //STY Zeropage,Y 94
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        ram[lowByte] = static_cast<uint8_t>(Reg_Y);
        break;

    case 140: //STY Absolute 8C
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);
        ram[adres] = static_cast<uint8_t>(Reg_Y);
        break;

        // JMP komutları
    case 76: //JMP Absolute 4C
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        JMPAdrress = ((uint16_t)highByte << 8 | lowByte) - 1;
        PC_Index = JMPAdrress;
        break;

        // JIZ komutları
    case 02: //JIZ Absolute 02
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        JMPAdrress = ((uint16_t)highByte << 8 | lowByte) - 1;
        if (ZF == true) {
            PC_Index = JMPAdrress;
        }
        break;

        // JNZ komutları
    case 04: //JNZ Absolute 04
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        JMPAdrress = ((uint16_t)highByte << 8 | lowByte) - 1;
        if (ZF == false) {
            PC_Index = JMPAdrress;
        }
        break;

        // JSR Komutları
    case 32: // JSR Absolute 20
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        JMPAdrress = ((uint16_t)highByte << 8 | lowByte) - 1;

        push_word(PC_Index);
        SPlist();

        PC_Index = JMPAdrress;
        break;

        // RTS komutları
    case 96: // RTS 60
        adres = pull_word();
        SPlist();

        PC_Index = adres;
        break;

        // INC komutları
    case 230: // INC Zeropage E6
        PC_Index++;
        lowByte = ram[PC_Index];

        ram[lowByte] = ram[lowByte] + 1;
        Flag_stat(ram[lowByte]);
        break;

    case 246: // INC Zeropage,X F6
        PC_Index++;
        lowByte = ram[PC_Index];

        ram[static_cast<uint8_t>(lowByte + Reg_X)] = ram[static_cast<uint8_t>(lowByte + Reg_X)] + 1;
        Flag_stat(ram[static_cast<uint8_t>(lowByte + Reg_X)]);
        break;

    case 238: // INC Absolute EE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);

        ram[adres] = ram[adres] + 1;
        Flag_stat(ram[adres]);
        break;

    case 254: // INC Absolute,X FE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);

        ram[static_cast<uint8_t>(adres + Reg_X)] = ram[static_cast<uint8_t>(adres + Reg_X)] + 1;
        Flag_stat(ram[static_cast<uint8_t>(adres + Reg_X)]);
        break;

        // INX komutları
    case 232: // INX E8
        Reg_X++;
        Flag_stat(Reg_X);
        break;

        // INY komutları
    case 200: // INY C8
        Reg_Y++;
        Flag_stat(Reg_Y);
        break;

        //DEC komutları
    case 198: // DEC Zeropage C6
        PC_Index++;
        lowByte = ram[PC_Index];

        ram[lowByte] = ram[lowByte] - 1;
        Flag_stat(ram[lowByte]);
        break;

    case 214: // DEC Zeropage,X D6
        PC_Index++;
        lowByte = ram[PC_Index];

        ram[static_cast<uint8_t>(lowByte + Reg_X)] = ram[static_cast<uint8_t>(lowByte + Reg_X)] - 1;
        Flag_stat(ram[static_cast<uint8_t>(lowByte + Reg_X)]);
        break;

    case 206: // DEC Absolute CE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);

        ram[adres] = ram[adres] - 1;
        Flag_stat(ram[adres]);
        break;

    case 222: // DEC Absolute,X DE
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);

        ram[static_cast<uint8_t>(adres + Reg_X)] = ram[static_cast<uint8_t>(adres + Reg_X)] - 1;
        Flag_stat(ram[static_cast<uint8_t>(adres + Reg_X)]);
        break;

        // DEX komutları
    case 202: // DEX CA
        Reg_X--;
        Flag_stat(Reg_X);
        break;

        // DEY komutları
    case 136: // DEY 88
        Reg_Y--;
        Flag_stat(Reg_Y);
        break;

        // CMP komutları
    case 201: // CMP Immediate C9
        PC_Index++;
        lowByte = ram[PC_Index];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 197: // CMP Zeropage C5
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 213: // CMP Zeropage,X D5
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        lowByte = ram[lowByte];
        std::cout << static_cast<int>(lowByte) << "\n";

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 205: // CMP Absolute CD
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);
        lowByte = ram[adres];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 221: // CMP Absolute,X DD
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        lowByte = ram[adres];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 217: // CMP Absolute,Y D9
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;
        lowByte = ram[adres];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_A - (uint16_t)lowByte;

        ZF = (Reg_A == lowByte);           // Eşitse ZF = 1
        CF = (Reg_A >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

        //CPX komutları
    case 224: // CPX Immediate C0
        PC_Index++;
        lowByte = ram[PC_Index];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_X - (uint16_t)lowByte;

        ZF = (Reg_X == lowByte);           // Eşitse ZF = 1
        CF = (Reg_X >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 192: // CPX Zeropage C4
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_X - (uint16_t)lowByte;

        ZF = (Reg_X == lowByte);           // Eşitse ZF = 1
        CF = (Reg_X >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 196: // CPX Absolute EC
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);
        lowByte = ram[adres];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_X - (uint16_t)lowByte;

        ZF = (Reg_X == lowByte);           // Eşitse ZF = 1
        CF = (Reg_X >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

        //CPY komutları
    case 204: // CPY Immediate CC
        PC_Index++;
        lowByte = ram[PC_Index];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_Y - (uint16_t)lowByte;

        ZF = (Reg_Y == lowByte);           // Eşitse ZF = 1
        CF = (Reg_Y >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 228: // CPY Zeropage E4
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_Y - (uint16_t)lowByte;

        ZF = (Reg_Y == lowByte);           // Eşitse ZF = 1
        CF = (Reg_Y >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    case 236: // CPY Absolute EC
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte);
        lowByte = ram[adres];

        // Doğru karşılaştırma ve bayrak ayarlama
        temp_result = (uint16_t)Reg_Y - (uint16_t)lowByte;

        ZF = (Reg_Y == lowByte);           // Eşitse ZF = 1
        CF = (Reg_Y >= lowByte);           // A >= M ise CF = 1
        NF = (temp_result & 0x80) != 0;    // Sonucun 7. biti NF'ye
        break;

    // Transfer komutları karışık [TAX AA, TAY A8, TSX BA, TXA 8A, TXS 9A, TYA 98]
    case 170: // TAX AA 
        Reg_X = Reg_A;
        break;

    case 168: // TAY A8 
        Reg_Y = Reg_A;
        break;

    case 186: // TSX BA
        Reg_X = SP_Index;
        break;

    case 138: // TXA 8A
        Reg_A = Reg_X;
        break;

    case 154: // TXS 9A
        SP_Index = Reg_X;
        break;

    case 152: // TYA 98
        Reg_A = Reg_Y;
        break;

        //SP komutları karışık [PHA 48, PHP 08, PLA 68, PLP 28]
    case 72: // PHA 48
        push_byte(Reg_A);
        break;

    case 8: // PHP 08
        push_byte(SP_Index);
        break;

    case 104: // PLA 68
        Reg_A = pull_byte();
        Flag_stat(Reg_A);
        break;

    case 40: // PLP 28
        SP_Index = pull_byte();
        Flag_stat(SP_Index);
        break;

        // ADC komutları
    case 105: // ADC Immediate 69
        PC_Index++;
        lowByte = ram[PC_Index];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

    case 101: // ADC Zeropage 65
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

    case 117: // ADC Zeropage,X 75
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte + Reg_X];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

    case 109: // ADC Absolute 6D
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        lowByte = ram[adres];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

    case 125: // ADC Absolute,X 7D
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        lowByte = ram[adres];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

    case 121: // ADC Absolute,Y 79
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;
        lowByte = ram[adres];

        if (DF) {// BDC
            Reg_A = ADC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { // BIN
            result = Reg_A + lowByte + CF;
            if (result > 255) {
                CF = true;
            }
            else {
                CF = false;
            }
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (lowByte ^ result) & 0x80) != 0;

            Reg_A = result;
            Flag_stat(Reg_A);
        }
        break;

        // SBC komutları
    case 233: // SBC Immediate E9
        PC_Index++;
        lowByte = ram[PC_Index];

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;
    
    case 229: // SBC Zeropage E5
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte];

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;

    case 245: // SBC Zeropage,X F5
        PC_Index++;
        lowByte = ram[PC_Index];
        lowByte = ram[lowByte + Reg_X];
        result = Reg_A - lowByte - (1 - CF);

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;

    case 237: // SBC Absolute ED
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        lowByte = ram[adres];

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;

    case 253: // SBC Absolute,X FD
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        lowByte = ram[adres];

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;

    case 249: // SBC Absolute,Y F9
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_Y;
        lowByte = ram[adres];

        if (DF) { // BDC
            Reg_A = SBC_BCD(lowByte);
            Flag_stat(Reg_A);
        }
        else { //BIN
            result = Reg_A - lowByte - (1 - CF);
            operand1_sign = (Reg_A & 0x80) != 0;
            operand2_sign = (lowByte & 0x80) != 0;
            result_sign = (result & 0x80) != 0;

            OF = ((Reg_A ^ result) & (~lowByte ^ result) & 0x80) != 0;
            if (Reg_A >= (lowByte + (1 - CF))) {
                CF = true;
            }
            else {
                CF = false;
            }
            Reg_A = (unsigned char)result;
            Flag_stat(Reg_A);
        }
        break;
    
        // Clear flag komutları
    case 24: // CLC 18,
        CF = false;
        break;

    case 216: // CLD 18,
        DF = false;
        break;

    case 184: // CLV 18,
        OF = false;
        break;
    
        // Set flag komutları
    case 56: // SEC 38
        CF = true;
        break;

    case 248: // SED F8
        DF = true;
        break;

    case 130: // SEV 82
        OF = true;
        break;

        // AFC
    case 231: // AFC E7
        ZF = false;
        NF = false;
        CF = false;
        OF = false;
        DF = false;
        break;

        //Dallanma Ailesi [BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC]
    case 240: // BEQ F0
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (ZF == true) {
            PC_Index += offset-1;
        }
        break;

    case 208: // BNE D0
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (ZF == false) {
            PC_Index += offset - 1;
        }
        break;

    case 176: // BCS B0
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (CF == true) {
            PC_Index += offset - 1;
        }
        break;

    case 144: // BCC 90
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (CF == false) {
            PC_Index += offset - 1;
        }
        break;

    case 48: // BMI 30
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (NF == true) {
            PC_Index += offset - 1;
        }
        break;

    case 16: // BPL 10
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (NF == false) {
            PC_Index += offset - 1;
        }
        break;

    case 112: // BVS 70
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (OF == true) {
            PC_Index += offset - 1;
        }
        break;

    case 80: // BVC 50
        PC_Index++;
        offset = (int8_t)ram[PC_Index];
        if (OF == false) {
            PC_Index += offset - 1;
        }
        break;

    
        // ASL komutları
    case 10: // ASL Accumulator 0A
        CF = (Reg_A & 0x80) >> 7;
        Reg_A <<= 1;
        Flag_stat(Reg_A);
        break;

        
    case 6: // ASL Zeropage 06
        PC_Index++;
        lowByte = ram[PC_Index];
        CF = (ram[lowByte] & 0x80) >> 7;
        ram[lowByte] <<= 1;
        Flag_stat(ram[lowByte]);
        break;

        
    case 22: // ASL Zeropage,X 16
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        CF = (ram[lowByte] & 0x80) >> 7;
        ram[lowByte] <<= 1;
        Flag_stat(ram[lowByte]);
        break;

        
    case 14: // ASL Absolute 0E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        CF = (ram[adres] & 0x80) >> 7;
        ram[adres] <<= 1;
        Flag_stat(ram[adres]);
        break;

        
    case 30: // ASL Absolute,X 1E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        CF = (ram[adres] & 0x80) >> 7;
        ram[adres] <<= 1;
        Flag_stat(ram[adres]);
        break;

        // LSR komutları
    case 74: // LSR Accumulator 4A
        CF = (Reg_A & 0x01);   
        Reg_A >>= 1;           
        Flag_stat(Reg_A);      
        NF = false;            
        break;

    case 70: // LSR Zeropage 46
        PC_Index++;
        lowByte = ram[PC_Index];
        CF = (ram[lowByte] & 0x01);   
        ram[lowByte] >>= 1;           
        Flag_stat(ram[lowByte]);      
        NF = false;
        break;

    case 86: // LSR Zeropage,X 56
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        CF = (ram[lowByte] & 0x01);   
        ram[lowByte] >>= 1;
        Flag_stat(ram[lowByte]);
        NF = false;
        break;

    case 78: // LSR Absolute 4E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;

        CF = (ram[adres] & 0x01);
        ram[adres] >>= 1;
        Flag_stat(ram[adres]);
        NF = false;
        break;

    case 94: // LSR Absolute,X 5E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;

        CF = (ram[adres] & 0x01);   
        ram[adres] >>= 1;
        Flag_stat(ram[adres]);
        NF = false;
        break;

        // ROL komutları 
    case 42: // ROL Accumulator 2A
        oldCarry = CF ? 1 : 0;
        CF = (Reg_A & 0x80) >> 7;
        Reg_A = (Reg_A << 1) | oldCarry;
        Flag_stat(Reg_A);
        break;

    
    case 38: // ROL Zeropage 26
        PC_Index++;
        lowByte = ram[PC_Index];
        oldCarry = CF ? 1 : 0;
        CF = (ram[lowByte] & 0x80) >> 7;
        ram[lowByte] = (ram[lowByte] << 1) | oldCarry;
        Flag_stat(ram[lowByte]);
        break;

        
    case 54: // ROL Zeropage,X 36
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        oldCarry = CF ? 1 : 0;
        CF = (ram[lowByte] & 0x80) >> 7;
        ram[lowByte] = (ram[lowByte] << 1) | oldCarry;
        Flag_stat(ram[lowByte]);
        break;

        
    case 46: // ROL Absolute 2E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        oldCarry = CF ? 1 : 0;
        CF = (ram[adres] & 0x80) >> 7;
        ram[adres] = (ram[adres] << 1) | oldCarry;
        Flag_stat(ram[adres]);
        break;

         
    case 62: // ROL Absolute,X 3E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        oldCarry = CF ? 1 : 0;
        CF = (ram[adres] & 0x80) >> 7;
        ram[adres] = (ram[adres] << 1) | oldCarry;
        Flag_stat(ram[adres]);
        break;

        // ROR komutları
    case 106: // ROR Accumulator 6A
        oldCarry = CF ? 1 : 0;
        CF = (Reg_A & 0x01);
        Reg_A = (Reg_A >> 1) | (oldCarry << 7);
        Flag_stat(Reg_A);
        break;

    case 102: // ROR Zeropage 66
        PC_Index++;
        lowByte = ram[PC_Index];
        oldCarry = CF ? 1 : 0;
        CF = (ram[lowByte] & 0x01);
        ram[lowByte] = (ram[lowByte] >> 1) | (oldCarry << 7);
        Flag_stat(ram[lowByte]);
        break;

    case 118: // ROR Zeropage,X 76
        PC_Index++;
        lowByte = ram[PC_Index] + Reg_X;
        oldCarry = CF ? 1 : 0;
        CF = (ram[lowByte] & 0x01);
        ram[lowByte] = (ram[lowByte] >> 1) | (oldCarry << 7);
        Flag_stat(ram[lowByte]);
        break;

    case 110: // ROR Absolute 6E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = (uint16_t)highByte << 8 | lowByte;
        oldCarry = CF ? 1 : 0;
        CF = (ram[adres] & 0x01);
        ram[adres] = (ram[adres] >> 1) | (oldCarry << 7);
        Flag_stat(ram[adres]);
        break;

    case 126: // ROR Absolute,X 7E
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8 | lowByte) + Reg_X;
        oldCarry = CF ? 1 : 0;
        CF = (ram[adres] & 0x01);
        ram[adres] = (ram[adres] >> 1) | (oldCarry << 7);
        Flag_stat(ram[adres]);
        break;

        // Bit komutları
    case 36: // BIT Zeropage 24
        PC_Index++;
        lowByte = ram[PC_Index];
        outdata = Reg_A & ram[lowByte];
        ZF = (outdata == 0);
        NF = (ram[lowByte] & 0x80) != 0;
        OF = (ram[lowByte] & 0x40) != 0;
        break;

    case 44: // BIT Absolute 2C
        PC_Index++;
        lowByte = ram[PC_Index];
        PC_Index++;
        highByte = ram[PC_Index];
        adres = ((uint16_t)highByte << 8) | lowByte;
        outdata = Reg_A & ram[adres];
        ZF = (outdata == 0);
        NF = (ram[adres] & 0x80) != 0;
        OF = (ram[adres] & 0x40) != 0;
        break;
    
    case 243: // VRC
        for (size_t i = 512; i < 1600; i++)
        {
            ram[i] = 0x00;
        }
        break;

    default:
        break;
    }
}

void Program_Counter() {
    PC_Index = PC_Start;

    while (true) {
        int PCRamData = (ram[PC_Index]);
        if (PCRamData == 0x00) {
            break;
        }
        keyInput();
        Control_Unit(PCRamData);
        HexaGrap65();

        if (ram[0x1204] == 0x01) {
            HexaBuff65(ram[0x1202], ram[0x1203]);
        }
        PC_Index++;
    }

}

void ramlist() {

    PC_Index = 2564;

    while (true)
    {   
        PC_Index++;
        int PCRamData = (ram[PC_Index]);
        std::cout << "PC: " << PC_Index << ", Opcode: " << PCRamData << "\n";
        if (PCRamData == 0) {
            break;
        }
    }
}

std::vector<std::string> splitString(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);  // İŞTE BURADA KULLANILIYOR
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}


//        ROM API
// 
// RAM OPERATOR
void RamWrite(uint16_t adrress, uint8_t data) {
    ram[adrress] = data;
}
void RamTransfer_AB(uint16_t adrressA, uint16_t adrressB) {
    ram[adrressB] = ram[adrressA];
}
void RamAdd_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    ram[adrressC] = ram[adrressA] + ram[adrressB];
}
void RamSbc_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    ram[adrressC] = ram[adrressA] - ram[adrressB];
}
void RamAnd_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    ram[adrressC] = ram[adrressA] & ram[adrressB];
}
void RamOr_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    ram[adrressC] = ram[adrressA] | ram[adrressB];
}
void RamXor_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    ram[adrressC] = ram[adrressA] ^ ram[adrressB];
}
void RamNot_AB(uint16_t adrressA, uint16_t adrressB) {
    ram[adrressB] = ~ram[adrressA];
}
int RamCompare_AB(uint16_t adrressA, uint16_t adrressB) {
    if (ram[adrressA] == ram[adrressB]) {
        return 0; // Eşit
    }
    else if (ram[adrressA] > ram[adrressB]) {
        return 1; // Birinci değer büyük
    }
    else {
        return -1; // İkinci değer büyük
    }
}
void RamShiftLeft_AB(uint16_t adrressA, uint16_t adrressB) {
    ram[adrressB] = ram[adrressA] << 1;
}
void RamShiftRight_AB(uint16_t adrressA, uint16_t adrressB) {
    ram[adrressB] = ram[adrressA] >> 1;
}
void PushByte(uint8_t data) {
    push_byte(data);
}
void RamMultiply_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    // 8-bit verilerle çarpma
    uint8_t result = ram[adrressA] * ram[adrressB];
    ram[adrressC] = result;
}
void RamDivide_ABC(uint16_t adrressA, uint16_t adrressB, uint16_t adrressC) {
    // 8-bit verilerle çarpma
    uint8_t result = ram[adrressA] / ram[adrressB];
    ram[adrressC] = result;
}
uint8_t PullByte() {
    return pull_byte();
}

// PPU OPERATOR
void PpuClearVram() {
    for (size_t i = 0; i < (SCREEN_LAYER*2); i++)
    {
        ram[SCREEN_BASE + i] = 0x00;
    }
}
void PpuClearColor() {
    for (size_t i = 0; i < SCREEN_LAYER; i++)
    {
        ram[COLOR_BASE + i] = 0x00;
    }

}
void PpuClearPixel() {
    for (size_t i = 0; i < SCREEN_LAYER; i++)
    {
        ram[SCREEN_BASE + i] = 0x00;
    }

}
void PpuClearLine(int line) {
    for (size_t i = 0; i < SCREEN_WIDTH; i++)
    {
        ram[(SCREEN_BASE + line * SCREEN_WIDTH) + i] = 0x00;
    }   
}
void PpuWriteText(int x, int y, std::string text, uint8_t color) {

    for (size_t i = 0; i < text.size(); i++)
    {
        ram[SCREEN_BASE + ((SCREEN_WIDTH * (y)) + x + i)] = int(text[i]);
        ram[COLOR_BASE + ((SCREEN_WIDTH * (y)) + x + i)] = color;
    }
}
void PpuDrawPixel(int x, int y, uint8_t pixel, uint8_t color) {
    ram[SCREEN_BASE + ((SCREEN_WIDTH * (y)) + x)] = pixel;
    ram[COLOR_BASE + ((SCREEN_WIDTH * (y)) + x)] = color;
}
void PpuDrawLine(int x0, int y0, int x1, int y1, uint8_t pixel, uint8_t color) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        // Piksel koy
        PpuDrawPixel(x0, y0, pixel, color);

        // Hedefe ulaştıysak bitir
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
void PpuDrawRect(int x0, int y0, int x1, int y1, uint8_t pixel, uint8_t color) {
    PpuDrawLine(x0, y0, x1, y0, pixel, color);
    PpuDrawLine(x0, y0, x0, y1, pixel, color);
    PpuDrawLine(x1, y1, x1, y0, pixel, color);
    PpuDrawLine(x1, y1, x0, y1, pixel, color);
}
uint8_t PpuGetPixel(int x, int y) {
    return ram[SCREEN_BASE + ((SCREEN_WIDTH * (y)) + x)];
}
uint8_t PpuGetColor(int x, int y) {
    return ram[COLOR_BASE + ((SCREEN_WIDTH * (y)) + x)];
}

// APU OPERAATOR
void ApuPlay(int F, int ms) {
    HexaBuff65(F, ms);
}

// I/O OPERATOR
char InputKey() {
    return ram[0xff00];
}
int InputKeyPush() {
    bool x = ram[0xff01];
    ram[0xff01] = 0x00;
    return x;
}
int IsKeyPressed(char keyCode) {
    if (ram[0xFF00] == keyCode and ram[0xFF01] == 0x01)
    {   
        ram[0xFF00] = 0x00;
        return 1;
    }
    return 0;
}
// -------------------------------------------------------------------------------------


//HEXA BASIC V1.0 ----------------------------------------------------------------------
struct BasicVariable {
    enum Type { NUMBER, STRING };
    Type type;
    int numberValue;
    std::string stringValue;

    // Kurucu metotlar (Constructors)
    BasicVariable() : type(NUMBER), numberValue(0) {} // Varsayılan olarak sayı
    BasicVariable(int num) : type(NUMBER), numberValue(num), stringValue("") {}
    BasicVariable(const std::string& str) : type(STRING), numberValue(0), stringValue(str) {}
};

struct ArrayVariable {
    BasicVariable::Type type; // Dizinin türü (tüm elemanlar aynı türdedir)
    std::vector<BasicVariable> elements; // Elemanları tutan vektör
};
// Dizi değişkenleri için YENİ map'imiz
std::map<std::string, ArrayVariable> basicArrays;

// GLOBALS VALS
std::map<std::string, BasicVariable> basicVariables; 
std::map<int, std::string> basicProgram;           
int basicCursorX = 0, basicCursorY = 0;             
uint8_t basicTextColor = 15;                       
bool runMode = false;
int gotoLine = -1; 
std::map<int, std::string>::iterator currentProgramIterator;
std::map<int, std::string>::iterator jumpToIterator;
std::stack<std::map<int, std::string>::iterator> gosubStack;
struct ForLoopState {
    std::string variableName; // Döngü değişkeninin adı (örn: "I", "X")
    int limitValue;           // Döngünün bitiş değeri (örn: 10)
    int stepValue;            // Artış miktarı (örn: 1 veya 2)
    int startLine;            // Döngünün başladığı satır numarası ('FOR' komutunun olduğu satır)

    // map'in iterator'ü, döngü başına dönmek için kullanılır.
    // Bu, GOTO'dan daha hızlı ve verimlidir.
    std::map<int, std::string>::iterator loopIterator;
};

// BASIC SYSTEM
std::stack<ForLoopState> forLoopStack;
void basicPrint(const std::string& text, bool newline) {
    // Önceki satırları kaydırma (scroll) mantığı
    if (basicCursorY >= SCREEN_HEIGHT) {
        // Ekran RAM'ini bir satır yukarı kaydır
        for (int y = 0; y < SCREEN_HEIGHT - 1; ++y) {
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                uint8_t charToMove = PpuGetPixel(x, y + 1);
                uint8_t colorToMove = PpuGetColor(x, y + 1);
                PpuDrawPixel(x, y, charToMove, colorToMove);
            }
        }
        // Son satırı temizle
        PpuClearLine(SCREEN_HEIGHT - 1);
        basicCursorY = SCREEN_HEIGHT - 1;
    }

    PpuWriteText(basicCursorX, basicCursorY, text, basicTextColor);
    basicCursorX += text.length();

    if (newline) {
        basicCursorX = 0;
        basicCursorY++;
    }
}
std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}
BasicVariable evaluateExpression(const std::string& expr) {
    std::string trimmedExpr = trim(expr);
    if (trimmedExpr.empty()) return BasicVariable("");

    // MATEMATİKSEL İŞLEM (+, -, *, /) AYRIŞTIRMA
    size_t addPos = trimmedExpr.rfind('+');
    if (addPos != std::string::npos) { return BasicVariable(evaluateExpression(trimmedExpr.substr(0, addPos)).numberValue + evaluateExpression(trimmedExpr.substr(addPos + 1)).numberValue); }
    size_t subPos = trimmedExpr.rfind('-');
    if (subPos != std::string::npos && subPos > 0) { return BasicVariable(evaluateExpression(trimmedExpr.substr(0, subPos)).numberValue - evaluateExpression(trimmedExpr.substr(subPos + 1)).numberValue); }
    size_t mulPos = trimmedExpr.rfind('*');
    if (mulPos != std::string::npos) { return BasicVariable(evaluateExpression(trimmedExpr.substr(0, mulPos)).numberValue * evaluateExpression(trimmedExpr.substr(mulPos + 1)).numberValue); }
    size_t divPos = trimmedExpr.rfind('/');
    if (divPos != std::string::npos) {
        int divisor = evaluateExpression(trimmedExpr.substr(divPos + 1)).numberValue;
        if (divisor == 0) { basicPrint("?DIVISION BY ZERO ERROR", true); return BasicVariable(0); }
        return BasicVariable(evaluateExpression(trimmedExpr.substr(0, divPos)).numberValue / divisor);
    }

    // PARANTEZLİ İFADELER: FONKSİYON MU, DİZİ Mİ?
    size_t openParen = trimmedExpr.find('(');
    if (openParen != std::string::npos && trimmedExpr.back() == ')') {
        std::string namePart = trimmedExpr.substr(0, openParen);
        std::string upperNamePart = namePart;
        std::transform(upperNamePart.begin(), upperNamePart.end(), upperNamePart.begin(), ::toupper);

        // BİLİNEN FONKSİYONLARI KONTROL ET
        if (upperNamePart == "PEEK" || upperNamePart == "ABS" || upperNamePart == "SQR" || upperNamePart == "INT" || upperNamePart == "RND" || upperNamePart == "SIN" || upperNamePart == "COS" || upperNamePart == "CHR$" || upperNamePart == "LEN" || upperNamePart == "ASC" || upperNamePart == "LEFT$" || upperNamePart == "RIGHT$" || upperNamePart == "MID$") {

            std::string allArgsStr = trimmedExpr.substr(openParen + 1, trimmedExpr.length() - openParen - 2);
            std::vector<std::string> args;
            std::stringstream ss(allArgsStr);
            std::string arg;
            while (std::getline(ss, arg, ',')) { args.push_back(trim(arg)); }

            // Tek argümanlı fonksiyonlar
            if (args.size() == 1) {
                BasicVariable argVar = evaluateExpression(args[0]);
                if (upperNamePart == "LEN" || upperNamePart == "ASC") { // String argüman alanlar
                    if (argVar.type != BasicVariable::STRING) { basicPrint("?TYPE MISMATCH ERROR", true); return BasicVariable(0); }
                    if (upperNamePart == "LEN") { return BasicVariable(static_cast<int>(argVar.stringValue.length())); }
                    if (upperNamePart == "ASC") {
                        if (argVar.stringValue.empty()) { basicPrint("?ILLEGAL ARGUMENT ERROR", true); return BasicVariable(0); }
                        return BasicVariable(static_cast<int>(static_cast<unsigned char>(argVar.stringValue[0])));
                    }
                }
                else { // Sayısal argüman alanlar
                    if (argVar.type != BasicVariable::NUMBER) { basicPrint("?TYPE MISMATCH ERROR", true); return BasicVariable(0); }
                    double numArg = static_cast<double>(argVar.numberValue);
                    if (upperNamePart == "PEEK") { return BasicVariable(static_cast<int>(ram[static_cast<int>(numArg)])); }
                    if (upperNamePart == "ABS") { return BasicVariable(static_cast<int>(std::abs(numArg))); }
                    if (upperNamePart == "SQR") { return BasicVariable(static_cast<int>(std::sqrt(numArg))); }
                    if (upperNamePart == "INT") { return BasicVariable(static_cast<int>(std::floor(numArg))); }
                    if (upperNamePart == "RND") { return BasicVariable(numArg <= 0 ? 0 : (rand() % static_cast<int>(numArg))); }
                    if (upperNamePart == "SIN") { return BasicVariable(static_cast<int>(std::sin(numArg))); }
                    if (upperNamePart == "COS") { return BasicVariable(static_cast<int>(std::cos(numArg))); }
                    if (upperNamePart == "CHR$") { return BasicVariable(std::string(1, static_cast<char>(numArg))); }
                }
            }
            // Çok argümanlı fonksiyonlar
            else if (upperNamePart == "LEFT$" && args.size() == 2) {
                BasicVariable strVar = evaluateExpression(args[0]); BasicVariable numVar = evaluateExpression(args[1]);
                if (strVar.type != BasicVariable::STRING || numVar.type != BasicVariable::NUMBER) { basicPrint("?TYPE MISMATCH ERROR", true); return BasicVariable(""); }
                int len = numVar.numberValue; if (len < 0) len = 0; return BasicVariable(strVar.stringValue.substr(0, len));
            }
            else if (upperNamePart == "RIGHT$" && args.size() == 2) {
                BasicVariable strVar = evaluateExpression(args[0]); BasicVariable numVar = evaluateExpression(args[1]);
                if (strVar.type != BasicVariable::STRING || numVar.type != BasicVariable::NUMBER) { basicPrint("?TYPE MISMATCH ERROR", true); return BasicVariable(""); }
                int len = numVar.numberValue; if (len < 0) len = 0; if (len > strVar.stringValue.length()) len = strVar.stringValue.length(); return BasicVariable(strVar.stringValue.substr(strVar.stringValue.length() - len));
            }
            else if (upperNamePart == "MID$" && args.size() == 3) {
                BasicVariable strVar = evaluateExpression(args[0]); BasicVariable posVar = evaluateExpression(args[1]); BasicVariable lenVar = evaluateExpression(args[2]);
                if (strVar.type != BasicVariable::STRING || posVar.type != BasicVariable::NUMBER || lenVar.type != BasicVariable::NUMBER) { basicPrint("?TYPE MISMATCH ERROR", true); return BasicVariable(""); }
                return BasicVariable(strVar.stringValue.substr(posVar.numberValue - 1, lenVar.numberValue));
            }
        }

        // DİZİ ELEMANI ERİŞİMİ
        if (basicArrays.count(namePart)) {
            ArrayVariable& arr = basicArrays.at(namePart);
            // HATA BURADAYDI: openPen -> openParen OLARAK DÜZELTİLDİ
            std::string indexStr = trimmedExpr.substr(openParen + 1, trimmedExpr.length() - openParen - 2);
            int index = evaluateExpression(indexStr).numberValue;
            if (index >= 0 && index < arr.elements.size()) {
                return arr.elements[index];
            }
            else {
                basicPrint("?BAD SUBSCRIPT ERROR", true); runMode = false; return BasicVariable(0);
            }
        }
    }

    // TEKİL DEĞİŞKEN ERİŞİMİ
    if (basicVariables.count(trimmedExpr)) {
        return basicVariables.at(trimmedExpr);
    }

    // SABİT DEĞERLER (Sayı veya Metin)
    if (!trimmedExpr.empty() && trimmedExpr.front() == '"' && trimmedExpr.back() == '"') {
        return BasicVariable(trimmedExpr.substr(1, trimmedExpr.length() - 2));
    }
    try {
        if (trimmedExpr.find_first_not_of("0123456789-") == std::string::npos) {
            return BasicVariable(std::stoi(trimmedExpr));
        }
    }
    catch (...) {}

    return BasicVariable("");
}
bool evaluateCondition(const std::string& condition) {
    std::string trimmedCond = trim(condition);

    // Büyük/küçük harf duyarsız arama için koşulun büyük harfli bir kopyasını oluştur.
    std::string upperCond = trimmedCond;
    std::transform(upperCond.begin(), upperCond.end(), upperCond.begin(), ::toupper);

    // Önce "OR" ara, çünkü "AND"e göre daha düşük önceliği vardır.
    // Sağdan sola arama, "A=1 OR B=2 OR C=3" gibi ifadelerin doğru çalışmasını sağlar.
    size_t orPos = upperCond.rfind(" OR ");
    if (orPos != std::string::npos) {
        // "OR"un solundaki ve sağındaki koşulları ayır.
        std::string lhs = trimmedCond.substr(0, orPos);
        std::string rhs = trimmedCond.substr(orPos + 4);
        // Her iki tarafı da ayrı ayrı değerlendir ve sonuçları birleştir.
        return evaluateCondition(lhs) || evaluateCondition(rhs);
    }

    // Sonra "AND" ara.
    size_t andPos = upperCond.rfind(" AND ");
    if (andPos != std::string::npos) {
        // "AND"in solundaki ve sağındaki koşulları ayır.
        std::string lhs = trimmedCond.substr(0, andPos);
        std::string rhs = trimmedCond.substr(andPos + 4);
        // Her iki tarafı da ayrı ayrı değerlendir ve sonuçları birleştir.
        return evaluateCondition(lhs) && evaluateCondition(rhs);
    }

    // Eğer "AND" veya "OR" bulunamazsa, bu basit bir karşılaştırmadır (örn: A > 10).
    // Önceki kodumuzun bu kısmı olduğu gibi kalıyor.
    const std::vector<std::string> ops = { ">=", "<=", "<>", "=", ">", "<" };
    std::string op;
    size_t opPos = std::string::npos;

    for (const auto& currentOp : ops) {
        if ((opPos = trimmedCond.find(currentOp)) != std::string::npos) {
            op = currentOp;
            break;
        }
    }

    if (op.empty()) return false;

    std::string lhs_str = trim(trimmedCond.substr(0, opPos));
    std::string rhs_str = trim(trimmedCond.substr(opPos + op.length()));

    BasicVariable leftVal = evaluateExpression(lhs_str);
    BasicVariable rightVal = evaluateExpression(rhs_str);

    if (leftVal.type != rightVal.type) {
        if (op == "<>") return true;
        if (op == "=") return false;
        return false;
    }

    if (leftVal.type == BasicVariable::NUMBER) {
        if (op == "=") return leftVal.numberValue == rightVal.numberValue;
        if (op == "<>") return leftVal.numberValue != rightVal.numberValue;
        if (op == ">") return leftVal.numberValue > rightVal.numberValue;
        if (op == "<") return leftVal.numberValue < rightVal.numberValue;
        if (op == ">=") return leftVal.numberValue >= rightVal.numberValue;
        if (op == "<=") return leftVal.numberValue <= rightVal.numberValue;
    }
    else { // type == STRING
        if (op == "=") return leftVal.stringValue == rightVal.stringValue;
        if (op == "<>") return leftVal.stringValue != rightVal.stringValue;
        if (op == ">") return leftVal.stringValue > rightVal.stringValue;
        if (op == "<") return leftVal.stringValue < rightVal.stringValue;
        if (op == ">=") return leftVal.stringValue >= rightVal.stringValue;
        if (op == "<=") return leftVal.stringValue <= rightVal.stringValue;
    }

    return false;
}
std::string basicReadLine() {
    std::string input = "";
    int startX = basicCursorX; // Girdinin başladığı X konumu

    while (true) {
        keyInput(); // API'nı kullanarak tuş girdisini al
        if (InputKeyPush()) { // Yeni bir tuşa basıldıysa
            char key = InputKey();

            if (key == '\r') { // Enter tuşu
                basicPrint("", true); // Yeni satıra geç
                return input;
            }
            else if (key == '\b' && !input.empty()) { // Backspace tuşu
                input.pop_back();
                basicCursorX--;
                PpuWriteText(basicCursorX, basicCursorY, " ", basicTextColor);
            }
            else if (isprint(key) && basicCursorX < SCREEN_WIDTH - 1) { // Yazdırılabilir karakterler
                input += key;
                PpuWriteText(basicCursorX, basicCursorY, std::string(1, key), basicTextColor);
                basicCursorX++;
            }
        }
        HexaGrap65(); // Ekranı her döngüde yenile
        Sleep(10); // CPU'yu yormamak için kısa bir bekleme
    }
}
void executeBasicCommand(const std::string& line); 
void runProgram() {
    runMode = true;
    gotoLine = -1;
    jumpToIterator = basicProgram.end();

    // Her çalıştırmadan önce yığınları temizle
    while (!forLoopStack.empty()) forLoopStack.pop();
    while (!gosubStack.empty()) gosubStack.pop();

    if (basicProgram.empty()) {
        runMode = false;
        return;
    }

    auto it = basicProgram.begin();

    while (runMode && it != basicProgram.end()) {
        currentProgramIterator = it;

        // --- YENİ ÇOKLU KOMUT DESTEĞİ ---
        // Mevcut satırın tamamını al
        std::string full_line = it->second;
        // Satırı ':' karakterine göre bölmek için bir string stream oluştur
        std::stringstream line_stream(full_line);
        std::string segment;

        // Satırı ':' karakterine göre böl ve her parçayı ayrı ayrı çalıştır
        while (std::getline(line_stream, segment, ':')) {
            if (!runMode) break; // Bir komut (örn: END) programı durdurduysa, satırın geri kalanını çalıştırma
            executeBasicCommand(trim(segment));
        }
        // --- ÇOKLU KOMUT DESTEĞİ SONU ---

        HexaGrap65();
        Sleep(1);

        // Zıplama komutları (GOTO, RETURN, NEXT) satırın tamamı işlendikten sonra kontrol edilir
        if (jumpToIterator != basicProgram.end()) {
            it = jumpToIterator;
            jumpToIterator = basicProgram.end();
            continue;
        }

        if (gotoLine != -1) {
            it = basicProgram.lower_bound(gotoLine);
            if (it == basicProgram.end()) { runMode = false; }
            gotoLine = -1;
        }
        else {
            it++;
        }
    }

    runMode = false;
    gotoLine = -1;
    if (!forLoopStack.empty()) {
        basicPrint("?FOR WITHOUT NEXT ERROR", true);
    }
}
void executeBasicCommand(const std::string& line) {
    std::vector<std::string> tokens = splitString(line);
    if (tokens.empty()) return;

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (tokens.empty()) return;
    std::string cmd_or_var = tokens[0];
    std::string upper_cmd_or_var = cmd_or_var;
    std::transform(upper_cmd_or_var.begin(), upper_cmd_or_var.end(), upper_cmd_or_var.begin(), ::toupper);

    if (upper_cmd_or_var == "DIM") {
        std::string expr = line.substr(line.find(tokens[1]));
        size_t openParen = expr.find('(');
        size_t closeParen = expr.rfind(')');

        if (openParen == std::string::npos || closeParen == std::string::npos) {
            basicPrint("?SYNTAX ERROR IN DIM", true); return;
        }

        std::string varName = trim(expr.substr(0, openParen));
        std::string sizeStr = expr.substr(openParen + 1, closeParen - openParen - 1);
        int size = evaluateExpression(sizeStr).numberValue;

        if (size < 0) { basicPrint("?ILLEGAL QUANTITY ERROR", true); return; }

        ArrayVariable newArray;
        newArray.type = (varName.back() == '$') ? BasicVariable::STRING : BasicVariable::NUMBER;
        newArray.elements.resize(size + 1); // A(10) -> 11 eleman (0-10)
        basicArrays[varName] = newArray;
        return;
    }
    size_t equalsPos = line.find('=');
    if ((upper_cmd_or_var == "LET") || (equalsPos != std::string::npos && tokens.size() > 1 && tokens[1] == "=")) {
        std::string varExpr = (upper_cmd_or_var == "LET") ? tokens[1] : tokens[0];
        std::string valExpr = trim(line.substr(line.find("=") + 1));

        size_t openParen = varExpr.find('(');
        // Dizi elemanına atama: A(5) = 123
        if (openParen != std::string::npos) {
            std::string varName = varExpr.substr(0, openParen);
            if (basicArrays.count(varName)) {
                ArrayVariable& arr = basicArrays.at(varName);
                std::string indexStr = varExpr.substr(openParen + 1, varExpr.rfind(')') - openParen - 1);
                int index = evaluateExpression(indexStr).numberValue;

                if (index >= 0 && index < arr.elements.size()) {
                    arr.elements[index] = evaluateExpression(valExpr);
                }
                else {
                    basicPrint("?BAD SUBSCRIPT ERROR", true);
                }
            }
            else { basicPrint("?UNDIM'D ARRAY ERROR", true); }
        }
        // Normal değişkene atama: A = 123
        else {
            basicVariables[varExpr] = evaluateExpression(valExpr);
        }
        return;
    }

    else if (upper_cmd_or_var == "PRINT") {
        std::string content = trim(line.substr(5)); // "PRINT " komutundan sonrasını al
        std::stringstream content_stream(content);
        std::string segment;
        bool addNewLine = true;

        if (!content.empty() && (content.back() == ';' || content.back() == ',')) {
            addNewLine = false;
        }

        while (std::getline(content_stream, segment, ';')) {
            BasicVariable val = evaluateExpression(trim(segment));
            if (val.type == BasicVariable::STRING) {
                basicPrint(val.stringValue, false);
            }
            else {
                basicPrint(std::to_string(val.numberValue), false);
            }
        }

        if (addNewLine) {
            basicPrint("", true); // Satır sonuna gelince yeni satıra geç
        }
    }
    else if (upper_cmd_or_var == "INPUT") {
        std::string content = trim(line.substr(5)); // "INPUT " komutundan sonrasını al
        std::string prompt = "";
        std::string varName = "";

        size_t semicolonPos = content.find(';');
        if (semicolonPos != std::string::npos) {
            prompt = evaluateExpression(content.substr(0, semicolonPos)).stringValue;
            varName = trim(content.substr(semicolonPos + 1));
        }
        else {
            varName = content;
        }

        if (!prompt.empty()) {
            basicPrint(prompt, false);
        }
        basicPrint("? ", false);

        std::string input_value = basicReadLine();

        if (varName.back() == '$') {
            basicVariables[varName] = BasicVariable(input_value);
        }
        else {
            try {
                basicVariables[varName] = BasicVariable(std::stoi(input_value));
            }
            catch (...) {
                basicPrint("!NUMBER EXPECTED", true);
            }
        }
    }
    else if (cmd == "LET" || (tokens.size() >= 3 && tokens[1] == "=")) {
        std::string varName = (cmd == "LET") ? tokens[1] : tokens[0];
        std::string valueStr = (cmd == "LET") ? tokens[3] : tokens[2];

        basicVariables[varName] = evaluateExpression(valueStr);
    }
    else if (cmd == "NEW") {
        basicVariables.clear();
        basicProgram.clear();
        PpuClearVram();
        basicCursorX = 0; basicCursorY = 0;
        basicPrint("HEXA BASIC V1.0", true);
        basicPrint("READY.", true);
    }
    else if (cmd == "LIST") {
        for (const auto& linePair : basicProgram) {
            basicPrint(std::to_string(linePair.first) + " " + linePair.second, true);
        }
    }
    else if (cmd == "RUN") {
        runProgram();
    }
    else if (cmd == "HOME" or cmd == "CLS") {
        PpuClearVram();
        basicCursorX = 0; basicCursorY = 0;
    }
    else if (cmd == "COLOR") {
        if (tokens.size() > 1) basicTextColor = std::stoi(tokens[1]);
    }
    else if (cmd == "PSET") {
        if (tokens.size() >= 4) { // PSET X Y RENK formatı için 4 token gerekir
            BasicVariable x = evaluateExpression(tokens[1]);
            BasicVariable y = evaluateExpression(tokens[2]);
            BasicVariable color = evaluateExpression(tokens[3]);

            // Tüm parametrelerin sayı olduğundan emin olalım
            if (x.type == BasicVariable::NUMBER && y.type == BasicVariable::NUMBER && color.type == BasicVariable::NUMBER) {
                PpuDrawPixel(x.numberValue, y.numberValue, 0xDB, color.numberValue);
            }
            else {
                basicPrint("?TYPE MISMATCH ERROR", true);
            }
        }
        else {
            basicPrint("?SYNTAX ERROR", true);
        }
    }
    else if (cmd == "LINE") {
        if (tokens.size() >= 5) {
            PpuDrawLine(std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]), 0xdb, std::stoi(tokens[5]));
        }
    }
    else if (cmd == "BEEP" || cmd == "SOUND") {
        if (tokens.size() >= 3) {
            ApuPlay(std::stoi(tokens[1]), std::stoi(tokens[2]));
        }
    }
    else if (cmd == "GOTO") {
        if (!runMode) {
            basicPrint("?CAN'T GOTO IN DIRECT MODE", true);
            return; // GOTO sadece program çalışırken (RUN) anlamlıdır.
        }
        if (tokens.size() > 1) {
            BasicVariable target = evaluateExpression(tokens[1]);
            if (target.type == BasicVariable::NUMBER) {
                gotoLine = target.numberValue; // Dallanılacak satırı ayarla
            }
            else {
                basicPrint("?SYNTAX ERROR IN GOTO", true);
                runMode = false; // Hata durumunda programı durdur
            }
        }
        else {
            basicPrint("?SYNTAX ERROR IN GOTO", true);
            runMode = false; // Hata durumunda programı durdur
        }
    }
    else if (cmd == "IF") {
        // IF komutunu işlemek için 'THEN' kelimesini bulmamız gerekiyor.
        // Büyük/küçük harf duyarlılığını ortadan kaldırmak için tüm satırı büyük harfe çevirelim.
        std::string upperLine = line;
        std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);

        size_t thenPos = upperLine.find(" THEN ");
        if (thenPos == std::string::npos) {
            basicPrint("?SYNTAX ERROR IN IF", true);
            return;
        }

        // Koşul, 'IF' ile 'THEN' arasındaki kısımdır.
        std::string conditionStr = line.substr(tokens[0].length(), thenPos - tokens[0].length());

        // Koşul doğruysa devam et...
        if (evaluateCondition(conditionStr)) {
            // Aksiyon, 'THEN' kelimesinden sonraki kısımdır.
            std::string actionStr = trim(line.substr(thenPos + strlen(" THEN ")));

            // Aksiyon bir sayı mı? Öyleyse bu bir GOTO'dur.
            try {
                gotoLine = std::stoi(actionStr);
            }
            catch (...) {
                // Değilse, bu başka bir komuttur (PRINT, INPUT vb.).
                // executeBasicCommand'ı bu alt komut için tekrar çağırırız.
                executeBasicCommand(actionStr);
            }
        }
        return; // IF komutu işlendi, fonksiyondan çık.
    }
    else if (cmd == "POKE") {
        // "POKE" komutundan sonraki argümanları al: "adres, deger"
        std::string args = trim(line.substr(4));

        size_t commaPos = args.find(',');
        if (commaPos == std::string::npos) {
            basicPrint("?SYNTAX ERROR IN POKE", true);
            return;
        }

        // Virgülün solunu adres, sağını değer olarak ayır
        std::string addrStr = trim(args.substr(0, commaPos));
        std::string valStr = trim(args.substr(commaPos + 1));

        BasicVariable addrVar = evaluateExpression(addrStr);
        BasicVariable valVar = evaluateExpression(valStr);

        if (addrVar.type != BasicVariable::NUMBER || valVar.type != BasicVariable::NUMBER) {
            basicPrint("?TYPE MISMATCH ERROR", true);
            return;
        }

        int address = addrVar.numberValue;
        int value = valVar.numberValue;

        // Adres ve değer aralıklarını kontrol et
        if (address < 0 || address > 65535) {
            basicPrint("?ILLEGAL ADDRESS ERROR", true);
            return;
        }
        if (value < 0 || value > 255) {
            basicPrint("?ILLEGAL VALUE ERROR", true);
            return;
        }

        // Her şey yolundaysa, RAM'e yaz!
        ram[address] = static_cast<uint16_t>(value);

        // POKE'un etkisini anında görmek için ekranı yenileyelim.
        HexaGrap65();
        }
    else if (cmd == "FOR") {
            if (!runMode) {
                basicPrint("?CAN'T USE FOR IN DIRECT MODE", true);
                return;
            }

            std::string lineWithoutFor = trim(line.substr(3));

            size_t toPos = lineWithoutFor.find(" TO ");
            size_t stepPos = lineWithoutFor.find(" STEP ");
            size_t eqPos = lineWithoutFor.find("=");

            if (toPos == std::string::npos || eqPos == std::string::npos || eqPos > toPos) {
                basicPrint("?SYNTAX ERROR IN FOR", true);
                return;
            }

            ForLoopState newState;
            newState.variableName = trim(lineWithoutFor.substr(0, eqPos));

            std::string startStr = trim(lineWithoutFor.substr(eqPos + 1, toPos - (eqPos + 1)));

            int startValue = evaluateExpression(startStr).numberValue;

            if (stepPos != std::string::npos) {
                newState.limitValue = evaluateExpression(trim(lineWithoutFor.substr(toPos + 4, stepPos - (toPos + 4)))).numberValue;
                newState.stepValue = evaluateExpression(trim(lineWithoutFor.substr(stepPos + 6))).numberValue;
            }
            else {
                newState.limitValue = evaluateExpression(trim(lineWithoutFor.substr(toPos + 4))).numberValue;
                newState.stepValue = 1;
            }

            basicVariables[newState.variableName] = BasicVariable(startValue);

            // --- DEĞİŞEN SATIR BURASI ---
            // Döngünün başlangıcı olarak 'FOR' satırını değil, 'FOR'dan BİR SONRAKİ satırı kaydet.
            newState.loopIterator = std::next(currentProgramIterator);

            forLoopStack.push(newState);
            }
    else if (cmd == "NEXT") {
        if (!runMode) {
            basicPrint("?CAN'T USE NEXT IN DIRECT MODE", true);
            return;
        }
        if (forLoopStack.empty()) {
            basicPrint("?NEXT WITHOUT FOR ERROR", true);
            HexaGrap65(); // Hata mesajını ekrana çiz!
            runMode = false;
            return;
        }

        // Yığının en üstündeki döngü durumunu al
        ForLoopState& currentState = forLoopStack.top();

        // Döngü değişkeninin mevcut değerini al ve artır
        int currentValue = basicVariables[currentState.variableName].numberValue;
        currentValue += currentState.stepValue;
        basicVariables[currentState.variableName] = BasicVariable(currentValue);

        // Döngünün bitip bitmediğini kontrol et
        bool loopFinished = (currentState.stepValue > 0) ? (currentValue > currentState.limitValue)
            : (currentValue < currentState.limitValue);

        if (loopFinished) {
            // Döngü bittiyse, yığından çıkar
            forLoopStack.pop();
        }
        else {
            // Döngü devam ediyorsa, 'FOR'dan sonraki satıra geri dön
            jumpToIterator = currentState.loopIterator;
        }
        }
    else if (cmd == "GOSUB") {
        if (!runMode) {
            basicPrint("?CAN'T USE GOSUB IN DIRECT MODE", true);
            return;
        }
        if (tokens.size() < 2) {
            basicPrint("?SYNTAX ERROR IN GOSUB", true);
            return;
        }

        // Gidilecek satır numarasını al
        BasicVariable target = evaluateExpression(tokens[1]);
        if (target.type != BasicVariable::NUMBER) {
            basicPrint("?SYNTAX ERROR IN GOSUB", true);
            return;
        }

        // Geri dönülecek adresi (GOSUB'dan bir sonraki satırı) yığına ekle
        gosubStack.push(std::next(currentProgramIterator));

        // Hedef satıra zıpla
        gotoLine = target.numberValue;
    }
    else if (cmd == "RETURN") {
        if (!runMode) {
            basicPrint("?CAN'T USE RETURN IN DIRECT MODE", true);
            return;
        }
        if (gosubStack.empty()) {
            basicPrint("?RETURN WITHOUT GOSUB ERROR", true);
            HexaGrap65(); // Hata mesajını ekrana çiz!
            runMode = false;
            return;
        }

        // Yığının en üstündeki geri dönüş adresini al
        std::map<int, std::string>::iterator returnIterator = gosubStack.top();
        gosubStack.pop(); // Adresi yığından çıkar

        // Geri dönüş adresine zıpla
        jumpToIterator = returnIterator;
        }
    else if (cmd == "SAVE") {
        if (tokens.size() < 2) {
            basicPrint("?MISSING FILE NAME", true);
            return;
        }
        std::string filename = tokens[1];
        // Tırnak işaretlerini temizle
        filename.erase(remove(filename.begin(), filename.end(), '\"'), filename.end());

        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            basicPrint("?FILE I/O ERROR", true);
            return;
        }

        for (const auto& linePair : basicProgram) {
            outfile << linePair.first << " " << linePair.second << std::endl;
        }

        outfile.close();
        basicPrint("PROGRAM SAVED.", true);
    }
    else if (cmd == "LOAD") {
        if (tokens.size() < 2) {
            basicPrint("?MISSING FILE NAME", true);
            return;
        }
        std::string filename = tokens[1];
        filename.erase(remove(filename.begin(), filename.end(), '\"'), filename.end());

        std::ifstream infile(filename);
        if (!infile.is_open()) {
            basicPrint("?FILE NOT FOUND ERROR", true);
            return;
        }

        // Yüklemeden önce mevcut programı ve değişkenleri temizle (NEW komutu gibi)
        basicProgram.clear();
        basicVariables.clear();
        PpuClearVram();
        basicCursorX = 0; basicCursorY = 0;

        std::string line;
        while (std::getline(infile, line)) {
            std::stringstream ss(line);
            int lineNum;
            std::string code;

            ss >> lineNum; // Satır numarasını oku
            std::getline(ss, code); // Satırın geri kalanını (kodu) oku

            basicProgram[lineNum] = trim(code);
        }

        infile.close();
        basicPrint("PROGRAM LOADED.", true);
        }
    else if (cmd == "CATALOG" || cmd == "DIR") {
            basicPrint("DISK CATALOG:", true);

            WIN32_FIND_DATAA findData;
            // Mevcut klasördeki (*.) tüm .bas uzantılı dosyaları ara
            HANDLE hFind = FindFirstFileA("*.bas", &findData);

            if (hFind == INVALID_HANDLE_VALUE) {
                basicPrint(" NO .BAS FILES FOUND.", true);
            }
            else {
                do {
                    // Bulunan dosyanın adını ekrana yazdır
                    basicPrint(" " + std::string(findData.cFileName), true);
                } while (FindNextFileA(hFind, &findData) != 0);
                FindClose(hFind);
            }

            basicPrint("READY.", true);
            }
    else if (cmd == "INTKEY$") {
        keyInput();
        int intkey = IsKeyPressed(tokens[2][0]);
        std::cout << tokens[2][0];
        if (intkey == 1)
        {
            basicVariables[tokens[1]] = BasicVariable(std::stoi("1"));
        }
        else
        {
            basicVariables[tokens[1]] = BasicVariable(std::stoi("0"));
        }
    }
    else if (cmd == "SYS") {
        PC_Start = std::stoi(tokens[1]);
        Program_Counter();
    }
    else if (cmd == "END" or cmd == "BREAK") {
        runMode = false;
    }

    else if (tokens.size() >= 3 && tokens[1] == "=") {
        basicVariables[tokens[0]] = evaluateExpression(tokens[2]);
    }
    else {
        basicPrint("?SYNTAX ERROR", true);
    }
}
void HexaBasic() {
    PpuClearVram();
    PpuClearColor();
    basicCursorX = 0;
    basicCursorY = 0;
    basicTextColor = 15;

    basicPrint("HEXA BASIC V1.0", true);
    basicPrint("READY.", true);

    while (true) {
        basicPrint("]", false); // Apple ][ gibi prompt
        std::string line = basicReadLine();

        if (line == "EXIT") break; // Gizli komut: BASIC'ten çıkmak için

        std::string firstToken;
        std::stringstream ss(line);
        ss >> firstToken;

        try {
            int lineNum = std::stoi(firstToken);
            // Satırın geri kalanını al
            std::string code = line.substr(firstToken.length());
            // Başındaki boşlukları temizle
            size_t firstChar = code.find_first_not_of(" \t");
            if (std::string::npos != firstChar) {
                code = code.substr(firstChar);
            }

            if (code.empty()) { // Satır numarası yazıp enter'a basıldıysa satırı sil
                basicProgram.erase(lineNum);
            }
            else { // Satırı programa ekle/değiştir
                basicProgram[lineNum] = code;
            }
        }
        catch (...) {
            // Sayı ile başlamıyorsa, komutu anında çalıştır
            executeBasicCommand(line);
        }
    }
}
//---------------------------------------------------------------------------------------

void main(){

    std::string SystemCommand = "";
    std::string file;
    ram[0x1201] = 0x00;
    ram[0x1202] = 0x00;
    ram[0x1203] = 0x00;

    // Konsolun ANSI escape kodlarını işlemesini sağla
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, consoleMode);

    HexaBasic();

    while (true)
    {
        std::cout << "\nHexa:> ";
        std::cin >> SystemCommand;

        if (SystemCommand == "LOAD") {
            std::cin >> file;
            Loadfile(file);
        }
        else if (SystemCommand == "HEXL") {
            load();
        }
        else if (SystemCommand == "GAME") {
            
            system("cls"); 
        }
        else if (SystemCommand == "RAM") {
            RamSys();
        }
        else if (SystemCommand == "BREAK") {
            break;
        }
        else if (SystemCommand == "RUN") {
            Program_Counter();
        }
        else if (SystemCommand == "FROP") {
            RegList();
        }
        else if (SystemCommand == "RAMLIST") {
            ramlist();
        }
        else if (SystemCommand == "SPLIST") {
            SPlist();
        }
        else if (SystemCommand == "CLS") {
            system("cls");
        }
        else if (SystemCommand == "ROM") {
            HexaBasic();
        }
        else if (SystemCommand == "PC") {
            PcSys();
        }
    }

}
