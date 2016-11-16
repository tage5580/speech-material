/*****************************************************************************
*   CONVERT (v 1.2)                                                          *
*   Convert a DARPA TIMIT speech file(s) into the european standard          *
*   SAM file format: that is  - a speech file                                *
*                             - an associated label file                     *
*                                                                            *
*   How to produce CONVERT.EXE:  msc /Ox convert.c    (Microsoft C - v 4.0)  *
*                                link convert;                               *
*   CONVERT.MSG includes define texts.                                       *
*                                                                            *
*   ICP - Jan 90                                        Author: J.Zeiliger   *
*****************************************************************************/
/* Note: Convert was slightly modified at NIST for TIMIT CD-ROM - October 1990.
   THIS ADAPTATION OF THE CONVERT PROGRAM WILL ONLY WORK WITH
   THE TIMIT CD-ROM CD1-1.1                                                  */

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <conio.h>
#include <string.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "convert.msg"

#define TRUE 1
#define FALSE 0
#define IN(c,chaine) ( (strchr ((chaine), (c)) != NULL) ? 1 : 0 )
#define BIP printf("\7");        /* ring bell */
#define ENDHEADER 1024L

#define CR 13
#define BACKSPACE 8

#define RELEASE "1.1"
#define VDATE "Jan 90"
#define SPKCODMAP "SPKR_MAP.SAM"
#define SENTCODMAP "SENT_MAP.SAM"

char Version[5] = "1.0";              /* labelling version */
short ChCount;                        /* number of channel */
short SamplingRate;
short SampNBytes;
char SampBytOrder[3];
short SampSigBits;

char CDDate[5] = "19";                /* CDROM making year */
int DbYear;                           /* 88, 89, 90 ....*/
char SpkMapFile[80] = "\0";           /* mapping file for speaker code */
char SamSpkCode[3];                   /* speaker code in SAM format */
char SamCorpCode[3];                  /* corpus code in SAM format */
char SamFileNum[5] = "0000";          /* SAM file number */
char SrcPath[80] = "\0";              /* Path for Source file */
short CDPresent;                      /* boolean: CD present or not */
char CDDrive[2];                      /* CD drive letter */
char DbId[13] = "RM1";                /* database identifier */
char PromptSpecif[80] = "\0";         /* full prompt file specification (DARPA) */
char PromptFile[80] = "\0";           /* name of prompt file if any (TIMIT) */
char SpeakersFile[80]= "\0";          /* Full file of speakers specification */
char SpkDir[80] = "\0";               /* directory of the speaker */
char SourceFile[80] = "\0";           /* source speech file name */
char DestFile[80] = "\0";             /* Full destination speech file name specif */
char LabelFile[80] = "\0";            /* full label file name specification */
char CmdFile[80] = "\0";              /* full command file specification */
char Warea[100];                      /* working area */
FILE * FichLabel;
FILE * FichCmd;
int Fich;
int Ext;
long SampCount;                  /* -> length of the speech signal DestFile */
char UtterId[12];                     /* utterance identifier */
short SampMax;                        /* sample max value */
short SampMin;                        /* sample min value */
char SpkSex = '\0';                   /* speaker sex: M or F */
char SpkNatLang = '\0';               /* speaker native language  DR ? */
short SpkAge = 0;                  /* speaker age when recording */

