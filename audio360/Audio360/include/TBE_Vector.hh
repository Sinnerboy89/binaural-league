#ifndef FBA_TBE_VECTOR_H
#define FBA_TBE_VECTOR_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <stdio.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <string>

namespace TBE {
#ifndef TBE_SMALL_NUMBER
#define TBE_SMALL_NUMBER 1.e-8f
#endif

const float M_PIF = 3.14159265358979323846f;

struct Aed {
  float azimuth;
  float elevation;
  float distance;

  Aed() : azimuth(0.f), elevation(0.f), distance(0.f) {}

  Aed(float azi, float ele, float dist) : azimuth(azi), elevation(ele), distance(dist) {}

  inline void set(float azi, float ele, float dist) {
    azimuth = azi;
    elevation = ele;
    distance = dist;
  }

  inline bool operator==(const Aed& other) const {
    return azimuth == other.azimuth && elevation == other.elevation && distance == other.distance;
  }

  inline bool operator!=(const Aed& other) const {
    return azimuth != other.azimuth || elevation != other.elevation || distance != other.distance;
  }
};

/// Vector class with overloaded operators and helper methods.
/// Unless specified, +x is right. +y is up. +z is into the screen.
class TBVector {
 public:
  float x{0.f};
  float y{0.f};
  float z{0.f};

  /// Default constructor
  inline TBVector();

  /// Overloaded constructor, specify X, Y, Z values
  /// @param xValue x axis value
  /// @param yValue y axis value
  /// @param zValue z axis value
  explicit inline TBVector(float xValue, float yValue, float zValue);

  /// Overloaded constructor. X, Y, Z will be assigned the same value
  /// @param value The value assigned to all three axis
  explicit inline TBVector(float value);

  /// Returns the cross product of two vectors
  /// @param vectorA Vector a as TBVector
  /// @param vectorB Vector b as TBVector
  /// @return Cross product of a and b as TBVector
  inline static TBVector CrossProduct(const TBVector& vectorA, const TBVector& vectorB);

  /// @return A vector initialised to 0 on all axis.
  inline static TBVector zero();

  inline static TBVector forward() {
    return TBVector(0.f, 0.f, 1.f);
  }

  inline static TBVector up() {
    return TBVector(0.f, 1.f, 0.f);
  }

  /// Returns the dot product of two vectors
  /// @param vectorA Vector a as TBVector
  /// @param vectorB Vector b as TBVector
  /// @return Dot product of a and b as float
  inline static float DotProduct(const TBVector& vectorA, const TBVector& vectorB);

  /// Returns the angle between two vectors
  inline static float Angle(const TBVector& vectorA, const TBVector& vectorB);

  /// Returns the magnitude of this vector
  /// @return Magnitude as float
  inline static float magnitude(const TBVector& vector);

  /// Clamps the magnitude of the vector
  /// @param vector Input/output vector
  /// @param maxValue The maximum allowed magnitude
  inline static void clampMagnitude(TBVector& vector, float maxValue);

  /// Returns the squared magnitude of this vector. Useful for comparing magnitudes without using
  /// expensive sqrt function.
  /// @return Squared magnitude as float
  inline static float magSquared(const TBVector& vector);

  /// Normalise the vector in place
  /// @param vector The vector to be normalised in place
  inline static void normalise(TBVector& vector);

  /// Overloaded operator to add two vectors
  inline TBVector operator+(const TBVector& vector) const;

  /// Overloaded operator to subtract two vectors
  inline TBVector operator-(const TBVector& vector) const;

  /// Overloaded operator to multiply two vectors
  inline TBVector operator*(const TBVector& vector) const;

  /// Overloaded operator to divide two vectors
  inline TBVector operator/(const TBVector& vector) const;

  /// Overloaded operator to add a vector with a float value
  inline TBVector operator+(float value) const;

  /// Overloaded operator to subtract a vector with a float value
  inline TBVector operator-(float value) const;

  /// Overloaded operator to multiply a vector with a float value
  inline TBVector operator*(float value) const;

  /// Overloaded operator to divide a vector with a float value
  inline TBVector operator/(float value) const;

  /// Overloaded operator to check if two vectors are equal
  inline bool operator==(const TBVector& vector) const;

