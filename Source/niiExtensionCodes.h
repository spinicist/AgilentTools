/** \file Header.h
 \brief Nifti Extension Codes. The user is more likely to need these, so they are made available separately to the other NIFTI_... defines
 - Written by Tobias Wood, IoP KCL
 - This code is released to the public domain. Do with it what you will.
 */
#ifndef LIBNIFTI_EXTENSIONCODES
#define LIBNIFTI_EXTENSIONCODES

#pragma mark Extension Codes
/* NIfTI-1.1 extension codes: see http://nifti.nimh.nih.gov/nifti-1/documentation/faq#Q21 */

#define NIFTI_ECODE_IGNORE           0  /* changed from UNKNOWN, 29 June 2005 */
#define NIFTI_ECODE_DICOM            2  /* intended for raw DICOM attributes  */
#define NIFTI_ECODE_AFNI             4  /* Robert W Cox: rwcox@nih.gov http://afni.nimh.nih.gov/afni */
#define NIFTI_ECODE_COMMENT          6  /* plain ASCII text only              */
#define NIFTI_ECODE_XCEDE            8  /* David B Keator: dbkeator@uci.edu http://www.nbirn.net/Resources/Users/Applications//xcede/index.htm */
#define NIFTI_ECODE_JIMDIMINFO      10  /* Mark A Horsfield:mah5@leicester.ac.uk http://someplace/something         */
#define NIFTI_ECODE_WORKFLOW_FWDS   12  /* Kate Fissell: fissell@pitt.edu http://kraepelin.wpic.pitt.edu/~fissell/NIFTI_ECODE_WORKFLOW_FWDS/NIFTI_ECODE_WORKFLOW_FWDS.html   */
#define NIFTI_ECODE_FREESURFER      14  /* http://surfer.nmr.mgh.harvard.edu  */
#define NIFTI_ECODE_PYPICKLE        16  /* embedded Python objects http://niftilib.sourceforge.net/pynifti                     */
/* LONI MiND codes: http://www.loni.ucla.edu/twiki/bin/view/Main/MiND */
#define NIFTI_ECODE_MIND_IDENT      18  /* Vishal Patel: vishal.patel@ucla.edu*/
#define NIFTI_ECODE_B_VALUE         20
#define NIFTI_ECODE_SPHERICAL_DIRECTION 22
#define NIFTI_ECODE_DT_COMPONENT    24
#define NIFTI_ECODE_SHC_DEGREEORDER 26  /* end LONI MiND codes                */
#define NIFTI_ECODE_VOXBO           28  /* Dan Kimberg: www.voxbo.org         */
#define NIFTI_ECODE_CARET           30  /* John Harwell: john@brainvis.wustl.edu http://brainvis.wustl.edu/wiki/index.php/Caret:Documentation:CaretExtension */
#define NIFTI_MAX_ECODE             30  /******* maximum extension code *******/
#define LNI_MAX_NIA_EXT_LEN 100000  /* consider a longer extension invalid */

#endif