/*---------------------------------------------------------------------------*/
/* Copy only speech signal from SourceFile to DestFile                       */
short CopySignal()
{
  unsigned int count;
  long result;
  char *buffer;
  unsigned int taillebuf= 16384;
  short OK;

  buffer = (char *) calloc(taillebuf,sizeof(char));
  if (buffer == NULL) {
   fprintf(stderr, error1);
   return(FALSE);
  } /* endif */

  if (CDPresent) {
    if (strlen(SrcPath) == 0) {
    sprintf(Warea, "%s\\%s", SpkDir, SourceFile);
    } else {
      sprintf(Warea, "%s\\%s", SrcPath, SourceFile);
    } /* endif */
  } else {
    if (strlen(SrcPath) == 0) sprintf(Warea, "%s", SourceFile);
    else  sprintf(Warea, "%s\\%s", SrcPath, SourceFile);
  } /* endif */

 if ((Fich = open(Warea, O_RDONLY|O_BINARY)) == -1) {
   fprintf(stderr, error2, Warea);
   return(FALSE);
 } else {
   if ((Ext = open(DestFile,O_WRONLY|O_BINARY|O_APPEND|O_CREAT|O_EXCL,S_IWRITE)) == -1) {
       fprintf(stderr, error3, DestFile);
       OK = FALSE;
   } else {
      result = lseek(Fich,1024L,SEEK_SET);
      if (result==-1L){
        fprintf(stderr, error4, DestFile);
        OK = FALSE;
      } else {
        while (!eof(Fich)) {
          count= read(Fich, buffer, taillebuf);
          write(Ext, buffer, count);
        } /* endwhile */
        OK = TRUE;
      } /* endif */
      close(Ext);
   } /* endif */
   free(buffer);
   close(Fich);
   return(OK);
 } /* endif */
}

