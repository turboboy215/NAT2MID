/*Natsume (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long masterBank;
long offset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[4];
long songPtr;
int chanMask;
long bankAmt;
long firstPtr = 0;
int format = 1;

unsigned static char* romData;
unsigned static char* exRomData;

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Natsume (GB/GBC) to TXT converter\n");
	if (args < 4)
	{
		printf("Usage: NAT2TXT <rom> <bank> <offset> (<format>)\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);

			offset = strtol(argv[3], NULL, 16);

			if (args == 5)
			{
				format = strtol(argv[4], NULL, 16);
				masterBank = strtol(argv[2], NULL, 16);
			}
			if (format != 2)
			{
				printf("Using bank: %02X, offset: 0x%04X\n", bank, offset);
			}
			else if (format == 2)
			{
				printf("Using bank: %02X, offset: 0x%04X\n", masterBank, offset);
			}

			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}

			if (format != 2)
			{
				fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
				romData = (unsigned char*)malloc(bankSize);
				fread(romData, 1, bankSize, rom);
				fclose(rom);
			}
			else if (format == 2)
			{
				fseek(rom, ((masterBank - 1) * bankSize), SEEK_SET);
				romData = (unsigned char*)malloc(bankSize);
				fread(romData, 1, bankSize, rom);

			}

			/*Convert the songs*/
			i = offset - bankAmt;
			firstPtr = ReadLE16(&romData[i]);
			songNum = 1;

			while (i < (firstPtr - bankAmt))
			{
				songPtr = ReadLE16(&romData[i]);
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				if (songPtr > bankAmt && songPtr < (bankAmt * 2))
				{
					song2txt(songNum, songPtr);
				}
				i += 2;
				songNum++;
			}


			printf("The operation was successfully completed!\n");
			return 0;
		}
	}
}

