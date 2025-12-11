#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene2.h"
#include <MMath.h>
#include "Debug.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"

Scene2::Scene2(Renderer* renderer_) :
	Scene(nullptr), renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene2::~Scene2() {
}

bool Scene2::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;

	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		lightsUBO = vRenderer->CreateUniformBuffers<LightsData>();
		//cameraUBO = vRenderer->CreateUniformBuffers<Camera>();

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		camera = new Camera(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -5.0f)); // -5 in front of mario | +5 behind mario
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

		//colour picking pipeline
		DescriptorSetBuilder descriptorSetBuilderColourPick(vRenderer->getDevice());
		descriptorSetBuilderColourPick.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		colourPickDescriptorSetInfo = descriptorSetBuilderColourPick.BuildDescriptorSet(vRenderer->getNumSwapchains());
 		colourPickPipelineInfo = vRenderer->CreateGraphicsPipeline(colourPickDescriptorSetInfo.descriptorSetLayout, "shaders/colourPicking.vert.spv", "shaders/colourPicking.frag.spv");


		DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice());
		descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		descriptorSetBuilder.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &mariosPants);

		DescriptorSetInfo mariosdescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains());

		DescriptorSetBuilder descriptorSetBuilder2(vRenderer->getDevice());
		descriptorSetBuilder2.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		descriptorSetBuilder2.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		descriptorSetBuilder2.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &luigisPants);

		DescriptorSetInfo luigisdescriptorSetInfo = descriptorSetBuilder2.BuildDescriptorSet(vRenderer->getNumSwapchains());

		

		// test of the ECS
		Ref<Actor> marioActor = std::make_shared<Actor>(nullptr);
		marioActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f));
		marioActor->SetDescriptorSetInfo(std::make_shared<DescriptorSetInfo>(mariosdescriptorSetInfo));
		//marioActor->AddComponent<MeshComponent>(mariosMesh);
		//marioActor->AddComponent<MaterialComponent>(nullptr, "./textures/mario_fire.png", &descriptorSetBuilder, renderer);
		//marioActor->AddComponent<MeshComponent>
		marioActor->setColourID(10000);
		marioActor->OnCreate();

		Ref<Actor> luigiActor = std::make_shared<Actor>(nullptr);
		luigiActor->AddComponent<TransformComponent>(nullptr, Vec3(2.0f, 0.0f, 0.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f));
		luigiActor->SetDescriptorSetInfo(std::make_shared<DescriptorSetInfo>(luigisdescriptorSetInfo));
		luigiActor->setColourID(9445865);
		luigiActor->OnCreate();

		actors.insert({ "Mario", marioActor });
		actors.insert({ "Luigi", luigiActor });

		pipelineInfo = vRenderer->CreateGraphicsPipeline(luigisdescriptorSetInfo.descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");

		vRenderer->CreateColourPickerResources();
		vRenderer->CreateColourPickerPipeline("shaders/colourPicking.vert.spv", "shaders/colourPicking.frag.spv", camera->getUBO());
	}
	break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene2::HandleEvents(const SDL_Event& sdlEvent) {

	switch (sdlEvent.type) {
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
		printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
		float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
		///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
		camera->setProjectionMatrix(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f));

		if (renderer->getRendererType() == RendererType::VULKAN) {
			dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
			VulkanRenderer* vRenderer;
			vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
			vRenderer->DestroyPipeline(pipelineInfo);
			pipelineInfo = vRenderer->CreateGraphicsPipeline(actors.find("Mario")->second->GetDescriptorSetInfo()->descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");
		}
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN: {
		//colour picker handling
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		int32_t x, y;
		x = sdlEvent.button.x;
		y = sdlEvent.button.y;

		vRenderer->cmdColourPickingBeginRecording();
		vRenderer->cmdColourPickingBindPipeline();
		vRenderer->cmdColourPickingBindDescriptor();
		vRenderer->cmdColourPickingBindMesh(mariosMesh);

		for (auto a : actors) {
			vRenderer->cmdColourPickingPushConstant(a.second->GetModelMatrixPushConst());
			vRenderer->cmdColourPickingDrawIndexed(mariosMesh);
		}
		vRenderer->cmdColourPickingEndRenderPass(x, y); // pass in the screen coords (also stop recording)
		int32_t colourID = vRenderer->ReadPixel(x, y); // extract colourID
		std::cout << "ColourID: " << colourID << std::endl << "X: " << x << "Y: " << y << std::endl;
		break;
	}
	}
}

