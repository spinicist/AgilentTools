//
//  fdf.h
//  Agilent
//
//  Created by Tobias Wood on 23/08/2013.
//
//

#ifndef AGILENT_FDF
#define AGILENT_FDF

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <exception>
#include <memory>

#include <dirent.h>

#include "Eigen/Geometry"

#include "fdfFile.h"
#include "procpar.h"

using namespace std;
using namespace Eigen;

namespace Agilent {

class fdfImage {
	public:
		enum class OpenMode : size_t {
			Read
		};
		
	protected:
		string m_folderPath, m_filePrefix, m_dtype;
		ProcPar m_pp;
		size_t m_rank, m_slabs, m_echoes, m_images;
		Array<size_t, 3, 1> m_dim; // x y z
		Array3d m_voxdim;
		map<string, shared_ptr<fdfFile>> m_files;
		Affine3d m_transform;
		const string filePath(const size_t slice, const size_t image, const size_t echo) const;
		
	public:
		fdfImage();
		fdfImage(const string &path);
		
		void open(const string &path);
		void close();
		
		const size_t dim(const size_t d) const;    //!< Return a particular dimension
		const double voxdim(const size_t d) const; //!< Return a particular voxel size
		const Array<size_t, 3, 1> &dims() const;   //!< Return the whole dimension array
		const Array3d &voxdims() const;            //!< Return all the voxel sizes
		const size_t voxelsPerSlice() const;
		const size_t voxelsPerVolume() const;
		const Affine3d &transform() const;
		
		template<typename T>
		vector<T> readVolume(const size_t vol, const size_t echo = 0) {
			vector<T> Tbuffer(voxelsPerVolume());
			for (size_t sl = 0; sl < m_slabs; sl++) { // Slice or slab
				string name = filePath(sl, vol, echo);
				auto file = m_files.find(name);
				if (file == m_files.end())
					throw(runtime_error("Could not find file: " + name));
				vector<T> Tsl = file->second->readData<T>();
				size_t offset = Tsl.size() * sl;
				for (size_t i = 0; i < Tsl.size(); i++)
					Tbuffer[offset + i] = Tsl[i];
			}
			return Tbuffer;
		}
};

} // End namespace Agilent

#endif
