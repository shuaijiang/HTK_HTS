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
/*  HMGenS.c : generate speech parameters (spectrum and f0 ) from   */
/*             HMMs based on Maximum Likelihood criterion with      */
/*             dynamic feature window constraints                   */
/*                                                                  */
/*                                   2002/12/25 by Heiga Zen        */
/* ---------------------------------------------------------------- */

char *hgens_version = "!HVER!HMGenS:   1.0 [zen 25/12/02]";
char *hgens_sccs_id = "@(#)HMGenS.c    1.0 25/12/02 JST";

/* ------------------- HMM ToolKit Modules ------------------- */

#include <HShell.h>
#include <HMem.h>
#include <HMath.h>
#include <HSigP.h>
#include <HAudio.h>
#include <HWave.h>
#include <HVQ.h>
#include <HParm.h>
#include <HLabel.h>
#include <HModel.h>
#include <HTrain.h>
#include <HUtil.h>
#include <HGen.h>

/* ----------------------- Trace Flags ----------------------- */

#define	T_DUR 0002      /* show duration of each state */
#define	T_GEN 0004      /* trace parameter generation */
#define T_DMC 0010      /* show sum mean and sum variance
                            of duration distribution */

/* ---------------- Global Data Structures --------------------- */

static Source source;      /* the current input file */
static MemHeap hmmHeap;    /* Heap holds all hmm related info */
static MemHeap tmpHeap;    /* Heap holds all calcuration info */

static HMMSet hmset;       /* Set of HMMs */
static HMMSet dmset;       /* Set of Duration Models */
static PStream ceppst;       /* PDF Stream for cepstrum */
static PStream f0pst;       /* PDF Stream for F0 */

static Boolean outMean;    /* output mean vector */
static Boolean outPDF;     /* output pdf */
static Boolean phnDur;     /* use phoneme duration */
static float tmpFactor = 0.0; /* control tempo */
static float frmShift = 50000;     /* frame shift */

static ConfParam *cParm[MAXGLOBS];   /* configuration parameters */
static int nParm = 0;               /* total num params */

static char *outDir = NULL;   /* directory to output result */
static char *labDir = NULL;   /* directory to look for label files */
static char *labExt = "lab";  /* label file extension */
static char *hmmExt = NULL;   /* hmm def file extension */
static char *cepExt = "mcep"; /* mel-cepstrum parameter file extension */
static char *f0Ext  = "lf0";  /* log F0 parameter file extension */
static char *durExt = "dur";  /* state duration file extension */
static char *mcpExt = "mcp";  /* mean of melcep parameter file extension */
static char *mptExt = "mf0";  /* mean of f0 parameter file extension */
static char *cpfExt = "pcp";  /* PDF file extension */
static char *fpfExt = "pf0";  /* PDF file extension */

static int trace = 0;           /* trace level */
static int total_frame = 0;

static Boolean LoadModel = FALSE;
static Boolean SetOrder  = FALSE;

/* --------------------- Process Command Line ---------------------- */

void SetConfParms(void)
{
   Boolean b;
   double d;
   
   nParm = GetConfig("HMGENS", TRUE, cParm, MAXGLOBS);
   if (nParm>0) {
      if (GetConfBool(cParm,nParm,"OUTMEAN",&b))   outMean = TRUE;
      if (GetConfBool(cParm,nParm,"OUTPDF",&b))    outPDF = TRUE;
      if (GetConfBool(cParm,nParm,"PHNDUR",&b))    phnDur = TRUE;
      if (GetConfFlt (cParm,nParm,"FRMSHIFT",&d))  frmShift = (float)d;
      if (GetConfFlt (cParm,nParm,"TMPFACTOR",&d)) tmpFactor = (float)d;
   }
}

void Summary(void)
{
   printf("\nHMGenS Command Summary\n\n");
   printf("LM n hmmlist mmf [dmmlist dmf]       - Load Models\n");
   printf("LW n window ..                       - Load Windows\n");
   printf("SD directry                          - Set Output Directry\n");
   printf("SI order                             - Set Order \n");
   printf("SL directry                          - Set Label Directry\n");
   printf("GE n labelfile ..                    - Generate Parameters\n");
   printf("TR level                             - Trace Level\n\n");
   Exit(0);
}