void Scene2::Update(const float deltaTime) {
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;
	//mariosModelMatrix = MMath::rotate(elapsedTime * 90.0f, Vec3(0.0f, 1.0f, 0.0f));
	actors.find("Mario")->second->GetComponent<TransformComponent>()->ApplyRotation(1.0f, Vec3(0.0f,1.0f,1.0f));
	actors.find("Luigi")->second->GetComponent<TransformComponent>()->ApplyRotation(-1.0f, Vec3(1.0f, 1.0f, 0.0f));
	//luigisModelMatrix = MMath::translate(2.0, 0.0f, 0.0f) * MMath::rotate(elapsedTime * 90.0f, Vec3(1.0f, 0.0f, 0.0f));

	CheckFrustrum();
	//FusturmCheck();


}

static Vec4 ExtractRow(const Matrix4& matrix, int row) {

	return Vec4{
		matrix[row],
		matrix[row + 4],
		matrix[row + 8],
		matrix[row + 12]
	};
}

static Plane EnsurePlaneFacesCamera(const Plane& p) {
	// In view space, the camera is at the origin (0,0,0)
	// Distance from origin is just p.d
	Plane out = p;
	if (p.d < 0.0f) {
		out.x = -out.x;
		out.y = -out.y;
		out.z = -out.z;
		out.d = -out.d;
	}
	return out;
}

static Plane MakePlane(Vec4 rowVector) {
	Plane p;

	p.x = rowVector.x;
	p.y = rowVector.y;
	p.z = rowVector.z;
	p.d = rowVector.w;

	p = PMath::normalize(p);
	p = EnsurePlaneFacesCamera(p);
	return p;
}

std::vector<Plane> Scene2::GenerateFusturmPlane(Matrix4& model)
{
	std::vector<Plane> fusturm;
	Matrix4 proj = camera->getProjectionMatrix() * camera->getViewMatrix();
	Plane left, right, top, bottom, near, far;
	left.x = proj[3] + proj[0];
	left.y = proj[7] + proj[4];
	left.z = proj[11] + proj[8];
	left.d = (proj[15] + proj[12]);

	right.x = proj[3] - proj[0];
	right.y = proj[7] - proj[4];
	right.z = proj[11] - proj[8];
	right.d = (proj[15] - proj[12]);

	bottom.x = proj[3] + proj[1];
	bottom.y = proj[7] + proj[5];
	bottom.z = proj[11] + proj[9];
	bottom.d = (proj[15] + proj[13]);

	top.x = proj[3] - proj[1];
	top.y = proj[7] - proj[5];
	top.z = proj[11] - proj[9];
	top.d = (proj[15] - proj[13]);

	near.x = proj[3] + proj[2];
	near.y = proj[7] + proj[6];
	near.z = proj[11] + proj[10];
	near.d = proj[15] + proj[14];

	far.x = proj[3] - proj[2];
	far.y = proj[7] - proj[6];
	far.z = proj[11] - proj[10];
	far.d = (proj[15] - proj[14]);

	//Normalizaiont matters if we care for the actual distance
	// when we do the dot product.
	// if we are just checking below or above 0 then 
	// no need to normalize.
	left = PMath::normalize(left);
	right = PMath::normalize(right);
	bottom = PMath::normalize(bottom);
	top = PMath::normalize(top);
	near = PMath::normalize(near);
	far = PMath::normalize(far);

	fusturm.push_back(left);
	fusturm.push_back(right);
	fusturm.push_back(bottom);
	fusturm.push_back(top);
	fusturm.push_back(near);
	fusturm.push_back(far);
	return fusturm;
}

void Scene2::FusturmCheck()
{

	Matrix4 view = camera->getViewMatrix();
	std::vector<Plane> fusturm = GenerateFusturmPlane(view);
	for (auto& a : actors) {
		Vec3 viewSpacePos = a.second->GetComponent<TransformComponent>()->GetPosition();
		a.second->culled = false;
		for (const Plane& p : fusturm) {
			float d = PMath::distance(viewSpacePos, p);
			if (d < 0) { // Can easly Changed to a Radius of a sphere around the Postion by -r instead of 0

				a.second->culled = true;
				break;
			}
		}
	}

}

Frustrum Scene2::CreateFrustrum() const
{
	
	Matrix4 clip = camera->getProjectionMatrix() * camera->getViewMatrix();
	Frustrum frustrum;
	Vec4 row0 = ExtractRow(clip, 0);
	Vec4 row1 = ExtractRow(clip, 1);
	Vec4 row2 = ExtractRow(clip, 2);
	Vec4 row3 = ExtractRow(clip, 3);

	frustrum.left	= MakePlane(row3 + row0);
	frustrum.right	= MakePlane(row3 - row0);
	frustrum.bottom	= MakePlane(row3 + row1);
	frustrum.top	= MakePlane(row3 - row1);
	frustrum.near	= MakePlane(row3 + row2);
	frustrum.far	= MakePlane(row3 - row2);

	return frustrum;
	//Frustrum f;

	//f.left.x = clip[3] + clip[0];
	//f.left.y = clip
	//f.left.z = 
	//f.left.d = 

	//f.right.x = 
	//f.right.y =
	//f.right.z =
	//f.right.d =

	//f.bottom.x = 
	//f.bottom.y =
	//f.bottom.z =
	//f.bottom.d =

	//f.top.x = 
	//f.top.y =
	//f.top.z =
	//f.top.d =

	//f.near.x = 
	//f.near.y =
	//f.near.z =
	//f.near.d =

	//f.far.x = 
	//f.far.y =
	//f.far.z =
	//f.far.d =

}

