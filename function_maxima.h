#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include<map>
#include<set>
#include<utility>

using std::pair;
using std::map;
using std::set;


template <typename A, typename V>
class FunctionMaxima
{
    private:
    map <A, V> function_map;
    set<pair<V, A> > maxima_set;
    //TODO mapa maximow po wartosciach
    public:
    FunctionMaxima() = default;
    //TODO konstruktor kopiujacy Konrad
    //TODO operator= Konrad 
    V const& value_at(A const& a) const {
        //TODO
    }
    void set_value(A const& a, V const& v){
        
        //function_map.insert_or_assign();
    }
    void erase(A const& a){
        //TODO
    }
    //TODO typ point_type Konrad
};



#endif //FUNCTION_MAXIMA_H