#include <string.h>
#include <stdlib.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <pthread.h>

#define PIPE_NAME "pipe_name"

/* Pipe state */
typedef struct {
    FILE *pipe;
    PanelApplet *applet;
    GtkLabel *label;
} pipe_data;

/* XML for the popup menu */
static const char context_menu_xml[] =
        "<popup name=\"button3\">\n"
        "   <menuitem name=\"File Item\" "
        "           verb=\"GnomePipeAppletSelectFile\" "
        "           _label=\"_Select file...\"\n"
        "           pixtype=\"stock\" "
        "           pixname=\"gtk-properties\"/>\n"
        "</popup>\n";

/* Show a dialog */
static void display_select_file_dialog(BonoboUIComponent *uic,
        pipe_data *data) {
    GtkWidget *dialog;

    /* Create the dialog */
    dialog = gtk_file_chooser_dialog_new ("Select File", 0,
            GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

    /* Run the dialog */
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *pipe_name;
        pipe_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        /* Set the pipe name */
        panel_applet_gconf_set_string(data->applet, PIPE_NAME, pipe_name);
        if(data->pipe) fclose(data->pipe);
        data->pipe = 0;

        g_free(pipe_name);
    }

    gtk_widget_destroy (dialog);
}

/* Bonobo verbs for the popup menu */
static const BonoboUIVerb context_menu_verbs[] = {
        BONOBO_UI_VERB ("GnomePipeAppletSelectFile",
                display_select_file_dialog),
        BONOBO_UI_VERB_END
};

/* Wait for the pipe to be opened */
void wait_for_pipe(pipe_data *data) {
    char *pipe_name;

    /* Try to open the pipe */
    while(!data->pipe || feof(data->pipe) || ferror(data->pipe)) {
        pipe_name = panel_applet_gconf_get_string(data->applet, PIPE_NAME, 0);
        if(pipe_name) {
            data->pipe = fopen(pipe_name, "r");
            g_free(pipe_name);
        }

        sleep(1);
    }
}

/* Thread running, reading from the pipe */
void *pipe_thread(void *pthread_data) {
    pipe_data *data = (pipe_data *) pthread_data;
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    int i;

    /* Run... */
    while(1) {
        /* Try to open the pipe */
        wait_for_pipe(data);

        char *result = fgets(buffer, buffer_size, data->pipe);

        if(result) {
            /* Fix end of string */
            for(i = 0; i < buffer_size; i++) {
                if(buffer[i] == '\n') buffer[i] = '\0';
            }

            /* Set label */
            gtk_label_set_text(data->label, buffer);
        }

        pthread_yield();
    }

    /* Not executed but my intentions are good */
    free(buffer);
    fclose(data->pipe);
}

static gboolean pipe_applet_fill(PanelApplet *applet, const gchar *iid,
        gpointer g_data) {
    GtkWidget *label;
    pthread_t thread;
    pipe_data *data = malloc(sizeof(pipe_data));

    if (strcmp (iid, "OAFIID:GnomePipeApplet") != 0)
        return FALSE;

    label = gtk_label_new("Waiting for pipe...");
    gtk_container_add(GTK_CONTAINER (applet), label);
    gtk_widget_show_all(GTK_WIDGET (applet));

    /* Start the pipe */
    data->pipe = 0;
    data->applet = applet;
    data->label = (GtkLabel *) label;
    pthread_create(&thread, NULL, pipe_thread, data);

    /* Setup the menu */
    panel_applet_setup_menu(applet, context_menu_xml, context_menu_verbs, data);

    return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY(
        "OAFIID:GnomePipeApplet_Factory", PANEL_TYPE_APPLET,
        "Gnome Pipe Applet", "0", pipe_applet_fill, NULL);
