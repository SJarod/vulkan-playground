#include "context.hpp"

#include "surface.hpp"

Surface::Surface(std::weak_ptr<Context> cx, PFN_CreateSurfacePredicate predicate, void *windowHandle) : m_cx(cx)
{
    VkResult result = predicate(m_cx.lock()->getInstanceHandle(), windowHandle, nullptr, &m_handle);
    if (result != VK_SUCCESS)
        std::cerr << "Failed to create window surface : " << result << std::endl;
}

Surface::~Surface()
{
    if (m_handle)
        vkDestroySurfaceKHR(m_cx.lock()->getInstanceHandle(), m_handle, nullptr);
}