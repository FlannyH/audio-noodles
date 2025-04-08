#pragma once
#include <cstdint>
#include <string>
#include <map>
#define N_VALUES 256

namespace UI {
    struct ValuePool {
        std::map<std::string, uint64_t> values;

        // Get value from name
        template <typename T> T& get(const std::string& name) {
            static_assert(sizeof(T) <= sizeof(uint64_t));
            return reinterpret_cast<T&>(values[name]);
        }

        // Set the current value
        template <typename T> void set_value(const std::string& name, T value) {
            static_assert(sizeof(T) <= sizeof(uint64_t));
            values[name] = *reinterpret_cast<uint64_t*>(&value);
        }

        // Set the current pointer
        template <typename T> void set_ptr(const std::string& name, T* value) {
            values[name] = reinterpret_cast<uint64_t>(value);
        }
    };

    enum class VarType { none, wstring, float64 };

    struct Value {
        // Index into value pool
        std::string name;
        VarType type{};
        bool has_changed = false;
        ValuePool& value_pool;

        Value(ValuePool& set_value_pool) : value_pool(set_value_pool) {}

        Value(const std::string& set_name, const VarType var_type, ValuePool& set_value_pool) : value_pool(set_value_pool) {
            // If this name already exists, it will bind to the name's corresponding value
            name = set_name;
            type = var_type;
        }

        template <typename T> T& get_as_ref() {
            static_assert(sizeof(T) <= sizeof(uint64_t));
            return reinterpret_cast<T&>(value_pool.values[name]);
        }
        template <typename T> T* get_as_ptr() { return reinterpret_cast<T*>(value_pool.values[name]); }

        template <typename T> void set(T value) {
            static_assert(sizeof(T) <= sizeof(uint64_t));
            if (get_as_ref<T>() != static_cast<T>(value)) has_changed = true;
            get_as_ref<T>() = static_cast<T>(value);
        }
    };
} // namespace UI
