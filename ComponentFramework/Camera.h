#pragma once
#include "VulkanRenderer.h"
#include <DualQuat.h>
#include <MMath.h>
#include "CoreStructs.h"

using namespace MATH;
using namespace MATHEX;
class Camera {
private:
    DualQuat orientationPosition; // IS!!! your view matrix
	float angle;
	Vec3 axis;
	Vec3 position;
    Matrix4 projectionMatrix;
    //Matrix4 viewMatrix;
    std::vector<BufferMemory> cameraUBO;
	VulkanRenderer* vRenderer;
	CameraData cameraData;


public:

    //other camera constructors
	Camera(Matrix4 proj_, float angle_, Vec3 axis_, Vec3 pos_) : projectionMatrix(proj_), angle(angle_), axis(axis_), position(pos_){
		projectionMatrix[5] *= -1.0f; // invert Y for vulkan

		orientationPosition = DualQuat(angle, axis, position);
		cameraData = { projectionMatrix, MMath::toMatrix4(orientationPosition) };
	}
    Camera() {
		//orientationPosition = DualQuat(Quaternion(1, 0, 0, 0), Vec3(0, 0, 0));
		projectionMatrix = MMath::perspective(45.0f, 4.0f / 3.0f, 0.5f, 100.0f);
		projectionMatrix[5] *= -1.0f; // invert Y for vulkan

		angle = 0.0f;
		axis = Vec3(0.0f, 1.0f, 0.0f);
		position = Vec3(0.0f, 0.0f, -5.0f);
		orientationPosition = DualQuat(angle, axis, position);
		cameraData = { projectionMatrix, MMath::toMatrix4(orientationPosition) };
    }

    ~Camera() { 
		vRenderer->DestroyUBO(cameraUBO);
    }

	void setUBO() { cameraUBO = vRenderer->CreateUniformBuffers<CameraData>(); }
	void updateUBO() { vRenderer->UpdateUniformBuffer<CameraData>(cameraData, cameraUBO); }
	void setRenderer(VulkanRenderer* renderer) { vRenderer = renderer; }
	void setProjectionMatrix(const Matrix4& proj);

	// for a FPS controller update the position and create a temp DQ and multiply your old DW with the new one
	// for regular setting of a position just set the position and recalculate the DQ

	void setPosition(const Vec3& pos);
	void setOrientation(const float& angle_, const Vec3& axis_);

	std::vector<BufferMemory> getUBO() { return cameraUBO; }
	Matrix4 getProjectionMatrix() { return projectionMatrix; }

    Matrix4 getViewMatrix() { return MMath::toMatrix4(orientationPosition); }
    //cameraUBO = vRenderer->CreateUniformBuffers<Camera>();

};