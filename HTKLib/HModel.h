/* ----------------------------------------------------------- */
/*                                                             */
/*                          ___                                */
/*                       |_| | |_/   SPEECH                    */
/*                       | | | | \   RECOGNITION               */
/*                       =========   SOFTWARE                  */ 
/*                                                             */
/*                                                             */
/* ----------------------------------------------------------- */
/* developed at:                                               */
/*                                                             */
/*      Speech Vision and Robotics group                       */
/*      Cambridge University Engineering Department            */
/*      http://svr-www.eng.cam.ac.uk/                          */
/*                                                             */
/*      Entropic Cambridge Research Laboratory                 */
/*      (now part of Microsoft)                                */
/*                                                             */
/* ----------------------------------------------------------- */
/*         Copyright: Microsoft Corporation                    */
/*          1995-2000 Redmond, Washington USA                  */
/*                    http://www.microsoft.com                 */
/*                                                             */
/*               2002 Cambridge University                     */
/*                    Engineering Department                   */
/*                                                             */
/*   Use of this software is governed by a License Agreement   */
/*    ** See the file License for the Conditions of Use  **    */
/*    **     This banner notice must not be removed      **    */
/*                                                             */
/* ----------------------------------------------------------- */
/*         File: HModel.h  HMM Model Definition Data Type      */
/* ----------------------------------------------------------- */


/* *** THIS IS A MODIFIED VERSION OF HTK ***                        */
/* ---------------------------------------------------------------- */
/*                                                                  */
/*     The HMM-Based Speech Synthesis System (HTS): version 1.0     */
/*            HTS Working Group                                     */
/*                                                                  */
/*       Department of Computer Science                             */
/*       Nagoya Institute of Technology                             */
/*                and                                               */
/*   Interdisciplinary Graduate School of Science and Engineering   */
/*       Tokyo Institute of Technology                              */
/*          Copyright (c) 2001-2002                                 */
/*            All Rights Reserved.                                  */
/*                                                                  */
/* Permission is hereby granted, free of charge, to use and         */
/* distribute this software in the form of patch code to HTK and    */
/* its documentation without restriction, including without         */
/* limitation the rights to use, copy, modify, merge, publish,      */
/* distribute, sublicense, and/or sell copies of this work, and to  */
/* permit persons to whom this work is furnished to do so, subject  */
/* to the following conditions:                                     */
/*                                                                  */
/*   1. Once you apply the HTS patch to HTK, you must obey the      */
/*      license of HTK.                                             */
/*                                                                  */
/*   2. The code must retain the above copyright notice, this list  */
/*      of conditions and the following disclaimer.                 */
/*                                                                  */
/*   3. Any modifications must be clearly marked as such.           */
/*                                                                  */
/* NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF TECHNOLOGY,   */
/* HTS WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM    */
/* ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/* SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF         */
/* TECHNOLOGY, SPTK WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE   */
/* FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        */
/* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  */
/* WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   */
/* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          */
/* PERFORMANCE OF THIS SOFTWARE.                                    */
/*                                                                  */
/* ---------------------------------------------------------------- */ 
/*     HModel.h modified for HTS-1.0 2002/12/20 by Heiga Zen        */
/* ---------------------------------------------------------------- */

/* !HVER!HModel:   3.2 [CUED 09/12/02] */

#ifndef _HMODEL_H_
#define _HMODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 
   The following types define the in-memory representation of a HMM.
   All HMM's belong to a HMMSet which includes a macro table for
   rapidly mapping macro/hmm names into structures.  
*/

#define MACHASHSIZE 1277   /* Size of each HMM Set macro hash table */
#define PTRHASHSIZE  513   /* Size of each HMM Set ptr map hash table */
#define MINMIX  1.0E-5     /* Min usable mixture weight */
#define LMINMIX -11.5     /* log(MINMIX) */

#define MINDLOGP 0.000001  /* prob = exp(shortform/DLOGSCALE) */
#define DLOGSCALE -2371.8  /* = 32767/ln(MINDLOGP) */
#define DLOGZERO 32767  

#define MixWeight(hset,weight) (weight)
#define MixLogWeight(hset,weight) (weight<MINMIX ? LZERO : log(weight))
#define MixFloor(hset)            ( MINMIX )


/* ------------------ Master Model File Info ----------------- */

typedef struct _MMFInfo *MILink;

