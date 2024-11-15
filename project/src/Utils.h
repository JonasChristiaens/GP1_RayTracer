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
			const Vector3 L{ ray.origin - sphere.origin };
			const float B{ Vector3::Dot((ray.direction), L) };
			const float C{ (Vector3::Dot(L, L)) - (sphere.radius * sphere.radius) };

			//calculate discriminant
			const float discriminant{ Square(B) - C };

			if (discriminant < 0) return false;

			const float sqrtDiscriminant{ sqrtf(discriminant) };
			float t{ (-B - sqrtDiscriminant) >= ray.min ? (-B - sqrtDiscriminant) : (-B + sqrtDiscriminant) };

			if (t > ray.max || t < ray.min) return false;

			//only calculate hit record if needed
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

			//ray is parallel to plane
			if (denominator > 0) return false;

			//calculate intersection distance
			const float numerator{ Vector3::Dot((plane.origin - ray.origin), plane.normal) };
			const float t{ numerator / denominator };

			if (t > ray.max || t < ray.min) return false;

			//only calculate hit record if needed
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
			//cache frequently used values
			const Vector3& dir = ray.direction;
			const float planeIntersection = Vector3::Dot(triangle.normal, dir);

			//early culling check
			if (!ignoreHitRecord)
			{
				if ((planeIntersection > 0 && triangle.cullMode == TriangleCullMode::BackFaceCulling) ||
					(planeIntersection < 0 && triangle.cullMode == TriangleCullMode::FrontFaceCulling))
					return false;
			}
			else
			{
				if ((planeIntersection < 0 && triangle.cullMode == TriangleCullMode::BackFaceCulling) ||
					(planeIntersection > 0 && triangle.cullMode == TriangleCullMode::FrontFaceCulling))
					return false;
			}

			if (AreEqual(planeIntersection, 0)) return false;

			//calculate intersection distance
			const float t = Vector3::Dot((triangle.v0 - ray.origin), triangle.normal) / planeIntersection;

			//early distance check
			if (t < ray.min || t > ray.max) return false;

			//calculate intersection point once and reuse
			const Vector3 intersectionPoint = ray.origin + (dir * t);

			//create e and p
			const Vector3 e0 = triangle.v1 - triangle.v0;
			const Vector3 e1 = triangle.v2 - triangle.v1;
			const Vector3 e2 = triangle.v0 - triangle.v2;

			const Vector3 p0 = intersectionPoint - triangle.v0;
			const Vector3 p1 = intersectionPoint - triangle.v1;
			const Vector3 p2 = intersectionPoint - triangle.v2;

			//perform edge tests
			if (Vector3::Dot(Vector3::Cross(e0, p0), triangle.normal) < 0.0f ||
				Vector3::Dot(Vector3::Cross(e1, p1), triangle.normal) < 0.0f ||
				Vector3::Dot(Vector3::Cross(e2, p2), triangle.normal) < 0.0f)
				return false;

			//only update hit record if needed
			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.normal = triangle.normal;
				hitRecord.materialIndex = triangle.materialIndex;

				//reuse already calculated point
				hitRecord.origin = intersectionPoint;  
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
		//direction from target to light
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