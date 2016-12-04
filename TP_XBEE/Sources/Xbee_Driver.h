/*
 * Xbee_Driver.h
 *
 *  Created on: Nov 3, 2016
 *      Author: cheik
 */

#ifndef SOURCES_XBEE_DRIVER_H_
#define SOURCES_XBEE_DRIVER_H_

// DIGIMESH API COMMANDS
#define DM_API_CMD_AT_CMD							0x08
#define DM_API_CMD_AT_CMD_QUEUE_REGISTER_VALUE		0x09
#define DM_API_CMD_TX_REQUEST						0x10
#define DM_API_CMD_EXPLICIT_ADDRESSING_CMD_FRAME	0x11
#define DM_API_CMD_REMOTE_AT_CMD					0x17
#define DM_API_CMD_AT_CMD_RESPONSE					0x88
#define DM_API_CMD_MODEM_STATUS						0x8A
#define DM_API_CMD_TX_STATUS						0x8B
#define DM_API_CMD_ROUTE_INFO_PACKET				0x8D
#define DM_API_CMD_AGGREGATE_ADDRESSING_UPDATE		0x8E
#define DM_API_CMD_RECEIVE_PACKET					0x90
#define DM_API_CMD_EXPLICIT_RX_INDICATOR			0x91
#define DM_API_CMD_NODE_ID_INDICATOR				0x95
#define DM_API_CMD_REMOTE_AT_CMD_RESPONSE			0x97

#define XB_FRAME_START_DELIMITER 					0x7E
#define XB_PAYLOAD_MAX_SIZE							100 // For Xbee Series 1
#define TX_HEADER_LENGTH							18
#define RX_HEADER_LENGTH							16

#include "stdint.h"
#include "uart_xb.h"

typedef struct xbee_tx_frame_header {
	uint8_t			frame_type;				///< TYPE IS DIGIMESH TX_REQUEST (0x10)
	uint8_t			frame_id;
	uint64_t		destination_address;
	uint16_t		network_address_be;
	uint8_t			broadcast_radius;		///< set to 0 for maximum hop value
	uint8_t			options;
} xbee_tx_frame_header;



typedef struct xbee_rx_frame_header {
	uint8_t			frame_type;				///< RECEIVE_REQUEST (0x90)
	uint64_t		source_address;
	uint16_t		network_address_be;
	uint8_t			options;
} xbee_rx_frame_header;


typedef struct xbee_tx_frame {
	uint8_t 				start_delimiter;
	uint8_t					length_msb;
	uint8_t					length_lsb;
	xbee_tx_frame_header 	tx_header;
	uint8_t 				checksum;
	uint8_t					payload[XB_PAYLOAD_MAX_SIZE]; // max size
	uint8_t					payload_size;
	uint8_t					raw[XB_PAYLOAD_MAX_SIZE+TX_HEADER_LENGTH];

} xbee_tx_frame;

typedef struct xbee_rx_frame {
	uint8_t					start_delimiter;
	uint8_t					length_msb;
	uint8_t					length_lsb;
	xbee_rx_frame_header	rx_header;
	uint8_t					payload[XB_PAYLOAD_MAX_SIZE];
	uint8_t					checksum;
	uint8_t					payload_size;
	uint8_t					raw[XB_PAYLOAD_MAX_SIZE+RX_HEADER_LENGTH];
} xbee_rx_frame;


void address_to_byte_array(uint64_t address, uint8_t *byte_array);
xbee_tx_frame_header createTxHeader(uint64_t address);
xbee_rx_frame_header createRxHeader(uint64_t address);

xbee_tx_frame createTxFrame(uint64_t address, uint8_t *data, uint8_t data_length);
xbee_rx_frame createRxFrame(uint8_t *raw_data, uint8_t data_length);

void sendFrame(uint64_t address, uint8_t *msg, uint8_t data_length);


#endif /* SOURCES_XBEE_DRIVER_H_ */
