/*
 *  MultiArray.h
 *  Part of the QUantitative Image Toolbox
 *
 *  Copyright (c) 2014 Tobias Wood. All rights reserved.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef QUIT_MULTIARRAY_H
#define QUIT_MULTIARRAY_H

#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <exception>
#include <stdexcept>

#include "Eigen/Core"
#include "Eigen/Geometry"

namespace QUIT {

template<typename Tp, size_t rank>
class MultiArray {
	public:
		typedef Eigen::Array<size_t, rank, 1> Index;
		typedef Eigen::Array<size_t, rank - 1, 1> SmallIndex;
		typedef std::vector<Tp> StorageTp;
		typedef std::shared_ptr<StorageTp> PtrTp;
		typedef Eigen::Map<Eigen::Array<Tp, Eigen::Dynamic, Eigen::Dynamic>, Eigen::Unaligned, Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>> MapTp;
		// These typedefs are for STL Iterator compatibility
		typedef Tp        value_type;
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef typename StorageTp::const_reference const_reference;
		typedef typename StorageTp::reference       reference;
		
		class MultiArrayIterator {
			public:
				typedef std::forward_iterator_tag iterator_category;
				typedef Tp        value_type;
				typedef size_t    size_type;
				typedef ptrdiff_t difference_type;
				typedef Tp        *pointer;
				typedef const Tp  *const_pointer;
				typedef Tp        &reference;
				typedef const Tp  &const_reference;

			private:
				MultiArray &m_array;
				Index m_voxelIndex;
				size_t m_dataIndex;
				bool m_endflag;

			public:
				MultiArrayIterator(MultiArray &array, Index start = Index::Zero());
				Tp &operator*();

				MultiArrayIterator &operator++();
				MultiArrayIterator operator++(int);

				bool operator==(const MultiArrayIterator &other) const;
				bool operator!=(const MultiArrayIterator &other) const;
		};
		typedef MultiArrayIterator iterator;

		static const size_t MaxIndex{std::numeric_limits<size_t>::max()};
	protected:
		PtrTp  m_ptr;
		size_t m_offset;
		Index  m_dims, m_strides;
		bool   m_packed;
		
		static Index CalcStrides(const Index &dims);
	public:
		MultiArray();
		MultiArray(const Index &dims);
		MultiArray(const Index &dims, const PtrTp &ptr, const Index &strides = Index::Zero(), const size_t offset = 0);
		MultiArray(const SmallIndex &dims, const size_t finalDim);

		const Index &dims() const;
		const Index &strides() const;
		size_t size() const;
		bool isPacked() const;
		MultiArray<Tp, rank> pack() const; //!< If the multi-array is not packed, create a new one and copy data to it.
		void resize(const Index &newDims);
		template<size_t newRank> MultiArray<Tp, newRank> reshape(const typename MultiArray<Tp, newRank>::Index &newDims);
		template<size_t newRank> MultiArray<Tp, newRank> slice(const Index &start, const Index &size, const Index &strides = Index::Ones()) const;
		MapTp asArray() const;

		// STL-like interface
		const_reference operator[](const size_t i) const;
		const_reference operator[](const Index &vox) const;
		reference operator[](const size_t i);
		reference operator[](const Index &vox);
		
		iterator begin();
		iterator end();

		std::string print() const;
		friend std::ostream &operator<<(std::ostream &os, const MultiArray &v) {
			os << v.print();
			return os;
		}
};

// Template definitions
#include "MultiArray-inl.h"

} // End namespace QUIT

#endif //MULTIARRAY_H
