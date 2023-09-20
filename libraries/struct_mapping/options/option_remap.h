#pragma once

#include <functional>

namespace struct_mapping
{

template <typename T>
class Remap
{
public:
    Remap(std::function<T(const T&)> remapper_)
        :   remapper(remapper_) {}

    template<typename M>
    void check_option() const
    {
        static_assert(
                !std::is_same_v<detail::remove_optional_t<M>, bool> || std::is_same_v<T, bool>,
                "bad option (Default): type error, expected bool");

        static_assert(
                !detail::is_integer_v<detail::remove_optional_t<M>> || detail::is_integer_v<T>,
                "bad option (Default): type error, expected integer");

        static_assert(
                !std::is_floating_point_v<detail::remove_optional_t<M>> || detail::is_integer_or_floating_point_v<T>,
                "bad option (Default): type error, expected integer or floating point");

        static_assert(
                !std::is_same_v<detail::remove_optional_t<M>, std::string>
                || std::is_same_v<T, std::string>
                || std::is_same_v<T, const char*>,
                "bad option (Default): type error, expected string");

        static_assert(
                !std::is_enum_v<detail::remove_optional_t<M>>
                || std::is_enum_v<T>, "bad option (Default): type error, expected enumeration");

        static_assert(
                detail::is_integral_or_floating_point_or_string_v<detail::remove_optional_t<M>>
                || std::is_same_v<detail::remove_optional_t<M>, T>
                || (std::is_class_v<detail::remove_optional_t<M>>
                    && (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>)),
                "bad option (Default): type error");
    }

    inline std::function<T(const T&)> get_remapper () const {
        return remapper;
    }

private:
    std::function<T(const T&)> remapper;
};

}