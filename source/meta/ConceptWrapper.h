/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ConceptWrapper.h
 *  @brief A template wrapper that will either enforce functionality or provide default functions.
 *
 *  Starting in future versions of C++, a concept is a set of requirements for a class to be used
 *  in a template.  This wrapper around a class can either REQUIRE functions to be present in the
 *  internal class, or provide DEFAULT functionality when functions are missing.
 * 
 *  Use the EMP_BUILD_CONCEPT macro to create a new concept wrapper.  Provide it with the wrapper
 *  name, and all of the rules.  The allowable rule types are:
 * 
 *  REQUIRED_FUN ( FUNCTION_NAME, ERROR_MESSAGE )
 *    Setup a function that is required to be present in the wrapped class.  If it does not
 *    exist there, throw the provided error.
 * 
 *  OPTIONAL_FUN ( FUNCTION_NAME, DEFAULT_ACTION, RETURN_TYPE, ARG_TYPES... )
 *    Setup a function.  If it exists, call the version in the wrapped class.  If it does not
 *    exist, return the default instead.  The function signature is needed as part of the 
 *    automated testing as to whether the function exists.
 * 
 *  PRIVATE ( CODE )
 *    All code provided will appear in the private portion of the wrapper.
 * 
 *  PROTECTED ( CODE )
 *    All code provided will appear in the protected portion of the wrapper.
 *  
 *  PUBLIC ( CODE )
 *    All code provided will appear in the public portion of the wrapper.
 * 
 * 
 *  @note: Requires C++-17 to function properly!
 */

#ifndef EMP_CONCEPT_WRAPPER_H
#define EMP_CONCEPT_WRAPPER_H

#include <string>
#include <utility>

#include "meta.h"

// Macro to dynamically call a function either in the wrapped type (if it exists)
// or return the default provided (otherwise).  The first two arguments are the
// function name and its default return.  The remaining arguments in the ... must
// be the return type (required) and all argument types (if any exist)

#define MABE_GENOME_TEST_FUN_impl(NAME, DEFAULT, USE_ARGS, RETURN_T, ...)     \
    /* Determine the return type if we call this function in the base class.  \
       It should be undefined if the member functon does not exist!        */ \
    template <typename T>                                                     \
    using return_t_ ## NAME =                                                 \
      EMP_IF( USE_ARGS,                                                       \
        decltype( std::declval<T>().NAME() );,                                \
        decltype( std::declval<T>().NAME(MABE_TYPES_TO_VALS(__VA_ARGS__)) );  \
      )                                                                       \
    /* Test whether function exists, based on SFINAE in using return type. */ \
    static constexpr bool HasFun_ ## NAME() {                                 \
      return emp::test_type<return_t_ ## NAME, WRAPPED_T>();                  \
    }                                                                         \
    /* Call appropriate version of the function.  First determine if there    \
       is a non-void return type (i.e., do we return th result?) then         \
       check if we can call the function in the wrapped class (if it          \
       exists) or should we call/return the default (otherwise).           */ \
    template <typename... Ts>                                                 \
    RETURN_T NAME(Ts &&... args) {                                            \
      EMP_IF( MABE_TEST_IF_VOID(RETURN_T),                                    \
        {                                                                     \
          if constexpr (HasFun_ ## NAME()) {                                  \
            WRAPPED_T::NAME( std::forward<Ts>(args)... );                     \
          }                                                                   \
          else { DEFAULT; }                                                   \
        },                                                                    \
        {                                                                     \
          if constexpr (HasFun_ ## NAME()) {                                  \
            return WRAPPED_T::NAME( std::forward<Ts>(args)... );              \
          }                                                                   \
          else { return DEFAULT; }                                            \
        }                                                                     \
      )                                                                       \
    }

#define MABE_GENOME_TEST_FUN(NAME, DEFAULT, ...)                                              \
  MABE_GENOME_TEST_FUN_impl(NAME, DEFAULT,                                                    \
                            EMP_EQU( EMP_COUNT_ARGS(__VA_ARGS__), 1), /* Are there args? */   \
                            EMP_GET_ARG(1, __VA_ARGS__),              /* Return type */       \
                            EMP_POP_ARG(__VA_ARGS__) )                /* Arg types */


#define EMP_BUILD_CONCEPT( NAME, ... )

namespace emp {

  template <typename WRAPPED_T>
  class ConceptWrapper : public WRAPPED_T {
  private:
    using this_t = ConceptWrapper<WRAPPED_T>;

    static emp::Config empty_config;

  public:
    // Create a set of functions to determine if a memeber exists on WRAPPED_T in the form of
    // constexpr bool has_fun_X()
    MABE_GENOME_TEST_FUN(GetClassName, "NoName", std::string);
    MABE_GENOME_TEST_FUN(GetConfig, empty_config, emp::Config);
    MABE_GENOME_TEST_FUN(Randomize, false, bool, emp::Random &);
    MABE_GENOME_TEST_FUN(Print, false, bool);
    MABE_GENOME_TEST_FUN(OnBeforeRepro, , void);              // Genome about to be reproduced.
    MABE_GENOME_TEST_FUN(OnOffspringReady, , void, this_t &); // Genome offspring; arg is parent genome
    MABE_GENOME_TEST_FUN(OnInjectReady, , void);              // Genome about to be injected.
    MABE_GENOME_TEST_FUN(OnBeforePlacement, , void);          // Genome about to be placed
    MABE_GENOME_TEST_FUN(OnPlacement, , void);                // Genome just placed.
    MABE_GENOME_TEST_FUN(OnOrgDeath, , void);                 // Genome about to die.
  };

}

#endif