typedef struct _MMFInfo{
   Boolean isLoaded;       /* true if contents are loaded */
   char *fName;            /* MMF file name */
   int fidx;               /* MMF file index */
   MILink next;            /* next external file name in list */
} MMFInfo;

/* -------------------- HMM Definition ----------------------- */


enum _DurKind {NULLD, POISSOND, GAMMAD, RELD, GEND, GAUSSD};
typedef enum _DurKind DurKind;

enum _HSetKind {PLAINHS, SHAREDHS, TIEDHS, DISCRETEHS};
typedef enum _HSetKind HSetKind;

typedef struct {
   SVector mean;        /* mean vector */
   CovKind ckind;       /* kind of covariance */
   Covariance cov;      /* covariance matrix or vector */
   float gConst;        /* Precomputed component of b(x) */
   short rClass;        /* regression base class number (zero if unused) */
   int mIdx;            /* MixPDF index */
   int nUse;            /* usage counter */
   Ptr hook;            /* general hook */
} MixPDF;

typedef struct {        /* 1 of these per mixture per stream */
   float weight;        /* mixture weight */
   MixPDF *mpdf;        /* -> mixture pdf */
} MixtureElem;

typedef union {         /* array[1..numMixtures] of Mixture */
   MixtureElem *cpdf;    /* PLAINHS or SHAREDHS */
   Vector tpdf;          /* TIEDHS */
   ShortVec dpdf;        /* DISCRETE */
} MixtureVector; 

typedef struct{         /* used for tied mixture prob calculations */
   short index;          /* mixture index */
   float prob;           /* mixture prob scaled by maxP */
}TMProb;

typedef struct {        /* A Tied Mixture "Codebook" */
   LabId mixId;          /* id of macro base name */
   short nMix;           /* num mixtures M in set */
   short topM;           /* num TMProbs actually used */
   MixPDF ** mixes;      /* array[1..M] of MixPDF */
   LogFloat maxP;        /* max log mixture prob */
   TMProb *probs;        /* array[1..M] of TMProb */
} TMixRec;

typedef struct {        /* 1 of these per stream */
   int nMix;            /* num mixtures in this stream */
   short stream;        /* position of stream of this stream info */
   MixtureVector spdf;  /* Mixture Vector */
   int pIdx;            /* Strean index */
   int nUse;            /* usage counter */
   Ptr hook;            /* general hook */
}StreamInfo;

typedef struct {        /* 1 of these per stream */
   StreamInfo *info;    /* information for this stream */
} StreamElem;

typedef struct {
   SVector weights;     /* vector of stream weights */
   StreamElem *pdf;     /* array[1..numStreams] of StreamElem */
   SVector dur;         /* vector of state duration params, if any */   
   int sIdx;            /* State index */
   int nUse;            /* usage counter */
   Ptr hook;            /* general hook */
   int stateCounter;    /* # of state occurrences */
} StateInfo;

typedef struct {        /* 1 of these per state */
   StateInfo *info;     /* information for this state */
} StateElem;

typedef struct {
   struct _HMMSet *owner;  /* owner of this model */
   short numStates;        /* includes entry and exit states */
   StateElem *svec;        /* array[2..numStates-1] of StateElem */  
   SVector dur;            /* vector of model duration params, if any */   
   SMatrix transP;         /* transition matrix (logs) */
   int tIdx;               /* Transition matrix index */
   int nUse;               /* num logical hmm's sharing this def */
   Ptr hook;               /* general hook */
} HMMDef;

typedef HMMDef * HLink;

/* ---------------------- Input Transform  ------------------- */

typedef struct {
   int vecSize;         /* must be matched to a stream width! */
   IntVec blockSize;    /* block sizes in the linear transform */
   SMatrix* xform;      /* 1..numBlocks matrix transforms */
   SVector bias;        /* bias vector, if no bias equals NULL */
   float det;           /* determinant of linxform */
   int nUse;            /* usage counter */
} LinXForm;

typedef struct {
   char* xformName;     /* name of the transform (macroname) */
   char* mmfIdMask;     /* mask of model sets that appropriate for */
   char *fname;         /* filename of where the input xform was loaded */
   ParmKind pkind;      /* parameter kind for xform to be applied to */
   Boolean preQual;     /* is this applied prior to qualifiers? */
   LinXForm *xform;     /* actual transform to be applied */
   int nUse;            /* usage counter */
} InputXForm;

/* ---------------------- Macros/HMM Hashing ------------------- */

