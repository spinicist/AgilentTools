//
//  fdfFile.cpp
//  Agilent
//
//  Created by Tobias Wood on 23/08/2013.
//
//

#include "fdfFile.h"

namespace Agilent {

//*************************
#pragma mark fdfValue
//*************************
fdfType to_Type(const string &s) {
	if (s == "int") {
		return fdfType::Integer;
	} else if (s == "float") {
		return fdfType::Float;
	} else if (s == "char") {
		return fdfType::String;
	} else {
		throw(invalid_argument("Unknown field type: " + s));
	}
}

string to_string(const fdfType &t) {
	switch (t) {
		case fdfType::Integer: return "int"; break;
		case fdfType::Float: return "float"; break;
		case fdfType::String: return "char"; break;
	}
}



istream &operator>>(istream &is, fdfType &t) {
	string type_string;
	is >> type_string;
	t = to_Type(type_string);
	if (t == fdfType::String) {
		// Eat the * for strings
		string temp;
		getline(is, temp, '*');
	}
	return is;
}
		
ostream &operator<<(ostream &os, const fdfType &t) {
	os << to_string(t);
	return os;
}

fdfValue::fdfValue() { m_value.i_val = 0; m_type = fdfType::Integer; }
fdfValue::fdfValue(const int &ival) { m_value.i_val = ival; m_type = fdfType::Integer; }
fdfValue::fdfValue(const float &fval) { m_value.f_val = fval; m_type = fdfType::Float; }
fdfValue::fdfValue(const string &sval) { m_value.s_val = new string(sval); m_type = fdfType::String; }
fdfValue::fdfValue(const fdfValue &other) noexcept : m_type(other.m_type) {
	switch (m_type) {
		case (fdfType::Integer) : m_value.i_val = other.m_value.i_val; break;
		case (fdfType::Float)   : m_value.f_val = other.m_value.f_val; break;
		case (fdfType::String)  : m_value.s_val = new string(*(other.m_value.s_val)); break;
	}
}
fdfValue::fdfValue(fdfValue &&other) noexcept : m_type(other.m_type) {
	switch (m_type) {
		case (fdfType::Integer) : m_value.i_val = other.m_value.i_val; break;
		case (fdfType::Float)   : m_value.f_val = other.m_value.f_val; break;
		case (fdfType::String)  : m_value.s_val = other.m_value.s_val; other.m_value.s_val = nullptr; break;
	}
}
fdfValue::~fdfValue() {
	if ((m_type == fdfType::String) && (m_value.s_val != nullptr)) {
		delete m_value.s_val;
	}
}

fdfValue &fdfValue::operator=(const fdfValue &other) {
	m_type = other.m_type;
	switch (m_type) {
		case (fdfType::Integer) : m_value.i_val = other.m_value.i_val; break;
		case (fdfType::Float)   : m_value.f_val = other.m_value.f_val; break;
		case (fdfType::String)  : m_value.s_val = new string(*(other.m_value.s_val)); break;
	}
	return *this;
}
fdfValue &fdfValue::operator=(fdfValue &&other) {
	m_type = other.m_type;
	switch (m_type) {
		case (fdfType::Integer) : m_value.i_val = other.m_value.i_val; break;
		case (fdfType::Float)   : m_value.f_val = other.m_value.f_val; break;
		case (fdfType::String)  : m_value.s_val = other.m_value.s_val; other.m_value.s_val = nullptr; break;
	}
	return *this;
}

template <>
const string fdfValue::value() const {
	switch (m_type) {
		case fdfType::Integer: return std::to_string(m_value.i_val);
		case fdfType::Float:   return std::to_string(m_value.f_val);
		case fdfType::String: return *(m_value.s_val);
	}
}

//*************************
#pragma mark fdfField
//*************************
const string &fdfField::name() const { return m_name; }
istream &operator>> (istream &is, fdfField &f) {
	string name, values;
	is >> f.m_type;
	f.m_values.resize(0);
	getline(is, name, '=');
	if (name.find("[]") == string::npos) {
		string temp, value;
		getline(is, value, ';');
		getline(is, temp);
		switch (f.m_type) {
			case fdfType::Integer: f.m_values.emplace_back(stoi(value)); break;
			case fdfType::Float: f.m_values.emplace_back(stof(value)); break;
			case fdfType::String: {
				size_t leftQuote = value.find("\"") + 1;
				size_t rightQuote = value.rfind("\"");
				f.m_values.emplace_back(value.substr(leftQuote, rightQuote - leftQuote));
			} break;
		}
	} else {
		// We have an array
		name.erase(name.find("[]"));
		string temp;
		getline(is, temp, '{'); // Get rid of the opening brace
		getline(is, values, '}');
		getline(is, temp); // Eat the remainder of the line
		size_t thisComma = 0, nextComma = values.find(",");
		do {
			string value = values.substr(thisComma, nextComma - thisComma);
			switch (f.m_type) {
				case fdfType::Integer: f.m_values.emplace_back(stoi(value)); break;
				case fdfType::Float: f.m_values.emplace_back(stof(value)); break;
				case fdfType::String: {
					size_t leftQuote = value.find("\"") + 1;
					size_t rightQuote = value.rfind("\"");
					f.m_values.emplace_back(value.substr(leftQuote, rightQuote - leftQuote));
				} break;
			}
			thisComma = nextComma + 1; // Want to point after the comma
			nextComma = values.find(",", thisComma);
			if (nextComma == string::npos)
				nextComma = values.size();
		} while (thisComma < values.size());
	}
	size_t firstChar = name.find_first_not_of(" ");
	size_t lastChar  = name.find_last_not_of(" ");
	f.m_name = name.substr(firstChar, lastChar - firstChar + 1);
	return is;
}

ostream &operator<<(ostream &os, const fdfField &f) {
	os << f.m_type << "  ";
	if (f.m_type == fdfType::String) {
		cout << " *";
	} else if (f.m_type == fdfType::Integer) {
		cout << "  ";
	}
	cout << f.m_name;
	if (f.m_values.size() > 1) cout << "[]";
	cout << " = ";
	if (f.m_values.size() > 1) cout << "{";
	for (size_t i = 0; i < f.m_values.size(); i++) {
		cout << f.m_values.at(i).value<string>();
		if (i < (f.m_values.size() - 1)) cout << ",";
	}
	if (f.m_values.size() > 1) cout << "}";
	cout << ";";
	return os;
}

//******************
#pragma mark fdfFile
//******************

fdfFile::fdfFile(fdfFile &&f) noexcept :
	m_path(f.m_path),
	m_dtype(f.m_dtype),
	m_hdrSize(f.m_hdrSize),
	m_rank(f.m_rank),
	m_dims(f.m_dims),
	m_header(f.m_header)
{}

fdfFile::fdfFile(const string &path) {
	open(path);
}

void fdfFile::open(const string &path) {	
	m_file.open(path, ios::in);
	string nextLine;
		
	if (!m_file) {
		throw(runtime_error("Could not open file: " + path));
	}
	m_path = path;
	
	m_header.clear();
	if (!getline(m_file, nextLine) || nextLine != "#!/usr/local/fdf/startup") {
		throw(runtime_error("Could not magic string in file: " + path));
	}
	fdfField temp;
	// Checksum is always the last header field
	while (temp.name() != "checksum") {
		m_file >> temp;
		m_header.insert(pair<string, fdfField>(temp.name(), temp));
	}
	// Now search for the null character for end of the header
	while (m_file.get() != '\0') {}
	m_hdrSize = m_file.tellg();
	m_dtype = headerValue<string>("storage");
	m_rank = headerValue<size_t>("rank");
	if (m_rank == 3) {
		m_dims[0] = headerValue<size_t>("matrix", 0);
		m_dims[1] = headerValue<size_t>("matrix", 1);
		m_dims[2] = headerValue<size_t>("matrix", 2);
	} else {
		// 2D fdfs have a horrible data flip in them.
		m_dims[0] = headerValue<size_t>("matrix", 1);
		m_dims[1] = headerValue<size_t>("matrix", 0);
		m_dims[2] = 1;
	}
	// Unfortunately cannot keep all the headers open in a large dataset as
	// there is a hard limit on the number of open file descriptors.
	m_file.close();
}

void fdfFile::close() {
	m_file.close();
}

const string &fdfFile::path() const { return m_path; }
const size_t fdfFile::rank() const { return m_rank; }
const size_t fdfFile::dim(const size_t d) const {
	if (d > 2) {
		throw(invalid_argument("Tried to access dimension " + std::to_string(d) + " of file: " + m_path));
	} else {
		return m_dims[d];
	}
}
const size_t fdfFile::dataSize() const {
	return m_dims[0] * m_dims[1] * m_dims[2];
}
const map<string, fdfField> &fdfFile::header() const { return m_header; }
} // End namespace Agilent