#include "Actor.h"
#include "Debug.h"
#include "TransformComponent.h"
Actor::Actor(Component* parent_) :Component(parent_) {}

bool Actor::OnCreate() {
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	for (auto component : components) {
		if (component->OnCreate() == false) {
			Debug::Error("Loading assets for Actor/Components: ", __FILE__, __LINE__);
			isCreated = false;
			return isCreated;
		}
	}
	isCreated = true;
	return isCreated;
}


Actor::~Actor() {
	OnDestroy();
}

void Actor::OnDestroy() {
	Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
	RemoveAllComponents();
	isCreated = false;
}



void Actor::Update(const float deltaTime) {
	std::cout << "Hello from Update\n";
}

void Actor::Render()const {}

void Actor::RemoveAllComponents() {
	components.clear();
}



ModelMatrixPushConst Actor::GetModelMatrixPushConst() const
{
	if (GetComponent<TransformComponent>() == nullptr){ return ModelMatrixPushConst(); }
	ModelMatrixPushConst modelMatrixPushConst;
	Matrix4 modelMatrix_ = GetComponent<TransformComponent>()->GetTransformMatrix();
	modelMatrixPushConst.modelMatrix = modelMatrix_;
	Matrix3 normalMatrix = Matrix3(MMath::transpose(MMath::inverse(modelMatrix_)));

	/// See the header file for an explanation of how I laid out this out in memory
	modelMatrixPushConst.normalMatrix[0].x = normalMatrix[0];
	modelMatrixPushConst.normalMatrix[0].y = normalMatrix[1];
	modelMatrixPushConst.normalMatrix[0].z = normalMatrix[2];

	modelMatrixPushConst.normalMatrix[1].x = normalMatrix[3];
	modelMatrixPushConst.normalMatrix[1].y = normalMatrix[4];
	modelMatrixPushConst.normalMatrix[1].z = normalMatrix[5];

	modelMatrixPushConst.normalMatrix[2].x = normalMatrix[6];
	modelMatrixPushConst.normalMatrix[2].y = normalMatrix[7];
	modelMatrixPushConst.normalMatrix[2].z = normalMatrix[8];

	modelMatrixPushConst.colour = colourID; // colour data

	return modelMatrixPushConst;
}

void Actor::ListComponents() const {
	std::cout << typeid(*this).name() << " contains the following components:\n";
	for (auto& component : components) {
		std::cout << typeid(*component).name() << std::endl;
	}
	std::cout << '\n';
}
//
//Matrix4 Actor::GetModelMatrix() {
//	Matrix4 modelMatrix;
//	// Get the TransformComponent from the Actor, if it exists.  If it does, update the model matrix.  
//	// If it doesn't, assume identity.  Then, if the Actor has a parent, multiply the parent's model matrix by the current model matrix.  Return the final model matrix.
//	Ref<TransformComponent> transform = GetComponent<TransformComponent>();
//	if (transform) {
//		modelMatrix = transform->GetTransformMatrix();
//	}
//	else {
//		modelMatrix.loadIdentity();
//	}
//	if (parent) { /// <== LOOK AT IT!  Think!
//		modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix() * modelMatrix;
//	}
//	return modelMatrix;
//}
