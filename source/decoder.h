#define   DFLT_OPEXT        ".dec"

#if !defined(MS_DOS) && !defined(AIX)
#define   FILTYP_DEC_AIFF   'AIFF'
#define   FILTYP_DEC_BNRY   'TEXT'
#define   CREATR_DEC_AIFF   'Sd2a'
#define   CREATR_DEC_BNRY   0x3f3f3f3f
#else
#define   FILTYP_DEC_AIFF   "AIFF"
#define   FILTYP_DEC_BNRY   "TEXT"
#define   CREATR_DEC_AIFF   "Sd2a"
#define   CREATR_DEC_BNRY   "????"
#endif

#define   SYNC_WORD         (long) 0xfff
#define   SYNC_WORD_LNGTH   12

#define   MUTE              0

#ifdef   PROTO_ARGS
static void   usage(void);
#else
static void   usage();
#endif

#ifdef   PROTO_ARGS
extern void   decode_info(Bit_stream_struc*, frame_params*);
extern void   II_decode_bitalloc(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   I_decode_bitalloc(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   I_decode_scale(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*);
extern void   II_decode_scale(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       unsigned int[2][SBLIMIT], unsigned int[2][3][SBLIMIT],
                       frame_params*);
extern void   I_buffer_sample(Bit_stream_struc*, unsigned int[2][3][SBLIMIT],
                       unsigned int[2][SBLIMIT], frame_params*);
extern void   II_buffer_sample(Bit_stream_struc*, unsigned int[2][3][SBLIMIT],
                       unsigned int[2][SBLIMIT], frame_params*);
extern void   read_quantizer_table(double[17], double[17]);
extern void   II_dequantize_sample(unsigned int[2][3][SBLIMIT], 
                       unsigned int[2][SBLIMIT], double[2][3][SBLIMIT],
                       frame_params*);
extern void   I_dequantize_sample(unsigned int[2][3][SBLIMIT],
                       double[2][3][SBLIMIT], unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   read_scale_factor(double[SCALE_RANGE]);
extern void   II_denormalize_sample(double[2][3][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*, int);
extern void   I_denormalize_sample(double[2][3][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*);
extern void   create_syn_filter(double[64][SBLIMIT]);
extern int    SubBandSynthesis (double*, int, short*);
extern void   read_syn_window(double[HAN_SIZE]);
extern void   window_sample(double*, double*);
extern void   out_fifo(short[2][SSLIMIT][SBLIMIT], int, frame_params*, int,
                       FILE*, unsigned long*);
extern void   buffer_CRC(Bit_stream_struc*, unsigned int*);
extern void   recover_CRC_error(short[2][SSLIMIT][SBLIMIT], int, frame_params*,
                       FILE*, unsigned long*);
extern void   III_dequantize_sample(long int[SBLIMIT][SSLIMIT],
			double [SBLIMIT][SSLIMIT], III_scalefac_t *,
                        struct gr_info_s *, int, frame_params *);
extern void   III_antialias(double[SBLIMIT][SSLIMIT], double[SBLIMIT][SSLIMIT], 
                          struct gr_info_s *, frame_params *);
extern void   inv_mdct(double[18], double[36], int);
extern void   III_hybrid(double[SSLIMIT], double[SSLIMIT] , int, int,
                       struct gr_info_s *, frame_params *);
extern void   III_get_side_info(Bit_stream_struc *,
                                 III_side_info_t *,
                                 frame_params *);

extern void   III_put_side_info(Bit_stream_struc *,
                                 III_side_info_t *,
                                 frame_params *);


extern void III_get_scale_factors(III_scalefac_t *,
                                 III_side_info_t *, 
                                 int, 
                                 int, 
                                 frame_params *);

#else
extern void   decode_info();
extern void   II_decode_bitalloc();
extern void   I_decode_bitalloc();
extern void   I_decode_scale();
extern void   II_decode_scale();
extern void   I_buffer_sample();
extern void   II_buffer_sample();
extern void   read_quantizer_table();
extern void   II_dequantize_sample();
extern void   I_dequantize_sample();
extern void   read_scale_factor();
extern void   II_denormalize_sample();
extern void   I_denormalize_sample();
extern void   create_syn_filter();
extern int    SubBandSynthesis ();
extern void   read_syn_window();
extern void   window_sample();
extern void   out_fifo();
extern void   buffer_CRC();
extern void   recover_CRC_error();
extern void   III_dequantize_sample();
extern void   III_antialias();
extern void   inv_mdct();
extern void   III_hybrid();
#endif