/*----------------------------------------------------------------------------*/
/* Look in SpeakersFile for sex, age, native language of the speaker   */
void GetSpkInfo()
{
 FILE *FSpk;
 char line[200], *ptr, *ptr2;
 char Code[5], SpkId[5], Recdate[10], Bdate[10], DR, Sex, *Date;
 int Year;
 short result, Trouve = FALSE;


    strncpy(SpkId, UtterId, 4);
    SpkId[4] = '\0';
    if ((FSpk = fopen(SpeakersFile, "rt")) != NULL ) {
       do {
        fgets(line, sizeof(line), FSpk);
        if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
        sscanf(line,"%s", Code);
        if (!strcmpi(SpkId, Code) ) {
          Trouve = TRUE;
          result = sscanf(line,"%*s %c %c %*s %s %s", &Sex, &DR, Recdate, Bdate);
        }
       } while ((!Trouve) && (!feof(FSpk))  ); /* enddo */

      if (result == 4) {
        SpkSex =  Sex;
        SpkNatLang = DR;
        if ((ptr = strrchr(Bdate, '/')) != NULL) {
           ptr2 = strrchr(Recdate, '/');
           SpkAge = atoi(ptr2+1) - atoi(ptr+1);
        }
        else SpkAge = 0;
      } else {
        SpkSex = SpkNatLang = '\0';
        SpkAge = (short) '\0';
      } /* endif */
    } else {
      SpkSex = SpkNatLang = '\0';
      SpkAge = (short) '\0';
    } /* endif */
}
/*----------------------------------------------------------------------------*/
/* Writes header block in the label file                                      */
void LabelHeader()
{
  char *ptr, Path[80];

     fputs("LHD: V1.1\n", FichLabel);
     fputs("FIL: label\n",FichLabel);
     fputs("TYP: orthographic\n", FichLabel);

     if (CDPresent) {
       fprintf(FichLabel, "DBN: %s\n", DbId);
       fputs("VOL: \n", FichLabel);
       fputs("DIR: \n", FichLabel);
       if ((ptr = strrchr(DestFile, '\\')) != NULL) {
         fprintf(FichLabel, "SRC: %s\n", (ptr+1));
       } else {
          if ((ptr = strrchr(DestFile, ':')) != NULL) {
            fprintf(FichLabel, "SRC: %s\n", (ptr+1));
           } else fprintf(FichLabel, "SRC: %s\n", DestFile);
       } /* endif */
       if (strlen(SrcPath) != 0) strcpy(Path, SrcPath);
       else strcpy(Path, SpkDir);
       if ((ptr = strchr(Path, ':'))!= NULL)
        fprintf(FichLabel, "CMT: CONVERT(%s) from DARPA file: %s\\%s\n",\
         RELEASE,(ptr+1), SourceFile);
       else
        fprintf(FichLabel, "CMT: CONVERT(%s) from DARPA file: %s\\%s\n",\
         RELEASE, Path, SourceFile);
       if (*PromptFile != '\0') {
          ptr = &PromptFile[2];
          fprintf(FichLabel, "TXF: %s\n", ptr);
       } else {
         ptr = &PromptSpecif[2];
         fprintf(FichLabel, "TXF: %s\n", ptr);
       } /* endif */
       fputs("CMT: Information about the recording session\n", FichLabel);
       fprintf(FichLabel, "SAM: %d\n", SamplingRate);
       fputs("BEG: 0\n", FichLabel);
       fprintf(FichLabel, "END: %ld\n", SampCount);
       fputs("RED: \n", FichLabel);
       fputs("RET: \n", FichLabel);
       fputs("REP: \n", FichLabel);
       fprintf(FichLabel,"SNB: %hd\n", SampNBytes);
       fprintf(FichLabel,"SBF: %s\n", SampBytOrder);
       fprintf(FichLabel,"SSB: %hd\n", SampSigBits);
       fputs("RCC: \n",FichLabel);
       fprintf(FichLabel, "NCH: %hd\n", ChCount);
       fputs("CMT: Third parameter in SPI is american dialect region number\n", FichLabel);
       GetSpkInfo();
       fprintf(FichLabel, "SPI: %c, %hd, %c\n", SpkSex, SpkAge, SpkNatLang);

     } else {        /* minimal information */

       fputs("DBN: \n", FichLabel);
       fputs("VOL: \n", FichLabel);
       fputs("DIR: \n", FichLabel);
       if ((ptr = strrchr(DestFile, '\\')) != NULL) {
         fprintf(FichLabel, "SRC: %s\n", (ptr+1));
       } else {
          if ((ptr = strrchr(DestFile, ':')) != NULL) {
            fprintf(FichLabel, "SRC: %s\n", (ptr+1));
          } else fprintf(FichLabel, "SRC: %s\n", DestFile);
       } /* endif */
       fprintf(FichLabel, "CMT: CONVERT(%s) from DARPA file: %s\n", RELEASE, SourceFile);
       fputs("TXF: \n",FichLabel);
       fputs("CMT: Information about the recording session\n", FichLabel);
       fprintf(FichLabel, "SAM: %d\n", SamplingRate);
       fputs("BEG: 0\n", FichLabel);
       fprintf(FichLabel, "END: %ld\n", SampCount);
       fputs("RED: \n", FichLabel);
       fputs("RET: \n", FichLabel);
       fputs("REP: \n", FichLabel);
       fprintf(FichLabel,"SNB: %hd\n", SampNBytes);
       fprintf(FichLabel,"SBF: %s\n", SampBytOrder);
       fprintf(FichLabel,"SSB: %hd\n", SampSigBits);
       fputs("RCC: \n",FichLabel);
       fprintf(FichLabel, "NCH: %hd\n", ChCount);
       fputs("CMT: Third parameter in SPI is american dialect region number\n", FichLabel);
       fputs("SPI: \n", FichLabel);

     } /* endif */

     fputs("PCF: \n", FichLabel);
     fputs("CMT: Information about the labelling session\n", FichLabel);
     fputs("EXP: \n",FichLabel);
     fputs("SYS: \n",FichLabel);
     fputs("DAT: \n",FichLabel);
     fputs("SPA: \n", FichLabel);
     fputs("CMT: Item: label start, end, input gain, min level, max level,\
 string\n",FichLabel);
     fputs("LBD: \n", FichLabel);

}
/*---------------------------------------------------------------------------*/
 void cesure(mnem, ligne, fichier)
 char * mnem;
 char * ligne;
 FILE * fichier;
 {
  char * ptr;
  short n;
    if ((n = strlen(ligne)) > 75) {
      ptr = (ligne + 75);
      while (*ptr != ' ') ptr--;
      *ptr = '\0';
      fprintf(fichier, "%s: %s\n", mnem, ligne);
      cesure("EXT", (ptr+1), fichier);
    } else {
      fprintf(fichier, "%s: %s", mnem, ligne);
    } /* endif */
 }