  /// Overloaded operator to check if two vectors are not equal
  inline bool operator!=(const TBVector& vector) const;

  /// Access the reference to x,y,z values of the vector like an array.
  /// @param index vector[0] is x, vector[1] is y, vector[2] is z
  /// @return Returns the address of the float value
  inline float& operator[](int index);

  /// Access the x,y,z values of the vector like an array. vector[0] is x, vector[1] is y, vector[2]
  /// is z
  /// @param index vector[0] is x, vector[1] is y, vector[2] is z
  /// @return Returns the float value
  inline float operator[](int index) const;

  /// Set the x,y,z values through a method
  /// @param xValue x value
  /// @param yValue y value
  /// @param zValue z value
  inline void set(float xValue, float yValue, float zValue);

  /// Converts contents of this vector to abs
  inline void abs();

  /// Return a new version of the vector with absolute values
  inline static TBVector abs(const TBVector& inputVector);

  /// Return the the projection of the input vector onto a plane. Projection is always orthogonal to
  /// plane surface.
  /// @param inputVector The vector to get projected
  /// @param unitNormalToPlane Unit vector normal to plane
  /// @return The vector projection onto the plane
  inline static TBVector projectOntoPlane(
      const TBVector inputVector,
      const TBVector unitNormalToPlane);

  /// rotate a vector point by a forward and up vector (matrix multiplication).
  inline static void rotateByVectors(
      const TBVector& forwardVector,
      const TBVector& upVector,
      TBVector& rotatedVectorOut);

  inline float min_val();

  inline float max_val();

  inline std::string toString(int precision = 6) const;

  /// Get the 3x3 rotation matrix represented by a forward and up vector.
  /// @param forwardVector The rotation's forward vector. Must be orthogonal to the up vector!
  /// @param upVector The rotation's up vector. Must be orthogonal to the forward vector!
  /// @param matrixOut The output matrix. Needs to be initialised elsewhere as an array of floats at
  /// least 9 long.
  inline static void
  getMatrixFromVectors(TBVector forwardVector, TBVector upVector, float* matrixOut);

  /// Get the 3x3 rotation matrix which represents a rotation from one vector to another.
  /// @param fromPoint The vector to start from.
  /// @param toPoint The vector to rotate to.
  /// @param matrixOut The output matrix. Needs to be initialised elsewhere as an array of floats at
  /// least 9 long.
  inline static void
  getFromToRotationMatrix(TBVector fromPoint, TBVector toPoint, float* matrixOut);

  /// Get the azimuth/elevation/distance of a vector (conversion from Cartesian to gaming type
  /// spherical coordinates)
  /// @param vector The input vector.
  /// @return The azimuth/elevation/distance of the input vector.
  inline static Aed getAedFromVector(TBVector vector);

  /// Get the Cartesian unit vector defined by the azimuth and elevation
  /// @param azimuth The input azimuth. +ve values => right.
  /// @param elevation The input elevation. +ve values => up.
  /// @return The azimuth/elevation/distance of the input vector.
  inline static TBVector getVectorFromAziEle(float azimuth, float elevation);

  /// Get the Cartesian unit vector defined by the azimuth, elevation and distance
  /// @param azimuth The input azimuth. +ve values => right.
  /// @param elevation The input elevation. +ve values => up.
  /// @param distance The input distance. +ve values only, in meters.
  /// @return The azimuth/elevation/distance of the input vector.
  inline static TBVector getVectorFromAziEleDist(float azimuth, float elevation, float dist);

  /// Get the Cartesian unit vector (forward vector) defined by the input Euler angles.
  /// @param eulerAngles The input Euler angles, in left-handed system. In degrees!
  /// @return The azimuth/elevation/distance of the input vector.
  inline static TBVector getVectorFromEuler(TBVector eulerAngles);

