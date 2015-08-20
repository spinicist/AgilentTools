//
//  fdfFile.h
//  Agilent
//
//  Created by Tobias Wood on 23/08/2013.
//
//

#ifndef AGILENT_FDFFILE
#define AGILENT_FDFFILE

#include <string>
#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <fstream>
#include <exception>

using namespace std;

namespace Agilent {

enum class fdfType : size_t { Integer, Float, String };
fdfType to_Type(const string &s);
string to_string(const fdfType &t);
istream &operator>>(istream &is, fdfType &t);
ostream &operator<<(ostream &os, const fdfType &t);

class fdfValue {
	protected:
		union storage_type {
			int i_val;
			float f_val;
			string *s_val;
		};
		
		fdfType m_type;
		storage_type m_value;
	public:
		fdfValue();
		fdfValue(const int &ival);
		fdfValue(const float &fval);
		fdfValue(const string &sval);
		fdfValue(const fdfValue &other) noexcept;
		fdfValue(fdfValue &&other) noexcept;
		~fdfValue();
		
		fdfValue &operator=(const fdfValue &other);
		fdfValue &operator=(fdfValue &&other);
		
		template<typename T> const T value() const {
			switch (m_type) {
				case fdfType::Integer: return static_cast<T>(m_value.i_val);
				case fdfType::Float:   return static_cast<T>(m_value.f_val);
				case fdfType::String:  throw(runtime_error("No suitable conversion for string field."));
			}
		}
};

class fdfField {
	protected:
		string m_name;
		fdfType m_type;
		vector<fdfValue> m_values;
	
	public:
		template<typename T> const T value(const size_t i = 0) {
			if (i > m_values.size())
				throw(runtime_error("Field " + m_name + " has " + std::to_string(m_values.size()) + " values, tried to access value " + std::to_string(i)));
			return m_values.at(i).value<T>();
		}
		
		const string &name() const;
		
		friend istream &operator>>(istream &is, fdfField &f);
		friend ostream &operator<<(ostream &os, const fdfField &f);
};

class fdfFile {
	protected:
		ifstream m_file;
		string m_path, m_dtype;
		size_t m_hdrSize, m_rank;
		array<size_t, 3> m_dims;
		map<string, fdfField> m_header;
		template<typename T> const T headerValue(const string &n, const size_t i = 0) {
			auto field = m_header.find(n);
			if (field == m_header.end())
				throw(runtime_error("Field " + n + " does not exist in file: " + m_path));
			else
				return field->second.value<T>(i);
		}
	
	public:
		fdfFile(fdfFile &&f) noexcept;
		fdfFile(const string &path);
		
		void open(const string &path);
		void close();
		const string &path() const;
		const size_t rank() const;
		const size_t dim(const size_t d) const;
		const size_t dataSize() const;
		const map<string, fdfField> &header() const;
		
		template<typename T> vector<T>readData() {
			m_file.open(m_path, ios::in);
			m_file.seekg(m_hdrSize, ios::beg);
			if (!m_file) {
				throw(runtime_error("Error while opening fdf file: " + m_path));
			}
			vector<T> Tbuffer(dataSize());
			if (m_dtype == "float") {
				vector<float> floatBuffer(dataSize());
				m_file.read(reinterpret_cast<char *>(floatBuffer.data()), dataSize() * sizeof(float));
				if (!m_file) {
					throw(runtime_error("Could not read data from file: " + m_path));
				}
				if (m_rank == 2) {
					// 2D fdfs have an extra, really stupid, flip of the data ordering
          			for(size_t i = 0; i < m_dims[1]; i++) {
						for (size_t j = 0; j < m_dims[0]; j++) {
							size_t ix1 = i * m_dims[0] + j;
							size_t ix2 = (m_dims[0] - j - 1) * m_dims[1] + m_dims[1] - i - 1;
							Tbuffer[ix1] = static_cast<T>(floatBuffer[ix2]);
						}
					}
				} else if (m_rank == 3) {
					for (size_t i = 0; i < dataSize(); i++) {
						Tbuffer[i] = static_cast<T>(floatBuffer[i]);
					}
				}
			} else {
				throw(runtime_error("Unsupported datatype " + m_dtype + " in file: " + m_path));
			}
			m_file.close();
			return Tbuffer;
		}
		
};

} // End namespace Agilent
#endif
