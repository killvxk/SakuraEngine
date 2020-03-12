/*
 * @CopyRight: MIT License
 * Copyright (c) 2020 SaeruHikari
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THESOFTWARE.
 * 
 * 
 * @Description: 
 * @Version: 0.1.0
 * @Autor: SaeruHikari
 * @Date: 2020-03-08 21:06:12
 * @LastEditTime: 2020-03-11 22:09:04
 */
#include "GraphicsPipelineVk.h"
#include "../Flags/GraphicsPipelineStatesVk.h"
#include "RenderProgressVk.h"
#include "../ResourceObjects/ResourceViewVk.h"
#include "../CGD_Vulkan.h"
#include "Core/EngineUtils/SHash.h"

using namespace Sakura::Graphics;
using namespace Sakura::Graphics::Vk;

GraphicsPipelineVk::~GraphicsPipelineVk()
{
    for(auto&& iter : fbs)
        vkDestroyFramebuffer(cgd.GetCGDEntity().device, iter.second, nullptr);
    vkDestroyPipeline(cgd.GetCGDEntity().device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(
        cgd.GetCGDEntity().device, pipelineLayout, nullptr);
}

GraphicsPipelineVk::GraphicsPipelineVk(const GraphicsPipelineCreateInfo& info, 
    const RenderProgressVk& progVk, const CGD_Vk& _cgd)
        :cgd(_cgd), progress(progVk)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount 
        = (uint32)info.pipelineLayoutInfo.setLayouts.size();
    pipelineLayoutInfo.pushConstantRangeCount 
        = (uint32)info.pipelineLayoutInfo.pushConstantRanges.size();

    if (vkCreatePipelineLayout(cgd.GetCGDEntity().device,
        &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        Sakura::Graphics::Vk::CGD_Vk::debug_error("failed to create pipeline layout!");
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = (uint32)info.shaderStages.size();
    std::vector<VkPipelineShaderStageCreateInfo> pStages(info.shaderStages.size()); 
    for(auto i = 0; i < info.shaderStages.size(); i++)
        pStages[i] = Transfer(info.shaderStages[i]);
    pipelineInfo.pStages = pStages.data();
    auto vertInput = Transfer(info.vertexInputInfo);
    auto ia = Transfer(info.inputassembly);
    auto vps = Transfer(info.viewportStateCreateInfo);
    auto raster = Transfer(info.rasterizationStateCreateInfo);
    auto ms = Transfer(info.multisampleStateCreateInfo);
    pipelineInfo.pVertexInputState = &vertInput;
    pipelineInfo.pInputAssemblyState = &ia;
    pipelineInfo.pViewportState = &vps;
    pipelineInfo.pRasterizationState = &raster;
    pipelineInfo.pMultisampleState = &ms;
    std::vector<VkPipelineColorBlendAttachmentState> 
        attachmentStates(info.colorBlendStateCreateInfo.colorBlendAttachment.size());
    for(auto i = 0u; i < attachmentStates.size(); i++)
        attachmentStates[i] = Transfer(info.colorBlendStateCreateInfo.colorBlendAttachment[i]);
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = 
        info.colorBlendStateCreateInfo.logicOpEnable ? VK_TRUE : VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = (uint32)attachmentStates.size();
    colorBlending.pAttachments = attachmentStates.data();
    colorBlending.blendConstants[0] = info.colorBlendStateCreateInfo.blendConstants[0];
    colorBlending.blendConstants[1] = info.colorBlendStateCreateInfo.blendConstants[0];
    colorBlending.blendConstants[2] = info.colorBlendStateCreateInfo.blendConstants[0];
    colorBlending.blendConstants[3] = info.colorBlendStateCreateInfo.blendConstants[0];
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = progVk.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(
        cgd.GetCGDEntity().device, VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
    {
        Sakura::Graphics::Vk::CGD_Vk::debug_error("failed to create graphics pipeline!");
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

std::unique_ptr<GraphicsPipeline> CGD_Vk::CreateGraphicsPipeline(
    const GraphicsPipelineCreateInfo& info,
    const RenderProgress& progress) const
{
    const RenderProgressVk& prog = (const RenderProgressVk&)progress;
    auto* vkPipeline = new GraphicsPipelineVk(info, prog, *this);
    return std::move(std::unique_ptr<GraphicsPipeline>(vkPipeline));
}


VkFramebuffer GraphicsPipelineVk::FindFrameBuffer(const RenderTargetSet& rts)
{
    static unsigned long long seed = 1549356486765631;
    const void* data = (const void*)rts.rts;
    auto _h = Sakura::hash::hash(data, rts.rtNum * sizeof(RenderTarget*), seed);
    if(fbs.find(_h) == fbs.end())
        fbs[_h] = createFrameBuffer(rts);
    auto fb = fbs[_h];
    return fb;
}

VkFramebuffer GraphicsPipelineVk::createFrameBuffer(const RenderTargetSet& rts)
{
    VkFramebuffer result;
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = progress.renderPass;
    framebufferInfo.attachmentCount = rts.rtNum;
    std::vector<VkImageView> images(rts.rtNum);
    for(auto i = 0u; i < rts.rtNum; i++)
        images[i] = ((const ResourceViewVkImage*)rts.rts[i].srv)->vkImgView;
    const VkImageView* attach = (const VkImageView*)images.data();
    framebufferInfo.pAttachments = attach;
    framebufferInfo.width = rts.rts[0].resource->GetExtent().width;
    framebufferInfo.height = rts.rts[0].resource->GetExtent().height;
    framebufferInfo.layers = 1;
    
    if (vkCreateFramebuffer(cgd.GetCGDEntity().device, &framebufferInfo,
            nullptr, &result) != VK_SUCCESS)
    {
        Sakura::Graphics::Vk::CGD_Vk::debug_error("failed to create framebuffer!");
        throw std::runtime_error("failed to create framebuffer!");
    }
    return result;
}