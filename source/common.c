#include        "common.h"

#ifdef  MACINTOSH

#include        <SANE.h>
#include        <pascal.h>

#endif

#include	<string.h>
#include	<ctype.h>

extern Bit_stream_struc ptrNewMP3;
extern int hiding;

char *mode_names[5] = { "stereo", "j-stereo", "dual-ch", "single-ch" , "multi-ch"};
char *layer_names[3] = { "I", "II", "III" };
char *version_names[2] = { "MPEG-2 LSF", "MPEG-1" };


double  s_freq[2][4] = {{22.05, 24, 16, 0}, {44.1, 48, 32, 0}};

int     bitrate[2][3][15] = {{
          {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256},
          {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160},
          {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160}},
   
          {{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
          {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
          {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
          }};

double FAR multiple[64] = {
2.00000000000000, 1.58740105196820, 1.25992104989487,
1.00000000000000, 0.79370052598410, 0.62996052494744, 0.50000000000000,
0.39685026299205, 0.31498026247372, 0.25000000000000, 0.19842513149602,
0.15749013123686, 0.12500000000000, 0.09921256574801, 0.07874506561843,
0.06250000000000, 0.04960628287401, 0.03937253280921, 0.03125000000000,
0.02480314143700, 0.01968626640461, 0.01562500000000, 0.01240157071850,
0.00984313320230, 0.00781250000000, 0.00620078535925, 0.00492156660115,
0.00390625000000, 0.00310039267963, 0.00246078330058, 0.00195312500000,
0.00155019633981, 0.00123039165029, 0.00097656250000, 0.00077509816991,
0.00061519582514, 0.00048828125000, 0.00038754908495, 0.00030759791257,
0.00024414062500, 0.00019377454248, 0.00015379895629, 0.00012207031250,
0.00009688727124, 0.00007689947814, 0.00006103515625, 0.00004844363562,
0.00003844973907, 0.00003051757813, 0.00002422181781, 0.00001922486954,
0.00001525878906, 0.00001211090890, 0.00000961243477, 0.00000762939453,
0.00000605545445, 0.00000480621738, 0.00000381469727, 0.00000302772723,
0.00000240310869, 0.00000190734863, 0.00000151386361, 0.00000120155435,
1E-20
};

FILE *OpenTableFile(name)
char *name;
{
char fulname[80];
char *envdir;
FILE *f;

     fulname[0] = '\0';

#ifdef TABLES_PATH
       strcpy(fulname, TABLES_PATH);
#endif

#ifdef UNIX
       {
        char *getenv();

        envdir = getenv(MPEGTABENV);
        if(envdir != NULL)
            strcpy(fulname, envdir);
        strcat(fulname, PATH_SEPARATOR);
      }
#endif

    strcat(fulname, name);
    if( (f=fopen(fulname,"r"))==NULL ) {
        fprintf(stderr,"OpenTable: could not find %s\n", fulname);

#ifdef UNIX
          if(envdir != NULL)
            fprintf(stderr,"Check %s directory '%s'\n",MPEGTABENV, envdir);
          else
            fprintf(stderr,"Check local directory './%s' or setenv %s\n",
                    TABLES_PATH, MPEGTABENV);
#else 

#ifdef TABLES_PATH
            fprintf(stderr,"Check local directory './%s'\n",TABLES_PATH);
#endif
#endif
    }
    return f;
}

int read_bit_alloc(table, alloc)
int table;
al_table *alloc;
{
        unsigned int a, b, c, d, i, j;
        FILE *fp;
        char name[16], t[80];
        int sblim;

        strcpy(name, "alloc_0");

        switch (table) {
                case 0 : name[6] = '0';         break;
                case 1 : name[6] = '1';         break;
                case 2 : name[6] = '2';         break;
                case 3 : name[6] = '3';         break;
		case 4 : name[6] = '4';		break;
                default : name[6] = '0';
        }

        if (!(fp = OpenTableFile(name))) {
                printf("Please check bit allocation table %s\n", name);
                exit(1);
        }

        printf("using bit allocation table %s\n", name);

        fgets(t, 80, fp);
        sscanf(t, "%d\n", &sblim);
        while (!feof(fp)) {
                fgets(t, 80, fp);
                sscanf(t, "%d %d %d %d %d %d\n", &i, &j, &a, &b, &c, &d);
                        (*alloc)[i][j].steps = a;
                        (*alloc)[i][j].bits  = b;
                        (*alloc)[i][j].group = c;
                        (*alloc)[i][j].quant = d;
        }
        fclose(fp);
        return sblim;
}

int pick_table(fr_ps)
frame_params *fr_ps;
{
        int table, lay, ws, bsp, br_per_ch, sfrq,ver;
        int sblim = fr_ps->sblimit;
	         
	ver = fr_ps->header->version;
        lay = fr_ps->header->lay - 1;
        bsp = fr_ps->header->bitrate_index;
        br_per_ch = bitrate[ver][lay][bsp] / fr_ps->stereo;
        ws = fr_ps->header->sampling_frequency;
        sfrq = s_freq[ver][ws];

	if (fr_ps->header->version == MPEG_AUDIO_ID) {
	    if ((sfrq == 48 && br_per_ch >= 56) ||
		(br_per_ch >= 56 && br_per_ch <= 80)) table = 0;
	    else if (sfrq != 48 && br_per_ch >= 96) table = 1;
	    else if (sfrq != 32 && br_per_ch <= 48) table = 2;
	    else table = 3;
	}
	else {
	    table = 4;
	}

        if (fr_ps->tab_num != table) {
           if (fr_ps->tab_num >= 0)
              mem_free((void **)&(fr_ps->alloc));
           fr_ps->alloc = (al_table FAR *) mem_alloc(sizeof(al_table),
                                                         "alloc");
           sblim = read_bit_alloc(fr_ps->tab_num = table, fr_ps->alloc);
        }
        return sblim;
}

int js_bound(lay, m_ext)
int lay, m_ext;
{
static int jsb_table[3][4] =  { { 4, 8, 12, 16 }, { 4, 8, 12, 16},
                                { 0, 4, 8, 16} };

    if(lay<1 || lay >3 || m_ext<0 || m_ext>3) {
        fprintf(stderr, "js_bound bad layer/modext (%d/%d)\n", lay, m_ext);
        exit(1);
    }
    return(jsb_table[lay-1][m_ext]);
}

void hdr_to_frps(fr_ps)
frame_params *fr_ps;
{
layer *hdr = fr_ps->header;

    fr_ps->actual_mode = hdr->mode;
    fr_ps->stereo = (hdr->mode == MPG_MD_MONO) ? 1 : 2;
    if (hdr->lay == 2)          fr_ps->sblimit = pick_table(fr_ps);
    else                        fr_ps->sblimit = SBLIMIT;
    if(hdr->mode == MPG_MD_JOINT_STEREO)
        fr_ps->jsbound = js_bound(hdr->lay, hdr->mode_ext);
    else
        fr_ps->jsbound = fr_ps->sblimit;
}

void WriteHdr(fr_ps, s)
frame_params *fr_ps;
FILE *s;
{
layer *info = fr_ps->header;

   fprintf(s, "HDR: s=FFF, id=%X, l=%X, ep=%s, br=%X, sf=%X, pd=%X, ",
           info->version, info->lay, ((info->error_protection) ? "on" : "off"),
           info->bitrate_index, info->sampling_frequency, info->padding);
   fprintf(s, "pr=%X, m=%X, js=%X, c=%X, o=%X, e=%X\n",
           info->extension, info->mode, info->mode_ext,
           info->copyright, info->original, info->emphasis);
   fprintf(s, "alg.=%s, layer=%s, tot bitrate=%d, sfrq=%.1f\n",
           version_names[info->version], layer_names[info->lay-1],
	   bitrate[info->version][info->lay-1][info->bitrate_index],
           s_freq[info->version][info->sampling_frequency]);
   fprintf(s, "mode=%s, sblim=%d, jsbd=%d, ch=%d\n",
           mode_names[info->mode], fr_ps->sblimit, fr_ps->jsbound, fr_ps->stereo);
   fflush(s);
}

void WriteBitAlloc(bit_alloc, f_p, s)
unsigned int bit_alloc[2][SBLIMIT];
frame_params *f_p;
FILE *s;
{
int i,j;
int st = f_p->stereo;
int sbl = f_p->sblimit;
int jsb = f_p->jsbound;

    fprintf(s, "BITA ");
    for(i=0; i<sbl; ++i) {
        if(i == jsb) fprintf(s,"-");
        for(j=0; j<st; ++j)
            fprintf(s, "%1x", bit_alloc[j][i]);
    }
    fprintf(s, "\n");   fflush(s);
}

void WriteScale(bit_alloc, scfsi, scalar, fr_ps, s)
unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT], scalar[2][3][SBLIMIT];
frame_params *fr_ps;
FILE *s;
{
int stereo  = fr_ps->stereo;
int sblimit = fr_ps->sblimit;
int lay     = fr_ps->header->lay;
int i,j,k;

        if(lay == 2) {
            fprintf(s, "SFSI ");
            for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
                if (bit_alloc[k][i])  fprintf(s,"%d",scfsi[k][i]);
            fprintf(s, "\nSCFs ");
            for (k=0;k<stereo;k++) {
                for (i=0;i<sblimit;i++)
                    if (bit_alloc[k][i])
                        switch (scfsi[k][i]) {
                          case 0: for (j=0;j<3;j++)
                                  fprintf(s,"%2d%c",scalar[k][j][i],
                                          (j==2)?';':'-');
                                  break;
                          case 1:
                          case 3: fprintf(s,"%2d-",scalar[k][0][i]);
                                  fprintf(s,"%2d;",scalar[k][2][i]);
                                  break;
                          case 2: fprintf(s,"%2d;",scalar[k][0][i]);
                        }
                fprintf(s, "\n");
            }
        }
        else{
            fprintf(s, "SCFs ");
            for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
                if (bit_alloc[k][i])  fprintf(s,"%2d;",scalar[k][0][i]);
            fprintf(s, "\n");
        }
}

void WriteSamples(ch, sample, bit_alloc, fr_ps, s)
int ch;
unsigned int FAR sample[SBLIMIT];
unsigned int bit_alloc[SBLIMIT];
frame_params *fr_ps;
FILE *s;
{
int i;
int stereo = fr_ps->stereo;
int sblimit = fr_ps->sblimit;

        fprintf(s, "SMPL ");
        for (i=0;i<sblimit;i++)
                if ( bit_alloc[i] != 0)
                    fprintf(s, "%d:", sample[i]);
        if(ch==(stereo-1) )     fprintf(s, "\n");
        else                    fprintf(s, "\t");
}

int NumericQ(s)
char *s;
{
char    c;

    while( (c = *s++)!='\0' && isspace((int)c))
        ;
    if( c == '+' || c == '-' )
        c = *s++;
    return isdigit((int)c);
}

int BitrateIndex(ver,layr, bRate)
int     layr,ver;
int     bRate;
{
int     index = 0;
int     found = 0;

    while(!found && index<15)   {
        if(bitrate[ver][layr-1][index] == bRate)
            found = 1;
        else
            ++index;
    }
    if(found)
        return(index);
    else {
        fprintf(stderr, "BitrateIndex: %d (layer %d) is not a legal bitrate\n",
                bRate, layr);
        return(-1);
    }
}

int SmpFrqIndex(sRate, version)
long sRate;
int  *version;
{
    if (sRate == 44100L) {
        *version = MPEG_AUDIO_ID; return(0);
    }
    else if (sRate == 48000L) {
        *version = MPEG_AUDIO_ID; return(1);
    }
    else if (sRate == 32000L) {
        *version = MPEG_AUDIO_ID; return(2);
    }
    else if (sRate == 24000L) {
        *version = MPEG_PHASE2_LSF; return(1);
    }
    else if (sRate == 22050L) {
        *version = MPEG_PHASE2_LSF; return(0);
    }
    else if (sRate == 16000L) {
        *version = MPEG_PHASE2_LSF; return(2);
    }
    else {
        fprintf(stderr, "SmpFrqIndex: %ld is not a legal sample rate\n", sRate);
        return(-1);
    }
}

void  FAR *mem_alloc(block, item)
unsigned long   block;
char            *item;
{

    void    *ptr;

#ifdef  MACINTOSH
    ptr = NewPtr(block);
#endif

#ifdef MSC60
    ptr = (void FAR *) malloc((unsigned int)block);
#endif

#if ! defined (MACINTOSH) && ! defined (MSC60)
    ptr = (void FAR *) malloc(block);
#endif

    if (ptr != NULL){
#ifdef  MSC60
        _fmemset(ptr, 0, (unsigned int)block);
#else
        memset(ptr, 0, block);
#endif
    }
    else{
        printf("Unable to allocate %s\n", item);
        exit(0);
    }
    return(ptr);
}

void    mem_free(ptr_addr)
void    **ptr_addr;
{

    if (*ptr_addr != NULL){
#ifdef  MACINTOSH
        DisposPtr(*ptr_addr);
#else
        free(*ptr_addr);
#endif
        *ptr_addr = NULL;
    }

}

int memcheck(array, test, num)
char *array;
int test;
int num;
{
 int i=0;

   while (array[i] == test && i<num) i++;
   if (i==num) return TRUE;
   else return FALSE;
}

enum byte_order NativeByteOrder = order_unknown;

enum byte_order DetermineByteOrder()
{
    char s[ sizeof(long) + 1 ];
    union
    {
        long longval;
        char charval[ sizeof(long) ];
    } probe;
    probe.longval = 0x41424344L;
    strncpy( s, probe.charval, sizeof(long) );
    s[ sizeof(long) ] = '\0';
    if ( strcmp(s, "ABCD") == 0 )
        return order_bigEndian;
    else
        if ( strcmp(s, "DCBA") == 0 )
            return order_littleEndian;
        else
            return order_unknown;
}

void SwapBytesInWords( short *loc, int words )
{
    int i;
    short thisval;
    char *dst, *src;
    src = (char *) &thisval;
    for ( i = 0; i < words; i++ )
    {
        thisval = *loc;
        dst = (char *) loc++;
        dst[0] = src[1];
        dst[1] = src[0];
    }
}

int aiff_read_headers( FILE *file_ptr, IFF_AIFF *aiff_ptr )
{
    int i, chunkSize, subSize, sound_position;
    
    if ( fseek(file_ptr, 0, SEEK_SET) != 0 )
        return -1;
    
    if ( Read32BitsHighLow(file_ptr) != IFF_ID_FORM )
        return -1;
    
    chunkSize = Read32BitsHighLow( file_ptr );
    
    if ( Read32BitsHighLow(file_ptr) != IFF_ID_AIFF )
        return -1;
    
    sound_position = 0;
    while ( chunkSize > 0 )
    {
        chunkSize -= 4;
        switch ( Read32BitsHighLow(file_ptr) )
	{
            
          case IFF_ID_COMM:
            chunkSize -= subSize = Read32BitsHighLow( file_ptr );
            aiff_ptr->numChannels = Read16BitsHighLow( file_ptr );
            subSize -= 2;
            aiff_ptr->numSampleFrames = Read32BitsHighLow( file_ptr );
            subSize -= 4;
            aiff_ptr->sampleSize = Read16BitsHighLow( file_ptr );
            subSize -= 2;
            aiff_ptr->sampleRate  = ReadIeeeExtendedHighLow( file_ptr );
            subSize -= 10;
            while ( subSize > 0 )
	    {
                getc( file_ptr );
                subSize -= 1;
	    }
            break;
            
          case IFF_ID_SSND:
            chunkSize -= subSize = Read32BitsHighLow( file_ptr );
            aiff_ptr->blkAlgn.offset = Read32BitsHighLow( file_ptr );
            subSize -= 4;
            aiff_ptr->blkAlgn.blockSize = Read32BitsHighLow( file_ptr );
            subSize -= 4;
            sound_position = ftell( file_ptr ) + aiff_ptr->blkAlgn.offset;
            if ( fseek(file_ptr, (long) subSize, SEEK_CUR) != 0 )
                return -1;
            aiff_ptr->sampleType = IFF_ID_SSND;
            break;
            
          default:
            chunkSize -= subSize = Read32BitsHighLow( file_ptr );
            while ( subSize > 0 )
	    {
                getc( file_ptr );
                subSize -= 1;
	    }
            break;
	}
    }
    return sound_position;
}

int aiff_seek_to_sound_data( FILE *file_ptr )
{
	if ( fseek(file_ptr, AIFF_FORM_HEADER_SIZE + AIFF_SSND_HEADER_SIZE, SEEK_SET) != 0 )
        return(-1);
    return(0);
}

int aiff_write_headers( FILE *file_ptr, IFF_AIFF *aiff_ptr )
{
    int chunkSize;
    int sampleBytes = (aiff_ptr->sampleSize / 8) + (aiff_ptr->sampleSize % 8 ? 1 : 0);
    
    if ( fseek(file_ptr, 0L, SEEK_SET) != 0 )
        return -1;
    
    chunkSize = 8 + 18 + 8 + aiff_ptr->numChannels * aiff_ptr->numSampleFrames * sampleBytes;
    Write32BitsHighLow( file_ptr, IFF_ID_FORM );
    Write32BitsHighLow( file_ptr, chunkSize );
    Write32BitsHighLow( file_ptr, IFF_ID_AIFF );
    Write32BitsHighLow( file_ptr, IFF_ID_COMM );
    Write32BitsHighLow( file_ptr, 18 ); 
    Write16BitsHighLow( file_ptr, aiff_ptr->numChannels );
    Write32BitsHighLow( file_ptr, aiff_ptr->numSampleFrames );
    Write16BitsHighLow( file_ptr, aiff_ptr->sampleSize );
    WriteIeeeExtendedHighLow( file_ptr, aiff_ptr->sampleRate );
    chunkSize = 8 + aiff_ptr->numChannels * aiff_ptr->numSampleFrames * sampleBytes;
    Write32BitsHighLow( file_ptr, IFF_ID_SSND );
    Write32BitsHighLow( file_ptr, chunkSize );
    Write32BitsHighLow( file_ptr, 0 );
    Write32BitsHighLow( file_ptr, 0 );
    return 0;
}

int refill_buffer(bs)
Bit_stream_struc *bs;
{
   register int i=bs->buf_size-2-bs->buf_byte_idx;
   register unsigned long n=1;
   register int index=0;
   char val[2];

   while ((i>=0) && (!bs->eob)) {

      if (bs->format == BINARY)
         n = fread(&bs->buf[i--], sizeof(unsigned char), 1, bs->pt);

      else {
         while((index < 2) && n) {
            n = fread(&val[index], sizeof(char), 1, bs->pt);
            switch (val[index]) {
                  case 0x30:
                  case 0x31:
                  case 0x32:
                  case 0x33:
                  case 0x34:
                  case 0x35:
                  case 0x36:
                  case 0x37:
                  case 0x38:
                  case 0x39:
                  case 0x41:
                  case 0x42:
                  case 0x43:
                  case 0x44:
                  case 0x45:
                  case 0x46:
                  index++;
                  break;
                  default: break;
            }
         }

         if (val[0] <= 0x39)   bs->buf[i] = (val[0] - 0x30) << 4;
                 else  bs->buf[i] = (val[0] - 0x37) << 4;
         if (val[1] <= 0x39)   bs->buf[i--] |= (val[1] - 0x30);
                 else  bs->buf[i--] |= (val[1] - 0x37);
         index = 0;
      }

      if (!n) {
         bs->eob= i+1;
      }

    }
}

static char *he = "0123456789ABCDEF";

void empty_buffer(bs, minimum)
Bit_stream_struc *bs;
int minimum;
{
   register int i;

#if BS_FORMAT == BINARY
   for (i=bs->buf_size-1;i>=minimum;i--)
      fwrite(&bs->buf[i], sizeof(unsigned char), 1, bs->pt);
#else
   for (i=bs->buf_size-1;i>=minimum;i--) {
       char val[2];
       val[0] = he[((bs->buf[i] >> 4) & 0x0F)];
       val[1] = he[(bs->buf[i] & 0x0F)];
       fwrite(val, sizeof(char), 2, bs->pt);
   }
#endif

   for (i=minimum-1; i>=0; i--)
       bs->buf[bs->buf_size - minimum + i] = bs->buf[i];

   bs->buf_byte_idx = bs->buf_size -1 - minimum;
   bs->buf_bit_idx = 8;
}

void open_bit_stream_w(bs, bs_filenam, size)
Bit_stream_struc *bs;
char *bs_filenam;
int size;
{
   if ((bs->pt = fopen(bs_filenam, "wb")) == NULL) {
      printf("Aborted!\n");
      printf("Could not create \"%s\".\n", bs_filenam);
      exit(1);
   }
   alloc_buffer(bs, size);
   bs->buf_byte_idx = size-1;
   bs->buf_bit_idx=8;
   bs->totbit=0;
   bs->mode = WRITE_MODE;
   bs->eob = FALSE;
   bs->eobs = FALSE;
}

void open_bit_stream_r(bs, bs_filenam, size)
Bit_stream_struc *bs;   
char *bs_filenam;       
int size;               
{
   register unsigned long n;
   register unsigned char flag = 1;
   unsigned char val;

   if ((bs->pt = fopen(bs_filenam, "rb")) == NULL) {
      printf("Could not find \"%s\".\n", bs_filenam);
      exit(1);
   }

   do {
     n = fread(&val, sizeof(unsigned char), 1, bs->pt);
     switch (val) {
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34:
      case 0x35:
      case 0x36:
      case 0x37:
      case 0x38:
      case 0x39:
      case 0x41:
      case 0x42:
      case 0x43:
      case 0x44:
      case 0x45:
      case 0x46:
      case 0xa:
      case 0xd:
      case 0x1a:
          break;

      default:
          flag--;
          break;
     }

   } while (flag & n);

   if (flag) {
      bs->format = ASCII;
   }
   else {
      bs->format = BINARY;
   }

   fclose(bs->pt);

   if ((bs->pt = fopen(bs_filenam, "rb")) == NULL) {
      printf("Aborted!\n");
      printf("Could not find \"%s\".\n", bs_filenam);
      exit(1);
   }

   alloc_buffer(bs, size);
   bs->buf_byte_idx=0;
   bs->buf_bit_idx=0;
   bs->totbit=0;
   bs->mode = READ_MODE;
   bs->eob = FALSE;
   bs->eobs = FALSE;
}

void close_bit_stream_r(bs)
Bit_stream_struc *bs;
{
   fclose(bs->pt);
   desalloc_buffer(bs);
}

void close_bit_stream_w(bs)
Bit_stream_struc *bs;
{
   empty_buffer(bs, bs->buf_byte_idx);
   fclose(bs->pt);
   desalloc_buffer(bs);
}

void alloc_buffer(bs, size)
Bit_stream_struc *bs;
int size;
{
   bs->buf = (unsigned char FAR *) mem_alloc(size*sizeof(unsigned
              char), "buffer");
   bs->buf_size = size;
}

void desalloc_buffer(bs)
Bit_stream_struc *bs; 
{
   free(bs->buf);
}

int putmask[9]={0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
int clearmask[9]={0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x0};

void back_track_buffer(bs, N)
Bit_stream_struc *bs; 
int N;
{
   int tmp = N - (N/8)*8;
   register int i;

   bs->totbit -= N;
   for (i=bs->buf_byte_idx;i< bs->buf_byte_idx+N/8-1;i++) bs->buf[i] = 0;
   bs->buf_byte_idx += N/8;
   if ( (tmp + bs->buf_bit_idx) <= 8) {
      bs->buf_bit_idx += tmp;
   }
   else {
      bs->buf_byte_idx ++;
      bs->buf_bit_idx += (tmp - 8);
   }
   bs->buf[bs->buf_byte_idx] &= clearmask[bs->buf_bit_idx];
}


int mask[8]={0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

unsigned int get1bit(bs)
Bit_stream_struc *bs;
{
   unsigned int bit;
   register int i;

   bs->totbit++;

   if (!bs->buf_bit_idx) {
        bs->buf_bit_idx = 8;
        bs->buf_byte_idx--;
        if ((bs->buf_byte_idx < MINIMUM) || (bs->buf_byte_idx < bs->eob)) {
             if (bs->eob)
                bs->eobs = TRUE;
             else {
                for (i=bs->buf_byte_idx; i>=0;i--)
                  bs->buf[bs->buf_size-1-bs->buf_byte_idx+i] = bs->buf[i];
                refill_buffer(bs);
                bs->buf_byte_idx = bs->buf_size-1;
             }
        }
   }
   bit = bs->buf[bs->buf_byte_idx]&mask[bs->buf_bit_idx-1];
   bit = bit >> (bs->buf_bit_idx-1);
   bs->buf_bit_idx--;
   return(bit);
}

void put1bit(bs, bit)
Bit_stream_struc *bs;
int bit;
{
   bs->totbit++;

   bs->buf[bs->buf_byte_idx] |= (bit&0x1) << (bs->buf_bit_idx-1);
   bs->buf_bit_idx--;
   if (!bs->buf_bit_idx) {
       bs->buf_bit_idx = 8;
       bs->buf_byte_idx--;
       if (bs->buf_byte_idx < 0)
          empty_buffer(bs, MINIMUM);
       bs->buf[bs->buf_byte_idx] = 0;
   }
}

unsigned long look_ahead(bs, N)
Bit_stream_struc *bs;
int N;
{
 unsigned long val=0;
 register int j = N;
 register int k, tmp;
 register int bit_idx = bs->buf_bit_idx;
 register int byte_idx = bs->buf_byte_idx;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 while (j > 0) {
    if (!bit_idx) {
        bit_idx = 8;
        byte_idx--;
    }
    k = MIN (j, bit_idx);
    tmp = bs->buf[byte_idx]&putmask[bit_idx];
    tmp = tmp >> (bit_idx-k);
    val |= tmp << (j-k);
    bit_idx -= k;
    j -= k;
 }
 return(val);
}

unsigned long getbits(bs, N)
Bit_stream_struc *bs;
int N;
{
 unsigned long val=0;
 register int i;
 register int j = N;
 register int k, tmp;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 bs->totbit += N;
 while (j > 0) {
   if (!bs->buf_bit_idx) {
        bs->buf_bit_idx = 8;
        bs->buf_byte_idx--;
        if ((bs->buf_byte_idx < MINIMUM) || (bs->buf_byte_idx < bs->eob)) {
             if (bs->eob)
                bs->eobs = TRUE;
             else {
                for (i=bs->buf_byte_idx; i>=0;i--)
                   bs->buf[bs->buf_size-1-bs->buf_byte_idx+i] = bs->buf[i];
                refill_buffer(bs);
                bs->buf_byte_idx = bs->buf_size-1;
             }
        }
   }
   k = MIN (j, bs->buf_bit_idx);
   tmp = bs->buf[bs->buf_byte_idx]&putmask[bs->buf_bit_idx];
   tmp = tmp >> (bs->buf_bit_idx-k);
   val |= tmp << (j-k);
   bs->buf_bit_idx -= k;
   j -= k;
 }
 return(val);
}

void putbits(bs, val, N)
Bit_stream_struc *bs;
unsigned int val;
int N;
{
 register int j = N;
 register int k, tmp;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 bs->totbit += N;
 while (j > 0) {
   k = MIN(j, bs->buf_bit_idx);
   tmp = val >> (j-k);
   bs->buf[bs->buf_byte_idx] |= (tmp&putmask[k]) << (bs->buf_bit_idx-k);
   bs->buf_bit_idx -= k;
   if (!bs->buf_bit_idx) {
       bs->buf_bit_idx = 8;
       bs->buf_byte_idx--;
       if (bs->buf_byte_idx < 0)
          empty_buffer(bs, MINIMUM);
       bs->buf[bs->buf_byte_idx] = 0;
   }
   j -= k;
 }
}

void byte_ali_putbits(bs, val, N)
Bit_stream_struc *bs;
unsigned int val;
int N;
{
 unsigned long aligning, sstell();

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);
 aligning = sstell(bs)%8;
 if (aligning)
     putbits(bs, (unsigned int)0, (int)(8-aligning)); 

 putbits(bs, val, N);
}

unsigned long byte_ali_getbits(bs, N)
Bit_stream_struc *bs;
int N;
{
 unsigned long aligning, sstell();

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);
 aligning = sstell(bs)%8;
 if (aligning)
    getbits(bs, (int)(8-aligning));

 return(getbits(bs, N));
}

unsigned long sstell(bs)
Bit_stream_struc *bs;
{
  return(bs->totbit);
}

int end_bs(bs)
Bit_stream_struc *bs;
{
  return(bs->eobs);
}

int seek_sync(bs, sync, N)
Bit_stream_struc *bs;
long sync;
int N;
{
 double pow();
 unsigned long aligning, stell();
 unsigned long val;
 unsigned long tempVal;
 long maxi = (int)pow(2.0, (FLOAT)N) - 1;

 aligning = sstell(bs)%ALIGNING;
 if (aligning) {
    tempVal = getbits(bs, (int)(ALIGNING-aligning));
    if (hiding) {
      putbits(&ptrNewMP3, tempVal, (int)(ALIGNING-aligning));
    }
  }

  val = getbits(bs, N);

  if (hiding) {
    putbits(&ptrNewMP3,val, N);
  }

  while (((val&maxi) != sync) && (!end_bs(bs))) {
        val <<= ALIGNING;

	tempVal = getbits(bs, ALIGNING);
	val |= tempVal;
        if (hiding) {
          putbits(&ptrNewMP3,tempVal, ALIGNING);
        }
  }

 if (end_bs(bs)) return(0);
 else return(1);
}

void I_CRC_calc(fr_ps, bit_alloc, crc)
frame_params *fr_ps;
unsigned int bit_alloc[2][SBLIMIT];
unsigned int *crc;
{
        int i, k;
        layer *info = fr_ps->header;
        int stereo  = fr_ps->stereo;
        int jsbound = fr_ps->jsbound;

        *crc = 0xffff;
        update_CRC(info->bitrate_index, 4, crc);
        update_CRC(info->sampling_frequency, 2, crc);
        update_CRC(info->padding, 1, crc);
        update_CRC(info->extension, 1, crc);
        update_CRC(info->mode, 2, crc);
        update_CRC(info->mode_ext, 2, crc);
        update_CRC(info->copyright, 1, crc);
        update_CRC(info->original, 1, crc);
        update_CRC(info->emphasis, 2, crc);

        for (i=0;i<SBLIMIT;i++)
                for (k=0;k<((i<jsbound)?stereo:1);k++)
                        update_CRC(bit_alloc[k][i], 4, crc);
}

void II_CRC_calc(fr_ps, bit_alloc, scfsi, crc)
frame_params *fr_ps;
unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT];
unsigned int *crc;
{
        int i, k;
        layer *info = fr_ps->header;
        int stereo  = fr_ps->stereo;
        int sblimit = fr_ps->sblimit;
        int jsbound = fr_ps->jsbound;
        al_table *alloc = fr_ps->alloc;

        *crc = 0xffff;
        update_CRC(info->bitrate_index, 4, crc);
        update_CRC(info->sampling_frequency, 2, crc);
        update_CRC(info->padding, 1, crc);
        update_CRC(info->extension, 1, crc);
        update_CRC(info->mode, 2, crc);
        update_CRC(info->mode_ext, 2, crc);
        update_CRC(info->copyright, 1, crc);
        update_CRC(info->original, 1, crc);
        update_CRC(info->emphasis, 2, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<((i<jsbound)?stereo:1);k++)
                        update_CRC(bit_alloc[k][i], (*alloc)[i][0].bits, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<stereo;k++)
                        if (bit_alloc[k][i])
                                update_CRC(scfsi[k][i], 2, crc);
}

void update_CRC(data, length, crc)
unsigned int data, length, *crc;
{
        unsigned int  masking, carry;

        masking = 1 << length;

        while((masking >>= 1)){
                carry = *crc & 0x8000;
                *crc <<= 1;
                if (!carry ^ !(data & masking))
                        *crc ^= CRC16_POLYNOMIAL;
        }
        *crc &= 0xffff;
}

#ifdef  MACINTOSH

void    set_mac_file_attr(fileName, vRefNum, creator, fileType)
char    fileName[MAX_NAME_SIZE];
short   vRefNum;
OsType  creator;
OsType  fileType;
{

short   theFile;
char    pascal_fileName[MAX_NAME_SIZE];
FInfo   fndrInfo;

        CtoPstr(strcpy(pascal_fileName, fileName));

        FSOpen(pascal_fileName, vRefNum, &theFile);
        GetFInfo(pascal_fileName, vRefNum, &fndrInfo);
        fndrInfo.fdCreator = creator;
        fndrInfo.fdType = fileType;
        SetFInfo(pascal_fileName, vRefNum, &fndrInfo);
        FSClose(theFile);

}
#endif


#ifdef  MS_DOS
char *new_ext(char *filename, char *extname)
{
  int found, dotpos;
static
  char newname[80];

  dotpos=strlen(filename); found=0;
  do
  {
    switch (filename[dotpos])
    {
      case '.' : found=1; break;
      case '\\':                  
      case '/' :                  
      case ':' : found=-1; break; 
      default  : dotpos--; if (dotpos<0) found=-1; break;
    }
  } while (found==0);
  if (found==-1) strcpy(newname,filename);
  if (found== 1) strncpy(newname,filename,dotpos); newname[dotpos]='\0';
  strcat(newname,extname);
  return(newname);
}
#endif

#define BUFSIZE 4096
static unsigned long offset,totbit=0, buf_byte_idx=0;
static unsigned int buf[BUFSIZE];
static unsigned int buf_bit_idx=8;

unsigned long hsstell()
{
  return(totbit);
}

extern int putmask[9];

unsigned long hgetbits(N)
int N;                
{
 unsigned long val=0;
 register int j = N;
 register int k, tmp;

 totbit += N;
 while (j > 0) {
   if (!buf_bit_idx) {
        buf_bit_idx = 8;
        buf_byte_idx++;
	if (buf_byte_idx > offset)
	  { printf("Buffer overflow \n");exit(3); }
   }
   k = MIN (j, buf_bit_idx);
   tmp = buf[buf_byte_idx%BUFSIZE]&putmask[buf_bit_idx];
   tmp = tmp >> (buf_bit_idx-k);
   val |= tmp << (j-k);
   buf_bit_idx -= k;
   j -= k;
 }
 return(val);
}

unsigned int hget1bit()
{
return(hgetbits(1));
}

void hputbuf(val, N)
unsigned int val;   
int N;                
{
  if (N != 8) { printf("Not supported\n"); exit(-3); }
  buf[offset % BUFSIZE] = val;
  offset++;
}

void rewindNbits( N )
int N;
{
   totbit -= N;
   buf_bit_idx += N;
   while( buf_bit_idx >= 8 )
   {  buf_bit_idx -= 8;
      buf_byte_idx--;
   }
}

void rewindNbytes( N )
int N;
{
   totbit -= N*8;
   buf_byte_idx -= N;
}
