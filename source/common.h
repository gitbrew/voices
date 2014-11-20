#ifndef COMMON_H
#define COMMON_H

#if defined(MSC60) 
#ifndef MS_DOS
#define MS_DOS
#endif
#ifndef PROTO_ARGS
#define PROTO_ARGS
#endif
#endif

#ifdef  UNIX
#define         TABLES_PATH     "tables"
#define         MPEGTABENV      "MPEGTABLES"
#define         PATH_SEPARATOR  "/"
#endif

#ifdef  MACINTOSH
#define      TABLES_PATH ":tables:"
#endif

#define FAR

#ifdef __STDC__
#ifndef PROTO_ARGS
#define PROTO_ARGS 
#endif
#endif

#ifdef CONVEX
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

#include        <stdio.h>
#include        <string.h>
#include        <math.h>
#include "ieeefloat.h"
#include "portableio.h"

#ifdef  UNIX
#include        <unistd.h>
#endif

#ifdef __sgi
#include	<stdlib.h>
#endif

#ifdef  MACINTOSH
#include        <stdlib.h>
#include        <console.h>
#endif

#ifdef  MS_DOS
#include        <stdlib.h>
#ifdef MSC60
#include        <memory.h>
#else
#include        <alloc.h>
#include        <mem.h>
#endif
#endif

#ifdef  MS_DOS
#define         FLOAT                   double
#else
#define         FLOAT                   float
#endif

#define         FALSE                   0
#define         TRUE                    1
#define         NULL_CHAR               '\0'

#define         MAX_U_32_NUM            0xFFFFFFFF
#ifndef PI
#define         PI                      3.14159265358979
#endif

#define         PI4                     PI/4
#define         PI64                    PI/64
#define         LN_TO_LOG10             0.2302585093

#define         VOL_REF_NUM             0
#define         MAC_WINDOW_SIZE         24

#define         MONO                    1
#define         STEREO                  2
#define         BITS_IN_A_BYTE          8
#define         WORD                    16
#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         SSLIMIT                 18
#define         FFT_SIZE                1024
#define         HAN_SIZE                512
#define         SCALE_BLOCK             12
#define         SCALE_RANGE             64
#define         SCALE                   32768
#define         CRC16_POLYNOMIAL        0x8005

#define         MPEG_AUDIO_ID           1
#define		    MPEG_PHASE2_LSF 		0

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define         MPG_MD_LR_LR            0
#define         MPG_MD_LR_I             1
#define         MPG_MD_MS_LR            2
#define         MPG_MD_MS_I             3

#define IFF_ID_FORM 0x464f524d
#define IFF_ID_AIFF 0x41494646
#define IFF_ID_COMM 0x434f4d4d
#define IFF_ID_SSND 0x53534e44
#define IFF_ID_MPEG 0x4d504547

#define AIFF_FORM_HEADER_SIZE 12
#define AIFF_SSND_HEADER_SIZE 16


typedef struct  blockAlign_struct {
    unsigned long   offset;
    unsigned long   blockSize;
} blockAlign;

typedef struct  IFF_AIFF_struct {
    short           numChannels;
    unsigned long   numSampleFrames;
    short           sampleSize;
    double          sampleRate;
    unsigned long   sampleType;
    blockAlign      blkAlgn;
} IFF_AIFF;

enum byte_order { order_unknown, order_bigEndian, order_littleEndian };
extern enum byte_order NativeByteOrder;

#define         MINIMUM         4 
#define         MAX_LENGTH      32
#define         READ_MODE       0
#define         WRITE_MODE      1
#define         ALIGNING        8
#define         BINARY          0
#define         ASCII           1
#ifndef BS_FORMAT
#define         BS_FORMAT       ASCII
#endif
#define         BUFFER_SIZE     4096

#define         MIN(A, B)       ((A) < (B) ? (A) : (B))
#define         MAX(A, B)       ((A) > (B) ? (A) : (B))

