/**
 * 	crypter.c
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 08/11/2017
 *  Description : functions needed for encryption / decryption
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypter.h"

unsigned char key_bits[4][8];
unsigned char key_order[4];
char get_bit(char num, int bit) {
	return 1 == ( (num >> bit) & 1);
}

int split_message(size_t key_length, unsigned char * message, size_t message_length, unsigned char * result) {

	int j = 0;

	if(result == NULL) {
		fputs("Memory error while trying to allocate for splitting.\n", stderr);
		return 0;
	}

	for(int i = 0 ; i < message_length; i++) {
		result[j++] = message[i] >> key_length; //shift right X bits to get the first part
		result[j] = message[i] << key_length;//shift left X bits then shift right X bits to get the second part
		result[j] = result[j] >> key_length;
		j++;
	}

	return 1;
}

unsigned char bit_add(int index, unsigned char message) {
	unsigned char bit = 0;

	for(int i = 0 ; i < 4 ; i++) {
		bit += get_bit(message,i) * key_bits[3-i][index];
	}

	return get_bit(bit,0);
}

unsigned char multiply_matrix(unsigned char message) {
	unsigned char result = 0;

	for(int i = 0 ; i < 8 ; i++) {
		result ^= (-bit_add(i,message) ^ result) & (1 << i);
	}
	return result;
}

void encrypt(unsigned char * message, size_t message_length, message_t * res) {

	res->length = 0;

	//Allocate the result
	res->array = malloc(message_length*2* sizeof(unsigned char));
	if(res->array == NULL) {
		fputs("Memory error while trying to allocate res.array for encryption.\n", stderr);
		return;
	}

	if(!split_message(4, message, message_length, res->array)) {
		fputs("Error while trying to split message.\n", stderr);
		return;
	}

	for(int i = 0 ; i < message_length*2 ; i++) {
		res->array[i] = multiply_matrix(res->array[i]);
		if(res->array[i] == -1) {
			fputs("Matrix multiplication failed.\n", stderr);
			return;
		}
		res->length++;
	}

}

void decrypt(unsigned char * message, size_t message_length, message_t * res) {

	unsigned char byte;
	res->length = 0;

	res->array = malloc((message_length/2)*sizeof(unsigned char));
	if(res->array == NULL) {
		fputs("Memory error while trying to allocate for decryption.\n", stderr);
		return;
	}

	for(int i = 0 ; i < message_length ; i++) {
		byte = 0;

		int k = 0;
		for(int j = 7 ; j >= 4 ; j--) {
			byte ^= (-get_bit(message[i],7-key_order[k]) ^ byte) & (1 << j);
			k++;
		}
		i++;
		k = 0;
		for(int j = 7 ; j >= 4 ; j--) {
			byte ^= (-get_bit(message[i],7-key_order[k]) ^ byte) & (1 << (j-4));
			k++;
		}

		res->array[res->length++] = byte;
	}

}

void swap_col(unsigned char * key, int col1, int col2) {
	char bit1, bit2;
	for(int i = 0 ; i < 4 ; i++) {
		bit1 = get_bit(key[i], 7-col1);
		bit2 = get_bit(key[i], 7-col2);
		key[i] ^= (-bit2 ^ key[i]) & (1 << (7-col1));
		key[i] ^= (-bit1 ^ key[i]) & (1 << (7-col2));
	}
}

int find_col(unsigned char* key, int j) {
	char ok = 1;
	for(int col = 0 ; col < 8 ; col++) {
		ok = 1;
		for(int i = 0 ; i < 4 ; i++) {

			if((i == j && get_bit(key[i],7-col) != 1) || (i != j && get_bit(key[i],7-col) != 0)) {
				ok = 0;
				break;
			}
		}

		if(ok) return col;

	}
	return -1;
}

int sort_key(unsigned char* key) {
	char ok = 1;
	int j = 0;
	for(int col = 0 ; col < 4 ; col++) {
		ok = 1;
		for(int i = 0 ; i < 4 ; i++) {
			if((i == j && get_bit(key[i],7-col) != 1) || (i != j && get_bit(key[i],7-col) != 0)) {
				ok = 0;
				break;
			}
		}

		if(!ok) {
			int to_swap = find_col(key,j);
			if(to_swap == -1) {
				fputs("Invalid key\n", stderr);
				return 0;
			}
			key_order[col] = to_swap;
			//swap_col(key,col,to_swap);
		} else
			key_order[col] = col;

		j++;
	}

	for(int i = 0 ; i < 4 ; i++) {
		for(int j = 0 ; j < 8 ; j++) {
			key_bits[i][j] = get_bit(key[i],j);
		}
	}
	return 1;
}


