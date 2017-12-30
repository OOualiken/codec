/**
 * 	window.h
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 19/11/2017
 *  Description : manages the graphical interface
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include<gtk/gtk.h>

/**
 * Starts the program. Returns the frame exit code.
 */
int window_start(int argc, char** argv);

#endif /* WINDOW_H_ */
