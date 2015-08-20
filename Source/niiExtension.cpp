//
//  Extension.cpp
//  NiftiImage
//
//  Created by Tobias Wood on 18/09/2013.
//  Copyright (c) 2013 Tobias Wood. All rights reserved.
//

#include "Nifti/Nifti.h"
#include "Nifti/ExtensionCodes.h"

using namespace std;

namespace Nifti {

//*********************************
#pragma mark Methods for Extension
//*********************************
const string &Extension::CodeName(const int code) {
	static const map<int, string> Codes {
		{ NIFTI_ECODE_IGNORE,           "Ignore" },
		{ NIFTI_ECODE_DICOM,            "DICOM Attributes" },
		{ NIFTI_ECODE_AFNI,             "AFNI" },
		{ NIFTI_ECODE_COMMENT,          "Plain ASCII text" },
		{ NIFTI_ECODE_XCEDE,            "XCEDE" },
		{ NIFTI_ECODE_JIMDIMINFO,       "JIM Dimension Information" },
		{ NIFTI_ECODE_WORKFLOW_FWDS,    "Workflow Forwards" },
		{ NIFTI_ECODE_FREESURFER,       "Freesurfer" },
		{ NIFTI_ECODE_PYPICKLE,         "Pickled Python Objects" },
		{ NIFTI_ECODE_MIND_IDENT,       "Mind Ident" },
		{ NIFTI_ECODE_B_VALUE,          "B Value" },
		{ NIFTI_ECODE_SPHERICAL_DIRECTION, "Spherical Direction" },
		{ NIFTI_ECODE_DT_COMPONENT,     "DT Component" },
		{ NIFTI_ECODE_SHC_DEGREEORDER,  "SHC Degree Order" },
		{ NIFTI_ECODE_VOXBO,            "VOXBO" },
		{ NIFTI_ECODE_CARET,            "CARET" }
	};
	static string unknown("Unknown extension code");
	auto it = Codes.find(code);
	if (it == Codes.end())
		return unknown;
	else
		return it->second;
}

Extension::Extension(int code, vector<char> data) :
	m_code(code), m_data(data)
{}
Extension::Extension(int size, int code, char *data) :
	m_code(code)
{
	m_data.resize(size - 8);
	for (int i = 0; i < (size - 8); i++) {
		m_data[i] = data[i];
	}
}
size_t Extension::rawSize() const {
	return m_data.size();
}
int Extension::padding() const {
	return static_cast<int>(16 - ((m_data.size() + 8) % 16));
}
int Extension::size() const {
	// Must leave 8 bytes for the code and size fields (ints)
	return static_cast<int>(rawSize()) + 8 + padding();
}

int Extension::code() const { return m_code; }
const string &Extension::codeName() const { return CodeName(m_code); }
void Extension::setCode(int code) {
	// Code must be in range and even
	if ((code > NIFTI_ECODE_IGNORE) && (code < NIFTI_MAX_ECODE) && !(code & 1 ))
		m_code = code;
	else
		throw(std::invalid_argument("Invalid Nifti::Extension code."));
}

const vector<char> &Extension::data() const { return m_data; };
void Extension::setData(const vector<char> &data) {
	m_data = data;
}

} // End namespace Nifti
