/**
 * 	fileutils.h
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 14/11/2017
 *  Description : helpful functions for file manipulation
 */

#ifndef FILEUTILS_H_
#define FILEUTILS_H_

struct crypting_args {
	unsigned char* key;
	int buffer_size;
	char * filename;
	long int start;
};

typedef struct file_cutting {
	long int parts;
	long int rest;
	long int size;
} file_cutting_t;

/**
 * Substrings in a new string, handles \0. Returns NULL in cas of error
 * @param char* str , the string to substring
 * @param int from , the start index included
 * @param int count , the amount of characters kept
 */
char * strsubstr(char * str , int from, int count);

/**
 * Reads a full text file and returns the result in a string. NULL if an error occurred. SHOULD BE ONLY BE USED ON SMALL FILES.
 * @param char* filename
 * @return char* content
 */
char * read_txt_file(char* filename);

/**
 * Parses the given string into a 4 char array. Returns 0 if an error occurred
 * @param char * string
 * @return the key array
 */
unsigned char * get_key(char * string);

/**
 * Gets the size of the file and performs a calculation of how many parts it needs to be split according to the amount of planned threads.
 * @param FILE* file
 * @param int thread count
 * @return file_cutting_t the result values, values are set to -1 if file is null
 */
file_cutting_t cut_file_threaded(FILE* file, int thread_count);

file_cutting_t cut_file_decrypt_threaded(FILE* file, int thread_count);

/**
 * Finds the last index of the given char in the string. Return -1 if not found.
 */
int lastIndexOf(char * str, char chr);

/**
 * Generates a name for the encrypted file. Appends a 'c' at the end of the original file name. Handles '\0'.
 * @param char* filename
 * @return char* result, exemple: for an input "file.txt", the result will be "file.txtc"
 */
char * generate_crypted_filename(char * filename);

/**
 * Generates a name for the encrypted file. Replaces the 'c' at the end of the original file name with a 'd'. Handles '\0'.
 * @param char* filename
 * @return char* result, exemple: for an input "file.txtc", the result will be "file.txtd"
 */
char * generate_decrypted_filename(char * filename);

/**
 * The function handled by a single thread for decryption.
 * @param void * args should be a struct crypting_args
 */
void* decrypt_threaded(void * args);

/**
 * Reads the source file "filename", decrypts it according to the key and writes the result into a new file. Uses 8 threads.
 * @param unsigned char* key , a 4 values array
 * @param char* filename
 */
int decrypt_file_threaded(unsigned char* key, char * filename);

/**
 * The function handled by a single thread for encryption.
 * @param void * args should be a struct crypting_args
 */
void* encrypt_threaded(void * args);

/**
 * Reads the source file "filename", encrypts it according to the key and writes the result into a new file. Uses 8 threads.
 * @param unsigned char* key , a 4 values array
 * @param char* filename
 */
int encrypt_file_threaded(unsigned char* key, char * filename);

#endif /* FILEUTILS_H_ */
