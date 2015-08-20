/*
 *  fid.h
 *  Nrecon
 *
 *  Created by Tobias Wood on 21/11/2012.
 *  Copyright (c) 2012 Tobias Wood. All rights reserved.
 */

#ifndef AGILENT_FIDFILE
#define AGILENT_FIDFILE

#include <iostream>
#include <fstream>
#include <bitset>
#include <complex>

#include "util.h"

using namespace std;

//! Data file header status bits  (0-9)
/* Bits 0-6:  file header and block header status bits (bit 6 = unused) */
#define S_DATA		0x1	/* 0 = no data      1 = data       */
#define S_SPEC		0x2	/* 0 = fid	    1 = spectrum   */
#define S_32		0x4	/* 0 = 16 bit	    1 = 32 bit	   */
#define S_FLOAT 	0x8	/* 0 = integer      1 = fltng pt   */
#define S_COMPLEX	0x10	/* 0 = real	    1 = complex	   */
#define S_HYPERCOMPLEX	0x20	/* 1 = hypercomplex		   */

/* Bits 7-10:  file header status bits (bit 10 = unused) */
#define S_DDR		0x80    /* 0 = not DDR acq  1 = DDR acq    */
#define S_SECND		0x100	/* 0 = first ft     1 = second ft  */
#define S_TRANSF	0x200	/* 0 = regular      1 = transposed */
#define S_3D		0x400	/* 1 = 3D data			   */

/* Bits 11-14:  status bits for processed dimensions (file header) */
#define S_NP		0x800	/* 1 = np  dimension is active	*/
#define S_NF		0x1000	/* 1 = nf  dimension is active	*/
#define S_NI		0x2000	/* 1 = ni  dimension is active	*/
#define S_NI2		0x4000	/* 1 = ni2 dimension is active	*/


/*-------------------------------------------------------
|							|
|     Main data block header status bits  (7-15)	|
|							|
+------------------------------------------------------*/

/* Bit 7 */
#define MORE_BLOCKS	0x80	/* 0 = absent	    1 = present	*/

/* Bits 8-11:  bits 12-14 are currently unused */
#define NP_CMPLX	0x100	/* 1 = complex     0 = real	*/
#define NF_CMPLX	0x200	/* 1 = complex     0 = real	*/
#define NI_CMPLX	0x400	/* 1 = complex     0 = real	*/
#define NI2_CMPLX	0x800   /* 1 = complex     0 = real	*/


/*-----------------------------------------------
|						|
|  Main data block header mode bits  (0-13)	|
|						|
+----------------------------------------------*/

/* Bits 0-3:  bit 3 is used for pamode */
#define NP_PHMODE	0x1	/* 1 = ph mode  */
#define NP_AVMODE	0x2	/* 1 = av mode	*/
#define NP_PWRMODE	0x4	/* 1 = pwr mode */
#define NP_PAMODE	0x8	/* 1 = pa mode  */

/* Bits 4-7:  bit 7 is used for pamode */
#define NF_PHMODE	0x10	/* 1 = ph mode	*/
#define NF_AVMODE	0x20	/* 1 = av mode	*/
#define NF_PWRMODE	0x40	/* 1 = pwr mode	*/
#define NF_PAMODE	0x80	/* 1 = pa mode  */

/* Bits 8-10 */
#define NI_PHMODE	0x100	/* 1 = ph mode	*/
#define NI_AVMODE	0x200	/* 1 = av mode	*/
#define NI_PWRMODE	0x400	/* 1 = pwr mode	*/

/* Bits 11-13:  bits 14 and 15 are used for pamode */
#define NI2_PHMODE	0x800	/* 1 = ph mode	*/
#define NI2_AVMODE	0x1000	/* 1 = av mode	*/
#define NI2_PWRMODE	0x2000	/* 1 = pwr mode	*/

/* Bits 14-15 */
#define NI_PAMODE	0x4000	/* 1 = pa mode  */
#define NI2_PAMODE	0x8000	/* 1 = pa mode  */

/*-------------------------------
|				|
|    Software Version Status 	|
|				|
+------------------------------*/

/* Bits 0-5:  31 different software versions; 32-63 are for 3D */

/*-------------------------------
|				|
|	 File ID Status		|
|				|
+------------------------------*/

/* Bits 6-10:  31 different file types (64-1984 in steps of 64) */
#define FID_FILE	0x40
#define DATA_FILE	0x80
#define PHAS_FILE	0xc0

namespace Agilent {

class FIDFile {

	private:
		//! Used at the beginning of each data file (fid's, spectra, 2D)
		typedef struct datafilehead {
		   int     nblocks;      //!< Number of blocks in file
		   int     ntraces;      //!< Number of traces per block
		   int     np;           //!< Number of elements per trace
		   int     ebytes;       //!< Number of bytes per element
		   int     tbytes;       //!< Number of bytes per trace
		   int     bbytes;       //!< Number of bytes per block
		   short   vers_id;      //!< Software version and file_id status bits
		   short   status;       //!< Status of whole file
		   int	   nbheaders;	 //!< Number of block headers
		} FileHeader;
		
		
		//! Each file block contains the following header
		typedef struct datablockhead {
		   short   scale;	//!< Scaling factor
		   short   status;	//!< Status of data in block
		   short   index;	//!< Block index
		   short   mode;	//!< Mode of data in block
		   int	   ctcount;	//!< ct value for FID
		   float   lpval;	//!< F2 left phase in phasefile
		   float   rpval;	//!< F2 right phase in phasefile
		   float   lvl;		//!< F2 level drift correction
		   float   tlt;		//!< F2 tilt drift correction
		} BlockHeader;
		
		//! Additional datafile block header for hypercomplex 2D data
		typedef struct hypercmplxbhead {
		   short   s_spare1; //!< short word:  spare
		   short   status;	 //!< status word for block header
		   short   s_spare2; //!< short word:  spare
		   short   s_spare3; //!< short word:  spare
		   int	   l_spare1; //!< int word:  spare
		   float   lpval1;	 //!< additional left phase
		   float   rpval1;	 //!< additional right phase
		   float   f_spare1; //!< float word:  spare
		   float   f_spare2; //!< float word:  spare
		} HyperComplexHeader;
		
		static void SwapFileHeader(FileHeader *hdr);
		static void SwapBlockHeader(BlockHeader *hdr);
		
		//! Actual member variables
		ifstream _file;
		int _numBlocks, _numTraces, _numPoints, _numBlockHeaders,
		    _bytesPerPoint, _bytesPerTrace, _bytesPerBlock;
		bitset<16> _status, _version_id;
		bool _swap;
		
	public:
		enum FIDType {
			Float32Type = 0,
			Int32Type,
			Int16Type
		};
		
		FIDFile();
		FIDFile(const string &path);
		~FIDFile();
		
		void open(const string &path);
		
		bool hasData();
		bool isFID();
		const int nBlocks() const; //!< The number of blocks in the file
		const int nTraces() const; //!< The number of traces per block
		const int nPointsPerTrace() const; //!< The number of samples per trace
		const int nPointsPerBlock() const; //!< The number of samples per block
		const int nComplexPerTrace() const; //!< The number of complex points per trace (nPoints / 2)
		const int nComplexPerBlock() const; //!< The number of complex points per block (nPoints / 2)
		FIDType dataType() const; //!< The sample data type
		
		const complex<double> *readBlock(int block);
		
		const string print_header() const;
};

}; // End namespace Agilent

#endif /* defined(__Nrecon__fid__) */
