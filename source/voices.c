#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "decoder.h"

#define DEFAULT_SPACING   2

void usage();
void getArguments();

int hideFileSize(unsigned * info);
int hideByte(unsigned * info);
int readHiddenByte(unsigned info);
int readFileSize(unsigned info);  

int main_data_slots();


FILE * ptrHiddenFile;
Bit_stream_struc  ptrNewMP3;
int hiding;
int bitSpacing;
unsigned int inFileSize;
int tempFilePtr;
int nBytesHidden = 0;
int nBytesRead = 0;
int hiddenFileSize = 2;

int main(int argc, char **argv) {
  Bit_stream_struc ptrOrigMP3;
  Arguments_t Arguments;
  
  int i, sync;
  frame_params fr_ps;
  layer info;
  unsigned int old_crc;
  unsigned long bitsPerSlot, samplesPerFrame;
  unsigned long frameBits, gotBits = 0;
  III_side_info_t III_side_info;

  unsigned long tempLong;
  int tempInt;

  fr_ps.header = &info;
  fr_ps.tab_num = -1;   
  fr_ps.alloc = NULL;

  getArguments(argc, argv, &Arguments);

  if (Arguments.hiding == 1) {
    hiding = 1;

    tempFilePtr = open (Arguments.hiddenFileName, O_RDONLY);
    inFileSize = lseek (tempFilePtr, 0, SEEK_END);
    close (tempFilePtr);

    ptrHiddenFile = fopen(Arguments.hiddenFileName, "rb");
    if (!(ptrHiddenFile)) {
      printf ("Aborted!\n");
      printf ("Unable to open file to be hidden: %s\n",
               Arguments.hiddenFileName);
      exit(1);
    }

  } else if (Arguments.hiding == 0){
    hiding = 0;

    ptrHiddenFile = fopen(Arguments.hiddenFileName, "wb");
    if (!(ptrHiddenFile)) {
      printf ("Aborted!\n");
      printf ("Unable to open file to write hidden file to: %s\n",
               Arguments.hiddenFileName);
      exit(1);
    }
  }

  bitSpacing = Arguments.bitSpacing;

  open_bit_stream_r(&ptrOrigMP3, Arguments.origMP3FileName, BUFFER_SIZE);

  if (hiding) {
    open_bit_stream_w(&ptrNewMP3, Arguments.newMP3FileName, BUFFER_SIZE);
    printf ("Hiding file...\n");
  } else {
    printf ("Reading hidden file...\n");
  }

  while (!end_bs(&ptrOrigMP3)) {
    int nSlots;
    int main_data_end, flush_main;
    int bytes_to_discard;
    static int frame_start = 0;

    sync = seek_sync(&ptrOrigMP3, SYNC_WORD, SYNC_WORD_LNGTH);
    frameBits = sstell(&ptrOrigMP3) - gotBits;
    gotBits += frameBits;

    if (!sync) {
      break;
    }

    decode_info(&ptrOrigMP3, &fr_ps);
    hdr_to_frps(&fr_ps);

    if (info.error_protection) {
      buffer_CRC(&ptrOrigMP3, &old_crc);
    }

    bitsPerSlot = 8;        
    if(fr_ps.header->version == MPEG_PHASE2_LSF)
      samplesPerFrame = 576;
    else
      samplesPerFrame = 1152;

    III_get_side_info(&ptrOrigMP3, &III_side_info, &fr_ps);
    if (hiding) {
      III_put_side_info(&ptrNewMP3, &III_side_info, &fr_ps);
    }

    nSlots = main_data_slots(fr_ps);
    for (; nSlots > 0; nSlots--){
      tempLong = getbits(&ptrOrigMP3,8);
      hputbuf((unsigned int) tempLong, 8);
      if (hiding) {
        putbits(&ptrNewMP3, tempLong, 8);
      }
    }

    main_data_end = hsstell() / 8;
    if ( flush_main=(hsstell() % bitsPerSlot) ) { 
      hgetbits((int)(bitsPerSlot - flush_main));
      main_data_end++;
    }
    bytes_to_discard = frame_start - main_data_end
                         - III_side_info.main_data_begin ;
    if(main_data_end > 4096) {
      frame_start -= 4096;
      rewindNbytes(4096);
    }

    frame_start += main_data_slots(fr_ps);
    for (; bytes_to_discard > 0; bytes_to_discard--) {
      hgetbits(8);
    }

    if (info.bitrate_index > 0) {
      long anc_len;
      anc_len = (int)((double)samplesPerFrame /
                s_freq[info.version][info.sampling_frequency] *
	        (double)bitrate[info.version][info.lay-1][info.bitrate_index] /
			       (double)bitsPerSlot);
      if (info.padding) {
        anc_len++;
      }
      anc_len *= bitsPerSlot;
      anc_len -= sstell(&ptrOrigMP3)-gotBits+SYNC_WORD_LNGTH;
      for (i=0; i<anc_len; i++) {
        tempInt = get1bit(&ptrOrigMP3);
        if (hiding) {
          put1bit(&ptrNewMP3, tempInt);
        }
      }
    }
  }

  if (hiding && nBytesHidden == 0)  {
    printf("Error!\n");
    printf("Either there were problems hiding data in specified mp3: %s\n", 
            Arguments.origMP3FileName);
    printf("Or the specified file to be hidden is empty: %s\n",
            Arguments.hiddenFileName);
  } else if (!(hiding) && nBytesRead == 0) {
    printf("Error! Problems reading data from specified mp3: %s\n", 
            Arguments.origMP3FileName);

  } else {

    printf("Success!\n");
    if (hiding) {
      printf ("The file %s has been hidden into %s at a spacing of %d.\n",
               Arguments.hiddenFileName,
               Arguments.origMP3FileName,
               bitSpacing);
      printf("(To extract the file you must use the same spacing value)\n");
    } else {
      printf ("The file %s has been extracted from %s at a spacing of %d.\n",
               Arguments.hiddenFileName,
               Arguments.origMP3FileName,
               bitSpacing);
    }

    if (hiding && (nBytesHidden < inFileSize)) {
      printf("Warning! This mp3 was not big enough to hide the entire file.\n");
      printf("Not all of the file was able to be hidden into the mp3.\n");
      printf("Try a smaller spacing or larger mp3 file.\n");
    } else if (!(hiding) && (nBytesRead < hiddenFileSize)) {
      printf("Warning! The extracted hidden file may be incomplete.\n");
      printf("The hidden file size was larger than what was extracted.\n");
      printf("If possible try hiding the file again.\n");
    } 
  }     

  close_bit_stream_r(&ptrOrigMP3);
  if (hiding) {
    close_bit_stream_w(&ptrNewMP3);
  } 
  fclose(ptrHiddenFile);

  if (hiding && nBytesHidden != 0) {
    tempFilePtr = open (Arguments.newMP3FileName, O_RDONLY);
    inFileSize = lseek (tempFilePtr, 0, SEEK_END);
    close (tempFilePtr);
    if (truncate (Arguments.newMP3FileName, (inFileSize - 2)) != 0){
      printf("Error truncating new mp3 file %s.\n", 
                                        Arguments.newMP3FileName);
    }

  }

  exit(0);
}

