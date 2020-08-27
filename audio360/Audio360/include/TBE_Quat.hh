#ifndef FBA_TBE_QUAT_H
#define FBA_TBE_QUAT_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include "TBE_Vector.hh"

namespace TBE {
#ifndef TBE_SMALL_NUMBER
#define TBE_SMALL_NUMBER 1.e-8f
#endif
/// Quaternion class with helper methods and fully overloaded operators
/// Unless specifically stated otherwise, the quaternion operations in this class assume a
/// left-handed Unity-style coordinate system where x=right, y=up and z=forward. You can use the
/// convertFromOpenGLQuaternion(.) function to convert quaternions from an OpenGL-style coordinate
/// system.
class TBQuat {
 public:
  float x;
  float y;
  float z;
  float w;

  inline TBQuat();
  inline TBQuat(float xValue, float yValue, float zValue, float wValue);
  inline TBQuat(const TBQuat& quat);

  inline TBQuat& operator=(const TBQuat& other);

  inline TBQuat operator+(const TBQuat& other) const;
  inline TBQuat operator-(const TBQuat& other) const;
  inline TBQuat operator*(const TBQuat& other) const;

  inline TBQuat operator*(const float value) const;
  inline TBQuat operator/(const float value) const;

  bool operator==(const TBQuat& rhs) const;
  bool operator!=(const TBQuat& rhs) const;

  inline std::string toString(int precision = 6) const;

  inline TBQuat invert() const;

  /// Input an OpenGL-style quaternion and return a Unity-style TBQuat quaternion
  /// @param x X component of the OpenGL-style quaternion, x=right
  /// @param y Y component of the OpenGL-style quaternion, y=up
  /// @param z Y component of the OpenGL-style quaternion, z=out of screen (backwards)
  /// @param w W component of the OpenGL-style quaternion, Right-handed rotation
  inline static TBQuat convertFromOpenGLQuaternion(float x, float y, float z, float w);

  /// Calculate quaternion from Euler angles (in Unity coordinate system) in radians
  /// @param x_radians Pitch angle (radians) about the x axis (x=right), +ve rotates down
  /// @param y_radians Yaw angle (radians) about the y axis (y=up), +ve rotates right
  /// @param z_radians Roll angle (radians) about the z axis (z=forward), +ve rotates rolling left
  /// @return The quaternion rotation
  inline static TBQuat getQuatFromEulerAngles(float x_radians, float y_radians, float z_radians);

  /// Get the Euler angle rotation from a quaternion
  /// @param q Input quaternion rotation
  /// @return The Euler angles in radians
  inline static TBVector getEulerAnglesFromQuat(TBQuat q);

  /// Calculate azimuth, elevation and distance of a source respective to the listener's position
  /// and orientation
  /// @param listenerQuat Listener quaternion
  /// @param listenerPosition Listener position
  /// @param sourcePosition SpatialiserImpl position
  /// @return Azimuth, elevation and distance class returned.
  inline static Aed
  getAedFromQuat(TBQuat listenerQuat, TBVector listenerPosition, TBVector sourcePosition);

  /// Multiply two quaternions together. No checking for quaternion normalisation is carried out.
  /// For correct rotation the input quaternions should be normalised.
  /// @param a The first quaternion
  /// @param b The second quaternion
  /// @return The quaternion product a*b
  inline static TBQuat quatProductUnNormalised(TBQuat a, TBQuat b);

  /// Rotates the input vector by the specified quaternion
  /// @param quat Quaternion rotation
  /// @param vector Input vector
  /// @return The rotated vector
  inline static TBVector rotateVectorByQuat(TBQuat quat, TBVector vector);

  /// Rotates the input vector by the specified quaternion in the opposite direction
  /// @param rotQuat Quaternion rotation
  /// @param vector Input vector
  /// @return The rotated vector
  inline static TBVector antiRotateVectorByQuat(TBQuat rotQuat, TBVector vector);