void ReportUsage(void)
{
   printf("\nGenerate parameter sequence from trained HMMs\n\n");
   printf("USAGE: HMGenS [options] ScriptFile\n\n");
   printf(" Option                                                Default\n\n");
   printf(" -p      output mean and inversed covariance sequence  off\n");
   printf(" -m      output mean sequence (static coefficients)    off\n");
   printf(" -g      use phone duration from label                 off\n");
   printf(" -f f    frame shift [100ns]                           50000\n");
   printf(" -r f    tempo factor( f < 0 : fast   f > 0 : slow )   0.0\n");  
   PrintStdOpts("Q");
   printf("\n");
   Exit(0);
}

int main(int argc,char *argv[])
{
   char *s;
   void DoGen(char *scriptFn);
   void Initialise(void);

   InitShell(argc,argv,hgens_version,hgens_sccs_id);
   InitMem();
   InitMath();
   InitSigP();
   InitWave();
   InitParm();
   InitLabel();
   InitModel();
   InitTrain();
   InitUtil();

   SetConfParms();

   if (NumArgs() == 0)   ReportUsage();

   CreateHeap(&hmmHeap,"Model Heap",MSTAK,1,1.0,80000,400000);
   CreateHeap(&tmpHeap,"Parameter Heap",MSTAK,1,1.0,80000,400000);

   while (NextArg() == SWITCHARG) {
      s = GetSwtArg();
      if (strlen(s)!=1)
         HError(9919,"HMGenS: Bad switch %s; must be single letter",s);
      switch (s[0]) {
      case 'p': outPDF = TRUE;  break;
      case 'g': phnDur = TRUE;  break;
      case 'm': outMean = TRUE; break;
      case 'f': frmShift = GetChkedFlt(0.0,100000.0,s); break;
      case 'r': tmpFactor = GetChkedFlt(-10.0,10.0,s); break;
      case 'Q': Summary();      break;
      default : HError(9919,"HMGenS: Unknown switch %s",s);
      }
   }
   if (NextArg()!=STRINGARG)
      HError(9919,"HMGenS: Script file name expected");
   if (NumArgs()>1)
      HError(9919,"HMGenS: Unexpected extra args on command line");

   if (phnDur && tmpFactor!=0.0)
      HError(9999,"HMGenS: switch -g and -r are exclusive");

   Initialise();

   DoGen(GetStrArg());

   ResetHeap(&tmpHeap);
   ResetHeap(&hmmHeap);

   Exit(0);
   return (0);          /* never reached -- make compiler happy */
}

/* ----------------------- Initialise --------------------------- */
void Initialise(void)
{
   ceppst.dw.num = f0pst.dw.num = 1;
}
 
/* ----------------------- Lexical Routines --------------------- */

int ChkedInt(char *what,int min,int max)
{
   int ans;

   if (!ReadInt(&source,&ans,1,FALSE))
     HError(9999,"ChkedInt: Integer read error - %s",what);
   if (ans<min || ans>max)
     HError(9999,"ChkedInt: Integer out of range - %s",what);
   return(ans);
}

char *ChkedAlpha(char *what,char *buf)
{
   if (!ReadString(&source,buf))
     HError(9999,"ChkedAlpha: String read error - %s",what);
   return(buf);
}

/* ------------ Generate Speech Parameters from HMMs ----------- */

/* ChkBoundary : check current frame is on voiced/unvoiced boundary according to regression window */
Boolean ChkBoundary(IntVec voiced, int t) 
{
   int i,j; 
    
   for (i=1;i<f0pst.dw.num;i++)
      for (j=f0pst.dw.width[i][0];j<=f0pst.dw.width[i][1];j++)
         if ( (f0pst.dw.coef[i][j]!=0.0) && (t+j>0) && (t+j<f0pst.T) && (voiced[t+j]==0) )
            return TRUE;
 
   return FALSE;
}
   
