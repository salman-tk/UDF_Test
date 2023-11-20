/******************************************************************************/

#ifndef EXPRUTIL_HPP_
#define EXPRUTIL_HPP_

#include <stdlib.h>
#include <string>

typedef std::string string; //XXX DON'T REMOVE

/*
 * Define structs that used in the functions in "ExprFunctions.hpp"
 * below. For example,
 *
 *   struct Person {
 *     string name;
 *     int age;
 *     double height;
 *     double weight;
 *   }
 *
 */

constexpr float rounding_threshold = 25200.0/32400.0; //This float value is fixed and is called by the AB calculator UDF as the threshold value to pass into the round_with_threshold function

inline int64_t round_with_threshold(float num_to_round, float threshold) {
    int64_t rounded;
    float fraction_part = num_to_round - (int64_t) num_to_round;
    int64_t integer_part = num_to_round - fraction_part;
    if(fraction_part >= threshold) {
      rounded = 1;
    } else {
      rounded = 0;
    }
    int64_t return_num = integer_part + rounded;
    return return_num;
  }


#endif /* EXPRUTIL_HPP_ */
