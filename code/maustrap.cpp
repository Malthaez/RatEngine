#include "maustrap.h"

static void
GameOutputSound(Game_Sound_Output_Buffer *soundBuffer, int toneHz)
{
	static float tSine;
	int16_t toneVolume = 3000;
	int wavePeriod = soundBuffer->samplesPerSecond / toneHz;

	int16_t *sampleOut = soundBuffer->samples;
	for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
	{
		float sineValue = sinf(tSine);
		int16_t sampleValue = (int16_t)(sineValue * toneVolume);
		*sampleOut++ = sampleValue;
		*sampleOut++ = sampleValue;

		tSine += 2.0f * Pi32 * 1.0f / (float)wavePeriod;
	}
}

static void
RenderWeirdGradient(
	Game_Offscreen_Buffer *buffer,
	int blueOffset,
	int greenOffset,
	int redOffset)
{
	uint8_t *row = (uint8_t *)buffer->memory;
	for (int y = 0; y < buffer->height; ++y)
	{
		uint32_t *pixel = (uint32_t *)row;
		for (int x = 0; x < buffer->width; ++x)
		{
			uint8_t blue = (uint8_t)(x + blueOffset);
			uint8_t green = (uint8_t)(y + greenOffset);
			uint8_t red = (uint8_t)(y - redOffset);

			*pixel++ = ((red << 16) | (green << 8) | blue);
		}
		row += buffer->pitch;
	}
}

static void
GameUpdateAndRender(Game_Memory *memory,
	Game_Input *input,
	Game_Offscreen_Buffer *buffer,
	Game_Sound_Output_Buffer *soundBuffer)
{
	Assert(sizeof(Game_State) <= memory->permanentStorageSize);

	Game_State *gameState = (Game_State *)memory->permanentStorage;
	if (memory->isInitialized == false)
	{
		char *filename = (char *)__FILE__;

		debug_read_file_result file = DEBUGPlatformReadEntireFile(filename);
		if (file.contents)
		{
			DEBUGPlatformWriteEntireFile((char *)"test.out", file.contentsSize, file.contents);
			DEBUGPlatformFreeFileMemory(file.contents);
		}

		gameState->toneHz = 256;

		// TODO: This should probably belong to the platform
		memory->isInitialized = true;
	}

	Game_Controller_Input *input0 = &input->controllers[0];
	if (input0->isAnalog)
	{
		//Analog movement tuning
		gameState->blueOffset += (int)(4.0f * input0->stick.endX);
		gameState->toneHz = 256 + (int)(128.0f * input0->stick.endY);
	}
	else
	{
		//Digital movement tuning
	}

	// Input.buttonA.endedDown;
	// Input.buttonA.halfTransitions;
	if (input0->buttonDown.endedDown)
	{
		gameState->greenOffset += 1;
	}

	GameOutputSound(soundBuffer, gameState->toneHz);
	RenderWeirdGradient(buffer, gameState->blueOffset, gameState->greenOffset, gameState->redOffset);
}