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
            
            if(lower == f_end){
                lower--;
                if(lower->first < a){
                    
                }
            }
            else if(!(lower->first < a) && !(a < lower->first)){
            
            }
        }

        void erase(A const& a){
            auto point = function_map.find(a);
            if(point == function_map.end()) return;
            auto max = maxima_set.find(pair<V,A>(point->second, a));
            function_map.erase(point);
            if(max == maxima_set.end()) return;
            maxima_set.erase(max);
        }

        
        //TODO typ point_type Konrad
};



#endif //FUNCTION_MAXIMA_H