void usage() {
  printf ("usage: voices\n");
  printf ("\n");
  printf ("For hiding a file in an mp3:\n");
  printf ("\n");
  printf ("      voices -h [-c N] origMP3 modifiedMP3 hiddenFile\n");
  printf ("\n");
  printf ("For reading a hidden file from an mp3:\n");
  printf ("\n");
  printf ("      voices -r [-c N] modifiedMP3 newFile\n");
  printf ("\n");
  printf ("where\n");
  printf ("  -h           hiding file in mp3\n");
  printf ("  -r           reading hidden file from mp3\n");
  printf ("  -c N         bit spacing of hidden file in mp3(N defaults to %d)\n", DEFAULT_SPACING);
  printf ("  origMP3      original mp3 file to create a new mp3 from (with\n");
  printf ("               the hidden file inside. origMP3 will not be modified)\n");
  printf ("  modifiedMP3  name of modified mp3 that contains the hidden file\n");
  printf ("  hiddenFile   name of file to be hidden in mp3\n");
  printf ("  newFile      name of hidden file that is extracted from mp3\n");
  printf ("  -help        to display this usage menu\n");
  printf ("\n");
  printf ("  NOTES: For -c N, N must be greater than 0.\n");
  printf ("\n");
  printf ("         For -c N, N must be the same value for hiding and\n");
  printf ("         reading to be able to retrieve the hidden file.\n");
  printf ("\n");
  printf ("         If the modifiedMP3 has some audible differences\n");
  printf ("         (ex. slight tremelo effect) try increasing N to see\n");
  printf ("         if this helps.  But remember, by increasing N you will\n");
  printf ("         be decreasing the maximum size of a file that can be\n");
  printf ("         hidden.\n");
  printf ("\n");
  printf ("         To increase the maximum file size that can be hidden in\n");
  printf ("         a particular mp3 decrease N (-c N). But remember, low values\n");
  printf ("         of N may create slight audible differences. This will\n");
  printf ("         depend on the particular mp3.\n");
  printf ("\n");
 
  exit(1);
}

