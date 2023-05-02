//
// Created by spak on 6/5/21.
//

#ifndef NEO_GRADIENT_HPP
#define NEO_GRADIENT_HPP

#include <mlab/bin_data.hpp>
#include <neo/color.hpp>
#include <map>
#include <vector>
#include <neo/ranges.hpp>

namespace neo {
    using blending_method = srgb (&)(srgb l, srgb r, float t);

    [[maybe_unused]] srgb blend_linear(srgb l, srgb r, float t);
    [[maybe_unused]] srgb blend_round_down(srgb l, srgb, float);
    [[maybe_unused]] srgb blend_round_up(srgb, srgb r, float);
    [[maybe_unused]] srgb blend_nearest_neighbor(srgb l, srgb r, float t);

    class gradient_entry;
    class fixed_gradient_entry;
}// namespace neo

namespace mlab {
    bin_stream &operator>>(bin_stream &i, neo::gradient_entry &ge);
    bin_data &operator<<(bin_data &o, neo::fixed_gradient_entry const &fge);
}// namespace mlab

namespace neo {

    class fixed_gradient_entry {
    protected:
        float _pos = 0.f;
        srgb _color = srgb{};

        fixed_gradient_entry &operator=(fixed_gradient_entry const &) = default;

        fixed_gradient_entry &operator=(fixed_gradient_entry &&) noexcept = default;

        friend mlab::bin_stream &mlab::operator>>(mlab::bin_stream &, neo::gradient_entry &);

    public:
        [[nodiscard]] inline float position() const;

        [[nodiscard]] inline srgb color() const;

        inline void set_color(srgb color);

        fixed_gradient_entry() = default;

        fixed_gradient_entry(fixed_gradient_entry const &) = default;

        fixed_gradient_entry(fixed_gradient_entry &&) noexcept = default;

        inline fixed_gradient_entry(float t, srgb c);

        inline fixed_gradient_entry &operator=(srgb c);

        [[nodiscard]] std::string to_string() const;
    };

    class gradient_entry : protected fixed_gradient_entry {
        friend class gradient;
        friend mlab::bin_stream &mlab::operator>>(mlab::bin_stream &, gradient_entry &);

    public:
        using fixed_gradient_entry::fixed_gradient_entry;
        using fixed_gradient_entry::operator=;
        using fixed_gradient_entry::color;
        using fixed_gradient_entry::set_color;
        using fixed_gradient_entry::position;

        inline void set_position(float t);

        gradient_entry() = default;

        gradient_entry(gradient_entry const &) = default;

        gradient_entry(gradient_entry &&) noexcept = default;

        gradient_entry &operator=(gradient_entry const &) = default;

        gradient_entry &operator=(gradient_entry &&) noexcept = default;
    };


    struct positioned_compare {
        constexpr bool operator()(float l, float r) const {
            return std::abs(l - r) > std::numeric_limits<float>::epsilon() and l < r;
        }

        template <ranges::has_position_key L>
        constexpr bool operator()(L &&lhs, float r) const {
            return operator()(lhs.first, r);
        }

        template <ranges::has_position_key R>
        constexpr bool operator()(float l, R &&rhs) const {
            return operator()(l, rhs.first);
        }

        template <ranges::has_position_key L, ranges::has_position_key R>
        constexpr bool operator()(L &&lhs, R &&rhs) const {
            return operator()(lhs.first, rhs.first);
        }
    };

    class gradient2 {
    public:
        using entry_t = std::pair<float, srgb>;
        using entry_container_t = std::vector<entry_t>;

        constexpr gradient2() = default;
        explicit gradient2(entry_container_t entries);
        inline gradient2(std::initializer_list<entry_t> entries);

        gradient2(std::initializer_list<srgb> entries);

        template <std::input_iterator Iterator>
        gradient2(Iterator begin, Iterator end);

        using const_iterator = entry_container_t::const_iterator;

        [[nodiscard]] inline auto begin() const;
        [[nodiscard]] inline auto end() const;
        [[nodiscard]] inline auto size() const;
        [[nodiscard]] inline bool empty() const;

        [[nodiscard]] inline decltype(auto) front() const;
        [[nodiscard]] inline decltype(auto) back() const;
        [[nodiscard]] inline decltype(auto) operator[](std::size_t i) const;

        [[nodiscard]] const_iterator lower_bound(float t) const;
        [[nodiscard]] const_iterator upper_bound(float t) const;

        [[nodiscard]] std::pair<const_iterator, const_iterator> low_upp_bounds(float t) const;

    private:
        template <std::input_iterator Iterator>
        [[nodiscard]] static entry_container_t ensure_position(Iterator begin, Iterator end);

        entry_container_t _entries{};
    };

    /** CPP */
    gradient2::gradient2(gradient2::entry_container_t entries) : _entries{std::move(entries)} {
        std::sort(std::begin(_entries), std::end(_entries), positioned_compare{});
    }

    gradient2::gradient2(std::initializer_list<srgb> entries) : gradient2{ensure_position(std::begin(entries), std::end(entries))} {}

    gradient2::gradient2(std::initializer_list<entry_t> entries) : gradient2{entry_container_t{entries}} {}

    /** HERE */

    template <std::input_iterator Iterator>
    gradient2::gradient2(Iterator begin, Iterator end) : gradient2{ensure_position(begin, end)} {}

