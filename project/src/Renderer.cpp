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

			//Color to write to color buffer & hit verification
			ColorRGB finalColor{};
			HitRecord closestHit{};
			
			//Rendering full scene
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				//finalColor should be initialized black
				finalColor = {};

				for (int lightIdx{}; lightIdx < lights.size(); ++lightIdx)
				{
					Vector3 invLightDirection{ LightUtils::GetDirectionToLight(lights[lightIdx], closestHit.origin) };
					float distanceToLight{ invLightDirection.Normalize() };

					Ray lightRay{ closestHit.origin + (invLightDirection * 0.01f), invLightDirection };
					lightRay.max = distanceToLight;

					float observedAreaMeasure{ Vector3::Dot(closestHit.normal, invLightDirection) };
					if (observedAreaMeasure <= 0)
					{
						continue;
					}

					if (pScene->DoesHit(lightRay) && m_ShadowsEnabled)
					{
						continue;
					}
					
					switch (m_CurrentLightMode)
					{
					case dae::Renderer::LightMode::ObservedArea:
						//Dot(normal, lightdirection)
						finalColor += observedAreaMeasure * colors::White;
						break;
					case dae::Renderer::LightMode::Radiance:
						//Ergb
						finalColor += LightUtils::GetRadiance(lights[lightIdx], closestHit.origin);
						break;
					case dae::Renderer::LightMode::BRDF:
						//BRDFrgb
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, invLightDirection, -viewRay.direction);
						break;
					case dae::Renderer::LightMode::Combined:
						//Ergb * BRDFrgb * Dot(normal, lightdirection)
						finalColor += LightUtils::GetRadiance(lights[lightIdx], closestHit.origin)
							* materials[closestHit.materialIndex]->Shade(closestHit, invLightDirection, -viewRay.direction) * observedAreaMeasure;
						break;
					default:
						break;
					}
					
				}	
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

void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

void dae::Renderer::CycleLightingMode()
{
	int lightState{ int(m_CurrentLightMode) };
	m_CurrentLightMode = LightMode((lightState + 1) % 4);
}