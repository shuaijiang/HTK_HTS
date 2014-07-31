/*  ---------------------------------------------------------------  */
/*      The HMM-Based Speech Synthesis System (HTS): version 1.1.1   */
/*                        HTS Working Group                          */
/*                                                                   */
/*                   Department of Computer Science                  */
/*                   Nagoya Institute of Technology                  */
/*                                and                                */
/*    Interdisciplinary Graduate School of Science and Engineering   */
/*                   Tokyo Institute of Technology                   */
/*                      Copyright (c) 2001-2003                      */
/*                        All Rights Reserved.                       */
/*                                                                   */
/*  Permission is hereby granted, free of charge, to use and         */
/*  distribute this software and its documentation without           */
/*  restriction, including without limitation the rights to use,     */
/*  copy, modify, merge, publish, distribute, sublicense, and/or     */
/*  sell copies of this work, and to permit persons to whom this     */
/*  work is furnished to do so, subject to the following conditions: */
/*                                                                   */
/*    1. The code must retain the above copyright notice, this list  */
/*       of conditions and the following disclaimer.                 */
/*                                                                   */
/*    2. Any modifications must be clearly marked as such.           */
/*                                                                   */
/*  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF TECHNOLOGY,  */
/*  HTS WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM    */
/*  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       */
/*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF        */
/*  TECHNOLOGY, HTS WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE    */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        */
/*  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  */
/*  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   */
/*  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          */
/*  PERFORMANCE OF THIS SOFTWARE.                                    */
/*                                                                   */
/*  ---------------------------------------------------------------  */
/*     HGen.h : parameter generation from pdf sequence based on      */
/*              Maximum Likelihood criterion with dynamic feature    */
/*              window constraints                                   */
/*                                                                   */
/*                                    2003/12/26 by Heiga Zen        */
/*  ---------------------------------------------------------------  */


/* !HVER!HGen:   1.1.1 [NIT 26/12/03] */

#ifndef _HGEN_H_
#define _HGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WLEFT    0
#define WRIGHT   1
#define MAXDNUM 20   /* maximum number of static + deltas */

typedef struct {
   char *fn[MAXDNUM];  /* delta window coefficient file */
   int num;            /* number of static + deltas */
   int max_L;
   int maxw[2];        /* max width [0(left) 1(right)] */
   int  **width;       /* width [0..num-1][0(left) 1(right)] */
   double **coef;      /* coefficient [1..num][length[1]..length[2]] */
} DWin;

typedef struct {
   SVector *mseq;     /* sequence of mean vector */
   SVector *ivseq;    /* sequence of invarsed variance vector */
   Matrix C;          /* generated parameter c */
   DVector g;
   DMatrix WUW;
   DVector WUM;
} SMatrices;

typedef struct {
   int vSize;
   int order;
   int T;
   int width;
   DWin dw;
   SMatrices sm;
} PStream;

/* ---------------- parameter generation from pdf sequence ------------------- */

void InitPStream(MemHeap *x, PStream *);
/*
   Initialise PStream for parameter generation
*/

void pdf2par(PStream *);
/*
   parameter generation from pdf sequence 
*/

void FreePStream(MemHeap *x, PStream *);
/*
   free memory for parameter generation 
*/
   
#ifdef __cplusplus
}
#endif

#endif  /* _HGEN_H_ */

/* ------------------------- End of HGen.h --------------------------- */