/* 
   Every macro, logical HMM defn and physical HMM defn has an
   entry in a macro table.  The macro types are:   
     l logHMM     u mean      v variance  i invcovar  p pdf
     h phyHMM     d duration  t transP    m mixpdf    s state
     x xform      w strm wts  o options   c lltcovar  * deleted
     r regtree
   a HMMDef will have exactly 1 phyHMM macro referencing it but it can 
   have 0 or more logHMM macros referencing it.
*/

typedef struct _MacroDef *MLink;

typedef struct _MacroDef{
   MLink next;             /* next cell in hash table */
   char type;              /* type of macro [hluvixdtmps*] */
   short fidx;             /* idx of MMF file (0 = SMF) */
   LabId id;               /* name of macro */
   Ptr structure;          /* -> shared structure or HMM Def */
   Ptr hook;               /* general hook */
} MacroDef;

typedef struct _PtrMap {   /* used for finding macros via ptr's */
   struct _PtrMap *next;   /* next cell in hash table */
   Ptr ptr;                /* the structure */
   MLink m;                /* macro def for this structure */
} PtrMap;

/* ---------------------- HMM Sets ----------------------------- */

typedef struct _HMMSet{
   MemHeap *hmem;          /* memory heap for this HMM Set */   
   Boolean *firstElem;     /* first element added to hmem during MakeHMMSet*/
   char *hmmSetId;         /* identifier for the hmm set */
   MILink mmfNames;        /* List of external file names */
   int numLogHMM;          /* Num of logical HMM's */
   int numPhyHMM;          /* Num of distinct physical HMM's */
   int numFiles;           /* total number of ext files */
   int numMacros;          /* num macros used in this set */
   MLink * mtab;           /* Array[0..MACHASHSIZE-1]OF MLink */
   PtrMap ** pmap;         /* Array[0..PTRHASHSIZE-1]OF PtrMap* */
   Boolean allowTMods;     /* true if HMMs can have Tee Models */
   Boolean optSet;         /* true if global options have been set */
   short vecSize;          /* dimension of observation vectors */
   short swidth[SMAX];     /* [0]=num streams,[i]=width of stream i */
   short msdflag[SMAX];    /* flag for multi-space probability density */
   ParmKind pkind;         /* kind of obs vector components */
   DurKind dkind;          /* kind of duration model (model or state) */
   CovKind ckind;          /* cov kind - only global in V1.X */
   HSetKind hsKind;        /* kind of HMM set */
   TMixRec tmRecs[SMAX];   /* array[1..S]of tied mixture record */
   int numStates;          /* Number of states in HMMSet */
   int numSharedStates;    /* Number of shared states in HMMSet */
   int numStreams;         /* Number of streams in HMMSet */
   int numSharedStreams;   /* Number of shared streams in HMMSet */
   int numMix;             /* Number of mixture components in HMMSet */
   int numSharedMix;       /* Number of shared mixtures in HMMSet */
   int numTransP;          /* Number of distinct transition matrices */
   int ckUsage[NUMCKIND];  /* Number of components using given ckind */
   InputXForm *xf;         /* Input transform of HMMSet */
} HMMSet;

/* ---------------------- MSD Infomation ----------------------- */

/* Multi-Space probability Density information */
typedef struct _SpaceInfo {
   int order;                  /* order of space */
   int count;                  /* number of spaces with same order*/
   IntVec sindex;              /* space index */
   struct _SpaceInfo *next;            /* link to next SpaceInfo */
} SpaceInfo;

typedef struct _MSDInfo {
   int nSpace;                 /* number of space (= Mixture) */
   int nKindS;                 /* number of kind of space */
   IntVec sorder;              /* order of space sorder[1..nSpace] */
   SpaceInfo *next;            /* link to head SpaceInfo */
} MSDInfo;

/* --------------------------- Initialisation ---------------------- */

void InitModel(void);
/*
   Initialise the module
*/

/* ---------------- Macro Related Manipulations -------------------- */

void QuantiseObservation(HMMSet *hset, Observation *obs, int frame);


MLink NewMacro(HMMSet *hset, short fidx, char type, LabId id, Ptr structure);
/*
   Create a new macro definition for given HMM set with given values
   and insert it into the associated macro table.  Return a pointer to
   it.
*/

void DeleteMacro(HMMSet *hset, MLink p);
void DeleteMacroStruct(HMMSet *hset, char type, Ptr structure);
/* 
   Mark macro definition [for given structure] as deleted (ie type='*')
*/

