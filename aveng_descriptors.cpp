#include "aveng_descriptors.h"
#include <iostream>
// std
#include <cassert>
#include <stdexcept>

namespace aveng {

    // *************** Descriptor Set Layout Builder *********************

    AvengDescriptorSetLayout::Builder& AvengDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) 
    {
     
        std::cout << "Adding Binding (1 & 6): " << descriptorType << " at binding: " << binding << std::endl;
        assert(bindings.count(binding) == 0 && "Binding already in use");

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;

        return *this;
    }

    std::unique_ptr<AvengDescriptorSetLayout> AvengDescriptorSetLayout::Builder::build() const 
    {
        return std::make_unique<AvengDescriptorSetLayout>(engineDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    AvengDescriptorSetLayout::AvengDescriptorSetLayout(
        EngineDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : engineDevice{ device }, bindings{ bindings } 
    {
        std::cout << "Create layout" << std::endl;
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            engineDevice.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    AvengDescriptorSetLayout::~AvengDescriptorSetLayout() 
    {
        vkDestroyDescriptorSetLayout(engineDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    AvengDescriptorPool::Builder& AvengDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) 
    {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    AvengDescriptorPool::Builder& AvengDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) 
    {
        poolFlags = flags;
        return *this;
    }
    AvengDescriptorPool::Builder& AvengDescriptorPool::Builder::setMaxSets(uint32_t count) 
    {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<AvengDescriptorPool> AvengDescriptorPool::Builder::build() const 
    {
        return std::make_unique<AvengDescriptorPool>(engineDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    AvengDescriptorPool::AvengDescriptorPool(
        EngineDevice& device,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : engineDevice{ device } 
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(engineDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) 
            != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    AvengDescriptorPool::~AvengDescriptorPool() {
        vkDestroyDescriptorPool(engineDevice.device(), descriptorPool, nullptr);
    }

    bool AvengDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const 
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(engineDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void AvengDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const 
    {
        vkFreeDescriptorSets(
            engineDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void AvengDescriptorPool::resetPool() 
    {
        vkResetDescriptorPool(engineDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    AvengDescriptorWriter::AvengDescriptorWriter(AvengDescriptorSetLayout& setLayout, AvengDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    AvengDescriptorWriter& AvengDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) 
    {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    AvengDescriptorWriter& AvengDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) 
    {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool AvengDescriptorWriter::build(VkDescriptorSet& set) 
    {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) 
        {
            return false;
        }
        overwrite(set);
        return true;
    }

    void AvengDescriptorWriter::overwrite(VkDescriptorSet& set) 
    {
        std::cout << "Updating final descriptor sets!" << std::endl;
        for (auto& write : writes) 
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.engineDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }

}  // namespace aveng