#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


#include "aal2sdk.h"

aat_word32 gTestIndex;
aat_word32 gTestSuccess;
aat_word32 gTestFailure;

#define LOG_FILENAME "debug.log"

void pause()
{
  int c;
  printf("press enter to continue...\n");
  do
  {
    c = getchar();
    if(c == EOF) break;
  } while(c != '\n');
}

/************************************************************************/
/* Log message and result functions                                     */
/************************************************************************/

void LogMessage(aat_ascii *format, ...)
{
  va_list marker;
  FILE *logfile;

  va_start(marker, format);      /* initialize variable arguments. */
  vprintf( format, marker);
  va_end( marker );              /* Reset variable arguments.      */

  logfile= fopen(LOG_FILENAME, "a+");
  if ( logfile != NULL )
  {
    va_start(marker, format);      /* initialize variable arguments. */
    vfprintf( logfile, format, marker);
    va_end( marker );              /* Reset variable arguments.      */
    fclose( logfile);
  }

  return;
} /* LogToFile */

void LogOutput(aat_ascii *szFunctionName,
               aat_int32  ReturnCode,
               aat_int32  ExpectedReturnCode,
               aat_ascii *szTestName)
{
  aat_ascii ErrMsg[255];

  gTestIndex++;

  LogMessage("\nVC Test %d %s %s", gTestIndex, szFunctionName, szTestName);

  /* Is Test successful */
  if (ReturnCode != ExpectedReturnCode)
  {
    AAL2GetErrorMsg(ReturnCode, ErrMsg);
    LogMessage("\n   %s returned: %d - %s", szFunctionName, ReturnCode, ErrMsg);
    LogMessage("\nVC Test %d %s : FAILED", gTestIndex, szTestName);
    gTestFailure++;
  }
  else
  {
    LogMessage("\nVC Test %d %s %s : OK", gTestIndex, szFunctionName, szTestName);
    gTestSuccess++;
  }
}

void LogOutputDPX(aat_ascii *szFunctionName,
                  aat_int32  ReturnCode,
                  aat_int32  ExpectedReturnCode,
                  aat_ascii *szTestName)
{
  aat_ascii ErrMsg[255];

  gTestIndex++;

  LogMessage("\nVC Test %d %s %s", gTestIndex, szFunctionName, szTestName);

  /* Is Test successful */
  if (ReturnCode != ExpectedReturnCode)
  {
    AAL2DPXGetErrorMsg(ReturnCode, ErrMsg);
    LogMessage("\n   %s returned: %d - %s", szFunctionName, ReturnCode, ErrMsg);
    LogMessage("\nVC Test %d %s : FAILED", gTestIndex, szTestName);
    gTestFailure++;
  }
  else
  {
    LogMessage("\nVC Test %d %s %s : OK", gTestIndex, szFunctionName, szTestName);
    gTestSuccess++;
  }
}

/************************************************************************/
/* DPX blobs import implementation                                      */
/************************************************************************/

aat_int32 DPXImport(TDigipassBlob DPBlob[],
                    TKernelParms  *CallParms,
                    aat_ascii     *DPXFile,
                    aat_ascii     *InitKey,
                    aat_ascii     *TestDesc)
{
    aat_int32  ret = 0;
    TDPXHandle DPXHandle;
    char       ApplNames[13*8];
    aat_int16  ApplCount;
    aat_int16  TokenCount;
    char       sw_out_serial_no[8][23];
    char       sw_out_type[5+1];
    char       sw_out_authmode[8][2];


    /* Read DPX file */
    ret = AAL2DPXInit(  &DPXHandle,
                        DPXFile,
                        InitKey,
                        &ApplCount,
                        ApplNames,
                        &TokenCount);

    if (ret) {
        LogOutputDPX("AAL2DPXInit", ret, 0, TestDesc);
        return ret;
    }

    /* Get all application blobs for this token */
    ret = AAL2DPXGetTokenBlobs( &DPXHandle,
                                CallParms,
                                &ApplCount,
                                sw_out_serial_no,   /* Serial number and appli name of each application */
                                sw_out_type,        /* DIGIPASS type */
                                sw_out_authmode,    /* Authentication mode of each application */
                                DPBlob);            /* DIGIPASS blobs */


    if (ret != 100)
    {
        AAL2DPXClose(&DPXHandle);
        LogOutputDPX("AAL2DPXGetTokenBlobs", ret, 100, TestDesc);
        return ret;
    }

    ret = AAL2DPXClose(&DPXHandle);

    if (ret)
    {
        LogOutputDPX("AAL2DPXClose", ret, 0, TestDesc);
        return ret;
    }

    LogOutputDPX("AAL2DPXGetTokenBlobs", ret, 0, TestDesc);

    return ret;

}