typedef struct {
            char    origMP3FileName[MAX_NAME_SIZE];
            char    newMP3FileName[MAX_NAME_SIZE];
            char    hiddenFileName[MAX_NAME_SIZE];
            int     bitSpacing;
            int     hiding;
              }Arguments_t;

typedef struct {
    unsigned int    steps;
    unsigned int    bits;
    unsigned int    group;
    unsigned int    quant;
} sb_alloc, *alloc_ptr;

typedef sb_alloc        al_table[SBLIMIT][16];

typedef struct {
    int version;
    int lay;
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
} layer, *the_layer;

typedef struct {
    layer       *header;     
    int         actual_mode; 
    al_table    *alloc;      
    int         tab_num;     
    int         stereo;      
    int         jsbound;     
    int         sblimit;     
} frame_params;

typedef struct  IEEE_DBL_struct {
    unsigned long   hi;
    unsigned long   lo;
} IEEE_DBL;

typedef struct  SANE_EXT_struct {
    unsigned long   l1;
    unsigned long   l2;
    unsigned short  s1;
} SANE_EXT;

typedef struct  bit_stream_struc {
    FILE        *pt;           
    unsigned char *buf;        
    int         buf_size;     
    long        totbit;        
    int         buf_byte_idx;   
    int         buf_bit_idx;    
    int         mode;           
    int         eob;            
    int         eobs;           
    char        format;
    
} Bit_stream_struc;

typedef struct {
	unsigned part2_3_length;
	unsigned big_values;
	unsigned global_gain;
	unsigned scalefac_compress;
	unsigned window_switching_flag;
	unsigned block_type;
	unsigned mixed_block_flag;
	unsigned table_select[3];
	unsigned subblock_gain[3];
	unsigned region0_count;
	unsigned region1_count;
	unsigned preflag;
	unsigned scalefac_scale;
	unsigned count1table_select;
} my_gr_info ;

typedef struct {
	unsigned main_data_begin;
	unsigned private_bits;
	struct {
	    unsigned scfsi[4];
		struct gr_info_s {
			unsigned part2_3_length;
			unsigned big_values;
			unsigned global_gain;
			unsigned scalefac_compress;
			unsigned window_switching_flag;
			unsigned block_type;
			unsigned mixed_block_flag;
			unsigned table_select[3];
			unsigned subblock_gain[3];
			unsigned region0_count;
			unsigned region1_count;
			unsigned preflag;
			unsigned scalefac_scale;
			unsigned count1table_select;
			} gr[2];
		} ch[2];
	} III_side_info_t;

typedef struct {
	int l[23];       
	int s[3][13];        
	} III_scalefac_t[2];

extern char     *mode_names[5];
extern char     *layer_names[3];
extern char	*version_names[2];
extern double   s_freq[2][4];
extern int      bitrate[2][3][15];
extern double FAR multiple[64];

#ifdef  PROTO_ARGS
extern FILE           *OpenTableFile(char*);
extern int            read_bit_alloc(int, al_table*);
extern int            pick_table(frame_params*);
extern int            js_bound(int, int);
extern void           hdr_to_frps(frame_params*);
extern void           WriteHdr(frame_params*, FILE*);
extern void           WriteBitAlloc(unsigned int[2][SBLIMIT], frame_params*,
                        FILE*);
extern void           WriteScale(unsigned int[2][SBLIMIT],
                        unsigned int[2][SBLIMIT], unsigned int[2][3][SBLIMIT],
                        frame_params*, FILE*);
extern void           WriteSamples(int, unsigned int FAR [SBLIMIT],
                        unsigned int[SBLIMIT], frame_params*, FILE*);
