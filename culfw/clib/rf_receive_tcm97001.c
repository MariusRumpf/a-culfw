/* 
 * Copyright by B.Hempel
 * License: GPL v2
 */

#include "rf_receive_tcm97001.h"
#include "display.h"

uint8_t mirror(uint8_t a )
 {
   a = ((a >> 4) & 0x0F0F) | ((a << 4) & 0xF0F0);
   a = ((a >> 2) & 0x3333) | ((a << 2) & 0xCCCC);
   a = ((a >> 1) & 0x5555) | ((a << 1) & 0xAAAA);

   return a;
 }

/*
 * Description in header
 */
void analyze_tcm97001(bucket_t *b, uint8_t *datatype, uint8_t *obuf, uint8_t *oby)
{
#ifdef HAS_TCM97001
  if (IS433MHZ && *datatype == 0) {
    
    if ((b->byteidx != 3 && b->byteidx != 4) || (b->bitidx != 7 && b->bitidx != 3) || b->state != STATE_TCM97001) {
		  return;
    }
    if (b->byteidx == 4 && b->bitidx == 3) {
      // Protocoll 2
//      DS("Protocoll 2 found!\n");
      b->state = STATE_TCM97001_P2;
      //DU(b->byteidx, 2);
      //b->byteidx++;
    }
    uint8_t i;
    uint8_t checksum = 0;
    
    //DU(b->byteidx, 2);
    for (i=0;i<b->byteidx;i++) {
      obuf[i]=b->data[i];
      if (b->state == STATE_TCM97001_P2) {
        //DU(i, 2);
        //DH2(b->data[i]);
        
        uint8_t value = mirror(b->data[i]);
        //if (i<4) {
          checksum += (value>>4) + (value & 15);
        //}
        //DH2(mirror(b->data[i]));
      }
    }
    if (b->state == STATE_TCM97001_P2) {
     // DC('!');
     // DU(checksum,        4);
      checksum &= 15;
    //  DU(checksum,        4);
      uint8_t crcVal = mirror(b->data[4]);
      obuf[4]=b->data[4];
      i++;
     // DU(crcVal,        4);
      crcVal += checksum;
    //  DU(crcVal,        4);
    //  DC(':');
      if (crcVal == 15) {
        // CRC OK
        *oby = i;
        *datatype = TYPE_TCM97001;
      }
    } else {
      *oby = i;
      *datatype = TYPE_TCM97001;
    }
  }
#endif
}

/*
 * Description in header
 */
uint8_t sync_tcm97001(bucket_t *b, pulse_t *hightime, pulse_t *lowtime) 
{
#ifdef HAS_TCM97001
  if (IS433MHZ && b->state == STATE_TCM97001 && b->sync == 0) {
	  b->sync=1;
		b->zero.hightime = *hightime;
		b->one.hightime = *hightime;

		if (*lowtime < 187) { // < 3000=187 156=2500
			b->zero.lowtime = *lowtime;
			b->one.lowtime = b->zero.lowtime*2;
		} else {
			b->zero.lowtime = *lowtime;
			b->one.lowtime = b->zero.lowtime/2;
		}
    return 1;
	}
#endif
  return 0;
}

/*
 * Description in header
 */
uint8_t is_tcm97001(bucket_t *b, pulse_t *hightime, pulse_t *lowtime) 
{
#ifdef HAS_TCM97001
  if (IS433MHZ && (*hightime < TSCALE(540) && *hightime > TSCALE(410)) &&
				   (*lowtime  < TSCALE(9000) && *lowtime > TSCALE(8200)) ) {
		  OCR1A = 4800L;
			TIMSK1 = _BV(OCIE1A);
			b->sync=0;
      
			b->state = STATE_TCM97001;
			b->byteidx = 0;
			b->bitidx  = 7;
			b->data[0] = 0;  
		  return 1;
	}
#endif
  return 0;
}

/*
 * Description in header
 */
uint8_t addbit_tcm97001(bucket_t *b, pulse_t *hightime, pulse_t *lowtime) 
{
#ifdef HAS_TCM97001
  if (IS433MHZ && b->state==STATE_TCM97001) {
    if (*lowtime < 156) { // < 3000=187 156=2500
      addbit(b,0);
      //b->zero.hightime = makeavg(b->zero.hightime, *hightime);
      //b->zero.lowtime  = makeavg(b->zero.lowtime,  *lowtime);
    } else {
      addbit(b,1);
      //b->one.hightime = makeavg(b->one.hightime, *hightime);
      //b->one.lowtime  = makeavg(b->one.lowtime,  *lowtime);
   /* } else {
      DC('?');
      DU(b->zero.lowtime*16, 4);
      DC(':');
      DU(b->one.lowtime*16, 4);
      DC('?');
      DC('!');
      DU(*hightime*16, 4);
      DC(':');
      DU(*lowtime*16, 4);
      DC('!');*/
    }
    return 1;
  }
#endif
  return 0;
}

