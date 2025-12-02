#include <iostream>
#include "TransformComponent.h"
#include "QMath.h"
#include "MMath.h"
using namespace MATH;
using namespace MATHEX;

TransformComponent::TransformComponent(Component* parent_) :Component(parent_) {
	//pos = Vec3(0.0f, 0.0f, 0.0f);
	//orientation = Quaternion(1.0f, Vec3(0.0f, 0.0f, 0.0f));
	orientationPosition = DualQuat(0.0f, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f));
	scale = Vec3(1.0f, 1.0f, 1.0f);
}

TransformComponent::TransformComponent(Component* parent_, Vec3 pos_, float angle_, Vec3 axis_, Vec3 scale_) :
	Component{ parent_ }, orientationPosition{ DualQuat(angle_, axis_, pos_) }, scale{ scale_ } {
}

TransformComponent::~TransformComponent() {}

bool TransformComponent::OnCreate() {
	if (isCreated == true) return true;
	isCreated == true;
	return true;
}

void TransformComponent::OnDestroy() {}

void TransformComponent::Update(const float deltaTime) {
	std::cout << "Hello from Update " << deltaTime << '\n';

}

void TransformComponent::Render()const {}

Matrix4 TransformComponent::GetTransformMatrix() const {
	return MMath::translate(DQMath::getTranslation(orientationPosition)) * MMath::scale(scale) * MMath::toMatrix4(DQMath::getRotation(orientationPosition));
}

void TransformComponent::ApplyRotation(float angle_, Vec3 axis_)
{
	orientationPosition = orientationPosition * DQMath::rotate(angle_,axis_);
}

void TransformComponent::SetPosition(Vec3 pos_)
{	
	orientationPosition = orientationPosition * DQMath::translate(pos_);

}

void TransformComponent::SetOrientation(float angle, Vec3 axis)
{
	orientationPosition = DualQuat(angle, axis, DQMath::getTranslation(orientationPosition));
}
