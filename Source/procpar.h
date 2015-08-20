//
//  procpar.h
//  procparse
//
//  Created by Tobias Wood on 10/07/2012.
//  Copyright (c) 2012 Tobias Wood. All rights reserved.
//

#ifndef AGILENT_PROCPAR
#define AGILENT_PROCPAR

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>

#include "Eigen/Core"

namespace Agilent {
	
	class Parameter {
		public:
			enum class Type : size_t {
				Real = 1, String
			};
			
			enum class SubType : size_t {
				Junk = 0, Real = 1,	String,
				Delay, Flag, Freq, Pulse, Int
			};
	
		protected:
			std::string m_name;
			Type m_type;
			SubType m_subtype;
			double  m_max, m_min, m_step;
			int  m_ggroup, m_dgroup, m_protection, m_active, m_intptr;
			std::vector<std::string> m_stringValues, m_stringAllowed;
			Eigen::ArrayXd m_realValues, m_realAllowed;
			
		public:
			Parameter();
			Parameter(const std::string &name, const SubType &st, const std::string &val); //!< Construct a string parameter with one value
			Parameter(const std::string &name, const SubType &st, const double &val);      //!< Construct a string parameter with multiple values
			Parameter(const std::string &name, const SubType &st,
			          const std::vector<std::string> &vals, const std::vector<std::string> &allowed,
					  const int ggroup, const int dgroup,
					  const double max, const double min, const double step,
					  const int protection, const int active, const int intptr); //!< Construct a real parameter with one value
			Parameter(const std::string &name, const SubType &st,
			          const Eigen::ArrayXd &vals, const Eigen::ArrayXd &allowed,
					  const int ggroup, const int dgroup,
					  const double max, const double min, const double step,
					  const int protection, const int active, const int intptr); //!< Construct a real parameter with multiple values
			Parameter(const std::string &name, const SubType &st, const int n);  //!< Construct a blank parameter with space for several values

			const Type   &type() const;
			const std::string &name() const;
			const size_t nvals() const;
			const size_t nallowed() const;
			const std::string &type_name() const;
			const std::string &subtype_name() const;
			
			const std::string &stringValue(const size_t i) const;
			const std::vector<std::string> &stringValues() const;
			const double &realValue(const size_t i) const;
			const Eigen::ArrayXd &realValues() const;
			
			const std::string print_values() const;
			const std::string print_allowed() const;
			const std::string print() const;
			
			const bool operator==(const Parameter &other);
			const bool operator!=(const Parameter &other);
			friend std::ostream& operator<<(std::ostream &os, const Parameter &p);
			friend std::istream& operator>>(std::istream &is, Parameter &p);
	};
	
	class ProcPar {
		protected:
			typedef std::map<std::string, Parameter> Parmap;
			Parmap m_parameters;
		
		public:
			friend std::ostream& operator<<(std::ostream &os, const ProcPar &p);
			friend std::istream& operator>>(std::istream &is, ProcPar &p);
			explicit operator bool() const;

			const bool contains(const std::string &name) const;
			void insert(const Parameter &p);
			void remove(const std::string &name);
			size_t count() const;
			
			const Parameter &parameter(const std::string &name) const;
			const std::vector<std::string> names() const;
			const double realValue(const std::string &name, const size_t index = 0) const;
			const Eigen::ArrayXd &realValues(const std::string &name) const;
			const std::string &stringValue(const std::string &name, const size_t index = 0) const;
			const std::vector<std::string> &stringValues(const std::string &name) const;
	};
} // End namespace Agilent

#endif
