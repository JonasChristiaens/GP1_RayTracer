//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	//Setting aspectRatio and FOV angle upfront so we can reuse this
	float aspectRatio{ float(m_Width) / m_Height };
	float AngleInRadians{ camera.fovAngle * (PI/180.f)};
	float FOV{ tan(AngleInRadians / 2) };
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//Setting ray we are casting from camera towards each pixel
			float rayXValue{ ((2.f * ((float(px) + 0.5f) / m_Width)) - 1.f) * aspectRatio * FOV };
			float rayYValue{ (1.f - (2.f * ( (float(py) + 0.5f) / m_Height))) * FOV };
			float rayZValue{ 1.f }; 

			Vector3 rayDirection{ rayXValue, rayYValue, rayZValue };
			rayDirection = cameraToWorld.TransformVector(rayDirection);
			Ray viewRay{ camera.origin, rayDirection.Normalized()};
			

			//Initial test to generate ray for each pixel of screen
			//ColorRGB finalColor{rayDirection.x, rayDirection.y, rayDirection.z};

			//Color to write to color buffer & hit verification
			ColorRGB finalColor{};
			HitRecord closestHit{};
			
			//Testing Sphere and Plane Hit Implementation
			/*Sphere testSphere{{0.f, 0.f, 100.f}, 50.f, 0};
			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);
			Plane testPlane{ {0.f, -50.f, 0.f}, {0.f, 1.f, 0.f}, 0 };
			GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);*/
			
			//Rendering full scene
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				//if hit, set final color to material color, else keep black
				//finalColor = materials[closestHit.materialIndex]->Shade();
				for (int lightIdx{}; lightIdx < lights.size(); ++lightIdx)
				{
					finalColor += LightUtils::GetRadiance(lights[lightIdx], closestHit.origin);

					Vector3 lightDirection{ LightUtils::GetDirectionToLight(lights[lightIdx], closestHit.origin) };
					float hitToLightMagnitude{ lightDirection.Magnitude() };
					Ray lightRay{ closestHit.origin + (closestHit.normal / 100.f), lightDirection.Normalized() };
					lightRay.max = hitToLightMagnitude;

					if (pScene->DoesHit(lightRay))
					{
						finalColor *= 0.5f;
					}
				}	

				//verify T values
				/*const float scaled_t = (closestHit.t - 50.f) / 40.f;
				const float scaled_t = closestHit.t / 500.f;
				finalColor = {scaled_t, scaled_t, scaled_t};*/
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		ToggleShadows();
	}

	if (pKeyboardState[SDL_SCANCODE_F3])
	{
		int lightningState{ int(m_CurrentLightingMode) };

		m_CurrentLightingMode = LightingMode((lightningState + 1) % 4);
	}
}
