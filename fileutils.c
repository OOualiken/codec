/**
 * 	fileutils.c
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 14/11/2017
 *  Description : helpful functions for file manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "fileutils.h"
#include "crypter.h"

char * strsubstr(char * str , int from, int count) {
	char * result;

	if(str == NULL) return NULL;

	result = malloc((count+1) * sizeof(char));

	if(result == NULL) return NULL;

	strncpy(result, str+from, count);
	return result;
}

char * read_txt_file(char* filename) {
	FILE* file;
	int size;
	char* content;

	if ((file = fopen(filename, "r")) == NULL) {
		fputs("Can't open txt file\n", stderr);
		return NULL;
	}

	fseek(file, 0L, SEEK_END); //Getting file size
	size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	content = malloc(size+1);
	if(content == NULL) {
		fputs("Error while allocating buffer for txt file reading.\n", stderr);
		return NULL;
	}
	fread( content , size, 1 , file); //Read the file
	content[size] = '\0';
	return content;
}

unsigned char * get_key(char * string) {

	if(string == NULL) return NULL;

	unsigned char * result = malloc(4);
	if(result == NULL) {
		fputs("Error while allocating for key parsing.\n", stderr);
		return 0;
	}
	int length = strlen(string);
	int i;
	int j = 0; //Array index
	int value; //Parsed substring converted to int using atoi
	char * sub; //Substring for each byte in the input string

	for(i = 5 ; i < length-2 ; i+=9) {

		sub = strsubstr(string, i, 8);
		if(sub == NULL) {
			fputs("Substring error\n", stderr);
			return NULL;
		}
		value = (int) strtol(sub, NULL, 2); //Parse the substring to binary

		if(value == 0) {
			fputs("Parsing error\n", stderr);
			free(sub);
			return NULL;
		}
		free(sub);

		result[j++] = (unsigned char) value;
	}


	return result;
}

int lastIndexOf(char * str, char chr) {

	int length = strlen(str);

	for(int i = length-1 ; i > 0 ; i--) {
		if(str[i] == chr)
			return i;
		if(str[i] == '.' || str[i] != 'd') return -1;
	}
	return -1;
}

char * generate_crypted_filename(char * filename) {
	char* filenamecpy;
	int len;
	len = strlen(filename);
	filenamecpy = malloc(len+2);
	if(filenamecpy == NULL) {
		fputs("Error while allocating for crypted filename generation.\n", stderr);
		return NULL;
	}
	strcpy(filenamecpy,filename);
	strcat(filenamecpy, "c"); //Append 'c' to filenamecpy
	return filenamecpy;
}

char * generate_decrypted_filename(char * filename) {
	char* filenamecpy;
	int len, index;
	len = strlen(filename);
	filenamecpy = malloc(len+1);
	if(filenamecpy == NULL) {
		fputs("Error while allocating for decrypted filename generation.\n", stderr);
		return NULL;
	}
	strcpy(filenamecpy,filename);

	index = lastIndexOf(filename, 'c');
	if(index >= 0)
		filenamecpy[index] = 'd'; //Replaces the 'c' with a 'd'
	else
		return NULL;
	return filenamecpy;
}

file_cutting_t cut_file_threaded(FILE* file, int thread_count) {
	file_cutting_t cut;
	cut.size = -1;
	cut.parts = -1;
	cut.rest = -1;

	if(file == NULL) return cut;

	//Getting size
	fseek(file, 0L, SEEK_END);
	cut.size = ftell(file);

	fseek(file, 0L, SEEK_SET); //Rewind to file start
	cut.parts = cut.size / thread_count;
	cut.rest = cut.size % thread_count;
	return cut;
}

file_cutting_t cut_file_decrypt_threaded(FILE* file, int thread_count) {
	file_cutting_t cut;
	cut.size = -1;
	cut.parts = -1;
	cut.rest = -1;

	if(file == NULL) return cut;

	//Getting size
	fseek(file, 0L, SEEK_END);
	cut.size = ftell(file);

	fseek(file, 0L, SEEK_SET); //Rewind to file start
	cut.parts = cut.size / thread_count;
	cut.rest = cut.size % thread_count;
	if(cut.parts % 2 != 0) {
		cut.parts--;
		cut.rest += 8;
	}
	return cut;
}

void* decrypt_threaded(void * args) {
	struct crypting_args * param = args;
	message_t * message = malloc(sizeof(message_t));

	FILE * file;
	if ((file = fopen(param->filename, "rb")) == NULL) {
		fputs("Can't open file\n", stderr);
		return message;
	}
	fseek(file, param->start, SEEK_SET); //Start at the correct offset

	unsigned char * buf = malloc(param->buffer_size);
	fread(buf, sizeof(unsigned char), param->buffer_size, file);
	decrypt(buf,param->buffer_size, message);
	fclose(file);
	free(buf);
	free(param);
	return message;
}

int decrypt_file_threaded(unsigned char* key, char * filename) {
	pthread_t threads[9] = {0};
	FILE * file;
	FILE * filec;
	file_cutting_t cut;
	message_t * message;
	char * filenamecpy;
	int ret;

	if ((file = fopen(filename, "rb")) == NULL) {
		fputs("Can't open file\n", stderr);
		return 0;
	}

	filenamecpy = generate_decrypted_filename(filename);
	if(filenamecpy == NULL)	return 0;

	if ((filec = fopen(filenamecpy, "wb")) == NULL) {
		fputs("Can't create copy file\n", stderr);
		return 0;
	}

	cut = cut_file_decrypt_threaded(file, 8);
	fclose(file);
	if(cut.parts < 0)
		return 0;

	if(cut.parts > 0) {
		for(int i = 0 ; i < 8 ; i++) {

			//Create thread
			struct crypting_args * args = malloc(sizeof(struct crypting_args));
			args->buffer_size = cut.parts;
			args->filename = strdup(filename);
			args->key = key;
			args->start = cut.parts*i;

			ret = pthread_create(&threads[i], NULL, decrypt_threaded, (void*)args);

			if(ret)
				fprintf(stderr, "%s\n", strerror(ret));
		}
	}

	if(cut.rest > 0) {
		struct crypting_args * args = malloc(sizeof(struct crypting_args));
		args->buffer_size = cut.rest;
		args->filename = strdup(filename);
		args->key = key;
		args->start = cut.parts*8;

		ret = pthread_create(&threads[8], NULL, decrypt_threaded, (void*)args);
		if(ret)
			fprintf(stderr, "%s\n", strerror(ret));
	}

	for(int i = 0 ; i < 9 ; i++) {
		if(threads[i] != 0) {
			pthread_join(threads[i],(void*)&message);
			fwrite(message->array, sizeof(unsigned char), message->length, filec);
			free(message->array);
			free(message);
		}
	}

	fclose(filec);
	free(filenamecpy);
	return 1;
}

void* encrypt_threaded(void * args) {
	struct crypting_args * param = args;
	message_t * message = malloc(sizeof(message_t));

	FILE * file;
	if ((file = fopen(param->filename, "rb")) == NULL) {
		fputs("Can't open file\n", stderr);
		return message;
	}
	fseek(file, param->start, SEEK_SET); //Start at the correct offset

	unsigned char * buf = malloc(param->buffer_size);
	fread(buf, sizeof(unsigned char), param->buffer_size, file);
	encrypt(buf,param->buffer_size, message);
	fclose(file);
	free(buf);
	free(param);
	return message;
}

int encrypt_file_threaded(unsigned char* key, char * filename) {
	pthread_t threads[8] = {0};
	FILE * file;
	FILE * filec;
	file_cutting_t cut;
	message_t * message;
	char * filenamecpy;
	int ret;

	if ((file = fopen(filename, "rb")) == NULL) {
		fputs("Can't open file\n", stderr);
		return 0;
	}

	filenamecpy = generate_crypted_filename(filename);
	if(filenamecpy == NULL)	return 0;

	if ((filec = fopen(filenamecpy, "wb")) == NULL) {
		fputs("Can't create copy file\n", stderr);
		return 0;
	}

	cut = cut_file_threaded(file, 7);
	fclose(file);

	if(cut.parts > 0) {
		for(int i = 0 ; i < 7 ; i++) {

			//Create thread
			struct crypting_args * args = malloc(sizeof(struct crypting_args));
			args->buffer_size = cut.parts;
			args->filename = strdup(filename);
			args->key = key;
			args->start = cut.parts*i;

			ret = pthread_create(&threads[i], NULL, encrypt_threaded, (void*)args);

			if(ret)
				fprintf(stderr, "%s\n", strerror(ret));
		}
	}

	if(cut.rest > 0) {
		struct crypting_args * args = malloc(sizeof(struct crypting_args));
		args->buffer_size = cut.rest;
		args->filename = strdup(filename);
		args->key = key;
		args->start = cut.parts*7;

		ret = pthread_create(&threads[7], NULL, encrypt_threaded, (void*)args);
		if(ret)
			fprintf(stderr, "%s\n", strerror(ret));
	}

	for(int i = 0 ; i < 8 ; i++) {
		if(threads[i] != 0) {
			pthread_join(threads[i],(void*)&message);
			fwrite(message->array, sizeof(unsigned char), message->length, filec);
			free(message->array);
			free(message);
		}
	}

	fclose(filec);
	free(filenamecpy);
	return 1;
}
