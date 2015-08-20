//
//  procpar.c
//  procparse
//
//  Created by Tobias Wood on 10/07/2012.
//  Copyright (c) 2012 Tobias Wood. All rights reserved.
//

#include "procpar.h"

using namespace std;
using namespace Eigen;

namespace Agilent {

//******************************************************************************
#pragma mark Constructors
//******************************************************************************
Parameter::Parameter()
{


}

Parameter::Parameter(const string &name, const SubType &st, const string &val)
{
	switch (st) {
		case SubType::Real: case SubType::Delay: case SubType::Freq: case SubType::Pulse: case SubType::Int:
			throw(invalid_argument("Tried to create string parameter " + m_name + " with a real subtype"));
			break;
		case SubType::String: case SubType::Flag:
			break;
		case SubType::Junk:
			// It's the stupid "junk" parameter
			break;
	}
	m_name = name;
	m_type = Type::String;
	m_subtype = st;
	m_stringValues.push_back(val);
}

// The default value of 8 for max is taken from observations of procpar
Parameter::Parameter(const string &name, const SubType &st,
					 const vector<string> &vals, const vector<string> &allowed = vector<string>(),
					 const int ggroup = 0, const int dgroup = 0,
					 const double max = 8,
					 const double min = 0,
					 const double step = 0,
					 const int protection = 0, const int active = 0, const int intptr = 0) :
					 m_ggroup(ggroup), m_dgroup(dgroup), m_protection(protection),
				     m_active(active), m_intptr(intptr), m_max(max), m_min(min), m_step(step)
{
	switch (st) {
		case SubType::Real: case SubType::Delay: case SubType::Freq: case SubType::Pulse: case SubType::Int:
			throw(invalid_argument("Tried to create string parameter " + m_name + " with a real subtype"));
			break;
		case SubType::String: case SubType::Flag:
			break;
		case SubType::Junk:
			// Junk subtype
			break;
	}
	m_name = name;
	m_subtype = st;
	m_type = Type::String;
	m_stringValues = vals;
	m_stringAllowed = allowed;
}

Parameter::Parameter(const string &name, const SubType &st, const double &val)
{
	switch (st) {
		case SubType::Real: case SubType::Delay: case SubType::Freq: case SubType::Pulse: case SubType::Int:
			break;
		case SubType::String: case SubType::Flag:
			throw(invalid_argument("Tried to create real parameter " + m_name + " with a string subtype"));
			break;
		case SubType::Junk:
			break;
	}
	m_name = name;
	m_type = Type::Real;
	m_subtype = st;
	m_realValues.resize(1); m_realValues(0) = val;
}

Parameter::Parameter(const string &name, const SubType &st,
                     const ArrayXd &vals, const ArrayXd &allowed,
					 const int ggroup = 0, const int dgroup = 0,
					 const double max = numeric_limits<double>::max(),
					 const double min = -numeric_limits<double>::max(),
					 const double step = 0,
					 const int protection = 0, const int active = 0, const int intptr = 0) :
					 m_ggroup(ggroup), m_dgroup(dgroup), m_protection(protection),
				     m_active(active), m_intptr(intptr), m_max(max), m_min(min), m_step(step)
{
	switch (st) {
		case SubType::Real: case SubType::Delay: case SubType::Freq: case SubType::Pulse: case SubType::Int:
			break;
		case SubType::String: case SubType::Flag:
			throw(invalid_argument("Tried to create real parameter " + m_name + " with a string subtype"));
			break;
		case SubType::Junk:
			// Junk subtype
			break;
	}
	m_name = name;
	m_type = Type::Real;
	m_subtype = st;
	m_realValues = vals;
	m_realAllowed = allowed;
}

Parameter::Parameter(const string &name, const SubType &st, const int n)
{
	m_name = name;
	m_subtype = st;
	switch (st) {
		case SubType::Real: case SubType::Delay: case SubType::Freq: case SubType::Pulse: case SubType::Int:
			m_type = Type::String;
			m_stringValues.resize(n);
			break;
		case SubType::String: case SubType::Flag:
			m_type = Type::Real;
			m_realValues.resize(n);
			break;
		case SubType::Junk:
			// Junk subtype, appears to be a string
			m_type = Type::String;
			m_stringValues.resize(n);
			break;
	}
}

//******************************************************************************
#pragma mark Property accessors
//******************************************************************************
const Parameter::Type &Parameter::type() const { return m_type; }
const string &Parameter::name() const { return m_name; }
const string &Parameter::type_name() const {
	static const array<const string, 2> types = { "Real", "String" };
	return types[static_cast<size_t>(m_type)];
}

const string &Parameter::subtype_name() const {
	static const array<const string, 8> subtypes =
		{ "", "Real", "String", "Delay", "Flag", "Frequency", "Pulse", "Integer" };
	return subtypes[static_cast<size_t>(m_subtype)];
}

const size_t Parameter::nvals() const {
	if (m_type == Type::String)
		return m_stringValues.size();
	else
		return m_realValues.size();
}

const size_t Parameter::nallowed() const {
	if (m_type == Type::String)
		return m_stringAllowed.size();
	else
		return m_realAllowed.size();
}

const string &Parameter::stringValue(const size_t i = 0) const {
	if (m_type == Type::Real)
		throw(runtime_error("Tried to read a string value from real parameter " + m_name));
	return m_stringValues.at(i);
}
const vector<string> &Parameter::stringValues() const {
	if (m_type == Type::String)
		return m_stringValues;
	else
		throw(runtime_error("Tried to read string values from real parameter " + m_name));
}

const double &Parameter::realValue(const size_t i = 0) const {
	if (m_type == Type::String)
		throw(runtime_error("Tried to read a real value from string parameter " + m_name));
	if (i < static_cast<size_t>(m_realValues.rows()))
		return m_realValues(i);
	else {
		throw(out_of_range("Parameter " + m_name + " has " + to_string(m_realValues.size()) + " values, tried to access value " + to_string(i)));
	}
}
const ArrayXd &Parameter::realValues() const {
	if (m_type == Type::Real)
		return m_realValues;
	else
		throw(runtime_error("Tried to read real values from string parameter " + m_name));
}

//******************************************************************************
#pragma mark Output
//******************************************************************************
const string Parameter::print_values() const {
	stringstream ss;
	
	if (m_type == Type::Real) {
		for (ArrayXd::Index i = 0; i < m_realValues.rows(); i++)
			ss << m_realValues(i) << " ";
	} else {
		vector<string>::const_iterator it = m_stringValues.begin();
		ss << "\"" << *it << "\"";
		for (it++; it < m_stringValues.end(); it++)
			ss << endl << "\"" << *it << "\"";
	}
	return ss.str();
}

const string Parameter::print_allowed() const {
	stringstream ss;
	
	if (m_type == Type::Real) {
		for (ArrayXd::Index i = 0; i < m_realAllowed.rows(); i++)
			ss << m_realAllowed(i);
	} else {
		for (auto &s: m_stringAllowed)
			ss << "\"" << s << "\" ";
	}
	return ss.str();
}

const string Parameter::print() const {
	stringstream s;

	s << m_name << " " << static_cast<size_t>(m_subtype) << " " << static_cast<size_t>(m_type) << " "
	  << m_max << " " << m_min << " " << m_step << " "
	  << m_ggroup << " " << m_dgroup << " " << m_protection << " " << m_active << " " << m_intptr << endl;
	if (m_type == Type::Real) {
		s << m_realValues.size() << " " << print_values() << endl
		  << m_realAllowed.size() << " " << print_allowed();
	} else {
		s << m_stringValues.size() << " " << print_values() << endl
		  << m_stringAllowed.size() << " " << print_allowed();
	}
	return s.str();
}

ostream& operator<<(ostream &os, const Parameter &p) {
	os << p.print();
	return os;
}

//******************************************************************************
#pragma mark Input
//******************************************************************************
string readQuotedString(istream & is) {
	stringstream dummy, quoted;
	if (!is.get(*dummy.rdbuf(), '\"')) {
		throw(runtime_error("Could not find an opening quote."));
		return "";
	}
	is.get(); // Consume the quote, get(...,delim) does not consume it
	if (is.peek() == '\"') {
		// We have an empty string
		is.get(); // Consume closing quote
		return "";
	} else if (!is.get(*quoted.rdbuf(), '\"')) {
		throw(runtime_error("Could not find a closing quote."));
	} else
		is.get(); // Consume the quote
	return quoted.str();
}

istream& operator>>(istream &is, Parameter &p) {
	string name;
	int subtype_in, type_in, ggroup, dgroup, protection, active, intptr, nvals, nallowed;
	double max, min, step;
	ArrayXd realValues, realAllowed;
	vector<string> stringValues, stringAllowed;
	if (is >> name >> subtype_in >> type_in
	       >> max >> min >> step
		   >> ggroup >> dgroup >> protection >> active >> intptr)
	{
		Parameter::Type type = static_cast<Parameter::Type>(type_in);
		Parameter::SubType subtype = static_cast<Parameter::SubType>(subtype_in);
		if (!(is >> nvals))
			throw(runtime_error("Failed while reading number of values for parameter " + name));
		if (type == Parameter::Type::Real) {
			realValues.resize(nvals);
			for (int i = 0; i < nvals; i++)
				is >> realValues(i);
		} else if (type == Parameter::Type::String) {
			stringValues.resize(nvals);
			for (int i = 0; i < nvals; i++)
				stringValues[i] = readQuotedString(is);
		} else
			throw(runtime_error("Invalid type value for parameter " + name + ", no values read"));
		if (!is)
			throw(runtime_error("Failed while reading allowed values for parameter " + name + " from procpar file"));
		if (!(is >> nallowed))
			throw(runtime_error("Failed while reading number of allowed values for parameter " + name));
		if (type == Parameter::Type::Real) {
			realAllowed.resize(nallowed);
			for (int i = 0; i < nallowed; i++)
				is >> realAllowed[i];
		} else if (type == Parameter::Type::String) {
			stringAllowed.resize(nallowed);
			for (int i = 0; i < nallowed; i++)
				stringAllowed[i] = readQuotedString(is);
		}
		if (!is)
			throw(runtime_error("Failed while reading allowed values for parameter " + name + " from procpar file"));
		
		if (type == Parameter::Type::Real)
			p = Parameter(name, subtype, realValues, realAllowed,
			              ggroup, dgroup, max, min, step, protection, active, intptr);
		else if (type == Parameter::Type::String)
			p = Parameter(name, subtype, stringValues, stringAllowed,
			              ggroup, dgroup, max, min, step, protection, active, intptr);
		return is;
	} else {
		// An eof here is probably not an error, as procpar files have a trailing line
		// Anything else is an error.
		if (is.eof())
			return is;
		else
			throw(runtime_error("Error while reading parameter definition line."));
	}
}

//******************************************************************************
#pragma mark Equality
//******************************************************************************
const bool Parameter::operator==(const Parameter &other) {
	if (m_type != other.m_type)
		return false;
	if (m_subtype != other.m_subtype)
		return false;
	
	if (m_type == Type::String) {
		if (m_stringValues.size() != other.m_stringValues.size())
			return false;
		for (size_t i = 0; i < m_stringValues.size(); i++)
			if (m_stringValues[i] != other.m_stringValues[i]) return false;
	} else if (m_type == Type::Real) {
		if (m_realValues.size() != other.m_realValues.size())
			return false;
		for (ArrayXd::Index i = 0; i < m_realValues.size(); i++)
			if (m_realValues[i] != other.m_realValues[i]) return false;
	}
	return true;
}

const bool Parameter::operator!=(const Parameter &other) { return !(operator==(other)); }
//******************************************************************************
#pragma mark ProcPar Class
//******************************************************************************
/*
 *  Extracts entire ProcPar into p
 */
istream &operator>>(istream &is, ProcPar &pp) {
	Parameter p;
	while(is >> p)
		pp.insert(p);
	return is;
}

ostream &operator<<(ostream &os, const ProcPar &pp) {
	for (auto &p : pp.m_parameters)
		os << p.second << endl;
	return os;
}

/*
 *  Checks wheter a ProcPar contains valid entries
 */
ProcPar::operator bool() const {
	return (m_parameters.size() > 0);
}

const bool ProcPar::contains(const string &name) const {
	auto p = m_parameters.find(name);
	if (p == m_parameters.end())
		return false;
	return true;
}

void ProcPar::insert(const Parameter &p) {
	m_parameters.insert(pair<string, Parameter>(p.name(), p));
}

void ProcPar::remove(const string &name) {
	if (contains(name)) {
		m_parameters.erase(m_parameters.find(name));
	} else {
		throw(runtime_error("Tried to remove non-existent parameter " + name));
	}
}

size_t ProcPar::count() const {
	return m_parameters.size();
}

const Parameter &ProcPar::parameter(const string &name) const {
	return m_parameters.find(name)->second;
}

const vector<string> ProcPar::names() const {
	vector<string> n;
	n.reserve(m_parameters.size());
	for (auto &p : m_parameters)
		n.emplace_back(p.first);
	return n;
}

const double ProcPar::realValue(const string &name, const size_t index) const {
	auto p = m_parameters.find(name);
	if (p == m_parameters.end())
		throw(invalid_argument("Could not find parameter " + name));
	else
		return p->second.realValue(index);
}

const ArrayXd &ProcPar::realValues(const string &name) const {
	auto p = m_parameters.find(name);
	if (p == m_parameters.end())
		throw(invalid_argument("Could not find parameter " + name));
	else
		return p->second.realValues();
}

const string &ProcPar::stringValue(const string &name, const size_t index) const {
	auto p = m_parameters.find(name);
	if (p == m_parameters.end())
		throw(invalid_argument("Could not find parameter " + name));
	else
		return p->second.stringValue(index);
}

const vector<string> &ProcPar::stringValues(const string &name) const {
	auto p = m_parameters.find(name);
	if (p == m_parameters.end())
		throw(invalid_argument("Could not find parameter " + name));
	else
		return p->second.stringValues();
}

}; // End namespace Agilent
