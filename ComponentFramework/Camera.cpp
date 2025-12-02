#include "Camera.h"

void Camera::setProjectionMatrix(const Matrix4& proj)
{
	projectionMatrix = proj;
	projectionMatrix[5] *= -1.0f; // invert Y for vulkan
	cameraData.projectionMatrix = projectionMatrix;
	updateUBO();
}

void Camera::setPosition(const Vec3& pos_)
{
	position = pos_;
	orientationPosition = DualQuat(angle, axis, position);
	cameraData.viewMatrix = MMath::toMatrix4(orientationPosition);
	updateUBO();
}

void Camera::setOrientation(const float& angle_, const Vec3& axis_)
{
	angle = angle_;
	axis = axis_;
	orientationPosition = DualQuat(angle, axis, position);
	cameraData.viewMatrix = MMath::toMatrix4(orientationPosition);
	updateUBO();
}
