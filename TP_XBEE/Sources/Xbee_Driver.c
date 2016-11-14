/*
 * Xbee_Driver.c
 *
 *  Created on: Nov 3, 2016
 *      Author: cheik
 */
#include "Xbee_Driver.h"



xbee_tx_frame_header createTxHeader(uint64_t address)
{
	xbee_tx_frame_header header;
	header.frame_type = DM_API_CMD_TX_REQUEST;
	header.frame_id = 0x01;
	header.destination_address = address;
	header.network_address_be = 0xFFFE;
	header.broadcast_radius = 0x0;
	header.options = 0x0;

	return header;
}

xbee_rx_frame_header createRxHeader(uint64_t address)
{
	xbee_rx_frame_header header;
	header.frame_type = DM_API_CMD_RECEIVE_PACKET;
	header.source_address = address;
	header.network_address_be = 0xFFFE;
	header.options = 0x01;

	return header;
}

void calculate_tx_checksum(xbee_tx_frame *frame)
{
	// Checksum is FF minus 8-bit sum of bytes between length and checksum
	uint8_t sum = 0;
	for (int i = 3; i < frame->raw[2] + 3; i++) {
		sum +=  frame->raw[i];
	}

	uint8_t checksum = 0xFF - sum;

	// replace dummy checksum with calculated value in raw
	frame->raw[TX_HEADER_LENGTH + frame->payload_size - 1] = checksum;

	frame->checksum = checksum;

}

void check_rx_checksum(xbee_rx_frame *frame)
{
	uint8_t sum = 0;
	for(int i = 3; i < frame->raw[2]+3; i++) {
		sum += frame->raw[i];
	}

	uint8_t checksum = 0xFF - sum;

	if(checksum == frame->raw[RX_HEADER_LENGTH + frame->payload_size -1]) {
		// Valid checksum, don't do anything
		frame->checksum = checksum;
	}
	else {
		// Signals error, must be processed by calling method.
		frame->checksum = 0xFF;
	}
}

/**
 * Extract byte array from a 64-bit number
 */
void address_to_byte_array(uint64_t address, uint8_t *byte_array)
{
	for (int i = 0; i < 8; i++) {
		*byte_array++ = ( address >> (64 - (8*(i+1))) ) & 0xFF;
	}
}

void tx_frame_generate_raw(xbee_tx_frame *frame)
{
	frame->raw[0] = frame->start_delimiter;
	frame->raw[1] = frame->length_msb;
	frame->raw[2] = frame->length_lsb;
	frame->raw[3] = frame->tx_header.frame_type;
	frame->raw[4] = frame->tx_header.frame_id;

	// address
	uint8_t address_bytes[8];
	address_to_byte_array(frame->tx_header.destination_address, address_bytes);

	for (int i = 5; i < 13; i++) {
		frame->raw[i] = address_bytes[i-5];
	}

	frame->raw[13] = (frame->tx_header.network_address_be >> 2) & 0xFF;
	frame->raw[14] = frame->tx_header.network_address_be & 0xFF;
	frame->raw[15] = frame->tx_header.broadcast_radius;
	frame->raw[16] = frame->tx_header.options;

	// Puts payload in the raw data
	// (starts at header_length -1 because checksum must be at the end)
	for (int i = 17; i < 17 + frame->payload_size; i++) {
		frame->raw[i] = frame->payload[i-17];
	}

	// dummy checksum because checksum is calculated after this method
	frame->raw[17 + frame->payload_size] = 0xFF;
}

xbee_tx_frame createTxFrame (uint64_t address, uint8_t *data, uint8_t data_length)
{
	xbee_tx_frame txFrame;

	txFrame.payload_size = data_length;
	txFrame.length_msb = 0x00;

	// Length = Number of bytes between length and checksum (Header length + Data length)
	// Header length = type (1) + ID(1) + address (8) + network(2) + broadcast(1) + options(1)
	// Header length = 14 (dec) = 0xE (hex)
	txFrame.length_lsb = data_length + 0xE;

	txFrame.start_delimiter = XB_FRAME_START_DELIMITER;

	for(int i = 0; i < data_length; i++) {
		txFrame.payload[i] = *data++;
	}

 	txFrame.tx_header = createTxHeader(address);

 	tx_frame_generate_raw(&txFrame);
 	calculate_tx_checksum(&txFrame);

 	return txFrame;

}

/**
 * Creates an actual rx_frame from raw bytes
 */
xbee_rx_frame createRxFrame(uint8_t *raw_data, uint8_t data_length)
{
	xbee_rx_frame rxFrame;

	rxFrame.start_delimiter = XB_FRAME_START_DELIMITER;
	rxFrame.length_msb = raw_data[1];
	rxFrame.length_lsb = raw_data[2];

	uint64_t address = 0;
	for(int i = 4; i < 12; i++) {
		address <<= 8;
		address |= (uint64_t)raw_data[i];
	}

	xbee_rx_frame_header header = createRxHeader(address);
	header.options = raw_data[14];
	rxFrame.rx_header = header;
	rxFrame.payload_size = data_length;
	for(int i = 0; i < data_length; i++) {
		rxFrame.payload[i] = raw_data[i+15];
	}
	for (int i = 0; i < data_length; i++) {
		rxFrame.raw[i] = raw_data[i];
	}

	// Fill checksum value if it's valid. If not valid checksum = 0xFF (signals error somewhere)
	check_rx_checksum(&rxFrame);

	return rxFrame;

}

void sendFrame(uint64_t address, uint8_t *data, uint8_t data_length)
{
	xbee_tx_frame frame = createTxFrame(address, data, data_length);

	for(int i = 0; i < (frame.payload_size + TX_HEADER_LENGTH); i++) {
		UART_DRV_SendDataBlocking(uart_xb_IDX, &frame.raw[i], 1, 10);
	}
}


