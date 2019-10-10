#include "win32_maustrap.h"

static bool gRunning;
static Win32_Offscreen_Buffer gBackBuffer;
static LPDIRECTSOUNDBUFFER gSecondaryBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static debug_read_file_result
DEBUGPlatformReadEntireFile(char *filename)
{
	debug_read_file_result result = {};
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			uint32_t fileSize32 = SafeTruncateSizeUInt64((uint32_t)fileSize.QuadPart);
			result.contents = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result.contents)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, result.contents, fileSize32, &bytesRead, NULL) && (fileSize32 == bytesRead))
				{
					// Read Succeeded
					result.contentsSize = fileSize32;
				}
				else
				{
					// Read Failed
					DEBUGPlatformFreeFileMemory(result.contents);
					result.contents = 0;
				}
			}
			else
			{
				// Logging
			}
		}
		else
		{
			// Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// Logging
	}

	return (result);
}

static void
DEBUGPlatformFreeFileMemory(void *memory)
{
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

static bool32
DEBUGPlatformWriteEntireFile(char *filename, uint32_t memorySize, void *memory)
{
	bool32 result = false;

	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, NULL))
		{
			// Write Succeeded
			result = (bytesWritten == memorySize);
		}
		else
		{
			// Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// Logging
	}

	return (result);
}

static void
Win32LoadXInput(void)
{
	HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!xInputLibrary)
	{
		// Logging
		xInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if (!xInputLibrary)
	{
		// Logging
		xInputLibrary = LoadLibraryA("xinput1_3.dll");
	}

	if (xInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
		if (!XInputGetState)
		{
			XInputGetState = XInputGetStateStub;
		}
		else
		{
			//Logging
		}

		XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
		if (!XInputSetState)
		{
			XInputSetState = XInputSetStateStub;
		}
		else
		{
			//Logging
		}
	}
	else
	{
		//Logging
	}
}

static void
Win32InitDSound(HWND hWnd, int32_t samplesPerSecond, int32_t bufferSize)
{
	//Load Library
	HMODULE dSoundLibrary = LoadLibraryA("dsound.dll");

	if (dSoundLibrary)
	{
		//Get a DirectSound object - cooperative
		direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(dSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND directSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &directSound, 0)))
		{
			WAVEFORMATEX waveFormat = {};

			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = samplesPerSecond;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;

			if (SUCCEEDED(directSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY)))
			{
				//Create a primary buffer
				DSBUFFERDESC bufferDescription = {};

				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

				LPDIRECTSOUNDBUFFER primaryBuffer;
				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
				{
					HRESULT Error = primaryBuffer->SetFormat(&waveFormat);
					if (SUCCEEDED(Error))
					{
						OutputDebugStringA("Primary buffer format was set!\n");
					}
					else
					{
						//Logging
					}
				}
				else
				{
					//Logging
				}
			}
			else
			{
				//Logging
			}

			DSBUFFERDESC bufferDescription = {};

			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwFlags = 0;
			bufferDescription.dwBufferBytes = bufferSize;
			bufferDescription.lpwfxFormat = &waveFormat;

			HRESULT Error = directSound->CreateSoundBuffer(&bufferDescription, &gSecondaryBuffer, 0);
			if (SUCCEEDED(Error))
			{
				OutputDebugStringA("Secondary buffer created successfully!\n");
			}
			else
			{
				//Logging
			}
		}
		else
		{
			//Logging
		}
	}
	else
	{
		//Logging
	}
}

static Win32_Window_Dimensions
Win32GetWindowDimensions(HWND window)
{
	Win32_Window_Dimensions result;

	RECT clientRect;
	GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

	return (result);
}

static void
Wind32ResizeDIBSection(
	Win32_Offscreen_Buffer *buffer,
	int width,
	int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	int bytesPerPixel = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bmpMemSize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bmpMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = (buffer->width * bytesPerPixel);
	//Clear to black
}

