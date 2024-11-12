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

#include <execution>
#define PARALLEL_EXECUTION


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
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	const float fovAngle = camera.fovAngle * TO_RADIANS;
	const float fov = tan( fovAngle / 2.f );

#if defined(PARALLEL_EXECUTION)
	// Parallel logic
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height)};
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve( amountOfPixels );
	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
		});

#else
	// Synchronous logic (no threading)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };

	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, pixelIndex, fov, aspectRatio,cameraToWorld, camera.origin);
	}

#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix &cameraToWorld, const Vector3 &cameraOrigin) const
{
	auto& materials{ pScene->GetMaterials() };
	auto& lights{ pScene->GetLights() };
	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	float rx{ px + 0.5f }, ry{ py + 0.5f };
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

	Vector3 rayDirection{ cx, cy, 1.0f };
	rayDirection = cameraToWorld.TransformVector(rayDirection);
	Ray viewRay{ cameraOrigin, rayDirection.Normalized() };

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
			if (observedAreaMeasure <= 0.f) continue;
			if (pScene->DoesHit(lightRay) && m_ShadowsEnabled) continue;

			switch (m_CurrentLightMode)
			{
			case dae::Renderer::LightMode::ObservedArea:
				//Dot(normal, lightdirection)
				finalColor += observedAreaMeasure * ColorRGB{ 1.f, 1.f, 1.f };
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
				finalColor += LightUtils::GetRadiance(lights[lightIdx], closestHit.origin) *
					materials[closestHit.materialIndex]->Shade(closestHit, invLightDirection, -viewRay.direction) * observedAreaMeasure;
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