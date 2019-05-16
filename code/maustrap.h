#if !defined(MAUSTRAP_H)

/*
    NOTE:
    MAUSTRAP_RC:
        0 - Developer-only Build
        1 - Release Candidate Build

    MAUSTRAP_SLOW:
        0 - No slow code allowed
        1 - Slow code executes
*/

#if MAUSTRAP_SLOW
#define Assert(expression) if((expression) == false) { *(int *)0 = 0; }
#else
#define Assert(expression)
#endif

#define Kilobytes(value) ((uint64_t)(value) * 1024)
#define Megabytes(value) ((Kilobytes((uint64_t)(value))) * 1024)
#define Gigabytes(value) ((Megabytes((uint64_t)(value))) * 1024)
#define Terabytes(value) ((Gigabytes((uint64_t)(value))) * 1024)
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

struct Game_Offscreen_Buffer
{
	void *memory;
	int   width;
	int   height;
	int   pitch;
};

struct Game_Sound_Output_Buffer
{
    int samplesPerSecond;
    int sampleCount;
    int16_t *samples;
};

struct Game_Input_Analog_Stick
{
    float startX;
    float startY;

    float minX;
    float minY;

    float maxX;
    float maxY;

    float endX;
    float endY;
};

struct Game_Input_Button
{
    int  halfTransitions;
    bool32 endedDown;
};

struct Game_Controller_Input
{
    bool32 isAnalog;
    Game_Input_Analog_Stick stick;

    union
    {
        Game_Input_Button buttons[8];
        struct
        {
            Game_Input_Button       buttonUp;
            Game_Input_Button       buttonDown;
            Game_Input_Button       buttonLeft;
            Game_Input_Button       buttonRight;
            Game_Input_Button       buttonLshoulder;
            Game_Input_Button       buttonRshoulder;
            Game_Input_Button       buttonStart;
            Game_Input_Button       buttonBack;
        };
    };
};

struct Game_Input
{
    Game_Controller_Input controllers[4];
};

struct Game_Memory
{
    bool32 isInitialized;

    uint64_t permanentStorageSize;
    void *   permanentStorage; // NOTE: REQUIRED to be cleared to zero at startup
    uint64_t transientStorageSize;
    void *   transientStorage; // NOTE: REQUIRED to be cleared to zero at startup
};

//We need: Time, Controller, Bitmap Buffer, Sound Buffer
static void GameUpdateAndRender(Game_Memory *memory,
                                Game_Input *input,
                                Game_Offscreen_Buffer *buffer,
                                Game_Sound_Output_Buffer *soundBuffer);

//
//
//

struct Game_State
{
    int toneHz;

    int blueOffset;
    int greenOffset;
    int redOffset;
};

#define MAUSTRAP_H
#endif