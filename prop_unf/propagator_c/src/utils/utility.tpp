#pragma once
#include "utility.h"
// #include "logger.h"
#include <assert.h>
#include <map>

template <typename V>
void PerfectHash<V>::set(const int& key, const V& value){
    // Determine the index for positive or negative literals
    int i = (key > 0) ? key : (abs(key) + N);
    if(i < 0 or i >= values.size()){
        print("in method void PerfectHash<V>::set(const int& key, const V& value) ",i," out of bound, size: ",values.size());
        setExitCode(SETTINGS::ERROR_CODE);
    }
    values[i] = value;
}

template <typename V>
V PerfectHash<V>::get(int lit) const{
    // Determine the index for positive or negative literals
    int i = (lit > 0) ? lit : (abs(lit) + N);
    return values[i];
}


template <typename V>
V* get_perfect_hash_with_pointer(PerfectHash<V*>* ph, int lit){
    // Determine the index for positive or negative literals
    auto res = ph->get(lit);

    if(res == nullptr) {
        V* pointer = new V();
        ph->set(lit, pointer);
        return pointer ;
    }
    else return res ;
}


template <typename K, typename V>
void update_map_value_vector(std::unordered_map<K, std::vector<V>>& umap, K key, V value) {
    // Ensure the key exists and has an empty vector if not present
    if (umap.find(key) == umap.end()) {
        umap[key] = std::vector<V>();
    }

    // Append the values
    umap[key].push_back(value);

}


template <typename K, typename V>
std::vector<V> get_map_value_vector(std::unordered_map<K, std::vector<V>>& umap, K key) {
   
    if (umap.find(key) == umap.end()) {
        return std::vector<V>();
    }

    return umap[key];
}


template <typename K, typename V>
std::vector<V> get_map_value_vector(std::map<K, std::vector<V>>& umap, K key) {
   
    if (umap.find(key) == umap.end()) {
        return std::vector<V>();
    }
    return umap[key];
}


template< typename T>
void extend_vector(std::vector<T>& to_extend, const std::vector<T>& input, int i, int j){

    if (j == -1 || j > input.size()) j = input.size();
    if (i < 0) i = 0;

    for (; i < j; i++) to_extend.push_back(input[i]);
}

template <typename T>
std::string vector_to_string(const std::vector<T>& vec, std::string name){
    std::ostringstream oss;
    int n = vec.size() ;
    
    oss<<name<<"[";
    for (int i = 0; i < n-1; i++)  oss<<"'"<<vec[i]<<"'"<<"," ;
    if (n > 0) oss<<"'"<<vec[n-1]<<"'";

    oss<<"]";
    return oss.str();
}

template <typename T>
std::string vector_to_string_name(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::vector<T>& vec, std::string name){
    std::ostringstream oss;
    int n = vec.size() ;
    
    oss<<name<<"[";
    for (int i = 0; i < n-1; i++)  oss<<"'"<<getName(atomNames,vec[i])<<"'"<<"," ;
    if (n > 0) oss<<"'"<<getName(atomNames,vec[n-1])<<"'";

    oss<<"]";
    return oss.str();
}

template <typename T>
std::string array_to_string(const T* vec, size_t size,  std::string name){
    std::ostringstream oss;
    int n = size ;
    
    oss<<name<<"[";
    for (int i = 0; i < n-1; i++)  oss<<"'"<<vec[i]<<"'"<<"," ;
    if (n > 0) oss<<"'"<<vec[n-1]<<"'";

    oss<<"]";
    return oss.str();
}

template <typename T>
std::string unordered_set_to_string(const std::unordered_set<T>& uset, std::string name ){
    std::ostringstream oss;
    int n = uset.size() ;
    
    oss<<name<<"[";

    int i = 0 ;
    for(auto& e: uset){
        oss<<"'"<<e<<"'"<<( i < n - 1 ? "," : "") ;
        ++i ;
    }

    oss<<"]";
    return oss.str();
}

template <typename T1, typename T2>
std::string unordered_map_to_string(const std::unordered_map<T1,T2>& umap, std::string name ){
    std::ostringstream oss;
    int n = umap.size() ;
    
    oss<<name<<"[";

    int i = 0 ;
    for(const auto& [key, value]: umap){
        oss<<"'"<<key<<"':"<<"'"<<value<<"'"<<( i < n - 1 ? "," : "") ;
        ++i ;
    }

    oss<<"]";
    return oss.str();
}

template <typename T>
std::string unordered_set_to_string_name(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::unordered_set<T>& uset, std::string name ){
    std::ostringstream oss;
    int n = uset.size() ;
    
    oss<<name<<"[";

    int i = 0 ;
    for(auto& e: uset){
        oss<<"'"<<getName(atomNames,e)<<"'"<<( i < n - 1 ? "," : "") ;
        ++i ;
    }

    oss<<"]";
    return oss.str();
}


template<typename T>
void PerfectVector<T>::add(T lit){
    if(checker.get(lit) == true){
        return ; 
    }
    checker.set(lit, true);
    _data_.push_back(lit);
}

template<typename T>
bool PerfectVector<T>::has(T lit) const {
    return checker.get(lit) == true;
}

template<typename T>
void PerfectVector<T>::clear(){
    checker.clear();
    _data_.clear();
}