  /// Get the 3x3 rotation matrix represented by a set of Euler angles.
  /// @param eulerAngles The rotation's Euler angles in degrees!
  /// @param matrixOut The output matrix. Needs to be initialised elsewhere as an array of floats at
  /// least 9 long.
  inline static void getMatrixFromEuler(TBVector eulerAngles, float* matrixOut);
};

inline TBVector::TBVector() {}

inline TBVector::TBVector(float xValue, float yValue, float zValue) {
  x = xValue;
  y = yValue;
  z = zValue;
}

inline TBVector::TBVector(float value) : x(value), y(value), z(value) {}

inline float TBVector::min_val() {
  // find the lowest value in the vector:
  float min = x;

  if (y < x && y < z) {
    min = y;
  }

  if (z < x && z < y) {
    min = z;
  }

  return min;
}

inline float TBVector::max_val() {
  // find the lowest value in the vector:
  float max = x;

  if (y > x && y > z) {
    max = y;
  }

  if (z > x && z > y) {
    max = z;
  }

  return max;
}

inline std::string TBVector::toString(int precision) const {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << "(" << x << ", " << y << ", " << z << ")";
  return stream.str();
}

inline TBVector TBVector::CrossProduct(const TBVector& vectorA, const TBVector& vectorB) {
  return TBVector(
      vectorA.y * vectorB.z - vectorA.z * vectorB.y,
      vectorA.z * vectorB.x - vectorA.x * vectorB.z,
      vectorA.x * vectorB.y - vectorA.y * vectorB.x);
}

inline float TBVector::DotProduct(const TBVector& vectorA, const TBVector& vectorB) {
  return vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z;
}

inline TBVector TBVector::zero() {
  return TBVector(0.f, 0.f, 0.f);
}

inline float TBVector::Angle(const TBVector& vectorA, const TBVector& vectorB) {
  const float denom = static_cast<float>(magnitude(vectorA) * magnitude(vectorB));
  if (denom < TBE_SMALL_NUMBER) {
    return 0.0f;
  }
  const float normDot = TBVector::DotProduct(vectorA, vectorB) / denom;
  return static_cast<float>(acos(normDot));
}

inline float TBVector::magnitude(const TBVector& vector) {
  return (std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z));
}

inline void TBVector::clampMagnitude(TBVector& vector, float maxValue) {
  float mag = magnitude(vector);
  if (mag > maxValue) {
    float scale_down = maxValue / mag;
    vector.x = vector.x * scale_down;
    vector.y = vector.y * scale_down;
    vector.z = vector.z * scale_down;
  }
}

inline float TBVector::magSquared(const TBVector& vectorA) {
  return (vectorA.x * vectorA.x + vectorA.y * vectorA.y + vectorA.z * vectorA.z);
}

inline void TBVector::normalise(TBVector& vectorA) {
  float mag = static_cast<float>(magnitude(vectorA));
  if (mag < TBE_SMALL_NUMBER) {
    vectorA.x = 0.f;
    vectorA.y = 0.f;
    vectorA.z = 0.f;
  } else {
    float magInv = 1.f / mag;
    vectorA.x *= magInv;
    vectorA.y *= magInv;
    vectorA.z *= magInv;
  }
}

inline TBVector TBVector::operator+(const TBVector& vector) const {
  return TBVector(x + vector.x, y + vector.y, z + vector.z);
}

inline TBVector TBVector::operator-(const TBVector& vector) const {
  return TBVector(x - vector.x, y - vector.y, z - vector.z);
}

inline TBVector TBVector::operator*(const TBVector& vector) const {
  return TBVector(x * vector.x, y * vector.y, z * vector.z);
}

inline TBVector TBVector::operator/(const TBVector& vector) const {
  return TBVector(x / vector.x, y / vector.y, z / vector.z);
}

inline TBVector TBVector::operator+(float value) const {
  return TBVector(x + value, y + value, z + value);
}

inline TBVector TBVector::operator-(float value) const {
  return TBVector(x - value, y - value, z - value);
}

inline TBVector TBVector::operator*(float value) const {
  return TBVector(x * value, y * value, z * value);
}

inline TBVector TBVector::operator/(float value) const {
  const float divide = 1.f / value;
  return TBVector(x * divide, y * divide, z * divide);
}

inline float& TBVector::operator[](int index) {
  if (index == 0) {
    return x;
  } else if (index == 1) {
    return y;
  } else {
    return z;
  }
}

inline float TBVector::operator[](int index) const {
  if (index == 0) {
    return x;
  } else if (index == 1) {
    return y;
  } else {
    return z;
  }
}

inline bool TBVector::operator==(const TBVector& vector) const {
  return x == vector.x && y == vector.y && z == vector.z;
}

