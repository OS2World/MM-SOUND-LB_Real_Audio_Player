
#define USE_OS2_TOOLKIT_HEADERS
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#define INCL_OS2MM

#include <stdio.h>
#include <os2.h>
#include <os2me.h>

#define DATA_RATE 8000
#define BUFFER_SIZE 16384
#define PRIORTY_DELTA 30

int use8bit=FALSE;
int dyn_prio=TRUE;
int max_buffers=16;

int dart_open();
void dart_flush();
void dart_close(void);
void dart_put(short *buff, int count);
LONG MyEvent (ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags);


static int gohigh,offhigh;
static MCI_MIX_BUFFER Buffers[100];
static MCI_MIXSETUP_PARMS MixSetupParms = {NULLHANDLE,0,MCI_WAVE_FORMAT_PCM,0,0,MCI_PLAY,MCI_DEVTYPE_WAVEFORM_AUDIO,0,NULL,NULL,MyEvent,NULL,0,0};
static MCI_AMP_OPEN_PARMS AmpOpenParms = {NULLHANDLE,0,0,(PSZ)MCI_DEVTYPE_AUDIO_AMPMIX,NULL,NULL,NULL};
static MCI_BUFFER_PARMS BufferParms = {NULLHANDLE,sizeof(MCI_BUFFER_PARMS),0,
				BUFFER_SIZE,0,0,0,Buffers};
static int buffcount=0,
    numout=0,
    blocking=FALSE,
    flushing=FALSE,
    hipri=FALSE,
    isplaying=FALSE;
static HEV sem;
static TID threadID;

LONG MyEvent (ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags) {
   if (ulFlags & MIX_STREAM_ERROR && ulStatus==ERROR_DEVICE_UNDERRUN) {
      if (!hipri && !flushing && dyn_prio) {
	 hipri=TRUE;
	 DosSetPriority(PRTYS_THREAD,PRTYC_NOCHANGE,PRIORTY_DELTA,threadID);
      }
      if (flushing) {
	 DosPostEventSem(sem);
	 blocking=FALSE;
      }
   }
   if (ulFlags & MIX_WRITE_COMPLETE) {
      numout--;
      if (numout<gohigh && !hipri && !flushing && dyn_prio) {
         hipri=TRUE;
	 DosSetPriority(PRTYS_THREAD,PRTYC_NOCHANGE,PRIORTY_DELTA,threadID);
      }
      if (blocking || (flushing && !numout)) {
         DosPostEventSem(sem);
         blocking=FALSE;
      }
   }
   return 0;
}

void error(ULONG rc) {
   char errstr[80];

   mciGetErrorString(rc,errstr,80);
   fprintf(stderr,"\nMMOS/2 error: %s\n",errstr);
}

int dart_open(void) {
   ULONG rc;
   PPIB ppib=NULL;
   PTIB ptib=NULL;

   rc = mciSendCommand(0,MCI_OPEN,MCI_WAIT | MCI_OPEN_TYPE_ID,&AmpOpenParms,0);
   if (ULONG_LOWD(rc)!=MCIERR_SUCCESS) error(rc);
   MixSetupParms.ulBitsPerSample=use8bit?8:16;
   MixSetupParms.ulSamplesPerSec=DATA_RATE;
   MixSetupParms.ulChannels=1;
   rc=mciSendCommand(AmpOpenParms.usDeviceID,MCI_MIXSETUP,
		     MCI_WAIT | MCI_MIXSETUP_INIT,&MixSetupParms,0);
   if (ULONG_LOWD(rc)!=MCIERR_SUCCESS) error(rc);
   if (!use8bit) BufferParms.ulBufferSize*=2;
   BufferParms.ulNumBuffers=max_buffers;
   gohigh=max_buffers/4;
   offhigh=max_buffers/2;
   rc=mciSendCommand(AmpOpenParms.usDeviceID,MCI_BUFFER,
                     MCI_WAIT | MCI_ALLOCATE_MEMORY,&BufferParms,0);
   if (ULONG_LOWD(rc)!=MCIERR_SUCCESS) error(rc);
   DosCreateEventSem(NULL,&sem,0,TRUE);
   DosGetInfoBlocks(&ptib,&ppib);
   threadID=ptib->tib_ptib2->tib2_ultid;
   return 0;
}

void dart_put(short *buff, int count) {
   static ULONG numposted;

   if (!isplaying) numout=0;

   if (numout>=max_buffers-1) {
      DosResetEventSem(sem,&numposted);
      blocking=TRUE;
      DosWaitEventSem(sem,-1);
   }
   if (numout>offhigh && hipri) {
      hipri=FALSE;
      DosSetPriority(PRTYS_THREAD,PRTYC_NOCHANGE,-PRIORTY_DELTA,threadID);
   }
   if (use8bit) {
      int i;
      unsigned char *dp;

      dp=Buffers[buffcount].pBuffer;
      for (i=count;i;i--) *dp++ = 0x80 ^ ((unsigned char) ((*buff++)>>8) );
      }
   else {
      count*=2;
      memcpy(Buffers[buffcount].pBuffer,buff,count);
      }
   Buffers[buffcount].ulBufferLength=count;
   MixSetupParms.pmixWrite(MixSetupParms.ulMixHandle,&Buffers[buffcount++],1);
   if (buffcount>=max_buffers) buffcount=0;
   numout++;
   isplaying=TRUE;
}

void dart_close(void) {
   MCI_GENERIC_PARMS GenericParms;
   ULONG rc;

   DosCloseEventSem(sem);
   rc=mciSendCommand(AmpOpenParms.usDeviceID,MCI_BUFFER,
                     MCI_WAIT | MCI_DEALLOCATE_MEMORY,&BufferParms,0);
   if (ULONG_LOWD(rc)!=MCIERR_SUCCESS) error(rc);
   rc=mciSendCommand(AmpOpenParms.usDeviceID,MCI_CLOSE,MCI_WAIT,&GenericParms,0);
   if (ULONG_LOWD(rc)!=MCIERR_SUCCESS) error(rc);
}

void dart_flush() {
   ULONG numposted;

   DosResetEventSem(sem,&numposted);
   flushing=TRUE;
   isplaying=FALSE;
   if (!numout) DosPostEventSem(sem);
   else DosWaitEventSem(sem,-1);
   flushing=FALSE;
   if (hipri) {
      hipri=FALSE;
      DosSetPriority(PRTYS_THREAD,PRTYC_NOCHANGE,-PRIORTY_DELTA,threadID);
   }
}