/*Convert the song data to TXT*/
void song2txt(int songNum, long ptr)
{
	unsigned char mask[3];
	long chanPtrs[4];
	long romPos = 0;
	long seqPos = 0;
	int curTrack = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int chanSpeed = 0;
	int octave = 0;
	int transpose = 0;
	long macroPos = 0;
	long macroRet = 0;
	int repeat1 = 0;
	int repeat2 = 0;
	long repeat1Pos = 0;
	long repeat2Pos = 0;
	unsigned char command[8];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;

	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file seqs.txt!\n");
		exit(2);
	}
	else
	{
		romPos = ptr - bankAmt;
		mask[0] = romData[romPos];
		mask[1] = romData[romPos + 1];

		fprintf(txt, "Mask: %01X, %01X\n", mask[0], mask[1]);

		/*If later format, also get the current song's bank*/
		if (format == 2)
		{
			bank = romData[romPos + 2];
			fseek(rom, (bank * bankSize), SEEK_SET);
			exRomData = (unsigned char*)malloc(bankSize);
			fread(exRomData, 1, bankSize, rom);
			fprintf(txt, "Bank: %01X\n", bank);
			romPos++;
		}

		romPos += 2;

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			chanPtrs[curTrack] = ReadLE16(&romData[romPos]);
			fprintf(txt, "Channel %i: 0x%01X\n", (curTrack + 1), chanPtrs[curTrack]);
			romPos += 2;
		}
		fprintf(txt, "\n");

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			seqPos = chanPtrs[curTrack] - bankAmt;
			fprintf(txt, "Channel %i:\n", (curTrack + 1));

			octave = 0;
			transpose = 0;
			seqEnd = 0;
			while (seqEnd == 0 && seqPos < 0x4000)
			{
				if (format != 2)
				{
					command[0] = romData[seqPos];
					command[1] = romData[seqPos + 1];
					command[2] = romData[seqPos + 2];
					command[3] = romData[seqPos + 3];
					command[4] = romData[seqPos + 4];
					command[5] = romData[seqPos + 5];
					command[6] = romData[seqPos + 6];
					command[7] = romData[seqPos + 7];

					if (command[0] < 0xC0)
					{
						lowNibble = (command[0] >> 4);
						highNibble = (command[0] & 15);
						curNote = lowNibble;
						curNoteLen = highNibble;
						fprintf(txt, "Play note: %01X, length: %01X\n", curNote, curNoteLen);
						seqPos++;

					}

					else if (command[0] >= 0xC0 && command[0] < 0xD0)
					{
						highNibble = (command[0] & 15);
						curNoteLen = highNibble;
						fprintf(txt, "Rest: %01X\n", curNoteLen);
						seqPos++;
					}

					else if (command[0] >= 0xD0 && command[0] < 0xD8)
					{
						highNibble = (command[0] & 15);
						octave = highNibble;
						transpose = 0;
						fprintf(txt, "Set octave: %01X\n", octave);
						seqPos++;
					}

					else if (command[0] >= 0xD8 && command[0] < 0xE0)
					{
						highNibble = (command[0] & 15);
						octave = highNibble;
						transpose = 1;
						fprintf(txt, "Set octave + transpose +1: %01X\n", octave);
						seqPos++;
					}

					else if (command[0] >= 0xE0 && command[0] < 0xE6)
					{
						fprintf(txt, "Noise effects?: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] >= 0xE6 && command[0] < 0xE9)
					{
						if (format == 3)
						{
							fprintf(txt, "Set volume: %01X, %01X\n", highNibble, command[1]);
							seqPos += 2;
						}
						else
						{
							fprintf(txt, "Set volume: %01X\n", highNibble);
							seqPos++;
						}
						highNibble = (command[0] & 15);

					}

					else if (command[0] == 0xE9)
					{
						fprintf(txt, "Set up waveform: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xEA)
					{
						fprintf(txt, "Set echo: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xEB)
					{
						fprintf(txt, "Set envelope sweep: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xEC)
					{
						fprintf(txt, "Turn off ''glide''\n");
						seqPos++;
					}

					else if (command[0] >= 0xED && command[0] < 0xF0)
					{
						highNibble = (command[0] & 15);
						fprintf(txt, "Set volume: %01X\n", highNibble);
						seqPos++;
					}

					else if (command[0] == 0xF0)
					{
						repeat1Pos = seqPos + 2;
						repeat1 = command[1];
						fprintf(txt, "Repeat the following notes (v1) %i times\n", repeat1);
						seqPos += 2;
					}

					else if (command[0] == 0xF1)
					{
						fprintf(txt, "End of repeat pattern (v1)\n");
						seqPos++;
					}

					else if (command[0] == 0xF2)
					{
						macroPos = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Go to loop point: 0x%04X\n\n", macroPos);
						seqEnd = 1;
					}

					else if (command[0] == 0xF3)
					{
						repeat2Pos = seqPos + 2;
						repeat2 = command[1];
						fprintf(txt, "Repeat the following notes (v2) %i times\n", repeat2);
						seqPos += 2;
					}

					else if (command[0] == 0xF4)
					{
						fprintf(txt, "End of repeat pattern (v2)\n");
						seqPos++;
					}

					else if (command[0] == 0xF5)
					{
						macroPos = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Go to macro: 0x%04X\n", macroPos);
						seqPos += 3;
					}

					else if (command[0] == 0xF6)
					{
						fprintf(txt, "Return from macro\n");
						seqPos++;
					}

					else if (command[0] == 0xF7)
					{
						fprintf(txt, "Sweep up: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xF8)
					{
						fprintf(txt, "Sweep down: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xF9)
					{
						fprintf(txt, "Set attack rate values?: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}

					else if (command[0] == 0xFA)
					{
						fprintf(txt, "Set duty/envelope: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xFB)
					{
						fprintf(txt, "Set envelope parameters: %01X, %01X, %01X, %01X\n", command[1], command[2], command[3], command[4]);
						seqPos += 5;
					}

					else if (command[0] == 0xFC)
					{
						fprintf(txt, "Turn on ''glide''\n");
						seqPos++;
					}

					else if (command[0] == 0xFD)
					{
						chanSpeed = command[1];
						fprintf(txt, "Set channel speed: %01X\n", chanSpeed);
						seqPos += 2;
					}

					else if (command[0] == 0xFE)
					{
						fprintf(txt, "Set noise effect?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xFF)
					{
						fprintf(txt, "End of channel track\n\n");
						seqEnd = 1;
					}

					else
					{
						fprintf(txt, "Unknown command: %01X\n", command[0]);
						seqPos++;
					}
				}

				else if (format == 2)
				{
					command[0] = exRomData[seqPos];
					command[1] = exRomData[seqPos + 1];
					command[2] = exRomData[seqPos + 2];
					command[3] = exRomData[seqPos + 3];
					command[4] = exRomData[seqPos + 4];
					command[5] = exRomData[seqPos + 5];
					command[6] = exRomData[seqPos + 6];
					command[7] = exRomData[seqPos + 7];

					if (command[0] < 0x80)
					{
						curNote = command[0];
						curNoteLen = command[1];
						fprintf(txt, "Play note: %01X, length: %01X\n", curNote, curNoteLen);
						seqPos += 2;
					}

					else if (command[0] == 0x80)
					{
						curNoteLen = command[1];
						fprintf(txt, "Rest, length: %01X\n", curNoteLen);
						seqPos += 2;
					}

					else if (command[0] > 0x80 && command[0] < 0xD0)
					{
						curNote = command[0];
						curNoteLen = command[1];
						fprintf(txt, "Play note: %01X, length: %01X\n", curNote, curNoteLen);
						seqPos += 2;
					}

					else if (command[0] >= 0xD0 && command[0] < 0xE0)
					{
						fprintf(txt, "Set volume/attack rate: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xE0)
					{
						fprintf(txt, "Set envelope size?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xE1)
					{
						fprintf(txt, "Stereo flag 1?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xE2)
					{
						fprintf(txt, "Stereo flag 2?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xE3)
					{
						fprintf(txt, "Set waveform: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] >= 0xE4 && command[0] < 0xE8)
					{
						fprintf(txt, "Set duty: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] == 0xE8)
					{
						transpose = (signed char)command[1];
						fprintf(txt, "Set transpose: %i\n", transpose);
						seqPos += 2;
					}

					else if (command[0] == 0xE9)
					{
						fprintf(txt, "Set echo: %01X\n", command[0]);
						seqPos += 2;
					}

					else if (command[0] >= 0xEA && command[0] < 0xF0)
					{
						fprintf(txt, "Set volume?: %01X\n", command[0]);
						seqPos++;
					}

					else if (command[0] == 0xF0)
					{
						repeat1Pos = seqPos + 2;
						repeat1 = command[1];
						fprintf(txt, "Repeat the following notes (v1) %i times\n", repeat1);
						seqPos += 2;
					}

					else if (command[0] == 0xF1)
					{
						fprintf(txt, "End of repeat pattern (v1)\n");
						seqPos++;
					}

					else if (command[0] == 0xF2)
					{
						repeat2Pos = seqPos + 2;
						repeat2 = command[1];
						fprintf(txt, "Repeat the following notes (v2) %i times\n", repeat2);
						seqPos += 2;
					}

					else if (command[0] == 0xF3)
					{
						fprintf(txt, "End of repeat pattern (v2)\n");
						seqPos++;
					}

					else if (command[0] == 0xF4)
					{
						macroPos = ReadLE16(&exRomData[seqPos + 1]);
						fprintf(txt, "Go to loop point: 0x%04X\n\n", macroPos);
						seqEnd = 1;
					}

					else if (command[0] == 0xF5)
					{
						macroPos = ReadLE16(&exRomData[seqPos + 1]);
						fprintf(txt, "Go to macro: 0x%04X\n", macroPos);
						seqPos += 3;
					}

					else if (command[0] == 0xF6)
					{
						fprintf(txt, "Return from macro\n");
						seqPos++;
					}

					else if (command[0] == 0xF7)
					{
						fprintf(txt, "Set sweep?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xF8)
					{
						fprintf(txt, "Set tuning: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xF9)
					{
						fprintf(txt, "Set attack rate values?: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}

					else if (command[0] == 0xFA)
					{
						fprintf(txt, "Set envelope: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xFB)
					{
						fprintf(txt, "Set envelope parameters: %01X, %01X, %01X, %01X\n", command[1], command[2], command[3], command[4]);
						seqPos += 5;
					}

					else if (command[0] == 0xFC)
					{
						fprintf(txt, "Turn on ''glide''\n");
						seqPos++;
					}

					else if (command[0] == 0xFD)
					{
						fprintf(txt, "Turn off ''glide''\n");
						seqPos++;
					}

					else if (command[0] == 0xFE)
					{
						fprintf(txt, "Set noise effect?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xFF)
					{
						fprintf(txt, "End of channel track\n\n");
						seqEnd = 1;
					}

					else
					{
						fprintf(txt, "Unknown command: %01X\n", command[0]);
						seqPos++;
					}

				}


			}
		}
		fclose(txt);
	}
}