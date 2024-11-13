//External includes
#ifdef ENABLE_VLD
#include "vld.h"
#endif
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

using namespace dae;

enum class WeeklyScenes
{
	SphereScene,
	BunnyScene,
	count
};

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintSettings()
{
	std::cout << "\tCamera controls" << std::endl;
	std::cout << "===========================\n" << std::endl;
	std::cout << "Rclick : rotate camera" << std::endl;
	std::cout << "Lclick : move along x & z axis" << std::endl;
	std::cout << "L&Rclick : move along y axis" << std::endl;
	std::cout << "WASD : Move camera\n" << std::endl;
	std::cout << "\tSettings" << std::endl;
	std::cout << "===========================\n" << std::endl;
	std::cout << "X : Take a screenshot" << std::endl;
	std::cout << "F2 : Toggle Shadows" << std::endl;
	std::cout << "F3 : Cycle Lighting Mode" << std::endl;
	std::cout << "F4 : Cycle between Scenes\n" << std::endl;
}

int main(int argc, char* args[])
{
	PrintSettings();

	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - **Jonas Christiaens 2GD11**",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	const auto pBunnyScene = new Scene_Bunny();
	pBunnyScene->Initialize();
	const auto pSphereScene = new Scene_W4();
	pSphereScene->Initialize();
	
	WeeklyScenes currentScene{ WeeklyScenes::SphereScene };
	//Start loop
	pTimer->Start();

	// Start Benchmark
	// pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleShadows();

				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
					pRenderer->CycleLightingMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					int current{ int(currentScene) };
					currentScene = WeeklyScenes((current + 1) % int(WeeklyScenes::count));
				}
				break;
			}
		}

		switch (currentScene)
		{
		case WeeklyScenes::SphereScene:
			//--------- Update ---------
			pSphereScene->Update(pTimer);

			//--------- Render ---------
			pRenderer->Render(pSphereScene);
			break;
		case WeeklyScenes::BunnyScene:
			//--------- Update ---------
			pBunnyScene->Update(pTimer);

			//--------- Render ---------
			pRenderer->Render(pBunnyScene);
			break;
		default:
			break;
		}

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pBunnyScene;
	delete pSphereScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}