/* Generator : Generate speech parameter (spectrum and f0 ) and output */   
void Generator(char *labfn)
{
   char labFn[256];
   char cepfn[256],f0fn[256],durfn[256];
   char mcpfn[256],mptfn[256];
   char cpffn[256],fpffn[256];
   int i,j,k,l,m,n,s,t,pt;
   int nframe,tframe,phoneme_duration,vframe;
   int labseqlen,numstates;
   int lw,rw,nobound;
   IntVec voiced;
   int **durseq;
   float diffdur,dsum,dsqr,rho,bunbo;
   FILE *cepfp,*f0fp,*durfp;
   FILE *mcpfp,*mf0fp;
   FILE *cpffp,*fpffp;
   Transcription *labseq;
   LabList *lablist;
   Label *label;
   HMMDef *hmm,*dmm;
   MLink hmacro,dmacro;
   Vector f0;
   SVector zero;

   /* open output_files for cepstrum, f0 and duration */
   MakeFN(labfn,outDir,cepExt,cepfn);
   if ((cepfp = fopen(cepfn,"w")) == NULL)
      HError(9911,"Generator: cannot open file <%s>",cepfn);
   MakeFN(labfn,outDir,f0Ext,f0fn);
   if ((f0fp = fopen(f0fn,"w")) == NULL)
      HError(9911,"Generator: cannot open file <%s>",f0fn);
   MakeFN(labfn,outDir,durExt,durfn);
   if ((durfp = fopen(durfn,"w")) == NULL)
      HError(9911,"Generator: cannot open file <%s>",durfn);

   if (outMean) {
      MakeFN(labfn,outDir,mcpExt,mcpfn);
      if ((mcpfp = fopen(mcpfn,"w")) == NULL)
         HError(9911,"Generator: cannot open file <%s>",mcpfn);
      MakeFN(labfn,outDir,mptExt,mptfn);
      if ((mf0fp = fopen(mptfn,"w")) == NULL)
         HError(9911,"Generator: cannot open file <%s>",mptfn);
   }
   if (outPDF) {
      MakeFN(labfn,outDir,cpfExt,cpffn);
      if ((cpffp = fopen(cpffn,"w")) == NULL)
         HError(9911,"Generator: cannot open file <%s>",cpffn);
      MakeFN(labfn,outDir,fpfExt,fpffn);
      if ((fpffp = fopen(fpffn,"w")) == NULL)
         HError(9911,"Generator: cannot open file <%s>",fpffn);
   }

   /* read label file */
   MakeFN(labfn,labDir,labExt,labFn);
   labseq = LOpen(&hmmHeap,labFn,HTK);
   lablist = labseq->head;
   labseqlen = CountLabs(lablist);
   tframe = 0;

   if ((durseq = (int **) New(&tmpHeap,labseqlen*sizeof(int *)))== NULL)
      HError(9905,"Generator: Cannot allocate memory for duration storage");
   durseq--;
   
   diffdur = 0;
   /* store state duration to durseq */
   for (i=1;i<=labseqlen;i++) {
      label = GetLabN(lablist,i);

      /* find duration model */
      if ((dmacro = FindMacroName(&dmset,'l',label->labid)) == NULL)
         HError(9999,"Generator: Cannot find duration model for '%s'",
                label->labid->name);
      dmm = dmacro->structure;
      numstates = VectorSize(dmm->svec[2].info
                             ->pdf[1].info->spdf.cpdf[1].mpdf->mean);

      if ((durseq[i] = (int *)New(&tmpHeap,numstates*sizeof(int))) == NULL)
          HError(9905,"Generator: Cannot allocate memory for duration storage");
      durseq[i]-=2;

      rho = tmpFactor;
      if (phnDur){
         if (label->start==-1 || label->end==-1)
            HError(9999,"Generator: phoneme duration is not specified in %d-th label",i); 
         phoneme_duration = (label->end - label->start)/frmShift;
         dsum = dsqr = 0;
         for (j=1;j<=numstates;j++){
            dsum += dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->mean[j];
            dsqr += dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->cov.var[j];
         }
         rho = (phoneme_duration - dsum)/dsqr;
      }

      /* calculate state duration */
      for (j=2;j<=numstates+1;j++){
         durseq[i][j]
             = (int)(dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->mean[j-1]
                +rho*dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->cov.var[j-1]
                +diffdur+0.5);

         if (durseq[i][j]<1)
            durseq[i][j] = 1;

         diffdur += dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->mean[j-1]
                    +rho*dmm->svec[2].info->pdf[1].info->spdf.cpdf[1].mpdf->cov.var[j-1]
                    -(float)durseq[i][j];
         tframe += durseq[i][j];
         /* output state duration */
         fprintf(durfp,"%s   %2d   %3d\n", 
                 label->labid->name,j,durseq[i][j]);
         fflush(durfp);
         if (trace & T_DUR) {
             printf("%s   %2d   %3d\n",
                    label->labid->name,j,durseq[i][j]);
             fflush(stdout);
         }
      }
   }
   
   /* allocate memory for results of voiced/unvoiced judgement */
   voiced = CreateIntVec(&tmpHeap,tframe);
   f0 = CreateVector(&tmpHeap,tframe);
   nframe = 1;
   vframe = 0;

   /* judge whether voiced or unvoiced */
   for (i=1;i<=labseqlen;i++) {
      label = GetLabN(lablist,i);
      /* find hmm */
      if ((hmacro = FindMacroName(&hmset,'l',label->labid)) == NULL)
         HError(9999,"Generator: Cannot find HMM for '%s'",label->labid->name);
      hmm = hmacro->structure;

      for (j=2;j<=hmm->numStates-1;j++) {
         for (s=1;s<=hmset.swidth[0];s++) {
            if ( NumNonZeroSpace(hmm->svec[j].info->pdf[s].info)>1 )
               HError(9999, "Generator: More than 2 non-zero order distribution are 
                             exist at %s.state[%d].stream[%d]", label->labid->name, j, s);
            if (hmm->svec[j].info->pdf[s].info->spdf.cpdf[1].mpdf->ckind==FULLC)
               HError(9999, "Generator: parameter generation only currently available for 
                             diagonal covariance matrix");
         }
         for (k=1;k<=durseq[i][j];k++) {
            if (hmm->svec[j].info->pdf[2].info->spdf.cpdf[1].weight>=0.5){
               voiced[nframe++] = 1;
               vframe++;
            }
            else 
               voiced[nframe++] = 0;
         }
      }
   }
   if (trace) printf("  frame [voiced frame] : %d[%d]\n",tframe,vframe);
               
   ceppst.T = tframe;
   f0pst.T = vframe;   
   total_frame += tframe;

   InitPStream(&tmpHeap,&ceppst);
   InitPStream(&tmpHeap,&f0pst);
   zero = CreateSVector(&tmpHeap,f0pst.vSize);
   for (i=1;i<=f0pst.vSize;i++) zero[i] = INVINF;

   for (i=1,t=1,pt=1;i<=labseqlen;i++) {
      label = GetLabN(lablist,i);
      /* find hmm */
      if ((hmacro = FindMacroName(&hmset,'l',label->labid)) == NULL)
         HError(9999,"Generator: Cannot find HMM for '%s'",label->labid->name);
      hmm = hmacro->structure;
      for (j=2;j<hmm->numStates;j++)
         for(k=1;k<=durseq[i][j];k++) {
            /* copy vector for cepstrum */
            ceppst.sm.mseq[t]  = hmm->svec[j].info->pdf[1].info->spdf.cpdf[1].mpdf->mean;  
            ceppst.sm.ivseq[t] = hmm->svec[j].info->pdf[1].info->spdf.cpdf[1].mpdf->cov.var;
            /* copy vector for f0 */
            if (voiced[t]) {
               f0pst.sm.mseq[pt]  = CreateSVector(&tmpHeap,f0pst.vSize);
               f0pst.sm.ivseq[pt] = CreateSVector(&tmpHeap,f0pst.vSize);
               for (s=1;s<=f0pst.vSize;s++) {
                  f0pst.sm.mseq[pt][s]  = hmm->svec[j].info->pdf[s+1].info->spdf.cpdf[1].mpdf->mean[1];
                  f0pst.sm.ivseq[pt][s] = hmm->svec[j].info->pdf[s+1].info->spdf.cpdf[1].mpdf->cov.var[1];
               }
               if ( ChkBoundary(voiced,t) )   /* voiced/unvoiced boundary */
                  f0pst.sm.ivseq[pt] = zero;
               pt++;
            }
            t++;
         }
   }

   /* generate parameters */
   pdf2par(&ceppst); 
   if (vframe>0) pdf2par(&f0pst);

   /* output generated spectrum sequence */
   WriteMatrix(cepfp,ceppst.sm.C,TRUE);

   /* copy f0 parameter */
   pt = 1;
   for (t=1;t<=tframe;t++) {
      if (voiced[t] && vframe>0) {
         f0[t] = f0pst.sm.C[pt][1];
         pt++;
      }
      else
         f0[t] = 0.0;
   }
   
   /* output generated f0 sequence */
   WriteVector(f0fp,f0,TRUE);
  
   if (outMean || outPDF) {
      pt = 1; 
      ZeroVector(zero);
      for (t=1;t<=tframe;t++) {       
         if (outMean) 
            WriteFloat(mcpfp,ceppst.sm.mseq[t]+1,ceppst.order,TRUE);
         if (outPDF) {
            WriteVector(cpffp,ceppst.sm.mseq[t],TRUE);
            WriteVector(cpffp,ceppst.sm.ivseq[t],TRUE);
         }  
         if (voiced[t] && vframe>0) {
            if (outMean)
               WriteFloat(mf0fp,f0pst.sm.mseq[pt]+1,f0pst.order,TRUE);
            if (outPDF) {
               WriteVector(fpffp,f0pst.sm.mseq[pt],TRUE);
               WriteVector(fpffp,f0pst.sm.ivseq[pt],TRUE);
            }
            pt++;
         }
         else {
            if (outMean)
               WriteFloat(mf0fp,zero+1,f0pst.order,TRUE);
            if (outPDF) {
               WriteVector(fpffp,zero,TRUE);
               WriteVector(fpffp,zero,TRUE);
            }
         }
      }
   }

   /* close output files */
   fclose(cepfp);
   fclose(f0fp);
   fclose(durfp);
   
   if (outMean) {
      fclose(mcpfp);
      fclose(mf0fp);
   }
   if (outPDF) {
      fclose(cpffp);
      fclose(fpffp);
   }
   
   /* free memory */
   FreePStream(&tmpHeap, &f0pst);
   FreePStream(&tmpHeap, &ceppst);

   return;
}

/* ----------------- Load HMM Sets for spectrum, f0, and duration  ------------------ */

void ChkModel(void)
{  
   printf("\tlogical  model          : %d\n",hmset.numLogHMM);
   printf("\tphysical model          : %d\n",hmset.numPhyHMM);
   printf("\tlogical  duration model : %d\n",dmset.numLogHMM);
   printf("\tphysical duration model : %d\n",dmset.numPhyHMM);
   fflush(stdout);
}

void LoadMMFCommand(void)
{
   char listfn[256],hmmfn[256],dlistfn[256],dmmfn[256];
   int ch;

   ChkedAlpha("hmmlist file name",listfn);
   ChkedAlpha("mmf file name",hmmfn);
   
   do {
      ch = GetCh(&source);
      if (ch == '\n') break;
   } while(ch != EOF && isspace(ch));

   UnGetCh(ch,&source);

   ChkedAlpha("duration model list file name",dlistfn);
   ChkedAlpha("duration mmf file name",dmmfn);
   
   if (trace) {
      printf("Load hmmlist\t: %s\n",listfn);
      printf("Load mmffile\t: %s\n",hmmfn);
      printf("Load dmmlist\t: %s\n",dlistfn);
      printf("Load dmffile\t: %s\n",dmmfn);
      fflush(stdout);
   }

   CreateHMMSet(&hmset,&hmmHeap,TRUE);
   AddMMF(&hmset,hmmfn);
   if (MakeHMMSet(&hmset, listfn )<SUCCESS)
      HError(9999,"LoadMMF: MakeHMMSet failed");
   if (LoadHMMSet(&hmset,NULL, hmmExt)<SUCCESS)
      HError(9999,"LoadMMF: LoadHMMSet failed");
   
   CreateHMMSet(&dmset,&hmmHeap,TRUE);
   AddMMF(&dmset,dmmfn);
   if (MakeHMMSet(&dmset, dlistfn )<SUCCESS)
      HError(9999,"LoadMMF: MakeHMMSet failed");
   if (LoadHMMSet(&dmset,NULL, hmmExt)<SUCCESS)
      HError(9999,"LoadMMF: LoadHMMSet failed");

   if (trace)
      ChkModel();

   LoadModel = TRUE;

   return;
}

/* ---- Load Regression Window for Dynamic Feature Constraints ---- */  
void LoadWindowCommand(void)
{
   char *winfn;
   int i,j;

   j = ceppst.dw.num;
   ceppst.dw.num += ChkedInt("number of delta window for spectrum",0,3);
   for (i=j;i<ceppst.dw.num;i++){
      winfn = (char *)malloc(256*sizeof(char));
      ChkedAlpha("window file",winfn);
      ceppst.dw.fn[i] = winfn;
      if (trace){
         printf("Set window file for spectrum <%d>\t: %s\n",i,winfn);
      }  
   }
   
   j = f0pst.dw.num;
   f0pst.dw.num += ChkedInt("number of delta window for f0 ",0,3);
   for (i=j;i<f0pst.dw.num;i++){
      winfn = (char *)malloc(256*sizeof(char));
      ChkedAlpha("window file",winfn);
      f0pst.dw.fn[i] = winfn;
      if (trace){
         printf("Set window file for f0 <%d>\t: %s\n",i,winfn);
      }
   }
}

/* ---------- Set Output Directory ----------- */
void SetOutputDirCommand(void)
{
   if(outDir==NULL && (outDir=(char *)malloc(256*sizeof(char)))== NULL)
      HError(9905,"SetOutputDir: Cannot allocate memory for Directory name");
   
   ChkedAlpha("directory to store results",outDir);

   if(trace){
      printf("Set output dirctory\t: %s\n",outDir);
      fflush(stdout);
   }
}

/* ---------- Set Order of 1st stream (spectrum) --------- */
void SetInfoCommand(void)
{
   ceppst.order = ChkedInt("order of cepstrum",0,50)+1;
   f0pst.order = 1;
   SetOrder = TRUE;
}

/* ---------- Set Label Directory ---------- */
void SetLabelDirCommand(void)
{
   if (labDir==NULL && (labDir=(char *)malloc(256*sizeof(char)))== NULL)
   HError(9905,"SetLabelDir: Cannot allocate memory for directory name");
   
   ChkedAlpha("label file directry",labDir);

   if (trace){
      printf("Set label file dirctory\t: %s\n",labDir);
      fflush(stdout);
   }
}

/* --------- Generate Speech Parameters --------- */
void GenerateCommand()
{
   char  labfn[256][256];
   int   i,number,max_T = 0;
   
   if (!LoadModel)
	   HError(9999, "Generate: Load HMM set before parameter generation");
   if (!SetOrder)
       HError(9999, "Generate: Set order before parameter generation");
 
   number = ChkedInt("label file number",0,256);
   ConvDiagC(&hmset, TRUE);
 
   for (i=0;i<number;i++){
      ChkedAlpha("label file name",labfn[i]);
      if (trace) {
         printf("Generating speech parameter\t: %s\n",labfn[i]);
         fflush(stdout);
      }
      Generator(labfn[i]);
   }
   ConvDiagC(&hmset, TRUE);
}

/* ---------- Set Trace Level ----------- */
void SetTraceCommand(void)
{
   int m;

   m = ChkedInt("Trace level",0,31);
   if (trace) {
      printf("Adjusting trace level\t: %d\n",m);
      fflush(stdout);
   }
   trace = m&0xf;
}

/* ----------------- Top Level of Generator ------------------ */

static int  nCmds = 7;
static char *cmdmap[] = { "LM","LW","SD","SI","SL","GE","TR","" };
typedef enum            { LM=1, LW , SD , SI , SL , GE , TR }
cmdNum;

/* CmdIndex: return index 1..N of given command */
int CmdIndex(char *s)
{
   int i;
   
   for (i=1; i<=nCmds; i++)
     if (strcmp(cmdmap[i-1],s) == 0) return i;
   return 0;
}

/* DoGen: Generate parameter using given command file */
void DoGen(char *scriptFn)
{
   int thisCommand = 0,c1,c2;
   char cmds[] = "  ";
   
   /* Open the Script File */
   InitSource(scriptFn,&source,NoFilter);
     
   /* Apply each Command */
   for (;;) {
     SkipWhiteSpace(&source);
     if ( (c1 = GetCh(&source)) == EOF) break;
     if ( (c2 = GetCh(&source)) == EOF) break;
     cmds[0] = c1; cmds[1] = c2;
     if (!isspace(GetCh(&source))) 
        HError(9999,"DoGen: White space expected after command %s",cmds);
     thisCommand = CmdIndex(cmds);
     switch (thisCommand) {
     case LM: LoadMMFCommand();break;
     case LW: LoadWindowCommand();break;
     case SD: SetOutputDirCommand();break;
     case SI: SetInfoCommand();break;
     case SL: SetLabelDirCommand();break;
     case GE: GenerateCommand();break;
     case TR: SetTraceCommand();break;          
     default: 
        HError(9999,"DoGen: Command %s not recognised",cmds);
     }
   }
   CloseSource(&source);
}

/* ----------------------------------------------------------- */
/*                        END:  HMGenS.c                       */
/* ----------------------------------------------------------- */
