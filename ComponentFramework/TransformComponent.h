#pragma once
#include "Component.h"
#include "Matrix.h"
#include "QMath.h"
#include "DQMath.h"
#include "DualQuat.h"
#include "Euler.h"
#include "CoreStructs.h"

using namespace MATH;
using namespace MATHEX;



class TransformComponent : public Component {
	friend class PhysicsComponent;
private:
	//Vec3 pos;
	Vec3 scale;
	DualQuat orientationPosition;
	//uint32_t colourID;
	

public:
	TransformComponent(Component* parent_);
	TransformComponent(Component* parent_, Vec3 pos_, float angle_, Vec3 axis_, Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f));
	~TransformComponent();
	bool OnCreate();
	void OnDestroy();
	void Update(const float deltaTime_);
	void Render() const;

	//Vec3 GetPosition() const { return DQMath::getTranslation(orientationPosition); }
	//Quaternion GetRotationQuat() const { return DQMath::getRotation(orientationPosition); }

	Vec3 GetPosition() const { return DQMath::getTranslation(orientationPosition); }
	Quaternion GetRotationQuat() const { return DQMath::getRotation(orientationPosition); }
	Vec3 GetScale() { return scale; }

	Matrix4 GetTransformMatrix() const;
	void SetTransform(Vec3 pos_, float angle, Vec3 axis, Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f)) {
		//pos = pos_;
		//orientation = orientation_;
		orientationPosition = DualQuat(angle, axis, pos_);
		scale = scale_;
	}
	void ApplyRotation(float angle, Vec3 axis);

	void SetScale(Vec3 scale_) { scale = scale_; }
	void SetPosition(Vec3 pos_);
	void SetOrientation(float angle, Vec3 axis);
};
