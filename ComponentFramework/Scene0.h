#ifndef SCENE0_H
#define SCENE0_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
#include "Camera.h"
#include "CoreStructs.h"
using namespace MATH;

/// Forward declarations 
union SDL_Event;


class Scene0 : public Scene {
private:
	
	Renderer *renderer;
	Camera *camera;
	Matrix4 mariosModelMatrix;
	Matrix4 luigisModelMatrix;
	Sampler2D  mariosPants;
	Sampler2D luigisPants;
	IndexedVertexBuffer mariosMesh;

	//std::vector<BufferMemory> cameraUBO;
	//CameraData camera;
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;


	DescriptorSetInfo mariosdescriptorSetInfo;
	DescriptorSetInfo luigisdescriptorSetInfo;

	PipelineInfo pipelineInfo;
	CommandBufferData commandBufferData;

public:

	explicit Scene0(Renderer* renderer_);
	virtual ~Scene0();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;
};


#endif // SCENE0_H