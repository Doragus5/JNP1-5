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
    struct argumentComparator {
        bool operator()(const std::shared_ptr<A> &lhs, const std::shared_ptr<A> &rhs) const {
            return *lhs < *rhs;
        }
    };

    struct pairComparator {
        bool operator()(const std::pair<std::shared_ptr<V>, std::shared_ptr<A> > &lhs,
                        const std::pair<std::shared_ptr<V>, std::shared_ptr<A> > &rhs) const {
            if (*(lhs.first) < *(rhs.first))
                return true;
            if (*(rhs.first) < *(lhs.first))
                return false;
            if (*(lhs.second) < *(rhs.second))
                return true;
            return false;
        }
    };

    bool equalV(std::shared_ptr<V> x, std::shared_ptr<V> y) {
        return !(*x < *y) && !(*y < *x);
    }


    using values_map_t = std::map<std::shared_ptr<A>, std::shared_ptr<V>, argumentComparator>;
    using maxima_set_t = std::set<std::pair<std::shared_ptr<V>, std::shared_ptr<A> >, pairComparator>;
    values_map_t function_map;
    maxima_set_t maxima_set;

    typename maxima_set_t::iterator make_maximum_if_is(typename values_map_t::iterator point){
        if(point == function_map.end() || maxima_set.end() != maxima_set.find(std::make_pair(point->second, point->first)))
            return maxima_set.end();
        auto prev = point;
        prev--;
        auto next = point;
        next++;
        bool lhs_maximum = (point == function_map.begin() || !(*(point->second) < *(prev->second)));
        bool rhs_maximum = (next == function_map.end() || !(*(point->second) < *(next->second)));
        if(lhs_maximum && rhs_maximum)
            return maxima_set.insert(maxima_set.begin(), std::make_pair(point->second, point->first));
        return maxima_set.end();
    }

    typename maxima_set_t::iterator make_previous_maximum_if_is(typename values_map_t::iterator point){
        if(point == function_map.begin())
            return maxima_set.end();
        point--;
        return make_maximum_if_is(point);
    }

    typename maxima_set_t::iterator make_next_maximum_if_is(typename values_map_t::iterator point){
        if(point == function_map.end() || (point++) == function_map.end())
            return maxima_set.end();
        return make_maximum_if_is(point);
    }

