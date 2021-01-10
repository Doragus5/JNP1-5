#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include<map>
#include<set>
#include<utility>
#include<memory>
#include<cassert>
#include<list>
#include<typeinfo>

// do usuniecia
#include <iostream>

class InvalidArg : public std::exception {
public:
    virtual const char *what() const throw() {
        return "invalid argument value";
    }
} InvArg;

template<typename A, typename V>
class FunctionMaxima {
private:
    using maxima_set_value_t = std::pair<std::shared_ptr<V>, std::shared_ptr<A>>;

    struct argumentComparator {
        using is_transparent = std::true_type;

        bool operator()(const std::shared_ptr<A> &lhs, const std::shared_ptr<A> &rhs) const {
            return *lhs < *rhs;
        }

        bool operator()(const std::shared_ptr<A> &lhs, const A &rhs) const {
            return *lhs < rhs;
        }

        bool operator()(const A &lhs, const std::shared_ptr<A> &rhs) const {
            return lhs < *rhs;
        }

        bool operator()(const A &lhs, const A &rhs) const {
            return lhs < rhs;
        }

    };

    struct maximaSetComparator {
        bool operator()(const maxima_set_value_t &lhs, const maxima_set_value_t &rhs) const {
            if (*(rhs.first) < *(lhs.first))
                return true;
            if (*(lhs.first) < *(rhs.first))
                return false;
            return *(lhs.second) < *(rhs.second);
        }
    };

    bool are_pointed_values_equal(std::shared_ptr<V> x, std::shared_ptr<V> y) {
        return !(*x < *y) && !(*y < *x);
    }

    using values_map_t = std::map<std::shared_ptr<A>, std::shared_ptr<V>, argumentComparator>;
    using maxima_set_t = std::set<std::pair<std::shared_ptr<V>, std::shared_ptr<A> >, maximaSetComparator>;
    values_map_t function_map;
    maxima_set_t maxima_set;

    typename maxima_set_t::iterator make_maximum_if_is(typename values_map_t::iterator point) {
        if (point == function_map.end() ||
            maxima_set.end() != maxima_set.find(std::make_pair(point->second, point->first)))
            return maxima_set.end();
        /* czy point != begin ? */
        auto prev = point;
        prev--;
        auto next = point;
        next++;
        bool lhs_maximum = (point == function_map.begin() || !(*(point->second) < *(prev->second)));
        bool rhs_maximum = (next == function_map.end() || !(*(point->second) < *(next->second)));
        if (lhs_maximum && rhs_maximum)
            return maxima_set.insert(maxima_set.begin(), std::make_pair(point->second, point->first));
        return maxima_set.end();
    }

    typename maxima_set_t::iterator make_previous_maximum_if_is(typename values_map_t::iterator point) {
        if (point == function_map.begin())
            return maxima_set.end();
        point--;
        return make_maximum_if_is(point);
    }

    typename maxima_set_t::iterator make_next_maximum_if_is(typename values_map_t::iterator point) {
        if (point == function_map.end() || (point++) == function_map.end())
            return maxima_set.end();
        return make_maximum_if_is(point);
    }