/*---------------------------------------------------------------------------*/
/* Search for sentence in the text prompt file on the CD if present, and     */
/* writes label body in the label file.                                       */
void LabelBody()
{
   FILE * FichSent;
   char Sentence[140];
   char line[180];
   char SentNum[80];
   register char *ptr;
   register short Trouve = FALSE;
   short n, result;

   *Sentence = '\0';
   if (CDPresent) {
     /* test for the accompanying transcription file */
     if ((FichSent = fopen(PromptFile, "rt")) != NULL ) {
          fgets(line, sizeof(line), FichSent);
          if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
          strcpy(Sentence, line);
          fclose(FichSent);
          strcpy(PromptSpecif, Warea);
     } else {
       /* looking for sentence in the prompts file */
       if ((FichSent = fopen(PromptSpecif, "rt")) == NULL ) {
          fprintf(stderr, error5, PromptSpecif);
       } else {
          strcpy(SentNum, SourceFile);
          if ((ptr = strrchr(SentNum, '.')) != NULL) *ptr = '\0';
          n = strlen(SentNum);
          while ((!feof(FichSent)) && (!Trouve) ) {
             fgets(line, sizeof(line), FichSent);
             if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
             if ((ptr = strrchr(line, '(')) != NULL) {
               result = strnicmp(SentNum, (ptr+1), n);
               if (result == 0) {
                  Trouve = TRUE;
                  *(ptr-1) = '\0';
                  strcpy(Sentence, line);
               } /* endif */
             } /* endif */
          } /* endwhile */
          fclose(FichSent);
       } /* endif */
     } /* endif */
     sprintf(line,"0, %ld, , %hd, %hd, %s\n", SampCount, SampMin, SampMax,\
     Sentence);
     cesure("LBR", line, FichLabel);
   } else {  /* if cdrom not present */
     fprintf(FichLabel,"LBR: 0, %ld, , %hd, %hd, .\n", SampCount, SampMin, SampMax );
   } /* endif */
   fputs("ELF: \n", FichLabel);
}

