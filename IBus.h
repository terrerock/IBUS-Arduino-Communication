
#ifndef rc_ibus_c_H_
#define rc_ibus_c_H_

#include <Arduino.h>

constexpr uint8_t  PACKET_LENGTH            = 32U;     // num bytes in IBUS eng_unit
constexpr uint8_t  HDR1_INDEX               = 0U;      // header 1 index
constexpr uint8_t  HDR2_INDEX               = 1U;      // header 2 index
constexpr uint8_t  CS1_INDEX                = 30U;     // checksum LSB
constexpr uint8_t  CS2_INDEX                = 31U;     // checksum MSB

constexpr uint8_t  HEADER1_BYTE             = 0x20U;   // IBUS header byte 1
constexpr uint8_t  HEADER2_BYTE             = 0x40U;   // IBUS header byte 2

constexpr uint8_t  NUM_HDR_BYTE             = 2U;      // 2-byte header ---> store as 0x00008020 in      eng_unit_buffer[0]
constexpr uint8_t  NUM_CHAN_BYTE            = 28U;     // 28-byte for eng_unit data store in 14 slots in eng_unit_butter[1..14]
constexpr uint8_t  NUM_CHKSUM_BYTE          = 2U;      // 2-byte checksum.                               eng_unit_buffer[15]
constexpr uint8_t  NUM_STATUS_BYTE          = 2U;      // 2-byte status                                  eng_unit_buffer[16]
constexpr uint8_t  NUM_BYTE_AVAILABLE       = 2U;      // 2-byte byte available initially                eng_unit_buffer[17]
constexpr uint8_t  NUM_TIMESTAMP_BYTE       = 4U;      // 4-byte time stamp                              eng_unit_buffer[18]
constexpr uint8_t  NUM_DELTA_TIME_BYTE      = 4U;      // 4-byte delta time                              eng_unit_buffer[19]
constexpr uint8_t  NUM_BYTE_IN_BUFFER       = ( NUM_HDR_BYTE +
                                                NUM_CHAN_BYTE +
                                                NUM_CHKSUM_BYTE +
                                                NUM_STATUS_BYTE +
                                                NUM_BYTE_AVAILABLE +
                                                NUM_TIMESTAMP_BYTE +
                                                NUM_DELTA_TIME_BYTE );

constexpr uint32_t STANDARD_IBUS_BAUD_BPS   = 115200U; // standard IBUS bit per second baud rate
constexpr uint16_t LOST_COMM_TIMEOUT_MSEC   = 10U;     // if packet is not received for > 10 msec, declare lost comm
constexpr uint16_t INVALID_PROTOL_TIMEOUT_MSEC   = 8U; // no header bytes are found

enum ibus_status_t {
  UNKNOWN_STATUS = 0,
  SUCCESS        = 1,
  HDR1_NOT_FOUND = 2,
  HDR2_NOT_FOUND = 3,
  LOST_COMM      = 4,
  BAD_CHECKSUM   = 5,
  NO_VALID_DATA  = 6
};

class rc_ibus_c {

  public:          

    rc_ibus_c( HardwareSerial& UART, const uint32_t baud ); 
    ~rc_ibus_c();
    int32_t        begin( void );
    bool           verify_checksum( void );
    uint32_t       get_eng_unit_item( const uint8_t chan_index );
    uint8_t        get_eng_unit_item_count( void );
    ibus_status_t  update_buffer( void );

  private:

    HardwareSerial& serial_port;
    const uint32_t  baud_bps;

    uint8_t  buffer[ NUM_BYTE_IN_BUFFER ];    // should be 44 bytes total
    uint32_t eng_unit_buffer[ (sizeof(buffer) / 2) - 2 ];           // should be (36 / 2) + (8 / 4) = 20 elements in this uint32_t array

    ibus_status_t  handle_serial_data( void );
    uint16_t       calc_buf_sum( void );
    void           get_bufferSum( void );
    uint16_t       get_recv_check_sum( void );
    void           convert_buffer_to_eng_value( void );

};

#endif