    void rollback(typename values_map_t::iterator p1, typename maxima_set_t::iterator m1,
                  typename maxima_set_t::iterator m2, typename maxima_set_t::iterator m3) noexcept {
        auto f_end = function_map.end();
        auto m_end = maxima_set.end();
        if (p1 != f_end)
            function_map.erase(p1);
        if (m1 != m_end)
            maxima_set.erase(m1);
        if (m3 != m_end)
            maxima_set.erase(m3);
        if (m2 != m_end)
            maxima_set.erase(m2);
    }

public:
    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima &toCopy) = default;

    FunctionMaxima &operator=(FunctionMaxima other) noexcept {
        function_map.swap(other.function_map);
        maxima_set.swap(other.maxima_set);
        return *this;
    }

    V const &value_at(A const &a) const {
        auto point = function_map.find(a);
        if (point == function_map.end()) {
            throw InvArg;
        } else {
            return *(point->second);
        }
    }

    void set_value(A const &a, V const &v) {
        auto a_ptr = std::make_shared<A>(a);
        auto v_ptr = std::make_shared<V>(v);
        auto point = make_pair(a_ptr, v_ptr);
        auto max = make_pair(v_ptr, a_ptr);
        auto f_end = function_map.end();
        auto f_begin = function_map.begin();
        auto m_end = maxima_set.end();
        if (function_map.size() == 0) {
            auto inserted_point = function_map.insert(f_end, point);
            try {
                maxima_set.insert(max);
            } catch (...) {
                function_map.erase(inserted_point);
                throw;
            }
            return;
        }
        auto lower = function_map.lower_bound(a_ptr);
        auto upper = function_map.upper_bound(a_ptr);
        auto next_max = m_end;
        auto prev_max = m_end;
        auto insert_max = m_end;
        auto inserted_prev_max = m_end;
        auto inserted_next_max = m_end;
        auto prev = lower;
        prev--;
        if (upper != f_end)
            next_max = maxima_set.find(make_pair(upper->second, upper->first));
        if (lower != f_begin)
            prev_max = maxima_set.find(make_pair(prev->second, prev->first));
        bool erase_prev = lower == f_begin || *(prev->second) < *v_ptr;
        bool erase_next = upper == f_end || *(upper->second) < *v_ptr;
        //point is already in the domain
        if (lower != f_end && !(*a_ptr < *(lower->first))) {
            if (are_pointed_values_equal(lower->second, v_ptr))
                return;
            auto already_max = maxima_set.find(make_pair(lower->second, lower->first));
            auto dummy = lower->second;
            try {
                lower->second = v_ptr;
                insert_max = make_maximum_if_is(lower);                //Theselines may throw an exception
                inserted_prev_max = make_previous_maximum_if_is(lower);//
                inserted_next_max = make_next_maximum_if_is(lower);    //
                if (m_end != prev_max && erase_prev)//These lines are noexcept
                    maxima_set.erase(prev_max);     //
                if (m_end != next_max && erase_next)//
                    maxima_set.erase(next_max);     //
                if (m_end != already_max)           //
                    maxima_set.erase(already_max);  //
            } catch (...) {
                lower->second = dummy;
                rollback(f_end, insert_max, inserted_next_max, inserted_prev_max);
                throw;
            }
        }
            //point is not in the domain
        else {
            if (upper != f_end)
                next_max = maxima_set.find(make_pair(upper->second, upper->first));
            auto prev = lower;
            prev--;
            if (lower != f_begin)
                prev_max = maxima_set.find(make_pair(prev->second, prev->first));
            auto inserted_point = function_map.insert(f_end, point);
            try {
                insert_max = make_maximum_if_is(inserted_point);                //Theselines may throw an exception
                inserted_prev_max = make_previous_maximum_if_is(inserted_point);//
                inserted_next_max = make_next_maximum_if_is(inserted_point);    //
                if (m_end != prev_max && erase_prev)//These lines are noexcept
                    maxima_set.erase(prev_max);     //
                if (m_end != next_max && erase_next)//
                    maxima_set.erase(next_max);     //
            } catch (...) {
                rollback(inserted_point, insert_max, inserted_next_max, inserted_prev_max);
                throw;
            }
        }
    }

    void erase(A const &a) {
        auto a_ptr = std::make_shared<A>(a);
        auto point = function_map.find(a);
        if (point == function_map.end())
            return;
        auto prev = point;
        prev--;
        auto next = point;
        next++;
        auto f_end = function_map.end();
        auto m_end = maxima_set.end();
        auto prev_max = maxima_set.end();
        auto next_max = maxima_set.end();
        auto inserted_prev_max = maxima_set.end();
        auto inserted_next_max = maxima_set.end();
        bool erase_prev_max = false;
        bool erase_next_max = false;
        auto dummy = point->second;

        if (point != function_map.begin() && next != f_end) {
            prev_max = maxima_set.find(make_pair(prev->second, prev->first));
            next_max = maxima_set.find(make_pair(next->second, next->first));
            erase_next_max = *(next->second) < *(prev->second);
            erase_prev_max = *(prev->second) < *(next->second);
        }
        auto max = maxima_set.find(make_pair(point->second, a_ptr));
        try {
            if (next != function_map.end())//noexcept
                point->second = next->second;//noexcept modification
            if (point != function_map.begin())//noexcept
                point->second = prev->second;//noexcept modification
            if (point != function_map.begin() && next != function_map.end() &&
                *(prev->second) < *(next->second))//may throw comparator exception
                point->second = next->second;//noexcept modification
            inserted_prev_max = make_previous_maximum_if_is(point);//first modification that can throw an exception
            inserted_next_max = make_next_maximum_if_is(point);//last modification that can throw an exception
            function_map.erase(point);  //noexcept modification
            if (max != maxima_set.end())//noexcept
                maxima_set.erase(max);  //noexcept modification
            if (erase_next_max && next_max != maxima_set.end())//noexcept
                maxima_set.erase(next_max);//noexcept modification
            if (erase_prev_max && prev_max != maxima_set.end())//noexcept
                maxima_set.erase(prev_max);//noexcept modification
        } catch (...) {
            point->second = dummy;
            rollback(f_end, m_end, inserted_next_max, inserted_prev_max);
            throw;
        }
    }

    using size_type = size_t;

    size_type size() const {
        return function_map.size();
    }

    class point_type {
    private:
        friend class FunctionMaxima;

        std::shared_ptr<A> argument_pointer;
        std::shared_ptr<V> value_pointer;

        point_type(const std::shared_ptr<A> argument_ptr, const std::shared_ptr<V> value_ptr) {
            argument_pointer = argument_ptr;
            value_pointer = value_ptr;
        }

    public:
        point_type() = delete;

        A const &arg() const {
            return *argument_pointer;
        }

        V const &value() const {
            return *value_pointer;
        }
    };

