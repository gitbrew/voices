#include "common.h"
#include "huffman.h"
#include "stdlib.h"
     
HUFFBITS dmask = 1 << (sizeof(HUFFBITS)*8-1);
unsigned int hs = sizeof(HUFFBITS)*8;

struct huffcodetab ht[HTN];
int read_huffcodetab(fi) 
FILE *fi;
{

  char line[100],command[40],huffdata[40];
  unsigned int t,i,j,k,nn,x,y,n=0;
  unsigned int xl, yl, len;
  HUFFBITS h;
  int	hsize;
  
  hsize = sizeof(HUFFBITS)*8; 
  do {
      fgets(line,99,fi);
  } while ((line[0] == '#') || (line[0] < ' ') );
  
  do {    
    while ((line[0]=='#') || (line[0] < ' ')) {
      fgets(line,99,fi);
    } 

    sscanf(line,"%s %s %u %u %u",command,ht[n].tablename,
			         &xl,&yl,&ht[n].linbits);
    if (strcmp(command,".end")==0)
      return n;
    else if (strcmp(command,".table")!=0) {
      fprintf(stderr,"huffman table %u data corrupted\n",n);
      return -1;
    }
    ht[n].linmax = (1<<ht[n].linbits)-1;
   
    sscanf(ht[n].tablename,"%u",&nn);
    if (nn != n) {
      fprintf(stderr,"wrong table number %u\n",n);
      return(-2);
    } 

    ht[n].xlen = xl;
    ht[n].ylen = yl;

    do {
      fgets(line,99,fi);
    } while ((line[0] == '#') || (line[0] < ' '));

    sscanf(line,"%s %u",command,&t);
    if (strcmp(command,".reference")==0) {
      ht[n].ref   = t;
      ht[n].table = ht[t].table;
      ht[n].hlen  = ht[t].hlen;
      if ( (xl != ht[t].xlen) ||
           (yl != ht[t].ylen)  ) {
        fprintf(stderr,"wrong table %u reference\n",n);
        return (-3);
      };
      do {
        fgets(line,99,fi);
      } while ((line[0] == '#') || (line[0] < ' ') );
    } 
    else {
      ht[n].ref  = -1;
      ht[n].table=(HUFFBITS *) calloc(xl*yl,sizeof(HUFFBITS));
      if (ht[n].table == NULL) {
         fprintf(stderr,"unsufficient heap error\n");
         return (-4);
      }
      ht[n].hlen=(unsigned char *) calloc(xl*yl,sizeof(unsigned char));
      if (ht[n].hlen == NULL) {
         fprintf(stderr,"unsufficient heap error\n");
         return (-4);
      }
      for (i=0; i<xl; i++) {
        for (j=0;j<yl; j++) {
	  if (xl>1) 
            sscanf(line,"%u %u %u %s",&x, &y, &len,huffdata);
	  else 
            sscanf(line,"%u %u %s",&x,&len,huffdata);
          h=0;k=0;
	  while (huffdata[k]) {
            h <<= 1;
            if (huffdata[k] == '1')
              h++;
            else if (huffdata[k] != '0'){
              fprintf(stderr,"huffman-table %u bit error\n",n);
              return (-5);
            };
            k++;
          };
          if (k != len) {
           fprintf(stderr,
              "warning: wrong codelen in table %u, pos [%2u][%2u]\n",
	       n,i,j);
          };
          ht[n].table[i*xl+j] = h;
          ht[n].hlen[i*xl+j] = (unsigned char) len;
	  do {
            fgets(line,99,fi);
          } while ((line[0] == '#') || (line[0] < ' '));
        }
      }
    }
    n++;
  } while (1);
}

int read_decoder_table(fi) 
FILE *fi;
{
  int n,i,nn,t;
  unsigned int v0,v1;
  char command[100],line[100];
  for (n=0;n<HTN;n++) {
    do {
      fgets(line,99,fi);
    } while ((line[0] == '#') || (line[0] < ' '));
     
    sscanf(line,"%s %s %u %u %u %u",command,ht[n].tablename,
           &ht[n].treelen, &ht[n].xlen, &ht[n].ylen, &ht[n].linbits);
    if (strcmp(command,".end")==0)
      return n;
    else if (strcmp(command,".table")!=0) {
      fprintf(stderr,"huffman table %u data corrupted\n",n);
      return -1;
    }
    ht[n].linmax = (1<<ht[n].linbits)-1;
   
    sscanf(ht[n].tablename,"%u",&nn);
    if (nn != n) {
      fprintf(stderr,"wrong table number %u\n",n);
      return(-2);
    } 
    do {
      fgets(line,99,fi);
    } while ((line[0] == '#') || (line[0] < ' '));

    sscanf(line,"%s %u",command,&t);
    if (strcmp(command,".reference")==0) {
      ht[n].ref   = t;
      ht[n].val   = ht[t].val;
      ht[n].treelen  = ht[t].treelen;
      if ( (ht[n].xlen != ht[t].xlen) ||
           (ht[n].ylen != ht[t].ylen)  ) {
        fprintf(stderr,"wrong table %u reference\n",n);
        return (-3);
      };
      while ((line[0] == '#') || (line[0] < ' ') ) {
        fgets(line,99,fi);
      }
    }    
    else if (strcmp(command,".treedata")==0) {
      ht[n].ref  = -1;
      ht[n].val = (unsigned char (*)[2]) 
        calloc(2*(ht[n].treelen),sizeof(unsigned char));
      if (ht[n].val == NULL) {
    	fprintf(stderr, "heaperror at table %d\n",n);
    	exit (-10);
      }
      for (i=0;i<ht[n].treelen; i++) {
        fscanf(fi,"%x %x",&v0, &v1);
        ht[n].val[i][0]=(unsigned char)v0;
        ht[n].val[i][1]=(unsigned char)v1;
      }
      fgets(line,99,fi);
    }
    else {
      fprintf(stderr,"huffman decodertable error at table %d\n",n);
    }
  }
  return n;
}

