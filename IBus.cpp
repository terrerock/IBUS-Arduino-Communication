#include <sys/_stdint.h>
#include "api/Common.h"

#include "IBus.h"

//-----------------------------------------------------------------------------
// Constructor rc_ibus_c class
//-----------------------------------------------------------------------------
rc_ibus_c::rc_ibus_c( HardwareSerial& UART, const uint32_t baud ) : 
                       serial_port( UART ), baud_bps( baud )
{
}

//-----------------------------------------------------------------------------
// Destructor rc_ibus_c class
//-----------------------------------------------------------------------------
rc_ibus_c::~rc_ibus_c()
{
}

//-----------------------------------------------------------------------------
// update_buffer method simply invokes the private method handle_serial_data
// to update the buffer.
//-----------------------------------------------------------------------------
ibus_status_t rc_ibus_c::update_buffer( void )
{
    return rc_ibus_c::handle_serial_data();
}

//-----------------------------------------------------------------------------
// verify_checksum verifies the calculate checksum is equal to the checksum
// transmited by the sender by substrating the sum of all 30-byte in the
// packet from 0xFFFF then compare the result with the received checksum.
// returns 0 if checksum is invalid otherwise returns the calculated checksum
//-----------------------------------------------------------------------------
bool rc_ibus_c::verify_checksum( void )
{
   bool status = false;
   uint16_t recv_checksum = rc_ibus_c::get_recv_check_sum();
   uint16_t sum_recv_bytes = rc_ibus_c::calc_buf_sum();
   uint16_t calc_checksum = 0xFFFF - sum_recv_bytes;

   if( calc_checksum == recv_checksum ) {
      status = true;
   }
   return status;
}

//-----------------------------------------------------------------------------
// get_eng_unit_item method returns the engineering unit value requested.
//
// requested             returns
// ---------             -------
// index =  0       ---> HEADER
// index =  1 to 14 ---> CHAN 01 to CHAN 14
// index = 15.      ---> CHECKSUM 
//-----------------------------------------------------------------------------
uint32_t rc_ibus_c::get_eng_unit_item( const uint8_t index )
{
  uint32_t eng_value = 0;
  uint8_t  eng_count = rc_ibus_c::get_eng_unit_item_count();

  if( index >= 0 && index < eng_count ) {
    eng_value = eng_unit_buffer[ index ];
  } 
  return eng_value;
}

//-----------------------------------------------------------------------------
// begin method starts the UART serial bus with 8 bit data byte, no parity,
// and one stop bit (SERIAL_8N1) with the IBUS baud rate of 115200 bps.
//-----------------------------------------------------------------------------
int32_t rc_ibus_c::begin(void)
{  
  int32_t status = 0;
  serial_port.begin( baud_bps, SERIAL_8N1 );  // Serial1.begin( 115200, default config ) equilvalent in setup()
  return status;
}

