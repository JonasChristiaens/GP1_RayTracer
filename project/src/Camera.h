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
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			return Matrix{ {right, 0}, {up, 0}, {forward, 0}, {origin, 1} };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float movementSpeed{ 20.f };
			const float rotationSpeed{ 20.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//done in week 2
			//Transforming camera's origin ( Movement )
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed * deltaTime;
			}
			
			//Transforming camera's forward vector ( Rotation )
			if (mouseState == 4)
			{
				totalPitch += mouseY * rotationSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if(mouseState == 1)
			{
				origin -= forward * mouseY * movementSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState == 5)
			{
				origin.y -= mouseY * movementSpeed * deltaTime;
			}
			Matrix rotationMatrix{ Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS) };

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			CalculateCameraToWorld();
		}
	};
}
