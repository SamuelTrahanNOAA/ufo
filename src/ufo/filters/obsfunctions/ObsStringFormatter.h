/*
 * CC0 - No Copyright (Public Domain)
 * 
 * The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.
 *
 * You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission. See Other Information below.
 *
 * Other Information
 * In no way are the patent or trademark rights of any person affected by CC0, nor are the rights that other persons may have in the work or in how the work is used, such as publicity or privacy rights.
 *
 * The person who associated a work with this deed makes no warranties about the work, and disclaims liability for all uses of the work, to the fullest extent permitted by applicable law.
 *
 * When using or citing the work, you should not imply endorsement by the author or the affirmer.
 */

#ifndef UFO_FILTERS_OBSFUNCTIONS_OBSSTRINGFORMATTER_H_
#define UFO_FILTERS_OBSFUNCTIONS_OBSSTRINGFORMATTER_H_

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "oops/util/parameters/OptionalParameter.h"
#include "oops/util/parameters/Parameters.h"
#include "oops/util/parameters/ParameterTraits.h"
#include "oops/util/parameters/ParameterTraitsScalarOrMap.h"
#include "oops/util/parameters/RequiredParameter.h"
#include "ufo/filters/FilterParametersBase.h"
#include "ufo/filters/obsfunctions/ObsFunctionBase.h"
#include "ufo/filters/ObsFilterData.h"
#include "ufo/filters/Variable.h"
#include "ufo/filters/Variables.h"
#include "ufo/utils/parameters/ParameterTraitsVariable.h"

namespace ufo {
///
/// \brief Options for each variable: variable and date format
///
class ObsStringFormatterVariableParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(ObsStringFormatterVariableParameters, Parameters)

 public:
  /// The variable to insert into the format.
  oops::RequiredParameter<Variable> variable{"variable", this};

  /// For DateTime variables, an optional format to send to DateTime.formatString()
  /// If "date format" is absent, the default DateTime string conversion is used.
  oops::OptionalParameter<std::string> dateFormat{"date format", this};
};

///
/// \brief Options for obs function: String format and list of variables
///
class ObsStringFormatterParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(ObsStringFormatterParameters, Parameters)

 public:
  /// The format of the string for boost::format.
  oops::RequiredParameter<std::string> format{"format", this};

  /// Variables to substitute in the format.
  oops::RequiredParameter<std::vector<ObsStringFormatterVariableParameters>> variables
    {"format", this};
};


///
/// \brief Generates a string variable from a list of input variables using boost::format.
///        Can also format DateTimes using DateTime.formatString().
///
class ObsStringFormatter : public ObsFunctionBase<std::string> {
 public:
  explicit ObsStringFormatter(const eckit::LocalConfiguration & conf);
  ~ObsStringFormatter();

  void compute(const ObsFilterData & in, ioda::ObsDataVector<std::string> & out) const;
  const ufo::Variables & requiredVariables() const;
 private:
  ObsStringFormatterParameters options_;
  ufo::Variables requiredVariables_;
};

}  // namespace ufo

#endif  // UFO_FILTERS_OBSFUNCTIONS_OBSSTRINGFORMATTER_H_
