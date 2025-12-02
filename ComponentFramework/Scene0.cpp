#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene0.h"
#include <MMath.h>
#include "Debug.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"

Scene0::Scene0(Renderer *renderer_): 
	Scene(nullptr),renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0::~Scene0() {
}

bool Scene0::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;

	switch (renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		
		lightsUBO = vRenderer->CreateUniformBuffers<LightsData>();
		//cameraUBO = vRenderer->CreateUniformBuffers<Camera>();

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		camera = new Camera(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -5.0f));
		//camera = new Camera();
		//camera.projectionMatrix = MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
		//camera.projectionMatrix[5] *= -1.0f;
		//camera.viewMatrix = MMath::translate(0.0f, 0.0f, -5.0f);
		camera->setRenderer(vRenderer);
		camera->setUBO();
		camera->updateUBO();

		lights.numLights = 3;
		lights.ambient = Vec4(0.1, 0.1, 0.1, 0.0);

		lights.diffuse[0] = Vec4(0.5, 0.0, 0.0, 0.0);
		lights.specular[0] = Vec4(0.3, 0.0, 0.0, 0.0);
		lights.pos[0] = Vec4(-5.0f, 0.0f, -5.0f, 0.0f);

		lights.diffuse[1] = Vec4(0.0, 0.0, 0.5, 0.0);
		lights.specular[1] = Vec4(0.0, 0.0, 0.3, 0.0);
		lights.pos[1] = Vec4(5.0f, 0.0f, -5.0f, 0.0f);

		lights.diffuse[2] = Vec4(0.0, 0.5, 0.0, 0.0);
		lights.specular[2] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.pos[2] = Vec4(0.0f, 5.0f, -5.0f, 0.0f);

		//lights.diffuse[3] = Vec4(0.0, 0.5, 0.5, 0.0);
		//lights.specular[3] = Vec4(0.0, 0.3, 0.3, 0.0);
		//lights.pos[3] = Vec4(0.0f, -4.0f, -5.0f, 0.0f);

		vRenderer->UpdateUniformBuffer<LightsData>(lights, lightsUBO); // lights UBO (YOU WILL BE MODIFYING THIS)
		//vRenderer->UpdateUniformBuffer<Camera>(camera, camera->getUBO);

		mariosPants = vRenderer->Create2DTextureImage("./textures/mario_fire.png");
		luigisPants = vRenderer->Create2DTextureImage("./textures/mario_mime.png");	
		mariosMesh = vRenderer->LoadModelIndexed("./meshes/Mario.obj");

		DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice());
		descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		
		descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);

		descriptorSetBuilder.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &mariosPants);

		mariosdescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains());

		DescriptorSetBuilder descriptorSetBuilder2(vRenderer->getDevice());
		descriptorSetBuilder2.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		descriptorSetBuilder2.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		descriptorSetBuilder2.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &luigisPants);

		luigisdescriptorSetInfo = descriptorSetBuilder2.BuildDescriptorSet(vRenderer->getNumSwapchains());

		pipelineInfo = vRenderer->CreateGraphicsPipeline(mariosdescriptorSetInfo.descriptorSetLayout,"shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");

	
	
	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
			float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
			///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
			camera->setProjectionMatrix(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f));

			if(renderer->getRendererType() == RendererType::VULKAN){
				dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
				VulkanRenderer* vRenderer;
				vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
				vRenderer->DestroyPipeline(pipelineInfo);
				pipelineInfo = vRenderer->CreateGraphicsPipeline(mariosdescriptorSetInfo.descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");
			}
			break;
		}
	
}
void Scene0::Update(const float deltaTime) {
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;
	mariosModelMatrix = MMath::rotate(elapsedTime * 90.0f, Vec3(0.0f, 1.0f, 0.0f)) ;
	luigisModelMatrix =  MMath::translate(2.0, 0.0f,0.0f) * MMath::rotate(elapsedTime * 90.0f, Vec3(1.0f, 0.0f, 0.0f)) ;
}

void Scene0::Render() const {
		switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		
		
		vRenderer->RecordCommandBuffers(Recording::START);
		vRenderer->BindMesh(mariosMesh);
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, mariosdescriptorSetInfo.descriptorSet);
		vRenderer->BindPipeline(pipelineInfo.pipeline);
		vRenderer->SetPushConstant(pipelineInfo, mariosModelMatrix); // draw mario twices
		vRenderer->DrawIndexed(mariosMesh);
		vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, luigisdescriptorSetInfo.descriptorSet);
		vRenderer->SetPushConstant(pipelineInfo, luigisModelMatrix);
		vRenderer->DrawIndexed(mariosMesh);
		vRenderer->RecordCommandBuffers(Recording::STOP);
		vRenderer->Render();
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);
		
		break;
	}
}


void Scene0::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyCommandBuffers();
		vRenderer->DestroyPipeline(pipelineInfo);
		vRenderer->DestroyDescriptorSet(mariosdescriptorSetInfo);
		vRenderer->DestroyUBO(lightsUBO);
		//vRenderer->DestroyUBO(cameraUBO);
		vRenderer->DestroySampler2D(mariosPants);
		vRenderer->DestroyIndexedMesh(mariosMesh);
		
		
		}

	
}