/*---------------------------------------------------------------------------*/
/* Creates and fills the label file                                          */
short CopyLabel()
{

    strcpy(LabelFile, DestFile);
    LabelFile[(strlen(LabelFile) -1)] = 'O';
    if ((FichLabel = fopen(LabelFile, "wt")) == NULL ) {
       fprintf(stderr, error6, LabelFile);
       return(FALSE);
    } else {
      LabelHeader();
      LabelBody();
      fclose(FichLabel);
      return(TRUE);
    } /* endif */
}
/*---------------------------------------------------------------------------*/
/* Search for a string which triplet begins by the keyword, in the header    */
/* Stops when found or at end_head or at header length.                      */
void SearchForString(FilePt, Keyword, Dest, MaxLength )
 FILE * FilePt;
 char * Keyword, * Dest;
 short MaxLength;
{
   char line[80], area[80];
   char * ptr;
   short res, Fin = FALSE;
   short Trouve = FALSE;

   fseek(FilePt, 0L, SEEK_SET);
   do {
     fgets(line, sizeof(line), FilePt);
        if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
     res = sscanf(line,"%s %*s %*s", area );
     if (res == 1) {
       if (!(strcmpi(Keyword, area))) {
         res = sscanf(line,"%*s %*s %s", area );
         if (strlen(area)<= MaxLength) strcpy(Dest, area);
         else { strncpy(Dest, area, MaxLength);
                *(Dest+MaxLength) = '\0';
         } /* endif */
         Trouve = TRUE;
       } /* endif */
     } /* endif */
     if ((!(strcmpi("end_head", area))) || (ftell(FilePt) >= ENDHEADER)) Fin = TRUE;
   } while (!Fin && !Trouve); /* enddo */
   if (!Trouve) *Dest = '\0';
}
/*---------------------------------------------------------------------------*/
/* Search for a short value which triplet begins by the keyword, in the header*/
void SearchForShort(FilePt, Keyword, Dest)
 FILE * FilePt;
 char * Keyword;
 short * Dest;
{
   char line[80], area[80], *ptr;
   short res, Fin = FALSE;
   short Trouve = FALSE;

   fseek(FilePt, 0L, SEEK_SET);
   do {
     fgets(line, sizeof(line), FilePt);
        if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
     res = sscanf(line,"%s %*s %*s", area );
     if (res == 1) {
       if (!(strcmpi(Keyword, area))) {
         res = sscanf(line,"%*s %*s %hd", Dest);
         Trouve = TRUE;
       } /* endif */
     } /* endif */
     if ((!(strcmpi("end_head", area))) || (ftell(FilePt) >= ENDHEADER)) Fin = TRUE;
   } while (!Fin && !Trouve); /* enddo */
   if (!Trouve) *Dest = 0;
}
/*---------------------------------------------------------------------------*/
/* Search for a long value which triplet begins by the keyword, in the header*/
void SearchForLong(FilePt, Keyword, Dest)
 FILE * FilePt;
 char * Keyword;
 long * Dest;
{
   char line[80], area[80], *ptr;
   short res, Fin = FALSE;
   short Trouve = FALSE;

   fseek(FilePt, 0L, SEEK_SET);
   do {
     fgets(line, sizeof(line), FilePt);
        if ((ptr = strrchr(line, '\n')) != NULL) *ptr = '\0';
     res = sscanf(line,"%s %*s %*s", area );
     if (res == 1) {
       if (!(strcmpi(Keyword, area))) {
         res = sscanf(line,"%*s %*s %ld", Dest);
         Trouve = TRUE;
       } /* endif */
     } /* endif */
     if ((!(strcmpi("end_head", area))) || (ftell(FilePt) >= ENDHEADER)) Fin = TRUE;
   } while (!Fin && !Trouve); /* enddo */
   if (!Trouve) *Dest = 0;
}
/*---------------------------------------------------------------------------*/
/* Extract information from the SourceFile header                            */
short ExtractInfos()
{
   FILE * Source;

      if (CDPresent) {
        if (strlen(SrcPath) == 0) {
          sprintf(Warea, "%s\\%s", SpkDir, SourceFile);
        } else {
          sprintf(Warea,"%s\\%s",SrcPath, SourceFile);
        } /* endif */
      } else {
        if (strlen(SrcPath) == 0) strcpy(Warea, SourceFile);
        else  sprintf(Warea, "%s\\%s", SrcPath, SourceFile);
      } /* endif */

      if ((Source = fopen(Warea, "rt")) == NULL ) {
         fprintf(stderr, error2, Warea);
         return(FALSE);
      } else {
        SearchForString(Source, "database_id", DbId, 12);
        SearchForString(Source, "database_version", Version, 4);
        SearchForString(Source, "utterance_id", UtterId, 11);
        SearchForShort(Source, "channel_count", &ChCount);
        SearchForLong(Source, "sample_count", &SampCount);
        SampCount -= 1;   /* Sample nø start from 0 */
        SearchForShort(Source, "sample_rate", &SamplingRate);
        SearchForShort(Source, "sample_min", &SampMin);
        SearchForShort(Source, "sample_max", &SampMax);
        SearchForShort(Source, "sample_n_bytes", &SampNBytes);
        SearchForString(Source, "sample_byte_format", SampBytOrder, 2);
        SearchForShort(Source, "sample_sig_bits", &SampSigBits);
        fclose(Source);
        return(TRUE);
      } /* endif */
}
/*****************************************************************************/
/*                             Editing functions                             */
/*---------------------------------------------------------------------------*/
short TouchFonct (car)
short* car;
{
   *car = getch();
   if ((kbhit()!=0) && (*car == '\0') ) {
      *car = getch();
     return(TRUE);
   } else { return(FALSE) ;}
}
/*---------------------------------------------------------------------------*/
void Keyboarding(Name, Length)
char * Name;
short Length;
{
   int c;
   short Fin = FALSE;
   register char * Pos = Name;
   char * End = Name + (Length-1);

     while (kbhit()) getch(); /* vider buffer clavier */
     printf("%s", Name);
     while (*Pos) Pos++;
     while (!Fin) {
       if ( !TouchFonct(&c) ) {
         switch (c) {
           case BACKSPACE: if (Pos > Name) {
                                printf("\b \b");
                                Pos--;
                           } /* endif */
              break;
           case CR: Fin = TRUE;
                      *Pos = '\0';
              break;
           default: if (Pos < End) {
                       *(Pos++) = (char) c;
                       printf("%c", c);
                    } else BIP;
             break;
         } /* endswitch */
       } /* endif */
     } /* endwhile */
}
/*---------------------------------------------------------------------------*/
int GetYesNo()
{
   int Rep;
   do {
     Rep = toupper (getch());
   } while (!IN (Rep, "YN") ); /* enddo */
   printf("%c\n", Rep);
   return(Rep);
}
/******************************************************************************/

