#pragma once
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//done in week 3
			ColorRGB perfectDiffuseReflectance{ cd * kd };
			ColorRGB lambert{ perfectDiffuseReflectance / PI };
			return lambert;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//done in week 3
			ColorRGB perfectDiffuseReflectance{ cd * kd };
			ColorRGB lambert{ perfectDiffuseReflectance / PI };
			return lambert;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//done in week 3
			Vector3 reflect{ l - ((2 * Vector3::Dot(n, l)) * n)};
			float cosAngle{ std::max(Vector3::Dot(reflect, v), 0.0f) };
			float phongSpecularReflection{ ks * pow(cosAngle, exp) };
			return { phongSpecularReflection * colors::White };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//done in week 3
			float dotHv{ Vector3::Dot(h, v) };
			return f0 + ( (ColorRGB{ 1.0f, 1.0f, 1.0f } - f0) * std::powf((1.0f - dotHv), 5) );
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			float a{ roughness * roughness };
			float dotNh{ Vector3::Dot(n, h) };
			
			float something{ Square(dotNh) * (Square(a) - 1.0f) + 1.0f };
			float denominator{ PI * Square(something)};

			float result{ Square(a) / denominator };
			return result;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//done in week 3
			float a{ roughness * roughness };
			float k{ Square(a + 1.0f) / 8.0f };

			float numerator{ Vector3::Dot(n, v) };
			float denominator{ (numerator * (1.0f - k)) + k };

			float result{ numerator / denominator };
			return result;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//done in week 3
			float masking{ GeometryFunction_SchlickGGX(n, v, roughness) };
			float shadowing{ GeometryFunction_SchlickGGX(n, l, roughness) };

			float approximation{ masking * shadowing };
			return approximation;
		}

	}
}