void Scene2::CheckFrustrum()
{
	Matrix4 view = camera->getViewMatrix();
	Frustrum fr = CreateFrustrum();
	for (auto a : actors) {
		auto actor = a.second;
		actor->culled = false;
		Vec3 worldPos = actor->GetComponent<TransformComponent>()->GetPosition();
		//Vec3 viewSpacePos = Vec3(view * Vec4(worldPos, 1.0f));
		for (const Plane& p : fr) {
			float d = PMath::distance(worldPos, p);
			std::cout << d << " ";
			if (d < 0) { // 
				actor->culled = true;
				break; // if its out of one its out of the others
			}
			std::cout << std::endl;
		}
	}

}




void Scene2::Render() const {
	switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);


		vRenderer->RecordCommandBuffers(Recording::START);

		//vRenderer->BindMesh(mariosMesh);
		//vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actors.find("Mario")->second->GetDescriptorSetInfo()->descriptorSet);
		//vRenderer->BindPipeline(pipelineInfo.pipeline);
		//vRenderer->SetPushConstant(pipelineInfo, actors.find("Mario")->second->GetComponent<TransformComponent>()->GetTransformMatrix()); // draw mario twices
		//vRenderer->DrawIndexed(mariosMesh);
		//vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actors.find("Luigi")->second->GetDescriptorSetInfo()->descriptorSet);
		//vRenderer->SetPushConstant(pipelineInfo, actors.find("Luigi")->second->GetComponent<TransformComponent>()->GetTransformMatrix());
		//vRenderer->DrawIndexed(mariosMesh);

		//vRenderer->BindDescriptorSet(colourPickPipelineInfo.pipelineLayout, colourPickDescriptorSetInfo.descriptorSet);
		//vRenderer->BindPipeline(colourPickPipelineInfo.pipeline);
		//vRenderer->SetPushConstant(colourPickPipelineInfo, actors.find("Mario")->second->GetModelMatrixPushConst());
		//vRenderer->DrawIndexed(mariosMesh);
		//vRenderer->SetPushConstant(colourPickPipelineInfo, actors.find("Luigi")->second->GetModelMatrixPushConst());
		//vRenderer->DrawIndexed(mariosMesh);
		vRenderer->BindPipeline(pipelineInfo.pipeline);

		for (const auto& a : actors) {
			auto actor = a.second;
			std::cout << "Actor: " << a.first << " Culled: " << (actor->culled ? "True" : "False") << std::endl;
			if (actor->culled) { continue; }
			vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actor->GetDescriptorSetInfo()->descriptorSet);
			vRenderer->SetPushConstant(pipelineInfo, actor->GetComponent<TransformComponent>()->GetTransformMatrix());
			vRenderer->DrawIndexed(mariosMesh); // hard coded because only two things in scene and both use same model
		}

		vRenderer->RecordCommandBuffers(Recording::STOP);
		vRenderer->Render();
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);

		break;
	}
}


void Scene2::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if (vRenderer) {
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyCommandBuffers();
		vRenderer->DestroyPipeline(pipelineInfo);
		//vRenderer->DestroyDescriptorSet(mariosdescriptorSetInfo);
		if (actors.find("Mario") != actors.end() && actors.find("Mario")->second->GetDescriptorSetInfo()) {
			vRenderer->DestroyDescriptorSet(*actors.find("Mario")->second->GetDescriptorSetInfo());
		}
		if (actors.find("Luigi") != actors.end() && actors.find("Luigi")->second->GetDescriptorSetInfo()) {
			vRenderer->DestroyDescriptorSet(*actors.find("Luigi")->second->GetDescriptorSetInfo());
		}
		vRenderer->DestroyUBO(lightsUBO);
		//vRenderer->DestroyUBO(cameraUBO);
		vRenderer->DestroySampler2D(mariosPants);
		vRenderer->DestroySampler2D(luigisPants);
		vRenderer->DestroyIndexedMesh(mariosMesh);

		vRenderer->DestroyColourPickerPipeline();
		vRenderer->DestroyColourPickerResources();
	}


}