  /// Get the quaternion defined by rotating from one vector to another
  /// @param fromVec The starting vector for the rotation
  /// @param toVec The finishing vector for the rotation
  /// @return The output quaternion
  inline static TBQuat getFromToQuatRotation(TBVector fromVec, TBVector toVec);

  /// Get the forward vector for a particular rotation
  /// @param quat Input rotation quaternion
  /// @return The forward vector
  inline static TBVector getForwardFromQuat(TBQuat quat);

  /// Get the up vector for a particular rotation
  /// @param quat Input rotation quaternion
  /// @return The up vector
  inline static TBVector getUpFromQuat(TBQuat quat);

  /// Get the right vector for a particular rotation
  /// @param quat Input rotation quaternion
  /// @return The right vector
  inline static TBVector getRightFromQuat(TBQuat quat);

  /// Get the quaternion rotation defined by the specified forward and up vectors
  /// @param forwardVector The input forward vector
  /// @param upVector The input up vector
  /// @return The output quaternion
  inline static TBQuat getQuatFromForwardAndUpVectors(TBVector forwardVector, TBVector upVector);

  /// Get the quaternion from a particular 3x3 rotation matrix
  /// @param inputMatrix The 3x3 rotation matrix input in row-major order
  /// @return The rotation as a quaternion
  inline static TBQuat getQuatFromMatrix(float* inputMatrix);

  /// Get the quaternion from a particular 3x3 rotation matrix
  /// @param m00 Matrix entry [0,0]
  /// @param m01 Matrix entry [0,1]
  /// @param m02 Matrix entry [0,2]
  /// @param m10 Matrix entry [1,0]
  /// @param m11 Matrix entry [1,1]
  /// @param m12 Matrix entry [1,2]
  /// @param m20 Matrix entry [2,0]
  /// @param m21 Matrix entry [2,1]
  /// @param m22 Matrix entry [2,2]
  /// @return The rotation as a quaternion
  inline static TBQuat getQuatFromMatrix(
      float m00,
      float m01,
      float m02,
      float m10,
      float m11,
      float m12,
      float m20,
      float m21,
      float m22);

  /// Get the 3x3 rotation matrix for a particular quaternion (numerically the same as
  /// 'getClockwiseRotationMatrixFromQuat')
  /// @param quat Input quaternion rotation
  /// @param outputMatrix The rotation matrix output
  inline static void getMatrixFromQuat(TBQuat quat, float* outputMatrix);

  /// Get the 3x3 rotation matrix for a particular quaternion
  /// @param quat Input quaternion rotation
  /// @param outputMatrix The rotation matrix output
  inline static void getClockwiseRotationMatrixFromQuat(TBQuat quat, float* outputMatrix);

  /// Get the 3x3 anti-rotation matrix for a particular quaternion
  /// @param quat Input quaternion rotation
  /// @param outputMatrix The anti-rotation matrix output
  inline static void getAntiClockRotationMatrixFromQuat(TBQuat quat, float* outputMatrix);

  /// Return the quaternion identity (0,0,0,1)
  /// @return Quaternion identity
  inline static TBQuat identity();

  /// Normalise the quaternion in place
  inline void normalise();

