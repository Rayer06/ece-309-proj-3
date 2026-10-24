// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

ToolRegistry::ToolRegistry() {
    table_.resize(64);
    numBuckets_ = 64;
    size_ = 0;
}

ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h  = 1469598103934665603ULL;
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return static_cast<std::size_t>(h);
}
// Registers a tool. If the tool already exists, keeps the original tool and destroys
// the new one
bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    size_t index = hash(tool->name()) & (bucket_count()-1);
    if (!tool) {
        return false;
    }
    if (find(tool->name())) {
        tool.release();
        return false;
    }
    return true;


    // TODO
}

Tool* ToolRegistry::find(std::string_view name) const {
    size_t index = hash(name) & (bucket_count()-1);
    //for (Tool* t : )
    return nullptr;   // TODO
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& /*fn*/) const {
    // TODO
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return numBuckets_;
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(numBuckets_);
}

std::size_t ToolRegistry::max_probe_length() const {
    return 0;   // TODO
}