//-----------------------------------------------------------------------------
// handle_serial_data method searches for the header bytes then copies the raw 
// data from each received packet in the serial FIFO to the local buffer[]. The 
// checksum of each packet is also verified. 
// returns the one of the following status:
// enum serial_status {
//   UNKNOWN_STATUS = 0,
//   LOST_COMM      = 1,
//   HDR1_NOT_FOUND = 2,
//   HDR2_NOT_FOUND = 3,
//   SUCCESS        = 4,
//   BAD_CHECKSUM   = 5
// according to the output, recorded when using FlySky FS-IA 10B as an RC Receiver, the
// ibus packet (32-byte) is being transmitted at the rate of 7.7 msec (Delta column).
// therefore, it is important to invoke the method handle_serial_data method at
// frequency of 143 Hz or greater.
// HDR   Roll Pit Throt Yaw  Pot1 Pot2 Ch07 Ch08 Ch09 Ch10 Ch11 Ch12 Ch13 Ch14 Csum  Stat  Cnt Time usec  Delta
// 16416 1525 1544 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62437   1    32 25125330   7701  
// 16416 1525 1544 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62437   1    32 25133032   7702  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25140728   7696  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25148426   7698  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25156129   7703  
// 16416 1544 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62428   1    32 25163829   7700  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25171528   7699  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25179229   7701  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25186928   7699  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25194628   7700  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25202328   7700
//-----------------------------------------------------------------------------
 ibus_status_t rc_ibus_c::handle_serial_data( void )
  {
      ibus_status_t status          = UNKNOWN_STATUS;
      uint32_t   num_byte_available = 0;
      uint32_t   ts_now             = 0;
      static uint32_t   ts_prev     = 0;
      uint32_t   ts_delta           = 0;
      int32_t    data_read          = 0;
      uint32_t   start              = 0;
      uint32_t   elapse             = 0;
      size_t     byte_read_cnt      = 0;

      start = millis();
      do {
        num_byte_available = serial_port.available();          // wait here about 4.5 msec for the data to be available (7 msec - 2.7 msec = 4.3 msec idle)
        if( (millis() - start) > LOST_COMM_TIMEOUT_MSEC ) {    // 2.7 msec is required to transmit 32 bytes at 115200 bps with 1 start/stop bit and 8 data bit
          status = LOST_COMM;                                  // 7 msec is the packet period
          break;
        }
      } while( num_byte_available < PACKET_LENGTH );
 
      if( num_byte_available >= PACKET_LENGTH )     // serial FIFO has at least 32-byte availble for processing
      {
        start = millis();
        do {
          data_read = serial_port.read();                             // search for the header byte here because byte read() may not be the header byte
          if( (millis() - start) > INVALID_PROTOL_TIMEOUT_MSEC ) {    // 2.7 msec is required to transmit 32 bytes at 115200 bps with 1 start 1 stop bit and 8 data bit
            status = HDR1_NOT_FOUND;                                  // 7 msec is the packet transmission period
            break;
          }
        } while( data_read != HEADER1_BYTE );   // discard any byte that is not the header byte

        if( data_read == HEADER1_BYTE )
        {
          ts_now = micros();                  // found packet header, record the time stamp at this instant
          ts_delta = ts_now - ts_prev;  
          ts_prev = ts_now;
          buffer[HDR1_INDEX] = data_read;
          data_read = serial_port.read();

          if( data_read == HEADER2_BYTE ) {     // expected 0x40 here for HDR2
            buffer[HDR2_INDEX] = data_read;
            byte_read_cnt = serial_port.readBytes(&buffer[HDR1_INDEX + 2], PACKET_LENGTH - 2);  // found 2-byte header, read additional 30 bytes
            if( byte_read_cnt == (PACKET_LENGTH - 2) ) {
              if( rc_ibus_c::verify_checksum() == true ) {
                status = SUCCESS;
              } else {
                status = BAD_CHECKSUM;
              }
            } else { // 0 indicates no valid data was found
              status = NO_VALID_DATA;
            }
          } else {
            status = HDR2_NOT_FOUND;
          }
        }
      }  

      // store some useful data in the buffer
      *(uint16_t*)&buffer[32] = status;  // 2-byte
      *(uint16_t*)&buffer[34] = num_byte_available; // 2-byte
      *(uint32_t*)&buffer[36] = ts_now; // 4-byte to fit time stamp in micro-second
      *(uint32_t*)&buffer[40] = ts_delta; // 4- byte delta time = time stamp now - previous time stamp

      rc_ibus_c::convert_buffer_to_eng_value();

      return status;
  }

//-----------------------------------------------------------------------------
// calc_buf_sum method returns the sum of all bytes from buffer[0] to buffer[29]
//-----------------------------------------------------------------------------
uint16_t rc_ibus_c::calc_buf_sum( void )
{
  uint32_t sum_of_thirty_bytes = 0;
  for( uint32_t i = 0; i < CS1_INDEX; i++ ) {
    sum_of_thirty_bytes += buffer[i];
  }
  return sum_of_thirty_bytes;
}

//-----------------------------------------------------------------------------
// get_recv_check_sum method returns the checksum that was transmitted on the
// serial bus by the RC Receiver.
//-----------------------------------------------------------------------------
uint16_t rc_ibus_c::get_recv_check_sum( void )
{
  uint16_t checksum_received = 0;
  checksum_received = *(uint16_t *)(&buffer[CS1_INDEX]);  // get the checksum value at index 30 and 31 in buffer[]
  return checksum_received;
}

//-----------------------------------------------------------------------------
// get_eng_unit_item_count returns how many engineering values are available
// this includes 16 IBUS values and extra values used for debug and display.
//-----------------------------------------------------------------------------
uint8_t rc_ibus_c::get_eng_unit_item_count( void )
{
  return (sizeof(buffer) / 2) - 2;
}

void rc_ibus_c::convert_buffer_to_eng_value( void )
{
  // conversion from raw data to engineering values is done here. (header, roll, pitch, throttle ...)
  for( uint32_t chan_index = 0, buf_index = 0; chan_index < 18; chan_index++, buf_index += 2 ) {
      eng_unit_buffer[chan_index] = *(uint16_t *)( &buffer[buf_index] );
  }                 
  // eng_unit_buffer[ 0 .. 19 ] // elements 18 and 19 are for time stamp
                                // elements 0 .. 17 are for header->0 to byte available->17
  eng_unit_buffer[ 18 ] = *(uint32_t*)( &buffer[36] ); // convert to 4-byte time stamp
  eng_unit_buffer[ 19 ] = *(uint32_t*)( &buffer[40] ); // convert to 4-byte delta time
}