private:

    class shared_ptr_wrapper {
    private:
        std::shared_ptr<point_type> stored_pointer;
    public:
        shared_ptr_wrapper(std::shared_ptr<point_type> pointer) : stored_pointer(pointer) {}

        shared_ptr_wrapper() : stored_pointer(nullptr) {}

        const std::shared_ptr<point_type> get_pointed_value() {
            return stored_pointer;
        }
    };

public:
    class iterator {
    private:
        using self_type = iterator;
        using wrapped_iterator_t = typename values_map_t::const_iterator;
        using wrapped_value_t = point_type;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = point_type;
        using pointer = value_type *;
        using reference = value_type &;
        using difference_type = std::ptrdiff_t;

        iterator() = delete;

        const point_type *operator->() {
            point_type_to_return = shared_ptr_wrapper(
                    std::make_shared<point_type>(point_type(wrapped_iterator->first, wrapped_iterator->second)));
            return point_type_to_return.get_pointed_value().get();
        }

        point_type const &operator*() {
            point_type_to_return = shared_ptr_wrapper(
                    std::make_shared<point_type>(point_type(wrapped_iterator->first, wrapped_iterator->second)));
            return *(point_type_to_return.get_pointed_value());
        }

        self_type operator++() {
            self_type i = *this;
            wrapped_iterator++;
            return i;
        }

        self_type operator++(int) {
            wrapped_iterator++;
            return *this;
        }

        self_type operator--() {
            self_type i = *this;
            wrapped_iterator--;
            return i;
        }

        self_type operator--(int) {
            wrapped_iterator--;
            return *this;
        }

        self_type &operator=(const self_type &other) = default;

        bool operator==(const self_type &rhs) const { return wrapped_iterator == rhs.wrapped_iterator; }

        bool operator!=(const self_type &rhs) const { return wrapped_iterator != rhs.wrapped_iterator; }

    private:
        friend class FunctionMaxima;

        wrapped_iterator_t wrapped_iterator;
        shared_ptr_wrapper point_type_to_return;

        iterator(wrapped_iterator_t iterator_to_wrap) : wrapped_iterator(iterator_to_wrap) {
            point_type_to_return = shared_ptr_wrapper();
        }
    };

    iterator begin() const {
        return iterator(function_map.begin());
    }

    iterator end() const {
        return iterator(function_map.end());
    }

    iterator find(A const &a) const {
        return iterator(function_map.find(a));
    }

    class mx_iterator {
    private:
        using self_type = mx_iterator;
        using wrapped_iterator_t = typename maxima_set_t::const_iterator;
        using wrapped_value_t = point_type;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = self_type *;
        using reference = self_type &;
        using value_type = typename maxima_set_t::const_iterator;

        mx_iterator() = delete;

        const point_type *operator->() {
            point_type_to_return = shared_ptr_wrapper(
                    std::make_shared<point_type>(point_type(wrapped_iterator->second, wrapped_iterator->first)));
            return point_type_to_return.get_pointed_value().get();
        }

        const point_type &operator*() {
            point_type_to_return = shared_ptr_wrapper(
                    std::make_shared<point_type>(point_type(wrapped_iterator->second, wrapped_iterator->first)));
            return *(point_type_to_return.get_pointed_value());
        }

        self_type operator++() {
            self_type i = *this;
            wrapped_iterator++;
            return i;
        }

        self_type operator++(int) {
            wrapped_iterator++;
            return *this;
        }

        self_type operator--() {
            self_type i = *this;
            wrapped_iterator--;
            return i;
        }

        self_type operator--(int) {
            wrapped_iterator--;
            return *this;
        }

        self_type &operator=(const self_type &other) = default;

        bool operator==(const self_type &rhs) const { return wrapped_iterator == rhs.wrapped_iterator; }

        bool operator!=(const self_type &rhs) const { return wrapped_iterator != rhs.wrapped_iterator; }

    private:
        friend class FunctionMaxima;

        wrapped_iterator_t wrapped_iterator;
        shared_ptr_wrapper point_type_to_return;

        mx_iterator(wrapped_iterator_t iterator_to_wrap) : wrapped_iterator(iterator_to_wrap) {
            point_type_to_return = shared_ptr_wrapper();
        }
    };

    mx_iterator mx_begin() const {
        return mx_iterator(maxima_set.begin());
    }

    mx_iterator mx_end() const {
        return mx_iterator(maxima_set.end());
    }

};

#endif //FUNCTION_MAXIMA_H