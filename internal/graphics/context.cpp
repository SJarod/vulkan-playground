#include "context.hpp"

Context::Context()
{
    volkInitialize();

    m_instance = std::make_unique<Instance>(*this);

    volkLoadInstance(m_instance->getInstance());
}

int Context::enumerateAvailablePhysicalDevice() const
{
    return 0;
}