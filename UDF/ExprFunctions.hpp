/******************************************************************************
 * Copyright (c) 2015-2016, TigerGraph Inc.
 * All rights reserved.
 * Project: TigerGraph Query Language
 * udf.hpp: a library of user defined functions used in queries.
 *
 * - This library should only define functions that will be used in
 *   TigerGraph Query scripts. Other logics, such as structs and helper
 *   functions that will not be directly called in the GQuery scripts,
 *   must be put into "ExprUtil.hpp" under the same directory where
 *   this file is located.
 *
 * - Supported type of return value and parameters
 *     - int64_t
 *     - float
 *     - double
 *     - bool
 *     - string (don't use std::string)
 *     - accumulators
 *
 * - Function names are case sensitive, unique, and can't be conflict with
 *   built-in math functions and reserve keywords.
 *
 * - Please don't remove necessary codes in this file
 *
 * - A backup of this file can be retrieved at
 *     <tigergraph_root_path>/dev_<backup_time>/gdk/gsql/src/QueryUdf/ExprFunctions.hpp
 *   after upgrading the system.
 *
 ******************************************************************************/

#ifndef EXPRFUNCTIONS_HPP_
#define EXPRFUNCTIONS_HPP_

#include <stdlib.h>
#include <string>
#include <math.h>

/**     XXX Warning!! Put self-defined struct in ExprUtil.hpp **
 *  No user defined struct, helper functions (that will not be directly called
 *  in the GQuery scripts) etc. are allowed in this file. This file only
 *  contains user-defined expression function's signature and body.
 *  Please put user defined structs, helper functions etc. in ExprUtil.hpp
 */
#include "ExprUtil.hpp"

namespace UDIMPL {
  typedef std::string string; //XXX DON'T REMOVE

  /****** BIULT-IN FUNCTIONS **************/
  /****** XXX DON'T REMOVE ****************/
  inline int64_t str_to_int (string str) {
    return atoll(str.c_str());
  }

  inline int64_t float_to_int (float val) {
    return (int64_t) val;
  }

//TODO: Might consider migrating to uint64 once validation is complete?
/*
Tuple should be defined in the calling GSQL as follows:
TYPEDEF TUPLE <valid BOOL, num_breaks_10_h INT, num_breaks_30_m INT, on_duty_time INT, on_duty_drive_time INT, arrival_clock_14_hr INT, departure_clock_14_hr INT, arrival_clock_11_hr INT, arrival_clock_8_hr INT, departure_clock_8_hr INT, arrival_time INT, departure_time_new INT, dwell_after_break INT, driven_miles INT> Assignment_Tuple2;
*/

inline int64_t round_threshold(float num_to_round, float threshold) {
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

inline int64_t multiply(int64_t a, int64_t b) {
    int64_t result;
    result = a*b;
    return result;
  }

inline Assignment_Tuple2 Driver_PTA(
    int64_t clock_8_h,
    int64_t clock_11_h,
    int64_t clock_14_h,
    int64_t current_time,
    int64_t duty_status_last_changed_timeafaf) {
	
	
	  if ( (clock_8_h == 0) && (clock_11_h == 0) && (clock_14_h == 0) )
	  {
		return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, 0, 0, current_time, current_time, 0, 0, "");
	  }	
		
	  const int64_t remaining_8_h = 28800 - clock_8_h;
      const int64_t remaining_11_h = 39600 - clock_11_h;
      const int64_t remaining_14_h = 50400 - clock_14_h;
	  
	  if ( (remaining_14_h > 5400) && (remaining_11_h > 5400) && (remaining_8_h > 5400) )
	  {
		return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, 0, 0, (duty_status_last_changed_time + 1800), (duty_status_last_changed_time + 1800), 0, 0, "");
	  }

	  bool break_30min, break_10hr;

	  if (remaining_8_h <= 0) 
	  {
		break_30min = true;
	  }	
	  if (remaining_11_h <= 0 || remaining_14_h <= 0)
	  {
		break_10hr = true;
	  }
	  