void huffman_coder( x, y, h, bs)
unsigned int x;
unsigned int y;
struct huffcodetab *h;
Bit_stream_struc *bs;
{
  HUFFBITS huffbits;
  HUFFBITS linbitsX; 
  HUFFBITS linbitsY;
  unsigned int len;
  unsigned int xl1 = h->xlen-1;
  unsigned int yl1 = h->ylen-1;
  linbitsX = 0;
  linbitsY = 0;
  if (h->table == NULL) return;
  if (((x < xl1) || (xl1==0)) && (y < yl1)) {
    huffbits = h->table[x*(h->xlen)+y];
    len = h->hlen[x*(h->xlen)+y];
    putbits(bs,huffbits,len);
    return;
  }  
  else if (x >= xl1) {
    linbitsX = x-xl1;
    if (linbitsX > h->linmax) {
      fprintf(stderr,"warning: Huffman X table overflow\n");
      linbitsX= h->linmax;
    };
    if (y >= yl1) {
      huffbits = h->table[(h->ylen)*(h->xlen)-1];
      len = h->hlen[(h->ylen)*(h->xlen)-1];
      putbits(bs,huffbits,len);
      linbitsY = y-yl1;
      if (linbitsY > h->linmax) {
        fprintf(stderr,"warning: Huffman Y table overflow\n");
        linbitsY = h->linmax;
      };
      if (h->linbits) {
        putbits(bs,linbitsX,h->linbits);
        putbits(bs,linbitsY,h->linbits);
      }
    } 
    else {
      huffbits = h->table[(h->ylen)*xl1+y];
      len = h->hlen[(h->ylen)*xl1+y];
      putbits(bs,huffbits,len);
      if (h->linbits) {
        putbits(bs,linbitsX,h->linbits);
      }
    }
  }
  else  {
    huffbits = h->table[(h->ylen)*x+yl1];
    len = h->hlen[(h->ylen)*x+yl1];
    putbits(bs,huffbits,len);
    linbitsY = y-yl1;
    if (linbitsY > h->linmax) {
      fprintf(stderr,"warning: Huffman Y table overflow\n");
      linbitsY = h->linmax;
    };
    if (h->linbits) {
       putbits(bs,linbitsY,h->linbits);
    }
  }
}

int huffman_decoder(h, x, y, v, w)
struct huffcodetab *h;
int *x;
int *y;
int *v;
int *w;
{  
  HUFFBITS level;
  int point = 0;
  int error = 1;
  level     = dmask;
  if (h->val == NULL) return 2;

  if ( h->treelen == 0)
  {  *x = *y = 0;
     return 0;
  }

  do {
    if (h->val[point][0]==0) {
      *x = h->val[point][1] >> 4;
      *y = h->val[point][1] & 0xf;

      error = 0;
      break;
    } 
    if (hget1bit()) {
      while (h->val[point][1] >= MXOFF) point += h->val[point][1]; 
      point += h->val[point][1];
    }
    else {
      while (h->val[point][0] >= MXOFF) point += h->val[point][0]; 
      point += h->val[point][0];
    }
    level >>= 1;
  } while (level  || (point < ht->treelen) );
  
  if (error) {
    printf("Illegal Huffman code in data.\n");
    *x = (h->xlen-1 << 1);
    *y = (h->ylen-1 << 1);
  }

  if (h->tablename[0] == '3'
      && (h->tablename[1] == '2' || h->tablename[1] == '3')) {
     *v = (*y>>3) & 1;
     *w = (*y>>2) & 1;
     *x = (*y>>1) & 1;
     *y = *y & 1;

     if (*v)
        if (hget1bit() == 1) *v = -*v;
     if (*w)
        if (hget1bit() == 1) *w = -*w;
     if (*x)
        if (hget1bit() == 1) *x = -*x;
     if (*y)
        if (hget1bit() == 1) *y = -*y;
     }
     
  else {
  
     if (h->linbits)
       if ((h->xlen-1) == *x) 
         *x += hgetbits(h->linbits);
     if (*x)
        if (hget1bit() == 1) *x = -*x;
     if (h->linbits)	  
       if ((h->ylen-1) == *y)
         *y += hgetbits(h->linbits);
     if (*y)
        if (hget1bit() == 1) *y = -*y;
     }
	  
  return error;  
}