void getArguments(int argc, char **argv, Arguments_t *Arguments) {
  int i = 0;
  int  err = 0;

  if(argc == 1 || 
    (strcmp(argv[1], "-help") == 0)) {
    usage();

  } else {
    Arguments->origMP3FileName[0] = '\0';
    Arguments->newMP3FileName[0] = '\0';
    Arguments->hiddenFileName[0] = '\0';
    Arguments->hiding = 2;
    Arguments->bitSpacing = DEFAULT_SPACING;

    while(++i<argc && err == 0) {
      char c, *token, *arg, *nextArg;
      int argUsed;
      token = argv[i];

      if(*token++ == '-') {
        if(i+1 < argc) {
          nextArg = argv[i+1];
        } else {
          nextArg = "";
        }
        argUsed = 0;
        while(c = *token++) {
          if(*token) {
            arg = token;
          } else {
            arg = nextArg;
          }

          switch(c) {
            case 'c': Arguments->bitSpacing = atoi(arg);
                      argUsed = 1;
                      break;
 
            case 'r': Arguments->hiding = 0;
                      break;

            case 'h': Arguments->hiding = 1;
                      break;

            default: fprintf(stderr,"voices: unrecognized option %c\n", c);
                     err = 1; 
                     break;
          }

          if(argUsed) {
            if(arg == token) {
              token = ""; 
            } else {
              ++i; 
            }
            arg = ""; 
            argUsed = 0;
          }
        }

      } else {
        if(Arguments->origMP3FileName[0] == '\0') {
          strcpy(Arguments->origMP3FileName, argv[i]);

        } else if(Arguments->newMP3FileName[0] == '\0' &&
                  Arguments->hiding == 1) {
          strcpy(Arguments->newMP3FileName, argv[i]);

        } else if(Arguments->hiddenFileName[0] == '\0' &&
	          Arguments->hiding == 1) {
          strcpy(Arguments->hiddenFileName, argv[i]); 

        } else if(Arguments->hiddenFileName[0] == '\0' &&
                  Arguments->hiding == 0) {
          strcpy(Arguments->hiddenFileName, argv[i]);
           
        } else {
          fprintf(stderr,"voices: excess arg %s\n", argv[i]);
          err = 1;
        }
      }
    }

    if (err)  {
      printf ("Type \"voices -help\" for usage.\n");
      exit(1);
    }

    if (Arguments->origMP3FileName[0] == '\0')  {
      if (Arguments->hiding == 1) {
        printf("Need to specify name of original mp3 to hide file in.\n");
      } else {
        printf("Need to specify name of mp3 that contains the hidden file.\n");
      }
      printf("Type \"voices -help\" for usage.\n");
      exit(1);
    }

    if (Arguments->hiding == 1) {
      if (Arguments->newMP3FileName[0] == '\0') {
        printf("Need to specify new mp3 file name.\n");
        printf("Type \"voices -help\" for usage.\n");
        exit(1);
      }
      if (Arguments->hiddenFileName[0] == '\0' && 
          Arguments->hiding == 1) {
        printf("Need to specify the name of the file to be hidden.\n");
        printf("Type \"voices -help\" for usage.\n");
        exit(1);
      }
      if (Arguments->hiddenFileName[0] == '\0' && 
          Arguments->hiding == 0) {
        printf("Need to specify the name of the extracted hidden file.\n");
        printf("Type \"voices -help\" for usage.\n");
        exit(1);
      }
    }

    if (Arguments->hiding == 2) {
      printf ("Need to specify hiding(-h) or reading(-r).\n");
      printf("Type \"voices -help\" for usage.\n");
      exit(1); 
    }

    if (Arguments->bitSpacing < 1) {
      printf("Spacing number(-c) needs to be larger than 0.\n");
      printf("Type \"voices -help\" for usage.\n");
      exit(1);
    }
  }
}