extern int            NumericQ(char*);
extern int            BitrateIndex(int, int, int);
extern int            SmpFrqIndex(long, int*);
extern int            memcheck(char*, int, int);
extern void           FAR *mem_alloc(unsigned long, char*);
extern void           mem_free(void**);
extern void           double_to_extended(double*, char[10]);
extern void           extended_to_double(char[10], double*);
extern int            aiff_read_headers(FILE*, IFF_AIFF*);
extern int            aiff_seek_to_sound_data(FILE*);
extern int            aiff_write_headers(FILE*, IFF_AIFF*);
extern int            refill_buffer(Bit_stream_struc*);
extern void           empty_buffer(Bit_stream_struc*, int);
extern void           open_bit_stream_w(Bit_stream_struc*, char*, int);
extern void           open_bit_stream_r(Bit_stream_struc*, char*, int);
extern void           close_bit_stream_r(Bit_stream_struc*);
extern void           close_bit_stream_w(Bit_stream_struc*);
extern void           alloc_buffer(Bit_stream_struc*, int);
extern void           desalloc_buffer(Bit_stream_struc*);
extern void           back_track_buffer(Bit_stream_struc*, int);
extern unsigned int   get1bit(Bit_stream_struc*);
extern void           put1bit(Bit_stream_struc*, int);
extern unsigned long  look_ahead(Bit_stream_struc*, int);
extern unsigned long  getbits(Bit_stream_struc*, int);
extern void           putbits(Bit_stream_struc*, unsigned int, int);
extern void           byte_ali_putbits(Bit_stream_struc*, unsigned int, int);
extern unsigned long  byte_ali_getbits(Bit_stream_struc*, int);
extern unsigned long  sstell(Bit_stream_struc*);
extern int            end_bs(Bit_stream_struc*);
extern int            seek_sync(Bit_stream_struc*, long, int);
extern void           I_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
                        unsigned int*);
extern void           II_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
                        unsigned int[2][SBLIMIT], unsigned int*);
extern void           update_CRC(unsigned int, unsigned int, unsigned int*);
extern void           read_absthr(FLOAT*, int);
extern unsigned int   hget1bit();
extern unsigned long  hgetbits(int);
extern unsigned long  hsstell();
extern void           hputbuf(unsigned int,int);
extern void           rewindNbytes( int );
enum byte_order DetermineByteOrder();
void SwapBytesInWords( short *loc, int words );

#ifdef  MACINTOSH
extern void           set_mac_file_attr(char[MAX_NAME_SIZE], short, OsType,
                        OsType);
#endif
#ifdef MS_DOS
extern char           *new_ext(char *filename, char *extname); 
#endif

#else
extern FILE           *OpenTableFile();
extern int            read_bit_alloc();
extern int            pick_table();
extern int            js_bound();
extern void           hdr_to_frps();
extern void           WriteHdr();
extern void           WriteBitAlloc();
extern void           WriteScale();
extern void           WriteSamples();
extern int            NumericQ();
extern int            BitrateIndex();
extern int            SmpFrqIndex();
extern int            memcheck();
extern void           FAR *mem_alloc();
extern void           mem_free();
extern void           double_to_extended();
extern void           extended_to_double();
extern int            aiff_read_headers();
extern int            aiff_seek_to_sound_data();
extern int            aiff_write_headers();
extern int            refill_buffer();
extern void           empty_buffer();
extern void           open_bit_stream_w();
extern void           open_bit_stream_r();
extern void           close_bit_stream_r();
extern void           close_bit_stream_w();
extern void           alloc_buffer();
extern void           desalloc_buffer();
extern void           back_track_buffer();
extern unsigned int   get1bit();
extern void           put1bit();
extern unsigned long  look_ahead();
extern unsigned long  getbits();
extern void           putbits();
extern void           byte_ali_putbits();
extern unsigned long  byte_ali_getbits();
extern unsigned long  sstell();
extern int            end_bs();
extern int            seek_sync();
extern void           I_CRC_calc();
extern void           II_CRC_calc();
extern void           update_CRC();
extern void           read_absthr();

extern unsigned int   hget1bit();
extern unsigned long  hgetbits();
extern unsigned long  hsstell();
extern void           hputbuf();
extern void           rewindNbytes();

#ifdef MS_DOS
extern char           *new_ext(); 
#endif
#endif
#endif