MLink FindMacroName(HMMSet *hset, char type, LabId id);
/*
   Find the macro def of given name in hset.  Return NULL if 
   not found.  This uses a fast hashtable lookup.
*/

MLink FindMacroStruct(HMMSet *hset, char type, Ptr structure);
/* 
   Return macro definition for given structure. The first
   request for this structure requires a slow exhaustive 
   search through whole macro table.  The ptr is then added
   to a hashtable stored in hset so that subsequent
   requests are fast.
*/

Boolean HasMacros(HMMSet *hset, char * types);
/*
   Returns true if shared structure macros (other than regular
   hmmdefs) have been used in any HMM in set.  If types is not NULL,
   then a string containing all the different macro types is also
   returned.
*/

void SetVFloor(HMMSet *hset, Vector *vFloor, float minVar);
/*
   Initialise a set of variance floor vectors.  Marks varFloor vectors
   as used then if vFloor!=NULL and macro ~v "varFloorN"
   exists then vFloor[N] is set to this vector; otherwise a vector of
   length hset->swidth[s] is created in vFloor[s] and all its
   components are set to minVar.
*/

void ApplyVFloor(HMMSet *hset);
/* 
   Apply the variance floors in hset to all covariances in the model set 
 */

void PrintHMMProfile(FILE *f, HLink hmm);
/*
   Print a profile (num states, streams, mixes, etc) to f of
   given HMM definition.
*/

void PrintHSetProfile(FILE *f, HMMSet *hset);
/*
   Print a profile to f of given HMM set.
*/

/* ------------------- HMM Set Load/Store ---------------------- */

/* the basic sequence needed to create a HMM set is
      CreateHMMSet {AddMMF} (MakeHMMSet | MakeOneHMM) LoadHMMSet
*/

void CreateHMMSet(HMMSet *hset, MemHeap *heap, Boolean allowTMods);
/* 
   Create a HMMSet using given heap.  This routine simply
   initialises the basic HMMSet structure. It must be followed by
   a call to either MakeOneHMM or MakeHMMSet.  
*/

MILink AddMMF(HMMSet *hset, char *fname);
/*
   Add MMF file to HMM set (must not be already in MMF list).  This is
   mainly used for pre-specifying definition files but it can also be
   used to force loading of supplementary information coded as macros
   such as variance floors.  Returns a pointer to added MMFInfo
   record.  AddMMF can be used before or after a call to MakeOneHMM or 
   MakeHMMSet.  However, it must not be called after LoadHMMSet.
*/

ReturnStatus MakeHMMSet(HMMSet *hset, char *fname);
/*
   Make a HMMSet by reading the file fname.  Each line of fname
   should contain the logical name of a HMM optionally followed by a
   physical name.  If no physical name is given it is assumed to be
   the same as the logical name.  This routine creates the macro hash
   table, macro definitions for each HMM and HMMDef structures.  It
   does NOT load the actual HMM definitions.
*/

ReturnStatus MakeOneHMM(HMMSet *hset, char *hname);
/*
   As above but hname is a single HMM definition which is used
   to create a singleton HMM set
*/

ReturnStatus LoadHMMSet(HMMSet *hset, char *hmmDir, char *hmmExt);
/*
   Load any preloaded MMF files.  Scan the physical list of hset and
   for each HMM def which is still unloaded, the physical name is
   modified by the given hmmDir and hmmExt, if any, and the resulting
   file is loaded.
*/

void ResetHMMSet(HMMSet *hset);
/*
  Returns HMMSet to state it was in after calling CreateHMMSet. 
  Frees memory on hmem, (and logicalHeap if HMM_STORE enabled) etc.
*/

void SaveInOneFile(HMMSet *hset, char *fname);
/*
   Called before SaveHMMSet to ignore all original file names and store
   the entire HMM set in a single file.
*/

ReturnStatus SaveHMMSet(HMMSet *hset, char *hmmDir, char *hmmExt, Boolean binary);
/*
   Store the given HMM set.  Each HMM def and macro is stored in the
   same file as it was loaded from except that if hmmDir is specified
   then it replaces the original location of the file.  In the case of
   individual HMM files only, the original extension if any is
   replaced by the given hmmExt if any.  If the HMM set is TIEDHS or
   DISCRETEHS then the mix weights are stored in a compact form.  If
   binary is set then all output uses compact binary mode.
*/

ReturnStatus SaveHMMList(HMMSet *hset, char *fname);
/*
   Save a HMM list in fname describing given HMM set 
*/


