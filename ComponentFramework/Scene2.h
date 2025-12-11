#ifndef SCENE2_H
#define SCENE2_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
#include "Camera.h"
#include "CoreStructs.h"
#include "Component.h"
#include "Actor.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "TransformComponent.h"
#include <unordered_map>

using namespace MATH;

/// Forward declarations 
union SDL_Event;

struct Frustrum {
	Plane left;
	Plane right;
	Plane top;
	Plane bottom;
	Plane near;
	Plane far;

	Plane* begin() { return &left; }
	Plane* end() { return (&far) + 1; }

	const Plane* begin() const { return &left; }
	const Plane* end()   const { return (&far) + 1; }

};


class Scene2 : public Scene {
private:

	Renderer* renderer;
	Camera* camera;

	std::unordered_map<std::string, Ref<Actor>> actors;

	//Matrix4 mariosModelMatrix;
	//Matrix4 luigisModelMatrix;
	Sampler2D  mariosPants;
	Sampler2D luigisPants;
	IndexedVertexBuffer mariosMesh;

	//std::vector<BufferMemory> cameraUBO;
	//CameraData camera;
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;


	//DescriptorSetInfo mariosdescriptorSetInfo;
	//DescriptorSetInfo luigisdescriptorSetInfo;

	PipelineInfo pipelineInfo;

	PipelineInfo colourPickPipelineInfo;
	DescriptorSetInfo colourPickDescriptorSetInfo;
	CommandBufferData commandBufferData;

public:

	explicit Scene2(Renderer* renderer_);
	virtual ~Scene2();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event& sdlEvent) override;

	// mine
	Frustrum CreateFrustrum() const; // only one camera
	void CheckFrustrum();


	// kevins
	std::vector<Plane> GenerateFusturmPlane(Matrix4& model);
	void FusturmCheck();

};


#endif // Scene2_H