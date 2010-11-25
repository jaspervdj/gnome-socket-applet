#include <string.h>
#include <stdlib.h>
#include <panel-applet.h>
#include <gtk/gtklabel.h>
#include <pthread.h>

FILE *pipe_in;

void *pipe_thread(void *data) {
    GtkLabel *label = (GtkLabel *) data;
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    int i;

    /* Run... */
    while(1) {
        /* Try to open the pipe */
        while(!pipe_in || feof(pipe_in) || ferror(pipe_in)) {
            pipe_in = fopen("/tmp/gnome-pipe-applet", "r");
            sleep(1);
        }

        char *result = fgets(buffer, buffer_size, pipe_in);

        if(result) {
            /* Fix end of string */
            i = 0;
            while(i < buffer_size && buffer[i] != '\n' && buffer[i] != '\0') {
                i++;
            }
            if(i < buffer_size) buffer[i] = '\0';
            else if(i > 0) buffer[i - 1] = '\0';
            else buffer[0] = '\0';

            FILE *out = fopen("/tmp/debug", "a");
            fprintf(out, "'%s'\n", buffer);
            fclose(out);

            /* Set label */
            gtk_label_set_text(label, buffer);
        }

        pthread_yield();
    }

    /* Not executed but my intentions are good */
    free(buffer);
    fclose(pipe_in);
}

static gboolean pipe_applet_fill(PanelApplet *applet, const gchar *iid,
        gpointer data) {
	GtkWidget *label;
    pthread_t thread;

	if (strcmp (iid, "OAFIID:GnomePipeApplet") != 0)
		return FALSE;

	label = gtk_label_new("Waiting for pipe...");
	gtk_container_add(GTK_CONTAINER (applet), label);
	gtk_widget_show_all(GTK_WIDGET (applet));

    pipe_in = 0;
    pthread_create(&thread, NULL, pipe_thread, label);

	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY(
        "OAFIID:GnomePipeApplet_Factory", PANEL_TYPE_APPLET,
        "Gnome Pipe Applet", "0", pipe_applet_fill, NULL);
