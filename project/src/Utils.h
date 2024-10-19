#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//done week 1
			Vector3 oc{ ray.origin - sphere.origin };
			float A{ Vector3::Dot(ray.direction, ray.direction) };
			float B{ Vector3::Dot((2 * ray.direction), oc) };
			float C{ (Vector3::Dot(oc, oc)) - (sphere.radius * sphere.radius) };
			float discriminant{ (B * B) - (4 * A * C) };

			if (discriminant > 0)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;

				float sqrtDiscriminant{ sqrt(discriminant) };
				float denominator{ 2 * A };
				
				float t_0{ (-B - sqrtDiscriminant) / denominator };
				float t_1{ (-B + sqrtDiscriminant) / denominator };
				//possible use to improve fps
				//float t{ (-B - sqrtDiscriminant) / denominator >= ray.min ? (-B - sqrtDiscriminant) / denominator >= ray.min : (-B + sqrtDiscriminant) / denominator >= ray.min };


				if (t_0 >= ray.min && t_0 < ray.max)
				{
					hitRecord.t = t_0;
					hitRecord.origin = ray.origin + (t_0 * ray.direction);
					Vector3 pointDirection{ hitRecord.origin - sphere.origin };
					hitRecord.normal = pointDirection.Normalized();

					return true;
				}
				else if(t_1 >= ray.min && t_1 < ray.max)
				{
					hitRecord.t = t_1;
					hitRecord.origin = ray.origin + (t_1 * ray.direction);
					Vector3 pointDirection{ hitRecord.origin - sphere.origin };
					hitRecord.normal = pointDirection.Normalized();

					return true;
				}
			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//done in week 1
			float numerator{ Vector3::Dot((plane.origin - ray.origin), plane.normal) };
			float denominator{ Vector3::Dot(ray.direction, plane.normal) };
			float t{ numerator / denominator };

			if (t >= ray.min and t < ray.max)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = plane.normal;

				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//done in week 5
			//calculating ray with plane interesection
			float planeIntersection{ Vector3::Dot(triangle.normal, ray.direction) };

			//backface culling and shadows
			if (!ignoreHitRecord)
			{
				//back-face or front-face
				if (planeIntersection > 0 && triangle.cullMode == TriangleCullMode::BackFaceCulling
					or planeIntersection < 0 && triangle.cullMode == TriangleCullMode::FrontFaceCulling) return false;
			}
			else {
				//back-face shadows or front-face shadows
				if (planeIntersection < 0 && triangle.cullMode == TriangleCullMode::BackFaceCulling
					or planeIntersection > 0 && triangle.cullMode == TriangleCullMode::FrontFaceCulling)	return false;
			}

			if (AreEqual(planeIntersection, 0))
			{
				return false;
			}

			//plane logic for distance t, applied to triangles
			float numerator{ Vector3::Dot((triangle.v0 - ray.origin), triangle.normal) };
			float denominator{ Vector3::Dot(ray.direction, triangle.normal) };
			float t{ numerator / denominator };

			if (t < ray.min or t > ray.max)
			{
				return false;
			}

			//calculate point of intersection
			Vector3 intersectionPoint{ ray.origin + (ray.direction * t) };
			
			//Add all points to a vector
			Vector3 triangleVerteces[3]{ triangle.v0, triangle.v1, triangle.v2 };

			for (int vertexIdx{}; vertexIdx < 3; ++vertexIdx)
			{
				Vector3 e{ triangleVerteces[(vertexIdx + 1) % 3] - triangleVerteces[vertexIdx] };
				Vector3 p{ intersectionPoint - triangleVerteces[vertexIdx] };

				float checker{ Vector3::Dot(Vector3::Cross(e, p), triangle.normal) };
				if (checker < 0)
				{
					return false;
				}
			}

			//fill in hitrecord
			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.normal = triangle.normal;
				hitRecord.origin = ray.origin + (ray.direction * t);
				hitRecord.t = t;
			}
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//done in week 5
			bool returnState{};
			HitRecord closestHit{};

			for (int indicesIdx{}; indicesIdx < mesh.indices.size(); indicesIdx += 3)
			{
				//adding all verteces individually
				Vector3 v0{ mesh.transformedPositions[mesh.indices[indicesIdx]] };
				Vector3 v1{ mesh.transformedPositions[mesh.indices[indicesIdx + 1]] };
				Vector3 v2{ mesh.transformedPositions[mesh.indices[indicesIdx + 2]] };

				//making new triangle with verteces and setting cullmode and materialindex of new triangle to mesh values
				Triangle triangle{ v0, v1, v2, mesh.transformedNormals[indicesIdx / 3] };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(triangle, ray, closestHit, ignoreHitRecord)) returnState = true;
				if (closestHit.t < hitRecord.t and closestHit.didHit) hitRecord = closestHit;
			}
			return returnState;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//done in week 2
			Vector3 shadowRay{ light.origin - origin };
			return shadowRay;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//done in week 3
			if (light.type == LightType::Point)
			{
				float irradiance{ light.intensity / ((light.origin - target).Magnitude() * (light.origin - target).Magnitude()) };
				ColorRGB lightColor{ light.color * irradiance };

				return lightColor;
			}
			else if (light.type == LightType::Directional)
			{

			}
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}