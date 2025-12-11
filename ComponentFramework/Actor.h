#pragma once
#include <MMath.h>
#include "CoreStructs.h"
#include <MMath.h>
#include "Component.h"
#include "VulkanRenderer.h"

using namespace MATH;
using namespace MATHEX;	
class Actor : public Component{
	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;
	Actor& operator= (const Actor&) = delete;
	Actor& operator=(Actor&&) = delete;
	std::vector<Ref<Component>> components;

	Ref<DescriptorSetInfo> descriptorSetInfo;

private:
	//Vec3 position;
	//Vec3 axis;
	//float angle;


	//Ref<VulkanRenderer> vRenderer;
	// coreStructs.h
	//Ref<DescriptorSetInfo> descriptorSetInfo;
	//IndexedVertexBuffer mesh;
	//Sampler2D texture;
	//Matrix4 modelMatrix;

	uint32_t colourID;

public:
	Actor(Component* parent_);
	~Actor();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;

	bool culled = false;

	template<typename ComponentTemplate>
	void RemoveComponent() {
		for (size_t i = 0; i < components.size(); i++) {
			if (dynamic_cast<ComponentTemplate*>(components[i]) != nullptr) {
				components[i]->OnDestroy();
				delete components[i];
				components.erase(components.begin() + i);
				break;
			}
		}
	}

	template<typename ComponentTemplate>
	void AddComponent(Ref<ComponentTemplate> component_) {
		if (GetComponent<ComponentTemplate>() != nullptr) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		}
		components.push_back(component_);
	}

	template<typename ComponentTemplate, typename ... Args>
	void AddComponent(Args&& ... args_) {
		/// before you add the component ask if you have the component in the list already,
		/// if so - don't add a second one. 
		if (GetComponent<ComponentTemplate>()) {
#ifdef _DEBUG
			std::cerr << "WARNING: Trying to add a component type that is already added - ignored\n";
#endif
			return;
		}
		/// Finish building the component and add the component to the list
		components.push_back(std::make_shared<ComponentTemplate>(std::forward<Args>(args_)...));
	}

	template<typename ComponentTemplate>
	Ref<ComponentTemplate> GetComponent() const {
		for (auto component : components) {
			if (dynamic_cast<ComponentTemplate*>(component.get())) {
				/// This is a dynamic cast designed for shared_ptr's
				/// https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
				return std::dynamic_pointer_cast<ComponentTemplate>(component);
			}
		}
		return Ref<ComponentTemplate>(nullptr);
	}

	void SetDescriptorSetInfo(Ref<DescriptorSetInfo> descriptorSetInfo_) {
		descriptorSetInfo = descriptorSetInfo_;
	}
	Ref<DescriptorSetInfo> GetDescriptorSetInfo() const {
		return descriptorSetInfo;
	}

	ModelMatrixPushConst GetModelMatrixPushConst() const;
	void setColourID(uint32_t colourID_) {
		colourID = colourID_;
	}

	void ListComponents() const;

	void RemoveAllComponents();


};