/*---------------------------------------------------------------------------*/
/* Increments the SamFileNum and let it under a formatted string              */
void Increment()
{
  int x, l;
  char area[5];

    x = atoi(SamFileNum);
    x+=1;
    itoa(x, SamFileNum, 10);
    l = strlen(SamFileNum);
    if (l<4) {
       switch (l) {
       case 1: sprintf(area,"000%s",SamFileNum);
          break;
       case 2: sprintf(area,"00%s",SamFileNum);
          break;
       case 3: sprintf(area,"0%s",SamFileNum);
          break;
       default: ;
       } /* endswitch */
    strcpy(SamFileNum,area);
    } /* endif */
}


/*---------------------------------------------------------------------------*/
/* Convert the files one by one, asking each time for the names of SpkDir,   */
/* SourceFile, DestFile.                                                     */
void InputCommandes()
{
   int Fin = FALSE;
   char * ptr, SentNum[80];
   short l;
   FILE * FichSent;

   do {
     if (CDPresent) {
       printf(msg3);
       Keyboarding(SpkDir, 80);
       strupr(SpkDir);
       l = strlen(SpkDir);
       ptr = &SpkDir[(l - 1)];
       if (*ptr == '\\')  *ptr = '\0';
       printf(msg4);
     } else printf(msg5);  /* endif */

     Keyboarding(SourceFile, 80);
     strupr(SourceFile);

     printf(msg6);
     Keyboarding(DestFile, 80);
     strupr(DestFile);
     l = (strlen(DestFile) - 1);
     if (DestFile[l] == 'O') DestFile[l] = 'S';
     printf(msg18);

     /* test for an accompanying transcription file */
     strcpy(SentNum, SourceFile);
     if ((ptr = strrchr(SentNum, '.')) != NULL) *ptr = '\0';
     if (strlen(SrcPath) == 0) {
       sprintf(Warea, "%s\\%s.TXT", SpkDir, SentNum);
     } else {
       sprintf(Warea, "%s\\%s.TXT", SrcPath, SentNum);
     } /* endif */
     if ((FichSent = fopen(Warea, "rt")) != NULL ) {
        strcpy(PromptFile, Warea);
        fclose(FichSent);
     } else {
       *PromptFile = '\0';
     } /* endif */

     if (ExtractInfos()) {
       if (CopySignal()) {
         if (CopyLabel()) {
           printf(msg7, DestFile, LabelFile);
         } /* endif */
       } /* endif */
     } /* endif */
     printf(msg8);
     if (GetYesNo() == 'N') Fin = TRUE;
   } while (!Fin  ); /* enddo */

}