/************************************************************************/
/* VACMAN Controller sample main program:                               */
/************************************************************************/
int main( )
{
    TKernelParms    KernelParms;
    aat_int32       ret;

   aat_ascii       ChallengeMessage;
    TDigipassBlob   DPData[8];
    aat_ascii       *p;
    aat_ascii Password[40+1];
    aat_ascii       Challenge[16+1];
    aat_ascii       Signature[22];
    aat_ascii       SignedDataFields[8][20];
    aat_ascii       HostCode[16+1];
    aat_int32       ChallengeLen;
    aat_int32       DataFieldCount;
    aat_int32       HostCodeLen;
    time_t          now;
    aat_int16       appl_count ;
    aat_int32        Property;
    aat_ascii       Value;
    aat_ascii       szLastTimeUsed[24+1];
    aat_int32       LastTimeUsed;
    char buffer[256];
    aat_ascii       szTime[27];
    aat_ascii       szMessage[80];

    int       i;

    gTestIndex   = 0;
    gTestSuccess = 0;
    gTestFailure = 0;


    LogMessage("************************************************************\n");
    LogMessage("        Authentification forte a deux vacteur .\n");
    LogMessage("       Realiser par M@hdiben ohtmen && Med amine jridi\n");
    LogMessage("**************Encadrer par : Mr Marouene Boubakri************\n");


    /************************************************************************/
    /* Time settings Display                                                */
    /************************************************************************/
    LogMessage("Time settings : \n");

    now = time(NULL);
    strftime (szTime, 21, "%m/%d/%Y %H:%M:%S", localtime(&now));
    LogMessage("\n  Local Time : %s", szTime);
    strftime (szTime, 21, "%m/%d/%Y %H:%M:%S", gmtime(&now));
    LogMessage("\n  Gmt Time   : %s\n\n\n", szTime);




    /*Initialize KernelParms*/

    /****************************************************************************/
    /* The runtime kernel parameters are initialized here with some specific    */
    /* values for the context of this sample.                                   */
    /* CAUTION: The runtime kernel parameters are very important as they modify */
    /* the behavior of some specific features.                                  */
    /* For default values and details about each runtime parameters, refer to   */
    /* the VACMAN Controller Product Guide, section "Runtime Parameters".       */
    /****************************************************************************/

    memset(&KernelParms, 0, sizeof(TKernelParms));
    KernelParms.ParmCount           = 19;         /* Number of valid parameters in this list            */
    KernelParms.ITimeWindow         = 30;         /* Identification Window size in time steps           */
    KernelParms.STimeWindow         = 24;         /* Signature Window size in secs                      */
    KernelParms.DiagLevel           = 0;          /* Requested Diagnostic Level                         */
    KernelParms.GMTAdjust           = 0;          /* GMT Time adjustment to perform                     */
    KernelParms.CheckChallenge      = 0;          /* Verify Challenge Corrupted (mandatory for Gordian) */
    KernelParms.IThreshold          = 3;          /* Identification Error Threshold                     */
    KernelParms.SThreshold          = 1;          /* Signature Error Threshold                          */
    KernelParms.ChkInactDays        = 0;          /* Check Inactive Days                                */
    KernelParms.DeriveVector        = 0;          /* Vector used to make Data Encryption unique         */
    KernelParms.SyncWindow          = 1;          /* Synchronisation Time Window (h)                    */
    KernelParms.OnLineSG            = 1;          /* Online  Signature                                  */
    KernelParms.EventWindow         = 30;         /* Event Window size in nbr of iterations             */
    KernelParms.HSMSlotId           = 0;          /* Slot id uses to store Storage and Transport Key    */
    KernelParms.StorageKeyId        = 0;
    KernelParms.TransportKeyId      = 0x7FFFFF;
    KernelParms.StorageDeriveKey1   = 0;
    KernelParms.StorageDeriveKey2   = 0;
    KernelParms.StorageDeriveKey3   = 0;
    KernelParms.StorageDeriveKey4   = 0;

    /*  1) Load Response-Only, Signature and Challenge/Response blobs from demo DPX file */
    /*  2) Validate a Respnse-only OTP */
    /*  3) Catch a Code Replay Attempt */
    /*  4) Validate a Signature */
    /*  5) Validate a Challenge/Response OTP */
     /********************************************/
    /* 0) Generate password OTP */
    /********************************************/
/*ret=AAL2GenerateChallengeEx (
                             &DPData,
                              &KernelParms,
                            Challenge,
                             &ChallengeLen,
                        &ChallengeMessage);
      LogOutput("AAL2GenerateChallengeEx", ret, 0, "Validate a Response-Only OTP");*/


/*ret=AAL2SetTokenProperty(
&DPData, &KernelParms, szLastTimeUsed, szTime);

AAL2SetTokenProperty(
&DPData, &KernelParms, LAST_TIME_USED | INT_VALUE, (aat_ascii*)
&LastTimeUsed);


// output example: 1186568676
/*printf("%d\n",LastTimeUsed);
*/

   /* LogOutput("AAL2GetTokenProperty", ret, 0, " ");*/
  /*ret = AAL2GenPasswordEx( &DPData,
                               &KernelParms,
                              buffer,
                               NULL, /* No Challenge */
                             /*  HostCode,
                               &HostCodeLen );*/

    /********************************************/
    /* 1) Import Digipass Blobs from  DPX file  */
    /********************************************/
     ret = DPXImport(DPData,
                    &KernelParms,
                    "demo.dpx",
                    "11111111111111111111111111111111",
                    "DPX Import Demo Token");
    if (ret != 0)
    {
        LogMessage("\n");
        return ret;
    }
    AAL2SyncTokenBlob (&DPData[8],
                               KernelParms.SyncWindow  ,
                            &KernelParms)  ;

AAL2GenPassword( &DPData,
                               &KernelParms,
                              Password,
                               NULL /* No Challenge */
                             );
  LogOutput("AAL2GenPasswordEx", ret, 0, " ");
    /************************************/
    /* 2) Validate a Response-Only OTP  */
    /************************************/

    LogMessage("\n  DEMO APPLI 1:\n  Enter Digipass password: ");
    fgets(Password, sizeof(Password), stdin);
    /* Discard newline character */
    if((p = strchr(Password, '\n')) != NULL)
        *p = '\0';

    ret = AAL2VerifyPasswordEx(&DPData[0],
                               &KernelParms,
                               Password,
                               NULL, /* No Challenge */
                               HostCode,
                               &HostCodeLen );

    LogOutput("AAL2VerifyPasswordEx", ret, 0, "Validate a Response-Only OTP");
    LogMessage("\n");

    /***********************************/
    /* 3) Catch a Code Replay Attempt  */
    /***********************************/
    ret = AAL2VerifyPasswordEx(&DPData[0],
                               &KernelParms,
                               Password,
                               0, /* No Challenge */
                               HostCode,
                               &HostCodeLen );

  LogOutput("AAL2VerifyPasswordEx", ret, 0, "Validate a Response-Only OTP");
    LogMessage("\n");


    /****************************/
    /* 4) Validate a Signature  */
    /****************************/
    memset(SignedDataFields, 0, sizeof(SignedDataFields));
    DataFieldCount = 3;
    strcpy(SignedDataFields[0], "11111");
    strcpy(SignedDataFields[1], "22222");
    strcpy(SignedDataFields[2], "33333");

    LogMessage("\n  DEMO APPLI 2:");

    for (i = 0; i < DataFieldCount; i++)
    {
        sprintf(szMessage, "\n  Data field %d: %s", i+1, SignedDataFields[i]);
        LogMessage(szMessage);
    }

    LogMessage("\n  Enter Digipass signature: ");
    fgets(Signature, sizeof(Signature), stdin);
    /* Discard newline character */
    if((p = strchr(Signature, '\n')) != NULL)
        *p = '\0';

    ret = AAL2VerifySignatureEx(&DPData[1],
                                &KernelParms,
                                Signature,
                                SignedDataFields,
                                DataFieldCount,
                                0,
                                HostCode,
                                &HostCodeLen );

    LogOutput("AAL2VerifySignatureEx", ret, 0, "Validate a signature");
    LogMessage("\n");

    /******************************************/
    /* 5) Validate a Challenge/Response OTP   */
    /******************************************/
    ChallengeLen = 0;
    ret = AAL2GenerateChallenge((TDigipassBlob *)&DPData[2], &KernelParms, Challenge, &ChallengeLen);

    if (ret == 0)
    {
        sprintf(szMessage, "\n  DEMO APPLI 3:\n  Enter Digipass password for challenge %s: ", Challenge);
        LogMessage(szMessage);

        fgets(Password, sizeof(Password), stdin);
        /* Discard newline character */
        if((p = strchr(Password, '\n')) != NULL)
            *p = '\0';

        ret = AAL2VerifyPasswordEx(&DPData[2],
                                   &KernelParms,
                                   Password,
                                   Challenge,
                                   HostCode,
                                   &HostCodeLen );
        LogOutput("AAL2VerifyPasswordEx", ret, 0, "Validate a C/R OTP");
    }
    else
        LogOutput("AAL2GenerateChallenge", ret, 0, "Validate a C/R OTP");

    LogMessage("\n");

    /************************/
    /* Test result summary  */
    /************************/
    LogMessage("\n\n  Successful Tests - %d", gTestSuccess);
    LogMessage("\n  Failed     Tests - %d", gTestFailure);

    LogMessage("\n");
    LogMessage("\n");
    pause();

    return 0;
}