public:
    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima &toCopy) = default;

    FunctionMaxima &operator=(const FunctionMaxima &other) {
        function_map = other.function_map;
        maxima_set = other.maxima_set;
        return *this;
    }

    V const &value_at(A const &a) const {
        std::shared_ptr<A> shr_ptr = std::make_shared<A>(a);
        auto point = function_map.find(shr_ptr);
        if (point == function_map.end()) {
            throw InvArg;
        } else {
            return *(point->second);
        }
    }

    void set_value(A const &a, V const &v) {
        std::shared_ptr<A> a_ptr = std::make_shared<A>(a);
        std::shared_ptr<V> v_ptr = std::make_shared<V>(v);
        std::pair<std::shared_ptr<A>, std::shared_ptr<V> > point(a_ptr, v_ptr);
        std::pair<std::shared_ptr<V>, std::shared_ptr<A> > max(v_ptr, a_ptr);
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
            if (equalV(lower->second, v_ptr))
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
                if(insert_max != m_end)
                    maxima_set.erase(insert_max);
                if(inserted_next_max != m_end)
                    maxima_set.erase(inserted_next_max);
                if(inserted_prev_max != m_end)
                    maxima_set.erase(inserted_prev_max);
                throw;
            }
        }
            //point is not in the domain
        else {
            if(upper != f_end)
                next_max = maxima_set.find(make_pair(upper->second, upper->first));
            auto prev = lower;
            prev--;
            if(lower != f_begin)
                prev_max = maxima_set.find(make_pair(prev->second, prev->first));
            auto inserted_point = function_map.insert(f_end, point);
            try {
                insert_max = make_maximum_if_is(lower);                //Theselines may throw an exception
                inserted_prev_max = make_previous_maximum_if_is(lower);//
                inserted_next_max = make_next_maximum_if_is(lower);    //
                if (m_end != prev_max && erase_prev)//These lines are noexcept
                    maxima_set.erase(prev_max);     //
                if (m_end != next_max && erase_next)//
                    maxima_set.erase(next_max);     //
            } catch (...) {
                function_map.erase(inserted_point);
                if(insert_max != m_end)
                    maxima_set.erase(insert_max);
                if(inserted_next_max != m_end)
                    maxima_set.erase(inserted_next_max);
                if(inserted_prev_max != m_end)
                    maxima_set.erase(inserted_prev_max);
                throw;
            }
        }
    }

    void erase(A const &a) {
        std::shared_ptr<A> a_ptr = std::make_shared<A>(a);
        auto point = function_map.find(a_ptr);
        auto prev = point;
        prev--;
        auto next = point;
        next++;
        auto prev_max = maxima_set.end();
        auto next_max = maxima_set.end();
        if (point == function_map.end())
            return;
        auto max = maxima_set.find(make_pair(point->second, a_ptr));
        try{
            prev_max = make_previous_maximum_if_is(point);//first modification that can throw an exception
            next_max = make_next_maximum_if_is(point);//last modification that can throw an exception
            function_map.erase(point);  //noexcept
            if (max == maxima_set.end())//noexcept
                return;                 //noexcept
            maxima_set.erase(max);      //noexcept
        } catch (...) {
            if(!(prev_max == maxima_set.end()))
                maxima_set.erase(prev_max);
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

        std::shared_ptr<A> argumentPointer_;
        std::shared_ptr<V> valuePointer_;

        point_type(const std::shared_ptr<A> argumentPointer, const std::shared_ptr<V> valuePointer) {
            argumentPointer_ = argumentPointer;
            valuePointer_ = valuePointer;
        }

    public:
        point_type() = delete;

        A const &arg() const {
            assert(argumentPointer_ != nullptr);
            return *argumentPointer_;
        }

        V const &value() const {
            return *valuePointer_;
        }
    };

    // iterator
    // from: https://gist.github.com/jeetsukumaran/307264

    class iterator
    {
    public:
        using self_type = iterator;
        using wrapped_iterator_t = typename values_map_t::const_iterator;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        iterator(wrapped_iterator_t it_) : it(it_) { }
        self_type operator++() { self_type i = *this; it++; return i; }
        self_type operator++(int) { it++; return *this; }
        self_type operator--() {self_type i = *this; it--; return i; }
        self_type operator--(int) { it--; return *this; }
        self_type operator+(const difference_type& movement) {
            auto oldPtr = it;
            it += movement;
            auto temp(*this);
            it = oldPtr;
            return temp;
        }
        self_type operator-(const difference_type& movement) {
            return operator+(-movement);
        }
        point_type const& operator*() {
            std::shared_ptr<point_type> result = std::make_shared<point_type>(point_type(it->first, it->second));
            return *result;
        }
        bool operator==(const self_type& rhs) const { return it == rhs.it; }
        bool operator!=(const self_type& rhs) const { return it != rhs.it; }
    private:
        wrapped_iterator_t it;
    };

    iterator begin() const {
        return iterator(function_map.begin());
    }

    iterator end() const {
        return  iterator(function_map.end());
    }

    iterator find(A const& a) const {
        return iterator(function_map.find(std::make_shared<A>(a)));
    }

    class mx_iterator
    {
    public:
        using self_type = mx_iterator;
        using wrapped_iterator_t = typename maxima_set_t::const_iterator;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = self_type*;
        using reference = self_type&;
        using value_type = typename maxima_set_t::const_iterator;
        mx_iterator(wrapped_iterator_t it_) : it(it_) { }
        self_type operator++() { self_type i = *this; it++; return i; }
        self_type operator++(int) { it++; return *this; }
        self_type operator--() {self_type i = *this; it--; return i; }
        self_type operator--(int) { it--; return *this; }
        self_type operator+(const difference_type& movement) {
            auto oldPtr = it;
            it += movement;
            auto temp(*this);
            it = oldPtr;
            return temp;
        }
        self_type operator-(const difference_type& movement) {
            return operator+(-movement);
        }
        self_type&  operator=(const self_type& rawIterator) {
        };
        point_type const& operator*() {
            std::shared_ptr<point_type> result = std::make_shared<point_type>(point_type(it->second, it->first));
            return *result;
        }
        bool operator==(const self_type& rhs) const { return it == rhs.it; }
        bool operator!=(const self_type& rhs) const { return it != rhs.it; }
    private:
        wrapped_iterator_t it;
    };

    mx_iterator mx_begin() const {
        return mx_iterator(maxima_set.begin());
    }

    mx_iterator mx_end() const {
        return mx_iterator(maxima_set.end());
    }

};

#endif //FUNCTION_MAXIMA_H