/*---------------------------------------------------------------------------*/
/* if SpkMapFile is specified SamSpkCode is searched in it, else if CD Present*/
/* SamSpkCode is SPKCODMAP else in current directory else asked to the user  */
/* else filled with UtterId[0..1]  (2 first letters from original speaker code)*/
 void GetSpkCod()
{
  FILE * FichSpk;
  char line[80], *ptr;
  char DarpaId[5] , Code[5], SamId[3];
  short Trouve = FALSE;
  short FILEOPEN = FALSE;

  if (*SpkMapFile != '\0') {       /* already given by user */
    if ((FichSpk = fopen( SpkMapFile, "rt")) != NULL )  FILEOPEN = TRUE;
    else fprintf(stderr, error5, SpkMapFile);
  } else {                        /* no yet specified */
    if (CDPresent) {
      /* Building speakers mapping file name */
      sprintf(Warea, "%s:\\convert\\%s",CDDrive,SPKCODMAP);
      if ((FichSpk = fopen( Warea, "rt")) != NULL )  FILEOPEN = TRUE;
    } else {              /* CD not present -> look in current directory */
      sprintf(Warea, "%s", SPKCODMAP);
      if ((FichSpk = fopen( Warea, "rt")) != NULL ) FILEOPEN = TRUE;
    } /* endif */
  } /* endif */
   if (!FILEOPEN) {
      printf(msg19);
      Keyboarding(SpkMapFile, 80);
      printf("\n");
      if ((FichSpk = fopen( SpkMapFile, "rt")) != NULL )  FILEOPEN = TRUE;
      else fprintf(stderr, error5, SpkMapFile);
   } /* endif */
   if (FILEOPEN) {
      strncpy(Code, UtterId, 4);
      Code[4] = '\0';
      do {
        fgets (line, sizeof(line), FichSpk);
        if ((ptr = strrchr(line, '\n'))!= NULL) *ptr = '\0';
        sscanf(line, "%s %s", DarpaId, SamId);
        if (!(strcmpi(Code, DarpaId))) {
          strcpy(SamSpkCode, SamId) ;
          Trouve = TRUE;
        } /* endif */
      } while ((!Trouve) && (!feof(FichSpk))  ); /* enddo */
      if (!Trouve) printf("SAM ID not found\n");
      fclose(FichSpk);
   } else {
       sprintf(SamSpkCode, "%c%c", UtterId[0], UtterId[1]);
   } /* endif */
}
/*---------------------------------------------------------------------------*/
/* if CDPresent SamCorpCode is the two first letters of the SourceFile Name */
/* (ex: SA), else SamCorpCode is the one given by user.                      */
void GetCorpusCod()
{
  char SentNum[80], * ptr;

  if (CDPresent) {
     strcpy(SentNum, SourceFile);
     if ((ptr = strrchr(SentNum, '.')) != NULL) *ptr = '\0';
     strncpy(SamCorpCode, SentNum, 2);
     SamCorpCode[2] = '\0';
  } else {
     printf(msg1);
     Keyboarding(SamCorpCode, 3);
  } /* endif */
}
/*---------------------------------------------------------------------------*/
/* Asks for Destination path and executes the command file.                  */
short LisCommandes ()
{
   char line[80], *ptr;
   char DestPath[80];
   short l;
   short Fin = FALSE;
   char SentNum[80];
   FILE * FichSent;

      if ((FichCmd = fopen(CmdFile, "rt")) == NULL ) {
         fprintf(stderr, error8 , CmdFile);
         return(FALSE);
      } else {
        *DestPath = '\0';
        printf(msg9);
        Keyboarding(DestPath, 80);
        printf("\n");
        l = strlen(DestPath);
        ptr = &DestPath[(l - 1)];
        if (*ptr == '\\')  *ptr = '\0';
        if (l == 1) { *(++ptr) = ':';
                      *(++ptr) = '\0';
        } /* endif */

        while (!Fin) {
         if (fgets(line, sizeof(line), FichCmd) == NULL) {
           if (feof(FichCmd)) Fin = TRUE;
         } else {
           if ((ptr = strrchr(line, '\n'))!= NULL) *ptr = '\0';
           if ((ptr = strrchr(line, '\\'))!= NULL) {
              strcpy(SourceFile, (ptr+1));
              *ptr = '\0';
              strcpy(SrcPath, line);
           } else {
              strcpy(SourceFile, line);
           } /* endif */

           /* test for an accompanying transcription file */
           strcpy(SentNum, SourceFile);
           if ((ptr = strrchr(SentNum, '.')) != NULL) *ptr = '\0';
           if (strlen(SrcPath) == 0) {
             sprintf(Warea, "%s\\%s.TXT", SpkDir, SentNum);
           } else {
             sprintf(Warea, "%s\\%s.TXT", SrcPath, SentNum);
           } /* endif */
           if ((FichSent = fopen(Warea, "rt")) != NULL ) {
             strcpy(PromptFile, Warea);
             fclose(FichSent);
           } else {
            *PromptFile = '\0';
           } /* endif */

           printf(msg18);
           if (ExtractInfos()) {
             GetSpkCod();
             GetCorpusCod();
               if ( strlen(DestPath) == 0) {
                 sprintf(DestFile, "%s%s%s.SAS", SamSpkCode, SamCorpCode , \
                 SamFileNum);
               } else {
                 sprintf(DestFile, "%s\\%s%s%s.SAS", DestPath, SamSpkCode, \
                 SamCorpCode, SamFileNum);
                 } /* endif */
                 strupr(DestFile);
                 if (CopySignal()) {
                   if (CopyLabel()) printf(msg7, DestFile, LabelFile);
                 } /* endif */
                 Increment();
               } /* endif */
           } /* endif */
        } /* endwhile */
      } /* endif */
}

