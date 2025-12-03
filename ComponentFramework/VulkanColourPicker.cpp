#include "VulkanRenderer.h"

// VkRenderPass cpRenderPass; // offscreen render pass
//
// VkImage cpImage; // main image
// VkDeviceMemory cpImageMemory;
// VkImageView cpImageView;
//
// VkImage cpDepthImage; //  depth image
// VkDeviceMemory cpDepthImageMemory;
// VkImageView cpDepthImageView;
//
// VkFramebuffer cpFramebuffer; // GPU framebuffer
// BufferMemory cpBufferMemory; // buffer to read pixels back to CPU
//
// CommandBufferData cpCommandBuffer;
// VkFence offscreencpFence; // fence to sync offscreen rendering
//
// DescriptorSetInfo cpDescriptorSetInfo;
// PipelineInfo cpPipelineInfo;

void VulkanRenderer::CreateColourPickerPipeline(const char* vertFile_, const char* fragFile_, std::vector<BufferMemory> cameraUBO)
{
	DescriptorSetBuilder descriptorSetBuilder(device);

	descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, cameraUBO);
	cpDescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(numSwapchains);

	PipelineInfo cpPipelineInfo = CreateColourPickerPipeline(vertFile_, fragFile_, cpDescriptorSetInfo.descriptorSetLayout, cpRenderPass);

}

void VulkanRenderer::DestroyColourPickerPipeline()
{
    vkDestroyPipelineLayout(device, cpPipelineInfo.pipelineLayout, nullptr);
    vkDestroyPipeline(device, cpPipelineInfo.pipeline, nullptr);
}

