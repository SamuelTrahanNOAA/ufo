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

#include "ufo/filters/obsfunctions/ObsStringFormatter.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "boost/format.hpp"
#include "boost/optional.hpp"
#include "eckit/exception/Exceptions.h"
#include "ioda/ObsDataVector.h"
#include "ioda/ObsSpace.h"
#include "oops/util/DateTime.h"
#include "oops/util/Logger.h"
#include "ufo/filters/DiagnosticFlag.h"
#include "ufo/filters/ObsFilterData.h"

namespace ufo {

  struct ObsTypedef {
    using Float = float;
    using Integer = int;
    using Integer_64 = int;  // ObsSpace get() routines don't have a special type for this.
    using String = std::string;
    using DateTime = util::DateTime;
    using Bool = ufo::DiagnosticFlag;
  };

  // -----------------------------------------------------------------------------

  template<class ToType>
  static void format_var_type(std::vector<boost::format> &fmt, const ObsFilterData &obs,
                              const Variable &var) {
    std::vector<ToType> data;
    obs.get(var, data);

    const size_t nlocs = obs.nlocs();
    for (size_t i = 0; i < nlocs; i++)
      fmt[i] % data[i];
  }

  // -----------------------------------------------------------------------------

  static void format_one_var(std::vector<boost::format> &fmt, const ObsFilterData &obs,
                             const Variable &var,
                             boost::optional<std::string> optionalDateFormat) {
    const ioda::ObsDtype targetType = obs.dtype(var);

    // Handle the special case of DateTime.formatString()
    if (targetType == ioda::ObsDtype::DateTime && optionalDateFormat) {
      const std::string dateFormat = *optionalDateFormat;

      std::vector<ObsTypedef::DateTime> data;
      obs.get(var, data);

      const size_t nlocs = obs.nlocs();
      for (size_t i = 0; i < nlocs; i++)
        fmt[i] % data[i].formatString(dateFormat);

    // Remaining cases use the original obs type.
    } else if (targetType == ioda::ObsDtype::Integer)
      format_var_type<ObsTypedef::Integer>(fmt, obs, var);
    else if (targetType == ioda::ObsDtype::Integer_64)
      format_var_type<ObsTypedef::Integer_64>(fmt, obs, var);
    else if (targetType == ioda::ObsDtype::Float)
      format_var_type<ObsTypedef::Float>(fmt, obs, var);
    else if (targetType == ioda::ObsDtype::String)
      format_var_type<ObsTypedef::String>(fmt, obs, var);
    else if (targetType == ioda::ObsDtype::Bool)
      format_var_type<ObsTypedef::Bool>(fmt, obs, var);
    else if (targetType == ioda::ObsDtype::DateTime)
      format_var_type<ObsTypedef::DateTime>(fmt, obs, var);
    else
      throw eckit::BadCast("ObsStringFormatter encountered an unknown type. It can only handle: "
                           "Bool, DateTime, Float, Integer, Integer_64, and String.", Here());
  }

  // -----------------------------------------------------------------------------

  ObsStringFormatter::ObsStringFormatter(const eckit::LocalConfiguration & conf) {
    oops::Log::trace() << "ObsStringFormatter constructor" << std::endl;
    options_.validateAndDeserialize(conf);

    const auto &params = options_.variables.value();
    for (const auto &param : params)
      requiredVariables_ += param.variable.value();
  }

  // -----------------------------------------------------------------------------

  ObsStringFormatter::~ObsStringFormatter() {
    oops::Log::trace() << "ObsStringFormatter destructor" << std::endl;
  }

  // -----------------------------------------------------------------------------

  void ObsStringFormatter::compute(const ObsFilterData & in,
                                   ioda::ObsDataVector<std::string> & out) const {
    oops::Log::trace() << "ObsStringFormatter formatting start" << std::endl;

    const size_t nlocs = in.nlocs();
    const auto &variables = options_.variables.value();

    // Create a boost::format that we'll duplicate for each observation.
    const std::string formatString = options_.format.value();
    const boost::format format(formatString);

    // Abort if the user provides the wrong number of variables.
    if (format.size() != variables.size()) {
      std::ostringstream err;
      err << "ObsStringFormatter format length mismatch."
          << " Format wants " << format.size()
          << " variables, but you supplied " << variables.size();
      throw eckit::BadParameter(err.str(), Here());
    }

    // We'll use a vector of boost::format to construct each observation's string.
    std::vector<boost::format> fmt(nlocs, format);

    // For each variable, loop over all observation, and give them to the format object.
    for (const auto &param : variables)
      format_one_var(fmt, in, param.variable.value(), param.dateFormat.value());

    // Output each format object as a string to ``out[0]`` via and ostringstream.
    for (size_t i = 0; i < nlocs; i++)
      out[0][i] = (std::ostringstream() << fmt[i]).str();

    oops::Log::trace() << "ObsStringFormatter formatting complete" << std::endl;
  }

  // -----------------------------------------------------------------------------

  const ufo::Variables & ObsStringFormatter::requiredVariables() const {
    return requiredVariables_;
  }
} // namespace ufo