/*---------------------------------------------------------------------------*/
/* Asks for CD present or not. If yes, asks for CD drive, CD name, Speech    */
/* directory, Documentation directory, text prompts filename.                */
short AskCDRom()
{
  char *ptr;

   printf(msg10);
   if (GetYesNo() == 'N') {
      printf(msg11);
      return(FALSE);
   } else {
      printf(msg12);
      Keyboarding(CDDrive, 2);
      printf(msg20);
      Keyboarding(CDDate, 5);
      ptr = &CDDate[2];
      DbYear = atoi(ptr);
      sprintf(PromptSpecif, "%c:", CDDrive[0]);
      sprintf(SpkDir, "%c:", CDDrive[0]);
      sprintf(SpeakersFile, "%c:", CDDrive[0]);
      printf(msg16);
      Keyboarding(PromptSpecif, 80);
      printf(msg15);
      Keyboarding(SpeakersFile, 80);
      return(TRUE);
   } /* endif */

}
/*---------------------------------------------------------------------------*/
/* Presentation.                                                             */
void Welcome()
{

 printf("*******************************************************************************\n");
 printf("*                            WELCOME  TO  CONVERT                     (v %s) *\n", RELEASE);
 printf("*                 ('DARPA' to 'SAM' format file conversion)      ICP - %s *\n", VDATE);
 printf("*******************************************************************************\n");
 printf("*       DARPA convention:   \\TIMIT\\TRAIN\\DR5\\MJPG0\\SA1.WAV                    *\n");
 printf("*       -----------------                                                     *\n");
 printf("*                     with    TIMIT: Corpus code                              *\n");
 printf("*                             TRAIN: Data usage                               *\n");
 printf("*                               DR5: Dialect region code                      *\n");
 printf("*                             MJPG0: Sex and speaker code                     *\n");
 printf("*                               SA1: Sentence text code                       *\n");
 printf("*                               WAV: means WAVeform file                      *\n");
 printf("*                                                                             *\n");
 printf("*       SAM convention:       XXYYnnnn.SAS   &  XXYYnnnn.SAO                  *\n");
 printf("*       ---------------       (speech file)     (label file produced          *\n");
 printf("*                                                automatically)               *\n");
 printf("*                     with    XX: speaker code                                *\n");
 printf("*                             YY: corpus code                                 *\n");
 printf("*                           nnnn: unique file number                          *\n");
 printf("*                              S: means Sentence                              *\n");
 printf("*                              A: means American                              *\n");
 printf("*                              S: means Sampled speech                        *\n");
 printf("*                              O: means Orthographic time-aligned labelling   *\n");
 printf("*******************************************************************************\n");

}
/*****************************************************************************/
/*                                 M A I N                                   */
/*****************************************************************************/
main(argc, argv, envp)
   int argc;
   char *argv[];
   char *envp;
{

   Welcome();
   CDPresent = AskCDRom();
   if (argc > 1 ) strcpy(CmdFile , argv[1]);
   else {
      printf(msg17);
      Keyboarding(CmdFile, 80);
   } /* endif */
   if (strlen(CmdFile) != 0) {
     printf(msg2);
     Keyboarding(SamFileNum, 5);
     LisCommandes();
   } else {
     InputCommandes();
   } /* endif */
   printf("\nBYE !!! \n");
   printf("*******************************************************************************\n");
}
