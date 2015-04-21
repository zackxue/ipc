#ifndef __BASE64___H__
#define __BASE64___H__
  
int BASE64_encode(const void *lpSoure, int cbSource, void *lpDest, int cbDest); 
int BASE64_decode(const void *lpSoure, int cbSource, void *lpDest, int cbDest); 
 

#endif
