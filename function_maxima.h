#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include<map>
#include<set>
#include<utility>

using std::pair;
using std::map;
using std::set;

class InvalidArg : public std::exception{
   public:
    virtual const char* what() const throw()
  {
    return "invalid argument value";
  }
}InvArg;

template <typename A, typename V>
class FunctionMaxima
{
    private:
        map <A, V> function_map;
        set<pair<V, A> > maxima_set;
        bool equalV(V x, V y){
            return (!(x < y)) && (!(y < x));
        }
        bool equalA(A x, A y){
            return (!(x < y)) && (!(y < x));
        }
    public:
        FunctionMaxima() = default;
        //TODO konstruktor kopiujacy Konrad
        //TODO operator= Konrad 


        V const& value_at(A const& a) const {
            auto point = function_map.find(a);
            if(point == function_map.end()){
                throw InvArg;
            } else {
                return point->second;
            }

        }

        void set_value(A const& a, V const& v){
            A a2 = a;
            V v2 = v;
            pair<A, V> point (a2, v2);
            pair<V, A> max (v2, a2);
            auto f_end = function_map.end();
            auto f_begin = function_map.begin();
            auto m_end = maxima_set.end();
            if(function_map.size() == 0){
                auto inserted_point = function_map.insert(f_end, point);
                try{
                    maxima_set.insert(max);
                } catch(...){
                    function_map.erase(inserted_point);
                    throw;
                }
            }
            auto lower = function_map.lower_bound(a);
            auto upper = function_map.upper_bound(a);
            //new point is in the end of the current domain
            if(lower == f_end){
                lower--;
                //it a local maximum
                if(lower->second < v || equalV(lower->second, v)){
                    auto inserted_point = function_map.insert(f_end, point);
                    auto prev_max = maxima_set.find(std::make_pair(lower->second, lower->first));
                    try{
                        maxima_set.insert(max);
                        if(!equalV(lower->second, v) && m_end != prev_max)
                            maxima_set.erase(prev_max);
                        return;
                    } catch(...){
                        function_map.erase(inserted_point);
                        throw;
                    }
                }
                //it is not a local maximum
                else{
                    auto inserted_point = function_map.insert(f_end, point);
                    return;
                }
            }
            //new point is in the beginning of the current domain
            else if(a < lower->first && lower == f_begin){
                //it is a local maximum
                if(lower->second < v || equalV(lower->second, v)){
                    auto inserted_point = function_map.insert(f_end, point);
                    auto prev_max = maxima_set.find(std::make_pair(lower->second, lower->first));
                    try{
                        maxima_set.insert(max);
                        if(!equalV(lower->second, v) && m_end != prev_max)
                            maxima_set.erase(prev_max);
                        return;
                    } catch(...){
                        function_map.erase(inserted_point);
                        throw;
                    }
                }
                //it is not a local maximum
                else{
                    auto inserted_point = function_map.insert(f_end, point);
                    return;
                }
            }
            //point is already in the domain
            else if(!(a < lower->first)){
                
            
            }
        }

        void erase(A const& a){
            auto point = function_map.find(a);
            if(point == function_map.end()) 
                return;
            auto max = maxima_set.find(pair<V,A>(point->second, a));
            function_map.erase(point);
            if(max == maxima_set.end()) 
                return;
            maxima_set.erase(max);
        }


        //TODO typ point_type Konrad
};



#endif //FUNCTION_MAXIMA_H