inline bool TBVector::operator!=(const TBVector& vector) const {
  return x != vector.x || y != vector.y || z != vector.z;
}

inline void TBVector::set(float xValue, float yValue, float zValue) {
  x = xValue;
  y = yValue;
  z = zValue;
}

inline void TBVector::abs() {
  x = std::abs(x);
  y = std::abs(y);
  z = std::abs(z);
}

inline TBVector TBVector::abs(const TBVector& inputVector) {
  return TBVector(std::abs(inputVector.x), std::abs(inputVector.y), std::abs(inputVector.z));
}

inline TBVector TBVector::projectOntoPlane(
    const TBVector inputVector,
    const TBVector unitNormalToPlane) {
  // assert if not unit vector
  assert(std::abs(TBVector::magnitude(unitNormalToPlane) - 1.f) < 0.001f);

  TBVector proj = TBVector::CrossProduct(unitNormalToPlane, inputVector);
  return TBVector::CrossProduct(proj, unitNormalToPlane);
}

inline void TBVector::rotateByVectors(
    const TBVector& forwardVector,
    const TBVector& upVector,
    TBVector& rotatedVectorOut) {
  float rotMatrix[3][3] = {{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
  TBVector N1(forwardVector.x, forwardVector.y, forwardVector.z),
      V1(upVector.x, upVector.y, upVector.z);
  TBVector U1 = TBVector::CrossProduct(V1, N1);
  TBVector::normalise(U1);

  TBVector lPos_temp = rotatedVectorOut;

  // converting vectors to a Rotation Matrix
  rotMatrix[0][0] = U1[0];
  rotMatrix[1][0] = U1[1];
  rotMatrix[2][0] = U1[2];
  rotMatrix[0][1] = V1[0];
  rotMatrix[1][1] = V1[1];
  rotMatrix[2][1] = V1[2];
  rotMatrix[0][2] = N1[0];
  rotMatrix[1][2] = N1[1];
  rotMatrix[2][2] = N1[2];

  // multiply source and listener position with the 'compensating Rotation' matrix

  rotatedVectorOut.x =
      rotMatrix[0][0] * lPos_temp.x + rotMatrix[0][1] * lPos_temp.y + rotMatrix[0][2] * lPos_temp.z;
  rotatedVectorOut.y =
      rotMatrix[1][0] * lPos_temp.x + rotMatrix[1][1] * lPos_temp.y + rotMatrix[1][2] * lPos_temp.z;
  rotatedVectorOut.z =
      rotMatrix[2][0] * lPos_temp.x + rotMatrix[2][1] * lPos_temp.y + rotMatrix[2][2] * lPos_temp.z;
}

inline void
TBVector::getMatrixFromVectors(TBVector forwardVector, TBVector upVector, float* matrixOut) {
  TBVector rv = TBVector::CrossProduct(upVector, forwardVector);

  matrixOut[0] = rv[0];
  matrixOut[1] = upVector.x;
  matrixOut[2] = forwardVector.x;
  matrixOut[3] = rv[1];
  matrixOut[4] = upVector.y;
  matrixOut[5] = forwardVector.y;
  matrixOut[6] = rv[2];
  matrixOut[7] = upVector.z;
  matrixOut[8] = forwardVector.z;
}

// Warning: the output matrix is in normal right handed x,y,z coordinates.
inline void
TBVector::getFromToRotationMatrix(TBVector fromPoint, TBVector toPoint, float* matrixOut) {
  // swap y and z:
  float fr[3] = {fromPoint.x, fromPoint.z, fromPoint.y};
  float to[3] = {toPoint.x, toPoint.z, toPoint.y};

  float u[3] = {
      fr[1] * to[2] - fr[2] * to[1], fr[2] * to[0] - fr[0] * to[2], fr[0] * to[1] - fr[1] * to[0]};

  if (std::abs(u[0]) < TBE_SMALL_NUMBER && std::abs(u[1]) < TBE_SMALL_NUMBER &&
      std::abs(u[2]) < TBE_SMALL_NUMBER) {
    // this is the result of the cross product if 'fr' and 'to' are parallel, apply fudge to 'u':
    u[0] = fr[1];
    // this isn't a proper mathematical correction, just need to guarantee 'u' to be something not
    // parallel to 'fr'.
    u[1] = fr[2];
    u[2] = fr[0];
  }

  float inv_mag = 1.f / std::sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
  u[0] *= inv_mag;
  u[1] *= inv_mag;
  u[2] *= inv_mag;
  // u is now the unit vector about which we want to rotate.

  // construct the rotation matrix R = cos(theta)*I + sin(theta)[u]_x + (1-cos(theta))[u]x[u]
  // see https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle for
  // details
  float cos = fr[0] * to[0] + fr[1] * to[1] + fr[2] * to[2];
  float mag = std::sqrt(fr[0] * fr[0] + fr[1] * fr[1] + fr[2] * fr[2]) *
      std::sqrt(to[0] * to[0] + to[1] * to[1] + to[2] * to[2]);
  cos /= mag;

  float sin = std::sqrt(1.f - cos * cos);

  // main diagonal:
  matrixOut[0] = cos + (1 - cos) * u[0] * u[0];
  matrixOut[4] = cos + (1 - cos) * u[1] * u[1];
  matrixOut[8] = cos + (1 - cos) * u[2] * u[2];
  // top right:
  matrixOut[1] = -sin * u[2] + (1 - cos) * u[0] * u[1];
  matrixOut[2] = sin * u[1] + (1 - cos) * u[0] * u[2];
  matrixOut[5] = -sin * u[0] + (1 - cos) * u[1] * u[2];
  // bottom left:
  matrixOut[3] = sin * u[2] + (1 - cos) * u[0] * u[1];
  matrixOut[6] = -sin * u[1] + (1 - cos) * u[0] * u[2];
  matrixOut[7] = sin * u[0] + (1 - cos) * u[1] * u[2];
}

inline Aed TBVector::getAedFromVector(TBVector vector) {
  Aed result;
  result.azimuth = std::atan2(vector.x, vector.z) * 180.f / M_PIF;
  result.elevation =
      std::atan2(vector.y, std::sqrt(vector.x * vector.x + vector.z * vector.z)) * 180.f / M_PIF;
  result.distance = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

  return result;
}

inline TBVector TBVector::getVectorFromAziEle(float azimuth, float elevation) {
  return getVectorFromAziEleDist(azimuth, elevation, 1.f);
}

inline TBVector TBVector::getVectorFromAziEleDist(float azimuth, float elevation, float distance) {
  const float azi_r = azimuth * M_PIF / 180.f;
  const float ele_r = elevation * M_PIF / 180.f;
  const float cos_ele = std::cos(ele_r);
  return TBVector(
      distance * std::sin(azi_r) * cos_ele,
      distance * std::sin(ele_r),
      distance * std::cos(azi_r) * cos_ele);
}

inline TBVector TBVector::getVectorFromEuler(TBVector eulerAngles) {
  const float X = eulerAngles.x * M_PIF / 180.f;
  const float Y = eulerAngles.y * M_PIF / 180.f;
  const float cX = std::cos(X);
  const float sX = std::sin(X);
  const float cY = std::cos(Y);
  const float sY = std::sin(Y);

  return TBVector(sY * cX, -sX, cY * cX);
}

inline void TBVector::getMatrixFromEuler(TBVector eulerAngles, float* matrixOut) {
  const float X = eulerAngles.x * M_PIF / 180.f;
  const float Y = eulerAngles.y * M_PIF / 180.f;
  const float Z = eulerAngles.z * M_PIF / 180.f;
  const float cX = std::cos(X);
  const float sX = std::sin(X);
  const float cY = std::cos(Y);
  const float sY = std::sin(Y);
  const float cZ = std::cos(Z);
  const float sZ = std::sin(Z);

  matrixOut[0] = cY * cZ + sY * sX * sZ;
  matrixOut[1] = -cY * sZ + sY * sX * cZ;
  matrixOut[2] = sY * cX;
  matrixOut[3] = cX * sZ;
  matrixOut[4] = cX * cZ;
  matrixOut[5] = -sX;
  matrixOut[6] = -sY * cZ + cY * sX * sZ;
  matrixOut[7] = sY * sZ + cY * sX * cZ;
  matrixOut[8] = cY * cX;
}
} // namespace TBE

#endif // FBA_TBE_VECTOR_H
