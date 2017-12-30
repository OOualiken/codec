/**
 * 	crypter.h
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 08/11/2017
 *  Description : functions needed for encryption / decryption
 */

#ifndef CRYPTER_H_
#define CRYPTER_H_

typedef struct message {
	int length;
	unsigned char* array;
} message_t;

/**
 * Returns the value of the nth bit of the given char
 * @param char num
 * @param int bit , the bit index
 */
char get_bit(char num, int bit);

/**
 * Operates a single bit operation in the matrix multiplication
 */
unsigned char bit_add(int index, unsigned char message);

/**
 * Splits the message into the correct amount of values for encryption. Returns 0 if the split failed.
 * @param size_t key_length
 * @param unsigned char * message , the message matrix
 * @param size_t message_length
 * @param unsigned char * result , pointer to the result matrix (should already be allocated)
 * @return 1 if success, 0 if fail
 */
int split_message(size_t key_length, unsigned char * message, size_t message_length, unsigned char * result);

/**
 * Multiply the matrix message by the matrix key and fills the matrix result with the result. Returns -1 if the multiplication is impossible.
 * @param unsigned char * key , the matrix key
 * @param size_t key_length
 * @param unsigned char message
 * @return the result, -1 if fail
 */
unsigned char multiply_matrix(unsigned char message);

/**
 * Encrypts the matrix message by the matrix key and fills the matrix result with the result. Return 0 if the encryption failed.
 * @param unsigned char * message , the message matrix
 * @param size_t message_length
 * @param message_t * res , the result message
 */
void encrypt(unsigned char * message, size_t message_length, message_t * res);

/**
 * Decrypts the matrix message by the matrix key and fills the matrix result with the result. Return 0 if the decryption failed.
 * @param unsigned char * message , the message matrix
 * @param size_t message_length
 * @param message_t * res , the result message
 */
void decrypt(unsigned char * message, size_t message_length, message_t * res);

/**
 * Sorts the given key. Returns 1 if it's a success, 0 if the key is invalid.
 * @param unsigned char* key
 * @return 1 if it's a success, 0 if the key is invalid.
 */
int sort_key(unsigned char* key);

/**
 * Finds the column which is supposed to be at the j index in an ordered key matrix.
 * @param unsigned char* key
 * @param int j
 * @return int index
 */
int find_col(unsigned char* key, int j);

/**
 * Swaps two columns of the matrix key.
 * @param unsigned char * key
 * @param int col1
 * @param int col2
 */
void swap_col(unsigned char * key, int col1, int col2);

#endif /* CRYPTER_H_ */