    template <std::input_iterator Iterator>
    gradient2::entry_container_t gradient2::ensure_position(Iterator begin, Iterator end) {
        if constexpr (ranges::has_position_key<std::iter_value_t<Iterator>>) {
            return {begin, end};
        } else {
            auto kvps = std::ranges::subrange{begin, end} | ranges::unit_enumerate;
            return {std::begin(kvps), std::end(kvps)};
        }
    }

    auto gradient2::begin() const {
        return _entries.begin();
    }
    auto gradient2::end() const {
        return _entries.end();
    }
    auto gradient2::size() const {
        return _entries.size();
    }
    bool gradient2::empty() const {
        return _entries.empty();
    }

    decltype(auto) gradient2::front() const {
        return _entries.front();
    }
    decltype(auto) gradient2::back() const {
        return _entries.back();
    }
    decltype(auto) gradient2::operator[](std::size_t i) const {
        return _entries.at(i);
    }



    class gradient {
        std::vector<gradient_entry> _entries;

    public:
        using iterator = gradient_entry *;
        using const_iterator = fixed_gradient_entry const *;

        gradient() = default;
        explicit gradient(std::vector<gradient_entry> entries);
        explicit gradient(std::vector<fixed_gradient_entry> const &entries);
        explicit gradient(std::vector<srgb> const &colors);
        explicit gradient(std::vector<hsv> const &colors);

        [[nodiscard]] inline std::size_t size() const;

        [[nodiscard]] inline bool empty() const;

        [[nodiscard]] inline const_iterator begin() const;

        [[nodiscard]] inline const_iterator end() const;

        [[nodiscard]] inline iterator begin();

        [[nodiscard]] inline iterator end();

        [[nodiscard]] inline fixed_gradient_entry &front();

        [[nodiscard]] inline fixed_gradient_entry &back();

        [[nodiscard]] inline fixed_gradient_entry const &front() const;

        [[nodiscard]] inline fixed_gradient_entry const &back() const;

        inline std::pair<iterator, bool> emplace(fixed_gradient_entry entry);

        std::pair<iterator, bool> emplace(gradient_entry entry);

        [[nodiscard]] const_iterator lower_bound(float t) const;

        [[nodiscard]] const_iterator upper_bound(float t) const;

        void normalize();

        [[nodiscard]] inline fixed_gradient_entry const &operator[](std::size_t i) const;
        [[nodiscard]] inline fixed_gradient_entry &operator[](std::size_t i);

        [[nodiscard]] srgb sample(float t, blending_method method = blend_linear) const;
        [[nodiscard]] srgb sample(float progress, float offset = 0.f, float repeat = 1.f, blending_method method = blend_linear) const;

        template <class OutputIterator>
        OutputIterator fill(OutputIterator begin, OutputIterator end, float offset = 0.f, float repeat = 1.f, blending_method method = blend_linear) const;

        template <class OutputIterator>
        OutputIterator fill_n(OutputIterator out, std::size_t num, float offset = 0.f, float repeat = 1.f, blending_method method = blend_linear) const;

        [[nodiscard]] std::string to_string() const;
    };
}// namespace neo

namespace mlab {
    bin_data &operator<<(bin_data &o, neo::gradient const &g);
    bin_stream &operator>>(bin_stream &i, neo::gradient &g);
}// namespace mlab

namespace neo {

    template <class OutputIterator>
    OutputIterator gradient::fill(OutputIterator begin, OutputIterator end, float offset, float repeat, blending_method method) const {
        return fill_n(begin, std::distance(begin, end), offset, repeat, method);
    }

    template <class OutputIterator>
    OutputIterator gradient::fill_n(OutputIterator out, std::size_t num, float offset, float repeat, blending_method method) const {
        for (std::size_t i = 0; i < num; ++i) {
            *(out++) = sample(float(i) / float(num), offset, repeat, method);
        }
        return out;
    }

    bool gradient::empty() const {
        return _entries.empty();
    }

    std::size_t gradient::size() const {
        return _entries.size();
    }


    gradient::const_iterator gradient::begin() const {
        return _entries.data();
    }

    gradient::const_iterator gradient::end() const {
        return begin() + size();
    }

    gradient::iterator gradient::begin() {
        return _entries.data();
    }

    gradient::iterator gradient::end() {
        return begin() + size();
    }

    fixed_gradient_entry &gradient::front() {
        return _entries.front();
    }

    fixed_gradient_entry &gradient::back() {
        return _entries.back();
    }

    fixed_gradient_entry const &gradient::front() const {
        return _entries.front();
    }

    fixed_gradient_entry const &gradient::back() const {
        return _entries.back();
    }

    void fixed_gradient_entry::set_color(srgb color) {
        _color = color;
    }

    fixed_gradient_entry::fixed_gradient_entry(float t, srgb c) : _pos{t}, _color{c} {}

    float fixed_gradient_entry::position() const {
        return _pos;
    }

    srgb fixed_gradient_entry::color() const {
        return _color;
    }

    fixed_gradient_entry &fixed_gradient_entry::operator=(srgb c) {
        set_color(c);
        return *this;
    }

    void gradient_entry::set_position(float t) {
        _pos = t;
    }

    std::pair<gradient::iterator, bool> gradient::emplace(fixed_gradient_entry entry) {
        return emplace(gradient_entry{entry.position(), entry.color()});
    }

    fixed_gradient_entry const &gradient::operator[](std::size_t i) const {
        return _entries.at(i);
    }
    fixed_gradient_entry &gradient::operator[](std::size_t i) {
        return _entries.at(i);
    }
}// namespace neo
#endif//NEO_GRADIENT_HPP