static void
Win32DisplayBufferInWindow(
	Win32_Offscreen_Buffer *buffer,
	HDC deviceContext,
	int windowWidth,
	int windowHeight)
{
	//Aspect Ratio Correction

	StretchDIBits(deviceContext,
		0, 0, windowWidth, windowHeight,
		0, 0, buffer->width, buffer->height,
		buffer->memory,
		&buffer->info,
		DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT CALLBACK
Win32MainWindowCallback(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{
	case WM_DESTROY:
	{
		gRunning = false;
	}
	break;

	case WM_CLOSE:
	{
		gRunning = false;
	}
	break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	}
	break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32_t vKCode = (uint32_t)wParam;
		bool wasDown = ((lParam & (1 << 30)) != 0);
		bool isDown = ((lParam & (1 << 31)) == 0);

		if (isDown != wasDown)
		{
			switch (vKCode)
			{
			case 'W':
			{
				OutputDebugStringA("W: ");
				if (isDown)
				{
					OutputDebugStringA("IsDown ");
				}
				if (wasDown)
				{
					OutputDebugStringA("WasDown ");
				}
				OutputDebugStringA("\n");
			}
			break;
			case 'A':
			{
			}
			break;
			case 'S':
			{
			}
			break;
			case 'D':
			{
			}
			break;
			case 'Q':
			{
			}
			break;
			case 'E':
			{
			}
			break;
			case VK_UP:
			{
			}
			break;
			case VK_LEFT:
			{
			}
			break;
			case VK_DOWN:
			{
			}
			break;
			case VK_RIGHT:
			{
			}
			break;
			case VK_ESCAPE:
			{
			}
			break;
			case VK_RETURN:
			{
			}
			break;
			case VK_SPACE:
			{
			}
			break;
			}
		}

		bool32 altKeyWasDown = (lParam & (1 << 29));
		if ((vKCode == VK_F4) && altKeyWasDown)
		{
			gRunning = false;
		}
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(hwnd, &paint);
		Win32_Window_Dimensions dimensions = Win32GetWindowDimensions(hwnd);
		Win32DisplayBufferInWindow(&gBackBuffer, deviceContext, dimensions.width, dimensions.height);
		EndPaint(hwnd, &paint);
	}
	break;

	default:
	{
		//OutputDebugStringA("DEFAULT\n");
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	break;
	}

	return (result);
}

static void
Win32ClearSoundBuffer(Win32_Sound_Output *soundOutput)
{
	VOID *region1;
	DWORD region1size;
	VOID *region2;
	DWORD region2size;
	if (SUCCEEDED(gSecondaryBuffer->Lock(0, soundOutput->secondaryBufferSize,
		&region1, &region1size,
		&region2, &region2size,
		0)))
	{
		uint8_t *destSample = (uint8_t *)region1;
		for (DWORD byteIndex = 0; byteIndex < region1size; ++byteIndex)
		{
			*destSample++ = 0;
		}

		destSample = (uint8_t *)region2;
		for (DWORD byteIndex = 0; byteIndex < region2size; ++byteIndex)
		{
			*destSample++ = 0;
		}

		gSecondaryBuffer->Unlock(region1, region1size, region2, region2size);
	}
}

static void
Win32FillSoundBuffer(Win32_Sound_Output *soundOutput, DWORD byteToLock, DWORD bytesToWrite,
	Game_Sound_Output_Buffer *soundBuffer)
{
	VOID *region1;
	DWORD region1size;
	VOID *region2;
	DWORD region2size;
	if (SUCCEEDED(gSecondaryBuffer->Lock(byteToLock, bytesToWrite,
		&region1, &region1size,
		&region2, &region2size,
		0)))
	{
		DWORD region1sampleCount = region1size / soundOutput->bytesPerSample;
		int16_t *destSample = (int16_t *)region1;
		int16_t *sourceSample = soundBuffer->samples;
		for (DWORD sampleIndex = 0; sampleIndex < region1sampleCount; ++sampleIndex)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			++soundOutput->runningSampleIndex;
		}

		DWORD region2sampleCount = region2size / soundOutput->bytesPerSample;
		destSample = (int16_t *)region2;
		for (DWORD sampleIndex = 0; sampleIndex < region2sampleCount; ++sampleIndex)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			++soundOutput->runningSampleIndex;
		}

		gSecondaryBuffer->Unlock(region1, region1size, region2, region2size);
	}
	else
	{
		//Logging
	}
}