  /// Generate random normalised quaternion
  inline static TBQuat generateRandomQuat();
};

TBQuat::TBQuat() {}

TBQuat::TBQuat(float xValue, float yValue, float zValue, float wValue)
    : x(xValue), y(yValue), z(zValue), w(wValue) {}

TBQuat::TBQuat(const TBQuat& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

inline TBQuat& TBQuat::operator=(const TBQuat& other) {
  this->x = other.x;
  this->y = other.y;
  this->z = other.z;
  this->w = other.w;

  return *this;
}

inline TBQuat TBQuat::operator+(const TBQuat& other) const {
  return TBQuat(x + other.x, y + other.y, z + other.z, w + other.w);
}

inline TBQuat TBQuat::operator-(const TBQuat& other) const {
  return TBQuat(x - other.x, y - other.y, z - other.z, w - other.w);
}

inline TBQuat TBQuat::operator*(const TBQuat& other) const {
  // define new vector
  TBQuat result;

  // try new formula here

  result.w = (w * other.w) - (x * other.x) - (y * other.y) - (z * other.z);
  result.x = (w * other.x) + (x * other.w) + (y * other.z) - (z * other.y);
  result.y = (w * other.y) - (x * other.z) + (y * other.w) + (z * other.x);
  result.z = (w * other.z) + (x * other.y) - (y * other.x) + (z * other.w);

  // re-normalise the quat

  float n = std::sqrt(
      result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
  result.x /= n;
  result.y /= n;
  result.z /= n;
  result.w /= n;

  return result;
}

inline TBQuat TBQuat::operator*(const float value) const {
  return TBQuat(x * value, y * value, z * value, w * value);
}

inline TBQuat TBQuat::operator/(const float value) const {
  return TBQuat(x / value, y / value, z / value, w / value);
}

inline bool TBQuat::operator==(const TBQuat& rhs) const {
  return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

inline bool TBQuat::operator!=(const TBQuat& rhs) const {
  return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
}

inline std::string TBQuat::toString(int precision) const {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << "[" << x << ", " << y << ", " << z << ", "
         << w << "]";
  return stream.str();
}

inline TBQuat TBQuat::invert() const {
  return TBQuat(x, y, z, -w);
}

inline TBQuat TBQuat::convertFromOpenGLQuaternion(float x, float y, float z, float w) {
  return TBQuat(x, y, -z, -w);
}

inline TBQuat TBQuat::getQuatFromEulerAngles(float xRotation, float yRotation, float zRotation) {
  TBQuat result;

  const float cX = std::cos(xRotation * 0.5f);
  const float sX = std::sin(xRotation * 0.5f);
  const float cY = std::cos(yRotation * 0.5f);
  const float sY = std::sin(yRotation * 0.5f);
  const float cZ = std::cos(zRotation * 0.5f);
  const float sZ = std::sin(zRotation * 0.5f);

  // this calculation may output quaternions which have ALL components negative as compared to what
  // Unity might calculate, BUT, this result is still totally correct because they actually
  // represent the same rotation: q = -q
  result.w = cX * cY * cZ + sX * sY * sZ;
  result.x = sX * cY * cZ + cX * sY * sZ;
  result.y = cX * sY * cZ - sX * cY * sZ;
  result.z = cX * cY * sZ - sX * sY * cZ;

  return result;
}

inline TBVector TBQuat::getEulerAnglesFromQuat(TBQuat q) {
  const float w = q.w;
  const float x = q.x;
  const float y = q.y;
  const float z = q.z;

  const float discriminant = w * x - z * y;
  float yaw = 0.f;
  float pitch = 0.f;

  if (std::abs(discriminant) > 0.49f) {
    static const auto pi_2 = 0.5f * 3.14159265358979323846f;
    yaw = copysign(2.f * std::atan2(y, w), discriminant);
    pitch = copysign(pi_2, discriminant);
  } else {
    const float cos_yaw = 0.5 - (x * x + y * y);
    const float sin_yaw = w * y + x * z;
    yaw = std::atan2(sin_yaw, cos_yaw);
    pitch = std::asin(2.f * discriminant);
  }

  const float cos_roll = 0.5 - (x * x + z * z);
  const float sin_roll = w * z + x * y;
  const float roll = std::atan2(sin_roll, cos_roll);

  return TBVector(pitch, yaw, roll);
}

inline Aed
TBQuat::getAedFromQuat(TBQuat listenerQuat, TBVector sourcePosition, TBVector listenerPosition) {
  TBVector relativePos(
      sourcePosition.x - listenerPosition.x,
      sourcePosition.y - listenerPosition.y,
      sourcePosition.z - listenerPosition.z);
  TBQuat lisAntiRot = listenerQuat;
  lisAntiRot.x = lisAntiRot.x * -1.f;
  lisAntiRot.y = lisAntiRot.y * -1.f;
  lisAntiRot.z = lisAntiRot.z * -1.f;
  TBVector rotPos = rotateVectorByQuat(lisAntiRot, relativePos);

  return TBVector::getAedFromVector(rotPos);
}

inline TBQuat TBQuat::quatProductUnNormalised(TBQuat A, TBQuat B) {
  TBQuat result;
  result.w = (A.w * B.w) - (A.x * B.x) - (A.y * B.y) - (A.z * B.z);
  result.x = (A.w * B.x) + (A.x * B.w) + (A.y * B.z) - (A.z * B.y);
  result.y = (A.w * B.y) - (A.x * B.z) + (A.y * B.w) + (A.z * B.x);
  result.z = (A.w * B.z) + (A.x * B.y) - (A.y * B.x) + (A.z * B.w);
  return result;
}

inline TBVector TBQuat::rotateVectorByQuat(TBQuat rotQuat, TBVector vector) {
  // calculate conjugate
  TBQuat rotQuat_conj;
  rotQuat_conj.x = -rotQuat.x;
  rotQuat_conj.y = -rotQuat.y;
  rotQuat_conj.z = -rotQuat.z;
  rotQuat_conj.w = rotQuat.w;

  // multiply to calculate final quat after rotation
  TBQuat firstProd = quatProductUnNormalised(rotQuat, TBQuat(vector.x, vector.y, vector.z, 0.f));
  TBQuat outQuat = quatProductUnNormalised(firstProd, rotQuat_conj);

  return TBVector(outQuat.x, outQuat.y, outQuat.z);
}

inline TBVector TBQuat::antiRotateVectorByQuat(TBQuat rotQuat, TBVector vector) {
  // calculate conjugate
  TBQuat rotQuat_conj;
  rotQuat_conj.x = -rotQuat.x;
  rotQuat_conj.y = -rotQuat.y;
  rotQuat_conj.z = -rotQuat.z;
  rotQuat_conj.w = rotQuat.w;

  // multiply to calculate final quat after rotation
  TBQuat firstProd =
      quatProductUnNormalised(rotQuat_conj, TBQuat(vector.x, vector.y, vector.z, 0.0f));
  TBQuat outQuat = quatProductUnNormalised(firstProd, rotQuat);

  return TBVector(outQuat.x, outQuat.y, outQuat.z);
}

// get the quaternion rotation from one vector to another vector (same output as Unity's
// SetFromToRotation function).
inline TBQuat TBQuat::getFromToQuatRotation(TBVector fromVec, TBVector toVec) {
  TBVector tmp = TBVector::CrossProduct(fromVec, toVec);
  TBVector u = tmp;
  if (tmp.x < 0)
    tmp.x = -tmp.x;
  if (tmp.y < 0)
    tmp.y = -tmp.y;
  if (tmp.z < 0)
    tmp.z = -tmp.z;
  if (tmp.x + tmp.y + tmp.z < 0.0001f) {
    u = fromVec;
  }
  TBVector::normalise(u);
  float mag = TBVector::magnitude(fromVec) * TBVector::magnitude(toVec);
  float normDot = 0.f;
  if (mag > TBE_SMALL_NUMBER) {
    normDot = TBVector::DotProduct(fromVec, toVec) / mag;
  }
  float arg =
      std::abs(0.5f * (1.f - normDot)); // the 'abs' is not necessary in theory, but in practise is
                                        // needed due to precision errors, otherwise we might feed
                                        // negative numbers into the square root!
  float sinth = std::sqrt(arg);

  TBQuat q;
  q.x = u.x * sinth;
  q.y = u.y * sinth;
  q.z = u.z * sinth;
  q.w = std::sqrt(0.5f * (1.f + normDot));

  return q;
}

inline TBVector TBQuat::getForwardFromQuat(TBQuat q) {
  return TBVector(
      2 * (q.x * q.z + q.w * q.y), 2 * (q.y * q.z - q.w * q.x), 1 - 2 * (q.x * q.x + q.y * q.y));
}

inline TBVector TBQuat::getUpFromQuat(TBQuat q) {
  return TBVector(
      2 * (q.x * q.y - q.w * q.z), 1 - 2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z + q.w * q.x));
}

inline TBVector TBQuat::getRightFromQuat(TBQuat q) {
  return TBVector(
      1 - 2 * (q.y * q.y + q.z * q.z), 2 * (q.x * q.y + q.w * q.z), 2 * (q.x * q.z - q.w * q.y));
}

inline TBQuat TBQuat::getQuatFromForwardAndUpVectors(TBVector forwardVector, TBVector upVector) {
  const TBVector right = TBVector::CrossProduct(upVector, forwardVector);
  return TBQuat::getQuatFromMatrix(
      right.x,
      upVector.x,
      forwardVector.x,
      right.y,
      upVector.y,
      forwardVector.y,
      right.z,
      upVector.z,
      forwardVector.z);
}

inline TBQuat TBQuat::getQuatFromMatrix(float* m) {
  assert(m);
  return getQuatFromMatrix(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
}

inline TBQuat TBQuat::getQuatFromMatrix(
    float m00,
    float m01,
    float m02,
    float m10,
    float m11,
    float m12,
    float m20,
    float m21,
    float m22) {
  const double w = 0.5 * std::sqrt(std::max(0.0, 1.0 + m00 + m11 + m22));
  const double x = 0.5 * std::sqrt(std::max(0.0, 1.0 + m00 - m11 - m22));
  const double y = 0.5 * std::sqrt(std::max(0.0, 1.0 - m00 + m11 - m22));
  const double z = 0.5 * std::sqrt(std::max(0.0, 1.0 - m00 - m11 + m22));

  const float x_out = copysign(x, m21 - m12);
  const float y_out = copysign(y, m02 - m20);
  const float z_out = copysign(z, m10 - m01);

  return TBQuat(x_out, y_out, z_out, w);
}

inline void TBQuat::getMatrixFromQuat(TBQuat quat, float* outputMatrix) {
  assert(outputMatrix);
  const TBVector f = TBQuat::getForwardFromQuat(quat);
  const TBVector u = TBQuat::getUpFromQuat(quat);
  const TBVector r = TBQuat::getRightFromQuat(quat);

  outputMatrix[0] = r.x;
  outputMatrix[1] = u.x;
  outputMatrix[2] = f.x;
  outputMatrix[3] = r.y;
  outputMatrix[4] = u.y;
  outputMatrix[5] = f.y;
  outputMatrix[6] = r.z;
  outputMatrix[7] = u.z;
  outputMatrix[8] = f.z;
}

inline void TBQuat::getClockwiseRotationMatrixFromQuat(TBQuat quat, float* outputMatrix) {
  TBQuat::getMatrixFromQuat(quat, outputMatrix);
}

inline void TBQuat::getAntiClockRotationMatrixFromQuat(TBQuat quat, float* outputMatrix) {
  const TBQuat inv_q = TBQuat(quat.x, quat.y, quat.z, -quat.w);
  TBQuat::getMatrixFromQuat(inv_q, outputMatrix);
}

inline TBQuat TBQuat::identity() {
  return TBQuat(0.f, 0.f, 0.f, 1.f);
}

inline void TBQuat::normalise() {
  const float norm = 1.f / std::sqrt(x * x + y * y + z * z + w * w);
  x *= norm;
  y *= norm;
  z *= norm;
  w *= norm;
}

inline TBQuat TBQuat::generateRandomQuat() {
  auto random_num = []() {
    return 2.f * static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1.f;
  };

  TBQuat q(random_num(), random_num(), random_num(), random_num());
  q.normalise();

  return q;
}
} // namespace TBE

#endif // FBA_TBE_QUAT_H