/* -------------- Shared Structure "Seen" Flags -------------- */

/* When scanning HMM sets, it is convenient to mark certain objects
   as "seen".  The nUse and nMix fields are used for this.  If either
   is -ve then the attached object seen. */
Boolean IsSeen(int flag);
/*
   return true if flag is set (ie. -ve)
*/

void Touch(int *flag);
void Untouch(int *flag);
/*
   set or clear given flag (see also TouchV/UntouchV in HMem)
*/

typedef enum {CLR_HMMS,CLR_STATES,CLR_STREAMS,CLR_ALL} ClearDepth;

void ClearSeenFlags(HMMSet *hset, ClearDepth depth);
/*
   Clear all the seen flags in hset (ie all nUse and nMix fields)
   downto given level.
*/

/* ----------------- General HMM Operations --------------------- */

int MaxMixtures(HLink hmm);
int MaxMixInSet(HMMSet *hset);
/*
   Returns max number of mixtures in given HMM or set of HMMs
*/

int MaxMixInS(HLink hmm, int s);
int MaxMixInSetS(HMMSet *hset, int s);
/*
   Returns max number of mixtures in given stream in given HMM or set of HMMs
*/

int MaxStatesInSet(HMMSet *hset);
/*
   Returns max number of states in any HMM in set of HMMs
*/

char *DurKind2Str(DurKind dkind, char *buf);
char *CovKind2Str(CovKind ckind, char *buf);
/*
   Utility routines to convert enum types to strings.  In each case,
   string is stored in *buf and pointer to buf is returned
*/

/* -------------------- Input Transform Operations ---------------- */

InputXForm *LoadInputXForm(HMMSet *hset, char* macroname, char* fname);
/* 
   Loads, or returns, the specified transform 
*/

void SaveInputXForm(HMMSet *hset, InputXForm *xf, char *fname, Boolean binary);
/* 
   Saves an individual transform 
*/

/* ------------- HMM Output Probability Calculations --------------- */

void PrecomputeTMix(HMMSet *hset, Observation *x, float tmThresh, int topM);
/*
   Precompute the tied mixture probs stored in hset->tmRecs.  The
   array of mix probs is scaled by the max prob and sorted.  If topM>0
   then it is set in each stream, otherwise all mixes within tmThresh
   of the max are used
*/


   
LogFloat  OutP(Observation *x, HLink hmm, int state);
LogFloat POutP(HMMSet *hset, Observation *x, StateInfo *si);
/*
   Return the log output probability of observation x in given
   state of given model
*/

LogFloat SOutP(HMMSet *hset, int s, Observation *x, StreamInfo *sti);
/*
   Return Stream log output prob of stream s of observation x
*/

/*
   Return Mixture log output probability for given vector x.  
*/


LogFloat MOutP(Vector x, MixPDF *mp);
short DProb2Short(float p);
/* 
   Convert prob p to scaled log prob = ln(p)*DLOGSCALE
*/

LogFloat Short2DProb(short s);
/*
   Convert scaled log prob (s) to prob p = exp(s/DLOGSCALE)
*/

void FixFullGConst(MixPDF *mp, LogFloat ldet);
void FixLLTGConst(MixPDF *mp);
void FixInvDiagGConst(MixPDF *mp);
void FixDiagGConst(MixPDF *mp);
/*
   Set the gConst for the specific mixture component.  In the
   case of FullGConst, ldet is passed as a parameter since it
   will often have just been calculated during a cov inversion.
*/

void FixGConsts(HLink hmm);
/*
   Sets all gConst values in the given HMM
*/

void FixAllGConsts(HMMSet *hset);
/*
   Sets all gConst values in all HMMs in set
*/

/* ------------- for Multi-Space probability Density ------------------ */

MSDInfo ***CreateMSDInfo(MemHeap *mem, HLink hmm);
/*
   Create MSD infomation of each stream
*/

int SpaceOrder(Vector vec);
/*
   Count order of Vector which is excepted ignVal
*/

int IncludeSpace(MSDInfo *msdInfo, int order);
/*
   Search space in MSD information
*/

int NumNonZeroSpace(StreamInfo *sti);
/*
   NumNonZeroSpace: Return the number of space which order is not zero 
*/
 
#ifdef __cplusplus
}
#endif

#endif  /* _HMODEL_H_ */

/* ------------------------- End of HModel.h --------------------------- */
