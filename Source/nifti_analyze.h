//
//  nifti_analyze.h
//  NiftiImage
//
//  Created by Tobias Wood on 09/07/2013.
//  Copyright (c) 2013 Tobias Wood. All rights reserved.
//

#ifndef NIFTI_ANALYZE
#define NIFTI_ANALYZE

/*! \enum analyze_75_orient_code
 *  \brief Old-style analyze75 orientation
 *         codes.
 */
typedef enum _analyze75_orient_code {
	a75_transverse_unflipped = 0,
	a75_coronal_unflipped = 1,
	a75_sagittal_unflipped = 2,
	a75_transverse_flipped = 3,
	a75_coronal_flipped = 4,
	a75_sagittal_flipped = 5,
	a75_orient_unknown = 6
} analyze_75_orient_code;

/*****************************************************************************/
/*------------------ NIfTI version of ANALYZE 7.5 structure -----------------*/

/* (based on fsliolib/dbh.h, but updated for version 7.5) */

typedef struct {
	/* header info fields - describes the header    overlap with NIfTI */
	/*                                              ------------------ */
	int sizeof_hdr;                  /* 0 + 4        same              */
	char data_type[10];              /* 4 + 10       same              */
	char db_name[18];                /* 14 + 18      same              */
	int extents;                     /* 32 + 4       same              */
	short int session_error;         /* 36 + 2       same              */
	char regular;                    /* 38 + 1       same              */
	char hkey_un0;                   /* 39 + 1                40 bytes */
	
	/* image dimension fields - describes image sizes */
	short int dim[8];                /* 0 + 16       same              */
	short int unused8;               /* 16 + 2       intent_p1...      */
	short int unused9;               /* 18 + 2         ...             */
	short int unused10;              /* 20 + 2       intent_p2...      */
	short int unused11;              /* 22 + 2         ...             */
	short int unused12;              /* 24 + 2       intent_p3...      */
	short int unused13;              /* 26 + 2         ...             */
	short int unused14;              /* 28 + 2       intent_code       */
	short int datatype;              /* 30 + 2       same              */
	short int bitpix;                /* 32 + 2       same              */
	short int dim_un0;               /* 34 + 2       slice_start       */
	float pixdim[8];                 /* 36 + 32      same              */
	
	float vox_offset;                /* 68 + 4       same              */
	float funused1;                  /* 72 + 4       scl_slope         */
	float funused2;                  /* 76 + 4       scl_inter         */
	float funused3;                  /* 80 + 4       slice_end,        */
	/* slice_code,       */
	/* xyzt_units        */
	float cal_max;                   /* 84 + 4       same              */
	float cal_min;                   /* 88 + 4       same              */
	float compressed;                /* 92 + 4       slice_duration    */
	float verified;                  /* 96 + 4       toffset           */
	int glmax,glmin;                 /* 100 + 8              108 bytes */
	
	/* data history fields - optional */
	char descrip[80];                /* 0 + 80       same              */
	char aux_file[24];               /* 80 + 24      same              */
	char orient;                     /* 104 + 1      NO GOOD OVERLAP   */
	char originator[10];             /* 105 + 10     FROM HERE DOWN... */
	char generated[10];              /* 115 + 10                       */
	char scannum[10];                /* 125 + 10                       */
	char patient_id[10];             /* 135 + 10                       */
	char exp_date[10];               /* 145 + 10                       */
	char exp_time[10];               /* 155 + 10                       */
	char hist_un0[3];                /* 165 + 3                        */
	int views;                       /* 168 + 4                        */
	int vols_added;                  /* 172 + 4                        */
	int start_field;                 /* 176 + 4                        */
	int field_skip;                  /* 180 + 4                        */
	int omax, omin;                  /* 184 + 8                        */
	int smax, smin;                  /* 192 + 8              200 bytes */
} nifti_analyze75;                                   /* total:  348 bytes */

#endif
