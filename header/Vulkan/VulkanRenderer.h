#pragma once

#include "VulkanUtils.h"

#include "VulkanContext.h"
#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"
#include "VulkanImage.h"

#include <memory>

namespace VKRE {

    class VulkanRenderer {
    public:
        VulkanRenderer(std::shared_ptr<VulkanContext> context);
        ~VulkanRenderer();

        void Render();
        std::shared_ptr<VulkanImage2D> GetDrawImage() { return mDrawImage; }

        void ClearImage(VkCommandBuffer cmd, std::shared_ptr<VulkanImage2D> image);

    private:
        std::shared_ptr<VulkanContext> mContext;
        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
        std::shared_ptr<VulkanImage2D> mDrawImage; // TODO: Move to SceneRenderer?

        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
