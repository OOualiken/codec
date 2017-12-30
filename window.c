/**
 * 	window.c
 *
 *  Author : Ourdia Oualiken and Jérémy LAMBERT
 *  Date : 19/11/2017
 *  Description : manages the graphical interface
 */
#include "window.h"
#include "crypter.h"
#include "fileutils.h"
#include <stdlib.h>
#include <sys/time.h>

#define BUFFER_SIZE 10024000

GtkWidget *window;
GtkWidget *button_encrypt;
GtkWidget *button_decrypt;
char key_ok = 0;
char file_ok = 0;
unsigned char * key;
char * key_filename;
char * file_filename;

static void window_show_error(char * message) {
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}

static void window_show_result(long long time, char encrypt) {
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s effectué en %lld ms.", encrypt ? "Chiffrement" : "Déchiffrement" , time);
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}

static void update_action_buttons() {
	gtk_widget_set_sensitive(button_encrypt, key_ok && file_ok);
	gtk_widget_set_sensitive(button_decrypt, key_ok && file_ok);
}

static void start_encrypt() {

	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

	if(!encrypt_file_threaded(key,file_filename)) {
		window_show_error("Erreur lors du chiffrement.");
		return;
	}

	struct timeval te1;
	gettimeofday(&te1, NULL); // get current time
	milliseconds = te1.tv_sec*1000LL + te1.tv_usec/1000 - milliseconds; //Get the execution time
	window_show_result(milliseconds, 1);
}

static void start_decrypt() {

	char * filenamecpy = generate_decrypted_filename(file_filename);
	if(filenamecpy == NULL) {
		gtk_widget_set_sensitive(button_decrypt, 0);
		window_show_error("Le fichier a décrypter a un nom invalide.");
		return;
	}
	free(filenamecpy);

	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

	if(!decrypt_file_threaded(key,file_filename)) {
		window_show_error("Erreur lors du déchiffrement.");
		return;
	}

	struct timeval te1;
	gettimeofday(&te1, NULL); // get current time
	milliseconds = te1.tv_sec*1000LL + te1.tv_usec/1000 - milliseconds; //Get the execution time
	window_show_result(milliseconds, 0);
}

static void handle_key(GtkFileChooser *widget) {
	key_ok = 0;

	if(key_filename != NULL)
		g_free(key_filename);

	key_filename = gtk_file_chooser_get_filename (widget);

	if(key_filename == NULL) {
		window_show_error("Aucun fichier sélectionné.");
		return;
	}

	char * raw_key = read_txt_file(key_filename);

	if(raw_key == NULL) {
		window_show_error("Erreur lors de la lecture du fichier.");
		return;
	}

	key = get_key(raw_key);
	free(raw_key);

	if(key == NULL || !sort_key(key)) {

		if(key != NULL) free(key);
		window_show_error("Clé invalide.");
		return;
	}

	key_ok = 1;
	update_action_buttons();
}

static void handle_file(GtkFileChooser *widget) {
	file_ok = 0;

	if(file_filename != NULL)
		g_free(file_filename);

	file_filename = gtk_file_chooser_get_filename (widget);

	if(file_filename == NULL) {
		window_show_error("Fichier invalide.");
		return;
	}
	file_ok = 1;
	update_action_buttons();
}

static void window_activate (GtkApplication* app, gpointer user_data) {
	GtkWidget *grid;
	GtkWidget *button;
	GtkWidget *label;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "JO Crypter");
	gtk_window_set_resizable(GTK_WINDOW(window), 0);

	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID(grid), 5);
	gtk_container_add (GTK_CONTAINER (window), grid);

	label = gtk_label_new("Sélectionner la clé : ");
	gtk_label_set_xalign (GTK_LABEL(label), .0);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 2, 1);

	button = gtk_file_chooser_button_new ("Sélectionner la clé", GTK_FILE_CHOOSER_ACTION_OPEN);
	g_signal_connect (button, "file-set", G_CALLBACK (handle_key), NULL);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button), getenv("HOME"));

	gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);

	label = gtk_label_new("Sélectionner le fichier : ");
	gtk_label_set_xalign (GTK_LABEL(label), .0);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 2, 1);

	button = gtk_file_chooser_button_new ("Sélectionner le fichier", GTK_FILE_CHOOSER_ACTION_OPEN);
	g_signal_connect (button, "file-set", G_CALLBACK (handle_file), NULL);

	gtk_grid_attach (GTK_GRID (grid), button, 0, 3, 2, 1);


	button_encrypt = gtk_button_new_with_label ("Chiffrer");
	gtk_widget_set_sensitive(button_encrypt, 0);
	g_signal_connect (button_encrypt, "clicked", G_CALLBACK (start_encrypt), NULL);

	gtk_grid_attach (GTK_GRID (grid), button_encrypt, 0, 4, 1, 1);


	button_decrypt = gtk_button_new_with_label ("Déchiffrer");
	gtk_widget_set_sensitive(button_decrypt, 0);
	g_signal_connect (button_decrypt, "clicked", G_CALLBACK (start_decrypt), NULL);

	gtk_grid_attach (GTK_GRID (grid), button_decrypt, 1, 4, 1, 1);

	gtk_widget_show_all (window);
}

int window_start (int argc, char **argv) {
	GtkApplication *app;
	int status;

	app = gtk_application_new ("fr.sysgli.jocrypter", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (window_activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	if(key_filename != NULL)
		g_free(key_filename);
	if(file_filename != NULL)
		g_free(file_filename);
	free(key);

	return status;
}
