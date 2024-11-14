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
			const Vector3 oc{ ray.origin - sphere.origin };
			const float B{ Vector3::Dot((2 * ray.direction), oc) };
			const float C{ (Vector3::Dot(oc, oc)) - (sphere.radius * sphere.radius) };
			const float discriminant{ Square(B) - (4 * C)};

			if (discriminant < 0) return false;

			const float sqrtDiscriminant{ sqrt(discriminant) };
			float t{ (-B - sqrtDiscriminant) / 2 >= ray.min ? (-B - sqrtDiscriminant) / 2 : (-B + sqrtDiscriminant) / 2 };

			if (t > ray.max || t < ray.min) return false;

			if (!ignoreHitRecord) 
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			}
			return true;
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
			const float denominator{ Vector3::Dot(ray.direction, plane.normal) };
			if (denominator > 0) return false;

			const float numerator{ Vector3::Dot((plane.origin - ray.origin), plane.normal) };
			const float t{ numerator / denominator };

			if (t > ray.max || t < ray.min) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = plane.normal;
			}
			return true;
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
			const float planeIntersection{ Vector3::Dot(triangle.normal, ray.direction) };

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

			if (AreEqual(planeIntersection, 0))return false;

			//plane logic for distance t, applied to triangles
			const float numerator{ Vector3::Dot((triangle.v0 - ray.origin), triangle.normal) };
			const float denominator{ Vector3::Dot(ray.direction, triangle.normal) };
			const float t{ numerator / denominator };

			if (t < ray.min or t > ray.max)return false;

			//calculate point of intersection
			Vector3 intersectionPoint{ ray.origin + (ray.direction * t) };
			
			//Add all points to a vector
			Vector3 triangleVerteces[3]{ triangle.v0, triangle.v1, triangle.v2 };

			for (int vertexIdx{}; vertexIdx < 3; ++vertexIdx)
			{
				Vector3 e{ triangleVerteces[(vertexIdx + 1) % 3] - triangleVerteces[vertexIdx] };
				Vector3 p{ intersectionPoint - triangleVerteces[vertexIdx] };

				if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0.0f) return false;
			}

			//fill in hitrecord
			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.normal = triangle.normal;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + (ray.direction * t);

			}
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangleMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			// X
			float tx1 = ( mesh.transformedMinAABB.x - ray.origin.x ) / ray.direction.x;
			float tx2 = ( mesh.transformedMaxAABB.x - ray.origin.x ) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			// Y
			float ty1 = ( mesh.transformedMinAABB.y - ray.origin.y ) / ray.direction.y;
			float ty2 = ( mesh.transformedMaxAABB.y - ray.origin.y ) / ray.direction.y;

			tmin = std::max( tmin, std::min(ty1, ty2) );
			tmax = std::min( tmax, std::max(ty1, ty2) );

			// Z
			float tz1 = ( mesh.transformedMinAABB.z - ray.origin.z ) / ray.direction.z;
			float tz2 = ( mesh.transformedMaxAABB.z - ray.origin.z ) / ray.direction.z;
			
			tmin = std::max( tmin, std::min(tz1, tz2) );
			tmax = std::min( tmax, std::max(tz1, tz2) );

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//done in week 5
			//slab test
			if (!SlabTest_TriangleMesh(mesh, ray)) return false;

			bool returnState{};
			HitRecord closestHit{};

			for (int indicesIdx{}; indicesIdx < mesh.indices.size(); indicesIdx += 3)
			{
				//adding all verteces individually
				const Vector3 v0{ mesh.transformedPositions[mesh.indices[indicesIdx]] };
				const Vector3 v1{ mesh.transformedPositions[mesh.indices[indicesIdx + 1]] };
				const Vector3 v2{ mesh.transformedPositions[mesh.indices[indicesIdx + 2]] };

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
			if (light.type == LightType::Point )
			{
				Vector3 shadowRay{ light.origin - origin };
				return shadowRay;
			}

			if (light.type == LightType::Directional)
			{
				return light.direction;
			}
			
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

			if (light.type == LightType::Directional)
			{
				return { light.color * light.intensity };
			}

			return{};
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