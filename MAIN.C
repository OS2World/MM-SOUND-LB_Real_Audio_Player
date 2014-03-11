#include <stdio.h>
#include <string.h>
void ra144(FILE *fdo,FILE *fd,FILE *fdl);
void ra288(FILE *fdo,FILE *fd,FILE *fdl);

int main(int ac, char **av)
{
   static unsigned char in[256];
   FILE *fdl,*fd,*fdo=NULL;

   fdl=stdout;
   if ((ac == 3)||(ac==2)) {
      if (strcmp(av[1],"-")) {
         fd=fopen(av[1],"rb");
         if(fd==0) {
            fprintf(stderr,"file not found %s",av[1]);
            exit(1);
            }
         }
      else fd=stdin;
      if (ac==3) {
         if (strcmp(av[2],"-")) {
            fdo=fopen(av[2],"wb");
            if (!fdo) {
               fprintf(stderr,"Can't open output file %s",av[2]);
               exit(1);
               }
            }
         else {
            fdo=stdout;
            fdl=stderr;
            }
         }
      }
   else {
      fprintf(stderr,"Usage: RAPlay {infile|-} [outfile|-]");
      exit(1);
      }

   fread(in,6,1,fd);
   if(!strncmp(in,".ra",3) &&in[3]==0xfd && in[4]==0) {
		if (in[5]==3)
  	{
	 		fprintf(fdl,"recognized format: 14.4kbaud file\n");
			ra144(fdo,fd,fdl);
  	}
		else if (in[5]==4)
  	{
			long pos,l;
			unsigned char sig2[12];
			fread(sig2,12,1,fd);
			fread(in,4,1,fd);
  		pos=(in[0]<<24|in[1]<<16|in[2]<<8|in[3])+0x10;
			fread(in,0x16,1,fd);
			if (!strncmp(sig2+2,".ra4",4) && in[4] == 0 || in[5] == 0x26
					&& in[0x12] == 0 && in[0x13] == 12 && in[0x14] == 0 && in[0x15] == 0xe4) {
	 			fprintf(fdl,"recognized format: 28.8kbaud file\n");
                        pos-=6+12+4+0x16;
                        while (pos>0) {
                           if (pos>256) l=256; else l=pos;
                           fread(in,l,1,fd);
                           pos-=l;
                           }
			ra288(fdo,fd,fdl);
			}
			else {
	 			fprintf(stderr,"recognized format: 28.8kbaud, unrecognized file parameters");
				exit(1);
			}
  	}
  	else {
	 		fprintf(stderr,"RA file format version %d not supported",in[5]);
			exit(1);
		}
	}
	else if (!strncmp(in,".RMF",4)) {
		fprintf(stderr,"Real media files (RMF) not currently supported");
		exit(1);
	}
	else {
		fprintf(stderr,"unrecognized file format");
		exit(1);
	}
	fprintf(fdl,"\nDecoding done");
	return 0;
}