int hideFileSize(unsigned * info) {
  int msb = 0;
  static int nBits = 31;
  
  static int nBitsAdjust = 0;
  static int hideSizeStage = 1;
  static int headerSize = 4;

  if (hideSizeStage == 1) {
    while (msb != 1 && nBits >= 0){
      msb = (inFileSize & (1 << nBits)) ? 1 : 0;
      --nBits;
    }

    nBitsAdjust = nBits + 2;

    ++hideSizeStage;
    return 0;

  } else if (hideSizeStage == 2) {
    msb = (nBitsAdjust & (1 << headerSize)) ? 1 : 0;
    if(msb == 1) {
      *info = *info | 0x0001;
    } else {
      *info = *info & 0xfffe;
    }
    --headerSize;

    if (headerSize < 0) {
      ++hideSizeStage;
    }
    return 0;

  } else if (hideSizeStage == 3) {
    msb = (inFileSize & (1 << nBits)) ? 1 : 0;

    if(msb == 1) {
      *info = *info | 0x0001;
    } else {
      *info = *info & 0xfffe;
    }
    --nBits;

    if (nBits < 0) {
      ++hideSizeStage;
    }
    return 0;

  } else {
    return 1;
  }
}

int hideByte(unsigned * info) {
  char byte;    
  int msb = 0;  
  static int needNewByte = 1;   
  static unsigned char currByte = 0;
  static int bitNum = 0; 

  if (needNewByte) {
    if (!(fread(&byte, sizeof(char),1, ptrHiddenFile))) {
      return 1;
    }
    currByte = byte;

    needNewByte = 0;
  }

  msb = (currByte & (1 << 7)) ? 1 : 0;
  if(msb == 1) {
    *info = *info | 0x0001;
  } else {
    *info = *info & 0xfffe;
  }

  currByte = currByte << 1;
  ++bitNum;

  if (bitNum == 8) {
    needNewByte = 1;
    bitNum = 0;
  
    ++nBytesHidden;
  }

  return 0;
}

int readFileSize(unsigned info) {
  int lsb;             
  static int headerSize = 5;  
  static int nBits = 0;     
  static int readSizeStage = 1;  
  if (readSizeStage == 1) {
    lsb = (info & (1)) ? 1 : 0;
    nBits = nBits + lsb;
    --headerSize;

    if (headerSize < 0) {
      nBits = nBits & 0xffdf;
      ++readSizeStage;
      return 0;
    } else {
      nBits = nBits << 1;
      return 0;
    }

  } else if (readSizeStage == 2) {
    lsb = (info & (1)) ? 1 : 0;

    hiddenFileSize = hiddenFileSize + lsb;
    --nBits;

    if ((nBits-2) < 0) {
      ++readSizeStage;
      return 0;
    } else {
      hiddenFileSize = hiddenFileSize << 1;
      return 0;
    }

  } else {
    return 1;
  }
}

int readHiddenByte(unsigned info) {
  int lsb;  
  static int needNewByte = 1;  
  static unsigned char currByte = 0;  

  lsb = (info & (1)) ? 1 : 0;
  currByte = currByte + lsb;
  ++needNewByte;

  if (needNewByte > 8) {
    if (nBytesRead == hiddenFileSize) {
      return 1;
    }

    fputc(currByte, ptrHiddenFile);
    ++nBytesRead;

    needNewByte = 1;
    currByte = 0;

  } else {
    currByte = currByte << 1;
  }

  return 0;
}