	  if (break_30min == true)
	  {
		if (break_10hr == true) 
	    {
		  return Assignment_Tuple2(true, 0, 0, 0, 0, 0, 0, 0, 0, 0, (duty_status_last_changed_time + 36000), (duty_status_last_changed_time + 36000), 0, 0, "");
		}
		else
		{
		  return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, 0, 0, (duty_status_last_changed_time + 1800), (duty_status_last_changed_time + 1800), 0, 0, "");
		}
	  }

	  if (break_30min == false && break_10hr == false) 
	  {	  
		int64_t first_violated_clock = 14;
		if (remaining_8_h < remaining_11_h)
		{
		  if (remaining_8_h < remaining_14_h)
		  {
		  first_violated_clock = 8;
		  }
		}
		else if (remaining_11_h <= remaining_14_h)
		{
		  first_violated_clock = 11;
		}
		
		if ( (first_violated_clock == 14 && remaining_14_h <= 5400) || (first_violated_clock == 11 && remaining_11_h <= 5400) ) 
		{
		  return Assignment_Tuple2(true, 0, 0, 0, 0, 0, 0, 0, 0, 0, (duty_status_last_changed_time + 36000), (duty_status_last_changed_time + 36000), 0, 0, "");
		}
		else if (first_violated_clock == 8) 
		{
		  if (remaining_14_h <= 5400 || remaining_11_h <= 5400)
		  {
			return Assignment_Tuple2(true, 0, 0, 0, 0, 0, 0, 0, 0, 0, (duty_status_last_changed_time + 36000), (duty_status_last_changed_time + 36000), 0, 0, "");	
          }
		  else if (remaining_8_h <= 5400) 
		  {
			return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, 0, 0, (duty_status_last_changed_time + 1800), (duty_status_last_changed_time + 1800), 0, 0, "");
		  }
		}
	  }
    }

  inline Assignment_Tuple2 Assignment_Builder_Calculator(
    int64_t clock_8_h,
    int64_t clock_11_h,
    int64_t clock_14_h,
    int64_t departure_time,
    int64_t load_time,
    int64_t journey_time,
    float distance,
    int64_t must_end_by,
    int64_t must_start_by,
    string source_type,
    string target_type,
    bool break_permitted,
	bool journey_verification_mode,
	bool dispatched,
	bool facility,
	int64_t facility_open,
	int64_t facility_close,
	int64_t maximum_dwell) {
		
		/*	
		 Declare the global User Defined Tuple (UDT)
		 In your GSQL shell execute:
		 TYPEDEF TUPLE <valid BOOL, num_breaks_10_h INT, num_breaks_30_m INT, on_duty_time INT, on_duty_drive_time INT, arrival_clock_14_hr INT, departure_clock_14_hr INT, arrival_clock_11_hr INT, arrival_clock_8_hr INT, departure_clock_8_hr INT, arrival_time INT, departure_time_new INT, dwell_after_break INT, driven_miles INT> Assignment_Tuple2;
		*/

      /*===================*/
      /* CRITICAL TODOs
      - Define Assignment_Tuple2 structure
      */
      /*===================*/

      /*===================*/
      // TODO: define this and other constants outside the scope of this function (in ExprUtil)
      /*===================*/

	  int64_t must_start_by_new = must_start_by;	  
	  int64_t must_end_by_new = must_end_by;

      int64_t arrival_time = 0;

      if (source_type.compare("HTS") == 0 && target_type.compare("HTE") == 0)
      {
        arrival_time = departure_time + journey_time;
        if (journey_time < 1800)
        {
          return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, clock_8_h, clock_8_h, arrival_time, arrival_time, 0, 0, "");
        }
        else if (journey_time >= 1800 && journey_time < 36000)
        {
          return Assignment_Tuple2(true, 0, 0, 0, 0, clock_14_h, clock_14_h, clock_11_h, 0, 0, arrival_time, arrival_time, 0, 0, "");
        }
        return Assignment_Tuple2(true, 0, 0, 0, 0, 0, 0, 0, 0, 0, arrival_time, arrival_time, 0, 0, "");
      }

      const int64_t remaining_8_h = 28800 - clock_8_h;
      const int64_t remaining_11_h = 39600 - clock_11_h;
      const int64_t remaining_14_h = 50400 - clock_14_h;

      int64_t first_violated_clock = 14;
      int64_t first_remaining_time = remaining_14_h;
      if (remaining_8_h < remaining_11_h)
      {
        if (remaining_8_h < remaining_14_h)
        {
          first_violated_clock = 8;
          first_remaining_time = remaining_8_h;
        }
      }
      else if (remaining_11_h <= remaining_14_h)
      {
        first_violated_clock = 11;
        first_remaining_time = remaining_11_h;
      }

      bool multiple_breaks = journey_time >= first_remaining_time;

      int64_t second_violated_clock = (remaining_11_h <= remaining_14_h) ? 11 : 14;
      int64_t second_remaining_time = ((second_violated_clock == 11) ? remaining_11_h : remaining_14_h) - first_remaining_time;
      bool required_break_10_h = (first_violated_clock == 8 && (journey_time - first_remaining_time + 3600) > second_remaining_time);

      int64_t num_breaks_30_m = 0;
	    int64_t num_breaks_10_h = 0;

      /*===================*/
      // TODO: Analyze code to see what can and cannot be made more concise
      /*===================*/

      float remaining_journey_time = (journey_time - first_remaining_time + 3600);

      if (multiple_breaks == true)
      {
        if (first_violated_clock != 8)
        {
          num_breaks_30_m = round_with_threshold((remaining_journey_time / 32400.0), rounding_threshold);
          num_breaks_10_h = 1 + floor(remaining_journey_time / 32400.0);
        }
        else
        {
          if (required_break_10_h == true)
          {
            remaining_journey_time = journey_time - first_remaining_time - second_remaining_time + 7200;
            num_breaks_30_m = 1 + round_with_threshold((remaining_journey_time / 32400.0), rounding_threshold);
            num_breaks_10_h = 1 + floor(remaining_journey_time / 32400.0);
          }
          else
          {
            num_breaks_30_m = 1;
          }
        }
      }
	  else
	  {
		remaining_journey_time = journey_time;
	  }
	 
      int64_t total_journey_time = journey_time + (num_breaks_10_h * 39600) + (num_breaks_30_m * 5400);
      if (journey_verification_mode == false && (departure_time + total_journey_time + load_time) > must_end_by_new)
      {
		if (dispatched == true) 
		{
			if ((target_type.compare("LL") == 0) && (facility == true))
			{
				must_start_by_new = facility_open;
				must_end_by_new = facility_close;
				if (journey_verification_mode == false && (departure_time + total_journey_time + load_time) > must_end_by_new)
				{
					must_end_by_new += 86400;
					must_start_by_new += 86400;
					if (journey_verification_mode == false && (departure_time + total_journey_time + load_time) > must_end_by_new)
					{
						return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E15");
					}					
				}																
			}			
		}
		else
		{
			if (target_type.compare("O") == 0) 
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E10");
			}
			if (target_type.compare("P") == 0) 
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E12");
			}
			if (target_type.compare("S") == 0)
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E14");
			}
			if (target_type.compare("D") == 0) 
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E16");
			}			
			if (target_type.compare("HTS") == 0)
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E21");
			}
		}
	  } 

	  
      bool can_start_on_time = (departure_time + total_journey_time) <= must_start_by_new;
      int64_t initial_dwell = 0;
      if (can_start_on_time == true)
      {
        initial_dwell = must_start_by_new - (departure_time + total_journey_time);
      }
	  
	  if (journey_verification_mode == false && target_type.compare("O") == 0 && initial_dwell > maximum_dwell) 
	  {
		return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E20");
	  }
      
	  int64_t dwell_after_break = initial_dwell;
      int64_t clock_reset_case = 0;
      if (initial_dwell >= (36000 + (break_permitted == true ? 0 : 3600)) )
      {
        dwell_after_break = initial_dwell - (36000 + (break_permitted == true ? 0 : 3600));
        clock_reset_case = 2;
      }
      else if (initial_dwell >= (1800 + (break_permitted == true ? 0 : 3600)) )
      {
        dwell_after_break = initial_dwell - (1800 + (break_permitted == true ? 0 : 3600));
        clock_reset_case = 1;
      }


      if (initial_dwell < (36000 + (break_permitted == true ? 0 : 3600)) && ((multiple_breaks == false ? clock_14_h : 0) + remaining_journey_time - (num_breaks_10_h * 32400) + (num_breaks_10_h == 0 ? 0 : 1800) + load_time + initial_dwell) > 50400)
	  {
			if (target_type.compare("O") == 0) 
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E11");
			}
			if (target_type.compare("P") == 0)
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E13");
			}
			if (target_type.compare("S") == 0) 
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E15");
			}
			if (target_type.compare("D") == 0)
			{
				return Assignment_Tuple2(false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "E17");
			}	
	  }

      int64_t on_duty_time = journey_time + load_time + ((num_breaks_10_h + num_breaks_30_m) * 3600) + (clock_reset_case == 0 ? dwell_after_break : 0) + ( (initial_dwell >= 5400 && break_permitted == false) ? 3600 : 0);
      int64_t on_duty_drive_time = on_duty_time - load_time - (clock_reset_case == 0 ? dwell_after_break : 0);
      int64_t driven_miles = distance + (55 * (num_breaks_10_h + num_breaks_30_m)) + ( (initial_dwell >= 5400 && break_permitted == false) ? 110 : 0);

      arrival_time = departure_time + journey_time + (num_breaks_10_h * 39600) + (num_breaks_30_m * 5400);
      if (can_start_on_time == true && break_permitted == false)
      {
        arrival_time += initial_dwell;
      }


      int64_t departure_time_new = arrival_time + load_time;
      if (can_start_on_time == true && break_permitted == true)
      {
        departure_time_new = must_start_by_new + load_time;
      }


      int64_t arrival_clock_8_hr = 0;
      int64_t time_left_after_last_10_hr_break = remaining_journey_time - floor(remaining_journey_time / 32400.0)*32400.0;
      if (clock_reset_case == 0)
      {
        arrival_clock_8_hr = (multiple_breaks == false ? clock_8_h + journey_time : time_left_after_last_10_hr_break - round_with_threshold((time_left_after_last_10_hr_break / 32400.0), rounding_threshold) *  25200);
      }
	    else if (break_permitted == false)
	    {
	      arrival_clock_8_hr += 1800;
	    }

      int64_t departure_clock_8_hr = 0;
      if (load_time < 1800)
      {
        departure_clock_8_hr = arrival_clock_8_hr;
      }

      int64_t arrival_clock_14_hr = 0;
	    if (break_permitted == false && clock_reset_case == 2)
	    {
	      arrival_clock_14_hr += 1800;
	    }
      if (clock_reset_case == 1 || clock_reset_case == 0)
      {
		if (target_type != "HTS") 
		{
		  arrival_clock_14_hr = (multiple_breaks == false ? clock_14_h + journey_time + initial_dwell : time_left_after_last_10_hr_break + 1800 + initial_dwell);
		}
        else if (target_type == "HTS") 
		{
		  arrival_clock_14_hr = (multiple_breaks == false ? clock_14_h + journey_time: time_left_after_last_10_hr_break + 1800);
		}
      }

      int64_t departure_clock_14_hr = arrival_clock_14_hr + load_time;


      int64_t arrival_clock_11_hr = 0;
	    if (break_permitted == false && clock_reset_case == 2)
      {
        arrival_clock_11_hr += 1800;
      }
      if (clock_reset_case == 1)
      {
        arrival_clock_11_hr = (multiple_breaks == false ? clock_11_h + journey_time : time_left_after_last_10_hr_break);
	    if (break_permitted == false && can_start_on_time == true)
        {
          arrival_clock_11_hr += 3600;
        }
      }
      else if (clock_reset_case == 0)
      {
        arrival_clock_11_hr = (multiple_breaks == false ? clock_11_h + journey_time : time_left_after_last_10_hr_break);
      }

      //valid BOOL, num_breaks_10_h INT, num_breaks_30_m INT, on_duty_time INT, on_duty_drive_time INT, arrival_clock_14_hr INT, departure_clock_14_hr INT, arrival_clock_11_hr INT, arrival_clock_8_hr INT, departure_clock_8_hr INT, arrival_time INT, departure_time_new INT, dwell_after_break INT, driven_miles INT
      return Assignment_Tuple2(true, num_breaks_10_h, num_breaks_30_m, on_duty_time, on_duty_drive_time, arrival_clock_14_hr, departure_clock_14_hr, arrival_clock_11_hr, arrival_clock_8_hr, departure_clock_8_hr, arrival_time, departure_time_new, dwell_after_break, driven_miles, "");
    }


}

/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
