#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"
#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		
		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//done in week 2
			Vector3 rightLocal{ Vector3::Cross(Vector3::UnitY, forward).Normalized() };
			Vector3 upLocal{ Vector3::Cross(forward, Vector3::UnitX).Normalized() };
			return Matrix{ {rightLocal, 0}, {upLocal, 0}, {forward, 0}, {origin, 1} };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float DegToRadCalculation{ (PI / 180.f) };
			Matrix translationMatrix{ {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {origin, 1} };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//done in week 2
			//Transforming camera's origin ( Movement )
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				translationMatrix *= Matrix::CreateTranslation(Vector3{ 0, 0, 1 });
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				translationMatrix *= Matrix::CreateTranslation(Vector3{ -1, 0, 0 });
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				translationMatrix *= Matrix::CreateTranslation(Vector3{ 0, 0, -1 });
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				translationMatrix *= Matrix::CreateTranslation(Vector3{ 1, 0, 0 });
			}
			
			//Transforming camera's forward vector ( Rotation )
			if (mouseState == 4)
			{
				totalPitch += atan(mouseY / 20.f);
				totalYaw += atan(mouseX / 20.f);
			}
			float totalPitchInRad{ totalPitch * DegToRadCalculation };
			float totalYawInRad{ totalYaw * DegToRadCalculation };

			Matrix rotationMatrix{ Matrix::CreateRotationX(totalPitchInRad) * Matrix::CreateRotationY(totalYawInRad) };
			
			rotationMatrix *= translationMatrix;

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			origin = Vector3::Project({ Vector3{translationMatrix[3].x, translationMatrix[3].y, translationMatrix[3].z} }, forward);
			CalculateCameraToWorld();
		}
	};
}
