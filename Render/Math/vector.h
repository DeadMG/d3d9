#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

namespace Wide {
    namespace Math {
        typedef glm::vec3 Vector;
        typedef glm::quat Quaternion;
        typedef glm::mat4x4 Matrix;
        
		inline Vector Normalize(const Vector& lhs) {
			return glm::normalize(lhs);
		}
        inline Quaternion Normalize(const Quaternion& lhs) {
            return glm::normalize(lhs);
        }
        inline Quaternion RotateX(float x) {
            return glm::angleAxis(x, Vector(1, 0, 0));
        }
        inline Quaternion RotateY(float x) {
            return glm::angleAxis(x, Vector(0, 1, 0));
        }
        inline Quaternion RotateZ(float x) {
            return glm::angleAxis(x, Vector(0, 0, 1));
        }
        inline Quaternion RotateAxis(float x, Vector axis) {
            return glm::angleAxis(x, Normalize(axis));
        }
        inline Vector operator*(const Vector& lhs, const Quaternion& rhs) {
            return glm::rotate(rhs, lhs);
        }
		inline Vector Cross(const Vector& lhs, const Vector& rhs) {
			return -glm::cross(lhs, rhs); // Minus because we are in an LH co-ordinate system and the default cross is RH.
		}
	    inline float Dot(const Vector& lhs, const Vector& rhs) {
            return glm::dot(lhs, rhs);
        }

        // GLM's length function returns an integer! for some reason
        // So provide own length function which returns a float
        inline float Length(const Vector& v) {
            return std::sqrt(v.x*v.x + v.y * v.y + v.z * v.z);
        }
    }
}