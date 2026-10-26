// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

#include <algorithm>

ToolRegistry::ToolRegistry() {

    table_ = std::make_unique<std::unique_ptr<std::vector<std::unique_ptr<Tool>>>[]>(64);
    numBuckets_ = 64;
    size_ = 0;

    for (size_t i = 0; i < numBuckets_; i++) {
        auto chain = std::make_unique<std::vector<std::unique_ptr<Tool>>>();
        table_[i] = std::move(chain);
    }
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
    if (!tool) { // null tool
        return false;
    }
    if (find(tool->name())) { // tool already exists
        tool = std::move(nullptr);
        return false;
    }
    if (load_factor() > 0.75) { // table is too full
        auto newTable = std::make_unique<std::unique_ptr<std::vector<std::unique_ptr<Tool>>>[]>(2*numBuckets_);
        for (size_t i = 0; i < 2*numBuckets_; i++) {
            auto chain = std::make_unique<std::vector<std::unique_ptr<Tool>>>();
            newTable[i] = std::move(chain);
        }
        for (size_t i = 0; i < numBuckets_; i++) {
            for (auto & j : *table_[i]) {
                Tool* t = j.get();
                size_t newIndex = hash(t->name()) & (2*numBuckets_-1);
                newTable[newIndex]->push_back(std::move(j));
            }
            /*for (size_t j = 0; j < table_[index]->size(); j++) {
                Tool* t = table_[index]->at(j).get();
                size_t newIndex = hash(t->name());
                newTable[newIndex]->push_back(std::move(table_[index]->at(j)));
            }*/
        }
        table_ = std::move(newTable);
        numBuckets_ *= 2;
    }
    size_t index = hash(tool->name()) & (bucket_count()-1);
    table_[index]->push_back(std::move(tool));
    size_++;
    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    size_t index = hash(name) & (bucket_count()-1);
    for (const auto & i : *table_[index]) {
        Tool* t = i.get();
        if (t->name() == name) {return t;}
    }
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (size_t i = 0; i < numBuckets_; i++) {
        for (const auto & j : *table_[i]) {
            Tool* t = j.get();
            fn(*t);
        }
    }
    // TODO
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return numBuckets_;
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(numBuckets_);
}

std::size_t ToolRegistry::max_probe_length() const {
    size_t max_length = 0;
    for (size_t i = 0; i < numBuckets_; i++) {
        if (table_[i]->size() > max_length) {
            max_length = table_[i]->size();
        }
    }
    return max_length;
}
