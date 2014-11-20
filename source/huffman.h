
#define HUFFBITS unsigned long int
#define HTN	34
#define MXOFF	250
 
struct huffcodetab {
  char tablename[3];
  unsigned int xlen;
  unsigned int ylen;
  unsigned int linbits;
  unsigned int linmax;
  int ref;
  HUFFBITS *table;
  unsigned char *hlen;
  unsigned char(*val)[2];
  unsigned int treelen;
};

extern struct huffcodetab ht[HTN];
#ifdef PROTO_ARGS

extern int read_huffcodetab(FILE *); 
extern int read_decoder_table(FILE *);
 
extern void huffman_coder(unsigned int, unsigned int,
			  struct huffcodetab *, Bit_stream_struc *);
			  
extern int huffman_decoder(struct huffcodetab *,
			  int *, int*, int*, int*);

#else

extern int read_huffcodetab(); 
extern int read_decoder_table(); 
extern void huffman_coder();
extern int huffman_decoder();

#endif
