/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  DataMap.h
 *  @brief DataMaps track arbitrary data by name (slow) or id (faster).
 *
 */

#ifndef EMP_DATA_MAP_H
#define EMP_DATA_MAP_H

#include <string>
#include <tuple>
#include <typeinfo>
#include <unordered_map>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  template <typename... Ts>
  class DataMap {
  public:
    using data_blob_t = std::tuple<emp::vector<Ts>...>;     ///< Storage type for mapped data.

  private:
    data_blob_t default_blob;                               ///< Default values for data.
    std::unordered_map<std::string, size_t> id_map;         ///< Lookup vector positions by name.
    std::unordered_map<std::string, std::string> type_map;  ///< Lookup value types by name.

  public:
    DataMap() : default_blob(), id_map(), type_map() { ; }
    DataMap(const DataMap &) = default;
    DataMap(DataMap &&) = default;
    ~DataMap() { ; }

    DataMap & operator=(const DataMap &) = default;
    DataMap & operator=(DataMap &&) = default;

    /// Add a new variable with a specified type, name and value.
    template <typename T>
    void Add(const std::string & name, T value) {
      emp_assert(!Has(id_map, name), name);               // Make sure this doesn't already exist.
      auto & v = std::get<emp::vector<T>>(default_blob);  // Retrieve vector of the correct type.
      size_t pos = v.size();                              // Determine position of new entry.
      v.push_back(value);                                 // Add the new value to the vector.
      id_map[name] = pos;                                 // Store the position in the id map.
      type_map[name] = typeid(T).name();                  // Store the type of this entry.
    }

    /// Retrieve a default variable by its type and unique id.
    template <typename T>
    T & GetDefault(size_t id) {
      return std::get<emp::vector<T>>(default_blob)[id];  // Index into vector of correct type.
    }

    /// Retrieve a variable from a blob by its type and unique id.
    template <typename T>
    T & Get(data_blob_t & blob, size_t id) {
      return std::get<emp::vector<T>>(blob)[id];  // Index into vector of correct type.
    }

    /// Retrieve a constant default variable by its type and unique id.
    template <typename T>
    const T & Get(size_t id) const {
      return std::get<emp::vector<T>>(default_blob)[id];  // Index into vector of correct type.
    }

    /// Retrieve a constant variable from a blob by its type and unique id.
    template <typename T>
    const T & Get(const data_blob_t & blob, size_t id) const {
      return std::get<emp::vector<T>>(blob)[id];  // Index into vector of correct type.
    }

    /// Retrieve a default variable by its type and unique name.
    template <typename T>
    T & GetDefault(const std::string & name) {
      emp_assert(type_map[name] == typeid(T).name());
      return GetDefault<T>(id_map[name]);
    }

    /// Retrieve a const default variable by its type and unique name.
    template <typename T>
    const T & GetDefault(const std::string & name) const {
      emp_assert(Has(id_map, name), name);                         // Make sure this name exists
      emp_assert(type_map.find(name)->second == typeid(T).name()); // Ensure correct type is used.
      return GetDefault<T>( id_map.find(name)->second );
    }


    /// Retrieve a variable from a data blob by its type and unique name.
    template <typename T>
    T & Get(data_blob_t & blob, const std::string & name) {
      emp_assert(type_map[name] == typeid(T).name());
      return Get<T>(blob, id_map[name]);
    }

    /// Retrieve a variable from a const data blob by its type and unique name.
    template <typename T>
    const T & Get(const data_blob_t & blob, const std::string & name) const {
      emp_assert(Has(id_map, name), name);                         // Make sure this name exists
      emp_assert(type_map.find(name)->second == typeid(T).name()); // Ensure correct type is used.
      return Get<T>( blob, id_map.find(name)->second );
    }


    size_t GetID(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return id_map.find(name)->second;
    }

    const std::string & GetType(const std::string & name) const {
      emp_assert(Has(id_map, name), name);
      return type_map.find(name)->second;
    }
  };

}

#endif
