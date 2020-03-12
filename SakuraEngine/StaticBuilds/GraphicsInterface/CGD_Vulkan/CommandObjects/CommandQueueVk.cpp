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
 * @Date: 2020-03-03 10:41:13
 * @LastEditTime: 2020-03-12 13:52:17
 */
#include "CommandQueueVk.h"
#include "CommandContextVk.h"
#include "../CGD_Vulkan.h"
#include "../GraphicsObjects/FenceVk.h"

using namespace Sakura::Graphics;
using namespace Sakura::Graphics::Vk;

CommandQueueVk::CommandQueueVk(const CGD_Vk& _cgd)
    :cgd(_cgd)
{

}

void CommandQueueVk::Submit(CommandContext* commandContext)
{
    CommandContextVk* cmdVk = (CommandContextVk*)commandContext;
    VkSubmitInfo submitInfo;
    {
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = 0;
        submitInfo.pWaitSemaphores      = nullptr;
        submitInfo.pWaitDstStageMask    = 0;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmdVk->commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores    = nullptr;
    }
    
    // Ensure unbusy before submitting.
    vkWaitForFences(cgd.GetCGDEntity().device,
        1, &cmdVk->recordingFence, VK_TRUE, UINT64_MAX);
    vkResetFences(cgd.GetCGDEntity().device, 1, &cmdVk->recordingFence);
    if (vkQueueSubmit(vkQueue, 1,
            &submitInfo, cmdVk->recordingFence) != VK_SUCCESS) 
    {
        CGD_Vk::error("failed to submit draw command buffer!");
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

bool CommandQueueVk::Submit(Fence* fence, uint64 completedValue)
{
    auto FcVk = (FenceVk*)fence;
	const uint64_t waitValue = 0; // Wait until semaphore value is >= 2
	const uint64_t signalValue = completedValue; // Set semaphore value to 3

	VkTimelineSemaphoreSubmitInfo timelineInfo;
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.pNext = NULL;
	timelineInfo.waitSemaphoreValueCount = 1;
	timelineInfo.pWaitSemaphoreValues = &waitValue;
	timelineInfo.signalSemaphoreValueCount = 1;
	timelineInfo.pSignalSemaphoreValues = &signalValue;

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = &timelineInfo;
	submitInfo.waitSemaphoreCount = 0;
	//submitInfo.pWaitSemaphores = &FcVk->timelineSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &FcVk->timelineSemaphore;
	submitInfo.commandBufferCount = 0;
	submitInfo.pCommandBuffers = 0;

    if (vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
		CGD_Vk::error("failed to submit timeline semaphores!");
		throw std::runtime_error("failed to submit timeline semaphores!");
    }
    return true;
}

void CommandQueueVk::WaitIdle()
{
    vkQueueWaitIdle(vkQueue);
}