void VulkanRenderer::cmdColourPickingRecording(Recording start_stop)
{
    if (start_stop == Recording::START) {
        vkDeviceWaitIdle(device); /// This is bad
        for (size_t i = 0; i < cpCommandBuffer.commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(cpCommandBuffer.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.5f, 0.2f, 0.3f, 1.0f }; // background colour
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            vkCmdBeginRenderPass(cpCommandBuffer.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }
    }
    else if (start_stop == Recording::STOP) {
        for (size_t i = 0; i < cpCommandBuffer.commandBuffers.size(); i++) {
            vkCmdEndRenderPass(cpCommandBuffer.commandBuffers[i]);
            if (vkEndCommandBuffer(cpCommandBuffer.commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
}

void VulkanRenderer::cmdColourPickingPushConstant(const ModelMatrixPushConst& pushConst_)
{
    for (uint32_t i = 0; i < getNumSwapchains(); i++) {
        vkCmdPushConstants(cpCommandBuffer.commandBuffers[i], cpPipelineInfo.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelMatrixPushConst), &pushConst_);
    }
}

void VulkanRenderer::cmdColourPickingBindPipeline()
{
    for (uint32_t i = 0; i < getNumSwapchains(); i++) { // might just need the one
        vkCmdBindPipeline(cpCommandBuffer.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, cpPipelineInfo.pipeline);
    }
}

void VulkanRenderer::cmdColourPickingBindDescriptor() // only the first one
{
    for (uint32_t i = 0; i < numSwapchains; i++) {
        vkCmdBindDescriptorSets(cpCommandBuffer.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
            cpPipelineInfo.pipelineLayout, 0, 1, &cpDescriptorSetInfo.descriptorSet[0], 0, nullptr);
    }
}

void VulkanRenderer::cmdColourPickingBindMesh(IndexedVertexBuffer mesh)
{
    VkBuffer vertexBuffers[] = { mesh.vertBufferID };
    VkDeviceSize offsets[] = { 0 };
    for (uint32_t i = 0; i < getNumSwapchains(); i++) {
        vkCmdBindVertexBuffers(cpCommandBuffer.commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cpCommandBuffer.commandBuffers[i], mesh.indexBufferID, 0, VK_INDEX_TYPE_UINT32);
    }
}

void VulkanRenderer::cmdColourPickingDrawIndexed(IndexedVertexBuffer mesh)
{
    for (uint32_t i = 0; i < numSwapchains; i++) {
        vkCmdDrawIndexed(cpCommandBuffer.commandBuffers[i], static_cast<uint32_t>(mesh.indexBufferLength), 1, 0, 0, 0);
    }
}

uint32_t VulkanRenderer::ReadPixel(uint32_t x, uint32_t y)
{
    return 0;
}

void VulkanRenderer::CreateColourPickerResources()
{

}

void VulkanRenderer::DestroyColourPickerResources()
{
}

void VulkanRenderer::cmdCopyToBuffer()
{

}

void VulkanRenderer::SubmitRender()
{

}

PipelineInfo VulkanRenderer::CreateColourPickerPipeline(const char* vertFile, const char* fragFile, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass)
{
    PipelineInfo graphicsPipeInfo;

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;

    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};


    if (vertFile != nullptr && fragFile != nullptr) {
        vertShaderCode = readFile(vertFile);
        fragShaderCode = readFile(fragFile);
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
    }


    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";



    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if (vertShaderModule != VK_NULL_HANDLE) shaderStages.push_back(vertShaderStageInfo);

    if (fragShaderModule != VK_NULL_HANDLE) shaderStages.push_back(fragShaderStageInfo);


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset = 0;
    range.size = sizeof(ModelMatrixPushConst);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;
    //pipelineLayoutInfo.flags = VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &graphicsPipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }



    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = graphicsPipeInfo.pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }


    if (fragShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);

	return graphicsPipeInfo;
}

void VulkanRenderer::CreateColourPickerRenderPass(VkRenderPass renderPass_) // cpRenderPass
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanRenderer::DestroyColourPickerRenderPass(VkRenderPass renderPass_)
{
    vkDestroyRenderPass(device, renderPass_, nullptr);
}

void VulkanRenderer::CreateColourPickerDepthImage(VkImage& depthImage, VkDeviceMemory& depthImageMemory, VkImageView& depthImageView)
{
    uint32_t width = swapChainExtent.width;
    uint32_t height = swapChainExtent.height;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = findDepthFormat();
    //imageInfo.tiling = tiling;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //imageInfo.usage = usage;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &depthImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &depthImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device, depthImage, depthImageMemory, 0);

    //image view 
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = findDepthFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void VulkanRenderer::CreateColourPickerImage(VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView)
{
    uint32_t width = swapChainExtent.width;
    uint32_t height = swapChainExtent.height;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = swapChainImageFormat;
    //imageInfo.tiling = tiling;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //imageInfo.usage = usage;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device, image, imageMemory, 0);

    // image view

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void VulkanRenderer::DestroyImage(VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView)
{
    vkDestroyImage(device, imageView, nullptr);
    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
}

void VulkanRenderer::CreateColourPickerFramebuffer(VkFramebuffer& framebuffer, VkImageView imageView, VkImageView depthImageView, VkRenderPass renderPass)
{
    // TODO: change frame buffer to cpFrameBuffer
    swapChainFramebuffers.resize(numSwapchains);

    for (size_t i = 0; i < numSwapchains; i++) {
        std::array<VkImageView, 2> attachments = {
            imageView,
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void VulkanRenderer::DestroyFramebuffer(VkFramebuffer& framebuffer)
{
    vkDestroyFramebuffer(device, framebuffer, nullptr);
}

void VulkanRenderer::CreateColourPickerReadbackBuffer(BufferMemory& bufferMemory)
{
}

void VulkanRenderer::DestroyColourPickerReadbackBuffer(BufferMemory& bufferMemory)
{
}

void VulkanRenderer::CreateColourPickerCommandBuffer(CommandBufferData& commandBufferData)
{
    commandBufferData.commandBuffers.resize(numSwapchains);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandBufferData.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBufferData.commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBufferData.commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::CreateColourPickerCommandPool(VkCommandPool& commandPool)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &cpCommandBuffer.commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanRenderer::AllocateColourPickerCommandBuffers(CommandBufferData& commandBufferData, int count_)
{
	//  might not be needed (using vkAllocateCOmmandBuffers in CreateColourPickerCommandBuffer)
}

void VulkanRenderer::DestroyColourPickerCommandBuffer(CommandBufferData& commandBufferData)
{
    vkFreeCommandBuffers(device, cpCommandBuffer.commandPool, 1, cpCommandBuffer.commandBuffers.data());
    vkDestroyCommandPool(device, cpCommandBuffer.commandPool, nullptr);
}

void VulkanRenderer::CreateColourPickerFence(VkFence& fence)
{
}

void VulkanRenderer::DestroyColourPickerFence(VkFence& fence)
{
}
