#pragma once

#include "EngineDevice.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace aveng {

    class AvengDescriptorSetLayout {
    public:

        class Builder {
        public:
            Builder(EngineDevice& device) : engineDevice{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<AvengDescriptorSetLayout> build() const;

        private:
            EngineDevice& engineDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };


        AvengDescriptorSetLayout(EngineDevice& engineDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~AvengDescriptorSetLayout();
        AvengDescriptorSetLayout(const AvengDescriptorSetLayout&) = delete;
        AvengDescriptorSetLayout& operator=(const AvengDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        EngineDevice& engineDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class AvengDescriptorWriter;
    };

    class AvengDescriptorPool {
    public:
        class Builder {
        public:
            Builder(EngineDevice& device) : engineDevice{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<AvengDescriptorPool> build() const;

        private:
            EngineDevice& engineDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        AvengDescriptorPool(
            EngineDevice& engineDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~AvengDescriptorPool();
        AvengDescriptorPool(const AvengDescriptorPool&) = delete;
        AvengDescriptorPool& operator=(const AvengDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        EngineDevice& engineDevice;
        VkDescriptorPool descriptorPool;

        friend class AvengDescriptorWriter;
    };

    class AvengDescriptorWriter {
    public:
        AvengDescriptorWriter(AvengDescriptorSetLayout& setLayout, AvengDescriptorPool& pool);

        AvengDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        AvengDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        AvengDescriptorSetLayout& setLayout;
        AvengDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace Aveng