static void
Win32ProcessXInputDigitalButton(DWORD xInputButtonState, DWORD buttonBit,
	Game_Input_Button *oldState, Game_Input_Button *newState)
{
	newState->endedDown = ((xInputButtonState & buttonBit) == buttonBit);
	newState->halfTransitions = (oldState->endedDown != newState->endedDown) ? 1 : 0;
}

int CALLBACK
WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	LARGE_INTEGER performanceCountFrequencyResult;
	QueryPerformanceFrequency(&performanceCountFrequencyResult);
	int64_t performanceCountFrequency = performanceCountFrequencyResult.QuadPart;

	Win32LoadXInput();

	WNDCLASSA windowClass = {};

	Wind32ResizeDIBSection(&gBackBuffer, 1280, 720);

	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = hInstance;
	//windowClass.hIcon;
	windowClass.lpszClassName = "RatEngineWindowClass";

	if (RegisterClassA(&windowClass))
	{
		HWND window = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"RatEngine",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);
		if (window)
		{
			HDC deviceContext = GetDC(window);
			Win32_Sound_Output soundOutput = {};

			soundOutput.samplesPerSecond = 48000;
			soundOutput.runningSampleIndex = 0;
			soundOutput.bytesPerSample = sizeof(int16_t) * 2;
			soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
			soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;
			Win32InitDSound(window, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
			Win32ClearSoundBuffer(&soundOutput);
			gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			gRunning = true;

#if !MAUSTRAP_RC
			LPVOID baseAddress = (LPVOID)Terabytes(2);
#else
			LPVOID baseAddress = 0;
#endif
			int16_t *samples = (int16_t *)VirtualAlloc(0, soundOutput.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			Game_Memory gameMemory = {};
			gameMemory.permanentStorageSize = Megabytes(64);
			gameMemory.transientStorageSize = Gigabytes(1);

			uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
			gameMemory.permanentStorage = VirtualAlloc(baseAddress, (size_t)totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			gameMemory.transientStorage = ((uint8_t *)gameMemory.permanentStorage + gameMemory.permanentStorageSize);

			if (samples && gameMemory.permanentStorage && gameMemory.transientStorage)
			{
				Game_Input input[2] = {};
				Game_Input *newInput = &input[0];
				Game_Input *oldInput = &input[1];

				LARGE_INTEGER lastCounter;
				QueryPerformanceCounter(&lastCounter);
				uint64_t lastCycleCount = __rdtsc();
				while (gRunning)
				{
					MSG message;

					while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
					{
						if (message.message == WM_QUIT)
						{
							gRunning = false;
						}
						TranslateMessage(&message);
						DispatchMessageA(&message);
					}

					DWORD maxControllerCount = XUSER_MAX_COUNT;
					if (maxControllerCount > ArrayCount(newInput->controllers))
					{
						maxControllerCount = ArrayCount(newInput->controllers);
					}

					for (DWORD controllerIndex = 0; controllerIndex < maxControllerCount; ++controllerIndex)
					{
						Game_Controller_Input *oldController = &oldInput->controllers[controllerIndex];
						Game_Controller_Input *newController = &newInput->controllers[controllerIndex];

						XINPUT_STATE controllerState;
						if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
						{
							//Controller is plugged in
							XINPUT_GAMEPAD *gamePad = &controllerState.Gamepad;

							bool32 up = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
							bool32 down = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
							bool32 left = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
							bool32 right = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

							newController->isAnalog = true;
							newController->stick.startX = oldController->stick.endX;
							newController->stick.startY = oldController->stick.endY;

							float x;
							if (gamePad->sThumbLX < 0)
							{
								x = (float)gamePad->sThumbLX / 32768.0f;
							}
							else
							{
								x = (float)gamePad->sThumbLX / 32767.0f;
							}
							newController->stick.minX = newController->stick.maxX = newController->stick.endX = x;

							float y;
							if (gamePad->sThumbLY < 0)
							{
								y = (float)gamePad->sThumbLY / 32768.0f;
							}
							else
							{
								y = (float)gamePad->sThumbLY / 32767.0f;
							}
							newController->stick.minY = newController->stick.maxY = newController->stick.endY = y;

							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_START,
								&oldController->buttonStart, &newController->buttonStart);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_BACK,
								&oldController->buttonBack, &newController->buttonBack);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_A,
								&oldController->buttonDown, &newController->buttonDown);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_B,
								&oldController->buttonRight, &newController->buttonRight);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_X,
								&oldController->buttonLeft, &newController->buttonLeft);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_Y,
								&oldController->buttonUp, &newController->buttonUp);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER,
								&oldController->buttonLshoulder, &newController->buttonLshoulder);
							Win32ProcessXInputDigitalButton(gamePad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER,
								&oldController->buttonRshoulder, &newController->buttonRshoulder);
						}
						else
						{
							//Controller is not available
						}
					}

					DWORD byteToLock = 0;
					DWORD targetCursor = 0;
					DWORD bytesToWrite = 0;
					DWORD playCursor = 0;
					DWORD writeCursor = 0;
					bool32 soundIsValid = false;
					if (SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
					{
						byteToLock = ((soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize);
						targetCursor = ((playCursor + (soundOutput.latencySampleCount * soundOutput.bytesPerSample)) % soundOutput.secondaryBufferSize);

						if (byteToLock > targetCursor)
						{
							bytesToWrite = (soundOutput.secondaryBufferSize - byteToLock);
							bytesToWrite += targetCursor;
						}
						else
						{
							bytesToWrite = targetCursor - byteToLock;
						}

						soundIsValid = true;
					}

					Game_Sound_Output_Buffer soundBuffer = {};
					soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
					soundBuffer.sampleCount = bytesToWrite / soundOutput.bytesPerSample;
					soundBuffer.samples = samples;

					Game_Offscreen_Buffer buffer = {};
					buffer.memory = gBackBuffer.memory;
					buffer.height = gBackBuffer.height;
					buffer.width = gBackBuffer.width;
					buffer.pitch = gBackBuffer.pitch;

					GameUpdateAndRender(&gameMemory, newInput, &buffer, &soundBuffer);

					//DirectSound output test
					if (soundIsValid)
					{
						Win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite, &soundBuffer);
					}

					Win32_Window_Dimensions dimensions = Win32GetWindowDimensions(window);
					Win32DisplayBufferInWindow(&gBackBuffer, deviceContext, dimensions.width, dimensions.height);

					uint64_t endCycleCount = __rdtsc();

					LARGE_INTEGER endCounter;
					QueryPerformanceCounter(&endCounter);

					uint64_t cyclesElapsed = endCycleCount - lastCycleCount;
					int64_t counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
					float msPerFrame = ((1000.0f * ((float)counterElapsed) / (float)performanceCountFrequency));
					float fps = ((float)performanceCountFrequency / (float)counterElapsed);
					float mcpf = ((float)cyclesElapsed / (1000.0f * 1000.0f));

					// char buffer[256];
					// sprintf(buffer, "%.02fms/f, %.02ffps, %.02fMc/f\n", msPerFrame, fps, mcpf);
					// OutputDebugStringA(buffer);

					lastCounter = endCounter;
					lastCycleCount = endCycleCount;

					Game_Input *temp = newInput;
					newInput = oldInput;
					oldInput = temp;
				}
			}
			else
			{
				// Log invalid memory
			}
		}
		else
		{
			//TODO: Logging
		}
	}
	else
	{
		//TODO: Logging
	}

	return (0);
}